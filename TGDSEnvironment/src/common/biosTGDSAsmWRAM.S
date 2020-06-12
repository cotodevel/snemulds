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
.text
.arm
.code 32

#include "dsregs_asm.h"

@void swiSoftResetByAddress(u32 address);
.global 	swiSoftResetByAddress
.type   	swiSoftResetByAddress STT_FUNC
swiSoftResetByAddress:
	mov r4, r0
	#ifdef ARM9
	.arch	armv5te
	.cpu	arm946e-s
	@ Wait for write buffer to empty 
	mcr	p15, 0, r0, c7, c10, 4
	
	ldr	r1, =0x00002078			@ disable TCM and protection unit
	mcr	p15, 0, r1, c1, c0
	
	@ Disable cache
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ Instruction cache
	mcr	p15, 0, r0, c7, c6, 0		@ Data cache
	#endif
	
	@Enter SYS Mode, irqs disabled
	mrs r0, cpsr
	bic r0, r0, #(0xDF)
	orr r0, r0, #(0xDF)
	msr	cpsr, r0
	
	msr	spsr, r0
	bx	r4

.pool
.end