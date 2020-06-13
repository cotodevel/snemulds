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
	PROVIDE (__init_start__ = ABSOLUTE(.));
	.init : SUBALIGN(4) { 			/* Entrypoint */
		KEEP(*(.init*))
		FILL(0x4);
		PROVIDE (__init_end__ = ABSOLUTE(.));
		. = ALIGN(4 + (. & (4 - 1)));
	} > EWRAM = 0xff
	
	/* Text section must be first, so the DLDI patcher code can read DLDI section properly */
	PROVIDE (__text_start__ = ABSOLUTE(.));
	.text : SUBALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		KEEP(*(.text*))	
		FILL(0x4);
		PROVIDE (__text_end__ = ABSOLUTE(.));
		. = ALIGN(4 + (. & (4 - 1)));
	} > EWRAM = 0xff
	
	PROVIDE (__itcm_vma_start = ABSOLUTE(.));
	.itcm : SUBALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		KEEP (*(.itcm*))
		FILL(0x4);
		PROVIDE (__itcm_vma_end__ = ABSOLUTE(.));
		. = ALIGN(4 + (. & (4 - 1)));
	} > ITCM AT > EWRAM
	
	PROVIDE (__dtcm_vma_start = ABSOLUTE(.));
	.dtcm : SUBALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		KEEP (*(.dtcm*))
		FILL(0x4);
		PROVIDE (__dtcm_vma_end__ = ABSOLUTE(.));
		. = ALIGN(4 + (. & (4 - 1)));
	} > DTCM AT > EWRAM
	
	PROVIDE (___arm9sharedwram_vma_start = ABSOLUTE(.));
	.arm9sharedwram : SUBALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		KEEP (*(.arm9sharedwram*))
		FILL(0x4);
		PROVIDE (___arm9sharedwram_vma_end__ = ABSOLUTE(.));
		. = ALIGN(4 + (. & (4 - 1)));
	} > ARM9SHAREDWRAM AT > EWRAM
	
	
	PROVIDE (__rodata_start__ = ABSOLUTE(.));
	.rodata : SUBALIGN(4) {			/* used by TGDS-Newlib C/C++ code */
		*(.rdata)
		*(.rodata)
		*(.rodata.*)
    	*(.gnu.linkonce.r.*)
		FILL(0x4);
		PROVIDE (__rodata_end__ = ABSOLUTE(.));
		. = ALIGN(4 + (. & (4 - 1)));
	} > EWRAM = 0xff
	
	PROVIDE (__bss_vma_start = ABSOLUTE(.));
	.bss : SUBALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		*(.dynbss*)
		*(.bss*)
		*(.bss)
		KEEP(*(COMMON))					/* libc requires these for uninitialized objects, include any heap uninit vars*/
		*(.gnu.linkonce.b*.)
		FILL(0x4);
		PROVIDE (__bss_vma_end = ABSOLUTE(.));
		. = ALIGN(4 + (. & (4 - 1)));
	} > EWRAM = 0xff
	
	PROVIDE (__exidx_start = ABSOLUTE(.));
	.ARM.exidx : SUBALIGN(4) {			/* not used at all, but some code rely on ARM debugging symbols */
		*(.ARM.exidx)
		*(.ARM.exidx*)
		*(.gnu.linkonce.armexidx.)
		*(.gnu.linkonce.armexidx.*)
		FILL(0x4);
		PROVIDE (__exidx_end = ABSOLUTE(.));
		. = ALIGN(4 + (. & (4 - 1)));
	} > EWRAM = 0xff
	
	PROVIDE (__data_start__ = ABSOLUTE(.));
	.data : SUBALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		*(.gnu.linkonce.t*)
		FILL(0x4);
		PROVIDE (__data_end__ = ABSOLUTE(.));
		. = ALIGN(4 + (. & (4 - 1)));
	} > EWRAM = 0xff
	
	PROVIDE (__fini_start__ = ABSOLUTE(.));
	.fini : SUBALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		*(.fini)
		*(.fini*)
		FILL(0x4);
		PROVIDE (__fini_end__ = ABSOLUTE(.));
		. = ALIGN(4 + (. & (4 - 1)));
	} > EWRAM = 0xff
	
	/*
     * Stabs le debugging sections.
     *
     */

    .stab          0 : { *(.stab) }	> EWRAM = 0xff
    .stabstr       0 : { *(.stabstr) }	> EWRAM = 0xff
    .stab.excl     0 : { *(.stab.excl) }	> EWRAM = 0xff
    .stab.exclstr  0 : { *(.stab.exclstr) }	> EWRAM = 0xff
    .stab.index    0 : { *(.stab.index) }	> EWRAM = 0xff
    .stab.indexstr 0 : { *(.stab.indexstr) }	> EWRAM = 0xff
    .comment       0 : { *(.comment) }	> EWRAM = 0xff
    /* DWARF debug sections.
       Symbols in the DWARF debugging sections are relative to the beginning
       of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }	> EWRAM = 0xff
    .line           0 : { *(.line) }	> EWRAM = 0xff
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }	> EWRAM = 0xff
    .debug_sfnames  0 : { *(.debug_sfnames) }	> EWRAM = 0xff
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }	> EWRAM = 0xff
    .debug_pubnames 0 : { *(.debug_pubnames) }	> EWRAM = 0xff
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }	> EWRAM = 0xff
    .debug_abbrev   0 : { *(.debug_abbrev) }	> EWRAM = 0xff
    .debug_line     0 : { *(.debug_line) }	> EWRAM = 0xff
    .debug_frame    0 : { *(.debug_frame) }	> EWRAM = 0xff
    .debug_str      0 : { *(.debug_str) }	> EWRAM = 0xff
    .debug_loc      0 : { *(.debug_loc) }	> EWRAM = 0xff
    .debug_macinfo  0 : { *(.debug_macinfo) }	> EWRAM = 0xff
    /* SGI/MIPS DWARF 2 extensions */
    .debug_weaknames 0 : { *(.debug_weaknames) }	> EWRAM = 0xff
    .debug_funcnames 0 : { *(.debug_funcnames) }	> EWRAM = 0xff
    .debug_typenames 0 : { *(.debug_typenames) }	> EWRAM = 0xff
    .debug_varnames  0 : { *(.debug_varnames) }		> EWRAM = 0xff
	
	PROVIDE (__lib__end__ = __fini_end__ + 0x4);
	. = . + 0x4;
	PROVIDE (__vma_stub_end__ = __lib__end__ + 0x4);
}