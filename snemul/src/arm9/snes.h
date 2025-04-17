/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* Free for non-commercial use                             */
/***********************************************************/

#ifndef __snes_h__

#include <stdio.h>
#include "common.h"

#define MAP_PPU         1
#define MAP_CPU         2
#define MAP_DSP         3
#define MAP_LOROM_SRAM  4
#define MAP_HIROM_SRAM  5
#define MAP_NONE        6
#define MAP_LAST        10

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

  uchar    	*Map[256*8];
  
  int		SRAMMask;  
};

extern struct s_snescore	SNESC;

struct s_snes
{
/* memory mapping */
  uchar    	BlockIsRAM[256*8];
  uchar    	BlockIsROM[256*8];
  uchar    	*WriteMap[256*8];
  int		HiROM;

/* ports */
  ushort	PPU_Port[0x2000];
  ushort	DMA_Port[0x2000];

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
  uchar         PPU_NeedMultiply;
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
  
  int		stat_before;
  int		stat_before2;
  int		stat_before3;
  int		stat_before4;
  int		stat_CPU;
  int		stat_GFX;
  int		stat_IOREGS;
  int		stat_DMA;
  int		stat_OPC[256];
  int		stat_OPC_cnt[256];
};

extern struct s_snes	SNES;


void	pushb(uint8 value);
void	pushw(uint16 value);

uchar   mem_getbyte(uint32 offset, uchar bank);
void	mem_setbyte(uint32 offset, uchar bank, uchar byte);
ushort  mem_getword(uint32 offset, uchar bank);
void    mem_setword(uint32 offset, uchar bank, ushort word);
void	*map_memory(uint16 offset, uchar bank);

void	GoNMI();
void	GoIRQ();


void	reset_SNES();
int		get_joypad();
void	HDMA_write(uchar port);
void	HDMA_transfert(uchar port);
ROM_Info	*load_ROM(char *ROM, int ROM_size);



#endif