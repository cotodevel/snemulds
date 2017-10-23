#include "InterruptsARMCores_h.h"
#include "specific_shared.h"
#include "gui.h"
#include "dsregs_asm.h"
#include "fs.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"
#include "ram.h"
#include "core.h"
#include "conf.h"
#include "frontend.h"
#include "main.h"
#include "font_8x8_uv.h"
#include "ppu.h"
#include "keypad.h"
#include "interrupts.h"
#include "dswnifi.h"



//jumps here when desync
__attribute__((section(".itcm")))
void Vcounter(){

	//stuff that need to run only once per frames
	//frames lost here
	/*
	if(getWifiStartedFlag() == true){
		doGDBDaemon();
	}
	*/
	//printf("vcount! \n");
}


//---------------------------------------------------------------------------------
__attribute__((section(".itcm")))
void Vblank() {
//---------------------------------------------------------------------------------
	//handles DS-DS Comms
	if(doMULTIDaemon() >=0){
		do_multi();
	}
	
	//key event between frames
	do_keys();
	
	GFX.DSFrame++;
	GFX.v_blank=1;

	//FIX APU cycles
#if 0	
	if (APU.counter > 100 && APU.counter < 261)
	*APU_ADDR_CNT += 261 - APU.counter;
#endif		
	//*APU_ADDR_CNT += 262;
	if (CFG.Sound_output)
	*APU_ADDR_CNT = APU_MAX;
	APU2->counter = 0;

	//printf("vblank! \n");	
}


//---------------------------------------------------------------------------------
__attribute__((section(".itcm")))
void Hblank() {
//---------------------------------------------------------------------------------
	*APU_ADDR_CMD = 0xFFFFFFFF;
	if (REG_VCOUNT == 192) // We are last scanline, update first line GFX
	{
		PPU_updateGFX(0);
	}
	else if (REG_VCOUNT > 192)
	{
		//nothing
	}
	else{
		PPU_updateGFX(REG_VCOUNT);
	}
	*APU_ADDR_CMD = 0;
	
    //printf("hblank! \n");	
}