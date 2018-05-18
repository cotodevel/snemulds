/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* Free for non-commercial use                             */
/***********************************************************/

#include <nds.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "common.h"

#include "gui.h"

t_GUI GUI;

t_list	*GUI_createList(int nb_elems)
{
	t_list *list;
	list = (t_list *)malloc(sizeof(t_list)+nb_elems*sizeof(t_listItem));
	list->nb_items = nb_elems;
	list->curs = 0;
	return list;
}


void	GUI_setItem(t_list *list, int i, char *str, int check, int info)
{
	strncpy(list->items[i].str, str, 95);
	list->items[i].check = check;
	list->items[i].info = info;
}

void	GUI_printf(int x, int y, char *fmt, ...)
{
	va_list ap;
	char	buf[32];
	
    va_start(ap, fmt);
    vsnprintf(buf, 32, fmt, ap);
    va_end(ap);
    iprintf("\x1b[%d;%dH%s", y, x, buf);
}

void	GUI_printfC(int y, char *fmt, ...)
{
	va_list ap;
	char	buf[32];
	
    va_start(ap, fmt);
    vsnprintf(buf, 32, fmt, ap);
    va_end(ap);
    iprintf("\x1b[%d;%dH%s", y, 16-strlen(buf)/2, buf);    
}

void GUI_displayList(t_list *list)
{
	int	i;
	
	consoleClear();
	if (list->curs < 0)
		return;
    GUI_printfC(0, "Up");
	for (i = 2; i < 22; i++)
	{
		if (list->curs+(i-2) >= list->nb_items)
			break;
		//GUI_printfC(i, "%s", list->items[list->curs+(i-2)].str);
		GUI_printf(0, i, "%s", list->items[list->curs+(i-2)].str);
	}
	GUI_printfC(23, "Down");	
}

void GUI_displayMenu(t_list *list)
{
	int	i;
	
	consoleClear();
	for (i = 0; i < 8; i++)
	{
		GUI_printfC(6+i*2, "%s", list->items[i].str);
	}
	GUI_printf(0, 1, SNEMULDS_TITLE);	
}

void GUI_displayMenuBack(t_list *list)
{
	int	i;
	
	consoleClear();
	for (i = 0; i < 8; i++)
	{
		GUI_printfC(6+i*2, "%s", list->items[i].str);
	}
	GUI_printfC(6+i*2, "<Back>");	
	GUI_printf(0, 1, SNEMULDS_TITLE);	
}


void GUI_displayMenuTitle(char *title, t_list *list)
{
	int	i;
	
	consoleClear();
	for (i = 0; i < 8; i++)
	{
		GUI_printfC(6+i*2, "%s", list->items[i].str);
	}
	GUI_printfC(22, "<Back>");	
	GUI_printfC(1, title);	
}

int	GUI_checkList(t_list *list, int x, int y)
{
	int _x, _y;
	
	_x = x >> 3; _y = y >> 3;
	
	if (_y >= 22) // Down
	{
		if (list->curs < list->nb_items-20)
			list->curs++; 
		GUI_displayList(list);
		return -1;		
	}
	else
	if (_y <= 1) // Up
	{
		if (list->curs > 0)		
		list->curs--;
		GUI_displayList(list);
		return -1;
	}
	else
	if (_y-2+list->curs < list->nb_items)
	{
		return _y-2+list->curs;
	}
	return -1;
}

int	GUI_checkMenu(t_list *list, int x, int y)
{
	int _x, _y;
	
	_x = (x + 4) >> 3; _y = (y + 4) >> 3;
	
	if (_y >= 6 && _y < 6+list->nb_items*2)
	{
		return (_y-6)/2;
	}
	return -1;
}

void	LOG(char *fmt, ...)
{
	va_list ap;
	char	buf[101];

	if (!GUI.log)
		return;
	
    va_start(ap, fmt);
    vsnprintf(buf, 100, fmt, ap);
    va_end(ap);
    
    printf(buf);
}

void T(char *s)
{
}
