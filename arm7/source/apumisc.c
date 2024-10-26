#include <string.h>
#include "spcdefs.h"
#include "apu.h"
#include "ipcfifoTGDSUser.h"
#include "biosTGDS.h"
#include "main.h"

struct Timer timers[3];
uint32 apuTimerSkipCycles; //PocketSPCv0.9
uint8 apuShowRom;

void ApuResetTimer(int timer) {
    timers[timer].enabled = (APU_MEM[APU_CONTROL_REG] >> timer) & 1;
    timers[timer].cycles = 0;
    timers[timer].count = 0;
    timers[timer].target = APU_MEM[APU_TIMER0 + timer];
    if (timers[timer].target == 0) timers[timer].target = 0x100;
}

void ApuResetTimers() {
    int i = 0;
    for (i = 0; i < 3; i++) {
        ApuResetTimer(i);
    }
    if(PocketSPCVersion == 9){
    	apuTimerSkipCycles = 0; //PocketSPCv0.9
	}
}

void ApuWriteControlByte(uint8 byte) {
    
    if(APUSlowdownARM7 == 1){
	    swiDelay(18000 * 12);
    }

    uint8 orig = APU_MEM[APU_CONTROL_REG];
    if ((orig & 0x1) == 0 && (byte & 0x1) != 0) {
        ApuResetTimer(0);
		APU_MEM[APU_COUNTER0] = 0;
	}
    if ((orig & 0x2) == 0 && (byte & 0x2) != 0) {
        ApuResetTimer(1);
		APU_MEM[APU_COUNTER1] = 0;
	}
    if ((orig & 0x4) == 0 && (byte & 0x4) != 0) {
        ApuResetTimer(2);
		APU_MEM[APU_COUNTER2] = 0;
	}

    timers[0].enabled = byte & 0x1;
    timers[1].enabled = (byte >> 1) & 0x1;
    timers[2].enabled = (byte >> 2) & 0x1;

	if (byte & 0x10) {
		// Clear port 0 and 1
		APU_MEM[0xF4] = 0;
		APU_MEM[0xF5] = 0;
        ((volatile u8*)ADDRPORT_SNES_TO_SPC)[0] = 0;
        ((volatile u8*)ADDRPORT_SNES_TO_SPC)[1] = 0;
        //((volatile u8*)ADDRPORT_SPC_TO_SNES)[0] = 0; //enabling this will discard pending commands from SPC to SNES and will cause game blackscreens due to APU looping wait for acknowledge
        //((volatile u8*)ADDRPORT_SPC_TO_SNES)[1] = 0; //enabling this will discard pending commands from SPC to SNES and will cause game blackscreens due to APU looping wait for acknowledge
	}
	if (byte & 0x20) {
		// Clear port 0 and 1
		APU_MEM[0xF6] = 0;
		APU_MEM[0xF7] = 0;
        ((volatile u8*)ADDRPORT_SNES_TO_SPC)[2] = 0;
        ((volatile u8*)ADDRPORT_SNES_TO_SPC)[3] = 0;
        //((volatile u8*)ADDRPORT_SPC_TO_SNES)[2] = 0; //enabling this will discard pending commands from SPC to SNES and will cause game blackscreens due to APU looping wait for acknowledge
        //((volatile u8*)ADDRPORT_SPC_TO_SNES)[3] = 0; //enabling this will discard pending commands from SPC to SNES and will cause game blackscreens due to APU looping wait for acknowledge
	}
	
	int i=0;
	
	if (byte & 0x80) {
		if (!apuShowRom) {
			apuShowRom = 1;
			memcpy(APU_MEM+0xFFC0, iplRom, 0x40);
		}
	} else {
		if (apuShowRom) {
			apuShowRom = 0;
			memcpy(APU_MEM+0xFFC0, APU_EXTRA_MEM, 0x40);
		}
	}
}

void ApuPrepareStateAfterReload() {
	int i=0;
    APU_MEM[APU_COUNTER0] &= 0xf;
    APU_MEM[APU_COUNTER1] &= 0xf;
    APU_MEM[APU_COUNTER2] &= 0xf;

    ApuResetTimers();

    for (i = 0; i < 4; i++) ((volatile u8*)ADDRPORT_SNES_TO_SPC)[i] = APU_MEM[0xF4 + i];

    for (i = 0; i < 3; i++) {
        timers[i].cycles = 0;
        timers[i].count = 0;
        timers[i].target = APU_MEM[APU_TIMER0 + i];
        if (timers[i].target == 0) timers[i].target = 0x100;
        timers[i].enabled = APU_MEM[APU_CONTROL_REG] & (1 << i);
    }

	apuShowRom = APU_MEM[APU_CONTROL_REG] >> 7;
    if (apuShowRom) {
		for (i=0; i<=0x3F; i++) APU_MEM[0xFFC0 + i] = iplRom[i];
	} else {
		for (i=0; i<=0x3F; i++) APU_MEM[0xFFC0 + i] = APU_EXTRA_MEM[i];
	}
}

