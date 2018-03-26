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

	.TEXT
	.ARM
	.ALIGN


	.equ PAGE_4K,	(0b01011 << 1)
	.equ PAGE_8K,	(0b01100 << 1)
	.equ PAGE_16K,	(0b01101 << 1)
	.equ PAGE_32K,	(0b01110 << 1)
	.equ PAGE_64K,	(0b00111 << 1)
	.equ PAGE_128K,	(0b10000 << 1)
	.equ PAGE_256K,	(0b10001 << 1)
	.equ PAGE_512K,	(0b10010 << 1)
	.equ PAGE_1M,	(0b10011 << 1)
	.equ PAGE_2M,	(0b10100 << 1)
	.equ PAGE_4M,	(0b10101 << 1)
	.equ PAGE_8M,	(0b10110 << 1)
	.equ PAGE_16M,	(0b10111 << 1)
	.equ PAGE_32M,	(0b11000 << 1)
	.equ PAGE_64M,	(0b11001 << 1)
	.equ PAGE_128M,	(0b11010 << 1)
	.equ PAGE_256M,	(0b11011 << 1)
	.equ PAGE_512M,	(0b11100 << 1)
	.equ PAGE_1G,	(0b11101 << 1)
	.equ PAGE_2G,	(0b11110 << 1)
	.equ PAGE_4G,	(0b11111 << 1)

	.equ	ITCM_LOAD,		(1<<19)
	.equ	ITCM_ENABLE,	(1<<18)
	.equ	DTCM_LOAD,		(1<<17)
	.equ	DTCM_ENABLE,	(1<<16)
	.equ	DISABLE_TBIT,	(1<<15)
	.equ	ROUND_ROBIN,	(1<<14)
	.equ	ALT_VECTORS,	(1<<13)
	.equ	ICACHE_ENABLE,	(1<<12)
	.equ	BIG_ENDIAN,		(1<<7)
	.equ	DCACHE_ENABLE,	(1<<2)
	.equ	PROTECT_ENABLE,	(1<<0)
	


.GLOBAL initMem	
    .align 4
initMem:
	ldr	r1, =0x00002078			@ disable TCM and protection unit
	mcr	p15, 0, r1, c1, c0

@---------------------------------------------------------------------------------
@ Protection Unit Setup added by Sasq
@---------------------------------------------------------------------------------
	@ Disable cache
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ Instruction cache
	mcr	p15, 0, r0, c7, c6, 0		@ Data cache

	@ Wait for write buffer to empty 
	mcr	p15, 0, r0, c7, c10, 4

/*	ldr	r0, =0x0b000000
	orr	r0,r0,#0x0a
	mcr	p15, 0, r0, c9, c1,0		@ DTCM base = __dtcm_start, size = 16 KB

	mov r0,#0x20
	mcr	p15, 0, r0, c9, c1,1		@ ITCM base = 0 , size = 32 MB*/

@---------------------------------------------------------------------------------
@ Setup memory regions similar to Release Version
@---------------------------------------------------------------------------------

	@-------------------------------------------------------------------------
	@ Region 0 - All memory -> not cached
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_4G | 0x00000000 | 1)	
	mcr	p15, 0, r0, c6, c0, 0

	@-------------------------------------------------------------------------
	@ Region 1 - Main Memory -> cached
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_4M | 0x02000000 | 1)	
	mcr	p15, 0, r0, c6, c1, 0

/*	@-------------------------------------------------------------------------
	@ Region 2 - iwram
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_32K | 0x037F8000 | 1)	
	mcr	p15, 0, r0, c6, c2, 0*/

	@-------------------------------------------------------------------------
	@ Region 2 - VRAM 
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_32K | 0x06898000 | 1)	
	mcr	p15, 0, r0, c6, c2, 0


	@-------------------------------------------------------------------------
	@ Region 3 - DS Accessory (GBA Cart)
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_128M | 0x08000000 | 1)	
	mcr	p15, 0, r0, c6, c3, 0

	@-------------------------------------------------------------------------
	@ Region 4 - XXX
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_2M | 0x024C0000 | 1)	
	mcr	p15, 0, r0, c6, c4, 0

	@-------------------------------------------------------------------------
	@ Region 5 - XXX
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_1M | 0x026C0000 | 1) 
	mcr	p15, 0, r0, c6, c5, 0

	@-------------------------------------------------------------------------
	@ Region 6 - System ROM
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_32K | 0xFFFF0000 | 1)	
	mcr	p15, 0, r0, c6, c6, 0

	@-------------------------------------------------------------------------
	@ Region 7 - non cacheable main ram
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_4M  | 0x02400000 | 0)	
	mcr	p15, 0, r0, c6, c7, 0

	@-------------------------------------------------------------------------
	@ Write buffer enable
	@-------------------------------------------------------------------------
	ldr	r0,=0b00000110
	mcr	p15, 0, r0, c3, c0, 0

	@-------------------------------------------------------------------------
	@ DCache & ICache enable
	@-------------------------------------------------------------------------
	ldr	r0,=0b01110110
	mcr	p15, 0, r0, c2, c0, 0
	ldr	r0,=0b00000010
	mcr	p15, 0, r0, c2, c0, 1

	@-------------------------------------------------------------------------
	@ IAccess
	@-------------------------------------------------------------------------
	ldr	r0,=0x36666333
	mcr	p15, 0, r0, c5, c0, 3

	@-------------------------------------------------------------------------
	@ DAccess
	@-------------------------------------------------------------------------
	ldr	r0,=0x36663333
	mcr     p15, 0, r0, c5, c0, 2

	@-------------------------------------------------------------------------
	@ Enable ICache, DCache, ITCM & DTCM
	@-------------------------------------------------------------------------
	mrc	p15, 0, r0, c1, c0, 0
	ldr	r1,= ITCM_ENABLE | DTCM_ENABLE | ICACHE_ENABLE | DCACHE_ENABLE | PROTECT_ENABLE
	orr	r0,r0,r1
	mcr	p15, 0, r0, c1, c0, 0
	
	mov	pc, lr	


.GLOBAL DesMuMeDebug
DesMuMeDebug:
	swi		#0x1e0000
	bx		lr
	
/*	
.global sleep
sleep:
   mcr p15,0,r0,c7,c0,4
   mov r0,r0               @ a nop instruction to prevent failures
   BX lr 	
*/

	.section    .itcm, "awx", %progbits

	.align	4

.GLOBAL IntrHandlerAsm
IntrHandlerAsm:
    mov r0, #0x4000000
    ldr r1, [r0,#0x214] @ IF

    tsts    r1, #0x02               @ h-blank interrupt
    bne     Hblank
    tsts    r1, #0x01               @ v-blank interrupt
    bne     Vblank
    bx      lr

.GLOBAL MemCpy16
MemCpy16:
	ldr	r2, [r1], #4
	str r2, [r0], #4
	ldr	r2, [r1], #4
	str r2, [r0], #4
	ldr	r2, [r1], #4
	str r2, [r0], #4
	ldr	r2, [r1], #4
	str r2, [r0], #4
	bx		lr

	