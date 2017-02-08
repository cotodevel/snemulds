//Coto: these are my FIFO handling libs. Works fine with NIFI (trust me this is very tricky to do without falling into freezes).
//Use it at your will, just make sure you read WELL the descriptions below.

#ifndef nds_common_shared_ipc
#define nds_common_shared_ipc

#ifdef ARM7

#include <nds.h>
#include <nds/ndstypes.h>
#include <nds/system.h>
#include <nds/interrupts.h>
#include <string.h>

#include "pocketspc.h"
#include "apu.h"
#include "dsp.h"
#include "main.h"
#include "mixrate.h"
#include "common_shared.h"
#include "wifi_arm7.h"


#endif

#ifdef ARM9

#include <nds.h>
#include <nds/ndstypes.h>

#include "memmap.h"
#include "common.h"
#include "snes.h"
#include "cfg.h"
#include "main.h"
#include "common_shared.h"

#endif

#endif

//Software FIFO calls, Rely on Hardware FIFO calls so it doesnt matter if they are in different maps 
#ifdef ARM9
__attribute__((section(".dtcm")))
#endif    
volatile int FIFO_SOFT_PTR = 0;
#ifdef ARM9
__attribute__((section(".dtcm")))
#endif    
volatile u32 FIFO_BUF_SOFT[FIFO_NDS_HW_SIZE/4];

//GetSoftFIFO: Stores up to FIFO_NDS_HW_SIZE. Exposed to usercode for fetching 64 bytes sent from other core, until it returns false (empty buffer).

//Example: 
//u32 n = 0;
//while(GetSoftFIFO(&n)== true){
//	//n has 4 bytes from the other ARM Core.
//}
#ifdef ARM9
__attribute__((section(".itcm")))
#endif    
inline bool GetSoftFIFO(u32 * var)
{
	if(FIFO_SOFT_PTR > 0){
		FIFO_SOFT_PTR--;
		
		*var = (u32)FIFO_BUF_SOFT[FIFO_SOFT_PTR];
		FIFO_BUF_SOFT[FIFO_SOFT_PTR] = (u32)0;
		
		return true;
	}
	else
		return false;
}

//SetSoftFIFO == false means FULL
#ifdef ARM9
__attribute__((section(".itcm")))
#endif    
//returns ammount of inserted U32 blocks into FIFO hardware regs
inline bool SetSoftFIFO(u32 value)
{
	if(FIFO_SOFT_PTR < (int)(FIFO_NDS_HW_SIZE/4)){
		FIFO_BUF_SOFT[FIFO_SOFT_PTR] = value;
		FIFO_SOFT_PTR++;
		return true;
	}
	else
		return false;
}

//SendArm[7/9]Command: These send a command and up to 15 arguments. 
//The other ARM Core through a FIFO interrupt will execute HandleFifo()
//By default I use 4 (you can fill them with 0s if you want to use fewer)
#ifdef ARM9
__attribute__((section(".itcm")))
inline void SendArm7Command(u32 command1, u32 command2, u32 command3, u32 command4)
#endif
#ifdef ARM7
inline void SendArm9Command(u32 command1, u32 command2, u32 command3, u32 command4)
#endif    
{	
	FIFO_DRAINWRITE();
	
	REG_IPC_FIFO_TX = command1;
	REG_IPC_FIFO_TX = command2;
	REG_IPC_FIFO_TX = command3;
	REG_IPC_FIFO_TX = command4;
	
	//always send full fifo queue
	REG_IPC_FIFO_CR |= IPC_FIFO_ERROR;
	
}

//FIFO HANDLER INIT
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline void HandleFifoEmpty(){
	
	
}

//FIFO HANDLER INIT
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline void HandleFifoNotEmpty(){
	u32 command1 = 0,command2 = 0,command3 = 0,command4 = 0;
	
	if(!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY)){
		command1 = REG_IPC_FIFO_RX;
	}
	
	if(!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY)){
		command2 = REG_IPC_FIFO_RX;
	}
	
	if(!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY)){
		command3 = REG_IPC_FIFO_RX;
	}
	
	if(!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY)){
		command4 = REG_IPC_FIFO_RX;
	}
	
	
	//ARM7 command handler
	#ifdef ARM7
	switch (command1) {
        
		case 0x00000001:{
            // Reset
			StopSound();

			memset(playBuffer, 0, MIXBUFSIZE * 8);

			MyIPC->APU_ADDR_CNT = 0; 
			ApuReset();
			DspReset();

			SetupSound();
			paused = false;
			SPC_disable = false;
			SPC_freedom = false;
		}
        break;
        case 0x00000002:{
            // Pause/unpause
			if (!paused) {
				StopSound();
			} else {
				SetupSound();
			}
			if (SPC_disable)
				SPC_disable = false;        
			paused = !paused;
		}
        break;
        case 0x00000003:{ /* PLAY SPC */	
            LoadSpc(APU_RAM_ADDRESS);
			SetupSound();   	
			MyIPC->APU_ADDR_CNT = 0;             	
			paused = false;
			SPC_freedom = true;
			SPC_disable = false;
		}
        break;
            
        case 0x00000004:{ /* DISABLE */
            SPC_disable = true;
		}
        break;        
        
        case 0x00000005:{ /* CLEAR MIXER BUFFER */
            memset(playBuffer, 0, MIXBUFSIZE * 8);
		}
        break;

        case 0x00000006:{ /* SAVE state */
            SaveSpc(APU_RAM_ADDRESS);
		}
        break;  
            
        case 0x00000007:{ /* LOAD state */
            LoadSpc(APU_RAM_ADDRESS);
			MyIPC->APU_ADDR_CNT = 0; 
		}
        break;
        
        case 0x00000008:{
            writePowerManagement(PM_SOUND_AMP, (int)command2>>16);  // void * data == command2 
			
		}
        break;
        
		
		//keepalive fifo irq
		case(FIFO_SEND_EMPTY):{
			
		}
		break;
		
		//must be called from within timer irqs
		//update apu from nds irq
		case(FIFO_SEND_EXT):{
			
			//stack to fifo here
			SetSoftFIFO(command2);
			
		}
		break;
		
		
		//project independent
		
		//arm9 wants to send a WIFI context block address / userdata is always zero here
        case(0xc1710101):{
            //	wifiAddressHandler( void * address, void * userdata )
            wifiAddressHandler((Wifi_MainStruct *)(u32)command2, 0);
        }
        break;
		
	}
	#endif
	
	//ARM9 command handler
	#ifdef ARM9
	switch (command1) {   
		case(WIFI_SYNC):{
			Wifi_Sync();
		}
		break;
		
		//keepalive fifo irq
		case(FIFO_SEND_EMPTY):{
			
		}
		break;
		
		//must be called from within timer irqs
		//update apu from nds irq
		case(FIFO_SEND_EXT):{
			
			//stack to fifo here
			SetSoftFIFO(command2);
			
		}
		break;
		
		
		//project independent
		
    }
	#endif
	
	//Shared: Acknowledge
	REG_IPC_FIFO_CR |= IPC_FIFO_ERROR;
	
	
}



