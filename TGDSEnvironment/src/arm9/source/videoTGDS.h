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

#ifndef __nds_video_h__
#define __nds_video_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <stdbool.h>

#define VRAM_ENABLE		(uint8)(1<<7)
#define VRAM_MAX_LETTER_INDEX	(sint32)(9)
#define VRAM_A_INDEX	(sint32)(0)
#define VRAM_B_INDEX	(sint32)(1)
#define VRAM_C_INDEX	(sint32)(2)
#define VRAM_D_INDEX	(sint32)(3)
#define VRAM_E_INDEX	(sint32)(4)
#define VRAM_F_INDEX	(sint32)(5)
#define VRAM_G_INDEX	(sint32)(6)
#define VRAM_H_INDEX	(sint32)(7)
#define VRAM_I_INDEX	(sint32)(8)

typedef struct vramBankSetup{
	uint8 vrambankCR;	//a,b,c,d,e,f,g,h,i
	bool enabled;	//true = yes /false = no
	//sint32	setupMode;	//if mapped should setup accordingly by vramsetup(n)
}vramBankSetup;

typedef struct vramSetup{
	vramBankSetup vramBankSetupInst[VRAM_MAX_LETTER_INDEX];
}vramSetup;

//VRAM Setup 0: //MST 0
/*
 VRAM    SIZE  MST  OFS   ARM9, Plain ARM9-CPU Access (so-called LCDC mode)
  A       128K  0    -     6800000h-681FFFFh
  B       128K  0    -     6820000h-683FFFFh
  C       128K  0    -     6840000h-685FFFFh
  D       128K  0    -     6860000h-687FFFFh
  E       64K   0    -     6880000h-688FFFFh
  F       16K   0    -     6890000h-6893FFFh
  G       16K   0    -     6894000h-6897FFFh
  H       32K   0    -     6898000h-689FFFFh
  I       16K   0    -     68A0000h-68A3FFFh
 */

#define VRAM_A_LCDC_MODE 	(uint8)(0)
#define VRAM_B_LCDC_MODE 	(uint8)(0)
#define VRAM_C_LCDC_MODE 	(uint8)(0)
#define VRAM_D_LCDC_MODE 	(uint8)(0)
#define VRAM_E_LCDC_MODE 	(uint8)(0)
#define VRAM_F_LCDC_MODE 	(uint8)(0)
#define VRAM_G_LCDC_MODE 	(uint8)(0)
#define VRAM_H_LCDC_MODE 	(uint8)(0)
#define VRAM_I_LCDC_MODE 	(uint8)(0)

//note: ENGINE_A == "MAIN BG ENGINE" , ENGINE_B "SUB BG ENGINE"
//VRAM Setup 1: //MST 1
//ARM9, 2D Graphics Engine A, BG-VRAM (max 512K)
//Vram A (128K)
#define VRAM_A_0x06000000_ENGINE_A_BG 	(uint8)(1 | (0 << 3))	//OFS 0..3
#define VRAM_A_0x06020000_ENGINE_A_BG 	(uint8)(1 | (1 << 3))
#define VRAM_A_0x06040000_ENGINE_A_BG 	(uint8)(1 | (2 << 3))
#define VRAM_A_0x06060000_ENGINE_A_BG 	(uint8)(1 | (3 << 3))

//Vram B (128K)
#define VRAM_B_0x06000000_ENGINE_A_BG 	(uint8)(1 | (0 << 3))	//OFS 0..3
#define VRAM_B_0x06020000_ENGINE_A_BG 	(uint8)(1 | (1 << 3))
#define VRAM_B_0x06040000_ENGINE_A_BG 	(uint8)(1 | (2 << 3))
#define VRAM_B_0x06060000_ENGINE_A_BG 	(uint8)(1 | (3 << 3))

//Vram C (128K)
#define VRAM_C_0x06000000_ENGINE_A_BG 	(uint8)(1 | (0 << 3))	//OFS 0..3
#define VRAM_C_0x06020000_ENGINE_A_BG 	(uint8)(1 | (1 << 3))
#define VRAM_C_0x06040000_ENGINE_A_BG 	(uint8)(1 | (2 << 3))
#define VRAM_C_0x06060000_ENGINE_A_BG 	(uint8)(1 | (3 << 3))

