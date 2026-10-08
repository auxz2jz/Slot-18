idf_component_register(
    SRCS "main.c" "../components/waveshare_rgb_lcd_port.c"
    INCLUDE_DIRS "." "../components"
    REQUIRES nvs_flash esp_timer
)