// Ensures a SendArm[7/9]Command (FIFO message) command to be forcefully executed at target ARM Core, while the host ARM Core awaits. 
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void FIFO_DRAINWRITE(){
	while (!(REG_IPC_FIFO_CR & IPC_FIFO_SEND_EMPTY)){}	
}

//FIFO HANDLER END

//project specific stuff
#ifdef ARM9

//small hack to update SNES_ADDRESS at opcodes2.s
void update_ram_snes(){
    snes_ram_address = (u32)&snes_ram_bsram[0x6000];
}
#endif

#ifdef ARM7
//small hack to update IPC APU ports with APU assembly core (on ARM7)
void update_spc_ports(){
    ADDR_PORT_SNES_TO_SPC       =   (u32)(u8*)PORT_SNES_TO_SPC;
    ADDR_PORT_SPC_TO_SNES   =   (u32)(u8*)PORT_SPC_TO_SNES;
}
#endif


/*
//coto: in case you're wondering, these opcodes allow to reach specific region memory that is not available from the other core.

// u32 address, u8 read_mode (0 : u32 / 1 : u16 / 2 : u8)
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline u32 read_ext_cpu(u32 address,u8 read_mode){
    #ifdef ARM7
        MyIPC->status |= ARM9_BUSYFLAGRD;
        SendArm9Command(0xc2720000, address, read_mode,0x00000000);
        while(MyIPC->status & ARM9_BUSYFLAGRD){}
    #endif
        
    #ifdef ARM9
        MyIPC->status |= ARM7_BUSYFLAGRD;
        SendArm7Command(0xc2720000, address, read_mode,0x00000000);
        while(MyIPC->status & ARM7_BUSYFLAGRD){}
    #endif
    
    return (u32)MyIPC->buf_queue[0];
}

//Direct writes: Write ARMx<->ARMx opcodes:
// u32 address, u8 write_mode (0 : u32 / 1 : u16 / 2 : u8)
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline void write_ext_cpu(u32 address,u32 value,u8 write_mode){

    #ifdef ARM7
        MyIPC->status |= ARM9_BUSYFLAGWR;
        SendArm9Command(0xc2720001, address, write_mode, value);
        while(MyIPC->status& ARM9_BUSYFLAGWR){}
    #endif
        
    #ifdef ARM9
        MyIPC->status |= ARM7_BUSYFLAGWR;
        SendArm7Command(0xc2720001, address, write_mode, value);
        while(MyIPC->status& ARM7_BUSYFLAGWR){}
    #endif
    
}

//NDS hardware IPC
void sendbyte_ipc(uint8 word){
	//checkreg writereg (add,val) static int REG_IPC_add=0x04000180,REG_IE_add=0x04000210,REG_IF_add=0x04000214;
	*((u32*)0x04000180)=((*(u32*)0x04000180)&0xfffff0ff) | (word<<8);
}

u8 recvbyte_ipc(){
	return ((*(u32*)0x04000180)&0xf);
}

*/



//ipc clock opcodes
u8 gba_get_yearbytertc(){
	return (u8)(u32)MyIPC->clockdata[0];
}

u8 gba_get_monthrtc(){
	return (u8)(u32)MyIPC->clockdata[1];
}

u8 gba_get_dayrtc(){
	return (u8)(u32)MyIPC->clockdata[2];
}

u8 gba_get_dayofweekrtc(){
	return (u8)(u32)MyIPC->clockdata[3];
}


u8 gba_get_hourrtc(){
	return (u8)(u32)MyIPC->clockdata[4];
}

u8 gba_get_minrtc(){
	return (u8)(u32)MyIPC->clockdata[5];
}

u8 gba_get_secrtc(){
	return (u8)(u32)MyIPC->clockdata[6];
}
