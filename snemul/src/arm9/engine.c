/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* Free for non-commercial use                             */
/***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <nds/memory.h>
#include <string.h>
#include <time.h>
#ifdef WIN32
#include <allegro.h>
#endif

#include "opcodes.h"

#include "common.h"
#include "fs.h"

#include "snes.h"
#include "gfx.h"
#include "cpu.h"
#include "apu.h"
#include "cfg.h"

extern int CPU_break;

extern uint32 screen_mode;
//extern volatile uint32 h_blank;

struct s_snes SNES;
struct s_cfg CFG;

#define DS_SRAM          ((uint8*)0x0A000000)

void writeSRAM(int offset, uint8* src, int size) {
    
}

void readSRAM(int offset, uint8* dest, int size) {
    
} 


int loadSRAM()
{
  char *sramFile;
  	
  if (SNESC.SRAMMask)
    {
#ifdef USE_GBFS
/*		char header[16];
		
		readSRAM(0, (uint8 *)header, 16);
		if (!strcmp(header, "SNEmulDS SRAM"))
		{
			iprintf("Found SRAM header!\n");
			readSRAM(16, SNESC.SRAM, SNESC.SRAMMask+1);
			return 0;
		}*/ 
#endif    	
    	sramFile = strdup(CFG.ROMFile);
		strcpy(strrchr(sramFile, '.'), ".SRM");
    	FS_loadFile(sramFile, SNESC.SRAM, SNESC.SRAMMask+1);
    	free(sramFile);
    }	
	return 0;    
}


int saveSRAM()
{
  char *sramFile;
  	
  if (SNESC.SRAMMask)
    {
#ifndef USE_GBFS    	
    	sramFile = strdup(CFG.ROMFile);
		strcpy(strrchr(sramFile, '.'), ".SRM");
    	FS_saveFile(sramFile, SNESC.SRAM, SNESC.SRAMMask+1);
    	free(sramFile);
#else
/*		char header[16];
		
		memset(header, 0, 16);
		strcpy(header, "SNEmulDS SRAM");
		writeSRAM(0, (uint8 *)header, 16);
		writeSRAM(16, SNESC.SRAM, SNESC.SRAMMask+1);*/
#endif    	    	
    }	
}




/* ALLEGRO ----------------------------- */

/*
#ifndef WIN32
typedef 
		struct {
			uchar *buf;
		} BITMAP;
		
typedef 
		struct {
			uchar *buf;
		} RLE_SPRITE;
		
typedef
		struct
		{
			uchar r;
			uchar g;
			uchar b;
		} RGB;
		
typedef
		RGB *PALETTE;
		
void set_color(int i, RGB *rgb) { };
#define set_gfx_mode(mode, w, h, a, b)
#define clear_to_color(scr, c)
BITMAP *create_bitmap(int w, int h) { return NULL;};
void blit(BITMAP *bm, void *t, int a, int b, int c, int d, int w, int h) {};
void allegro_init() {};
void allegro_exit() {};		

#define LOCK_FUNCTION(x)
#define LOCK_VARIABLE(y)
#endif

BITMAP        *buf_screen, *page1, *page2;
BITMAP        *sprite_buf;
*/
unsigned char interrupted;
extern long Cycles;

extern int v_blank;

//END_OF_FUNCTION(V_Blank_timer);

void SetCFG()
{
  CFG.GUI_resol        = 0;
  CFG.FullScreen_resol = 0;
  CFG.Scanlines        = 0;

  CFG.FullScr          = 1;
  CFG.FullGUI          = 0;

  CFG.Joy1_Enabled     = 1;
  CFG.joy_type         = 0;
//  joy_type = alleg_joy_type[CFG.joy_type];

  CFG.ADSR_GAIN        = 0;
  CFG.Sound_output     = 1; // SOUND!!!
  CFG.Stereo           = 0;
  CFG.FastSound        = 0;

  CFG.auto_skip        = 1;
  CFG.frame_rate       = 1;
  CFG.GUI_frame_rate   = 1;
  CFG.CPU_speedhack    = 1;
  CFG.SPC_speedhack    = 1;
  CFG.ShowFPS = 0;
  CFG.Work_dir         = NULL;

  CFG.buffer_size = 31;
  CFG.PPU_Clip = 1;
  CFG.Debug = FALSE;
  CFG.Debug2 = FALSE;
  CFG.CPU_log = FALSE;

  CFG.WaitVBlank = 0;

  CFG.DSP1 = 0;
  CFG.SuperFX = 0;
}

