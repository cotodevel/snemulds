	.TEXT
	.ARM
	.ALIGN

#include "apudef.h"


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
    ldr r1,=ADDRPORT_SPC_TO_SNES
	ldr r1,[r1]
    sub r2, r12, #0xF4
    strb r0, [r1, r2]
	ldmfd sp!, {r0-r2}
    bx lr

.GLOBAL MemWriteCounter
MemWriteCounter:
	stmfd sp!, {r1-r2, lr}
	strb r0, [APU_RAMBASE, r12]
	ldmfd sp!, {r1-r2, lr}
    bx lr
.EXTERN		ApuReadCounter

.GLOBAL		MemReadDoNothing
MemReadDoNothing:
    bx lr

.GLOBAL		MemReadCounter
MemReadCounter:
    stmfd sp!, {r0-r3,lr}

    stmfd sp!, {r12}

    @ r0 = the memory value
    mov r0, r12
    ldr r1, =ApuReadCounter
	mov lr, pc
    bx r1
    add APU_CYCLES, APU_CYCLES, r0, lsl #CYCLE_SHIFT

    ldmfd sp!, {r12}

    @ Reset the counter
    mov r2, #0
    strb r2, [APU_RAMBASE, r12]

	ldmfd sp!, {r0-r3,lr}
    bx lr

.GLOBAL     MemReadApuPort
MemReadApuPort:
	stmfd sp!, {r1-r3}
    ldr r1,=ADDRPORT_SNES_TO_SPC
	ldr r1,[r1]
    sub r2, r12, #0xF4
    ldrb r0, [r1, r2]       @ Modifies the value that was read from RAM
    @ldr r1, =0x027FFFE8		@SNEMUL_BLK
    ldr r1,=ADDR_SNEMUL_BLK
	ldr r1,[r1]
	
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
	ldrb r0, [r2, r1]       @ Modifies the value that was read from RAM

	ldmfd sp!, {r1-r2, r12}
	bx lr

	.POOL
	.END
