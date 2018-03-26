/*
-------------------------------------------------------------------
Snezziboy v0.28

Copyright (C) 2006 bubble2k

This program is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License as 
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
GNU General Public License for more details.
-------------------------------------------------------------------
*/

@-------------------------------------------------------------------------
@ reg Aliases
@-------------------------------------------------------------------------
    EmuDecoder  .req    r3                  @ decoder table
    SnesMemMap  .req    r4                  @ memory mapper
    SnesD       .req    r5                  @ $DDDD--DB (direct page, shared with DBR)
    SnesDBR     .req    r5                  @ $DDDD--DB (data bank reg, shared with D)
    SnesNZ      .req    r6                  @ $---NNZZZ (15,16 N flag) (idea from SNES Advance)
    SnesSP      .req    r7                  @ $SSSS--PB (stack, shared with PBR)
    SnesPBR     .req    r7                  @ $SSSS--PB (prog bank reg, shared with SP)
    SnesMXDI    .req    r8                  @ $CCCCWEEF (EE = -e00mxdi flags)
    SnesCV      .req    r8                  @ $CCCCWEEF (bit 1=carry, bit 0=overflow)
    SnesCYCLES  .req    r8                  @ $CCCCWEEF (CCCCC = CPU cycles (-1364 to 0)?)
    SnesWAI     .req    r8                  @ $CCCCWEEF (W bit 0 = in WAI mode)
    SnesPC      .req    r9                  @ $-PPPPPPP (SNES PC translated to the GBA address)
    SnesX       .req    r10                 @ $----XXXX (X register)
    SnesY       .req    r11                 @ $----YYYY (Y register)
    SnesA       .req    r12                 @ $AAAA---- (m=0)   $AA------ (m=1)

@-------------------------------------------------------------------------
@ Timing counters 
@-------------------------------------------------------------------------
    .equ        CYCLE_SHIFT,                14
    .equ        SCANLINE_SHIFT,             20

    .equ        NUM_SCANLINES,              262
    
    @ version 0.27 fix

    .equ        CYCLES_PER_SCANLINE,        226
    .equ        CYCLES_HBLANK,              182
    
    .equ        CYCLES_PER_SCANLINE_FAST,   304     @ whats the basis for this? *shrug*
    .equ        CYCLES_HBLANK_FAST,         244
    @ version 0.27 fix end


@-------------------------------------------------------------------------
@ Interrupt Vectors
@-------------------------------------------------------------------------
    .equ        VECTOR_COUNTRY, 0x0000FFD9
    .equ        VECTOR_IRQ,     0x0000FFEE
    .equ        VECTOR_RESET,   0x0000FFFC
    .equ        VECTOR_NMI,     0x0000FFEA
    .equ        VECTOR_ABORT,   0x0000FFE8
    .equ        VECTOR_BRK,     0x0000FFE6
    .equ        VECTOR_COP,     0x0000FFE4

    .equ        VECTOR_NMI2,    0x0203FFEA


@-------------------------------------------------------------------------
@ flags (equivalent to the GBA flags)
@-------------------------------------------------------------------------
    .equ        SnesFlagC,  0x00000002
    .equ        SnesFlagV,  0x00000001
    .equ        SnesFlagWAI,0x00001000
	.equ		SnesFlagE,  0x00000400
	.equ		SnesFlagM,  0x00000080
	.equ		SnesFlagX,  0x00000040
	.equ		SnesFlagD,  0x00000020
	.equ		SnesFlagI,  0x00000010

    
    .equ        SnesFlagN,  0x00018000
    .equ        SnesFlagNH, 0x00010000
    .equ        SnesFlagNL, 0x00008000
    .equ        SnesFlagZ,  0xFFFFFFFF
	

@-------------------------------------------------------------------------
@ true SNES program register flags
@-------------------------------------------------------------------------
    .equ        SnesP_E,    0x100
    .equ        SnesP_N,    0x80
    .equ        SnesP_V,    0x40
    .equ        SnesP_M,    0x20
    .equ        SnesP_X,    0x10
    .equ        SnesP_D,    0x08
    .equ        SnesP_I,    0x04
    .equ        SnesP_Z,    0x02
    .equ        SnesP_C,    0x01

.macro Debug1
.ifeq   debug-1
    stmfd	sp!, {r3}
    ldr		r3, =AsmDebug1
    stmia	r3, {r0-r2,SnesPC}
    ldmfd	sp!, {r3}
.endif
.endm
.macro Debug2
.ifeq   debug-1
    stmfd	sp!, {r3}
    ldr		r3, =AsmDebug2
    stmia	r3, {r0-r2,SnesPC}
    ldmfd	sp!, {r3}
.endif    
.endm
.macro Debug3
.ifeq   debug-1
    stmfd	sp!, {r3}
    ldr		r3, =AsmDebug3
    stmia	r3, {r0-r2,SnesPC}
    ldmfd	sp!, {r3}
.endif
.endm


@=========================================================================
@ Translate SNES Address to effective GBA Address
@   r0: SNES Address
@
@ returns
@   r0: Effective GBA Address (if address is an IO port, most significant
@       bit of r0 will be set)
@
@ (can this go any any faster??? will sleep over it.)
@=========================================================================

.macro  SetWrite
    .equ    iWrite, 1
.endm
.macro  SetRead
    .equ    iWrite, 0
.endm

