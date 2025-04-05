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

//Note: Do not put any of the memory paging in ITCM, it'll break the streaming code.

//Coto: April 2, 2025: Added LoROM streaming. No official games use it, but some rom hacks do, and since these exceed the NDS 4MB EWRAM, this update adds support for NTR mode.

#include <string.h>
#include <stdlib.h>

#ifndef ARM9
#include "cpu.h"
#endif

#include "fs.h"
#include "ppu.h"
#include <string.h>
#include <stdlib.h>
#include "opcodes.h"
#include "common.h"
#include "snes.h"
#include "cfg.h"
#include "snemulds_memmap.h"
#include "c4.h"

#ifdef ARM9
#include "core.h"
#include "c4.h"
#include "ipcfifoTGDSUser.h"
#include "utilsTGDS.h"
#endif

#if defined(_MSC_VER)
u8 bgMapVRAM[128*1024];
u8 spriteMapVRAM[128*1024];
u8 VRAM[128*1024];
#endif

u8 * SNES_ROM_ADDRESS_NTR = NULL;
u8 * SNES_ROM_ADDRESS_TWL = NULL;
u8 * SNES_ROM_PAGING_ADDRESS = NULL;
u32* APU_BRR_HASH_BUFFER_NTR = NULL;
u8 * CX4_ROM_PAGING_ADDRESS = NULL;
bool LoROM_Direct_ROM_Mapping;
int ROM_PAGING_SIZE=0;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WriteProtectROM()
{
	int c;

	for (c = 0; c < 256*8; c++)
		WMAP[c] = MAP[c];

	for (c = 0; c < 0x800; c++)
	{
		if (SNES.BlockIsROM[c])
			WMAP[c] = (uchar*)MAP_NONE;
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void FixMap()
{
	int c;

	for (c = 0; c < 0x800; c++)
	{
		if ( ((u32)MAP[c] != (u32)MAP_PAGING) && REGULAR_MAP(MAP[c]))
		{
			MAP[c] -= ((c << 13)&0xFF0000);
		}
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void MapRAM()
{
	int c;

	for (c = 0; c < 8; c++)
	{
		MAP[c+0x3f0] = SNESC.RAM;
		MAP[c+0x3f8] = SNESC.RAM+0x10000;
		SNES.BlockIsRAM[c+0x3f0] = TRUE;
		SNES.BlockIsRAM[c+0x3f8] = TRUE;
		SNES.BlockIsROM[c+0x3f0] = FALSE;
		SNES.BlockIsROM[c+0x3f8] = FALSE;
	}

	for (c = 0; c < 0x40; c++)
	{
		MAP[c+0x380] = (uchar *)MAP_LOROM_SRAM;
		SNES.BlockIsRAM[c+0x380] = TRUE;
		SNES.BlockIsROM[c+0x380] = FALSE;
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void InitLoROMMap(int mode)
{
	int 	c;
	int 	i;
	int		maxRAM = 0;
	uint8	*largeROM = NULL;
	
	//Given the entire range of 128 Pages of 32K blocks (4MB) of ROM addressing, 2 memory layouts are possible:
	int LoROMMappedRange = 0;
	
		//1) 2MB or less LoROM Size: The bottom (32K x 64) blocks is mirrored, the next upper (32K x 64 pages) is mirrored as well. (Contiguous 2MB LoROM + mirrors)
		if( SNES.ROMSize <= ((2*1024*1024) + (512*1024)) ){
			char * romFname = NULL;
			
			//Fixes Megaman X2 lockups on NTR hardware
			LoROMMappedRange = 0x200000;
			
			//SnemulDS 0.2
			//romFname = (char*)&SNES.ROM_info->title[0];
			
			//SnemulDS 0.6d
			romFname = (char*)&SNES.ROM_info.title[0];

			if ((strncmpi((char*)romFname, "MEGAMAN X2", 10) == 0) && (__dsimode == false)){
				LoROMMappedRange = (8*1024*1024);
			}
		}
		
		//2) 3MB or higher LoROM Size: 
		//The bottom (32K x 64) blocks is mirrored from the 1MB+ bottom half of the 3M+ LoROM. 
		//The upper (32K x 64) blocks is mirrored from the 2MB upper half of the 3M+ LoROM.  (Non-contiguous 32K x 2 4MB LoROM)
		else{
			LoROMMappedRange = SNES.ROMSize;
		}
		
	if (mode == NOT_LARGE)
	{
		// Small ROM, use only SNES ROM size of RAM
		maxRAM = SNES.ROMSize;
		ROM_PAGING_SIZE = 0;
	}

	// Use Paging system... 
	else if (mode == USE_PAGING)
	{
		char * romFname = NULL;
		//SnemulDS 0.2
		//romFname = (char*)&SNES.ROM_info->title[0];
			
		//SnemulDS 0.6d
		romFname = (char*)&SNES.ROM_info.title[0];
		if (
			(
				(strncmpi((char*)romFname, "MEGAMAN X2", 10) == 0) 
				||
				(strncmpi((char*)romFname, "MEGAMAN X3", 10) == 0) 
			)
			&& (__dsimode == false)){
			ROM_PAGING_SIZE = maxRAM = ROM_MAX_SIZE_NTRMODE_MMX2;
		}
		else{
			ROM_PAGING_SIZE = maxRAM = ROM_MAX_SIZE_NTRMODE_MMX1;
		}
	}
	else if (mode == USE_EXTMEM){
		// Extended RAM mode...
		// All RAM available is used static
		// the remaining of mapping use extended RAM
		maxRAM = ROM_MAX_SIZE_NTRMODE;
		largeROM = (uint8 *)0x8000000 + SNES.ROMHeader;
		ROM_PAGING_SIZE = 0;
	}
	else{
		printf("Unhandled InitLoROMMap(): %d", mode);
		printf("Halting.");
		while(1==1){}
	}
	for (c = 0; c < 0x200; c += 8)
	{
		MAP[c+0] = MAP[c+0x400] = SNESC.RAM;		//RAM 000h-1FFFh  Mirror of 7E0000h-7E1FFFh (first 8Kbyte of WRAM)
		SNES.BlockIsRAM[c+0] = SNES.BlockIsRAM[c+0x400] = TRUE;

		MAP[c+1] = MAP[c+0x401] = (uchar *)MAP_PPU; //PPU 2100h-21FFh  I/O Ports (B-Bus) 
		MAP[c+2] = MAP[c+0x402] = (uchar *)MAP_CPU; //CPU 4000h-41FFh  I/O Ports (manual joypad access)
		if(CFG.CX4 == 1){
			MAP[c+3] = MAP[c+0x403] = (uchar *)MAP_CX4;
		}
		else{
			MAP[c+3] = MAP[c+0x403] = (uchar *)MAP_NONE;
		}
		for (i = c+4; i < c+8; i++)
		{
			int pageMap = ((c>>1)<<13)-0x8000;
			if (pageMap > SNES.ROMSize){ 
				// FIXED ?
			}
			else{
				if(pageMap < maxRAM){
					MAP[i] = MAP[i+0x400] = &SNESC.ROM[pageMap];
				}
				else{
					MAP[i] = MAP[i+0x400] = (uint8*)MAP_PAGING;
				}
			}		
			SNES.BlockIsROM[i] = SNES.BlockIsROM[i+0x400] = TRUE;
		}
	}

	if (CFG.DSP1)
	{
		for (c = 0x180; c < 0x200; c += 8)
		{
			MAP[c+4] = MAP[c+0x404] = (uchar *)MAP_DSP;
			MAP[c+5] = MAP[c+0x405] = (uchar *)MAP_DSP;
			MAP[c+6] = MAP[c+0x406] = (uchar *)MAP_DSP;
			MAP[c+7] = MAP[c+0x407] = (uchar *)MAP_DSP;
			SNES.BlockIsROM[c+4] = SNES.BlockIsROM[c+0x404] = FALSE;
			SNES.BlockIsROM[c+5] = SNES.BlockIsROM[c+0x405] = FALSE;
			SNES.BlockIsROM[c+6] = SNES.BlockIsROM[c+0x406] = FALSE;
			SNES.BlockIsROM[c+7] = SNES.BlockIsROM[c+0x407] = FALSE;
		}
	}

	for (c = 0; c < 0x200; c += 8)
	{
		for (i = c; i < c+4; i++)
		{
			if(LoROM_Direct_ROM_Mapping == true){

				int pageMap = ((c>>1)<<13);
				if (pageMap > SNES.ROMSize){ 
					// FIXED ?
				}
				else{
					if(pageMap < maxRAM){
						MAP[i+0x200] = MAP[i+0x600] = &SNESC.ROM[pageMap]; //upper 32K ROM addressed
					}
					else{
						MAP[i+0x200] = MAP[i+0x600] = (uint8*)MAP_PAGING;
					}
				}
			}
			else if ( (((c>>1)<<13)+0x200000 < SNES.ROMSize) && (LoROM_Direct_ROM_Mapping == false))
			{
				MAP[i+0x200] = MAP[i+0x600] = &SNESC.ROM[((c>>1)<<13)+0x200000];
				if (((c>>1)<<13)+0x200000 >= maxRAM)
				{
					if (mode == USE_PAGING)
						MAP[i+0x200] = MAP[i+0x600] = (uint8*)MAP_PAGING;
					else
						MAP[i+0x200] = MAP[i+0x600] = largeROM+((c>>1)<<13)+0x200000;
				}				
			}			
		}
		for (i = c+4; i < c+8; i++)
		{
			if(LoROM_Direct_ROM_Mapping == true){
				int pageMap = ((c>>1)<<13)+0x200000-0x8000;
				if (pageMap > SNES.ROMSize){ 
					// FIXED ?
				}
				else{		
					if(pageMap < maxRAM){
						MAP[i+0x200] = MAP[i+0x600] = &SNESC.ROM[pageMap]; //bottom 32K ROM addressed: mirror of first 2MB or second 2MB chip: todo
					}
					else{
						MAP[i+0x200] = MAP[i+0x600] = (uint8*)MAP_PAGING;
					}
				}
			}
			else if ( ((((c>>1)<<13)+0x200000-0x8000) < SNES.ROMSize) && (LoROM_Direct_ROM_Mapping == false))
			{
				MAP[i+0x200] = MAP[i+0x600] = &SNESC.ROM[((c>>1)<<13)+0x200000-0x8000];
				if (((c>>1)<<13)+0x200000-0x8000 >= maxRAM)
				{
					if (mode == USE_PAGING)
						MAP[i+0x200] = MAP[i+0x600] = (uint8*)MAP_PAGING;
					else
						MAP[i+0x200] = MAP[i+0x600] = largeROM+((c>>1)<<13)+0x200000-0x8000;
				}				
			}				
		}
		for (i = c; i < c+8; i++)
			SNES.BlockIsROM[i+0x200] = SNES.BlockIsROM[i+0x600] = TRUE;
	}

	if (CFG.DSP1)
	{
		for (c = 0; c < 0x80; c++)
		{
			MAP[c+0x700] = (uchar *)MAP_DSP;
			SNES.BlockIsROM[c+0x700] = FALSE;
		}
	}

	MapRAM();
	FixMap();
	WriteProtectROM();
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void InitHiROMMap(int mode)
{
	int 	c;
	int 	i;
	int		maxRAM = 0;
	uint8	*largeROM = NULL;

	if (mode == NOT_LARGE)
	{
		// Small ROM, use only SNES ROM size of RAM, or if TWL mode maps fully ROM from TWL's EWRAM.
		maxRAM = SNES.ROMSize;
		ROM_PAGING_SIZE = 0;
	}
	else if (mode == USE_PAGING)
	{
		// Use Paging system... 
		// Only a part of RAM is used static
		maxRAM = PAGE_HIROM;
		ROM_PAGING_SIZE = ROM_MAX_SIZE_NTRMODE;
	}
	else if (mode == USE_EXTMEM){
		// Extended RAM mode...
		// All RAM available is used static
		// the remaining of mapping use extended RAM
		maxRAM = ROM_MAX_SIZE_NTRMODE;
		largeROM = (uint8 *)0x8000000 + SNES.ROMHeader;
		ROM_PAGING_SIZE = 0;
	}
	else{
		printf("Unhandled InitHiROMMap(): %d", mode);
		printf("Halting.");
		while(1==1){}
	}
	for (c = 0; c < 0x200; c += 8)
	{
		MAP[c+0] = MAP[c+0x400] = SNESC.RAM;
		SNES.BlockIsRAM[c+0] = SNES.BlockIsRAM[c+0x400] = TRUE;

		MAP[c+1] = MAP[c+0x401] = (uchar *)MAP_PPU;
		MAP[c+2] = MAP[c+0x402] = (uchar *)MAP_CPU;
		if (CFG.DSP1)
			MAP[c+3] = MAP[c+0x403] = (uchar *)MAP_DSP;
		else
			MAP[c+3] = MAP[c+0x403] = (uchar *)MAP_NONE;

		for (i = c+4; i < c+8; i++)
		{
			if ( (c<<13) < SNES.ROMSize)
			{
				MAP[i] = MAP[i+0x400] = &SNESC.ROM[c<<13];
				if ((c<<13) >= maxRAM)
				{
					if (mode == USE_PAGING)
						MAP[i] = MAP[i+0x400] = (uint8*)MAP_PAGING;
					else
						MAP[i] = MAP[i+0x400] = largeROM+(c<<13);
				}				
			}
			SNES.BlockIsROM[i] = SNES.BlockIsROM[i+0x400] = TRUE;			
		}
	}

	for (c = 0; c < 16; c++)
	{
		MAP[0x183+(c<<3)] = (uchar *)MAP_HIROM_SRAM;
		MAP[0x583+(c<<3)] = (uchar *)MAP_HIROM_SRAM;
		SNES.BlockIsRAM[0x183+(c<<3)] = TRUE;
		SNES.BlockIsRAM[0x583+(c<<3)] = TRUE;
	}

	for (c = 0; c < 0x200; c += 8)
	{
		for (i = c; i < c+8; i++)
		{
			if ( (c<<13) < SNES.ROMSize)
			{
				MAP[i+0x200] = MAP[i+0x600] = &SNESC.ROM[c<<13];
				if ((c<<13) >= maxRAM)
				{
					if (mode == USE_PAGING)
						MAP[i+0x200] = MAP[i+0x600] = (uint8*)MAP_PAGING;
					else
						MAP[i+0x200] = MAP[i+0x600] = largeROM+(c<<13);
				}				
			}	
			SNES.BlockIsROM[i+0x200] = SNES.BlockIsROM[i+0x600] = TRUE;			
		}
	}
	MapRAM();
	FixMap();
	WriteProtectROM();
}

uint16 *ROM_paging_offs= NULL;
int ROM_paging_cur = 0;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void mem_clear_paging(){
	if (ROM_paging_offs)
	{
		TGDSARM9Free(ROM_paging_offs);
		ROM_paging_offs = NULL;
	}

	if(SNES_ROM_ADDRESS_NTR){
		TGDSARM9Free(SNES_ROM_ADDRESS_NTR);
	}
	if(SNES_ROM_ADDRESS_TWL){
		TGDSARM9Free(SNES_ROM_ADDRESS_TWL);
	}

	SNES_ROM_PAGING_ADDRESS = NULL;
	CX4_ROM_PAGING_ADDRESS  = NULL;
	APU_BRR_HASH_BUFFER_NTR = NULL;
}

void	mem_init_paging()
{
	mem_clear_paging();
	#ifdef ARM9
	SNES_ROM_ADDRESS_NTR = (u8*)(0x20F0000);
	SNES_ROM_ADDRESS_TWL = (u8*)(0x20F9F00);
#endif
#ifdef _MSC_VER
	SNES_ROM_ADDRESS_NTR = (u8*)TGDSARM9Malloc(ROM_MAX_SIZE_NTRMODE + ROM_MAX_SIZE_NTRMODE + CX4_PAGING_SIZE + APU_BRR_HASH_BUFFER_SIZE);
	SNES_ROM_ADDRESS_TWL = (u8*)TGDSARM9Malloc(ROM_MAX_SIZE_TWLMODE + ROM_MAX_SIZE_NTRMODE + CX4_PAGING_SIZE + APU_BRR_HASH_BUFFER_SIZE);
#endif
	SNES_ROM_PAGING_ADDRESS = (u8*)(SNES_ROM_ADDRESS_NTR+ROM_MAX_SIZE_NTRMODE);
	CX4_ROM_PAGING_ADDRESS = (u8*)(((int)SNES_ROM_PAGING_ADDRESS) + ROM_MAX_SIZE_NTRMODE);	//(334*1024) = 342016 bytes / 64K blocks = 5 pages less useable on paging mode
	APU_BRR_HASH_BUFFER_NTR = (u32*)(((int)CX4_ROM_PAGING_ADDRESS) + CX4_PAGING_SIZE);
	
	memset(SNES_ROM_PAGING_ADDRESS, 0, ROM_MAX_SIZE_NTRMODE);
	memset(CX4_ROM_PAGING_ADDRESS, 0, CX4_PAGING_SIZE);
	memset(APU_BRR_HASH_BUFFER_NTR, 0, APU_BRR_HASH_BUFFER_SIZE);
	ROM_paging_offs = (uint16 *)TGDSARM9Malloc((ROM_MAX_SIZE_NTRMODE/PAGE_HIROM)*2);
	if (!ROM_paging_offs)
	{
		printf("Not enough memory for ROM paging (2).\n");
		while (1){}
	}
	memset(ROM_paging_offs, 0xFF, (ROM_MAX_SIZE_NTRMODE/PAGE_HIROM)*2);
	ROM_paging_cur = 0;
}


#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void mem_removeCacheBlock(int block)
{
	int i;
	if (SNES.HiROM){
		block <<= PAGE_OFFSET_HIROM;
		for (i = 0; i < PAGE_HIROM/8192; i++, block++)
		{

			if ((block & 7) >= 4)
			{
				MAP[block] = (u8*)MAP_PAGING;
				MAP[block+0x400] = (u8*)MAP_PAGING;
			}
			if (SNES.BlockIsROM[block+0x200])
				MAP[block+0x200] = (u8*)MAP_PAGING;
			MAP[block+0x600] = (u8*)MAP_PAGING;
		}
	}
	else{
		block <<= PAGE_OFFSET_LOROM;
		for (i = 0; i < PAGE_LOROM/8192; i++, block++)
		{
			//LoROM: mirroring goes here. (for X-X3 AP fixes and then MMX3ZP)
			if ((block & 7) >= 4)
			{
				MAP[block] = (u8*)MAP_PAGING;
				MAP[block+0x400] = (u8*)MAP_PAGING;
			}
			if (SNES.BlockIsROM[block+0x200])
				MAP[block+0x200] = (u8*)MAP_PAGING;
			MAP[block+0x600] = (u8*)MAP_PAGING;
		}
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void mem_setCacheBlock(int block, uchar *ptr)
{
	int i;
	if (SNES.HiROM){
		block <<= PAGE_OFFSET_HIROM;
		for (i = 0; i < PAGE_HIROM/8192; i++, block++)
		{

			if ((block & 7) >= 4)
			{
				MAP[block] = ptr+i*8192-(block << 13);
				MAP[block+0x400] = ptr+i*8192-((block+0x400) << 13);
			}
			if (SNES.BlockIsROM[block+0x200])
				MAP[block+0x200] = ptr+i*8192-((block+0x200) << 13);
			MAP[block+0x600] = ptr+i*8192-((block+0x600) << 13);
		}
	}
	else{
		block <<= PAGE_OFFSET_LOROM;
		for (i = 0; i < PAGE_LOROM/8192; i++, block++)
		{
			//LoROM: mirroring goes here. (for X-X3 AP fixes and then MMX3ZP)
			if ((block & 7) >= 4)
			{
				MAP[block] = romPageToLoROMSnesPage(ptr, block);
				MAP[block+0x400] = romPageToLoROMSnesPage(ptr, (block+0x400)); 
			}
			if (SNES.BlockIsROM[block+0x200])
				MAP[block+0x200] = romPageToLoROMSnesPage(ptr, (block+0x200)); 
			MAP[block+0x600] = romPageToLoROMSnesPage(ptr, (block+0x600));  
		}
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
uint8 *	mem_checkReload(int blockInPage, uchar bank, uint32 offset)
{
	uint8* addr = NULL;
	if (SNES.HiROM){
		addr = mem_checkReloadHiROM(blockInPage);
	}
	else{

		int addressRom = LoROMHandleOffset(bank, offset);
		int blockRom = (addressRom>>13)&0x7FF;
		addr = mem_checkReloadLoROM(blockInPage, blockRom);
	}
	return addr;
}

static int storedNDSBanks[CX4_ROM_PAGING_SLOTS]; //Used by cx4 cache
static int cx4PagingOffset = 0;
//int block = internal block position pointing to 64K page
//int snesBank = physical snesBank. (if snesBank == PC Bank, reload entire bank)
//Note: This method will always resolve 64K pages, relative offsets are handled in caller function.
//		Because of this, it requires standalone memory for cx4 pages.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__((optnone))
#endif
uint8* mem_checkReloadCX4Cache(int bank, uint16 offset) {
	unsigned char* ptr; int ret;
#ifdef ARM9
	if (!CFG.LargeROM) {
		return NULL;
	}
#endif
	//We're here if a bank isn't mapped. search for it and if it's mapped, use it.
	int 	i;
	for (i = 0; i < CX4_ROM_PAGING_SLOTS; i++) {
		if (storedNDSBanks[i] == (int)(bank)) {
			return (CX4_ROM_PAGING_ADDRESS + (i * PAGE_HIROM));
		}

		//LoROM only: use upper bank if already loaded
		else if ((storedNDSBanks[i] - 1) == (int)(bank)) {
			return (CX4_ROM_PAGING_ADDRESS + ((i - 1) * PAGE_HIROM) + PAGE_LOROM);
		}
	}

	storedNDSBanks[cx4PagingOffset] = (int)bank;
	ptr = CX4_ROM_PAGING_ADDRESS + (cx4PagingOffset * PAGE_HIROM);
	
	cx4PagingOffset++;//alloc next one 
	if (cx4PagingOffset >= CX4_ROM_PAGING_SLOTS) {
		cx4PagingOffset = 0;
	}
	ret = FS_loadROMPage((char*)ptr, (int)(bank * 0x8000), PAGE_HIROM);
#ifdef ARM9
	//coherent_user_range_by_size((uint32)ptr, (int)PAGE_HIROM);	//Make coherent new page
#endif
	return ptr;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
uint8 *mem_checkReloadHiROM(int block){
	int i;uchar *ptr;int ret;
	if (CFG.LargeROM == false){
		return NULL;
	}
	i = (block & 0x1FF) >> PAGE_OFFSET_HIROM;

	//SnemulDS 0.6d
	#ifdef ARM9
	if (ROM_paging_offs[ROM_paging_cur] != 0xFFFF){
		// Check that we are not unloading program code 
		uint32 cPC = ((S&0xFFFF) << 16)|(uint32)((sint32)PCptr+(sint32)SnesPCOffset);
		uint32 PC_blk = ((cPC >> 13)&0x1FF) >> PAGE_OFFSET_HIROM;
		if (ROM_paging_offs[ROM_paging_cur] == PC_blk){
			ROM_paging_cur++;
			if (ROM_paging_cur >= ((ROM_PAGING_SIZE/PAGE_HIROM) - 6) ){
				ROM_paging_cur = 0;
			}
		}
		if (ROM_paging_offs[ROM_paging_cur] != 0xFFFF){
			mem_removeCacheBlock(ROM_paging_offs[ROM_paging_cur]);
		}
	}
	#endif

	ROM_paging_offs[ROM_paging_cur] = i;
	ptr = SNES_ROM_PAGING_ADDRESS+(ROM_paging_cur*PAGE_HIROM);
	ret = FS_loadROMPage((char*)ptr, SNES.ROMHeader+i*PAGE_HIROM, PAGE_HIROM);

	#ifdef ARM9
	coherent_user_range_by_size((uint32)ptr, (int)PAGE_HIROM);	//Make coherent new page
	#endif
	
	mem_setCacheBlock(i, ptr); // Give Read-only memory

	ROM_paging_cur++;
	if (ROM_paging_cur >= ((ROM_PAGING_SIZE/PAGE_HIROM) - 6) ){
		ROM_paging_cur = 0;
	}
	return ptr+(block&7)*8192-(block << 13);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
uint8 *	mem_checkReloadLoROM(int blockInPage, int blockInROM)
{
	int i;uchar *ptr;int ret;
	if (CFG.LargeROM == false){
		return NULL;
	}
	i = (blockInPage & 0x1FF) >> PAGE_OFFSET_LOROM; //8k x (1 << 3) = 64K pages
	if (ROM_paging_offs[ROM_paging_cur] != 0xFFFF){
		mem_removeCacheBlock(ROM_paging_offs[ROM_paging_cur]);
	}

	ROM_paging_offs[ROM_paging_cur] = i;
	ptr = SNES_ROM_PAGING_ADDRESS+(ROM_paging_cur*PAGE_HIROM);
	ret = FS_loadROMPage((char*)ptr, SNES.ROMHeader+ ((((blockInROM & 0x1FF)<<13)>>LoROM32kShift)<<LoROM32kShift) , PAGE_HIROM); //((RomOffset * 8192) / 32K) * 32K

	#ifdef ARM9
	coherent_user_range_by_size((uint32)ptr, (int)PAGE_HIROM);	//Make coherent new page
	#endif

	mem_setCacheBlock(i, ptr); // Give Read-only memory

	ROM_paging_cur++;
	if (ROM_paging_cur >= ((ROM_PAGING_SIZE/PAGE_HIROM) - 6) ){
		ROM_paging_cur = 0;
	}
	return romPageToLoROMSnesPage(ptr, blockInPage);
}

void	InitMap(){
	int i;
	int mode = 0;
	for (i = 0; i < 256*8; i++){
		MAP[i] = (uint8*)MAP_NONE;	
	}
	mode = (CFG.LargeROM == false) ? NOT_LARGE : USE_PAGING; 
	if (SNES.HiROM){
		InitHiROMMap(mode);
	}
	else{
		InitLoROMMap(mode);
	}
}


uint8 IO_getbyte(int addr, uint32 address){
	
	uint8 result;
	switch ((int)addr)
	{
	case MAP_PPU:{
		//START_PROFILE(IOREGS, 2);
		result= PPU_port_read(address&0xFFFF);
		//END_PROFILE(IOREGS, 2);
		return result;
	}
	break;
	case MAP_CPU:{
		//START_PROFILE(IOREGS, 2);
		result= DMA_port_read(address&0xFFFF);
		//END_PROFILE(IOREGS, 2);
		return result;
	}
	break;
	case MAP_LOROM_SRAM:{
		if (SNESC.SRAMMask == 0)
			return 0;
		return *(SNESC.SRAM+((address&SNESC.SRAMMask)));
	}
	break;

	case MAP_CX4:{
		return (S9xGetC4 ((address) & 0xffff));
	}
	break;
	
	case MAP_HIROM_SRAM:{
		if (SNESC.SRAMMask == 0)
			return 0;
		return *(SNESC.SRAM+(((address&0x7fff)-0x6000+
								((address&0xf0000)>>3))&SNESC.SRAMMask));
	}
	break;
	default:
		return 0;
	}
}


void IO_setbyte(int addr, uint32 address, uint8 byte){
	
	switch ((int)addr)
	{
	case MAP_PPU:{
		//START_PROFILE(IOREGS, 2);
		PPU_port_write(address&0xFFFF,byte);
		//END_PROFILE(IOREGS, 2);
		return;
	}break;
	case MAP_CPU:{
		//START_PROFILE(IOREGS, 2);
		DMA_port_write(address&0xFFFF,byte);
		//END_PROFILE(IOREGS, 2);
		return;
	}break;
	case MAP_LOROM_SRAM:{
		if (SNESC.SRAMMask == 0)
			return;
		*(SNESC.SRAM+((address&SNESC.SRAMMask))) = byte;
		SNES.SRAMWritten = 1;
		return;
	}break;
	
	case MAP_CX4:{
		S9xSetC4(byte, address & 0xffff);
		return;
	}
	break;
	
	case MAP_HIROM_SRAM:{
		if (SNESC.SRAMMask == 0)
			return;
		*(SNESC.SRAM+(((address&0x7fff)-0x6000+
				((address&0xf0000)>>3))&SNESC.SRAMMask)) = byte;
		SNES.SRAMWritten = 1;
		return;
	}break;
	case MAP_NONE:
		return;
	}
}


uint16 IO_getword(int addr, uint32 address)
{

	uint16 result;
	switch ((int)addr)
	{
	case MAP_PPU:{
		//START_PROFILE(IOREGS, 2);
		result= PPU_port_read(address&0xFFFF)+
			(PPU_port_read((address+1)&0xFFFF)<<8);
		//END_PROFILE(IOREGS, 2);
		return result;
	}break;
	case MAP_CPU:{
		//START_PROFILE(IOREGS, 2);
		result= DMA_port_read(address&0xFFFF)+
			(DMA_port_read((address+1)&0xFFFF)<<8);
		//END_PROFILE(IOREGS, 2);
		return result;
	}break;
	case MAP_LOROM_SRAM:{
		if (SNESC.SRAMMask == 0)
			return 0;
		result = SNESC.SRAM[address&SNESC.SRAMMask];
		result |= SNESC.SRAM[(address+1)&SNESC.SRAMMask]<<8;
		return result;
	}break;
	
	case MAP_CX4:{
		return (S9xGetC4 (address & 0xffff) |
			(S9xGetC4 ((address + 1) & 0xffff) << 8));
	}
	break;
	
	case MAP_HIROM_SRAM:{
		if (SNESC.SRAMMask == 0)
			return 0;
		address = ((address&0x7fff)-0x6000+((address&0xf0000)>>3));
		result = SNESC.SRAM[address&SNESC.SRAMMask];
		result |= SNESC.SRAM[(address+1)&SNESC.SRAMMask]<<8;
		return result;
	}break;
	default:
		return 0;
	}
}


void IO_setword(int addr, uint32 address, uint16 word){
	
	switch ((int)addr)
	{
	case MAP_PPU:{
		//START_PROFILE(IOREGS, 2);
		PPU_port_write(address&0xFFFF,word&0xFF);
		PPU_port_write((address+1)&0xFFFF,word>>8);
		//END_PROFILE(IOREGS, 2);
		return;
	}break;
	case MAP_CPU:{
		//START_PROFILE(IOREGS, 2);
		DMA_port_write(address&0xFFFF,word&0xFF);
		DMA_port_write((address+1)&0xFFFF,word>>8);
		//END_PROFILE(IOREGS, 2);
		return;
	}break;
	case MAP_LOROM_SRAM:{
		if (SNESC.SRAMMask == 0)
			return;
		SNESC.SRAM[address&SNESC.SRAMMask] = word&0xFF;
		SNESC.SRAM[(address+1)&SNESC.SRAMMask] = word>>8;
		SNES.SRAMWritten = 1;
		return;
	}break;

	case MAP_CX4:{
		S9xSetC4 (word & 0xff, address & 0xffff);
		S9xSetC4 ((uint8) (word >> 8), (address + 1) & 0xffff);
		return;
	}
	break;
	
	case MAP_HIROM_SRAM:{
		if (SNESC.SRAMMask == 0)
			return;
		address = ((address&0x7fff)-0x6000+((address&0xf0000)>>3));
		SNESC.SRAM[address&SNESC.SRAMMask] = word&0xFF;
		SNESC.SRAM[(address+1)&SNESC.SRAMMask] = word>>8;
		SNES.SRAMWritten = 1;
		return;
	}break;
	case MAP_NONE:
		return;
	}
}

uchar mem_getbyte(uint32 offset,uchar bank)
{
	int address = (bank<<16)+offset;
	int block;
	uchar *addr;

	block = (address>>13)&0x7FF;
	addr = MAP[block];

	if ((u32)addr == (u32)MAP_PAGING){
		addr = mem_checkReload(block, bank, offset);
	}
	if (REGULAR_MAP(addr)){ //if address is within indirect mapped memory define (snes.h), a forced (below) IO_xxxx opcode takes place
		return *(addr+address);
	}
	else{
		return IO_getbyte((int)addr, address);
	}
}


void mem_setbyte(uint32 offset, uchar bank, uchar byte)
{
	int address = (bank<<16)+offset;
	int block;
	uchar *addr;

	block = (address>>13)&0x7FF;
	addr = WMAP[block];
	if ((u32)addr == (u32)MAP_PAGING){
		addr = mem_checkReload(block, bank, offset);
	}
	
	if (REGULAR_MAP(addr) ){ //if address is within indirect mapped memory define (snes.h), a forced (below) IO_xxxx opcode takes place
		*(addr+address) = byte;
	}
	else{
		IO_setbyte((int)addr, address, byte);
	}
}


uint16 mem_getword(uint32 offset,uchar bank)
{
	int address = (bank<<16)+offset;
	int block;
	uchar *addr;

	block = (address>>13)&0x7FF;
	addr = MAP[block];

	if ((u32)addr == (u32)MAP_PAGING){
		addr = mem_checkReload(block, bank, offset);
	}
	
	if (REGULAR_MAP(addr)){ //if address is within indirect mapped memory define (snes.h), a forced (below) IO_xxxx opcode takes place
		return GET_WORD16(addr+address);
	}
	else{
		return IO_getword((int)addr, address);
	}
}


void mem_setword(uint32 offset, uchar bank, uint16 word)
{
	int address = (bank<<16)+offset;
	int block;
	uchar *addr;

	//  CPU.WaitAddress = -1;
	block = (address>>13)&0x7FF;
	addr = WMAP[block];
	if ((u32)addr == (u32)MAP_PAGING){
		addr = mem_checkReload(block, bank, offset);
	}
	
	if (REGULAR_MAP(addr)){ //if address is within indirect mapped memory define (snes.h), a forced (below) IO_xxxx opcode takes place
		SET_WORD16(addr+address, word);
	}
	else{
		IO_setword((int)addr, address, word);
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void *mem_getbaseaddress(uint16 offset, uchar bank)
{
	int block;
	int address = (bank<<16)+offset;
	uchar *ptr;

	block = (address>>13)&0x7FF;
	ptr = MAP[block];

	if ((u32)ptr == (u32)MAP_PAGING){
		ptr = mem_checkReload(block, bank, offset);
	}
	if (REGULAR_MAP(ptr))
		return ptr+(bank<<16);

	switch ((int)ptr)
	{
	case MAP_PPU:
	case MAP_CPU:
		//      case MAP_DSP:
		return 0;
	case MAP_LOROM_SRAM:
		return SNESC.SRAM;
	case MAP_HIROM_SRAM:
		return SNESC.SRAM;
	default:
		return 0;
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void *map_memory(uint16 offset, uchar bank)
{
	int block;
	int address = (bank<<16)+offset;
	uchar *ptr;

	block = (address>>13)&0x7FF;
	ptr = MAP[block];

	if ((u32)ptr == (u32)MAP_PAGING){
		ptr = mem_checkReload(block, bank, offset);
	}

	if (REGULAR_MAP(ptr))
		return ptr+address;

	switch ((int)ptr)
	{
	case MAP_PPU:
	case MAP_CPU:
		//      case MAP_DSP:
		return 0;
	case MAP_LOROM_SRAM:
		return SNESC.SRAM+(offset&SNESC.SRAMMask);
	case MAP_HIROM_SRAM:
		return SNESC.SRAM+(((address&0x7fff)-0x6000+
						((address&0xf0000)>>3))&SNESC.SRAMMask);
	case MAP_CX4:{
		return SNESC.C4RAM + (offset&0x1FFF); //(snes.h: intercept #define MAP_CX4         0x86000000	//I/O  00-3F,80-BF:6000-7FFF)
	}
	break;						
	
	default:
		return 0;
	}
}
