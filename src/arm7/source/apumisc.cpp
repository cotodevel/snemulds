#include "pocketspc.h"
#include "apu.h"

extern u8 iplRom[64];

Timer timers[3];

static u8 apuShowRom;

void ApuResetTimer(int timer) {
    timers[timer].cycles = 0;
    timers[timer].count = 0;
    timers[timer].target = APU_MEM[APU_TIMER0 + timer];
    if (timers[timer].target == 0) timers[timer].target = 0x100;
}

void ApuWriteControlByte(u8 byte) {
    u8 orig = APU_MEM[APU_CONTROL_REG];
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

	if (byte & 0x10) {
		// Clear port 0 and 1
		APU_MEM[0xF4] = 0;
		APU_MEM[0xF5] = 0;
        PORT_SNES_TO_SPC[0] = 0;
        PORT_SNES_TO_SPC[1] = 0;
//        PORT_SPC_TO_SNES[0] = 0;
//        PORT_SPC_TO_SNES[1] = 0;
	}
	if (byte & 0x20) {
		// Clear port 0 and 1
		APU_MEM[0xF6] = 0;
		APU_MEM[0xF7] = 0;
        PORT_SNES_TO_SPC[2] = 0;
        PORT_SNES_TO_SPC[3] = 0;
//        PORT_SPC_TO_SNES[2] = 0;
//        PORT_SPC_TO_SNES[3] = 0;
	}

	if (byte & 0x80) {
		if (!apuShowRom) {
			apuShowRom = 1;
			for (int i=0; i<=0x3F; i++) APU_MEM[0xFFC0 + i] = iplRom[i];
		}
	} else {
		if (apuShowRom) {
			apuShowRom = 0;
			for (int i=0; i<=0x3F; i++) APU_MEM[0xFFC0 + i] = APU_EXTRA_MEM[i];
		}
	}
}

void ApuPrepareStateAfterReload() {
    APU_MEM[APU_COUNTER0] &= 0xf;
    APU_MEM[APU_COUNTER1] &= 0xf;
    APU_MEM[APU_COUNTER2] &= 0xf;

    for (int i = 0; i < 4; i++) PORT_SNES_TO_SPC[i] = APU_MEM[0xF4 + i];

    for (int i = 0; i < 3; i++) {
        ApuResetTimer(i);
    }

	apuShowRom = APU_MEM[APU_CONTROL_REG] >> 7;
    if (apuShowRom) {
		for (int i=0; i<=0x3F; i++) APU_MEM[0xFFC0 + i] = iplRom[i];
	} else {
		for (int i=0; i<=0x3F; i++) APU_MEM[0xFFC0 + i] = APU_EXTRA_MEM[i];
	}
}

extern "C" {
u32 ApuReadCounterHack() {
    u8 control = APU_MEM[APU_CONTROL_REG];
    u32 val = 0xffffffff;
    if (control & 0x1) {
        u32 tmp = (timers[0].target - timers[0].count) * (spcCyclesPerSec / 8000);
        if (tmp < val) val = tmp;
    }
    if (control & 0x2) {
        u32 tmp = (timers[1].target - timers[1].count) * (spcCyclesPerSec / 8000);
        if (tmp < val) val = tmp;
    }
    if (control & 0x4) {
        u32 tmp = (timers[2].target - timers[2].count) * (spcCyclesPerSec / 64000);
        if (tmp < val) val = tmp;
    }
    return val;
}

void ApuWriteUpperByte(u8 byte, u32 address) {
    APU_EXTRA_MEM[address - 0xFFC0] = byte;

    if (apuShowRom)
        APU_MEM[address] = iplRom[address - 0xFFC0];
}
}
