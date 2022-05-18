/*
 * wifi.c
 *
 *  Created on: May 05, 2022
 *      Author: Phat.N
 */

#include <string.h>
#include "wifi.h"
#include "app_var.h"

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

static const char *WIFI_TAG = "WIFI";

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static bool is_wifi_init = false;       // Flag store wifi is initialize
static wifi_connect_event_t connect_callback;
uint8_t ip_addr[4];

void WIFI_init(const uint8_t *ssid, const uint8_t *password)
{
    // Check input param
    if (ssid == NULL || password == NULL)
    {
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };

    memcpy(wifi_config.sta.ssid, ssid, strlen((const char*)ssid));
    memcpy(wifi_config.sta.password, password, strlen((const char*)password));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    is_wifi_init = true;
}

void WIFI_start(wifi_connect_event_t callback)
{
    connect_callback = callback;

    ESP_ERROR_CHECK((is_wifi_init ? ESP_OK : ESP_FAIL));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void WIFI_stop(void)
{
    connect_callback = NULL;
    ESP_ERROR_CHECK(esp_wifi_stop());
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
        // TODO Callback disconnected
        if(connect_callback)
        {
            connect_callback(false);
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        // TODO callback wifi connected
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(WIFI_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        app_var_t var;

        var.u32 = event->ip_info.ip.addr;
        ip_addr[0] = var.buf[0];
        ip_addr[1] = var.buf[1];
        ip_addr[2] = var.buf[2];
        ip_addr[3] = var.buf[3];

        ESP_LOGI(WIFI_TAG, "Parse IP: %d.%d.%d.%d\n", ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3]);

        if(connect_callback)
        {
            connect_callback(true);
        }
    }
}