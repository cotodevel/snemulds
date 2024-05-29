////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////

//#define APU_MEM_IN_VRAM
#define APU_MEM_IN_RAM

#define APU_CONTROL_REG		0xF1

#define APU_TIMER0			0xFA
#define APU_TIMER1			0xFB
#define APU_TIMER2			0xFC

#define APU_COUNTER0		0xFD
#define APU_COUNTER1		0xFE
#define APU_COUNTER2		0xFF

#define PORT_SNES_TO_SPC ((vu8*)(0x027FFFF8))
#define PORT_SPC_TO_SNES ((vu8*)(0x027FFFFC))

// IPC is at 0x27FF000 - 0x10000 = 0x2EF000 - 0x100 = 0x2EEF00

#define APU_SNES_ADDRESS ((uint8*)(0x27EE000))
#ifndef APU_MEM_IN_VRAM 
#define APU_RAM_ADDRESS ((uint8*)(0x27EE000))
#else
#define APU_RAM_ADDRESS ((uint8*)(0x6010000))
#endif

#define SNEMUL_CMD ((vu32*)(0x02800000-16))
#define SNEMUL_ANS ((vu32*)(0x02800000-20))
#define SNEMUL_BLK ((vu32*)(0x02800000-24))
#define APU_ADDR_CNT ((vu32*)(0x02800000-60))

// Cycles per second
//#define spcCyclesPerSec 2048000
#define spcCyclesPerSec 1024000
#define spcUpdatesPerSec 2048
#define spcCyclesPerUpdate (spcCyclesPerSec / spcUpdatesPerSec)

// 64Khz timer clock divisor
#define t64Shift 4
// 8Khz timer clock divisor
#define t8Shift 7

extern "C" {
void ApuExecute(u32 cycles);

extern u8 *APU_MEM;
extern u8 APU_EXTRA_MEM[64];
extern u32 APU_SP;
extern u32 CpuJumpTable[];

extern u32 APU_STATE[16];
}

// 0 - A, 1 - X, 2 - Y, 3 - RAMBASE, 4 - DP, 5 - PC (Adjusted into rambase)
// 6 - Cycles (bit 0 - C, bit 1 - v, bit 2 - h, bits 3+ cycles left)
// 7 - Optable
// 8 - NZ

u8 MakeRawPSWFromState(u32 state[16]);
void SetStateFromRawPSW(u32 state[16], u8 psw);

void ApuReset();
void ApuPrepareStateAfterReload();
void ApuUpdateTimers(u32 cycles);

extern "C" {	
void ApuWriteControlByte(u8 byte);
u32 ApuReadCounter(u32 address);
void ApuWriteUpperByte(u8 byte, u32 address);
}

struct s_apu2
{
/* timers */
  uint32    T0, T1, T2;			// 0x27ED000 
  uint32 	TIM0, TIM1, TIM2;	/* 0x27ED00C */
  uint32	CNT0, CNT1, CNT2;	/* 0x27ED018 */
};

extern struct s_apu2 *APU2;

/*extern u32 APU_PC_save;
extern u32 APU_WaitAddress;*/
