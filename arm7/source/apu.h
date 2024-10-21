#ifndef __apu7_h__
#define __apu7_h__

#include "typedefsTGDS.h"

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

// Cycles per second
//#define spcCyclesPerSec 2048000
#define spcCyclesPerSec 4096000
#define spcUpdatesPerSec 2048
#define spcCyclesPerUpdate (spcCyclesPerSec / spcUpdatesPerSec)

// 64Khz timer clock divisor
#define t64Shift 4
// 8Khz timer clock divisor
#define t8Shift 7

struct Timer {
    uint8 enabled;
    uint32 target;
    uint32 cycles;
    uint32 count;
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int APUSlowdownARM7;
extern void ApuExecute(uint32 cycles);
extern uint8 *APU_MEM;
extern uint8 APU_EXTRA_MEM[64];
extern uint32 APU_SP;
extern uint32 CpuJumpTable[];

extern uint32 MemWriteCounter;
// 0 - A, 1 - X, 2 - Y, 3 - RAMBASE, 4 - DP, 5 - PC (Adjusted into rambase)
// 6 - Cycles (bit 0 - C, bit 1 - v, bit 2 - h, bits 3+ cycles left)
// 7 - Optable
// 8 - NZ
extern uint32 APU_STATE[16];
extern uint8 MakeRawPSWFromState(uint32 state[16]);
extern void SetStateFromRawPSW(uint32 state[16], uint8 psw);
extern void ApuReset(u32 inApuCacheSamples, bool inApuCacheSamplesTWLMode, u32 * inSavedROMForAPUCache);
extern void ApuPrepareStateAfterReload();
extern void ApuUpdateTimers(uint32 cycles);
extern void ApuWriteControlByte(uint8 byte);
extern uint32 ApuReadCounter(uint32 address);
extern void ApuWriteUpperByte(uint8 byte, uint32 address);
extern struct Timer timers[3];

extern void ApuResetTimers();

//apumisc.c
extern uint8 iplRom[64];
extern struct Timer timers[3];
extern uint32 apuTimerSkipCycles;
extern uint8 apuShowRom;

#ifdef __cplusplus
}
#endif
