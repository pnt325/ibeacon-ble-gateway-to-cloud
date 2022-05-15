/*
 * gateway.h
 *
 *  Created on: May 15, 2022
 *      Author: Phat.N
 */

#ifndef _GATE_WAY_H_
#define _GATE_WAY_H_

#include "ble_beacon_data.h"

void gateway_init(void);

/**
 * @brief Intialize know device to get data.
 * @param data Json data
 */
void gateway_init_device(char* data);
void gateway_device_data_set(beacon_data_t* data);

#endif /*_GATE_WAY_H_*/