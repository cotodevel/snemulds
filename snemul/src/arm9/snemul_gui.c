#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include "common.h"
#include "fs.h"

#include "fs.h"
#include "snes.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"

#include "conf.h"

#include "gui/gui.h"
#include "gui/gui_widgets.h"

#include "snemul_str.h"

#include <stdarg.h>

extern int _offsetY_tab[4];

extern t_GUIFont trebuchet_9_font;
extern t_GUIFont smallfont_7_font;

//extern char*  g_snemulds_str_jap[];
extern char*  g_snemulds_str_jpn[];
extern char*  g_snemulds_str_eng[];
extern char*  g_snemulds_str_fr[];
extern char*  g_snemulds_str_ger[];
extern char*  g_snemulds_str_ita[];
extern char*  g_snemulds_str_spa[];
extern char*  g_snemulds_str_pt[];
extern char*  g_snemulds_str_cat[];
extern char*  g_snemulds_str_pol[];
extern char*  g_snemulds_str_nl[];
extern char*  g_snemulds_str_dan[];

// Needed by qsort
static int sort_strcmp(char **a, char **b)
{
	return strcasecmp(*a, *b);
}

t_GUIScreen	*scr_main;

void GUI_buildCStatic(t_GUIScreen *scr, int nb, int x, int y, int sx, int str)
{
	GUI_setZone   (scr, nb, x, y, x+sx, y+16); 
	GUI_linkObject(scr, nb, (void *)str, GUIStatic_handler);	
}

void GUI_buildLStatic(t_GUIScreen *scr, int nb, int x, int y, int sx, int str, int arg)
{
	GUI_setZone   (scr, nb, x, y, x+sx, y+16); 
	GUI_linkObject(scr, nb, GUI_STATIC_LEFT(str, arg), GUIStaticEx_handler);	
}

void GUI_buildRStatic(t_GUIScreen *scr, int nb, int x, int y, int sx, int str, int arg)
{
	GUI_setZone   (scr, nb, x, y, x+sx, y+16); 
	GUI_linkObject(scr, nb, GUI_STATIC_RIGHT(str, arg), GUIStaticEx_handler);	
}

void GUI_buildChoice(t_GUIScreen *scr, int nb, int x, int y, int sx, int str, int cnt, int val)
{
	GUI_setZone   (scr, nb, x, y, x+sx, y+16); 
	GUI_linkObject(scr, nb, GUI_CHOICE(str, cnt, val), GUIChoiceButton_handler);	
}

t_GUIScreen *buildGFXConfigMenu()
{
	int y = 0;
	int sy;
	t_GUIScreen *scr = GUI_newScreen(40);
	
	GUI_buildCStatic(scr, 20, 0, 0, 256, IDS_GFX_CONFIG);

	GUI_setZone(scr, 0, 180, 20, 256, 80);
	GUI_linkObject(scr, 0, (void *)IDS_FIX_GRAPHICS, GUIStrButton_handler);
	
	y += 20;
	GUI_buildLStatic(scr, 21, 0, y, 90, IDS_PRIO_PER_TILE, 0);
	GUI_buildChoice (scr, 1, 92, y, 80, IDS_NONE, 3, CFG.TilePriorityBG+1);

	y += 18;
	GUI_buildLStatic(scr, 22, 0, y, 90, IDS_BLOCK_PRIO, 0);
	GUI_buildChoice (scr, 2, 92, y, 80, IDS_GC_ON, 2, CFG.BG3TilePriority);

	 y += 18;
	GUI_buildLStatic(scr, 23, 0, y, 90, IDS_BLANK_TILE, 0);
	GUI_buildChoice (scr, 3, 92, y, 24, IDS_DIGIT, 10, CFG.Debug2 / 100);
	GUI_buildChoice (scr, 4, 92+28, y, 24, IDS_DIGIT, 10, (CFG.Debug2 / 10)%10);
	GUI_buildChoice (scr, 5, 92+56, y, 24, IDS_DIGIT, 10, CFG.Debug2 % 10);	
	
	y += 18;
	
	GUI_buildLStatic(scr, 24, 24, y, 256, IDS_AUTO_ORDER, 0);
	GUI_buildChoice (scr, 6, 0, y, 16, IDS_CHECK, 2, CFG.LayersConf == 0);
	
	y += 18;
	sy = y;
	int i;
	for (i = 0; i < 3; i++)
	{
		GUI_buildLStatic(scr, 25+i, 0, y, 50, IDS_GC_BG, i);
		GUI_buildChoice (scr, 7+i, 60, y, 60, IDS_DIGIT, 2, CFG.LayerPr[i]);
		y += 18;		
	}
	GUI_buildLStatic(scr, 28, 0, y, 50, IDS_GC_BG_LOW, CFG.TilePriorityBG+1);
	GUI_buildChoice (scr, 10, 60, y, 60, IDS_DIGIT, 4, CFG.LayerPr[3]);
	y = sy;
	for (i = 0; i < 4; i++)
	{
		GUI_buildLStatic(scr, 29+i, 132, y, 50, IDS_GC_SPRITES, i);
		GUI_buildChoice (scr, 11+i, 192, y, 60, IDS_DIGIT, 4, CFG.SpritePr[i]);
		y += 18;		
	}

	
	for (i = 0; i < 40; i++)
	{
		scr->zones[i].font = &smallfont_7_font;
		scr->zones[i].keymask = KEY_A;
	}
	scr->zones[20].font = &trebuchet_9_font;
	
	// Three elements
	GUI_setZone(scr, 15, 0, 192-20, 0+88, 192);
	GUI_setZone(scr, 17, 88, 192-20, 88+80, 192);
	GUI_setZone(scr, 16, 88+80, 192-20, 256, 192);
	for (i = 15; i < 20; i++)
		scr->zones[i].font = &trebuchet_9_font;		
	
	GUI_linkStrButton(scr, 15, IDS_OK, KEY_X);
	GUI_linkStrButton(scr, 16, IDS_SAVE, KEY_Y);
	//GUI_linkStrButton(scr, 17, IDS_CANCEL, KEY_B);	

/*	scr->last_focus = 4;
	scr->incr_focus = 1;
	scr->flags = GUI_HANDLE_JOYPAD;*/	
	
	return scr;
}


