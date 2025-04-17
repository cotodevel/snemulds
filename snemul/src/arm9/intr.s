	.TEXT
	.ARM
	.ALIGN

.GLOBAL IntrHandlerAsm
IntrHandlerAsm:
    mov r0, #0x4000000
    ldr r1, [r0,#0x214] @ IF

    tsts    r1, #0x02               @ h-blank interrupt
    bne     Hblank
    tsts    r1, #0x01               @ v-blank interrupt
    bne     Vblank
    bx      lr