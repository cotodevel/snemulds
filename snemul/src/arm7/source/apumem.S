	.TEXT
	.ARM
	.ALIGN

#include "apudef.h"

/*
.GLOBAL BreakR11
BreakR11:
	mov r11, r11
	mov pc, lr
*/	

.GLOBAL IntrHandlerAsm
IntrHandlerAsm:
    mov r0, #0x4000000
    ldr r1, [r0,#0x214] @ IF

    tst r1, #1 << 18 @ tst for FIFO_NOT_EMPTY
    beq notFifo

	@ Read once from the FIFO
    mov r1, #0x4100000
    ldr r0, [r1]

    stmfd sp!,{lr}
    bl HandleFifo
    
finished:  
    mov r0, #0x4000000

    @ Clear IF
    mov r1, #1 << 18
    str r1, [r0,#0x214]

    ldmfd sp!,{lr}
    bx lr

notFifo:
    ldr r0, =InterruptHandler
    bx r0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Special memory write functions
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ r0 is the value written
@ r2 is temp
@ r12 is the location in APU ram written (MUST NOT BE MODIFIED)
@ r1, r3-lr must not be modified
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.EXTERN		DSP_MEM
.EXTERN		DspWriteByte
.EXTERN		ApuWriteControlByte
.EXTERN     ApuWriteUpperByte

.GLOBAL		MemWriteDoNothing
MemWriteDoNothing:
    bx lr

.GLOBAL		MemWriteApuControl
MemWriteApuControl:
	stmfd sp!, {r0-r3,r12,lr}
	@ r0 = the value written
	and r0, r0, #0xff
 	ldr r1, =ApuWriteControlByte
	mov lr, pc
	bx r1
	ldmfd sp!, {r0-r3,r12,lr}
	bx lr

@ r0 is the value written
.GLOBAL		MemWriteDspData
MemWriteDspData:
	stmfd sp!, {r0-r3,r12,lr}
	@Write to dsp
    sub r12,r12,#1
	ldrb r1, [APU_RAMBASE, r12]
	@ r0 = The value written
	@ r1 = The dsp address
	and r0, r0, #0xff
	ldr r2, =DspWriteByte
	mov lr, pc
	bx r2
	ldmfd sp!, {r0-r3,r12,lr}
	bx lr

.GLOBAL     MemWriteUpperByte
MemWriteUpperByte:
	stmfd sp!, {r0-r3,r12,lr}
	@ r0 = the value written
	@ r1 = the address
	and r0, r0, #0xff
	mov r1, r12
 	ldr r2, =ApuWriteUpperByte
	mov lr, pc
	bx r2
	ldmfd sp!, {r0-r3,r12,lr}
	bx lr

.GLOBAL     MemWriteApuPort
MemWriteApuPort:
	stmfd sp!, {r0-r2}	
	
    ldr r1, =0x027FFFFC
    sub r2, r12, #0xF4
    strb r0, [r1, r2]
    
    ldr	r0, =0x27E0000
    str	APU_PC, [r0]    
	ldmfd sp!, {r0-r2}
    bx lr

.GLOBAL MemWriteCounter
MemWriteCounter:
	stmfd sp!, {r1-r2, lr}
	
	strb r0, [APU_RAMBASE, r12]
	
/*	@ archeide: update T0 T1 T2 from CPU's APU2
	ldr r1, =0x27ED000
	sub r2, r12, #0xFA
	add r1, r1, r2, lsl #2
	@ Update the target
	mov r2, r0
	cmp r2, #0
	moveq r2, #0x100
	str r2, [r1]*/
	ldmfd sp!, {r1-r2, lr}
    bx lr

.EXTERN		ApuReadCounter

.GLOBAL		MemReadDoNothing
MemReadDoNothing:
    bx lr

/*.GLOBAL		MemReadCounter
MemReadCounter:
	stmfd sp!, {r1-r2} @ is it useful ?

	@ archeide: get APU2->CNT0
	ldr		r1, =0x27ED018	
	sub		r2, r12, #0xFD
	add 	r1, r1, r2, lsl #4	

	ldr		r0, [r1]
	
	@ APU2->CNT0 = 0
	mov		r2, #0 
	str		r2, [r1]
	
	ldmfd sp!, {r1-r2}
	bx lr
*/

.GLOBAL		MemReadCounter
MemReadCounter:
	stmfd sp!, {r1} @ is it useful ?

.ifeq CheckIntrLoop-1
	@ Set Loop detection address
    ldr	r1, =APU_PC_save
    ldr	r1, [r1]
    ldr	r0, =APU_WaitAddress
    str r1, [r0]  
.endif

	@ archeide: get APU2->CNT0
	ldrb	r0, [APU_RAMBASE, r12]	
	
	@ APU2->CNT0 = 0
	mov		r1, #0 
	strb	r1, [APU_RAMBASE, r12]
	
	ldmfd sp!, {r1}
	bx lr

/*
.GLOBAL		MemReadCounterFE
MemReadCounterFE:
	stmfd sp!, {r1-r2} @ is it useful ?

	@ archeide: get APU2->CNT1
	ldr		r1, =0x27ED01C
	ldrb	r0, [r1]
	
	@ APU2->CNT1 = 0
	mov		r2, #0 
	str		r2, [r1]
	
	ldmfd sp!, {r1-r2}
	bx lr

.GLOBAL		MemReadCounterFF
MemReadCounterFF:
	stmfd sp!, {r1-r2} @ is it useful ?

	@ archeide: get APU2->CNT2
	ldr		r1, =0x27ED020
	ldrb	r0, [r1]
	
	@ APU2->CNT2 = 0
	mov		r2, #0 
	str		r2, [r1]
	
	ldmfd sp!, {r1-r2}
	bx lr
*/


.GLOBAL     MemReadApuPort
MemReadApuPort:
	stmfd sp!, {r1-r3}
	
@Doesn't read APU port while CPU is not running
/*0:
	ldr	r1, =0x027FFFE8
	ldr	r1, [r1]
	cmp	r1,	#1
	beq	0b*/
	

    ldr	r0, =0x27E0000    
    str APU_PC, [r0]

    sub r2, r12, #0xF4
        
    ldr r1, =0x027FFFF8    
    ldrb r0, [r1, r2]       @ Modifies the value that was read from RAM
    
    ldr r1, =0x027FFFE8
    mov	r3, #0	
    strb r3, [r1, r2]		@ unblock
    
    
	ldmfd sp!, {r1-r3}
    bx lr

.GLOBAL		MemReadDspData
MemReadDspData:
	stmfd sp!, {r1-r2, r12}
    sub r12,r12,#1

    @ Get the DSP address into r1
	ldrb r1, [APU_RAMBASE, r12]

	ldr	r2, =DSP_MEM
	and r1, r1, #0x7f
	ldrb r0, [r2, r1]       @ Modifies the value that was read from RAM

	ldmfd sp!, {r1-r2, r12}
	bx lr

	.POOL
	.END
