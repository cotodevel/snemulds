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
#define PAGE_OFFSET_BIGLOROM		3
#define MAP_PAGING      0x80000000
#define MAP_PPU         0x81000000
#define MAP_CPU         0x82000000
#define MAP_DSP         0x83000000
#define MAP_LOROM_SRAM  0x84000000
#define MAP_HIROM_SRAM  0x85000000
#define MAP_NONE        0x8E000000
#define MAP_LAST        0x8F000000

extern u8 * SNES_ROM_ADDRESS_NTR; 
extern u8 * SNES_ROM_ADDRESS_TWL;
extern u8 * SNES_ROM_PAGING_ADDRESS;
extern u32* APU_BRR_HASH_BUFFER_NTR;
extern u8* SDD1_CACHED_VRAM_BLOCKS;

extern bool LoROM_Direct_ROM_Mapping;

#define ROM_MAX_SIZE_NTRMODE_BIGLOROM_PAGEMODE	((int)(2*1024*1024))	//LoROM streaming
#define INTERNAL_PAGING_SIZE_BIGLOROM_PAGEMODE	((int)(448*1024))
#define ROM_MAX_SIZE_TWLMODE	((6*1024*1024)+(512*1024)) //Max ROM size: 6.5MB
#define	PAGE_HIROM		(64*1024)
#define	PAGE_LOROM		(PAGE_HIROM >> 1)
#define SNES_ROM_PAGING_SLOTS (INTERNAL_PAGING_SIZE_BIGLOROM_PAGEMODE/PAGE_HIROM)

struct sdd1_cache_block {
	u32 snesAddressSrc;
	u8 * targetVRAMBuffer;
	int targetVRAMSize;
	//index itself is the cache offset
};

#define INTERNAL_SDD1_CACHED_BLOCKS_SIZE	((int)(576*1024)-(64*1024*4))
#define SDD1_CACHE_BLOCK_SIZE ((int)8*1024) //32K is recommended, but 8K is faster, there may be some artifacts
#define INTERNAL_SDD1_CACHED_SLOTS (INTERNAL_SDD1_CACHED_BLOCKS_SIZE/SDD1_CACHE_BLOCK_SIZE) //S-DD1 caching

extern struct sdd1_cache_block sdd1cache[INTERNAL_SDD1_CACHED_SLOTS];
extern int sdd1cacheIndex;

//334K ~ worth of Hashed Samples from the APU core to remove stuttering
#define APU_BRR_HASH_BUFFER_SIZE (512*1024)

#ifdef _MSC_VER
#define TGDSARM9Free	free
#define TGDSARM9Malloc	malloc
#endif

#define NOT_LARGE	0
#define USE_PAGING	1
#define USE_EXTMEM	2

#define SPECIAL_MAP(p) ((int)(p) & 0x80000000)
#define REGULAR_MAP(p) (!((int)(p) & 0x80000000))  	

#define romPageToBigLoROMSnesPage(ptr, page) ((u8*)ptr+(page&7)*8192-(page << 13))
#define romPageLoROMSnesPage(ptr, page) ((u8*)ptr+(page&3)*8192-(page << 13))

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
extern uint16 *ROM_paging_offs;
extern int ROM_paging_cur;
extern void mem_clear_paging();
extern void mem_init_directROM();
extern void mem_init_paging();
extern void mem_setCacheBlock(int block, uchar *ptr);
extern void mem_removeCacheBlock(int block);
extern uint8 *	mem_checkReload(int blockInPage, uchar bank, uint32 offset); //BigLoROM: SFA2 map additional "LoROM" banks into HiROM area
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
extern void AlphaROMMap(int mode);
extern int ROM_PAGING_SIZE;

extern uint8	IO_SDD1[8]; // 4800 -> 4807
extern u8 * SDD1_WORKBUFFER;
#ifdef ARM9
#ifdef __cplusplus
}
#endif
#endif