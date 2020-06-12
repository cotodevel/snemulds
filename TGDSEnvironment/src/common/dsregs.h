
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


#ifndef __dsregs_h__
#define __dsregs_h__

#include "typedefsTGDS.h"
#include "spitscTGDS.h"


/////////////////////////////////////////////////////////////// Shared
#define SCREEN_HEIGHT 192
#define SCREEN_WIDTH  256

#define		KEYINPUT	(*((uint16 volatile *) 0x04000130))
#define		KEYCNT		(*((uint16 volatile *) 0x04000132))
#define		WAITCNT		(*((uint16 volatile *) 0x04000204))
#define		WAIT_CR		WAITCNT
#define		IME			(*((uint16 volatile *) 0x04000208))
#define		IE			(*((uint32 volatile *) 0x04000210))
#define		IF			(*((uint32 volatile *) 0x04000214))
#define		HALTCNT		(*((uint16 volatile *) 0x04000300))

#define DISP_IN_VBLANK (1 << ( 0 ))
#define DISP_IN_HBLANK (1 << ( 1 ))
#define DISP_YTRIGGERED (1 << ( 2 ))
#define DISP_VBLANK_IRQ (1 << ( 3 ))
#define DISP_HBLANK_IRQ (1 << ( 4 ))
#define DISP_YTRIGGER_IRQ (1 << ( 5 ))

#define KEY_A      (1 << 0)
#define KEY_B      (1 << 1)
#define KEY_SELECT (1 << 2)
#define KEY_START  (1 << 3)
#define KEY_RIGHT  (1 << 4)
#define KEY_LEFT   (1 << 5)
#define KEY_UP     (1 << 6)
#define KEY_DOWN   (1 << 7)
#define KEY_R      (1 << 8)
#define KEY_L      (1 << 9)
#define KEY_X      (1 << 10)
#define KEY_Y      (1 << 11)
#define KEY_TOUCH  (1 << 12)
#define KEY_LID    (1 << 13)

#define EXMEMCNT	WAITCNT
#define REG_BG0CNT	BG0CNT
#define REG_BG1CNT	BG1CNT
#define REG_BG2CNT	BG2CNT
#define REG_BG3CNT	BG3CNT
#define REG_BG0HOFS	BG0HOFS
#define REG_BG1HOFS	BG1HOFS
#define REG_BG2HOFS	BG2HOFS
#define REG_BG3HOFS	BG3HOFS
#define		DISPCNT		(*((uint32 volatile *) 0x04000000))
#define		DISPSTAT	(*((uint16 volatile *) 0x04000004))
#define		VCOUNT		(*((uint16 volatile *) 0x04000006))
#define REG_IE	IE
#define REG_IF	IF
#define REG_IME	IME
#define IME_DISABLE	(uint32)(0)
#define IME_ENABLE	(uint32)(1)

#define REG_DISPSTAT	DISPSTAT
#define REG_DISPCNT		DISPCNT
#define REG_DISPCNT_SUB		DISPCNT2

//NDS ARM7 Power Controller management differs from NDS ARM9 Power Controller management 
#define REG_POWERCNT 	(*(vuint16*)0x4000304)
#define REG_POWERCNT_ADDR	(uint32)(0x04000304)
#define POWER_CR REG_POWERCNT

#define POWERCNT_WIFI 		(uint16)(1<<1)

//misc bits
#define IPC_PEN_DOWN 	(1 << 6)
#define IPC_X 			(1 << 0)
#define IPC_Y 			(1 << 1)
#define IPC_LID_CLOSED 	(1 << 7)

////////////


#define REG_VCOUNT	VCOUNT
#define		CART		((uint16 *) 0x08000000)
#define		DISPCNT2	(*((uint32 volatile *) 0x04001000))
#define		DISPSTAT2	(*((uint16 volatile *) 0x04001004))

//sound
#define SOUND_VOL(n)	(n)
#define SOUND_FREQ(n)	((-0x1000000 / (n)))
#define SOUND_ENABLE	(1<<15)
#define SOUND_REPEAT    (1<<17)
#define SOUND_ONE_SHOT  (1<<28)	
#define SOUND_FORMAT_16BIT (1<<29)
#define SOUND_FORMAT_8BIT	(0<<29)
#define SOUND_FORMAT_PSG    (3<<29)
#define SOUND_FORMAT_ADPCM  (2<<29)
#define SOUND_16BIT      (1<<29)
#define SOUND_8BIT       (0)
#define SOUND_PAN(n)	((n) << 16)
#define SCHANNEL_ENABLE (1<<31)

