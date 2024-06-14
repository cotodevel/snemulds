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

#define NB_CYCLES 180

#define REAL_A ((SaveR8 & 0x00000080) ? \
			(A >> 24 | (SnesB&0xFF000000) >> 16) : (A >> 16))
#define REAL_CYCLES (-((sint32)SaveR8 >> 14))
#define HCYCLES (CPU.HCycles+CPU.Cycles+((sint32)SaveR8 >> 14))
#define ADD_CYCLES(x)	(SaveR8 +=((x)<<14))
//#define FIX_VCOUNT		{ if ((sint32)SaveR8 > 0) { SNES.VCount++; (sint32)SaveR8 -= NB_CYCLES; ) }  
#define SET_WAITCYCLES(c) {	\
	CPU_WaitAddress = CPU_LoopAddress; \
	CPU_NextCycles = 0; }

#define SET_WAITCYCLESDELAY(delay) {	\
	CPU_WaitAddress = CPU_LoopAddress; \
	uint32 tmp = NB_CYCLES-delay-CPU.Cycles-CPU.HCycles; \
	if (tmp < 0) CPU_NextCycles = (tmp) << 14; \
	else CPU_NextCycles = 0;	\
}

struct s_cpu
{
  uint16	IRQ, NMI, BRK, COP; /* interruption address */
  int		cycles_tot;
  int		NMIActive;
  uchar		WAI_state;

/* debug */
  int		Trace_flag;
  int		Trace;
  int		Cycles2;

/* registers */
#define P_C  0x01
#define P_Z  0x02
#define P_I  0x04
#define P_D  0x08
#define P_X  0x10
#define P_M  0x20
#define P_V  0x40
#define P_N  0x80
#define P_E  0x100
  uint16        P; /* Flags Register */
  uint16        PC; /* Program Counter */
  uint16        PB, DB; /* Bank Registers */
  uint16        A, X, Y, D, S;

  int           Cycles;

#define IRQ_GSU	1
  int		IRQState;

/* speed hack */
  int           LastAddress;
  int           WaitAddress;
  int           WaitCycles;
  uint32		HCycles;
  
  int 			IsBreak;
  
  int			unpacked;
  int			packed;
};

extern struct s_cpu	CPU;

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
