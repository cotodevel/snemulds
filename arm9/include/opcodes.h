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

#ifndef __opcodes_h__
#define __opcodes_h__

#include	"common.h"
#include	"typedefsTGDS.h"

#define P_C  0x01
#define P_Z  0x02
#define P_I  0x04
#define P_D  0x08
#define P_X  0x10
#define P_M  0x20
#define P_V  0x40
#define P_N  0x80

#define P_E  0x100

#endif


#define REAL_A ((SaveR8 & 0x00000080) ? \
			(A >> 24 | (SnesB&0xFF000000) >> 16) : (A >> 16))
#define REAL_CYCLES (-((sint32)SaveR8 >> 14))
#define HCYCLES (CPU.HCycles+(CPU.Cycles-16)+((sint32)SaveR8 >> 14))
#define ADD_CYCLES(x)	(SaveR8 +=((x)<<14))
//#define FIX_VCOUNT		{ if ((sint32)SaveR8 > 0) { SNES.VCount++; (sint32)SaveR8 -= NB_CYCLES; ) }  
#define SET_WAITCYCLES(c) {	\
	CPU_WaitAddress = CPU_LoopAddress; \
	CPU_NextCycles = 0; }

#define SET_WAITCYCLESDELAY(delay) {	\
	CPU_WaitAddress = CPU_LoopAddress; \
	CPU_NextCycles = -1; \
}

//CPU Hardware
#ifndef __opcodes_cpu_snemul__
#define __opcodes_cpu_snemul__

#include "common.h"

#define NB_CYCLES 137

#endif

#ifdef __cplusplus
extern "C" {
#endif

//Snezzi Assembler Snes Core
extern unsigned short P;
extern unsigned short PC;
extern uint8  PB, DB, t;
extern unsigned int A, X, Y, D, S;
extern long Cycles;
extern uint8	*PCptr;
extern unsigned int		SnesPCOffset;
extern unsigned int		SaveR6;
extern unsigned int		SaveR8;
extern unsigned int		SnesB;
extern	uint32			CPU_log;
extern	uint32			AsmDebug[16];
extern	sint32			CPU_NextCycles;
extern	uint8	*CPU_WaitAddress;
extern	uint8	*CPU_LoopAddress;
extern	uint32			BRKaddress;
extern	uint32			COPaddress;
extern void CPU_pack();
extern void CPU_unpack();
extern void	pushb(uint8 b);
extern void pushw(uint16 w);
extern uint8	pullb();
extern uint16	pullw();
extern void CPU_goto(int cycles);


//opcodes2.s
extern void 	CPU_goto2(int CyclesToCrunch);
extern void 	CPU_update();
extern void		CPU_init();	
#ifdef __cplusplus
}
#endif
