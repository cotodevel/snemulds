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
#define SNES_RAM_ADDRESS	((uint8 *)(0x23C0000))

#define MAP  ((uchar **)(0x06898000))
#define WMAP ((uchar **)(0x0689A000))
//#define MAP ((uchar **)(0x27E0000))
//#define MAP ((uchar **)(0xB000014))
//#define MAP SNES.Map

extern struct s_snescore	SNESC;

struct s_snes
{
/* memory mapping */
  uchar    	BlockIsRAM[256*8];
  uchar    	BlockIsROM[256*8];
/*  uchar    	*Map[256*8];
  uchar    	*WriteMap[256*8];*/
  int		HiROM;

  ROM_Info	ROM_info;

/* HDMA */
  uchar		*HDMA_values[256][8];
  uchar		HDMA_nblines[8], HDMA_port[8], HDMA_info[8];
  uchar		HDMA_line;
  int		UsedCycles;

/* MISC */
  int		NTSC;
  uchar		HIRQ_ok;
  uchar		HIRQ_value;
  uchar     PPU_NeedMultiply;
  uint32	JOY_PORT16;
  uchar		Joy1_cnt, Joy1_rdst;
  uchar		Joy2_cnt, Joy2_rdst;
  int		V_Count;
  int		Mode7Repeat;
  int		v_blank; /* screen ray outside of the draw area */
  int		h_blank;
  uint32	DelayedNMI;

  int		ROMSize;
  int		ROMHeader;

  int		mouse_x;
  int		mouse_y;
  int		mouse_b;
  int		mouse_speed;
  int		prev_mouse_x;
  int		prev_mouse_y;
  int		joypads[4];
  
  int		Stopped;
  int		SRAMWritten;  
  
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

extern struct s_snes	SNES;

//#define SNES	((struct s_snes *)(0x23E0000))

extern uint16	PPU_PORT[0x90]; // 2100 -> 2183
extern uint16	DMA_PORT[0x180]; // 4200 -> 437F

#define EMPTYMEM		(ushort *)(0x23E0000)
//#define PPU_PORT	((ushort *)(0x23E0000))
//#define DMA_PORT	((ushort *)(0x23E4000))

#define SNES_SRAM_ADDRESS ((uchar *)(0x23E6000))

#define SNES_ROM_ADDRESS ((uchar *)(0x20C0000))

#define ROM_MAX_SIZE	(3*1024*1024)
//#define ROM_STATIC_SIZE	(1*1024*1024)
//#define ROM_PAGING_SIZE	(2*1024*1024)
#define ROM_STATIC_SIZE	(64*1024)
#define ROM_PAGING_SIZE	(ROM_MAX_SIZE-ROM_STATIC_SIZE)
#define SNES_ROM_PAGING_ADDRESS (SNES_ROM_ADDRESS+ROM_STATIC_SIZE)

void	pushb(uint8 value);
void	pushw(uint16 value);

uchar   mem_getbyte(uint32 offset, uchar bank);
void	mem_setbyte(uint32 offset, uchar bank, uchar byte);
ushort  mem_getword(uint32 offset, uchar bank);
void    mem_setword(uint32 offset, uchar bank, ushort word);
void	*map_memory(uint16 offset, uchar bank);
void	*mem_getbaseaddress(uint16 offset, uchar bank);

void	GoNMI();
void	GoIRQ();


void	reset_SNES();
int		get_joypad();
void	HDMA_write();
void	HDMA_transfert(uchar port);
void	load_ROM(char *ROM, int ROM_size);



#endif
