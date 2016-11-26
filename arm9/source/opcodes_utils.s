.section	.dtcm
.align 4
.code 32
.arm	

.global snes_ram_address    @wram memory
snes_ram_address:
    .word   0x00000000      @r0     a1

.pool