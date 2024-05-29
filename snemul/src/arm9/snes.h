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

#ifndef __snes_h__

#include <stdio.h>
#include "common.h"

#ifdef ASM_OPCODES
#define MAP_RELOAD      0x80000000
#define MAP_PPU         0x81000000
#define MAP_CPU         0x82000000
#define MAP_DSP         0x83000000
#define MAP_LOROM_SRAM  0x84000000
#define MAP_HIROM_SRAM  0x85000000
#define MAP_NONE        0x86000000
#define MAP_LAST        0x8F000000
#else
#define MAP_PPU         0x00000001
#define MAP_CPU         0x00000002
#define MAP_DSP         0x00000003
#define MAP_LOROM_SRAM  0x00000004
#define MAP_HIROM_SRAM  0x00000005
#define MAP_NONE        0x00000006
#define MAP_LAST        0x0000000F
#endif

typedef
       struct {
	      char            title[21];
	      unsigned char   banksize:4;
	      unsigned char   ROMspeed:4;
	      unsigned char   ROMtype;
	      unsigned char   ROMsize;
	      unsigned char   SRAMsize;
	      unsigned char   countrycode;
	      unsigned char   license;
	      unsigned char   version;
	      unsigned short  checksum_c;
	      unsigned short  checksum;
       } ROM_Info;

struct s_snescore
{
  uchar		*ROM;
  uchar		*RAM;
  uchar		*VRAM;
  uchar		*SRAM;
  uchar		*BSRAM; /* Battery-saved RAM of non-SuperFX ROM */  
  int		SRAMMask;  
};

/* DS Memory */
#define SNES_RAM_ADDRESS	((uint8 *)(0x27C0000))

#define MAP  ((uchar **)(0x06898000))
#define WMAP ((uchar **)(0x0689A000))
//#define MAP ((uchar **)(0x27E0000))
//#define MAP ((uchar **)(0xB000014))
//#define MAP SNES.Map

/*
  A = SNES.PPU_Port[0x1B]; B = SNES.PPU_Port[0x1C];
  C = SNES.PPU_Port[0x1D]; D = SNES.PPU_Port[0x1E];

  X0 = (int)SNES.PPU_Port[0x1F] << 19; X0 >>= 19;
  Y0 = (int)SNES.PPU_Port[0x20] << 19; Y0 >>= 19;
  Y1 = y;

  HOffset = (int)SNES.PPU_Port[0x0D] << 19; HOffset >>= 19;
  VOffset = (int)SNES.PPU_Port[0x0E] << 19; VOffset >>= 19;
*/  

typedef struct s_lineRegisters
{
	short	A, B, C, D;
	/*int		X0, Y0;
	int		HOffset, VOffset;*/
	int		CX, CY;
	int	    Mode;			
} t_lineRegisters;

struct s_snes
{
/* memory mapping */
  uchar    	BlockIsRAM[256*8];
  uchar    	BlockIsROM[256*8];
/*  uchar    	*Map[256*8];
  uchar    	*WriteMap[256*8];*/
  int		HiROM;

/* ports */
  ushort	PPU_Port[0x2000]; // FIXME: too big
  ushort	DMA_Port[0x2000]; // FIXME: too big

  ROM_Info	*ROM_info;

/* HDMA */
  uchar		*HDMA_values[8][256];
  uchar		HDMA_nblines[8], HDMA_port[8], HDMA_info[8];
  uchar		HDMA_line;
  int		UsedCycles;

/* MISC */
  int		NTSC;
  uchar		HIRQ_ok;
  uchar		HIRQ_value;
  uchar        PPU_NeedMultiply;
  uchar		Joy1_cnt, Joy1_rdst;
  uchar		Joy2_cnt, Joy2_rdst;
  int		V_Count;
  int		Mode7Repeat;
  int		v_blank; /* screen ray outside of the draw area */
  int		h_blank;

/* debug */
  FILE		*flog;
  int		ROMSize;
  int		ROMHeader;

  int		mouse_x;
  int		mouse_y;
  int		mouse_b;
  int		prev_mouse_x;
  int		prev_mouse_y;
  int		joypads[4];
  
  t_lineRegisters	lineRegisters[256];


  int		stat_before;
  int		stat_before2;
  int		stat_before3;
  int		stat_before4;
  int		stat_CPU;
  int		stat_GFX;
  int		stat_IOREGS;
  int		stat_DMA;
/*  int		stat_OPC[256];
  int		stat_OPC_cnt[256];*/
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern struct s_snescore	SNESC;
extern struct s_snes	SNES;
extern void	pushb(uint8 value);
extern void	pushw(uint16 value);
extern uchar   mem_getbyte(uint32 offset, uchar bank);
extern void	mem_setbyte(uint32 offset, uchar bank, uchar byte);
extern ushort  mem_getword(uint32 offset, uchar bank);
extern void    mem_setword(uint32 offset, uchar bank, ushort word);
extern void	*map_memory(uint16 offset, uchar bank);
extern void	*mem_getbaseaddress(uint16 offset, uchar bank);
extern void	GoNMI();
extern void	GoIRQ();
extern void	reset_SNES();
extern int		get_joypad();
extern void	HDMA_write(uchar port);
extern void	HDMA_transfert(uchar port);
extern ROM_Info	*load_ROM(char *ROM, int ROM_size);

//core.c
extern uchar	PPU_port_read(uint32 address);
extern void	PPU_port_write(uint32 address, uchar value);

#ifdef __cplusplus
}
#endif