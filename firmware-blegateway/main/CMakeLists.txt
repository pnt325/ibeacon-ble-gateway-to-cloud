file(GLOB bsp_lib "bsp/*.c")
idf_component_register(SRCS ${bsp_lib} "nvs_data.c" "wifi_config.c" "wifi_ap.c" "ble_server.c" "ble_beacon.c" "mqtt.c" "ble.c" "wifi.c" "app_main.c" "esp_ibeacon_api.c"
                    INCLUDE_DIRS "." "bsp")
