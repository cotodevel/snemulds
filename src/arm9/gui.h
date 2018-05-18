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


typedef struct
{
	char	str[96];
	int		check;
	int		info;
} t_listItem;

typedef struct
{
	int			nb_items;
	int			curs;	
	t_listItem	items[];
} t_list;


typedef struct
{
	int	log;
	
} t_GUI;

extern t_GUI GUI;

t_list	*GUI_createList(int nb_elems);
void	GUI_setItem(t_list *list, int i, char *str, int check, int info);
void	GUI_printf(int x, int y, char *fmt, ...);
void	GUI_printfC(int y, char *fmt, ...);
void	GUI_displayList(t_list *list, int back);
void	GUI_displayMenu(t_list *list);
int		GUI_checkList(t_list *list, int x, int y, int back);
int		GUI_checkMenu(t_list *list, int x, int y);


#endif /*GUI_H_*/