#define REG_KEYINPUT (*(uint16*)0x04000132)

//use POWERMAN_ARM9 | POWER_XXXX to write to ARM9 regs directly.
#define POWERMAN_ARM9		(uint16)(1<<16)

//DS Memory Control - WRAM
//0-1   ARM9/ARM7 (0-3 = 32K/0K, 2nd 16K/1st 16K, 1st 16K/2nd 16K, 0K/32K)
//2-7   Not used

#define WRAM_32KARM9_0KARM7				(uint8)(0)
#define WRAM_16KARM9_16KARM7FIRSTHALF7 	(uint8)(1)
#define WRAM_16KARM9_16KARM7FIRSTHALF9 	(uint8)(2)
#define WRAM_0KARM9_32KARM7 			(uint8)(3)

#ifdef ARM7
#define WRAM_CR			(*(vuint8*)0x04000241)
#endif

#ifdef ARM9
#define WRAM_CR			(*(vuint8*)0x04000247)
#endif

/////////////////////////////////////////////////////////////// Shared End




/////////////////////////////////////////////////////////////// ARM9
#ifdef ARM9

#define		PAL			((uint16 *) 0x05000000)
#define		VRAMBLOCK1		((uint16 *) 0x06000000)
#define		VRAMBLOCK2		((uint16 *) 0x06200000)

#define		BG0CNT		(*((uint16 volatile *) 0x04000008))
#define		BG1CNT		(*((uint16 volatile *) 0x0400000A))
#define		BG2CNT		(*((uint16 volatile *) 0x0400000C))
#define		BG3CNT		(*((uint16 volatile *) 0x0400000E))
#define		BG0HOFS		(*((uint16 volatile *) 0x04000010))
#define		BG0VOFS		(*((uint16 volatile *) 0x04000012))
#define		BG1HOFS		(*((uint16 volatile *) 0x04000014))
#define		BG1VOFS		(*((uint16 volatile *) 0x04000016))
#define		BG2HOFS		(*((uint16 volatile *) 0x04000018))
#define		BG2VOFS		(*((uint16 volatile *) 0x0400001A))
#define		BG3HOFS		(*((uint16 volatile *) 0x0400001C))
#define		BG3VOFS		(*((uint16 volatile *) 0x0400001E))
#define		BG2PA		(*((uint16 volatile *) 0x04000020))
#define		BG2PB		(*((uint16 volatile *) 0x04000022))
#define		BG2PC		(*((uint16 volatile *) 0x04000024))
#define		BG2PD		(*((uint16 volatile *) 0x04000026))
#define		BG2X		(*((uint32 volatile *) 0x04000028))
#define		BG2Y		(*((uint32 volatile *) 0x0400002C))
#define		BG3PA		(*((uint16 volatile *) 0x04000030))
#define		BG3PB		(*((uint16 volatile *) 0x04000032))
#define		BG3PC		(*((uint16 volatile *) 0x04000034))
#define		BG3PD		(*((uint16 volatile *) 0x04000036))
#define		BG3X		(*((uint32 volatile *) 0x04000038))
#define		BG3Y		(*((uint32 volatile *) 0x0400003C))
#define		WIN0H		(*((uint16 volatile *) 0x04000040))
#define		WIN1H		(*((uint16 volatile *) 0x04000042))
#define		WIN0V		(*((uint16 volatile *) 0x04000044))
#define		WIN1V		(*((uint16 volatile *) 0x04000046))
#define		WININ		(*((uint16 volatile *) 0x04000048))
#define		WINOUT		(*((uint16 volatile *) 0x0400004A))
#define		MOSAIC		(*((uint16 volatile *) 0x0400004C))
#define		BLDCNT		(*((uint16 volatile *) 0x04000050))
#define		BLDALPHA	(*((uint16 volatile *) 0x04000052))
#define		BLDY		(*((uint16 volatile *) 0x04000054))


