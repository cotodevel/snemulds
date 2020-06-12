
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

.section	.text
.arm

#define EWRAM_STACK_SIZE (32*1024)

.extern xenoTGDSARM9Malloc
.extern xenoTGDSARM9Free
@int do_decompression_ewramstack(r0 = char *inname, r1 = char *outname);
	.global	do_decompression_ewramstack
	.type	do_decompression_ewramstack STT_FUNC
do_decompression_ewramstack:
	push {r3-r12,lr}
	
	mov r7,r0		@save: (r0 = char *inname, r1 = char *outname, r9 = SP);
	mov r8,r1		
	mov r9,sp
	
	mov r0, #(EWRAM_STACK_SIZE)
	push {r7-r9}
	bl xenoTGDSARM9Malloc
	pop {r7-r9}
	add sp,r0,#(EWRAM_STACK_SIZE)	@use banked stack and forward to ARM full descending nature
	
	mov r0,r7
	mov r1,r8
	
	push {r3-r12}
	bl do_decompression
	pop {r3-r12}
	
	mov r1,r0	@r0 ret
	add r0,sp,#-(EWRAM_STACK_SIZE)	@use banked stack and rewind from ARM full descending nature to malloc/free linear deallocation
	
	push {r1,r9}
	bl xenoTGDSARM9Free
	pop {r1,r9}
	
	mov sp,r9
	pop {r3-r12,lr}
	
	mov r0,r1	@restore ret
	bx	lr


.pool
