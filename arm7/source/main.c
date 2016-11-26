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
#include <nds/memory.h>
#include <nds/system.h>
#include <unistd.h>

//#include <stdio.h>
#include "pocketspc.h"
#include "apu.h"
#include "dsp.h"
#include "apumisc.h"
#include "main.h"

// Play buffer, left buffer is first MIXBUFSIZE * 2 u16's, right buffer is next
u16 *playBuffer;
volatile int soundCursor;
int apuMixPosition;
int pseudoCnt;
int frame = 0;
int scanlineCount = 0;
bool paused = true;
bool SPC_disable = true;
bool SPC_freedom = false;

//---------------------------------------------------------------------------------
void HblankHandler(void) {
//---------------------------------------------------------------------------------
     	// Block execution until the hblank processing on ARM9
/*     	int	i;
     	for (i = 0; i < 100; i++)
     		pseudoCnt++;*/
/*        if (!SPC_disable)
        {     		
     	while (*SNEMUL_CMD == 0);
     	while (*SNEMUL_CMD == 0xFFFFFFFF);
        }*/
#if 1        
        if (!SPC_disable)
      	{
        //while (*SNEMUL_CMD == 0);
		int VCount = REG_VCOUNT;        

#ifndef USE_SCANLINE_COUNT
		//if (scanlineCount < 20)
			scanlineCount++;
#endif	
/*		if (VCount == 80)
		{		
			updateMyIPC();
		} else*/
		{
		 
		uint32 T0 = APU_MEM[APU_TIMER0]?APU_MEM[APU_TIMER0]:0x100;
		uint32 T1 = APU_MEM[APU_TIMER1]?APU_MEM[APU_TIMER1]:0x100;
		uint32 T2 = APU_MEM[APU_TIMER2]?APU_MEM[APU_TIMER2]:0x100;
		
		//*((vu32*)0x27E0004) = APU2->T0;
		//*((vu32*)0x27E0004) = scanlineCount;

//	    if (VCount & 63) {
	      if ((VCount & 1) == 1) {        		      	
	        if (++APU2->TIM0 >= T0) {
	          APU2->TIM0 -= T0;
	          APU_MEM[APU_COUNTER0]++;
	          APU_MEM[APU_COUNTER0] &= 0xf;
	        }
	        if (++APU2->TIM1 >= T1) {
	          APU2->TIM1 -= T1;
	          APU_MEM[APU_COUNTER1]++;
	          APU_MEM[APU_COUNTER1] &= 0xf;
	        }
	      }
	      APU2->TIM2 += 4;
	      if (APU2->TIM2 >= T2) {
	        APU2->TIM2 -= T2;
            APU_MEM[APU_COUNTER2]++;
	        APU_MEM[APU_COUNTER2] &= 0xf;
	      }
		}
//	    }
	    //while (*SNEMUL_CMD == 0xFFFFFFFF);
      	}
#endif
}

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
inputGetAndSend();
#ifdef USE_SCANLINE_COUNT        	
		if (SPC_freedom)
		{
		vu32 * const scanlinesRun = (vu32*)(0x2800000-60);
		*scanlinesRun += 265;		
			
		//*scanlinesRun += spcCyclesPerSec / (MIXRATE / MIXBUFSIZE) / 67;
//		*scanlinesRun = MIXBUFSIZE / 2;

//		const int cyclesToExecute = spcCyclesPerSec / (MIXRATE / MIXBUFSIZE);
/*        ApuExecute(cyclesToExecute * 21);
        DspMixSamplesStereo(MIXBUFSIZE, &playBuffer[soundCursor]);*/
		}
#endif		
    	
#if PROFILING_ON
        // Debug time data
        SPC_IPC->curTime += TIMER2_DATA | ((long long)TIMER3_DATA << 19);
        TIMER2_CR = 0;
        TIMER3_CR = 0;
        TIMER2_DATA = 0;
        TIMER2_CR = TIMER_DIV_64 | TIMER_ENABLE;
        TIMER3_DATA = 0;
        TIMER3_CR = TIMER_CASCADE | TIMER_ENABLE;
#endif
}

