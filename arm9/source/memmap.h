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

#ifndef MEMMAP_H_
#define MEMMAP_H_

#define NOT_LARGE	0
#define USE_PAGING	1
#define USE_EXTMEM	2

#ifdef ASM_OPCODES
#define SPECIAL_MAP(p) ((int)(p) & 0x80000000)
#define REGULAR_MAP(p) (!((int)(p) & 0x80000000))  	
#else
#define SPECIAL_MAP(p) ((int)(p) < MAP_LAST)
#define REGULAR_MAP(p) ((int)(p) >= MAP_LAST)  	
#endif

#endif /*MEMMAP_H_*/

#ifdef __cplusplus
extern "C" {
#endif

extern int OldPC;
extern char *ROM_Image;
extern void fillMemory( void * addr, u32 count, u32 value );
extern void zeroMemory( void * addr, u32 count );
extern int	FS_loadROMPage(char *buf, unsigned int pos, int size);

#ifdef __cplusplus
}
#endif
