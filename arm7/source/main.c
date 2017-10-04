#include <string.h>
#include "pocketspc.h"
#include "apu.h"
#include "dsp.h"
#include "main.h"
#include "InterruptsARMCores_h.h"	//new
#include "interrupts.h"	//ori

#include "specific_shared.h"
#include "wifi_arm7.h"
#include "usrsettings.h"
#include "timer.h"

// Play buffer, left buffer is first MIXBUFSIZE * 2 uint16's, right buffer is next
uint16 *playBuffer;
volatile int soundCursor;
int apuMixPosition;
int pseudoCnt;
int frame = 0;
int scanlineCount = 0;
bool paused = true;
bool SPC_disable = true;
bool SPC_freedom = false;

void SetupSound() {
    soundCursor = 0;
	apuMixPosition = 0;

    powerON(POWER_SOUND);
    REG_SOUNDCNT = SOUND_ENABLE | SOUND_VOL(0x7F);

    TIMERXDATA(0) = TIMER_FREQ(MIXRATE);
    TIMERXCNT(0) = TIMER_DIV_1 | TIMER_ENABLE;

    TIMERXDATA(1) = 0x10000 - MIXBUFSIZE;
    TIMERXCNT(1) = TIMER_CASCADE | TIMER_IRQ_REQ | TIMER_ENABLE;

    // Debug
	#if PROFILING_ON  
		TIMERXDATA(2) = 0;
		TIMERXCNT(2) = TIMER_DIV_64 | TIMER_ENABLE;

		TIMERXDATA(3) = 0;
		TIMERXCNT(3) = TIMER_CASCADE | TIMER_ENABLE;
	#endif    
}
 
void StopSound() {
    powerOFF(POWER_SOUND);
    REG_SOUNDCNT = 0;
    TIMERXCNT(0) = 0;
    TIMERXCNT(1) = 0;
}

void LoadSpc(const uint8 *spc) {
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

void SaveSpc(uint8 *spc) {
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

//---------------------------------------------------------------------------------
int main(int _argc, sint8 **_argv) {
//---------------------------------------------------------------------------------
	
	//ori
	/*
	interrupts_to_wait_arm7 = IRQ_TIMER1 | IRQ_HBLANK | IRQ_VBLANK | IRQ_VCOUNT | IRQ_FIFO_NOT_EMPTY;  //| IRQ_FIFO_EMPTY;    
	
    //fifo setups
    irqInitExt(IntrMainExt);
	fifoInit();
    
	installWifiFIFO();
	
    irqSet(IRQ_HBLANK,Hblank);
	irqSet(IRQ_VBLANK, Vblank);
	irqSet(IRQ_VCOUNT,Vcounter);
    irqSet(IRQ_TIMER1, Timer1handler);
	irqSet(IRQ_FIFO_NOT_EMPTY,HandleFifoNotEmpty);
    //irqSet(IRQ_FIFO_EMPTY,HandleFifoEmpty);
    
	irqEnable(interrupts_to_wait_arm7);
    
	REG_IPC_SYNC = 0;
    REG_IPC_FIFO_CR = IPC_FIFO_RECV_IRQ | IPC_FIFO_SEND_IRQ | IPC_FIFO_ENABLE;
    
    //set up ppu: do irq on hblank/vblank/vcount/and vcount line is 159
    REG_DISPSTAT = REG_DISPSTAT | DISP_HBLANK_IRQ | DISP_VBLANK_IRQ | DISP_YTRIGGER_IRQ | (VCOUNT_LINE_INTERRUPT << 15);
	*/
	
	IRQInit();
	
	// Block execution until we get control of vram D
	while (!(*((vuint8*)0x04000240) & 0x2));
	
	//Read DHCP settings (in order)
	LoadFirmwareSettingsFromFlash();
	installWifiFIFO();
	
    playBuffer = (uint16*)0x6000000;
    int i   = 0;
    for (i = 0; i < MIXBUFSIZE * 4; i++) {
        playBuffer[i] = 0;
    }
	
	update_spc_ports(); //updates asm ipc apu core with snemulds IPC apu ports 
    ApuReset();
    DspReset();
    SetupSound();
    
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
		//ori
		//else
			//swiWaitForVBlank();
		
		
		IRQVBlankWait();	//required for sound playback sync with vblank
	}
   
	return 0;
}