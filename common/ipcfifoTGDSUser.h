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

//TGDS required version: IPC Version: 1.3

//IPC FIFO Description: 
//		struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 														// Access to TGDS internal IPC FIFO structure. 		(ipcfifoTGDS.h)
//		struct sIPCSharedTGDSSpecific * TGDSUSERIPC = (struct sIPCSharedTGDSSpecific *)TGDSIPCUserStartAddress;		// Access to TGDS Project (User) IPC FIFO structure	(ipcfifoTGDSUser.h)

#ifndef __specific_shared_h__
#define __specific_shared_h__

#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "dswnifi.h"
#include "apu_shared.h"

//Enable for an ARM7DLDI build. Disable for an ARM9DLDI build
//#define SNEMULDS_ARM7_DLDI

#if defined(SNEMULDS_ARM7_DLDI) && defined(ARM9_DLDI)
#error "ToolchainGenericDS SDK builds ARM9DLDI TGDS Binaries! Make sure it builds ARM7DLDI TGDS Binaries!"
#endif

#if !defined(SNEMULDS_ARM7_DLDI) && defined(ARM7_DLDI)
#error "ToolchainGenericDS SDK builds ARM7DLDI TGDS Binaries! Make sure it builds ARM9DLDI TGDS Binaries!"
#endif

struct s_apu2
{
	/* timers */
	uint32    T0, T1, T2;
	uint32 	TIM0, TIM1, TIM2;
	uint32	CNT0, CNT1, CNT2;
  
	int	    skipper_cnt1;
	int	    skipper_cnt2;
	int	    skipper_cnt3;
	int	    skipper_cnt4;
	int		counter;
};

struct sIPCSharedTGDSSpecific{
	uint32 * IPC_ADDR;
    uint8 * ROM;   		//pointer to ROM page
    int rom_size;   	//rom total size
	struct s_apu2 APU2;
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
#define SNEMULDS_APUCMD_RESET 0xffff00a1
#define SNEMULDS_APUCMD_PAUSE 0xffff00a2
#define SNEMULDS_APUCMD_PLAYSPC 0xffff00a3
#define SNEMULDS_APUCMD_SPCDISABLE 0xffff00a4
#define SNEMULDS_APUCMD_CLRMIXERBUF 0xffff00a5
#define SNEMULDS_APUCMD_SAVESPC 0xffff00a6
#define SNEMULDS_APUCMD_LOADSPC 0xffff00a7

//Standardized SnemulDS defs
#define ARM7_SOUNDWORK_BASE     ((uint8*)(0x6000000))
#define ARM7_DLDI_BASE			((uint8*)(ARM7_SOUNDWORK_BASE + 0x10000 - 0x4000))	//These 16K are unused by sound code, so, reserved for DLDI ARM7 code
#define APU_RAM_ADDRESS     	((uint8*)(ARM7_SOUNDWORK_BASE + 0x10000))			//uses VRAM Block as APU WORK RAM

//GDB stub support
//#define GDB_ENABLE

//IPC Cmd
#define SNEMULDS_HANDLE_VCOUNT (u8)(0x2)

#ifdef ARM9
//Used by ARM9. Required internally by ARM7
#define TGDSDLDI_ARM7_ADDRESS (int)(0x06000000 + (64*1024) - (0x4000))
#endif

#endif

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

#ifdef __cplusplus
}
#endif