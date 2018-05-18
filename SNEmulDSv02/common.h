/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* Free for non-commercial use                             */
/***********************************************************/

#include <nds.h>

#ifndef __common_h__
#define __common_h__

#define SNEMULDS_TITLE "-= SNEmulDS 0.2 by archeide =-"

#define TIMER_Y

#define IN_DTCM __attribute__((section(".dtcm")))
#define IN_ITCM __attribute__((section(".itcm")))

#define IN_ITCM2
//#define IN_ITCM __attribute__((section(".itcm")))

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


#ifdef __cplusplus
extern "C" {
#endif

extern uchar	PPU_port_read(u32 address);


#ifdef __cplusplus
}
#endif
