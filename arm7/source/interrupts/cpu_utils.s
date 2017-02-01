.align 4
.code 32
.arm

@future ARM7 opcodes use this format, and here
@.global HALTCNT_ARM7
@.type	HALTCNT_ARM7 STT_FUNC
@HALTCNT_ARM7:
@	swi 0x40000
@	bx lr


.global ADDR_PORT_SNES_TO_SPC
ADDR_PORT_SNES_TO_SPC:
    .word 0x0

.global ADDR_PORT_SPC_TO_SNES
ADDR_PORT_SPC_TO_SNES:
    .word 0x0

.end