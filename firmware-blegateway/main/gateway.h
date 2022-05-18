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

/**
 * @brief Handle beacon message and sync to broker
 * @param data beacon_data_t
 */
void gateway_beacon_data_set(beacon_data_t* data);

void gateway_get_known_uuid(void);

#endif /*_GATE_WAY_H_*/