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

#include "typedefsTGDS.h"
#include "consoleTGDS.h"
#include "gui_console_connector.h"
#include "gui_widgets.h"
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char startFilePath[MAX_TGDSFILENAME_LENGTH+1];
extern char startSPCFilePath[MAX_TGDSFILENAME_LENGTH+1];

extern	void	LOG(sint8 *fmt, ...);
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
extern int GUI_update();	
#ifdef __cplusplus
}
#endif

#endif