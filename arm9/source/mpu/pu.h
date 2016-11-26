#include <nds.h>
#include <fat.h>
#include <filesystem.h>
#include <dirent.h>
#include <unistd.h>    // for sbrk()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#include "../main.h"


//PU setup all NDS region addresses
#define debug_vect	(*(intfuncptr *)(0x02FFFD9C)) //ori: #define EXCEPTION_VECTOR	(*(VoidFn *)(0x2FFFD9C))
#define except_vect	(*(void *)(0x00000000)) //0x00000000 or 0xffff0000

//These are for GBA emu

/*Processor modes

User	usr	%10000	Normal program execution, no privileges	All
FIQ	fiq	%10001	Fast interrupt handling	All
IRQ	irq	%10010	Normal interrupt handling	All
Supervisor	svc	%10011	Privileged mode for the operating system	All
Abort	abt	%10111	For virtual memory and memory protection	ARMv3+
Undefined	und	%11011	Facilitates emulation of co-processors in hardware	ARMv3+
System	sys	%11111	Runs programs with some privileges	ARMv4+

0x10 usr
0x11 fiq
0x12 irq
0x13 svc
0x17 abt
0x1b und
0x1f sys
*/

//THIS IS EXTREMELY IMPORTANT! TO DEBUG !!
//27FFD9Ch - RAM - NDS9 Debug Stacktop / Debug Vector (0=None)
//380FFDCh - RAM - NDS7 Debug Stacktop / Debug Vector (0=None)
//These addresses contain a 32bit pointer to the Debug Handler, and, memory below of the addresses is used as Debug Stack. 
//The debug handler is called on undefined instruction exceptions, on data/prefetch aborts (caused by the protection unit), 
//on FIQ (possibly caused by hardware debuggers). It is also called by accidental software-jumps to the reset vector, 
//and by unused SWI numbers within range 0..1Fh.


/* EXCEPTION VECTOR TRIGGERS
Exception		/Offset from vector base		/Mode on entry		/A bit on entry		/F bit on entry		/I bit on entry
Reset			0x00							Supervisor			Disabled			Disabled			Disabled

Undefined 
instruction		0x04							Undefined			Unchanged			Unchanged			Disabled

Software 
interrupt		0x08							Supervisor			Unchanged			Unchanged			Disabled

Prefetch 
Abort			0x0C							Abort				Disabled			Unchanged			Disabled

Data Abort		0x10							Abort				Disabled			Unchanged			Disabled

Reserved		0x14							Reserved			-					-					-

IRQ				0x18							IRQ					Disabled			Unchanged			Disabled

FIQ				0x1C							FIQ					Disabled			Disabled			Disabled

*/

//exception vectors:
//:FFFF0000             _reset_vector 
//:FFFF0004             _undefined_vector 
//:FFFF0008             _swi_vector
//:FFFF000C             _prefetch_abort_vector 
//:FFFF0010             _data_abort_vector 
//:FFFF0014             _reserved_vector 
//:FFFF0018             _irq_vector
//:FFFF001C             _fiq_vector

//THESE MEMORY AREAS AREN'T ON PU DEFAULT SETTINGS!
//Any r/w beyond 0x02xxxxxx will cause CPU to change to ABT mode + unresolved PU address 
//(guru meditation error with the default debug handler)
//#define ioMem caioMem //hack todo
//GBA Slot ROM 08000000h (max. 32MB) parts of gba code to execute is mapped here with various tricks

#define DISPCAPCNT (*(vu32*)0x4000064)
//BIOS_GBA wrapper nds<->   gba bios 00000000-00003FFF BIOS - System ROM (16 KBytes)
//GBA & NDS stacks are assigned to Data TCM (accesable only in ARM9, through CP15 mcr / mrc)

//08000000-09FFFFFF Game Pak ROM/FlashROM (max 32MB) - Wait State 0
//0A000000-0BFFFFFF Game Pak ROM/FlashROM (max 32MB) - Wait State 1
//0C000000-0DFFFFFF Game Pak ROM/FlashROM (max 32MB) - Wait State 2

/*	Region 0 - background	0x00000000 PU_PAGE_128M
	Region 1 - DTCM 0x0b000000 PAGE_16K   
	Region 2 - speedupone 0x02040000 PU_PAGE_256K
	Region 3 - speeduptwo 0x02080000 PU_PAGE_512K
	Region 4 - speedupthree 0x02100000 PU_PAGE_1M
	Region 5 - speedupfour 0x02200000 PU_PAGE_2M
	Region 6 - ITCM protector 0x00000000 PU_PAGE_16M
	Region 7 - IO 0x04000000 PU_PAGE_16M
*/

