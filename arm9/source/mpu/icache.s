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
//---------------------------------------------------------------------------------
	.arm
//---------------------------------------------------------------------------------
	.global	IC_InvalidateAll
	.type	IC_InvalidateAll STT_FUNC

//---------------------------------------------------------------------------------
IC_InvalidateAll:
/*---------------------------------------------------------------------------------
	Clean and invalidate entire data cache
---------------------------------------------------------------------------------*/
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0
	bx	lr

//---------------------------------------------------------------------------------
	.global	IC_InvalidateRange
	.type	IC_InvalidateRange STT_FUNC
//---------------------------------------------------------------------------------
IC_InvalidateRange:
/*---------------------------------------------------------------------------------
	Invalidate a range
---------------------------------------------------------------------------------*/
	add	r1, r1, r0
	bic	r0, r0, #CACHE_LINE_SIZE - 1
.invalidate:
	mcr	p15, 0, r0, c7, c5, 1
	add	r0, r0, #CACHE_LINE_SIZE
	cmp	r0, r1
	blt	.invalidate
	bx	lr

@C9,C0,1 - Instruction Cache Lockdown
@lockdown ICACHE (VA)
//---------------------------------------------------------------------------------
	.global	IC_lockdownVA
	.type	IC_lockdownVA STT_FUNC
//---------------------------------------------------------------------------------
IC_lockdownVA:
	mcr	p15, 0, r0, c9, c0, 1
	bx lr

@C9,C1,1 - Instruction TCM Size/Base (R/W)
//---------------------------------------------------------------------------------
	.global	getitcmbase
	.type	getitcmbase STT_FUNC
//---------------------------------------------------------------------------------
getitcmbase:
	mov r0, #0
	mrc	p15, 0, r0, c9, c1, 1
	lsr r0,r0,#12
	lsl r0,r0,#12
	bx lr

@setitcmbase(u32 base,u8 size)
//---------------------------------------------------------------------------------
	.global	setitcmbase
	.type	setitcmbase STT_FUNC
setitcmbase:
	mov	r0,#0x20
	mcr	p15, 0, r0, c9, c1,1		@ ITCM base = 0 , size = 32 MB
	bx lr

@C2,C0,0 - Instruction cachability region enable
//---------------------------------------------------------------------------------
	.global	icacheenable
	.type	icacheenable STT_FUNC
//---------------------------------------------------------------------------------
icacheenable:
	@stmfd  sp!, {LR} 
	mcr	p15, 0, r0, c2, c0, 1
	@ldmia sp!,{PC}
	bx lr