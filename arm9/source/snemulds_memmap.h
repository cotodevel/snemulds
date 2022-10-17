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

#include "typedefsTGDS.h"

#define NOT_LARGE	0
#define USE_PAGING	1

#define SPECIAL_MAP(p) ((int)(p) & 0x80000000)
#define REGULAR_MAP(p) (!((int)(p) & 0x80000000))  	

//indirect mapped memory
#define SNEMULDS_MAP_RELOAD      0x80000000
#define SNEMULDS_MAP_PPU         0x81000000
#define SNEMULDS_MAP_CPU         0x82000000
#define SNEMULDS_MAP_DSP         0x83000000
#define SNEMULDS_MAP_LOROM_SRAM  0x84000000
#define SNEMULDS_MAP_HIROM_SRAM  0x85000000
#define SNEMULDS_MAP_CX4         0x86000000	//I/O  00-3F,80-BF:6000-7FFF
#define SNEMULDS_MAP_NONE        0x8E000000
#define SNEMULDS_MAP_LAST        0x8F000000

#endif /*MEMMAP_H_*/

#ifdef __cplusplus
extern "C" {
#endif

extern uchar *ROM_paging;
extern uint16 *ROM_paging_offs;
extern int ROM_paging_cur;

extern void WriteProtectROM();
extern void FixMap();
extern void MapRAM();
extern void InitLoROMMap(int mode);
extern void InitHiROMMap(int mode);
extern void mem_setCacheBlock(int block, uchar *ptr);
extern void mem_removeCacheBlock(int block);
extern uint8 *mem_checkReload(int block);
extern void InitMap();
extern uint8 IO_getbyte(int addr, uint32 address);
extern void IO_setbyte(int addr, uint32 address, uint8 byte);
extern uint16 IO_getword(int addr, uint32 address);
extern void IO_setword(int addr, uint32 address, uint16 word);
extern uchar mem_getbyte(uint32 offset,uchar bank, int isProgramBankRegister);
extern void mem_setbyte(uint32 offset, uchar bank, uchar byte, int isProgramBankRegister);
extern ushort mem_getword(uint32 offset,uchar bank, int isProgramBankRegister);
extern void mem_setword(uint32 offset, uchar bank, ushort word, int isProgramBankRegister);
extern void *mem_getbaseaddress(uint16 offset, uchar bank, int isProgramBankRegister);
extern void *map_memory(uint16 offset, uchar bank);

#ifdef __cplusplus
}
#endif
