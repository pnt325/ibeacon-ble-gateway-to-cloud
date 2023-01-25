/*
 * app_led.h
 *
 *  Created on: May 15, 2022
 *      Author: Phat.N
 */

#ifndef _APP_LED_H_
#define _APP_LED_H_

#include <stdint.h>
#include "bg22_led.h"

/* LED control mode */
typedef enum 
{
    LedCtrlOff,
    LedCtrlOn,
    LedCtrlBlink,
    _LedCtrlNum
} led_ctrl_t;

void app_led_init(void);
void app_led_ctrl(led_t led, led_ctrl_t ctrl, uint32_t period);
void app_led_handle(void);

#endif /*_APP_LED_H_*/