/*
 * uuid.h
 *
 *  Created on: May 16, 2022
 *      Author: Phat.N
 */

#ifndef _UUID_H_
#define _UUID_H_

#include <stdint.h>

uint8_t* uuid_str2byte(const char* uuid);
char* uuid_byte2str(uint8_t* uuid);

#endif /*_UUID_H_*/