/*
 * wifi.h
 *
 *  Created on: May 05, 2022
 *      Author: Phat.N
 */

#ifndef _WIFI_H_
#define _WIFI_H_

#include <stdint.h>

void WIFI_init(const uint8_t* ssid, const uint8_t* password);
void WIFI_start(void);
void WIFI_stop(void);

#endif /*_WIFI_H_*/