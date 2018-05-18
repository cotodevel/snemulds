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

#ifndef FS_H_
#define FS_H_

#include "gui.h"

void	FS_init();
t_list 	*FS_getDirectoryList(char *path, char *mask);
void	FS_printlog(char *buf);
void	FS_flog(char *fmt, ...);
char	*FS_loadROM(char *filename, int *size);
char	*FS_loadROMForPaging(char *filename, int *size);
int		FS_loadROMPage(char *buf, unsigned int pos, int size);
int		FS_loadFile(char *filename, char *buf, int size);
int		FS_loadAllFile(char *filename, char *buf, int *size);
int		FS_shouldFreeROM();
int		FS_chdir(const char *path);

#endif /*FS_H_*/