int	changeROM(char *ROM, int size)
{
  CFG.frame_rate = 1;
  CFG.DSP1 = CFG.SuperFX = 0;
  CFG.InterleavedROM = CFG.InterleavedROM2 = 0;

   if (SNES.ROM_info)
   {
   		free(SNES.ROM_info);
   }
 
	// Write SRAM
    SNES.ROM_info = (ROM_Info *)load_ROM(ROM, size);
    SNES.ROM_info->title[21] = '\0';
    iprintf("Title   : %s\n", SNES.ROM_info->title);
    iprintf("Size    : %d bytes\n", size);
    if (SNES.HiROM) 
    iprintf("ROM type: HiROM\n");
    else 
    iprintf("ROM type: LoROM\n");
    iprintf("Country : %s\n", SNES.ROM_info->countrycode < 2 ? "NTSC" : "PAL");
    iprintf("\n");
    iprintf("Press START to launch ROM\n");
    iprintf("Press SELECT to go back\n");    

  reset_SNES();	
	// Clear screen
	// Read SRAM
  loadSRAM();	
}

int initSNESEmpty()
{
  CFG.BG3Squish = 2;
  CFG.WaitVBlank = 0;
  CFG.YScroll = 0;
  CFG.CPU_speedhack = 1;
  
  CFG.frame_rate = 1;
  CFG.DSP1 = CFG.SuperFX = 0;
  CFG.InterleavedROM = CFG.InterleavedROM2 = 0;
  CFG.Sound_output = 1;
//  CFG.Sound_output = 0;

  memset(&SNES, 0, sizeof(SNES));
  memset(&SNESC, 0, sizeof(SNESC));

//  SNES.flog = fopen("snemul.log", "w");
//	SNES.flog = stdout;

/* allocate memory */
//  SNES.ROM = (uchar *)malloc(0x420000);
  SNESC.RAM = (uchar *)malloc(0x020000);
  SNESC.VRAM = (uchar *)malloc(0x010000);
  SNESC.BSRAM = (uchar *)malloc(0x8000);
  init_GFX();

  GFX.Graph_enabled = 1;

  iprintf("Initialize ..\n");

#ifdef WIN32
 buf_screen = create_bitmap(256, 240); clear(buf_screen);
 sprite_buf = create_bitmap(8, 8);
 
  LOCK_VARIABLE(v_blank);
  LOCK_FUNCTION(V_Blank_timer);
#endif

  iprintf("Init OK...\n");
  return 0;
}
	
int OldPC;

extern int frame;
extern uint8 *PCptr;

extern void CPU_goto(int x);

