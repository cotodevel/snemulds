#include <nds.h>
#include "common.h"
#include "settings.h"


#ifdef ARM9
__attribute__((section(".itcm")))
inline void SendArm7Command(u32 command1, u32 command2, u32 command3, u32 command4) {
    
    while (!(REG_IPC_FIFO_CR & IPC_FIFO_SEND_EMPTY)){
    }
    
    //if (REG_IPC_FIFO_CR & IPC_FIFO_ERROR) {
    //    REG_IPC_FIFO_CR |= IPC_FIFO_SEND_CLEAR;
    //}
    
    REG_IPC_FIFO_TX = (u32)command1;
    REG_IPC_FIFO_TX = (u32)command2;
    REG_IPC_FIFO_TX = (u32)command3;
    REG_IPC_FIFO_TX = (u32)command4;
    
    //REG_IPC_FIFO_CR |= (1<<14); //1
}
#endif

#ifdef ARM7

//small hack to update IPC APU ports with APU assembly core (on ARM7)
void update_spc_ports(){
    ADDR_PORT_SNES_TO_SPC       =   (u32)(u8*)PORT_SNES_TO_SPC;
    ADDR_PORT_SPC_TO_SNES   =   (u32)(u8*)PORT_SPC_TO_SNES;
}
inline void SendArm9Command(u32 command1, u32 command2, u32 command3, u32 command4) {
    
    while (!(REG_IPC_FIFO_CR & IPC_FIFO_SEND_EMPTY)){
    }
    
    //if (REG_IPC_FIFO_CR & IPC_FIFO_ERROR) {
    //    REG_IPC_FIFO_CR |= IPC_FIFO_SEND_CLEAR;
    //}
    
    REG_IPC_FIFO_TX = (u32)command1;
    REG_IPC_FIFO_TX = (u32)command2;
    REG_IPC_FIFO_TX = (u32)command3;
    REG_IPC_FIFO_TX = (u32)command4;
    
    //REG_IPC_FIFO_CR |= (1<<14); //1
}
#endif

//NDS hardware IPC
void sendbyte_ipc(uint8 word){
	//checkreg writereg (add,val) static int REG_IPC_add=0x04000180,REG_IE_add=0x04000210,REG_IF_add=0x04000214;
	*((u32*)0x04000180)=((*(u32*)0x04000180)&0xfffff0ff) | (word<<8);
}

u8 recvbyte_ipc(){
	return ((*(u32*)0x04000180)&0xf);
}

//coto: in case you're wondering, these opcodes allow to reach specific region memory that is not available from the other core.

// u32 address, u8 read_mode (0 : u32 / 1 : u16 / 2 : u8)
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
u32 read_ext_cpu(u32 address,u8 read_mode){
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
void write_ext_cpu(u32 address,u32 value,u8 write_mode){

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