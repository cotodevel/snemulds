
@			Copyright (C) 2017  Coto
@This program is free software; you can redistribute it and/or modify
@it under the terms of the GNU General Public License as published by
@the Free Software Foundation; either version 2 of the License, or
@(at your option) any later version.

@This program is distributed in the hope that it will be useful, but
@WITHOUT ANY WARRANTY; without even the implied warranty of
@MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
@General Public License for more details.

@You should have received a copy of the GNU General Public License
@along with this program; if not, write to the Free Software
@Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
@USA


@Coto: this is template code that you can use if you ever run out of stack space (about 0x200 bytes!), and we extend to 32K.
@In this example code, the ZLIB library fall back to EWRAM as stack, through malloc/free which I already implemented.

.section	.dtcm
.arm

.global snapshotRegs
snapshotRegs:
	.word 0x00000000	@r0
	.word 0x00000000	@r1
	.word 0x00000000	@r2
	.word 0x00000000	@r3
	.word 0x00000000	@r4
	.word 0x00000000	@r5
	.word 0x00000000	@r6
	.word 0x00000000	@r7
	.word 0x00000000	@r8
	.word 0x00000000	@r9
	.word 0x00000000	@r10
	.word 0x00000000	@r11
	.word 0x00000000	@r12
	.word 0x00000000	@r13 / sp
	.word 0x00000000	@r14 / lr
	.word 0x00000000	@r15 / pc


.section	.text
.arm

@int do_decompression_ewramstack(r0 = char *inname, r1 = char *outname);
	.global	do_decompression_ewramstack
	.type	do_decompression_ewramstack STT_FUNC
do_decompression_ewramstack:
	push {r3-r12,lr}
	ldr r2,=snapshotRegs
	stmia	r2,{sp}
	
	mov r7,r0
	mov r8,r1		@save: (r0 = char *inname, r1 = char *outname);
	
	mov r0, #(32*1024)	@32K stack
	push {r7,r8}
	blx malloc 
	pop {r7,r8}
	add sp,r0,#(32*1024)	@use banked stack and forward to ARM full descending nature
	
	mov r0,r7
	mov r1,r8
	
	push {r3-r12,lr}
	blx do_decompression
	pop {r3-r12,lr}
	
	mov r1,r0	@r0 ret
	add r0,sp,#-(32*1024)	@use banked stack and rewind from ARM full descending nature to malloc/free linear deallocation
	
	ldr r2,=snapshotRegs
	ldmia	r2,{sp}
	pop {r3-r12,lr}
	
	push {r1,r3-r12,lr}
	blx free
	pop {r1,r3-r12,lr}
	
	mov r0,r1	@restore ret
	bx	lr


.pool
