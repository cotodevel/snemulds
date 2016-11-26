/*
 Modified by Cory1492 from moonshell's plug_ndsrom_sccf.h
 for use with M3 (since they dont bother to do this stuff)
 call 
    static void ClearMemory(void);
 from ARM9 before setting up anything else
*/
 
/*
 referrence from 2006-01-13 - v2.11
 
  NDS MP
 GBAMP NDS Firmware Hack Version 2.0
 An NDS aware firmware patch for the GBA Movie Player.
 By Michael Chisholm (Chishm)
 
 Large parts are based on MultiNDS loader by Darkain.
 Filesystem code based on gbamp_cf.c by Chishm (me).
 Flashing tool written by DarkFader.
 Chunks of firmware removed with help from Dwedit.
 
 GBAMP firmware flasher written by DarkFader.
 
 This software is completely free. No warranty is provided.
 If you use it, please give due credit and email me about your
 project at chishm@hotmail.com
*/

#include "memmap.h"
 
/*-------------------------------------------------------------------------
resetMemory1_ARM9
Clears the ARM9's icache and dcache
Written by Darkain.
Modified by Chishm:
 Changed ldr to mov & add
 Added clobber list
--------------------------------------------------------------------------*/
/*
#define resetMemory1_ARM9_size 0x400
static void __attribute__ ((long_call)) resetMemory1_ARM9 (void) 
{
	REG_IME = 0;
	REG_IE = 0;
	REG_IF = ~0;
 
  __asm volatile(
    //clean and flush cache
    "mov r1, #0                   \n"
    "outer_loop:                  \n"
    " mov r0, #0                  \n"
    " inner_loop:                 \n"
    "  orr r2, r1, r0             \n"
    "  mcr p15, 0, r2, c7, c14, 2 \n"
    "  add r0, r0, #0x20          \n"
    "  cmp r0, #0x400             \n"
    " bne inner_loop              \n"
    " add r1, r1, #0x40000000     \n"
    " cmp r1, #0x0                \n"
    "bne outer_loop               \n"
 
    "mov r10, #0                  \n"
    "mcr p15, 0, r10, c7, c5, 0   \n"  //Flush ICache
    "mcr p15, 0, r10, c7, c6, 0   \n"  //Flush DCache
    "mcr p15, 0, r10, c7, c10, 4  \n"  //empty write buffer
 
	"mcr p15, 0, r10, c3, c0, 0   \n"  //disable write buffer       (def = 0)
 
    "mcr p15, 0, r10, c2, c0, 0   \n"  //disable DTCM and protection unit
 
 
	//"ldr r10, =0x0080000A         \n"	// Relocated code can't load data
	"mov r10, #0x00800000		  \n"	// Use mov instead
	"add r10, r10, #0x00A		  \n"
	"mcr p15, 0, r10, c9, c1, 0   \n"  //DTCM base  (def = 0x0080000A) ???
 
	//"ldr r10, =0x0000000C         \n"	// Relocated code can't load data
	"mov r10, #0x0000000C		  \n"	// Use mov instead
	"mcr p15, 0, r10, c9, c1, 1   \n"  //ITCM base  (def = 0x0000000C) ???
 
    "mov r10, #0x1F               \n"
	"msr cpsr, r10                \n"
	: // no outputs
	: // no inputs
	: "r0","r1","r2","r10"			// Clobbered registers
	);
 
	register int i;
 
	for (i=0; i<16*1024; i+=4) {  //first 16KB
		(*(vu32*)(i+0x00000000)) = 0x00000000;      //clear ITCM
//		(*(vu32*)(i+0x00800000)) = 0x00000000;      //clear DTCM
	}
 
	for (i=16*1024; i<32*1024; i+=4) {  //second 16KB
		(*(vu32*)(i+0x00000000)) = 0x00000000;      //clear ITCM
	}
 
	(*(vu32*)0x00803FFC) = 0;   //IRQ_HANDLER ARM9 version
	(*(vu32*)0x00803FF8) = ~0;  //VBLANK_INTR_WAIT_FLAGS ARM9 version
 
}
*/

/*-------------------------------------------------------------------------
resetMemory2_ARM9
Clears the ARM9's DMA channels and resets video memory
Written by Darkain.
Modified by Chishm:
 * Changed MultiNDS specific stuff
--------------------------------------------------------------------------*/
#define resetMemory2_ARM9_size 0x400
static void __attribute__ ((long_call)) resetMemory2_ARM9 (void) 
{
 	register int i;
 
	//clear out ARM9 DMA channels
	for (i=0; i<4; i++) {
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}
 
	VRAM_CR = 0x80808080;
	VRAM_E_CR = 0x80;
	VRAM_F_CR = 0x80;
	VRAM_G_CR = 0x80;
	VRAM_H_CR = 0x80;
	VRAM_I_CR = 0x80;
	// clear vram
	#define VRAM          ((uint16*)0x6800000)	
	zeroMemory( VRAM, 656 * 1024 );
	// clear video palette
	zeroMemory( BG_PALETTE, 2048 );//BG_PALETTE[0] = RGB15(1,1,1);
	zeroMemory( BG_PALETTE_SUB, 2048 );
	// clear video object attribution memory
	zeroMemory( OAM, 2048 );
	zeroMemory( OAM_SUB, 2048 );
	// clear video object data memory
	zeroMemory( SPRITE_GFX, 128 * 1024 );
	zeroMemory( SPRITE_GFX_SUB, 128 * 1024 );
	// clear main display registers
	zeroMemory( (void*)0x04000000, 0x6c );
	// clear sub display registers
	zeroMemory( (void*)0x04001000, 0x6c );
	
	// clear maths registers
	zeroMemory( (void*)0x04000280, 0x40 );
 
	REG_DISPSTAT = 0;
	videoSetMode(0);
	videoSetModeSub(0);
	VRAM_A_CR = 0;
	VRAM_B_CR = 0;
	VRAM_C_CR = 0;
	VRAM_D_CR = 0;
	VRAM_E_CR = 0;
	VRAM_F_CR = 0;
	VRAM_G_CR = 0;
	VRAM_H_CR = 0;
	VRAM_I_CR = 0;
	VRAM_CR   = 0x03000000;
	REG_POWERCNT  = 0x820F;
 
	//set shared ram to ARM7
	WRAM_CR = 0x03;
 
}
