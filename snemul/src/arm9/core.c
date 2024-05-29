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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "opcodes.h"
//#include "snemul.h"

#ifdef WIN32
#include <allegro.h>
#endif


#include "cpu.h"
#include "apu.h"
#include "snes.h"
#include "gfx.h"
#include "cfg.h"

//#include "superfx.h"
//#include "sfxinst.h"

uchar   mem_getbyte(uint32 offset, uchar bank);
void	mem_setbyte(uint32 offset, uchar bank, uchar byte);
ushort  mem_getword(uint32 offset, uchar bank);
void    mem_setword(uint32 offset, uchar bank, ushort word);



int	SPC700_emu;

void	PPU_port_write(uint32 address, uint8 value);
uchar	PPU_port_read(uint32 address);


// A OPTIMISER
int	PPU_fastDMA_2118_1(int offs, int bank, int len)
{
	int i;
	uint8	*ptr;

	ptr = map_memory(offs, bank);

	if (PPU_PORT[0x15]&0x80) {
		if (!GFX.FS_incr && GFX.SC_incr == 1)
		{
			// Very fast DMA mode 1!!!!
//			fprintf(SNES.flog,"Very fast!");
			memcpy(SNESC.VRAM+((PPU_PORT[0x16]<<1)&0xFFFF), ptr, len);
			for (i = 0; i < len; i += 2)
			{
				if ((i & 15) == 0) 
					check_tile();
				PPU_PORT[0x16]++;
			}
			return offs+len;
		}
		for (i = 0; i < len; i+=2)
		{
//			if ((i & 15) == 0) 
				check_tile();			
			SNESC.VRAM[(PPU_PORT[0x16]<<1)&0xFFFF] = ptr[i];   
			SNESC.VRAM[((PPU_PORT[0x16]<<1)+1)&0xFFFF] = ptr[i+1];
			if (!GFX.FS_incr) {
				PPU_PORT[0x16] += GFX.SC_incr;
			} else {
				PPU_PORT[0x16] += 8;
				if (++GFX.FS_cnt == GFX.FS_incr) {
					GFX.FS_cnt = 0;
					if (++GFX.FS_cnt2 == 8) {
						GFX.FS_cnt2 = 0; PPU_PORT[0x16] -= 8-GFX.SC_incr;
					}
					else
						PPU_PORT[0x16] -= 8*GFX.FS_incr-GFX.SC_incr;
				}
			}
		}
	}
	else
	{
		for (i = 0; i < len; i+=2)
		{
//			if ((i & 15) == 0) 
				check_tile();
			SNESC.VRAM[(PPU_PORT[0x16]<<1)&0xFFFF] = ptr[i];
			PPU_PORT[0x16] += GFX.SC_incr;
			if (GFX.FS_incr) {
				PPU_PORT[0x16] += 8;
				if (++GFX.FS_cnt == GFX.FS_incr) {
					GFX.FS_cnt = 0;
					if (++GFX.FS_cnt2 == 8) {
						GFX.FS_cnt2 = 0; PPU_PORT[0x16] -= 7;
					} else
						PPU_PORT[0x16] -= 8*GFX.FS_incr-1;
				}
			}
			SNESC.VRAM[((PPU_PORT[0x16]<<1)+1)&0xFFFF] = ptr[i+1];
		} 
	}
	return offs+len;
}

void DMA_transfert(uchar port)
{
  uint		tmp;
  ushort	PPU_port;
  ushort	DMA_address;
  uint		DMA_len;
  uchar		DMA_bank, DMA_info;

  START_PROFILE(DMA, 4);
  DMA_address = DMA_PORT[0x102+port*0x10]+(DMA_PORT[0x103+port*0x10]<<8);
  DMA_bank = DMA_PORT[0x104+port*0x10];
  DMA_len = DMA_PORT[0x105+port*0x10]+(DMA_PORT[0x106+port*0x10]<<8);
  if (DMA_len == 0)
    DMA_len = 0x10000;
  PPU_port = 0x2100+DMA_PORT[0x101+port*0x10];
  DMA_info = DMA_PORT[0x100+port*0x10];

/*   FS_flog("DMA[%d] %06X->%04X SIZE:%05X VRAM : %04X\n", port,
      DMA_address+(DMA_bank<<16), PPU_port, DMA_len, PPU_PORT[0x16]);*/

  ADD_CYCLES (DMA_len % NB_CYCLES);
  if (DMA_len >= NB_CYCLES)
  {
  	  if (SNES.V_Count < GFX.ScreenHeight && 
  	  	  SNES.V_Count+(DMA_len / NB_CYCLES) >= GFX.ScreenHeight)
  	  	SNES.DelayedNMI = 1;
	  SNES.V_Count += (DMA_len / NB_CYCLES);
	   
  }  

  if (CFG.FastDMA && PPU_port == 0x2118 && DMA_info == 1)
  {
	  DMA_address = PPU_fastDMA_2118_1(DMA_address, DMA_bank, DMA_len);
  }
  else
  {
  if ((DMA_info&0x80) == 0) {
    for (tmp = 0;tmp < DMA_len;tmp++) {
      switch (DMA_info&7) {
        case 0x00 :
          PPU_port_write(PPU_port,mem_getbyte(DMA_address,DMA_bank)); break;
        case 0x01 :
          PPU_port_write(PPU_port+(tmp&1),mem_getbyte(DMA_address,DMA_bank)); break;
        case 0x02 :
          PPU_port_write(PPU_port,mem_getbyte(DMA_address,DMA_bank)); break;
        case 0x03 :
          PPU_port_write(PPU_port+(tmp&2)/2,mem_getbyte(DMA_address,DMA_bank)); break;
        case 0x04 :
          PPU_port_write(PPU_port+(tmp&3),mem_getbyte(DMA_address,DMA_bank)); break;
      }
      if (!(DMA_info & 0x08)) {
        if (DMA_info & 0x10) DMA_address--; else DMA_address++;
      }
    }
  } else {
    for (tmp = 0;tmp<DMA_len;tmp++) {
      switch (DMA_info & 7) {
        case 0x00 :
          mem_setbyte(DMA_address,DMA_bank,PPU_port_read(PPU_port)); break;
        case 0x01 :
          mem_setbyte(DMA_address,DMA_bank,PPU_port_read(PPU_port+(tmp&1))); break;
        case 0x02 :
          mem_setbyte(DMA_address,DMA_bank,PPU_port_read(PPU_port)); break;
        case 0x03 :
          mem_setbyte(DMA_address,DMA_bank,PPU_port_read(PPU_port+(tmp&2)/2)); break;
        case 0x04 :
          mem_setbyte(DMA_address,DMA_bank,PPU_port_read(PPU_port+(tmp&3))); break;
      }
      if (!(DMA_info & 0x08)) {
        if (DMA_info & 0x10) DMA_address--; else DMA_address++;
      }
    }
  }
  }
  DMA_PORT[0x106+port*0x10] = DMA_PORT[0x105+port*0x10] = 0;
  DMA_PORT[0x102+port*0x10] = DMA_address&0xff;
  DMA_PORT[0x103+port*0x10] = DMA_address>>8;
  END_PROFILE(DMA, 4);
}


