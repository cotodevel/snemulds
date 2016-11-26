/*---------------------------------------------------------------------------------

  Copyright (C) 2005
  	Michael Noland (joat)
  	Jason Rogers (dovoto)
  	Dave Murphy (WinterMute)

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.
  3. This notice may not be removed or altered from any source
     distribution.

.arch	armv5te
.cpu	arm946e-s

.text
.arm

.align  4

---------------------------------------------------------------------------------*/
#define ICACHE_SIZE	0x2000
#define DCACHE_SIZE	0x1000
#define CACHE_LINE_SIZE	32

#define PAGE_4K		(0b01011 << 1)
#define PAGE_8K		(0b01100 << 1)
#define PAGE_16K	(0b01101 << 1)
#define PAGE_32K	(0b01110 << 1)
#define PAGE_64K	(0b01111 << 1)
#define PAGE_128K	(0b10000 << 1)
#define PAGE_256K	(0b10001 << 1)
#define PAGE_512K	(0b10010 << 1)
#define PAGE_1M		(0b10011 << 1)
#define PAGE_2M		(0b10100 << 1)
#define PAGE_4M		(0b10101 << 1)
#define PAGE_8M		(0b10110 << 1)
#define PAGE_16M	(0b10111 << 1)
#define PAGE_32M	(0b11000 << 1)
#define PAGE_64M	(0b11001 << 1)
#define PAGE_128M	(0b11010 << 1)
#define PAGE_256M	(0b11011 << 1)
#define PAGE_512M	(0b11100 << 1)
#define PAGE_1G		(0b11101 << 1)
#define PAGE_2G		(0b11110 << 1)
#define PAGE_4G		(0b11111 << 1)

#define ITCM_LOAD	(1<<19)
#define ITCM_ENABLE	(1<<18)
#define DTCM_LOAD	(1<<17)
#define DTCM_ENABLE	(1<<16)
#define DISABLE_TBIT	(1<<15)
#define ROUND_ROBIN	(1<<14)
#define ALT_VECTORS	(1<<13)
#define ICACHE_ENABLE	(1<<12)
#define BIG_ENDIAN	(1<<7)
#define DCACHE_ENABLE	(1<<2)
#define PROTECT_ENABLE	(1<<0)

//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
	.global	DC_FlushAll
	.type	DC_FlushAll STT_FUNC
//---------------------------------------------------------------------------------
DC_FlushAll:
/*---------------------------------------------------------------------------------
	Clean and invalidate entire data cache
---------------------------------------------------------------------------------*/
	mov	r1, #0
outer_loop:
	mov	r0, #0
inner_loop:
	orr	r2, r1, r0					@ generate segment and line address
	mcr	p15, 0, r2, c7, c14, 2		@ clean and flush the line
	add	r0, r0, #CACHE_LINE_SIZE
	cmp	r0, #DCACHE_SIZE/4
	bne	inner_loop
	add	r1, r1, #0x40000000
	cmp	r1, #0
	bne	outer_loop
	bx	lr

//---------------------------------------------------------------------------------
	.global	DC_FlushRange
	.type	DC_FlushRange STT_FUNC
//---------------------------------------------------------------------------------
DC_FlushRange:
/*---------------------------------------------------------------------------------
	Clean and invalidate a range
---------------------------------------------------------------------------------*/
	add	r1, r1, r0
	bic	r0, r0, #(CACHE_LINE_SIZE - 1)
.flush:
	mcr	p15, 0, r0, c7, c14, 1		@ clean and flush address
	add	r0, r0, #CACHE_LINE_SIZE
	cmp	r0, r1
	blt	.flush
	bx	lr

//---------------------------------------------------------------------------------
	.global	DC_InvalidateAll
	.type	DC_InvalidateAll STT_FUNC
//---------------------------------------------------------------------------------
DC_InvalidateAll:
/*---------------------------------------------------------------------------------
	Clean and invalidate entire data cache
---------------------------------------------------------------------------------*/
	mov	r0, #0
	mcr	p15, 0, r0, c7, c6, 0
	bx	lr

//---------------------------------------------------------------------------------
	.global	DC_InvalidateRange
	.type	DC_InvalidateRange STT_FUNC