t_GUIScreen *buildMenu(int nb_elems, int flags, t_GUIFont *font, t_GUIFont *font_2)
{
	t_GUIScreen *scr = GUI_newScreen(10);
	scr->flags = GUI_HANDLE_JOYPAD;
	scr->last_focus = nb_elems-1;
	scr->incr_focus = 3;

	// Button
	GUI_setZone(scr, 0, 4, 16, 4+76, 16+76);
	GUI_setZone(scr, 1, 88, 16, 88+76, 16+76);
	GUI_setZone(scr, 2, 172, 16, 172+76, 16+76);
	GUI_setZone(scr, 3, 4, 94, 4+76, 94+76);
	GUI_setZone(scr, 4, 88, 94, 88+76, 94+76);
	GUI_setZone(scr, 5, 172, 94, 172+76, 94+76);

	GUI_setZone(scr, 9, 0, 0, 256, 14); // Title
	
	if ((flags&3) == 1)
	{
	  // One element
	  GUI_setZone(scr, 6, 0, 172, 256, 192);
	}	
	if ((flags&3) == 2)
	{
		// Two elements
		GUI_setZone(scr, 6, 0, 192-20, 128, 192);
		GUI_setZone(scr, 7, 128, 192-20, 256, 192);
	} 
	if ((flags&3) == 3)
	{
		// Three elements
		GUI_setZone(scr, 6, 0, 192-20, 0+88, 192);
		GUI_setZone(scr, 7, 88, 192-20, 88+80, 192);
		GUI_setZone(scr, 8, 88+80, 192-20, 256, 192);
	} 

	int i;
	for (i = 0; i < 6; i++)
	{
		scr->zones[i].font = font;
		scr->zones[i].keymask = KEY_A;					
	}
	for (; i < scr->nb_zones; i++)
	{
		scr->zones[i].font = font_2;					
	}
	
	return scr;
}

int GUI_getConfigInt(char *objname, char *field, int val)
{
#if 0	
	char name[32];
	
	strcpy(name, objname);
	strcat(name, ".");
	strcat(name, field);
	
	return get_config_int("GUI", name, val);
#else	
	char name[32];
	
	strcpy(name, "GUI::");
	strcat(name, objname);
	
	return get_config_int(name, field, val);
#endif	
}

char *GUI_getConfigStr(char *objname, char *field, char *str)
{
#if 0
	char name[32];
	
	strcpy(name, objname);
	strcat(name, ".");
	strcat(name, field);
	
	return get_config_string("GUI", name, str);
#else	
	char name[32];
	
	strcpy(name, "GUI::");
	strcat(name, objname);
	
	return get_config_string(name, field, str);
#endif		
}

void GUI_setObjFromConfig(t_GUIScreen *scr, int nb, char *objname)
{
	int x = GUI_getConfigInt(objname, "x", 0);
	int y = GUI_getConfigInt(objname, "y", 0);
	int sx = GUI_getConfigInt(objname, "sx", 0);
	int sy = GUI_getConfigInt(objname, "sy", 0);
	
	GUI_setZone(scr, nb, x, y, x+sx, y+sy);
	
	if (GUI_getConfigInt(objname, "type", -1) == 3) // Image button
	{
		int h1 = GUI_addImage(GUI_getConfigStr(objname, "file1", NULL),
							  sx, sy, IMG_NOLOAD);
		int h2 = GUI_addImage(GUI_getConfigStr(objname, "file2", NULL), 
							  sx, sy, IMG_NOLOAD);
	
		GUI_linkObject(scr, nb, GUI_PARAM2(h1, h2), GUIImgButton_handler);
	}
	if (GUI_getConfigInt(objname, "type", -1) == 1) // Image static
	{
		int h = GUI_addImage(GUI_getConfigStr(objname, "file", NULL),
							  sx, sy, IMG_NOLOAD);
		GUI_linkObject(scr, nb, GUI_PARAM(h), GUIImage_handler);
	}
}

t_GUIScreen *buildMainMenu()
{
	t_GUIScreen *scr = GUI_newScreen(10);
	scr->img_list = GUI.img_list;
	scr->flags = GUI_HANDLE_JOYPAD;
	scr->last_focus = 5;
	scr->incr_focus = 3;

	// Button
	GUI_setObjFromConfig(scr, 0, "LoadROM");
	
	GUI_setObjFromConfig(scr, 1, "LoadState");
	GUI_setObjFromConfig(scr, 2, "SaveState");
	GUI_setObjFromConfig(scr, 3, "Options");
	GUI_setObjFromConfig(scr, 4, "Jukebox");
	GUI_setObjFromConfig(scr, 5, "Advanced");

	GUI_setObjFromConfig(scr, 6, "StatusBar");
	GUI_setObjFromConfig(scr, 7, "HideGUI");
	
	GUI_setObjFromConfig(scr, 9, "Title");

	int i;
	for (i = 0; i < 6; i++)
	{
		scr->zones[i].font = &smallfont_7_font;
		scr->zones[i].keymask = KEY_A;					
	}
	for (; i < scr->nb_zones; i++)
	{
		scr->zones[i].font = &trebuchet_9_font;					
	}
	
	return scr;
}