void		HDMA_transfert(unsigned char port)
{
  uint		len;
  uchar		*ptr, *ptr2, repeat;
  ushort	tmp=0;

  START_PROFILE(DMA, 4);
  SNES.HDMA_nblines[port] = 0;
  ptr = map_memory((DMA_PORT[0x102+port*0x10])+(DMA_PORT[0x103+port*0x10]<<8),
                    DMA_PORT[0x104+port*0x10]);

  if (!ptr) {
/*    iprintf(" (invalid memory access during a H-DMA transfert : %06X)",
      DMA_PORT[0x102+port*0x10]+(DMA_PORT[0x103+port*0x10]<<8)+
      (DMA_PORT[0x104+port*0x10]<<16));*/
      return;
//    exit(255);
  }

  SNES.HDMA_port[port] = DMA_PORT[0x101+port*0x10];
  SNES.HDMA_info[port] = DMA_PORT[0x100+port*0x10]&7;

  while(*ptr++ && tmp < GFX.ScreenHeight)
  {
    if (*(ptr-1) == 0x80) {
      len = MIN(128,GFX.ScreenHeight-tmp); repeat = 1;
    } else {
      len    = MIN(*(ptr-1)&0x7f,GFX.ScreenHeight-tmp);
      repeat = !(*(ptr-1)&0x80);
    }
    if (DMA_PORT[0x100+port*0x10]&0x40) {
      ptr2 = map_memory(*ptr+(*(ptr+1)<<8), DMA_PORT[0x107+port*0x10]);
      ptr += 2;
      switch (DMA_PORT[0x100+port*0x10]&7) {
        case 0x00 :
          while (len--) {
            SNES.HDMA_values[tmp++][port] = ptr2; if (!repeat) ptr2++;
          } break;
        case 0x01 :
          while (len--) {
            SNES.HDMA_values[tmp++][port] = ptr2; if (!repeat) ptr2 += 2;
          } break;
        case 0x02 :
          while (len--) {
            SNES.HDMA_values[tmp++][port] = ptr2; if (!repeat) ptr2 += 2;
          } break;
        case 0x03 :
        while (len--) {
            SNES.HDMA_values[tmp++][port] = ptr2; if (!repeat) ptr2 += 4;
          } break;
        case 0x04 :
          while (len--) {
            SNES.HDMA_values[tmp++][port] = ptr2; if (!repeat) ptr2 += 4;
          } break;
      }
      continue;
    }
    switch (DMA_PORT[0x100+port*0x10] & 7) {
      case 0x00 :
        while (len--) {
          SNES.HDMA_values[tmp++][port] = ptr; if (!repeat) ptr++;
        } if (repeat) ptr++; break;
      case 0x01 :
        while (len--) {
          SNES.HDMA_values[tmp++][port] = ptr; if (!repeat) ptr += 2;
        } if (repeat) ptr += 2; break;
      case 0x02 :
        while (len--) {
          SNES.HDMA_values[tmp++][port] = ptr; if (!repeat) ptr += 2;
        } if (repeat) ptr += 2; break;
      case 0x03 :
        while (len--) {
          SNES.HDMA_values[tmp++][port] = ptr; if (!repeat) ptr += 4;
        } if (repeat) ptr += 4; break;
      case 0x04 :
        while (len--) {
          SNES.HDMA_values[tmp++][port] = ptr; if (!repeat) ptr += 4;
        } if (repeat) ptr += 4; break;
    }
  }

  SNES.HDMA_nblines[port] = MIN(GFX.ScreenHeight, tmp);
  SNES.HDMA_line = 0;
  END_PROFILE(DMA, 4);
}

/* ============================ I/O registers ========================== */

IN_ITCM
uint32	IONOP_DMA_READ(uint32 addr)
{
	return (DMA_PORT[addr]);
}
IN_ITCM
uint32	IONOP_PPU_READ(uint32 addr)
{
	return (PPU_PORT[addr]);
}

IN_ITCM
void	IONOP_PPU_WRITE(uint32 addr, uint32 byte)
{
	PPU_PORT[addr] = byte;
}
IN_ITCM
void	IONOP_DMA_WRITE(uint32 addr, uint32 byte)
{
	DMA_PORT[addr] = byte;
}
IN_ITCM
void	W4016(uint32 addr, uint32 value)
{
    			 if ((value&1) && !(SNES.JOY_PORT16&1))
    			   {
                     SNES.Joy1_cnt = 0;
                   }
                 SNES.JOY_PORT16 = value;
}
IN_ITCM                 
void	W4017(uint32 addr, uint32 value)
{
}
IN_ITCM
void	W4200(uint32 addr, uint32 value)
{
	if (value & 0x10)
    	SNES.HIRQ_ok = 0;
	DMA_PORT[0x00] = value;
}
IN_ITCM
void	W4203(uint32 addr, uint32 value)
{
      DMA_PORT[0x16]=DMA_PORT[0x02]*value;
      DMA_PORT[0x17]=(DMA_PORT[0x16]>>8);
      DMA_PORT[0x03] = value;
}
IN_ITCM      
void	W4206(uint32 addr, uint32 value)
{
      if (value) {
        int tmp = (DMA_PORT[0x05]<<8)+DMA_PORT[0x04];
        DMA_PORT[0x14]=tmp/value;
        DMA_PORT[0x15]=DMA_PORT[0x14]>>8;
        DMA_PORT[0x16]=tmp%value;
        DMA_PORT[0x17]=DMA_PORT[0x16]>>8;
      } else { /* division par zero */
        DMA_PORT[0x14] = DMA_PORT[0x15] = 0xFF;
        DMA_PORT[0x16] = DMA_PORT[0x04];
        DMA_PORT[0x17] = DMA_PORT[0x05];
      }
 	  DMA_PORT[0x06] = value;
}
IN_ITCM
void	W4207(uint32 addr, uint32 value)
{
		SNES.HIRQ_value = (SNES.HIRQ_value&0xFF00) | value;
		DMA_PORT[0x07] = value;
}
IN_ITCM
void	W4208(uint32 addr, uint32 value)
{
		SNES.HIRQ_value = (SNES.HIRQ_value&0x00FF) | (value << 8);
		DMA_PORT[0x08] = value;
}
IN_ITCM
void	W420B(uint32 addr, uint32 value)
{
			     if (value & 0x01) DMA_transfert(0);
                 if (value & 0x02) DMA_transfert(1);
                 if (value & 0x04) DMA_transfert(2);
                 if (value & 0x08) DMA_transfert(3);
                 if (value & 0x10) DMA_transfert(4);
                 if (value & 0x20) DMA_transfert(5);
                 if (value & 0x40) DMA_transfert(6);
                 if (value & 0x80) DMA_transfert(7);
                 DMA_PORT[0x0B] = value;
}
IN_ITCM
void	W420C(uint32 addr, uint32 value)
{
				 if (value & 0x01) HDMA_transfert(0);
                 if (value & 0x02) HDMA_transfert(1);
                 if (value & 0x04) HDMA_transfert(2);
                 if (value & 0x08) HDMA_transfert(3);
                 if (value & 0x10) HDMA_transfert(4);
                 if (value & 0x20) HDMA_transfert(5);
                 if (value & 0x40) HDMA_transfert(6);
                 if (value & 0x80) HDMA_transfert(7);
                 DMA_PORT[0x0C] = value;
}                 
                 
