idf_component_register(SRCS "sources/main.c" 
                            "sources/ina226.c" 
                            "sources/lcd.c" 
                            "sources/wifi_manager.c"
                       INCLUDE_DIRS "headers"
                       REQUIRES esp_driver_i2c esp_driver_gpio nvs_flash esp_wifi esp_netif esp_event lwip esp_http_server esp_timer)