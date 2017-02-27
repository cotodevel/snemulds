/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006-2007 archeide, All rights reserved. */
/***********************************************************/
/*
This program is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License as 
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
GNU General Public License for more details.
*/

#ifndef __common_h__
#define __common_h__

#define SNEMULDS_TITLE "-= SNEmulDS 0.6 by archeide =-\n"
#define SNEMULDS_SUBTITLE "CPU: bubble2k Sound: gladius\n"
#define GAMES_DIR "/SNES/"

//count/time VCOUNT
#define TIMER_Y
#define IN_DTCM __attribute__((section(".dtcm")))
#define IN_ITCM __attribute__((section(".itcm")))


#include <nds/timers.h>

#ifndef TIMER_Y
#define START_PROFILE(name, cnt) \
	SNES.stat_before##cnt = TIMER3_DATA;
	
#define END_PROFILE(name, cnt) \
{ \
   uint16 tmp = TIMER3_DATA; \
	if (SNES.stat_before##cnt > tmp) \
		SNES.stat_##name += 65536+tmp-SNES.stat_before##cnt; \
	else \
		SNES.stat_##name += tmp-SNES.stat_before##cnt; \
}
#else
	#define START_PROFILE(name, cnt)
	#define END_PROFILE(name, cnt) 
#endif

#ifdef VRAM
	#undef VRAM
#endif

#ifdef SRAM
	#undef SRAM
#endif

#ifdef LIBNDS
#define PM_BACKLIGHT_BOTTOM  BIT(2)    // Enable the top backlight if set
#define PM_BACKLIGHT_TOP     BIT(3)    // Enable the bottom backlight if set
#endif

#ifdef WIN32
#define STATIC_INLINE static _inline
#else
#define STATIC_INLINE static inline
#endif

#if (TRUE != 1)
#undef TRUE
#define TRUE    1
#endif

#if (FALSE != 0)
#undef FALSE
#define FALSE   0
#endif

typedef unsigned short	ushort;
typedef unsigned char	uchar;

#ifndef _NDSTYPES_INCLUDE
typedef	unsigned int	uint32;
typedef	unsigned short	uint16;
typedef	uchar			uint8;
#endif

typedef	int				sint32;
typedef	short			sint16;
typedef	char			sint8;
typedef	char			bool8;

#define GET_WORD16(a) (*((uint8 *)(a)) | (*(((uint8 *)(a))+1) << 8)) 
#define SET_WORD16(a, v) { *((uint8 *)(a)) = (v) & 0xFF; *(((uint8 *)(a))+1) = (v) >> 8; } 

#ifndef MIN
#define MIN(a, b) (((a) < (b))?(a):(b))
#define MAX(a, b) (((a) > (b))?(a):(b))
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

int	setBacklight(int flags);

#ifdef __cplusplus
}
#endif