//---------------------------------------------------------------------------------
void __attribute__((hot)) Timer1Handler() {
//---------------------------------------------------------------------------------
#if PROFILING_ON
    long long begin = TIMER2_DATA + ((long long)TIMER3_DATA << 19);
#endif
    soundCursor = MIXBUFSIZE - soundCursor;

#if 1
    // Left channel
    int channel = soundCursor == 0 ? 0 : 1;
    SCHANNEL_TIMER(channel) = SOUND_FREQ(MIXRATE);
    SCHANNEL_SOURCE(channel) = (uint32)&(playBuffer[MIXBUFSIZE - soundCursor]);
    SCHANNEL_LENGTH(channel) = (MIXBUFSIZE * 2) >> 2;
    SCHANNEL_REPEAT_POINT(channel) = 0;
    SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0) | SOUND_FORMAT_16BIT;

    // Right channel
    channel = soundCursor == 0 ? 2 : 3;
    SCHANNEL_TIMER(channel) = SOUND_FREQ(MIXRATE);
    SCHANNEL_SOURCE(channel) = (uint32)&(playBuffer[(MIXBUFSIZE - soundCursor) + (MIXBUFSIZE * 2)]);
    SCHANNEL_LENGTH(channel) = (MIXBUFSIZE * 2) >> 2;
    SCHANNEL_REPEAT_POINT(channel) = 0;
    SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0x7F) | SOUND_FORMAT_16BIT;

#ifndef SCANLINE_SYNC
	vu32 * const scanlinesRun = (vu32*)(0x3000000-60);

	if (*scanlinesRun < 20) {
		// Mix into soundCursor now
		const int cyclesToExecute = spcCyclesPerSec / (MIXRATE / MIXBUFSIZE);
		ApuExecute(cyclesToExecute);
	    DspMixSamplesStereo(MIXBUFSIZE, &playBuffer[soundCursor]);
		*scanlinesRun += cyclesToExecute / 67;
	}

#endif

#if PROFILING_ON
    long long end = TIMER2_DATA + ((long long)TIMER3_DATA << 19);
    SPC_IPC->cpuTime += end - begin;
//    SPC_IPC->dspTime += (TIMER2_DATA + ((long long)TIMER3_DATA << 19)) - end;
#endif

#endif
}

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
    //ori: memcpy(APU_MEM, spc+0x100, 65536); //byte
	dmaCopyHalfWords(1,(void *)(spc+0x100),(void *)APU_MEM, 0x8000); //halfw
	
#endif    
    /*for (int i=0; i<=0x7f; i++) {
        DSP_MEM[i] = spc[0x10100 + i];
    }*/
	dmaCopyHalfWords(1,(void *)(spc+0x10100),(void *)DSP_MEM, 0x40); //halfw
	
    //for (int i=0; i<=0x3f; i++) APU_EXTRA_MEM[i] = spc[0x101c0 + i];
    //ori: memcpy(DSP_MEM, spc+0x10100, 0x80); //byte
    dmaCopyHalfWords(1,(void *)(spc+0x10100),(void *)DSP_MEM, 0x40); //halfw
	
	//ori: memcpy(APU_EXTRA_MEM, spc+0x101c0, 0x40);   //byte
	dmaCopyHalfWords(1,(void *)(spc+0x101c0),(void *)APU_EXTRA_MEM, 0x20); //halfw
	
	
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
    //ori: memcpy(spc+0x100, APU_MEM, 65536);
	
	dmaCopyHalfWords(2,(void *)APU_MEM,(void *)(spc+0x100), 0x8000);
	
#endif    
/*    for (int i=0; i<=0x7f; i++) {
        spc[0x10100 + i] = DSP_MEM[i];
    }
    for (int i=0; i<=0x3f; i++) 
    	spc[0x101c0 + i] = APU_EXTRA_MEM[i];*/
		
    //ori: memcpy(spc+0x10100, DSP_MEM, 0x80); //byte
    dmaCopyHalfWords(2,(void *)DSP_MEM , (void *)(spc+0x10100), 0x40); //halfw
	
	//ori: memcpy(spc+0x101c0, APU_EXTRA_MEM, 0x40);    
	dmaCopyHalfWords(2,(void *)APU_EXTRA_MEM , (void *)(spc+0x101c0), 0x20); //halfw
	
}

