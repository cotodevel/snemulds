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

#ifndef __main_h__
#define __main_h__

#include "dsregs.h"
#include "typedefsTGDS.h"
#include "opcodes.h"
#include "fs.h"
#include "gfx.h"
#include "apu.h"
#include "snes.h"
#include "cfg.h"
#include "core.h"
#include "ppu.h"
#include "conf.h"
#include "engine.h"
#include "memmap.h"
#include "guiTGDS.h"
#include "ipcfifoTGDSUser.h"
#include "nds_cp15_misc.h"
#include "fatfslayerTGDS.h"
#include "about.h"
#include "utilsTGDS.h"
#include "clockTGDS.h"
#include "dswnifi_lib.h"
#include "gui_console_connector.h"

static inline int go()
{
	if ((CPU.IsBreak == true) || (SNES.Stopped == true)){
		return 0;
	}
	else if(waitforhblank == true){
		IRQWait(IRQ_HBLANK);
		waitforhblank = false;
		return 0;
	}
	
  while (1)
  {	
	if (GFX.v_blank)
	{
    	if (!CFG.WaitVBlank && GFX.need_update)
    	{
			draw_screen();
			GFX.need_update = 0;
    	}
		GFX.v_blank = 0;
		//update_joypads();
		return 0;
	}
	
	
	struct s_apu2 *APU2 = (struct s_apu2 *)(&IPC6->APU2);
	CPU.HCycles = SNES.UsedCycles;	
    if (DMA_PORT[0x00]&0x10) 
    {
      if (!(DMA_PORT[0x00]&0x20) ||
           SNES.V_Count == (DMA_PORT[0x09]&0xff)+((DMA_PORT[0x0A]&0xff)<<8)) 
      {
	    int H_cycles;
	    H_cycles = (DMA_PORT[0x07]+(DMA_PORT[0x08]<<8))/2;
		SET_WAITCYCLES(H_cycles);
		CPU.WaitAddress = -1;

		CPU_goto(H_cycles); 
		if (CPU.IsBreak) 
			return 0;
		CPU.HCycles += H_cycles;	
        GoIRQ();
        DMA_PORT[0x11] = 0x80;
      }
    } 

	SET_WAITCYCLES(NB_CYCLES-CPU.HCycles);
	CPU.WaitAddress = -1; 
    CPU_goto(NB_CYCLES-CPU.HCycles); 
    if (CPU.IsBreak) 
    	return 0;
    CPU.HCycles = NB_CYCLES;

    SNES.UsedCycles = 0;
	
	//HBLANK Starts here
	SNES.V_Count++;
	
	//Sync emulator frame count in host/guest if frame desync happens
	if(getMULTIMode() == dswifi_localnifimode){
		if(ThisSNESFrameCount != guestSNESFrameCount){
			//lowest frame takes precedence
			if(ThisSNESFrameCount > guestSNESFrameCount){
				ThisSNESFrameCount = guestSNESFrameCount;
			}
			return 0;
		}
	}
	
	if (SNES.V_Count > (SNES.NTSC ? 261 : 311)){
		if(ThisSNESFrameCount > 59){
			ThisSNESFrameCount = 0;
		}
		else{
			ThisSNESFrameCount++;
		}	
		SNES.V_Count = 0;
	}
#if 0
    if (CFG.Sound_output) {
      if (SNES.V_Count & 63) {
        if (SNES.V_Count & 1) {
          if (!SNES.NTSC && ((SNES.V_Count&7) == 1)) {
            APU2->T0++; APU2->T1++; APU2->T2+=4;
          }
          if (++APU2->T0 >= APU2->T0) {
            APU2->T0 -= APU2->T0; APU2->CNT0++;
            //if (APU2->CONTROL&1) { SPC700_emu = 1; APU_WaitCounter++; }
          }
          if (++APU2->T1 >= APU2->T1) {
            APU2->T1 -= APU2->T1; APU2->CNT1++;
            //if (APU2->CONTROL&2) { SPC700_emu = 1; APU_WaitCounter++; }
          }
        }
        APU2->T2 += 4;
        if (APU2->T2 >= APU2->T2) {
          APU2->T2 -= APU2->T2; APU2->CNT2++;
          //if (APU.CONTROL&4) { SPC700_emu = 1; APU_WaitCounter++; }
        }
      }
    }
#endif
    if (SNES.V_Count < GFX.ScreenHeight) 
    {   	
#if 0    	
      if (GFX.Sprites_table_dirty) {
/*        if ((CFG.BG_Layer&0x10))
          draw_sprites();*/

//draw_screen();    
        GFX.Sprites_table_dirty = 0;
      }
#endif      
      if (!(PPU_PORT[0x00]&0x80) && DMA_PORT[0x0C] && CFG.BG_Layer&0x80)
        HDMA_write();
      SNES.HDMA_line++;

      // draw line
      if (CFG.Scaled)
    	  PPU_line_render_scaled();
      else
    	  PPU_line_render();
    }
    if ((DMA_PORT[0x00]&0x20) && !(DMA_PORT[0x00]&0x10) &&
      SNES.V_Count == (DMA_PORT[0x09]&0xff)+((DMA_PORT[0x0A]&0xff)<<8))
      GoIRQ();
    if (SNES.V_Count == (SNES.NTSC ? 261 : 311)) {
    	
      // V_blank
      SNES.v_blank = 0; DMA_PORT[0x10] = 0;
      if (DMA_PORT[0x0C] && CFG.BG_Layer&0x80) {
        if (DMA_PORT[0x0C]&0x01) HDMA_transfert(0);
        if (DMA_PORT[0x0C]&0x02) HDMA_transfert(1);
        if (DMA_PORT[0x0C]&0x04) HDMA_transfert(2);
        if (DMA_PORT[0x0C]&0x08) HDMA_transfert(3);
        if (DMA_PORT[0x0C]&0x10) HDMA_transfert(4);
        if (DMA_PORT[0x0C]&0x20) HDMA_transfert(5);
        if (DMA_PORT[0x0C]&0x40) HDMA_transfert(6);
        if (DMA_PORT[0x0C]&0x80) HDMA_transfert(7);
      }
//      memset(GFX.tiles_ry,8,4);
/*      if (GFX.frame_d && (CFG.BG_Layer & 0x10))
        draw_sprites();*/
      draw_screen();
      GFX.BG_scroll_reg = 0;
      SNES.HDMA_line = 0;
      PPU_PORT[0x02] = GFX.Old_SpriteAddress;
      GFX.OAM_upper_byte = 0;
//      patch_memory();
	  GFX.brightness = PPU_PORT[0x00]&0xF;
    }
    if (SNES.V_Count == GFX.ScreenHeight || SNES.DelayedNMI) {
      if (DMA_PORT[0x00]&0x80) GoNMI();
      SNES.HIRQ_ok = 1;
//      GFX.was_not_blanked = 0; 
      GFX.nb_frames++;
      
      if (GFX.nb_frames >= 100 && !GUI.log && !CFG.Debug)
      {
    	  GFX.speed = GFX.nb_frames * 100 / (GFX.DSFrame - GFX.DSLastFrame);
    	  GFX.DSLastFrame = GFX.DSFrame;  
    	  //GUI_console_printf(0, 23, "% 3d%%             ",  GFX.speed);
//        GUI_console_printf(20, 23, "DBG=%d", CFG.Debug2),

		  //GUI_console_printf(26, 23, "%02d:%02d",
			  //(getTime()->tm_hour >= 40 ? getTime()->tm_hour - 40 : getTime()->tm_hour),
			  //getTime()->tm_min);		        	

          GFX.nb_frames = 0;
          
          if (CFG.EnableSRAM && SNES.SRAMWritten)
          {
        	  saveSRAM();
        	  printf("SRAM written");
        	  SNES.SRAMWritten = 0;
          }
      }
      
            
      if (!CPU.NMIActive) DMA_PORT[0x10] |= 0x80;
      SNES.v_blank = 1; CPU.NMIActive = 0;
      update_joypads();
      SNES.DelayedNMI = 0;
	}
   }

	return 0;
}

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int argc;
extern sint8 **argv;
extern int main(int _argc, sint8 **_argv);
extern int loadROM(struct sGUISelectorItem *name);
extern bool handleROMSelect;
extern bool handleSPCSelect;

#ifdef __cplusplus
}
#endif