#include <nds.h>
#include <nds/system.h>
#include <nds/interrupts.h>

#include "../pocketspc.h"
#include "../apu.h"
#include "../dsp.h"
#include "../main.h"
#include "../mixrate.h"

#include "../touch.h"
#include "../../../common/common.h"

#include "fifo_handler.h"
#include "interrupts.h"

//Coto: rewritten all of this. IRQs are now fixed :)

void hblank(){
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
		
		//*((vu32*)0x27E0004) = MyIPC->T0;
		//*((vu32*)0x27E0004) = scanlineCount;

//	    if (VCount & 63) {
	      if ((VCount & 1) == 1) {        		      	
	        if (++MyIPC->TIM0 >= T0) {
	          MyIPC->TIM0 -= T0;
	          APU_MEM[APU_COUNTER0]++;
	          APU_MEM[APU_COUNTER0] &= 0xf;
	        }
	        if (++MyIPC->TIM1 >= T1) {
	          MyIPC->TIM1 -= T1;
	          APU_MEM[APU_COUNTER1]++;
	          APU_MEM[APU_COUNTER1] &= 0xf;
	        }
	      }
	      MyIPC->TIM2 += 4;
	      if (MyIPC->TIM2 >= T2) {
	        MyIPC->TIM2 -= T2;
            APU_MEM[APU_COUNTER2]++;
	        APU_MEM[APU_COUNTER2] &= 0xf;
	      }
		}
//	    }
	    //while (*SNEMUL_CMD == 0xFFFFFFFF);
      	}
#endif

    REG_IF = IRQ_HBLANK;
}

void vblank(){
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

    updateMyIPC();    
    REG_IF = IRQ_VBLANK;
}

void vcounter(){
    
    //x y
    /*
    //Press X (pause menu)
    if (!(REG_KEYXY & KEY_XARM7)){
        SendArm9Command(0xc1710102, 0x00000000, 0x00000000,0x00000000);
    }
    */
    REG_IF = IRQ_VCOUNT;
}

void timer1(){
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


    REG_IF = IRQ_TIMER1;
}
