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

#include "core.h"
#include "snes.h"
#include "apu.h"
#include "gfx.h"
#include "cfg.h"
//#include "superfx.h"
#include "opcodes.h"
#include "ppu.h"
#include "snemulds_memmap.h"
#include "ipcfifoTGDSUser.h"

#define bzero(p, s)	memset(p, 0, s)

void	init_GFX()
{
  init_render();  
}

extern int _offsetY_tab[4];

void	reset_GFX()
{
  memset(&GFX, 0, sizeof(GFX));
//  bzero(GFX.line_spr, 240*128*sizeof(line_sprite_Info));
/*  bzero(GFX.tiles_def[0], 4096);
  bzero(GFX.tiles_def[1], 2048);
  bzero(GFX.tiles_def[2], 1024);*/

  bzero(GFX.spr_info, 128*(sizeof(sprite_Info)));
  bzero(GFX.spr_info_ext, 32);
  bzero(GFX.SNESPal, 256*2);
  //bzero(GFX.buf_screen.line[0], 256*240);
  GFX.BG_scroll_reg = 0;
  GFX.FS_incr = GFX.OAM_upper_byte = 0;
  GFX.ScreenHeight = 224;
  //bzero(GFX.spr_cnt,240);
  GFX.Sprites_table_dirty = 0;
  GFX.SC_incr = 1;
//  memset(GFX.tiles_ry, 8, 4);
  GFX.nb_frames = 0;
  GFX.YScroll = _offsetY_tab[CFG.YScroll];
  GFX.BG3YScroll = 16;
  
  GFX.tile_slot[0] = 4;          	
  GFX.tile_slot[1] = 4;
  GFX.tile_slot[2] = 4;          	
  GFX.tile_slot[3] = 4;  
    
//  update_palette(0);
}

void	reset_CPU()
{
  CPU.IRQ = mem_getword(0xffee, 0);
  CPU.NMI = mem_getword(0xffea, 0);
  CPU.BRK = mem_getword(0xffe6, 0);
  BRKaddress = CPU.BRK; 
  CPU.COP = mem_getword(0xffe4, 0);
  COPaddress = CPU.COP;
  CPU.PC  = mem_getword(0xfffc, 0);
  CPU.D = CPU.PB = CPU.DB = 0;
  CPU.P = P_E | P_M | P_X | P_I;
  CPU.A = CPU.X = CPU.Y = 0;
  CPU.S = 0x1ff;

  CPU_init();	
  PCptr = (u8*)map_memory(CPU.PC, CPU.PB);
  SnesPCOffset = -((sint32)mem_getbaseaddress(CPU.PC, CPU.PB));
  //GUI_printf("PCptr = %08x\n", PCptr);
  CPU.IsBreak = 0;
  CPU.packed = CPU.unpacked = 0;
  
}

void	reset_SNES()
{
  reset_GFX();

  bzero(SNES.HDMA_values, 8*256*4);
  bzero(SNES.HDMA_nblines, 8);
  bzero(SNES.HDMA_port, 8);
  bzero(SNES.HDMA_info, 8);

  memset(SNESC.RAM, 0xFF, 0x20000);
  bzero(SNESC.VRAM, 0x10000);
/*  bzero(PPU_PORT, 0x2000*2);
  bzero(DMA_PORT, 0x2000*2);*/
  
  memset(PPU_PORT, 0, 0x90*2);
  memset(DMA_PORT, 0, 0x100*2);
  memset(DMA_PORT+0x100, 0xFF, 0x80*2);
  SNES.JOY_PORT16 = 0;

    {
      SNESC.SRAMMask = SNES.ROM_info.SRAMsize ?
        ((1 << (SNES.ROM_info.SRAMsize + 3)) * 128) - 1 : 0;
      SNESC.SRAM = SNESC.BSRAM;
      memset(SNESC.SRAM, 0xAA, 0x8000);
    }
   struct s_apu2 *APU2 = (struct s_apu2 *)(&SNEMULDS_IPC->APU2);
//  if (CFG.Sound_output) 
  	APU_nice_reset();

  InitMap();
//  mem_reset_paging(); 
  reset_CPU();


  if (!CFG.Timing)
    SNES.NTSC = (SNES.ROM_info.countrycode < 2);
  else
    SNES.NTSC = CFG.Timing-1;
    
  SNES.PPU_NeedMultiply = 0;
  SNES.DelayedNMI = 0;    

  CFG.BG_Layer = 0xd7; //FIXME: BG3 is not used (MODE 0 broken)

  APU2->skipper_cnt1 = 0;  
  APU2->skipper_cnt2 = 0;  
  APU2->skipper_cnt3 = 0;  
  APU2->skipper_cnt4 = 0;
  
  SNES.V_Count = 0;
  PPU_reset();  
  
  SNES.mouse_x = 128;
  SNES.mouse_y = 112;
  SNES.mouse_speed = 0;
  
  SNES.Stopped = 0;
  
  SNES.SRAMWritten = 0;
}

