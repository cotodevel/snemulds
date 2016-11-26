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

//coto: 23/04/2016 stricmp was deprecated by GCC in 2015, so good candidates are both strcasecmp and strncasecmp

#ifndef stricmp
    #define stricmp  strcasecmp 	//stricmp
#endif

#ifndef strnicmp
    #define strnicmp strncasecmp   //strnicmp
#endif

#endif


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

//CPU Hardware
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

#endif

#ifdef __cplusplus
extern "C" {
#endif

//CPU opcodes
void	CPU_init();
void	CPU_goto(int cycles);
void    CPU_goto2(int cycles);

uchar   mem_getbyte(uint32 offset, uchar bank);
void	mem_setbyte(uint32 offset, uchar bank, uchar byte);
ushort  mem_getword(uint32 offset, uchar bank);
void    mem_setword(uint32 offset, uchar bank, ushort word);
int		map_duplicate(int snes_block);
void	LOG(char *fmt, ...);
void	CPU_pack();

void	PPU_port_write(uint32 address, uint8 value);
uchar	PPU_port_read(uint32 address);

u32 nopinlasm();
u8 copy8arm(u32 src,u32 dest, u32 size);

extern struct s_cpu	CPU;

//engine.c

void	GUI_showROMInfos(int size);
int		FS_saveFile(char *filename, char *buf, int size);
void	PPU_line_render_scaled();
void	PPU_line_render();

extern int CPU_break;
extern uint32 APU_MAX;
extern uint32 screen_mode;

//core.c
void	DMA_port_write(uint32 address, uint8 byte);
uint8	DMA_port_read(uint32 address);
static inline void HDMA_write_port(uchar port, uint8 *data);
void	HDMA_write();
void	read_joypads();
void	read_mouse();
void 	read_scope();
void	update_joypads();
void 	SNES_update();
void 	GoNMI();
void 	GoIRQ();
inline void HDMA_write_port(uchar port, uint8 *data);
int		PPU_fastDMA_2118_1(int offs, int bank, int len);
void 	DMA_transfert(uchar port);
void	HDMA_transfert(unsigned char port);

uint32	IONOP_DMA_READ(uint32 addr);
uint32	IONOP_PPU_READ(uint32 addr);
void	IONOP_PPU_WRITE(uint32 addr, uint32 byte);
void	IONOP_DMA_WRITE(uint32 addr, uint32 byte);

void CPU_pack();

//input.c
extern u32 keys;

//debug.c
extern	uint32			CPU_log;
extern uint32	APU_MAX;
void PPU_ChangeLayerConf(int i);

//opcodes2.s
extern void CPU_update();

#ifdef __cplusplus
}
#endif
