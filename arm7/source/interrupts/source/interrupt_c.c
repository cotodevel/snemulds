#include <nds.h>
#include <nds/system.h>
#include <nds/interrupts.h>

#include "common_shared.h"
#include "interrupts.h"
#include "touch.h"
#include "wifi_arm7.h"

#include <nds.h>
#include <nds/system.h>
#include <nds/interrupts.h>

#include "pocketspc.h"
#include "apu.h"
#include "dsp.h"
#include "main.h"
#include "mixrate.h"


#ifdef ARM7
#include <nds/arm7/i2c.h>
#endif

void hblank(){
	// Block execution until the hblank processing on ARM9
	if (!SPC_disable)
	{
		int VCount = REG_VCOUNT;        
		scanlineCount++;
		uint32 T0 = APU_MEM[APU_TIMER0]?APU_MEM[APU_TIMER0]:0x100;
		uint32 T1 = APU_MEM[APU_TIMER1]?APU_MEM[APU_TIMER1]:0x100;
		uint32 T2 = APU_MEM[APU_TIMER2]?APU_MEM[APU_TIMER2]:0x100;
	
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
}

void vblank(){
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
	Wifi_Update();
}

void vcounter(){
	
}

void timer1(){
	#if PROFILING_ON
		long long begin = TIMER2_DATA + ((long long)TIMER3_DATA << 19);
	#endif
    soundCursor = MIXBUFSIZE - soundCursor;

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

	#if PROFILING_ON
		long long end = TIMER2_DATA + ((long long)TIMER3_DATA << 19);
		SPC_IPC->cpuTime += end - begin;
	//    SPC_IPC->dspTime += (TIMER2_DATA + ((long long)TIMER3_DATA << 19)) - end;
	#endif

	REG_IF = IRQ_TIMER1;
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
	if (0==1) {
		irqSetAUX(IRQ_I2C, i2cIRQHandler);
		irqEnableAUX(IRQ_I2C);
	}
#endif
	REG_IME = 1;			// enable global interrupt
}
