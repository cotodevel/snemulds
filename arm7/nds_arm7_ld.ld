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

/* Coto: linker file for NDS ARM7 processor, could be adapted for DSi-3DS */

/* attributes can actually help in debugging because if you try to move functions to DTCM, for example, compiler will rant since DTCM is DATA only path and not execution*/
MEMORY
{
	IWRAM	(rwx)	: ORIGIN = 0x03800000, LENGTH = 64K
}

_iwram_start = ORIGIN(IWRAM);
_iwram_end = ORIGIN(IWRAM)	+	LENGTH(IWRAM);

/* ARM7 Specific */
_arm7_irqcheckbits = 0x04000000 - 8;	/* __irq_flags */
_arm7_irqhandler = 0x04000000 - 4;	/* __irq_vector */

/* ARM ABI says stacks are full decreasing */
sp_SVC	= _iwram_end - 0x200;
sp_IRQ	= sp_SVC - 0x200;
sp_FIQ	= sp_IRQ - 0x200;
sp_ABT	= sp_FIQ - 0x200;
sp_UND	= sp_ABT - 0x200;
sp_USR	= sp_UND - 0x200;
sp_SYS	= sp_USR;

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm","elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	.init : SUBALIGN(4)	{ __init_start__ = .;
		*(.init*)
	} > IWRAM = 0xff
	__init_end__ = .;
	__preinit_array_start__ = .;
	.preinit_array : SUBALIGN(4)	{ 
		*(.preinit_array*)
	} > IWRAM = 0xff
	__preinit_array_end__ = .;
	__init_array_start__ = .;
	.init_array : SUBALIGN(4)	{ 
		*(.init_array*)
	} > IWRAM = 0xff
	init_array_end__ = .;
	__fini_array_start__ = .;
	.fini_array : SUBALIGN(4)	{
		*(.fini_array*)
	} > IWRAM = 0xff
	__fini_array_end__ = .;
	__text_start__ = .;
	.text : SUBALIGN(4)	{
		*(.text)
		*(.text*)
		*(.gnu.linkonce.t*)
	} > IWRAM = 0xff
	__text_end__ = .;
	__bss_vma_start = .;
	.bss : SUBALIGN(4)	{
		*(.dynbss*)
		*(.bss)
		*(.gnu.linkonce.b*.)
		*(COMMON)					/* libc requires these for uninitialized objects, include any heap uninit vars*/
	} > IWRAM = 0xff
	__bss_vma_end = .;
	__sbss_start__ = .;
	.sbss : SUBALIGN(4)	{
		*(.sbss*)
	}> IWRAM = 0xff
	__sbss_vma_end = .;
	__data_start__ = .;
	.data : SUBALIGN(4)	{
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
	} > IWRAM = 0xff
	__data_end__ = .;
	__rodata_start__ = .;
	.rodata : SUBALIGN(4)	{ 
		*(.rdata)
		*(.rodata)
		*(.rodata.*)
    	*(.gnu.linkonce.r.*)
	} > IWRAM = 0xff
	__rodata_end__ = .;
	__eh_frame_start__ = .;
	.eh_frame : SUBALIGN(4)	{
		*(.eh_frame*)
	} > IWRAM = 0xff
	__eh_frame_end__ = .;
	__jcr_start__ = .;
	.jcr : SUBALIGN(4)	{
		*(.jcr*)
	} > IWRAM = 0xff
	__jcr_end__ = .;
	__vfp11_veneer_start__ = .;
	.vfp11_veneer : SUBALIGN(4)	{
		*(.vfp11_veneer*)
	} > IWRAM = 0xff
	__vfp11_veneer_end__ = .;
	__exidx_start = .;
    .ARM.exidx   : SUBALIGN(4)	{		/* newlib C++ require these constructors */
		*(.ARM.exidx)
		*(.ARM.exidx*)
		*(.gnu.linkonce.armexidx.)
		*(.gnu.linkonce.armexidx.*)
	} > IWRAM = 0xff
    __exidx_end = .;
	__v4_bx_start__ = .;
	.v4_bx : SUBALIGN(4)	{
		*(.v4_bx*)
	} > IWRAM = 0xff
	__v4_bx_end__ = .;
	__iplt_start__ = .;
	.iplt : SUBALIGN(4)	{
		*(.iplt*)
	} > IWRAM = 0xff
	__iplt_end__ = .;
	__igot_plt_start__ = .;
	.igot.plt : SUBALIGN(4)	{
		*(.igot.plt*)
	} > IWRAM = 0xff
	__igot_plt_end__ = .;
	__rel_dyn_start__ = .;
	.rel.dyn : SUBALIGN(4)	{ 
		*(.rel.dyn*)
	} > IWRAM = 0xff
	__rel_dyn_end__ = .;
	__fini_start__ = .;
	.fini : SUBALIGN(4)	{
		*(.fini*)
	} > IWRAM = 0xff
	__fini_end__ = .;
	__lib__end__ = __fini_end__;
	__vma_stub_end__ = __lib__end__;
	
	/* stubbed for ARM7i binaries crt0 in NTR mode, but actually used in TWL mode */
	__arm7i_lma__ = __vma_stub_end__ + 4;
	__arm7i_start__ = __arm7i_lma__ + 4;
	__arm7i_end__ = __arm7i_start__ + 4;
	
	__arm7_lma__ = __arm7i_end__ + 4;
	__arm7_start__ = __arm7_lma__ + 4;
	__arm7_end__ = __arm7_start__ + 4;
}