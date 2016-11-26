@0x10 usr
@0x11 fiq
@0x12 irq
@0x13 svc
@0x17 abt
@0x1b und
@0x1f sys

.text
.equ REG_IME, 0x04000208

.align 4
.code 32
.arm

.global cpuSetCPSR
.type   cpuSetCPSR STT_FUNC

cpuSetCPSR:
	msr cpsr, r0
	bx lr
	
.global puSetGbaIWRAM
.type   puSetGbaIWRAM STT_FUNC

puSetGbaIWRAM:
	ldr	r0,=( (0b01110 << 1) | 0x03000000 | 1)	
	mcr	p15, 0, r0, c6, c2, 0
	bx lr
	
.global cpu_GetMemPrem
.type   cpu_GetMemPrem STT_FUNC

cpu_GetMemPrem:
	mrc p15, 0, r0, c5, c0, 2
	bx lr

.global pu_Enable
.type   pu_Enable STT_FUNC

pu_Enable:
 	mrc	p15,0,r0,c1,c0,0
 	orr	r0,r0,#1
	mcr	p15,0,r0,c1,c0,0
	bx lr

.global cpu_SetCP15Cnt
.type   cpu_SetCP15Cnt STT_FUNC

cpu_SetCP15Cnt:
	mcr p15, 0, r0, c1, c0, 0
	bx lr
	
.global cpu_GetCP15Cnt
.type   cpu_GetCP15Cnt STT_FUNC

cpu_GetCP15Cnt:
	mrc p15, 0, r0, c1, c0, 0
	bx lr		

cpu_GbaSaveRegs:
.global cpuGetSPSR
.type   cpuGetSPSR STT_FUNC
cpuGetSPSR:
	
	mrs r0, spsr
	bx lr
	
.global cpuGetCPSR
.type   cpuGetCPSR STT_FUNC
cpuGetCPSR:
	
	mrs r0, cpsr
	bx lr

.global pu_SetRegion
.type   pu_SetRegion STT_FUNC
pu_SetRegion:
	
	ldr	r2, =_puSetRegion_jmp
	add	r2, r0, lsl #3
	bx		r2

_puSetRegion_jmp:
	mcr	p15, 0, r1, c6, c0, 0
	bx		lr
	mcr	p15, 0, r1, c6, c1, 0
	bx		lr
	mcr	p15, 0, r1, c6, c2, 0
	bx		lr
	mcr	p15, 0, r1, c6, c3, 0
	bx		lr
	mcr	p15, 0, r1, c6, c4, 0
	bx		lr
	mcr	p15, 0, r1, c6, c5, 0
	bx		lr
	mcr	p15, 0, r1, c6, c6, 0
	bx		lr
	mcr	p15, 0, r1, c6, c7, 0
	bx		lr

.global pu_SetDataPermissions
.type   pu_SetDataPermissions STT_FUNC

pu_SetDataPermissions:
	mcr	p15, 0, r0, c5, c0, 2
	bx		lr

@0-15  Access Permission (AP) bits for region 0-7 (Bits 0-1=AP0, 2-3=AP1, etc)
.global pu_SetCodePermissions
.type   pu_SetCodePermissions STT_FUNC

pu_SetCodePermissions:
	mcr	p15, 0, r0, c5, c0, 3
	bx		lr

.global pu_SetDataCachability
.type   pu_SetDataCachability STT_FUNC

pu_SetDataCachability:
	mcr	p15, 0, r0, c2, c0, 0 @set cachability for data/unified region
	bx		lr

.global pu_SetCodeCachability
.type   pu_SetCodeCachability STT_FUNC

pu_SetCodeCachability:
	mcr	p15, 0, r0, c2, c0, 1 @set cachability for instruction unified region
	bx		lr

.global pu_GetWriteBufferability 
.type   pu_GetWriteBufferability STT_FUNC

