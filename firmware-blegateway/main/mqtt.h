/*
 * mqtt.h
 *
 *  Created on: May 05, 2022
 *      Author: Phat.N
 */

#ifndef _MQTT_H_
#define _MQTT_H_

#include <stdint.h>
#include <stdbool.h>

typedef void(*mqtt_event_cb_t)(uint8_t connect);

void MQTT_init(mqtt_event_cb_t event, const char* host, const char* user, const char* password);
bool MQTT_publish(const char* topic, const char* data, uint16_t len);

#endif /*_MQTT_H_*/