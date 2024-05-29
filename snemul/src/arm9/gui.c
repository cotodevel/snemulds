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

#include <nds.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "common.h"

#include "gui.h"
#include "fs.h"

t_GUI GUI;

void	LOG(char *fmt, ...)
{
	va_list ap;
	char	buf[101];

	if (!GUI.log)
		return;
	
    va_start(ap, fmt);
    vsnprintf(buf, 100, fmt, ap);
    va_end(ap);
    
    iprintf(buf);
}


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

void	GUI_setZone(t_GUIScreen *scr, int i,
					int x1, int y1, int x2, int y2)
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

int GUI_loadPalette(char *path)
{
	uint8	*data;
	int 	size;
	int		i;
	
	size = FS_getFileSize(path);
	if (size <= 0)
		return -1;	
	
	data = malloc(size);
	
	FS_loadFile(path, (char *)data, size);
	
    for (i = 0; i < MIN(size, GUI_PAL*3); i+=3) 
	  	BG_PALETTE_SUB[i/3] = RGB8(data[i],data[i+1],data[i+2]);
    
    free(data);
    return 0;
}

t_GUIImage	*GUI_loadImage(char *path, int width, int height, int flags)
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

int GUI_addImage(char *path, int w, int h, int flags)
{
	t_GUIImage *img = GUI_loadImage(path, w, h, flags);
	
	GUI.img_list->img[GUI.img_list->cnt++] = img;
	return GUI.img_list->cnt-1;
}

void		GUI_deleteImage(t_GUIImage *image)
{
	free(image);
}

char	g_printfbuf[100];

void		GUI_console_printf(int x, int y, char *fmt, ...)
{
	va_list ap;

	
    va_start(ap, fmt);
    vsnprintf(g_printfbuf, 64, fmt, ap);
    va_end(ap);
    
    iprintf("\x1b[%d;%dH%s", y, x, g_printfbuf);
}

void		GUI_drawHLine(t_GUIZone *zone, int color, int x1, int y1, int x2)
{
	uint16		*ptr;
	
	ptr = GUI.DSFrameBuffer;
	ptr += (zone->x1 + x1) / 2 + ((zone->y1 + y1) * 128);

	uint32	c;
	
	c = color | (color << 8) | (color << 16) | (color << 24);

	swiFastCopy(&c, ptr, (x2-x1)/4 | COPY_MODE_FILL);
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
		swiFastCopy(&c, ptr, (x2-x1)/4 | COPY_MODE_FILL);
		ptr += 128;		
	}	
}

void		GUI_clearScreen(int color)
{
	uint16		*ptr;
	
	ptr = GUI.DSFrameBuffer;

	uint32	c = color | (color << 8) | (color << 16) | (color << 24);

	swiFastCopy(&c, ptr, ((256*192)/4) | COPY_MODE_FILL);
}