#define		BG0CNT2		(*((uint16 volatile *) 0x04001008))
#define		BG1CNT2		(*((uint16 volatile *) 0x0400100A))
#define		BG2CNT2		(*((uint16 volatile *) 0x0400100C))
#define		BG3CNT2		(*((uint16 volatile *) 0x0400100E))
#define		BG0HOFS2	(*((uint16 volatile *) 0x04001010))
#define		BG0VOFS2	(*((uint16 volatile *) 0x04001012))
#define		BG1HOFS2	(*((uint16 volatile *) 0x04001014))
#define		BG1VOFS2	(*((uint16 volatile *) 0x04001016))
#define		BG2HOFS2	(*((uint16 volatile *) 0x04001018))
#define		BG2VOFS2	(*((uint16 volatile *) 0x0400101A))
#define		BG3HOFS2	(*((uint16 volatile *) 0x0400101C))
#define		BG3VOFS2	(*((uint16 volatile *) 0x0400101E))
#define		BG2PA2		(*((uint16 volatile *) 0x04001020))
#define		BG2PB2		(*((uint16 volatile *) 0x04001022))
#define		BG2PC2		(*((uint16 volatile *) 0x04001024))
#define		BG2PD2		(*((uint16 volatile *) 0x04001026))
#define		BG2X2		(*((uint32 volatile *) 0x04001028))
#define		BG2Y2		(*((uint32 volatile *) 0x0400102C))
#define		BG3PA2		(*((uint16 volatile *) 0x04001030))
#define		BG3PB2		(*((uint16 volatile *) 0x04001032))
#define		BG3PC2		(*((uint16 volatile *) 0x04001034))
#define		BG3PD2		(*((uint16 volatile *) 0x04001036))
#define		BG3X2		(*((uint32 volatile *) 0x04001038))
#define		BG3Y2		(*((uint32 volatile *) 0x0400103C))
#define		WIN0H2		(*((uint16 volatile *) 0x04001040))
#define		WIN1H2		(*((uint16 volatile *) 0x04001042))
#define		WIN0V2		(*((uint16 volatile *) 0x04001044))
#define		WIN1V2		(*((uint16 volatile *) 0x04001046))
#define		WININ2		(*((uint16 volatile *) 0x04001048))
#define		WINOUT2		(*((uint16 volatile *) 0x0400104A))
#define		MOSAIC2		(*((uint16 volatile *) 0x0400104C))
#define		BLDCNT2		(*((uint16 volatile *) 0x04001050))
#define		BLDALPHA2	(*((uint16 volatile *) 0x04001052))
#define		BLDY2		(*((uint16 volatile *) 0x04001054))

// video memory defines
#define		PAL_BG1		((uint16 *) 0x05000000)
#define		PAL_FG1		((uint16 *) 0x05000200)
#define		PAL_BG2		((uint16 *) 0x05000400)
#define		PAL_FG2		((uint16 *) 0x05000600)

// other video defines
#define		VRAMBANKCNT	(((uint16 volatile *) 0x04000240))

#define		RGB(r,g,b)	( ((r)&31) | (((g)&31)<<5) | (((b)&31)<<10) )
#define		VRAM_SETBANK(bank, set)	\
	if((bank)&1) { VRAMBANKCNT[(bank)>>1] = (VRAMBANKCNT[(bank)>>1]&0x00ff) | (((set)&0xff)<<8); } else \
		{ VRAMBANKCNT[(bank)>>1] = (VRAMBANKCNT[(bank)>>1]&0xff00) | ((set)&0xff); }

		
#define BG_PALETTE       ((uint16*)0x05000000)		
#define BG_PALETTE_SUB   ((uint16*)0x05000400)		

#define SPRITE_PALETTE ((uint16*)0x05000200) 		
#define SPRITE_PALETTE_SUB ((uint16*)0x05000600)	

#define BG_GFX			((uint16*)0x6000000)		
#define BG_GFX_SUB		((uint16*)0x6200000)		
#define SPRITE_GFX			((uint16*)0x6400000)	
#define SPRITE_GFX_SUB		((uint16*)0x6600000)

#define OAM           ((uint16*)0x07000000)
#define OAM_SUB       ((uint16*)0x07000400)

#define RGB15(r,g,b)  ((r)|((g)<<5)|((b)<<10))
#define RGB5(r,g,b)  ((r)|((g)<<5)|((b)<<10))
#define RGB8(r,g,b)  (((r)>>3)|(((g)>>3)<<5)|(((b)>>3)<<10))
#define ARGB16(a, r, g, b) ( ((a) << 15) | (r)|((g)<<5)|((b)<<10))

