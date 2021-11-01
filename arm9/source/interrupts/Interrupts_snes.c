#include "InterruptsARMCores_h.h"
#include "ipcfifoTGDSUser.h"
#include "consoleTGDS.h"
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
void IpcSynchandlerUser(uint8 ipcByte){
	switch(ipcByte){
		
		default:{
			//ipcByte should be the byte you sent from external ARM Core through sendByteIPC(ipcByte);
		}
		break;
	}
}

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
	
    //GUI_printf("hblank! \n");	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void VblankUser(){	
	
	GFX.DSFrame++;
	GFX.v_blank=1;
	struct sIPCSharedTGDSSpecific* TGDSIPCUSER = getsIPCSharedTGDSSpecific();
	struct s_apu2 *APU2 = (struct s_apu2 *)(&TGDSIPCUSER->APU2);
	// FIX APU cycles	
	//*APU_ADDR_CNT += 262;
	if (CFG.Sound_output)
	TGDSIPCUSER->APU_ADDR_CNT = APU_MAX;
	//GUI_printf("vblank! \n");	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void VcounterUser(){

}

//Note: this event is hardware triggered from ARM7, on ARM9 a signal is raised through the FIFO hardware
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void screenLidHasOpenedhandlerUser(){
	//if console top screen, shut off console
	if(GUI.consoleAtTopScreen == true){
		detectAndTurnOffConsole();
	}
}

//Note: this event is hardware triggered from ARM7, on ARM9 a signal is raised through the FIFO hardware
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void screenLidHasClosedhandlerUser(){
	
}
