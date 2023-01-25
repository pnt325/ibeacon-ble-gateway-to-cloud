/*
 * ble_beacon.h
 *
 *  Created on: May 06, 2022
 *      Author: Phat.N
 */

#ifndef _BLE_BEACON_H_
#define _BLE_BEACON_H_

#include <stdint.h>
#include "ble_beacon_data.h"

typedef void(*beacon_callback_t)(beacon_data_t* data);

void BLE_BEACON_start(beacon_callback_t event);
void BLE_BEACON_stop(void);

#endif /*_BLE_BEACON_H_*/