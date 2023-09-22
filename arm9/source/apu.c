//***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006-2007 archeide, All rights reserved. */
/***********************************************************/
/*
This program is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License as 
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
GNU General Public License for more details.
*/

#include <string.h>
#include "cfg.h"
#include "apu.h"
#include "ipcfifoTGDSUser.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefsTGDS.h"
#include "InterruptsARMCores_h.h"
#include "core.h"
#include "apu_shared.h"
#include "biosTGDS.h"
#include "nds_cp15_misc.h"

////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void	APU_reset()
{
    APU_command(SNEMULDS_APUCMD_RESET); //APU_command(0x00000001);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void	APU_nice_reset()
{
#ifndef IN_EMULATOR	
	APU_stop();
	APU_reset();
#endif
	
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void	APU_pause()
{
    APU_command(SNEMULDS_APUCMD_PAUSE); //APU_command(0x00000002);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void	APU_stop()
{
#ifndef IN_EMULATOR	
	APU_command(SNEMULDS_APUCMD_SPCDISABLE); //APU_command(0x00000004);
	// Wait the APU disabling
	
	while (SNEMULDS_IPC->APU_ADDR_ANS != 0xFF00FF00){
		
	}
#endif	
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void	APU_playSpc(u8 * inSPCBuffer)
{
	struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[40] = (uint32)0xFFFFC070;
	
	//prevent APU from desync
	SendFIFOWordsITCM(SNEMULDS_APUCMD_PLAYSPC, (u32)inSPCBuffer);	//APU_command(0x00000003);
	
	while((uint32)fifomsg[40] == (uint32)0xFFFFC070){
		swiDelay(2);
	}
}

//Requires an empty buffer[0x10200] @ inSPCBuffer, saves ARM7 SNES APUMEMORY into it
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void	APU_saveSpc(u8 * inSPCBuffer)
{
	struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[40] = (uint32)0xFFFFC070;
	
	//prevent APU from desync
	SendFIFOWordsITCM(SNEMULDS_APUCMD_SAVESPC, (u32)inSPCBuffer);	//APU_command(0x00000006);
	
	while((uint32)fifomsg[40] == (uint32)0xFFFFC070){
		swiDelay(2);
	}
	
	coherent_user_range_by_size((uint32)inSPCBuffer, (int)0x10200);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void	APU_loadSpc(u8 * inSPCBuffer)
{
	coherent_user_range_by_size((uint32)inSPCBuffer, (int)0x10200);	
	struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
	fifomsg[40] = (uint32)0xFFFFC070;
	
	//prevent APU from desync
	SendFIFOWordsITCM(SNEMULDS_APUCMD_LOADSPC, (u32)inSPCBuffer);	//APU_command(0x00000007);
	
	while((uint32)fifomsg[40] == (uint32)0xFFFFC070){
		swiDelay(2);
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void	APU_clear()
{
	APU_command(SNEMULDS_APUCMD_CLRMIXERBUF); //APU_command(0x00000005);
	SNEMULDS_IPC->APU_ADDR_CNT = 0;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void APU_command(uint32 command){
	//prevent APU from desync
	SendFIFOWords(command, 0xFFFFFFFF);
}