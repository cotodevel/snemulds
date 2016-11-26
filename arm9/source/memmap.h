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

/*
uint8	IO_getbyte(int addr, uint32 address);
void	IO_setbyte(int addr, uint32 address, uint8 byte);
uint16	IO_getword(int addr, uint32 address);
void	IO_setword(int addr, uint32 address, uint16 word);
*/

#endif /*MEMMAP_H_*/

#ifdef __cplusplus
extern "C" {
#endif

extern int OldPC;
extern char *ROM_Image;
extern void fillMemory( void * addr, u32 count, u32 value );
extern void zeroMemory( void * addr, u32 count );
extern int	FS_loadROMPage(char *buf, unsigned int pos, int size);


/*
uchar DMA_port_read(long address);
void DMA_port_write(long address, unsigned short value);
void PPU_port_write(long address, unsigned short value);
uchar PPU_port_read(long address);
void	LOG(char *fmt, ...);
int	FS_loadROMPage(char *buf, unsigned int pos, int size);
*/

#ifdef __cplusplus
}
#endif
