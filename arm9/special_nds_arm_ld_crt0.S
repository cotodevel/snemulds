
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


/****************************************************************************/

/*
 *	linux/arch/arm/mach-nds/head.S
 *
 *	Copyright (C) 2005, Malcolm Parsons <pepsiman@blueyonder.co.uk>
 *
 *  uClinux kernel startup code for NDS
 *  which has no proper bootloader for linux startup
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifdef ARM7
.arch	armv4
.cpu	arm7tdmi
#endif

#ifdef ARM9
.arch	armv5te
.cpu	arm946e-s
#endif
.arm
.section ".init"
.global _start
_start:
@---------------------------------------------------------------------------------
	@Info from: http://problemkaputt.de/gbatek.htm
	@DS BIOS RAM Usage
	@Below memory at 27FFxxxh is mirrored to 23FFxxxh (on retail consoles with 4MB RAM), however, it should be accessed via address 27FFxxxh 
	@(for compatibility with debug consoles with 8MB RAM). Accessing it via mirrors at 2FFFxxxh is also valid (this is done by DSi enhanced games; 
	@even when running in non-DSi mode; this allows DSi games to use the same memory addresses in NDS and DSi mode).
	
	@Coto: vectors @ 0xffff0000 is bios area. Accidental undefined,dataabort,SWI,etc during MPU setup is expected. 
	@Thus, BIOS exceptions must be handled from such DS BIOS RAM stacks.
	
	@Also, MPU exceptions are triggered in USR mode (0x10&psr), SYS mode exceptions are ignored. (0x1f&psr)
	
	@disable exceptions when MPU setup. Enter SYS mode.
	ldr r0,=0x04000208
	mov r1,#0
	str r1,[r0]
	

	#ifdef ARM9
	mov	r0, #0xdf		@ sys
	msr	cpsr, r0
	ldr 	sp, =(0x027FF000 + 0x1000)
	
	push {r0-r3,lr}
	bl setSnemulDSSpecial0xFFFF0000MPUSettings
	pop {r0-r3,lr}
	#endif
	
	mov	r0, #0x12		@ irq
	msr	cpsr, r0
	ldr	sp, =sp_IRQ

	mov	r0, #0x13		@ svc dtcm stacks, irq enable
	msr	cpsr, r0
	ldr	sp, =sp_SVC		
	
	@ The debug handler is called: 
	@on undefined instruction exceptions, 
	@on data/prefetch aborts (caused by the protection unit), 
	@on FIQ (possibly caused by hardware debuggers). 
	@It is also called by accidental software-jumps to the reset vector, and by unused SWI numbers within range 0..1Fh.
	
	mov 	r3 , #0x1b	@ undef dtcm stacks, irq enable
	msr     CPSR, r3
	ldr 	sp, =sp_UND
	
	mov 	r3 , #0x17	@ dataabt dtcm stacks, irq enable
	msr     CPSR,r3 
	ldr 	sp, =sp_ABT
	
	mov 	r3 , #0x11	@FIRQ mode, irq enable
	msr     CPSR, r3
	ldr 	sp, =sp_FIQ
	
	mov	r0, #0x1F		@ usr/sys dtcm stacks, irq enable (ignore all exceptions usermode must be excluded here)
	msr	cpsr, r0
	ldr	sp, =sp_USR
	
	#ifdef ARM9
	bl DrainWriteBuffer
	/* copy the VMA sections to each LMA sections for hardware RAM not initialized at self boot crt0 stage*/
	ldr                 r1,=__itcm_vma_start                 /* point our source register to the start of the .itcm section (VMA)*/
	ldr                 r2,=_itcm_start                 /* point our destination register to the I-TCM dedicated to our writable data (LMA)*/
	ldr                 r3,=_itcm_end                  /* point r3 after the last byte we will be writing (LMA)*/
	bl                  copy  
	
	bl DrainWriteBuffer
	ldr                 r1,=__dtcm_vma_start                /* point our source register to the start of the .dtcm section (VMA)*/
	ldr                 r2,=_dtcm_start                 /* point our destination register to the D-TCM dedicated to our writable data (LMA)*/
	ldr                 r3,=_dtcm_end                  /* point r3 after the last byte we will be writing (LMA)*/
	bl                  copy
	
	bl DrainWriteBuffer
	/* set WRAM9 from vma to lma (must be before clearing up our relocated area) */
	ldr                 r0,=___arm9sharedwram_vma_start
	ldr                 r1,=___arm9sharedwram_vma_end__
	bl clear
	
	bl DrainWriteBuffer
	ldr                 r1,=___arm9sharedwram_vma_start                /* point our source register to the start of the .arm9sharedwram section (VMA)*/
	ldr                 r2,=_arm9sharedwram_start                 /* point our destination register to the ARM9 SHARED WRAM dedicated to our writable data (LMA)*/
	ldr                 r3,=_arm9sharedwram_end                  /* point r3 after the last byte we will be writing (LMA)*/
	bl                  copy
	
	/*  Clear BSS */
	ldr                 r0,=__bss_vma_start
	ldr                 r1,=__bss_vma_end
	bl clear
	
	@Clear vma sections used before
	ldr                 r0,=__lib__end__
	ldr                 r1,=__vma_stub_end__
	bl clear
	
	bl DrainWriteBuffer
	/* Wait until ARM7 reads firmware and we initialize ARM9 from this flag */
	ldr r1,=(0x027FF000 + 0x5D)	@(uint8)Console Type: 
	waitForFirmwareARM7Setup:
		ldrb r0, [r1]
		cmp r0, #0xFF		@DS Phat
		beq FirmwareARM7OK
		cmp r0, #0x20		@DS Lite
		beq FirmwareARM7OK
		cmp r0, #0x57		@DSi
		beq FirmwareARM7OK
		cmp r0, #0x43		@iQueDS
		beq FirmwareARM7OK
		cmp r0, #0x63		@iQueDS Lite
		beq FirmwareARM7OK
		b waitForFirmwareARM7Setup
	FirmwareARM7OK:
	#endif
	
	bl initHardware
	bl __libc_init_array
	
	ldr	lr, =exception_sysexit
	
	#ifdef ARM7
	b	main		@entrypoint
	#endif
	
	#ifdef ARM9
	b 	mainARGV	@ARGV Support: entrypoint
	#endif

@format:	r0 = src_vma, r1 = dest_vma 
clear:
	mov	r2, #0
clearlop:	cmp	r0, r1
	strcc	r2, [r0],#4
	bcc	clearlop
	bx lr

@format:	r0 = src_vma_section, r1 = dest_lma_start, r2 = dest_lma_end	(where both lma are a whole physical memory region from start to end range address)
copy:               
	cmp                 r2,r3                       /* check if we've reached the end */
	ldrlo               r0,[r1],#4                  /* if end not reached, get word and advance source pointer */
	strlo               r0,[r2],#4                  /* if end not reached, store word and advance destination pointer */
	blo                 copy                        /* if end not reached, branch back to loop */
	bx                  lr                          /* return to caller */

.pool
.align
.end