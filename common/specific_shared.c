
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
#include "common_shared.h"
#include "specific_shared.h"

#ifdef ARM7
#include <string.h>

#include "pocketspc.h"
#include "apu.h"
#include "dsp.h"
#include "main.h"
#include "mixrate.h"
#include "wifi_arm7.h"

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
#include "InterruptsARMCores_h.h"

#include "wifi_arm9.h"

#endif


//inherits what is defined in: common_shared.c
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoNotEmptyWeakRef(uint32 cmd1,uint32 cmd2,uint32 cmd3,uint32 cmd4,uint32 cmd5){
	if(cmd1 == fifo_requires_ack){
	
		switch (cmd2) {
		
			//Shared 
			case(WIFI_SYNC):{
				Wifi_Sync();
			}
			break;
			
			//must be called from within timer irqs
			//update emulated registers from within nds FIFO irqs!
			case(FIFO_SEND_EXT):{
				
				
			}
			break;
			
			//Process the packages (signal) that sent earlier FIFO_SEND_EXT
			case(FIFO_RECV_EXT):{
				//Handle_SoftFIFORECV();//does not work
			}
			break;
			
			
			
			
			//ARM7 command handler
			#ifdef ARM7
			
			//ARM7 Only
			case(FIFO_POWERCNT_ON):{
				powerON((u16)cmd3);
			}
			break;
			
			
			//arm9 wants to send a WIFI context block address / userdata is always zero here
			case(WIFI_STARTUP):{
				//	wifiAddressHandler( void * address, void * userdata )
				wifiAddressHandler((Wifi_MainStruct *)(uint32)cmd3, 0);
			}
			break;
			
			
			case WRITE_VALUE_32:{
				uint32 address = cmd3;
				*(uint32*)address = cmd4;
			}
			break;
			
			case SNEMULDS_APUCMD_RESET: //case 0x00000001:
			{
				// Reset
				StopSound();

				memset(playBuffer, 0, MIXBUFSIZE * 8);

				SpecificIPC->APU_ADDR_CNT = 0; 
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
				LoadSpc(APU_RAM_ADDRESS);
				SetupSound();   	
				SpecificIPC->APU_ADDR_CNT = 0;             	
				paused = false;
				SPC_freedom = true;
				SPC_disable = false;
			}
			break;
				
			case SNEMULDS_APUCMD_SPCDISABLE:{ //case 0x00000004:{ // DISABLE 
				SPC_disable = true;
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
				SpecificIPC->APU_ADDR_CNT = 0; 
			}
			break;
			
			//deprecate later
			case 0x00000008:{
				PowerManagementDeviceWrite(PM_SOUND_AMP, (int)cmd3>>16);  // void * data == command2
			}
			break;
			
			#endif
			
			
			
			//ARM9 command handler
			#ifdef ARM9
			//exception handler: arm7
			case(EXCEPTION_ARM7):{
				
				if((uint32)cmd2 == (uint32)unexpectedsysexit_7){
					exception_handler((uint32)unexpectedsysexit_7);	//r0 = EXCEPTION_ARM7 / r1 = unexpectedsysexit_7
				}
			}
			break;
			
			
			#endif
		
			
		}
		
		//MyIPC->fiforeply = (uint32)fifo_requires_ack_execok;
	
	}
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2,uint32 cmd3,uint32 cmd4,uint32 cmd5){
}

//project specific stuff
#ifdef ARM9

//small hack to update SNES_ADDRESS at opcodes2.s
void update_ram_snes(){
    snes_ram_address = (uint32)&snes_ram_bsram[0x6000];
}
#endif

#ifdef ARM7
//small hack to update IPC APU ports with APU assembly core (on ARM7)
void update_spc_ports(){
    ADDR_PORT_SNES_TO_SPC       =   (uint32)(u8*)PORT_SNES_TO_SPC;
    ADDR_PORT_SPC_TO_SNES   =   (uint32)(u8*)PORT_SPC_TO_SNES;
}
#endif

