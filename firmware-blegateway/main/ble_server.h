/*
 * ble_server.h
 *
 *  Created on: May 06, 2022
 *      Author: Phat.N
 * 
 * Handle the Bluetooth provisioning configuration WIFI for Gateway
 */

#ifndef _BLE_SERVER_H_
#define _BLE_SERVER_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

void BLE_SERVER_start(void);
void BLE_SERVER_stop(void);

#endif /*_BLE_SERVER_H_*/