pu_GetWriteBufferability:
	mcr	p15, 0, r0, c3, c0, 0
	bx		lr

@----------------------------------------------------------------------------------
@ Wait For Interrupt (Halt)
@----------------------------------------------------------------------------------
@HALTCNT wait for interrupt (ARM9)
.global HALTCNT_ARM9
HALTCNT_ARM9:
	mov r0,#0
	mcr	p15, 0, r0, c7, c0, 4
	bx lr

@----------------------------------------------------------------------------------
@ Wait For Interrupt (Halt), alternately to C7,C0,4
@----------------------------------------------------------------------------------
	.global HALTCNT_ARM9OPT
	.type	HALTCNT_ARM9OPT STT_FUNC
HALTCNT_ARM9OPT:
	mov r0,#0
	mcr p15, 0, r0, c7, c8, 2
	bx lr

@---------------------------------------------------------------------------------
	.global copyMode_5
	.type   copyMode_5 STT_FUNC
@---------------------------------------------------------------------------------
copyMode_5: @r0 = src r1 =tar
@---------------------------------------------------------------------------------
	push {r4-r11,lr}
	
	mov r2 , #0x80000000
	add r2 ,r2, #0x8000

	mov r3 , #0x8

loop:							@[i=10,j=0-7]copies: 	r1[4*i]*j = r0[4*i]*j orr 0x80008000
	LDMIA r0!, {r4-r12,r14}
	orr	r4, r4, r2
	orr	r5, r5, r2
	orr	r6, r6, r2
	orr	r7, r7, r2
	orr	r8, r8, r2
	orr	r9, r9, r2
	orr	r10, r10, r2
	orr	r11, r11, r2
	orr	r12, r12, r2
	orr	r14, r14, r2
	STMIA r1!, {r4-r12,r14}
	
	subs r3, #1
	BNE loop
	
	LDMIA r0!, {r4-r12}			@[i=9,j=8]copies: 	r1[4*i]*j=r0[4*i]*j orr 0x80008000
	orr	r4, r4, r2
	orr	r5, r5, r2
	orr	r6, r6, r2
	orr	r7, r7, r2
	orr	r8, r8, r2
	orr	r9, r9, r2
	orr	r10, r10, r2
	orr	r11, r11, r2
	orr	r12, r12, r2
	STMIA r1!, {r4-r12}
	
	pop {r4-r11,pc}			@check why r12 is not restored..

	
@---------------------------------------------------------------------------------
	.global copyMode_3
	.type   copyMode_3 STT_FUNC
@---------------------------------------------------------------------------------
copyMode_3: @r0 = src r1 =tar
@---------------------------------------------------------------------------------
	push {r4-r11,lr}
	
	mov r2 , #0x80000000
	add r2 ,r2, #0x8000

	mov r3 , #13 @13 times
loop2:

	LDMIA r0!, {r4-r12,r14}
	orr	r4, r4, r2
	orr	r5, r5, r2
	orr	r6, r6, r2
	orr	r7, r7, r2
	orr	r8, r8, r2
	orr	r9, r9, r2
	orr	r10, r10, r2
	orr	r11, r11, r2
	orr	r12, r12, r2
	orr	r14, r14, r2	
	STMIA r1!, {r4-r12,r14}
	
	subs r3, #1
	BNE loop2
	
	LDMIA r0!, {r4-r6} @end with one missing
	orr	r4, r4, r2
	orr	r5, r5, r2
	orr	r6, r6, r2
	STMIA r1!, {r4-r6}
	
	pop {r4-r11,pc}



@NOTE: SWI Code is invalid since you use new vectors
@----------------------------------------------------------------------------------
@ SWI 04h IntrWait
@----------------------------------------------------------------------------------
	.global nds9intrwait
	.type	nds9intrwait STT_FUNC
nds9intrwait:
	swi 4*0x10000	@this is required for ARM code bc otherwise stucks
	bx lr