//---------------------------------------------------------------------------------
DC_InvalidateRange:
/*---------------------------------------------------------------------------------
	Invalidate a range
---------------------------------------------------------------------------------*/
	add	r1, r1, r0
	bic	r0, r0, #CACHE_LINE_SIZE - 1
.invalidate:
	mcr	p15, 0, r0, c7, c6, 1
	add	r0, r0, #CACHE_LINE_SIZE
	cmp	r0, r1
	blt	.invalidate
	bx	lr

@Coto:

//---------------------------------------------------------------------------------
	.global	getdtcmsize
	.type	getdtcmsize STT_FUNC
//---------------------------------------------------------------------------------
getdtcmsize:
	stmfd  r11!, {r1,LR} 
	mrc	p15, 0, r0, c0, c0, 2
	lsr r0, r0, #18
	lsl r0,r0,#0x1c
	lsr r0,r0,#0x1c
	ldr r1, =0x200
	lsl r0, r1, r0
	ldmia  r11!, {r1,PC}

@C9,C1,0 - Data TCM Size/Base (R/W)
//---------------------------------------------------------------------------------
	.global	getdtcmbase
	.type	getdtcmbase STT_FUNC
//---------------------------------------------------------------------------------
getdtcmbase:
	mov r0, #0
	mrc	p15, 0, r0, c9, c1, 0
	lsr r0,r0,#12
	lsl r0,r0,#12
	bx lr
	
@setdtcmbase(u32 base,u8 size)
//---------------------------------------------------------------------------------
	.global	setdtcmbase
	.type	setdtcmbase STT_FUNC
setdtcmbase:
	ldr	r0, =__dtcm_start
	orr	r0,r0,#0x0a					@possibly causes stuck
	mcr	p15, 0, r0, c9, c1,0		@ DTCM base = __dtcm_start, size = 16 KB
	bx lr

@setdtcmsz(u32 base,u8 size)
//---------------------------------------------------------------------------------
	.global	setdtcmsz
	.type	setdtcmsz STT_FUNC
//---------------------------------------------------------------------------------
setdtcmsz:
	mov r1,#0
	mrc	p15, 0, r1, c9, c1, 0
	lsr r1,r1,#12	@base
	lsl r1,r1,#12
	lsl r0,r0,#1
	orr r0,r0,r1
	mcr	p15, 0, r0, c9, c1, 0
	bx lr

@C7,C10,4 - Drain Write Buffer
//---------------------------------------------------------------------------------
	.global	drainwrite
	.type	drainwrite STT_FUNC
//---------------------------------------------------------------------------------
drainwrite:
	mov r0,#0
	mcr	p15, 0, r0, c7, c10, 4
	bx lr

@C3,C0,0 - Write Buffer(ability) enable for DTCM / ITCM are read only
//---------------------------------------------------------------------------------
	.global	writebufenable
	.type	writebufenable STT_FUNC
//---------------------------------------------------------------------------------
writebufenable:
	mcr	p15, 0, r0, c3, c0, 0
	bx lr
	
@----------------------------------------------------------------------------------
@C2,C0,0 - DATA cache for region enable
//---------------------------------------------------------------------------------
	.global	dcacheenable
	.type	dcacheenable STT_FUNC
//---------------------------------------------------------------------------------
dcacheenable:
	mcr	p15, 0, r0, c2, c0, 0
	bx lr

@----------------------------------------------------------------------------------
@ Enable ICache, DCache, ITCM & DTCM
//---------------------------------------------------------------------------------
	.global	prepcache
	.type	prepcache STT_FUNC
//---------------------------------------------------------------------------------
prepcache:
	mrc	p15, 0, r0, c1, c0, 0
	ldr	r1,= ITCM_ENABLE | DTCM_ENABLE | ICACHE_ENABLE | DCACHE_ENABLE
	orr	r0,r0,r1
	mcr	p15, 0, r0, c1, c0, 0

	@ldr	r0,=dsmasks
	@str	r9,[r0]
	bx lr

//dsmasks:
//	.word	0x003fffff, 0x02000000, 0x02c00000

@lockdown DCACHE (VA)
//---------------------------------------------------------------------------------
	.global	DC_lockdownVA
	.type	DC_lockdownVA STT_FUNC
//---------------------------------------------------------------------------------
DC_lockdownVA:
	mcr	p15, 0, r0, c9, c0, 0
	bx lr