#include <nds.h>
#include <nds/system.h>
#include <nds/interrupts.h>

#include <stdio.h>
#include <nds/touch.h>
#include <nds/registers_alt.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "../main.h"
#include "../fs.h"
#include "../snes.h"
#include "../gfx.h"
#include "../cfg.h"
#include "../apu.h"
#include "../m3sd.h"
#include "../mpu/pu.h"
#include "../memmap.h"
#include "../common.h"              //snes common
#include "../../../common/common.h" //NDS IPC

#include "fifo_handler.h"

#include "interrupts.h"


IN_ITCM
void vcounter(){
    //iprintf("vcount! \n");
    REG_IF = IRQ_VCOUNT;
}

//---------------------------------------------------------------------------------
IN_ITCM
void Vblank() {
//---------------------------------------------------------------------------------
	
	GFX.DSFrame++;
	GFX.v_blank=1;

	//FIX APU cycles
#if 0	
	if (APU.counter > 100 && APU.counter < 261)
	*APU_ADDR_CNT += 261 - APU.counter;
#endif		
	//*APU_ADDR_CNT += 262;
	if (CFG.Sound_output)
	MyIPC->APU_ADDR_CNT = APU_MAX;
	MyIPC->counter = 0;


    //iprintf("vblank! \n");
    REG_IF = IRQ_VBLANK;
}

//---------------------------------------------------------------------------------
IN_ITCM
void Hblank() {
//---------------------------------------------------------------------------------
	MyIPC->APU_ADDR_CMD = 0xFFFFFFFF;

	if (REG_VCOUNT >= 192)
	{
		if (REG_VCOUNT == 192) // We are last scanline, update first line GFX
		{
			PPU_updateGFX(0);
		}
		MyIPC->APU_ADDR_CMD = 0;
	}
	else
	{
		PPU_updateGFX(REG_VCOUNT);
		//	h_blank=1;
		MyIPC->APU_ADDR_CMD = 0;
	}
    
    //iprintf("hblank! \n");
    REG_IF = IRQ_HBLANK;
}
