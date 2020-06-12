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

/* Coto: These regs are for inline Assembler with CPP definitions. (just for .S Assembler source files), if you use .s Assembler source files
 and try to include this file it won't work.*/

#ifndef __nds9_dsregs_asm_h__
#define __nds9_dsregs_asm_h__

#include "global_settings.h"

//Processor Status Register bits
/*
 * PSR bits
 */
#define USR26_MODE	0x00000000
#define FIQ26_MODE	0x00000001
#define IRQ26_MODE	0x00000002
#define SVC26_MODE	0x00000003
#define USR_MODE	0x00000010
#define FIQ_MODE	0x00000011
#define IRQ_MODE	0x00000012
#define SVC_MODE	0x00000013
#define ABT_MODE	0x00000017
#define UND_MODE	0x0000001b
#define SYSTEM_MODE	0x0000001f
#define MODE32_BIT	0x00000010
#define MODE_MASK	0x0000001f
#define PSR_T_BIT	0x00000020
#define PSR_F_BIT	0x00000040
#define PSR_I_BIT	0x00000080
#define PSR_J_BIT	0x01000000
#define PSR_Q_BIT	0x08000000
#define PSR_V_BIT	0x10000000
#define PSR_C_BIT	0x20000000
#define PSR_Z_BIT	0x40000000
#define PSR_N_BIT	0x80000000
#define PCMASK		0

/*
 * Groups of PSR bits
 */
#define PSR_f		0xff000000	/* Flags		*/
#define PSR_s		0x00ff0000	/* Status		*/
#define PSR_x		0x0000ff00	/* Extension		*/
#define PSR_c		0x000000ff	/* Control		*/


//MPU Definitions.
#define PAGE_4K		(0b01011 << 1)
#define PAGE_8K		(0b01100 << 1)
#define PAGE_16K	(0b01101 << 1)
#define PAGE_32K	(0b01110 << 1)
#define PAGE_64K	(0b01111 << 1)
#define PAGE_128K	(0b10000 << 1)
#define PAGE_256K	(0b10001 << 1)
#define PAGE_512K	(0b10010 << 1)
#define PAGE_1M		(0b10011 << 1)
#define PAGE_2M		(0b10100 << 1)
#define PAGE_4M		(0b10101 << 1)
#define PAGE_8M		(0b10110 << 1)
#define PAGE_16M	(0b10111 << 1)
#define PAGE_32M	(0b11000 << 1)
#define PAGE_64M	(0b11001 << 1)
#define PAGE_128M	(0b11010 << 1)
#define PAGE_256M	(0b11011 << 1)
#define PAGE_512M	(0b11100 << 1)
#define PAGE_1G		(0b11101 << 1)
#define PAGE_2G		(0b11110 << 1)
#define PAGE_4G		(0b11111 << 1)

/*
 * CR1 bits (CP#15 CR1)
*/
#define CR_M	(1 << 0)	/* MMU/MPU enable				*/
#define CR_A	(1 << 1)	/* Alignment abort enable		*/
#define CR_C	(1 << 2)	/* Dcache enable			*/
#define CR_W	(1 << 3)	/* Write buffer enable			*/
#define CR_P	(1 << 4)	/* 32-bit exception handler		*/
#define CR_D	(1 << 5)	/* 32-bit data address range		*/
#define CR_L	(1 << 6)	/* Implementation defined		*/
#define CR_B	(1 << 7)	/* Big endian				*/
#define CR_S	(1 << 8)	/* System MMU protection		*/
#define CR_R	(1 << 9)	/* ROM MMU protection			*/
#define CR_F	(1 << 10)	/* Implementation defined		*/
#define CR_Z	(1 << 11)	/* Implementation defined		*/
#define CR_I	(1 << 12)	/* Icache enable			*/
#define CR_V	(1 << 13)	/* Vectors relocated to 0xffff0000	*/
#define CR_RR	(1 << 14)	/* Round Robin cache replacement	*/
#define CR_L4	(1 << 15)	/* LDR pc can set T bit			*/
#define CR_DT	(1 << 16)	/* CP15 DTCM Enable bit */
#define CR_BR	(1 << 17)	/* MPU Background region enable (PMSA)  */
#define CR_IT	(1 << 18)	/* CP15 ITCM Enable bit */
#define CR_ST	(1 << 19)
#define CR_FI	(1 << 21)	/* Fast interrupt (lower latency mode)	*/
#define CR_U	(1 << 22)	/* Unaligned access operation		*/
#define CR_XP	(1 << 23)	/* Extended page tables			*/
#define CR_VE	(1 << 24)	/* Vectored interrupts			*/
#define CR_EE	(1 << 25)	/* Exception (Big) Endian		*/
#define CR_TRE	(1 << 28)	/* TEX remap enable			*/
#define CR_AFE	(1 << 29)	/* Access flag enable			*/
#define CR_TE	(1 << 30)	/* Thumb exception enable		*/