IN_ITCM
uint32	R4016(uint32 addr)
{
         uchar tmp;

         if (SNES.JOY_PORT16&1)
         {
         	SNES.mouse_speed++;
         	if (SNES.mouse_speed == 3)
         		SNES.mouse_speed = 0;;
           return 0;
         }
         tmp = SNES.joypads[0]>>(SNES.Joy1_cnt^15);
         SNES.Joy1_cnt++;
         return (tmp&1);
}
IN_ITCM
uint32	R4017(uint32 addr)
{
      return 0x00;
}
IN_ITCM
uint32	R4210(uint32 addr)
{
//FIXME
  	  if (HCYCLES < NB_CYCLES-6 && SNES.V_Count == GFX.ScreenHeight-1)    
     // if (Cycles >= CPU.Cycles-6 && SNES.V_Count == GFX.ScreenHeight-1)    
      {
        CPU.NMIActive = 1; DMA_PORT[0x10] = 0; return 0x80;
      }
      SET_WAITCYCLESDELAY(0);
      if (SNES.V_Count == GFX.ScreenHeight-1) SET_WAITCYCLESDELAY(6);
      if (DMA_PORT[0x10]&0x80) {
        DMA_PORT[0x10] &= ~0x80; return 0x80;
      }
      return DMA_PORT[0x10];
}
IN_ITCM
uint32	R4211(uint32 addr)
{
      SET_WAITCYCLESDELAY(0);
      if (DMA_PORT[0x11] & 0x80) {
        DMA_PORT[0x11] &= ~0x80; return 0x80;
      }
      return DMA_PORT[0x11];
}
IN_ITCM
uint32	R4212(uint32 addr)
{
      SET_WAITCYCLESDELAY(0);
      if (HCYCLES < NB_CYCLES - 65) SET_WAITCYCLESDELAY(60);
      if (SNES.V_Count == GFX.ScreenHeight-1)
        SET_WAITCYCLESDELAY(6);
      DMA_PORT[0x12] =
        SNES.V_Count >= GFX.ScreenHeight && SNES.V_Count < GFX.ScreenHeight+3;
	  // FiXME
	  if (HCYCLES > 120)
        DMA_PORT[0x12] |= 0x40;
// FIXME            
	  if (SNES.v_blank || (HCYCLES < NB_CYCLES-6 && SNES.V_Count == GFX.ScreenHeight-1))
//      if (SNES.v_blank || (Cycles >= CPU.Cycles-6 && SNES.V_Count == GFX.ScreenHeight-1))     
          DMA_PORT[0x12] |= 0x80;
      return DMA_PORT[0x12];
}
IN_ITCM 	
uint32	R2121(uint32 addr) 
{  	
      return (PPU_PORT[0x21]>>1);
}      
/* 2134 - 2136 */
IN_ITCM
uint32	R213X(uint32 addr) 
{
      if (SNES.PPU_NeedMultiply) {
        long result = (long)((short)(PPU_PORT[0x1B])) *
                      (long)((short)(PPU_PORT[0x1C])>>8);
//        long result = ((long)PPU_PORT[0x1B]*(long)PPU_PORT[0x1C])>>8;
        PPU_PORT[0x34] = (result)&0xFF;
        PPU_PORT[0x35] = (result>>8)&0xFF;
        PPU_PORT[0x36] = (result>>16)&0xFF;
        SNES.PPU_NeedMultiply = 0;
      }
      return PPU_PORT[addr];
}
IN_ITCM
uint32	R2137(uint32 addr) 
{
      PPU_PORT[0x3C] = (HCYCLES)*9/5; // FIXME    	
      PPU_PORT[0x3C] = (PPU_PORT[0x3C]>>8) | (PPU_PORT[0x3C]<<8);
      PPU_PORT[0x3D] = SNES.V_Count;
      PPU_PORT[0x3D] = (PPU_PORT[0x3D]>>8) | (PPU_PORT[0x3D]<<8);
      return PPU_PORT[0x37];
}
IN_ITCM
uint32	R2138(uint32 addr) 
{
      if ((PPU_PORT[0x02]) >= 0x100) {
        if ((PPU_PORT[0x02]) < 0x110) {
          if (GFX.OAM_upper_byte) {
            GFX.OAM_upper_byte = 0;
            PPU_PORT[0x02]++;
            return GFX.spr_info_ext[((PPU_PORT[0x02]-1)<<1)+1-0x200];
          } else {
            GFX.OAM_upper_byte = 1;
            return GFX.spr_info_ext[(PPU_PORT[0x02]<<1)-0x200];
          }
        } else
          PPU_PORT[0x02] = 0;
      } else {
        if (GFX.OAM_upper_byte) {
          GFX.OAM_upper_byte = 0;
          PPU_PORT[0x02]++;
          return ((uchar *)GFX.spr_info)[((PPU_PORT[0x02]-1)<<1)+1];
        } else {
          GFX.OAM_upper_byte = 1;
          return ((uchar *)GFX.spr_info)[(PPU_PORT[0x02]<<1)];
        }
      }
      return PPU_PORT[0x38];
}
IN_ITCM
uint32	R2139(uint32 addr) 
{
         if (PPU_PORT[0x15]&0x80) {
           return SNESC.VRAM[(PPU_PORT[0x16]<<1)&0xFFFF];
         } else {
           long result = SNESC.VRAM[(PPU_PORT[0x16]<<1)&0xFFFF];
           if (GFX.Dummy_VRAMRead) {
             GFX.Dummy_VRAMRead = 0;
           } else {
             PPU_PORT[0x16] += GFX.SC_incr;
             if (GFX.FS_incr) {
                PPU_PORT[0x16] += 8;
               if (++GFX.FS_cnt == GFX.FS_incr) {
                 GFX.FS_cnt = 0;
                 if (++GFX.FS_cnt2 == 8) {
                   GFX.FS_cnt2 = 0; PPU_PORT[0x16] -= 8-GFX.SC_incr;
                 } else
                   PPU_PORT[0x16] -= 8*GFX.FS_incr-GFX.SC_incr;
               }
             }
           }
           return result;         
         }
}
IN_ITCM
uint32	R213A(uint32 addr) 
{
         if ((PPU_PORT[0x15]&0x80) == 0) {
           return SNESC.VRAM[((PPU_PORT[0x16]<<1)+1)&0xFFFF];
         } else {
           long result = SNESC.VRAM[((PPU_PORT[0x16]<<1)+1)&0xFFFF];
           if (GFX.Dummy_VRAMRead) {
             GFX.Dummy_VRAMRead = 0;
           } else {
             PPU_PORT[0x16] += GFX.SC_incr;
             if (GFX.FS_incr) {
               PPU_PORT[0x16]+=8;
               if (++GFX.FS_cnt == GFX.FS_incr) {
                 GFX.FS_cnt = 0;
                 if (++GFX.FS_cnt2 == 8) {
                   GFX.FS_cnt2 = 0; PPU_PORT[0x16]-=8-GFX.SC_incr;
                 }
                 else
                   PPU_PORT[0x16]-=8*GFX.FS_incr-GFX.SC_incr;
               }
             }
           }
           return result;
         }
}
IN_ITCM         
uint32	R213B(uint32 addr) 
{
         if (PPU_PORT[0x21] == 0x200) PPU_PORT[0x21]=0;
         if ((PPU_PORT[0x21]&1) == 0) {
           GFX.CG_RAM_mem_temp = GFX.SNESPal[PPU_PORT[0x21]/2];
           PPU_PORT[0x3B] = GFX.CG_RAM_mem_temp&0xFF;
         } else {
           PPU_PORT[0x3B] = GFX.CG_RAM_mem_temp>>8;
         }
         PPU_PORT[0x21]++;
         return PPU_PORT[0x3B];
} 
IN_ITCM        
uint32	R213C(uint32 addr) 
{
      PPU_PORT[0x3C] = (PPU_PORT[0x3C]>>8)|(PPU_PORT[0x3C]<<8);
      return PPU_PORT[0x3C];
}
IN_ITCM
uint32	R213D(uint32 addr)
{
      PPU_PORT[0x3D] = (PPU_PORT[0x3D]>>8)|(PPU_PORT[0x3D]<<8);
      return PPU_PORT[0x3D];
}
IN_ITCM
uint32	R213F(uint32 addr)
{
      return (SNES.NTSC ? 0x01 : 0x11);
}
IN_ITCM      
uint32	R2140(uint32 addr)
{
//	LOG("0 %02x (%04x, %04x)\n", PORT_SPC_TO_SNES[0], (*(uint32*)(0x27E0000)) & 0xFFFF, (uint32)((sint32)PCptr+(sint32)SnesPCOffset));
      if (!CFG.Sound_output)
      { /* APU Skipper */
        switch ((APU.skipper_cnt1++)%11) {
          case 0: return PPU_PORT[0x40];
          case 1: return REAL_A;                                
          case 2: return X;
          case 3: return Y;
          case 4: return 0xAA;
          case 5: return 0xCC;
          case 6: return 0x55;
          case 7: return 0x01;
          case 8: return 0x07;
          case 9: return 0x00;
          case 10: return 0x09;
        }
      }
      return PORT_SPC_TO_SNES[0];      
}

static int oldapupc;

IN_ITCM      
uint32	R2141(uint32 addr)
{
	int newapupc = ((*(uint32*)(0x27E0000)) & 0xFFFF);
	
/*	 if (PORT_SPC_TO_SNES[1] == 0x33 || PORT_SPC_TO_SNES[1] == 0x11 && 
	 (*(uint32*)(0x27E0000)) & 0xFFFF == 0x111f)
	 APU_printLog();*/
#if 0	 
	 if (PORT_SPC_TO_SNES[1] == 0x33 /*&& 
	 (*(uint32*)(0x27E0000)) & 0xFFFF == 0x111f*/)
	 LOG(".");

	 if (/*PORT_SPC_TO_SNES[1] == 0x33 || */PORT_SPC_TO_SNES[1] == 0x11 /*&& 
	 (*(uint32*)(0x27E0000)) & 0xFFFF == 0x111f*/)
	LOG("1 %02x (%04x, %04x)", PORT_SPC_TO_SNES[1], (*(uint32*)(0x27E0000)) & 0xFFFF, (uint32)((sint32)PCptr+(sint32)SnesPCOffset));
#endif	
	
/*	if (newapupc != 0)
	{
	if (newapupc != oldapupc)
		LOG("%04x %04x\n", newapupc);
	oldapupc = newapupc;
	}*/
	
	//*(uint32*)(0x27E0000) = 0;	
      if (!CFG.Sound_output)
      { /* APU Skipper */
        switch ((APU.skipper_cnt2++)%13) {
          case 0: return PPU_PORT[0x41];
          case 1: return REAL_A;
          case 2: return X;
          case 3: return Y;
          case 4: return 0xCD;
          case 5: return 0xBB;
          case 6: return 0x33;
          case 7: return 0x11;
          case 8: return 0x00;
          case 9: return 0xFF;
          case 10: return 0x01;
          case 11: return 0x02;
          case 12: return REAL_A >> 8;        
        }
      }
      return PORT_SPC_TO_SNES[1];       
}
IN_ITCM      
uint32	R2142(uint32 addr)
{
      if (!CFG.Sound_output)
	  {
        switch ((APU.skipper_cnt3++)%7) {
          case 0: return PPU_PORT[0x42];
          case 1: return REAL_A;
          case 2: return X;
          case 3: return Y;
          case 4: return 0x00;
          case 5: return 0xAA;
          case 6: return 0xBB;
        }
      }
      return PORT_SPC_TO_SNES[2];      
}
IN_ITCM
uint32	R2143(uint32 addr)
{     
      if (!CFG.Sound_output)
	  {
        switch((APU.skipper_cnt4++) % 9) {
          case 0: return PPU_PORT[0x43];
          case 1: return REAL_A;
          case 2: return X;
          case 3: return Y;
          case 4: return 0x00;
          case 5: return 0xAA;
          case 6: return 0xBB;
          case 7: return 0x01;
          case 8: return REAL_A>>8;
        }
      }
      return PORT_SPC_TO_SNES[3];      
}
IN_ITCM
uint32	R2180(uint32 addr)
{     
      PPU_PORT[0x80] =
        SNESC.RAM[PPU_PORT[0x81]+(PPU_PORT[0x82]<<8)+((PPU_PORT[0x83]&1)<<16)];
      PPU_PORT[0x81] = (PPU_PORT[0x81]+1)&0xff;
      if (!PPU_PORT[0x81]) {
        PPU_PORT[0x82] = (PPU_PORT[0x82]+1)&0xff; if (!PPU_PORT[0x82]) PPU_PORT[0x83]++;
      }
      return PPU_PORT[0x80];
}

