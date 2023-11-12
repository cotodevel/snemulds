/*Coto: 11 / Sept / 2023: This current linker guarantees to boot TGDS ARM7i binaries through Unlaunch*/ 
/* it's got to do with how data is initialized & made relative, as opposed to the standard linker layout which does NOT work and it should*/
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)


MEMORY {
	ewram  : ORIGIN = 0x02380000, LENGTH = 64K
	rom    : ORIGIN = 0x08000000, LENGTH = 32M
	iwram  : ORIGIN = 0x03800000, LENGTH = 64K

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

__arm7_start__ = __iwram_start;
__arm7_end__ = __iwram_top;
		
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
	__arm7_lma__ = LOADADDR(.text);
	
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
		
		*(.itcm)
		/*
		*(.twl)
		*.twl*(.text .stub .text.* .gnu.linkonce.t.*)
		*.twl*(.rodata)
		*.twl*(.roda)
		*.twl*(.rodata.*)
		*.twl*(.data)
		*.twl*(.data.*)
		*.twl*(.gnu.linkonce.d*)
		*/
		
		. = ALIGN(4);  /* REQUIRED. LD is flaky without it. */
		
	} >iwram AT>ewram 
	
	.fini           :
	{
		KEEP (*(.fini))
		
		. = ALIGN(4);  /* REQUIRED. LD is flaky without it. */
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
		
		__bss_start = ABSOLUTE(.);
		__bss_start__ = ABSOLUTE(.);
		*(.dynbss)
		*(.gnu.linkonce.b*)
		*(.bss*)
		*(COMMON)
		*(.twl_bss)
		*.twl.*(.dynbss)
		*.twl.*(.gnu.linkonce.b*)
		*.twl.*(.bss*)
		__bss_end__ = ABSOLUTE(.);
		__end__ = ABSOLUTE(.);
		
		*.twl.*(COMMON)
		. = ALIGN(4);
		__data_end = ABSOLUTE(.) ;
	} >iwram AT>ewram

	
}