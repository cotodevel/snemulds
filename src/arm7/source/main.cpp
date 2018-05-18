#include "pocketspc.h"

#include "apu.h"
#include "dsp.h"

#include <stdio.h>

// Play buffer, left buffer is first MIXBUFSIZE * 2 u16's, right buffer is next
u16 *playBuffer;
volatile int soundCursor;
int apuMixPosition;
bool paused = false;
bool SPC_disable = false;
bool SPC_freedom = false;
extern "C" void IntrHandlerAsm();
extern "C" void BreakR11();
extern "C" void updateMyIPC(); // archeide

void SetupSound() {
    soundCursor = 0;
	apuMixPosition = 0;

    powerON(POWER_SOUND);
    SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);

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
    powerOFF(POWER_SOUND);
    SOUND_CR = 0;
    TIMER0_CR = 0;
    TIMER1_CR = 0;
}

inline void SendArm9Command(u32 command) {
    REG_IPC_FIFO_TX = command;
}

#define DEBUG_BUF ((char *)0x27FE200)

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

//    for (int i=0; i<=0xffff; i++) APU_MEM[i] = spc[0x100 + i];
    for (int i=0; i<=0x7f; i++) {
        DSP_MEM[i] = spc[0x10100 + i];
    }
    for (int i=0; i<=0x3f; i++) APU_EXTRA_MEM[i] = spc[0x101c0 + i];

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

//    for (int i=0; i<=0xffff; i++) spc[0x100 + i] = APU_MEM[i];
    for (int i=0; i<=0x7f; i++) {
        spc[0x10100 + i] = DSP_MEM[i];
    }
    for (int i=0; i<=0x3f; i++) 
    	spc[0x101c0 + i] = APU_EXTRA_MEM[i];
}


extern "C" {
void HandleFifo(u8 command) {
    switch (command) {
    case 1:
        // Reset
        StopSound();

    	for (int i = 0; i < MIXBUFSIZE * 4; i++) {
        	playBuffer[i] = 0;
    	}

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
    	LoadSpc(APU_RAM_ADDRESS-0x100);
        SetupSound();   	
		*APU_ADDR_CNT = 0;             	
    	paused = false;
    	SPC_freedom = true;
    	SPC_disable = false;
    	break;
    	
   	case 4: /* DISABLE */
    	SPC_disable = true;
    	*APU_ADDR_CNT = 0;    	
    	break;        
    
    case 5: /* CLEAR MIXER BUFFER */
    	for (int i = 0; i < MIXBUFSIZE * 4; i++) {
        	playBuffer[i] = 0;
    	}
    	break;

    case 6: /* SAVE state */
 		SaveSpc(APU_RAM_ADDRESS-0x100);
    	break;  
    	
   	case 7: /* LOAD state */
    	LoadSpc(APU_RAM_ADDRESS-0x100);
    	*APU_ADDR_CNT = 0; 
    	break;
    	        
    }
}


void InterruptHandler(void) {
    u32 flags = REG_IF & REG_IE;

    if (flags & IRQ_TIMER1) {
#if PROFILING_ON
        long long begin = TIMER2_DATA + ((long long)TIMER3_DATA << 19);
#endif
        soundCursor = MIXBUFSIZE - soundCursor;

		(*((uint32 *)SNEMUL_DBG1)) += 1;

        // Left channel
        int channel = soundCursor == 0 ? 0 : 1;
        SCHANNEL_TIMER(channel) = SOUND_FREQ(MIXRATE);
        SCHANNEL_SOURCE(channel) = (uint32)&(playBuffer[MIXBUFSIZE - soundCursor]);
        SCHANNEL_LENGTH(channel) = (MIXBUFSIZE * 2) >> 2;
        SCHANNEL_REPEAT_POINT(channel) = 0;
        SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0) | SOUND_16BIT;

        // Right channel
        channel = soundCursor == 0 ? 2 : 3;
        SCHANNEL_TIMER(channel) = SOUND_FREQ(MIXRATE);
        SCHANNEL_SOURCE(channel) = (uint32)&(playBuffer[(MIXBUFSIZE - soundCursor) + (MIXBUFSIZE * 2)]);
        SCHANNEL_LENGTH(channel) = (MIXBUFSIZE * 2) >> 2;
        SCHANNEL_REPEAT_POINT(channel) = 0;
        SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0x7F) | SOUND_16BIT;

#ifndef SCANLINE_SYNC
		vu32 * const scanlinesRun = (vu32*)(0x2800000-60);

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
//        SPC_IPC->dspTime += (TIMER2_DATA + ((long long)TIMER3_DATA << 19)) - end;
#endif
    }

    if (flags & IRQ_VBLANK) {
    	
		if (SPC_freedom)
		{
		vu32 * const scanlinesRun = (vu32*)(0x2800000-60);
			
		//*scanlinesRun += spcCyclesPerSec / (MIXRATE / MIXBUFSIZE) / 67;
//		*scanlinesRun = MIXBUFSIZE / 2;
		*scanlinesRun += 265;
//		const int cyclesToExecute = spcCyclesPerSec / (MIXRATE / MIXBUFSIZE);
/*        ApuExecute(cyclesToExecute * 21);
        DspMixSamplesStereo(MIXBUFSIZE, &playBuffer[soundCursor]);*/
		}
    	
/*    	     	
    	if (SPC_freedom)
    	{
    		vu32 * const scanlinesRun = (vu32*)(0x2800000-60);
    		*scanlinesRun = 262; // FIXME
    	}
*/    	
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
//   	    dspPreamp = *((vu16*)0x27ffff2);
        dspPreamp = 0x140;
        
        updateMyIPC();
/*		*(u32*)0x27ffff4=(REG_KEYINPUT|(REG_KEYXY<<10))^0x1ffff;
		*(u16*)0x27fffec=readtouch(TSC_MEASURE_X);
		*(u16*)0x27fffee=readtouch(TSC_MEASURE_Y);*/
//        rtcGetTime((u8*)0x27fff00);

    }

    VBLANK_INTR_WAIT_FLAGS |= flags;
    REG_IF = flags;
}
}

