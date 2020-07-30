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
#include "wifi_arm7.h"
#include "spifwTGDS.h"
#include "apu_shared.h"

#endif

#ifdef ARM9
#include <stdbool.h>
#include "memmap.h"
#include "common.h"
#include "cfg.h"
#include "main.h"
#include "core.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "wifi_arm9.h"
#endif


#ifdef ARM9
__attribute__((section(".itcm")))
#endif
struct sIPCSharedTGDSSpecific* getsIPCSharedTGDSSpecific(){
	struct sIPCSharedTGDSSpecific* sIPCSharedTGDSSpecificInst = (__attribute__((packed)) struct sIPCSharedTGDSSpecific*)(getUserIPCAddress());
	return sIPCSharedTGDSSpecificInst;
}

//inherits what is defined in: ipcfifoTGDS.c
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoNotEmptyWeakRef(uint32 cmd1,uint32 cmd2){
	
	switch (cmd1) {
		//ARM7 command handler
		#ifdef ARM7
		
		//ARM7 Only
		case(FIFO_POWERCNT_ON):{
			powerON((uint16)cmd2);
		}
		break;
		
		case (FIFO_POWERMGMT_WRITE):{
			PowerManagementDeviceWrite(PM_SOUND_AMP, (int)cmd2>>16);  // void * data == command2
		}
		break;
		
		
		case SNEMULDS_APUCMD_RESET: //case 0x00000001:
		{
			// Reset
			StopSound();

			memset(playBuffer, 0, MIXBUFSIZE * 8);

			getsIPCSharedTGDSSpecific()->APU_ADDR_CNT = 0; 
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
			getsIPCSharedTGDSSpecific()->APU_ADDR_CNT = 0; 
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
			getsIPCSharedTGDSSpecific()->APU_ADDR_CNT = 0;
		}
		break;        
		
		case SNEMULDS_APUCMD_CLRMIXERBUF:{ //case 0x00000005:{ // CLEAR MIXER BUFFER 
			memset(playBuffer, 0, MIXBUFSIZE * 8);
		}
		break;

		case SNEMULDS_APUCMD_SAVESPC:{ //case 0x00000006:{ // SAVE state 
			SaveSpc(APU_RAM_ADDRESS);
		}
		break;  
			
		case SNEMULDS_APUCMD_LOADSPC:{ //case 0x00000007:{ // LOAD state 
			LoadSpc(APU_RAM_ADDRESS);
			getsIPCSharedTGDSSpecific()->APU_ADDR_CNT = 0; 
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

#endif

//APU Ports from SnemulDS properly binded with Assembly APU Core
void update_spc_ports(){
    struct s_apu2 *APU2 = (struct s_apu2 *)(&getsIPCSharedTGDSSpecific()->APU2);
	//must reflect to ipcfifoTGDSUser.h defs
	ADDRPORT_SPC_TO_SNES	=	(uint32)(uint8*)&getsIPCSharedTGDSSpecific()->PORT_SPC_TO_SNES[0];
	ADDRPORT_SNES_TO_SPC	=	(uint32)(uint8*)&getsIPCSharedTGDSSpecific()->PORT_SNES_TO_SPC[0]; 
	ADDR_APU_PROGRAM_COUNTER=	(uint32)(volatile uint32*)&getsIPCSharedTGDSSpecific()->APU_PROGRAM_COUNTER;	//0x27E0000	@APU PC
	
	ADDR_SNEMUL_CMD	=	(uint32)(volatile uint32*)&getsIPCSharedTGDSSpecific()->APU_ADDR_CMD;	//0x027FFFE8	// SNEMUL_CMD
	ADDR_SNEMUL_ANS	=	(uint32)(volatile uint32*)&getsIPCSharedTGDSSpecific()->APU_ADDR_ANS;	//0x027fffec	// SNEMUL_ANS
	ADDR_SNEMUL_BLK	=	(uint32)(volatile uint32*)&getsIPCSharedTGDSSpecific()->APU_ADDR_BLK;	//0x027fffe8	// SNEMUL_BLK
	getsIPCSharedTGDSSpecific()->APU_ADDR_BLKP = (uint8 *)ADDR_SNEMUL_BLK;
	
	//todo: APU_ADDR_CNT: is unused by APU Core?
}


#ifdef ARM9
//Callback update sample implementation

__attribute__((section(".itcm")))
void updateSoundContextStreamPlaybackUser(u32 srcFrmt){
}
#endif

void setupSoundUser(u32 srcFrmtInst){
	#ifdef ARM7
	#endif
	
	#ifdef ARM9
	#endif
}

void stopSoundUser(u32 srcFrmt){
	#ifdef ARM7
	#endif
	
	#ifdef ARM9
	#endif
}