int ROMSelectorHandler(t_GUIZone *zone, int msg, int param, void *arg)
{
	if (msg == GUI_DRAW)
		GUI_clearScreen(0);
	if (msg == GUI_COMMAND && (param == 3|| param == 4)) // OK ou cancel
	{
		if (param == 3)
		{
			char *sel = GUISelector_getSelected(GUI.screen, NULL);
			loadROM(sel, 0);
			consoleClear();
		}
		if (param == 4)
		{
			if (CFG.Sound_output || CFG.Jukebox)
				APU_pause();			
		}
	    GUI.ScanJoypad = 0;
		SNES.Stopped = 0;
		GUI_deleteSelector(GUI.screen);
		GUI_switchScreen(scr_main);
		return 1;
	}
	return 0;
}

int SPCSelectorHandler(t_GUIZone *zone, int msg, int param, void *arg)
{
	if (msg == GUI_DRAW)
		GUI_clearScreen(0);
	if (msg == GUI_COMMAND && (param == 3|| param == 4)) // OK ou cancel
	{
		if (param == 3)
		{
			char *sel = GUISelector_getSelected(GUI.screen, NULL);
			selectSong(sel);
			
		}
		if (param == 4)
		{
			GUI_deleteSelector(GUI.screen);
			GUI_switchScreen(scr_main);
		}
		return 1;
	}
	return 0;
}

int LoadStateHandler(t_GUIZone *zone, int msg, int param, void *arg)
{
	if (msg == GUI_DRAW)
		GUI_clearScreen(0);
	if (msg == GUI_COMMAND&& (param == 3|| param == 4)) // OK ou cancel
	{
		if (param == 3)
		{
			int id;
			GUISelector_getSelected(GUI.screen, &id);

			char stateFile[100];
			strcpy(stateFile, CFG.ROMFile);
			strcpy(strrchr(stateFile, '.'), ".sml");

			read_snapshot(stateFile, id);
			CPU_unpack();
			SNES_update();
			PPU_update();
		}
		GUI_deleteList(GUI.screen);
		GUI_switchScreen(scr_main);

		APU_pause();
	    GUI.ScanJoypad = 0;
		SNES.Stopped = 0;
		return 1;
	}
	return 0;	
}

int SaveStateHandler(t_GUIZone *zone, int msg, int param, void *arg)
{
	if (msg == GUI_DRAW)
		GUI_clearScreen(0);
	if (msg == GUI_COMMAND && (param == 3 || param == 4)) // OK ou cancel
	{
		if (param == 3)
		{
			int id;
			GUISelector_getSelected(GUI.screen, &id);

			char stateName[64];
			char stateFile[100];

			strcpy(stateFile, CFG.ROMFile);
			strcpy(strrchr(stateFile, '.'), ".sml");

			sprintf(stateName,	"%04d/%02d/%02d %02d:%02d",
					2000+IPC->time.rtc.year, IPC->time.rtc.month, IPC->time.rtc.day,
					(IPC->time.rtc.hours >= 40 ? IPC->time.rtc.hours - 40 : IPC->time.rtc.hours), 
					IPC->time.rtc.minutes);

			CPU_pack();
			write_snapshot(stateFile, id, stateName);
		}
		GUI_deleteList(GUI.screen);
		GUI_switchScreen(scr_main);

		APU_pause();
	    GUI.ScanJoypad = 0;
		SNES.Stopped = 0;
		return 1;
	}
	return 0;	
}

int GFXConfigHandler(t_GUIZone *zone, int msg, int param, void *arg)
{
	switch (msg)
	{
	case GUI_DRAW:
		GUI_clearScreen(0);
		return 0;
	case GUI_COMMAND:
	{
		switch (param)
		{
		case 0: // FIX
			PPU_reset();
			return 1;
		case 1: // Prio per tile
			CFG.TilePriorityBG = ((int)arg >> 24)-1;
			return 1;
		case 2: // Block prio
			CFG.BG3TilePriority = (int)arg >> 24;
			return 1;
		case 3: // Blank tile
			CFG.Debug2 = (CFG.Debug2 % 100) + ((int)arg >> 24)*100;	
			return 1;
		case 4: // Blank tile
			CFG.Debug2 = CFG.Debug2-(CFG.Debug2 % 100)+(CFG.Debug2 % 10) 
							+ ((int)arg >> 24)*10;	
			return 1;
		case 5: // Blank tile			
			CFG.Debug2 = CFG.Debug2-(CFG.Debug2 % 10) + ((int)arg >> 24);		
			return 1;
		case 6: // Auto order			
			CFG.LayersConf = ((int)arg >> 24) ? 0 : 10;
			return 1;
		case 7: // BG			
		case 8: // BG
		case 9: // BG
		case 10: // BG			
			CFG.LayerPr[param-7] = ((int)arg >> 24);
			return 1;
		case 11: // Sprites			
		case 12: // Sprites
		case 13: // Sprites
		case 14: // Sprites		
			CFG.SpritePr[param-11] = ((int)arg >> 24);
			return 1;

		case 16: // Save
			set_config_int(SNES.ROM_info.title, "TilePriorityBG", CFG.TilePriorityBG);
			set_config_int(SNES.ROM_info.title, "BG3TilePriority", CFG.BG3TilePriority);
			set_config_int(SNES.ROM_info.title, "BlankTileNumber", CFG.Debug2);
			set_config_oct(SNES.ROM_info.title, "SpritePriority", 4,
				CFG.SpritePr[0]+(CFG.SpritePr[1]<<3)+(CFG.SpritePr[2]<<6)+(CFG.SpritePr[3]<<9));
			set_config_oct(SNES.ROM_info.title, "BGManualPriority", 4,
				CFG.LayerPr[3]+(CFG.LayerPr[2]<<3)+(CFG.LayerPr[1]<<6)+(CFG.LayerPr[0]<<9));
			save_config_file();
			break;			
		}
		free(GUI.screen);
		GUI_switchScreen(scr_main);	
		return 1;
	}
	}
	return 0;
}


