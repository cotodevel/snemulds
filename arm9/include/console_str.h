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
#define IDS_RESETCFG		109

#ifdef __cplusplus
extern "C" {
#endif

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

extern void	CPU_unpack();
extern void	SNES_update();
extern void	PPU_update();
extern void	CPU_pack();
extern void PPU_ChangeLayerConf(int i);
extern void saveOptionsToConfig(sint8 *section);
extern void	APU_clear();
extern void GUI_setLanguage(int lang);


#ifdef __cplusplus
}
#endif