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


//count/time VCOUNT
//#define SNEMUL_LOGGING

//#define TIMER_Y	//isnt TIMER3 used for wifi? NO cant enable this here
//#define IN_DTCM __attribute__((section(".dtcm")))
//#define IN_ITCM __attribute__((section(".itcm")))

#include "typedefsTGDS.h"
/*
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
*/

#ifdef VRAM
	#undef VRAM
#endif

#ifdef SRAM
	#undef SRAM
#endif

#ifdef WIN32
#define STATIC_INLINE static _inline
#else
#define STATIC_INLINE static inline
#endif

#undef TRUE
#define TRUE    1

#undef FALSE
#define FALSE   0


#define GET_WORD16(a) (*((uint8 *)(a)) | (*(((uint8 *)(a))+1) << 8)) 
#define SET_WORD16(a, v) { *((uint8 *)(a)) = (v) & 0xFF; *(((uint8 *)(a))+1) = (v) >> 8; } 

typedef struct s_Options
{
	uint8 BG3Squish :2;
	uint8 SoundOutput :1;
	uint8 LayersConf :6;
	uint8 TileMode :1;
	uint8 BG_Layer :8;
	uint8 YScroll :2;
	uint8 WaitVBlank :1;
	uint8 SpeedHack :3;
} t_Options;

#endif

#ifdef __cplusplus
extern "C" {
#endif

int	setBacklight(int flags);

#ifdef __cplusplus
}
#endif
