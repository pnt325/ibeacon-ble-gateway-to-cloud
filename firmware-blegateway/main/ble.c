/*
 * ble.c
 *
 *  Created on: May 05, 2022
 *      Author: Phat.N
 */

#include "ble.h"
#include "ble_server.h"
#include "ble_beacon.h"

#include "esp_log.h"
#include "esp_system.h"

#define BLE_MODE_INVALID    0xFF

static uint8_t ble_mode = BLE_MODE_INVALID;

void BLE_init(uint8_t mode)
{
    ble_mode = mode;
    ESP_LOGI("BLE", "Init");
}

void BLE_start(beacon_callback_t callback)
{
    switch (ble_mode)
    {
    case BLE_SERVER:
        BLE_SERVER_start();
        break;
    case BLE_BEACON:
        BLE_BEACON_start(callback);
        break;
    default:
        ESP_ERROR_CHECK(ESP_FAIL);
        break;
    }
}

void BLE_stop(void)
{
switch (ble_mode)
    {
    case BLE_SERVER:
        BLE_SERVER_stop();
        break;
    case BLE_BEACON:
        BLE_BEACON_stop();
        break;
    default:
        break;
    }
    ble_mode = BLE_MODE_INVALID;
}