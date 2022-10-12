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

//Note: Do not put any of the memory paging in ITCM, it'll break the whole thing.

#include <string.h>
#include <stdlib.h>
#include "core.h"
#include "opcodes.h"
#include "common.h"
#include "snes.h"
#include "cfg.h"
#include "memmap.h"
#include "utilsTGDS.h"
#include "c4.h"

uchar *ROM_paging= NULL;
uint16 *ROM_paging_offs= NULL;
int ROM_paging_cur = 0;

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
		if ( (MAP[c] != (uchar*)MAP_RELOAD) && REGULAR_MAP(MAP[c]))
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

	if (mode == NOT_LARGE)
	{
		// Small ROM, use only SNES ROM size of RAM
		maxRAM = SNES.ROMSize;
	}
	if (mode == USE_PAGING)
	{
		// Use Paging system... 
		// Only a part of RAM is used static
		maxRAM = PAGE_SIZE;
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
			if ( ((c>>1)<<13)-0x8000 < SNES.ROMSize)
			{
				MAP[i] = MAP[i+0x400] = &SNESC.ROM[(c>>1)<<13]-0x8000;
				if (((c>>1)<<13)-0x8000 >= maxRAM)
				{
					if (mode == USE_PAGING)
						MAP[i] = MAP[i+0x400] = (uint8*)MAP_RELOAD;
					else
						MAP[i] = MAP[i+0x400] = largeROM+((c>>1)<<13)-0x8000;
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
			if ( ((c>>1)<<13)+0x200000 < SNES.ROMSize)
			{
				MAP[i+0x200] = MAP[i+0x600] = &SNESC.ROM[((c>>1)<<13)+0x200000];
				if (((c>>1)<<13)+0x200000 >= maxRAM)
				{
					if (mode == USE_PAGING)
						MAP[i+0x200] = MAP[i+0x600] = (uint8*)MAP_RELOAD;
					else
						MAP[i+0x200] = MAP[i+0x600] = largeROM+((c>>1)<<13)+0x200000;
				}				
			}			
		}
		for (i = c+4; i < c+8; i++)
		{
			if ( ((c>>1)<<13)+0x200000-0x8000 < SNES.ROMSize)
			{
				MAP[i+0x200] = MAP[i+0x600] = &SNESC.ROM[((c>>1)<<13)+0x200000-0x8000];
				if (((c>>1)<<13)+0x200000-0x8000 >= maxRAM)
				{
					if (mode == USE_PAGING)
						MAP[i+0x200] = MAP[i+0x600] = (uint8*)MAP_RELOAD;
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
		// Small ROM, use only SNES ROM size of RAM
		maxRAM = SNES.ROMSize;
	}
	if (mode == USE_PAGING)
	{
		// Use Paging system... 
		// Only a part of RAM is used static
		maxRAM = PAGE_SIZE;
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
						MAP[i] = MAP[i+0x400] = (uint8*)MAP_RELOAD;
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
						MAP[i+0x200] = MAP[i+0x600] = (uint8*)MAP_RELOAD;
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void mem_setCacheBlock(int block, uchar *ptr)
{
	int i;

	block <<= PAGE_OFFSET;
	for (i = 0; i < PAGE_SIZE/8192; i++, block++)
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void mem_removeCacheBlock(int block)
{
	int i;

	block <<= PAGE_OFFSET;
	for (i = 0; i < PAGE_SIZE/8192; i++, block++)
	{

		if ((block & 7) >= 4)
		{
			MAP[block] = (uchar*)MAP_RELOAD;
			MAP[block+0x400] = (uchar*)MAP_RELOAD;
		}
		if (SNES.BlockIsROM[block+0x200])
			MAP[block+0x200] = (uchar*)MAP_RELOAD;
		MAP[block+0x600] = (uchar*)MAP_RELOAD;
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
uint8 *mem_checkReload(int block)
{
	int i;uchar *ptr;int ret;
	LOG("==> %d\n", block);

	if (!CFG.LargeROM){
		return NULL;
	}
	i = (block & 0x1FF) >> PAGE_OFFSET;
	
	//LOG("checkReload %d %d\r\n", i, ROM_paging_cur);
	if (ROM_paging_offs[ROM_paging_cur] != 0xFFFF){
		/* Check that we are not unloading program code */
		uint32 cPC = ((S&0xFFFF) << 16)|(uint32)((sint32)PCptr+(sint32)SnesPCOffset);
		uint32 PC_blk = ((cPC >> 13)&0x1FF) >> PAGE_OFFSET;
		if (ROM_paging_offs[ROM_paging_cur] == PC_blk){
			LOG("Detected PC unloading, skip it...\n");
			ROM_paging_cur++;
			if (ROM_paging_cur >= ((ROM_PAGING_SIZE/PAGE_SIZE) - 1) ){
				ROM_paging_cur = 0;
			}
		}
		if (ROM_paging_offs[ROM_paging_cur] != 0xFFFF){
			mem_removeCacheBlock(ROM_paging_offs[ROM_paging_cur]);
		}
	}

	ROM_paging_offs[ROM_paging_cur] = i;
	ptr = ROM_paging+(ROM_paging_cur*PAGE_SIZE);
	coherent_user_range_by_size((uint32)ptr, (int)PAGE_SIZE);	//Make coherent old page before destroy (could be being in cpu cache)
	ret = FS_loadROMPage(ptr, SNES.ROMHeader+i*PAGE_SIZE, PAGE_SIZE);
	coherent_user_range_by_size((uint32)ptr, (int)PAGE_SIZE);	//Make coherent new page
	mem_setCacheBlock(i, ptr); // Give Read-only memory

	ROM_paging_cur++;
	if (ROM_paging_cur >= ((ROM_PAGING_SIZE/PAGE_SIZE) - 1) ){
		ROM_paging_cur = 0;
	}
	LOG("<== %d %p\n", block, ptr+(block&7)*8192-(block << 13));
	return ptr+(block&7)*8192-(block << 13);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void InitMap(){
	int i;
	for (i = 0; i < 256*8; i++){
		MAP[i] = (uint8*)MAP_NONE;	
	}
	int mode = (!CFG.LargeROM) ? NOT_LARGE : USE_PAGING; 
	if (SNES.HiROM){
		InitHiROMMap(mode);
	}
	else{
		InitLoROMMap(mode);
	}
}


#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
uchar mem_getbyte(uint32 offset,uchar bank)
{
	int address = (bank<<16)+offset;
	int block;
	uchar *addr;

	block = (address>>13)&0x7FF;
	addr = MAP[block];

	if (addr == (uchar*)MAP_RELOAD)
	addr = mem_checkReload(block);
	
	if (REGULAR_MAP(addr)){ //if address is within indirect mapped memory define (snes.h), a forced (below) IO_xxxx opcode takes place
		return *(addr+address);
	}
	else{
		return IO_getbyte((int)addr, address);
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void mem_setbyte(uint32 offset, uchar bank, uchar byte)
{
	int address = (bank<<16)+offset;
	int block;
	uchar *addr;

	block = (address>>13)&0x7FF;
	addr = WMAP[block];
	if (addr == (uchar*)MAP_RELOAD)
	addr = mem_checkReload(block);
	
	if (REGULAR_MAP(addr) ){ //if address is within indirect mapped memory define (snes.h), a forced (below) IO_xxxx opcode takes place
		*(addr+address) = byte;
	}
	else{
		IO_setbyte((int)addr, address, byte);
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
ushort mem_getword(uint32 offset,uchar bank)
{
	int address = (bank<<16)+offset;
	int block;
	uchar *addr;

	block = (address>>13)&0x7FF;
	addr = MAP[block];

	if (addr == (uchar*)MAP_RELOAD)
	addr = mem_checkReload(block);
	
	if (REGULAR_MAP(addr)){ //if address is within indirect mapped memory define (snes.h), a forced (below) IO_xxxx opcode takes place
		return GET_WORD16(addr+address);
	}
	else{
		return IO_getword((int)addr, address);
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void mem_setword(uint32 offset, uchar bank, ushort word)
{
	int address = (bank<<16)+offset;
	int block;
	uchar *addr;

	//  CPU.WaitAddress = -1;
	block = (address>>13)&0x7FF;
	addr = WMAP[block];
	if (addr == (uchar*)MAP_RELOAD)
	addr = mem_checkReload(block);
	
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

	if (ptr == (uchar*)MAP_RELOAD)
		ptr = mem_checkReload(block);

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

	if (ptr == (uchar*)MAP_RELOAD)
		ptr = mem_checkReload(block);

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