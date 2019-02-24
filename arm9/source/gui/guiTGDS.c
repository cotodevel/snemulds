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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <malloc.h>
#include <ctype.h>
#include "ipcfifoTGDS.h"

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "consoleTGDS.h"

#include "guiTGDS.h"
#include "biosTGDS.h"
#include "gui_console_connector.h"

#include "gui_widgets.h"
#include "InterruptsARMCores_h.h"
#include "dmaTGDS.h"

#include "posixHandleTGDS.h"
#include "fsfatlayerTGDS.h"
#include "keypadTGDS.h"
#include "utilsTGDS.h"
#include "spifwTGDS.h"
#include "powerTGDS.h"
#include "videoTGDS.h"

t_GUIScreen	*GUI_newScreen(int nb_elems)
{
	t_GUIScreen *scr;
	
	scr = malloc(sizeof(t_GUIScreen)+nb_elems*sizeof(t_GUIZone));
	memset(scr, 0, sizeof(t_GUIScreen)+nb_elems*sizeof(t_GUIZone));
	scr->nb_zones = nb_elems;
	scr->curs = -1;
	scr->last_focus = scr->nb_zones-1;
	scr->incr_focus = 3;
	
	return scr;
}

void	GUI_setZone(t_GUIScreen *scr, int i,int x1, int y1, int x2, int y2)
{
	memset(&scr->zones[i], 0, sizeof(scr->zones[i]));
	scr->zones[i].x1 = x1;
	scr->zones[i].y1 = y1;
	scr->zones[i].x2 = x2;
	scr->zones[i].y2 = y2;
	scr->zones[i].id = i;
//	scr->zones[i].data = NULL;
}

void	GUI_linkObject(t_GUIScreen *scr, int i, void *data, t_GUIHandler handler)
{
	scr->zones[i].data = data;
	scr->zones[i].handler = handler;
}

int GUI_loadPalette(sint8 *path)
{
	uint8	*data;
	int 	size;
	int		i;
	
	size = FS_getFileSize(path);
	if (size <= 0)
		return -1;	
	
	data = malloc(size);
	
	FS_loadFile(path, (sint8 *)data, size);
	
    for (i = 0; i < MIN(size, GUI_PAL*3); i+=3) 
	  	BG_PALETTE_SUB[i/3] = RGB8(data[i],data[i+1],data[i+2]);
    
    free(data);
    return 0;
}

t_GUIImage	*GUI_loadImage(sint8 *path, int width, int height, int flags)
{
	t_GUIImage	*ptr = NULL;	
	int 	size;
	
	size = FS_getFileSize(path);
	if (size <= 0)
		return NULL;
	
	if (flags == IMG_IN_MEMORY)
	{
		ptr = malloc(sizeof(t_GUIImage)+size);
		if (ptr == NULL)
			return NULL;
		
		ptr->data = (uint8*)(ptr)+sizeof(t_GUIImage);   
		FS_loadFile(path, ptr->data, size);             
		
	}
	if (flags == IMG_NOLOAD)
	{
		ptr = malloc(sizeof(t_GUIImage)+strlen(path)+1);
		if (ptr == NULL)
			return NULL;
		
		ptr->data = (uint8*)(ptr)+sizeof(t_GUIImage);	
		strcpy(ptr->data, path);
	}
	
	if (flags == IMG_IN_VRAM)
	{
		// TODO
		if (ptr == NULL)
			return NULL;
		
	}	

	ptr->flags = flags;
	ptr->width = width;
	ptr->height = height;
	
	return ptr;
}

int GUI_addImage(sint8 *path, int w, int h, int flags)
{
	t_GUIImage *img = GUI_loadImage(path, w, h, flags);
	
	GUI.img_list->img[GUI.img_list->cnt++] = img;
	return GUI.img_list->cnt-1;
}

void		GUI_deleteImage(t_GUIImage *image)
{
	free(image);
}

void		GUI_drawHLine(t_GUIZone *zone, int color, int x1, int y1, int x2)
{
	uint16		*ptr;
	
	ptr = GUI.DSFrameBuffer;
	ptr += (zone->x1 + x1) / 2 + ((zone->y1 + y1) * 128);

	uint32	c;
	
	c = color | (color << 8) | (color << 16) | (color << 24);

	swiFastCopy((uint32*)&c, (uint32*)(uint16*)ptr, (x2-x1)/4 | COPY_FIXED_SOURCE);
}


