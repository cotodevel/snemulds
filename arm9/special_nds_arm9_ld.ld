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
	.init : ALIGN(4) { 			/* Entrypoint */
		PROVIDE (__init_start__ = ABSOLUTE(.));
		KEEP(*(.init*))
		PROVIDE (__init_end__ = ABSOLUTE(.));
	} > EWRAM = 0xff
	
	. = ALIGN(4);
	
	.data : ALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		PROVIDE (__data_start__ = ABSOLUTE(.));
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		*(.gnu.linkonce.t*)
		PROVIDE (__data_end__ = ABSOLUTE(.));
	} > EWRAM = 0xff
	
	. = ALIGN(4);
	
	.rodata : ALIGN(4) {			/* used by TGDS-Newlib C/C++ code */
		PROVIDE (__rodata_start__ = ABSOLUTE(.));
		*(.rdata)
		*(.rodata)
		*(.rodata.*)
    	*(.gnu.linkonce.r.*)
		PROVIDE (__rodata_end__ = ABSOLUTE(.));
	} > EWRAM = 0xff
	
	. = ALIGN(4);
	
	.bss (NOLOAD): {				/* used by TGDS-Newlib C/C++ code */
		PROVIDE (__bss_vma_start = ABSOLUTE(.));
		*(.dynbss*)
		*(.bss*)
		*(.bss)
		*(.gnu.linkonce.b*.)
		PROVIDE (__bss_vma_end = ABSOLUTE(.));
	} > EWRAM = 0xff
	
	. = ALIGN(4);
	
	.text : ALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		PROVIDE (__text_start__ = ABSOLUTE(.));
		KEEP(*(.text*))
		KEEP(*(COMMON))					/* libc requires these for uninitialized objects, include any heap uninit vars*/
		PROVIDE (__text_end__ = ABSOLUTE(.));
	} > EWRAM = 0xff
	
	. = ALIGN(4);
	
	.ARM.exidx : ALIGN(4) {			/* not used at all, but some code rely on ARM debugging symbols */
		PROVIDE (__exidx_start = ABSOLUTE(.));
		*(.ARM.exidx)
		*(.ARM.exidx*)
		*(.gnu.linkonce.armexidx.)
		*(.gnu.linkonce.armexidx.*)
		PROVIDE (__exidx_end = ABSOLUTE(.));	
	} > EWRAM = 0xff
	
	. = ALIGN(4);
	
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
	
	. = ALIGN(4);
	
	.fini : ALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		PROVIDE (__fini_start__ = ABSOLUTE(.));
		*(.fini)
		*(.fini*)
		PROVIDE (__fini_end__ = ABSOLUTE(.));
		FILL(0x40);
	} > EWRAM = 0xff
	
	. = ALIGN(4);
	
	PROVIDE (__lib__end__ = __fini_end__ + 0x40);
	
	__itcm_vma_start = .;
	.itcm : ALIGN(4) {
		_vectors_start = .;
		__vectors_vma_start = _vectors_start;
		KEEP (*(.vectors*))
		_vectors_end = .;
		KEEP (*(.itcm*))
	} > ITCM AT > EWRAM
	__itcm_vma_end__ = .;
	
	. = ALIGN(4);
	
	__dtcm_vma_start = .;
	.dtcm : ALIGN(4) {
		KEEP (*(.dtcm*))
	} > DTCM AT > EWRAM
	__dtcm_vma_end__ = .;
	
	. = ALIGN(4);
	
	___arm9sharedwram_vma_start = .;
	.arm9sharedwram : ALIGN(4) {
		KEEP (*(.arm9sharedwram*))
	} > ARM9SHAREDWRAM AT > EWRAM
	___arm9sharedwram_vma_end__ = .;
	
	PROVIDE (__vma_stub_end__ = __lib__end__ + SIZEOF(.itcm) + SIZEOF(.dtcm) + SIZEOF(.arm9sharedwram));
}