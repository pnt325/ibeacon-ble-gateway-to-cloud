/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifi.h"
#include "ble.h"

static const char* APP_TAG = "APP_MAIN";

static void wifi_connect_callback(bool connect);

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    uint8_t mac[8] = {0};
    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));
    ESP_LOGI(APP_TAG, "WIFI MAC address: %02x-%02x-%02x-%02x-%02x-%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    WIFI_init((const uint8_t*)"TP-Link_5BC8", (const uint8_t*)"33714592");
    WIFI_start(wifi_connect_callback);

    BLE_init(BLE_BEACON);
    BLE_start();
}


static void wifi_connect_callback(bool connect)
{
    ESP_LOGI(APP_TAG, "WIFI connection = %s", connect ? "True" : "False");
}
