/*
 Slot-18: LIVE C4002 RADAR DASHBOARD ON WAVESHARE 7-INCH LCD
 Candidate S18-RADAR-LCD-V2-20261110 — I2C driver fix, NOT YET USER VERIFIED

 Built on Waveshare's ESP32-S3-Touch-LCD-7 Arduino
 10_lvgl_v9_demo.ino (LVGL 9.5 + ESP32_Display_Panel)
 and verified Slot-18 radar plotter UART parser and data fields.

 Screen result: real-time motion and presence readouts + bars
 (0-11m), presence/motion state, and connection indicator.
 DATA LINK = UP only when checksum-valid notifications <=5sec old.
 The number fields come from C4002, which is a single-beam
 presence/distance sensor, NOT a human ID or XY position tracker.

 This v1 is a DISPLAY-ONLY demo, not yet a touch-enabled control
 panel: ESP_OPEN_TOUCH 0 in esp_panel_board_custom_conf.h.
 This deliberately avoids competing GT911 touch I2C reads during
 the first integration of SC16IS752 radar and LCD.

 Radar configuration: READ-ONLY (no sensor command frames), leaves
 original user-verified range 0-11m, motion/presence MID and delay 15s.
 Bridge initialization configures SC16IS752 UART registers ONLY.

 Wiring stays as the user's tested setup: external CJMCU bridge at
 I2C0x4D SDA8 SCL9, radar on bridge UART A 115200 8N1, shared GND.
 Keep red Waveshare I2C VCC lead disconnected.
 USB serial debug is CH343 UART USB-C port at 115200 baud.

 Dependencies: Arduino-ESP32 3.x,
 Waveshare ESP32-S3-Touch-LCD-7 model selected,
 ESP32_Display_Panel library, LVGL v9.5.0,
 and three local support files copied from the manufacturer v9 demo.
 See README.md in this directory. All other proven firmware preserved.
*/
#include <Arduino.h>
#include <driver/i2c.h>   // Legacy IDF driver used by Waveshare CH422G expander
#include <freertos/FreeRTOS.h>
#include <esp_display_panel.hpp>
#include <esp_err.h>
#include <lvgl.h>
#include "esp_lv_adapter_arduino.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;

static const uint8_t BRIDGE=0x4D;
static const uint8_t REG_RHR=0,REG_IER=1,REG_FCR=2;
static const uint8_t REG_LCR=3,REG_MCR=4,REG_LSR=5;
static uint8_t frame[128];
static size_t pos=0;
static uint16_t expected=0;
static uint32_t valid=0,invalid=0,results=0,rxCount=0;
static uint32_t overrun=0,parity=0,framing=0,breaks=0,i2cErrors=0;
static uint32_t lastDetectionMs=0,lastGraphMs=0,lastDebugMs=0;
static uint8_t reportedState=0;
static float presenceM=0.0f,motionM=0.0f;
static bool bridgeReady=false,uiReady=false;

static lv_obj_t *statusText=nullptr,*dataText=nullptr;
static lv_obj_t *presenceText=nullptr,*motionText=nullptr;
static lv_obj_t *presenceBar=nullptr,*motionBar=nullptr;
static lv_obj_t *frameText=nullptr;

uint8_t addr(uint8_t reg) { return (uint8_t)(reg<<3); } // channel A=0

// ESP32_Display_Panel's Waveshare CH422G expander uses the legacy I2C
// driver in this installed library stack. Arduino Wire on core 3.3
// pulls in the new driver and crashes before setup ("driver_ng CONFLICT").
// Share its ALREADY INITIALIZED I2C port 0; do not install I2C again.
static const i2c_port_t RADAR_BUS=I2C_NUM_0;
static const TickType_t I2C_TIMEOUT=pdMS_TO_TICKS(100);
bool writeReg(uint8_t r, uint8_t v) {
  const uint8_t msg[2]={addr(r),v};
  return i2c_master_write_to_device(
      RADAR_BUS,BRIDGE,msg,sizeof(msg),I2C_TIMEOUT)==ESP_OK;
}
bool readReg(uint8_t r, uint8_t &v) {
  const uint8_t reg=addr(r);
  return i2c_master_write_read_device(
      RADAR_BUS,BRIDGE,&reg,1,&v,1,I2C_TIMEOUT)==ESP_OK;
}
uint16_t u16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1]<<8); }
uint32_t u32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24); }
int16_t s16(const uint8_t *p) { return (int16_t)u16(p); }

