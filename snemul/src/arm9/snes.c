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

#include <malloc.h>
#include <string.h>

#include "cpu.h"
#include "snes.h"
#include "apu.h"
#include "gfx.h"
#include "cfg.h"
//#include "superfx.h"
#include "opcodes.h"

#define bzero(p, s)	memset(p, 0, s)

#if 0
struct s_SuperFX	SuperFX;

char    _use_lfn(const char *path)
{
  return (1);
}

void	SuperFX_init()
{
  SuperFX.Regs = malloc(768);
  SuperFX.RamBanks = 2;
  SuperFX.Ram = SNES.ROM+1024*1024*4;
  SuperFX.RomBanks = (2 * 1024 * 1024) / (32 * 1024);
  SuperFX.Rom = (uint8 *)SNES.ROM;
}

void	reset_SuperFX()
{
  SuperFX.RomBanks = SNES.ROMSize >> 15;
  SuperFX.vFlags = 0;
  SuperFXReset(&SuperFX);
}
#endif

void	init_GFX()
{
  /*GFX.tiles_def[0] = malloc(4096);
  GFX.tiles_def[1] = malloc(2048);
  GFX.tiles_def[2] = malloc(1024);*/
  init_render();  
}

static int _offsetY_tab[3] = { 16, 0, 32 };

void	reset_GFX()
{
  memset(&GFX, 0, sizeof(GFX));
//  bzero(GFX.line_spr, 240*128*sizeof(line_sprite_Info));
/*  bzero(GFX.tiles_def[0], 4096);
  bzero(GFX.tiles_def[1], 2048);
  bzero(GFX.tiles_def[2], 1024);*/

  bzero(GFX.spr_info, 128*(sizeof(sprite_Info)));
  bzero(GFX.spr_info_ext, 32);
  bzero(GFX.SNESPal, 256*4);
  //bzero(GFX.buf_screen.line[0], 256*240);
  GFX.BG_scroll_reg = 0;
  GFX.FS_incr = GFX.OAM_upper_byte = 0;
  GFX.ScreenHeight = 224;
  bzero(GFX.spr_cnt,240);
  GFX.Sprites_table_dirty = 0;
  GFX.SC_incr = 1;
//  memset(GFX.tiles_ry, 8, 4);
  GFX.nb_frames = 0;
  GFX.YScroll = _offsetY_tab[CFG.YScroll];
//  update_palette(0);
}

void	reset_CPU()
{
  CPU.IRQ = mem_getword(0xffee, 0);
  CPU.NMI = mem_getword(0xffea, 0);
  CPU.BRK = mem_getword(0xffe6, 0);
  CPU.COP = mem_getword(0xffe4, 0);
#ifndef ASM_OPCODES  
  PC  = mem_getword(0xfffc, 0);
  D = PB = DB = 0;
  P = P_E | P_M | P_X | P_I;
  A = X = Y = 0;
  S = 0x1ff;
#else
  CPU.PC  = mem_getword(0xfffc, 0);
  CPU.D = CPU.PB = CPU.DB = 0;
  CPU.P = P_E | P_M | P_X | P_I;
  CPU.A = CPU.X = CPU.Y = 0;
  CPU.S = 0x1ff;

  CPU_init();	
  PCptr = map_memory(CPU.PC, CPU.PB);
  SnesPCOffset = -((sint32)mem_getbaseaddress(CPU.PC, CPU.PB));
  iprintf("PCptr = %08x\n", PCptr);
#endif  
  CPU.IsBreak = 0;
  CPU.packed = CPU.unpacked = 0;
  
}

void	reset_SNES()
{
#if 0
	if (CFG.SuperFX)
    reset_SuperFX();

  if (CFG.DSP1)
    reset_DSP1();
#endif

  reset_GFX();

  bzero(SNES.HDMA_values, 8*256*4);
  bzero(SNES.HDMA_nblines, 8);
  bzero(SNES.HDMA_port, 8);
  bzero(SNES.HDMA_info, 8);

  memset(SNESC.RAM, 0xFF, 0x20000);
  bzero(SNESC.VRAM, 0x10000);
  bzero(SNES.PPU_Port, 0x2000*2);
  bzero(SNES.DMA_Port, 0x2000*2);
  memset(SNES.DMA_Port+0x100, 0xFF, 0x0100);

//  if (!CFG.SuperFX)
    {
      SNESC.SRAMMask = SNES.ROM_info->SRAMsize ?
        ((1 << (SNES.ROM_info->SRAMsize + 3)) * 128) - 1 : 0;
      SNESC.SRAM = SNESC.BSRAM;
      memset(SNESC.SRAM, 0xAA, 0x8000);
    }
/*  else
    {
      SNESC.SRAMMask = 0xFFFF;
      SNESC.SRAM = SNESC.ROM+1024*1024*4;
      memset(SNESC.SRAM, 0, 0x20000);
    }*/

  /*
  if (CFG.Sound_output)
    SPC700_emu = 1;
  else
    SPC700_emu = 0;
  SPC700_reset();
*/
  APU.counter = 0;
  if (CFG.Sound_output) 
  	APU_nice_reset();

  InitMap();
//  mem_reset_paging(); 
  reset_CPU();

  if (!CFG.Timing)
    SNES.NTSC = (SNES.ROM_info->countrycode < 2);
  else
    SNES.NTSC = CFG.Timing-1;

  CFG.BG_Layer = 0xdf;

  APU.skipper_cnt1 = 0;  
  APU.skipper_cnt2 = 0;  
  APU.skipper_cnt3 = 0;  
  APU.skipper_cnt4 = 0;
  
  SNES.V_Count = 0;
  PPU_reset();  
}

