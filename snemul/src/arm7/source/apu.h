#ifndef __apu7_h__
#define __apu7_h__

////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////

#define APU_CONTROL_REG		0xF1

#define APU_TIMER0			0xFA
#define APU_TIMER1			0xFB
#define APU_TIMER2			0xFC

#define APU_COUNTER0		0xFD
#define APU_COUNTER1		0xFE
#define APU_COUNTER2		0xFF

#define PORT_SNES_TO_SPC ((vu8*)(0x027ECFF8))
#define PORT_SPC_TO_SNES ((vu8*)(0x027ECFFC))
/*
#define PORT_SNES_TO_SPC ((vu8*)(0x027FFFF8))
#define PORT_SPC_TO_SNES ((vu8*)(0x027FFFFC))
*/
// IPC is at 0x27FF000 - 0x10000 = 0x2EF000 - 0x100 = 0x2EEF00 
#define APU_RAM_ADDRESS ((uint8*)(0x27EE000))

#define SNEMUL_CMD ((vu32*)(0x02800000-16))
#define SNEMUL_ANS ((vu32*)(0x02800000-20))
#define SNEMUL_DBG1 ((vu32*)(0x02800000-24))
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

struct Timer {
    u32 target;
    u32 cycles;
    u32 count;
    u32 padding;
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern Timer timers[3];
extern void ApuExecute(u32 cycles);
extern u8 *APU_MEM;
extern u8 APU_EXTRA_MEM[64];
extern u32 APU_SP;
extern u32 CpuJumpTable[];

// 0 - A, 1 - X, 2 - Y, 3 - RAMBASE, 4 - DP, 5 - PC (Adjusted into rambase)
// 6 - Cycles (bit 0 - C, bit 1 - v, bit 2 - h, bits 3+ cycles left)
// 7 - Optable
// 8 - NZ
extern u32 APU_STATE[16];

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
