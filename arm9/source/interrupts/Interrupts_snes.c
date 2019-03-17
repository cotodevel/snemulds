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

//User Handler Definitions
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer0handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer1handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer2handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer3handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
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
__attribute__((section(".dtcm")))
#endif
bool handleInputVcount = false;

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void VcounterUser(){
	handleInputVcount = true;
}



//Note: this event is hardware triggered from ARM7, on ARM9 a signal is raised through the FIFO hardware
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void ScreenlidhandlerUser(){

}
