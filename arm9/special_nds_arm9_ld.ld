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
	DTCM : ORIGIN = 0x0b000000, LENGTH = 16K	/*vector handlers are here in the 0x10 area*/
	ITCM : ORIGIN = 0x01000000, LENGTH = 32K
	GBAROM : ORIGIN = 0x08000000, LENGTH = 32M
	EWRAM : ORIGIN = 0x02000000, LENGTH = 4M
	ARM9SHAREDWRAM : ORIGIN = 0x03000000, LENGTH = 32K
}

_itcm_start = ORIGIN(ITCM);
_itcm_end = ORIGIN(ITCM)	+	LENGTH(ITCM);

_ewram_start = ORIGIN(EWRAM);
_ewram_end = ORIGIN(EWRAM)	+	LENGTH(EWRAM);

_gba_start = ORIGIN(GBAROM);
_gba_end = ORIGIN(GBAROM)	+	LENGTH(GBAROM);

_dtcm_start = ORIGIN(DTCM);
_dtcm_end = ORIGIN(DTCM)	+	LENGTH(DTCM);

_arm9sharedwram_start = ORIGIN(ARM9SHAREDWRAM);
_arm9sharedwram_end = ORIGIN(ARM9SHAREDWRAM)	+	LENGTH(ARM9SHAREDWRAM);

/* ARM9 Specific */
_arm9_irqcheckbits = ORIGIN(DTCM) + 0x3FF8;	/* __irq_flags */
_arm9_irqhandler = ORIGIN(DTCM) + 0x3FFC;	/* __irq_vector */

/* ARM ABI says stacks are full decreasing */
sp_SVC	= _dtcm_end  - 0x200;	/* reset, swi exception*/
sp_IRQ	= sp_SVC - 0x200;	/*irq exception*/
sp_FIQ	= sp_IRQ - 0x200;	/* fiq exception */
sp_ABT	= sp_FIQ - 0x200;	/* prefetch / data aborts exception */
sp_UND	= sp_ABT - 0x200;	/* undefined exception */
sp_USR	= sp_UND - 0x200;	/* usr / sys stacks */

ENTRY (_start)
SECTIONS
{
	.init : ALIGN(4) { __init_start__ = .;
		*(.init*)
	} > EWRAM = 0xff
	__init_end__ = .;
	.preinit_array : ALIGN(4) {
		__preinit_array_start__ = .;
		*(.preinit_array*)
	} > EWRAM = 0xff
	__preinit_array_end__ = .;
	.init_array : ALIGN(4) { 
		__init_array_start__ = .;
		*(.init_array*)
	} > EWRAM = 0xff
	init_array_end__ = .;
	.fini_array : ALIGN(4) {
		__fini_array_start__ = .;
		*(.fini_array*)
	} > EWRAM = 0xff
	__fini_array_end__ = .;
	.text : ALIGN(4) {
		__text_start__ = .;
		*(.text*)
		*(.text)
		*(.gnu.linkonce.t*)
	} > EWRAM = 0xff
	__text_end__ = .;
	.bss : ALIGN(4) {
		__bss_vma_start = .;
		*(COMMON)					/* libc requires these for uninitialized objects, include any heap uninit vars*/
		*(.dynbss*)
		*(.bss*)
		*(.bss)
		*(.gnu.linkonce.b*.)
	} > EWRAM = 0xff
	__bss_vma_end = .;
	.sbss : ALIGN(4) {
		__sbss_start__ = .;
		*(.sbss*)
		*(.sbss)
	}> EWRAM = 0xff
	__sbss_vma_end = .;
	.data : ALIGN(4) {
		__data_start__ = .;
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
	} > EWRAM = 0xff
	__data_end__ = .;
	.rodata : ALIGN(4) {
		__rodata_start__ = .;
		*(.rdata)
		*(.rodata)
		*(.rodata.*)
    	*(.gnu.linkonce.r.*)
	} > EWRAM = 0xff
	__rodata_end__ = .;
	.eh_frame : ALIGN(4) {
		__eh_frame_start__ = .;
		*(.eh_frame*)
	} > EWRAM = 0xff
	__eh_frame_end__ = .;
	.jcr : ALIGN(4) {
		__jcr_start__ = .;
		*(.jcr*)
	} > EWRAM = 0xff
	__jcr_end__ = .;
	.vfp11_veneer : ALIGN(4) {
		__vfp11_veneer_start__ = .;
		*(.vfp11_veneer*)
	} > EWRAM = 0xff
	__vfp11_veneer_end__ = .;
    .ARM.exidx : ALIGN(4) {
		__exidx_start = .;
		*(.ARM.exidx)
		*(.ARM.exidx*)
		*(.gnu.linkonce.armexidx.)
		*(.gnu.linkonce.armexidx.*)
	} > EWRAM = 0xff
    __exidx_end = .;
	.v4_bx : ALIGN(4) {
		__v4_bx_start__ = .;
		*(.v4_bx)
		*(.v4_bx*)
	} > EWRAM = 0xff
	__v4_bx_end__ = .;
	.iplt : ALIGN(4) {
		__iplt_start__ = .;
		*(.iplt*)
	} > EWRAM = 0xff
	__iplt_end__ = .;
	.igot.plt : ALIGN(4) {
		__igot_plt_start__ = .;
		*(.igot.plt*)
	} > EWRAM = 0xff
	__igot_plt_end__ = .;
	.rel.dyn : ALIGN(4) {
		__rel_dyn_start__ = .;
		*(.rel.dyn*)
	} > EWRAM = 0xff
	__rel_dyn_end__ = .;
	.fini : ALIGN(4) {
		__fini_start__ = .;
		*(.fini)
		*(.fini*)
	} > EWRAM = 0xff
	__fini_end__ = .;
	__lib__end__ = __fini_end__;
	. = ALIGN(4);
	__itcm_vma_start = .;
	.itcm : ALIGN(4) {
		KEEP (*(.itcm*))
		_vector_start = .;
		KEEP (*(.vectors*))
		_vector_end = .;
	} > ITCM AT > EWRAM
	__itcm_vma_end__ = .;
	__dtcm_vma_start = .;
	.dtcm : ALIGN(4) {
		KEEP (*(.dtcm*))
	} > DTCM AT > EWRAM
	__dtcm_vma_end__ = .;
	.arm9sharedwram : ALIGN(4) {
		___arm9sharedwram_vma_start = .;
		KEEP (*(.arm9sharedwram*))
	} > ARM9SHAREDWRAM AT > EWRAM
	___arm9sharedwram_vma_end__ = .;
	__vma_stub_end__ = __lib__end__ + 4 + SIZEOF(.itcm) + SIZEOF(.dtcm) + SIZEOF(.arm9sharedwram);
}