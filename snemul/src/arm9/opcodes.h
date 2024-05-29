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

// SNEMUL (c) 1997 Nicolas

#ifndef __snemul_opcodes_h__
#define __snemul_opcodes_h__

#include	"common.h"

#define P_C  0x01
#define P_Z  0x02
#define P_I  0x04
#define P_D  0x08
#define P_X  0x10
#define P_M  0x20
#define P_V  0x40
#define P_N  0x80

#define P_E  0x100

#ifdef ASM_OPCODES
extern unsigned short P;
extern unsigned short PC;
extern unsigned char  PB, DB, t;
extern unsigned int A, X, Y, D, S;
extern long Cycles;
extern unsigned char	*PCptr;

extern unsigned int		SnesPCOffset;
extern unsigned int		SaveR6;
extern unsigned int		SaveR8;

extern unsigned int		SnesB;

extern	uint32			CPU_log;
extern	uint32			AsmDebug[16];

extern	sint32			CPU_NextCycles;
extern	uint32			CPU_LoopSpeedHacks;
extern	unsigned char	*CPU_WaitAddress;
extern	unsigned char	*CPU_LoopAddress;

extern	uint32			BRKaddress;
extern	uint32			COPaddress;


#define REAL_A ((SaveR8 & 0x00000080) ? \
				(A >> 24 | (SnesB&0xFF000000) >> 16) : (A >> 16))
#define REAL_CYCLES (-((sint32)SaveR8 >> 14))
#define HCYCLES (CPU.HCycles+CPU.Cycles+((sint32)SaveR8 >> 14))
#define ADD_CYCLES(x)	(SaveR8 +=((x)<<14))
//#define FIX_VCOUNT		{ if ((sint32)SaveR8 > 0) { SNES.VCount++; (sint32)SaveR8 -= NB_CYCLES; ) }  
#define SET_WAITCYCLES(c) { CPU_WaitAddress = CPU_LoopAddress; \
								CPU_NextCycles = 0; }
#define SET_WAITCYCLESDELAY(delay) { CPU_WaitAddress = CPU_LoopAddress; \
									 uint32 tmp = NB_CYCLES-delay-CPU.Cycles-CPU.HCycles; \
									 if (tmp < 0) CPU_NextCycles = (tmp) << 14; \
									 else CPU_NextCycles = 0; }
#else

extern unsigned short P;
extern unsigned short PC;
extern unsigned char  PB, DB, t;
extern unsigned short A, X, Y, D, S;
extern long Cycles;

#define REAL_A	A
#define REAL_CYCLES	Cycles
#define HCYCLES	(CPU.HCycles+Cycles)
#define ADD_CYCLES(x)	(Cycles +=(x))
#define SET_WAITCYCLES(c) { CPU.WaitAddress = CPU.LastAddress; \
							CPU.WaitCycles = c; }
#define SET_WAITCYCLESDELAY(delay) { CPU.WaitAddress = CPU.LastAddress; \
									 CPU.WaitCycles = CPU.Cycles-(delay); }
#endif

void	CPU_init();
void	CPU_goto(int cycles);

#endif
