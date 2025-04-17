/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* Free for non-commercial use                             */
/***********************************************************/

#include "fat/gba_nds_fat.h"
#include <malloc.h>
#include <string.h>

#include "cpu.h"
#include "snes.h"
#include "apu.h"
#include "gfx.h"
#include "cfg.h"

#if 0
char current_version[16] = "SNEmulDS 0.3.00";
//char current_version[16] = "SNEmul 0.6.00";

typedef struct {
  char version[16]; // "SNEmul X.X.XX" pour eviter les pb de compatibilit? !
  char names[8][32];
  long entry_pos[8];
} TSnapShot_Header;

typedef struct {
  int            A, X, Y, S, P, D, PB, DB, PC;

  unsigned char  BG_scroll_reg;
  unsigned char  PPU_NeedMultiply, HI_1C, HI_1D, HI_1E, HI_1F, HI_20;
  unsigned char  SC_incr, FS_incr, OAM_upper_byte;

/* unused for compatibilty only */
  int            T0_LATCH, T1_LATCH, T2_LATCH, sPC_offs, cycles_tot;
  unsigned char  APU1, APU2, APU3, APU4, MasterVolume, SPC_CONTROL, DSP_address;
  unsigned char  sA, sX, sY, sSP, f_I, f_P, f_V, f_C, f_N, f_H, f_Z;
  unsigned short sPC, SPC_T0, SPC_T1, SPC_T2;
} TSnapShot;


char	*get_snapshot_name(char *file, uchar nb)
{
  FAT_FILE *f;

  if (nb > 8) return 0;
  if ((f = FAT_fopen(file, "r")) == NULL) return 0;

  TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
  FAT_fread(header, sizeof(TSnapShot_Header), 1, f);

  if (strcmp(header->version, current_version)) goto error;
  if (!header->entry_pos[nb]) goto error;
  if (FAT_fseek(f, header->entry_pos[nb], SEEK_SET)) goto error;

  char *name = strdup(header->names[nb]);

  free(header);
  FAT_fclose(f);
  return name;
  
error:
  free(header);
  FAT_fclose(f);
  return NULL;
}

int	read_snapshot(char *file, uchar nb)
{
  FAT_FILE *f;
  int	i;

  if (nb > 8)
  	 return 0;
  if ((f = FAT_fopen(file, "r")) == NULL)
  	 return 0;

  TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
  FAT_fread(header, sizeof(TSnapShot_Header), 1, f);

  if (strcmp(header->version, current_version) || 
	  !header->entry_pos[nb] ||
      FAT_fseek(f, header->entry_pos[nb], SEEK_SET))
  {
  	 FAT_fclose(f);
  }
 

  FAT_fseek(f, header->entry_pos[nb],SEEK_SET);

  FAT_fread(SNESC.RAM,  0x20000, 1, f);
  FAT_fread(SNESC.VRAM, 0x10000, 1, f);
  FAT_fread(SNESC.SRAM, 0x8000, 1, f);
  for (i = 0; i < 256; i++)
    FAT_fread(GFX.SNESPal+i, 3, 1, f);

  FAT_fread(SNES.PPU_Port, 2*0x100, 1, f);
  FAT_fread(SNES.DMA_Port, 2*0x200, 1, f);

  TSnapShot *snapshot = (TSnapShot *)malloc(sizeof(TSnapShot));
  FAT_fread(snapshot,  sizeof(TSnapShot), 1, f);
  
  iprintf("PC =  %02X:%04x\n", snapshot->PB, snapshot->PC);

  CPU.A  = snapshot->A;  CPU.X  = snapshot->X;  CPU.Y  = snapshot->Y;
  CPU.S  = snapshot->S;  CPU.P  = snapshot->P;  CPU.D  = snapshot->D;
  CPU.PB = snapshot->PB; CPU.DB = snapshot->DB; CPU.PC = snapshot->PC;
  GFX.BG_scroll_reg = snapshot->BG_scroll_reg;
  GFX.SC_incr = snapshot->SC_incr; GFX.FS_incr = snapshot->FS_incr;
  GFX.OAM_upper_byte = snapshot->OAM_upper_byte;
  SNES.PPU_NeedMultiply = snapshot->PPU_NeedMultiply;

   if (CFG.Sound_output) {
	APU_stop(); // Make sure that the APU is *completely* stopped
   	// Read SPC file format
	FAT_fread(APU_RAM_ADDRESS-0x100, 1, 0x10200, f);
	APU_loadSpc();   	
  }

  free(snapshot);
  free(header);
  FAT_fclose(f);

  GFX.tiles_dirty = 1;
  GFX.Sprites_table_dirty = 1;
}

