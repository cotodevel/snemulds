#ifndef nds_common_ipc
#define nds_common_ipc

#include <nds.h>
#include "wifi_shared.h"

//---------------------------------------------------------------------------------
typedef struct sMyIPC {
//---------------------------------------------------------------------------------
    int16 touchX,   touchY;   // raw x/y
    int16 touchXpx, touchYpx; // TFT x/y pixel

    int16 touchZ1,  touchZ2;  // TSC x-panel measurements
    uint16 tdiode1,  tdiode2;  // TSC temperature diodes
    uint32 temperature;        // TSC computed temperature

    uint16 buttons;            // keypad buttons
    uint16 buttons_xy_folding;  // X, Y, /PENIRQ buttons
    
    union {
        uint8 curtime[8];        // current time response from RTC

        struct {
                u8 rtc_command;
                u8 rtc_year;           //add 2000 to get 4 digit year
                u8 rtc_month;          //1 to 12
                u8 rtc_day;            //1 to (days in month)

                u8 rtc_incr;
                u8 rtc_hours;          //0 to 11 for AM, 52 to 63 for PM
                u8 rtc_minutes;        //0 to 59
                u8 rtc_seconds;        //0 to 59
        };
    };
    u8 touched;				    //TSC touched?
    u8 touch_pendown;           //TSC already held before?
    uint16 battery;            
    uint16 aux;                

    vuint8 mailBusy;
    
	//IPC Clock
    //[0]; //yy
    //[1]; //mth
    //[2]; //dd
    //[3]; //day of week
    //[4]; //hh
    //[5]; //mm
    //[6]; //ss
    u8 clockdata[0x20];
	
	//dswnifi specific
	TdsnwifisrvStr dswifiSrv;



	//project specific
	u32 * IPC_ADDR;
    u8 * ROM;   		//pointer to ROM page
    int rom_size;   	//rom total size
	
	//APU Core
    int	    skipper_cnt1;
    int	    skipper_cnt2;
    int	    skipper_cnt3;
    int	    skipper_cnt4;
    int		counter;

    //IPC APU
    u32 APU_ADDR_CNT;
    u32 APU_ADDR_CMD;

    u8 APU_ADDR_BLKP[4];
    u32 APU_ADDR_BLK;     //deprecated in v6 but declared
    
    uint32 	TIM0, TIM1, TIM2;
    uint32    T0, T1, T2;
	
} tMyIPC;

//Shared Work     027FF000h 4KB    -     -    -    R/W

//IPC Struct
#define MyIPC ((tMyIPC volatile *)(0x027FF000))
#define PORT_SNES_TO_SPC ((volatile uint8*)(0x027FF000+(sizeof(tMyIPC))+4+0))
#define PORT_SPC_TO_SNES ((volatile uint8*)(0x027FF000+(sizeof(tMyIPC))+4+4)) 

//irqs
#define VCOUNT_LINE_INTERRUPT 0

//arm7 specific
#define KEY_XARM7 (1<<0)
#define KEY_YARM7 (1<<1)
#define KEY_HINGE (1<<7)

#ifdef ARM7
#define 	BIOS_IRQFLAGS   *(__irq_flags)
#endif

#define     irq_vector_addr (__irq_vector)

//FIFO SPECIAL
#define FIFO_NDS_HW_SIZE (16*4)
#define FIFO_SEND_EXT	0xffff0001	//stream 64 bytes of data to other ARM Core, can be received through GetSoftFIFO 4 bytes a time, until it returns false (empty)
#define FIFO_SEND_EMPTY	0xffff0002	//keeps sending fifo on fifoempty

#endif

//processor ipc read/writes flags
/*
#define ARM7_BUSYFLAGRD (u8)(0x08)
#define ARM7_BUSYFLAGWR (u8)(0x0f)
#define ARM9_BUSYFLAGRD (u8)(0x80)
#define ARM9_BUSYFLAGWR (u8)(0xf0)
*/


#ifdef __cplusplus
extern "C" {
#endif

#ifdef ARM9
extern void SendArm7Command(u32 command1, u32 command2, u32 command3,u32 command4);
#endif

#ifdef ARM7
extern void SendArm9Command(u32 command1, u32 command2, u32 command3,u32 command4);
#endif

/*
extern void sendbyte_ipc(uint8 word);
extern u8 recvbyte_ipc();
extern u32 read_ext_cpu(u32 address,u8 read_mode);
extern void write_ext_cpu(u32 address,u32 value,u8 write_mode);
*/

//clock opcodes
extern u8 gba_get_yearbytertc();
extern u8 gba_get_monthrtc();
extern u8 gba_get_dayrtc();
extern u8 gba_get_dayofweekrtc();
extern u8 gba_get_hourrtc();
extern u8 gba_get_minrtc();
extern u8 gba_get_secrtc();

//FIFO 
extern void FIFO_DRAINWRITE();
extern bool SetSoftFIFO(u32 value);
extern bool GetSoftFIFO(u32 * var);

extern volatile int FIFO_SOFT_PTR;
extern volatile u32 FIFO_BUF_SOFT[FIFO_NDS_HW_SIZE/4];
extern volatile u32 FIFO_IN_BUF[FIFO_NDS_HW_SIZE/4];

extern void HandleFifoNotEmpty();
extern void HandleFifoEmpty();

extern int SendFIFOCommand(u32 * buf,int size);
extern int RecvFIFOCommand(u32 * buf);

//project specific
extern u32 ADDR_PORT_SNES_TO_SPC;
extern u32 ADDR_PORT_SPC_TO_SNES;

#ifdef ARM9
extern void update_ram_snes();
#endif

#ifdef ARM7
extern void update_spc_ports();
#endif


#ifdef __cplusplus
}
#endif
