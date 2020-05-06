/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006-2007 archeide, All rights reserved. */
/***********************************************************/
/*
This program is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License as 
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
GNU General Public License for more details.
*/

#ifndef __engine_snemul__
#define __engine_snemul__

#include "dsregs.h"
#include "typedefsTGDS.h"
#include "opcodes.h"
#include "common.h"
#include "fs.h"
#include "gfx.h"
#include "apu.h"
#include "cfg.h"
#include "core.h"
#include "ppu.h"
#include "conf.h"
#include "memmap.h"
#include "guiTGDS.h"
#include "ipcfifoTGDSUser.h"
#include "nds_cp15_misc.h"
#include "fatfslayerTGDS.h"
#include "about.h"
#include "utilsTGDS.h"
#include "clockTGDS.h"
#include "dswnifi_lib.h"


#ifdef __cplusplus
extern "C" {
#endif

extern int ThisSNESFrameCount;
extern int guestSNESFrameCount;

extern void writeSRAM(int offset, uint8* src, int size);
extern void readSRAM(int offset, uint8* dest, int size);
extern int loadSRAM();
extern int saveSRAM();

extern uint8 interrupted;

extern int initSNESEmpty();
extern void show_opcode(sint8 *buf, uint8 opcode, int pc, int pb, unsigned short flags);
extern int trace_CPU();
extern void trace_CPUFast();
extern int changeROM(sint8 *ROM, int size);

#ifdef __cplusplus
}
#endif

#endif