void		GUI_drawVLine(t_GUIZone *zone, int color, int x1, int y1, int y2)
{
	uint16		*ptr;
	int			y;
	
	ptr = GUI.DSFrameBuffer;
	ptr += (zone->x1 + x1) / 2 + ((zone->y1 + y1) * 128);

	if ((x1 & 1) == 0)
		for (y=0; y < y2-y1; y++, ptr+= 128) 
			*ptr = (*ptr & 0xFF00) | color;
	else
		for (y=0; y < y2-y1; y++, ptr+= 128) 
			*ptr = (*ptr & 0x00FF) | (color << 8);
}

void		GUI_drawRect(t_GUIZone *zone, int color, int x1, int y1, int x2, int y2)
{
	GUI_drawHLine(zone, color, x1, y1, x2);
	GUI_drawVLine(zone, color, x2-1, y1, y2);
	GUI_drawHLine(zone, color, x1, y2-1, x2);
	GUI_drawVLine(zone, color, x1, y1, y2);
}

void		GUI_drawBar(t_GUIZone *zone, int color, int x1, int y1, int x2, int y2)
{
	uint16		*ptr;
	int			y;
	
	ptr = GUI.DSFrameBuffer;
	if (zone)
		ptr += (zone->x1 + x1) / 2 + ((zone->y1 + y1) * 128);
	else
		ptr += x1 / 2 + y1 * 128;

	uint32	c = color | (color << 8) | (color << 16) | (color << 24);
	for (y=0; y < y2-y1; y++) 
	{
		swiFastCopy((uint32*)&c, (uint32*)(uint16*)ptr, (x2-x1)/4 | COPY_FIXED_SOURCE);
		ptr += 128;		
	}	
}



void		GUI_drawImage(t_GUIZone *zone, t_GUIImage *image, int x, int y)
{
	uint16		*ptr;
	uint16		*img = NULL;
	FILE		*f = NULL;
	
	//FIL fhandler;
//	printf("XXX %p %d %d %d %p\n", image, image->width, image->height, image->flags, image->data);

	ptr = GUI.DSFrameBuffer;
	ptr += (zone->x1 + x) / 2 + ((zone->y1 + y) * 128);

	if (image->flags == IMG_NOLOAD)
	{		
		FS_lock();
		f = fopen(image->data, "r");
		//f_open(&fhandler,image->data,FA_READ);
	}
	else
		img = image->data;
	

	for (y=0; y < image->height; y++) 
	{
		if (image->flags == IMG_NOLOAD)
		{
			fread(ptr, 4, image->width/4, f);
			//unsigned int read_so_far;
			//f_read(&fhandler, ptr, image->width, &read_so_far);
	
		}
		else
		{
			swiFastCopy((uint32*)(uint16*)img, (uint32*)(uint16*)ptr, image->width/4);
			img += (image->width)/2;
		}
		ptr += 128;
	}
	
	if (image->flags == IMG_NOLOAD)
	{
		fclose(f);
		//f_close(&fhandler);
		FS_unlock();
	}
}

int			GUI_sendMessage(t_GUIScreen *scr, int i, int msg, int param, void *arg)
{
	if (i >= 0 && i < scr->nb_zones && scr->zones[i].handler)
		return scr->zones[i].handler(&scr->zones[i], msg, param, arg);
	else
		return -1;
}

t_GUIMessage	PendingMessage;

int			GUI_dispatchMessageNow(t_GUIScreen *scr, int msg, int param, void *arg)
{
	int	i;
	
	for (i = 0; i < scr->nb_zones; i++)
	{
		t_GUIZone	*zone = &scr->zones[i];

		if (zone->handler &&
			zone->handler(zone, msg, param, arg))
			return i;
	}
	return -1;	
}

int			GUI_dispatchMessage(t_GUIScreen *scr, int msg, int param, void *arg)
{
#if 0	
	GUI_dispatchMessageNow(scr, msg, param, arg);
#else
	PendingMessage.scr = scr;
	PendingMessage.msg = msg;
	PendingMessage.param = param;
	PendingMessage.arg = arg;
#endif
	return 0;
}

int		GUI_setFocus(t_GUIScreen *scr, int id)
{
	if (scr->curs != id && id <= scr->last_focus)
	{
		GUI_sendMessage(scr, scr->curs, GUI_EVENT, GUI_EVENT_UNFOCUS, NULL);
		scr->curs = id;
		GUI_sendMessage(scr, scr->curs, GUI_EVENT, GUI_EVENT_FOCUS, NULL);
	}
	return 0;
}

