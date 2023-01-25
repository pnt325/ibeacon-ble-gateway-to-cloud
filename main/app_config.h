/*
 * app_config.h
 *
 *  Created on: May 09, 2022
 *      Author: Phat.N
 */

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

/* WIFI AP name using on the WIFI provisioning */
#define APP_WIFI_AP_NAME                    "BGATEWAY"
#define APP_WIFI_AP_PASSWORD                "password2342"

#define APP_BUTTON_RESET_PERIOD             5000    // ms
#define APP_TASK_BUTTON_LED_SLEEP_PERIOD    50      // ms
#define APP_LED_BLINKING_PERIOD             250     // ms

#define APP_GATEWAY_DEVICE_MAX              16

/* App topic format */
#define APP_UNKNOW_DEVICE_TOPIC             "ble_gateway/%02x:%02x:%02x:%02x:%02x:%02x"
#define APP_GATEWAY_PUBLISH_TOPIC           "internal_net/req/cl/%02x:%02x:%02x:%02x:%02x:%02x"

/* Device sync message format
 * macAddress: Gateway MAC address
 * clientMAC: Device MAC address
 * data: Device data name
 * value: Device data value
 */
#define APP_MQTT_MSG_DEVICE_SYNC_FORMAT     "{\"macAddress\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"clientMac\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"data\":\"%s\",\"value\":\"%s\"}"

/* Get known UUID message format
 * macAddress: Gateway MAC address
 * ipaddress: Gateway IP address
 * firmwareVersion: Gateway firmware version
 * data: NET_ID (Device device data name)
 */
#define APP_MQTT_MSG_KNOWN_UUID_FORMAT      "{\"macAddress\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"ipaddress\":\"%d:%d:%d:%d\",\"firmwareVersion\":\"%d.%d.%d\",\"data\":\"NET_ID\"}"

/* Gateway firmware version */
#define FW_MAJOR    0
#define FW_MINOR    0
#define FW_PATCH    0

#endif /*_APP_CONFIG_H_*/