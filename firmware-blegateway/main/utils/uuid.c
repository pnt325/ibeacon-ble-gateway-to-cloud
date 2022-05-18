/*
 * uuid.c
 *
 *  Created on: May 16, 2022
 *      Author: Phat.N
 */

#include <stdio.h>
#include <string.h>
#include "uuid.h"

static uint8_t hex_char2byte(char c)
{
    uint8_t val = 0;
    if ('0' <= c && c <= '9')
    {
        val = (c - '0');
    }
    else if ('a' <= c && c <= 'f')
    {
        val = (c - 'a' + 10);
    }
    else if ('A' <= c && c <= 'F')
    {
        val = (c - 'A' + 10);
    }
    return val;
}

uint8_t *uuid_str2byte(const char *uuid)
{
    // 00000000-0000-0000-0000-000000000000
    static uint8_t arr[16];

    if (strlen(uuid) < 36)
    {
        return NULL;
    }

    // Verify the UUID format
    for(uint8_t i = 0; i < 36; i++) 
    {
        if (i == 8 || i == 13 || i == 18 || i == 23)
        {
            continue;
        }

        if (('0' <= uuid[i] && uuid[i] <= '9') ||
            ('a' <= uuid[i] && uuid[i] <= 'f') ||
            ('A' <= uuid[i] && uuid[i] <= 'F'))
        {
            continue;
        }
        else 
        {
            return NULL;
        }
    }

    arr[0]  = (hex_char2byte(uuid[0]) << 4) | hex_char2byte(uuid[1]);
    arr[1]  = (hex_char2byte(uuid[2]) << 4) | hex_char2byte(uuid[3]);
    arr[2]  = (hex_char2byte(uuid[4]) << 4) | hex_char2byte(uuid[5]);
    arr[3]  = (hex_char2byte(uuid[6]) << 4) | hex_char2byte(uuid[7]);

    arr[4]  = (hex_char2byte(uuid[9])  << 4) | hex_char2byte(uuid[10]);
    arr[5]  = (hex_char2byte(uuid[11]) << 4) | hex_char2byte(uuid[12]);

    arr[6]  = (hex_char2byte(uuid[14]) << 4) | hex_char2byte(uuid[15]);
    arr[7]  = (hex_char2byte(uuid[16]) << 4) | hex_char2byte(uuid[17]);

    arr[8]  = (hex_char2byte(uuid[19]) << 4) | hex_char2byte(uuid[20]);
    arr[9]  = (hex_char2byte(uuid[21]) << 4) | hex_char2byte(uuid[22]);

    arr[10] = (hex_char2byte(uuid[24]) << 4) | hex_char2byte(uuid[25]);
    arr[11] = (hex_char2byte(uuid[26]) << 4) | hex_char2byte(uuid[27]);
    arr[12] = (hex_char2byte(uuid[28]) << 4) | hex_char2byte(uuid[29]);
    arr[13] = (hex_char2byte(uuid[30]) << 4) | hex_char2byte(uuid[21]);
    arr[14] = (hex_char2byte(uuid[32]) << 4) | hex_char2byte(uuid[33]);
    arr[15] = (hex_char2byte(uuid[34]) << 4) | hex_char2byte(uuid[35]);

    return arr;
}

char *uuid_byte2str(uint8_t *uuid)
{
    static char buf[37];
    if (!uuid)
    {
        return NULL;
    }

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7], uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);

    return buf;
}