const char *targetName(uint8_t state) {
  switch (state) {
    case 0: return "NO_TARGET";
    case 1: return "PRESENCE";
    case 2: return "MOTION";
    case 3: return "MOTION_OR_PRESENCE";
    case 4: return "MOTION_OR_NO_TARGET";
    case 5: return "PRESENCE_OR_NO_TARGET";
    default: return "OTHER";
  }
}



void decodeFrame() {
  if(expected<14) { ++invalid; return; }
  uint16_t checksum=0;
  for(size_t i=0;i<(size_t)expected-2;i++)
    checksum=(uint16_t)(checksum+frame[i]);
  if(checksum!=u16(&frame[expected-2])) { ++invalid; return; }
  ++valid;
  if(expected!=32 || frame[7]!=0x04 || frame[8]!=0x60 ||
     frame[9]!=0x01 || u16(&frame[10])!=22) return;

  ++results;
  lastDetectionMs=millis();
  const uint8_t *p=&frame[12];
  reportedState=p[0];
  if(reportedState==0||reportedState==255) {
    presenceM=0.0f;motionM=0.0f;return;
  }
  const uint16_t presenceCm=u16(&p[9]);
  const uint16_t motionCm=u16(&p[12]);
  presenceM=(p[11]!=0 && presenceCm<=1100) ? 
    ((float)presenceCm/100.0f):0.0f;
  motionM=(p[16]!=0 && motionCm<=1100) ?
    ((float)motionCm/100.0f):0.0f;
}

void appendByte(uint8_t b) {
  // Initial synchronization to FA F5 AA A5.
  static const uint8_t magic[]={0xFA,0xF5,0xAA,0xA5};
  if(pos<4) {
    if(b==magic[pos]) frame[pos++]=b;
    else {
      pos=(b==magic[0])?1:0;
      if(pos) frame[0]=b;
    }
    return;
  }
  if(pos>=sizeof(frame)) { pos=0; expected=0; ++invalid; return; }
  frame[pos++]=b;
  if(pos==6) {
    expected=u16(&frame[4]);
    if(expected<10 || expected>sizeof(frame)) {
      ++invalid; pos=0; expected=0;
    }
  }
  if(expected!=0 && pos==expected) {
    decodeFrame();
    pos=0; expected=0;
  }
}