#define VRAM_CR			(*(vuint32*)0x04000240)
#define VRAM_A_CR		(*(vuint8*)0x04000240)
#define VRAM_B_CR		(*(vuint8*)0x04000241)
#define VRAM_C_CR		(*(vuint8*)0x04000242)
#define VRAM_D_CR		(*(vuint8*)0x04000243)
#define VRAM_EFG_CR		(*(vuint32*)0x04000244)
#define VRAM_E_CR		(*(vuint8*)0x04000244)
#define VRAM_F_CR		(*(vuint8*)0x04000245)
#define VRAM_G_CR		(*(vuint8*)0x04000246)
#define VRAM_H_CR		(*(vuint8*)0x04000248)
#define VRAM_I_CR		(*(vuint8*)0x04000249)

#define REG_BG1VOFS BG1VOFS
#define REG_BG2VOFS BG2VOFS
#define REG_BG3VOFS BG3VOFS

//use POWERMAN_ARM9 | POWER_XXXX to write to ARM9 regs directly.
#define POWER_MATRIX	(1<<2) | POWERMAN_ARM9
#define	POWER_3D_CORE	(1<<3) | POWERMAN_ARM9
//#define POWER_LCD		(1<<0) | POWERMAN_ARM9	//remove and instead use setBacklight per LCD to turn on / off. according to gbatek
#define POWER_2D_A		(1<<1) | POWERMAN_ARM9
#define POWER_2D_B		(1<<9) | POWERMAN_ARM9
#define	POWER_SWAP_LCDS	(1<<15)| POWERMAN_ARM9

#define BG_32x32    (0 << 14)
#define BG_64x32    (1 << 14)
#define BG_32x64    (2 << 14)
#define BG_64x64    (3 << 14)
#define BG_RS_16x16 (0 << 14)
#define BG_RS_32x32   (1 << 14)
#define BG_RS_64x64   (2 << 14)
#define BG_RS_128x128 (3 << 14)
#define BG_BMP8_128x128  ((0 << 14) | (1 << 7))
#define BG_BMP8_256x256  ((1 << 14) | (1 << 7))
#define BG_BMP8_512x256  ((2 << 14) | (1 << 7))
#define BG_BMP8_512x512  ((3 << 14) | (1 << 7))
#define BG_BMP8_1024x512 (1 << 14)
#define BG_BMP8_512x1024 (0)
#define BG_BMP16_128x128  ((0 << 14) | (1 << 7) | (1 << 2))
#define BG_BMP16_256x256  ((1 << 14) | (1 << 7) | (1 << 2))
#define BG_BMP16_512x256  ((2 << 14) | (1 << 7) | (1 << 2))
#define BG_BMP16_512x512  ((3 << 14) | (1 << 7) | (1 << 2))
#define BG_256_COLOR   	(1 << 7)
#define BG_16_COLOR   	(0)
#define BG_MOSAIC_ON   (1 << 6)
#define BG_MOSAIC_OFF  (0)
#define BG_PRIORITY_0  (0)
#define BG_PRIORITY_1  (1)
#define BG_PRIORITY_2  (2)
#define BG_PRIORITY_3  (3)
#define BG_WRAP_OFF    (0)
#define BG_WRAP_ON     (1 << 13)
#define BG_PALETTE_SLOT0 (0)
#define BG_PALETTE_SLOT1 (0)
#define BG_PALETTE_SLOT2 (1 << 13)
#define BG_PALETTE_SLOT3 (1 << 13)
#define BG_COLOR_256		(0x80)
#define BG_COLOR_16			(0x00)

#define	BGCTRL			( (vuint16*)0x4000008)
#define	REG_BGOFFSETS	( (vuint16*)0x4000010)
#define	REG_BG0VOFS		(*(vuint16*)0x4000012)
#define	REG_BG2PA		(*(vsint16*)0x4000020)
#define	REG_BG2PB		(*(vsint16*)0x4000022)
#define	REG_BG2PC		(*(vsint16*)0x4000024)
#define	REG_BG2PD		(*(vsint16*)0x4000026)
#define	REG_BG2X		(*(vsint32*)0x4000028)
#define	REG_BG2Y		(*(vsint32*)0x400002C)
#define	REG_BG3PA		(*(vsint16*)0x4000030)
#define	REG_BG3PB		(*(vsint16*)0x4000032)
#define	REG_BG3PC		(*(vsint16*)0x4000034)
#define	REG_BG3PD		(*(vsint16*)0x4000036)
#define	REG_BG3X		(*(vsint32*)0x4000038)
#define	REG_BG3Y		(*(vsint32*)0x400003C)