#if 0
  if (address >= 0x3000 && address < 0x3000 + 768)
    {
      uchar	byte;

      if (!CFG.SuperFX)
        return (0x30);

      byte = SuperFX.Regs[address-0x3000];
      if (address == 0x3031)
        {
/*          CLEAR_IRQ_SOURCE(GSU_IRQ_SOURCE);*/
          SuperFX.Regs[0x31] = byte&0x7f;
        }
      if (address == 0x3030)
        {
          SET_WAITCYCLES(0);
        }
      return (byte);
    }
#endif
IN_ITCM
void	W2100(uint32 addr, uint32 value)
{
    GFX.Blank_Screen = (value&0x80) != 0;
    if ((value&0xf) != (PPU_PORT[0x00]&0xf)) {
          
    	GFX.new_color = 255;
	    PPU_setScreen(value);             
    }
    PPU_PORT[0x00] = value;
}
IN_ITCM
void	W2101(uint32 addr, uint32 value)
{
    	 if (value != PPU_PORT[0x01]) {
           GFX.spr_addr_base = (value&0x03)<<14;
           GFX.spr_addr_select = (value&0x18)<<10;
           check_sprite_addr();
         }
    	 PPU_PORT[0x01] = value;
}
IN_ITCM
void	W2102(uint32 addr, uint32 value)
{
         PPU_PORT[0x02] = (PPU_PORT[0x02]&0x100)+value;
         GFX.Old_SpriteAddress = PPU_PORT[0x02];
         if (PPU_PORT[0x03]&0x80)
           GFX.HighestSprite = (PPU_PORT[0x02]>>1)&0x7f;
         GFX.OAM_upper_byte = 0;
           GFX.Sprites_table_dirty = 1;
}
IN_ITCM                    
void	W2103(uint32 addr, uint32 value)
{
         PPU_PORT[0x02] = (PPU_PORT[0x02]&0xff)+(value&1)*0x100;
         if (PPU_PORT[0x02] >= 0x110)
           PPU_PORT[0x02] %= 0x110;
         GFX.Old_SpriteAddress = PPU_PORT[0x02];
         GFX.HighestSprite = (PPU_PORT[0x02]>>1)&0x7f;
         GFX.OAM_upper_byte = 0;
//         GFX.Sprites_table_dirty = 1;
         PPU_PORT[0x03] = value;
}
IN_ITCM
void	W2104(uint32 addr, uint32 value)
{
         if ((PPU_PORT[0x02]) >= 0x100) {
           if (GFX.OAM_upper_byte) {
             if (GFX.spr_info_ext[(PPU_PORT[0x02]<<1)+1-0x200] != value)
               {
                 GFX.spr_info_ext[(PPU_PORT[0x02]<<1)+1-0x200] = value;
                 GFX.Sprites_table_dirty = 1;
               }
             PPU_PORT[0x02]++;
             if (PPU_PORT[0x02] == 0x110)
               PPU_PORT[0x02] = 0;
             GFX.OAM_upper_byte = 0;
           } else {
             if (GFX.spr_info_ext[(PPU_PORT[0x02]<<1)-0x200] != value)
               {
                 GFX.spr_info_ext[(PPU_PORT[0x02]<<1)-0x200] = value;
                 GFX.Sprites_table_dirty = 1;
               }
             GFX.OAM_upper_byte = 1;
           }
         } else {
           if (GFX.OAM_upper_byte) {
             if (((uchar *)GFX.spr_info)[(PPU_PORT[0x02]<<1)+1] != value)
               {
                 ((uchar *)GFX.spr_info)[(PPU_PORT[0x02]<<1)+1] = value;
                 GFX.Sprites_table_dirty = 1;
               }
             PPU_PORT[0x02]++;
             GFX.OAM_upper_byte = 0;
           } else {
             if (((uchar *)GFX.spr_info)[(PPU_PORT[0x02]<<1)] != value)
               {
                 ((uchar *)GFX.spr_info)[(PPU_PORT[0x02]<<1)] = value;
                 GFX.Sprites_table_dirty = 1;
               }
             GFX.OAM_upper_byte = 1;
           }
         }
         PPU_PORT[0x04] = value;
}
IN_ITCM         
void	W2105(uint32 addr, uint32 value)
{		
	if (value == PPU_PORT[0x05])
		return;
	PPU_PORT[0x05] = value;		
	// Update tile system
	PPU_add_tile_address(0); 
	PPU_add_tile_address(1);
	PPU_add_tile_address(2);
}
IN_ITCM         
void	W2107(uint32 addr, uint32 value)
{		
	GFX.map_slot[0] = (value&0x7C)>>2;
	if (!(PPU_PORT[0x05]&(0x10 << 0)))	
	{
		GFX.map_slot_ds[0] = GFX.map_slot[0];
		GFX.map_size[0] = (value & 0x3) << 14; 
	}

	PPU_PORT[0x07] = value;
}
IN_ITCM
void	W2108(uint32 addr, uint32 value)
{		
	GFX.map_slot[1] = (value&0x7C)>>2;
	if (!(PPU_PORT[0x05]&(0x10 << 1)))	
	{
		GFX.map_slot_ds[1] = GFX.map_slot[1];
		GFX.map_size[1] = (value & 0x3) << 14;
	}
	
	PPU_PORT[0x08] = value;
}
IN_ITCM
void	W2109(uint32 addr, uint32 value)
{		
	GFX.map_slot[2] = (value&0x7C)>>2;
	if (!(PPU_PORT[0x05]&(0x10 << 2)))
	{	
		// Bad palette in bg3 dirty fix
		if (CFG.BG3PaletteFix)
			GFX.map_slot_ds[2] = map_duplicate(GFX.map_slot[2]);
		else
			GFX.map_slot_ds[2] = GFX.map_slot[2];
		GFX.map_size[2] = (value & 0x3) << 14;
	}
	
	PPU_PORT[0x09] = value;
}
IN_ITCM
void	W210A(uint32 addr, uint32 value)
{		
	GFX.map_slot[3] = (value&0x7C)>>2;
	if (!(PPU_PORT[0x05]&(0x10 << 3)))	
	{	
		GFX.map_slot_ds[3] = GFX.map_slot[3];
		GFX.map_size[3] = (value & 0x3) << 14;
	}
	PPU_PORT[0x0A] = value;
}
IN_ITCM
void	W210B(uint32 addr, uint32 value)
{		
    if (value != PPU_PORT[0x0B])
    {
  	  GFX.tile_address[0] = ((value&0x07) << 0xd);
  	  GFX.tile_address[1] = ((value&0x70) << 0x9);
  	  
  	  PPU_add_tile_address(0);
  	  PPU_add_tile_address(1);
    }
	PPU_PORT[0x0B] = value;    	  
}
IN_ITCM
void	W210C(uint32 addr, uint32 value)
{		
    if (value != PPU_PORT[0x0C])
    {
  	  GFX.tile_address[2] = ((value&0x07) << 0xd);
  	  GFX.tile_address[3] = ((value&0x70) << 0x9);
  	  
  	  PPU_add_tile_address(2);
  	  PPU_add_tile_address(3);
    }
	PPU_PORT[0x0C] = value;    	  
}
IN_ITCM
void	W210D(uint32 addr, uint32 value) {
     if ((GFX.BG_scroll_reg&0x01)==0) {
       GFX.old_scrollx[0] = PPU_PORT[0x0D];
       PPU_PORT[0x0D] = value; GFX.BG_scroll_reg |= 0x01;
     } else {
       PPU_PORT[0x0D] += (value<<8); GFX.BG_scroll_reg &= ~0x01;
//           update_scrollx(0);
     }
}
IN_ITCM         
void	W210E(uint32 addr, uint32 value) {
         if ((GFX.BG_scroll_reg&0x02)==0) {
           GFX.old_scrolly[0] = PPU_PORT[0x0E];
           PPU_PORT[0x0E] = value; GFX.BG_scroll_reg |= 0x02;
         } else {
           PPU_PORT[0x0E] += (value<<8); GFX.BG_scroll_reg &= ~0x02;
//           update_scrolly(0);
         }
}
IN_ITCM         
void	W210F(uint32 addr, uint32 value) {
         if ((GFX.BG_scroll_reg&0x04)==0) {
           GFX.old_scrollx[1] = PPU_PORT[0x0F];
           PPU_PORT[0x0F] = value; GFX.BG_scroll_reg |= 0x04;
         } else {
           PPU_PORT[0x0F] += (value<<8); GFX.BG_scroll_reg &= ~0x04;
  //         update_scrollx(1);
     }
}
IN_ITCM
void	W2110(uint32 addr, uint32 value) {
     if ((GFX.BG_scroll_reg&0x08)==0) {
       GFX.old_scrolly[1] = PPU_PORT[0x10];
       PPU_PORT[0x10] = value; GFX.BG_scroll_reg |= 0x08;
     } else {
       PPU_PORT[0x10] += (value<<8); GFX.BG_scroll_reg &= ~0x08;
//           update_scrolly(1);
     }
}
IN_ITCM     
void	W2111(uint32 addr, uint32 value) {
     if ((GFX.BG_scroll_reg&0x10)==0) {
       GFX.old_scrollx[2] = PPU_PORT[0x11];
       PPU_PORT[0x11] = value; GFX.BG_scroll_reg |= 0x10;
     } else {
       PPU_PORT[0x11] += (value<<8); GFX.BG_scroll_reg &= ~0x10;
//           update_scrollx(2);
     }
} 
IN_ITCM    
void	W2112(uint32 addr, uint32 value) {
     if ((GFX.BG_scroll_reg&0x20)==0) {
       GFX.old_scrolly[2] = PPU_PORT[0x12];
       PPU_PORT[0x12] = value; GFX.BG_scroll_reg |= 0x20;
     } else {
       PPU_PORT[0x12] += (value<<8); GFX.BG_scroll_reg &= ~0x20;
//           update_scrolly(2);
     }
}
IN_ITCM     
void	W2113(uint32 addr, uint32 value) {
     if ((GFX.BG_scroll_reg&0x40)==0) {
       PPU_PORT[0x13] = value; GFX.BG_scroll_reg |= 0x40;
     } else {
       PPU_PORT[0x13] += (value<<8); GFX.BG_scroll_reg &= ~0x40;
     } 
//         GFX.tiles_ry[3] = 8; return;
}
IN_ITCM
void	W2114(uint32 addr, uint32 value) {
     if ((GFX.BG_scroll_reg&0x80)==0) {
       PPU_PORT[0x14] = value; GFX.BG_scroll_reg |= 0x80;
     } else {
       PPU_PORT[0x14] += (value<<8); GFX.BG_scroll_reg &= ~0x80;
     } 
//         GFX.tiles_ry[3] = 8; return;
}
IN_ITCM
void	W2115(uint32 addr, uint32 value) 
{
         switch(value&0x3) {
           case 0x0 : GFX.SC_incr = 0x01; break;
           case 0x1 : GFX.SC_incr = 0x20; break;
           case 0x2 : GFX.SC_incr = 0x40; break;
           case 0x3 : GFX.SC_incr = 0x80; break;
         }
         switch(value&0xc) {
           case 0x0 : GFX.FS_incr = 0x00; break;
           case 0x4 : GFX.FS_incr = 0x20; GFX.FS_cnt2 = GFX.FS_cnt = 0; break;
           case 0x8 : GFX.FS_incr = 0x40; GFX.FS_cnt2 = GFX.FS_cnt = 0; break;
           case 0xC : GFX.FS_incr = 0x80; GFX.FS_cnt2 = GFX.FS_cnt = 0; break;
         }
         PPU_PORT[0x15] = value;
}
IN_ITCM
void	W2116(uint32 addr, uint32 value) 
{
         PPU_PORT[0x16] = (PPU_PORT[0x16]&0xff00) + value;
         GFX.Dummy_VRAMRead = 1;
}
IN_ITCM
void	W2117(uint32 addr, uint32 value) 
{
         PPU_PORT[0x16] = (PPU_PORT[0x16]&0xff) + (value << 8);
         GFX.Dummy_VRAMRead = 1;
         PPU_PORT[0x17] = value;
}
IN_ITCM
void	W2118(uint32 addr, uint32 value)
{
   	 if (PPU_PORT[0x15]&0x80) {
           if (SNESC.VRAM[(PPU_PORT[0x16]<<1)&0xFFFF] != value)
			 check_tile();
           SNESC.VRAM[(PPU_PORT[0x16]<<1)&0xFFFF] = value;
         } else {
           if (SNESC.VRAM[(PPU_PORT[0x16]<<1)&0xFFFF] != value)
			 check_tile();
           SNESC.VRAM[(PPU_PORT[0x16]<<1)&0xFFFF] = value;
           PPU_PORT[0x16] += GFX.SC_incr;
           if (GFX.FS_incr) {
             PPU_PORT[0x16] += 8;
             if (++GFX.FS_cnt == GFX.FS_incr) {
               GFX.FS_cnt = 0;
               if (++GFX.FS_cnt2 == 8) {
                 GFX.FS_cnt2 = 0; PPU_PORT[0x16] -= 7;
               } else
                 PPU_PORT[0x16] -= 8*GFX.FS_incr-1;
             }
           }
         }
	PPU_PORT[0x18] = value; // needed ?
}
IN_ITCM         
void	W2119(uint32 addr, uint32 value)
{
   	 if ((PPU_PORT[0x15]&0x80) == 0) {
           if (SNESC.VRAM[((PPU_PORT[0x16]<<1)+1)&0xFFFF] != value)
				check_tile();
           SNESC.VRAM[((PPU_PORT[0x16]<<1)+1)&0xFFFF] = value;
         } else {
           if (SNESC.VRAM[((PPU_PORT[0x16]<<1)+1)&0xFFFF] != value)
			 check_tile();
           SNESC.VRAM[((PPU_PORT[0x16]<<1)+1)&0xFFFF] = value;
           if (!GFX.FS_incr) {
             PPU_PORT[0x16] += GFX.SC_incr;
           } else {
             PPU_PORT[0x16] += 8;
             if (++GFX.FS_cnt == GFX.FS_incr) {
               GFX.FS_cnt = 0;
               if (++GFX.FS_cnt2 == 8) {
                 GFX.FS_cnt2 = 0; PPU_PORT[0x16] -= 8-GFX.SC_incr;
               }
               else
                 PPU_PORT[0x16] -= 8*GFX.FS_incr-GFX.SC_incr;
             }
           }
         }
	PPU_PORT[0x19] = value;
}
IN_ITCM
void	W211A(uint32 addr, uint32 value)
{
	 SNES.Mode7Repeat = value>>6;
/*	 SNES.Mode7VFlip = (Byte & 2) >> 1;
	 SNES.Mode7HFlip = Byte & 1;*/
	 PPU_PORT[0x1A] = value;
}
IN_ITCM
void	W211B(uint32 addr, uint32 value)
{
         PPU_PORT[0x1B] = (PPU_PORT[0x1B] >> 8) + (value << 8);
         SNES.PPU_NeedMultiply = 1;
}
IN_ITCM
void	W211C(uint32 addr, uint32 value)
{
         PPU_PORT[0x1C] = (PPU_PORT[0x1C] >> 8) + (value << 8);
         SNES.PPU_NeedMultiply = 1;
}
IN_ITCM         
void	W211D(uint32 addr, uint32 value)
{
         PPU_PORT[0x1D] = (PPU_PORT[0x1D] >> 8) + (value << 8);
}
IN_ITCM         
void	W211E(uint32 addr, uint32 value)
{
         PPU_PORT[0x1E] = (PPU_PORT[0x1E] >> 8) + (value << 8);
}
IN_ITCM         
void	W211F(uint32 addr, uint32 value)
{
         PPU_PORT[0x1F] = (PPU_PORT[0x1F] >> 8) + (value << 8);
}
IN_ITCM         
void	W2120(uint32 addr, uint32 value)
{
         PPU_PORT[0x20] = (PPU_PORT[0x20] >> 8) + (value << 8);
}
IN_ITCM
void	W2121(uint32 addr, uint32 value)
{
         PPU_PORT[0x21] = (value << 1);
}
IN_ITCM
void	W2122(uint32 addr, uint32 value)
{
         if (PPU_PORT[0x21] == 0x200) PPU_PORT[0x21]=0;
         if ((PPU_PORT[0x21]&1) == 0)
           GFX.CG_RAM_mem_temp = value;
         else {
           uint16	p;
           GFX.CG_RAM_mem_temp = (GFX.CG_RAM_mem_temp&0xff)+(value<<8);
           p = GFX.CG_RAM_mem_temp&0x7FFF;
           if (p != GFX.SNESPal[PPU_PORT[0x21]/2]) {
             GFX.SNESPal[PPU_PORT[0x21]/2] = p;
             GFX.new_color |= 1; GFX.new_colors[PPU_PORT[0x21]/2] = 1;
             if ((PPU_PORT[0x21] >> 1) != 0)
		     	PPU_setPalette(PPU_PORT[0x21] >> 1, p);
           }
         } 
         PPU_PORT[0x21]++;
         PPU_PORT[0x22] = value;
}
IN_ITCM
void	W2132(uint32 addr, uint32 value) 
{
	if ((value & 0x20)) GFX.BACK = (GFX.BACK&0x7fe0)|((value&0x1f)<<0); /* R */
    if ((value & 0x40)) GFX.BACK = (GFX.BACK&0x7c1f)|((value&0x1f)<<5); /* G */
    if ((value & 0x80)) GFX.BACK = (GFX.BACK&0x03ff)|((value&0x1f)<<10); /* B */
    PPU_PORT[0x32] = value;
}
IN_ITCM         
void	W2133(uint32 addr, uint32 value)
{
         GFX.ScreenHeight = (value&4)?240:224;
         PPU_PORT[0x33] = value;
}

