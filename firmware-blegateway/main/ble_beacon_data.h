/*
 * ble_beacon_data.h
 *
 *  Created on: May 07, 2022
 *      Author: Phat.N
 */

#ifndef _BLE_BEACON_DATA_H_
#define _BLE_BEACON_DATA_H_

#include <stdint.h>

typedef struct 
{
    uint8_t addr[6];
    uint8_t uuid[16];
    uint8_t data[4];
    uint8_t rssi;       
} beacon_data_t;

#endif /*_BLE_BEACON_DATA_H_*/