///////////////////////PocketSPCv0.9 start///////////////////////////
void ApuUpdateTimer(int timer, int cyclesRun) {
    int shift;
    if (timer == 2) {
        shift = t64Shift;
    } else {
        shift = t8Shift;
    }
    int mask = (1 << shift) - 1;

    timers[timer].cycles += cyclesRun;
    int updates = timers[timer].cycles >> shift;
    timers[timer].cycles &= mask;

    // Update the count
    timers[timer].count += updates;

    bool overflow = false;
    if (timers[timer].count > timers[timer].target) overflow = true;

    while (timers[timer].count > timers[timer].target) {
        timers[timer].count -= timers[timer].target;
        APU_MEM[APU_COUNTER0 + timer] = (APU_MEM[APU_COUNTER0 + timer] + 1) & 0xf;
    }

    if (overflow) {
        // Read the new value of the timer target in case it has changed
        timers[timer].target = APU_MEM[APU_TIMER0 + timer];
        if (timers[timer].target == 0) timers[timer].target = 0x100;
    }
}

// Call to update the timers
// int cycles - the number of cycles that have passed by since last timer update
void ApuUpdateTimers(uint32 cyclesRun) {
    apuTimerSkipCycles += cyclesRun;

    if (timers[0].enabled)
        ApuUpdateTimer(0, cyclesRun);
    if (timers[1].enabled)
        ApuUpdateTimer(1, cyclesRun);
    if (timers[2].enabled)
        ApuUpdateTimer(2, cyclesRun);
}

uint32 ApuReadCounter(uint32 address) {
	if ((APU_MEM[address] & 0xf) == 0 && (APU_MEM[APU_CONTROL_REG] & 0x7) != 0) {
        // There is a timer enabled and the current read timer was at zero

        // Check if more than 64 cycles has passed since the last read
        if (apuTimerSkipCycles > 64) goto noSkip;

        uint32 val = 0xffffffff;
        if (timers[0].enabled) {
            uint32 tmp = (timers[0].target - timers[0].count) * (spcCyclesPerSec / 8000);
            if (tmp < val) val = tmp;
        }
        if (timers[1].enabled) {
            uint32 tmp = (timers[1].target - timers[1].count) * (spcCyclesPerSec / 8000);
            if (tmp < val) val = tmp;
        }
        if (timers[2].enabled) {
            uint32 tmp = (timers[2].target - timers[2].count) * (spcCyclesPerSec / 64000);
            if (tmp < val) val = tmp;
        }
        return val;
	}

noSkip:
    // Reset the number of cycles since the last read
    apuTimerSkipCycles = 0;

	return 0;
}
///////////////////////PocketSPCv0.9 end///////////////////////////

///////////////////////PocketSPCv1.0 start///////////////////////////
u32 ApuReadCounterHack() {
    u8 control = APU_MEM[APU_CONTROL_REG];
    u32 val = 0xffffffff;
    if ((control & 0x1) && (timers[0].enabled)) {
        u32 tmp = (timers[0].target - timers[0].count) * (spcCyclesPerSec / 8000);
        if (tmp < val) val = tmp;
    }
    if ((control & 0x2) && (timers[1].enabled)) {
        u32 tmp = (timers[1].target - timers[1].count) * (spcCyclesPerSec / 8000);
        if (tmp < val) val = tmp;
    }
    if ((control & 0x4) && (timers[2].enabled)) {
        u32 tmp = (timers[2].target - timers[2].count) * (spcCyclesPerSec / 64000);
        if (tmp < val) val = tmp;
    }
    return val;
}
///////////////////////PocketSPCv1.0 end///////////////////////////

void ApuWriteUpperByte(uint8 byte, uint32 address) {
    APU_EXTRA_MEM[address - 0xFFC0] = byte;

    if (apuShowRom)
        APU_MEM[address] = iplRom[address - 0xFFC0];
}

void ApuSetShowRom()
{
	apuShowRom = 0;
}