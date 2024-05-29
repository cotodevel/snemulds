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

#include <string.h>
#include "cpu.h"
#include "opcodes.h"
#include "common.h"
#include "snes.h"
#include "cfg.h"

extern int OldPC;
extern char *ROM_Image;

#define NOT_LARGE	0
#define USE_PAGING	1
#define USE_EXTMEM	2


uchar DMA_port_read(long address);
void DMA_port_write(long address, unsigned short value);
void PPU_port_write(long address, unsigned short value);
uchar PPU_port_read(long address);

#ifdef ASM_OPCODES
#define SPECIAL_MAP(p) ((int)(p) & 0x80000000)
#define REGULAR_MAP(p) (!((int)(p) & 0x80000000))  	
#else
#define SPECIAL_MAP(p) ((int)(p) < MAP_LAST)
#define REGULAR_MAP(p) ((int)(p) >= MAP_LAST)  	
#endif

void WriteProtectROM()
{
	int c;

	for (c = 0; c < 256*8; c++)
		WMAP[c] = MAP[c];

	for (c = 0; c < 0x800; c++)
	{
		if (SNES.BlockIsROM[c])
			WMAP[c] = (uchar *)MAP_NONE;
	}
}

void FixMap()
{
	int c;

	for (c = 0; c < 0x800; c++)
	{
		if (MAP[c] != MAP_RELOAD && REGULAR_MAP(MAP[c]))
		{
			MAP[c] -= ((c << 13)&0xFF0000);
		}
	}
}

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
		maxRAM = ROM_STATIC_SIZE;
	}
	if (mode == USE_EXTMEM)
	{
		// Extended RAM mode...
		// All RAM available is used static
		// the remaining of mapping use extended RAM
		maxRAM = ROM_MAX_SIZE;
		largeROM = (uint8 *)0x8000000 + SNES.ROMHeader;
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
		maxRAM = ROM_STATIC_SIZE;
	}
	if (mode == USE_EXTMEM)
	{
		// Extended RAM mode...
		// All RAM available is used static
		// the remaining of mapping use extended RAM
		maxRAM = ROM_MAX_SIZE;
		largeROM = (uint8 *)0x8000000 + SNES.ROMHeader;
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

/*#define	PAGE_SIZE		8192
 #define PAGE_OFFSET		0*/
#define	PAGE_SIZE		65536
#define PAGE_OFFSET		3

uchar *ROM_paging= NULL;
uint16 *ROM_paging_offs= NULL;
int ROM_paging_cur = 0;

void mem_clear_paging()
{
	if (ROM_paging)
	{
//		iprintf("Memory paging cleared...\n");
		//		free(ROM_paging);
		free(ROM_paging_offs);
		ROM_paging = NULL;
		ROM_paging_offs = NULL;
	}
}

void mem_init_paging()
{
	/*	ROM_paging = malloc(ROM_PAGING_SIZE);
	 if (!ROM_paging)
	 {
	 iprintf("Not enough memory for ROM paging.\n");
	 while(1);
	 }*/
	ROM_paging = SNES_ROM_PAGING_ADDRESS;
	memset(ROM_paging, 0, ROM_PAGING_SIZE);
	ROM_paging_offs = malloc((ROM_PAGING_SIZE/PAGE_SIZE)*2);
	if (!ROM_paging_offs)
	{
		iprintf("Not enough memory for ROM paging (2).\n");
		while (1)
			;
	}
	memset(ROM_paging_offs, 0xFF, (ROM_PAGING_SIZE/PAGE_SIZE)*2);
	ROM_paging_cur = 0;
}

IN_ITCM3
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

IN_ITCM3
void mem_removeCacheBlock(int block)
{
	int i;

	block <<= PAGE_OFFSET;
	for (i = 0; i < PAGE_SIZE/8192; i++, block++)
	{

		if ((block & 7) >= 4)
		{
			MAP[block] = MAP_RELOAD;
			MAP[block+0x400] = MAP_RELOAD;
		}
		if (SNES.BlockIsROM[block+0x200])
			MAP[block+0x200] = MAP_RELOAD;
		MAP[block+0x600] = MAP_RELOAD;
	}
}

IN_ITCM3
uint8 *mem_checkReload(int block)
{
	int i;
	uchar *ptr;
	int ret;

	//	FS_flog("==> %d\n", block);
	LOG("==> %d\n", block);

	if (!CFG.LargeROM)
	return NULL;

#if 0	
	if (CFG.ExtRAMSize > 0) // FIXME ExtRAMUsed

	{
		//ptr = ram_unlock();
		ptr = (uint8 *)0x8000000 + SNES.ROMHeader + (block & 0x1FF)*8192 -(block << 13);
		return ptr;
	}
#endif	

	i = (block & 0x1FF) >> PAGE_OFFSET;

	//LOG("checkReload %d %d\r\n", i, ROM_paging_cur);

	if (ROM_paging_offs[ROM_paging_cur] != 0xFFFF)
	{
		/* Check that we are not unloading program code */
		uint32 cPC = ((S&0xFFFF) << 16)|(uint32)((sint32)PCptr+(sint32)SnesPCOffset);
		uint32 PC_blk = ((cPC >> 13)&0x1FF) >> PAGE_OFFSET;
		if (ROM_paging_offs[ROM_paging_cur] == PC_blk)
		{
			LOG("Detected PC unloading, pass it...\n");
			ROM_paging_cur++;
			if (ROM_paging_cur >= ROM_PAGING_SIZE/PAGE_SIZE)
			ROM_paging_cur = 0;
		}
		if (ROM_paging_offs[ROM_paging_cur] != 0xFFFF)
		{
			//  		LOG("remove %d\r\n", ROM_paging_offs[ROM_paging_cur]);
			mem_removeCacheBlock(ROM_paging_offs[ROM_paging_cur]);
		}
	}

	ROM_paging_offs[ROM_paging_cur] = i;
	ptr = ROM_paging+(ROM_paging_cur*PAGE_SIZE);

	//	LOG("@%d(%d) => blk %d\n", i*PAGE_SIZE, SNES.ROMHeader+i*PAGE_SIZE, ROM_paging_cur);
	ret = FS_loadROMPage(ptr, SNES.ROMHeader+i*PAGE_SIZE, PAGE_SIZE);
	//	LOG("ret = %d %x %x %x %x\n", ret, ptr[0], ptr[1], ptr[2], ptr[3]);

	mem_setCacheBlock(i, ptr+0x400000); // Give Read-only memory

	ROM_paging_cur++;
	if (ROM_paging_cur >= ROM_PAGING_SIZE/PAGE_SIZE)
		ROM_paging_cur = 0;

	//FS_flog("%d %p\n", i, ptr+(block&7)*8192-(block << 13));
	LOG("<== %d %p\n", block, ptr+(block&7)*8192-(block << 13));
	return ptr+(block&7)*8192-(block << 13)+0x400000;
}

void InitMap()
{
	int i;
	
	for (i = 0; i < 256*8; i++)
		MAP[i] = (uint8*)MAP_NONE;	

	int mode = (!CFG.LargeROM) ? NOT_LARGE :
				((CFG.MapExtMem && CFG.ExtRAMSize > 0) ? USE_EXTMEM : USE_PAGING); 
	
	if (SNES.HiROM)
		InitHiROMMap(mode);
	else
		InitLoROMMap(mode);
	
	if (mode == USE_PAGING)
		mem_init_paging();
}

IN_ITCM3
/*inline */uint8 IO_getbyte(int addr, uint32 address)
{
	uint8 result;

	switch ((int)addr)
	{
	case MAP_PPU:
		START_PROFILE(IOREGS, 2);
		result= PPU_port_read(address&0xFFFF);
		END_PROFILE(IOREGS, 2);
		return result;

	case MAP_CPU:
		START_PROFILE(IOREGS, 2);
		result= DMA_port_read(address&0xFFFF);
		END_PROFILE(IOREGS, 2);
		return result;
	case MAP_LOROM_SRAM:
		if (SNESC.SRAMMask == 0)
			return 0;
		return *(SNESC.SRAM+((address&SNESC.SRAMMask)));
	case MAP_HIROM_SRAM:
		if (SNESC.SRAMMask == 0)
			return 0;
		return *(SNESC.SRAM+(((address&0x7fff)-0x6000+
								((address&0xf0000)>>3))&SNESC.SRAMMask));
	default:
		return 0;
	}

}

IN_ITCM3
/*inline */void IO_setbyte(int addr, uint32 address, uint8 byte)
{
	switch ((int)addr)
	{
	case MAP_PPU:
		START_PROFILE(IOREGS, 2);
		PPU_port_write(address&0xFFFF,byte);
		END_PROFILE(IOREGS, 2);
		return;
	case MAP_CPU:
		START_PROFILE(IOREGS, 2);
		DMA_port_write(address&0xFFFF,byte);
		END_PROFILE(IOREGS, 2);
		return;
	case MAP_LOROM_SRAM:
		if (SNESC.SRAMMask == 0)
			return;
		*(SNESC.SRAM+((address&SNESC.SRAMMask))) = byte;
		SNES.SRAMWritten = 1;
		return;
	case MAP_HIROM_SRAM:
		if (SNESC.SRAMMask == 0)
			return;
		*(SNESC.SRAM+(((address&0x7fff)-0x6000+
				((address&0xf0000)>>3))&SNESC.SRAMMask)) = byte;
		SNES.SRAMWritten = 1;
		return;
	case MAP_NONE:
		return;
	}
}

IN_ITCM3
/*inline */uint16 IO_getword(int addr, uint32 address)
{
	uint16 result;

	switch ((int)addr)
	{
	case MAP_PPU:
		START_PROFILE(IOREGS, 2);
		result= PPU_port_read(address&0xFFFF)+
			(PPU_port_read((address+1)&0xFFFF)<<8);
		END_PROFILE(IOREGS, 2);
		return result;

	case MAP_CPU:
		START_PROFILE(IOREGS, 2);
		result= DMA_port_read(address&0xFFFF)+
			(DMA_port_read((address+1)&0xFFFF)<<8);
		END_PROFILE(IOREGS, 2);
		return result;
	case MAP_LOROM_SRAM:
		if (SNESC.SRAMMask == 0)
			return 0;
		result = SNESC.SRAM[address&SNESC.SRAMMask];
		result |= SNESC.SRAM[(address+1)&SNESC.SRAMMask]<<8;
		return result;
	case MAP_HIROM_SRAM:
		if (SNESC.SRAMMask == 0)
			return 0;
		address = ((address&0x7fff)-0x6000+((address&0xf0000)>>3));
		result = SNESC.SRAM[address&SNESC.SRAMMask];
		result |= SNESC.SRAM[(address+1)&SNESC.SRAMMask]<<8;
		return result;
	default:
		return 0;
	}
}

IN_ITCM3
/*inline */void IO_setword(int addr, uint32 address, uint16 word)
{
	switch ((int)addr)
	{
	case MAP_PPU:
		START_PROFILE(IOREGS, 2);
		PPU_port_write(address&0xFFFF,word&0xFF);
		PPU_port_write((address+1)&0xFFFF,word>>8);
		END_PROFILE(IOREGS, 2);
		return;
	case MAP_CPU:
		START_PROFILE(IOREGS, 2);
		DMA_port_write(address&0xFFFF,word&0xFF);
		DMA_port_write((address+1)&0xFFFF,word>>8);
		END_PROFILE(IOREGS, 2);
		return;
	case MAP_LOROM_SRAM:
		if (SNESC.SRAMMask == 0)
			return;
		SNESC.SRAM[address&SNESC.SRAMMask] = word&0xFF;
		SNESC.SRAM[(address+1)&SNESC.SRAMMask] = word>>8;
		SNES.SRAMWritten = 1;
		return;
	case MAP_HIROM_SRAM:
		if (SNESC.SRAMMask == 0)
			return;
		address = ((address&0x7fff)-0x6000+((address&0xf0000)>>3));
		SNESC.SRAM[address&SNESC.SRAMMask] = word&0xFF;
		SNESC.SRAM[(address+1)&SNESC.SRAMMask] = word>>8;
		SNES.SRAMWritten = 1;
		return;
	case MAP_NONE:
		return;
	}
}

//#include "memmap.h"
IN_ITCM
uchar mem_getbyte(uint32 offset,uchar bank)
{
	int address = (bank<<16)+offset;
	int block;
	uchar *addr;

	block = (address>>13)&0x7FF;
	addr = MAP[block];

	if (addr == MAP_RELOAD)
	addr = mem_checkReload(block);

	if (REGULAR_MAP(addr))
	{
		return *(addr+address);
	}
	else
	return IO_getbyte((int)addr, address);
}

IN_ITCM2
void mem_setbyte(uint32 offset, uchar bank, uchar byte)
{
	int address = (bank<<16)+offset;
	int block;
	uchar *addr;

	block = (address>>13)&0x7FF;
	addr = WMAP[block];
	if (addr == MAP_RELOAD)
	addr = mem_checkReload(block);
	if (REGULAR_MAP(addr))
	{
		*(addr+address) = byte;
	}
	else
	IO_setbyte((int)addr, address, byte);
}

IN_ITCM
ushort mem_getword(uint32 offset,uchar bank)
{
	int address = (bank<<16)+offset;
	int block;
	uchar *addr;

	block = (address>>13)&0x7FF;
	addr = MAP[block];

	if (addr == MAP_RELOAD)
	addr = mem_checkReload(block);
	if (REGULAR_MAP(addr))
	{
		return GET_WORD16(addr+address);
	}
	else
	return IO_getword((int)addr, address);
}

IN_ITCM2
void mem_setword(uint32 offset, uchar bank, ushort word)
{
	int address = (bank<<16)+offset;
	int block;
	uchar *addr;

	//  CPU.WaitAddress = -1;
	block = (address>>13)&0x7FF;
	addr = WMAP[block];
	if (addr == MAP_RELOAD)
	addr = mem_checkReload(block);

	if (REGULAR_MAP(addr))
	{
		SET_WORD16(addr+address, word);
	}
	else
	IO_setword((int)addr, address, word);
}

void *mem_getbaseaddress(uint16 offset, uchar bank)
{
	int block;
	int address = (bank<<16)+offset;
	uchar *ptr;

	block = (address>>13)&0x7FF;
	ptr = MAP[block];

	if (ptr == MAP_RELOAD)
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

//IN_ITCM2
void *map_memory(uint16 offset, uchar bank)
{
	int block;
	int address = (bank<<16)+offset;
	uchar *ptr;

	block = (address>>13)&0x7FF;
	ptr = MAP[block];

	if (ptr == MAP_RELOAD)
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
	default:
		return 0;
	}
}