static lv_obj_t *makePanel(lv_obj_t *parent,int x,int y,int w,int h) {
  lv_obj_t *panel=lv_obj_create(parent);
  lv_obj_set_pos(panel,x,y);
  lv_obj_set_size(panel,w,h);
  lv_obj_set_style_radius(panel,14,0);
  lv_obj_set_style_bg_color(panel,lv_color_hex(0x152339),0);
  lv_obj_set_style_border_width(panel,1,0);
  lv_obj_set_style_border_color(panel,lv_color_hex(0x345071),0);
  lv_obj_set_style_pad_all(panel,16,0);
  lv_obj_remove_flag(panel,LV_OBJ_FLAG_SCROLLABLE);
  return panel;
}
static lv_obj_t *makeLabel(lv_obj_t *parent,const char *str,int x,int y,
                           uint32_t rgb) {
  lv_obj_t *o=lv_label_create(parent);
  lv_label_set_text(o,str);
  lv_obj_set_pos(o,x,y);
  lv_obj_set_style_text_color(o,lv_color_hex(rgb),0);
  return o;
}
void makeUI() {
  lv_obj_t *root=lv_screen_active();
  lv_obj_set_style_bg_color(root,lv_color_hex(0x091523),0);
  lv_obj_remove_flag(root,LV_OBJ_FLAG_SCROLLABLE);
  makeLabel(root,"SLOT-18 / RADAR LIVE",24,19,0xEAF4FF);
  dataText=makeLabel(root,"LINK: WAITING",610,19,0xFAB653);

  lv_obj_t *p=makePanel(root,22,60,370,306);
  makeLabel(p,"PRESENCE DISTANCE",12,11,0xA6C6E8);
  presenceText=makeLabel(p,"--.-- m",12,55,0x83E0C3);
  makeLabel(p,"0m",12,160,0x9EAFCD);
  makeLabel(p,"11m",306,160,0x9EAFCD);
  presenceBar=lv_bar_create(p);
  lv_obj_set_pos(presenceBar,14,195);
  lv_obj_set_size(presenceBar,310,34);
  lv_bar_set_range(presenceBar,0,1100);
  lv_bar_set_value(presenceBar,0,LV_ANIM_OFF);
  lv_obj_set_style_bg_color(presenceBar,lv_color_hex(0x27374B),LV_PART_MAIN);
  lv_obj_set_style_bg_color(presenceBar,lv_color_hex(0x53D6AE),LV_PART_INDICATOR);

  lv_obj_t *m=makePanel(root,408,60,370,306);
  makeLabel(m,"MOTION DISTANCE",12,11,0xA6C6E8);
  motionText=makeLabel(m,"--.-- m",12,55,0xF9BC77);
  makeLabel(m,"0m",12,160,0x9EAFCD);
  makeLabel(m,"11m",306,160,0x9EAFCD);
  motionBar=lv_bar_create(m);
  lv_obj_set_pos(motionBar,14,195);
  lv_obj_set_size(motionBar,310,34);
  lv_bar_set_range(motionBar,0,1100);
  lv_bar_set_value(motionBar,0,LV_ANIM_OFF);
  lv_obj_set_style_bg_color(motionBar,lv_color_hex(0x27374B),LV_PART_MAIN);
  lv_obj_set_style_bg_color(motionBar,lv_color_hex(0xFAA45D),LV_PART_INDICATOR);

  lv_obj_t *bottom=makePanel(root,22,384,756,78);
  makeLabel(bottom,"RADAR STATE",10,8,0xA6C6E8);
  statusText=makeLabel(bottom,"WAITING FOR PACKETS",135,8,0xFFFFFF);
  frameText=makeLabel(bottom,"Valid: 0   Bad: 0",10,35,0x93A9C8);
}
void renderData() {
  if(!uiReady) return;
  const uint32_t now=millis();
  const bool recent=bridgeReady && results>0 &&
                    (uint32_t)(now-lastDetectionMs)<=5000;
  const int centiP=recent?(int)(presenceM*100.0f+0.5f):0;
  const int centiM=recent?(int)(motionM*100.0f+0.5f):0;
  const int state=!recent?0:
    (reportedState==2||reportedState==3||reportedState==4)?2:
    (reportedState==1||reportedState==5)?1:0;
  if(esp_lv_adapter_lock(-1)!=ESP_OK) return;
  lv_label_set_text(dataText,recent?"LINK: LIVE":"LINK: WAITING");
  lv_obj_set_style_text_color(dataText,
    lv_color_hex(recent?0x62E0AC:0xFAB653),0);
  lv_label_set_text_fmt(presenceText,"%d.%02d m",
                        centiP/100,centiP%100);
  lv_label_set_text_fmt(motionText,"%d.%02d m",
                        centiM/100,centiM%100);
  lv_bar_set_value(presenceBar,centiP,LV_ANIM_OFF);
  lv_bar_set_value(motionBar,centiM,LV_ANIM_OFF);
  lv_label_set_text(statusText,!recent?"NO NEW DATA":
                    state==2?"MOTION":state==1?"PRESENCE":"NO TARGET");
  lv_label_set_text_fmt(frameText,"Valid: %lu    Bad: %lu    I2C: %lu",
                       (unsigned long)valid,(unsigned long)invalid,
                       (unsigned long)i2cErrors);
  esp_lv_adapter_unlock();
}
void setup() {
  Serial.begin(115200);
  delay(1600);
  Serial.println("\nSLOT-18 / WAVESHARE LIVE RADAR LCD v2 (I2C compatibility)");
  Serial.println("DISPLAY ONLY: no radar configuration writes.");
  // No Wire; board->begin() initializes the old I2C bus 0 (GPIO8/9)
  // for its backlight/reset CH422G. Radar bridge shares that bus.

  Board *board=new Board();
  if((board==nullptr)||!board->init()) {
    Serial.println("STOP: Waveshare board init failed.");
    return;
  }
  const esp_lv_adapter_rotation_t rotation=ESP_LV_ADAPTER_ROTATE_0;
  const esp_lv_adapter_tear_avoid_mode_t tearMode=
       ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_RGB;
  const uint8_t count=
    esp_lv_adapter_get_required_frame_buffer_count(tearMode,rotation);
  LCD *lcd=board->getLCD();
  if(lcd==nullptr) {
    Serial.println("STOP: no LCD detected.");
    return;
  }
  auto *bus=lcd->getBus();
  if(bus->getBasicAttributes().type==ESP_PANEL_BUS_TYPE_RGB) {
    lcd->configFrameBufferNumber(count);
    static_cast<BusRGB*>(bus)->configRGB_BounceBufferSize(
      lcd->getFrameWidth()*10);
  }
  if(!board->begin()) {
    Serial.println("STOP: LCD board begin failed.");
    return;
  }
  esp_lv_adapter_config_t config=ESP_LV_ADAPTER_DEFAULT_CONFIG();
  config.task_stack_size=12*1024;
  config.task_priority=2;
  config.task_core_id=ARDUINO_RUNNING_CORE;
  if(esp_lv_adapter_init(&config)!=ESP_OK) {
    Serial.println("STOP: LVGL adapter init failed.");return;
  }
  esp_lv_adapter_display_config_t dc=
    ESP_LV_ADAPTER_DISPLAY_RGB_DEFAULT_CONFIG(
      lcd,lcd->getFrameWidth(),lcd->getFrameHeight(),rotation);
  dc.profile.use_psram=true;
  lv_display_t *display=esp_lv_adapter_register_display(&dc);
  if(display==nullptr) {
    Serial.println("STOP: LVGL display registration failed.");return;
  }
  // Touch deliberately disabled in board config for first shared-I2C pilot.
  if(esp_lv_adapter_start()!=ESP_OK) {
    Serial.println("STOP: LVGL task start failed.");return;
  }
  if(esp_lv_adapter_lock(-1)==ESP_OK) {
    makeUI();
    esp_lv_adapter_unlock();
    uiReady=true;
  }
  Serial.println("LCD DASHBOARD READY; starting C4002 link...");
  uint8_t probeResult=0;
  const uint8_t probe=addr(REG_LSR);
  if(i2c_master_write_read_device(
       RADAR_BUS,BRIDGE,&probe,1,&probeResult,1,I2C_TIMEOUT)!=ESP_OK) {
    Serial.println("RADAR BRIDGE NOT FOUND: CJMCU-752 I2C 0x4D");
    renderData();return;
  }
  if(!writeReg(REG_IER,0)||!writeReg(REG_MCR,0)||
     !writeReg(REG_LCR,0x80)||!writeReg(0,1)||
     !writeReg(1,0)||!writeReg(REG_LCR,0x03)||
     !writeReg(REG_FCR,0x07)) {
    Serial.println("RADAR BRIDGE INIT FAILED.");
    renderData();return;
  }
  bridgeReady=true;
  Serial.println("RADAR BRIDGE READY, UART A 115200.");
  lastDetectionMs=millis();
  lastGraphMs=millis();
  lastDebugMs=millis();
}
void loop() {
  if(!uiReady) { delay(500);return; }
  if(bridgeReady) {
    uint8_t lsr=0;
    if(readReg(REG_LSR,lsr)) {
      if(lsr&0x02) ++overrun;
      if(lsr&0x04) ++parity;
      if(lsr&0x08) ++framing;
      if(lsr&0x10) ++breaks;
      if(lsr&0x01) {
        uint8_t b=0;
        if(readReg(REG_RHR,b)) {
          ++rxCount;
          appendByte(b);
        } else ++i2cErrors;
      } else delay(1);
    } else {++i2cErrors;delay(10);}
  } else delay(10);
  const uint32_t now=millis();
  if((uint32_t)(now-lastGraphMs)>=500) {
    lastGraphMs=now;
    renderData();
  }
  if((uint32_t)(now-lastDebugMs)>=5000) {
    lastDebugMs=now;
    Serial.printf("LIVE: presence=%.2fm motion=%.2fm state=%u "
                  "valid=%lu bad=%lu I2C_errors=%lu\n",
       presenceM,motionM,(unsigned)reportedState,
       (unsigned long)valid,(unsigned long)invalid,
       (unsigned long)i2cErrors);
  }
}
