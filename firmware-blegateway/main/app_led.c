/*
 * app_led.c
 *
 *  Created on: May 15, 2022
 *      Author: Phat.N
 */

#include "app_led.h"
#include "bg22_config.h"
#include "esp_log.h"

typedef struct  {
    led_ctrl_t ctrl;
    led_ctrl_t ctrl_old;
    uint32_t period;
    uint32_t hold_time;
} ctrl_handle_t;

static ctrl_handle_t ctrl_handles[_LedNum];

void app_led_init(void) {
    bg22_led_init();

    // Initialize control state
    for(led_t led = 0; led < _LedNum; led++) {
        ctrl_handles[led].ctrl     = LedCtrlOff;
        ctrl_handles[led].ctrl_old = LedCtrlOff;
    }
}

void app_led_ctrl(led_t led, led_ctrl_t ctrl, uint32_t period) {
    bg22_assert(led < _LedNum);
    bg22_assert(ctrl < _LedCtrlNum);

    ctrl_handles[led].ctrl   = ctrl;
    ctrl_handles[led].period = period;
}

void app_led_handle(void) {
    for(uint8_t led = 0; led < _LedNum; led++) {
        ctrl_handle_t* ctrl_handle = &ctrl_handles[led];

        if (ctrl_handle->ctrl != ctrl_handle->ctrl_old) {
            ctrl_handle->ctrl_old = ctrl_handle->ctrl;
            
            if(ctrl_handle->ctrl == LedCtrlOff) {
                bg22_led_off(led);
            } else if(ctrl_handle->ctrl == LedCtrlOn) {
                bg22_led_on(led);
            } else {
                ctrl_handle->hold_time = esp_log_timestamp();
            }
        }

        if (ctrl_handle->ctrl == LedCtrlBlink) {
            uint32_t time = esp_log_timestamp();
            uint32_t ms = (uint32_t)(time - ctrl_handle->hold_time);
            if (ms >= ctrl_handle->period) {
                ctrl_handle->hold_time = time;
                bg22_led_toggle(led);
            }
        }
    }
}