int AdvancedHandler(t_GUIZone *zone, int msg, int param, void *arg)
{
	switch (msg)
	{
	case GUI_DRAW:
		GUI_clearScreen(0);
		return 0;
	case GUI_COMMAND:
	{
		switch (param)
		{
		case 0: // RESET
			reset_SNES();
			loadSRAM();
			break;
		case 1: // SAVE SRAM
			saveSRAM();
			GUI_console_printf(0, 23, "SRAM written");
			break;
		case 2: // GFX CONFIG
		{
			t_GUIScreen *scr = buildGFXConfigMenu();
			scr->handler = GFXConfigHandler;
			free(GUI.screen);
			GUI_switchScreen(scr);
			return 1;
		}
		}
		free(GUI.screen);
		GUI_switchScreen(scr_main);	
		return 1;
	}
	}
	return 0;
}


void LayerOptionsUpdate()
{
	int i;
	
	if (CFG.LayersConf == 0) // AUTO
	{
		for (i = 0; i < 4; i++)
		{
			GUI.screen->zones[1+i].state |= GUI_ST_HIDDEN;
			GUI.screen->zones[8+i].state |= GUI_ST_HIDDEN;
		}
		for (i = 12; i < 14; i++)
			GUI.screen->zones[i].state |= GUI_ST_HIDDEN;
	} else
	{
		for (i = 0; i < 4; i++)
		{
			GUI.screen->zones[1+i].state &= ~GUI_ST_HIDDEN;
			GUI.screen->zones[8+i].state &= ~GUI_ST_HIDDEN;
		}
		for (i = 12; i < 14; i++)
			GUI.screen->zones[i].state &= ~GUI_ST_HIDDEN;

		for (i = 0; i < 4; i++)
		{
			int c = ((CFG.BG_Layer&0x7) | ((CFG.BG_Layer&0x10)>>1)) & (1 << i) ?
					CFG.LayerPr[i]+1 : 0;
			GUI_SET_CHOICE(GUI.screen, 1+i, c);
		}
	}
}

int LayersOptionsHandler(t_GUIZone *zone, int msg, int param, void *arg)
{	
	switch (msg)
	{
	case GUI_DRAW:
		GUI_clearScreen(0);
		break;
	case GUI_COMMAND:
		//GUI_printf(0, 23, "Command %d %08x", param, arg);
		switch (param)
		{
		case 0: // Auto layer
		{
			int enabled = (int)arg >> 24;

			PPU_ChangeLayerConf(!enabled);
			if (enabled != 0)
				CFG.BG_Layer |= 0x1F; // In automatic mode, all layers are enabled
			LayerOptionsUpdate();
			GUI_drawScreen(GUI.screen, NULL);
			return 1;
		}
		case 1:
		case 2:
		case 3:			
			if ((int)arg >> 24 == 0)
				CFG.BG_Layer &= ~(1 << (param-1));
			else
				CFG.BG_Layer |= (1 << (param-1));
			CFG.LayerPr[param-1] = (int)arg >> 24;
			return 1;
		case 4: // SPRITES
			if ((int)arg >> 24 == 0)
				CFG.BG_Layer &= ~0x10;
			else
				CFG.BG_Layer |= 0x10;			
			return 1;
		case 12: // UP
		case 13: // DOWN			
		{
			if ((param == 12 && CFG.LayersConf-1 > 0) ||
				(param == 13 && CFG.LayersConf+1 < 10))
			{
				PPU_ChangeLayerConf(param == 12 ? CFG.LayersConf-1 : CFG.LayersConf+1);
				LayerOptionsUpdate();
				GUI_drawScreen(GUI.screen, NULL);
			}
			return 1;			
		}
		case 15: // IDSAVE	
			saveOptionsToConfig(SNES.ROM_info.title);
			return 1;
		case 14: // IDAPPLY
		case 16: // IDCANCEL
			free(GUI.screen);
			GUI.screen = NULL;
			GUI_switchScreen(scr_main);			
			return 1;		
		}
	}
	return 0;
}


