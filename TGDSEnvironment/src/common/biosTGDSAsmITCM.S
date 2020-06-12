@
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
@


@ ***************************************************************************
@ *                                                                         *
@ *  This file is part of DSOrganize.                                       *
@ *                                                                         *
@ *  DSOrganize is free software: you can redistribute it and/or modify     *
@ *  it under the terms of the GNU General Public License as published by   *
@ *  the Free Software Foundation, either version 3 of the License, or      *
@ *  (at your option) any later version.                                    *
@ *                                                                         *
@ *  DSOrganize is distributed in the hope that it will be useful,          *
@ *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
@ *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
@ *  GNU General Public License for more details.                           *
@ *                                                                         *
@ *  You should have received a copy of the GNU General Public License      *
@ *  along with DSOrganize.  If not, see <http://www.gnu.org/licenses/>.    *
@ *                                                                         *
@ ***************************************************************************



@NDS BIOS Routines ARM ASM code

#ifdef ARM7
.text
#endif

#ifdef ARM9
.section .itcm,"ax",%progbits
#endif

.arm
.code 32

#include "dsregs_asm.h"

@void swiDelay(uint32 delayvalue);
.global 	swiDelay
.type   	swiDelay	STT_FUNC
swiDelay:
	swi	0x30000
	bx	lr

@uint16 swiCRC16(uint16 crc, void * data, uint32 size);
.global 	swiCRC16
.type   	swiCRC16 STT_FUNC
swiCRC16:
	swi 0xe0000
bx r14

@void swiSleep(void);
.global 	swiSleep
.type   	swiSleep STT_FUNC
swiSleep:
	swi	0x70000
	bx	lr

@int swiIsDebugger(void);
.global 	swiIsDebugger
.type   	swiIsDebugger STT_FUNC
swiIsDebugger:
	swi	0xF0000
	bx	lr

@int swiDivide(int numerator, int divisor);
.global 	swiDivide
.type   	swiDivide STT_FUNC
swiDivide:
	swi	0x90000
	bx	lr

@int swiRemainder(int numerator, int divisor);
.global 	swiRemainder
.type   	swiRemainder STT_FUNC
swiRemainder:
	swi	0x90000
	mov	r0, r1
	bx	lr

@void swiDivMod(int numerator, int divisor, int * result, int * remainder);
.global 	swiDivMod
.type   	swiDivMod STT_FUNC
swiDivMod:
	push	{r2, r3}
	swi	0x90000
	pop	{r2, r3}
	str	r0, [r2]
	str	r1, [r3]
	bx	lr

@void swiCopy(const void * source, void * dest, int flags);
.global 	swiCopy
.type   	swiCopy STT_FUNC
swiCopy:
	swi	0xB0000
	bx	lr

@int swiSqrt(int value);
.global 	swiSqrt
.type   	swiSqrt STT_FUNC
swiSqrt:
	swi	0xD0000
	bx	lr


#ifdef ARM7
@void swiHalt(void);
.global 	swiHalt
.type   	swiHalt STT_FUNC
swiHalt:
	swi	0x60000
	bx	lr

@uint16 swiGetSineTable(int index);
.global 	swiGetSineTable
.type   	swiGetSineTable STT_FUNC
swiGetSineTable:
	swi	0x1A0000
	bx	lr

@uint16 swiGetPitchTable(int index);
.global 	swiGetPitchTable
.type   	swiGetPitchTable STT_FUNC
swiGetPitchTable:
	swi	0x1B0000
	bx	lr

@uint8 swiGetVolumeTable(int index);
.global 	swiGetVolumeTable
.type   	swiGetVolumeTable STT_FUNC
swiGetVolumeTable:
	swi	0x1C0000
	bx	lr


@void swiSetHaltCR(uint32 data);
.global 	swiSetHaltCR
.type   	swiSetHaltCR STT_FUNC
swiSetHaltCR:
	mov	r2, r0
	swi	0x1F0000
	bx	lr

@void swiChangeSoundBias(int enabled, int delay);
.global 	swiChangeSoundBias
.type   	swiChangeSoundBias STT_FUNC
swiChangeSoundBias:
	swi	0x80000
	bx	lr
	
#endif

@void swiSoftReset(void);
.global 	swiSoftReset
.type   	swiSoftReset STT_FUNC
swiSoftReset:
REG_IME = 0;
#ifdef ARM7
	ldr	r0,=0x02FFFE34
#endif

#ifdef ARM9
	.arch	armv5te
	.cpu	arm946e-s
	ldr	r1, =0x00002078			@ disable TCM and protection unit
	mcr	p15, 0, r1, c1, c0
	@ Disable cache
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ Instruction cache
	mcr	p15, 0, r0, c7, c6, 0		@ Data cache

	@ Wait for write buffer to empty 
	mcr	p15, 0, r0, c7, c10, 4
	
	ldr	r0,=0x02FFFE24
#endif

	ldr	r0,[r0]
	bx	r0

@void swiDecompressLZSSWram(void * source, void * destination);
.global 	swiDecompressLZSSWram
.type   	swiDecompressLZSSWram STT_FUNC
swiDecompressLZSSWram:
	swi	0x110000
	bx	lr

@int swiDecompressLZSSVram(void * source, void * destination, uint32 toGetSize, TDecompressionStream * stream);
.global 	swiDecompressLZSSVram
.type   	swiDecompressLZSSVram STT_FUNC
swiDecompressLZSSVram:
	swi	0x120000
	bx	lr

.pool
.end