static __attribute__((hot)) void HandleFifo(u32 value, void* data) {
    switch (value&0xFFFF) {
		case 1:
			// Reset
			StopSound();

			//ori: _memset(playBuffer, 0, MIXBUFSIZE * 8); //byte
			dmaFillHalfWords (0,(void *)(playBuffer) , (int)((MIXBUFSIZE * 4)));
	
			*APU_ADDR_CNT = 0; 
			ApuReset();
			DspReset();

			SetupSound();
			paused = false;
			SPC_disable = false;
			SPC_freedom = false;
			break;
		case 2: 
			// Pause/unpause
			if (!paused) {
				StopSound();
			} else {
				SetupSound();
			}
			if (SPC_disable)
				SPC_disable = false;        
			paused = !paused;        
			break;
		case 3: /* PLAY SPC */
			LoadSpc(APU_SNES_ADDRESS-0x100);
			SetupSound();   	
			*APU_ADDR_CNT = 0;             	
			paused = false;
			SPC_freedom = true;
			SPC_disable = false;
			break;
			
		case 4: /* DISABLE */   	
			SPC_disable = true;
			break;        
		
		case 5: /* CLEAR MIXER BUFFER */
			//ori: _memset(playBuffer, 0, MIXBUFSIZE * 8);
			dmaFillHalfWords (0,(void *)(playBuffer) , (int)(MIXBUFSIZE * 4));
			
			break;

		case 6: /* SAVE state */
			SaveSpc(APU_SNES_ADDRESS-0x100);
			break;  
			
		case 7: /* LOAD state */
			LoadSpc(APU_SNES_ADDRESS-0x100);
			*APU_ADDR_CNT = 0; 
			break;
			
		case 8:
			writePowerManagement(PM_SOUND_AMP, (int)data>>16); 
			break; 
			
		default:
			break;
    }
	
	fifoSendValue32(FIFO_USER_01,1);
}

int NDSType=0;
u8 *bootstub;
typedef void (*type_void)();
type_void bootstub_arm7;
static void sys_exit(){
	if(NDSType>=2)writePowerManagement(0x10, 1);
	else writePowerManagement(0, PM_SYSTEM_PWR);
}

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
	// read User Settings from firmware
	readUserSettings();
	
	irqInit();
	// Start the RTC tracking IRQ
	initClockIRQ();
	fifoInit();
	
#if 1
	// Block execution until we get control of vram D
	while (!(*((vu8*)0x04000240) & 0x2));
#endif

	SetYtrigger(80);
	
	fifoSetValue32Handler(FIFO_USER_01, HandleFifo, 0); 

	installSystemFIFO();
	
	playBuffer = (u16*)0x6000000;
    //ori: _memset(playBuffer, 0, MIXBUFSIZE * 8);
	dmaFillHalfWords(0,(void *)(playBuffer) , (int)(MIXBUFSIZE * 4));
	
	irqSet(IRQ_HBLANK, HblankHandler); //this seems to cause sync problems
	irqSet(IRQ_TIMER1, Timer1Handler);
	irqSet(IRQ_VBLANK, VblankHandler);
	irqEnable( IRQ_HBLANK | IRQ_TIMER1 | IRQ_VBLANK);  
	
	REG_DISPSTAT = DISP_VBLANK_IRQ | DISP_HBLANK_IRQ;
	
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

#ifdef USE_SCANLINE_COUNT
	// This is incremented by the arm9 on every *SNES* scanline
	vu32 *scanlinesRun = (vu32*)(0x3000000-60);
#endif
	// Keep the ARM7 mostly idle
	while (1) {
		if(0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))){
			sys_exit();
		}
		//inputGetAndSend();
		//continue;
		
		if (SPC_disable)
		{
			swiWaitForVBlank();			
			continue;
		}
		
#ifndef SCANLINE_SYNC
		swiWaitForVBlank();
#else
#ifdef USE_SCANLINE_COUNT
		int localCache = *scanlinesRun;
/*		if (SPC_freedom)
			localCache = 1;*/ 
			
		if (localCache > 0) {
			// every snes scanline is 2 samples (roughly) - 31440 samples in 60fps * 262 scanlines.
			// so, we need to add an extra sample every 66 scanlines (roughly)
			localCache--;
			*scanlinesRun = localCache;

			scanlineCount++;
#endif			
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
			
#if 1	
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
#endif /* 1 */	

#ifdef USE_SCANLINE_COUNT
		}
#endif		

#endif /* SCANLINESYNC */
	}
	return 0;
}