int go()
{
	if (CPU.IsBreak) return;
	
	while (1)
	{
		
		if (v_blank)
		{
    		if (!CFG.WaitVBlank && GFX.need_update) 	
				draw_screen();
			GFX.need_update = 0;
			v_blank = 0;
		}
		
	/* HBLANK */
    if (SNES.DMA_Port[0x00]&0x10) {
      if (!(SNES.DMA_Port[0x00]&0x20) ||
           SNES.V_Count == (SNES.DMA_Port[0x09]&0xff)+((SNES.DMA_Port[0x0A]&0xff)<<8)) {
	    int H_cycles;
	    H_cycles = (SNES.DMA_Port[0x07]+(SNES.DMA_Port[0x08]<<8))/2;
		CPU.WaitAddress = -1; CPU.Cycles = CPU.WaitCycles = H_cycles;
		CPU_goto(CPU.Cycles); if (CPU.IsBreak) return;
        /*if (CFG.BG_Layer&0x40) */GoIRQ();
        SNES.DMA_Port[0x11] = 0x80;
        CPU.Cycles = NB_CYCLES-H_cycles-SNES.UsedCycles;
      } else
        CPU.Cycles = NB_CYCLES-SNES.UsedCycles;
    } else
      CPU.Cycles = NB_CYCLES-SNES.UsedCycles;

	CPU.WaitAddress = -1; CPU.WaitCycles = CPU.Cycles;
    CPU_goto(CPU.Cycles); if (CPU.IsBreak) return;
    CPU.Cycles = NB_CYCLES;
#if 0
    if (CPU.IRQState)
        {
          CPU.IRQState = 0;
          /*if (CFG.BG_Layer&0x40) */GoIRQ();
          CPU.Cycles = CPU.SavedCycles;
          CPU.WaitAddress = -1; CPU.WaitCycles = CPU.Cycles;
          goto_h_blank();
        }
    }
#endif

    CPU.cycles_tot += NB_CYCLES; SNES.UsedCycles = 0;

    SNES.V_Count++;

	if  (CFG.Sound_output)
	{
/*		int incr;
		
		incr = 100 / GFX.speed;	
		
		100 / GFX.speed * 261;
		
		APU.counter += incr;
		*APU_ADDR_CNT += incr;
		
		if (APU.counter > GFX.speed*/ 
		
		 
#if 0		
	 	 if ((SNES.V_Count & 1   ) == 0)
				(*APU_ADDR_CNT)++; // I'm too slow :(
#endif				
		(*APU_ADDR_CNT)++;			
		APU.counter ++;
				
//		SNES.h_blank = 0;		
	}
 
#if 0
    if (CFG.Sound_output) {
      if (SNES.V_Count & 63) {
        if (SNES.V_Count & 1) {
          if (!SNES.NTSC && ((SNES.V_Count&7) == 1)) {
            APU.TIM0++; APU.TIM1++; APU.TIM2+=4;
          }
          if (++APU.TIM0 >= APU.T0) {
            APU.TIM0 -= APU.T0; APU.CNT0++;
            if (APU.CONTROL&1) { SPC700_emu = 1; APU_WaitCounter++; }
          }
          if (++APU.TIM1 >= APU.T1) {
            APU.TIM1 -= APU.T1; APU.CNT1++;
            if (APU.CONTROL&2) { SPC700_emu = 1; APU_WaitCounter++; }
          }
        }
        APU.TIM2 += 4;
        if (APU.TIM2 >= APU.T2) {
          APU.TIM2 -= APU.T2; APU.CNT2++;
          if (APU.CONTROL&4) { SPC700_emu = 1; APU_WaitCounter++; }
        }
      }
    } else
      SPC700_emu = 0;
#endif
	if (SNES.V_Count > (SNES.NTSC ? 261 : 311))
    {
      SNES.V_Count = 0;
      return 0;
    }


    if (SNES.V_Count < GFX.ScreenHeight) {
#if 0    	
      if (GFX.Sprites_table_dirty) {
/*        if ((CFG.BG_Layer&0x10))
          draw_sprites();*/

//draw_screen();    
        GFX.Sprites_table_dirty = 0;
      }
#endif      
      if (!(SNES.PPU_Port[0x00]&0x80) && SNES.DMA_Port[0x0C] && CFG.BG_Layer&0x80)
      {
        int HDMASel = SNES.DMA_Port[0x0C];

        if (HDMASel&0x01 && SNES.HDMA_line < SNES.HDMA_nblines[0]) HDMA_write(0);
        if (HDMASel&0x02 && SNES.HDMA_line < SNES.HDMA_nblines[1]) HDMA_write(1);
        if (HDMASel&0x04 && SNES.HDMA_line < SNES.HDMA_nblines[2]) HDMA_write(2);
        if (HDMASel&0x08 && SNES.HDMA_line < SNES.HDMA_nblines[3]) HDMA_write(3);
        if (HDMASel&0x10 && SNES.HDMA_line < SNES.HDMA_nblines[4]) HDMA_write(4);
        if (HDMASel&0x20 && SNES.HDMA_line < SNES.HDMA_nblines[5]) HDMA_write(5);
        if (HDMASel&0x40 && SNES.HDMA_line < SNES.HDMA_nblines[6]) HDMA_write(6);
        if (HDMASel&0x80 && SNES.HDMA_line < SNES.HDMA_nblines[7]) HDMA_write(7);
      }
      SNES.HDMA_line++;

      if ((!(SNES.PPU_Port[0x00]&0x80)) && (SNES.PPU_Port[0x00]&0x0f)) {
        GFX.was_not_blanked = 1;

		// draw line
      }
    }
    if ((SNES.DMA_Port[0x00]&0x20) && !(SNES.DMA_Port[0x00]&0x10) &&
      SNES.V_Count == (SNES.DMA_Port[0x09]&0xff)+((SNES.DMA_Port[0x0A]&0xff)<<8))
      GoIRQ();
    if (SNES.V_Count == (SNES.NTSC ? 261 : 311)) {
    	
      // V_blank
      SNES.v_blank = 0; SNES.DMA_Port[0x10] = 0;
      if (SNES.DMA_Port[0x0C] && CFG.BG_Layer&0x80) {
        if (SNES.DMA_Port[0x0C]&0x01) HDMA_transfert(0);
        if (SNES.DMA_Port[0x0C]&0x02) HDMA_transfert(1);
        if (SNES.DMA_Port[0x0C]&0x04) HDMA_transfert(2);
        if (SNES.DMA_Port[0x0C]&0x08) HDMA_transfert(3);
        if (SNES.DMA_Port[0x0C]&0x10) HDMA_transfert(4);
        if (SNES.DMA_Port[0x0C]&0x20) HDMA_transfert(5);
        if (SNES.DMA_Port[0x0C]&0x40) HDMA_transfert(6);
        if (SNES.DMA_Port[0x0C]&0x80) HDMA_transfert(7);
      }
//      memset(GFX.tiles_ry,8,4);
/*      if (GFX.frame_d && (CFG.BG_Layer & 0x10))
        draw_sprites();*/
//        if (GFX.nb_frames & 1)
      		draw_screen();
      GFX.BG_scroll_reg = 0;
      SNES.HDMA_line = 0;
      SNES.PPU_Port[0x02] = GFX.Old_SpriteAddress;
      GFX.OAM_upper_byte = 0;
//      patch_memory();
    }
    if (SNES.V_Count == GFX.ScreenHeight) {
      if (SNES.DMA_Port[0x00]&0x80) GoNMI();
      SNES.HIRQ_ok = 1;
      GFX.was_not_blanked = 0; GFX.nb_frames++;
      
      if (GFX.nb_frames > 100)
      {
        GUI_printf(0, 22, "% 3d%% %d", GFX.nb_frames * 100 / frame, *APU_ADDR_CNT);

		GFX.speed = GFX.nb_frames * 100 / frame;
        GFX.nb_frames = 0;
        frame = 0;      	
      }
      
            
      if (!CPU.NMIActive) SNES.DMA_Port[0x10] |= 0x80;
      SNES.v_blank = 1; CPU.NMIActive = 0;
      update_joypads();
	}
	
   }

	return 0;
}

