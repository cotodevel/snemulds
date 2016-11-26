#ifndef snemuldsv6_apuarm7
#define snemuldsv6_apuarm7

////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////

#define APU_MEM_IN_VRAM
#define APU_CONTROL_REG		0xF1

#define APU_TIMER0			0xFA
#define APU_TIMER1			0xFB
#define APU_TIMER2			0xFC

#define APU_COUNTER0		0xFD
#define APU_COUNTER1		0xFE
#define APU_COUNTER2		0xFF

// IPC is at 0x27FF000
//#define PORT_SNES_TO_SPC ((volatile uint8*)(0x3000000-8))
//#define PORT_SPC_TO_SNES ((volatile uint8*)(0x3000000-4))
//#define APU_SNES_ADDRESS ((uint8*)(0x3000000-0x12000))
//#define APU_ADDR_CNT ((vu32*)(0x3000000-60))

//@ IPC now
#define APU_RAM_ADDRESS     ((uint8*)(0x6010000))
#define APU_SNES_ADDRESS    APU_RAM_ADDRESS //ori: #define APU_SNES_ADDRESS ((uint8*)(0x3000000-0x12000))       //could cause lockups

//deprecated/unused: #define SNEMUL_CMD ((vu32*)(0x3000000-16))
//deprecated/unused: #define SNEMUL_ANS ((vu32*)(0x3000000-20))
//deprecated/unused: #define SNEMUL_BLK ((vu32*)(0x3000000-24))

// Cycles per second
//#define spcCyclesPerSec 2048000
#define spcCyclesPerSec 1024000
#define spcUpdatesPerSec 2048
#define spcCyclesPerUpdate (spcCyclesPerSec / spcUpdatesPerSec)

// 64Khz timer clock divisor
#define t64Shift 4
// 8Khz timer clock divisor
#define t8Shift 7


/*
struct s_apu2
{
// timers
  uint32 	TIM0, TIM1, TIM2;       // 0x27ED000
  uint32    T0, T1, T2;
//  uint32    T0, T1, T2;           // 0x27ED000
//  uint32 	TIM0, TIM1, TIM2;       // 0x27ED00C 
//  uint32	CNT0, CNT1, CNT2;       // 0x27ED018 
};

extern struct s_apu2 *APU2;
*/
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void ApuExecute(u32 cycles);
extern u8 *APU_MEM;
extern u8 APU_EXTRA_MEM[64];
extern u32 APU_SP;
extern u32 CpuJumpTable[];
extern u32 APU_STATE[16];

// Memory post read/write functions
extern u32 MemWriteDoNothing;
extern u32 MemWriteApuControl;
extern u32 MemWriteDspData;
extern u32 MemWriteUpperByte;
extern u32 MemWriteApuPort;
extern u32 MemWriteCounter;
extern u32 MemReadDoNothing;
extern u32 MemReadCounter;
extern u32 MemReadCounterFD;
extern u32 MemReadCounterFE;
extern u32 MemReadCounterFF;
extern u32 MemReadApuPort;
extern u32 MemReadDspData;
extern u32 MemZeroPageReadTable;
extern u32 MemZeroPageWriteTable;

// 0 - A, 1 - X, 2 - Y, 3 - RAMBASE, 4 - DP, 5 - PC (Adjusted into rambase)
// 6 - Cycles (bit 0 - C, bit 1 - v, bit 2 - h, bits 3+ cycles left)
// 7 - Optable
// 8 - NZ
extern u8 MakeRawPSWFromState(u32 state[16]);
extern void SetStateFromRawPSW(u32 state[16], u8 psw);
extern void ApuReset();
extern void ApuPrepareStateAfterReload();
extern void ApuUpdateTimers(u32 cycles);
extern void ApuWriteControlByte(u8 byte);
extern u32 ApuReadCounter(u32 address);
extern void ApuWriteUpperByte(u8 byte, u32 address);

#ifdef __cplusplus
}
#endif