#define MAP_BASE_SHIFT 8
#define TILE_BASE_SHIFT 2
#define BG_TILE_BASE(base) ((base) << TILE_BASE_SHIFT)
#define BG_MAP_BASE(base)  ((base) << MAP_BASE_SHIFT)
#define BG_BMP_BASE(base)  ((base) << MAP_BASE_SHIFT)
#define BG_PRIORITY(n) (n)
#define TILE_PALETTE(n) ((n)<<12)
#define TILE_FLIP_H 	(1<<10)
#define TILE_FLIP_V 	(1<<11)


#define BgType_Text8bpp (0)
#define BgType_Text4bpp (1)
#define BgType_Rotation (2)
#define BgType_ExRotation (3)
#define BgType_Bmp8 (4)
#define BgType_Bmp16 (5)
#define BgSize_R_128x128 	(0 << 14)
#define BgSize_R_256x256  	(1 << 14)
#define BgSize_R_512x512 	(2 << 14)
#define BgSize_R_1024x1024 	(3 << 14)

#define BgSize_T_256x256 	(0 << 14) | (1 << 16)
#define BgSize_T_512x256 	(1 << 14) | (1 << 16)
#define BgSize_T_256x512 	(2 << 14) | (1 << 16)
#define BgSize_T_512x512 	(3 << 14) | (1 << 16)

#define BgSize_ER_128x128 	(0 << 14) | (2 << 16)
#define BgSize_ER_256x256 	(1 << 14) | (2 << 16)
#define BgSize_ER_512x512 	(2 << 14) | (2 << 16)
#define BgSize_ER_1024x1024 	(3 << 14) | (2 << 16)

#define BgSize_B8_128x128 		((0 << 14) | (1<<7) | (3 << 16))
#define BgSize_B8_256x256 		((1 << 14) | (1<<7) | (3 << 16))
#define BgSize_B8_512x256 		((2 << 14) | (1<<7) | (3 << 16))
#define BgSize_B8_512x512 		((3 << 14) | (1<<7) | (3 << 16))
#define BgSize_B8_1024x512 		(1 << 14) | (3 << 16)
#define BgSize_B8_512x1024 		(0) | (3 << 16)

#define BgSize_B16_128x128 		((0 << 14) | (1<<7) | (1<<2) | (4 << 16))
#define BgSize_B16_256x256 		((1 << 14) | (1<<7) | (1<<2) | (4 << 16))
#define BgSize_B16_512x256 		((2 << 14) | (1<<7) | (1<<2) | (4 << 16))
#define BgSize_B16_512x512 		((3 << 14) | (1<<7) | (1<<2) | (4 << 16))
 
#define REG_MASTER_BRIGHT     (*(vuint16*)0x0400006C)
#define REG_MASTER_BRIGHT_SUB (*(vuint16*)0x0400106C)


// Window 0
#define WIN0_X0        (*(vuint8*)0x04000041)
#define WIN0_X1        (*(vuint8*)0x04000040)
#define WIN0_Y0        (*(vuint8*)0x04000045)
#define WIN0_Y1        (*(vuint8*)0x04000044)

// Window 1
#define WIN1_X0        (*(vuint8*)0x04000043)
#define WIN1_X1        (*(vuint8*)0x04000042)
#define WIN1_Y0        (*(vuint8*)0x04000047)
#define WIN1_Y1        (*(vuint8*)0x04000046)

#define WIN_IN         (*(vuint16*)0x04000048)
#define WIN_OUT        (*(vuint16*)0x0400004A)

// Window 0
#define SUB_WIN0_X0    (*(vuint8*)0x04001041)
#define SUB_WIN0_X1    (*(vuint8*)0x04001040)
#define SUB_WIN0_Y0    (*(vuint8*)0x04001045)
#define SUB_WIN0_Y1    (*(vuint8*)0x04001044)

// Window 1
#define SUB_WIN1_X0    (*(vuint8*)0x04001043)
#define SUB_WIN1_X1    (*(vuint8*)0x04001042)
#define SUB_WIN1_Y0    (*(vuint8*)0x04001047)
#define SUB_WIN1_Y1    (*(vuint8*)0x04001046)

#define SUB_WIN_IN     (*(vuint16*)0x04001048)
#define SUB_WIN_OUT    (*(vuint16*)0x0400104A)

