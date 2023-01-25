/*
 * ble.h
 *
 *  Created on: May 05, 2022
 *      Author: Phat.N
 */

#ifndef _BLE_H_
#define _BLE_H_

#include <stdint.h>
#include "esp_ibeacon_api.h"
#include "ble_beacon.h"

#define BLE_BEACON  0
#define BLE_SERVER  1
#define BLE_MODE    BLE_BEACON 

void BLE_init(uint8_t mode);

void BLE_start(beacon_callback_t callback);
void BLE_start_config(void);
void BLE_stop(void);

#endif /*_BLE_H_*/