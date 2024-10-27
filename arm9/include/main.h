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

#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"
#include "guiTGDS.h"
#include "gui_widgets.h"
#include "consoleTGDS.h"
#include "soundTGDS.h"

#endif

#ifdef __cplusplus
extern "C" {
#endif

//TGDS Soundstreaming API
extern int internalCodecType;
extern struct fd * _FileHandleVideo; 
extern struct fd * _FileHandleAudio;
extern bool stopSoundStreamUser();
extern void closeSoundUser();

extern int main(int argc, char **argv);
extern bool loadROM(char *name, int confirm);
extern bool handleROMSelect;
extern bool handleSPCSelect;
extern void ds_malloc_abortSkip(void);

extern char *GUI_getROMList(sint8 *rompath);
extern char *GUI_getSPCList(sint8 *spcpath);
extern char args[8][MAX_TGDSFILENAME_LENGTH];
extern char *argvs[8];
extern bool uninitializedEmu;
extern bool resetSnemulDSConfig();
extern void parseCFGFile();

#ifdef __cplusplus
}
#endif