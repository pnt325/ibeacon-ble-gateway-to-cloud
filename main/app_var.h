/*
 * app_var.h
 *
 *  Created on: May 06, 2022
 *      Author: Phat.N
 */

#ifndef _APP_VAR_H_
#define _APP_VAR_H_

#include <stdint.h>

typedef union
{
    uint8_t buf[4];
    uint8_t u8;
    int8_t i8;
    uint16_t u16;
    int16_t i16;
    uint32_t u32;
    int32_t i32;
    float single;
} app_var_t;

#endif /*_APP_VAR_H_*/