/*
 * #if 0	
	if (value == 0x55 && (newapupc & 0xf000) == 0xf000)
	LOG("%02x 1 (%04x, %04x)\n", value, newapupc, (uint32)((sint32)PCptr+(sint32)SnesPCOffset));
#else
	if (value == 0x55 && (newapupc & 0xf000) == 0xf000)
	{
	int i;
	for (i = 0; i < 100; i++)
		dummy++;
	}
#endif		          		
	if (APU_ADDR_BLKP[1])
	{
		//LOG("1 b %04x\n", newapupc);
		while (APU_ADDR_BLKP[1]);
#if 0
  		LOG("1 w %02x %04x\n", value, *(uint16 *)(APU_RAM_ADDRESS+0x18));
#else  		
	{
	int i;
	for (i = 0; i < 200; i++)
		dummy++;
	}
#endif			  		
  	}*/

static volatile uint32 dummy;	
void	pseudoSleep(int d)
{
	int i;
	
	for (i = 0; i < d; i++)
		dummy++;
}

#define	SYNC_TIME	500

//IN_ITCM
void	W2140(uint32 addr, uint32 value)
{
    if (CFG.Sound_output)
    {    	
//   		LOG("0<-%02x\n", value); 
    	if (CFG.SoundPortSync & 0x10)
    		pseudoSleep(SYNC_TIME);
		if (CFG.SoundPortSync & 1)
		{
			if (APU_ADDR_BLKP[0])
			{
				while (APU_ADDR_BLKP[0]);
			}
		}    	
    	PORT_SNES_TO_SPC[0] = value;
    	
		if ((CFG.SoundPortSync & 1) && value) 
			APU_ADDR_BLKP[0] = 1;    	
    }
    else
        PPU_PORT[0x40] = value; 
}

