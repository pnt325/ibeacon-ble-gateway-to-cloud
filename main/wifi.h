/*
 * wifi.h
 *
 *  Created on: May 05, 2022
 *      Author: Phat.N
 */

#ifndef _WIFI_H_
#define _WIFI_H_

#include <stdint.h>
#include <stdbool.h>

extern uint8_t ip_addr[4];

typedef void(*wifi_connect_event_t)(bool connected);

void WIFI_init(const uint8_t* ssid, const uint8_t* password);
void WIFI_start(wifi_connect_event_t callback);
void WIFI_stop(void);

#endif /*_WIFI_H_*/