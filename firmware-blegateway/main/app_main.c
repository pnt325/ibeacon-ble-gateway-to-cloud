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
#include "mqtt.h"
#include "ble_beacon_data.h"
#include "wifi_config.h"
#include "wifi_ap.h"
#include "nvs_data.h"

static const char* APP_TAG = "APP_MAIN";

static uint8_t mac[8] = {0};

static void wifi_connect_callback(bool connect);
static void beacon_event_callback(beacon_data_t* data);

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
    
    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));
    ESP_LOGI(APP_TAG, "WIFI MAC address: %02x-%02x-%02x-%02x-%02x-%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    NVS_DATA_init();
    WIFI_AP_init();
    WIFI_AP_start();
    WIFI_CONFIG_init();
    return;

    WIFI_init((const uint8_t*)"TP-Link_5BC8", (const uint8_t*)"33714592");
    WIFI_start(wifi_connect_callback);

    BLE_init(BLE_BEACON);
    BLE_start(beacon_event_callback);
}

static void mqtt_connected(uint8_t connect)
{
    ESP_LOGI(APP_TAG, "MQTT connection = %s", connect ? "True" : "False");
}


static void wifi_connect_callback(bool connect)
{
    static bool first_connected = true;
    ESP_LOGI(APP_TAG, "WIFI connection = %s", connect ? "True" : "False");

    if(first_connected)
    {
        first_connected = false;
        MQTT_init(mqtt_connected);
    }
}

static void beacon_event_callback(beacon_data_t* data)
{
    // TODO Handle the beacon message.
    // Received the beacon  message callback
    // Handle by compare beacon UUID with whitelist then add
    // to the whitelist item data update with flag notify that should be sync to the MQTT
    //



    char buf[256] = "test data";

    snprintf(buf, 256, "{\"addr\":\"%02x-%02x-%02x-%02x-%02x-%02x\",\"uuid\":\"%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\",\"data\":\"%02x %02x %02x %02x\"}",
             data->addr[0], data->addr[1], data->addr[2], data->addr[3], data->addr[4], data->addr[5],
             data->uuid[0], data->uuid[1],  data->uuid[2],  data->uuid[3],  data->uuid[4],  data->uuid[5],  data->uuid[6],  data->uuid[7],
             data->uuid[8], data->uuid[9], data->uuid[10], data->uuid[11], data->uuid[12], data->uuid[13], data->uuid[14], data->uuid[15],
             data->udata[0], data->udata[1], data->udata[2], data->udata[3]);

    char topic[32] = "ble_gateway";
    snprintf(topic, 32, "ble_gateway/%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if(MQTT_publish(topic, buf, strlen(buf)))
    {
        ESP_LOGI(APP_TAG,"Publish message: %s", buf);
    }
    else
    {
        ESP_LOGI(APP_TAG,"Publish message failure");
    }

    // esp_ble_ibeacon_t *ibeacon_data = (esp_ble_ibeacon_t*)(scan_result->scan_rst.ble_adv);
    // ESP_LOGI(DEMO_TAG, "----------iBeacon Found----------");
    // esp_log_buffer_hex("IBEACON_DEMO: Device address:", scan_result->scan_rst.bda, ESP_BD_ADDR_LEN );
    // esp_log_buffer_hex("IBEACON_DEMO: Proximity UUID:", ibeacon_data->ibeacon_vendor.proximity_uuid, ESP_UUID_LEN_128);

    // uint16_t major = ENDIAN_CHANGE_U16(ibeacon_data->ibeacon_vendor.major);
    // uint16_t minor = ENDIAN_CHANGE_U16(ibeacon_data->ibeacon_vendor.minor);
    // ESP_LOGI(DEMO_TAG, "Major: 0x%04x (%d)", major, major);
    // ESP_LOGI(DEMO_TAG, "Minor: 0x%04x (%d)", minor, minor);
    // ESP_LOGI(DEMO_TAG, "Measured power (RSSI at a 1m distance):%d dbm", ibeacon_data->ibeacon_vendor.measured_power);
    // ESP_LOGI(DEMO_TAG, "RSSI of packet:%d dbm, distance: %d", scan_result->scan_rst.rssi, scan_result->scan_rst.rssi/ibeacon_data->ibeacon_vendor.measured_power);
}