#define	REG_MOSAIC		(*(vuint16*)0x0400004C)
#define	REG_MOSAIC_SUB	(*(vuint16*)0x0400104C)

#define REG_BLDCNT     (*(vuint16*)0x04000050)
#define REG_BLDY	   (*(vuint16*)0x04000054)
#define REG_BLDALPHA   (*(vuint16*)0x04000052)

#define REG_BLDCNT_SUB     (*(vuint16*)0x04001050)
#define REG_BLDALPHA_SUB   (*(vuint16*)0x04001052)
#define REG_BLDY_SUB	   (*(vuint16*)0x04001054)


#define BLEND_NONE         (0<<6)
#define BLEND_ALPHA        (1<<6)
#define BLEND_FADE_WHITE   (2<<6)
#define BLEND_FADE_BLACK   (3<<6)

#define BLEND_SRC_BG0      (1<<0)
#define BLEND_SRC_BG1      (1<<1)
#define BLEND_SRC_BG2      (1<<2)
#define BLEND_SRC_BG3      (1<<3)
#define BLEND_SRC_SPRITE   (1<<4)
#define BLEND_SRC_BACKDROP (1<<5)

#define BLEND_DST_BG0      (1<<8)
#define BLEND_DST_BG1      (1<<9)
#define BLEND_DST_BG2      (1<<10)
#define BLEND_DST_BG3      (1<<11)
#define BLEND_DST_SPRITE   (1<<12)
#define BLEND_DST_BACKDROP (1<<13)

// Display capture control

#define	REG_DISPCAPCNT		(*(vuint32*)0x04000064)
#define REG_DISP_MMEM_FIFO	(*(vuint32*)0x04000068)

#define DCAP_ENABLE      (1<<31)
#define DCAP_MODE(n)     (((n) & 3) << 29)
#define DCAP_SRC_ADDR(n) (((n) & 3) << 26)
#define DCAP_SRC(n)      (((n) & 3) << 24)
#define DCAP_SRC_A(n)    (((n) & 1) << 24)
#define DCAP_SRC_B(n)    (((n) & 1) << 25)
#define DCAP_SIZE(n)     (((n) & 3) << 20)
#define DCAP_OFFSET(n)   (((n) & 3) << 18)
#define DCAP_BANK(n)     (((n) & 3) << 16)
#define DCAP_B(n)        (((n) & 0x1F) << 8)
#define DCAP_A(n)        (((n) & 0x1F) << 0)

#define DCAP_MODE_A     (0)
#define DCAP_MODE_B     (1)
#define DCAP_MODE_BLEND (2)
#define DCAP_SRC_A_COMPOSITED (0)
#define DCAP_SRC_A_3DONLY (1)
#define DCAP_SRC_B_VRAM (0)
#define DCAP_SRC_B_DISPFIFO (1)
#define DCAP_SIZE_128x128 (0)
#define DCAP_SIZE_256x64 (1)
#define DCAP_SIZE_256x128 (2)
#define DCAP_SIZE_256x192 (3)
#define DCAP_BANK_VRAM_A (0)
#define DCAP_BANK_VRAM_B (1)
#define DCAP_BANK_VRAM_C (2)
#define DCAP_BANK_VRAM_D (3)
#define DISPLAY_SPR_HBLANK			(1 << 23)
#define DISPLAY_SPR_1D_LAYOUT		(1 << 4)
#define DISPLAY_SPR_1D				(1 << 4)
#define DISPLAY_SPR_2D				(0 << 4)
#define DISPLAY_SPR_1D_BMP			(4 << 4)
#define DISPLAY_SPR_2D_BMP_128		(0 << 4)
#define DISPLAY_SPR_2D_BMP_256		(2 << 4)


#define DISPLAY_SPR_1D_SIZE_32		(0 << 20)
#define DISPLAY_SPR_1D_SIZE_64		(1 << 20)
#define DISPLAY_SPR_1D_SIZE_128		(2 << 20)
#define DISPLAY_SPR_1D_SIZE_256		(3 << 20)
#define DISPLAY_SPR_1D_BMP_SIZE_128	(0 << 22)
#define DISPLAY_SPR_1D_BMP_SIZE_256	(1 << 22)
#define DISPLAY_SPRITE_ATTR_MASK  ((7 << 4) | (7 << 20) | (1 << 31))
#define DISPLAY_SPR_EXT_PALETTE		(1 << 31)
#define DISPLAY_BG_EXT_PALETTE		(1 << 30)
#define DISPLAY_SCREEN_OFF     (1 << 7)
#define DISPLAY_CHAR_BASE(n) (((n)&7)<<24)
#define DISPLAY_SCREEN_BASE(n) (((n)&7)<<27)

