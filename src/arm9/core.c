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

uchar   mem_getbyte(uint offset, uchar bank);
void	mem_setbyte(uint offset, uchar bank, uchar byte);
ushort  mem_getword(uint offset, uchar bank);
void    mem_setword(uint offset, uchar bank, ushort word);

int	SPC700_emu;

void	PPU_port_write(uint address, uchar value);
uchar	PPU_port_read(uint address);


// A OPTIMISER
int	PPU_fastDMA_2118_1(int offs, int bank, int len)
{
	int i;
	uint8	*ptr;

	ptr = map_memory(offs, bank);

	if (SNES.PPU_Port[0x15]&0x80) {
		if (!GFX.FS_incr && GFX.SC_incr == 1)
		{
			// Very fast DMA mode 1!!!!
//			fprintf(SNES.flog,"Very fast!");
			memcpy(SNESC.VRAM+((SNES.PPU_Port[0x16]<<1)&0xFFFF), ptr, len);
			for (i = 0; i < len; i += 2)
			{
				if ((i & 15) == 0) 
					check_tile();
				SNES.PPU_Port[0x16]++;
			}
			return offs+len;
		}
		for (i = 0; i < len; i+=2)
		{
			check_tile();			
			SNESC.VRAM[(SNES.PPU_Port[0x16]<<1)&0xFFFF] = ptr[i];   
			SNESC.VRAM[((SNES.PPU_Port[0x16]<<1)+1)&0xFFFF] = ptr[i+1];
			if (!GFX.FS_incr) {
				SNES.PPU_Port[0x16] += GFX.SC_incr;
			} else {
				SNES.PPU_Port[0x16] += 8;
				if (++GFX.FS_cnt == GFX.FS_incr) {
					GFX.FS_cnt = 0;
					if (++GFX.FS_cnt2 == 8) {
						GFX.FS_cnt2 = 0; SNES.PPU_Port[0x16] -= 8-GFX.SC_incr;
					}
					else
						SNES.PPU_Port[0x16] -= 8*GFX.FS_incr-GFX.SC_incr;
				}
			}
		}
	}
	else
	{
		for (i = 0; i < len; i+=2)
		{
			SNESC.VRAM[(SNES.PPU_Port[0x16]<<1)&0xFFFF] = ptr[i];
			SNES.PPU_Port[0x16] += GFX.SC_incr;
			if (GFX.FS_incr) {
				SNES.PPU_Port[0x16] += 8;
				if (++GFX.FS_cnt == GFX.FS_incr) {
					GFX.FS_cnt = 0;
					if (++GFX.FS_cnt2 == 8) {
						GFX.FS_cnt2 = 0; SNES.PPU_Port[0x16] -= 7;
					} else
						SNES.PPU_Port[0x16] -= 8*GFX.FS_incr-1;
				}
			}
			SNESC.VRAM[((SNES.PPU_Port[0x16]<<1)+1)&0xFFFF] = ptr[i+1];
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
  DMA_address = SNES.DMA_Port[0x102+port*0x10]+(SNES.DMA_Port[0x103+port*0x10]<<8);
  DMA_bank = SNES.DMA_Port[0x104+port*0x10];
  DMA_len = SNES.DMA_Port[0x105+port*0x10]+(SNES.DMA_Port[0x106+port*0x10]<<8);
  if (DMA_len == 0)
    DMA_len = 0x10000;
  PPU_port = 0x2100+SNES.DMA_Port[0x101+port*0x10];
  DMA_info = SNES.DMA_Port[0x100+port*0x10];

/*   FS_flog("DMA[%d] %06X->%04X SIZE:%05X VRAM : %04X\n", port,
      DMA_address+(DMA_bank<<16), PPU_port, DMA_len, SNES.PPU_Port[0x16]);*/

  ADD_CYCLES (DMA_len + (DMA_len >> 2));

  if (PPU_port == 0x2118 && DMA_info == 1)
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
  SNES.DMA_Port[0x106+port*0x10] = SNES.DMA_Port[0x105+port*0x10] = 0;
  SNES.DMA_Port[0x102+port*0x10] = DMA_address&0xff;
  SNES.DMA_Port[0x103+port*0x10] = DMA_address>>8;
  END_PROFILE(DMA, 4);
}


void		HDMA_transfert(unsigned char port)
{
  uint		len;
  uchar		*ptr, *ptr2, repeat;
  ushort	tmp=0;

  START_PROFILE(DMA, 4);
  SNES.HDMA_nblines[port] = 0;
  ptr = map_memory((SNES.DMA_Port[0x102+port*0x10])+(SNES.DMA_Port[0x103+port*0x10]<<8),
                    SNES.DMA_Port[0x104+port*0x10]);

  if (!ptr) {
/*    iprintf(" (invalid memory access during a H-DMA transfert : %06X)",
      SNES.DMA_Port[0x102+port*0x10]+(SNES.DMA_Port[0x103+port*0x10]<<8)+
      (SNES.DMA_Port[0x104+port*0x10]<<16));*/
      return;
//    exit(255);
  }

  SNES.HDMA_port[port] = SNES.DMA_Port[0x101+port*0x10];
  SNES.HDMA_info[port] = SNES.DMA_Port[0x100+port*0x10]&7;

  while(*ptr++ && tmp < GFX.ScreenHeight)
  {
    if (*(ptr-1) == 0x80) {
      len = MIN(128,GFX.ScreenHeight-tmp); repeat = 1;
    } else {
      len    = MIN(*(ptr-1)&0x7f,GFX.ScreenHeight-tmp);
      repeat = !(*(ptr-1)&0x80);
    }
    if (SNES.DMA_Port[0x100+port*0x10]&0x40) {
      ptr2 = map_memory(*ptr+(*(ptr+1)<<8), SNES.DMA_Port[0x107+port*0x10]);
      ptr += 2;
      switch (SNES.DMA_Port[0x100+port*0x10]&7) {
        case 0x00 :
          while (len--) {
            SNES.HDMA_values[port][tmp++] = ptr2; if (!repeat) ptr2++;
          } break;
        case 0x01 :
          while (len--) {
            SNES.HDMA_values[port][tmp++] = ptr2; if (!repeat) ptr2 += 2;
          } break;
        case 0x02 :
          while (len--) {
            SNES.HDMA_values[port][tmp++] = ptr2; if (!repeat) ptr2 += 2;
          } break;
        case 0x03 :
        while (len--) {
            SNES.HDMA_values[port][tmp++] = ptr2; if (!repeat) ptr2 += 4;
          } break;
        case 0x04 :
          while (len--) {
            SNES.HDMA_values[port][tmp++] = ptr2; if (!repeat) ptr2 += 4;
          } break;
      }
      continue;
    }
    switch (SNES.DMA_Port[0x100+port*0x10] & 7) {
      case 0x00 :
        while (len--) {
          SNES.HDMA_values[port][tmp++] = ptr; if (!repeat) ptr++;
        } if (repeat) ptr++; break;
      case 0x01 :
        while (len--) {
          SNES.HDMA_values[port][tmp++] = ptr; if (!repeat) ptr += 2;
        } if (repeat) ptr += 2; break;
      case 0x02 :
        while (len--) {
          SNES.HDMA_values[port][tmp++] = ptr; if (!repeat) ptr += 2;
        } if (repeat) ptr += 2; break;
      case 0x03 :
        while (len--) {
          SNES.HDMA_values[port][tmp++] = ptr; if (!repeat) ptr += 4;
        } if (repeat) ptr += 4; break;
      case 0x04 :
        while (len--) {
          SNES.HDMA_values[port][tmp++] = ptr; if (!repeat) ptr += 4;
        } if (repeat) ptr += 4; break;
    }
  }

  SNES.HDMA_nblines[port] = MIN(GFX.ScreenHeight, tmp);
  SNES.HDMA_line = 0;
  END_PROFILE(DMA, 4);
}

IN_ITCM2
void	DMA_port_write(uint address, uchar value)
{
  switch (address) {
    case 0x4016: if ((value&1) && !(SNES.PPU_Port[0x1F16]&1))
                   {
                     SNES.Joy1_cnt = 0;
                   }
                 SNES.PPU_Port[0x1F16] = value;
                 return;
    case 0x4017:
    		 return;
    case 0x4200: if (value & 0x10)
                   SNES.HIRQ_ok = 0;
                 break;
    case 0x4203:
      SNES.DMA_Port[0x16]=SNES.DMA_Port[0x02]*value;
      SNES.DMA_Port[0x17]=(SNES.DMA_Port[0x16]>>8);
      break;
    case 0x4206:
      if (value) {
        int tmp = (SNES.DMA_Port[0x05]<<8)+SNES.DMA_Port[0x04];
        SNES.DMA_Port[0x14]=tmp/value;
        SNES.DMA_Port[0x15]=SNES.DMA_Port[0x14]>>8;
        SNES.DMA_Port[0x16]=tmp%value;
        SNES.DMA_Port[0x17]=SNES.DMA_Port[0x16]>>8;
      } else { /* division par zero */
        SNES.DMA_Port[0x14] = SNES.DMA_Port[0x15] = 0xFF;
        SNES.DMA_Port[0x16] = SNES.DMA_Port[0x04];
        SNES.DMA_Port[0x17] = SNES.DMA_Port[0x05];
      }
      break;
	case 0x4207:
		SNES.HIRQ_value = (SNES.HIRQ_value&0xFF00) | value;
		break;
	case 0x4208:
		SNES.HIRQ_value = (SNES.HIRQ_value&0x00FF) | (value << 8);
		break;
    case 0x420B: if (value & 0x01) DMA_transfert(0);
                 if (value & 0x02) DMA_transfert(1);
                 if (value & 0x04) DMA_transfert(2);
                 if (value & 0x08) DMA_transfert(3);
                 if (value & 0x10) DMA_transfert(4);
                 if (value & 0x20) DMA_transfert(5);
                 if (value & 0x40) DMA_transfert(6);
                 if (value & 0x80) DMA_transfert(7);
                 break;
    case 0x420C: if (value & 0x01) HDMA_transfert(0);
                 if (value & 0x02) HDMA_transfert(1);
                 if (value & 0x04) HDMA_transfert(2);
                 if (value & 0x08) HDMA_transfert(3);
                 if (value & 0x10) HDMA_transfert(4);
                 if (value & 0x20) HDMA_transfert(5);
                 if (value & 0x40) HDMA_transfert(6);
                 if (value & 0x80) HDMA_transfert(7);
                 break;
  }
  if (address >= 0x4200)
    SNES.DMA_Port[address-0x4200] = value;
}

IN_ITCM2
uchar	DMA_port_read(uint address)
{
  switch (address) {
    case 0x4016 :
       {
         uchar tmp;

         if (SNES.PPU_Port[0x1F16]&1)
           return 0;
         tmp = SNES.joypads[0]>>(SNES.Joy1_cnt^15);
         SNES.Joy1_cnt++;
         return (tmp&1);
       }
    case 0x4017 :
      return 0x00;
    case 0x4210:
//FIXME
  	  if (HCYCLES < NB_CYCLES-6 && SNES.V_Count == GFX.ScreenHeight-1)    
     // if (Cycles >= CPU.Cycles-6 && SNES.V_Count == GFX.ScreenHeight-1)    
      {
        CPU.NMIActive = 1; SNES.DMA_Port[0x10] = 0; return 0x80;
      }
      SET_WAITCYCLESDELAY(0);
      if (SNES.V_Count == GFX.ScreenHeight-1) SET_WAITCYCLESDELAY(6);
      if (SNES.DMA_Port[0x10]&0x80) {
        SNES.DMA_Port[0x10] &= ~0x80; return 0x80;
      } break;
    case 0x4211:
      SET_WAITCYCLESDELAY(0);
      if (SNES.DMA_Port[0x11] & 0x80) {
        SNES.DMA_Port[0x11] &= ~0x80; return 0x80;
      } break;
    case 0x4212:
      SET_WAITCYCLESDELAY(0);
      if (HCYCLES < NB_CYCLES - 65) SET_WAITCYCLESDELAY(60);
      if (SNES.V_Count == GFX.ScreenHeight-1)
        SET_WAITCYCLESDELAY(6);
      SNES.DMA_Port[0x12] =
        SNES.V_Count >= GFX.ScreenHeight && SNES.V_Count < GFX.ScreenHeight+3;
	  // FiXME
//          if (CPU.Cycles*2 < 120)
	  if (HCYCLES > 30)
        SNES.DMA_Port[0x12] |= 0x40;
// FIXME            
	  if (SNES.v_blank || (HCYCLES < NB_CYCLES-6 && SNES.V_Count == GFX.ScreenHeight-1))
//      if (SNES.v_blank || (Cycles >= CPU.Cycles-6 && SNES.V_Count == GFX.ScreenHeight-1))     
          SNES.DMA_Port[0x12] |= 0x80;
      break;
  }
  if (address >= 0x4200)
    return SNES.DMA_Port[address-0x4200];
  else
    return 0;
}

IN_ITCM2
uchar	PPU_port_read(uint address)
{
  switch (address) {
    case 0x2121 :
      return (SNES.PPU_Port[0x21]>>1);
    case 0x2134 :
    case 0x2135 :
    case 0x2136 :
      if (SNES.PPU_NeedMultiply) {
        long result = (long)((short)(SNES.PPU_Port[0x1B])) *
                      (long)((short)(SNES.PPU_Port[0x1C])>>8);
//        long result = ((long)SNES.PPU_Port[0x1B]*(long)SNES.PPU_Port[0x1C])>>8;
        SNES.PPU_Port[0x34] = (result)&0xFF;
        SNES.PPU_Port[0x35] = (result>>8)&0xFF;
        SNES.PPU_Port[0x36] = (result>>16)&0xFF;
        SNES.PPU_NeedMultiply = 0;
      } break;
    case 0x2137 :
      SNES.PPU_Port[0x3C] = (HCYCLES)*9/5; // FIXME    	
      SNES.PPU_Port[0x3C] = (SNES.PPU_Port[0x3C]>>8) | (SNES.PPU_Port[0x3C]<<8);
      SNES.PPU_Port[0x3D] = SNES.V_Count;
      SNES.PPU_Port[0x3D] = (SNES.PPU_Port[0x3D]>>8) | (SNES.PPU_Port[0x3D]<<8);
      break;
    case 0x2138 :
      if ((SNES.PPU_Port[0x02]) >= 0x100) {
        if ((SNES.PPU_Port[0x02]) < 0x110) {
          if (GFX.OAM_upper_byte) {
            GFX.OAM_upper_byte = 0;
            SNES.PPU_Port[0x02]++;
            return GFX.spr_info_ext[((SNES.PPU_Port[0x02]-1)<<1)+1-0x200];
          } else {
            GFX.OAM_upper_byte = 1;
            return GFX.spr_info_ext[(SNES.PPU_Port[0x02]<<1)-0x200];
          }
        } else
          SNES.PPU_Port[0x02] = 0;
      } else {
        if (GFX.OAM_upper_byte) {
          GFX.OAM_upper_byte = 0;
          SNES.PPU_Port[0x02]++;
          return ((uchar *)GFX.spr_info)[((SNES.PPU_Port[0x02]-1)<<1)+1];
        } else {
          GFX.OAM_upper_byte = 1;
          return ((uchar *)GFX.spr_info)[(SNES.PPU_Port[0x02]<<1)];
        }
      } break;
    case 0x2139 :
         if (SNES.PPU_Port[0x15]&0x80) {
           return SNESC.VRAM[(SNES.PPU_Port[0x16]<<1)&0xFFFF];
         } else {
           long result = SNESC.VRAM[(SNES.PPU_Port[0x16]<<1)&0xFFFF];
           if (GFX.Dummy_VRAMRead) {
             GFX.Dummy_VRAMRead = 0;
           } else {
             SNES.PPU_Port[0x16] += GFX.SC_incr;
             if (GFX.FS_incr) {
                SNES.PPU_Port[0x16] += 8;
               if (++GFX.FS_cnt == GFX.FS_incr) {
                 GFX.FS_cnt = 0;
                 if (++GFX.FS_cnt2 == 8) {
                   GFX.FS_cnt2 = 0; SNES.PPU_Port[0x16] -= 8-GFX.SC_incr;
                 } else
                   SNES.PPU_Port[0x16] -= 8*GFX.FS_incr-GFX.SC_incr;
               }
             }
           }
           return result;
         } break;
    case 0x213A :
         if ((SNES.PPU_Port[0x15]&0x80) == 0) {
           return SNESC.VRAM[((SNES.PPU_Port[0x16]<<1)+1)&0xFFFF];
         } else {
           long result = SNESC.VRAM[((SNES.PPU_Port[0x16]<<1)+1)&0xFFFF];
           if (GFX.Dummy_VRAMRead) {
             GFX.Dummy_VRAMRead = 0;
           } else {
             SNES.PPU_Port[0x16] += GFX.SC_incr;
             if (GFX.FS_incr) {
               SNES.PPU_Port[0x16]+=8;
               if (++GFX.FS_cnt == GFX.FS_incr) {
                 GFX.FS_cnt = 0;
                 if (++GFX.FS_cnt2 == 8) {
                   GFX.FS_cnt2 = 0; SNES.PPU_Port[0x16]-=8-GFX.SC_incr;
                 }
                 else
                   SNES.PPU_Port[0x16]-=8*GFX.FS_incr-GFX.SC_incr;
               }
             }
           }
           return result;
         } break;
    case 0x213B :
         if (SNES.PPU_Port[0x21] == 0x200) SNES.PPU_Port[0x21]=0;
         if ((SNES.PPU_Port[0x21]&1) == 0) {
           GFX.CG_RAM_mem_temp =
             (GFX.SNESPal[SNES.PPU_Port[0x21]/2].b<<9)+
             (GFX.SNESPal[SNES.PPU_Port[0x21]/2].g<<4)+
             (GFX.SNESPal[SNES.PPU_Port[0x21]/2].r>>1);
           SNES.PPU_Port[0x3B] = GFX.CG_RAM_mem_temp&0xFF;
         } else {
           SNES.PPU_Port[0x3B] = GFX.CG_RAM_mem_temp>>8;
         }
         SNES.PPU_Port[0x21]++; break;
    case 0x213C :
      SNES.PPU_Port[0x3C] = (SNES.PPU_Port[0x3C]>>8)|(SNES.PPU_Port[0x3C]<<8); break;
    case 0x213D :
      SNES.PPU_Port[0x3D] = (SNES.PPU_Port[0x3D]>>8)|(SNES.PPU_Port[0x3D]<<8); break;
    case 0x213F :
      return (SNES.NTSC ? 0x01 : 0x11);
    case 0x2140 :
      if (CFG.Sound_output)
        return PORT_SPC_TO_SNES[0];
      else { /* APU Skipper */
        switch ((APU.skipper_cnt1++)%11) {
          case 0: return SNES.PPU_Port[0x40];
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
      } break;
    case 0x2141 :
      if (CFG.Sound_output)
        return PORT_SPC_TO_SNES[1];
      else {
        switch ((APU.skipper_cnt2++)%13) {
          case 0: return SNES.PPU_Port[0x41];
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
      } break;
    case 0x2142 :
      if (CFG.Sound_output)
        return PORT_SPC_TO_SNES[2];
      else {
        switch ((APU.skipper_cnt3++)%7) {
          case 0: return SNES.PPU_Port[0x42];
          case 1: return REAL_A;
          case 2: return X;
          case 3: return Y;
          case 4: return 0x00;
          case 5: return 0xAA;
          case 6: return 0xBB;
        }
      } break;
    case 0x2143 :
      if (CFG.Sound_output)
        return PORT_SPC_TO_SNES[3];
      else {
        switch((APU.skipper_cnt4++) % 9) {
          case 0: return SNES.PPU_Port[0x43];
          case 1: return REAL_A;
          case 2: return X;
          case 3: return Y;
          case 4: return 0x00;
          case 5: return 0xAA;
          case 6: return 0xBB;
          case 7: return 0x01;
          case 8: return REAL_A>>8;
        }
      } break;
    case 0x2180 :
      SNES.PPU_Port[0x80] =
        SNESC.RAM[SNES.PPU_Port[0x81]+(SNES.PPU_Port[0x82]<<8)+((SNES.PPU_Port[0x83]&1)<<16)];
      SNES.PPU_Port[0x81] = (SNES.PPU_Port[0x81]+1)&0xff;
      if (!SNES.PPU_Port[0x81]) {
        SNES.PPU_Port[0x82] = (SNES.PPU_Port[0x82]+1)&0xff; if (!SNES.PPU_Port[0x82]) SNES.PPU_Port[0x83]++;
      } break;

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

  if (address >= 0x2100)
    return SNES.PPU_Port[address-0x2100];
  return 0;
}

IN_ITCM2
void	PPU_port_write(uint address, uchar value)
{
  switch (address) {
    case 0x2100 : 
         if (GFX.Blank_Screen && ((value&0x80) == 0)) {
           GFX.Blank_Screen = 0; GFX.new_color = 255;
		   PPU_setScreen(value);           
           SNES.PPU_Port[0x00] = value; return;
         } else {
           GFX.Blank_Screen = (value&0x80) != 0;
           if ((value&0xf) != (SNES.PPU_Port[0x00]&0xf) && !GFX.Blank_Screen) {
             GFX.new_color = 255;
		     PPU_setScreen(value);             
             SNES.PPU_Port[0x00] = value; 	return;
           }
         } 
		 break;
    case 0x2101 :
    	 if (value != SNES.PPU_Port[0x01]) {
           GFX.spr_addr_base = (value&0x03)<<14;
           GFX.spr_addr_select = (value&0x18)<<10;
           check_sprite_addr();
         } break;
    case 0x2102 :
         SNES.PPU_Port[0x02] = (SNES.PPU_Port[0x02]&0x100)+value;
         GFX.Old_SpriteAddress = SNES.PPU_Port[0x02];
         if (SNES.PPU_Port[0x03]&0x80)
           GFX.HighestSprite = (SNES.PPU_Port[0x02]>>1)&0x7f;
         GFX.OAM_upper_byte = 0;
           GFX.Sprites_table_dirty = 1;         
         return;
    case 0x2103:
         SNES.PPU_Port[0x02] = (SNES.PPU_Port[0x02]&0xff)+(value&1)*0x100;
         if (SNES.PPU_Port[0x02] >= 0x110)
           SNES.PPU_Port[0x02] %= 0x110;
         GFX.Old_SpriteAddress = SNES.PPU_Port[0x02];
         GFX.HighestSprite = (SNES.PPU_Port[0x02]>>1)&0x7f;
         GFX.OAM_upper_byte = 0;
         GFX.Sprites_table_dirty = 1;         
         break;
    case 0x2104:
         if ((SNES.PPU_Port[0x02]) >= 0x100) {
           if (GFX.OAM_upper_byte) {
             if (GFX.spr_info_ext[(SNES.PPU_Port[0x02]<<1)+1-0x200] != value)
               {
                 GFX.spr_info_ext[(SNES.PPU_Port[0x02]<<1)+1-0x200] = value;
                 GFX.Sprites_table_dirty = 1;
               }
             SNES.PPU_Port[0x02]++;
             if (SNES.PPU_Port[0x02] == 0x110)
               SNES.PPU_Port[0x02] = 0;
             GFX.OAM_upper_byte = 0;
           } else {
             if (GFX.spr_info_ext[(SNES.PPU_Port[0x02]<<1)-0x200] != value)
               {
                 GFX.spr_info_ext[(SNES.PPU_Port[0x02]<<1)-0x200] = value;
                 GFX.Sprites_table_dirty = 1;
               }
             GFX.OAM_upper_byte = 1;
           }
         } else {
           if (GFX.OAM_upper_byte) {
             if (((uchar *)GFX.spr_info)[(SNES.PPU_Port[0x02]<<1)+1] != value)
               {
                 ((uchar *)GFX.spr_info)[(SNES.PPU_Port[0x02]<<1)+1] = value;
                 GFX.Sprites_table_dirty = 1;
               }
             SNES.PPU_Port[0x02]++;
             GFX.OAM_upper_byte = 0;
           } else {
             if (((uchar *)GFX.spr_info)[(SNES.PPU_Port[0x02]<<1)] != value)
               {
                 ((uchar *)GFX.spr_info)[(SNES.PPU_Port[0x02]<<1)] = value;
                 GFX.Sprites_table_dirty = 1;
               }
             GFX.OAM_upper_byte = 1;
           }
         } break;
	case 0x2105 :
		//printf("Graph mode : %d\n", value); 
		break;
/*	case 0x2107 : 
    case 0x2108 : 
	case 0x2109 : 
    case 0x210A :  
//		printf("BG: %x SC mode : %d\n", address-0x2107, value&3); 
		break;*/
    case 0x210B :
          GFX.tile_address[0] = ((value&0x0f) << 0xd);
          GFX.tile_address[1] = ((value&0xf0) << 0x9);
          if (GFX.old_tile_address[0] != GFX.tile_address[0] ||
              GFX.old_tile_address[1] != GFX.tile_address[1])
          {
		  	check_tile_addr();
		  	GFX.old_tile_address[0] = GFX.tile_address[0];
            GFX.old_tile_address[1] = GFX.tile_address[1];
          }
          break;
    case 0x210C :
          GFX.tile_address[2] = ((value&0x0f) << 0xd);
          GFX.tile_address[3] = ((value&0xf0) << 0x9);
          if (GFX.old_tile_address[2] != GFX.tile_address[2] ||
              GFX.old_tile_address[3] != GFX.tile_address[3])
          {
		  	check_tile_addr();
		  	GFX.old_tile_address[2] = GFX.tile_address[2];
            GFX.old_tile_address[3] = GFX.tile_address[3];
          }
          break;
    case 0x210D :
         if ((GFX.BG_scroll_reg&0x01)==0) {
           GFX.old_scrollx[0] = SNES.PPU_Port[0x0D];
           SNES.PPU_Port[0x0D] = value; GFX.BG_scroll_reg |= 0x01;
         } else {
           SNES.PPU_Port[0x0D] += (value<<8); GFX.BG_scroll_reg &= ~0x01;
//           update_scrollx(0);
         } return;
   case 0x210E :
         if ((GFX.BG_scroll_reg&0x02)==0) {
           GFX.old_scrolly[0] = SNES.PPU_Port[0x0E];
           SNES.PPU_Port[0x0E] = value; GFX.BG_scroll_reg |= 0x02;
         } else {
           SNES.PPU_Port[0x0E] += (value<<8); GFX.BG_scroll_reg &= ~0x02;
//           update_scrolly(0);
         } return;
    case 0x210F :
         if ((GFX.BG_scroll_reg&0x04)==0) {
           GFX.old_scrollx[1] = SNES.PPU_Port[0x0F];
           SNES.PPU_Port[0x0F] = value; GFX.BG_scroll_reg |= 0x04;
         } else {
           SNES.PPU_Port[0x0F] += (value<<8); GFX.BG_scroll_reg &= ~0x04;
  //         update_scrollx(1);
         } return;
    case 0x2110 :
         if ((GFX.BG_scroll_reg&0x08)==0) {
           GFX.old_scrolly[1] = SNES.PPU_Port[0x10];
           SNES.PPU_Port[0x10] = value; GFX.BG_scroll_reg |= 0x08;
         } else {
           SNES.PPU_Port[0x10] += (value<<8); GFX.BG_scroll_reg &= ~0x08;
//           update_scrolly(1);
         } return;
    case 0x2111 :
         if ((GFX.BG_scroll_reg&0x10)==0) {
           GFX.old_scrollx[2] = SNES.PPU_Port[0x11];
           SNES.PPU_Port[0x11] = value; GFX.BG_scroll_reg |= 0x10;
         } else {
           SNES.PPU_Port[0x11] += (value<<8); GFX.BG_scroll_reg &= ~0x10;
//           update_scrollx(2);
         } return;
    case 0x2112 :
         if ((GFX.BG_scroll_reg&0x20)==0) {
           GFX.old_scrolly[2] = SNES.PPU_Port[0x12];
           SNES.PPU_Port[0x12] = value; GFX.BG_scroll_reg |= 0x20;
         } else {
           SNES.PPU_Port[0x12] += (value<<8); GFX.BG_scroll_reg &= ~0x20;
//           update_scrolly(2);
         } return;
    case 0x2113 :
         if ((GFX.BG_scroll_reg&0x40)==0) {
           SNES.PPU_Port[0x13] = value; GFX.BG_scroll_reg |= 0x40;
         } else {
           SNES.PPU_Port[0x13] += (value<<8); GFX.BG_scroll_reg &= ~0x40;
         } 
//         GFX.tiles_ry[3] = 8; return;
    case 0x2114 :
         if ((GFX.BG_scroll_reg&0x80)==0) {
           SNES.PPU_Port[0x14] = value; GFX.BG_scroll_reg |= 0x80;
         } else {
           SNES.PPU_Port[0x14] += (value<<8); GFX.BG_scroll_reg &= ~0x80;
         } 
//         GFX.tiles_ry[3] = 8; return;
    case 0x2115 :
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
         } break;
    case 0x2116 :
         SNES.PPU_Port[0x16] = (SNES.PPU_Port[0x16]&0xff00) + value;
         GFX.Dummy_VRAMRead = 1;
         return;
    case 0x2117 :
         SNES.PPU_Port[0x16] = (SNES.PPU_Port[0x16]&0xff) + (value << 8);
         GFX.Dummy_VRAMRead = 1;
         break;
    case 0x2118 :
   	 if (SNES.PPU_Port[0x15]&0x80) {
//           if (SNESC.VRAM[(SNES.PPU_Port[0x16]<<1)&0xFFFF] != value) check_tile();
           SNESC.VRAM[(SNES.PPU_Port[0x16]<<1)&0xFFFF] = value;
         } else {
           if (SNESC.VRAM[(SNES.PPU_Port[0x16]<<1)&0xFFFF] != value) check_tile();
           SNESC.VRAM[(SNES.PPU_Port[0x16]<<1)&0xFFFF] = value;
           SNES.PPU_Port[0x16] += GFX.SC_incr;
           if (GFX.FS_incr) {
             SNES.PPU_Port[0x16] += 8;
             if (++GFX.FS_cnt == GFX.FS_incr) {
               GFX.FS_cnt = 0;
               if (++GFX.FS_cnt2 == 8) {
                 GFX.FS_cnt2 = 0; SNES.PPU_Port[0x16] -= 7;
               } else
                 SNES.PPU_Port[0x16] -= 8*GFX.FS_incr-1;
             }
           }
         } break;
    case 0x2119 :
   	 if ((SNES.PPU_Port[0x15]&0x80) == 0) {
/*           if (SNESC.VRAM[((SNES.PPU_Port[0x16]<<1)+1)&0xFFFF] != value)
             check_tile();*/
           SNESC.VRAM[((SNES.PPU_Port[0x16]<<1)+1)&0xFFFF] = value;
         } else {
           if (SNESC.VRAM[((SNES.PPU_Port[0x16]<<1)+1)&0xFFFF] != value) check_tile();
           SNESC.VRAM[((SNES.PPU_Port[0x16]<<1)+1)&0xFFFF] = value;
           if (!GFX.FS_incr) {
             SNES.PPU_Port[0x16] += GFX.SC_incr;
           } else {
             SNES.PPU_Port[0x16] += 8;
             if (++GFX.FS_cnt == GFX.FS_incr) {
               GFX.FS_cnt = 0;
               if (++GFX.FS_cnt2 == 8) {
                 GFX.FS_cnt2 = 0; SNES.PPU_Port[0x16] -= 8-GFX.SC_incr;
               }
               else
                 SNES.PPU_Port[0x16] -= 8*GFX.FS_incr-GFX.SC_incr;
             }
           }
         } break;
    case 0x211B :
         SNES.PPU_Port[0x1B] = (SNES.PPU_Port[0x1B] >> 8) + (value << 8);
         SNES.PPU_NeedMultiply = 1;
         return;
    case 0x211C :
         SNES.PPU_Port[0x1C] = (SNES.PPU_Port[0x1C] >> 8) + (value << 8);
         SNES.PPU_NeedMultiply = 1;
         return;
    case 0x211D :
         SNES.PPU_Port[0x1D] = (SNES.PPU_Port[0x1D] >> 8) + (value << 8);
         return;
    case 0x211E :
         SNES.PPU_Port[0x1E] = (SNES.PPU_Port[0x1E] >> 8) + (value << 8);
         return;
    case 0x211F :
         SNES.PPU_Port[0x1F] = (SNES.PPU_Port[0x1F] >> 8) + (value << 8);
         return;
    case 0x2120 :
         SNES.PPU_Port[0x20] = (SNES.PPU_Port[0x20] >> 8) + (value << 8);
         return;
    case 0x2121 :
         SNES.PPU_Port[0x21] = (value << 1); return;
    case 0x2122 :
         if (SNES.PPU_Port[0x21] == 0x200) SNES.PPU_Port[0x21]=0;
         if ((SNES.PPU_Port[0x21]&1) == 0)
           GFX.CG_RAM_mem_temp = value;
         else {
           RealColor p;
           GFX.CG_RAM_mem_temp = (GFX.CG_RAM_mem_temp&0xff)+(value<<8);
           p.b = ((GFX.CG_RAM_mem_temp>>10)&0x1f)*2;
           p.g = ((GFX.CG_RAM_mem_temp>>5)&0x1f)*2;
           p.r = ((GFX.CG_RAM_mem_temp)&0x1f)*2;
           p.t = 0;
           if (p.b != GFX.SNESPal[SNES.PPU_Port[0x21]/2].b ||
               p.g != GFX.SNESPal[SNES.PPU_Port[0x21]/2].g ||
               p.r != GFX.SNESPal[SNES.PPU_Port[0x21]/2].r) {
             GFX.SNESPal[SNES.PPU_Port[0x21]/2] = p;
             GFX.new_color |= 1; GFX.new_colors[SNES.PPU_Port[0x21]/2] = 1;
		     PPU_setPalette(SNES.PPU_Port[0x21] >> 1, p.r/2, p.g/2, p.b/2);
           }
         }; SNES.PPU_Port[0x21]++; break;
    case 0x211A :
	 SNES.Mode7Repeat = value>>6;
/*	 SNES.Mode7VFlip = (Byte & 2) >> 1;
	 SNES.Mode7HFlip = Byte & 1;*/
         break;
    case 0x2132 :
         if ((value & 0x80)) GFX.BACK.b = ((value&0x1f)<<1);
         if ((value & 0x40)) GFX.BACK.g = ((value&0x1f)<<1);
         if ((value & 0x20)) GFX.BACK.r = ((value&0x1f)<<1);
         if (SNES.PPU_Port[0x31]&0x20) {
           if (!GFX.BACK.b && !GFX.BACK.g && !GFX.BACK.r) {
             GFX.new_colors[0] = 1;
           }
           GFX.new_color |= 1;
         } else {
           GFX.new_colors[0] = 1;
           GFX.new_color |= 1;
         }
         break;
    case 0x2133 :
         GFX.ScreenHeight = (value&4)?240:224; break;
    case 0x2140 :
    	   if (CFG.Sound_output)
    	     PORT_SNES_TO_SPC[0] = value;
    	   else
    	     SNES.PPU_Port[0x40] = value;  
//         APU.MEM[0xF4] = value;
//         if (CFG.Sound_output) { SPC700_emu = 1; APU_WaitCounter++; } break;
    	   break;
    case 0x2141 :
    	   if (CFG.Sound_output)
    	     PORT_SNES_TO_SPC[1] = value;
    	   else
    	     SNES.PPU_Port[0x41] = value;  
//         APU.MEM[0xF5] = value;
//         if (CFG.Sound_output) { SPC700_emu = 1; APU_WaitCounter++; } break;
    	   break;
    case 0x2142 :
    	   if (CFG.Sound_output)
    	     PORT_SNES_TO_SPC[2] = value;
    	   else
    	     SNES.PPU_Port[0x42] = value;  
//         APU.MEM[0xF6] = value;
//         if (CFG.Sound_output) { SPC700_emu = 1; APU_WaitCounter++; } break;
		   break;
    case 0x2143 :
    	   if (CFG.Sound_output)
    	     PORT_SNES_TO_SPC[3] = value;
    	   else
    	     SNES.PPU_Port[0x43] = value;  
//         APU.MEM[0xF7] = value;
//         if (CFG.Sound_output) { SPC700_emu = 1; APU_WaitCounter++; } break;
		   break;
    case 0x2180 :
      SNESC.RAM[SNES.PPU_Port[0x81]+(SNES.PPU_Port[0x82]<<8)+((SNES.PPU_Port[0x83]&1)<<16)] = value;
      SNES.PPU_Port[0x81] = (SNES.PPU_Port[0x81]+1)&0xff;
      if (!SNES.PPU_Port[0x81]) {
        SNES.PPU_Port[0x82] = (SNES.PPU_Port[0x82]+1)&0xff;
        if (!SNES.PPU_Port[0x82]) SNES.PPU_Port[0x83]++;
      } return;
   
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
  if (address >= 0x2100)
    SNES.PPU_Port[address-0x2100]   = value;
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

  if (CFG.CPU_log) fprintf(SNES.flog, "--> NMI\n");
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
  
  SNES.DMA_Port[0x11] = 0x80;
  if (CFG.CPU_log) fprintf(SNES.flog, "--> IRQ\n");
}

#define HDMA_getbyte(N) *(SNES.HDMA_values[port][SNES.HDMA_line]+N)

void HDMA_write(uchar port)
{
  START_PROFILE(DMA, 4);	
  switch(SNES.HDMA_info[port])
  {
    case 0x00 :
      SNES.UsedCycles += 1;
      PPU_port_write(0x2100+SNES.HDMA_port[port], HDMA_getbyte(0)); break;
    case 0x01 :
      SNES.UsedCycles += 3;
      PPU_port_write(0x2100+SNES.HDMA_port[port], HDMA_getbyte(0));
      PPU_port_write(0x2101+SNES.HDMA_port[port], HDMA_getbyte(1)); break;
    case 0x02 :
      SNES.UsedCycles += 3;
      PPU_port_write(0x2100+SNES.HDMA_port[port], HDMA_getbyte(0));
      PPU_port_write(0x2100+SNES.HDMA_port[port], HDMA_getbyte(1)); break;
    case 0x03 :
      SNES.UsedCycles += 6;
      PPU_port_write(0x2100+SNES.HDMA_port[port], HDMA_getbyte(0));
      PPU_port_write(0x2100+SNES.HDMA_port[port], HDMA_getbyte(1));
      PPU_port_write(0x2101+SNES.HDMA_port[port], HDMA_getbyte(2));
      PPU_port_write(0x2101+SNES.HDMA_port[port], HDMA_getbyte(3)); break;
    case 0x04 :
      SNES.UsedCycles += 6;
      PPU_port_write(0x2100+SNES.HDMA_port[port], HDMA_getbyte(0));
      PPU_port_write(0x2101+SNES.HDMA_port[port], HDMA_getbyte(1));
      PPU_port_write(0x2102+SNES.HDMA_port[port], HDMA_getbyte(2));
      PPU_port_write(0x2103+SNES.HDMA_port[port], HDMA_getbyte(3)); break;
  }
  END_PROFILE(DMA, 4);  
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

#ifdef WIN32
	poll_keyboard();
	if (key[KEY_D])       SNES.joypads[0] |= 0x80;
	if (key[KEY_S])       SNES.joypads[0] |= 0x40;
	if (key[KEY_A])       SNES.joypads[0] |= 0x20;
	if (key[KEY_Z])       SNES.joypads[0] |= 0x10;
	if (key[KEY_C])       SNES.joypads[0] |= 0x8000;
	if (key[KEY_X])       SNES.joypads[0] |= 0x4000;
	if (key[KEY_LSHIFT])  SNES.joypads[0] |= 0x2000;
	if (key[KEY_LCONTROL]) SNES.joypads[0] |= 0x1000;
	if (key[KEY_UP])   	  SNES.joypads[0] |= 0x0800;
	if (key[KEY_DOWN])    SNES.joypads[0] |= 0x0400;
	if (key[KEY_LEFT])    SNES.joypads[0] |= 0x0200;
	if (key[KEY_RIGHT])   SNES.joypads[0] |= 0x0100;
#else
	SNES.joypads[0] = get_joypad();
#endif

  SNES.joypads[0] |= 0x80000000;
}

void	read_mouse()
{
  int	tmp;
    
/*  if (SNES.Controller == SNES_MOUSE)
    {*/
      int delta_x, delta_y;

#define MOUSE_SIGNATURE 0x1
      tmp = 0x1|/*(SNES.mouse_speed<<4)|*/
            ((SNES.mouse_b&1)<<6)|((SNES.mouse_b&2)<<6);
      delta_x = SNES.mouse_x-SNES.prev_mouse_x;
      delta_y = SNES.mouse_y-SNES.prev_mouse_y;

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

        SNES.PPU_Port[0x3C] = x;
        SNES.PPU_Port[0x3C] = (SNES.PPU_Port[0x3C]>>8) | (SNES.PPU_Port[0x3C]<<8);
        SNES.PPU_Port[0x3D] = y+1;
        SNES.PPU_Port[0x3D] = (SNES.PPU_Port[0x3D]>>8) | (SNES.PPU_Port[0x3D]<<8);

	SNES.PPU_Port[0x3F] |= 0x40;
	SNES.joypads[1] = scope;
    }
}

void	update_joypads()
{
  read_joypads();	
  if (SNES.DMA_Port[0x00]&1)
    {
//      read_joypads();
      if (CFG.mouse)
        read_mouse();
      if (CFG.scope)
        read_scope();

      SNES.Joy1_cnt = 0x10;

      SNES.DMA_Port[0x18] = SNES.joypads[0];
      SNES.DMA_Port[0x19] = SNES.joypads[0]>>8;
      SNES.DMA_Port[0x1A] = SNES.joypads[1];
      SNES.DMA_Port[0x1B] = SNES.joypads[1]>>8;
    }
}

void SNES_update()
{ 
  int value;
  
  value = SNES.PPU_Port[0x01];
  GFX.spr_addr_base = (value&0x03)<<14;
  GFX.spr_addr_select = (value&0x18)<<10;
  
  value = SNES.PPU_Port[0x0B];
  GFX.tile_address[0] = ((value&0x0f) << 0xd);
  GFX.tile_address[1] = ((value&0xf0) << 0x9);
  value = SNES.PPU_Port[0x0C];  
  GFX.tile_address[2] = ((value&0x0f) << 0xd);
  GFX.tile_address[3] = ((value&0xf0) << 0x9);
  
	
}

