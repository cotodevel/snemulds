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
_ewram_start =	0x02000800;
_ewram_end = _ewram_start + 4M;
__text_start = _ewram_start + 0x1000;  
_itcm_start = 0x01FF8000;
_itcm_end = _itcm_start + 32K;
_dtcm_start = 0x0b000000;
_dtcm_end = _dtcm_start + 16K;
_arm9sharedwram_start = 0x03000000;
_arm9sharedwram_end = _arm9sharedwram_start + 32K;
/* ARM ABI says stacks are full decreasing */
sp_SVC	= _dtcm_end  - 0x200;	/* reset, swi exception*/
sp_IRQ	= sp_SVC - 0x200;	/*irq exception*/
sp_FIQ	= sp_IRQ - 0x200;	/* fiq exception */
sp_ABT	= sp_FIQ - 0x200;	/* prefetch / data aborts exception */
sp_UND	= sp_ABT - 0x200;	/* undefined exception */
sp_USR	= sp_UND - 0x200;	/* usr / sys stacks */
sp_SYS = sp_USR;
/* ARM9 Specific */
_arm9_irqcheckbits = _dtcm_start + 0x3FF8;	/* __irq_flags */
_arm9_irqhandler = _dtcm_start + 0x3FFC;	/* __irq_vector */

SECTIONS
{
	. = _ewram_start; /* crt0 bootstub */
	.init :
    {
        KEEP(*(.init.boot))
        KEEP(*(.init))
		. = ALIGN(4);
    }
	. = ALIGN(4);
    . = __text_start;
    .text :
    {
		KEEP(*(.initDldiSection))
		KEEP(*(.dldiSection))
		. = ALIGN(4);
        KEEP(*(.text.boot))
        KEEP(*(.text))
		. = ALIGN(4);
    }
    __text_end = .; 
	. = ALIGN(4);
    __rodata_start = .;
    .rodata :
    {
        KEEP(*(.rodata))
		. = ALIGN(4);
    }
    __rodata_end = .;
	. = ALIGN(4);
    __data_start = .;
    .data :
    {
        KEEP(*(.data))
		. = ALIGN(4);
    }
    __data_end = .;
	. = ALIGN(4);
    __bss_start = .;
    .bss :
    {
        bss = .;
        KEEP(*(.bss))
		. = ALIGN(4);
    }
    __bss_end = .;
	. = ALIGN(4);
	__exidx_start = .;
    .ARM.exidx :
    {
        ARM.exidx = .;
        *(.ARM.exidx)
		. = ALIGN(4);
    }
    __exidx_end = .;
	. = ALIGN(4);
	/* Stub to hook ITCM + other independent sections*/
	.arm9stub :
    {
        arm9stub = .;
        *(.arm9stub)
		. = ALIGN(4);
    }
    . = ADDR (.arm9stub) + SIZEOF (.arm9stub);
    . = ALIGN(4);
	/* ITCM */
	__itcm_vma_start = .;
	.itcm _itcm_start : AT ( ADDR (.arm9stub) + SIZEOF (.arm9stub) )
    {
        KEEP(*(.itcm))
		. = ALIGN(4);
    } 
    . = ADDR (.arm9stub) + SIZEOF (.arm9stub) + SIZEOF (.itcm);
    __itcm_vma_end__ = .;
	. = ALIGN(4);
	/* DTCM */
	__dtcm_vma_start = .;
    .dtcm _dtcm_start : AT ( ADDR (.arm9stub) + SIZEOF (.arm9stub) + SIZEOF (.itcm) )
    {
        KEEP(*(.dtcm))
		. = ALIGN(4);
    } 
    . = ADDR (.arm9stub) + SIZEOF (.arm9stub) + SIZEOF (.itcm) + SIZEOF (.dtcm);
	__dtcm_vma_end__ = .;
	. = ALIGN(4);
	___arm9sharedwram_vma_start = .;
    .arm9sharedwram _arm9sharedwram_start : AT ( ADDR (.arm9stub) + SIZEOF (.arm9stub) + SIZEOF (.itcm) + SIZEOF (.dtcm) )
    {
        KEEP(*(.arm9sharedwram))
		*(.arm9sharedwram)
		. = ALIGN(4);
    } 
    . = ADDR (.arm9stub) + SIZEOF (.arm9stub) + SIZEOF (.itcm) + SIZEOF (.dtcm) + SIZEOF (.arm9sharedwram);
	___arm9sharedwram_vma_end__ = .;
	. = ALIGN(4);
	__vma_stub_end__ = .;
    __end = __vma_stub_end__ + 0x100;
	. = ALIGN(4);
	.pad ALIGN(4) : AT (__end)
	{
		LONG(0x70C070C0)
		LONG(0xEEEEFFC0)
		. = ALIGN(4); 
	} = 0xff
}