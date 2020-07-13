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

#include "consoleTGDS.h"
#include "videoTGDS.h"
#include "dswnifi_lib.h"
#include "dswnifi.h"
#include "fs.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"
#include "conf.h"
#include "spifwTGDS.h"
#include "main.h"
#include "timerTGDS.h"
#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"
#include "dmaTGDS.h"
#include "ipcfifoTGDSUser.h"

//GUI defs

#define GUI_EVENT			1
#define GUI_DRAW			2
#define GUI_COMMAND			3

#define GUI_EVENT_STYLUS	100
#define GUI_EVENT_BUTTON	101

#define GUI_EVENT_ENTERZONE	110
#define GUI_EVENT_LEAVEZONE	111
#define	GUI_EVENT_FOCUS		112
#define	GUI_EVENT_UNFOCUS	113

#define	EVENT_STYLUS_PRESSED 	1000
#define	EVENT_STYLUS_RELEASED	1001
#define	EVENT_STYLUS_DRAGGED 	1002

#define	EVENT_BUTTON_ANY	 	2000
#define	EVENT_BUTTON_PRESSED	2001
#define	EVENT_BUTTON_RELEASED	2002
#define	EVENT_BUTTON_HELD	 	2003

#define IMG_IN_MEMORY	0
#define IMG_IN_VRAM		1
#define IMG_NOLOAD		2

#define GUI_TEXT_ALIGN_CENTER	0
#define GUI_TEXT_ALIGN_LEFT		1
#define GUI_TEXT_ALIGN_RIGHT	2

#define GUI_ST_PRESSED			1
#define GUI_ST_SELECTED			2
#define GUI_ST_FOCUSED			4
#define GUI_ST_HIDDEN			8
#define GUI_ST_DISABLED			16

#define GUI_HANDLE_JOYPAD		1

#define GUI_PARAM(a) (void *)(a)
#define GUI_PARAM2(a, b) (void *)((((uint16)(a)) << 16) | ((uint16)(b)))
#define GUI_PARAM3(s, n, c) (void *)((s) | ((n) << 16) | ((c) << 24))

#define _STR(x) GUI.string[x]

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
extern vramSetup * SNEMULDS_2DVRAM_SETUP();

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
extern int		GUI_getZoneTextHeight(t_GUIZone *zone);
extern int GUI_drawAlignText(t_GUIZone *zone, int flags, int y, int col, sint8 *text);
extern void GUI_getROMFirstTime(sint8 *rompath);

#ifdef __cplusplus
}
#endif