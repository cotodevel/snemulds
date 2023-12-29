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

#ifndef GUI_H_
#define GUI_H_

#include <stdio.h>
#include <string.h>
#include "typedefsTGDS.h"
#include "consoleTGDS.h"
#include "videoTGDS.h"
#include "timerTGDS.h"
#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"
#include "dmaTGDS.h"
#include "ipcfifoTGDSUser.h"
/* --------------------------------------------------------------------------- */

typedef struct
{
	int x;
	int y;
	
	int	dx;
	int dy;
} t_GUIStylusEvent;

typedef struct
{
	uint32	buttons;
	uint32  pressed;
	uint32	repeated;
	uint32	released;
} t_GUIJoypadEvent;

typedef struct
{
	int	event;
	union
	{
		t_GUIStylusEvent stl;
		t_GUIJoypadEvent joy;
	};
		
} t_GUIEvent;

typedef struct
{
	t_GUIScreen	*scr;
	int			msg;
	int 		param;
	void		*arg;
} t_GUIMessage;

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


#ifdef __cplusplus
extern "C" {
#endif

extern char startFilePath[MAX_TGDSFILENAME_LENGTH+1];
extern char startSPCFilePath[MAX_TGDSFILENAME_LENGTH+1];

extern t_GUIScreen	*GUI_newScreen(int nb_elems);
extern void	GUI_setZone(t_GUIScreen *scr, int i,int x1, int y1, int x2, int y2);
extern void	GUI_linkObject(t_GUIScreen *scr, int i, void *data, t_GUIHandler handler);
extern int GUI_loadPalette(sint8 *path);
extern t_GUIImage	*GUI_loadImage(sint8 *path, int width, int height, int flags);
extern int GUI_addImage(sint8 *path, int w, int h, int flags);
extern void		GUI_deleteImage(t_GUIImage *image);
extern void		GUI_drawHLine(t_GUIZone *zone, int color, int x1, int y1, int x2);
extern void		GUI_drawVLine(t_GUIZone *zone, int color, int x1, int y1, int y2);
extern void		GUI_drawRect(t_GUIZone *zone, int color, int x1, int y1, int x2, int y2);
extern void		GUI_drawBar(t_GUIZone *zone, int color, int x1, int y1, int x2, int y2);
extern void		GUI_drawImage(t_GUIZone *zone, t_GUIImage *image, int x, int y);
extern int		GUI_sendMessage(t_GUIScreen *scr, int i, int msg, int param, void *arg);
extern t_GUIMessage	PendingMessage;
extern int			GUI_dispatchMessageNow(t_GUIScreen *scr, int msg, int param, void *arg);
extern int			GUI_dispatchMessage(t_GUIScreen *scr, int msg, int param, void *arg);
extern int		GUI_setFocus(t_GUIScreen *scr, int id);
extern void	GUI_clearFocus(t_GUIScreen *scr);
extern int		GUI_dispatchEvent(t_GUIScreen *scr, int event, void *param);
extern void		GUI_drawScreen(t_GUIScreen *scr, void *param);
extern t_GUIEvent	g_event;
extern int GUI_update();
extern int		GUI_start();
extern t_GUIImgList	*GUI_newImageList(int nb);
extern int		GUI_switchScreen(t_GUIScreen *scr);
extern int sort_strcmp(const void *a, const void *b);
extern t_GUIScreen	*scr_main;
extern void GUI_buildCStatic(t_GUIScreen *scr, int nb, int x, int y, int sx, int str);
extern void GUI_buildLStatic(t_GUIScreen *scr, int nb, int x, int y, int sx, int str, int arg);
extern void GUI_buildRStatic(t_GUIScreen *scr, int nb, int x, int y, int sx, int str, int arg);
extern void GUI_buildChoice(t_GUIScreen *scr, int nb, int x, int y, int sx, int str, int cnt, int val);
extern t_GUIScreen *buildMenu(int nb_elems, int flags, t_GUIFont *font, t_GUIFont *font_2);

//simple hook for writing values to volatile config file loaded earlier from setting file. Updates config file.
extern void GUI_setConfigStrUpdateFile(sint8 *objname, sint8 *field, sint8 *value);
extern void GUI_setLanguage(int lang);
extern struct sGUISelectorItem guiSelItem;

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
extern int selectSong(sint8 *name);		//Jukebox load SPC method
extern void	CPU_unpack();
extern void	SNES_update();
extern void	CPU_pack();
extern void PPU_ChangeLayerConf(int i);
extern void saveOptionsToConfig(sint8 *section);
extern void	APU_clear();
extern int		GUI_getZoneTextHeight(t_GUIZone *zone);
extern int GUI_drawAlignText(t_GUIZone *zone, int flags, int y, int col, sint8 *text);

extern void switchToTGDSConsoleColors();
extern void switchToSnemulDSConsoleColors();

#ifdef __cplusplus
}
#endif

#endif /*GUI_H_*/

