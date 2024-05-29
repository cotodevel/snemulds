
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

//NTR mode:
//SNES_ROM_ADDRESS ((uchar *)(0x20C0000)) + ROM_MAX_SIZE_NTRMODE = 0x023CC000 < 0x27FF000 (NTR: Mirror #1 4MB)

//TWL mode:
//SNES_ROM_ADDRESS ((uchar *)(0x20C0000)) + ROM_MAX_SIZE_TWLMODE = 0x026CC000 < 0x27FF000 (TWL: 16MB IPC shared)

#define SNEMULDS_IPC ((struct sIPCSharedTGDSSpecific*)( ((int)0x2FFF000) - (80*16)))

#define ALIGNED __attribute__ ((aligned(4)))

#ifdef __cplusplus
extern "C" {
#endif

//NOT weak symbols : the implementation of these is project-defined (here)
extern void HandleFifoNotEmptyWeakRef(uint32 cmd1,uint32 cmd2);
extern void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2);

//project specific
extern uint32 ADDR_PORT_SNES_TO_SPC;
extern uint32 ADDR_PORT_SPC_TO_SNES;
extern void update_spc_ports();

//ARM7 & ARM9 shared
extern int ROM_MAX_SIZE;

extern int ROM_PAGING_SIZE;

#ifdef __cplusplus
}
#endif

//Standardized SnemulDS defs + TGDS Memory Layout ARM7/ARM9 Cores
#define TGDS_ARM7_MALLOCSTART (u32)(0x03800000) //ARM7 TWL end is : 0x0380d9b4, and ARM7 NTR is 6K behind that
#define TGDS_ARM7_MALLOCSIZE (int)(512)

#define TGDSDLDI_ARM7_ADDRESS (u32)(0x06000000 + (32*1024)) 	// 0x06008000 ~ 32K: DLDI
#define APU_RAM_ADDRESS     ((uint8*)(TGDSDLDI_ARM7_ADDRESS + (32*1024)))	//0x06010000 ~ 64K APU WORK RAM

#endif
