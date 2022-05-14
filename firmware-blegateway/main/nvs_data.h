/*
 * nvs_data.h
 *
 *  Created on: May 09, 2022
 *      Author: Phat.N
 */

#ifndef _NVS_DATA_H_
#define _NVS_DATA_H_

#include <stdint.h>

void NVS_DATA_init(void);
void NVS_DATA_ssid_get(char* data);
void NVS_DATA_ssid_set(char* data);
void NVS_DATA_password_get(char* data);
void NVS_DATA_password_set(char* data);
void NVS_DATA_mqtt_addr_get(char* data);
void NVS_DATA_mqtt_addr_set(char* data);
void NVS_DATA_mqtt_user_get(char* data);
void NVS_DATA_mqtt_user_set(char* data);
void NVS_DATA_mqtt_pass_get(char* data);
void NVS_DATA_mqtt_pass_set(char* data);
void NVS_DATA_init_config_set(uint8_t data);
void NVS_DATA_init_config_get(uint8_t* data);

#endif /*_NVS_DATA_H_*/