int write_snapshot(char *file, unsigned char nb, const char *name)
{
  FAT_FILE *f;
  int	i;

  if (nb > 8) return 0;
  TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
  if ((f = FAT_fopen(file, "r+")) == NULL) {
  	iprintf("New file\n");
    if ((f = FAT_fopen(file, "w")) == NULL) return 0;
  	iprintf("New file OK\n");    
    bzero(header, sizeof(TSnapShot_Header));
    strcpy(header->version, current_version);
    header->entry_pos[nb] = sizeof(TSnapShot_Header);
  	iprintf("nb = %d %d\n", nb,header->entry_pos[nb] );    
  } else {
    FAT_fread(header, sizeof(TSnapShot_Header), 1, f);
    if (strcmp(header->version, current_version)) {
      FAT_fclose(f); 
      if ((f = FAT_fopen(file, "w")) == NULL) return 0;
      bzero(header, sizeof(TSnapShot_Header));
      strcpy(header->version, current_version);
      header->entry_pos[nb] = sizeof(TSnapShot_Header);
    }
    if (!header->entry_pos[nb]) {
      FAT_fseek(f,0,SEEK_END);
      header->entry_pos[nb] = FAT_ftell(f);
    }
  }  
  strcpy(header->names[nb], name);
  	iprintf("nb = %d %s\n", nb,header->names[nb] );  
  FAT_fseek(f, 0, SEEK_SET); 
  FAT_fwrite(header, sizeof(TSnapShot_Header), 1, f);
  	iprintf("nb = %d %d\n", nb,header->entry_pos[nb] );  
  FAT_fseek(f, header->entry_pos[nb],SEEK_SET);

  FAT_fwrite(SNESC.RAM,  0x20000, 1, f);
  FAT_fwrite(SNESC.VRAM, 0x10000, 1, f);
  FAT_fwrite(SNESC.SRAM, 0x8000, 1, f);
  for (i = 0; i < 256; i++)
    FAT_fwrite(GFX.SNESPal+i, 3, 1, f);
  FAT_fwrite(SNES.PPU_Port,  2*0x100, 1, f);
  FAT_fwrite(SNES.DMA_Port,   2*0x200, 1, f);

  TSnapShot *snapshot = (TSnapShot *)malloc(sizeof(TSnapShot));

  snapshot->A = CPU.A; snapshot->X = CPU.X; snapshot->Y = CPU.Y;
  snapshot->S = CPU.S; snapshot->P = CPU.P; snapshot->D = CPU.D;
  snapshot->PB = CPU.PB; snapshot->DB = CPU.DB; snapshot->PC = CPU.PC;

  snapshot->BG_scroll_reg = GFX.BG_scroll_reg;
  snapshot->SC_incr = GFX.SC_incr; snapshot->FS_incr = GFX.FS_incr;
  snapshot->OAM_upper_byte = GFX.OAM_upper_byte;
  snapshot->PPU_NeedMultiply = SNES.PPU_NeedMultiply;

  	iprintf("snap = %d\n", sizeof(TSnapShot) );
  FAT_fwrite(snapshot,  sizeof(TSnapShot), 1, f);
  
  if (CFG.Sound_output) {
  	APU_stop(); // Make sure that the APU is *completely* stopped
	APU_saveSpc(); 
   	// Write SPC file format
	FAT_fwrite(APU_RAM_ADDRESS-0x100, 1, 0x10200, f);
  }
  
  free(snapshot);
  free(header);
  FAT_fclose(f);
  return (1);
}
#else


typedef struct {
  char name[16];
} TSnapShot_Header;

typedef struct {
  int            A, X, Y, S, P, D, PB, DB, PC;

  unsigned char  BG_scroll_reg;
  unsigned char  PPU_NeedMultiply, HI_1C, HI_1D, HI_1E, HI_1F, HI_20;
  unsigned char  SC_incr, FS_incr, OAM_upper_byte;
} TSnapShot;

char	*get_snapshot_name(char *file, uchar nb)
{
  FAT_FILE *f;

  if (nb > 8) return 0;
  file[strlen(file)-1] = '0'+nb; // XXX.SM1 XXXX.SM2 ....  
  if ((f = FAT_fopen(file, "r")) == NULL) return 0;

  TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
  FAT_fread(header, sizeof(TSnapShot_Header), 1, f);

  char *name = strdup(header->name);

  free(header);
  FAT_fclose(f);
  return name;
  
error:
  free(header);
  FAT_fclose(f);
  return NULL;
}