//IN_ITCM
void	W2141(uint32 addr, uint32 value)
{
    if (CFG.Sound_output)
    {
    	//LOG("1<-%02x\n", value);    	
    	if (CFG.SoundPortSync & 0x20)
    		pseudoSleep(SYNC_TIME);
		if (CFG.SoundPortSync & 2)
		{
			if (APU_ADDR_BLKP[1])
			{
				while (APU_ADDR_BLKP[1]);
			}
		}
/*				    	
#ifdef USE_APU_PORT_BLK    	
		int newapupc = (*(uint32*)(0x27E0000)) & 0xFFFF;
		if (value == 0x55 && (newapupc & 0xf000) == 0x1000)
			pseudoSleep(2000);	
		if (APU_ADDR_BLKP[1])
		{
			while (APU_ADDR_BLKP[1]);
			pseudoSleep(2000);
		}
#endif
*/
    	PORT_SNES_TO_SPC[1] = value;
    	
		if ((CFG.SoundPortSync & 2) && value) 
			APU_ADDR_BLKP[1] = 1;			    	
    }
    else
        PPU_PORT[0x41] = value;
}

//IN_ITCM
void	W2142(uint32 addr, uint32 value)
{
    if (CFG.Sound_output)
    {    
//    	LOG("2<-%02x\n", value);    	
    	if (CFG.SoundPortSync & 0x40)
    		pseudoSleep(SYNC_TIME);    	
		if (CFG.SoundPortSync & 4)
		{
			if (APU_ADDR_BLKP[2])
			{
				while (APU_ADDR_BLKP[2]);
			}
		}

    	PORT_SNES_TO_SPC[2] = value;
    	
		if ((CFG.SoundPortSync & 4) && value) 
			APU_ADDR_BLKP[2] = 1;			    	
    }
    else
        PPU_PORT[0x42] = value;    	     
}

//IN_ITCM
void	W2143(uint32 addr, uint32 value)
{
    if (CFG.Sound_output)
    {  
//    	LOG("3<-%02x\n", value);    	
    	if (CFG.SoundPortSync & 0x80)
    		pseudoSleep(SYNC_TIME);    	
		if (CFG.SoundPortSync & 8)
		{	
			if (APU_ADDR_BLKP[3])
			{
				while (APU_ADDR_BLKP[3]);
			}
		}

    	PORT_SNES_TO_SPC[3] = value;
   	
		if ((CFG.SoundPortSync & 8) && value) 
			APU_ADDR_BLKP[3] = 1;			    	
    }
    else
        PPU_PORT[0x43] = value; 
}

IN_ITCM
void	W2180(uint32 addr, uint32 value)
{
      SNESC.RAM[PPU_PORT[0x81]+(PPU_PORT[0x82]<<8)+((PPU_PORT[0x83]&1)<<16)] = value;
      PPU_PORT[0x81] = (PPU_PORT[0x81]+1)&0xff;
      if (!PPU_PORT[0x81]) {
        PPU_PORT[0x82] = (PPU_PORT[0x82]+1)&0xff;
        if (!PPU_PORT[0x82]) PPU_PORT[0x83]++;
      }
}

void	WW210D(uint32 addr, uint32 value) {
   GFX.old_scrollx[0] = PPU_PORT[0x0D];
   PPU_PORT[0x0D] = value;
//           update_scrollx(0);
}         
void	WW210E(uint32 addr, uint32 value) {
   GFX.old_scrolly[0] = PPU_PORT[0x0E];
   PPU_PORT[0x0E] = value;
//           update_scrolly(0);
}         
void	WW210F(uint32 addr, uint32 value) {
   GFX.old_scrollx[1] = PPU_PORT[0x0F];
   PPU_PORT[0x0F] = value;
//         update_scrollx(1);
}
void	WW2110(uint32 addr, uint32 value) {
   GFX.old_scrolly[1] = PPU_PORT[0x10];
   PPU_PORT[0x10] = value;
//           update_scrolly(1);
}     
void	WW2111(uint32 addr, uint32 value) {
   GFX.old_scrollx[2] = PPU_PORT[0x11];
   PPU_PORT[0x11] = value;
//           update_scrollx(2);
}     
void	WW2112(uint32 addr, uint32 value) {
   GFX.old_scrolly[2] = PPU_PORT[0x12];
   PPU_PORT[0x12] = value;
//           update_scrolly(2);
}     
void	WW2113(uint32 addr, uint32 value) {
    PPU_PORT[0x13] = value;
//         GFX.tiles_ry[3] = 8; return;
}
void	WW2114(uint32 addr, uint32 value) {
	PPU_PORT[0x14] = value;
//         GFX.tiles_ry[3] = 8; return;
}
void	WW2122(uint32 addr, uint32 value) {
     if (PPU_PORT[0x21] == 0x200) PPU_PORT[0x21]=0;
     uint16	p;
     GFX.CG_RAM_mem_temp = value;
     p = GFX.CG_RAM_mem_temp & 0x7FFF;
     if (p != GFX.SNESPal[PPU_PORT[0x21]/2]) {
       GFX.SNESPal[PPU_PORT[0x21]/2] = p;
       GFX.new_color |= 1; GFX.new_colors[PPU_PORT[0x21]/2] = 1;
       if ((PPU_PORT[0x21] >> 1) != 0)
	   		PPU_setPalette(PPU_PORT[0x21] >> 1, p);
     }
     PPU_PORT[0x21]+=2;
}

