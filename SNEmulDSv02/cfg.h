/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* Free for non-commercial use                             */
/***********************************************************/

#ifndef __cfg_h__
#define __cfg_h__

#include "common.h"

struct s_cfg
{
  int	frame_rate;
  int	GUI_frame_rate;
  int	log_file;
  int	CPU_log;
  int	SPC_log;
  int	BG_Layer;
  int	BG_priority;
  int	PPU_Clip;

  int	DSP1;
  int	SuperFX;
  int	Sound_output;
  int	ADSR_GAIN;
  int	Stereo;
  int	FastSound;
  int	song_log;
  int	buffer_size;

  int	joypad_disabled; /* used to disable the joypad in the GUI */
  int	Joy1_Enabled;
  int	joy_type;
  int	mouse;
  int	scope;
  uchar	joypad_def[13], joypad_def2[13];

/* GUI */
  int	FullScr;
  int	FullGUI;
  int	FullScreen_resol;
  int	GUI_resol;
  int	Scanlines;
  int   auto_skip;
  int	ShowFPS;
  int	Timing;

  int	CPU_speedhack;
  int	SPC_speedhack;

  int	Debug, Debug2;

  char	*Work_dir;

  int	InterleavedROM;
  int	InterleavedROM2;
  
  int	BG3Squish;
  int	YScroll;
  int	WaitVBlank;
  
  int	LargeROM;
  char	*ROMFile;
};

extern struct s_cfg	CFG;

#endif
