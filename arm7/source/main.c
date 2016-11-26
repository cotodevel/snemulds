/*---------------------------------------------------------------------------------

	default ARM7 core

		Copyright (C) 2005 - 2010
		Michael Noland (joat)
		Jason Rogers (dovoto)
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
#include <nds.h>
//#include <stdio.h>
#include "pocketspc.h"
#include "apu.h"
#include "dsp.h"
#include "main.h"
#include "interrupts/fifo_handler.h"
#include "interrupts/interrupts.h"
#include "interrupts/cpu_utils.h"
#include "../../common/common.h"

// Play buffer, left buffer is first MIXBUFSIZE * 2 u16's, right buffer is next
u16 *playBuffer;
volatile int soundCursor;
int apuMixPosition;
int pseudoCnt;
int frame = 0;
int scanlineCount = 0;
u32 interrupts_to_wait_arm7 = 0;
bool paused = true;
bool SPC_disable = true;
bool SPC_freedom = false;

//snemulDS stuff
void SetupSound() {
    soundCursor = 0;
	apuMixPosition = 0;

    powerOn((PM_Bits)POWER_SOUND);
    REG_SOUNDCNT = SOUND_ENABLE | SOUND_VOL(0x7F);

    TIMER0_DATA = TIMER_FREQ(MIXRATE);
    TIMER0_CR = TIMER_DIV_1 | TIMER_ENABLE;

    TIMER1_DATA = 0x10000 - MIXBUFSIZE;
    TIMER1_CR = TIMER_CASCADE | TIMER_IRQ_REQ | TIMER_ENABLE;

    // Timing stuff
#if PROFILING_ON  
    TIMER2_DATA = 0;
    TIMER2_CR = TIMER_DIV_64 | TIMER_ENABLE;

    TIMER3_DATA = 0;
    TIMER3_CR = TIMER_CASCADE | TIMER_ENABLE;
#endif    
}
 
void StopSound() {
    powerOff((PM_Bits)POWER_SOUND);
    REG_SOUNDCNT = 0;
    TIMER0_CR = 0;
    TIMER1_CR = 0;
}

void LoadSpc(const u8 *spc) {
// 0 - A, 1 - X, 2 - Y, 3 - RAMBASE, 4 - DP, 5 - PC (Adjusted into rambase)
// 6 - Cycles (bit 0 - C, bit 1 - v, bit 2 - h, bits 3+ cycles left)
// 7 - Optable
// 8 - NZ

    APU_STATE[0] = spc[0x27]<<24; // A
    APU_STATE[1] = spc[0x28]<<24; // X
    APU_STATE[2] = spc[0x29]<<24; // Y
    SetStateFromRawPSW(APU_STATE, spc[0x2A]);
    APU_SP = 0x100 | spc[0x2B]; // SP
    APU_STATE[5] = APU_STATE[3] + (spc[0x25] | (spc[0x26] << 8)); // PC    

#if defined (APU_MEM_IN_VRAM) || defined (APU_MEM_IN_RAM) 
    //for (int i=0; i<=0xffff; i++) APU_MEM[i] = spc[0x100 + i];
    memcpy(APU_MEM, spc+0x100, 65536);
#endif    
    /*for (int i=0; i<=0x7f; i++) {
        DSP_MEM[i] = spc[0x10100 + i];
    }*/
    //for (int i=0; i<=0x3f; i++) APU_EXTRA_MEM[i] = spc[0x101c0 + i];
    memcpy(DSP_MEM, spc+0x10100, 0x80);
    memcpy(APU_EXTRA_MEM, spc+0x101c0, 0x40);   

    ApuPrepareStateAfterReload();    
    DspPrepareStateAfterReload();    
}

void SaveSpc(u8 *spc) {
// 0 - A, 1 - X, 2 - Y, 3 - RAMBASE, 4 - DP, 5 - PC (Adjusted into rambase)
// 6 - Cycles (bit 0 - C, bit 1 - v, bit 2 - h, bits 3+ cycles left)
// 7 - Optable
// 8 - NZ
    uint32 savePC;

    savePC =  APU_STATE[5] - APU_STATE[3];
    spc[0x25] = savePC & 0xFF;    
    spc[0x26] = (savePC >> 8) & 0xFF;
    spc[0x27] = APU_STATE[0] >> 24; // A
    spc[0x28] = APU_STATE[1] >> 24; // X
    spc[0x29] = APU_STATE[2] >> 24; // Y
    spc[0x2A] = MakeRawPSWFromState(APU_STATE);
    spc[0x2B] = APU_SP & 0xFF; // SP

#if defined (APU_MEM_IN_VRAM) || defined (APU_MEM_IN_RAM)
    //for (int i=0; i<=0xffff; i++) spc[0x100 + i] = APU_MEM[i];
    memcpy(spc+0x100, APU_MEM, 65536);
#endif    
/*    for (int i=0; i<=0x7f; i++) {
        spc[0x10100 + i] = DSP_MEM[i];
    }
    for (int i=0; i<=0x3f; i++) 
    	spc[0x101c0 + i] = APU_EXTRA_MEM[i];*/
    memcpy(spc+0x10100, DSP_MEM, 0x80);
    memcpy(spc+0x101c0, APU_EXTRA_MEM, 0x40);       	
}

