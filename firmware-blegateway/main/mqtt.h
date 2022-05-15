/*
 * mqtt.h
 *
 *  Created on: May 05, 2022
 *      Author: Phat.N
 */

#ifndef _MQTT_H_
#define _MQTT_H_

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* MQTT data subscribe received callback */
typedef void(*mqtt_data_callback_t)(char* data, size_t data_len, char* topic, size_t topic_len);

/* MQTT event structure */
typedef struct  {
    void(*connected)(void);
    void(*disconnected)(void);
} mqtt_event_t;

void MQTT_init(mqtt_event_t* event, const char* host, const char* user, const char* password);
bool MQTT_publish(const char* topic, const char* data, uint16_t len);
bool MQTT_subscribe(const char* topic, mqtt_data_callback_t callback);
bool MQTT_unsubscribe(const char* topic);

#endif /*_MQTT_H_*/