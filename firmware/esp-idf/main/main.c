/*
 * Slot-18 Handheld Radar v0.2.0 — simulated UI.
 * Hardware radar drivers: NOT YET CONNECTED.
 */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_lv_adapter.h"
#include "lvgl.h"
#include "waveshare_rgb_lcd_port.h"

static const char *TAG = "slot18";
static lv_obj_t *dot, *target_label, *facing_label, *status_label, *coords_label, *guide_label, *pause_caption;
static lv_obj_t *b_heading,*b_pause,*b_test,*b_pass,*b_fail,*b_logs;
static uint32_t session_id=0;
static unsigned long seq=0;
static unsigned facing=0, test_step=3, pass_count=0, fail_count=0;
static float sim_seconds=0;
static bool running=true, nvs_ok=false;

static void emit(const char *category, const char *operation, const char *result)
{
    char line[250];
    unsigned long id=++seq;
    snprintf(line,sizeof(line),
     "{\"session\":\"%08" PRIx32 "\",\"seq\":%lu,\"ms\":%lld,\"clock\":\"uptime\","
     "\"category\":\"%s\",\"operation\":\"%s\",\"result\":\"%s\",\"source\":\"simulation\"}",
     session_id,id,(long long)(esp_timer_get_time()/1000),category,operation,result);
    ESP_LOGI(TAG,"%s",line);
    if (nvs_ok) {
        nvs_handle_t nvs;
        if (nvs_open("slot18",NVS_READWRITE,&nvs)==ESP_OK) {
            char key[12];
            snprintf(key,sizeof(key),"event%02lu",id%32);
            esp_err_t err=nvs_set_str(nvs,key,line);
            if (err==ESP_OK) err=nvs_commit(nvs);
            if (err!=ESP_OK) ESP_LOGW(TAG,"NVS persist failure: %s",esp_err_to_name(err));
            nvs_close(nvs);
        }
    }
}
static void logs_to_serial(void)
{
    if (!nvs_ok) {ESP_LOGW(TAG,"NVS unavailable, only live serial logs");return;}
    nvs_handle_t h;
    if (nvs_open("slot18",NVS_READONLY,&h)!=ESP_OK) {ESP_LOGE(TAG,"NVS open failed");return;}
    ESP_LOGI(TAG,"BEGIN DIAGNOSTIC EXPORT");
    for (int i=0;i<32;i++){
        char key[12];
        snprintf(key,sizeof(key),"event%02d",i);
        size_t size=0;
        if(nvs_get_str(h,key,NULL,&size)==ESP_OK && size>0 && size<512){
            char line[512];
            if(nvs_get_str(h,key,line,&size)==ESP_OK)ESP_LOGI(TAG,"DIAG %s",line);
        }
    }
    ESP_LOGI(TAG,"END DIAGNOSTIC EXPORT");
    nvs_close(h);
}
static lv_obj_t *box(lv_obj_t *parent,int x,int y,int w,int h,uint32_t color,uint32_t edge,int radius){
    lv_obj_t *obj=lv_obj_create(parent);
    lv_obj_set_pos(obj,x,y);lv_obj_set_size(obj,w,h);
    lv_obj_set_style_bg_color(obj,lv_color_hex(color),0);
    lv_obj_set_style_border_color(obj,lv_color_hex(edge),0);
    lv_obj_set_style_border_width(obj,edge?1:0,0);
    lv_obj_set_style_radius(obj,radius,0);lv_obj_set_style_pad_all(obj,0,0);
    lv_obj_remove_flag(obj,LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}
static lv_obj_t *label(lv_obj_t *p,const char *str,int x,int y,int width,uint32_t color){
    lv_obj_t *obj=lv_label_create(p);
    lv_label_set_text(obj,str);lv_obj_set_pos(obj,x,y);lv_obj_set_width(obj,width);
    lv_label_set_long_mode(obj,LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(obj,lv_color_hex(color),0);
    return obj;
}
static lv_obj_t *make_button(lv_obj_t *p,const char *caption,int x){
    lv_obj_t *b=box(p,x,63,120,42,0x1E5140,0x518F71,8);
    lv_obj_add_flag(b,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *t=label(b,caption,0,0,114,0xE6FFE9);
    lv_obj_center(t);return b;
}
static void set_guide(void) {
    static const char *instructions[]={
      "TEST 1: Sim label + moving dot? PASS/FAIL",
      "TEST 2: Facing button changes heading? PASS/FAIL",
      "TEST 3: Pause/Resume changes motion? PASS/FAIL"
    };
    if(test_step<3)lv_label_set_text(guide_label,instructions[test_step]);
    else lv_label_set_text_fmt(guide_label,"TEST: PASS %u / FAIL %u",pass_count,fail_count);
}
static void update_status(void) {
    const char *cardinal[]={"NORTH","EAST","SOUTH","WEST"};
    lv_label_set_text_fmt(facing_label,"FACING %s",cardinal[(facing/90)%4]);
    lv_label_set_text(status_label,running?"SIMULATED · RUNNING":"SIMULATED · PAUSED");
    lv_label_set_text(pause_caption,running?"Pause":"Resume");
}
static void touch_cb(lv_event_t *e) {
    lv_obj_t *o=lv_event_get_target_obj(e);
    if(o==b_heading){
        emit("USER_ACTION","heading","requested");
        facing=(facing+90)%360;emit("OPERATION_RESULT","heading","success");
    }else if(o==b_pause){
        emit("USER_ACTION","pause","requested");
        running=!running;emit("STATE_TRANSITION","pause",running?"running":"paused");
        emit("OPERATION_RESULT","pause","success");
    }else if(o==b_test){
        emit("USER_ACTION","guided_test","requested");
        test_step=0;pass_count=0;fail_count=0;set_guide();
        emit("OPERATION_RESULT","guided_test","started");
    }else if(o==b_pass || o==b_fail){
        emit("USER_ACTION","rate_step","requested");
        if(test_step<3){
            if(o==b_pass){pass_count++;emit("TEST_RESULT","guided_step","PASS_user_checked");}
            else{fail_count++;emit("TEST_RESULT","guided_step","FAIL_user_checked");}
            test_step++;set_guide();
        }else emit("OPERATION_RESULT","rate_step","ignored_no_active_test");
    }else if(o==b_logs){
        emit("USER_ACTION","export_logs","requested");
        logs_to_serial();emit("OPERATION_RESULT","export_logs",nvs_ok?"printed":"unavailable");
    }
    update_status();
}
static void tick(lv_timer_t *timer){
    (void)timer;
    if(running)sim_seconds+=0.15f;
    float x=2.1f*sinf(sim_seconds*.4f);
    float y=3.1f+0.6f*cosf(sim_seconds*.38f);
    float a=(float)facing*0.01745329252f;
    float east=x*cosf(a)+y*sinf(a);
    float north=-x*sinf(a)+y*cosf(a);
    int px=245+(int)(east*27.0f), py=178-(int)(north*27.0f);
    lv_obj_set_pos(dot,px-6,py-6);
    lv_obj_set_pos(target_label,px+12,py-13);
    lv_label_set_text_fmt(coords_label,"LD2450 DEMO\nRange %.1f m\nRight %.1f m\nForward %.1f m",
                          sqrtf(x*x+y*y),x,y);
}
static void create_ui(void){
    lv_obj_t *root=lv_screen_active();
    lv_obj_set_style_bg_color(root,lv_color_hex(0x081B17),0);
    lv_obj_remove_flag(root,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *top=box(root,10,8,780,48,0x123127,0x377257,8);
    lv_obj_t *title=label(top,"SLOT-18   HANDHELD RADAR v0.2.0",14,10,550,0xE3FFEE);
    lv_obj_set_style_text_font(title,&lv_font_montserrat_20,0);
    label(top,"SIMULATED ONLY",616,15,160,0xFFCC85);
    b_heading=make_button(root,"Facing",10);b_pause=make_button(root,"Pause",142);
    b_test=make_button(root,"Test",274);b_pass=make_button(root,"PASS",406);
    b_fail=make_button(root,"FAIL",538);b_logs=make_button(root,"LOGS",670);
    pause_caption=lv_obj_get_child(b_pause,0);
    lv_obj_t *buttons[]={b_heading,b_pause,b_test,b_pass,b_fail,b_logs};
    for(int i=0;i<6;i++)lv_obj_add_event_cb(buttons[i],touch_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_t *radar=box(root,10,114,490,356,0x081E17,0x367051,8);
    lv_obj_t *side=box(root,508,114,282,356,0x153427,0x367051,8);
    label(radar,"XY TRACKER — SIMULATION",15,11,460,0xA9F3CB);
    for (int r=30;r<=150;r+=30){
        lv_obj_t *ring=box(radar,245-r,178-r,2*r,2*r,0x081E17,0x38745B,LV_RADIUS_CIRCLE);
        lv_obj_set_style_bg_opa(ring,LV_OPA_TRANSP,0);
    }
    box(radar,245,27,1,302,0x295741,0,0);box(radar,94,178,302,1,0x295741,0,0);
    label(radar,"N",240,38,20,0xA9DEBC);label(radar,"S",240,313,20,0xA9DEBC);
    label(radar,"W",82,171,20,0xA9DEBC);label(radar,"E",397,171,20,0xA9DEBC);
    dot=box(radar,240,173,12,12,0x67E8B3,0,LV_RADIUS_CIRCLE);
    target_label=label(radar,"TARGET 1",258,160,120,0xA1EFBE);
    label(radar,"Presence-only devices do not give precise dots.",12,333,470,0xE6C184);
    status_label=label(side,"SIMULATED · RUNNING",13,15,257,0xFFCA76);
    facing_label=label(side,"FACING NORTH",13,50,257,0xA5FFDB);
    coords_label=label(side,"LD2450 DEMO",13,94,257,0xD8F5E5);
    label(side,"C4001: range only\nC4002: presence only\nREAL SENSORS: 0",13,181,256,0xA3BFB0);
    guide_label=label(side,"Press Test to start guided tests.",13,255,255,0xF8D58A);
    label(side,"UART diagnostics + NVS event ring",13,332,255,0x81C4A2);
    set_guide();update_status();
    lv_timer_create(tick,150,NULL);tick(NULL);
}
void app_main(void){
    session_id=esp_random();
    esp_err_t nr=nvs_flash_init();
    nvs_ok=(nr==ESP_OK);
    if(!nvs_ok)ESP_LOGW(TAG,"NVS not ready: %s",esp_err_to_name(nr));
    emit("OPERATION_START","display_boot","started");
    const esp_lv_adapter_rotation_t rotation=ESP_LV_ADAPTER_ROTATE_0;
    const esp_lv_adapter_tear_avoid_mode_t tear_mode=ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_RGB;
    esp_lcd_panel_handle_t panel=NULL;
    esp_lcd_touch_handle_t touch=NULL;
    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init(tear_mode,rotation,&panel,&touch));
    ESP_ERROR_CHECK(waveshare_rgb_lcd_backlight_on());
    esp_lv_adapter_config_t ac=ESP_LV_ADAPTER_DEFAULT_CONFIG();
    ac.task_stack_size=12*1024;ac.stack_in_psram=true;
    ESP_ERROR_CHECK(esp_lv_adapter_init(&ac));
    esp_lv_adapter_display_config_t dc=ESP_LV_ADAPTER_DISPLAY_RGB_DEFAULT_CONFIG(
      panel,NULL,EXAMPLE_LCD_H_RES,EXAMPLE_LCD_V_RES,rotation);
    dc.profile.use_psram=true;
    lv_display_t *display=esp_lv_adapter_register_display(&dc);
    assert(display);
    if(touch){
        esp_lv_adapter_touch_config_t tc=ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(display,touch);
        assert(esp_lv_adapter_register_touch(&tc));
    }
    ESP_ERROR_CHECK(esp_lv_adapter_start());
    if(esp_lv_adapter_lock(-1)==ESP_OK){
        create_ui();
        esp_lv_adapter_unlock();
        emit("OPERATION_RESULT","display_boot","ui_created_unverified");
    }else emit("ERROR","display_boot","lvgl_lock_failed");
}
