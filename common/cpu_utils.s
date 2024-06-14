#ifdef ARM7
.text
#endif

#ifdef ARM9
.section .dtcm,"ax",%progbits
#endif


.align 4
.code 32
.arm

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

.pool
.end
