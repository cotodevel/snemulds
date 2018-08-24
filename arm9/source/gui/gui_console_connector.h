/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

*/

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


#ifndef __gui_console_snemulds_h__
#define __gui_console_snemulds_h__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <malloc.h>
#include <ctype.h>
#include "common.h"
#include "specific_shared.h"

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "consoleTGDS.h"
#include "core.h"
#include "opcodes.h"
#include "snapshot.h"
#include "ppu.h"

#include "guiTGDS.h"
#include "fs.h"
#include "common.h"

#include "gfx.h"
#include "cfg.h"
#include "apu.h"
#include "biosTGDS.h"

#include "conf.h"
#include "gui_widgets.h"
#include "InterruptsARMCores_h.h"
#include "about.h"
#include "dmaTGDS.h"

#include "posixHandleTGDS.h"
#include "fsfatlayerTGDS.h"
#include "keypadTGDS.h"
#include "videoTGDS.h"




#define IDS_INITIALIZATION	0
#define IDS_FS_FAILED		1
#define IDS_FS_SUCCESS		2

#define IDS_OK				10
#define IDS_CANCEL			11
#define IDS_APPLY			12
#define IDS_SAVE			13

#define IDS_SELECT_ROM		20
#define IDS_LOAD_STATE		21
#define IDS_SAVE_STATE		22
#define IDS_OPTIONS			23
#define IDS_JUKEBOX			24
#define IDS_ADVANCED		25

#define IDS_RESET			27
#define IDS_SAVE_SRAM		28

#define IDS_SOUND			30

#define IDS_SPEED			32

#define IDS_SCREEN			36
#define IDS_LAYERS			37

#define IDS_HACKS			38

#define IDS_HUD				44
#define IDS_YSCROLL			49
#define IDS_SCALING			54

#define IDS_LAYERS_TITLE	60

#define IDS_LAYERS_HELP		62
#define IDS_AUTO_ORDER		65
#define IDS_LAYER			66
#define IDS_SPRITES			67

#define IDS_OFF				69
#define IDS_DIGIT			70

#define IDS_CHECK			80
#define IDS_AUTO_SRAM		82

#define IDS_USE_MEM_PACK	85

#define IDS_TITLE			90
#define IDS_SIZE			91
#define IDS_ROM_TYPE		92
#define IDS_COUNTRY			93

#define IDS_GFX_CONFIG		96
#define IDS_PRIO_PER_TILE	97

#define IDS_NONE			98
#define IDS_BG1				99
#define IDS_BG2				100

#define IDS_BLOCK_PRIO		101

#define IDS_GC_ON			102
#define IDS_GC_OFF			103

#define IDS_BLANK_TILE		104
#define IDS_FIX_GRAPHICS	105
#define IDS_GC_BG			106
#define IDS_GC_BG_LOW		107
#define IDS_GC_SPRITES		108

#define IDS_MULTIPLAYER_MODE		109



#endif

#ifdef __cplusplus
extern "C" {
#endif

extern t_GUIScreen *buildGFXConfigMenu();
extern int GUI_getConfigInt(sint8 *objname, sint8 *field, int val);
extern sint8 *GUI_getConfigStr(sint8 *objname, sint8 *field, sint8 *str);

//simple hook for writing values to volatile config file loaded earlier from setting file. Does not update config file.
extern void GUI_setConfigStr(sint8 *objname, sint8 *field, sint8 *value);

extern void GUI_setConfigStrUpdateFile(sint8 *objname, sint8 *field, sint8 *value);
extern void GUI_setObjFromConfig(t_GUIScreen *scr, int nb, sint8 *objname);
extern t_GUIScreen *buildMainMenu();
extern int ROMSelectorHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern int SPCSelectorHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern int LoadStateHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern int SaveStateHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern int GFXConfigHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern int AdvancedHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern int LayersOptionsHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern t_GUIScreen *buildLayersMenu();
extern int ScreenOptionsHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern t_GUIScreen *buildScreenMenu();
extern int OptionsHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern t_GUIScreen *buildOptionsMenu();
extern int MainScreenHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern int FirstROMSelectorHandler(t_GUIZone *zone, int msg, int param, void *arg);
//read rom from (path)touchscreen:output rom -> CFG.ROMFile
extern void GUI_getROM(sint8 *rompath);
extern void GUI_deleteROMSelector();
extern void GUI_createMainMenu();
extern void GUI_getConfig();
extern void	GUI_showROMInfos(int size);


//Definition that overrides the weaksymbol expected from toolchain to init console video subsystem
extern vramSetup * getProjectSpecificVRAMSetup();

//Custom console VRAM layout setup

//1) VRAM Layout
extern bool InitProjectSpecificConsole();

//2) Uses subEngine: VRAM Layout -> Console Setup
extern vramSetup * SNEMULDS_2DVRAM_SETUP();



extern sint8*  g_snemulds_str_jpn[];
extern sint8*  g_snemulds_str_eng[];
extern sint8*  g_snemulds_str_fr[];
extern sint8*  g_snemulds_str_ger[];
extern sint8*  g_snemulds_str_ita[];
extern sint8*  g_snemulds_str_spa[];
extern sint8*  g_snemulds_str_pt[];
extern sint8*  g_snemulds_str_cat[];
extern sint8*  g_snemulds_str_pol[];
extern sint8*  g_snemulds_str_nl[];
extern sint8*  g_snemulds_str_dan[];

extern int selectSong(sint8 *name);
extern void	CPU_unpack();
extern void	SNES_update();
extern void	PPU_update();
extern void	CPU_pack();
extern int loadSRAM();
extern int saveSRAM();
extern void PPU_ChangeLayerConf(int i);
extern void saveOptionsToConfig(sint8 *section);
extern void	APU_clear();
extern void GUI_setLanguage(int lang);
extern int GUI_getStrWidth(t_GUIZone *zone, sint8 *text);
extern int		GUI_getZoneTextHeight(t_GUIZone *zone);
extern int GUI_drawAlignText(t_GUIZone *zone, int flags, int y, int col, sint8 *text);
extern void		GUI_printf2(int cx, int cy, sint8 *fmt, ...);
extern void		GUI_align_printf(int flags, sint8 *fmt, ...);

#ifdef __cplusplus
}
#endif