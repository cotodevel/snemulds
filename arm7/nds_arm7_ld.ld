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
sp_SVC	= _iwram_end - 0x300;	/* reset, swi exception*/
sp_IRQ	= sp_SVC - 0x300;	/*irq exception*/
sp_FIQ	= sp_IRQ - 0x300;	/* fiq exception */
sp_ABT	= sp_FIQ - 0x300;	/* prefetch / data aborts exception */
sp_UND	= sp_ABT - 0x300;	/* undefined exception */
sp_USR	= sp_UND - 0x300;	/* usr / sys stacks */

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm","elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = ALIGN(4);
	.init      :	SUBALIGN(4)
	{
		KEEP(*(.init))
	} > IWRAM = 0xFF
	
	.text      :	SUBALIGN(4)
	{
		*(.stub)
		/* .gnu.warning sections are handled specially by elf32.em.  */
		*(.gnu.warning)
		*(.gnu.linkonce.t*)
		*(.glue_7t) *(.glue_7)
		KEEP(*(COMMON))
		KEEP(*(.text))
		KEEP(*(.text.*))
	} > IWRAM = 0xFF
	
	.data : SUBALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		PROVIDE (__data_start__ = ABSOLUTE(.));
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		*(.gnu.linkonce.t*)
		PROVIDE (__data_end__ = ABSOLUTE(.));
	} > IWRAM = 0xff
	
	.rodata : SUBALIGN(4) {			/* used by TGDS-Newlib C/C++ code */
		PROVIDE (__rodata_start__ = ABSOLUTE(.));
		*(.rdata)
		*(.rodata)
		*(.rodata.*)
    	*(.gnu.linkonce.r.*)
		PROVIDE (__rodata_end__ = ABSOLUTE(.));
	} > IWRAM = 0xff
	
	.bss (NOLOAD): SUBALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		PROVIDE (__bss_vma_start = ABSOLUTE(.));
		*(.dynbss*)
		*(.bss*)
		*(.bss)
		*(.gnu.linkonce.b*.)
		PROVIDE (__bss_vma_end = ABSOLUTE(.));
	} > IWRAM
	
	.ARM.exidx : SUBALIGN(4) {			/* not used at all, but some code rely on ARM debugging symbols */
		PROVIDE (__exidx_start = ABSOLUTE(.));
		*(.ARM.exidx)
		*(.ARM.exidx*)
		*(.gnu.linkonce.armexidx.)
		*(.gnu.linkonce.armexidx.*)
		PROVIDE (__exidx_end = ABSOLUTE(.));
	} > IWRAM = 0xff
	
	/*
     * Stabs le debugging sections.
     *
     */

    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    /* DWARF debug sections.
       Symbols in the DWARF debugging sections are relative to the beginning
       of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
	
	.fini : SUBALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		PROVIDE (__fini_start__ = ABSOLUTE(.));
		*(.fini)
		*(.fini*)
		PROVIDE (__fini_end__ = ABSOLUTE(.));
		FILL(0x40);
	} > IWRAM = 0xff
	
	PROVIDE (__lib__end__ = __fini_end__);
	. = ALIGN(4);
	PROVIDE (__vma_stub_end__ = __lib__end__ + 0x40);
}