int cnt_alphachar(char *str)
{
  int i = 0, cnt = 0;

  while (i < 21) {
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

   tmp = TGDSARM9Malloc(0x8000);
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
   TGDSARM9Free(tmp);
}

void	load_ROM(char *ROM, int ROM_size)
{
  int       fileheader, filesize;
  int		cnt1, cnt2;
  char		jap;
  ROM_Info	LoROM_info;
  ROM_Info	HiROM_info;

  filesize = ROM_size;
  fileheader = filesize & 8191;

  if (fileheader != 0 && fileheader != 512)
    fileheader = 512;

  SNES.ROMHeader = fileheader;
  SNES.ROMSize = filesize-fileheader;
  SNESC.ROM = (u8*)(ROM+fileheader);

  if (CFG.InterleavedROM)
    UnInterleaveROM();

  memcpy(&LoROM_info, SNESC.ROM+0x7FC0, sizeof(ROM_Info));
  memcpy(&HiROM_info, SNESC.ROM+0xFFC0, sizeof(ROM_Info));

bool hirom = false;
bool foundByHeader = false;
// conditions necessaires
  if (
      *(unsigned short *)&SNESC.ROM[0xfffc] == 0xFFFF ||
      *(unsigned short *)&SNESC.ROM[0xfffc] < 0x8000) {
    SNES.HiROM = 0; 
    memcpy(&SNES.ROM_info, (void*)&LoROM_info, sizeof(ROM_Info));
    foundByHeader = true;
    hirom = false;
  }
  else if (*(unsigned short *)&SNESC.ROM[0x7ffc] == 0xFFFF ||
      *(unsigned short *)&SNESC.ROM[0x7ffc] < 0x8000) {
    SNES.HiROM = 1; 
    memcpy(&SNES.ROM_info, (void*)&HiROM_info, sizeof(ROM_Info));
    foundByHeader = true;
    hirom = true;
  }

// conditions suffisantes
  else if ((LoROM_info.checksum ^ LoROM_info.checksum_c) == 0xFFFF) {
    SNES.HiROM = 0;
    memcpy(&SNES.ROM_info, (void*)&LoROM_info, sizeof(ROM_Info));
    foundByHeader = true;
    hirom = false;
  }
  else if ((HiROM_info.checksum ^ HiROM_info.checksum_c) == 0xFFFF) {
    SNES.HiROM = 1; 
    memcpy(&SNES.ROM_info, (void*)&HiROM_info, sizeof(ROM_Info));
    foundByHeader = true; 
    hirom = true;
  }

  if(foundByHeader == true){
    if(hirom == true){
    	/*
    	clrscr();
    	printf("---");
    	printf("---");
    	printf("HiROM.");
    	while(1==1){}
    	*/
    	return;
  	}
  	else{
    	/*
    	clrscr();
    	printf("---");
    	printf("---");
    	printf("LoROM.");
    	while(1==1){}
    	*/
    	return;
  	}
  }
// algorithme de ZoOp
  SNES.HiROM = 0;

  cnt1 = cnt_alphachar(LoROM_info.title);
  cnt2 = cnt_alphachar(HiROM_info.title);

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
      if (SNES.HiROM == 1 && jap && (HiROM_info.banksize != 1))
        SNES.HiROM = 0;
    }

  if (SNES.HiROM == 0) {
    memcpy(&SNES.ROM_info, &LoROM_info, sizeof(ROM_Info));
  } else {
    memcpy(&SNES.ROM_info, &HiROM_info, sizeof(ROM_Info));
  }
}