t_GUIScreen *buildLayersMenu()
{
	t_GUIScreen *scr = GUI_newScreen(19);
	
	GUI_setZone   (scr, 5, 0, 0, 256, 24); // Title zone
	GUI_linkObject(scr, 5, (void *)IDS_LAYERS, GUIStatic_handler);
	GUI_setZone   (scr, 6, 0, 24, 256, 56); // Description zone
	GUI_linkObject(scr, 6, (void *)IDS_LAYERS_HELP, GUIStatic_handler);
	
	GUI_setZone   (scr, 0, 0, 56, 16, 56+16); // Auto order Check button
	GUI_linkObject(scr, 0, GUI_CHOICE(IDS_CHECK, 2, CFG.LayersConf == 0), GUIChoiceButton_handler);
	GUI_setZone   (scr, 7, 24, 56, 256, 56+16); // Auto order static
	GUI_linkObject(scr, 7, GUI_STATIC_LEFT(IDS_AUTO_ORDER, 0), GUIStaticEx_handler);
	
	int i;
	for (i = 0; i < 4; i++)
	{
		GUI_setZone   (scr, 8+i, 64, 80+i*20, 120, 80+i*20+24); // Layer static
		GUI_linkObject(scr, 8+i, GUI_STATIC_RIGHT(i < 3 ? IDS_LAYER : IDS_SPRITES, i),
						GUIStaticEx_handler);
		GUI_setZone   (scr, 1+i, 128, 80+i*20, 256, 80+i*20+24); // Layer order button
		GUI_linkObject(scr, 1+i, GUI_CHOICE(IDS_OFF, 5, 0), GUIChoiceButton_handler);
	}
	LayerOptionsUpdate();

	for (i = 0; i < 12; i++)
	{
		scr->zones[i].font = &smallfont_7_font;
		scr->zones[i].keymask = KEY_A;
	}
	scr->zones[5].font = &trebuchet_9_font;
	
	GUI_setZone(scr, 12, 20, 100, 60, 120);
	GUI_setZone(scr, 13, 20, 120, 60, 140);
	
	GUI_linkStrButton(scr, 12, (int)"\024", KEY_L);
	GUI_linkStrButton(scr, 13, (int)"\025", KEY_R);
	
	// Three elements
	GUI_setZone(scr, 14, 0, 192-20, 0+88, 192);
	GUI_setZone(scr, 16, 88, 192-20, 88+80, 192);
	GUI_setZone(scr, 15, 88+80, 192-20, 256, 192);
	for (i = 12; i < 17; i++)
		scr->zones[i].font = &trebuchet_9_font;		
	
	GUI_linkStrButton(scr, 14, IDS_OK, KEY_X);
	GUI_linkStrButton(scr, 15, IDS_SAVE, KEY_Y);
	//GUI_linkStrButton(scr, 16, IDS_CANCEL, KEY_B);	

	scr->last_focus = 4;
	scr->incr_focus = 1;
	scr->flags = GUI_HANDLE_JOYPAD;	
	
	return scr;
}

int ScreenOptionsHandler(t_GUIZone *zone, int msg, int param, void *arg)
{
	switch (msg)
	{
	case GUI_DRAW:
		GUI_clearScreen(0);
		return 0;
	case GUI_COMMAND:
		switch (param)
		{
		case 0:
			CFG.BG3Squish = (int)arg >> 24;
			return 1;
		case 1:
			CFG.YScroll = (int)arg >> 24;
			GFX.YScroll = _offsetY_tab[CFG.YScroll];
			return 1;
		case 2:
			CFG.Scaled = (int)arg >> 24;
			return 1;		
			
			
		case 11: // IDSAVE	
			saveOptionsToConfig(SNES.ROM_info.title);
			return 1;			
		case 10: // IDAPPLY
		case 12: // IDCANCEL
			free(GUI.screen);
			GUI.screen = NULL;
			GUI_switchScreen(scr_main);			
			return 1;
		}
	}
	return 0;
}

t_GUIScreen *buildScreenMenu()
{
	t_GUIScreen *scr = GUI_newScreen(13);
	
	GUI_setZone   (scr, 3, 0, 0, 256, 24); // Title zone
	GUI_linkObject(scr, 3, (void *)IDS_SCREEN, GUIStatic_handler);

	GUI_setZone   (scr, 4, 0, 30, 80, 30+16); // Layer static
	GUI_linkObject(scr, 4, GUI_STATIC_LEFT(IDS_HUD, 0), GUIStaticEx_handler);
	GUI_setZone   (scr, 0, 80, 30, 256, 30+16); // Layer order button
	GUI_linkObject(scr, 0, GUI_CHOICE(IDS_HUD+1, 3, CFG.BG3Squish), GUIChoiceButton_handler);

	GUI_setZone   (scr, 5, 0, 50, 80, 50+16); // Layer static
	GUI_linkObject(scr, 5, GUI_STATIC_LEFT(IDS_YSCROLL, 0), GUIStaticEx_handler);
	GUI_setZone   (scr, 1, 80, 50, 256, 50+16); // Layer order button
	GUI_linkObject(scr, 1, GUI_CHOICE(IDS_YSCROLL+1, 4, CFG.YScroll), GUIChoiceButton_handler);

	GUI_setZone   (scr, 6, 0, 70, 80, 70+16); // Layer static
	GUI_linkObject(scr, 6, GUI_STATIC_LEFT(IDS_SCALING, 0), GUIStaticEx_handler);
	GUI_setZone   (scr, 2, 80, 70, 256, 70+16); // Layer order button
	GUI_linkObject(scr, 2, GUI_CHOICE(IDS_SCALING+1, 3, CFG.Scaled), GUIChoiceButton_handler);

	// Three elements
	GUI_setZone(scr, 10, 0, 192-20, 0+88, 192);
	GUI_setZone(scr, 12, 88, 192-20, 88+80, 192);
	GUI_setZone(scr, 11, 88+80, 192-20, 256, 192);	
	
	int i;
	for (i = 0; i < 7; i++)
	{
		scr->zones[i].font = &smallfont_7_font;
		scr->zones[i].keymask = KEY_A;
	}
	scr->zones[3].font = &trebuchet_9_font;
	for (i = 10; i < 13; i++)
		scr->zones[i].font = &trebuchet_9_font;		
	
	GUI_linkStrButton(scr, 10, IDS_OK, KEY_X);
	GUI_linkStrButton(scr, 11, IDS_SAVE, KEY_Y);
	//GUI_linkStrButton(scr, 12, IDS_CANCEL, KEY_B);	

	scr->last_focus = 2;
	scr->incr_focus = 1;
	scr->flags = GUI_HANDLE_JOYPAD;	
	
	return scr;
}

