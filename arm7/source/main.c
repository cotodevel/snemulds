#include <string.h>
#include "spcdefs.h"
#include "apu.h"
#include "dsp.h"
#include "main.h"
#include "InterruptsARMCores_h.h"
#include "interrupts.h"
#include "usrsettingsTGDS.h"
#include "timerTGDS.h"
#include "powerTGDS.h"
#include "dldi.h"
#include "ipcfifoTGDSUser.h"
#include "spifwTGDS.h"

//TGDS-MB v3 bootloader
void bootfile(){
}

int sampleRateDivider = 0;

volatile int soundCursor;
bool paused = false;
bool SPC_disable = true;
int PocketSPCVersion = 0; //9 = pocketspcv0.9 / 10 = pocketspcv1.0

void SetupSoundSPC() {
    soundCursor = 0;
	SoundPowerON(127);		//volume

    TIMERXDATA(1) = TIMER_FREQ(MIXRATE);
    TIMERXCNT(1) = TIMER_DIV_1 | TIMER_ENABLE;

    TIMERXDATA(2) = 0x10000 - (MIXBUFSIZE - 1);
    TIMERXCNT(2) = TIMER_CASCADE | TIMER_IRQ_REQ | TIMER_ENABLE;
	
    // Debug
	#if PROFILING_ON  
		TIMERXDATA(3) = 0;
		TIMERXCNT(3) = TIMER_DIV_64 | TIMER_ENABLE;

		TIMERXDATA(4) = 0;
		TIMERXCNT(4) = TIMER_CASCADE | TIMER_ENABLE;
	#endif    
	
	irqEnable(IRQ_TIMER2);
	
	u16 timerval = SOUND_FREQ(MIXRATE);
	// make sure we don't write sound data at the position hardware is reading from
	swiDelay(((0x10000 - timerval) * MIXBUFSIZE) >> 2);
	
}
 
void StopSoundSPC() {
    powerOFF(POWER_SOUND);
    REG_SOUNDCNT = 0;
    TIMERXCNT(1) = 0;
    TIMERXCNT(2) = 0;
	
	irqDisable(IRQ_TIMER2);
}

//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int main(int _argc, char **_argv) {
//---------------------------------------------------------------------------------
	/*			TGDS 1.6 Standard ARM7 Init code start	*/
	while(!(*(u8*)0x04000240 & 2) ){} //wait for VRAM_D block
	ARM7InitDLDI(TGDS_ARM7_MALLOCSTART, TGDS_ARM7_MALLOCSIZE, TGDSDLDI_ARM7_ADDRESS);
	/*			TGDS 1.6 Standard ARM7 Init code end	*/
	
	//Set up PPU IRQ: HBLANK/VBLANK/VCOUNT
	REG_DISPSTAT = (DISP_HBLANK_IRQ | DISP_VBLANK_IRQ | DISP_YTRIGGER_IRQ);
	REG_IE = IRQ_TIMER1 | IRQ_HBLANK | IRQ_VBLANK | IRQ_VCOUNT | IRQ_IPCSYNC | IRQ_RECVFIFO_NOT_EMPTY | IRQ_SCREENLID;
	
	//Set up PPU IRQ Vertical Line
	setVCountIRQLine(TGDS_VCOUNT_LINE_INTERRUPT);
	
	REG_IPC_FIFO_CR = (REG_IPC_FIFO_CR | IPC_FIFO_SEND_CLEAR);	//bit14 FIFO ERROR ACK + Flush Send FIFO
	
	update_spc_ports();
	int i = 0; 
    for (i = 0; i < MIXBUFSIZE; i++) {
        playBuffer[i] = 0;
    }
	
	SendFIFOWords(0xFF, 0xFF);
	enableARM7TouchScreen();
	
    struct sIPCSharedTGDSSpecific* TGDSUSERIPC = SNEMULDS_IPC;
    while (1) {
		if(SPC_disable == false){
            ApuExecute(cyclesToExecute);
        }
		else{
			TGDSUSERIPC->APU_ADDR_ANS = (uint32)0xFF00FF00;
		}
        HaltUntilIRQ(); //Save power until next irq
	}
   
	return 0;
}

//////////////////////////////////////////////////////////////////////

//Custom Button Mapping Handler implementation: IRQ Driven
void CustomInputMappingHandler(uint32 readKeys){
	
}

//Project specific: ARM7 Setup for TGDS sound stream
void initSoundStreamUser(u32 srcFmt){
	
}