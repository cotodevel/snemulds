/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* Free for non-commercial use                             */
/***********************************************************/

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


/* timers */
  int           T0, T1, T2;
  int		TIM0, TIM1, TIM2;
  int		T0_LATCH, T1_LATCH, T2_LATCH;
  uchar		CNT0, CNT1, CNT2;

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

#define PORT_SNES_TO_SPC ((volatile uint8*)(0x027FFFF8))
#define PORT_SPC_TO_SNES ((volatile uint8*)(0x027FFFFC))

#define APU_RAM_ADDRESS ((uint8*)(0x27EE000))

#define APU_ADDR_CNT ((volatile uint32*)(0x2800000-60))
#define APU_ADDR_CMD ((volatile uint32*)(0x2800000-16))
#define APU_ADDR_ANS ((volatile uint32*)(0x2800000-20))

#endif