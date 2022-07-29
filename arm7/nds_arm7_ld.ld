/*
			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA
*/

/* TGDS Linker file changelog:
0.3 (Date 08 May 2022): Rewritten so data is mapped manually across ARM payload. Better relocation handling.
0.2 (Somewhere 2020): Ported to TWL, broken when relocating stuff sometimes.
0.1 (Somewhere 2017): Base, broken when relocating stuff sometimes. 
*/
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
_iwram_start =	0x03800000;
_iwram_end = _iwram_start + 64K;
/* ARM ABI says stacks are full decreasing */
sp_SVC	= _iwram_end - 0x200; /* reset, swi exception*/
sp_IRQ	= sp_SVC - 0x200;	/*irq exception*/
sp_FIQ	= sp_IRQ - 0x200;	/* fiq exception */
sp_ABT	= sp_FIQ - 0x200;	/* prefetch / data aborts exception */
sp_UND	= sp_ABT - 0x200;	/* undefined exception */
sp_USR	= sp_UND - 0x200;	/* usr / sys stacks */
sp_SYS = sp_USR;
/* ARM7 Specific */
_arm7_irqcheckbits = 0x0380FFF8;	/* __irq_flags */
__irq_flagsaux	=	0x04000000 - 0x40;
_arm7_irqhandler = 0x0380FFFC;	/* __irq_vector */

SECTIONS
{
	. = _iwram_start; /* crt0 bootstub */
	.init :
    {
        KEEP(*(.init.boot))
        KEEP(*(.init))
		. = ALIGN(4);
    }
    . = ALIGN(4); /* align to a block of code*/
    .text :
    {
        KEEP(*(.text.boot))
        KEEP(*(.text))
		. = ALIGN(4);
    }
    . = ALIGN(4); /* align to a block of code*/
	__text_end = .; 
    __rodata_start = .;
    .rodata :
    {
        KEEP(*(.rodata))
		. = ALIGN(4);
    }
    . = ALIGN(4); /* align to a block of code*/
    __rodata_end = .;
 
    __data_start = .;
    .data :
    {
        KEEP(*(.data))
		. = ALIGN(4);
    }
    . = ALIGN(4); /* align to a block of code*/
    __data_end = .;
 
    __bss_start = .;
    .bss :
    {
        bss = .;
        KEEP(*(.bss))
		. = ALIGN(4);
    }
    . = ALIGN(4); /* align to a block of code*/
    __bss_end = .;
	
	__exidx_start = .;
    .ARM.exidx :
    {
        ARM.exidx = .;
        *(.ARM.exidx)
		. = ALIGN(4);
    }
    . = ALIGN(4); /* align to a block of code*/
    __exidx_end = .;
	
	. = ADDR (.ARM.exidx) + SIZEOF (.ARM.exidx);
	
	__vma_stub_end__ = .;
    __end = __vma_stub_end__ + 0x100;
	
	.pad ALIGN(4) : AT (__end)
	{
		LONG(0x70C070C0)
		LONG(0xEEEEFFC0)
		. = ALIGN(4); 
	} = 0xff
	
	/* stubbed for ARM7i binaries crt0 in NTR mode, but actually used in TWL mode */
	__arm7i_lma__ = __vma_stub_end__ + 4;
	__arm7i_start__ = __arm7i_lma__ + 4;
	__arm7i_end__ = __arm7i_start__ + 4;
	
	__arm7_lma__ = __arm7i_end__ + 4;
	__arm7_start__ = __arm7_lma__ + 4;
	__arm7_end__ = __arm7_start__ + 4;
}