int OptionsHandler(t_GUIZone *zone, int msg, int param, void *arg)
{
	int i;
	
	switch (msg)
	{
	case GUI_DRAW:
		GUI_clearScreen(0);
		break;
	case GUI_COMMAND:
//		GUI_printf(0, 23, "Command %d", param);
		switch (param)
		{
		case 0: // OPTIONS SCREEN
		{
			t_GUIScreen *scr = buildScreenMenu();
			scr->handler = ScreenOptionsHandler;
				
			GUI_switchScreen(scr);
			return 1;			
		}
		case 1: // OPTIONS LAYERS
		{
			t_GUIScreen *scr = buildLayersMenu();
			scr->handler = LayersOptionsHandler;
			GUI_switchScreen(scr);
			return 0;
		}
		case 2: // Sound
			APU_pause();
			swiWaitForVBlank();			
			CFG.Sound_output = (int)arg >> 24;
			if (!CFG.Sound_output)
				APU_clear();
			swiWaitForVBlank();			
			APU_pause();
			return 1;
		case 3: // Speed
			CFG.WaitVBlank = !((int)arg >> 24);			
			return 1;
		case 4: // Speed hacks
			CFG.CPU_speedhack = (int)arg >> 24;
			return 1;	
		case 5: // Automatic SRAM saving
			CFG.AutoSRAM = (int)arg >> 24;
			return 1;			
		case 14: // IDSAVE			
			saveOptionsToConfig(SNES.ROM_info.title);
			return 1;						
		case 13: // IDAPPLY
		case 15: // IDCANCEL
			free(GUI.screen);
			GUI.screen = NULL;
			GUI_switchScreen(scr_main);			
			return 1;
		}		
	}
	return 0;	
}

t_GUIScreen *buildOptionsMenu()
{
	t_GUIScreen *scr = GUI_newScreen(16);
		
	GUI_setZone   (scr, 7, 0, 0, 256, 24); // Title zone
	GUI_linkObject(scr, 7, (void *)IDS_OPTIONS, GUIStatic_handler);

	GUI_setZone   (scr, 0, 0, 28, 124, 28+52); // -> Layers menu
	GUI_linkObject(scr, 0, GUI_PARAM(IDS_SCREEN), GUIStrButton_handler);
	
	GUI_setZone   (scr, 1, 132, 28, 256, 28+52); // -> Layers menu
	GUI_linkObject(scr, 1, GUI_PARAM(IDS_LAYERS), GUIStrButton_handler);	
	
	GUI_setZone   (scr, 2, 0, 84, 16, 84+16); // Sound Check button
	GUI_linkObject(scr, 2, GUI_CHOICE(IDS_CHECK, 2, CFG.Sound_output), GUIChoiceButton_handler);
	GUI_setZone   (scr, 8, 24, 84, 100, 84+16); // Sound
	GUI_linkObject(scr, 8, GUI_STATIC_LEFT(IDS_SOUND, 0), GUIStaticEx_handler);
	
	GUI_setZone   (scr, 9, 0, 104, 80, 104+16); // static
	GUI_linkObject(scr, 9, GUI_STATIC_LEFT(IDS_SPEED, 0), GUIStaticEx_handler);
	GUI_setZone   (scr, 3, 80, 104, 256, 104+16); // Speed
	GUI_linkObject(scr, 3, GUI_CHOICE(IDS_SPEED+1, 2, !CFG.WaitVBlank), GUIChoiceButton_handler);

	GUI_setZone   (scr, 10, 0, 124, 80, 124+16); // static
	GUI_linkObject(scr, 10, GUI_STATIC_LEFT(IDS_HACKS, 0), GUIStaticEx_handler);
	GUI_setZone   (scr, 4, 80, 124, 256, 124+16); // Hacks
	GUI_linkObject(scr, 4, GUI_CHOICE(IDS_HACKS+1, 4, CFG.CPU_speedhack), GUIChoiceButton_handler);

	GUI_setZone   (scr, 5, 0, 144, 16, 144+16); // Automatic SRAM saving
	GUI_linkObject(scr, 5, GUI_CHOICE(IDS_CHECK, 2, CFG.AutoSRAM), GUIChoiceButton_handler);
	GUI_setZone   (scr, 11, 24, 144, 256, 144+16); // Auto order static
	GUI_linkObject(scr, 11, GUI_STATIC_LEFT(IDS_AUTO_SRAM, 0), GUIStaticEx_handler);

	GUI_setZone   (scr, 6, 100, 84, 100+16, 84+16); // Memory pak extension
	GUI_linkObject(scr, 6, GUI_CHOICE(IDS_CHECK, 2, CFG.ExtRAMSize > 0), GUIChoiceButton_handler);
	GUI_setZone   (scr, 12, 100+24, 84, 256, 84+16); 
	GUI_linkObject(scr, 12, GUI_STATIC_LEFT(IDS_USE_MEM_PACK, 0), GUIStaticEx_handler);		
	scr->zones[6].state |= GUI_ST_DISABLED;
	
	
	// Three elements
	GUI_setZone(scr, 13, 0, 192-20, 0+88, 192);
	GUI_setZone(scr, 15, 88, 192-20, 88+80, 192);
	GUI_setZone(scr, 14, 88+80, 192-20, 256, 192);	
	
	int i;
	for (i = 0; i < 13; i++)
	{
		scr->zones[i].font = &smallfont_7_font;
		scr->zones[i].keymask = KEY_A;
	}
	scr->zones[7].font = &trebuchet_9_font;
	for (i = 13; i < 16; i++)
		scr->zones[i].font = &trebuchet_9_font;	
	
	GUI_linkStrButton(scr, 13, IDS_OK, KEY_X);
	GUI_linkStrButton(scr, 14, IDS_SAVE, KEY_Y);
	//GUI_linkStrButton(scr, 15, IDS_CANCEL, KEY_B);	

	scr->last_focus = 5;
	scr->incr_focus = 1;
	scr->flags = GUI_HANDLE_JOYPAD;	
	
	scr->handler = OptionsHandler;	
	
	return scr;
}


