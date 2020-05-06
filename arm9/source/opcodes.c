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

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "InterruptsARMCores_h.h"
#include "interrupts.h"
#include "ipcfifoTGDSUser.h"
#include "timerTGDS.h"
#include "common.h"
#include "opcodes.h"
#include "snes.h"
#include "core.h"
#include "cfg.h"
#include "apu.h"

#ifdef WIN32
#define OPCODE _inline
#else
#define OPCODE static inline
#endif

/*
uint8 IORead8(uint32 addr)
{
	uint32 address = addr & 0xFFFFFF;
	addr &= 0xFF000000;
	return IO_getbyte((int)addr, address);
}

void IOWrite8(uint32 addr, uint8 byte)
{
	uint32 address = addr & 0xFFFFFF;
	addr &= 0xFF000000;
	IO_setbyte((int)addr, address, byte);
}

uint16 IORead16(uint32 addr)
{
	uint32 address = addr & 0xFFFFFF;
	addr &= 0xFF000000;
	return IO_getword((int)addr, address);
}

void IOWrite16(uint32 addr, uint16 word)
{
	uint32 address = addr & 0xFFFFFF;
	addr &= 0xFF000000;
	return IO_setword((int)addr, address, word);
}
*/

#include "opc_asm.h"

__attribute__((section(".itcm")))
void	pushb(uint8 b)
{
	SNESC.RAM[CPU.S] = b;
	CPU.S--;
}

__attribute__((section(".itcm")))
void pushw(uint16 w)
{
	CPU.S--;
	SET_WORD16(SNESC.RAM+CPU.S, w);
	CPU.S--;
}

__attribute__((section(".itcm")))
uint8	pullb()
{
	CPU.S++;
	return SNESC.RAM[CPU.S];
}

__attribute__((section(".itcm")))
uint16	pullw()
{
	uint16 w;
	
	CPU.S++;	
	w = GET_WORD16(SNESC.RAM+CPU.S);
	CPU.S++;
	return w;
}


void CPU_goto(int cycles)
{	
	if (CFG.CPU_speedhack & 1)
		cycles -= cycles / 4; // Speed hack: 25 % speed up
	CPU_LoopSpeedHacks = (CFG.CPU_speedhack >= 2);
	CPU.Cycles = cycles;
		
	CPU_unpack();
	
//	*APU_ADDR_BLK = 0;
	CPU_goto2(cycles);
//	*APU_ADDR_BLK = 1;
	CPU.packed = 0;

//	CPU_pack();
}
