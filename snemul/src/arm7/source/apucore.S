	.TEXT
	.ARM
	.ALIGN

#include "apudef.h"

.MACRO notDone opc
@    mov r0, #\opc
@    bl unimpl
.ENDM

//.equ        CheckIntrLoop,                0

/*
ApuTrace:
    ldr		r1, =g_ApuTrace
    ldr		r12, [r1]

    bic		r12, r12, #0xC000
   
    str		APU_PC,		[r12], #4
    str		APU_REG_A,	[r12], #4
@    str		APU_CYCLES, [r12], #4
    ldr		r0, =0x027FFFF9
    ldrb	r2, [r0]
    str		r2,			[r12], #4
    ldr		r0, =g_ApuCnt
    ldr		r2, [r0]
    add		r2, r2, #1
    str		r2,			[r12], #4
    str		r2, [r0]

    str		r12, [r1]

    ldrmib r0, [APU_PC], #1
    ldrmi pc, [APU_OPTABLE, r0, lsl #2]
    b doneDispatch
*/

.ifeq CheckIntrLoop-1
CheckBranch:
	ldr	r0, =APU_WaitAddress
	ldr	r0, [r0]
	sub	r0, r0, #1 @ PC is saved after opcode byte
	
	cmp	r0, APU_PC @ If PC = WaitAddres We have detected an timer loop
	
	ldreq r1, =0x27E0004
	ldreq r0, [r1]
	addeq r0, r0, #1
	streq r0, [r1]
	andeq  APU_CYCLES, APU_CYCLES, #7 @ Clear cycles => Skip line
	bx lr
.endif