void	GUI_clearFocus(t_GUIScreen *scr)
{
	if (scr->curs != -1)
	{
		GUI_sendMessage(scr, scr->curs, GUI_EVENT, GUI_EVENT_UNFOCUS, NULL);
		scr->curs = -1;
	}
}

int		GUI_dispatchEvent(t_GUIScreen *scr, int event, void *param)
{
	int i;
	
	if (event == GUI_EVENT_BUTTON)
	{
		// In handle joypad mode, intercept focus chage
		if (scr->flags & GUI_HANDLE_JOYPAD)
		{
			t_GUIEvent	*e = (t_GUIEvent*)(param);
						
			if (e->joy.repeated & KEY_LEFT && scr->curs >= 1)
				GUI_setFocus(scr, scr->curs-1);
			if (e->joy.repeated & KEY_RIGHT && scr->curs < scr->last_focus)
				GUI_setFocus(scr, scr->curs+1);

			if (e->joy.repeated & KEY_UP)
				GUI_setFocus(scr, MAX(scr->curs-scr->incr_focus, 0));			
			if (e->joy.repeated & KEY_DOWN)
				GUI_setFocus(scr, MIN(scr->curs+scr->incr_focus, scr->last_focus));
			
		}
		// First send message to focused item
		if (GUI_sendMessage(scr, scr->curs, GUI_EVENT, event, param) > 0)
			return scr->curs;
		
		// Then to other items
		//for (i = 0; i < scr->nb_zones; i++)
		i = (scr->flags & GUI_HANDLE_JOYPAD) ? scr->last_focus+1 : 0;
		for (; i < scr->nb_zones; i++)
		{
			if (i == scr->curs)
				continue;
			if (GUI_sendMessage(scr, i, GUI_EVENT, event, param) > 0)
				return i;
		}
	}
	
	if (event == GUI_EVENT_STYLUS)
	{
		t_GUIEvent	*e = (t_GUIEvent*)(param);

		if (scr->stylus_zone >= 0 && scr->stylus_zone < scr->nb_zones)
		{
			t_GUIZone	*czone = &scr->zones[scr->stylus_zone];
		
			if (!(e->stl.x >= czone->x1 && e->stl.x < czone->x2 &&
				  e->stl.y >= czone->y1 && e->stl.y < czone->y2))
			{
				if (czone->handler)
					czone->handler(czone, GUI_EVENT, GUI_EVENT_LEAVEZONE, param);
				scr->stylus_zone = -1;
			}
		}
		
		for (i = 0; i < scr->nb_zones; i++)
		{
			t_GUIZone	*zone = &scr->zones[i];

			if (e->stl.x >= zone->x1 && e->stl.x < zone->x2 &&
				e->stl.y >= zone->y1 && e->stl.y < zone->y2)
			{
				if (scr->stylus_zone != i)
				{
					if (zone->handler)
						zone->handler(zone, GUI_EVENT, GUI_EVENT_ENTERZONE, param);
					scr->stylus_zone = i;
				}
				
				if (zone->handler && zone->handler(zone, GUI_EVENT, event, param))
					return i;
			}
		}
	}
	
	return -1;
}

void		GUI_drawScreen(t_GUIScreen *scr, void *param)
{
	int i;
	
	GUI.screen = scr;
	
	if (scr->handler)
		scr->handler(NULL, GUI_DRAW, 0, param);
	
	for (i = 0; i < scr->nb_zones; i++)
	{
		if (scr->zones[i].handler)
		{
			if ((scr->zones[i].state & GUI_ST_HIDDEN) == 0)
				scr->zones[i].handler(&scr->zones[i], GUI_DRAW, 0, param);
		}
	}
}

t_GUIEvent	g_event;


