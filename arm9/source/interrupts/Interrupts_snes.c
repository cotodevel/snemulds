#include "InterruptsARMCores_h.h"
#include "specific_shared.h"
#include "guiTGDS.h"
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
#include "keypadTGDS.h"

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
	getsIPCSharedTGDSSpecific()->APU_ADDR_CMD = 0xFFFFFFFF;

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
	getsIPCSharedTGDSSpecific()->APU_ADDR_CMD = 0;
	
    //printf("hblank! \n");	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void VblankUser(){	
	
	GFX.DSFrame++;
	GFX.v_blank=1;
	struct s_apu2 *APU2 = (struct s_apu2 *)(&getsIPCSharedTGDSSpecific()->APU2);
	// FIX APU cycles
#if 0	
	if (/*CFG.Sound_output && */APU2->counter > 100 && APU2->counter < 261)
	getsIPCSharedTGDSSpecific()->APU_ADDR_CNT += 261 - APU2->counter;
#endif		
	//*APU_ADDR_CNT += 262;
	if (CFG.Sound_output)
	getsIPCSharedTGDSSpecific()->APU_ADDR_CNT = APU_MAX;
	APU2->counter = 0;


	//printf("vblank! \n");	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void VcounterUser(){
}