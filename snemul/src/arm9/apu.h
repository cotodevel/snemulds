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

#ifndef __apu_h__
#define __apu_h__

#include "common.h"



struct s_apu
{
#if 0	
  uchar		*MEM;
  uchar		DSP[0x80];
  int		Port1, Port2, Port3, Port4;
  uchar		CONTROL, DSP_address;

  uint32	T0_LATCH, T1_LATCH, T2_LATCH;

/* sound */
  int		MasterVolume, MasterPanning;
  uchar		DSP_channel[8];

/* sample */
  uchar		sample_nb[256];
  ushort	CheckSum[256], Sample_len[256];
  uchar		sample_cnt;
  uchar         need_decode[8];
  short         samp[0x8000];

/* voice */
  long		Voice_pos[8], old_Voice_pos[8];
  uchar		Voice_envx[8], Voice_start_envx[8];
  uchar		Voice_mode[8];
  uchar		Voice_volume[8];
  uchar  	Voice_SL[8];
  ushort	Voice_AR[8], Voice_DR[8], Voice_SR[8];
  int           Voice_envpos[8];
#endif  

  int	    skipper_cnt1;
  int	    skipper_cnt2;
  int	    skipper_cnt3;
  int	    skipper_cnt4;
  
  int		counter;
};

extern struct s_apu	APU;

struct s_apu2
{
/* timers */
  uint32    T0, T1, T2;			// 0x27ED000 
  uint32 	TIM0, TIM1, TIM2;	/* 0x27ED00C */
  uint32	CNT0, CNT1, CNT2;	/* 0x27ED018 */
};

extern struct s_apu2 *APU2;


#define PORT_SNES_TO_SPC ((volatile uint8*)(0x027FFFF8))
#define PORT_SPC_TO_SNES ((volatile uint8*)(0x027FFFFC))

#define APU_RAM_ADDRESS ((uint8*)(0x27EE000))

#define APU_ADDR_CNT ((volatile uint32*)(0x2800000-60))
#define APU_ADDR_CMD ((volatile uint32*)(0x2800000-16))
#define APU_ADDR_ANS ((volatile uint32*)(0x2800000-20))
#define APU_ADDR_BLK ((volatile uint32*)(0x2800000-24))
#define APU_ADDR_BLKP ((volatile uint8*)(0x2800000-24))

void SendArm7Command(u32 command);

#endif