int MainScreenHandler(t_GUIZone *zone, int msg, int param, void *arg)
{
	int i;
	
	if (msg == GUI_DRAW)
		GUI_clearScreen(0);
	if (msg == GUI_COMMAND)
	{
//		GUI_console_printf(0, 0, "Command %d", param);		
		if (param == 0) // ROM list
		{
		    SNES.Stopped = 1;
		    GUI.ScanJoypad = 1;
		    if (CFG.Sound_output || CFG.Jukebox)
		    	APU_pause();
			
			// Get ROMs list
  		    int		cnt;
  		    char **dir_list = FS_getDirectoryList(CFG.ROMPath, "SMC|SFC|SWC|FIG", &cnt);
  		    
  		    // Alphabetical sort
  		    if (CFG.GUISort)
  		    	qsort(dir_list, cnt, sizeof(char *), sort_strcmp);

		    // Create ROM selector
  		    t_GUIScreen *scr = 
  		    	GUI_newSelector(cnt, dir_list, IDS_SELECT_ROM, &trebuchet_9_font);
			scr->handler = ROMSelectorHandler;		    
		    
			// Switch GUI Screen
		    GUI_switchScreen(scr);
		    return 1;
		}
		if (param == 1 || param == 2) // Load / Save
		{
		    SNES.Stopped = 1;
		    GUI.ScanJoypad = 1;		    
	    	APU_pause();
		    	
			t_GUIScreen *scr = GUI_newList(8, 32, 
					param == 1 ? IDS_LOAD_STATE : IDS_SAVE_STATE, &trebuchet_9_font);
			
			char stateName[33];
			char stateFile[100];
			strcpy(stateFile, CFG.ROMFile);
			strcpy(strrchr(stateFile, '.'), ".sml");
			
			for (i = 0; i < 8; i++)
			{
				if (get_snapshot_name(stateFile, i, stateName))
					GUI_setItem(scr, i, stateName, 32);
				else
					GUI_setItem(scr, i, "--Empty--", 32);
			}

			scr->handler = param == 1 ? LoadStateHandler : SaveStateHandler;
			GUI_switchScreen(scr);
		    return 1;			
		}
		if (param == 3) // Options
		{
			t_GUIScreen *scr = buildOptionsMenu();
			GUI_switchScreen(scr);
			return 1;
		}
		if (param == 4) // Jukebox
		{
			// Get ROMs list
  		    int		cnt;
  		    char **dir_list = FS_getDirectoryList(CFG.ROMPath, "SPC", &cnt);

  		    // Alphabetical sort
  		    if (CFG.GUISort)
  		    	qsort(dir_list, cnt, sizeof(char *), sort_strcmp);  		    
  		    
		    // Create ROM selector
  		    t_GUIScreen *scr = 
  		    	GUI_newSelector(cnt, dir_list, IDS_JUKEBOX, &trebuchet_9_font);
			scr->handler = SPCSelectorHandler;		    
		    
			// Switch GUI Screen
		    GUI_switchScreen(scr);
		    return 1;		    
		}		
		if (param == 5) // Advanced
		{
			t_GUIScreen *scr = buildMenu(3, 1, &smallfont_7_font, &trebuchet_9_font);
			GUI_linkObject(scr, 9, (void *)IDS_ADVANCED, GUIStatic_handler);
			GUI_linkObject(scr, 0, GUI_PARAM(IDS_RESET), GUIStrButton_handler);
			GUI_linkObject(scr, 1, GUI_PARAM(IDS_SAVE_SRAM), GUIStrButton_handler);
			GUI_linkObject(scr, 2, "GFX Config", GUIStrButton_handler);
			
			GUI_linkStrButton(scr, 6, IDS_OK, KEY_X);
			
			scr->handler = AdvancedHandler;
			GUI_switchScreen(scr);
			return 1;
		}		
		if (param == 7) // HideGUI
		{
			GUI.hide = 1;
			swiWaitForVBlank();
			powerOFF(POWER_2D_B);
			setBacklight(PM_BACKLIGHT_TOP);
		}
		return 1;
	}
	return 0;
}

int FirstROMSelectorHandler(t_GUIZone *zone, int msg, int param, void *arg)
{
	switch (msg)
	{
	case GUI_DRAW:
		GUI_clearScreen(0);
		break;
	case GUI_COMMAND:
		if (param == 3)
		{
			GUI.exit = 1;
			return 1;
		}
	}
	return 0;
}

char *GUI_getROM(char *rompath)
{
    GUI.ScanJoypad = 1;
	consoleClear();

		// Get ROMs list
	int		cnt;
#ifndef IN_EMULATOR	
    char **dir_list = FS_getDirectoryList(rompath, "SMC|SFC|SWC|FIG", &cnt);
#else
	char **dir_list = FS_getDirectoryList(rompath, NULL, &cnt);
#endif
	
	
	// Alphabetical sort
	if (CFG.GUISort)
		qsort(dir_list, cnt, sizeof(char *), sort_strcmp);
	
	// Create ROM selector
	t_GUIScreen *scr = 
		GUI_newSelector(cnt, dir_list, IDS_SELECT_ROM, &trebuchet_9_font);
    scr->zones[4].handler = NULL; // Remove CANCEL button
	scr->handler = FirstROMSelectorHandler;
	
	GUI_drawScreen(scr, NULL);

	GUI_start();
	
	char *sel;
	sel = GUISelector_getSelected(scr, NULL);

    GUI.ScanJoypad = 0;
	return sel;
}

