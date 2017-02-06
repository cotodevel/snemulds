#include "touch_ipc.h"
#include <nds.h>
#include <nds/touch.h>
#include "common_shared.h"

__attribute__((section(".itcm")))
void touchRead_customIPC(touchPosition * touchpos_inst){
    
    touchpos_inst->rawx =   MyIPC->touchX;
    touchpos_inst->rawy =   MyIPC->touchY;
    
    //TFT x/y pixel
    touchpos_inst->px   =   MyIPC->touchXpx;
    touchpos_inst->py   =   MyIPC->touchYpx;
    
    touchpos_inst->z1   =   MyIPC->touchZ1;
    touchpos_inst->z2   =   MyIPC->touchZ2;
	
}

__attribute__((section(".itcm")))
u32 keyscurr_ipc(){
	return (( ((~REG_KEYINPUT)&0x3ff) | (((~MyIPC->buttons_xy_folding)&3)<<10) | (((~MyIPC->buttons_xy_folding)<<6) & (KEY_TOUCH|KEY_LID) ))^KEY_LID);
}