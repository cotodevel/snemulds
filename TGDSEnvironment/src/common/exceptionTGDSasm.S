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

.text	@ARM9: Cant be at itcm
.arm
.code 32

#include "dsregs_asm.h"

@only in 0x00000000 vectors mode
#ifdef EXCEPTION_VECTORS_0x00000000
@todo
#endif

@only in 0xffff0000 vectors mode
#ifdef EXCEPTION_VECTORS_0xffff0000
.global DebugException
DebugException:
	
	@exception stack
	#ifdef ARM7
	ldr r12,=exceptionArmRegsShared		@uint8 * exceptionArmRegsShared;
	ldr r12, [r12]
	#endif
	
	#ifdef ARM9
	ldr r12,=exceptionArmRegs
	#endif
	
	stmia r12, {r0-r11}	@r0 -- r11
	
	@get R12,SPSR & PC from source abort
	#ifdef ARM7
	ldr r0, =(0x0380FFDC)
	#endif

	#ifdef ARM9
	ldr r0, =(0x02FFFD90)
	#endif
	
	ldr	r2,[r0,#0]	@SPSR
	str r2, [r12, #(4*17)]
	
	ldr	r2,[r0,#4]	@source LR
	str r2, [r12, #(4*14)]
	
	ldr	r2,[r0,#8]	@source PC
	str r2, [r12, #(4*15)]
	
	mrs r3, cpsr	@this CPSR
	str r3, [r12, #(4*18)]
	
	mrs r3, spsr	@source CPSR
	str r3, [r12, #(4*16)]
	
	@new stack Data Abort
	ldr	r13,=sp_ABT
	mov r7,r12		@highest register that does not change when changing PSR
	
	@grab source r13
	msr cpsr, r3
	str r13, [r7, #(4*13)]
	
	@go back
	mrs r0, spsr
	msr cpsr,r0
	
	@this PSR USR? Enter SYS
	mov r2,r0
	and r2,r2,#0x10
	cmp r2,#0x10	
	biceq r2,r0,#0x1f
	orreq r2,r2,#0x1f
	msreq cpsr,r2
	
	#ifdef ARM9
	@disable MPU
	mrc	p15,0,r0,c1,c0,0
	bic	r0,r0,#1
	mcr	p15,0,r0,c1,c0,0
	#endif
	
	push {r0-r12,lr}
	mov lr, pc
	ldr r0, =generalARMExceptionHandler
	bx r0
	pop {r0-r12,lr}
	
	bx lr


.global CustomDebugException
CustomDebugException:
	
	@exception stack
	#ifdef ARM7
	ldr r12,=exceptionArmRegsShared		@uint8 * exceptionArmRegsShared;
	ldr r12, [r12]
	#endif
	
	#ifdef ARM9
	ldr r12,=exceptionArmRegs
	#endif
	
	stmia r12, {r0-r11}	@r0 -- r11
	
	@get R12,SPSR & PC from source abort
	#ifdef ARM7
	ldr r0, =(0x0380FFDC)
	#endif

	#ifdef ARM9
	ldr r0, =(0x02FFFD90)
	#endif
	
	ldr	r2,[r0,#0]	@SPSR
	str r2, [r12, #(4*17)]
	
	ldr	r2,[r0,#4]	@source LR
	str r2, [r12, #(4*14)]
	
	ldr	r2,[r0,#8]	@source PC
	str r2, [r12, #(4*15)]
	
	mrs r3, cpsr	@this CPSR
	str r3, [r12, #(4*18)]
	
	mrs r3, spsr	@source CPSR
	str r3, [r12, #(4*16)]
	
	@new stack Data Abort
	ldr	r13,=sp_ABT
	mov r7,r12		@highest register that does not change when changing PSR
	
	@grab source r13
	msr cpsr, r3
	str r13, [r7, #(4*13)]
	
	@go back
	mrs r0, spsr
	msr cpsr,r0
	
	@this PSR USR? Enter SYS
	mov r2,r0
	and r2,r2,#0x10
	cmp r2,#0x10	
	biceq r2,r0,#0x1f
	orreq r2,r2,#0x1f
	msreq cpsr,r2
	
	#ifdef ARM9
	@disable MPU
	mrc	p15,0,r0,c1,c0,0
	bic	r0,r0,#1
	mcr	p15,0,r0,c1,c0,0
	#endif
	
	push {r0-r12,lr}
	mov lr, pc
	ldr r0, =CustomHandler
	ldr r0, [r0]
	bx r0
	pop {r0-r12,lr}
	
	bx lr


@allows the GDB process to run.
.global LeaveExceptionMode
LeaveExceptionMode:
	push {r0-r3, lr}
	
	#ifdef ARM9
	@disable MPU
	mrc	p15,0,r0,c1,c0,0
	bic	r0,r0,#1
	mcr	p15,0,r0,c1,c0,0
	#endif
	
	pop {r0-r3, lr}
	bx lr

#endif