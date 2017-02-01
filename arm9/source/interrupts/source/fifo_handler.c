#include <nds.h>
#include <nds/ndstypes.h>

#include "fifo_handler.h"
#include "memmap.h"
#include "common.h"
#include "snes.h"
#include "cfg.h"
#include "main.h"
#include "ipc_libnds_extended.h"

//Coto:
IN_ITCM
inline void HandleFifo() {

    volatile u32 command1=0,command2=0,command3=0,command4=0;
    while(!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY)){
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
            break; //dont care after this
        }
    }
    
    
    switch (command1) {
        
        //MISC Inter-Processor Opcodes
        //perform u32 read (from other core)
        case(0xc2720000):{
            //require read
            if(MyIPC->status & ARM9_BUSYFLAGRD){
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
                MyIPC->status &= ~(ARM9_BUSYFLAGRD);
            }
        }
        break;
        
        //Direct Write
        //perform u32 write(u32 address,u32 value,u8 write_mode) (from other core)
        //command2 = address / command3 = value / command4 = write_mode
        case(0xc2720001):{
            //require write
            if(MyIPC->status & ARM9_BUSYFLAGWR){
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
                MyIPC->status &= ~(ARM9_BUSYFLAGWR);
            }
        }
        break;
        
    }
    
    REG_IF = IRQ_FIFO_NOT_EMPTY;
    
}