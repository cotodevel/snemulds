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

char current_version[16] = "SNEmulDS 0.2.00";
//char current_version[16] = "SNEmul 0.6.00";

typedef struct {
  char version[16]; // "SNEmul X.X.XX" pour eviter les pb de compatibilit‚ !
  char names[8][32];
  long entry_pos[8];
} TSnapShot_Header;

typedef struct {
  int            A, X, Y, S, P, D, PB, DB, PC;

  unsigned char  BG_scroll_reg;
  unsigned char  PPU_NeedMultiply, HI_1C, HI_1D, HI_1E, HI_1F, HI_20;
  unsigned char  SC_incr, FS_incr, OAM_upper_byte;

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
 iprintf("RS:0 %d\n", nb);
  if ((f = FAT_fopen(file, "r")) == NULL)
  	 return 0;
 iprintf("RS:0.5\n");  	 

  TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
  FAT_fread(header, sizeof(TSnapShot_Header), 1, f);

 iprintf("RS:1\n");

  if (strcmp(header->version, current_version)) return 0;
 iprintf("RS:2\n");  
  if (!header->entry_pos[nb]) return 0;
 iprintf("RS:3\n");  
  if (FAT_fseek(f, header->entry_pos[nb], SEEK_SET)) return 0;
 iprintf("RS41\n");  

  FAT_fseek(f, header->entry_pos[nb],SEEK_SET);

  FAT_fread(SNESC.RAM,  0x20000, 1, f);
  FAT_fread(SNESC.VRAM, 0x10000, 1, f);
  FAT_fread(SNESC.SRAM, 0x8000, 1, f);
  for (i = 0; i < 256; i++)
    FAT_fread(GFX.SNESPal+i, 3, 1, f);

  FAT_fread(SNES.PPU_Port, 2*0x100, 1, f);
  FAT_fread(SNES.DMA_Port, 2*0x200, 1, f);

/*  FAT_fseek(f, 0xffc0, SEEK_CUR);
//  FAT_fread(APU.MEM, 0xffc0, 1, f);
  FAT_fread(APU.DSP, 0x80, 1, f);*/

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

#if 0
  sA  = snapshot->sA;  sX  = snapshot->sX;  sY  = snapshot->sY;
  sSP = snapshot->sSP; sPC = snapshot->sPC; sPC_base = snapshot->sPC_offs+(int)APU.MEM;
  f_I  = snapshot->f_I; f_P  = snapshot->f_P; f_V  = snapshot->f_V;
  f_C  = snapshot->f_C; f_N  = snapshot->f_N; f_H  = snapshot->f_H;
  f_Z  = snapshot->f_Z; CPU.cycles_tot = 0/*snapshot->cycles_tot*/;
  APU.T0 = snapshot->SPC_T0; APU.T0_LATCH = 0/*snapshot->T0_LATCH*/;
  APU.T1 = snapshot->SPC_T1; APU.T1_LATCH = 0/*snapshot->T1_LATCH*/;
  APU.T2 = snapshot->SPC_T2; APU.T2_LATCH = 0/*snapshot->T2_LATCH*/;
  APU.Port1 = snapshot->APU1; APU.Port2 = snapshot->APU2;
  APU.Port3 = snapshot->APU3; APU.Port4 = snapshot->APU4;
  APU.MasterVolume = snapshot->MasterVolume;
  APU.DSP_address = snapshot->DSP_address;
  APU.CONTROL = snapshot->SPC_CONTROL;
  if (CFG.Sound_output) {
    DSP_close(); DSP_init();
  }
#endif  
//  SPC700_update();

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

/*  FAT_fseek(f, 0xffc0, SEEK_CUR);
  //FAT_fwrite(APU.MEM,  0xffc0, 1, f);
  FAT_fwrite(APU.DSP, 0x80, 1, f);*/

  TSnapShot *snapshot = (TSnapShot *)malloc(sizeof(TSnapShot));

  snapshot->A = CPU.A; snapshot->X = CPU.X; snapshot->Y = CPU.Y;
  snapshot->S = CPU.S; snapshot->P = CPU.P; snapshot->D = CPU.D;
  snapshot->PB = CPU.PB; snapshot->DB = CPU.DB; snapshot->PC = CPU.PC;

  snapshot->BG_scroll_reg = GFX.BG_scroll_reg;
  snapshot->SC_incr = GFX.SC_incr; snapshot->FS_incr = GFX.FS_incr;
  snapshot->OAM_upper_byte = GFX.OAM_upper_byte;
  snapshot->PPU_NeedMultiply = SNES.PPU_NeedMultiply;

#if 0
  snapshot->sA  = sA;  snapshot->sX  = sX;  snapshot->sY  = sY;
  snapshot->sSP = sSP; snapshot->sPC = sPC; snapshot->sPC_offs = sPC_base-(int)APU.MEM;
  snapshot->f_I = f_I; snapshot->f_P = f_P; snapshot->f_V = f_V;
  snapshot->f_C = f_C; snapshot->f_N = f_N; snapshot->f_H = f_H;
  snapshot->f_Z = f_Z; snapshot->cycles_tot = CPU.cycles_tot;
  snapshot->SPC_T0 = APU.T0; snapshot->T0_LATCH = APU.T0_LATCH;
  snapshot->SPC_T1 = APU.T1; snapshot->T1_LATCH = APU.T1_LATCH;
  snapshot->SPC_T2 = APU.T2; snapshot->T2_LATCH = APU.T2_LATCH;
  snapshot->APU1 = APU.Port1; snapshot->APU2 = APU.Port2;
  snapshot->APU3 = APU.Port3; snapshot->APU4 = APU.Port4;

  snapshot->MasterVolume = APU.MasterVolume;
  snapshot->DSP_address = APU.DSP_address;
  snapshot->SPC_CONTROL = APU.CONTROL;
#endif

  	iprintf("snap = %d\n", sizeof(TSnapShot) );

  FAT_fwrite(snapshot,  sizeof(TSnapShot), 1, f);
  free(snapshot);
  free(header);
  FAT_fclose(f);
  return (1);
}