//Protection Unit DCACHE / ICACHE setup
#define PU_PAGE_4K		(0x0B << 1)
#define PU_PAGE_8K		(0x0C << 1)
#define PU_PAGE_16K		(0x0D << 1)
#define PU_PAGE_32K		(0x0E << 1)
#define PU_PAGE_64K		(0x0F << 1)
#define PU_PAGE_128K		(0x10 << 1)
#define PU_PAGE_256K		(0x11 << 1)
#define PU_PAGE_512K		(0x12 << 1)
#define PU_PAGE_1M		(0x13 << 1)
#define PU_PAGE_2M		(0x14 << 1)
#define PU_PAGE_4M		(0x15 << 1)
#define PU_PAGE_8M		(0x16 << 1)
#define PU_PAGE_16M		(0x17 << 1)
#define PU_PAGE_32M		(0x18 << 1)
#define PU_PAGE_64M		(0x19 << 1)
#define PU_PAGE_128M		(0x1A << 1)
#define PU_PAGE_256M		(0x1B << 1)
#define PU_PAGE_512M		(0x1C << 1)
#define PU_PAGE_1G		(0x1D << 1)
#define PU_PAGE_2G		(0x1E << 1)
#define PU_PAGE_4G		(0x1F << 1)

//debug dump PU format:
//0x3003db4 	80a548a 	153123
//PU address 	value held 	times accessed PU address

#ifdef __cplusplus
extern "C" {
#endif

// extern void puSetMemPerm(u32 perm);
extern void pu_Enable();

// extern void puSetGbaIWRAM();
extern void pu_SetRegion(u32 region, u32 value);

extern void pu_SetDataPermissions(u32 v);
extern void pu_SetCodePermissions(u32 v);
extern void pu_SetDataCachability(u32 v);
extern void pu_SetCodeCachability(u32 v);
extern void pu_GetWriteBufferability(u32 v);

extern void cpu_SetCP15Cnt(u32 v); //mask bit 1 for: 0 disable, 1 enable, PU
extern u32 cpu_GetCP15Cnt(); //get PU status: 0 disable, 1 enable

//instruction cache CP15
extern void IC_InvalidateAll();
extern void IC_InvalidateRange(const void *, u32 v);
extern void setitcmbase(); //@ ITCM base = 0 , size = 32 MB
extern void icacheenable(int);

//data cache CP15
extern void DC_FlushAll();
extern void DC_FlushRange(const void *, u32 v);
extern void setdtcmbase(); //@ DTCM base = __dtcm_start, size = 16 KB
extern void drainwrite();
extern void dcacheenable(int); //Cachability Bits for Data/Unified Protection Region (R/W)
extern void setgbamap();

//CP15 other opcodes
void HALTCNT_ARM9();
void HALTCNT_ARM9OPT();

u32 getdtcmbase();
u32 getitcmbase();

extern u32 DC_clean_invalidate_range(u32 VA_range);

extern u32 DC_lockdownVA(u32 VA_range);
extern u32 IC_lockdownVA(u32 VA_range);

extern int setdtcmsz(u8 size);

extern void writebufenable(int);

extern void prepcache();
extern void gbamode();
extern void ndsmode();
extern void emulateedbiosstart();
extern u32 vector_addr;
extern u32 vector_end_addr;

//extern u32 biosirq_hdlr(); //user bios exception handler (irq) (already added @ irq vector 0x00000000 + 0x

extern int cpuGetCPSR();

//install user interrupt handler at: (dtcm+top-0x4)
extern int instusrhdlr();

//MPU & other
extern int inter_swi(int);	
extern int cpuGetSPSR();
extern u32 vblankcallC; 		//vblank C asm function address
extern u32 mpu_setup();

//C vector exceptions
extern u32 exceptswi(u32); 		//swi vector
extern u32 exceptundef(u32 undef);	//undefined vector
extern u32 exceptirq(u32 nds_iemask,u32 nds_ifmask,u32 sp_ptr);
extern u32 swicaller(u32 arg);

//exception tests
extern u32 inter_irq();

//extern u32 __attribute__((section(".dtcm"))) curr_exception[]; //inter_regs.s

//extern void __attribute__((section(".dtcm"))) (*exHandler)();
//extern void __attribute__((section(".dtcm"))) (*exHandlerswi)();
//extern void __attribute__((section(".dtcm"))) (*exHandlerundifined)();
//extern s32  __attribute__((section(".dtcm"))) exRegs[];
//extern s32  __attribute__((section(".dtcm"))) BIOSDBG_SPSR;

//cpu_SetCP15Cnt(cpu_GetCP15Cnt() & ~0x1);
//2 = 2048 / 3 = 4096 / 4 = 8192 / 5 = 16384
//iprintf("%x \n",setdtcmsz(5)); //0x027C0000
//pu_Enable();

#ifdef __cplusplus
}
#endif