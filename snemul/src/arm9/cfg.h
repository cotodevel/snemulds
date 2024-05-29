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

#ifndef __cfg_h__
#define __cfg_h__

#include "common.h"

struct s_cfg
{
  int	frame_rate;
  int	CPU_log;
  int	BG_Layer;
  int	BG_priority;

  int	DSP1;
  int	SuperFX;
  int	Sound_output;

  int	joypad_disabled; /* used to disable the joypad in the GUI */
  int	mouse;
  int	scope;
  //uchar	joypad_def[13], joypad_def2[13];

  int	CPU_speedhack;
//  int	SPC_speedhack;
  int	Timing;

  int	Debug, Debug2;

  int	ExtRAMSize;
  int	MapExtMem;
  
  int	AutoSRAM;

  int	InterleavedROM;
  int	InterleavedROM2;
  
  int	BG3Squish;
  int	YScroll;
  int	WaitVBlank;
  
  int	LargeROM;
  
  uint32	LayersConf;  
  uint8		LayerPr[4];
  uint8		SpritePr[4];
  
  uint32	Scaled;
  
  uint32	Transparency;
  uint32	FastDMA;
  
  uint16	MouseXAddr;
  uint16	MouseYAddr;
  sint16	MouseXOffset;
  sint16	MouseYOffset;  
  uint32	MouseMode;
  
  uint32	SoundPortSync;
  
  int		TilePriorityBG;
  int		BG3TilePriority;
  int		BG3PaletteFix;

  char		ROMFile[100];
  
  char		*ROMPath;
  
  int		Jukebox;  
  char		Playlist[100];

// GUI  
  sint8		GUISort;
  sint16	Language;
};

extern struct s_cfg	CFG;

void	packOptions(uint8 *ptr);
void	unpackOptions(int version, uint8 *ptr);

#endif
