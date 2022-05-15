/*
 * app_main.c
 *
 *  Created on: May 06, 2022
 *      Author: Phat.N
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
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
#include "bg22_led.h"
#include "bg22_button.h"
#include "app_config.h"
#include "app_led.h"

static const char* APP_TAG = "APP_MAIN";

uint8_t mac[8] = {0};

static void wifi_connect_callback(bool connect);
static void beacon_event_callback(beacon_data_t* data);

static void mqtt_event_connected(void);
static void mqtt_event_disconnected(void);

static QueueHandle_t btn_queue;

static void button_callback(button_state_t state, button_type_t type) {
    xQueueSendFromISR(btn_queue, &state, NULL);
}

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

    // LED Initialize
    app_led_init();

    // Button Initlialize
    bg22_button_init(ButtonUser, button_callback);
    btn_queue = xQueueCreate(2, sizeof(button_state_t));
    bg22_assert(btn_queue);

    // NVS Data Initialize 
    NVS_DATA_init();

    uint8_t is_config = 0;
    NVS_DATA_init_config_get(&is_config);
    if(is_config) {
        ESP_LOGI(APP_TAG, "Start Gateway\n");
        char* wifi_name = (char*)malloc(33);
        char* wifi_pass = (char*)malloc(65);
        bg22_assert(wifi_name);
        bg22_assert(wifi_pass);
        memset(wifi_name, 0, 33);
        memset(wifi_pass, 0, 65);

        NVS_DATA_ssid_get(wifi_name);
        NVS_DATA_password_get(wifi_pass);

        WIFI_init((const uint8_t *)wifi_name, (const uint8_t *)wifi_pass);
        WIFI_start(wifi_connect_callback);
        // BLE_init(BLE_BEACON);
        // BLE_start(beacon_event_callback);

        memset(wifi_name, 0, 33);
        memset(wifi_pass, 0, 65);
        free(wifi_name);
        free(wifi_pass);
        
        wifi_name = NULL;
        wifi_pass = NULL;

        // Enabled button
        bg22_button_enabled(ButtonUser);

        // Blinking LED until connected to broker.
        app_led_ctrl(LedGreen, LedCtrlBlink, APP_LED_BLINKING_PERIOD);
        app_led_ctrl(LedRed, LedCtrlOff, 0);
        app_led_ctrl(LedBlue, LedCtrlOff, 0);
    } else {
        ESP_LOGI(APP_TAG, "Start WIFI provisioning\n");
        WIFI_AP_init();
        WIFI_AP_start();
        WIFI_CONFIG_init();
        
        app_led_ctrl(LedRed, LedCtrlOn, 0);
        app_led_ctrl(LedGreen, LedCtrlOff, 0);
        app_led_ctrl(LedBlue, LedCtrlOff, 0);
    }

    // Will be continue handle button and LED blinking control.
    button_state_t btn_state_old = ButtonReleased;
    uint32_t btn_press_period = esp_log_timestamp();
    while(true) {
        button_state_t btn_state_cur;   // Button current state
        if(xQueueReceive(btn_queue, &btn_state_cur, 0) == pdTRUE)  {
            // Button state change handle.
            if(btn_state_cur != btn_state_old) {
                ESP_LOGI(APP_TAG, "Button %s\n", btn_state_cur == ButtonPressed ? "pressed" : "released");
                btn_state_old = btn_state_cur;
                if(btn_state_cur == ButtonPressed) {
                    btn_press_period = esp_log_timestamp();
                } else {
                    // TODO handle button release event
                    uint32_t hold_time = (uint32_t)(esp_log_timestamp() - btn_press_period);
                    ESP_LOGI(APP_TAG, "Button hold period: %d\n", hold_time);

                    // TODO Reset the device
                    if(hold_time >= APP_BUTTON_RESET_PERIOD) {
                        NVS_DATA_init_config_set(0);
                        esp_restart();
                    } else {

                    }
                }
            }
        }

        if(btn_state_old == ButtonPressed) {
            uint32_t ms = (uint32_t)(esp_log_timestamp() - btn_press_period);
            if(ms >= APP_BUTTON_RESET_PERIOD) {
                // TODO Handle the reach time button hold time for reset.
            }
        }

        app_led_handle();
        vTaskDelay(pdMS_TO_TICKS(APP_TASK_BUTTON_LED_SLEEP_PERIOD));
    }
}

static void wifi_connect_callback(bool connect)
{
    static bool first_connected = true;
    ESP_LOGI(APP_TAG, "WIFI connection = %s", connect ? "True" : "False");

    if(first_connected)
    {
        first_connected = false;

        char* mqtt_host = (char*)malloc(32);
        char* mqtt_user = (char*)malloc(32);
        char* mqtt_pass = (char*)malloc(32);
        bg22_assert(mqtt_host);
        bg22_assert(mqtt_user);
        bg22_assert(mqtt_pass);
        memset(mqtt_host, 0, 32);
        memset(mqtt_user, 0, 32);
        memset(mqtt_pass, 0, 32);

        NVS_DATA_mqtt_addr_get(mqtt_host);
        NVS_DATA_mqtt_user_get(mqtt_user);
        NVS_DATA_mqtt_pass_get(mqtt_pass);

        mqtt_event_t mqtt_event;
        mqtt_event.connected = mqtt_event_connected;
        mqtt_event.disconnected = mqtt_event_disconnected;

        MQTT_init(&mqtt_event, (const char*)mqtt_host, (const char*)mqtt_user, (const char*)mqtt_pass);
        memset(mqtt_host, 0, 32);
        memset(mqtt_user, 0, 32);
        memset(mqtt_pass, 0, 32);
        free(mqtt_host);
        free(mqtt_user);
        free(mqtt_pass);
        mqtt_host = NULL;
        mqtt_user = NULL;
        mqtt_pass = NULL;
    }
}

static void beacon_event_callback(beacon_data_t* data)
{
    // TODO Handle the beacon message.
    // Received the beacon  message callback
    // Handle by compare beacon UUID with whitelist then add
    // to the whitelist item data update with flag notify that should be sync to the MQTT

/*
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
*/

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

static void mqtt_event_connected(void) {
    app_led_ctrl(LedGreen, LedCtrlOn, 0);
}

static void mqtt_event_disconnected(void) {
    app_led_ctrl(LedGreen, LedCtrlBlink, APP_LED_BLINKING_PERIOD);
}