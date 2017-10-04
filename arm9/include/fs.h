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

#include "typedefs.h"
#include "dsregs.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void	FS_printlog(sint8 *buf);
extern void	FS_flog(sint8 *fmt, ...);
extern int		FS_loadROM(sint8 *ROM, sint8 *filename);
extern int		FS_loadROMForPaging(sint8 *ROM, sint8 *filename, int size);
extern int		FS_loadROMPage(sint8 *buf, unsigned int pos, int size);
extern int		FS_loadAllFile(sint8 *filename, sint8 *buf, int *size);
extern int		FS_shouldFreeROM();
extern int		FS_chdir(const sint8 *path);
extern sint8	**FS_getDirectoryList(sint8 *path, sint8 *mask, int *cnt);
extern sint8 	*FS_getFileName(sint8 *filename);


#ifdef __cplusplus
}
#endif


#endif /*FS_H_*/