typedef void (*IOWriteFunc)(uint32 addr, uint32 byte);
typedef uint32 (*IOReadFunc)(uint32 addr);

/*
	00		01		02		03		04		05		06		07
	08		09		0A		0B		0C		0D		0E		0F */

#define NOP IONOP_PPU_WRITE
IN_DTCM
IOWriteFunc	IOWrite_PPU[0x90] =
{ 
  W2100,  W2101,  W2102,  W2103,  W2104,  W2105,    NOP,  W2107,	/* 2100 */		
  W2108,  W2109,  W210A,  W210B,  W210C,  W210D,  W210E,  W210F,	
  W2110,  W2111,  W2112,  W2113,  W2114,  W2115,  W2116,  W2117,	/* 2110 */
  W2118,  W2119,  W211A,  W211B,  W211C,  W211D,  W211E,  W211F,	
  W2120,  W2121,  W2122,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2120 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
	NOP,	NOP,  W2132,  W2133,	NOP,	NOP,	NOP,	NOP,	/* 2130 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
  W2140,  W2141,  W2142,  W2143,	NOP,	NOP,	NOP,	NOP,	/* 2140 */	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2150 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2160 */	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2170 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
  W2180,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2180 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
};

/*
	00		01		02		03		04		05		06		07
	08		09		0A		0B		0C		0D		0E		0F */

IOWriteFunc	IOWriteWord_PPU[0x90] =
{ 
  W2100,  W2101,  W2102,  W2103,  W2104,  W2105,    NOP,  W2107,	/* 2100 */		
  W2108,  W2109,  W210A,  W210B,  W210C, WW210D, WW210E, WW210F,	
 WW2110, WW2111, WW2112, WW2113, WW2114,  W2115,  W2116,  W2117,	/* 2110 */
  W2118,  W2119,  W211A,    NOP,    NOP,	NOP, 	NOP,	NOP,	
	NOP,  W2121, WW2122,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2120 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
	NOP,	NOP,  W2132,  W2133,	NOP,	NOP,	NOP,	NOP,	/* 2130 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
  W2140,  W2141,  W2142,  W2143,	NOP,	NOP,	NOP,	NOP,	/* 2140 */	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2150 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2160 */	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2170 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
  W2180,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2180 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
};
#undef NOP

/*
	00		01		02		03		04		05		06		07
	08		09		0A		0B		0C		0D		0E		0F */

#define NOP IONOP_PPU_READ
IN_DTCM
IOReadFunc	IORead_PPU[0x90] =
{ 
  	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2100 */		
  	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
  	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2110 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
	NOP,  R2121,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2120 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
	NOP,	NOP,	NOP,	NOP,  R213X,  R213X,  R213X,  R2137,	/* 2130 */
  R2138,  R2139,  R213A,  R213B,  R213C,  R213D,    NOP,  R213F,	
  R2140,  R2141,  R2142,  R2143,	NOP,	NOP,	NOP,	NOP,	/* 2140 */	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2150 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2160 */	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2170 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
  R2180,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	/* 2180 */
	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	NOP,	
};
#undef NOP

/*
	00		01		02		03		04		05		06		07
	08		09		0A		0B		0C		0D		0E		0F */

#define NOP IONOP_DMA_WRITE
IN_DTCM
IOWriteFunc	IOWrite_DMA[0x20] =
{ 
  W4200,    NOP,    NOP,  W4203,    NOP,    NOP,  W4206,  W4207,	/* 4200 */		
  W4208,    NOP,    NOP,  W420B,  W420C,    NOP,    NOP,    NOP,	
    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,	/* 4210 */
    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,	
};
#undef NOP

#define NOP IONOP_DMA_READ
IN_DTCM
IOReadFunc	IORead_DMA[0x20] =
{ 
    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,	/* 4200 */		
    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,	
  R4210,  R4211,  R4212,    NOP,    NOP,    NOP,    NOP,    NOP,	/* 4210 */
    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,    NOP,	
};
#undef NOP

IN_ITCM
void	PPU_port_write(uint32 address, uint8 byte)
{
	if (address >= 0x2100 && address < 0x2190)
		IOWrite_PPU[address-0x2100](address-0x2100, byte);
}

IN_ITCM
uint8	PPU_port_read(uint32 address)
{
	if (address >= 0x2100 && address < 0x2190)
		return (uint8)IORead_PPU[address-0x2100](address-0x2100);
	return 0;
}

IN_ITCM
void	DMA_port_write(uint32 address, uint8 byte)
{
	if (address >= 0x4200)
	{
		if (address < 0x4220)
			IOWrite_DMA[address-0x4200](address-0x4200, byte);
		else
		if (address < 0x4380)
			DMA_PORT[address-0x4200]=byte;
	}
	else
		if (address == 0x4016)
			W4016(0x16, byte);
		else
		if (address == 0x4017)
			W4017(0x17, byte);
				
}
IN_ITCM
uint8	DMA_port_read(uint32 address)
{
	if (address >= 0x4200)
	{
		if (address < 0x4220)
			return (uint8)IORead_DMA[address-0x4200](address-0x4200);
		else
		if (address < 0x4380)
			return (uint8)DMA_PORT[address-0x4200];
	}
	else
		if (address == 0x4016)
			return (uint8)R4016(0x16);
		else
		if (address == 0x4017)
			return (uint8)R4017(0x17);
	return 0; 
}

#if 0
  if (address >= 0x3000 && address < 0x3000 + 768)
    {
      if (!CFG.SuperFX)
        return;

      switch (address) {
        case 0x301F:
          SuperFX.Regs[0x1F] = value;
          SuperFX.Regs[GSU_SFR] |= FLG_G;
          SuperFXExec();
          return;
        case 0x3030:
          if ((SuperFX.Regs[0x30]^value)&FLG_G)
            {
              SuperFX.Regs[0x30] = value;
              if (value&FLG_G)
                SuperFXExec();
              else
                SuperFXFlushCache();
            }
          else
            SuperFX.Regs[0x30] = value;
          break;
        case 0x3034:
        case 0x3036:
          SuperFX.Regs[address-0x3000] = value & 0x7f;
          break;
        case 0x303B:
          break;
        default:
          SuperFX.Regs[address-0x3000] = value;
          if (address >= 0x3100)
            {
              SuperFXCacheWriteAccess(address);
            }
          break;
      }
      return;
    }
#endif


/*
 * Port used by chrono trigger:
2126 : window
2107 : tile map address
210d : scroll
210f : scroll
2111 : scroll
212c : main screen / sub screen
2130 : color addition
2105 : bg mode
 */ 

static inline void HDMA_write_port(uchar port, uint8 *data)
{
  uint32 	PPUport = SNES.HDMA_port[port];
  
  //LOG("HDMA %02x %04x %p\n", SNES.HDMA_info[port], 0x2100+PPUport, data);  
  switch(SNES.HDMA_info[port])
  {
    case 0x00 :
      SNES.UsedCycles += 1;
      IOWrite_PPU[PPUport+0](PPUport+0, *data);
      break;
    case 0x01 :
      SNES.UsedCycles += 3;
      IOWrite_PPU[PPUport+0](PPUport+0, *data++);
      IOWrite_PPU[PPUport+1](PPUport+1, *data);
      break;
    case 0x02 :
      SNES.UsedCycles += 3;
      IOWriteWord_PPU[PPUport+0](PPUport+0, data[0]+(data[1]<<8));
      break;
    case 0x03 :
      SNES.UsedCycles += 6;
	  IOWriteWord_PPU[PPUport+0](PPUport+0, data[0]+(data[1]<<8)); data+=2;
	  IOWriteWord_PPU[PPUport+1](PPUport+1, data[0]+(data[1]<<8));        
      break;      
    case 0x04 :
      SNES.UsedCycles += 6;
      IOWrite_PPU[PPUport+0](PPUport+0, *data++);
      IOWrite_PPU[PPUport+1](PPUport+1, *data++);
      IOWrite_PPU[PPUport+2](PPUport+2, *data++);
      IOWrite_PPU[PPUport+3](PPUport+3, *data);
      break;
  }
}

