/*
 * bg22_config.h
 *
 *  Created on: May 12, 2022
 *      Author: Phat.N
 */

#ifndef _BG22_CONFIG_H_
#define _BG22_CONFIG_H_

#include <stdint.h>

/* GPIO configuration */
#define BG22_GPIO_BUTTON_IO     9
#define BG22_GPIO_LED_GREEN_IO  4
#define BG22_GPIO_LED_RED_IO    3
#define BG22_GPIO_LED_BLUE_IO   5

#define BG22_DEBUG

#ifdef BG22_DEBUG
#ifndef bg22_assert
#include "esp_err.h"
#define bg22_assert(__v)                \
    do{                                 \
        if(!(__v)){                        \
            ESP_ERROR_CHECK(ESP_FAIL);  \
        }                               \
    }while(0)
#endif 
#else 
#define bg22_assert(__v)
#endif

#endif /*_BG22_CONFIG_H_*/