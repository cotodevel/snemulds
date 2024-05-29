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

#ifndef __cpu_h__
#define __cpu_h__

#include "common.h"

#define NB_CYCLES 180

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