.MACRO fetch cycles
    adds APU_CYCLES, APU_CYCLES, #\cycles << CYCLE_SHIFT

    @ Trace
    @b ApuTrace    
    
    ldrmib r0, [APU_PC], #1
    ldrmi pc, [APU_OPTABLE, r0, lsl #2]
    b doneDispatch
.ENDM

.MACRO fetchSetC cycles
    adcs APU_CYCLES, APU_CYCLES, #\cycles << CYCLE_SHIFT
    
    @ Trace
    @b ApuTrace    
        
    ldrmib r0, [APU_PC], #1
    ldrmi pc, [APU_OPTABLE, r0, lsl #2]
    b doneDispatch
.ENDM

.ALIGN

.ALIGN
.GLOBAL ApuExecute
@ r0 - Number of cycles to execute
ApuExecute:
	stmfd sp!, {r4-r11,lr}


	ldr r1, =APU_STATE
	ldmia r1, {r3-r11}

/*
@DEBUG
    stmfd sp!, {r0}
    mov r2, #0x2800000
    subs r2, r2, #56
    sub r0, r8, r6
    str r0, [r2], #4 @ pc
    str r9, [r2], #4 @ cycles to execute
    ldrb r0, [APU_PC]
    str r0, [r2], #4 @ opcode
    ldr r1, =DSP_MEM
    ldrb r0, [r1, #0x4C]
    str r0, [r2], #4 @ key on
    ldr r1, =DSP_MEM
    ldrb r0, [r1, #0x08]
    str r0, [r2], #4 @ envx chan 0
    ldmfd sp!, {r0}
afterdebug:
@DEBUG
*/

	subs APU_CYCLES, APU_CYCLES, r0, lsl #CYCLE_SHIFT

/*    ldr		r1, =g_ApuTrace
    ldr		r12, [r1]
    ldr		r0, =0x027E3FFF
    and		r12, r0
    str		APU_PC,		[r12], #4
    str		APU_PC,		[r12], #4
    str		APU_PC,		[r12], #4
    str		APU_PC,		[r12], #4
    str		r12, [r1]*/

    @ Load the next opcode
    ldrmib r0, [APU_PC], #1
    
    
    ldrmi pc, [APU_OPTABLE, r0, lsl #2]

doneDispatch:
	ldr r1, =APU_STATE
    stmia r1, {r3-r11}
    ldmfd sp!, {r4-r11, lr}
    bx lr

.POOL


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Memory access macros
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ In: r12 - address to read
@ Out: r0 - byte read (in bits 0..7), r1 - unchanged, r2 - trashed, r12 - unchanged, flags - trashed
.MACRO readMem8
    ldrb r0, [APU_RAMBASE, r12]
    cmps r12, #0xff
    blle zeroPageMemRead8
.ENDM

.MACRO readMem8NoCheck
    ldrb r0, [APU_RAMBASE, r12]
    cmps r12, #0xff
    blle zeroPageMemRead8
.ENDM

@ In: r0 - value to write (in bits 0.. 7), r12 - address to read
@ Out: r0 - ???, r1 - unchanged, r2 - trashed, r12 - unchanged, flags - trashed
.MACRO writeMem8
    mov r2, #0x40 << 16
    add r2, r2, r12, lsl #16
    cmps r2, #0x140 << 16
    bllo zeroAndHighPageMemWrite8
    strb r0, [APU_RAMBASE, r12]
.ENDM

@ Does not work for general purpose writes
@ In: r0 - value to write (in bits 0.. 7), r12 - address to read
@ Out: r0 - ???, r1 - unchanged, r2 - trashed, r12 - unchanged, flags - trashed
.MACRO writeMem8ZeroPage
    cmps r12, #0xff
    blle zeroPageMemWrite8
    strb r0, [APU_RAMBASE, r12]
.ENDM

zeroPageMemRead8:
    add r2, APU_OPTABLE, #0x400
    ldr pc, [r2, r12, lsl #2]

zeroPageMemWrite8:
    add r2, APU_OPTABLE, #0x900         @ Skip read mem table and first 0x40 (*4) mem lookup tables
    ldr pc, [r2, r12, lsl #2]

zeroAndHighPageMemWrite8:
    add r2, APU_OPTABLE, r2, lsr #14
    ldr pc, [r2, #0x800]

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Address loading macros
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.MACRO loadDp
    ldrb r12, [APU_PC], #1
    add r12, r12, APU_DP
.ENDM

.MACRO loadDpOfs reg
    ldrb r12, [APU_PC], #1
    add r12, r12, \reg, lsr #24
    and r12, r12, #0xff
    add r12, r12, APU_DP
.ENDM

.MACRO loadImm
    ldrb r12, [APU_PC], #1
    ldrb r0, [APU_PC], #1
    orr r12, r12, r0, lsl #8
.ENDM

.MACRO loadImmOfs reg
    loadImm
    add r12, r12, \reg, lsr #24
    bic r12, r12, #0xFF0000
.ENDM

.MACRO loadDpX
    loadDpOfs APU_REG_X
    readMem8
    mov r1, r0
    add r12, r12, #1
    readMem8
    orr r12, r1, r0, lsl #8
.ENDM

.MACRO loadDpY
    loadDp
    readMem8
    mov r1, r0
    add r12, r12, #1
    readMem8
    orr r12, r1, r0, lsl #8
    add r12, r12, APU_REG_Y, lsr #24
    bic r12, r12, #0xff0000
.ENDM    

.MACRO loadXY
    mov r12, APU_REG_Y, lsr #24
    add r12, r12, APU_DP
    readMem8
    mov r1, r0, lsl #24
    add r12, APU_DP, APU_REG_X, lsr #24
    readMem8NoCheck
.ENDM

.MACRO loadDpDp
    loadDp
    readMem8
    mov r1, r0, lsl #24
    loadDp
    readMem8NoCheck
.ENDM

.MACRO loadDpImm
    ldrb r1, [APU_PC], #1
    mov r1, r1, lsl #24
    loadDp
    readMem8
.ENDM

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Convenience macros
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.MACRO loadMovR reg
    readMem8
    mov APU_NZ, r0
    mov \reg, APU_NZ, lsl #24
.ENDM

.MACRO storeR reg
    mov r0, \reg, lsr #24
    writeMem8
.ENDM

.MACRO adcFromR0
    @ Get C bit into C flag
    movs r1, APU_CYCLES, lsr #1
    subcs r0, r0, #0x100
    adcs APU_REG_A, APU_REG_A, r0, ror #8
    mov APU_NZ, APU_REG_A, lsr #24
    bic APU_CYCLES, APU_CYCLES, #(CYCLES_C_FLAG | CYCLES_V_FLAG)
    orrvs APU_CYCLES, APU_CYCLES, #CYCLES_V_FLAG
    @ Will be set in fetchSetC
.ENDM

.MACRO adcToMem
    movs r2, APU_CYCLES, lsr #1
    @ Result of operation into r1
    subcs r0, r0, #0x100
    adcs r1, r1, r0, ror #8
    mov APU_NZ, r1, lsr #24
    bic APU_CYCLES, APU_CYCLES, #(CYCLES_C_FLAG | CYCLES_V_FLAG)
    orrvs APU_CYCLES, APU_CYCLES, #CYCLES_V_FLAG
    orrcs APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG   
    @ Write the result back
    mov r0, APU_NZ
    writeMem8ZeroPage
.ENDM

.MACRO sbcFromR0
    @ Get C bit into C flag
    movs r1, APU_CYCLES, lsr #1
    sbcs APU_REG_A, APU_REG_A, r0, lsl #24
    mov APU_NZ, APU_REG_A, lsr #24
    bic APU_CYCLES, APU_CYCLES, #(CYCLES_C_FLAG | CYCLES_V_FLAG)
    orrvs APU_CYCLES, APU_CYCLES, #CYCLES_V_FLAG
.ENDM

.MACRO sbcToMem
    movs r2, APU_CYCLES, lsr #1
    @ Result of operation into r1
    @ Is actually r0 - r1, but r0 needs to be shifted, so we use rsb
    rscs r1, r1, r0, lsl #24
    mov APU_NZ, r1, lsr #24
    bic APU_CYCLES, APU_CYCLES, #(CYCLES_C_FLAG | CYCLES_V_FLAG)
    orrvs APU_CYCLES, APU_CYCLES, #CYCLES_V_FLAG
    orrcs APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG   
    @ Write the result back
    mov r0, APU_NZ
    writeMem8ZeroPage
.ENDM

.MACRO cmpFromR0 reg
    subs APU_NZ, \reg, r0, lsl #24
    mov APU_NZ, APU_NZ, lsr #24
    bic APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
.ENDM

.MACRO cmpToMem
    @ Is actually r0 - r1, but r0 needs to be shifted, so we use rsb
    rsbs APU_NZ, r1, r0, lsl #24
    mov APU_NZ, APU_NZ, lsr #24
    bic APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
.ENDM

.MACRO andFromR0
    and APU_NZ, r0, APU_REG_A, lsr #24
    mov APU_REG_A, APU_NZ, lsl #24
.ENDM

.MACRO andToMem
    and APU_NZ, r0, r1, lsr #24
    mov r0, APU_NZ
    writeMem8ZeroPage
.ENDM

.MACRO orFromR0
    orr APU_NZ, r0, APU_REG_A, lsr #24
    mov APU_REG_A, APU_NZ, lsl #24
.ENDM

.MACRO orToMem
    orr APU_NZ, r0, r1, lsr #24
    mov r0, APU_NZ
    writeMem8ZeroPage
.ENDM

.MACRO eorFromR0
    eor APU_NZ, r0, APU_REG_A, lsr #24
    mov APU_REG_A, APU_NZ, lsl #24
.ENDM

.MACRO eorToMem
    eor APU_NZ, r0, r1, lsr #24
    mov r0, APU_NZ
    writeMem8ZeroPage
.ENDM

.MACRO incToMem
    readMem8NoCheck
    add r0, r0, #1
    and APU_NZ, r0, #0xff
    writeMem8
.ENDM

.MACRO decToMem
    readMem8NoCheck
    sub r0, r0, #1
    mov APU_NZ, r0
    writeMem8 
.ENDM

.MACRO aslToMem
    readMem8NoCheck
    movs r0, r0, lsl #25
    mov APU_NZ, r0, lsr #24
    mov r0, APU_NZ
    bic APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
    orrcs APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
    writeMem8
.ENDM

.MACRO lsrToMem
    readMem8NoCheck
    movs APU_NZ, r0, lsr #1
    mov r0, APU_NZ
    bic APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
    orrcs APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
    writeMem8
.ENDM

.MACRO rolToMem
    readMem8NoCheck
    movs APU_NZ, APU_CYCLES, lsr #1 @ Get C flag
    adc r0, r0, r0
    and APU_NZ, r0, #0xff
    bic APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
    orr APU_CYCLES, APU_CYCLES, r0, lsr #8
    writeMem8
.ENDM

.MACRO rorToMem
    readMem8NoCheck
    movs APU_CYCLES, APU_CYCLES, lsr #1 @ Get C flag
    orrcs r0, r0, #(1 << 8)
    movs r0, r0, lsr #1
    mov APU_NZ, r0
    adc APU_CYCLES, APU_CYCLES, APU_CYCLES
    writeMem8
.ENDM

.MACRO branchDpBitSet bit
    loadDp
    readMem8
    tsts r0, #1 << \bit

    ldrsb r0, [APU_PC], #1
    addne APU_PC, APU_PC, r0
    addne APU_CYCLES, APU_CYCLES, #2 << CYCLE_SHIFT
    
    fetch 5
.ENDM

.MACRO branchDpBitClear bit
    loadDp
    readMem8
    tsts r0, #1 << \bit

    ldrsb r0, [APU_PC], #1
    addeq APU_PC, APU_PC, r0
    addeq APU_CYCLES, APU_CYCLES, #2 << CYCLE_SHIFT
    
    fetch 5
.ENDM

.MACRO tcall page
    ldr r12, =APU_EXTRA_MEM
    ldrb r0, [r12, #(15 - \page) * 2]
    ldrb r1, [r12, #((15 - \page) * 2) + 1]
    orr r12, r0, r1, lsl #8
    
    sub r1, APU_PC, APU_RAMBASE
    add APU_PC, APU_RAMBASE, r12
    push16

    fetch 8
.ENDM

.MACRO setDpBit bit
    loadDp
    readMem8NoCheck
    orr r0, r0, #1 << \bit
    writeMem8
    fetch 4
.ENDM

.MACRO clearDpBit bit
    loadDp
    readMem8NoCheck
    bic r0, r0, #1 << \bit
    writeMem8
    fetch 4
.ENDM

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Stack & Misc. Macros
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

@ In: r0 - Byte to be pushed
.MACRO push8
    ldr r12, APU_SP
    sub r1, r12, #1
    orr r1, r1, #0x100
    str r1, APU_SP
    writeMem8ZeroPage
.ENDM

@ Out: r0 - Byte popped
.MACRO pop8
    ldr r12, APU_SP
    add r12, r12, #1
    bic r12, r12, #0xFE00
    str r12, APU_SP
    readMem8
.ENDM

@ In: r1 - Halfword to be pushed
.MACRO push16
    ldr r12, APU_SP
    mov r0, r1, lsr #8
    writeMem8ZeroPage
    sub r12, r12, #1
    orr r12, r12, #0x100
    mov r0, r1
    writeMem8ZeroPage
    sub r12, r12, #1
    orr r12, r12, #0x100
    str r12, APU_SP
.ENDM

@ Out: r1 - Halfword popped
.MACRO pop16
    ldr r12, APU_SP
    add r12, r12, #1
    bic r12, r12, #0xFE00
    readMem8
    mov r1, r0
    add r12, r12, #1
    bic r12, r12, #0xFE00
    readMem8
    orr r1, r1, r0, lsl #8
    str r12, APU_SP
.ENDM

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ CPU Opcodes
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

////////////////////////////////////////////////////////////////////////////
// 8 bit load commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    A, #inm    E8    2     2     A <- inm                  N......Z
opE8:
    ldrb APU_NZ, [APU_PC], #1
    mov APU_REG_A, APU_NZ, lsl #24
    fetch 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    A, (X)     E6    1     3     A <- (X)                  N......Z
opE6:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    add r12, APU_DP, APU_REG_X, lsr #24
    loadMovR APU_REG_A
    fetch 3
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    A, (X)+    BF    1     4     A <- (X) with auto inc    N......Z
opBF:
    add r12, APU_DP, APU_REG_X, lsr #24
    add APU_REG_X, APU_REG_X, #(1 << 24)
    loadMovR APU_REG_A
    fetch 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    A, dp      E4    2     3     A <- (dp)                 N......Z
opE4:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	 
    loadDp
    loadMovR APU_REG_A
    fetch 3
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    A, dp+X    F4    2     4     A <- (dp+X)               N......Z
opF4:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadDpOfs APU_REG_X
    loadMovR APU_REG_A
    fetch 4

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    A, labs    E5    3     4     A <- (abs)                N......Z
opE5:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadImm
    loadMovR APU_REG_A  
    fetch 4

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    A, labs+X  F5    3     5     A <- (abs+X)              N......Z
opF5:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadImmOfs APU_REG_X
    loadMovR APU_REG_A
    fetch 5

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    A, labs+Y  F6    3     5     A <- (abs+Y)              N......Z
opF6:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadImmOfs APU_REG_Y
    loadMovR APU_REG_A
    fetch 5

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    A, (dp+X)  E7    2     6     A <- ((dp+X+1)(dp+X))     N......Z
opE7:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadDpX
    loadMovR APU_REG_A  
    fetch 6
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    A, (dp)+Y  F7    2     6     A <- ((dp+1)(dp)+Y)       N......Z
opF7:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadDpY
    loadMovR APU_REG_A
    fetch 6
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    X, #inm    CD    2     2     X <- inm                  N......Z
opCD:
    ldrb APU_NZ, [APU_PC], #1
    mov APU_REG_X, APU_NZ, lsl #24
    fetch 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    X, dp      F8    2     3     X <- (dp)                 N......Z
opF8:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadDp
    loadMovR APU_REG_X
    fetch 3    

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    X, dp+Y    F9    2     4     X <- (dp+Y)               N......Z
opF9:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadDpOfs APU_REG_Y
    loadMovR APU_REG_X  
    fetch 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    X, labs    E9    3     4     X <- (abs)                N......Z
opE9:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadImm
    loadMovR APU_REG_X  
    fetch 4

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    Y, #inm    8D    2     2     Y <- inm                  N......Z
op8D:
    ldrb APU_NZ, [APU_PC], #1
    mov APU_REG_Y, APU_NZ, lsl #24
    fetch 2

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    Y, dp      EB    2     3     Y <- (dp)                 N......Z
opEB:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadDp
    loadMovR APU_REG_Y
    fetch 3

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    Y, dp+X    FB    2     4     Y <- (dp+X)               N......Z
opFB:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadDpOfs APU_REG_X
    loadMovR APU_REG_Y
    fetch 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    Y, labs    EC    3     4     Y <- (abs)                N......Z
opEC:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadImm
    loadMovR APU_REG_Y   
    fetch 4
    
////////////////////////////////////////////////////////////////////////////
// 8 bit store commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    (X),A      C6    1     4     A -> (X)                  ........
opC6:
    add r12, APU_DP, APU_REG_X, lsr #24
    storeR APU_REG_A
    fetch 4

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    (X)+,A     AF    1     4     A -> (X) with auto inc    ........
opAF:
    add r12, APU_DP, APU_REG_X, lsr #24
    add APU_REG_X, APU_REG_X, #(1 << 24)
    storeR APU_REG_A
    fetch 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    dp,A       C4    2     4     A -> (dp)                 ........
opC4:
    loadDp
    storeR APU_REG_A
    fetch 4

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    dp+X,A     D4    2     5     A -> (dp+X)               ........
opD4:
    loadDpOfs APU_REG_X
    storeR APU_REG_A
    fetch 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    labs,A     C5    3     5     A -> (abs)                ........
opC5:
    loadImm
    storeR APU_REG_A
    fetch 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    labs+X,A   D5    3     6     A -> (abs+X)              ........
opD5:
    loadImmOfs APU_REG_X
    storeR APU_REG_A
    fetch 6

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    labs+Y,A   D6    3     6     A -> (abs+Y)              ........
opD6:
    loadImmOfs APU_REG_Y
    storeR APU_REG_A
    fetch 6
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    (dp+X),A   C7    2     7     A -> ((dp+X+1)(dp+X))     ........
opC7:
    loadDpX
    storeR APU_REG_A
    fetch 7

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    (dp)+Y,A   D7    2     7     A -> ((dp+1)(dp)+Y)       ........
opD7:
    loadDpY
    storeR APU_REG_A
    fetch 7

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    dp,X       D8    2     4     X -> (dp)                 ........
opD8:
    loadDp
    storeR APU_REG_X
    fetch 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    dp+Y,X     D9    2     5     X -> (dp+Y)               ........
opD9:
    loadDpOfs APU_REG_Y
    storeR APU_REG_X
    fetch 5

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    labs,X     C9    3     5     X -> (abs)                ........
opC9:
    loadImm
    storeR APU_REG_X
    fetch 5

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    dp,Y       CB    2     4     Y -> (dp)                 ........
opCB:
    loadDp
    storeR APU_REG_Y
    fetch 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    dp+X,Y     DB    2     5     Y -> (dp+X)               ........
opDB:
    loadDpOfs APU_REG_X
    storeR APU_REG_Y
    fetch 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    labs,Y     CC    3     5     Y -> (abs)                ........
opCC:
    loadImm
    storeR APU_REG_Y
    fetch 5

////////////////////////////////////////////////////////////////////////////
// 8 bit misc. commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    A, X        7D    1     2     A <- X                   N......Z
op7D:
    mov APU_REG_A, APU_REG_X
    mov APU_NZ, APU_REG_X, lsr #24
    fetch 2

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    A, Y        DD    1     2     A <- Y                   N......Z
opDD:
    mov APU_REG_A, APU_REG_Y
    mov APU_NZ, APU_REG_Y, lsr #24
    fetch 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    X, A        5D    1     2     X <- A                   N......Z
op5D:
    mov APU_REG_X, APU_REG_A
    mov APU_NZ, APU_REG_A, lsr #24
    fetch 2

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    Y, A        FD    1     2     Y <- A                   N......Z
opFD:
    mov APU_REG_Y, APU_REG_A
    mov APU_NZ, APU_REG_A, lsr #24
    fetch 2
  
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    dp(d),dp(s) FA    3     5    (dp(d)) <- (dp(s))        ........
opFA:
    loadDp
    readMem8
    loadDp
    writeMem8ZeroPage
    fetch 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    dp,#inm     8F    3     5    (dp) <- inm               ........
op8F:
    ldrb r0, [APU_PC], #1
    loadDp
    writeMem8ZeroPage
    fetch 5

.ifeq CheckIntrLoop-1
.GLOBAL APU_PC_save
APU_PC_save:
.word 0  
.endif

////////////////////////////////////////////////////////////////////////////
// Add with carry commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   ADC    A,#inm      88    2     2     A <- A+inm+C            NV..H..ZC
op88:
    ldrb r0, [APU_PC], #1
    adcFromR0
    fetchSetC 2

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   ADC    A,(X)       86    1     3     A <- A+(X)+C            NV..H..ZC
op86:
    add r12, APU_DP, APU_REG_X, lsr #24
    readMem8
    adcFromR0
    fetchSetC 3
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   ADC    A,dp        84    2     3     A <- A+(dp)+C           NV..H..ZC
op84:
    loadDp
    readMem8
    adcFromR0
    fetchSetC 3

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   ADC    A,dp+X      94    2     4     A <- A+(dp+X)+C         NV..H..ZC
op94:
    loadDpOfs APU_REG_X
    readMem8
    adcFromR0
    fetchSetC 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   ADC    A,labs      85    3     4     A <- A+(abs)+C          NV..H..ZC
op85:
    loadImm
    readMem8
    adcFromR0
    fetchSetC 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   ADC    A,labs+X    95    3     5     A <- A+(abs+X)+C        NV..H..ZC
op95:
    loadImmOfs APU_REG_X
    readMem8
    adcFromR0
    fetchSetC 5

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   ADC    A,labs+Y    96    3     5     A <- A+(abs+Y)+C        NV..H..ZC
op96:
    loadImmOfs APU_REG_Y
    readMem8
    adcFromR0
    fetchSetC 5

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   ADC    A,(dp+X)    87    2     6     A <- A+((dp+X+1)(dp+X)) NV..H..ZC
op87:
    loadDpX
    readMem8
    adcFromR0
    fetchSetC 6

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   ADC    A,(dp)+Y    97    2     6     A <- A+((dp+1)(dp)+Y)   NV..H..ZC
op97:
    loadDpY
    readMem8
    adcFromR0
    fetchSetC 6
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   ADC    (X),(Y)     99    1     5   (X) <- (X)+(Y)+C          NV..H..ZC
op99:
    loadXY
    adcToMem
    fetch 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   ADC    dp(d),dp(s) 89    3     6 (dp(d))<-(dp(d))+(dp(s))+C  NV..H..ZC
op89:
    loadDpDp
    adcToMem
    fetch 6

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   ADC    dp,#inm     98    3     5  (dp) <- (dp)+inm+C         NV..H..ZC
op98:
    loadDpImm
    adcToMem
    fetch 5
    
////////////////////////////////////////////////////////////////////////////
// Subtract with carry commands
////////////////////////////////////////////////////////////////////////////
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SBC    A,#inm      A8    2     2     A <- A-inm-!C           NV..H..ZC
opA8:
    ldrb r0, [APU_PC], #1
    sbcFromR0
    fetchSetC 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SBC    A,(X)       A6    1     3     A <- A-(X)-!C           NV..H..ZC
opA6:
    add r12, APU_DP, APU_REG_X, lsr #24
    readMem8
    sbcFromR0
    fetchSetC 3

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SBC    A,dp        A4    2     3     A <- A-(dp)-!C          NV..H..ZC
opA4:
    loadDp
    readMem8
    sbcFromR0
    fetchSetC 3
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SBC    A,dp+X      B4    2     4     A <- A-(dp+X)-!C        NV..H..ZC
opB4:
    loadDpOfs APU_REG_X
    readMem8
    sbcFromR0
    fetchSetC 4
   
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SBC    A,labs      A5    3     4     A <- A-(abs)-!C         NV..H..ZC
opA5:
    loadImm
    readMem8
    sbcFromR0
    fetchSetC 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SBC    A,labs+X    B5    3     5     A <- A-(abs+X)-!C       NV..H..ZC
opB5:
    loadImmOfs APU_REG_X
    readMem8
    sbcFromR0
    fetchSetC 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SBC    A,labs+Y    B6    3     5     A <- A-(abs+Y)-!C       NV..H..ZC
opB6:
    loadImmOfs APU_REG_Y
    readMem8
    sbcFromR0
    fetchSetC 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SBC    A,(dp+X)    A7    2     6  A <- A-((dp+X+1)(dp+X))-!C NV..H..ZC
opA7:
    loadDpX
    readMem8
    sbcFromR0
    fetchSetC 6
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SBC    A,(dp)+Y    B7    2     6  A <- A-((dp+1)(dp)+Y)-!C   NV..H..ZC
opB7:
    loadDpY
    readMem8
    sbcFromR0
    fetchSetC 6   

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SBC    (X),(Y)     B9    1     5   (X) <- (X)-(Y)-!C         NV..H..ZC
opB9:
    loadXY
    sbcToMem
    fetch 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SBC    dp(d),dp(s) A9    3     6 (dp(d))<-(dp(d))-(dp(s))-!C NV..H..ZC
opA9:
    loadDpDp
    sbcToMem
    fetch 6    
      
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SBC    dp,#inm     B8    3     5  (dp) <- (dp)-inm-!C        NV..H..ZC
opB8:
    loadDpImm
    sbcToMem
    fetch 5

////////////////////////////////////////////////////////////////////////////
// Comparison commands
////////////////////////////////////////////////////////////////////////////
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    A,#inm      68    2     2     A-inm                   N......ZC
op68:
    ldrb r0, [APU_PC], #1
    cmpFromR0 APU_REG_A
    fetchSetC 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    A,(X)       66    1     3     A-(X)                   N......ZC
op66:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	 
    add r12, APU_DP, APU_REG_X, lsr #24
    readMem8
    cmpFromR0 APU_REG_A
    fetchSetC 3

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    A,dp        64    2     3     A-(dp)                  N......ZC
op64:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	 
    loadDp
    readMem8
    cmpFromR0 APU_REG_A
    fetchSetC 3

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    A,dp+X      74    2     4     A-(dp+X)                N......ZC
op74:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	 
    loadDpOfs APU_REG_X
    readMem8
    cmpFromR0 APU_REG_A
    fetchSetC 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    A,labs      65    3     4     A-(abs)                 N......ZC
op65:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	 
    loadImm
    readMem8
    cmpFromR0 APU_REG_A
    fetchSetC 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    A,labs+X    75    3     5     A-(abs+X)               N......ZC
op75:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	 
    loadImmOfs APU_REG_X
    readMem8
    cmpFromR0 APU_REG_A
    fetchSetC 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    A,labs+Y    76    3     5     A-(abs+Y)               N......ZC
op76:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	 
    loadImmOfs APU_REG_Y
    readMem8
    cmpFromR0 APU_REG_A
    fetchSetC 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    A,(dp+X)    67    2     6     A-((dp+X+1)(dp+X))      N......ZC
op67:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	 
    loadDpX
    readMem8
    cmpFromR0 APU_REG_A
    fetchSetC 6    
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    A,(dp)+Y    77    2     6     A-((dp+1)(dp)+Y)        N......ZC
op77:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	 
    loadDpY
    readMem8
    cmpFromR0 APU_REG_A
    fetchSetC 6
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    (X),(Y)     79    1     5     (X)-(Y)                 N......ZC
op79:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	 
    loadXY
    cmpToMem
    fetchSetC 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    dp(d),dp(s) 69    3     6     (dp(d))-(dp(s))         N......ZC
op69:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	 
    loadDpDp
    cmpToMem
    fetchSetC 6
    
  
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    dp,#inm     78    3     5     (dp)-inm                N......ZC
op78:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	 
    loadDpImm
    cmpToMem
    fetchSetC 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    X,#inm      C8    2     2     X-inm                   N......ZC
opC8:
    ldrb r0, [APU_PC], #1
    cmpFromR0 APU_REG_X
    fetchSetC 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    X,dp        3E    2     3     X-(dp)                  N......ZC
op3E:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadDp
    readMem8
    cmpFromR0 APU_REG_X
    fetchSetC 3
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    X,labs      1E    3     4     X-(abs)                 N......ZC
op1E:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadImm
    readMem8
    cmpFromR0 APU_REG_X
    fetchSetC 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    Y,#inm      AD    2     2     Y-inm                   N......ZC
opAD:
    ldrb r0, [APU_PC], #1
    cmpFromR0 APU_REG_Y
    fetchSetC 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    Y,dp        7E    2     3     Y-(dp)                  N......ZC
op7E:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadDp
    readMem8
    cmpFromR0 APU_REG_Y
    fetchSetC 3
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CMP    Y,labs      5E    3     4     Y-(abs)                 N......ZC
op5E:
.ifeq CheckIntrLoop-1
	str APU_PC, APU_PC_save
.endif	
    loadImm
    readMem8
    cmpFromR0 APU_REG_Y
    fetchSetC 4
    
////////////////////////////////////////////////////////////////////////////
// Logical AND commands
////////////////////////////////////////////////////////////////////////////
   
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   AND    A,#inm      28    2     2     A <- A AND inm           N......Z.
op28:
    ldrb r0, [APU_PC], #1
    andFromR0
    fetch 2

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   AND    A,(X)       26    1     3     A <- A AND (X)           N......Z.
op26:
    add r12, APU_DP, APU_REG_X, lsr #24
    readMem8
    andFromR0
    fetch 3

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   AND    A,dp        24    2     3     A <- A AND (dp)          N......Z.
op24:
    loadDp
    readMem8
    andFromR0
    fetch 3
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   AND    A,dp+X      34    2     4     A <- A AND (dp+X)        N......Z.
op34:
    loadDpOfs APU_REG_X
    readMem8
    andFromR0
    fetch 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   AND    A,labs      25    3     4     A <- A AND (abs)         N......Z.
op25:
    loadImm
    readMem8
    andFromR0
    fetch 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   AND    A,labs+X    35    3     5     A <- A AND (abs+X)       N......Z.
op35:
    loadImmOfs APU_REG_X
    readMem8
    andFromR0
    fetch 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   AND    A,labs+Y    36    3     5     A <- A AND (abs+Y)       N......Z.
op36:
    loadImmOfs APU_REG_Y
    readMem8
    andFromR0
    fetch 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   AND    A,(dp+X)    27    2     6  A <- A AND ((dp+X+1)(dp+X)) N......Z.
op27:
    loadDpX
    readMem8
    andFromR0
    fetch 6
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   AND    A,(dp)+Y    37    2     6   A <- A AND ((dp+1)(dp)+Y)  N......Z.
op37:
    loadDpY
    readMem8
    andFromR0
    fetch 6
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   AND    (X),(Y)     39    1     5   (X) <- (X) AND (Y)         N......Z.
op39:
    loadXY
    andToMem
    fetch 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   AND    dp(d),dp(s) 29    3     6 (dp(d))<-(dp(d)) AND (dp(s)) N......Z.
op29:
    loadDpDp
    andToMem
    fetch 6    
           
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   AND    dp,#inm     38    3     5  (dp) <- (dp) AND inm        N......Z.
op38:
    loadDpImm
    andToMem
    fetch 5
    
////////////////////////////////////////////////////////////////////////////
// Logical OR operations
////////////////////////////////////////////////////////////////////////////
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   OR     A,#inm      08    2     2     A <- A OR inm            N......Z.
op08:
    ldrb r0, [APU_PC], #1
    orFromR0
    fetch 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   OR     A,(X)       06    1     3     A <- A OR (X)            N......Z.
op06:
    add r12, APU_DP, APU_REG_X, lsr #24
    readMem8
    orFromR0
    fetch 3
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   OR     A,dp        04    2     3     A <- A OR (dp)           N......Z.
op04:
    loadDp
    readMem8
    orFromR0
    fetch 3
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   OR     A,dp+X      14    2     4     A <- A OR (dp+X)         N......Z.
op14:
    loadDpOfs APU_REG_X
    readMem8
    orFromR0
    fetch 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   OR     A,labs      05    3     4     A <- A OR (abs)          N......Z.
op05:
    loadImm
    readMem8
    orFromR0
    fetch 4
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   OR     A,labs+X    15    3     5     A <- A OR (abs+X)        N......Z.
op15:
    loadImmOfs APU_REG_X
    readMem8
    orFromR0
    fetch 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   OR     A,labs+Y    16    3     5     A <- A OR (abs+Y)        N......Z.
op16:
    loadImmOfs APU_REG_Y
    readMem8
    orFromR0
    fetch 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   OR     A,(dp+X)    07    2     6  A <- A OR ((dp+X+1)(dp+X))  N......Z.
op07:
    loadDpX
    readMem8
    orFromR0
    fetch 6

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   OR     A,(dp)+Y    17    2     6   A <- A OR ((dp+1)(dp)+Y)   N......Z.
op17:
    loadDpY
    readMem8
    orFromR0
    fetch 6
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   OR     (X),(Y)     19    1     5   (X) <- (X) OR (Y)          N......Z.
op19:
    loadXY
    orToMem
    fetch 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   OR     dp(d),dp(s) 09    3     6 (dp(d))<-(dp(d)) OR (dp(s))  N......Z.
op09:
    loadDpDp
    orToMem
    fetch 6
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   OR		dp,#inm     18    3     5  (dp) <- (dp) OR inm         N......Z.
op18:
    loadDpImm
    orToMem
    fetch 5
    
////////////////////////////////////////////////////////////////////////////
// Logical EOR operations
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   EOR    A,#inm      48    2     2     A <- A EOR inm           N......Z.
op48:
    ldrb r0, [APU_PC], #1
    eorFromR0
    fetch 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   EOR    A,(X)       46    1     3     A <- A EOR (X)           N......Z.
op46:
    add r12, APU_DP, APU_REG_X, lsr #24
    readMem8
    eorFromR0
    fetch 3
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   EOR    A,dp        44    2     3     A <- A EOR (dp)          N......Z.
op44:
    loadDp
    readMem8
    eorFromR0
    fetch 3
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   EOR    A,dp+X      54    2     4     A <- A EOR (dp+X)        N......Z.
op54:
    loadDpOfs APU_REG_X
    readMem8
    eorFromR0
    fetch 4

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   EOR    A,labs      45    3     4     A <- A EOR (abs)         N......Z.
op45:
    loadImm
    readMem8
    eorFromR0
    fetch 4

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   EOR    A,labs+X    55    3     5     A <- A EOR (abs+X)       N......Z.
op55:
    loadImmOfs APU_REG_X
    readMem8
    eorFromR0
    fetch 5

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   EOR    A,labs+Y    56    3     5     A <- A EOR (abs+Y)       N......Z.
op56:
    loadImmOfs APU_REG_Y
    readMem8
    eorFromR0
    fetch 5
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   EOR    A,(dp+X)    47    2     6  A <- A EOR ((dp+X+1)(dp+X)) N......Z.
op47:
    loadDpX
    readMem8
    eorFromR0
    fetch 6

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   EOR    A,(dp)+Y    57    2     6   A <- A EOR ((dp+1)(dp)+Y)  N......Z.
op57:
    loadDpY
    readMem8
    eorFromR0
    fetch 6
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   EOR    (X),(Y)     59    1     5   (X) <- (X) EOR (Y)         N......Z.
op59:
    loadXY
    eorToMem
    fetch 5

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   EOR    dp(d),dp(s) 49    3     6 (dp(d))<-(dp(d)) EOR (dp(s)) N......Z.
op49:
    loadDpDp
    eorToMem
    fetch 6
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   EOR    dp,#inm     58    3     5  (dp) <- (dp) EOR inm        N......Z.
op58:
    loadDpImm
    eorToMem
    fetch 5
    
////////////////////////////////////////////////////////////////////////////
// Increment commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   INC      A         BC    1     2      ++ A                  N......Z.
opBC:
    add APU_REG_A, APU_REG_A, #(1 << 24)
    mov APU_NZ, APU_REG_A, lsr #24
    fetch 2
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   INC      dp        AB    2     4      ++ (dp)               N......Z.
opAB:
    loadDp
    incToMem
    fetch 4

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   INC     dp+X       BB    2     5      ++ (dp+X)             N......Z.
opBB:
    loadDpOfs APU_REG_X
    incToMem
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   INC     labs       AC    3     5      ++ (abs)              N......Z.
opAC:
    loadImm
    incToMem
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   INC      X         3D    1     2      ++ X                  N......Z.
op3D:
    add APU_REG_X, APU_REG_X, #(1 << 24)
    mov APU_NZ, APU_REG_X, lsr #24
    fetch 2
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   INC      Y         FC    1     2      ++ Y                  N......Z.
opFC:
    add APU_REG_Y, APU_REG_Y, #(1 << 24)
    mov APU_NZ, APU_REG_Y, lsr #24
    fetch 2
    
////////////////////////////////////////////////////////////////////////////
// Decrement commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   DEC      A         9C    1     2      -- A                  N......Z.
op9C:
    sub APU_REG_A, APU_REG_A, #(1 << 24)
    mov APU_NZ, APU_REG_A, lsr #24
    fetch 2
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   DEC      dp        8B    2     4      -- (dp)               N......Z.
op8B:
    loadDp
    decToMem
    fetch 4

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   DEC     dp+X       9B    2     5      -- (dp+X)             N......Z.
op9B:
    loadDpOfs APU_REG_X
    decToMem
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   DEC     labs       8C    3     5      -- (abs)              N......Z.
op8C:
    loadImm
    decToMem
    fetch 5

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   DEC      X         1D    1     2      -- X                  N......Z.
op1D:
    sub APU_REG_X, APU_REG_X, #(1 << 24)
    mov APU_NZ, APU_REG_X, lsr #24
    fetch 2
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   DEC      Y         DC    1     2      -- Y                  N......Z.
opDC:
    sub APU_REG_Y, APU_REG_Y, #(1 << 24)
    mov APU_NZ, APU_REG_Y, lsr #24
    fetch 2
    
////////////////////////////////////////////////////////////////////////////
// Shift and rotation commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   ASL      A         1C    1     2      C << A      <<0       N......ZC
op1C:
    @ Make sure no other bits are set in A right now
    and APU_REG_A, APU_REG_A, #0xFF000000
    movs APU_REG_A, APU_REG_A, lsl #1
    mov APU_NZ, APU_REG_A, lsr #24
    bic APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
    fetchSetC 2

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   ASL      dp        0B    2     4      C << (dp)   <<0       N......ZC
op0B:
    loadDp
    aslToMem
    fetch 4

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   ASL     dp+X       1B    2     5      C << (dp+X) <<0       N......ZC
op1B:
    loadDpOfs APU_REG_X
    aslToMem
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   ASL     labs       0C    3     5      C << (abs)  <<0       N......ZC
op0C:
    loadImm
    aslToMem
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   LSR      A         5C    1     2      0 >> A      <<C       N......ZC
op5C:
    movs APU_NZ, APU_REG_A, lsr #25
    mov APU_REG_A, APU_NZ, lsl #24
    bic APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
    fetchSetC 2

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   LSR      dp        4B    2     4      0 >> (dp)   <<C       N......ZC
op4B:
    loadDp
    lsrToMem
    fetch 4
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   LSR     dp+X       5B    2     5      0 >> (dp+X) <<C       N......ZC
op5B:
    loadDpOfs APU_REG_X
    lsrToMem
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   LSR     labs       4C    3     5      0 >> (abs)  <<C       N......ZC
op4C:
    loadImm
    lsrToMem
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   ROL      A         3C    1     2      C << A      <<C       N......ZC
op3C:
    movs APU_CYCLES, APU_CYCLES, lsr #1 @ Get C flag
    and APU_REG_A, APU_REG_A, #0xFF000000
    orrcs APU_REG_A, APU_REG_A, #(1 << 23)
    movs APU_REG_A, APU_REG_A, lsl #1
    mov APU_NZ, APU_REG_A, lsr #24
    adc APU_CYCLES, APU_CYCLES, APU_CYCLES
    fetch 2
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   ROL      dp        2B    2     4      C << (dp)   <<C       N......ZC
op2B:
    loadDp
    rolToMem
    fetch 4

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   ROL     dp+X       3B    2     5      C << (dp+X) <<C       N......ZC
op3B:
    loadDpOfs APU_REG_X
    rolToMem
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   ROL     labs       2C    3     5      C << (abs)  <<C       N......ZC
op2C:
    loadImm
    rolToMem
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   ROR      A         7C    1     2      C >> A      <<C       N......ZC
op7C:
    movs APU_CYCLES, APU_CYCLES, lsr #1 @ Get C flag
    mov APU_NZ, APU_REG_A, rrx
    movs APU_NZ, APU_NZ, lsr #24
    mov APU_REG_A, APU_NZ, lsl #24
    adc APU_CYCLES, APU_CYCLES, APU_CYCLES
    fetch 2

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   ROR      dp        6B    2     4      C >> (dp)   <<C       N......ZC
op6B:
    loadDp
    rorToMem
    fetch 4

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   ROR     dp+X       7B    2     5      C >> (dp+X) <<C       N......ZC
op7B:
    loadDpOfs APU_REG_X
    rorToMem
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   ROR     labs       6C    3     5      C >> (abs)  <<C       N......ZC
op6C:
    loadImm
    rorToMem
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   XCN      A         9F    1     5      A(7-4) <-> A(3-0)     N......Z.
op9F:
    mov r0, APU_REG_A, lsr #28
    mov APU_REG_A, APU_REG_A, lsl #4
    orr APU_REG_A, APU_REG_A, r0, lsl #24
    mov APU_NZ, APU_REG_A, lsr #24
    fetch 5
    
////////////////////////////////////////////////////////////////////////////
// 16 bit transmission commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   MOVW     YA,dp     BA    2     5     YA  <- (dp+1)(dp)      N......Z.
opBA:
    loadDp
    readMem8
    and APU_NZ, r0, #0x7f
    mov APU_REG_A, r0, lsl #24
    add r12, r12, #1
    readMem8
    orr APU_NZ, APU_NZ, r0
    mov APU_REG_Y, r0, lsl #24
    fetch 5

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   MOVW     dp,YA     DA    2     4    (dp+1)(dp) <- YA        .........
opDA:
    loadDp
    mov r0, APU_REG_A, lsr #24
    writeMem8ZeroPage
    add r12, r12, #1
    mov r0, APU_REG_Y, lsr #24
    writeMem8ZeroPage
    fetch 4
    
///////////////////////////////////////////////////////////////////////////
// 16 bit operation commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   INCW     dp        3A    2     6  Increment dp memory pair  N......Z.
op3A:
    loadDp
    readMem8NoCheck
    mov r1, r0, lsl #16
    add r12, r12, #1
    readMem8NoCheck
    orr r1, r1, r0, lsl #24
    adds r1, r1, #(1 << 16)
    @ Set NZ flags badly :(
    mov APU_NZ, #0
    movmi APU_NZ, #0x80
    orrne APU_NZ, APU_NZ, #1
    @ Write the bits back to memory
    mov r0, r1, lsr #24
    writeMem8ZeroPage
    sub r12, r12, #1
    mov r0, r1, lsr #16
    writeMem8ZeroPage
    fetch 6

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   DECW     dp        1A    2     6  Decrement dp memory pair  N......Z.
op1A:
    loadDp
    readMem8NoCheck
    mov r1, r0, lsl #16
    add r12, r12, #1
    readMem8NoCheck
    orr r1, r1, r0, lsl #24
    subs r1, r1, #(1 << 16)
    @ Set NZ flags badly :(
    mov APU_NZ, #0
    movmi APU_NZ, #0x80
    orrne APU_NZ, APU_NZ, #1
    @ Write the bits back to memory
    mov r0, r1, lsr #24
    writeMem8ZeroPage
    sub r12, r12, #1
    mov r0, r1, lsr #16
    writeMem8ZeroPage
    fetch 6
    

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   ADDW    YA,dp      7A    2     5   YA  <- YA + (dp+1)(dp)   NV..H..ZC
op7A:
    @ Load the DP word pair
    loadDp
    readMem8
    mov r1, r0, lsl #16
    add r12, r12, #1
    readMem8
    orr r1, r1, r0, lsl #24

    @ Build YA and add r1 to it
    orr APU_REG_A, APU_REG_Y, APU_REG_A, lsr #8
    adds APU_REG_A, APU_REG_A, r1

    @ Set NZ flags badly :(
    mov APU_NZ, #0
    movmi APU_NZ, #0x80
    orrne APU_NZ, APU_NZ, #1

    @ Set the normal flags
    bic APU_CYCLES, APU_CYCLES, #(CYCLES_C_FLAG | CYCLES_V_FLAG)
    orrvs APU_CYCLES, APU_CYCLES, #CYCLES_V_FLAG
    @ C flag will be set in fetchSetC
    
    @ Move YA back into A and Y
    and APU_REG_Y, APU_REG_A, #0xff000000
    mov APU_REG_A, APU_REG_A, lsl #8
    
    fetchSetC 5

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   SUBW    YA,dp      9A    2     5   YA  <- YA - (dp+1)(dp)   NV..H..ZC
op9A:
    @ Load the DP word pair
    loadDp
    readMem8
    mov r1, r0, lsl #16
    add r12, r12, #1
    readMem8
    orr r1, r1, r0, lsl #24

    @ Build YA and sub r1 to it
    orr APU_REG_A, APU_REG_Y, APU_REG_A, lsr #8
    subs APU_REG_A, APU_REG_A, r1

    @ Set NZ flags badly :(
    mov APU_NZ, #0
    movmi APU_NZ, #0x80
    orrne APU_NZ, APU_NZ, #1

    @ Set the normal flags
    bic APU_CYCLES, APU_CYCLES, #(CYCLES_C_FLAG | CYCLES_V_FLAG)
    orrvs APU_CYCLES, APU_CYCLES, #CYCLES_V_FLAG
    @ C flag will be set in fetchSetC
    
    @ Move YA back into A and Y
    and APU_REG_Y, APU_REG_A, #0xff000000
    mov APU_REG_A, APU_REG_A, lsl #8
    
    fetchSetC 5

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   CMPW    YA,dp      5A    2     4     YA - (dp+1)(dp)        N......Z.
op5A:
    @ Load the DP word pair
    loadDp
    readMem8
    mov r1, r0, lsl #16
    add r12, r12, #1
    readMem8
    orr r1, r1, r0, lsl #24

    @ Build YA and sub r1 to it
    orr r0, APU_REG_Y, APU_REG_A, lsr #8
    subs r0, r0, r1

    @ Set NZ flags badly :(
    mov APU_NZ, #0
    movmi APU_NZ, #0x80
    orrne APU_NZ, APU_NZ, #1

    @ Set the normal flags
    bic APU_CYCLES, APU_CYCLES, #(CYCLES_C_FLAG | CYCLES_V_FLAG)
    orrvs APU_CYCLES, APU_CYCLES, #CYCLES_V_FLAG

    fetchSetC 4
    
////////////////////////////////////////////////////////////////////////////
// Multiplication and division commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   MUL      YA        CF    1     9    YA(16 bits) <- Y * A    N......Z.
opCF:
    mov r0, APU_REG_A, lsr #24
    mov r1, APU_REG_Y, lsr #24
    mul APU_REG_Y, r0, r1
    movs APU_REG_Y, APU_REG_Y, lsl #16
    mov APU_REG_A, APU_REG_Y, lsl #8
    
    mov APU_NZ, APU_REG_Y, lsr #24
    movs r0, APU_REG_A, lsr #24
    orrne APU_NZ, APU_NZ, #1

    fetch 9

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   DIV      YA,X      9E    1    12    Q:A B:Y <- YA / X       NV..H..Z.
op9E:
    mov r1, APU_REG_X, lsr #24
    cmps r1, #0
    bne 1f

    @ Divide by zero, set NV, clear Z
    orr APU_CYCLES, APU_CYCLES, #CYCLES_V_FLAG
    mov APU_NZ, #0x81
    mov APU_REG_Y, #0xff << 24
    mov APU_REG_A, #0xff << 24
    b 2f
1:
    @ Normal divide, Y = modulo result, A = result
    bic APU_CYCLES, APU_CYCLES, #CYCLES_V_FLAG
    cmps APU_REG_Y, APU_REG_A
    @ Set V flag when Y >= A
    orrge APU_CYCLES, APU_CYCLES, #CYCLES_V_FLAG

    mov r0, APU_REG_Y, lsr #16
    bic r0, r0, #0xff
    orr r0, r0, APU_REG_A, lsr #24
    
    @ Swi 0x9 is divide, r0 = numerator, r1 = divisor.  Result, r0 = result, r1 = modulus
    swi	#0x09 << 16
    mov APU_REG_Y, r1, lsl #24
    mov APU_REG_A, r0, lsl #24
    
    @ Set NZ flags
    mov APU_NZ, r0

2:
    fetch 12

////////////////////////////////////////////////////////////////////////////
// Decimal compensation commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   DAA       A        DF    1     3    decimal adjust for add  N......ZC
opDF:
    notDone 0xDF
    fetch 3

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   DAS       A        BE    1     3    decimal adjust for sub  N......ZC
opBE:
    notDone 0xBE
    fetch 3
    
////////////////////////////////////////////////////////////////////////////
// Branching commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   BRA     rel        2F    2     4   branch always                  ...
op2F:
    ldrsb r0, [APU_PC], #1
    add APU_PC, APU_PC, r0
.ifeq CheckIntrLoop-1
    bl CheckBranch
.endif
    fetch 4

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   BEQ     rel        F0    2    2/4  branch on Z=1                  ...
opF0:
    ldrsb r0, [APU_PC], #1
    tsts APU_NZ, #0xFF
    addeq APU_PC, APU_PC, r0
    addeq APU_CYCLES, APU_CYCLES, #2 << CYCLE_SHIFT
.ifeq CheckIntrLoop-1
    bleq CheckBranch
.endif
    fetch 2

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   BNE     rel        D0    2    2/4  branch on Z=0                  ...
opD0:
    ldrsb r0, [APU_PC], #1
    tsts APU_NZ, #0xFF
    addne APU_PC, APU_PC, r0
    addne APU_CYCLES, APU_CYCLES, #2 << CYCLE_SHIFT
.ifeq CheckIntrLoop-1
    blne CheckBranch
.endif
    fetch 2

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   BCS     rel        B0    2    2/4  branch on C=1                  ...
opB0:
    ldrsb r0, [APU_PC], #1
    tsts APU_CYCLES, #CYCLES_C_FLAG
    addne APU_PC, APU_PC, r0
    addne APU_CYCLES, APU_CYCLES, #2 << CYCLE_SHIFT
.ifeq CheckIntrLoop-1
    blne CheckBranch
.endif
    fetch 2
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   BCC     rel        90    2    2/4  branch on C=0                  ...
op90:
    ldrsb r0, [APU_PC], #1
    tsts APU_CYCLES, #CYCLES_C_FLAG
    addeq APU_PC, APU_PC, r0
    addeq APU_CYCLES, APU_CYCLES, #2 << CYCLE_SHIFT
.ifeq CheckIntrLoop-1
    bleq CheckBranch
.endif
    fetch 2

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   BVS     rel        70    2    2/4  branch on V=1                  ...
op70:
    ldrsb r0, [APU_PC], #1
    tsts APU_CYCLES, #CYCLES_V_FLAG
    addne APU_PC, APU_PC, r0
    addne APU_CYCLES, APU_CYCLES, #2 << CYCLE_SHIFT
.ifeq CheckIntrLoop-1
    blne CheckBranch
.endif   
    fetch 2
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   BVC     rel        50    2    2/4  branch on V=0                  ...
op50:
    ldrsb r0, [APU_PC], #1
    tsts APU_CYCLES, #CYCLES_V_FLAG
    addeq APU_PC, APU_PC, r0
    addeq APU_CYCLES, APU_CYCLES, #2 << CYCLE_SHIFT
.ifeq CheckIntrLoop-1
    bleq CheckBranch
.endif    
    fetch 2
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   BMI     rel        30    2    2/4  branch on N=1                  ...
op30:
    ldrsb r0, [APU_PC], #1
    tsts APU_NZ, #0x80
    addne APU_PC, APU_PC, r0
    addne APU_CYCLES, APU_CYCLES, #2 << CYCLE_SHIFT
.ifeq CheckIntrLoop-1
    blne CheckBranch
.endif
    fetch 2

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   BPL     rel        10    2    2/4  branch on N=0                  ...
op10:
    ldrsb r0, [APU_PC], #1
    tsts APU_NZ, #0x80
    addeq APU_PC, APU_PC, r0
    addeq APU_CYCLES, APU_CYCLES, #2 << CYCLE_SHIFT
.ifeq CheckIntrLoop-1
    bleq CheckBranch
.endif
    fetch 2

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   BBS   dp.bit,rel   x3    3    5/7  branch on dp.bit=1             ...
op03: branchDpBitSet 0
op23: branchDpBitSet 1
op43: branchDpBitSet 2
op63: branchDpBitSet 3
op83: branchDpBitSet 4
opA3: branchDpBitSet 5
opC3: branchDpBitSet 6
opE3: branchDpBitSet 7

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   BBC   dp.bit,rel   y3    3    5/7  branch on dp.bit=0             ...
op13: branchDpBitClear 0
op33: branchDpBitClear 1
op53: branchDpBitClear 2
op73: branchDpBitClear 3
op93: branchDpBitClear 4
opB3: branchDpBitClear 5
opD3: branchDpBitClear 6
opF3: branchDpBitClear 7

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   CBNE   dp,rel      2E    3    5/7  compare A with (dp) then BNE   ...
op2E:
    loadDp
    readMem8
    ldrsb r1, [APU_PC], #1
    cmps r0, APU_REG_A, lsr #24
    addne APU_PC, APU_PC, r1
    addne APU_CYCLES, APU_CYCLES, #2 << CYCLE_SHIFT
.ifeq CheckIntrLoop-1
    blne CheckBranch
.endif
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   CBNE  dp+X,rel     DE    3    6/8  compare A with (dp+X) then BNE ...
opDE:
    loadDpOfs APU_REG_X
    readMem8
    ldrsb r1, [APU_PC], #1
    cmps r0, APU_REG_A, lsr #24
    addne APU_PC, APU_PC, r1
    addne APU_CYCLES, APU_CYCLES, #2 << CYCLE_SHIFT
.ifeq CheckIntrLoop-1
    blne CheckBranch
.endif
    fetch 6
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   DBNZ   dp,rel      6E    3    5/7  decrement memory (dp) then JNZ ...
op6E:
    loadDp
    readMem8NoCheck
    subs r0, r0, #1
    ldrsb r1, [APU_PC], #1
    addne APU_PC, APU_PC, r1
    addne APU_CYCLES, APU_CYCLES, #2 << CYCLE_SHIFT
    writeMem8
    fetch 5

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   DBNZ    Y,rel      FE    2    4/6  decrement Y then JNZ           ...
opFE:
    ldrsb r0, [APU_PC], #1
    subs APU_REG_Y, APU_REG_Y, #(1 << 24)
    addne APU_PC, APU_PC, r0
    addne APU_CYCLES, APU_CYCLES, #2 << CYCLE_SHIFT
    fetch 4
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   JMP     labs       5F    3     3   jump to new location           ...
op5F:
    loadImm
    add APU_PC, APU_RAMBASE, r12
    fetch 3
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   JMP    (labs+X)    1F    3     6   PC <- (abs+X+1)(abs+X)         ...
op1F:
    loadImmOfs APU_REG_X
    @ Read the 16 bit value
    readMem8
    mov r1, r0
    add r12, r12, #1
    readMem8
    orr r0, r1, r0, lsl #8
    @ Modify the PC
    add APU_PC, APU_RAMBASE, r0
    fetch 6

// Mnemonic  Operand   Code Bytes Cycles Operation                 Flag
//   CALL     labs      3F    3     8   subroutine call          ........ 
op3F:
    loadImm
    sub r1, APU_PC, APU_RAMBASE
    add APU_PC, APU_RAMBASE, r12
    push16
    fetch 8

// Mnemonic  Operand   Code Bytes Cycles Operation                 Flag
//   PCALL   upage      4F    2     6   upage call               ........ 
op4F:
    ldrb r0, [APU_PC], #1
    sub r1, APU_PC, APU_RAMBASE
    add APU_PC, APU_RAMBASE, r0
    add APU_PC, APU_PC, #0xff00
    push16
    fetch 6

// Mnemonic  Operand   Code Bytes Cycles Operation                 Flag
//   TCALL     n        n1    1     8   table call               ........ 
op01: tcall 0x0
op11: tcall 0x1
op21: tcall 0x2
op31: tcall 0x3
op41: tcall 0x4
op51: tcall 0x5
op61: tcall 0x6
op71: tcall 0x7
op81: tcall 0x8
op91: tcall 0x9
opA1: tcall 0xA
opB1: tcall 0xB
opC1: tcall 0xC
opD1: tcall 0xD
opE1: tcall 0xE
opF1: tcall 0xF

@ Pool here for the tcall loading APU_EXTRA_MEM address
.POOL

// Mnemonic  Operand   Code Bytes Cycles Operation                 Flag
//   BRK                0F    1     8   software interrupt       ...1.0..                                                    
op0F:
    notDone 0x0F
    fetch 8
    
// Mnemonic  Operand   Code Bytes Cycles Operation                 Flag
//   RET                6F    1     5   return from subroutine   ........ 
op6F:
    pop16
    add APU_PC, APU_RAMBASE, r1
    fetch 5                 

// Mnemonic  Operand   Code Bytes Cycles Operation                 Flag
//   RET1               7F    1     6   return from interrupt   (Restored)
op7F:
    notDone 0x7F
    fetch 6

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Stack variable
@ Located here so that ldr rx, APU_SP works
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.GLOBAL APU_SP
APU_SP:
.hword 0
.ALIGN

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    X, SP       9D    1     2     X <- SP                  N......Z
op9D:
    ldrb APU_NZ, APU_SP
    mov APU_REG_X, APU_NZ, lsl #24
    fetch 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   MOV    SP, X       BD    1     2    SP <- X                   ........
opBD:
    mov r1, APU_REG_X, lsr #24
    strb r1, APU_SP
    fetch 2

////////////////////////////////////////////////////////////////////////////
// Stack operation commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   PUSH      A        2D    1     4    push A to stack         .........
op2D:
    mov r0, APU_REG_A, lsr #24
    push8
    fetch 4
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   PUSH      X        4D    1     4    push X to stack         .........
op4D:
    mov r0, APU_REG_X, lsr #24
    push8
    fetch 4

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   PUSH      Y        6D    1     4    push Y to stack         .........
op6D:
    mov r0, APU_REG_Y, lsr #24
    push8
    fetch 4

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   PUSH     PSW       0D    1     4    push PSW to stack       .........
op0D:
    and r0, APU_NZ, #1 << 7     @ N flag
    tsts APU_CYCLES, #CYCLES_V_FLAG
    orrne r0, r0, #1 << 6       @ V flag
    tsts APU_DP, #0x100
    orrne r0, r0, #1 << 5       @ Dp flag
    tsts APU_CYCLES, #CYCLES_H_FLAG
    orrne r0, r0, #1 << 3       @ H flag
    tsts APU_NZ, #0xFF
    orreq r0, r0, #1 << 1       @ Z flag
    tsts APU_CYCLES, #CYCLES_C_FLAG
    orrne r0, r0, #1            @ C flag
    push8
    fetch 4

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   POP       A        AE    1     4    pop A from stack        .........
opAE:
    pop8
    mov APU_REG_A, r0, lsl #24
    fetch 4
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   POP       X        CE    1     4    pop X from stack        .........
opCE:
    pop8
    mov APU_REG_X, r0, lsl #24
    fetch 4
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   POP       Y        EE    1     4    pop Y from stack        .........
opEE:
    pop8
    mov APU_REG_Y, r0, lsl #24
    fetch 4
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   POP      PSW       8E    1     4    pop PSW from stack     (Restored)
op8E:
    pop8
    @ Set CVH flags
    bic APU_CYCLES, APU_CYCLES, #(CYCLES_C_FLAG | CYCLES_V_FLAG | CYCLES_H_FLAG)
    tsts r0, #1 << 6
    orrne APU_CYCLES, APU_CYCLES, #CYCLES_V_FLAG
    tsts r0, #1
    orrne APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
    tsts r0, #1 << 3
    orrne APU_CYCLES, APU_CYCLES, #CYCLES_H_FLAG
    @ Set DP
    mov APU_DP, #0
    tsts r0, #1 << 5
    movne APU_DP, #0x100
    @ Set NZ
    and APU_NZ, r0, #0x82
    @ Toggle the Z bit
    eor APU_NZ, APU_NZ, #2
    fetch 4
    
////////////////////////////////////////////////////////////////////////////
// Bit operation commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   SET1    dip.bit    x2    2     4  set direct page bit       .........
op02: setDpBit 0
op22: setDpBit 1
op42: setDpBit 2
op62: setDpBit 3
op82: setDpBit 4
opA2: setDpBit 5
opC2: setDpBit 6
opE2: setDpBit 7

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   CLR1    dip.bit    y2    2     4  clear direct page bit     .........
op12: clearDpBit 0
op32: clearDpBit 1
op52: clearDpBit 2
op72: clearDpBit 3
op92: clearDpBit 4
opB2: clearDpBit 5
opD2: clearDpBit 6
opF2: clearDpBit 7

// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   TSET1    labs      0E    3     6 test and set bits with A   N......Z.
op0E:
    loadImm
    readMem8NoCheck
    and APU_NZ, r0, APU_REG_A, lsr #24
    orr r0, r0, APU_REG_A, lsr #24
    writeMem8
    fetch 6
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   TCLR1    labs      4E    3     6 test and clear bits with A N......Z.
op4E:
    loadImm
    readMem8NoCheck
    and APU_NZ, r0, APU_REG_A, lsr #24
    bic r0, r0, APU_REG_A, lsr #24
    writeMem8
    fetch 6
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   AND1   C,mem.bit   4A    3     4  C <- C AND (mem.bit)      ........C
op4A:
    loadImm
    mov r1, r12, lsr #13
    @ Clear the top 3 bits of the address bit
    bic r12, r12, #0xE000
    readMem8
    mov r2, #1
    @ If the bit isn't set, then clear whatever is there, otherwise, it's 1 and it remains the same
    tst r2, r0, lsr r1
    biceq APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
    fetch 4
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   AND1   C,/mem.bit  6A    3     4  C <- C AND !(mem.bit)     ........C
op6A:
    loadImm
    mov r1, r12, lsr #13
    @ Clear the top 3 bits of the address bit
    bic r12, r12, #0xE000
    readMem8
    mov r2, #1
    @ If the bit isn't set, then clear whatever is there, otherwise, it's 1 and it remains the same
    tst r2, r0, lsr r1
    bicne APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
    fetch 4
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   OR1    C,mem.bit   0A    3     5  C <- C OR  (mem.bit)      ........C
op0A:
    loadImm
    mov r1, r12, lsr #13
    @ Clear the top 3 bits of the address bit
    bic r12, r12, #0xE000
    readMem8
    mov r2, #1
    and r0, r2, r0, lsr r1
    @ Cycle flag is in lowest bit so we are fine here
    orr APU_CYCLES, APU_CYCLES, r0
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   OR1    C,/mem.bit  2A    3     5  C <- C OR  !(mem.bit)     ........C
op2A:
    loadImm
    mov r1, r12, lsr #13
    @ Clear the top 3 bits of the address bit
    bic r12, r12, #0xE000
    readMem8
    mov r2, #1
    and r0, r2, r0, lsr r1
    @ Do the !(mem.bit)
    eor r0, r0, r2
    @ Cycle flag is in lowest bit so we are fine here
    orr APU_CYCLES, APU_CYCLES, r0
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   EOR1   C,mem.bit   8A    3     5  C <- C EOR (mem.bit)      ........C
op8A:
    loadImm
    mov r1, r12, lsr #13
    @ Clear the top 3 bits of the address bit
    bic r12, r12, #0xE000
    readMem8
    mov r2, #1
    and r0, r2, r0, lsr r1
    @ Cycle flag is in lowest bit so we are fine here
    eor APU_CYCLES, APU_CYCLES, r0
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   NOT1   mem.bit     EA    3     5  complement (mem.bit)      .........
opEA:
    loadImm
    mov r1, r12, lsr #13
    @ Clear the top 3 bits of the address bit
    bic r12, r12, #0xE000
    readMem8
    mov r2, #1
    eor r0, r0, r2, lsl r1
    writeMem8ZeroPage
    fetch 5
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   MOV1   C,mem.bit   AA    3     4  C <- (mem.bit)            ........C
opAA:
    loadImm
    mov r1, r12, lsr #13
    @ Clear the top 3 bits of the address bit
    bic r12, r12, #0xE000
    readMem8
    mov r2, #1
    and r0, r2, r0, lsr r1
    bic APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
    @ Cycle flag is in lowest bit so we are fine here
    orr APU_CYCLES, APU_CYCLES, r0
    fetch 4
    
// Mnemonic  Operand   Code Bytes Cycles Operation                  Flag
//   MOV1   mem.bit,C   CA    3     6  C -> (mem.bit)            .........
opCA:
    loadImm
    mov r1, r12, lsr #13
    @ Clear the top 3 bits of the address bit
    bic r12, r12, #0xE000
    readMem8NoCheck
    tsts APU_CYCLES, #CYCLES_C_FLAG
    mov r2, #1
    biceq r0, r0, r2, lsl r1
    orrne r0, r0, r2, lsl r1
    writeMem8ZeroPage
    fetch 6
    
////////////////////////////////////////////////////////////////////////////
// Program status flag commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CLRC               60    1     2   clear carry flag          .......0 
op60:
    bic APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
    fetch 2
   
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SETC               80    1     2   set carry flag            .......1 
op80:
    orr APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
    fetch 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   NOTC               ED    1     3   complement carry flag     .......C 
opED:
    eor APU_CYCLES, APU_CYCLES, #CYCLES_C_FLAG
    fetch 3
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CLRV               E0    1     2   clear V and H             .0..0... 
opE0:
    bic APU_CYCLES, APU_CYCLES, #CYCLES_V_FLAG
    fetch 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   CLRP               20    1     2   clear direct page flag    ..0..... 
op20:
    mov APU_DP, #0
    fetch 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SETP               40    1     2   set direct page flag      ..1..0..
op40:
    mov APU_DP, #0x100
    fetch 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   EI                 A0    1     3  set interrup enable flag   .....1..
opA0:
    notDone 0xA0
    fetch 3
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   DI                 C0    1     3  clear interrup enable flag .....0..
opC0:
    notDone 0xC0
    fetch 3
    
////////////////////////////////////////////////////////////////////////////
// Other commands
////////////////////////////////////////////////////////////////////////////

// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   NOP               00    1     2    no operation               .........
op00:
    fetch 2
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   SLEEP             EF    1     3    standby SLEEP mode         .........
opEF:
/*    ldr r0, =apuSleeping
    ldrb r1, [r0]
    cmp r1, #1
    bne notSleeping
    @ We are sleeping already, check for a difference
    ldr r2, =0x027FFFF8
    ldr r2, [r2]
    ldr r1, apuSleepCheck
    cmp r2, r1
    beq keepSleeping
    @ Difference, we can set apuSleeping = 0 and not repeat this instruction
    mov r1, #0
    strb r1, [r0]
    b doneSleeping
notSleeping:
    @ Set apuSleeping = 1, and apuSleepCheck = inport value
    mov r1, #1
    strb r1, [r0]
    ldr r0, =0x027FFFF8
    ldr r0, [r0]
    str r0, apuSleepCheck
keepSleeping:
    @ Repeat this instruction until there is a difference on the inport
    sub APU_PC, APU_PC, #1
doneSleeping:*/
    fetch 3
    
// Mnemonic  Operand  Code Bytes Cycles Operation                    Flag
//   STOP              FF    1     3    standby STOP mode          .........
opFF:
    @ Just keep repeating the stop instruction
@    sub APU_PC, APU_PC, #1
    fetch 3

apuSleepCheck:
.word 0

.POOL

.GLOBAL CpuJumpTable
.GLOBAL MemZeroPageWriteTable
.GLOBAL MemZeroPageReadTable

.ALIGN
CpuJumpTable:
.word op00, op01, op02, op03, op04, op05, op06, op07, op08, op09, op0A, op0B, op0C, op0D, op0E, op0F
.word op10, op11, op12, op13, op14, op15, op16, op17, op18, op19, op1A, op1B, op1C, op1D, op1E, op1F
.word op20, op21, op22, op23, op24, op25, op26, op27, op28, op29, op2A, op2B, op2C, op2D, op2E, op2F
.word op30, op31, op32, op33, op34, op35, op36, op37, op38, op39, op3A, op3B, op3C, op3D, op3E, op3F
.word op40, op41, op42, op43, op44, op45, op46, op47, op48, op49, op4A, op4B, op4C, op4D, op4E, op4F
.word op50, op51, op52, op53, op54, op55, op56, op57, op58, op59, op5A, op5B, op5C, op5D, op5E, op5F
.word op60, op61, op62, op63, op64, op65, op66, op67, op68, op69, op6A, op6B, op6C, op6D, op6E, op6F
.word op70, op71, op72, op73, op74, op75, op76, op77, op78, op79, op7A, op7B, op7C, op7D, op7E, op7F
.word op80, op81, op82, op83, op84, op85, op86, op87, op88, op89, op8A, op8B, op8C, op8D, op8E, op8F
.word op90, op91, op92, op93, op94, op95, op96, op97, op98, op99, op9A, op9B, op9C, op9D, op9E, op9F
.word opA0, opA1, opA2, opA3, opA4, opA5, opA6, opA7, opA8, opA9, opAA, opAB, opAC, opAD, opAE, opAF
.word opB0, opB1, opB2, opB3, opB4, opB5, opB6, opB7, opB8, opB9, opBA, opBB, opBC, opBD, opBE, opBF
.word opC0, opC1, opC2, opC3, opC4, opC5, opC6, opC7, opC8, opC9, opCA, opCB, opCC, opCD, opCE, opCF
.word opD0, opD1, opD2, opD3, opD4, opD5, opD6, opD7, opD8, opD9, opDA, opDB, opDC, opDD, opDE, opDF
.word opE0, opE1, opE2, opE3, opE4, opE5, opE6, opE7, opE8, opE9, opEA, opEB, opEC, opED, opEE, opEF
.word opF0, opF1, opF2, opF3, opF4, opF5, opF6, opF7, opF8, opF9, opFA, opFB, opFC, opFD, opFE, opFF

MemZeroPageReadTable:
.rept 0x100
.word 0
.endr

MemZeroPageWriteTable:
.rept 0x140
.word 0
.endr