.macro TranslateAddress reload=1
    @ldr     r2, =MemoryMap          @ 1 cycles
    .ifeq   \reload-1
        ldr     SnesMemMap, =MemoryMap
    .endif
     .ifeq   iWrite-1
        ldr     SnesMemMap, =MemoryWriteMap
    .endif   
    mov     r1, r0, lsr #13         @ 1 cycles
    ldr     r2, [SnesMemMap, r1, lsl #2]    @ 6 cycles
    add     r0, r0, r2              @ 1 cycles
    .ifeq   iWrite-1
        ldr     SnesMemMap, =MemoryMap
    .endif    
.endm

/*
.macro TranslateAddressDMA targetReg=r0
    ldr     r6, =MemoryMap
    mov     r1, r0, lsr #13
    ldr     r6, [r6, r1, lsl #2]
    add     \targetReg, r0, r6
.endm
*/


.macro TranslateAddressFromMapCache
    ldr     r2, MapCacheOffset
    mov     r1, r0, lsr #13
    ldr     r2, [r2, r1, lsl #2]
    add     r0, r0, r2
.endm

.macro TranslateAddressFromDPCache
    ldr     r2, =DPCache
    mov     r1, r0, lsr #13
    ldr     r2, [r2, r1, lsl #2]
    add     r0, r0, r2
.endm


.macro CacheMemoryMap
/*
	stmfd sp!, {r3}

    and     r0, SnesDBR, #0xff
    add     r1, SnesMemMap, r0, lsl #5		@ Source address
    ldr     r2, =MapCache			@ Dest address

    sub     r3, r2, r0, lsl #5
    ldr     r0, =MapCacheOffset     @ save the offset
    str     r3, [r0]

	mov r3, #8						@ Copy 16 words
1:
	ldr r0, [r1], #4
	str r0, [r2], #4
	ldr r0, [r1], #4
	str r0, [r2], #4
	subs r3, r3, #1
	bne 1b

	ldmfd sp!, {r3}
*/	
.endm


@=========================================================================
@ Read address operands at program counter.
@   SnesPC: current program counter
@ returns
@   r0:     address
@=========================================================================
.macro ReadAddrOperand8             @ (3 cycles)
    ldrb    r0, [SnesPC], #1        @ 3
.endm

.macro ReadAddrOperand16            @ (6 cycles)
    ldrb    r0, [SnesPC], #1        @ 3
    ldrb    r2, [SnesPC], #1        @ 2
    add     r0, r0, r2, lsl #8      @ 1
.endm

.macro ReadAddrOperand24            @ (9 cycles)
    ldrb    r0, [SnesPC], #1        @ 3
    ldrb    r2, [SnesPC], #1        @ 2
    add     r0, r0, r2, lsl #8      @ 1
    ldrb    r2, [SnesPC], #1        @ 2
    add     r0, r0, r2, lsl #16     @ 1
.endm

@=========================================================================
@ Read data operands at program counter.
@   SnesPC: current program counter
@ returns
@   r1:     data
@=========================================================================
.macro ReadDataOperand 
    .ifeq mBit-8
        ReadDataOperand8 \index
    .else
        ReadDataOperand16 \index
    .endif
.endm

.macro ReadDataOperand8 index=1                 @ (3 cycles)
    ldrb    r1, [SnesPC], #1
.endm

.macro ReadDataOperand16 index=1                @ (7 cycles)
    ldrb    r1, [SnesPC], #1
    ldrb    r2, [SnesPC], #1
    add     r1, r1, r2, lsl #8
.endm


@=========================================================================
@ Read 8 bits from effective GBA address
@   r0: Effective GBA Address
@ returns
@   r0: addr read
@=========================================================================
.macro ReadAddr8 index=0
    ldrb    r0,[r0, #(\index)]
.endm

.macro ReadAddr16 index=0
    ldrb    r2,[r0, #(\index+1)]
    ldrb    r0,[r0, #(\index)]
    add     r0, r0, r2, lsl #8
.endm

.macro ReadAddr24 index=0
    ldrb    r2,[r0, #(\index+2)]
    ldrb    r1,[r0, #(\index+1)]
    ldrb    r0,[r0, #(\index)]
    add     r0, r0, r1, lsl #8
    add     r0, r0, r2, lsl #16
.endm


@=========================================================================
@ Write 8 bits to effective GBA address
@   r0: Effective GBA Address
@       (if r0 is negative, call IO routine)
@   r1: data to be written 
@=========================================================================
.macro WriteAddr8 index=0
    strb    r1,[r0, #(\index)]
.endm

.macro WriteAddr16 index=0
    strb    r1,[r0, #(\index)]
    mov     r1, r1, lsr #8
    strb    r1,[r0, #(\index+1)]
.endm

.macro WriteAddr24 index=0
    strb    r1,[r0, #(\index)]
    mov     r1, r1, lsr #8
    strb    r1,[r0, #(\index+1)]
    mov     r1, r1, lsr #8
    strb    r1,[r0, #(\index+2)]
.endm


@=========================================================================
@ Read 8 bits from effective GBA address
@   r0: Effective GBA Address
@       (if r0 is negative, call IO routine)
@ returns
@   r1: data read
@=========================================================================
.macro ReadData index=0
    .ifeq mBit-8
        ReadData8 \index
    .else
        ReadData16 \index
    .endif
.endm

.macro ReadDataXY index=0
    .ifeq xBit-8
        ReadData8 \index
    .else
        ReadData16 \index
    .endif
.endm

.macro TestReadIO
    tst     r0, #0x80000000
.endm

.macro ReadIO8
    blne    IORead8
    bne		2f  
.endm

.macro ReadIO16
    blne	IORead16
    bne		2f
.endm


.macro ReadData8 index=0
    .ifeq   immMode-0
        TestReadIO
        ReadIO8
1:        
        ldrb    r1,[r0, #(\index)] @ SNEMULDS
2:        
    .else
        ldrb    r1,[SnesPC], #1
    .endif
.endm

.macro ReadData16 index=0
    .ifeq   immMode-0
        TestReadIO
        ReadIO16
1:        
        ldrb    r1,[r0, #(\index)]
        ldrb    r2,[r0, #(\index+1)]
        add     r1, r1, r2, lsl #8
2:        

    .else
        ldrb    r1,[SnesPC], #1
        ldrb    r2,[SnesPC], #1
        add     r1, r1, r2, lsl #8
    .endif
.endm 


@=========================================================================
@ Write 8 bits to effective GBA address
@   r0: Effective GBA Address
@       (if r0 is negative, call IO routine)
@   r1: data to be written 
@=========================================================================
.macro WriteData index=0
    .ifeq mBit-8
        WriteData8 \index
    .else
        WriteData16 \index
    .endif
.endm

.macro WriteDataXY index=0
    .ifeq xBit-8
        WriteData8 \index
    .else
        WriteData16 \index
    .endif
.endm

.macro TestWriteIO
    tst     r0, #0x80000000
.endm

.macro WriteIO8
    blne    IOWrite8
    bne		2f
.endm

.macro WriteIO16
    blne    IOWrite16
    bne		2f
.endm

.macro WriteData8 index=0
    TestWriteIO
    WriteIO8
1:
    strb    r1,[r0, #(\index)]
2:
.endm

.macro WriteData16 index=0
    TestWriteIO
    WriteIO16
1:
    strb    r1,[r0, #(\index)]
    mov     r1, r1, lsr #8
    strb    r1,[r0, #(\index+1)]
2:
.endm

.macro WriteData24 index=0
    strb    r1,[r0, #(\index)]
    mov     r1, r1, lsr #8
    strb    r1,[r0, #(\index+1)]
    mov     r1, r1, lsr #8
    strb    r1,[r0, #(\index+2)]
.endm


@=========================================================================
@ New Addressing Mode Macros:
@ returns:
@   r0 - SNES Address
@=========================================================================

@-------------------------------------------------------------------------
@ Data Bank Register
@-------------------------------------------------------------------------
.macro AddDBR
    add     r0, r0, SnesDBR, lsl #16
.endm


@-------------------------------------------------------------------------
@ Data Bank Register
@-------------------------------------------------------------------------
.macro AddPBR
    add     r0, r0, SnesPBR, lsl #16
.endm


@-------------------------------------------------------------------------
@ Immediate #1234
@-------------------------------------------------------------------------
.macro  Immediate
    @add     r0, SnesPC, #1
.endm


@-------------------------------------------------------------------------
@ Indirect ($1234)
@-------------------------------------------------------------------------
.macro  Indirect
	SetRead
    TranslateAddress    0
/*    tsts		r0, #0x80000000						@ reload
    blne		MemReload2*/   
    ReadAddr16
    AddDBR
.endm

@-------------------------------------------------------------------------
@ Indirect ($1234)
@-------------------------------------------------------------------------
.macro  IndirectPBR
	SetRead
    TranslateAddress    0
/*    tsts		r0, #0x80000000						@ reload
    blne		MemReload2*/    
    ReadAddr16
    AddPBR
.endm

@-------------------------------------------------------------------------
@ Indirect [$1234]
@-------------------------------------------------------------------------
.macro  IndirectLong
	SetRead
    TranslateAddress    0
/*    tsts		r0, #0x80000000						@ reload
    blne		MemReload2*/  
    ReadAddr24
.endm


@-------------------------------------------------------------------------
@ X Index
@-------------------------------------------------------------------------
.macro IndexedX
    mov     r2, SnesX, lsl #16
    add     r0, r0, r2, lsr #16
.endm


@-------------------------------------------------------------------------
@ Y Index
@-------------------------------------------------------------------------
.macro IndexedY
    mov     r2, SnesY, lsl #16
    add     r0, r0, r2, lsr #16
.endm


@-------------------------------------------------------------------------
@ Absolute Addr: $1234
@   Addr:       LLLLLLLLHHHHHHHH
@   Effective:  00000000HHHHHHHHLLLLLLLL
@-------------------------------------------------------------------------
.macro AbsoluteBankZero
    ReadAddrOperand16
.endm

@-------------------------------------------------------------------------
@ Absolute Addr: $1234
@   Addr:       LLLLLLLLHHHHHHHH
@   Effective:  DBRxxxxxHHHHHHHHLLLLLLLL
@-------------------------------------------------------------------------
.macro Absolute
    ReadAddrOperand16
    AddDBR
.endm

@-------------------------------------------------------------------------
@ Absolute Addr: $1234
@   Addr:       LLLLLLLLHHHHHHHH
@   Effective:  PBRxxxxxHHHHHHHHLLLLLLLL
@-------------------------------------------------------------------------
.macro AbsolutePC
    ReadAddrOperand16
    AddPBR
.endm

@-------------------------------------------------------------------------
@ Absolute Addr: ($1234)
@   Addr:       LLLLLLLLHHHHHHHH
@   Effective:  PBRxxxxxIIIIIIIIIIIIIIII = [00000000HHHHHHHHLLLLLLLL]
@-------------------------------------------------------------------------
.macro AbsoluteIndirectPC
    AbsoluteBankZero
    IndirectPBR
.endm

@-------------------------------------------------------------------------
@ Absolute Addr: [$1234]
@   Addr:       LLLLLLLLHHHHHHHH
@   Effective:  IIIIIIIIIIIIIIIIIIIIIIII = [DBRxxxxxHHHHHHHHLLLLLLLL]
@-------------------------------------------------------------------------
.macro AbsoluteIndirectLong
    Absolute
    IndirectLong
.endm

@-------------------------------------------------------------------------
@ Absolute Addr: [$1234]
@   Addr:       LLLLLLLLHHHHHHHH
@   Effective:  IIIIIIIIIIIIIIIIIIIIIIII = [DBRxxxxxHHHHHHHHLLLLLLLL]
@-------------------------------------------------------------------------
.macro AbsoluteIndirectLongPC
    AbsoluteBankZero
    IndirectLong
.endm

@-------------------------------------------------------------------------
@ Absolute Indexed X Addr: $1234,X
@   Addr:       LLLLLLLLHHHHHHHH
@   Effective:  DBRxxxxxHHHHHHHHLLLLLLLL
@              +        XXXXXXXXXXXXXXXX
@-------------------------------------------------------------------------
.macro AbsoluteIndexedX
    Absolute
    IndexedX
.endm

@-------------------------------------------------------------------------
@ Absolute Indexed X Addr: ($1234,X)
@   Addr:       LLLLLLLLHHHHHHHH
@   Effective:  PBRxxxxxIIIIIIIIIIIIIIII = [PBRxxxxxHHHHHHHHLLLLLLLL+XXXXXXXXXXXXXXXX]
@-------------------------------------------------------------------------
.macro AbsoluteIndexedXIndirectPC
    AbsolutePC
    IndexedX
    IndirectPBR
.endm

@-------------------------------------------------------------------------
@ Absolute Indexed Y Addr: $1234,Y
@   Addr:       LLLLLLLLHHHHHHHH
@   Effective:  DBRxxxxxHHHHHHHHLLLLLLLL
@              +        YYYYYYYYYYYYYYYY
@-------------------------------------------------------------------------
.macro AbsoluteIndexedY 
    Absolute
    IndexedY
.endm

@-------------------------------------------------------------------------
@ Absolute Long Addr: $123456
@   Addr:       LLLLLLLLHHHHHHHHBBBBBBBB
@   Effective:  BBBBBBBBHHHHHHHHLLLLLLLL
@-------------------------------------------------------------------------
.macro AbsoluteLong 
    ReadAddrOperand24
.endm

@-------------------------------------------------------------------------
@ Absolute Long Addr: $123456
@   Addr:       LLLLLLLLHHHHHHHHBBBBBBBB
@   Effective:  BBBBBBBBHHHHHHHHLLLLLLLL
@-------------------------------------------------------------------------
.macro AbsoluteLongPC 
    ReadAddrOperand24
.endm

@-------------------------------------------------------------------------
@ Absolute Long X Addr: $123456,X
@   Addr:       LLLLLLLLHHHHHHHHBBBBBBBB
@   Effective:  BBBBBBBBHHHHHHHHLLLLLLLL
@              +        XXXXXXXXXXXXXXXX
@-------------------------------------------------------------------------
.macro AbsoluteLongIndexedX
    AbsoluteLong
    IndexedX
.endm


@-------------------------------------------------------------------------
@ Direct Page Addr: $12
@   Addr:       LLLLLLLL
@   Effective:  00000000DDDDDDDDDDDDDDDD
@              +                LLLLLLLL
@-------------------------------------------------------------------------
.macro DP
    ReadAddrOperand8
    add     r0, r0, SnesD, lsr #16
.endm

@-------------------------------------------------------------------------
@ Direct Page X Addr: $12,X
@   Addr:       LLLLLLLL
@   Effective:  00000000DDDDDDDDDDDDDDDD
@              +                LLLLLLLL
@              +        XXXXXXXXXXXXXXXX
@-------------------------------------------------------------------------
.macro DPIndexedX 
    DP
    IndexedX
.endm


@-------------------------------------------------------------------------
@ Direct Page Y Addr: $12,X
@   Addr:       LLLLLLLL
@   Effective:  00000000DDDDDDDDDDDDDDDD
@              +                LLLLLLLL
@              +        YYYYYYYYYYYYYYYY
@-------------------------------------------------------------------------
.macro DPIndexedY 
    DP
    IndexedY
.endm


@-------------------------------------------------------------------------
@ Direct Page Indirect Addr: ($12)
@   Addr:       LLLLLLLL
@   Effective:  DBRxxxxxIIIIIIIIIIIIIIII
@   where               IIIIIIIIIIIIIIII = [DDDDDDDDDDDDDDDD+LLLLLLLL]
@-------------------------------------------------------------------------
.macro DPIndirect
    DP
    Indirect
.endm


@-------------------------------------------------------------------------
@ Direct Page Indirect X Addr: ($12,X)
@   Addr:       LLLLLLLL
@   Effective:  DBRxxxxxIIIIIIIIIIIIIIII
@   where               IIIIIIIIIIIIIIII = [DDDDDDDDDDDDDDDD+LLLLLLLL+XXXXXXXX]
@-------------------------------------------------------------------------
.macro DPIndexedXIndirect
    DPIndexedX
    Indirect
.endm


@-------------------------------------------------------------------------
@ Direct Page Indirect Y Addr: ($12),Y
@   Addr:       LLLLLLLL
@   Effective:  DBRxxxxxIIIIIIIIIIIIIIII
@              +        YYYYYYYYYYYYYYYY
@
@   where               IIIIIIIIIIIIIIII = [DDDDDDDDDDDDDDDD+LLLLLLLL]
@-------------------------------------------------------------------------
.macro DPIndirectIndexedY 
    DPIndirect
    IndexedY
.endm


@-------------------------------------------------------------------------
@ Direct Page Indirect Long Addr: [$12]
@   Addr:       LLLLLLLL
@   Effective:  IIIIIIIIIIIIIIIIIIIIIIII
@
@   where       IIIIIIIIIIIIIIIIIIIIIIII = [DDDDDDDDDDDDDDDD+LLLLLLLL]
@-------------------------------------------------------------------------
.macro DPIndirectLong
    DP
    IndirectLong
.endm

@-------------------------------------------------------------------------
@ Direct Page Indirect Long Addr: [$12], Y
@   Addr:       LLLLLLLL
@   Effective:  IIIIIIIIIIIIIIIIIIIIIIII
@              +        YYYYYYYYYYYYYYYY
@
@   where       IIIIIIIIIIIIIIIIIIIIIIII = [DDDDDDDDDDDDDDDD+LLLLLLLL]
@-------------------------------------------------------------------------
.macro DPIndirectLongIndexedY
    DP
    IndirectLong    
    IndexedY   
.endm

@-------------------------------------------------------------------------
@ Stack Relative: $12,S
@   Addr:       LLLLLLLL
@   Effective:  00000000SSSSSSSSSSSSSSSS
@              +                LLLLLLLL 
@-------------------------------------------------------------------------
.macro StackRelative
    ldrb    r1, [SnesPC], #1
    add     r0, r1, SnesSP, lsr #16
.endm

@-------------------------------------------------------------------------
@ Stack Relative: ($12,S),Y
@   Addr:       LLLLLLLL
@   Effective:  DBRxxxxxIIIIIIIIIIIIIIII
@               +       YYYYYYYYYYYYYYYY
@
@   where       IIIIIIIIIIIIIIII = [SSSSSSSSSSSSSSSSS+LLLLLLLL]
@-------------------------------------------------------------------------
.macro StackRelativeIndirectIndexedY 
    StackRelative 
    Indirect
    IndexedY
.endm


@=========================================================================
@ Used by the opcodes to specify addressing mode and translate the 
@ SNES addresss to a GBA address.
@=========================================================================
.macro  Translate addrMode, pcinc=0, cycles=0, startExecute=1
    .ifnc \addrMode, None
    .ifnc \addrMode, NonePC
        \addrMode

@		TranslateAddress    0            
  
       
        .ifc \addrMode, Absolute
            @TranslateAddressFromMapCache
            TranslateAddress	0
        .endif
        .ifc \addrMode, AbsoluteIndexedX
            @TranslateAddressFromMapCache
            TranslateAddress	0
        .endif
        .ifc \addrMode, AbsoluteIndexedY
            @TranslateAddressFromMapCache
            TranslateAddress	0
        .endif
        
        .ifc \addrMode, AbsoluteLong
            TranslateAddress    0
        .endif
        .ifc \addrMode, AbsoluteLongIndexedX
            TranslateAddress    0
        .endif
        
        .ifc \addrMode, DP
            @TranslateAddressFromDPCache
            TranslateAddress	0            
        .endif
        .ifc \addrMode, DPIndexedX
            @TranslateAddressFromDPCache
            TranslateAddress	0            
        .endif
        .ifc \addrMode, DPIndexedY
            @TranslateAddressFromDPCache
            TranslateAddress	0            
        .endif

        .ifc \addrMode, DPIndirect
            TranslateAddress    0
        .endif
        .ifc \addrMode, DPIndirectLong
            TranslateAddress    0
        .endif
        .ifc \addrMode, DPIndirectLongIndexedY
            TranslateAddress    0
        .endif
        .ifc \addrMode, DPIndirectIndexedY
            TranslateAddress    0
        .endif
        .ifc \addrMode, DPIndexedXIndirect
            TranslateAddress    0
        .endif
        .ifc \addrMode, StackRelative
            TranslateAddress    0
        .endif
        .ifc \addrMode, StackRelativeIndirectIndexedY
            TranslateAddress    0
        .endif

       
    .endif
    .endif

    AddPCNoJump   \pcinc, \cycles
    
    .ifeq \startExecute-1
        StartExecute
    .endif
.endm


@-------------------------------------------------------------------------
@ Translate the SNES PC to GBA PC and saves it.
@   r0: New SNES PC
@-------------------------------------------------------------------------
.macro  TranslateAndSavePC  fast = 0
    bic     SnesPBR, SnesPBR, #0xFF 
    orr     SnesPBR, SnesPBR, r0, lsr #16       @ save PBR
    .ifeq   \fast-0
        ldr     r1,=SnesPCOffset        
        str     r0,[r1]                             @ r0 = the actual SNES PC
    .else
        str     r0, SnesPCOffset                    @ r0 = the actual SNES PC
    .endif
    SetRead
    TranslateAddress        0
    tsts		r0, #0x80000000						@ reload
    blne		MemReloadPC
    
    Debug2
    
    .ifeq   \fast-0
        ldr     r1,=SnesPCOffset                    @ r0 = the translated GBA PC, 
        ldr     r2,[r1]                             @ r2 = actual SNES PC
    .else
        ldr     r2, SnesPCOffset
    .endif
    sub     r2, r2, r0
    .ifeq   \fast-0
        str     r2,[r1]                             @ save (SNES PC - GBA PC) 
    .else
        str     r2, SnesPCOffset
    .endif
    
    mov     SnesPC, r0
.endm

@=========================================================================
@ Arithmetic Instructions
@=========================================================================

/*
.macro OpADCD mode, pcinc, cycles
    
    tsts    SnesMXDI, #SnesFlagM
    bne     ADCD_m1

ADCD_m0:    @ r1 = 0000XXxx, SnesA = AAaa0000
    @ 16-bit add
    tsts    SnesCV, #SnesFlagC

    and     r0, r1, #0xff                       @ r0 = 000000xx
    addne   r0, r0, #1                          @ r0 = r0 + 1 (if carry flag set)
    add     r0, r0, SnesA, lsr #8               @ r0 = 00AAaaxx
    bic     r0, r0, #0x00ff0000                 @ r0 = 0000aaxx
    ldr     r2, =decimalAdd
    add     r2, r2, r0, lsl #1  
    ldrh    r0, [r2]                            @ r0 = 00000??? (aa+xx)

    mov     r2, r0, lsr #8
    bic     r0, r0, #0xf00
    add     r1, r1, r2, lsl #8
    and     r1, r1, #0x0000ff00                 @ r1 = 0000XX00
    add     r1, r1, SnesA, lsr #24              @ r1 = 0000XXAA
    ldr     r2, =decimalAdd
    add     r2, r2, r1, lsl #1
    ldrh    r1, [r2]                            @ r1 = 00000??? (XX+AA)
    add     r1, r0, r1, lsl #8
    
    
    movs    SnesA, r1, lsl #16
    
    mov     SnesNZ, SnesA, lsr #16
    orrcs   SnesCV, SnesCV, #SnesFlagC
    biccc   SnesCV, SnesCV, #SnesFlagC
    
    AddPC   \pcinc, \cycles

ADCD_m1:
    @ 8-bit add
    tsts    SnesCV, #SnesFlagC
    add     r1, r1, SnesA, lsr #16
    addne   r1, r1, #1                          @ r0 = r0 + 1 (if carry flag set)
    ldr     r2, =decimalAdd
    add     r2, r2, r1, lsl #1
    ldrh    r2, [r2]
    movs    SnesA, r2, lsl #24

    mov     SnesNZ, SnesA, lsr #16
    orrcs   SnesCV, SnesCV, #SnesFlagC
    biccc   SnesCV, SnesCV, #SnesFlagC

    AddPC   0, 0
.endm
*/

.macro OpADCD mode, pcinc, cycles
    
    tsts    SnesMXDI, #SnesFlagM
    bne     ADCD_m1

ADCD_m0:    @ r1 = 0000XXxx, SnesA = AAaa0000
    @ 16-bit add
    
    @ Took from SnesAdvance    
    movs    r0, SnesCV, lsr #2      @ get carry flag
    
    stmfd	sp!, {r3}
    
    mov		r0, r1, lsr #8			@ r0 = high byte
    and		r1,	r1, #0xFF			@ r1 = low byte
    
	eor		r0, r0, #0xFF			
	eor 	r1,	r1, #0xFF

	mov 	r2, SnesA, lsl#12
	sbcs 	r3,	r2,	r1,	lsl#28
	and 	r3,	r3,	#0xf0000000
	cmncc 	r3,#0x60000000
	addcs 	r3, r3, #0x60000000

	mov 	r2,	SnesA, lsr#20
	and 	r2, r2, #0xF
	sbcs	r1, r2, r1, lsr#4
	mov 	r1,	r1,	lsl#28
	cmncc 	r1, #0x60000000
	addcs	r1,	r1, #0x60000000
	orr 	r3, r1, r3, lsr#4

	mov 	r2,	SnesA, lsl#4
	and 	r2, r2, #0xF0000000
	sbcs 	r1, r2, r0, lsl#28
	and 	r1, r1, #0xf0000000
	cmncc  	r1, #0x60000000
	addcs 	r1, r1, #0x60000000
	orr 	r3, r1, r3,lsr#4

	mov 	r2, SnesA, lsr#28
	sbcs 	r1, r2, r0, lsr#4
	mov 	r1, r1, lsl#28
	cmncc 	r1, #0x60000000
	addcs 	r1, r1, #0x60000000
	orr		SnesA, r1, r3,lsr#4    

    mov     SnesNZ, SnesA, lsr #16
    orrcs   SnesCV, SnesCV, #SnesFlagC
    biccc   SnesCV, SnesCV, #SnesFlagC
    
    ldmfd	sp!, {r3}
    
    AddPC   \pcinc, \cycles

ADCD_m1: @ Took from SnesAdvance
    @ 8-bit add
    movs    r0, SnesCV, lsr #2      @ get carry flag
    
    and		r0, r1, #0xf			@ r0 = W1 & 0xF
    subcs	r0, r0, #0x10			@ Pour Propagation de la carry 0xX -> 0xFFFFFFFX 
    mov		r0, r0, ror #4			@ r0 = W1 (<<24)  0xFFFFFFFX -> 0xXFFFFFFF
    adcs	r0, r0, SnesA, lsl #4   @ r0 = W1 + A + C (<<24)
    cmncc	r0, #0x60000000			@ if X >= 10
    addcs	r0, r0, #0x60000000		@ r0 += 6
    
    mov		r2, SnesA, lsr#28
    adc		r1, r2, r1, lsr #4
    movs	r1, r1, lsl #28
    cmncc	r1, #0x60000000
    addcs	r1, r1, #0x60000000
    orr		SnesA, r1, r0, lsr#4
    
    mov     SnesNZ, SnesA, lsr #16
    orrcs   SnesCV, SnesCV, #SnesFlagC
    biccc   SnesCV, SnesCV, #SnesFlagC

    AddPC   0, 0
.endm

.macro OpADC mode, pcinc, cycles
    \mode
    ReadData

    AddPCNoJump   \pcinc, \cycles

    tsts    SnesMXDI, #SnesFlagD
    bne     OpADCDCode

    @ elegant ADC from Snes Advance
    @
    movs    r0, SnesCV, lsr #2      @ get carry flag
    subcs   r1, r1, #(1<<mBit)
    adcs    SnesA, SnesA, r1, ror #mBit
    bic     SnesCV, SnesCV, #(SnesFlagC+SnesFlagV)
    orrcs   SnesCV, SnesCV, #SnesFlagC
    orrvs   SnesCV, SnesCV, #SnesFlagV
    mov     SnesNZ, SnesA, lsr #16
    
    AddPC   0, 0
.endm

.macro OpAND mode, pcinc, cycles
    \mode
    ReadData

    ands    SnesA, SnesA, r1, ror #mBit
    mov     SnesNZ, SnesA, lsr #16

    AddPC   \pcinc, \cycles
.endm

.macro OpASLA mode, pcinc, cycles
    movs    SnesA, SnesA, lsl #1
    biccc   SnesCV, SnesCV, #SnesFlagC
    orrcs   SnesCV, SnesCV, #SnesFlagC
    mov     SnesNZ, SnesA, lsr #16

    AddPC   \pcinc, \cycles
.endm

.macro OpASL mode, pcinc, cycles
    \mode
    stmfd	sp!, {r0}
    ReadData

    movs    r1, r1, lsl #(32-mBit+1)
    biccc   SnesCV, SnesCV, #SnesFlagC
    orrcs   SnesCV, SnesCV, #SnesFlagC
    mov     r1, r1, lsr #(32-mBit)
    mov     SnesNZ, r1, lsl #(16-mBit)

	ldmfd	sp!, {r0}
    WriteData

    AddPC   \pcinc, \cycles
.endm

.macro OpBIT mode, pcinc, cycles
    \mode
    ReadData

    @ update N/V flag only if we are not in immediate mode
    .ifeq   immMode-0
        mov     SnesNZ, #0
    
        tsts    SnesA, r1, ror #mBit
        orrne   SnesNZ, SnesNZ, #0x1

        tsts    r1, #(1 << (mBit-1))
        orrne   SnesNZ, SnesNZ, #SnesFlagNH
        
        tsts    r1, #(1 << (mBit-2))
        biceq   SnesCV, SnesCV, #SnesFlagV
        orrne   SnesCV, SnesCV, #SnesFlagV
    .else
        and     r0, SnesNZ, #SnesFlagNL
        orr     SnesNZ, SnesNZ, r0, lsl #1
        
        tsts    SnesA, r1, ror #mBit
        biceq   SnesNZ, SnesNZ, #0x00ff
        biceq   SnesNZ, SnesNZ, #0xff00
        orrne   SnesNZ, SnesNZ, #0x1
    .endif

    AddPC   \pcinc, \cycles
.endm

.macro OpCMP mode, pcinc, cycles
    \mode
    ReadData

    subs    SnesNZ, SnesA, r1, ror #mBit  
    mov     SnesNZ, SnesNZ, lsr #16
    biccc   SnesCV, SnesCV, #SnesFlagC
    orrcs   SnesCV, SnesCV, #SnesFlagC

    AddPC   \pcinc, \cycles
.endm

.macro OpCMPXY regToCompare, mode, pcinc, cycles
	
    \mode
    ReadDataXY
    
    mov     r2, \regToCompare, lsl #(32-xBit)       @ r2 = xxxxxxxx 00000000 00000000 00000000
    subs    SnesNZ, r2, r1, lsl #(32-xBit)          @ r1 = rrrrrrrr 00000000 00000000 00000000, SnesNZ = r2-r1
    mov     SnesNZ, SnesNZ, lsr #16

    biccc   SnesCV, SnesCV, #SnesFlagC
    orrcs   SnesCV, SnesCV, #SnesFlagC

    AddPC   \pcinc, \cycles
.endm

.macro OpCPX mode, pcinc, cycles
    OpCMPXY SnesX, \mode, \pcinc, \cycles
.endm

.macro OpCPY mode, pcinc, cycles
    OpCMPXY SnesY, \mode, \pcinc, \cycles
.endm

.macro OpDEC mode, pcinc, cycles
    \mode
    stmfd	sp!, {r0}		@ archeide
    ReadData

    subs    r1, r1, #1
    
    @ version 0.23 fix
    @
    mov     SnesNZ, r1, ror #mBit
    mov     SnesNZ, SnesNZ, lsr #16
    ldmfd	sp!, {r0}		@ archeide
    WriteData

    AddPC   \pcinc, \cycles
.endm

.macro OpDEA mode, pcinc, cycles
    \mode
    subs    SnesA, SnesA, #(1 << (32-mBit))
    mov     SnesNZ, SnesA, lsr #16

    AddPC   \pcinc, \cycles
.endm

.macro OpDEXY snesReg
    orr     \snesReg, \snesReg, #0x00010000
    subs    \snesReg, \snesReg, #1
    .ifeq xBit-8
        bic     \snesReg, \snesReg, #0x0000ff00
    .endif
    bic     \snesReg, \snesReg, #0x000f0000
    mov     SnesNZ, \snesReg, lsl #(16-xBit)
.endm

.macro OpDEX mode, pcinc, cycles
    \mode
    OpDEXY  SnesX
    AddPC   \pcinc, \cycles
.endm

.macro OpDEY mode, pcinc, cycles
    \mode
    OpDEXY  SnesY
    AddPC   \pcinc, \cycles
.endm

.macro OpEOR mode, pcinc, cycles
    \mode
    ReadData

    eors    SnesA, SnesA, r1, ror #mBit
    mov     SnesNZ, SnesA, lsr #16

    AddPC   \pcinc, \cycles
.endm

.macro OpINC mode, pcinc, cycles
    \mode
    stmfd	sp!, {r0}		@ archeide
    ReadData

    adds    r1, r1, #1
    @ version 0.23 fix
    @
    mov     SnesNZ, r1, ror #mBit
    mov     SnesNZ, SnesNZ, lsr #16

    ldmfd	sp!, {r0}		@ archeide
    WriteData

    AddPC   \pcinc, \cycles
.endm

.macro OpINA mode, pcinc, cycles
    \mode
    adds    SnesA, SnesA, #(1 << (32-mBit))
    mov     SnesNZ, SnesA, lsr #16

    AddPC   \pcinc, \cycles
.endm

.macro OpINXY snesReg
    add     \snesReg, \snesReg, #1
    .ifeq xBit-8
        bic     \snesReg, \snesReg, #0x0000ff00
    .endif
    .ifeq xBit-16
        bic     \snesReg, \snesReg, #0x000f0000
    .endif
    mov     SnesNZ, \snesReg, lsl #(16-xBit)
.endm

.macro OpINX mode, pcinc, cycles
    \mode
    OpINXY  SnesX
    AddPC   \pcinc, \cycles
.endm

.macro OpINY mode, pcinc, cycles
    \mode
    OpINXY  SnesY
    AddPC   \pcinc, \cycles
.endm


.macro OpLSRA mode, pcinc, cycles
    \mode
    movs    SnesA, SnesA, lsr #(32-mBit+1)
    biccc   SnesCV, SnesCV, #SnesFlagC
    orrcs   SnesCV, SnesCV, #SnesFlagC
    mov     SnesA, SnesA, lsl #(32-mBit)
    mov     SnesNZ, SnesA, ror #16

    AddPC   \pcinc, \cycles
.endm

.macro OpLSR mode, pcinc, cycles
    \mode
    stmfd	sp!, {r0}		@ archeide
    ReadData 

    movs    r1, r1, lsr #1
    biccc   SnesCV, SnesCV, #SnesFlagC
    orrcs   SnesCV, SnesCV, #SnesFlagC
    mov     SnesNZ, r1, lsl #(16-mBit)

    ldmfd	sp!, {r0}		@ archeide
    WriteData
    AddPC   \pcinc, \cycles
.endm

.macro OpORA mode, pcinc, cycles
    \mode
    ReadData

    orr     SnesA, SnesA, r1, lsl #(32-mBit)
    mov     SnesNZ, SnesA, lsr #16

    AddPC   \pcinc, \cycles
.endm

.macro OpROLA mode, pcinc, cycles
    \mode

    mov     r2, SnesCV
    
    movs    SnesA, SnesA, lsl #1
    biccc   SnesCV, SnesCV, #SnesFlagC
    orrcs   SnesCV, SnesCV, #SnesFlagC
    
    tsts    r2, #SnesFlagC
    orrne   SnesA, SnesA, #(1 << (32-mBit))
    mov     SnesNZ, SnesA, lsr #16

    AddPC   \pcinc, \cycles
.endm

.macro OpROL mode, pcinc, cycles
    \mode
    stmfd	sp!, {r0}		@ archeide
    ReadData 

    mov     r2, SnesCV

    movs    r1, r1, lsl #(32-mBit+1)
    biccc   SnesCV, SnesCV, #SnesFlagC
    orrcs   SnesCV, SnesCV, #SnesFlagC
    mov     r1, r1, ror #(32-mBit)

    tsts    r2, #SnesFlagC
    orrne   r1, r1, #1
    mov     SnesNZ, r1, lsl #(16-mBit)
    
    ldmfd	sp!, {r0}		@ archeide
    WriteData

    AddPC   \pcinc, \cycles
.endm

.macro OpRORA mode, pcinc, cycles
    \mode
    
    mov     r2, SnesCV

    movs    SnesA, SnesA, lsr #((32-mBit)+1)
    biccc   SnesCV, SnesCV, #SnesFlagC
    orrcs   SnesCV, SnesCV, #SnesFlagC
    mov     SnesA, SnesA, ror #mBit

    tsts    r2, #SnesFlagC
    orrne   SnesA, SnesA, #0x80000000
    mov     SnesNZ, SnesA, lsr #16

    AddPC   \pcinc, \cycles
.endm

.macro OpROR mode, pcinc, cycles
    \mode
    stmfd	sp!, {r0}		@ archeide
    ReadData 

    mov     r2, SnesCV

    movs    r1, r1, lsr #1
    biccc   SnesCV, SnesCV, #SnesFlagC
    orrcs   SnesCV, SnesCV, #SnesFlagC

    tsts    r2, #SnesFlagC
    orrne   r1, r1, #(1<<(mBit-1))
    mov     SnesNZ, r1, lsl #(16-mBit)
    
    ldmfd	sp!, {r0}		@ archeide
    WriteData

    AddPC   \pcinc, \cycles
.endm

.macro OpSBCD mode, pcinc, cycles
    tsts    SnesMXDI, #SnesFlagM
    bne     SBCD_m1

    @ another elegant solution from Snes Advance
    @ 16-bit
    @
SBCD_m0:
    @ r1 = 0000XXxx, SnesA = AAaa0000
    stmfd   sp!, {r3}
    
	movs    r0, SnesCV, lsr #2          @ get C
	
	@ version 0.27 fix
	mov     r0, r1, lsr #8              @ r0 = high
	@ version 0.27 fix end
	
	and     r1, r1, #0xff               @ r1 = low

	mov     r2, SnesA, lsl#12
	sbcs    r3, r2, r1, lsl#28
	and     r3, r3, #0xf0000000
	subcc   r3, r3, #0x60000000

	mov     r2, SnesA, lsr #20
	and     r2, r2, #0xF
	sbcs    r1, r2, r1, lsr#4
	mov     r1, r1, lsl #28
	subcc   r1, r1, #0x60000000
	orr     r3, r1, r3, lsr #4

	mov     r2, SnesA, lsl #4
	and     r2, r2, #0xF0000000
	sbcs    r1, r2, r0, lsl #28
	and     r1, r1, #0xf0000000
	subcc   r1, r1, #0x60000000
	orr     r3, r1, r3, lsr #4

	mov     r2, SnesA, lsr #28
	sbcs    r1, r2, r0, lsr #4
	mov     r1, r1, lsl #28
	subcc   r1, r1, #0x60000000
	orr     SnesA, r1, r3, lsr #4
	
	mov     SnesNZ, SnesA, lsr #16
    biccc   SnesCV, SnesCV, #SnesFlagC
    orrcs   SnesCV, SnesCV, #SnesFlagC
    
    ldmfd   sp!, {r3}
	AddPC   0, 0

    @ 8-bit
    @
SBCD_m1:
	movs    r0, SnesCV, lsr #2          @ get C
    mov     r0, r1
	
	mov     r2, SnesA, lsl #4
	sbcs    r1, r2, r0, lsl #28
	and     r1, r1, #0xf0000000
	subcc   r1, r1, #0x60000000

	mov     r2, SnesA, lsr #28
	sbcs    r0, r2, r0, lsr #4
	mov     r0, r0, lsl #28
	subcc   r0, r0, #0x60000000
	orr     SnesA, r0, r1, lsr #4
	
	mov     SnesNZ, SnesA, lsr #16
    biccc   SnesCV, SnesCV, #SnesFlagC
    orrcs   SnesCV, SnesCV, #SnesFlagC
	AddPC   0, 0
.endm

.macro OpSBC mode, pcinc, cycles
    \mode
    ReadData

    AddPCNoJump   \pcinc, \cycles

    tsts    SnesMXDI, #SnesFlagD
    bne     OpSBCDCode

    @ elegant SBC from SNES Advance
    @
    movs    r0, SnesCV, lsr #2      @ get carry flag
    sbcs    SnesA, SnesA, r1, lsl #(32-mBit)
    .ifeq   mBit-16
        bic     SnesA, SnesA, #0x000000ff
        bic     SnesA, SnesA, #0x0000ff00
    .else
        and     SnesA, SnesA, #0xff000000
    .endif
    bic     SnesCV, SnesCV, #(SnesFlagV+SnesFlagC)
    orrcs   SnesCV, SnesCV, #SnesFlagC
    orrvs   SnesCV, SnesCV, #SnesFlagV
    mov     SnesNZ, SnesA, lsr #16
    
    AddPC   0, 0
.endm

.macro OpTSB mode, pcinc, cycles
    \mode
    stmfd	sp!, {r0}		@ archeide
    ReadData

    orr     SnesNZ, SnesNZ, SnesNZ, lsl #1
    and     SnesNZ, SnesNZ, #0x00010000
    ands    r2, r1, SnesA, lsr #(32-mBit)
    orrne   SnesNZ, SnesNZ, #1
    orr     r1, r1, SnesA, lsr #(32-mBit)
    ldmfd	sp!, {r0}		@ archeide
    WriteData

    AddPC   \pcinc, \cycles
.endm

.macro OpTRB mode, pcinc, cycles
    \mode
    stmfd	sp!, {r0}		@ archeide
    ReadData

    orr     SnesNZ, SnesNZ, SnesNZ, lsl #1
    and     SnesNZ, SnesNZ, #0x00010000
    ands    r2, r1, SnesA, lsr #(32-mBit)
    orrne   SnesNZ, SnesNZ, #1
    mvn     r2, SnesA
    and     r1, r1, r2, lsr #(32-mBit)
    ldmfd	sp!, {r0}		@ archeide
    WriteData

    AddPC   \pcinc, \cycles
.endm



@=========================================================================
@ Register transfer
@=========================================================================
.macro OpTAXY regXY
    mov     \regXY, SnesA, lsr #(32-mBit)
    .ifeq mBit-8
    .ifeq xBit-16
        @ if in 8-bit accumulator mode, 
        @ and in 16-bit accumulator mode, 
        @ transfer the B byte too
        ldr r0, SnesB
        orr \regXY, \regXY, r0, lsr #16 
    .endif
    .endif
    .ifeq mBit-16
    .ifeq xBit-8
        and \regXY, \regXY, #0x00ff
    .endif
    .endif
    mov     SnesNZ, \regXY, lsl #(16-xBit)
.endm

.macro OpTXYA regXY
@ FIXME: gestion de B??
    mov     SnesA, \regXY, ror #mBit
    mov     SnesNZ, SnesA, lsr #16
.endm

.macro OpTAX mode, pcinc, cycles
    \mode
    OpTAXY  SnesX
    AddPC   \pcinc, \cycles
.endm

.macro OpTAY mode, pcinc, cycles
    \mode
    OpTAXY  SnesY
    AddPC   \pcinc, \cycles
.endm

.macro OpTXA mode, pcinc, cycles
    \mode
    OpTXYA  SnesX
    AddPC   \pcinc, \cycles
.endm

.macro OpTYA mode, pcinc, cycles
    \mode
    OpTXYA  SnesY
    AddPC   \pcinc, \cycles
.endm

.macro OpTSX mode, pcinc, cycles
	
    \mode
    mov     SnesX, SnesSP, lsr #16
    tsts    SnesMXDI, #SnesFlagX
    bicne   SnesX, SnesX, #0xff00           @ clear the high byte for 8-bit X
    movne   SnesNZ, SnesX, lsl #8           @ 8-bit
    moveq   SnesNZ, SnesX                   @ 16-bit
    AddPC   \pcinc, \cycles
.endm

.macro OpTXS mode, pcinc, cycles
	
    \mode
    and     SnesSP, SnesSP, #0x000000ff     @ retain the PBR
    orr     SnesSP, SnesSP, SnesX, ror #16
    AddPC   \pcinc, \cycles
.endm

.macro OpTXY mode, pcinc, cycles
	
    \mode
    mov     SnesY, SnesX
    mov     SnesNZ, SnesY, lsl #(16-xBit)    
    AddPC   \pcinc, \cycles
.endm

.macro OpTYX mode, pcinc, cycles
	
    \mode
    mov     SnesX, SnesY
    mov     SnesNZ, SnesX, lsl #(16-xBit)
    AddPC   \pcinc, \cycles
.endm

.macro OpTCD mode, pcinc, cycles
	
    \mode
    and     SnesD, SnesD, #0x000000ff       @ SnesD = 000000DB
    
    .ifeq   mBit-16
    @ if A=16bit
    orr       SnesD, SnesD, SnesA           @ SnesD = AAAA00DB, if A=16 bit

    .else
    @ if A=8bit
    ldr     r1, SnesB
    and     r0, r1, #0xff000000             @ r0 = BB000000
    orr     r0, r0, SnesA, lsr #8           @ r0 = BBAA0000
    orr     SnesD, SnesD, r0                @ SnesD = BBAA00DB
    .endif

    mov     SnesNZ, SnesD, lsr #16

    AddPC   \pcinc, \cycles
.endm

.macro OpTDC mode, pcinc, cycles
	
    \mode
    mov     SnesA, SnesD, lsr #16
    mov     SnesA, SnesA, ror #mBit
    mov     SnesNZ, SnesD, lsr #16

    @ version 0.21 fix
    @
    .ifeq   mBit-8
        @ store high byte into B
        @
        and     r0, SnesD, #0xff000000
        str     r0, SnesB
    .endif
    
    AddPC   \pcinc, \cycles
.endm

.macro OpTCS mode, pcinc, cycles
	
    \mode
    and     SnesSP, SnesSP, #0x000000ff

    .ifeq   mBit-16
    @ if A=16bit
    orr       SnesSP, SnesSP, SnesA           @ SnesD = AAAA00DB, if A=16 bit

    .else
    @ if A=8bit
    ldr     r1, SnesB
    and     r0, r1, #0xff000000             @ r0 = BB000000
    orr     r0, r0, SnesA, lsr #8           @ r0 = BBAA0000
    orr     SnesSP, SnesSP, r0                @ SnesD = BBAA00DB

    .endif

    AddPC   \pcinc, \cycles
.endm

.macro OpTSC mode, pcinc, cycles
	
    \mode
    mov     SnesA, SnesSP, lsr #16
    mov     SnesA, SnesA, ror #mBit
    mov     SnesNZ, SnesSP, lsr #16
    
    @ version 0.21 fix
    @
    .ifeq   mBit-8
        @ store high byte into B
        @
        and     r0, SnesSP, #0xff000000
        str     r0, SnesB
    .endif
    
    AddPC   \pcinc, \cycles
.endm

.macro OpXBA mode, pcinc, cycles
	
    \mode
    .ifeq   mBit-8
    @ if A=8bit
    ldr     r0, SnesB
    and     r0, r0, #0xff000000             @ r0    = BB000000
    and     r1, SnesA, #0xff000000          @ r1    = AA000000
    mov     SnesA, r0                       @ SnesA = BB000000
    str     r1, SnesB

    mov     SnesNZ, SnesA, lsr #16          @ set the N/Z flag with the value of A

    .else
    @ if A=16bit
    and     r0, SnesA, #0xff000000          @ r0    = BB000000
    add     SnesA, SnesA, r0, lsr #16       @ SnesA = BBAABB00
    mov     SnesA, SnesA, lsl #8            @ SnesA = AABB0000

    mov     r0, SnesA, lsl #8               @ set the N/Z flag with the value of A
    mov     SnesNZ, r0, lsr #16
    .endif

    AddPC   \pcinc, \cycles
.endm


@=========================================================================
@ Memory store/load
@=========================================================================
.macro OpLDA mode, pcinc, cycles
	
    \mode
    ReadData

    mov     SnesA, r1, ror #mBit
    mov     SnesNZ, SnesA, lsr #16

    AddPC   \pcinc, \cycles
.endm

.macro OpLDX mode, pcinc, cycles
	
    \mode
    ReadDataXY

    mov     SnesX, r1
    mov     SnesNZ, r1, lsl #(16-xBit)

    AddPC   \pcinc, \cycles
.endm

.macro OpLDY mode, pcinc, cycles
				
    \mode
    ReadDataXY

    mov     SnesY, r1
    mov     SnesNZ, r1, lsl #(16-xBit)

    AddPC   \pcinc, \cycles
.endm

.macro OpMVP mode, pcinc, cycles
				@ FIXME ???
    \mode
    sub     SnesPC, SnesPC, #1

    stmfd   sp!, {r7}
    .ifeq mBit-8
        mov     SnesA, SnesA, lsr #8
        ldr     r0, SnesB
        orr     SnesA, SnesA, r0
    .endif
    .ifeq xBit-8
        mov     r7, #0x0000ff00
    .else
        mov     r7, #0x00ff0000
    .endif

    bl      OpMVP_Code

    .ifeq mBit-8
        and     r0, SnesA, #0xff000000
        str     r0, SnesB
        mov     SnesA, SnesA, lsl #8
    .endif
    ldmfd   sp!, {r7}

    AddPC   \pcinc, \cycles

.endm

.macro OpMVN mode, pcinc, cycles
				@ FIXME ???
    \mode
    sub     SnesPC, SnesPC, #1
    
    stmfd   sp!, {r7}
    .ifeq mBit-8
        mov     SnesA, SnesA, lsr #8
        ldr     r0, SnesB
        orr     SnesA, SnesA, r0
    .endif
    .ifeq xBit-8
        mov     r7, #0x0000ff00
    .else
        mov     r7, #0x00ff0000
    .endif

    bl      OpMVN_Code

    .ifeq mBit-8
        and     r0, SnesA, #0xff000000
        str     r0, SnesB
        mov     SnesA, SnesA, lsl #8
    .endif
    ldmfd   sp!, {r7}

    AddPC   \pcinc, \cycles
.endm

.macro OpSTA mode, pcinc, cycles
	
    \mode

    mov     r1, SnesA, lsr #(32-mBit)
    WriteData
    AddPC   \pcinc, \cycles
.endm

.macro OpSTX mode, pcinc, cycles
	
    \mode

    mov     r1, SnesX
    bic     r1, r1, #0xff000000
    bic     r1, r1, #0x00ff0000
    WriteDataXY
    AddPC   \pcinc, \cycles
.endm

.macro OpSTY mode, pcinc, cycles
	
    \mode

    mov     r1, SnesY
    bic     r1, r1, #0xff000000
    bic     r1, r1, #0x00ff0000
    WriteDataXY
    AddPC   \pcinc, \cycles
.endm

.macro OpSTZ mode, pcinc, cycles
	
    \mode

    mov     r1, #0
    WriteData
    AddPC   \pcinc, \cycles
.endm


@=========================================================================
@ Stack Functions for pushing and popping
@   r1: data to be pushed
@ returns:
@   r1: data popped
@=========================================================================
.macro Push8
    mov     r2, SnesSP, lsr #16
    add     r2, r2, #snesWramBase
    strb    r1, [r2]                
    sub     SnesSP, SnesSP, #0x00010000
.endm

.macro Push16
    mov     r2, SnesSP, lsr #16
    add     r2, r2, #snesWramBase
    strb    r1, [r2, #-1]           @ low byte
    mov     r1, r1, lsr #8
    strb    r1, [r2]                @ high byte
    sub     SnesSP, SnesSP, #0x00020000
.endm

.macro Push24
    mov     r2, SnesSP, lsr #16
    add     r2, r2, #snesWramBase
    strb    r1, [r2, #-2]           @ low byte
    mov     r1, r1, lsr #8
    strb    r1, [r2, #-1]           @ middle byte
    mov     r1, r1, lsr #8
    strb    r1, [r2]                @ high byte
    sub     SnesSP, SnesSP, #0x00030000
.endm

.macro Pop8
    add     SnesSP, SnesSP, #0x00010000
    mov     r0, SnesSP, lsr #16
    add     r0, r0, #snesWramBase
    ldrb    r1, [r0]                
.endm

.macro Pop16
    add     SnesSP, SnesSP, #0x00020000
    mov     r0, SnesSP, lsr #16
    add     r0, r0, #snesWramBase
    ldrb    r1, [r0, #-1]           @ low byte
    ldrb    r2, [r0]                @ high byte
    add     r1, r1, r2, lsl #8
.endm

.macro Pop24
    add     SnesSP, SnesSP, #0x00030000
    mov     r0, SnesSP, lsr #16
    add     r0, r0, #snesWramBase
    ldrb    r1, [r0, #-2]           @ low byte
    ldrb    r2, [r0, #-1]           @ low byte
    add     r1, r1, r2, lsl #8
    ldrb    r2, [r0]                @ high byte
    add     r1, r1, r2, lsl #16
.endm

.macro Push bitSize
    .ifeq \bitSize-8
        Push8
    .else
        Push16
    .endif
.endm

.macro Pop bitSize
    .ifeq \bitSize-8
        Pop8
    .else
        Pop16
    .endif
.endm


@=========================================================================
@ Composes the P register
@ returns:
@   r1: Snes P register
@=========================================================================
.macro ComposeP
    mov     r1, #0                      @ r1 = 00000000
    
    tsts    SnesCV, #SnesFlagC
    orrne   r1, r1, #SnesP_C            @ r1 = 0000000C
    tsts    SnesCV, #SnesFlagV
    orrne   r1, r1, #SnesP_V            @ r1 = 0V00000C

    tsts    SnesNZ, #SnesFlagN          
    orrne   r1, r1, #SnesP_N            @ r1 = NV00000C
    movs    r0, SnesNZ, lsl #16
    orreq   r1, r1, #SnesP_Z            @ r1 = NV0000ZC
    
										@ SnesMXDI = cccccccc cccccccc 0000-e00 MXDIffff
    mov     r0, SnesMXDI, lsl #22       @ r0 = 00MXDIff ff000000 00000000 00000000
    and     r0, r0, #0x3c000000         @ r0 = 00MXDI00 00000000 00000000 00000000
    orr     r1, r1, r0, lsr #24         @ r1 = 00000000 00000000 00000000 NVMXDIZC
.endm


@-------------------------------------------------------------------------
@ Set the decoder based on the MXDI flags
@   r1: 00MXDI00 bits
@-------------------------------------------------------------------------
.macro  SetMXDIDecoder
                                            @ r1 = 00mxdi00
    mov     r2, r1, lsr #4                  @ r1 = 000000mx
    ldr     r0, =m0x0Decoder
    add     EmuDecoder, r0, r2, lsl #11
.endm

@-------------------------------------------------------------------------
@ Store and set the MXDI flags
@   r1: 00MXDI00 bits
@-------------------------------------------------------------------------
.macro SetMXDI  fast = 1
    SetMXDIDecoder

    mov     r0, SnesMXDI
    bic     SnesMXDI, SnesMXDI, #0x03F0     @ SnesMXDI = cccccccc cccccccc 0000-e00 0000----
    orr     SnesMXDI, SnesMXDI, r1, lsl #2  @ SnesMXDI = cccccccc cccccccc 0000-e00 MXDI----

    @ check X bit
    tsts    SnesMXDI, #SnesFlagX            @ 00mxdi00 & 00010000
    bicne   SnesX, SnesX, #0x0000ff00       @ clear the high byte if X/Y=8bit
    bicne   SnesY, SnesY, #0x0000ff00       @ clear the high byte if X/Y=8bit

    @ check M bit
    tsts    r0, #SnesFlagM                  @ 00mxdi00 (prev) & 00100000
    
    .ifeq   \fast-1
    ldrne   r0, SnesB
    .else
    ldrne   r0, =SnesB
    ldrne   r0, [r0]
    .endif
    andne   r0, r0, #0xff000000             @ r0 = BB000000     if A=8bit
    addne   SnesA, r0, SnesA, lsr #8        @ rotate right by 8 if A=8bit
                                            @ SnesA = xxAA0000

    tsts    SnesMXDI, #SnesFlagM            @ 00mxdi00 (new) & 00100000
    andne   r1, SnesA, #0xFF000000          @ r1 = BB000000    if A=8bit
    .ifeq   \fast-1
    ldrne   r0, SnesB
    bicne   r0, r0, #0xFF000000       @ SnesB = 00II0eFF if A=8bit
    orrne   r0, r0, r1                @ SnesB = BBII0eFF if A=8bit
    strne   r0, SnesB
    .else
    ldr     r2, =SnesB
    ldrne   r0, [r2]
    bicne   r0, r0, #0xFF000000       @ SnesB = 00II0eFF if A=8bit
    orrne   r0, r0, r1                @ SnesB = BBII0eFF if A=8bit
    strne   r0, [r2]
    .endif
    movne   SnesA, SnesA, lsl #8            @ SnesA = AA000000 if A=8bitt

.endm

@-------------------------------------------------------------------------
@ Decompose the P register
@   r1: Snes P register
@-------------------------------------------------------------------------
.macro DecomposeP   fast = 1
    bic     SnesCV, SnesCV, #0xF
    tsts    r1, #SnesP_C
    orrne   SnesCV, SnesCV, #SnesFlagC
    tsts    r1, #SnesP_V
    orrne   SnesCV, SnesCV, #SnesFlagV

    mov     SnesNZ, #0
    tsts    r1, #SnesP_N
    orrne   SnesNZ, SnesNZ, #SnesFlagNH
    tsts    r1, #SnesP_Z
    orreq   SnesNZ, SnesNZ, #1
                                            @ r1 = nvmxdizc
    bic     r1, r1, #0xffffffc3             @ r1 = 00mxdi00
    SetMXDI \fast
.endm


@=========================================================================
@ Flags manipulation
@=========================================================================

.macro OpCLC mode, pcinc, cycles
    bic     SnesCV, SnesCV, #SnesFlagC

    AddPC   \pcinc, \cycles
.endm

.macro OpSEC mode, pcinc, cycles
    orr     SnesCV, SnesCV, #SnesFlagC
    AddPC   \pcinc, \cycles
.endm

.macro OpCLV mode, pcinc, cycles
    bic     SnesCV, SnesCV, #SnesFlagV

    AddPC   \pcinc, \cycles
.endm

.macro OpSEV mode, pcinc, cycles
    orr     SnesCV, SnesCV, #SnesFlagV
    AddPC   \pcinc, \cycles
.endm

.macro OpSED mode, pcinc, cycles
    orr     SnesMXDI, SnesMXDI, #SnesFlagD   @ MXDI = 00mx1i00
    AddPC   \pcinc, \cycles
.endm

.macro OpCLD mode, pcinc, cycles
    bic     SnesMXDI, SnesMXDI, #SnesFlagD   @ MXDI = 00mx0i00
    AddPC   \pcinc, \cycles
.endm

.macro OpXCE mode, pcinc, cycles
    mov     r1, SnesMXDI 
    tsts    SnesCV, #SnesFlagC
    biceq   SnesMXDI, SnesMXDI, #SnesFlagE
    orrne   SnesMXDI, SnesMXDI, #SnesFlagE
    tsts    r1, #SnesFlagE
    biceq   SnesCV, SnesCV, #SnesFlagC
    orrne   SnesCV, SnesCV, #SnesFlagC
    AddPC   \pcinc, \cycles
.endm

.macro OpSEI mode, pcinc, cycles
    orr     SnesMXDI, SnesMXDI, #SnesFlagI         @ r1 = 00mxd100
    AddPC   \pcinc, \cycles
.endm

.macro OpCLI mode, pcinc, cycles
    bic     SnesMXDI, SnesMXDI, #SnesFlagI         @ r1 = 00mxd000
    AddPC   \pcinc, \cycles
.endm

.macro OpSEP mode, pcinc, cycles
    ComposeP
    ReadAddrOperand8
    orr     r1, r1, r0
    DecomposeP
    AddPC   \pcinc, \cycles
.endm

.macro OpREP mode, pcinc, cycles
    ComposeP
    ReadAddrOperand8
    bic     r1, r1, r0
    DecomposeP
    AddPC   \pcinc, \cycles
.endm



@=========================================================================
@ Stack Opcodes
@=========================================================================

.macro OpPEA mode, pcinc, cycles
    ReadDataOperand16
    Push16
    
    AddPC   \pcinc, \cycles
.endm

.macro OpPEI mode, pcinc, cycles
    DPIndirect
    
    @ version 0.21 fix
    mov     r1, r0
    Push16

    AddPC   \pcinc, \cycles
.endm

.macro OpPER mode, pcinc, cycles
    ReadDataOperand16
    add     r1, r1, SnesPC
    ldr     r0, SnesPCOffset
    add     r1, r1, r0
    Push16

    AddPC   \pcinc, \cycles
.endm

.macro OpPLA mode, pcinc, cycles
    \mode
    Pop     mBit
    mov     SnesA, r1, ror #mBit
    mov     SnesNZ, SnesA, lsr #16

    AddPC   \pcinc, \cycles
.endm

.macro OpPHA mode, pcinc, cycles
    \mode
    mov     r1, SnesA, lsr #(32-mBit)
    Push    mBit

    AddPC   \pcinc, \cycles
.endm

.macro OpPLXY reg
    Pop     xBit
    mov     \reg, r1
    mov     SnesNZ, r1, lsl #(16-xBit)
.endm

.macro OpPLX mode, pcinc, cycles
    \mode
    OpPLXY SnesX
    AddPC   \pcinc, \cycles
.endm

.macro OpPLY mode, pcinc, cycles
    \mode
    OpPLXY SnesY
    AddPC   \pcinc, \cycles
.endm

.macro OpPHXY reg
    mov     r1, \reg
    Push    xBit
.endm

.macro OpPHX mode, pcinc, cycles
    \mode
    OpPHXY SnesX
    AddPC   \pcinc, \cycles
.endm

.macro OpPHY mode, pcinc, cycles
    \mode
    OpPHXY SnesY
    AddPC   \pcinc, \cycles
.endm

.macro OpPHB mode, pcinc, cycles
    mov     r1, SnesDBR
    Push8
    AddPC   \pcinc, \cycles
.endm

.macro OpPHD mode, pcinc, cycles
    mov     r1, SnesD, lsr #16
    Push16
    AddPC   \pcinc, \cycles
.endm

.macro OpPHK mode, pcinc, cycles
    mov     r1, SnesPBR
    Push8
    AddPC   \pcinc, \cycles
.endm

.macro OpPHP mode, pcinc, cycles
    ComposeP
    Push8
    AddPC   \pcinc, \cycles
.endm

.macro OpPLB mode, pcinc, cycles
    Pop8
    mov     SnesNZ, r1, lsl #8
    @ archeide: dont recompute cache if DB doesnt change
@    mov		r0, SnesDBR	
    bic     SnesDBR, SnesDBR, #0x000000ff
    orr     SnesDBR, SnesDBR, r1
/*    cmp		r0, SnesDBR
    beq		3f

    CacheMemoryMap
3:*/   
    AddPC   \pcinc, \cycles
.endm

.macro OpPLD mode, pcinc, cycles
    Pop16
    mov     SnesNZ, r1
    and     SnesD, SnesD, #0x000000ff
    orr     SnesD, SnesD, r1, lsl #16

    AddPC   \pcinc, \cycles
.endm

.macro OpPLP mode, pcinc, cycles
    Pop8
    DecomposeP
    AddPC   \pcinc, \cycles
.endm



@=========================================================================
@ Branch, Jump and Return opcodes
@=========================================================================
.macro OpBRL mode, pcinc, cycles
    ReadDataOperand16

	sub     SnesPC, SnesPC, #3
    mov     r1, r1, lsl #16
    mov     r1, r1, asr #16

	ldr		r0, =SnesPCOffset
	ldr		r2,	[r0]
	add		r0, SnesPC, r2
    mov     r0, r0, lsl #16
    add     r0, r0, r1, lsl #16
    add     r0, r0, #(\pcinc << 16)
    mov     r0, r0, lsr #16	
	add		r0, r0, SnesPBR, lsl #16
    
    TranslateAndSavePC  1    
    
/*    sub     SnesPC, SnesPC, #3
    mov     r1, r1, lsl #16
    mov     r1, r1, asr #16
    
    @ safe jump to retain program bank
    @
    mov     SnesPC, SnesPC, ror #16
    add     SnesPC, SnesPC, r1, lsl #16
    add     SnesPC, SnesPC, #(\pcinc << 16)
    mov     SnesPC, SnesPC, ror #16
    
@    CheckPCReload FIXME*/
    
    AddPC   0, \cycles
.endm

.macro CheckWaitAddress
	ldr		r1, CPU_LoopSpeedHacks
	cmp		r1,	#0
	beq		3f

	@ archeide: First, save Loop address
	str		SnesPC,	CPU_LoopAddress

	@ archeide: Short loop detection, a loop with only 2 instr 
/*	sub		r0, r2, SnesPC
	cmp		r0, #4
	bls		4f		@ do addition check there
@	b		3f*/

 	@ archeide: Buggy Interrupt Loop Detection
    ldr		r0, CPU_WaitAddress
    cmp		SnesPC,	r0
    bne		3f
	b		5f    

4:
	@ archeide: Short loop additionnal checker, seems not enough for ALLSTARS
	ldr		r0, =OpcShortLoop
	ldrb	r1, [SnesPC]
	ldrb	r0, [r0, r1]
	cmp		r0, #0
	beq		3f		  
5:    
	@ Skip execution to next event
    mov     SnesCYCLES, SnesCYCLES, lsl #(32-CYCLE_SHIFT)
    mov     SnesCYCLES, SnesCYCLES, lsr #(32-CYCLE_SHIFT)
    ldr		r0, =CPU_NextCycles
    ldr		r0, [r0]        
   	sub		SnesCYCLES, SnesCYCLES, r0
3:
.endm

.macro OpBRA mode, pcinc, cycles
    ldrsb   r1, [SnesPC], #-1
    
    mov		r2, SnesPC @ archeide: used by CheckAddress
    @ safe jump to retain program bank
    @
    mov     SnesPC, SnesPC, ror #16
    add     SnesPC, SnesPC, r1, lsl #16
    add     SnesPC, SnesPC, #(\pcinc << 16)
    mov     SnesPC, SnesPC, ror #16
    
	CheckWaitAddress
    
    AddPC   0, \cycles
.endm

.macro OpBR bitToTest, set, pcinc, cycles
    ldrsb   r1, [SnesPC], #-1
    
    mov		r2, SnesPC @ archeide: used by CheckAddress
    add     SnesPC, SnesPC, #2
    .ifeq \bitToTest-SnesFlagZ
        movs    r0, SnesNZ, lsl #16
        .ifeq \set-1
            addeq     SnesPC, SnesPC, r1
            CheckWaitAddress
        .else
            addne     SnesPC, SnesPC, r1
            CheckWaitAddress
        .endif
    .endif
    .ifeq \bitToTest-SnesFlagN
        tsts    SnesNZ, #SnesFlagN
        .ifeq \set-1
            addne     SnesPC, SnesPC, r1
            CheckWaitAddress
        .else
            addeq     SnesPC, SnesPC, r1
            CheckWaitAddress
        .endif
    .endif
    .ifeq \bitToTest-SnesFlagC
        tsts    SnesCV, #SnesFlagC
        .ifeq \set-1
            addne     SnesPC, SnesPC, r1
            CheckWaitAddress
        .else
            addeq     SnesPC, SnesPC, r1
            CheckWaitAddress
        .endif
    .endif
    .ifeq \bitToTest-SnesFlagV
        tsts    SnesCV, #SnesFlagV
        .ifeq \set-1
            addne     SnesPC, SnesPC, r1
            CheckWaitAddress
        .else
            addeq     SnesPC, SnesPC, r1
            CheckWaitAddress
        .endif
    .endif   
    
    AddPC   \pcinc, \cycles
.endm

.macro OpJMP mode, pcinc, cycles
    TranslateAndSavePC  1
    AddPC   0, \cycles
.endm

.macro OpJSR mode, pcinc, cycles
    .ifc \mode, AbsoluteLongPC
        and     r1, SnesPBR, #0x000000ff
        Push8
    .endif
    sub     SnesPC, SnesPC, #1
    ldr     r1, SnesPCOffset
    add     r1, SnesPC, r1
    Push16

    add     SnesPC, SnesPC, #1
    TranslateAndSavePC  1
    AddPC   0, \cycles
.endm

.macro OpRTL mode, pcinc, cycles
    Pop24

    @ save the PBR
    add     r1, r1, #1
    and     r2, r1, #0x00ff0000
    bic     SnesPBR, SnesPBR, #0x000000ff
    orr     SnesPBR, SnesPBR, r2, lsr #16

    @ Translate the address and move it to PC
    mov     r0, r1
    TranslateAndSavePC  1

    AddPC   \pcinc, \cycles
.endm

.macro OpRTS mode, pcinc, cycles
    Pop16

    @ add PBR to address
    add     r0, r1, SnesPBR, lsl #16
    add     r0, r0, #1
    TranslateAndSavePC  1   
    AddPC   \pcinc, \cycles
.endm

.macro OpRTI mode, pcinc, cycles
    Pop8
    DecomposeP 0 @ FIXME was 1

    Pop24
    mov     r0, r1
    @ version 0.22 fix
    @add     r0, r0, #1
    TranslateAndSavePC  1  

    @bic     SnesIRQ, SnesIRQ, #IRQ_NMI
/*    ldr		r0, =CPU_log
    mov		r1, #0
    str		r1, [r0]*/  

    AddPC   \pcinc, \cycles
.endm

@-------------------------------------------------------------------------
@ stub
@-------------------------------------------------------------------------
.macro OpNOP mode, pcinc, cycles
    AddPC   \pcinc, \cycles
.endm

.macro XXXXX mode, pcinc, cycles
    AddPC   \pcinc, \cycles
.endm


@=========================================================================
@ Others 
@=========================================================================


.macro OpBRK mode, pcinc, cycles
    mov     r11, r11
    ldr		r0, =CPU_log
    mov		r1, #1
    str		r1, [r0]
    add     SnesPC, SnesPC, #1
    ExecuteInterrupt    BRKaddress
    AddPC   \pcinc, \cycles
.endm

.macro OpCOP mode, pcinc, cycles
    @ added for version 0.22
    @
    add     SnesPC, SnesPC, #1
    ExecuteInterrupt    COPaddress
    AddPC   \pcinc, \cycles
.endm

.macro  OpWAI mode, pcinc, cycles

    sub     SnesPC, SnesPC, #1
    
    @ version 0.27 fix
    mov     SnesCYCLES, SnesCYCLES, lsl #(32-CYCLE_SHIFT)      @ set the cycles to zero to trigger next interrupt
    mov     SnesCYCLES, SnesCYCLES, lsr #(32-CYCLE_SHIFT)
    @ version 0.27 fix end
    
    orr     SnesWAI, SnesWAI, #SnesFlagWAI

    @ force interrupt to occur
    @
/*  I've disabled this for the DS version, as we need to maintain sync with the APU
	ldrb    r0, regNMI                          
    mov     r0, r0, lsr #4
    ldr     r1, =ScanlineSkipTable
    mov     lr, pc
    ldr     pc, [r1, r0, lsl #2]
*/
    AddPC   \pcinc, 0
.endm

@ $DB
.macro  OpSTP mode, pcinc, cycles

    @ version 0.27 fix
    mov     SnesCYCLES, SnesCYCLES, lsl #(32-CYCLE_SHIFT)      @ set the cycles to zero to trigger next interrupt
    mov     SnesCYCLES, SnesCYCLES, lsr #(32-CYCLE_SHIFT)
    @ version 0.27 fix end

    ldrb    r1, [SnesPC], #-1
    and     r0, r1, #0x80                       @ r0 = instruction to branch to
    mov     r0, r0, lsr #2
    orr     r0, r0, #0xD0                       @ r0 = $DO (BNE) / $FO (BEQ)
    
    orr     r1, r1, #0x80
    mov     r1, r1, lsl #25
    mov     r1, r1, asr #25                     @ r1 = -ve offset

    ldr     lr, [EmuDecoder, r0, lsl #3]
    add     pc, lr, #4                          @ skip the first instruction of the branch 
                                                @ which loads the signed offset from ROM
.endm

@ $42 (WDM)
.macro  OpRES mode, pcinc, cycles

    @ version 0.27 fix
    mov     SnesCYCLES, SnesCYCLES, lsl #(32-CYCLE_SHIFT)      @ set the cycles to zero to trigger next interrupt
    mov     SnesCYCLES, SnesCYCLES, lsr #(32-CYCLE_SHIFT)
    @ version 0.27 fix end

    ldrb    r1, [SnesPC], #-1
    and     r0, r1, #0xF0                       @ r0 = instruction to branch to
    
    orr     r1, r1, #0xF0
    mov     r1, r1, lsl #24
    mov     r1, r1, asr #24                     @ r1 = -ve offset

    ldr     lr, [EmuDecoder, r0, lsl #3]
    add     pc, lr, #4                          @ skip the first instruction of the branch 
                                                @ which loads the signed offset from ROM
.endm

.macro ExecuteInterrupt address, nonMaskable=1
    .ifeq \nonMaskable-0
        @ version 0.22 fix
        tsts    SnesMXDI, #SnesFlagI
        bne     1f
    .endif

    and     r1, SnesPBR, #0x000000ff
    
    @ version 0.22 fix for WAI
    @
    tsts    SnesWAI, #SnesFlagWAI
    addne   SnesPC, SnesPC, #1
    bicne   SnesWAI, SnesWAI, #SnesFlagWAI
    
    Push8
    @ version 0.22 fix
    @ sub     SnesPC, SnesPC, #1
    ldr     r1, =SnesPCOffset
    ldr     r1, [r1]
    add     r1, SnesPC, r1
    Push16

	ComposeP                        @ r1 = nvmxdizc
    Push8
    bic     SnesMXDI, SnesMXDI, #SnesFlagD
    orr     SnesMXDI, SnesMXDI, #SnesFlagI

    ldr     r0, =\address
    ldrh    r0, [r0]
    TranslateAndSavePC
1:
.endm

@-------------------------------------------------------------------------
@ Setting M/X bits
@-------------------------------------------------------------------------
.macro  M1
    .equ    mBit, 8
    .equ    immMode, 0
.endm

.macro  M0
    .equ    mBit, 16
    .equ    immMode, 0
.endm

.macro  X1
    .equ    xBit, 8
    .equ    immMode, 0
.endm

.macro  X0
    .equ    xBit, 16
    .equ    immMode, 0
.endm

.macro  M0X0
    .equ    mBit, 16
    .equ    xBit, 16
    .equ    immMode, 0
.endm

.macro  M0X1
    .equ    mBit, 16
    .equ    xBit, 8
    .equ    immMode, 0
.endm

.macro  M1X0
    .equ    mBit, 8
    .equ    xBit, 16
    .equ    immMode, 0
.endm

.macro  M1X1
    .equ    mBit, 8
    .equ    xBit, 8
    .equ    immMode, 0
.endm

@-------------------------------------------------------------------------
@ Setting M/X bits (with immediate mode)
@-------------------------------------------------------------------------
.macro  M1IMM
    .equ    mBit, 8
    .equ    immMode, 1
.endm

.macro  M0IMM
    .equ    mBit, 16
    .equ    immMode, 1
.endm

.macro  X1IMM
    .equ    xBit, 8
    .equ    immMode, 1
.endm

.macro  X0IMM
    .equ    xBit, 16
    .equ    immMode, 1
.endm

.macro  M0X0IMM
    .equ    mBit, 16
    .equ    xBit, 16
    .equ    immMode, 1
.endm

.macro  M0X1IMM
    .equ    mBit, 16
    .equ    xBit, 8
    .equ    immMode, 1
.endm

.macro  M1X0IMM
    .equ    mBit, 8
    .equ    xBit, 16
    .equ    immMode, 1
.endm

.macro  M1X1IMM
    .equ    mBit, 8
    .equ    xBit, 8
    .equ    immMode, 1
.endm

.macro  NA
.endm

@-------------------------------------------------------------------------
@ Add PC and end execution, return to fetch
@-------------------------------------------------------------------------
.macro OpcodeFetch
    ldrmib  r0, [SnesPC], #1                    @ 3 (for WRAM/ROM access)
    addmi   r0, EmuDecoder, r0, lsl #3          @ 1
    ldmmiia r0, {lr, pc}                        @ 4 ?    
    b		EndOfCPULoop
@    ldr     r0, =TimeOutHandlerAddress
@    ldr     pc, [r0]
.endm

.macro FastFetch
    .ifeq   debug-0
        @ do a fast fetch (uses more IWRAM)
        @
        OpcodeFetch
    .else
        @ do a slow fetch with jump to debug routine
        b       Fetch
    .endif
.endm

.macro AddPCNoJump v, cycles
    @.ifne   \v
    @    add     SnesPC, SnesPC, #\v
    @.endif

	adds    SnesCYCLES, SnesCYCLES, #(\cycles << CYCLE_SHIFT)
.endm

.macro AddPC v, cycles
    AddPCNoJump \v, \cycles
    FastFetch
.endm

   
.macro StartExecute
    bx      lr
.endm

.macro EndExecute
    b       Fetch
.endm

