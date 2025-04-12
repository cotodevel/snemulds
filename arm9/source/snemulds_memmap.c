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

//Coto: April 2, 2025: Added LoROM streaming. This enables several games using the BigLoROM, SpecialLoROM or ExLoROM maps to run in NTR hardware as well TWL hardware.

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
#include "sdd1.h"
#include "sdd1emu.h"

#ifdef ARM9
#include "core.h"
#include "ipcfifoTGDSUser.h"
#include "utilsTGDS.h"
#endif

#if defined(_MSC_VER)
u8 bgMapVRAM[128*1024];
u8 spriteMapVRAM[128*1024];
u8 VRAM[128*1024];
#endif

u8 * SDD1_WORKBUFFER = NULL;
u8 * SNES_ROM_ADDRESS_NTR = NULL;
u8 * SNES_ROM_ADDRESS_TWL = NULL;
u8 * SNES_ROM_PAGING_ADDRESS = NULL;
u32* APU_BRR_HASH_BUFFER_NTR = NULL;
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
void AlphaROMMap(int mode)
{
	int 	c;
	int 	i;
	int		maxRAM = 0;
	uint8	*largeROM = SNESC.ROM;
	
	if (mode == NOT_LARGE)
	{
		// Small ROM, use only SNES ROM size of RAM
		maxRAM = SNES.ROMSize;
	}
	else if (mode == USE_PAGING)
	{
		char * romFname = NULL;
		//SnemulDS 0.2
		//romFname = (char*)&SNES.ROM_info->title[0];
			
		//SnemulDS 0.6d
		romFname = (char*)&SNES.ROM_info.title[0];
		if (
			(
				(strncmpi((char*)romFname, "STREET FIGHTER ALPHA", 20) == 0)
				||
				(strncmpi((char*)romFname, "STAR OCEAN", 10) == 0)
			)
			&& (__dsimode == false)){
			maxRAM = ROM_MAX_SIZE_NTRMODE_BIGLOROM_PAGEMODE;
			ROM_PAGING_SIZE = INTERNAL_PAGING_SIZE_BIGLOROM_PAGEMODE;
		}
		else{
			maxRAM = ROM_MAX_SIZE_TWLMODE; //TWL mode has plenty of RAM available for ROM mapping
			ROM_PAGING_SIZE = 0;
		}
	}
	else{
		printf("Unhandled AlphaROMMap(): %d", mode);
		printf("Halting.");
		while(1==1){}
	}
	
	// Banks 00->3f and 80->bf
	for (c = 0; c < 0x200; c += 8)
	{
		MAP[c+0] = MAP[c+0x400] = SNESC.RAM;		//RAM 000h-1FFFh  Mirror of 7E0000h-7E1FFFh (first 8Kbyte of WRAM)
		SNES.BlockIsRAM[c+0] = SNES.BlockIsRAM[c+0x400] = TRUE;

		MAP[c+1] = MAP[c+0x401] = (uchar *)MAP_PPU; //PPU 2100h-21FFh  I/O Ports (B-Bus) 
		MAP[c+2] = MAP[c+0x402] = (uchar *)MAP_CPU; //CPU 4000h-41FFh  I/O Ports (manual joypad access)
		MAP[c+3] = MAP[c+0x403] = (uchar *)MAP_NONE;
		
		for (i = c+4; i < c+8; i++)
		{
			int pageMap = ((c>>1)<<13)-0x8000;
			if ( pageMap < SNES.ROMSize)
			{
				if(pageMap < maxRAM){
					MAP[i] = MAP[i+0x400] = &SNESC.ROM[pageMap];
				}
				else{
					MAP[i] = MAP[i+0x400] = (uint8*)MAP_PAGING; //OK, full bottom 2MB is mapped as ROM (doesn't call this)
				}	
				SNES.BlockIsROM[i] = SNES.BlockIsROM[i+0x400] = TRUE;
			}
		}
	}
	
	// Banks 40->7f and c0->ff
	for (c = 0; c < 0x200; c += 8)
	{
		for (i = c; i < c+8; i++)
		{
			int pageMap = ((c>>1)<<14);
			if ( pageMap < SNES.ROMSize)
			{
				if(pageMap < maxRAM){
					MAP[i+0x200] = MAP[i+0x600]  = &SNESC.ROM[pageMap];
				}
				else{
					MAP[i+0x200] = MAP[i+0x600]  = (uint8*)MAP_PAGING;
				}	
				SNES.BlockIsROM[i+0x200] = SNES.BlockIsROM[i+0x600] = TRUE;
			}
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
void mem_init_directROM(){
	mem_clear_paging();
#ifdef ARM9
	SNES_ROM_ADDRESS_NTR = (u8*)(0x20B8000);
	SNES_ROM_ADDRESS_TWL = (u8*)(0x20B8F00);
	SDD1_WORKBUFFER = (u8*)(0x02FEE000);
	#endif
#ifdef _MSC_VER
	SNES_ROM_ADDRESS_NTR = (u8*)TGDSARM9Malloc(ROM_MAX_SIZE_NTRMODE_BIGLOROM_PAGEMODE);
	SNES_ROM_ADDRESS_TWL = (u8*)TGDSARM9Malloc(ROM_MAX_SIZE_TWLMODE);
#endif
	//Special LoROM 2M ROM + Paging. For SFA2 on NTR mode.
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

#ifdef _MSC_VER
	if(SNES_ROM_ADDRESS_NTR){
		TGDSARM9Free(SNES_ROM_ADDRESS_NTR);
	}
	if(SNES_ROM_ADDRESS_TWL){
		TGDSARM9Free(SNES_ROM_ADDRESS_TWL);
	}
#endif
	SNES_ROM_PAGING_ADDRESS = NULL;
	APU_BRR_HASH_BUFFER_NTR = NULL;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void	mem_init_paging()
{
	mem_clear_paging();
#ifdef ARM9
	SNES_ROM_ADDRESS_NTR = (u8*)(0x20F0000);
	SNES_ROM_ADDRESS_TWL = (u8*)(0x20F9F00);
	SDD1_WORKBUFFER = (u8*)(0x02FEE000);
#endif
#ifdef _MSC_VER
	SNES_ROM_ADDRESS_NTR = (u8*)TGDSARM9Malloc(ROM_MAX_SIZE_NTRMODE_BIGLOROM_PAGEMODE + INTERNAL_PAGING_SIZE_BIGLOROM_PAGEMODE + APU_BRR_HASH_BUFFER_SIZE);
	SNES_ROM_ADDRESS_TWL = (u8*)TGDSARM9Malloc(ROM_MAX_SIZE_TWLMODE + INTERNAL_PAGING_SIZE_BIGLOROM_PAGEMODE + APU_BRR_HASH_BUFFER_SIZE);
#endif
	SNES_ROM_PAGING_ADDRESS = (u8*)(SNES_ROM_ADDRESS_NTR+ROM_MAX_SIZE_NTRMODE_BIGLOROM_PAGEMODE);
	APU_BRR_HASH_BUFFER_NTR = NULL; //Need at least 512K of EWRAM, but S-DD1 requires at least 1MB of SNES_ROM_PAGING_ADDRESS to work properly. Disabled.
	
	memset(SNES_ROM_PAGING_ADDRESS, 0, INTERNAL_PAGING_SIZE_BIGLOROM_PAGEMODE);
	ROM_paging_offs = (uint16 *)TGDSARM9Malloc(SNES_ROM_PAGING_SLOTS*2);
	if (!ROM_paging_offs)
	{
		printf("Not enough memory for ROM paging (2).\n");
		while (1){}
	}
	memset(ROM_paging_offs, 0xFF, SNES_ROM_PAGING_SLOTS*2);
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
		//Unsupported. Use TGDS1.65 branch instead
	}
	else{
		block <<= PAGE_OFFSET_BIGLOROM;
		for (i = 0; i < PAGE_LOROM/8192; i++, block++)
		{
			//BigLoROM: mirroring goes here.
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
		//Unsupported. Use TGDS1.65 branch instead
	}
	else{
		block <<= PAGE_OFFSET_BIGLOROM;
		for (i = 0; i < PAGE_LOROM/8192; i++, block++)
		{
			//BigLoROM: mirroring goes here.
			if (SNES.BlockIsROM[block+0x200])
				MAP[block+0x200] = romPageLoROMSnesPage(ptr, (block+0x200)); 
			MAP[block+0x600] = romPageLoROMSnesPage(ptr, (block+0x600));  
		}
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
uint8 *	mem_checkReload(int blockInPage, uchar bank, uint32 offset){
	uint8* addr = NULL;
	if (CFG.LargeROM == 1) {
		addr = mem_checkReloadBigLoROM(blockInPage);
	}
	return addr;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
uint8 *mem_checkReloadBigLoROM(int block){
	int i;uchar *ptr;int ret;
	i = (block & 0x1FF) >> PAGE_OFFSET_BIGLOROM;

	//SnemulDS 0.6d
	
	#ifdef ARM9
	if (ROM_paging_offs[ROM_paging_cur] != 0xFFFF){
		// Check that we are not unloading program code 
		uint32 cPC = ((S&0xFFFF) << 16)|(uint32)((sint32)PCptr+(sint32)SnesPCOffset);
		uint32 PC_blk = ((cPC >> 13)&0x1FF) >> PAGE_OFFSET_BIGLOROM;
		if (ROM_paging_offs[ROM_paging_cur] == PC_blk){
			ROM_paging_cur++;
			if (ROM_paging_cur >= ((ROM_PAGING_SIZE/PAGE_HIROM)-3) ){ //heap protection if out of bounds is read
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
	if (ROM_paging_cur >= ((ROM_PAGING_SIZE/PAGE_HIROM)-3) ){ //heap protection if out of bounds is read
		ROM_paging_cur = 0;
	}
	return romPageToBigLoROMSnesPage(ptr, block);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void	InitMap(){
	int i;
	int mode = 0;
	for (i = 0; i < 256*8; i++){
		MAP[i] = (uint8*)MAP_NONE;	
	}
	mode = (CFG.LargeROM == false) ? NOT_LARGE : USE_PAGING; 
	if(CFG.SDD1 == 0){
		#ifdef ARM9
		clrscr();
		#endif
		printf("----");
		printf("----");
		printf("----");
		printf("This branch only supports S-DD1 games.");
		printf("Use SnemulDS TGDS1.65 (default) instead.");
		while(1==1){}
	}
	else{
		AlphaROMMap(mode);
	}
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
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

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void IO_setbyte(int addr, uint32 address, uint8 byte){
	switch (address){
		//Byte DMA Writes to S-DD1
		case 0x4804:
		case 0x4805:
		case 0x4806:
		case 0x4807:
		S9xSetSDD1MemoryMap (((int)address - 0x4804), byte & 7);
		break;
	}

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

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
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

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
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

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
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

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
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

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
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

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
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

#ifdef ARM9
__attribute__((section(".itcm")))
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
	
	default:
		return 0;
	}
}