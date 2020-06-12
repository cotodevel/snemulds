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
#endif
#ifdef ARM9
.section .itcm,"ax",%progbits
#endif
.arm
.code 32

#include "dsregs_asm.h"

.global 	IRQWait
.type   	IRQWait STT_FUNC
IRQWait:
	stmdb sp!, {r0-r4,lr}

	@r0 = IRQtowaitfor / r1,r2 = IF/IE/temp / r3 = io / r4 = temp
	mov r3,#0x04000000
	
waitirq:
	add	r4, r3, #0x210
	ldmia r4, {r1,r2}		@r1 = IE / r2 = IF
	and r2,r2,r1			@ IF = IF & IE

	@BIOS Flag
	#ifdef ARM7
	ldr	r4, =_arm7_irqcheckbits
	#endif
	#ifdef ARM9
	ldr	r4, =_arm9_irqcheckbits
	#endif
	ldr r4,[r4]
	orr r2,r2,r4			@orr swi flags
	
	@sleep if interrupt to wait for (r1) hasnt happened.
	tst r2,r0
	
	#ifdef ARM7
	@4000301h - NDS7 - Halt function (HALTCNT) - Low Power Mode Control (R/W)
	ldrne	r1, =0x04000301
	movne 	r2,#(2<<6)			@2=Halt
	strne 	r2,[r1]
	#endif
	
	#ifdef ARM9
	@NDS9	-	Halt function	CP15
	movne 	r1,#0
	mcrne 	p15,0,r1,c7,c0,4				@low power mode: waitforIrq CP15
	#endif
	
	bne waitirq		@retry until r1 happens
	
exitirq:
	ldmia sp!, {r0-r4,lr}
bx lr

@----------
.global 	IRQVBlankWait
.type   	IRQVBlankWait STT_FUNC
IRQVBlankWait:
	push {r0,lr}
	mov r0,#IRQ_VBLANK
	bl IRQWait
	pop {r0,lr}
bx lr

.pool
.end