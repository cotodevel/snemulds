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

// GUI Colors

#define GUI_PAL			216
//#define GUI_BLACK		0
#define GUI_BLACK		(GUI_PAL+0)
#define GUI_DARKGREY2	(GUI_PAL+1)
#define GUI_DARKGREY	(GUI_PAL+2)
#define GUI_GREY		(GUI_PAL+3)

#define GUI_DARKRED		(GUI_PAL+4)
#define GUI_RED			(GUI_PAL+5)
#define GUI_LIGHTRED	(GUI_PAL+6)

#define GUI_DARKGREEN	(GUI_PAL+12)
#define GUI_GREEN		(GUI_PAL+13)
#define GUI_LIGHTGREEN	(GUI_PAL+14)

#define GUI_DARKBLUE	(GUI_PAL+8)
#define GUI_BLUE		(GUI_PAL+9)
#define GUI_LIGHTBLUE	(GUI_PAL+10)

#define GUI_DARKYELLOW	(GUI_PAL+17)
#define GUI_YELLOW		(GUI_PAL+18)
#define GUI_LIGHTYELLOW	(GUI_PAL+19)

#define GUI_LIGHTGREY	(GUI_WHITE-2)
#define GUI_LIGHTGREY2	(GUI_WHITE-1)
#define GUI_WHITE		255

#define _STR(x) GUI.string[x]

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


typedef struct s_GUIZone t_GUIZone;

typedef int (*t_GUIHandler)(t_GUIZone *zone, int message, int param, void *arg);

typedef struct
{
    int		width;
    int		height;
    int		bpp;
    uint8	*palette;   
    uint8   data[];
} t_GUIGlyph;

typedef struct
{
    int		width;
    int		height;
    int		bpp;
    uint8	*palette;   
    uint16   data[];
} t_GUIGlyph16;


typedef struct
{
    int height;
    int	offset;
    
    int space;
    int interline;

    t_GUIGlyph **glyphs;
} t_GUIFont;

struct s_GUIZone
{
	int				x1;
	int 			y1;
	int 			x2;
	int				y2;

	uint8			id;	
	uint8			state;	
	uint16			keymask;

	t_GUIHandler	handler;
	void			*data;
	
	t_GUIFont		*font;
};

typedef struct
{
	uint16	width;
	uint8	height;
	uint8	flags;
	
	void	*data;
} t_GUIImage;

typedef struct
{
	int			nb;
	int			cnt;
	t_GUIImage	*img[];
} t_GUIImgList;

typedef struct
{
	int			nb_zones;
	int			curs;
	int			stylus_zone;
	int			flags;
	t_GUIHandler	handler;
	t_GUIImgList	*img_list;
	
	uint16		last_focus; // Number of focusable zones 
	uint16		incr_focus; // Increment factor for joypad focus
	
	t_GUIZone	zones[];
} t_GUIScreen;



typedef struct
{
// TODO : put in one flag field
	int	log;
	int exit;
	int hide;	
	int ScanJoypad; 
	
	u16	*DSFrameBuffer; // Frame Buffer Layer
	u16	*DSText; // Text Layer
	u16	*DSBack; // Back Text Layer;
	u16	*DSTileMemory;
	
	u16	*Palette;
	
	t_GUIScreen	*screen;
	
	char	**string;
	
	t_GUIImgList	*img_list;
	
	uint16	printfy;
} t_GUI;

typedef struct
{
	t_GUIScreen	*scr;
	int			msg;
	int 		param;
	void		*arg;
} t_GUIMessage;

/* --------------------------------------------------------------------------- */

extern t_GUI GUI;


t_GUIScreen	*GUI_newScreen(int nb_elems);
void	GUI_setZone(t_GUIScreen *scr, int i, int x1, int y1, int x2, int y2);
void	GUI_linkObject(t_GUIScreen *scr, int i, void *data, t_GUIHandler handler);

int			GUI_loadPalette(char *path);

t_GUIImage	*GUI_loadImage(char *path, int width, int height, int flags);
void		GUI_deleteImage(t_GUIImage *image);

t_GUIImgList	*GUI_newImageList(int nb);

int 		GUI_addImage(char *path, int w, int h, int flags);

int			GUI_getFontHeight(t_GUIZone *zone);

void		GUI_drawHLine(t_GUIZone *zone, int color, int x1, int y1, int x2);
void		GUI_drawVLine(t_GUIZone *zone, int color, int x1, int y1, int y2);
void		GUI_drawRect(t_GUIZone *zone, int color, int x1, int y1, int x2, int y2);

void		GUI_drawBar(t_GUIZone *zone, int color, int x1, int y1, int x2, int y2);
void		GUI_console_printf(int cx, int cy, char *fmt, ...);
void		GUI_printf(char *fmt, ...);
int		    GUI_drawText(t_GUIZone *zone, uint16 x, uint16 y, int col, char *text);
void		GUI_drawImage(t_GUIZone *zone, t_GUIImage *image, int x, int y);

int 		GUI_drawAlignText(t_GUIZone *zone, int flags, int y, int col, char *text);

int			GUI_sendMessage(t_GUIScreen *scr, int i, int msg, int param, void *arg);
int			GUI_dispatchEvent(t_GUIScreen *scr, int event, void *param);
int			GUI_dispatchMessage(t_GUIScreen *scr, int msg, int param, void *arg);

void		GUI_drawScreen(t_GUIScreen *screen, void *param);

int			GUI_getStrWidth(t_GUIZone *zone, char *text);

int			GUI_setFocus(t_GUIScreen *scr, int id);
void		GUI_clearFocus(t_GUIScreen *scr);

void		GUI_init();

void		GUI_clearScreen(int color);
int 		GUI_update();
int			GUI_start();

int			GUI_switchScreen(t_GUIScreen *scr);

int			GUI_getZoneTextHeight(t_GUIZone *zone);

#endif /*GUI_H_*/
