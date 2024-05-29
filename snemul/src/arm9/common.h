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

//#define ASM_OPCODES

/* Enable this if you want the GBFS version (the one which doesn't use the flash cart) */
//#define USE_GBFS
#define USE_LIBFAT
//#define FAKE_FS

#define USE_EXTRAM

//#define DSEMUL_BUILD

#ifndef DSEMUL_BUILD
#define SNEMULDS_TITLE "-= SNEmulDS 0.6 by archeide =-\n"
#else
#define SNEMULDS_TITLE "DSEmul - SNES on DS\n"
#endif
#define SNEMULDS_SUBTITLE "CPU: bubble2k Sound: gladius\n"

#ifndef IN_EMULATOR
#define GAMES_DIR "/SNES/"
#else
#define GAMES_DIR "/"
#endif

#define TIMER_Y

#ifdef IN_EMULATOR
#define IN_DTCM
#define IN_ITCM
#define IN_ITCM2
#define IN_ITCM3

#else

#define IN_DTCM __attribute__((section(".dtcm")))
#define IN_ITCM __attribute__((section(".itcm")))


#ifdef ASM_OPCODES
#define IN_ITCM2 __attribute__((section(".itcm")))
#define IN_ITCM3 __attribute__((section(".itcm")))
#else
#define IN_ITCM3
#define IN_ITCM2 
#endif

#endif

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


#define PM_BACKLIGHT_BOTTOM  BIT(2)    // Enable the top backlight if set
#define PM_BACKLIGHT_TOP     BIT(3)    // Enable the bottom backlight if set
int	setBacklight(int flags);

//#include <>

#ifdef WIN32
#define STATIC_INLINE static _inline
#else
#define STATIC_INLINE static inline
#endif


#undef TRUE
#define TRUE    1
#undef FALSE
#define FALSE   0

typedef
	unsigned char	uchar;

/*
#ifdef WIN32

#endif
*/
#ifndef NDS_JTYPES_INCLUDE
typedef	unsigned int	uint32;
typedef	unsigned short	uint16;
typedef	uchar	uint8;
#endif
typedef	int	sint32;
typedef	short	sint16;
typedef	char	sint8;
typedef	char	bool8;

#define GET_WORD16(a) (*((uint8 *)(a)) | (*(((uint8 *)(a))+1) << 8)) 
#define SET_WORD16(a, v) { *((uint8 *)(a)) = (v) & 0xFF; *(((uint8 *)(a))+1) = (v) >> 8; } 

#ifndef MIN
#define MIN(a, b) (((a) < (b))?(a):(b))
#define MAX(a, b) (((a) > (b))?(a):(b))
#endif

#endif
