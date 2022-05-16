/*
 * hex2byte.c
 *
 *  Created on: May 16, 2022
 *      Author: Phat.N
 */

#include <stdint.h>
#include <stdio.h>
#include "hex2byte.h"

// 00000000-0000-0000-0000-000000000000

hex_t hex2byte_convert(const char* data) 
{
    hex_t hex = {};
    uint8_t len = strlen(data);
    hex.data = (uint8_t *)malloc(len/2);
    if (hex == NULL)
    {
        return NULL;
    }

    // make sure the hex format
    for(uint8_t i = 0; i < len; i++) 
    {
        if (('0' <= data[i] && data[i] <= '9') ||
            ('a' <= data[i] && data[i] <= 'f') || 
            ('A' <= data[i] && data[i] <= 'F'))
        {
            continue;
        }
        else 
        {
            return NULL;    
        }
    }




    return hex;
}
