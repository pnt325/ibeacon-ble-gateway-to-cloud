/*
 * ble_beacon.c
 *
 *  Created on: May 06, 2022
 *      Author: Phat.N
 */

#include "ble_beacon.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_defs.h"
#include "esp_ibeacon_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char *BEACON_TAG = "BLE_BEACON";
extern esp_ble_ibeacon_vendor_t vendor_config;

static beacon_callback_t beacon_event;
static beacon_data_t bdata;

/// Declare static functions
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

#if (IBEACON_MODE == IBEACON_RECEIVER)
static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x50,
    .scan_window = 0x30,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE};

#elif (IBEACON_MODE == IBEACON_SENDER)
static esp_ble_adv_params_t ble_adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_NONCONN_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};
#endif

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    esp_err_t err;

    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
    {
#if (IBEACON_MODE == IBEACON_SENDER)
        esp_ble_gap_start_advertising(&ble_adv_params);
#endif
        break;
    }
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
    {
#if (IBEACON_MODE == IBEACON_RECEIVER)
        // the unit of the duration is second, 0 means scan permanently
        uint32_t duration = 0;
        esp_ble_gap_start_scanning(duration);
#endif
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        // scan start complete event to indicate scan start successfully or failed
        if ((err = param->scan_start_cmpl.status) != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(BEACON_TAG, "Scan start failed: %s", esp_err_to_name(err));
        }
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        // adv start complete event to indicate adv start successfully or failed
        if ((err = param->adv_start_cmpl.status) != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(BEACON_TAG, "Adv start failed: %s", esp_err_to_name(err));
        }
        break;
    case ESP_GAP_BLE_SCAN_RESULT_EVT:
    {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt)
        {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
            /* Search for BLE iBeacon Packet */
            if (esp_ble_is_ibeacon_packet(scan_result->scan_rst.ble_adv, scan_result->scan_rst.adv_data_len))
            {
                esp_ble_ibeacon_t *ibeacon_data = (esp_ble_ibeacon_t *)(scan_result->scan_rst.ble_adv);

                memcpy(bdata.mac_addr, scan_result->scan_rst.bda, ESP_BD_ADDR_LEN);
                memcpy(bdata.uuid, ibeacon_data->ibeacon_vendor.proximity_uuid, ESP_UUID_LEN_128);

                // uint16_t major = ENDIAN_CHANGE_U16(ibeacon_data->ibeacon_vendor.major);
                // uint16_t minor = ENDIAN_CHANGE_U16(ibeacon_data->ibeacon_vendor.minor);

                uint8_t *tmp = (uint8_t *)&ibeacon_data->ibeacon_vendor.major;
                bdata.data[0] = tmp[0];
                bdata.data[1] = tmp[1];
                tmp = (uint8_t *)&ibeacon_data->ibeacon_vendor.minor;
                bdata.data[2] = tmp[0];
                bdata.data[3] = tmp[1];
                bdata.type = ibeacon_data->ibeacon_vendor.measured_power;

                if (beacon_event)
                {
                    beacon_event(&bdata);
                }

                // ESP_LOGI(BEACON_TAG, "----------iBeacon Found----------");
                // esp_log_buffer_hex("IBEACON_DEMO: Device address:", scan_result->scan_rst.bda, ESP_BD_ADDR_LEN );
                // esp_log_buffer_hex("IBEACON_DEMO: Proximity UUID:", ibeacon_data->ibeacon_vendor.proximity_uuid, ESP_UUID_LEN_128);

                // ESP_LOGI(BEACON_TAG, "Major: 0x%04x (%d)", major, major);
                // ESP_LOGI(BEACON_TAG, "Minor: 0x%04x (%d)", minor, minor);
                // ESP_LOGI(BEACON_TAG, "Measured power (RSSI at a 1m distance):%d dbm", ibeacon_data->ibeacon_vendor.measured_power);
                // ESP_LOGI(BEACON_TAG, "RSSI of packet:%d dbm, distance: %d", scan_result->scan_rst.rssi, scan_result->scan_rst.rssi/ibeacon_data->ibeacon_vendor.measured_power);
            }
            break;
        default:
            break;
        }
        break;
    }

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        if ((err = param->scan_stop_cmpl.status) != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(BEACON_TAG, "Scan stop failed: %s", esp_err_to_name(err));
        }
        else
        {
            ESP_LOGI(BEACON_TAG, "Stop scan successfully");
        }
        break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if ((err = param->adv_stop_cmpl.status) != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(BEACON_TAG, "Adv stop failed: %s", esp_err_to_name(err));
        }
        else
        {
            ESP_LOGI(BEACON_TAG, "Stop adv successfully");
        }
        break;

    default:
        break;
    }
}

void ble_ibeacon_appRegister(void)
{
    esp_err_t status;

    ESP_LOGI(BEACON_TAG, "register callback");

    // register the scan callback function to the gap module
    if ((status = esp_ble_gap_register_callback(esp_gap_cb)) != ESP_OK)
    {
        ESP_LOGE(BEACON_TAG, "gap register error: %s", esp_err_to_name(status));
        return;
    }
}

void BLE_BEACON_start(beacon_callback_t event)
{
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);

    esp_bluedroid_init();
    esp_bluedroid_enable();
    ble_ibeacon_appRegister();

    /* set scan parameters */
#if (IBEACON_MODE == IBEACON_RECEIVER)
    esp_ble_gap_set_scan_params(&ble_scan_params);
#elif (IBEACON_MODE == IBEACON_SENDER)
    esp_ble_ibeacon_t ibeacon_adv_data;
    esp_err_t status = esp_ble_config_ibeacon_data(&vendor_config, &ibeacon_adv_data);
    if (status == ESP_OK)
    {
        esp_ble_gap_config_adv_data_raw((uint8_t *)&ibeacon_adv_data, sizeof(ibeacon_adv_data));
    }
    else
    {
        ESP_LOGE(BEACON_TAG, "Config iBeacon data failed: %s\n", esp_err_to_name(status));
    }
#endif

    beacon_event = event;

    ESP_LOGI(BEACON_TAG, "Start");
}

void BLE_BEACON_stop(void)
{
}