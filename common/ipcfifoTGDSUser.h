
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

//inherits what is defined in: ipcfifoTGDS.h
#ifndef __ipcfifoTGDSUser_h__
#define __ipcfifoTGDSUser_h__

#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "apu_shared.h"

struct sIPCSharedTGDSSpecific{
	uint8	PORT_SNES_TO_SPC[4];
	uint8	PORT_SPC_TO_SNES[4];
	uint32	APU_PROGRAM_COUNTER;	//0x27E0000	@APU PC
	uint32	APU_ADDR_CMD;	//SNEMUL_CMD / APU_ADDR_CMD ((volatile uint32*)(0x2800000-16))	//0x027FFFE8
	uint32	APU_ADDR_ANS;	//SNEMUL_ANS / ADDR_SNEMUL_ANS : //#define APU_ADDR_ANS ((volatile uint32*)(0x2800000-20))
	uint32	APU_ADDR_BLK;	//APU_ADDR_BLK / SNEMUL_BLK ((volatile uint32*)(0x2800000-24))
	volatile uint8 * APU_ADDR_BLKP;	//#define (vuint8*)APU_ADDR_BLKP == APU_ADDR_BLK
	uint32	APU_ADDR_CNT;	//#define APU_ADDR_CNT ((volatile uint32*)(0x2800000-60))	/ 0x27fffc4 // used a SNES SCanline counter, unused by snemulds
	int APUSlowdown; 
	char snesHeaderName[10];
};

// Project Specific
#define SNEMULDS_SETUP_ARM7 (u32)(0xffff00a0)
#define SNEMULDS_APUCMD_RESET (u32)(0xffff00a1)
#define SNEMULDS_APUCMD_PAUSE (u32)(0xffff00a2)
#define SNEMULDS_APUCMD_PLAYSPC (u32)(0xffff00a3)
#define SNEMULDS_APUCMD_SPCDISABLE (u32)(0xffff00a4)
#define SNEMULDS_APUCMD_CLRMIXERBUF (u32)(0xffff00a5)
#define SNEMULDS_APUCMD_SAVESPC (u32)(0xffff00a6)
#define SNEMULDS_APUCMD_LOADSPC (u32)(0xffff00a7)

//Standardized SnemulDS defs + TGDS Memory Layout ARM7/ARM9 Cores
#define TGDSDLDI_ARM7_ADDRESS	((u32)(0x06000000)) // 0x06000000 ~ 0x06007FFF = 32K: DLDI
#define APU_RAM_ADDRESS 		(u32)( ((int)TGDSDLDI_ARM7_ADDRESS) + (32*1024)) 	// 0x06008000 ~ 0x06017FFF = 96K APU WORK RAM
#define TGDS_ARM7_MALLOCSTART (u32)( ((int)APU_RAM_ADDRESS) + (96*1024) )	// 0x06018000 ~ 0x06020000 = 32K ARM7 Malloc
#define TGDS_ARM7_MALLOCSIZE (int)( (32*1024) )

#define SNEMULDS_IPC ((struct sIPCSharedTGDSSpecific*)( ((int)0x2FFF000) - (80*16)))
#define ALIGNED __attribute__ ((aligned(4)))

#endif

#ifdef __cplusplus
extern "C" {
#endif

//NOT weak symbols : the implementation of these is project-defined (here)
extern void HandleFifoNotEmptyWeakRef(uint32 cmd1,uint32 cmd2);

//project specific
extern uint32 ADDR_PORT_SNES_TO_SPC;
extern uint32 ADDR_PORT_SPC_TO_SNES;
extern void update_spc_ports();

//ARM7 & ARM9 shared
extern int ROM_MAX_SIZE;
extern int ROM_PAGING_SIZE;

#define SNES_ROM_ADDRESS_NTR ((uchar *)(0x20C0000)) 
#define SNES_ROM_ADDRESS_TWL ((uchar *)(0x20C9F00))

#define ROM_MAX_SIZE_NTRMODE	(3*1024*1024)
#define ROM_MAX_SIZE_TWLMODE	((6*1024*1024)+(512*1024)) //Max ROM size: 6.5MB

#define	PAGE_SIZE		(64*1024)
#define SNES_ROM_PAGING_ADDRESS (SNES_ROM_ADDRESS_NTR+PAGE_SIZE)

//334K ~ worth of Hashed Samples from the APU core to remove stuttering
#define APU_BRR_HASH_BUFFER_NTR	(volatile u32*)(((int)SNES_ROM_ADDRESS_NTR) + ROM_MAX_SIZE_NTRMODE - (334*1024) )	//(334*1024) = 342016 bytes / 64K blocks = 5 pages less useable on paging mode //  0x2AC800 (2.8~ MB) free SNES ROM non-paged

extern u32 apuCacheSamples;
extern bool apuCacheSamplesTWLMode;
extern u32 * savedROMForAPUCache;


#ifdef __cplusplus
}
#endif