int main(int argc, char ** argv) {
    REG_IME = 0;

	// Reset the clock if needed
    rtcReset();

#if 1
	// Block execution until we get control of vram D
	while (!(*((vu8*)0x04000240) & 0x2));
#endif

//    playBuffer = (u16*)0x26A0000;

	playBuffer = (u16*)0x6000000;
    for (int i = 0; i < MIXBUFSIZE * 4; i++) {
        playBuffer[i] = 0;
    }


    // Set up the interrupt handler
	IRQ_HANDLER = &IntrHandlerAsm;

    REG_IE = IRQ_TIMER1 | IRQ_FIFO_NOT_EMPTY | IRQ_VBLANK | IRQ_HBLANK;
    DISP_SR = DISP_VBLANK_IRQ | DISP_HBLANK_IRQ;
    REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR | IPC_FIFO_RECV_IRQ;

    REG_IF = ~0;
    REG_IME = 1;

    ApuReset();
    DspReset();

    SetupSound();

	// This is incremented by the arm9 on every *SNES* scanline
	vu32 *scanlinesRun = (vu32*)(0x2800000-60);
	int scanlineCount = 0;
    for(;;) {
		if (SPC_disable)
		{
			*SNEMUL_ANS = 0xFF00FF00;
			swiWaitForVBlank();			
			continue;
		}
		if (SPC_freedom)
		{
//			swiWaitForVBlank();
//			continue;
		}
    	
#if 1    	
#ifndef SCANLINE_SYNC
		swiWaitForVBlank();
#else
		int localCache = *scanlinesRun;
/*		if (SPC_freedom)
			localCache = 1;*/ 
			
		if (localCache > 0) {
			// every snes scanline is 2 samples (roughly) - 31440 samples in 60fps * 262 scanlines.
			// so, we need to add an extra sample every 66 scanlines (roughly)
			localCache--;
			*scanlinesRun = localCache;

			scanlineCount++;
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
#else
			if (scanlineCount >= 12) {
				scanlineCount -= 12;
#endif							
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
#endif
#endif
    }

    return 0;
}