void		GUI_drawImage(t_GUIZone *zone, t_GUIImage *image, int x, int y)
{
	uint16		*ptr;
	uint16		*img = NULL;
	FILE		*f = NULL;

//	iprintf("XXX %p %d %d %d %p\n", image, image->width, image->height, image->flags, image->data);

	ptr = GUI.DSFrameBuffer;
	ptr += (zone->x1 + x) / 2 + ((zone->y1 + y) * 128);

	if (image->flags == IMG_NOLOAD)
	{		
		FS_lock();
		f = fopen(image->data, "rb");
	}
	else
		img = image->data;
	

	for (y=0; y < image->height; y++) 
	{
		if (image->flags == IMG_NOLOAD)
		{
			fread(ptr, 4, image->width/4, f);
		}
		else
		{
			swiFastCopy(img, ptr, image->width/4);
			img += (image->width)/2;
		}
		ptr += 128;
	}
	
	if (image->flags == IMG_NOLOAD)
	{
		fclose(f);
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

static t_GUIEvent	g_event;

extern touchPosition superTouchReadXY();

int GUI_update()
{
	int			new_event = 0;
		
	scanKeys();
	uint32 keys = keysHeld() | keysUp() | keysDown();

	//GUI_printf(0, 22, "keys = %x\n", keys);
	
	if (GUI.hide)
	{
		if (keysUp() & KEY_TOUCH)
		{
			// Show GUI
			GUI.hide = 0;
			swiWaitForVBlank();
			powerON(POWER_2D_B);
			setBacklight(PM_BACKLIGHT_TOP | PM_BACKLIGHT_BOTTOM); 
			return 0;
		}
	}
	
	if (keys & KEY_TOUCH)
	{
		//touchPosition touchXY=touchReadXY();
		touchPosition  touchXY=superTouchReadXY();

		if (keysUp() & KEY_TOUCH)
		{
			g_event.event = EVENT_STYLUS_RELEASED;
//				GUI_printf(0, 12, "released\n");
		} else
		if (keysDown() & KEY_TOUCH)
		{
			g_event.event = EVENT_STYLUS_PRESSED;
			g_event.stl.x = touchXY.px;
			g_event.stl.y = touchXY.py;							
//				GUI_printf(0, 13, "pressed\n");
		} else
		{
			g_event.event = EVENT_STYLUS_DRAGGED;
			g_event.stl.dx = touchXY.px - g_event.stl.x;
			g_event.stl.dy = touchXY.py - g_event.stl.y;
			g_event.stl.x = touchXY.px;
			g_event.stl.y = touchXY.py;											
//				GUI_printf(0, 12, "dragged\n");
		}
		new_event = GUI_EVENT_STYLUS;
	} else
		if (keys != 0 && GUI.ScanJoypad)
		{
			g_event.event = EVENT_BUTTON_ANY;
			new_event = GUI_EVENT_BUTTON;
			g_event.joy.buttons = keysHeld();
			g_event.joy.pressed = keysDown();
			g_event.joy.repeated = keysDownRepeat();
			g_event.joy.released = keysUp();
		}
			
	if (new_event)
	{
		/*if (new_event == GUI_EVENT_STYLUS)
			GUI_printf(0, 3, "x = %d y = %d\n", ev.stl.x, ev.stl.y);*/
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
	return 0;
}

int		GUI_start()
{
	GUI.exit = 0;
	g_event.event = 0;
	while (!GUI.exit)
	{
		GUI_update();
		swiWaitForVBlank();
	}
	return 0;
}

void	GUI_init()
{
	keysSetRepeat( 50, 5 );
	
	SUB_DISPLAY_CR = MODE_5_2D |
		DISPLAY_BG3_ACTIVE | DISPLAY_BG0_ACTIVE | DISPLAY_BG1_ACTIVE;
	
	// BG3: FrameBuffer : 64(TILE:4) - 128 Kb
	SUB_BG3_CR = BG_BMP_BASE(4) | BG_BMP8_256x256 | BG_PRIORITY_1;
	// BG1: Text layer : 
	SUB_BG1_CR = BG_MAP_BASE(31) | BG_TILE_BASE(7) | BG_32x32 | BG_16_COLOR;
	// BG0: Background layer :
	SUB_BG0_CR = BG_MAP_BASE(30) | BG_TILE_BASE(7) | BG_32x32 | BG_16_COLOR | BG_PRIORITY_3;
	// Available : 0 - 60 Ko

#if 1	
	consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(31),
					   (u16*)CHAR_BASE_BLOCK_SUB(7), 16);
#endif	
	SUB_BG3_CX = 0;
	SUB_BG3_CY = 0;
	SUB_BG3_XDX = 1 << 8;
	SUB_BG3_XDY = 0;
	SUB_BG3_YDX = 0;
	SUB_BG3_YDY = 1 << 8;
	
	GUI.DSFrameBuffer = (uint16 *)BG_BMP_RAM_SUB(4);
	GUI.DSText = (uint16 *)BG_MAP_RAM_SUB(31);
	GUI.DSBack = (uint16 *)BG_MAP_RAM_SUB(30);
	GUI.DSTileMemory = (uint16 *)BG_TILE_RAM_SUB(7);
	
	BG_PALETTE_SUB[0] = 0;
	BG_PALETTE_SUB[255] = 0xFFFF;
	
	GUI.Palette = &BG_PALETTE_SUB[216];
	GUI.ScanJoypad = 0;
	
	GUI.Palette[0] = RGB8(0, 0, 0); // Black
	GUI.Palette[1] = RGB8(32, 32, 32); // Dark Grey 2
	GUI.Palette[2] = RGB8(64, 64, 64); // Dark Grey
	GUI.Palette[3] = RGB8(128, 128, 128); // Grey
	
	GUI.Palette[4] = RGB8(160, 0, 0); // Dark red / Maroon
	GUI.Palette[5] = RGB8(255, 0, 0); // Red
	GUI.Palette[6] = RGB8(255, 128, 128); // Light red
	GUI.Palette[7] = RGB8(255, 192, 192); // Light red 2

	GUI.Palette[8] = RGB8(0, 0, 128); // Dark blue / Navy blue
	GUI.Palette[9] = RGB8(0, 0, 255); // Blue
	GUI.Palette[10] = RGB8(128, 128, 255); // Light blue
	GUI.Palette[11] = RGB8(192, 192, 255); // Light blue 2

	GUI.Palette[12] = RGB8(0, 160, 0); // Dark green
	GUI.Palette[13] = RGB8(0, 255, 0); // Green
	GUI.Palette[14] = RGB8(128, 255, 128); // Light green
	GUI.Palette[15] = RGB8(192, 255, 192); // Light green 2

	GUI.Palette[16] = RGB8(128, 128, 0); // Olive / Dark yellow 2
	GUI.Palette[17] = RGB8(192, 192, 0); // Gold / Dark yellow
	GUI.Palette[18] = RGB8(255, 255, 0); // Yellow
	GUI.Palette[19] = RGB8(255, 255, 128); // Light yellow  

	GUI.Palette[20] = RGB8(0, 128, 128); // Cerulean / Dark cyan 2
	GUI.Palette[21] = RGB8(0, 192, 192); // Turquoise / Dark cyan
	GUI.Palette[22] = RGB8(0, 255, 255); // Cyan
	GUI.Palette[23] = RGB8(128, 255, 255); // Baby blue / Light cyan    

	GUI.Palette[24] = RGB8(128, 0, 128); // Dark magenta 2 
	GUI.Palette[25] = RGB8(192, 0, 192); // Dark magenta
	GUI.Palette[26] = RGB8(255, 0, 255); // Magenta
	GUI.Palette[27] = RGB8(255, 128, 255); // Pink / Light magenta     

	GUI.Palette[28] = RGB8(255, 127, 0); // Orange
	GUI.Palette[29] = RGB8(255, 0, 127); // Cerise

	GUI.Palette[30] = RGB8(127, 255, 0); // Lime green
	GUI.Palette[31] = RGB8(0, 255, 127); // Spring green

	GUI.Palette[32] = RGB8(0, 127, 255); // Azur / Sky blue
	GUI.Palette[33] = RGB8(127, 0, 255); // Purple / Violet

	// Misc palette

	GUI.Palette[34] = RGB8(150, 75, 0); // Brown
	GUI.Palette[35] = RGB8(245, 222, 179); // Wheat

	GUI.Palette[37] = RGB8(192, 192, 192); // Light grey
	GUI.Palette[38] = RGB8(224, 224, 224); // Light grey 2
	
	GUI.Palette[39] = RGB8(255, 255, 255); // White
	
	GUI.printfy = 0;
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
//	GUI_clearScreen(0);							
	GUI_drawScreen(scr, NULL);
	return 0;
}


