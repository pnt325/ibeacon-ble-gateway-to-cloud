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

#define NVS_KEY_WIFI_SSID "wifi_ssid"
#define NVS_KEY_WIFI_PASSWORD "wiif_password"
#define NVS_KEY_INIT_CONFIG "init_config"
#define NVS_KEY_MQTT_ADDR "mqtt_addr"
#define NVS_KEY_MQTT_USER "mqtt_usr"
#define NVS_KEY_MQTT_PASS "mqtt_pass"

void NVS_DATA_init(void)
{
    ESP_ERROR_CHECK(nvs_open("user_config", NVS_READWRITE, &nvs_cfg));
}

void NVS_DATA_ssid_get(char *data, size_t *buf_size)
{
    ESP_ERROR_CHECK(nvs_get_str(nvs_cfg, NVS_KEY_WIFI_SSID, data, buf_size));
}

void NVS_DATA_ssid_set(char *data)
{
    ESP_ERROR_CHECK(nvs_set_str(nvs_cfg, NVS_KEY_WIFI_SSID, data));
    ESP_ERROR_CHECK(nvs_commit(nvs_cfg));
}

void NVS_DATA_password_get(char *data, size_t *buf_size)
{
    ESP_ERROR_CHECK(nvs_get_str(nvs_cfg, NVS_KEY_WIFI_PASSWORD, data, buf_size));
}

void NVS_DATA_password_set(char *data)
{
    ESP_ERROR_CHECK(nvs_set_str(nvs_cfg, NVS_KEY_WIFI_PASSWORD, data));
    ESP_ERROR_CHECK(nvs_commit(nvs_cfg));
}

void NVS_DATA_mqtt_addr_get(char *data, size_t *buf_size)
{
    ESP_ERROR_CHECK(nvs_get_str(nvs_cfg, NVS_KEY_MQTT_ADDR, data, buf_size));
}

void NVS_DATA_mqtt_addr_set(char *data)
{
    ESP_ERROR_CHECK(nvs_set_str(nvs_cfg, NVS_KEY_MQTT_ADDR, data));
    ESP_ERROR_CHECK(nvs_commit(nvs_cfg));
}

void NVS_DATA_mqtt_user_get(char *data, size_t *buf_size)
{
    ESP_ERROR_CHECK(nvs_get_str(nvs_cfg, NVS_KEY_MQTT_USER, data, buf_size));
}

void NVS_DATA_mqtt_user_set(char *data)
{
    ESP_ERROR_CHECK(nvs_set_str(nvs_cfg, NVS_KEY_MQTT_USER, data));
    ESP_ERROR_CHECK(nvs_commit(nvs_cfg));
}

void NVS_DATA_mqtt_pass_get(char *data, size_t* buf_size)
{
    ESP_ERROR_CHECK(nvs_get_str(nvs_cfg, NVS_KEY_MQTT_PASS, data, buf_size));
}
void NVS_DATA_mqtt_pass_set(char *data)
{
    ESP_ERROR_CHECK(nvs_set_str(nvs_cfg, NVS_KEY_MQTT_PASS, data));
    ESP_ERROR_CHECK(nvs_commit(nvs_cfg));
}

void NVS_DATA_init_config_set(uint8_t data)
{
    ESP_ERROR_CHECK(nvs_set_u8(nvs_cfg, NVS_KEY_INIT_CONFIG, data));
    ESP_ERROR_CHECK(nvs_commit(nvs_cfg));
}

void NVS_DATA_init_config_get(uint8_t *data)
{
    ESP_ERROR_CHECK(nvs_get_u8(nvs_cfg, NVS_KEY_INIT_CONFIG, data));
}
