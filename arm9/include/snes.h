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
#define __snes_h__

#include "typedefsTGDS.h"
#include <stdio.h>
#include "common.h"

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

#define MAP  ((uchar **)(0x06898000))
#define WMAP ((uchar **)(0x0689A000))
extern struct s_snescore	SNESC;
extern struct s_snes	SNES;
extern uint16	PPU_PORT[0x90]; // 2100 -> 2183
extern uint8	DMA_PORT[0x180]; // 4200 -> 437F
extern int cnt_alphachar(char *str);

struct s_snes
{
/* memory mapping */
  uchar    	BlockIsRAM[256*8];
  uchar    	BlockIsROM[256*8];
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
};

//indirect mapped memory
#define MAP_RELOAD      0x80000000
#define MAP_PPU         0x81000000
#define MAP_CPU         0x82000000
#define MAP_DSP         0x83000000
#define MAP_LOROM_SRAM  0x84000000
#define MAP_HIROM_SRAM  0x85000000
#define MAP_NONE        0x8E000000
#define MAP_LAST        0x8F000000

//Rom Page variables
#define EMPTYMEM		(ushort *)(0x2FE0000)
#define SNES_SRAM_ADDRESS ((uchar *)(0x2FE6000))
#define DS_SRAM          ((uint8*)0x0A000000)

/* DS Memory */
#define SNES_RAM_ADDRESS	((uint8 *)(0x02FC0000))

#endif


#ifdef __cplusplus
extern "C"{
#endif

extern uint8	IO_SDD1[8]; // 4800 -> 4807

#ifdef __cplusplus
}
#endif
