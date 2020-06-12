/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

*/

#ifndef __dmaTGDS_h__
#define __dmaTGDS_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#define		DMAINCR_DEST	(uint32)(0<<(16+5))	//aka DMA_DST_INC
#define		DMADECR_DEST	(uint32)(1<<(16+5))
#define		DMAFIXED_DEST	(uint32)(2<<(16+5))

#define		DMAINCR_SRC		(uint32)(0<<(16+7))	//aka DMA_SRC_INC
#define		DMADECR_SRC		(uint32)(1<<(16+7))
#define		DMAFIXED_SRC	(uint32)(2<<(16+7))	//aka DMA_SRC_FIX

#define 	DMA16BIT   	(uint32)(0 << 26)	//aka DMA_16_BIT or COPY_MODE_HWORD
#define 	DMA32BIT   	(uint32)(1 << 26)	//aka DMA_32_BIT or COPY_MODE_WORD 

#define 	COPY_FIXED_SOURCE (uint32)(1<<24)	//aka COPY_MODE_FILL
#define 	COPY_SRCDEST_DMA (uint32)(0)		//aka COPY_MODE_COPY

#define		DMASTART_INMEDIATE	(uint32)(0<<(16+12))	//aka DMA_START_NOW
#define		DMASTART_VBLANK		(uint32)(1<<(16+12))
#define		DMASTART_HBLANK		(uint32)(2<<(16+12))
#define		DMASTART_SPECIAL	(uint32)(3<<(16+12))

#define		DMAENABLED	(uint32)(1<<(16+15))	//DMAs disable itself when transfer done	//aka DMA_BUSY	//aka DMA_ENABLE

#define 	DMARAISEIRQ     (uint32)(1<<(16+14))	//DMAs will raise a DMA IRQ when Word Count fed (transfer) is complete

//word count
#define DMA0CNT_L (vuint16*)(0x040000B8)
#define DMA1CNT_L (vuint16*)(0x040000C4)
#define DMA2CNT_L (vuint16*)(0x040000D0)
#define DMA3CNT_L (vuint16*)(0x040000DC)

//cnt
#define DMA0CNT_H	(vuint16*)(0x040000BA) 
#define DMA1CNT_H	(vuint16*)(0x040000C6) 
#define DMA2CNT_H	(vuint16*)(0x040000D2) 
#define DMA3CNT_H	(vuint16*)(0x040000DE) 


#define DMAXSAD(n)     (*(vuint32*)(0x040000B0+(n*12)))
#define DMAXDAD(n)    (*(vuint32*)(0x040000B4+(n*12)))
#define DMAXCNT(n)      (*(vuint32*)(0x040000B8+(n*12)))	//DMAXCNT_L (0xB8 & 0xB9) DMAXCNT_H (0xBA & 0xBB)


//40000E(X*4)h - NDS9 only - DMAXFILL - DMA X Filldata (R/W)
#ifdef ARM9
#define DMAXFILL(dmaindx)      (*(vuint32*)(0x040000E0+(dmaindx*0x4)))
#endif

//DMA:
static inline void dmaFill(sint32 dmachannel,uint32 value, uint32 dest, uint32 mode){
#ifdef ARM7	
	DMAXSAD(dmachannel) = (uint32)&value;
#endif

#ifdef ARM9
	DMAXFILL(dmachannel) = (vuint32)value;
	DMAXSAD(dmachannel) = (uint32)&DMAXFILL(dmachannel);
#endif
	DMAXDAD(dmachannel) = (uint32)dest;
	DMAXCNT(dmachannel) = mode;
	while(DMAXCNT(dmachannel) & DMAENABLED);
}

static inline void dmaTransfer(sint32 dmachannel, uint32 source, uint32 dest, uint32 mode){	
	DMAXSAD(dmachannel)= source;
	DMAXDAD(dmachannel)= dest;
	DMAXCNT(dmachannel)= mode;	
	while(DMAXCNT(dmachannel) & DMAENABLED);
}

#endif

#ifdef __cplusplus
extern "C"{
#endif

extern void dmaFillWord(sint32 dmachannel,uint32 value, uint32 dest, uint32 word_count);
extern void dmaFillHalfWord(sint32 dmachannel,uint32 value, uint32 dest, uint32 word_count);
extern void dmaTransferHalfWord(sint32 dmachannel, uint32 source, uint32 dest, uint32 word_count);
extern void dmaTransferWord(sint32 dmachannel, uint32 source, uint32 dest, uint32 word_count);

#ifdef __cplusplus
}
#endif
