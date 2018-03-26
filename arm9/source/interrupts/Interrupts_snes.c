#include "InterruptsARMCores_h.h"
#include "specific_shared.h"
#include "gui.h"
#include "dsregs_asm.h"
#include "fs.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"
//#include "ram.h"
#include "core.h"
#include "conf.h"
//#include "frontend.h"
#include "main.h"
//#include "font_8x8_uv.h"
#include "ppu.h"
#include "keypad.h"



//jumps here when desync
__attribute__((section(".itcm")))
void Vcounter(){

	//stuff that need to run only once per frames
	//sendcmd();	//UDP Netplay: exception here cant
	
	//printf("vcount! \n");
}


//---------------------------------------------------------------------------------
__attribute__((section(".itcm")))
void Vblank() {
//---------------------------------------------------------------------------------
	//key event between frames
	do_keys();
	
	
	GFX.DSFrame++;
	GFX.v_blank=1;

	// FIX APU cycles
#if 0	
	if (/*CFG.Sound_output && */APU2->counter > 100 && APU2->counter < 261)
	*APU_ADDR_CNT += 261 - APU2->counter;
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

	if (REG_VCOUNT >= 192)
	{
		if (REG_VCOUNT == 192) // We are last scanline, update first line GFX

		{
			PPU_updateGFX(0);
		}
		goto end;
	}

	PPU_updateGFX(REG_VCOUNT);

	//	h_blank=1;
	end:
	*APU_ADDR_CMD = 0;
	
    //printf("hblank! \n");	
}