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

/* Coto: own linker stuff. Written by me. for NDS hardware (non DSi-3DS)*/
OUTPUT_FORMAT("elf32-littlearm")

/* attributes can actually help in debugging because if you try to move functions to DTCM, for example, compiler will rant since DTCM is DATA only path and not execution*/
MEMORY
{
	DTCM : ORIGIN = 0x0b000000, LENGTH = 16K
	ITCM : ORIGIN = 0x01FF8300, LENGTH = 32K - 1K + 256
	VECTORS : ORIGIN = 0x01FF8000, LENGTH = 1K - 256
	GBAROM : ORIGIN = 0x08000000, LENGTH = 32M
	EWRAM : ORIGIN = 0x02000000, LENGTH = 4M
	GBAEW   : ORIGIN = 0x02000000, LENGTH = 256k	/* todo... */
}

_itcm_start = ORIGIN(ITCM);
_itcm_end = ORIGIN(ITCM)	+	LENGTH(ITCM);

_vector_start = ORIGIN(VECTORS);
_vector_end = ORIGIN(VECTORS)	+	LENGTH(VECTORS);

_ewram_start = ORIGIN(EWRAM);
_ewram_end = ORIGIN(EWRAM)	+	LENGTH(EWRAM);

_gba_start = ORIGIN(GBAROM);
_gba_end = ORIGIN(GBAROM)	+	LENGTH(GBAROM);

_gbawram_start = ORIGIN(GBAEW);
_gbawram_end = ORIGIN(GBAEW)	+	LENGTH(GBAEW);

_dtcm_start = ORIGIN(DTCM);
_dtcm_end = ORIGIN(DTCM)	+	LENGTH(DTCM);

/* ARM9 Specific */
_arm9_irqcheckbits = ORIGIN(DTCM) + 0x3FF8;	/* __irq_flags */
_arm9_irqhandler = ORIGIN(DTCM) + 0x3FFC;	/* __irq_vector */

/* ARM ABI says stacks are full decreasing */
sp_SVC	= _dtcm_end - 0x200;
sp_IRQ	= sp_SVC - 0x200;
sp_FIQ	= sp_IRQ - 0x200;
sp_ABT	= sp_FIQ - 0x200;
sp_UND	= sp_ABT - 0x200;
sp_USR	= sp_UND - 0x200;
sp_SYS	= sp_USR - 0x200;
sp_IRQCHANDLER	=	sp_SYS - 0x1000;

SECTIONS
{
	.init : { __init_start__ = .;
		*(.init*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	__init_end__ = .;
	__preinit_array_start__ = .;
	.preinit_array : { 
		*(.preinit_array*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	__preinit_array_end__ = .;
	__init_array_start__ = .;
	.init_array : { 
		*(.init_array*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	init_array_end__ = .;
	__fini_array_start__ = .;
	.fini_array : {
		*(.fini_array*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	__fini_array_end__ = .;
	__text_start__ = .;
	.text : {
		*(.text)
		*(.text*)
		*(.gnu.linkonce.t*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	__text_end__ = .;
	__bss_vma_start = .;
	.bss : {
		*(.dynbss*)
		*(.bss)
		*(.gnu.linkonce.b*.)
		*(COMMON)					/* libc requires these for uninitialized objects, include any heap uninit vars*/
		. = ALIGN(4);
	} > EWRAM = 0xff
	__bss_vma_end = .;
	__sbss_start__ = .;
	.sbss : {
		*(.sbss*)
		. = ALIGN(4);
	}> EWRAM = 0xff
	__sbss_vma_end = .;
	__data_start__ = .;
	.data : {
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	__data_end__ = .;
	__rodata_start__ = .;
	.rodata : { 
		*(.rdata)
		*(.rodata)
		*(.rodata.*)
    	*(.gnu.linkonce.r.*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	__rodata_end__ = .;
	__eh_frame_start__ = .;
	.eh_frame : {
		*(.eh_frame*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	__eh_frame_end__ = .;
	__jcr_start__ = .;
	.jcr : {
		*(.jcr*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	__jcr_end__ = .;
	__vfp11_veneer_start__ = .;
	.vfp11_veneer : {
		*(.vfp11_veneer*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	__vfp11_veneer_end__ = .;
	__exidx_start = .;
    .ARM.exidx   : {		/* newlib C++ require these constructors */
		*(.ARM.exidx)
		*(.ARM.exidx*)
		*(.gnu.linkonce.armexidx.)
		*(.gnu.linkonce.armexidx.*)
	} > EWRAM = 0xff
    __exidx_end = .;
	__v4_bx_start__ = .;
	.v4_bx : {
		*(.v4_bx*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	__v4_bx_end__ = .;
	__iplt_start__ = .;
	.iplt : {
		*(.iplt*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	__iplt_end__ = .;
	__igot_plt_start__ = .;
	.igot.plt : {
		*(.igot.plt*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	__igot_plt_end__ = .;
	__rel_dyn_start__ = .;
	.rel.dyn : { 
		*(.rel.dyn*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	__rel_dyn_end__ = .;
	__fini_start__ = .;
	.fini : {
		*(.fini*)
		. = ALIGN(4);
	} > EWRAM = 0xff
	__fini_end__ = .;
	__lib__end__ = __fini_end__;
	__itcm_vma_start = .;
	.itcm : {
		KEEP (*(.itcm*))
		. = ALIGN(4);
	} > ITCM AT > EWRAM
	__itcm_vma_end__ = .;
	__dtcm_vma_start = .;
	.dtcm : {
		KEEP (*(.dtcm*))
		. = ALIGN(4);
	} > DTCM AT > EWRAM
	__dtcm_vma_end__ = .;
	__vectors_vma_start = .;
	.vectors : {
		KEEP (*(.vectors*))
		. = ALIGN(4);
	} > VECTORS AT > EWRAM
	__vectors_vma_end__ = .;
	__vma_stub_end__ = __lib__end__ + SIZEOF(.itcm) + SIZEOF(.dtcm) + SIZEOF(.vectors);
}