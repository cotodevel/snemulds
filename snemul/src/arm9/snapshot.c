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

#include "common.h"

#ifdef USE_GBA_FAT_LIB
#include "fat/gba_nds_fat.h"
#define FILE FAT_FILE
#define fopen FAT_fopen
#define fwrite FAT_fwrite
#define fread FAT_fread
#define fclose FAT_fclose 

#elif defined(USE_LIBFAT)
#include <stdio.h>
#endif
#include <malloc.h>
#include <string.h>

#include "cpu.h"
#include "snes.h"
#include "apu.h"
#include "gfx.h"
#include "cfg.h"

typedef struct {
  char name[16];
} TSnapShot_Header;

typedef struct {
  int            A, X, Y, S, P, D, PB, DB, PC;

  unsigned char  BG_scroll_reg;
  unsigned char  PPU_NeedMultiply;
  unsigned char	 options[5];
  //unsigned char HI_1C, HI_1D, HI_1E, HI_1F, HI_20;
  unsigned char  SC_incr, FS_incr, OAM_upper_byte;
} TSnapShot;

int		get_snapshot_name(char *file, uchar nb, char *name)
{
  FILE *f;

  if (nb > 8) return 0;
  file[strlen(file)-1] = '0'+nb; // XXX.SM1 XXXX.SM2 ....
  FS_lock();
  if ((f = fopen(file, "r")) == NULL)
  {
	  FS_unlock();
	  return 0;
  }

  TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
  fread(header, sizeof(TSnapShot_Header), 1, f);

  char header_name[17];
  memcpy(header_name, header->name, 16);
  header_name[16] = 0;
  sprintf(name, "Save #%d - %s", nb, header_name);

  free(header);
  fclose(f);
  FS_unlock();
  return 1;
}

int	read_snapshot(char *file, uchar nb)
{
  FILE *f;
  int	i;

  if (nb > 8)
  	 return 0;
  file[strlen(file)-1] = '0'+nb; // XXX.SM1 XXXX.SM2 ....
  FS_lock();
  if ((f = fopen(file, "r")) == NULL)
  {
	 FS_unlock();
  	 return 0;
  }

  TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
  fread(header, sizeof(TSnapShot_Header), 1, f);

  fread(SNESC.RAM,  0x20000, 1, f);
  fread(SNESC.VRAM, 0x10000, 1, f);
  fread(SNESC.SRAM, 0x8000, 1, f);
  for (i = 0; i < 256; i++)
  {
  	uint8	pal[3];
    fread(pal, 3, 1, f);
    GFX.SNESPal[i] = (pal[2]>>1)|((pal[1]>>1)<<5)|((pal[0]>>1)<<10);
  }

/*  fread(PPU_PORT, 2*0x100, 1, f);
  fread(DMA_PORT, 2*0x200, 1, f);*/

  fread(PPU_PORT, 2*0x90, 1, f);
  fread(EMPTYMEM, 2*0x70, 1, f); // unused
  fread(DMA_PORT, 2*0x180, 1, f);
  fread(EMPTYMEM, 2*0x80, 1, f); // unused

  TSnapShot *snapshot = (TSnapShot *)malloc(sizeof(TSnapShot));
  fread(snapshot,  sizeof(TSnapShot), 1, f);
  
  //iprintf("PC =  %02X:%04x\n", snapshot->PB, snapshot->PC);

  CPU.A  = snapshot->A;  CPU.X  = snapshot->X;  CPU.Y  = snapshot->Y;
  CPU.S  = snapshot->S;  CPU.P  = snapshot->P;  CPU.D  = snapshot->D;
  CPU.PB = snapshot->PB; CPU.DB = snapshot->DB; CPU.PC = snapshot->PC;
  GFX.BG_scroll_reg = snapshot->BG_scroll_reg;
  GFX.SC_incr = snapshot->SC_incr; GFX.FS_incr = snapshot->FS_incr;
  GFX.OAM_upper_byte = snapshot->OAM_upper_byte;
  SNES.PPU_NeedMultiply = snapshot->PPU_NeedMultiply;
  
  // FIXME: we should also save sprite infos
  
  if (snapshot->options[0] >= 1)
  {
  	unpackOptions(snapshot->options[0], &snapshot->options[1]); 	
  }

   if (CFG.Sound_output) {
	APU_stop(); // Make sure that the APU is *completely* stopped
   	// Read SPC file format
	fread(APU_RAM_ADDRESS-0x100, 1, 0x10200, f);
	APU_loadSpc(); 
  }

  free(snapshot);
  free(header);
  fclose(f);
  FS_unlock();

  GFX.tiles_dirty = 1;
  GFX.Sprites_table_dirty = 1;
  CPU.unpacked = 0; // Update ASM  
}

int write_snapshot(char *file, unsigned char nb, const char *name)
{
  FILE *f;
  int	i;

  if (nb > 8) return 0;
  file[strlen(file)-1] = '0'+nb; // XXX.SM1 XXXX.SM2 ....
  // 3 retries for my buggy M3 slim  
  FS_lock();
  if ((f = fopen(file, "w")) == NULL)
/*	  if ((f = fopen(file, "w")) == NULL)
	  	if ((f = fopen(file, "w")) == NULL)*/
  {
	  FS_unlock();
  	  return 0;
  }
  
  TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
  strcpy(header->name, name);  
  fwrite(header, sizeof(TSnapShot_Header), 1, f);

  fwrite(SNESC.RAM,  0x20000, 1, f);
  fwrite(SNESC.VRAM, 0x10000, 1, f);
  fwrite(SNESC.SRAM, 0x8000, 1, f);
  for (i = 0; i < 256; i++)
  {
  	uint8	pal[3];
  	pal[2] = GFX.SNESPal[i]<<1;
  	pal[1] = (GFX.SNESPal[i]>>5)<<1;
  	pal[0] = (GFX.SNESPal[i]>>10)<<1;  	
    fwrite(pal, 3, 1, f);
  }
    
  fwrite(PPU_PORT,  2*0x90, 1, f);
  fwrite(EMPTYMEM, 	2*0x70, 1, f); // unused
  fwrite(DMA_PORT,  2*0x180, 1, f);
  fwrite(EMPTYMEM, 	2*0x80, 1, f); // unused
  
  TSnapShot *snapshot = (TSnapShot *)malloc(sizeof(TSnapShot));

  snapshot->A = CPU.A; snapshot->X = CPU.X; snapshot->Y = CPU.Y;
  snapshot->S = CPU.S; snapshot->P = CPU.P; snapshot->D = CPU.D;
  snapshot->PB = CPU.PB; snapshot->DB = CPU.DB; snapshot->PC = CPU.PC;

  snapshot->BG_scroll_reg = GFX.BG_scroll_reg;
  snapshot->SC_incr = GFX.SC_incr; snapshot->FS_incr = GFX.FS_incr;
  snapshot->OAM_upper_byte = GFX.OAM_upper_byte;
  snapshot->PPU_NeedMultiply = SNES.PPU_NeedMultiply;
  
  snapshot->options[0] = 2;
//	iprintf("\nUpdate Options\n");
  packOptions(&snapshot->options[1]);

  GUI_console_printf(0, 23, "State written");
  
  fwrite(snapshot,  sizeof(TSnapShot), 1, f);
  
  if (CFG.Sound_output) {
  	APU_stop(); // Make sure that the APU is *completely* stopped
	APU_saveSpc(); 
   	
	fwrite(APU_RAM_ADDRESS-0x100, 1, 0x10200, f);
  }
  
  free(snapshot);
  free(header);
  fclose(f);
  FS_unlock();
  return (1);
}
