#ifdef ARM7
.text
#endif

#ifdef ARM9
.section .dtcm,"ax",%progbits
#endif


.align 4
.code 32
.arm

.global APU_T0_ASM_ADDR
APU_T0_ASM_ADDR:
    .word 0x0

.global APU_T1_ASM_ADDR
APU_T1_ASM_ADDR:
    .word 0x0

.global APU_T2_ASM_ADDR
APU_T2_ASM_ADDR:
    .word 0x0

.global APU_TIM0_ASM_ADDR
APU_TIM0_ASM_ADDR:
    .word 0x0

.global APU_TIM1_ASM_ADDR
APU_TIM1_ASM_ADDR:
    .word 0x0

.global APU_TIM2_ASM_ADDR
APU_TIM2_ASM_ADDR:
    .word 0x0

.global APU_CNT0_ASM_ADDR
APU_CNT0_ASM_ADDR:
    .word 0x0

.global APU_CNT1_ASM_ADDR
APU_CNT1_ASM_ADDR:
    .word 0x0
	
.global APU_CNT2_ASM_ADDR
APU_CNT2_ASM_ADDR:
    .word 0x0

.global ADDRPORT_SPC_TO_SNES
ADDRPORT_SPC_TO_SNES:
    .word 0x0

.global ADDRPORT_SNES_TO_SPC
ADDRPORT_SNES_TO_SPC:
    .word 0x0

.global ADDR_APU_PROGRAM_COUNTER
ADDR_APU_PROGRAM_COUNTER:
    .word 0x0

.global ADDR_SNEMUL_CMD
ADDR_SNEMUL_CMD:
    .word 0x0

.global ADDR_SNEMUL_ANS
ADDR_SNEMUL_ANS:
    .word 0x0

.global ADDR_SNEMUL_BLK
ADDR_SNEMUL_BLK:
    .word 0x0


@to deprecate:

#ifdef ARM9

.global snes_ram_address    @wram memory
snes_ram_address:
    .word   0x00000000      @r0     a1

#endif
.pool
.end
