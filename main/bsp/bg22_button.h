/*
 * bg22_button.h
 *
 *  Created on: May 12, 2022
 *      Author: Phat.N
 */

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "bg22_config.h"

#include "driver/gpio.h"

/* Button state */
typedef enum 
{
    ButtonPressed,
    ButtonReleased
} button_state_t;

typedef enum 
{
    ButtonUser,
    _ButtonNum
} button_type_t;

/* Button callback */
typedef void(*button_callback_t)(button_state_t state, button_type_t type);

/**
 * @brief Button Initiailize
 * @param type     button_type_t
 * @param callback button_callback_t
 */
void bg22_button_init(button_type_t type, button_callback_t callback);

/**
 * @brief Button DeInitialize
 * @param type button_type_t
 */
void bg22_button_deinit(button_type_t type);

/**
 * @brief Button enabled
 * @param type      button_type_t
 */
void bg22_button_enabled(button_type_t type);

/**
 * @brief Button disabled
 * @param type button_type_t
 */
void bg22_button_disabled(button_type_t type);

/**
 * @brief Button get state
 * @param  type button_type_t
 * @return      button_state_t
 */
button_state_t bg22_button_state_get(button_type_t type);

#endif /*_BUTTON_H_*/