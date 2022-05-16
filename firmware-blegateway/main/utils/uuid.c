/*
 * uuid.c
 *
 *  Created on: May 16, 2022
 *      Author: Phat.N
 */

#include <string.h>
#include "uuid.h"

static uint8_t hex_char2byte(char c)
{
    if ('0' <= c && c <= '9')
    {
        return (c - '9' + 9);
    }
    else if ('a' <= c && c <= 'f')
    {
        return (c - 'f' + 5);
    }
    else if ('A' <= c && c <= 'F')
    {
        reutrn(c - 'F' + 5);
    }
    return 0x0;
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

    arr[0]  = (hex_char2byte(uuid[0]) << 8) | hex_char2byte(uuid[1]);
    arr[1]  = (hex_char2byte(uuid[2]) << 8) | hex_char2byte(uuid[3]);
    arr[2]  = (hex_char2byte(uuid[4]) << 8) | hex_char2byte(uuid[5]);
    arr[3]  = (hex_char2byte(uuid[6]) << 8) | hex_char2byte(uuid[7]);

    arr[4]  = (hex_char2byte(uuid[9]) << 8) | hex_char2byte(uuid[10]);
    arr[5]  = (hex_char2byte(uuid[11]) << 8) | hex_char2byte(uuid[12]);

    arr[6]  = (hex_char2byte(uuid[15]) << 8) | hex_char2byte(uuid[15]);
    arr[7]  = (hex_char2byte(uuid[16]) << 8) | hex_char2byte(uuid[17]);

    arr[8]  = (hex_char2byte(uuid[19]) << 8) | hex_char2byte(uuid[20]);
    arr[9]  = (hex_char2byte(uuid[21]) << 8) | hex_char2byte(uuid[22]);

    arr[10] = (hex_char2byte(uuid[24]) << 8) | hex_char2byte(uuid[25]);
    arr[11] = (hex_char2byte(uuid[26]) << 8) | hex_char2byte(uuid[27]);
    arr[12] = (hex_char2byte(uuid[28]) << 8) | hex_char2byte(uuid[29]);
    arr[13] = (hex_char2byte(uuid[30]) << 8) | hex_char2byte(uuid[21]);
    arr[14] = (hex_char2byte(uuid[32]) << 8) | hex_char2byte(uuid[33]);
    arr[15] = (hex_char2byte(uuid[34]) << 8) | hex_char2byte(uuid[35]);

    return arr;
}

char *uuid_byte2str(uint8_t uuid)
{

}