int GUI_update()
{
	scanKeys();
	int new_event = 0;
	int pressed = keysPressed(); 	// buttons pressed this loop
	int released = keysReleased();
	int held = keysHeld();				//touch screen
	int repeated = keysRepeated();
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	
	if (GUI.hide)
	{
		if (penIRQread() == false)
		{
			// Show GUI
			GUI.hide = 0;
			powerON(POWER_2D_B);
			setBacklight(POWMAN_BACKLIGHT_TOP_BIT | POWMAN_BACKLIGHT_BOTTOM_BIT); 
		}
	}
	else{
		if((pressed & KEY_TOUCH) && !(held & KEY_TOUCH))
		{
			g_event.event = EVENT_STYLUS_PRESSED;
			g_event.stl.x = TGDSIPC->touchXpx;
			g_event.stl.y = TGDSIPC->touchYpx;		
			new_event = GUI_EVENT_STYLUS;
		}
		
		else if((held & KEY_TOUCH) && !(released & KEY_TOUCH))
		{
			if (penIRQread() == false){
				return 0;
			}
			
			g_event.event = EVENT_STYLUS_DRAGGED;

			g_event.stl.dx = TGDSIPC->touchXpx - g_event.stl.x;
			g_event.stl.dy = TGDSIPC->touchYpx - g_event.stl.y;
			g_event.stl.x = TGDSIPC->touchXpx;
			g_event.stl.y = TGDSIPC->touchYpx;
			new_event = GUI_EVENT_STYLUS;
		
		}
		else if (!(held & KEY_TOUCH) && (released & KEY_TOUCH)) //too much fast: (penIRQread() == false)
		{
			g_event.event = EVENT_STYLUS_RELEASED;
			new_event = GUI_EVENT_STYLUS;
		}	

		else if((TGDSIPC->buttons7 != 0) && GUI.ScanJoypad){
				g_event.event = EVENT_BUTTON_ANY;
				new_event = GUI_EVENT_BUTTON;
				g_event.joy.buttons = TGDSIPC->buttons7;
				g_event.joy.pressed = pressed;
				g_event.joy.repeated = repeated;
				g_event.joy.released = released;
		}
			
		//serve & dispatch (destroys) events
		if (new_event)
		{
			GUI_dispatchEvent(GUI.screen, new_event, &g_event);
		}
		
		if (PendingMessage.msg != 0)
		{
			int ret = 0;
			if (PendingMessage.scr->handler)
				ret = PendingMessage.scr->handler(NULL, 
						PendingMessage.msg, PendingMessage.param, PendingMessage.arg);
			
			if (ret == 0)
			{
				ret = GUI_dispatchMessageNow(PendingMessage.scr,
						PendingMessage.msg, PendingMessage.param, PendingMessage.arg);
			}
			memset(&PendingMessage, 0, sizeof(PendingMessage));
		}
	}
	return 0;
}

int		GUI_start()
{
	GUI.exit = 0;
	g_event.event = 0;
	while (!GUI.exit)
	{
		GUI_update();
	}
	return 0;
}


t_GUIImgList	*GUI_newImageList(int nb)
{
	t_GUIImgList	*img_list;
	
	img_list = malloc(sizeof(t_GUIImgList)+nb*sizeof(t_GUIImage *));
	img_list->nb = nb;
	img_list->cnt = 0;
	return img_list;
}

int		GUI_switchScreen(t_GUIScreen *scr)
{
/*	if (GUI.screen)
		GUI_clearFocus(GUI.screen);*/
	GUI.screen = scr;						
	GUI_drawScreen(scr, NULL);
	return 0;
}

// Needed by qsort
int sort_strcmp(const void *a, const void *b)
{
	return strcasecmp(*(sint8 **)a, *(sint8 **)b);
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



void GUI_setLanguage(int lang)
{
	switch (lang)
	{
	case 0: 
		GUI.string = (sint8 **)&g_snemulds_str_jpn; // JAPANESE
		break;
	case 1:
		GUI.string = (sint8 **)&g_snemulds_str_eng; // ENGLISH
		break;
	case 2:
		GUI.string = (sint8 **)&g_snemulds_str_fr; // FRENCH
		break;
	case 3:		
		GUI.string = (sint8 **)&g_snemulds_str_ger; // GERMAN
		break;		
	case 4:		
		GUI.string = (sint8 **)&g_snemulds_str_ita; // ITALIAN
		break;		
	case 5:		
		GUI.string = (sint8 **)&g_snemulds_str_spa; // SPANISH
		break;		
	case 106:		
		GUI.string = (sint8 **)&g_snemulds_str_pt; // PORTUGUESE
		break;		
	case 107:		
		GUI.string = (sint8 **)&g_snemulds_str_cat; // CATALAN
		break;
	case 108:		
		GUI.string = (sint8 **)&g_snemulds_str_pol; // POLISH
		break;
	case 109:		
		GUI.string = (sint8 **)&g_snemulds_str_nl; // DUTCH
		break;
	case 110:		
		GUI.string = (sint8 **)&g_snemulds_str_dan; // DANISH
		break;		
		
	default:
		GUI.string = (sint8 **)&g_snemulds_str_eng; // ENGLISH
		break;		
	}		
}
