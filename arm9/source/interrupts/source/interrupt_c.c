#include "interrupts.h"
#include "common_shared.h"
#include "gui.h"

#include "fs.h"
#include "snes.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"

#include "ram.h"

#include "conf.h"
#include "snemul_str.h"
#include "frontend.h"
#include "main.h"
#include "font_8x8_uv.h"

#include "ppu.h"
#include "libfat.h"


volatile u32 interrupts_to_wait_arm9 = 0;


//jumps here when desync
__attribute__((section(".itcm")))
void vcounter(){
	
	//stuff that need to run only once per frames
	//sendcmd();	//exception here cant
		
	//consoletext(64*2-32,"     vcount!__________________________",0);	//works
}


//---------------------------------------------------------------------------------
__attribute__((section(".itcm")))
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
}

//---------------------------------------------------------------------------------
__attribute__((section(".itcm")))
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
	
}




/*---------------------------------------------------------------------------------
	Copyright (C) 2005
		Dave Murphy (WinterMute)
	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.
	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:
	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source
		distribution.
---------------------------------------------------------------------------------*/


//we need the irq handler exposed (so other projects that use custom IRQ handler can benefit..)

//---------------------------------------------------------------------------------
void irqInitExt(IntFn handler) {
//---------------------------------------------------------------------------------
	int i;

	irqInitHandler(handler);

	// Set all interrupts to dummy functions.
	for(i = 0; i < MAX_INTERRUPTS; i ++)
	{
		irqTable[i].handler = irqDummy;
		irqTable[i].mask = 0;
	}

#ifdef ARM7
	if (isDSiMode()) {
		irqSetAUX(IRQ_I2C, i2cIRQHandler);
		irqEnableAUX(IRQ_I2C);
	}
#endif
	REG_IME = 1;			// enable global interrupt
}