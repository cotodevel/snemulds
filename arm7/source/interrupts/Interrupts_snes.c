#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDSUser.h"
#include "InterruptsARMCores_h.h"
#include "spcdefs.h"
#include "apu.h"
#include "dsp.h"
#include "main.h"
#include "apu_shared.h"
#include "biosTGDS.h"
#include "exceptionTGDS.h"

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

static int apuMixPosition = 0;

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void Timer2handlerUser(){
	if(SPC_disable == false){
		soundCursor = MIXBUFSIZE - soundCursor;
		s16 * leftOutputChannel = (s16 *)&(playBuffer[MIXBUFSIZE - soundCursor]);
		s16 * rightOutputChannel = leftOutputChannel + (MIXBUFSIZE << 1);
		
		// Left channel
		int channel = soundCursor == 0 ? 0 : 1;
		SCHANNEL_TIMER(channel) = SOUND_FREQ(MIXRATE);
		SCHANNEL_SOURCE(channel) = (uint32)leftOutputChannel;
		SCHANNEL_LENGTH(channel) = (MIXBUFSIZE) >> 1;
		SCHANNEL_REPEAT_POINT(channel) = 0;
		SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0) | SOUND_16BIT;

		// Right channel
		channel = soundCursor == 0 ? 2 : 3;
		SCHANNEL_TIMER(channel) = SOUND_FREQ(MIXRATE);
		SCHANNEL_SOURCE(channel) = (uint32)rightOutputChannel;
		SCHANNEL_LENGTH(channel) = (MIXBUFSIZE) >> 1;
		SCHANNEL_REPEAT_POINT(channel) = 0;
		SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0x7F) | SOUND_16BIT;

		int samplesToMix = MIXBUFSIZE;
		DspMixSamplesStereo(samplesToMix, &playBuffer[soundCursor]);
		if (apuMixPosition + samplesToMix > MIXBUFSIZE ) {
			int tmp = (apuMixPosition + samplesToMix) - (MIXBUFSIZE );
			if (tmp != samplesToMix) {
				DspMixSamplesStereo(samplesToMix - tmp, &playBuffer[apuMixPosition]);
			}
			samplesToMix = MIXBUFSIZE;
			apuMixPosition = 0;
		}
		apuMixPosition += samplesToMix;	
		ApuUpdateTimers(sampleRateDivider);	//Coto: New timer code will synchronize to NDS timer ticks. Will require several patches per game to adjust the correct samplerate for each one.
	}	
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
	
}

static int touchscreenTimeoutCounter = 0;
static bool TSCKeyActive = false;

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void VblankUser(){
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
	
	//Count 10 seconds
	if(SPC_disable == false){

		scanKeys();
		u32 keyHld = keysDown();
		if(keyHld & KEY_TOUCH){
			enableARM7TouchScreen();
			touchscreenTimeoutCounter = 0;
			TSCKeyActive = true;
		}

		//60 fps in 10 seconds: 600
		if(touchscreenTimeoutCounter < 600){
			touchscreenTimeoutCounter++;
		}
		else{

			//disable tsc here
			if(TSCKeyActive == true){
				disableARM7TouchScreen();
				touchscreenTimeoutCounter = 0;
				TSCKeyActive = false;
			}
		}

	}
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void VcounterUser(){
	if(TSCKeyActive == true){
		//TGDS Touchscreen handling X/Y/Touchscreen
	}
	else{
		//If touchscreen disabled, still get X/Y/Touchscreen keys (but no TSC coords)
		u16 keys= REG_KEYXY;
		struct sIPCSharedTGDS * sIPCSharedTGDSInst = (struct sIPCSharedTGDS *)TGDSIPCStartAddress;
		struct touchPosition * sTouchPosition = (struct touchPosition *)&sIPCSharedTGDSInst->tscIPC;
		
		//ARM7 Keypad has access to X/Y/Hinge/Pen down bits
		sIPCSharedTGDSInst->KEYINPUT7 = (uint16)REG_KEYINPUT;
		sIPCSharedTGDSInst->buttons7	= keys;
	}
}

//Note: this event is hardware triggered from ARM7, on ARM9 a signal is raised through the FIFO hardware
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void screenLidHasOpenedhandlerUser(){
	
}

//Note: this event is hardware triggered from ARM7, on ARM9 a signal is raised through the FIFO hardware
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void screenLidHasClosedhandlerUser(){
	
}