//Interrupts
/*
4000208h - NDS9/NDS7 - IME - Interrupt Master Enable (R/W)
  0     Disable all interrupts  (0=Disable All, 1=See IE register)
  1-31  Not used

4000210h - NDS9/NDS7 - IE - 32bit - Interrupt Enable (R/W)
4000214h - NDS9/NDS7 - IF - 32bit - Interrupt Request Flags (R/W)
Bits in the IE register are 0=Disable, 1=Enable.
Reading IF returns 0=No request, 1=Interrupt Request.
Writing IF acts as 0=No change, 1=Acknowledge (clears that bit).
  0     LCD V-Blank
  1     LCD H-Blank
  2     LCD V-Counter Match
  3     Timer 0 Overflow
  4     Timer 1 Overflow
  5     Timer 2 Overflow
  6     Timer 3 Overflow
  7     NDS7 only: SIO/RCNT/RTC (Real Time Clock)
  8     DMA 0
  9     DMA 1
  10    DMA 2
  11    DMA 3
  12    Keypad
  13    GBA-Slot (external IRQ source) / DSi: None such
  14    Not used                       / DSi9: NDS-Slot Card change?
  15    Not used                       / DSi: dito for 2nd NDS-Slot?
  16    IPC Sync
  17    IPC Send FIFO Empty
  18    IPC Recv FIFO Not Empty
  19    NDS-Slot Game Card Data Transfer Completion
  20    NDS-Slot Game Card IREQ_MC
  21    NDS9 only: Geometry Command FIFO
  22    NDS7 only: Screens unfolding
  23    NDS7 only: SPI bus
  24    NDS7 only: Wifi    / DSi9: XpertTeak DSP
  25    Not used           / DSi9: Camera
  26    Not used           / DSi9: Undoc, IF.26 set on FFh-filling 40021Axh
  27    Not used           / DSi:  Maybe IREQ_MC for 2nd gamecard?
  28    Not used           / DSi: NewDMA0
  29    Not used           / DSi: NewDMA1
  30    Not used           / DSi: NewDMA2
  31    Not used           / DSi: NewDMA3
  ?     DSi7: any further new IRQs on ARM7 side...?
*/

#define IRQ_VBLANK	(1<<0)
#define IRQ_HBLANK	(1<<1)
#define IRQ_VCOUNT	(1<<2)
#define IRQ_TIMER0	(1<<3)
#define IRQ_TIMER1	(1<<4)
#define IRQ_TIMER2	(1<<5)
#define IRQ_TIMER3	(1<<6)

#define IRQ_RTCLOCK	(1<<7)

#define IRQ_DMA0	(1<<8)
#define IRQ_DMA1	(1<<9)
#define IRQ_DMA2	(1<<10)
#define IRQ_DMA3	(1<<11)

#define IRQ_KEYPAD	(1<<12)
#define IRQ_GBACART	(1<<13)				//raised only by gba hardware embedded on cart
#define IRQ_UNUSED1	(1<<14)
#define IRQ_UNUSED2	(1<<15)
#define IRQ_IPCSYNC	(1<<16)
#define IRQ_SENDFIFO_EMPTY	(1<<17)
#define IRQ_RECVFIFO_NOT_EMPTY	(1<<18)
#define IRQ_SLOT1_DATATRANSFER_COMPLETE	(1<<19)
#define IRQ_SLOT1_IREQ_MC	(1<<20)		//raised only by ds hardware embedded on cart
#define IRQ_GEOMETRYCMD_FIFO	(1<<21)
#define IRQ_SCREENLID	(1<<22)	//IRQ_LID
#define IRQ_SPI_BUS	(1<<23)
#define IRQ_WIFI	(1<<24)


#endif