//Vram D (128K)
#define VRAM_D_0x06000000_ENGINE_A_BG 	(uint8)(1 | (0 << 3))	//OFS 0..3
#define VRAM_D_0x06020000_ENGINE_A_BG 	(uint8)(1 | (1 << 3))
#define VRAM_D_0x06040000_ENGINE_A_BG 	(uint8)(1 | (2 << 3))
#define VRAM_D_0x06060000_ENGINE_A_BG 	(uint8)(1 | (3 << 3))

//Vram E  64K   1    -     6000000h
#define VRAM_E_0x06000000_ENGINE_A_BG 	(uint8)(1)		//static @ 0x06000000

//Vram F (16K), takes in account OFS0 and OFS1 : 06000000h+(4000h*OFS.0)+(10000h*OFS.1)
#define VRAM_F_0x06000000_ENGINE_A_BG  	(uint8)(1 | (( 0 )<<3))	//OFS 0
#define VRAM_F_0x06004000_ENGINE_A_BG 	(uint8)(1 | (( 1 )<<3))	//OFS 1
#define VRAM_F_0x06010000_ENGINE_A_BG  	(uint8)(1 | (( 2 )<<3))	//OFS 2
#define VRAM_F_0x06014000_ENGINE_A_BG  	(uint8)(1 | (( 3 )<<3))	//OFS 3

//Vram G (16K),	takes in account OFS0 and OFS1 : 06000000h+(4000h*OFS.0)+(10000h*OFS.1)
#define VRAM_G_0x060XXXXX_ENGINE_A_BG 	(uint8)(1)	//OFS 0..3


//VRAM Setup 2: //MST 2
//ARM9, 2D Graphics Engine A, OBJ-VRAM (max 256K) (OFS 0..1)
#define VRAM_A_0x06400000_ENGINE_A_BG 	(uint8)(2 | (0 << 3))	//0x064(20000*OFS.0) (OFS.1 always zero)
#define VRAM_A_0x06420000_ENGINE_A_BG 	(uint8)(2 | (1 << 3))	

#define VRAM_B_0x06400000_ENGINE_A_BG 	(uint8)(2 | (0 << 3))	//0x064(20000*OFS.0) (OFS.1 always zero)
#define VRAM_B_0x06420000_ENGINE_A_BG 	(uint8)(2 | (1 << 3))	

//both can coexist through OFS 
#define VRAM_E_0x06400000_ENGINE_A_BG 	(uint8)(2)	//static	: VRAM_E: Main Engine 0x06400000 64K MST 2

//Vram F (16K), takes in account OFS0 and OFS1 : 6400000h+(4000h*OFS.0)+(10000h*OFS.1)
#define VRAM_F_0x06400000_ENGINE_A_BG  (uint8)(2 | (( 0 )<<3))	//OFS 0
#define VRAM_F_0x06404000_ENGINE_A_BG  (uint8)(2 | (( 1 )<<3))	//OFS 1
#define VRAM_F_0x06410000_ENGINE_A_BG  (uint8)(2 | (( 2 )<<3))	//OFS 2
#define VRAM_F_0x06414000_ENGINE_A_BG  (uint8)(2 | (( 3 )<<3))	//OFS 3

//Vram G (16K),	takes in account OFS0 and OFS1 : 6400000h+(4000h*OFS.0)+(10000h*OFS.1)
#define VRAM_G_0x06400000_ENGINE_A_BG	(uint8)(2 | (( 0 )<<3))
#define VRAM_G_0x06404000_ENGINE_A_BG	(uint8)(2 | (( 1 )<<3))
#define VRAM_G_0x06410000_ENGINE_A_BG	(uint8)(2 | (( 2 )<<3))
#define VRAM_G_0x06414000_ENGINE_A_BG	(uint8)(2 | (( 3 )<<3))

