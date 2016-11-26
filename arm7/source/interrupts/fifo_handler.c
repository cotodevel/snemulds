#include <nds.h>
#include <nds/ndstypes.h>
#include <nds/system.h>
#include <nds/interrupts.h>
#include <string.h>

#include "fifo_handler.h"
#include "../pocketspc.h"
#include "../apu.h"
#include "../dsp.h"
#include "../main.h"
#include "../mixrate.h"
#include "ipc_libnds_extended.h"

//Coto:
void HandleFifo() {
    
    volatile u32 command1 = 0, command2 = 0, command3 = 0, command4 = 0;
    
    while(!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY)){
		if (!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY)){
			command1 = REG_IPC_FIFO_RX;
		}
		
		if (!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY)){
			command2 = REG_IPC_FIFO_RX;
		}
		
        if (!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY)){
			command3 = REG_IPC_FIFO_RX;
        }
        
        if (!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY)){
			command4 = REG_IPC_FIFO_RX;
            break; //dont care after this
        }
	}
    
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
			swiWaitForVBlank();
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
			swiWaitForVBlank();
        }
        break;
        case 0x00000003:{ /* PLAY SPC */	
            LoadSpc(APU_SNES_ADDRESS-0x100);
			SetupSound();   	
			MyIPC->APU_ADDR_CNT = 0;             	
			paused = false;
			SPC_freedom = true;
			SPC_disable = false;
			swiWaitForVBlank();
        }
        break;
            
        case 0x00000004:{ /* DISABLE */
            SPC_disable = true;
			swiWaitForVBlank();
        }
        break;        
        
        case 0x00000005:{ /* CLEAR MIXER BUFFER */
            memset(playBuffer, 0, MIXBUFSIZE * 8);
			swiWaitForVBlank();
		}
        break;

        case 0x00000006:{ /* SAVE state */
            SaveSpc(APU_SNES_ADDRESS-0x100);
			swiWaitForVBlank();
		}
        break;  
            
        case 0x00000007:{ /* LOAD state */
            LoadSpc(APU_SNES_ADDRESS-0x100);
			MyIPC->APU_ADDR_CNT = 0; 
			swiWaitForVBlank();
		}
        break;
        
        case 0x00000008:{
            writePowerManagement(PM_SOUND_AMP, (int)command2>>16);  // void * data == command2 
			
		}
        break;
        
        //arm7 wants to WifiSync
        case(0xc1710100):{
            //Wifi_Sync();
        }
        break;
        //arm9 wants to send a WIFI context block address / userdata is always zero here
        case(0xc1710101):{
            //	wifiAddressHandler( void * address, void * userdata )
            //wifiAddressHandler((Wifi_MainStruct *)(u32)command2, 0);
        }
        break;
        
        //MISC Inter-Processor Opcodes
        
        //Direct Read
        //perform u32 read (from other core)
        case(0xc2720000):{
            //require read
            if(MyIPC->status & ARM7_BUSYFLAGRD){
                switch(command3){
                    case(0):{
                        MyIPC->buf_queue[0]= *(u32*)command2; 
                    }
                    break;
                    case(1):{
                        MyIPC->buf_queue[0]= *(u16*)command2; 
                    }
                    break;
                    case(2):{
                        MyIPC->buf_queue[0]= *(u8*)command2; 
                    }
                    break;
                }
                MyIPC->status &= ~(ARM7_BUSYFLAGRD);
            }
        }
        break;
        
        //Direct Write
        //perform u32 write(u32 address,u32 value,u8 write_mode) (from other core)
        //command2 = address / command3 = value / command4 = write_mode
        case(0xc2720001):{
            //require write
            if(MyIPC->status & ARM7_BUSYFLAGWR){
                switch(command3){
                    case(0):{
                        *(u32*)command2 = (u32)(command4); 
                    }
                    break;
                    case(1):{
                        *(u16*)command2 = (u16)(command4); 
                    }
                    break;
                    case(2):{
                        *(u8*)command2 = (u8)(command4); 
                    }
                    break;
                }
                MyIPC->status &= ~(ARM7_BUSYFLAGWR);
            }
        }
        break;
        
    }
    
    REG_IF = IRQ_FIFO_NOT_EMPTY;
    
}