#include "InterruptsARMCores_h.h"
#include "ipcfifoTGDSUser.h"
#include "guiTGDS.h"
#include "dsregs_asm.h"
#include "fs.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"
#include "core.h"
#include "conf.h"
#include "interrupts.h"
#include "main.h"
#include "ppu.h"
#include "keypadTGDS.h"
#include "dswnifi_lib.h"
#include "dswnifi.h"
#include "utilsTGDS.h"
#include "spifwTGDS.h"

//User Handler Definitions

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void IpcSynchandlerUser(){
	uint8 ipcByte = receiveByteIPC();
	switch(ipcByte){
		//External ARM Core's sendMultipleByteIPC(uint8 inByte0, uint8 inByte1, uint8 inByte2, uint8 inByte3) received bytes:
		case(IPC_SEND_MULTIPLE_CMDS):{
			struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
			uint8 * ipcMsg = (uint8 *)&TGDSIPC->ipcMesaggingQueue[0];
			#ifdef ARM9
			coherent_user_range_by_size((uint32)ipcMsg, sizeof(TGDSIPC->ipcMesaggingQueue));
			#endif
			uint8 inByte0 = (u8)ipcMsg[0];
			uint8 inByte1 = (u8)ipcMsg[1];
			uint8 inByte2 = (u8)ipcMsg[2];
			uint8 inByte3 = (u8)ipcMsg[3];
			
			//Do stuff.
			ipcMsg[3] = ipcMsg[2] = ipcMsg[1] = ipcMsg[0] = 0;
		}
		break;
		
		//Update VCOUNT through ARM7
		case(SNEMULDS_HANDLE_VCOUNT):{
			//GUI_update();			//Works! But sadly this breaks DSWNIFI, thus, it can't be called here
		}
		break;
		
		default:{
			//ipcByte should be the byte you sent from external ARM Core through sendByteIPC(ipcByte);
		}
		break;
	}
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void Timer0handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void Timer1handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void Timer2handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void Timer3handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void HblankUser(){
	int DS_VCOUNT = REG_VCOUNT;
	struct sIPCSharedTGDSSpecific * TGDSUSERIPC = (struct sIPCSharedTGDSSpecific *)TGDSIPCUserStartAddress;
	TGDSUSERIPC->APU_ADDR_CMD = 0xFFFFFFFF;
	
	switch(DS_VCOUNT){
		case(192):{
			PPU_updateGFX(0);
		}
		break;
		default:{
			if(DS_VCOUNT < 192){
				PPU_updateGFX(DS_VCOUNT);
			}
		}
		break;
	}
	
	TGDSUSERIPC->APU_ADDR_CMD = 0;
    //printf("hblank! \n");	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void VblankUser(){	
	
	bool nifiRunning = donifi((int)REG_VCOUNT);
	
	GFX.DSFrame++;
	GFX.v_blank=1;
	
	struct sIPCSharedTGDSSpecific * TGDSUSERIPC = (struct sIPCSharedTGDSSpecific *)TGDSIPCUserStartAddress;
	struct s_apu2 *APU2 = (struct s_apu2 *)(&TGDSUSERIPC->APU2);
	// FIX APU cycles
#if 0	
	if (/*CFG.Sound_output && */APU2->counter > 100 && APU2->counter < 261)
	TGDSUSERIPC->APU_ADDR_CNT += 261 - APU2->counter;
#endif		
	//*APU_ADDR_CNT += 262;
	if (CFG.Sound_output)
	TGDSUSERIPC->APU_ADDR_CNT = APU_MAX;
	APU2->counter = 0;
	
	//printf("vblank! \n");	
}


#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void VcounterUser(){
	
}


//Note: this event is hardware triggered from ARM7, on ARM9 a signal is raised through the FIFO hardware
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void screenLidHasOpenedhandlerUser(){
	setBacklight(POWMAN_BACKLIGHT_TOP_BIT | POWMAN_BACKLIGHT_BOTTOM_BIT);	//both lit screens
}

//Note: this event is hardware triggered from ARM7, on ARM9 a signal is raised through the FIFO hardware
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void screenLidHasClosedhandlerUser(){
	
}