IN_ITCM
void	HDMA_write()
{
	int		HDMASel = DMA_PORT[0x0C];
	uint8	**HDMAdata;
	int		i;

	HDMAdata = &SNES.HDMA_values[SNES.HDMA_line][0];
	for (i = 0; i < 8; i++)
	{
		
    	if (HDMASel&(1<<i) && SNES.HDMA_line < SNES.HDMA_nblines[i]) 
    		HDMA_write_port(i, HDMAdata[i]);
	}
    
    /*if (HDMASel&0x01 && SNES.HDMA_line < SNES.HDMA_nblines[0]) HDMA_write_port(0);
    if (HDMASel&0x02 && SNES.HDMA_line < SNES.HDMA_nblines[1]) HDMA_write_port(1);
    if (HDMASel&0x04 && SNES.HDMA_line < SNES.HDMA_nblines[2]) HDMA_write_port(2);
    if (HDMASel&0x08 && SNES.HDMA_line < SNES.HDMA_nblines[3]) HDMA_write_port(3);
    if (HDMASel&0x10 && SNES.HDMA_line < SNES.HDMA_nblines[4]) HDMA_write_port(4);
	if (HDMASel&0x20 && SNES.HDMA_line < SNES.HDMA_nblines[5]) HDMA_write_port(5);
    if (HDMASel&0x40 && SNES.HDMA_line < SNES.HDMA_nblines[6]) HDMA_write_port(6);
    if (HDMASel&0x80 && SNES.HDMA_line < SNES.HDMA_nblines[7]) HDMA_write_port(7);*/
}    



/* SuperFX */

/*void SuperFXExec()
{
  if (CFG.SuperFX)
    {
      if ((SuperFX.Regs[GSU_SFR]&FLG_G) &&
	  (SuperFX.Regs[GSU_SCMR]&0x18) == 0x18)
	{
          int GSUStatus;

          SuperFXEmulate((SuperFX.Regs[GSU_CLSR]&1)?1330:650);
          GSUStatus = SuperFX.Regs[GSU_SFR]|(SuperFX.Regs[GSU_SFR+1]<<8);
          if ((GSUStatus&(FLG_G|FLG_IRQ)) == FLG_IRQ)
	    {
		// Trigger a GSU IRQ.
              CPU.SavedCycles = CPU.Cycles;
              CPU.IRQState |= IRQ_GSU;
              CPU.Cycles = 0;
	    }
	}
    }
}*/

void	read_joypads()
{
  SNES.joypads[0] = 0;
  if (CFG.joypad_disabled)
    return;

  SNES.joypads[0] = get_joypad();
  SNES.joypads[0] |= 0x80000000;
}

void	read_mouse()
{
  int	tmp;
    
/*  if (SNES.Controller == SNES_MOUSE)
    {*/
      int delta_x, delta_y;

      tmp = 0x1|(SNES.mouse_speed<<4)|
            ((SNES.mouse_b&1)<<6)|((SNES.mouse_b&2)<<6);
      delta_x = SNES.mouse_x-SNES.prev_mouse_x;
      delta_y = SNES.mouse_y-SNES.prev_mouse_y;
      
      if (delta_x || delta_y)
            LOG("%x %x\n", delta_x, delta_y);   

      if (delta_x > 63)
	{
	  delta_x = 63;
          SNES.prev_mouse_x += 63;
	}
      else
	if (delta_x < -63)
	  {
	    delta_x = -63;
	    SNES.prev_mouse_x -= 63;
  	  }
	else
	    SNES.prev_mouse_x = SNES.mouse_x;

	if (delta_y > 63)
	  {
	    delta_y = 63;
	    SNES.prev_mouse_y += 63;
	  }
	else
	if (delta_y < -63)
	  {
	    delta_y = -63;
	    SNES.prev_mouse_y -= 63;
	  }
	else
	  SNES.prev_mouse_y = SNES.mouse_y;

	if (delta_x < 0)
	  {
	    delta_x = -delta_x;
	    tmp |= (delta_x | 0x80) << 16;
	  }
	else
	  tmp |= delta_x << 16;

	if (delta_y < 0)
	  {
	    delta_y = -delta_y;
	    tmp |= (delta_y | 0x80) << 24;
	  }
	else
	  tmp |= delta_y << 24;

	SNES.joypads[0] = tmp;
/*    }*/
}

void read_scope()
{
    int	x, y;
    uint buttons;
    uint scope;

    if ((buttons = SNES.mouse_b))
      {
        x = SNES.mouse_x;
        y = SNES.mouse_y;

	scope = 0x00FF | ((buttons & 1) << (7 + 8)) |
		((buttons & 2) << (5 + 8)) | ((buttons & 4) << (3 + 8)) |
		((buttons & 8) << (1 + 8));
	if (x > 255)
	    x = 255;
	if (x < 0)
	    x = 0;
	if (y > GFX.ScreenHeight - 1)
	    y = GFX.ScreenHeight - 1;
	if (y < 0)
	    y = 0;

        PPU_PORT[0x3C] = x;
        PPU_PORT[0x3C] = (PPU_PORT[0x3C]>>8) | (PPU_PORT[0x3C]<<8);
        PPU_PORT[0x3D] = y+1;
        PPU_PORT[0x3D] = (PPU_PORT[0x3D]>>8) | (PPU_PORT[0x3D]<<8);

	PPU_PORT[0x3F] |= 0x40;
	SNES.joypads[1] = scope;
    }
}

void	update_joypads()
{
//  read_joypads();	
  int joypad = get_joypad();
//      read_joypads();
  SNES.joypads[0] = joypad;
  SNES.joypads[0] |= 0x80000000;
  if (CFG.mouse)
    read_mouse();
  if (CFG.scope)
  	read_scope();

  if (DMA_PORT[0x00]&1)
    {
  	  SNES.Joy1_cnt = 16;    	
      DMA_PORT[0x18] = SNES.joypads[0];
      DMA_PORT[0x19] = SNES.joypads[0]>>8;
      DMA_PORT[0x1A] = SNES.joypads[1];
      DMA_PORT[0x1B] = SNES.joypads[1]>>8;
    }
}

void SNES_update()
{ 
  int value;
  
  value = PPU_PORT[0x01];
  GFX.spr_addr_base = (value&0x03)<<14;
  GFX.spr_addr_select = (value&0x18)<<10;
  
  value = PPU_PORT[0x0B];
  GFX.tile_address[0] = ((value&0x0f) << 0xd);
  GFX.tile_address[1] = ((value&0xf0) << 0x9);
 
  value = PPU_PORT[0x0C];  
  GFX.tile_address[2] = ((value&0x0f) << 0xd);
  GFX.tile_address[3] = ((value&0xf0) << 0x9);
  
  GFX.map_slot[0] = (PPU_PORT[0x07]&0x7C)>>2;
  GFX.map_slot[1] = (PPU_PORT[0x08]&0x7C)>>2;
  GFX.map_slot[2] = (PPU_PORT[0x09]&0x7C)>>2;
  GFX.map_slot[3] = (PPU_PORT[0x0A]&0x7C)>>2;
}


void GoNMI()
{
#ifndef ASM_OPCODES	
  if (CPU.WAI_state) {
    CPU.WAI_state = 0; PC++;
  };

  pushb(PB);
  pushw(PC);
  pushb(P);
  PC = CPU.NMI;
  PB = 0;
  P &= ~P_D;
#else
  CPU_pack();

  if (CPU.WAI_state) {
    CPU.WAI_state = 0; CPU.PC++;
  };

  pushb(CPU.PB);
  pushw(CPU.PC);
  pushb(CPU.P);
  CPU.PC = CPU.NMI;
  CPU.PB = 0;
  CPU.P &= ~P_D;
  
  CPU.unpacked = 0; // ASM registers to update
#endif

//  if (CFG.CPU_log) fprintf(SNES.flog, "--> NMI\n");
}

void GoIRQ()
{
#ifndef ASM_OPCODES	
  if (CPU.WAI_state) {
    CPU.WAI_state = 0; PC++;
  };

  if (!(P&P_I)) {
    pushb(PB);
    pushw(PC);
    pushb(P);
    PC = CPU.IRQ; 
    PB = 0;
    P |= P_I;
    P &= ~P_D;
  }
#else
  CPU_pack();

  if (CPU.WAI_state) {
    CPU.WAI_state = 0; CPU.PC++;
  };

  if (!(CPU.P&P_I)) {
    pushb(CPU.PB);
    pushw(CPU.PC);
    pushb(CPU.P);
    CPU.PC = CPU.IRQ; 
    CPU.PB = 0;
    CPU.P |= P_I;
    CPU.P &= ~P_D;
  }
  CPU.unpacked = 0; // ASM registers to update  
#endif  
  
  DMA_PORT[0x11] = 0x80;
//  if (CFG.CPU_log) fprintf(SNES.flog, "--> IRQ\n");
}