void GUI_deleteROMSelector()
{
	GUI_deleteSelector(GUI.screen); // Should also delete dir_list
	GUI_clearScreen(0);
}

void GUI_createMainMenu()
{
	consoleClear();
	
#if 1
	// Create Main screen
	scr_main = buildMenu(6, 1, &smallfont_7_font, &trebuchet_9_font);
	GUI_linkObject(scr_main, 9, SNEMULDS_TITLE, GUIStatic_handler);
	
	GUI_setZone(scr_main, 7, 240, 0, 256, 16);
	scr_main->zones[7].font = &smallfont_7_font;
	GUI_linkObject(scr_main, 7,  "*", GUIStrButton_handler);
	
//	scr_main->img_list = img_list;
	scr_main->handler = MainScreenHandler;

	int i;
	for (i = 0; i < 6; i++)
	{
		GUI_linkObject(scr_main, i, GUI_PARAM(IDS_SELECT_ROM+i), GUIStrButton_handler);
	}
/*	GUI_linkObject(scr_main, 6,
			"\001 \002 \003 \004 \005 \006 \016 \017 \020 \021 \022 \023 \024",
			GUIStatic_handler);*/
#else
	
	GUI_loadPalette(get_config_string("GUI", "Palette", "test.dat"));
	GUI.img_list = GUI_newImageList(32);
	
	scr_main = buildMainMenu();
	GUI_linkObject(scr_main, 9, SNEMULDS_TITLE, GUIStatic_handler);	
	
	scr_main->handler = MainScreenHandler;
#endif
	
	GUI_drawScreen(scr_main, NULL);
	GUI.screen = scr_main;
}
	
void GUI_getConfig()
{
	CFG.GUISort = get_config_int("GUI", "FileChooserSort", 1);
	CFG.Language = get_config_int("GUI", "Language", -1);
	
	if (CFG.Language != -1)
		GUI_setLanguage(CFG.Language);
}

void GUI_setLanguage(int lang)
{
	switch (lang)
	{
	case 0: 
		GUI.string = (char **)&g_snemulds_str_jpn; // JAPANESE
		break;
	case 1:
		GUI.string = (char **)&g_snemulds_str_eng; // ENGLISH
		break;
	case 2:
		GUI.string = (char **)&g_snemulds_str_fr; // FRENCH
		break;
	case 3:		
		GUI.string = (char **)&g_snemulds_str_ger; // GERMAN
		break;		
	case 4:		
		GUI.string = (char **)&g_snemulds_str_ita; // ITALIAN
		break;		
	case 5:		
		GUI.string = (char **)&g_snemulds_str_spa; // SPANISH
		break;		
	case 106:		
		GUI.string = (char **)&g_snemulds_str_pt; // PORTUGUESE
		break;		
	case 107:		
		GUI.string = (char **)&g_snemulds_str_cat; // CATALAN
		break;
	case 108:		
		GUI.string = (char **)&g_snemulds_str_pol; // POLISH
		break;
	case 109:		
		GUI.string = (char **)&g_snemulds_str_nl; // DUTCH
		break;
	case 110:		
		GUI.string = (char **)&g_snemulds_str_dan; // DANISH
		break;		
		
	default:
		GUI.string = (char **)&g_snemulds_str_eng; // ENGLISH
		break;		
	}		
}

extern char	g_printfbuf[100];

void		GUI_printf(char *fmt, ...)
{
	va_list ap;

	
    va_start(ap, fmt);
    vsnprintf(g_printfbuf, 64, fmt, ap);
    va_end(ap);

    // FIXME
    t_GUIZone zone;
    zone.x1 = 0; zone.y1 = 0; zone.x2 = 256; zone.y2 = 192;
    zone.font = &trebuchet_9_font;
    GUI_drawText(&zone, 0, GUI.printfy, 255, g_printfbuf);
    GUI.printfy += GUI_getFontHeight(&zone);
}

void		GUI_align_printf(int flags, char *fmt, ...)
{
	va_list ap;

	
    va_start(ap, fmt);
    vsnprintf(g_printfbuf, 64, fmt, ap);
    va_end(ap);

    // FIXME
    t_GUIZone zone;
    zone.x1 = 0; zone.y1 = 0; zone.x2 = 256; zone.y2 = 192;
    zone.font = &trebuchet_9_font;
    // FIXME
    GUI.printfy += GUI_drawAlignText(&zone, flags, GUI.printfy, 255, g_printfbuf);
}

void	GUI_clear()
{
	consoleClear();
	GUI_clearScreen(0);
	GUI.printfy = 0;
}

void	GUI_showROMInfos(int size)
{
    GUI_printf("%s %s\n", _STR(IDS_TITLE), SNES.ROM_info.title);
    GUI_printf("%s %d bytes\n", _STR(IDS_SIZE), size);
    if (SNES.HiROM) 
    	GUI_printf("%s HiROM\n", _STR(IDS_ROM_TYPE));
    else 
    	GUI_printf("%s LoROM\n", _STR(IDS_ROM_TYPE));
    GUI_printf("%s %s\n", _STR(IDS_COUNTRY), SNES.ROM_info.countrycode < 2 ? "NTSC" : "PAL");	
}


  
