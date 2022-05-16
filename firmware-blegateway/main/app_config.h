/*
 * app_config.h
 *
 *  Created on: May 09, 2022
 *      Author: Phat.N
 */

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

/* WIFI AP name using on the WIFI provisioning */
#define APP_WIFI_AP_NAME        "BGATEWAY"
#define APP_WIFI_AP_PASSWORD    "password2342"

#define APP_BUTTON_RESET_PERIOD             5000    // ms
#define APP_TASK_BUTTON_LED_SLEEP_PERIOD    50      // ms
#define APP_LED_BLINKING_PERIOD             250     // ms

#define APP_GATEWAY_DEVICE_MAX              16

#define APP_GET_UNKNOW_DEVICE_TOPIC         "internal_net/req/cl/%02x:%02x:%02x:%02x:%02x:%02x"
#define APP_GATEWAY_PUBLISH_TOPIC           "internal_net/req/cl/%02x:%02x:%02x:%02x:%02x:%02x"

#endif /*_APP_CONFIG_H_*/