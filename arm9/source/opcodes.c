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
#include "snemulds_memmap.h"

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

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void CPU_pack(){
	if (CPU.packed){
		return;
	}
	CPU.PC = (uint32)((sint32)PCptr+(sint32)SnesPCOffset); 
	CPU.PB = S&0xFFFF;
  
	CPU.A = REAL_A;
	CPU.X = X;
	CPU.Y = Y;
	Cycles = -((sint32)SaveR8 >> 14);
  
	CPU.S = S >> 16;
	CPU.P = 0; 
	if (SaveR8 & 0x00000002) CPU.P |= P_C;
	if (SaveR8 & 0x00000001) CPU.P |= P_V;
	if (SaveR8 & 0x00000400) CPU.P |= P_E;    
	if (SaveR6 & 0x00018000) CPU.P |= P_N;
	if (!(SaveR6 << 16)) CPU.P |= P_Z;
	CPU.P |= ((SaveR8 << 22) & 0x3c000000) >> 24;
	CPU.D = D >> 16;
	CPU.DB = D & 0xFF;

	CPU.WAI_state = (SaveR8 & 0x00001000)?1:0;  

	CPU.packed = 1;
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void CPU_unpack(){
	if (CPU.unpacked){
		return;	
	}
	SnesPCOffset = -(sint32)mem_getbaseaddress(CPU.PC, CPU.PB);
	PCptr = map_memory(CPU.PC, CPU.PB);

	S = CPU.PB;
	S |= CPU.S << 16;

	// FIXME: "B" register
	if (CPU.P & P_M){
		A = CPU.A << 24;
		SnesB = (CPU.A & 0xFF00) << 16;
	}
	else{
		A = CPU.A << 16;
	}
	X = CPU.X;
	Y = CPU.Y;

	SaveR8 = SaveR6 = 0;
	if (CPU.P & P_C) SaveR8 |= 0x00000002;
	if (CPU.P & P_V) SaveR8 |= 0x00000001;
	if (CPU.P & P_N) SaveR6 |= 0x00018000;
	if (CPU.P & P_E) SaveR8 |= 0x00000400;   

	if (!(CPU.P & P_Z)) SaveR6 |= 0x00000001;
	SaveR8 |= ((CPU.P << 24) & 0x3c000000) >> 22; 

	D = CPU.DB;  
	D |= CPU.D << 16;

	CPU.unpacked = 1;
	CPU_update();
}

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
	CPU.Cycles = cycles;
		
	CPU_unpack();
	
//	*APU_ADDR_BLK = 0;
	CPU_goto2(cycles);
//	*APU_ADDR_BLK = 1;
	CPU.packed = 0;

//	CPU_pack();
}