void show_opcode(char *buf, unsigned char opcode, int pc, int pb, unsigned short flags)
{
  switch(opcode) {
    case 0xEA : sprintf(buf, "NOP"); break;
    case 0x14 : sprintf(buf, "TRB $%02X",mem_getbyte(pc+1,pb)); break;
    case 0x1C : sprintf(buf, "TRB $%04X", mem_getword(pc+1,pb)); break;
    case 0x04 : sprintf(buf, "TSB $%02X",mem_getbyte(pc+1,pb)); break;
    case 0x0C : sprintf(buf, "TSB $%04X", mem_getword(pc+1,pb)); break;
    case 0x29 :
if (flags&P_M)  sprintf(buf, "AND #%02X", mem_getbyte(pc+1,pb));
           else sprintf(buf, "AND #%04X", mem_getword(pc+1,pb)); break;
    case 0x21 : sprintf(buf, "AND ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0x31 : sprintf(buf, "AND ($%02X),Y", mem_getbyte(pc+1,pb)); break;
    case 0x32 : sprintf(buf, "AND ($%02X)", mem_getbyte(pc+1,pb)); break;
    case 0x33 : sprintf(buf, "AND ($%02X,S),Y", mem_getbyte(pc+1,pb)); break;
    case 0x27 : sprintf(buf, "AND [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0x37 : sprintf(buf, "AND [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0x23 : sprintf(buf, "AND $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0x25 : sprintf(buf, "AND $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x35 : sprintf(buf, "AND $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0x3D : sprintf(buf, "AND $%04X,X", mem_getword(pc+1,pb)); break;
    case 0x39 : sprintf(buf, "AND $%04X,Y", mem_getword(pc+1,pb)); break;
    case 0x2D : sprintf(buf, "AND $%04X", mem_getword(pc+1,pb)); break;
    case 0x2F : sprintf(buf, "AND $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x3F : sprintf(buf, "AND $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x09 :
if (flags&P_M)  sprintf(buf, "ORA #%02X", mem_getbyte(pc+1,pb));
           else sprintf(buf, "ORA #%04X", mem_getword(pc+1,pb)); break;
    case 0x01 : sprintf(buf, "ORA ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0x13 : sprintf(buf, "ORA ($%02X,S),Y", mem_getbyte(pc+1,pb)); break;
    case 0x07 : sprintf(buf, "ORA [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0x17 : sprintf(buf, "ORA [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0x03 : sprintf(buf, "ORA $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0x05 : sprintf(buf, "ORA $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x15 : sprintf(buf, "ORA $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0x1D : sprintf(buf, "ORA $%04X,X", mem_getword(pc+1,pb)); break;
    case 0x19 : sprintf(buf, "ORA $%04X,Y", mem_getword(pc+1,pb)); break;
    case 0x0D : sprintf(buf, "ORA $%04X", mem_getword(pc+1,pb)); break;
    case 0x0F : sprintf(buf, "ORA $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x1F : sprintf(buf, "ORA $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x49 :
if (flags&P_M)  sprintf(buf, "EOR #%02X", mem_getbyte(pc+1,pb));
           else sprintf(buf, "EOR #%04X", mem_getword(pc+1,pb)); break;
    case 0x41 : sprintf(buf, "EOR ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0x53 : sprintf(buf, "EOR ($%02X,S),Y", mem_getbyte(pc+1,pb)); break;
    case 0x45 : sprintf(buf, "EOR $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x47 : sprintf(buf, "EOR [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0x43 : sprintf(buf, "EOR $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0x57 : sprintf(buf, "EOR [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0x55 : sprintf(buf, "EOR $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0x4D : sprintf(buf, "EOR $%04X", mem_getbyte(pc+1,pb)); break;
    case 0x5D : sprintf(buf, "EOR $%04X,X", mem_getbyte(pc+1,pb)); break;
    case 0x59 : sprintf(buf, "EOR $%04X,Y", mem_getbyte(pc+1,pb)); break;
    case 0x5F : sprintf(buf, "EOR $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x06 : sprintf(buf, "ASL $%02X", mem_getbyte(pc+1, pb)); break;
    case 0x16 : sprintf(buf, "ASL $%02X,X", mem_getbyte(pc+1, pb)); break;
    case 0x0E : sprintf(buf, "ASL $%04X", mem_getword(pc+1, pb)); break;
    case 0x1E : sprintf(buf, "ASL $%04X,X", mem_getword(pc+1, pb)); break;
    case 0x0A : sprintf(buf, "ASL A"); break;
    case 0x46 : sprintf(buf, "LSR $%02X", mem_getbyte(pc+1, pb)); break;
    case 0x56 : sprintf(buf, "LSR $%02X,X", mem_getbyte(pc+1, pb)); break;
    case 0x4E : sprintf(buf, "LSR $%04X", mem_getword(pc+1, pb)); break;
    case 0x5E : sprintf(buf, "LSR $%04X,X", mem_getword(pc+1, pb)); break;
    case 0x4A : sprintf(buf, "LSR A"); break;
    case 0xE6 : sprintf(buf, "INC $%02X", mem_getbyte(pc+1, pb)); break;
    case 0xF6 : sprintf(buf, "INC $%02X,X", mem_getbyte(pc+1, pb)); break;
    case 0xEE : sprintf(buf, "INC $%04X", mem_getword(pc+1, pb)); break;
    case 0xFE : sprintf(buf, "INC $%04X,X", mem_getword(pc+1, pb)); break;
    case 0x1A : sprintf(buf, "INC A"); break;
    case 0xE8 : sprintf(buf, "INX"); break;
    case 0xC8 : sprintf(buf, "INY"); break;
    case 0xC6 : sprintf(buf, "DEC $%02X", mem_getbyte(pc+1, pb)); break;
    case 0xD6 : sprintf(buf, "DEC $%02X,X", mem_getbyte(pc+1, pb)); break;
    case 0xCE : sprintf(buf, "DEC $%04X", mem_getword(pc+1, pb)); break;
    case 0xDE : sprintf(buf, "DEC $%04X,X", mem_getword(pc+1, pb)); break;
    case 0x3A : sprintf(buf, "DEC A"); break;
    case 0xCA : sprintf(buf, "DEX"); break;
    case 0x88 : sprintf(buf, "DEY"); break;
    case 0x89 :
if (flags&P_M)  sprintf(buf, "BIT #%02X", mem_getbyte(pc+1,pb));
           else sprintf(buf, "BIT #%04X", mem_getword(pc+1,pb)); break;
    case 0x24 : sprintf(buf, "BIT $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x34 : sprintf(buf, "BIT $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0x2C : sprintf(buf, "BIT $%04X", mem_getword(pc+1,pb)); break;
    case 0x3C : sprintf(buf, "BIT $%04X,X", mem_getword(pc+1,pb)); break;
    case 0x26 : sprintf(buf, "ROL $%02X", mem_getbyte(pc+1, pb)); break;
    case 0x36 : sprintf(buf, "ROL $%02X,X", mem_getbyte(pc+1, pb)); break;
    case 0x2E : sprintf(buf, "ROL $%04X", mem_getword(pc+1, pb)); break;
    case 0x3E : sprintf(buf, "ROL $%04X,X", mem_getword(pc+1, pb)); break;
    case 0x2A : sprintf(buf, "ROL A"); break;
    case 0x66 : sprintf(buf, "ROR $%02X", mem_getbyte(pc+1, pb)); break;
    case 0x76 : sprintf(buf, "ROR $%02X,X", mem_getbyte(pc+1, pb)); break;
    case 0x6E : sprintf(buf, "ROR $%04X", mem_getword(pc+1, pb)); break;
    case 0x7E : sprintf(buf, "ROR $%04X,X", mem_getword(pc+1, pb)); break;
    case 0x6A : sprintf(buf, "ROR A"); break;
    case 0x64 : sprintf(buf, "STZ $%02X", mem_getbyte(pc+1, pb)); break;
    case 0x74 : sprintf(buf, "STZ $%02X,X", mem_getbyte(pc+1, pb)); break;
    case 0x9C : sprintf(buf, "STZ $%04X", mem_getword(pc+1, pb)); break;
    case 0x9E : sprintf(buf, "STZ $%04X,X", mem_getword(pc+1, pb)); break;
    case 0x69 :
if (flags&P_M)  sprintf(buf, "ADC #%02X", mem_getbyte(pc+1,pb));
           else sprintf(buf, "ADC #%04X", mem_getword(pc+1,pb)); break;
    case 0x61 : sprintf(buf, "ADC ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0x71 : sprintf(buf, "ADC ($%02X),Y", mem_getbyte(pc+1,pb)); break;
    case 0x72 : sprintf(buf, "ADC ($%02X)", mem_getbyte(pc+1,pb)); break;
    case 0x73 : sprintf(buf, "ADC ($%02X,S),Y", mem_getbyte(pc+1,pb)); break;
    case 0x67 : sprintf(buf, "ADC [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0x77 : sprintf(buf, "ADC [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0x63 : sprintf(buf, "ADC $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0x65 : sprintf(buf, "ADC $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x75 : sprintf(buf, "ADC $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0x7D : sprintf(buf, "ADC $%04X,X", mem_getword(pc+1,pb)); break;
    case 0x79 : sprintf(buf, "ADC $%04X,Y", mem_getword(pc+1,pb)); break;
    case 0x6D : sprintf(buf, "ADC $%04X", mem_getword(pc+1,pb)); break;
    case 0x6F : sprintf(buf, "ADC $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x7F : sprintf(buf, "ADC $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0xE9 :
if (flags&P_M)  sprintf(buf, "SBC #%02X", mem_getbyte(pc+1,pb));
           else sprintf(buf, "SBC #%04X", mem_getword(pc+1,pb)); break;
    case 0xE1 : sprintf(buf, "SBC ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0xF1 : sprintf(buf, "SBC ($%02X),Y", mem_getbyte(pc+1,pb)); break;
    case 0xF2 : sprintf(buf, "SBC ($%02X)", mem_getbyte(pc+1,pb)); break;
    case 0xF3 : sprintf(buf, "SBC ($%02X,S),Y", mem_getbyte(pc+1,pb)); break;
    case 0xE7 : sprintf(buf, "SBC [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0xF7 : sprintf(buf, "SBC [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0xE3 : sprintf(buf, "SBC $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0xE5 : sprintf(buf, "SBC $%02X", mem_getbyte(pc+1,pb)); break;
    case 0xF5 : sprintf(buf, "SBC $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0xFD : sprintf(buf, "SBC $%04X,X", mem_getword(pc+1,pb)); break;
    case 0xF9 : sprintf(buf, "SBC $%04X,Y", mem_getword(pc+1,pb)); break;
    case 0xED : sprintf(buf, "SBC $%04X", mem_getword(pc+1,pb)); break;
    case 0xEF : sprintf(buf, "SBC $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0xFF : sprintf(buf, "SBC $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x81 : sprintf(buf, "STA ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0x91 : sprintf(buf, "STA ($%02X),Y", mem_getbyte(pc+1,pb)); break;
    case 0x92 : sprintf(buf, "STA ($%02X)", mem_getbyte(pc+1,pb)); break;
    case 0x87 : sprintf(buf, "STA [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0x97 : sprintf(buf, "STA [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0x83 : sprintf(buf, "STA $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0x85 : sprintf(buf, "STA $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x95 : sprintf(buf, "STA $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0x9D : sprintf(buf, "STA $%04X,X", mem_getword(pc+1,pb)); break;
    case 0x99 : sprintf(buf, "STA $%04X,Y", mem_getword(pc+1,pb)); break;
    case 0x8D : sprintf(buf, "STA $%04X", mem_getword(pc+1,pb)); break;
    case 0x8F : sprintf(buf, "STA $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x9F : sprintf(buf, "STA $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x84 : sprintf(buf, "STY $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x94 : sprintf(buf, "STY $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0x8C : sprintf(buf, "STY $%04X", mem_getword(pc+1,pb)); break;
    case 0x86 : sprintf(buf, "STX $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x96 : sprintf(buf, "STX $%02X,Y", mem_getbyte(pc+1,pb)); break;
    case 0x8E : sprintf(buf, "STX $%04X", mem_getword(pc+1,pb)); break;
    case 0xA9 :
if (flags&P_M)  sprintf(buf, "LDA #%02X", mem_getbyte(pc+1,pb));
           else sprintf(buf, "LDA #%04X", mem_getword(pc+1,pb)); break;
    case 0xA1 : sprintf(buf, "LDA ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0xB1 : sprintf(buf, "LDA ($%02X),Y", mem_getbyte(pc+1,pb)); break;
    case 0xB2 : sprintf(buf, "LDA ($%02X)", mem_getbyte(pc+1,pb)); break;
    case 0xA7 : sprintf(buf, "LDA [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0xB7 : sprintf(buf, "LDA [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0xA3 : sprintf(buf, "LDA $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0xA5 : sprintf(buf, "LDA $%02X", mem_getbyte(pc+1,pb)); break;
    case 0xB5 : sprintf(buf, "LDA $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0xBD : sprintf(buf, "LDA $%04X,X", mem_getword(pc+1,pb)); break;
    case 0xB9 : sprintf(buf, "LDA $%04X,Y", mem_getword(pc+1,pb)); break;
    case 0xAD : sprintf(buf, "LDA $%04X", mem_getword(pc+1,pb)); break;
    case 0xAF : sprintf(buf, "LDA $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0xBF : sprintf(buf, "LDA $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0xA0 :
if (flags&P_X)  sprintf(buf, "LDY #%02X", mem_getbyte(pc+1,pb));
           else sprintf(buf, "LDY #%04X", mem_getword(pc+1,pb)); break;
    case 0xA4 : sprintf(buf, "LDY $%02X", mem_getbyte(pc+1,pb)); break;
    case 0xB4 : sprintf(buf, "LDY $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0xAC : sprintf(buf, "LDY $%04X", mem_getword(pc+1,pb)); break;
    case 0xBC : sprintf(buf, "LDY $%04X,X", mem_getword(pc+1,pb)); break;
    case 0xA2 :
if (flags&P_X)  sprintf(buf, "LDX #%02X", mem_getbyte(pc+1,pb));
           else sprintf(buf, "LDX #%04X", mem_getword(pc+1,pb)); break;
    case 0xA6 : sprintf(buf, "LDX $%02X", mem_getbyte(pc+1,pb)); break;
    case 0xB6 : sprintf(buf, "LDX $%02X,Y", mem_getbyte(pc+1,pb)); break;
    case 0xAE : sprintf(buf, "LDX $%04X", mem_getword(pc+1,pb)); break;
    case 0xC9 :
if (flags&P_M)  sprintf(buf, "CMP #%02X", mem_getbyte(pc+1,pb));
           else sprintf(buf, "CMP #%04X", mem_getword(pc+1,pb)); break;
    case 0xC1 : sprintf(buf, "CMP ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0xD1 : sprintf(buf, "CMP ($%02X),Y", mem_getbyte(pc+1,pb)); break;
    case 0xD2 : sprintf(buf, "CMP ($%02X)", mem_getbyte(pc+1,pb)); break;
    case 0xC7 : sprintf(buf, "CMP [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0xD7 : sprintf(buf, "CMP [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0xC3 : sprintf(buf, "CMP $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0xC5 : sprintf(buf, "CMP $%02X", mem_getbyte(pc+1,pb)); break;
    case 0xD5 : sprintf(buf, "CMP $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0xDD : sprintf(buf, "CMP $%04X,X", mem_getword(pc+1,pb)); break;
    case 0xD9 : sprintf(buf, "CMP $%04X,Y", mem_getword(pc+1,pb)); break;
    case 0xCD : sprintf(buf, "CMP $%04X", mem_getword(pc+1,pb)); break;
    case 0xCF : sprintf(buf, "CMP $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0xDF : sprintf(buf, "CMP $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0xC0 :
if (flags&P_X)  sprintf(buf, "CPY #%02X", mem_getbyte(pc+1,pb));
           else sprintf(buf, "CPY #%04X", mem_getword(pc+1,pb)); break;
    case 0xC4 : sprintf(buf, "CPY $%02X", mem_getbyte(pc+1,pb)); break;
    case 0xCC : sprintf(buf, "CPY $%04X", mem_getword(pc+1,pb)); break;
    case 0xE0 :
if (flags&P_X)  sprintf(buf, "CPX #%02X", mem_getbyte(pc+1,pb));
           else sprintf(buf, "CPX #%04X", mem_getword(pc+1,pb)); break;
    case 0xE4 : sprintf(buf, "CPX $%02X", mem_getbyte(pc+1,pb)); break;
    case 0xEC : sprintf(buf, "CPX $%04X", mem_getword(pc+1,pb)); break;
    case 0xC2 : sprintf(buf, "REP #%02X", mem_getbyte(pc+1,pb)); break;
    case 0xE2 : sprintf(buf, "SEP #%02X", mem_getbyte(pc+1,pb)); break;
    case 0xD0 : sprintf(buf, "BNE %04X", pc+1+(char)mem_getbyte(pc+1, pb)+1); break;
    case 0xF0 : sprintf(buf, "BEQ %04X", pc+1+(char)mem_getbyte(pc+1, pb)+1); break;
    case 0x10 : sprintf(buf, "BPL %04X", pc+1+(char)mem_getbyte(pc+1, pb)+1); break;
    case 0x30 : sprintf(buf, "BMI %04X", pc+1+(char)mem_getbyte(pc+1, pb)+1); break;
    case 0x90 : sprintf(buf, "BCC %04X", pc+1+(char)mem_getbyte(pc+1, pb)+1); break;
    case 0xB0 : sprintf(buf, "BCS %04X", pc+1+(char)mem_getbyte(pc+1, pb)+1); break;
    case 0x80 : sprintf(buf, "BRA %04X", pc+1+(char)mem_getbyte(pc+1, pb)+1); break;
    case 0x70 : sprintf(buf, "BVS %04X", pc+1+(char)mem_getbyte(pc+1, pb)+1); break;
    case 0x82 : sprintf(buf, "BRL %04X", pc+1+(short)mem_getword(pc+1, pb)+2); break;
    case 0x08 : sprintf(buf, "PHP"); break;
    case 0x48 : sprintf(buf, "PHA"); break;
    case 0xDA : sprintf(buf, "PHX"); break;
    case 0x5A : sprintf(buf, "PHY"); break;
    case 0x4B : sprintf(buf, "PHK"); break;
    case 0x68 : sprintf(buf, "PLA"); break;
    case 0xFA : sprintf(buf, "PLX"); break;
    case 0x7A : sprintf(buf, "PLY"); break;
    case 0xAB : sprintf(buf, "PLB"); break;
    case 0x28 : sprintf(buf, "PLP"); break;
    case 0xEB : sprintf(buf, "XBA"); break;
    case 0x22 : sprintf(buf, "JSL $%02X:%04X", mem_getbyte(pc+1+2, pb), mem_getword(pc+1, pb)); break;
    case 0x20 : sprintf(buf, "JSR $%04X", mem_getword(pc+1, pb)); break;
    case 0x40 : sprintf(buf, "RTI"); break;
    case 0x60 : sprintf(buf, "RTS"); break;
    case 0x6B : sprintf(buf, "RTL"); break;
    case 0x7B : sprintf(buf, "TCD"); break;
    case 0x98 : sprintf(buf, "TYA"); break;
    case 0x9A : sprintf(buf, "TXS"); break;
    case 0xA8 : sprintf(buf, "TAY"); break;
    case 0xAA : sprintf(buf, "TAX"); break;
    case 0xBA : sprintf(buf, "TSX"); break;
    case 0x8A : sprintf(buf, "TXA"); break;
    case 0x38 : sprintf(buf, "SEC"); break;
    case 0x78 : sprintf(buf, "SEI"); break;
    case 0x18 : sprintf(buf, "CLC"); break;
    case 0x58 : sprintf(buf, "CLI"); break;
    case 0xB8 : sprintf(buf, "CLV"); break;
    case 0xFB : sprintf(buf, "XCE"); break;
    case 0x8B : sprintf(buf, "PHB"); break;
    case 0x0B : sprintf(buf, "PHD"); break;
    case 0x2B : sprintf(buf, "PLD"); break;
    case 0x1B : sprintf(buf, "TCS"); break;
    case 0x3B : sprintf(buf, "TSC"); break;
    case 0x5B : sprintf(buf, "TCD"); break;
    case 0xF4 : sprintf(buf, "PEA #%04X", mem_getword(pc+1, pb)); break;
    case 0xD4 : sprintf(buf, "PEI $%02X", mem_getbyte(pc+1, pb)); break;
    case 0x4C : sprintf(buf, "JMP $%04X", mem_getword(pc+1, pb)); break;
    case 0x5C : sprintf(buf, "JMP $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1, pb)); break;
    case 0x6C : sprintf(buf, "JMP ($%04X)", mem_getword(pc+1, pb)); break;
    case 0x7C : sprintf(buf, "JMP ($%04X,X)", mem_getword(pc+1, pb)); break;
    case 0xDC : sprintf(buf, "JML (%X)", mem_getword(pc+1, pb)); break;
    case 0xFC : sprintf(buf, "JSR ($%04X,X)", mem_getword(pc+1, pb)); break;
    case 0x54 : sprintf(buf, "MVN %X:%X->%X:%X %x", mem_getbyte(pc+1+1, pb),
                          CPU.X, mem_getbyte(pc+1, pb), CPU.Y, CPU.A); break;
    case 0x44 : sprintf(buf, "MVP %X:%X->%X:%X %x", mem_getbyte(pc+1+1, pb),
                          CPU.X, mem_getbyte(pc+1, pb), CPU.Y, CPU.A); break;
    case 0x00 : sprintf(buf, "BRK $%02X", mem_getbyte(pc+1, pb)); break;
	case 0x50 : sprintf(buf, "BVC %04X", pc+1+(char)mem_getbyte(pc+1, pb)+1); break;
    case 0xCB : sprintf(buf, "WAI"); break;
    default :   sprintf(buf, "\?\?(%02X)",opcode);
  }
}


int trace_CPU()
{
//  unsigned char opcode;
  char	buf[64];
  char	buf2[256];

//  opcode = mem_getbyte(OldPC, PB);

  CPU.PC = CPU.LastAddress;
  CPU.A = A;
  CPU.X = X;
  CPU.Y = Y;
//  CPU.Cycles = Cycles;
  CPU.S = S;
  CPU.P = P;
  CPU.D = D;
  CPU.DB = DB;
  CPU.PB = PB;

#if 1
  show_opcode(buf, mem_getbyte(CPU.PC, CPU.PB), CPU.PC, CPU.PB, P);
  sprintf(buf2,
          "A:%04X X:%04X Y:%04X S:%04X D:%02X/%04X VC:%03d ?:%02d %d%d%d%d%d%d%d%d %02X:%04X %s\n",
          CPU.A, CPU.X, CPU.Y, CPU.S, CPU.DB, CPU.D, SNES.V_Count, Cycles,
          (P>>7)&1,(P>>6)&1,(P>>5)&1,(P>>4)&1,(P/8)&1,(P/4)&1,(P/2)&1,P&1,
          CPU.PB, CPU.PC, buf);
#else
  sprintf(buf2,"%02X:%04X ; ", CPU.PB, CPU.PC);
#endif          

	FS_printlog(buf2);


  return 0;
}