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

#include "common.h"

#if defined(_MSC_VER)
#define strcmpi _stricmp
#define strncmpi _strnicmp
#endif

//SnemulDS 0.2 & SnemulDS VS2012 port only start
/*
#define WMAP SNES.WriteMap
#define MAP SNESC.Map
*/
//SnemulDS 0.2 & SnemulDS VS2012 port only end

//Rom Page variables
#define PAGE_OFFSET_LOROM		2	
#define PAGE_OFFSET_HIROM		3
#define MAP_PAGING      0x80000000
#define MAP_PPU         0x81000000
#define MAP_CPU         0x82000000
#define MAP_DSP         0x83000000
#define MAP_LOROM_SRAM  0x84000000
#define MAP_HIROM_SRAM  0x85000000
#define MAP_CX4         0x86000000	//I/O  00-3F,80-BF:6000-7FFF
#define MAP_NONE        0x8E000000
#define MAP_LAST        0x8F000000

extern u8 * SNES_ROM_ADDRESS_NTR; 
extern u8 * SNES_ROM_ADDRESS_TWL;
extern u8 * SNES_ROM_PAGING_ADDRESS;
extern u32* APU_BRR_HASH_BUFFER_NTR;
extern bool LoROM_Direct_ROM_Mapping;

#define ROM_MAX_SIZE_NTRMODE	((int)3*1024*1024) //LoROM direct

#define ROM_MAX_SIZE_NTRMODE_LOROM_PAGEMODE	((int)(1024*1024))			//LoROM streaming
#define ROM_MAX_SIZE_NTRMODE_MMX2	ROM_MAX_SIZE_NTRMODE_LOROM_PAGEMODE	//LoROM streaming: MMX2
#define ROM_MAX_SIZE_NTRMODE_MMX1	((int)(768*1024))		//LoROM streaming: MMX1
#define ROM_MAX_SIZE_TWLMODE	((6*1024*1024)+(512*1024)) //Max ROM size: 6.5MB

#define	PAGE_HIROM		(64*1024)
#define	PAGE_LOROM		(PAGE_HIROM >> 1)

#define SNES_ROM_PAGING_SLOTS (ROM_MAX_SIZE_NTRMODE_LOROM_PAGEMODE/PAGE_HIROM)

#define CX4_PAGING_SIZE (ROM_MAX_SIZE_NTRMODE_LOROM_PAGEMODE/4)
#define CX4_ROM_PAGING_SLOTS (CX4_PAGING_SIZE/PAGE_HIROM)

//334K ~ worth of Hashed Samples from the APU core to remove stuttering
#define APU_BRR_HASH_BUFFER_SIZE (512*1024)

#ifndef TGDSARM9Free
#define TGDSARM9Free	free
#endif

#ifndef TGDSARM9Malloc
#define TGDSARM9Malloc	malloc
#endif

#define NOT_LARGE	0
#define USE_PAGING	1
#define USE_EXTMEM	2

#define SPECIAL_MAP(p) ((int)(p) & 0x80000000)
#define REGULAR_MAP(p) (!((int)(p) & 0x80000000))  	

#define romPageToLoROMSnesPage(ptr, page) ((u8*)ptr+(page&3)*8192-(page << 13))
#define LoROM32kShift ((int)15)
#define LoROMHandleOffset(bank, offset) (int)( ( ( bank >= 0x80 ? bank % 0x80 : bank ) <<LoROM32kShift) + (offset&0x7FFF) )

#endif /*MEMMAP_H_*/

#ifdef ARM9
#ifdef __cplusplus
extern "C" {
#endif
#endif

extern int OldPC;
extern char *ROM_Image;
extern void WriteProtectROM();
extern void FixMap();
extern void MapRAM();
extern void InitLoROMMap(int mode);
extern void InitHiROMMap(int mode);
extern uint16 *ROM_paging_offs;
extern int ROM_paging_cur;
extern void mem_clear_paging();
extern void mem_init_directROM();
extern void mem_init_paging();
extern void mem_setCacheBlock(int block, uchar *ptr);
extern void mem_removeCacheBlock(int block);
extern uint8 *mem_checkReloadHiROM(int block);	//HiROM
extern uint8 *mem_checkReloadLoROM(int blockInPage, int blockInROM);	//LoROM
extern uint8* mem_checkReloadCX4Cache(int bank, uint16 offset);
extern uint8 *	mem_checkReload(int blockInPage, uchar bank, uint32 offset);
extern void InitMap();
extern uint8 IO_getbyte(int addr, uint32 address);
extern void IO_setbyte(int addr, uint32 address, uint8 byte);
extern uint16 IO_getword(int addr, uint32 address);
extern void IO_setword(int addr, uint32 address, uint16 word);
extern uchar mem_getbyte(uint32 offset,uchar bank);
extern void mem_setbyte(uint32 offset, uchar bank, uchar byte);
extern uint16 mem_getword(uint32 offset,uchar bank);
extern void mem_setword(uint32 offset, uchar bank, uint16 word);
extern void *mem_getbaseaddress(uint16 offset, uchar bank);
extern void *map_memory(uint16 offset, uchar bank);

extern u8 * CX4_ROM_PAGING_ADDRESS;
extern int ROM_PAGING_SIZE;

#ifdef ARM9
#ifdef __cplusplus
}
#endif
#endif