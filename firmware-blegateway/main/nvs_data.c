/*
 * nvs_data.c
 *
 *  Created on: May 09, 2022
 *      Author: Phat.N
 */

#include "nvs_data.h"

#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static nvs_handle_t nvs_cfg;

#define NVS_KEY_WIFI_SSID        "wifi_ssid"
#define NVS_KEY_WIFI_PASSWORD    "wifi_password"
#define NVS_KEY_INIT_CONFIG      "init_config"

void NVS_DATA_init(void)
{
    ESP_ERROR_CHECK(nvs_open("user_config", NVS_READWRITE, &nvs_cfg));
}

void NVS_DATA_ssid_get(char* data)
{
    size_t len;
    ESP_ERROR_CHECK(nvs_get_str(nvs_cfg, NVS_KEY_WIFI_SSID, data, &len));
}

void NVS_DATA_ssid_set(char* data)
{
    ESP_ERROR_CHECK(nvs_set_str(nvs_cfg, NVS_KEY_WIFI_SSID, data));
    ESP_ERROR_CHECK(nvs_commit(nvs_cfg));
}

void NVS_DATA_password_get(char* data)
{
    size_t len;
    ESP_ERROR_CHECK(nvs_get_str(nvs_cfg, NVS_KEY_WIFI_PASSWORD, data, &len));
}

void NVS_DATA_password_set(char *data)
{
    ESP_ERROR_CHECK(nvs_set_str(nvs_cfg, NVS_KEY_WIFI_PASSWORD, data));
    ESP_ERROR_CHECK(nvs_commit(nvs_cfg));
}

void NVS_DATA_init_config_set(uint8_t data)
{
    ESP_ERROR_CHECK(nvs_set_u8(nvs_cfg, NVS_KEY_INIT_CONFIG, data));
    ESP_ERROR_CHECK(nvs_commit(nvs_cfg));
}

void NVS_DATA_init_config_get(uint8_t* data)
{
    ESP_ERROR_CHECK(nvs_get_u8(nvs_cfg, NVS_KEY_INIT_CONFIG, data));
}