int NDSType=0;
u8 *bootstub;
type_void bootstub_arm7;
static void sys_exit(){
	//if(!bootstub_arm7){
		if(NDSType>=2)writePowerManagement(0x10, 1);
		else writePowerManagement(0, PM_SYSTEM_PWR);
	//}
	//bootstub_arm7(); //won't return
}

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
	
    int i   = 0;
	REG_IME = 0;

	// Reset the clock if needed
    rtcReset();

	// Block execution until we get control of vram D
	while (!(*((vu8*)0x04000240) & 0x2));
	
    playBuffer = (u16*)0x6000000;
    
    for (i = 0; i < MIXBUFSIZE * 4; i++) {
        playBuffer[i] = 0;
    }

    interrupts_to_wait_arm7 = IRQ_HBLANK | IRQ_VBLANK | IRQ_VCOUNT | IRQ_FIFO_NOT_EMPTY | IRQ_TIMER1 | IRQ_NETWORK;
    
    irqInit();
    fifoInit();
    
    REG_IPC_SYNC = 0;
    REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR | IPC_FIFO_RECV_IRQ;
    REG_IPC_FIFO_CR |= (1<<2);  //send empty irq
    REG_IPC_FIFO_CR |= (1<<10); //recv empty irq
	
    irqSet(IRQ_HBLANK,hblank);
    irqSet(IRQ_VBLANK, vblank);
	irqSet(IRQ_VCOUNT,vcounter);
    irqSet(IRQ_TIMER1,timer1);
    irqSet(IRQ_FIFO_NOT_EMPTY,HandleFifo);
    
	//upon data transfer IRQ_CARD
	irqEnable(interrupts_to_wait_arm7);
    
    *(u16*)0x04000184 = *(u16*)0x04000184 | (1<<15); //enable fifo send recv

    //set up ppu: do irq on hblank/vblank/vcount/and vcount line is 159
    REG_DISPSTAT = REG_DISPSTAT | DISP_HBLANK_IRQ |  DISP_VBLANK_IRQ | DISP_YTRIGGER_IRQ | (VCOUNT_LINE_INTERRUPT << 15);
    
    REG_IF = ~0;
    REG_IME = 1;
    
    update_spc_ports(); //updates asm ipc apu core with snemulds IPC apu ports 
    ApuReset();
    DspReset();
    SetupSound();
    
	{
		NDSType=0;
		u32 myself = readPowerManagement(4); //(PM_BACKLIGHT_LEVEL);
		if(myself & (1<<6))
			NDSType=(myself==readPowerManagement(5))?1:2;
	}
	setPowerButtonCB(sys_exit);
	bootstub=(u8*)0x02ff4000;
	bootstub_arm7=(*(u64*)bootstub==0x62757473746F6F62ULL)?(*(type_void*)(bootstub+0x0c)):0;
   
    while (1) {
        
        //Coto: Sound is best handled when NDS is NOT waiting for interrupt. 
        if(!SPC_disable){
            int cyclesToExecute, samplesToMix;
			//if (scanlineCount >= 66) {
			//	scanlineCount -= 66;
			//	samplesToMix = 17;
			//	cyclesToExecute = spcCyclesPerSec / (32000 / 3);
			//} else {
			//	samplesToMix = 16;
			//	cyclesToExecute = spcCyclesPerSec / (32000 / 2);
			//}
			cyclesToExecute = spcCyclesPerSec / (32000 / 2);
			ApuExecute(cyclesToExecute);
			
			if (scanlineCount >= 16) {
				scanlineCount -= 16;		
				samplesToMix = 32;
				if (apuMixPosition + samplesToMix > MIXBUFSIZE * 2) {
					int tmp = (apuMixPosition + samplesToMix) - (MIXBUFSIZE * 2);
					if (tmp != samplesToMix) {
						DspMixSamplesStereo(samplesToMix - tmp, &playBuffer[apuMixPosition]);
					}
					samplesToMix = tmp;
					apuMixPosition = 0;
				}
				DspMixSamplesStereo(samplesToMix, &playBuffer[apuMixPosition]);
				apuMixPosition += samplesToMix;								
			}			

            
        }
        else
            //swiWaitForVBlank();
            swiIntrWait(1,interrupts_to_wait_arm7);
    }
   
	return 0;
}