int	read_snapshot(char *file, uchar nb)
{
  FAT_FILE *f;
  int	i;

  if (nb > 8)
  	 return 0;
  file[strlen(file)-1] = '0'+nb; // XXX.SM1 XXXX.SM2 ....  	 
  if ((f = FAT_fopen(file, "r")) == NULL)
  	 return 0;

  TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
  FAT_fread(header, sizeof(TSnapShot_Header), 1, f);

  FAT_fread(SNESC.RAM,  0x20000, 1, f);
  FAT_fread(SNESC.VRAM, 0x10000, 1, f);
  FAT_fread(SNESC.SRAM, 0x8000, 1, f);
  for (i = 0; i < 256; i++)
    FAT_fread(GFX.SNESPal+i, 3, 1, f);

  FAT_fread(SNES.PPU_Port, 2*0x100, 1, f);
  FAT_fread(SNES.DMA_Port, 2*0x200, 1, f);

  TSnapShot *snapshot = (TSnapShot *)malloc(sizeof(TSnapShot));
  FAT_fread(snapshot,  sizeof(TSnapShot), 1, f);
  
  iprintf("PC =  %02X:%04x\n", snapshot->PB, snapshot->PC);

  CPU.A  = snapshot->A;  CPU.X  = snapshot->X;  CPU.Y  = snapshot->Y;
  CPU.S  = snapshot->S;  CPU.P  = snapshot->P;  CPU.D  = snapshot->D;
  CPU.PB = snapshot->PB; CPU.DB = snapshot->DB; CPU.PC = snapshot->PC;
  GFX.BG_scroll_reg = snapshot->BG_scroll_reg;
  GFX.SC_incr = snapshot->SC_incr; GFX.FS_incr = snapshot->FS_incr;
  GFX.OAM_upper_byte = snapshot->OAM_upper_byte;
  SNES.PPU_NeedMultiply = snapshot->PPU_NeedMultiply;

   if (CFG.Sound_output) {
	APU_stop(); // Make sure that the APU is *completely* stopped
   	// Read SPC file format
	FAT_fread(APU_RAM_ADDRESS-0x100, 1, 0x10200, f);
	APU_loadSpc();   	
  }

  free(snapshot);
  free(header);
  FAT_fclose(f);

  GFX.tiles_dirty = 1;
  GFX.Sprites_table_dirty = 1;
}

int write_snapshot(char *file, unsigned char nb, const char *name)
{
  FAT_FILE *f;
  int	i;

  if (nb > 8) return 0;
  file[strlen(file)-1] = '0'+nb; // XXX.SM1 XXXX.SM2 ....
  // 3 retries for my buggy M3 slim  
  if ((f = FAT_fopen(file, "w")) == NULL)
	  if ((f = FAT_fopen(file, "w")) == NULL)
	  	if ((f = FAT_fopen(file, "w")) == NULL)	  
  		return 0;
  
  TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
  strcpy(header->name, name);  
  FAT_fwrite(header, sizeof(TSnapShot_Header), 1, f);

  FAT_fwrite(SNESC.RAM,  0x20000, 1, f);
  FAT_fwrite(SNESC.VRAM, 0x10000, 1, f);
  FAT_fwrite(SNESC.SRAM, 0x8000, 1, f);
  for (i = 0; i < 256; i++)
    FAT_fwrite(GFX.SNESPal+i, 3, 1, f);
  FAT_fwrite(SNES.PPU_Port,  2*0x100, 1, f);
  FAT_fwrite(SNES.DMA_Port,   2*0x200, 1, f);

  TSnapShot *snapshot = (TSnapShot *)malloc(sizeof(TSnapShot));

  snapshot->A = CPU.A; snapshot->X = CPU.X; snapshot->Y = CPU.Y;
  snapshot->S = CPU.S; snapshot->P = CPU.P; snapshot->D = CPU.D;
  snapshot->PB = CPU.PB; snapshot->DB = CPU.DB; snapshot->PC = CPU.PC;

  snapshot->BG_scroll_reg = GFX.BG_scroll_reg;
  snapshot->SC_incr = GFX.SC_incr; snapshot->FS_incr = GFX.FS_incr;
  snapshot->OAM_upper_byte = GFX.OAM_upper_byte;
  snapshot->PPU_NeedMultiply = SNES.PPU_NeedMultiply;

  	iprintf("State written\n", sizeof(TSnapShot) );
  FAT_fwrite(snapshot,  sizeof(TSnapShot), 1, f);
  
  if (CFG.Sound_output) {
  	APU_stop(); // Make sure that the APU is *completely* stopped
	APU_saveSpc(); 
   	// Write SPC file format
	FAT_fwrite(APU_RAM_ADDRESS-0x100, 1, 0x10200, f);
  }
  
  free(snapshot);
  free(header);
  FAT_fclose(f);
  return (1);
}
#endif