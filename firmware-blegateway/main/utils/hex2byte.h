/*
 * hex2byte.h
 *
 *  Created on: May 16, 2022
 *      Author: Phat.N
 */

#ifndef _HEX2BYTE_H_
#define _HEX2BYTE_H_

#include <stdint.h>

typedef struct 
{
    uint8_t data;
    uint8_t len;
} hex_t;

hex_t hex2byte_convert(const char* data);

#endif /*_HEX2BYTE_H_*/