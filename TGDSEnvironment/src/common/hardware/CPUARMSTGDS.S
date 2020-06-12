@
@			Copyright (C) 2017  Coto
@This program is free software; you can redistribute it and/or modify
@it under the terms of the GNU General Public License as published by
@the Free Software Foundation; either version 2 of the License, or
@(at your option) any later version.

@This program is distributed in the hope that it will be useful, but
@WITHOUT ANY WARRANTY; without even the implied warranty of
@MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
@General Public License for more details.

@You should have received a copy of the GNU General Public License
@along with this program; if not, write to the Free Software
@Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
@USA
@


#ifdef ARM7
.text
.arm
.code 32

@----------

@allows to change to VRAM stacks on the fly.
.global 	useARM7VRAMStacks
.type   	useARM7VRAMStacks STT_FUNC
useARM7VRAMStacks:
	mrs	r4,cpsr
	ldr r5,=interStack
	stmia	r5,{r0-r4,r6-r12,lr}
	
	@use ARM7 VRAM stack malloc'd !:
	push {lr}
	bl allocVRAMStacks	@r0: ptr to new full ascending stack (vram), requires to turn into full descending.
	pop {lr}
	
	add r11,r0,#(8*1024)	@r11: stacks full descending
	
	mov	r0, #0x13		@ svc dtcm stacks, irq enable
	msr	cpsr, r0
	add sp,r11,#-(0x200 * 1)
	
	mov	r0, #0x12		@ irq
	msr	cpsr, r0
	add sp,r11,#-(0x200 * 2)
	
	mov 	r3 , #0x11	@FIRQ mode, irq enable
	msr     CPSR, r3
	add sp,r11,#-(0x200 * 3)
	
	@ The debug handler is called: 
	@on undefined instruction exceptions, 
	@on data/prefetch aborts (caused by the protection unit), 
	@on FIQ (possibly caused by hardware debuggers). 
	@It is also called by accidental software-jumps to the reset vector, and by unused SWI numbers within range 0..1Fh.
	
	mov 	r3 , #0x17	@ dataabt dtcm stacks, irq enable
	msr     CPSR,r3 
	add sp,r11,#-(0x200 * 4)
	
	mov 	r3 , #0x1b	@ undef dtcm stacks, irq enable
	msr     CPSR, r3
	add sp,r11,#-(0x200 * 5)
	
	mov	r0, #0x1F		@ usr/sys dtcm stacks, irq enable (ignore all exceptions usermode must be excluded here)
	msr	cpsr, r0
	add sp,r11,#-(0x200 * 6)
	
	ldr 	r5,=interStack
	ldmia	r5,{r0-r4,r6-r12,lr}
	msr	cpsr, r4
	bx lr

.pool

.global interStack
interStack:
	.word   0x00000000      @r0     a1
	.word   0x00000000      @r1             a2
	.word   0x00000000      @r2             a3
	.word   0x00000000      @r3             a4
	.word   0x00000000      @r4             s1
	.word   0x00000000      @r5             s2
	.word   0x00000000      @r6             s3
	.word   0x00000000      @r7             s4
	.word   0x00000000      @r8             s5
	.word   0x00000000      @r9             s6
	.word   0x00000000      @r10    sl
	.word   0x00000000      @r11    ip
	.word   0x00000000      @r12    fp
	.word   0x00000000      @r13    sp
	.word   0x00000000      @r14    lr
	.word   0x00000000      @r15    pc
.pool

.end

#endif