/*
 * bg22_button.c
 *
 *  Created on: May 12, 2022
 *      Author: Phat.N
 */

#include "bg22_button.h"
#include "bg22_config.h"

#include "esp_err.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct 
{
    uint8_t           pin;
    button_callback_t callback;
    volatile bool     enabled;
} button_t;

static volatile button_t buttons[] = {
    [ButtonUser] = {.pin = BG22_GPIO_BUTTON_IO, .callback = NULL, .enabled = false}
};
static bool install_gpio_isr_service = true;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    for(uint8_t i = 0; i < _ButtonNum; i++) {
        if(buttons[i].pin == gpio_num && buttons[i].enabled) {
            if(buttons[i].callback) {
                buttons[i].callback(bg22_button_state_get(i) ,i);
            }
            break;
        }
    }
}

void bg22_button_init(button_type_t type, button_callback_t callback) {
    bg22_assert(type < _ButtonNum);

    gpio_config_t config = {};
    config.intr_type    = GPIO_INTR_ANYEDGE;
    config.mode         = GPIO_MODE_INPUT;
    config.pin_bit_mask = (1ULL << buttons[type].pin);
    config.pull_down_en = 0;
    config.pull_up_en   = 1;

    buttons[type].callback = callback;
    buttons[type].enabled  = false;  

    ESP_ERROR_CHECK(gpio_config(&config));
    if(install_gpio_isr_service) {
        install_gpio_isr_service = false;
        ESP_ERROR_CHECK(gpio_install_isr_service(0));
    }
    
    ESP_ERROR_CHECK(gpio_isr_handler_add(buttons[type].pin, gpio_isr_handler, (void*)buttons[type].pin));
}

void bg22_button_deinit(button_type_t type) {
    bg22_assert(type < _ButtonNum);

    ESP_ERROR_CHECK(gpio_isr_handler_remove(buttons[type].pin));

    gpio_config_t config = {};
    config.mode = GPIO_MODE_DISABLE;
    config.pin_bit_mask = (1ULL << buttons[type].pin);
    config.pull_down_en = 0;
    config.pull_up_en   = 1;
    ESP_ERROR_CHECK(gpio_config(&config));
}

void bg22_button_enabled(button_type_t type){
    bg22_assert(type < _ButtonNum);

    if (buttons[type].callback) {
        buttons[type].enabled = true;
    }
}

void bg22_button_disabled(button_type_t type) {
    bg22_assert(type < _ButtonNum);
    
    if (buttons[type].callback) {
        buttons[type].enabled = false;
    }
}

button_state_t bg22_button_state_get(button_type_t type) {
    bg22_assert(type < _ButtonNum);

    if(gpio_get_level(buttons[type].pin)) {
        return ButtonReleased;
    }
    return ButtonPressed;
}