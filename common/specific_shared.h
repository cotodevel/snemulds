
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

//inherits what is defined in: common_shared.h
#ifndef __specific_shared_h__
#define __specific_shared_h__

#include "dsregs.h"
#include "dsregs_asm.h"
#include "common_shared.h"
#include "dswnifi.h"
#include "apu_shared.h"

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
}__attribute__ ((aligned (4)));

//---------------------------------------------------------------------------------
typedef struct sSpecificIPC {
//---------------------------------------------------------------------------------
	//project specific
	uint32 * IPC_ADDR;
    uint8 * ROM;   		//pointer to ROM page
    int rom_size;   	//rom total size
	
	//dswnifi specific
	TdsnwifisrvStr dswifiSrv;

	//IPC APU Ports
	uint32 APU_ADDR_CNT;
    uint32 APU_ADDR_CMD;

    uint8 APU_ADDR_BLKP[4];
    uint32 APU_ADDR_BLK;     //deprecated in v6 but declared
    
	//struct s_apu2 APU2;	//the unaligned access here kills the SnemulDS APU sync. Must be word aligned.
	
} tSpecificIPC __attribute__ ((aligned (4)));

//project specific IPC
#define SpecificIPC ((volatile tSpecificIPC*)(0x027FF000+(sizeof(tMyIPC))))
#define PORT_SNES_TO_SPC ((volatile uint8*)(0x027FF000+(sizeof(tMyIPC))+(sizeof(tSpecificIPC))+(4*1)))
#define PORT_SPC_TO_SNES ((volatile uint8*)(0x027FF000+(sizeof(tMyIPC))+(sizeof(tSpecificIPC))+(4*2))) 
#define APU2 ((volatile struct s_apu2*)(0x027FF000+(sizeof(tMyIPC))+(sizeof(tSpecificIPC))+(4*3)))

// Project Specific
#define SNEMULDS_APUCMD_RESET 0xffff00a1
#define SNEMULDS_APUCMD_PAUSE 0xffff00a2
#define SNEMULDS_APUCMD_PLAYSPC 0xffff00a3
#define SNEMULDS_APUCMD_SPCDISABLE 0xffff00a4
#define SNEMULDS_APUCMD_CLRMIXERBUF 0xffff00a5
#define SNEMULDS_APUCMD_SAVESPC 0xffff00a6
#define SNEMULDS_APUCMD_LOADSPC 0xffff00a7


#endif

#ifdef __cplusplus
extern "C" {
#endif

//NOT weak symbols : the implementation of these is project-defined (here)
extern void HandleFifoNotEmptyWeakRef(uint32 cmd1,uint32 cmd2,uint32 cmd3,uint32 cmd4);
extern void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2,uint32 cmd3,uint32 cmd4);

//project specific
extern uint32 ADDR_PORT_SNES_TO_SPC;
extern uint32 ADDR_PORT_SPC_TO_SNES;

#ifdef ARM9
extern void update_ram_snes();
#endif

#ifdef ARM7
extern void update_spc_ports();
#endif

#ifdef __cplusplus
}
#endif