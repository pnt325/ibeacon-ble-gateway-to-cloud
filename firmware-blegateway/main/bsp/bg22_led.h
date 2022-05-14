/*
 * bg22_led.c
 *
 *  Created on: May 14, 2022
 *      Author: Phat.N
 */

#ifndef _BG22_LED_H_
#define _BG22_LED_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef enum 
{
    LedRed,
    LedGreen,
    LedBlue,
    _LedNum
} led_t;

/**
 * @brief Initalize LED
 */
void bg22_led_init(void);

/**
 * @brief Turn LED on
 * @param led led_t
 */
void bg22_led_on(led_t led);

/**
 * @brief Turn OFF led
 * @param led led_t
 */
void bg22_led_off(led_t led);

/**
 * @brief Toggle LED
 * @param led led_t
 */
void bg22_led_toggle(led_t led);

#endif /*_BG22_LED_H_*/