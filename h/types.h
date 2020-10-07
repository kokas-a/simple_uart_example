/*
 * types.h
 *
 *  Created on: 7.10.2020
 *      Author: user
 */

#ifndef H_TYPES_H_
#define H_TYPES_H_

typedef __signed__ char s8;
typedef unsigned char u8;

typedef s8 int8_t;
typedef u8 uint8_t;

typedef __signed__ short s16;
typedef unsigned short u16;

typedef s16 int16_t;
typedef u16 uint16_t;

typedef __signed__ int s32;
typedef unsigned int u32;

typedef s32 int32_t;
typedef u32 uint32_t;

typedef int32_t status_t;

#ifndef NULL
#define NULL ((void *)0)
#endif

#endif /* H_TYPES_H_ */
