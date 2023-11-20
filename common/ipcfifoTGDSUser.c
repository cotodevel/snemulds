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

#include "ipcfifoTGDS.h"
#include "ipcfifoTGDSUser.h"
#include "apu_shared.h"
#include "InterruptsARMCores_h.h"
#include "powerTGDS.h"

#ifdef ARM7
#include <string.h>
#include "pocketspc.h"
#include "apu.h"
#include "dsp.h"
#include "main.h"
#include "mixrate.h"
#include "spifwTGDS.h"
#include "apu_shared.h"

#endif

#ifdef ARM9
#include <stdbool.h>
#include "snemulds_memmap.h"
#include "common.h"
#include "cfg.h"
#include "main.h"
#include "core.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#endif

//inherits what is defined in: ipcfifoTGDS.c
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void HandleFifoNotEmptyWeakRef(uint32 cmd1,uint32 cmd2){
	
	switch (cmd1) {
		//ARM7 command handler
		#ifdef ARM7
		case (SNEMULDS_APUCMD_FORCESYNCON):{
			APUSYNC=true;
		}break;
		
		case (SNEMULDS_SETUP_ARM7):{
			playBuffer = (uint16*)SNES_PLAYBUFFER_ADDRESS;
			int i   = 0;
			for (i = 0; i < MIXBUFSIZE * 4; i++) {
				playBuffer[i] = 0;
			}
			update_spc_ports(); //ARM7: APU Ports from SnemulDS properly binded with Assembly APU Core
			ApuReset();
			DspReset();
			SetupSound();
			
			struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
			uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
			u32 apuFix7 = getValueSafe(&fifomsg[11]); //u32 APUFixes 
			*(u32*)0x038000c0 = apuFix7;
			setValueSafe(&fifomsg[10], (uint32)0);
		}
		break;
		
		case SNEMULDS_APUCMD_RESET: //case 0x00000001:
		{
			// Reset
			StopSound();

			memset(playBuffer, 0, MIXBUFSIZE * 8);

			SNEMULDS_IPC->APU_ADDR_CNT = 0; 
			ApuReset();
			DspReset();

			SetupSound();
			paused = false;
			SPC_disable = false;
			SPC_freedom = false;
		}
		break;
		case SNEMULDS_APUCMD_PAUSE:{ //case 0x00000002:{
			// Pause/unpause
			if (!paused) {
				StopSound();
			} else {
				SetupSound();
			}
			if (SPC_disable)
				SPC_disable = false;        
			paused = !paused;
		}
		break;
		case SNEMULDS_APUCMD_PLAYSPC:{ //case 0x00000003:{ // PLAY SPC
			//Reset APU
			StopSound();
			memset(playBuffer, 0, MIXBUFSIZE * 8);
			SNEMULDS_IPC->APU_ADDR_CNT = 0; 
			ApuReset();
			DspReset();
			SetupSound();
			
			//Load APU payload
			LoadSpc((const u8*)cmd2);
			struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
			uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
			fifomsg[40] = (uint32)0;	//release ARM9 APU_playSpc()
			
			paused = false;
			SPC_disable = false;
			SPC_freedom = false;
		}
		break;
			
		case SNEMULDS_APUCMD_SPCDISABLE:{ //case 0x00000004:{ // DISABLE 
			SPC_disable = true;
			SNEMULDS_IPC->APU_ADDR_CNT = 0;
		}
		break;        
		
		case SNEMULDS_APUCMD_CLRMIXERBUF:{ //case 0x00000005:{ // CLEAR MIXER BUFFER 
			memset(playBuffer, 0, MIXBUFSIZE * 8);
		}
		break;

		case SNEMULDS_APUCMD_SAVESPC:{ //case 0x00000006:{ // //Save APU Memory Snapshot -> u8 * inSPCBuffer @ ARM9 EWRAM
			SaveSpc((const u8*)cmd2);
			struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
			uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
			fifomsg[40] = (uint32)0;	//release ARM9 APU_playSpc()
		}
		break;  
			
		case SNEMULDS_APUCMD_LOADSPC:{ //case 0x00000007:{ // LOAD state 
			LoadSpc((const u8*)cmd2);
			struct sIPCSharedTGDS * TGDSIPC = getsIPCSharedTGDS();
			uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
			fifomsg[40] = (uint32)0;	//release ARM9 APU_loadSpc()
			SNEMULDS_IPC->APU_ADDR_CNT = 0; 
		}
		break;
		#endif
	}
	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2){
}

//project specific stuff

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
int ROM_MAX_SIZE = 0;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
int ROM_PAGING_SIZE = 0;

//APU Ports from SnemulDS properly binded with Assembly APU Core
void update_spc_ports(){
	struct s_apu2 *APU2 = (struct s_apu2 *)(&SNEMULDS_IPC->APU2);
	//must reflect to ipcfifoTGDSUser.h defs
	ADDRPORT_SPC_TO_SNES	=	(uint32)(uint8*)&SNEMULDS_IPC->PORT_SPC_TO_SNES[0];
	ADDRPORT_SNES_TO_SPC	=	(uint32)(uint8*)&SNEMULDS_IPC->PORT_SNES_TO_SPC[0]; 
	ADDR_APU_PROGRAM_COUNTER=	(uint32)(volatile uint32*)&SNEMULDS_IPC->APU_PROGRAM_COUNTER;	//0x27E0000	@APU PC
	
	ADDR_SNEMUL_CMD	=	(uint32)(volatile uint32*)&SNEMULDS_IPC->APU_ADDR_CMD;	//0x027FFFE8	// SNEMUL_CMD
	ADDR_SNEMUL_ANS	=	(uint32)(volatile uint32*)&SNEMULDS_IPC->APU_ADDR_ANS;	//0x027fffec	// SNEMUL_ANS
	ADDR_SNEMUL_BLK	=	(uint32)(volatile uint32*)&SNEMULDS_IPC->APU_ADDR_BLK;	//0x027fffe8	// SNEMUL_BLK
	SNEMULDS_IPC->APU_ADDR_BLKP = (uint8 *)ADDR_SNEMUL_BLK;
	
	//todo: APU_ADDR_CNT: is unused by APU Core?
}


//project specific stuff

#ifdef ARM9

void updateStreamCustomDecoder(u32 srcFrmt){

}

void freeSoundCustomDecoder(u32 srcFrmt){

}

#endif


//Libutils setup: TGDS project doesn't use any libutils extensions.
void setupLibUtils(){
	//libutils:
	
	//Stage 0
	#ifdef ARM9
	initializeLibUtils9(
		NULL, //ARM7 & ARM9
		NULL, //ARM9 
		NULL, //ARM9: bool stopSoundStream(struct fd * tgdsStructFD1, struct fd * tgdsStructFD2, int * internalCodecType)
		NULL,  //ARM9: void updateStream() 
		NULL, //ARM7 & ARM9: DeInitWIFI()
		NULL //ARM9: bool switch_dswnifi_mode(sint32 mode)
	);
	#endif
	
	//Stage 1
	#ifdef ARM7
	initializeLibUtils7(
		NULL, //ARM7 & ARM9
		NULL, //ARM7
		NULL, //ARM7
		NULL, //ARM7: void TIMER1Handler()
		NULL, //ARM7: void stopSound()
		NULL, //ARM7: void setupSound()
		NULL, //ARM7: void initARM7Malloc(u32 ARM7MallocStartaddress, u32 ARM7MallocSize);
		NULL, //ARM7 & ARM9: DeInitWIFI()
		NULL  //ARM7: micInterrupt()
	);
	#endif
}