/*Coto: 11 / Sept / 2023: This current linker guarantees to boot TGDS ARM7i binaries through Unlaunch*/ 
/* it's got to do with how data is initialized & made relative, as opposed to the standard linker layout which does NOT work and it should*/
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

PHDRS {
	crt0  PT_LOAD FLAGS(7);
	arm7  PT_LOAD FLAGS(7);
	arm7i PT_LOAD FLAGS(0x100007);
}


MEMORY {
	ewram  : ORIGIN = 0x03800000, LENGTH = 64K
	rom    : ORIGIN = 0x08000000, LENGTH = 32M
	iwram  : ORIGIN = 0x03800000, LENGTH = 64K	/* copying sections is WIP on TWL */

	twl_ewram : ORIGIN = 0x02e80000, LENGTH = 512K - 64K
	twl_iwram : ORIGIN = 0x03000000, LENGTH = 256K
}

__iwram_start	=	ORIGIN(iwram);
__iwram_top	=	ORIGIN(iwram)+ LENGTH(iwram);

__irq_flags	=	0x04000000 - 8;
__irq_flagsaux	=	0x04000000 - 0x40;
__irq_vector	=	0x04000000 - 4;

_iwram_start =	__iwram_start;
_iwram_end = __iwram_top;

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

	.twl :
	{
		__arm7i_lma__ = LOADADDR(.twl);
		__arm7i_start__ = .;
		__arm7i_end__ = .;
	} >twl_iwram AT>twl_ewram :arm7i

	.twl_bss ALIGN(4) (NOLOAD) :
	{
		__twl_bss_start__ = .;
		__twl_bss_end__ = .;
	} >twl_iwram :NONE
	
	.text :
	{
		KEEP (*(SORT_NONE(.init)))
		*(.plt)
		*(.text .stub .text.* .gnu.linkonce.t.*)
		KEEP (*(.text.*personality*))
		/* .gnu.warning sections are handled specially by elf32.em.  */
		*(.gnu.warning)
		*(.glue_7t) *(.glue_7) *(.vfp11_veneer)
		. = ALIGN(4);  /* REQUIRED. LD is flaky without it. */
		
		*(.twl)
		*.twl*(.text .stub .text.* .gnu.linkonce.t.*)
		*.twl*(.rodata)
		*.twl*(.roda)
		*.twl*(.rodata.*)
		*.twl*(.data)
		*.twl*(.data.*)
		*.twl*(.gnu.linkonce.d*)
		. = ALIGN(4);  /* REQUIRED. LD is flaky without it. */
		
	} >iwram AT>ewram :arm7
	
	.fini           :
	{
		KEEP (*(.fini))
	} >iwram AT>ewram

	.rodata :
	{
		*(.rodata)
		*all.rodata*(*)
		*(.roda)
		*(.rodata.*)
		*(.gnu.linkonce.r*)
		SORT(CONSTRUCTORS)
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >iwram AT>ewram

	.ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >iwram AT>ewram

	.ARM.exidx   : {
		__exidx_start = .;
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		__exidx_end = .;
	 } >iwram AT>ewram

/* Ensure the __preinit_array_start label is properly aligned.  We
   could instead move the label definition inside the section, but
   the linker would then create the section even if it turns out to
   be empty, which isn't pretty.  */
	.preinit_array     : {
		. = ALIGN(32 / 8);
		PROVIDE (__preinit_array_start = .);
		KEEP (*(.preinit_array))
		PROVIDE (__preinit_array_end = .);
	} >iwram AT>ewram

	.init_array     : {
		PROVIDE (__init_array_start = .);
		KEEP (*(.init_array))
		PROVIDE (__init_array_end = .);
	} >iwram AT>ewram

	.fini_array     : {
		PROVIDE (__fini_array_start = .);
		KEEP (*(.fini_array))
		PROVIDE (__fini_array_end = .);
	} >iwram AT>ewram

	.ctors :
	{
	/* gcc uses crtbegin.o to find the start of the constructors, so
		we make sure it is first.  Because this is a wildcard, it
		doesn't matter if the user does not actually link against
		crtbegin.o; the linker won't look for a file to match a
		wildcard.  The wildcard also means that it doesn't matter which
		directory crtbegin.o is in.  */
		KEEP (*crtbegin.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >iwram AT>ewram

	.dtors :
	{
		KEEP (*crtbegin.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >iwram AT>ewram

	.eh_frame :
	{
		KEEP (*(.eh_frame))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >iwram AT>ewram

	.gcc_except_table :
	{
		*(.gcc_except_table)
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >iwram AT>ewram
	.jcr            : { KEEP (*(.jcr)) } >iwram AT>ewram
	.got            : { *(.got.plt) *(.got) } >iwram AT>ewram

	.data ALIGN(4) : 	{
		__data_start = ABSOLUTE(.);
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		CONSTRUCTORS
		. = ALIGN(4);
		*(.dynbss)
		*(.gnu.linkonce.b*)
		*(.bss*)
		*(COMMON)
		*(.twl_bss)
		*.twl.*(.dynbss)
		*.twl.*(.gnu.linkonce.b*)
		*.twl.*(.bss*)
		*.twl.*(COMMON)
		. = ALIGN(4);
		__data_end = ABSOLUTE(.) ;
	} >iwram AT>ewram

	.bss ALIGN(4) (NOLOAD) :
	{
		__bss_start = ABSOLUTE(.);
		__bss_start__ = ABSOLUTE(.);
		. = ALIGN(4);    /* REQUIRED. LD is flaky without it. */
		__bss_end__ = ABSOLUTE(.);
		__end__ = ABSOLUTE(.);
	} >iwram
	
	/* stubbed for ARM7i binaries crt0 in NTR mode, but actually used in TWL mode */
	__arm7i_lma__ = __end__ + 4;
	__arm7i_start__ = __arm7i_lma__ + 4;
	__arm7i_end__ = __arm7i_start__ + 4;
	
	__arm7_lma__ = __arm7i_end__ + 4;
	__arm7_start__ = __arm7_lma__ + 4;
	__arm7_end__ = __arm7_start__ + 4;
	
	/* Stabs debugging sections.  */
	.stab 0 : { *(.stab) }
	.stabstr 0 : { *(.stabstr) }
	.stab.excl 0 : { *(.stab.excl) }
	.stab.exclstr 0 : { *(.stab.exclstr) }
	.stab.index 0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment 0 : { *(.comment) }
	/*	DWARF debug sections.
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
	.debug_info     0 : { *(.debug_info) }
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
	/* These must appear regardless of  .  */
}