int cnt_alphachar(char *str)
{
  int i = 0, cnt = 0;

  while (i <= 21) {
    if ((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z'))
      cnt++;
    i++;
  }
  return cnt;
}

void	UnInterleaveROM()
{
  int	i, j;
  int	nblocks = SNES.ROMSize >> 15;
  int	step = 64;
  uchar blocks[256], b;
  uchar	*tmp;

  while (nblocks <= step)
    step >>= 1;
	    
  nblocks = step;

  if (CFG.InterleavedROM2)
    {
      for (i = 0; i < nblocks * 2; i++)
        blocks[i] = (i&~0x1e)|((i&2)<<2)|((i&4)<<2)|((i&8)>>2)|((i&16)>>2);
    }
  else
    {
      SNES.HiROM ^= 1;

      for (i = 0; i < nblocks; i++)
        {
  	  blocks[i*2] = i + nblocks;
	  blocks[i*2+1] = i;
        }
   }

   tmp = malloc(0x8000);
   for (i = 0;i < nblocks*2;i++)
     {
       for (j = i;j < nblocks*2;j++)
         {
  	   if (blocks[j] == i)
	     {
	       memmove(tmp, &SNESC.ROM[blocks[j]*0x8000], 0x8000);
	       memmove(&SNESC.ROM[blocks[j]*0x8000],
		       &SNESC.ROM[blocks[i]*0x8000], 0x8000);
	       memmove(&SNESC.ROM[blocks[i]*0x8000], tmp, 0x8000);
	       b = blocks[j];
	       blocks[j] = blocks [i];
	       blocks [i] = b;
	       break;
	     }
	 }
     }
   free(tmp);
}

ROM_Info	*load_ROM(char *ROM, int ROM_size)
{
  ROM_Info	*LoROM_info, *HiROM_info;
  int           fileheader, filesize;
  int		cnt1, cnt2;
  char		jap;

  filesize = ROM_size;
  fileheader = filesize & 8191;

  if (fileheader != 0 && fileheader != 512)
    fileheader = 512;

  SNES.ROMHeader = fileheader;
  SNES.ROMSize = filesize-fileheader;
  SNESC.ROM = ROM+fileheader;

  if (CFG.InterleavedROM)
    UnInterleaveROM();

  LoROM_info = (ROM_Info *)malloc(sizeof(ROM_Info));
  HiROM_info = (ROM_Info *)malloc(sizeof(ROM_Info));
  memcpy(LoROM_info, SNESC.ROM+0x7FC0, sizeof(ROM_Info));
  memcpy(HiROM_info, SNESC.ROM+0xFFC0, sizeof(ROM_Info));

// conditions necessaires
  if (filesize < 0x80000 ||
      *(unsigned short *)&SNESC.ROM[0xfffc] == 0xFFFF ||
      *(unsigned short *)&SNESC.ROM[0xfffc] < 0x8000) {
    SNES.HiROM = 0; free(HiROM_info); return LoROM_info;
  }
  if (*(unsigned short *)&SNESC.ROM[0x7ffc] == 0xFFFF ||
      *(unsigned short *)&SNESC.ROM[0x7ffc] < 0x8000) {
    SNES.HiROM = 1; free(LoROM_info); return HiROM_info;
  }

// conditions suffisantes
  if ((LoROM_info->checksum ^ LoROM_info->checksum_c) == 0xFFFF) {
    SNES.HiROM = 0; free(HiROM_info); return LoROM_info;
  }
  if ((HiROM_info->checksum ^ HiROM_info->checksum_c) == 0xFFFF) {
    SNES.HiROM = 1; free(LoROM_info); return HiROM_info;
  }

// algorithme de ZoOp
  SNES.HiROM = 0;

  cnt1 = cnt_alphachar(LoROM_info->title);
  cnt2 = cnt_alphachar(HiROM_info->title);

  jap = 1;
  if (cnt1 > 0 || cnt2 > 0)
    {
      if (cnt1 == 0 && cnt2 >  0)
        {
          SNES.HiROM = 1; jap = 0;
        }
      if (cnt1 >  0 && cnt2 == 0)
        {
          SNES.HiROM = 0; jap = 0;
        }
      if (cnt1 == cnt2)
        jap = 0;
      if (SNES.HiROM == 0 && cnt2 > cnt1)
        SNES.HiROM = 1;
      if (SNES.HiROM == 1 && jap && (HiROM_info->banksize != 1))
        SNES.HiROM = 0;
    }

  if (SNES.HiROM == 0) {
    free(HiROM_info); return LoROM_info;
  } else {
    free(LoROM_info); return HiROM_info;
  }
}


