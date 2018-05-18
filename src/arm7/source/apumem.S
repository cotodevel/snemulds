	.TEXT
	.ARM
	.ALIGN

#include "apudef.h"

.GLOBAL BreakR11
BreakR11:
	mov r11, r11
	mov pc, lr

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
	ldr r1, =0x027ECFFC
    @ldr r1, =0x027FFFFC
    sub r2, r12, #0xF4
    strb r0, [r1, r2]
	ldmfd sp!, {r0-r2}
    bx lr

.GLOBAL MemWriteCounter
MemWriteCounter:
	stmfd sp!, {r1-r2, lr}
	bl EmulateTimers
	ldr r1, =timers
	sub r2, r12, #0xFA
	add r1, r1, r2, lsl #4
	@ Update the target
	mov r2, r0
	cmp r2, #0
	moveq r2, #0x100
	str r2, [r1]
	ldmfd sp!, {r1-r2, lr}
    bx lr

.EXTERN		ApuReadCounter

.GLOBAL		MemReadDoNothing
MemReadDoNothing:
    bx lr

.MACRO runTimer timer, shift
    tst r0, #1 << \timer
    beq timerDisabled\timer

    ldmia r1, {r3-r5}
    @ r3 - target
    @ r4 - cycles
    @ r5 - count
    add r4, r4, r2
    add r5, r5, r4, lsr #\shift
    and r4, r4, #(1 << \shift) - 1
    ldrb r6, [r7, #\timer]
    cmp r5, r3
    blt timerDone\timer
timerLoop\timer :
    sub r5, r5, r3
    add r6, r6, #1
    cmp r5, r3
    bge timerLoop\timer
timerDone\timer :
    and r6, r6, #0xf
    strb r6, [r7, #\timer]
    stmia r1, {r3-r5}
timerDisabled\timer :
    add r1, r1, #16
.ENDM

@ Returns r2 - number of cycles executed
EmulateTimers:
	stmfd sp!, {r0,r1,r3-r7}

    ldr r0, =totalCpuCycles
    ldr r1, [r0]
    add r2, r1, APU_CYCLES, asr #CYCLE_SHIFT
    sub r1, r1, r2
    str r1, [r0]

    @ r0 - timersEnabled
    @ r1 - timer base
    @ r2 - number of cycles to emulate
    @ r7 - first counter

    add r7, APU_RAMBASE, #0xFD

    ldr r1, =timers
    ldrb r0, [APU_RAMBASE, #0xF1]

    runTimer 0, 7
    runTimer 1, 7
    runTimer 2, 4

	ldmfd sp!, {r0,r1,r3-r7}
	bx lr

.GLOBAL		MemReadCounter
MemReadCounter:
    stmfd sp!, {r1,r2,lr}
    bl EmulateTimers

    @ Is the counter != 0, then no hack needed
    ldrb r0, [APU_RAMBASE, r12]
    cmp r0, #0
    bne noHack
    @ Make sure at least one timer is enabled
    ldrb r1, [APU_RAMBASE, #0xF1]
    tst r1, #7
    beq noHack
    cmp r2, #64
    bgt noHack

    @ We can do the hack
	stmfd sp!, {r0,r3,r12,lr}
	ldr r0, =ApuReadCounterHack
	mov lr, pc
	bx r0
	@ r0 is the number of cpu cycles to skip
	add APU_CYCLES, APU_CYCLES, r0, lsl #CYCLE_SHIFT
	ldmfd sp!, {r0,r3,r12,lr}
noHack:

    mov r1, #0
    strb r1, [APU_RAMBASE, r12]

    ldmfd sp!, {r1,r2, lr} 
    bx lr

.GLOBAL     MemReadApuPort
MemReadApuPort:
	stmfd sp!, {r1-r2}
	ldr r1, =0x027ECFF8
    @ldr r1, =0x027FFFF8
    sub r2, r12, #0xF4
    ldrb r0, [r1, r2]       @ Modifies the value that was read from RAM
	ldmfd sp!, {r1-r2}
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