//VRAM Setup 3
//2D Graphics Engine A, BG Extended Palette. This mode maps memory to VRAM and not CPU addresses
//Vram E (64K) : Slot 0-3  ;only lower 32K used (No OFS)
#define VRAM_E_SLOT0_ENGINE_A_BG_EXTENDED 	(uint8)(4 | (( 0 )<<3))	
#define VRAM_E_SLOT1_ENGINE_A_BG_EXTENDED 	(uint8)(4 | (( 1 )<<3))	
#define VRAM_E_SLOT2_ENGINE_A_BG_EXTENDED 	(uint8)(4 | (( 2 )<<3))	
#define VRAM_E_SLOT3_ENGINE_A_BG_EXTENDED 	(uint8)(4 | (( 3 )<<3))

//Vram F (16K): ofst 0..1
//Vram G (16K): ofst 0..1
#define VRAM_F_SLOT01_ENGINE_A_BG_EXTENDED 	(uint8)(4 | (( 0 )<<3))	  	//Slot 0-1 (OFS=0)
#define VRAM_F_SLOT23_ENGINE_A_BG_EXTENDED 	(uint8)(4 | (( 1 )<<3))		//Slot 2-3 (OFS=1)
#define VRAM_G_SLOT01_ENGINE_A_BG_EXTENDED 	(uint8)(4 | (( 0 )<<3))	  	//Slot 0-1 (OFS=0)
#define VRAM_G_SLOT23_ENGINE_A_BG_EXTENDED 	(uint8)(4 | (( 1 )<<3))		//Slot 2-3 (OFS=1)


//VRAM Setup 7
//Vram C (128K)
#define VRAM_C_0x06200000_ENGINE_B_BG 	(uint8)(4 | (0 << 3))	//static

//Vram H (32K)
#define VRAM_H_0x06200000_ENGINE_B_BG 	(uint8)(1)	// //VRAM H: 32K 06200000h:  map ENGINE_B BG-VRAM, static.

//Vram I (16K)
#define VRAM_I_0x06208000_ENGINE_B_BG 	(uint8)(1)	//static. 


//VRAM Setup 8
#define VRAM_D_0x06600000_ENGINE_B_OBJVRAM 	(uint8)(4)	//VRAM D: 128K 06600000h: map ENGINE_B, OBJ-VRAM. static, no OFFSet.
 

//VRAM Setup 11

//Vram C,D     128K  2    0..1  6000000h+(20000h*OFS.0)  ;OFS.1 must be zero
#define VRAM_C_0x06000000_ARM7 	(uint8)(2 | (0 << 3))	//static
#define VRAM_C_0x06020000_ARM7 	(uint8)(2 | (1 << 3))	//static

#define VRAM_D_0x06000000_ARM7 	(uint8)(2 | (0 << 3))	//static
#define VRAM_D_0x06020000_ARM7 	(uint8)(2 | (1 << 3))	//static

//todo other VRAM setups


//Screen Rotation registers
#define ORIENTATION_0 (int)(0)
#define ORIENTATION_90 (int)(1)
#define ORIENTATION_180 (int)(2)
#define ORIENTATION_270 (int)(3)

#endif

#ifdef __cplusplus
extern "C"{
#endif

//power
extern void SWAP_LCDS();
extern void SET_MAIN_TOP_LCD();
extern void SET_MAIN_BOTTOM_LCD();

//VRAM
extern void SETDISPCNT_MAIN(uint32 mode);	//videoSetMode
extern void SETDISPCNT_SUB(uint32 mode);	//videoSetModeSub

extern void VRAMBLOCK_SETBANK_A(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_B(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_C(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_D(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_E(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_F(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_G(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_H(uint8 vrambits);
extern void VRAMBLOCK_SETBANK_I(uint8 vrambits);

extern void ENABLE_BG_MAIN(int bg);
extern void ENABLE_BG_SUB(int bg);
extern void DISABLE_BG_MAIN(int number);
extern void DISABLE_BG_SUB(int number);

extern void initFBModeSubEngine0x06200000();
extern void renderFBMode3Engine(u16 * srcBuf, u16 * targetBuf, int srcWidth, int srcHeight);
extern void setOrientation(int orientation, bool mainEngine);

extern void initFBModeMainEngine0x06000000();	//set FB mode, saves old MainEngine context
extern void restoreFBModeMainEngine();	//restore from old MainEngine context discards such context

#ifdef __cplusplus
}
#endif