#endif



//////////////////////////////////////////////////////////////////////////
// ARM7 specific registers
#ifdef ARM7
#define REG_KEYXY 		(*(vuint16*)0x04000136)
#define	POWERCNT7	(*((uint16 volatile *) 0x04000304))

#define REG_SOUNDCNT 	(*(vuint16*)0x4000500)
#define SOUND_CR			REG_SOUNDCNT
#define SCHANNEL_CR(n)				(*(vuint32*)(0x04000400 + ((n)<<4)))
#define SCHANNEL_VOL(n)				(*(vuint8*)(0x04000400 + ((n)<<4)))
#define SCHANNEL_PAN(n)				(*(vuint8*)(0x04000402 + ((n)<<4)))
#define SCHANNEL_SOURCE(n)			(*(vuint32*)(0x04000404 + ((n)<<4)))
#define SCHANNEL_TIMER(n)			(*(vuint16*)(0x04000408 + ((n)<<4)))
#define SCHANNEL_REPEAT_POINT(n)	(*(vuint16*)(0x0400040A + ((n)<<4)))
#define SCHANNEL_LENGTH(n)			(*(vuint32*)(0x0400040C + ((n)<<4)))


//touch
#define TSC_MEASURE_TEMP1    0x84
#define TSC_MEASURE_Y        0x90
#define TSC_MEASURE_BATTERY  0xA4
#define TSC_MEASURE_Z1       0xB4
#define TSC_MEASURE_Z2       0xC4
#define TSC_MEASURE_X        0xD0
#define TSC_MEASURE_AUX      0xE4
#define TSC_MEASURE_TEMP2    0xF4

//power controller
//use POWERMAN_ARM7 | POWER_XXXX to write to ARM7 regs directly.
#define POWERMAN_ARM7		0				//only a mask for writing to ARM7/ARM9 IO
#define POWER_SOUND (1<<0)					//sound controller power bit (hw)

#define PM_CONTROL_REG		(int)(0)	//PM control register
#define	PM_BATTERY_REG		(int)(1)	//PM battery register
#define	PM_AMPLIFIER_REG	(int)(2)	//PM amplifier register
#define PM_READ_REGISTER	(int)(1<<7)	//PM read register
#define	PM_AMP_OFFSET		(int)(2)	//PM amp register
#define PM_GAIN_OFFSET		(int)(3)	//PM gain register
#define PM_BACKLIGHT_LEVEL	(int)(4)	//DS Lite backlight register
#define PM_GAIN_20			(int)(0)	//mic gain to 20db
#define	PM_GAIN_40			(int)(1)	//mic gain to 40db
#define PM_GAIN_80			(int)(2)	//mic gain to 80db
#define PM_GAIN_160			(int)(3)	//mic gain to 160db
#define PM_AMP_ON			(int)(1)	//sound amp on
#define PM_AMP_OFF			(int)(0)	//Turns the sound amp off

#define GAIN_20           0
#define GAIN_40           1
#define GAIN_80           2
#define GAIN_160          3

//sound
#define REG_SOUNDCNT		(*(vuint16*)0x4000500)	//#define SOUND_CR          (*(vuint16*)0x04000500)
#define REG_MASTER_VOLUME	(*(vuint8*)0x4000500)	
#define REG_SOUNDBIAS		(*(vuint32*)0x4000504)
#define SOUND_BIAS        (*(vuint16*)0x04000504)
#define SOUND508          (*(vuint16*)0x04000508)
#define SOUND510          (*(vuint16*)0x04000510)
#define SOUND514                  (*(vuint16*)0x04000514)
#define SOUND518          (*(vuint16*)0x04000518)
#define SOUND51C          (*(vuint16*)0x0400051C)
#define	PM_SOUND_AMP		(1<<0)
#define	PM_SOUND_MUTE		(1<<1)

//LED
#define PM_LED_CONTROL(m)  ((m)<<4)

#endif
//ARM7 END

// End of file!
#endif


#ifdef __cplusplus
extern "C"{
#endif


#ifdef __cplusplus
}
#endif