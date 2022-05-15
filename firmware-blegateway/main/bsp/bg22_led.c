/*
 * bg22_led.c
 *
 *  Created on: May 14, 2022
 *      Author: Phat.N
 */

#include "bg22_led.h"
#include "bg22_config.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BG22_LED_PIN_SEL    ((1ull << BG22_GPIO_LED_GREEN_IO) | \
                             (1ull << BG22_GPIO_LED_RED_IO) |   \
                             (1ull << BG22_GPIO_LED_BLUE_IO))

const static uint8_t leds[] = {
    [LedRed]   = BG22_GPIO_LED_RED_IO,
    [LedBlue]  = BG22_GPIO_LED_BLUE_IO,
    [LedGreen] = BG22_GPIO_LED_GREEN_IO
};

static uint8_t led_states[_LedNum];

void bg22_led_init(void)
{
    gpio_config_t config = {};
    config.intr_type    = GPIO_INTR_DISABLE;
    config.mode         = GPIO_MODE_OUTPUT;
    config.pin_bit_mask = BG22_LED_PIN_SEL;
    config.pull_down_en = 0;
    config.pull_up_en   = 1;
    ESP_ERROR_CHECK(gpio_config(&config));
}

void bg22_led_on(led_t led) {
    bg22_assert(led < _LedNum);
    gpio_set_level(leds[led], 1);
    led_states[led] = 1;
}

void bg22_led_off(led_t led) {
    bg22_assert(led < _LedNum);
    gpio_set_level(leds[led], 0);
    led_states[led] = 0;
}

void bg22_led_toggle(led_t led) {
    bg22_assert(led < _LedNum);
    if(led_states[led]) {
        bg22_led_off(led);
    } else {
        bg22_led_on(led);
    }
}