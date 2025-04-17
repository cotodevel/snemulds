/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* Free for non-commercial use                             */
/***********************************************************/

#include <string.h>
#include "cpu.h"
#include "common.h"
#include "snes.h"
#include "cfg.h"

extern int OldPC;
extern char *ROM_Image;

uchar DMA_port_read(long address);
void DMA_port_write(long address, unsigned short value);
void PPU_port_write(long address, unsigned short value);
uchar PPU_port_read(long address);

void    WriteProtectROM ()
{
  int   c;

  memmove ((void *)SNES.WriteMap,(void *)SNESC.Map,sizeof(SNESC.Map));
  for (c = 0; c < 0x800; c++)
    {
      if (SNES.BlockIsROM[c])
        SNES.WriteMap[c] = (uchar *)MAP_NONE;
    }
}

void    MapRAM ()
{
  int   c;

  for (c = 0;c < 8;c++)
    {
      SNESC.Map[c+0x3f0] = SNESC.RAM;
      SNESC.Map[c+0x3f8] = SNESC.RAM+0x10000;
      SNES.BlockIsRAM[c+0x3f0] = TRUE;
      SNES.BlockIsRAM[c+0x3f8] = TRUE;
      SNES.BlockIsROM[c+0x3f0] = FALSE;
      SNES.BlockIsROM[c+0x3f8] = FALSE;
    }

  for (c = 0;c < 0x40;c++)
    {
      SNESC.Map[c+0x380] = (uchar *)MAP_LOROM_SRAM;
      SNES.BlockIsRAM[c+0x380] = TRUE;
      SNES.BlockIsROM[c+0x380] = FALSE;
    }
}

void    InitLoROMMap()
{
  int   c;
  int   i;

  for (c = 0; c < 0x200; c += 8)
    {
      SNESC.Map[c+0] = SNESC.Map[c+0x400] = SNESC.RAM;
      SNES.BlockIsRAM[c+0] = SNES.BlockIsRAM[c+0x400] = TRUE;

      SNESC.Map[c+1] = SNESC.Map[c+0x401] = (uchar *)MAP_PPU;
      SNESC.Map[c+2] = SNESC.Map[c+0x402] = (uchar *)MAP_CPU;
      if (CFG.DSP1)
        SNESC.Map[c+3] = SNESC.Map[c+0x403] = (uchar *)MAP_DSP;
      else
        SNESC.Map[c+3] = SNESC.Map[c+0x403] = (uchar *)MAP_NONE;
      for (i = c+4; i < c+8; i++)
	{
		if ( (((i << 13)+0x7ff)&0xFFFF)-0x8000+((c>>1)<<13) > SNES.ROMSize) // FIXED ?
			SNESC.Map[i] = SNESC.Map[i+0x400] = (uint8 *)MAP_NONE;
		else		
	  		SNESC.Map[i] = SNESC.Map[i+0x400] = &SNESC.ROM[(c>>1)<<13]-0x8000;
	  SNES.BlockIsROM[i] = SNES.BlockIsROM[i+0x400] = TRUE;
	}
    }

  if (CFG.DSP1)
    {
      for (c = 0x180; c < 0x200; c += 8)
	{
          SNESC.Map[c+4] = SNESC.Map[c+0x404] = (uchar *)MAP_DSP;
	  SNESC.Map[c+5] = SNESC.Map[c+0x405] = (uchar *)MAP_DSP;
	  SNESC.Map[c+6] = SNESC.Map[c+0x406] = (uchar *)MAP_DSP;
	  SNESC.Map[c+7] = SNESC.Map[c+0x407] = (uchar *)MAP_DSP;
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
		if ( (((i << 13)+0x7ff)&0xFFFF)+0x200000+((c>>1)<<13) > SNES.ROMSize) // FIXED ?
			SNESC.Map[i+0x200] = SNESC.Map[i+0x600] = (uint8 *)MAP_NONE;
		else
			SNESC.Map[i+0x200] = SNESC.Map[i+0x600] = &SNESC.ROM[((c>>1)<<13)+0x200000];
	   }
       for (i = c+4; i < c+8; i++)
	   {
		if ( (((i << 13)+0x7ff)&0xFFFF)+0x200000-0x8000+((c>>1)<<13) > SNES.ROMSize) // FIXED ?
			SNESC.Map[i+0x200] = SNESC.Map[i+0x600] = (uint8 *)MAP_NONE;
		else
			SNESC.Map[i+0x200] = SNESC.Map[i+0x600] = &SNESC.ROM[((c>>1)<<13)+0x200000-0x8000];
	   }
       for (i = c; i < c+8; i++)
         SNES.BlockIsROM[i+0x200] = SNES.BlockIsROM[i+0x600] = TRUE;
    }

  if (CFG.DSP1)
    {
      for (c = 0; c < 0x80; c++)
	{
	  SNESC.Map[c+0x700] = (uchar *)MAP_DSP;
	  SNES.BlockIsROM[c+0x700] = FALSE;
	}
    }
      
  MapRAM();
  
  WriteProtectROM();
}

void    InitHiROMMap()
{
  int   c;
  int   i;


  for (c = 0; c < 0x200; c += 8)
    {
      SNESC.Map[c+0] = SNESC.Map[c+0x400] = SNESC.RAM;
      SNES.BlockIsRAM[c+0] = SNES.BlockIsRAM[c+0x400] = TRUE;

      SNESC.Map[c+1] = SNESC.Map[c+0x401] = (uchar *)MAP_PPU;
      SNESC.Map[c+2] = SNESC.Map[c+0x402] = (uchar *)MAP_CPU;
      if (CFG.DSP1)
        SNESC.Map[c+3] = SNESC.Map[c+0x403] = (uchar *)MAP_DSP;
      else
        SNESC.Map[c+3] = SNESC.Map[c+0x403] = (uchar *)MAP_NONE;
	    
      for (i = c+4;i < c+8;i++)
	{
		if ( (((i << 13)+0x7ff)&0xFFFF)+(c<<13) > SNES.ROMSize) // FIXED ?
			SNESC.Map[i] = SNESC.Map[i+0x400] = (uint8 *)MAP_NONE;
		else	
	  		SNESC.Map[i] = SNESC.Map[i+0x400] = &SNESC.ROM[c<<13];
	  SNES.BlockIsROM[i] = SNES.BlockIsROM[i+0x400] = TRUE;
	}
    }

  for (c = 0; c < 16; c++)
    {
      SNESC.Map[0x183+(c<<3)] = (uchar *)MAP_HIROM_SRAM;
      SNESC.Map[0x583+(c<<3)] = (uchar *)MAP_HIROM_SRAM;
      SNES.BlockIsRAM[0x183+(c<<3)] = TRUE;
      SNES.BlockIsRAM[0x583+(c<<3)] = TRUE;
    }

  for (c = 0; c < 0x200; c += 8)
    {
      for (i = c; i < c+8; i++)
	{
		if ( (((i << 13)+0x7ff)&0xFFFF)+(c<<13) > SNES.ROMSize) // FIXED ?
			SNESC.Map[i+0x200] = SNESC.Map[i+0x600] = (uint8 *)MAP_NONE;
		else		
          SNESC.Map[i+0x200] = SNESC.Map[i+0x600] = &SNESC.ROM[c<<13];
	  SNES.BlockIsROM[i+0x200] = SNES.BlockIsROM[i+0x600] = TRUE;
	}
    }
  MapRAM();
  WriteProtectROM();
}

void    InitLargeLoROMMap()
{
	iprintf("Large LoROM not supported.\n");
	while (1);
}

void    InitLargeHiROMMap()
{
  int   c;
  int   i;


  for (c = 0; c < 0x200; c += 8)
    {
      SNESC.Map[c+0] = SNESC.Map[c+0x400] = SNESC.RAM;
      SNES.BlockIsRAM[c+0] = SNES.BlockIsRAM[c+0x400] = TRUE;

      SNESC.Map[c+1] = SNESC.Map[c+0x401] = (uchar *)MAP_PPU;
      SNESC.Map[c+2] = SNESC.Map[c+0x402] = (uchar *)MAP_CPU;
      if (CFG.DSP1)
        SNESC.Map[c+3] = SNESC.Map[c+0x403] = (uchar *)MAP_DSP;
      else
        SNESC.Map[c+3] = SNESC.Map[c+0x403] = (uchar *)MAP_NONE;
	    
      for (i = c+4;i < c+8;i++)
	{
/*		if ( (c<<13)+0xFFFF >= SNES.ROMSize) // FIXED ?
			SNESC.Map[i] = SNESC.Map[i+0x400] = (uint8 *)MAP_NONE;
		else*/
		if ( (c<<13)+8191 < 1024*1024)
		 	SNESC.Map[i] = SNESC.Map[i+0x400] = &SNESC.ROM[c<<13];
		 else
			SNESC.Map[i] = SNESC.Map[i+0x400] = NULL;
			

	  SNES.BlockIsROM[i] = SNES.BlockIsROM[i+0x400] = TRUE;
	}
    }

  for (c = 0; c < 16; c++)
    {
      SNESC.Map[0x183+(c<<3)] = (uchar *)MAP_HIROM_SRAM;
      SNESC.Map[0x583+(c<<3)] = (uchar *)MAP_HIROM_SRAM;
      SNES.BlockIsRAM[0x183+(c<<3)] = TRUE;
      SNES.BlockIsRAM[0x583+(c<<3)] = TRUE;
    }

  for (c = 0; c < 0x200; c += 8)
    {
      for (i = c; i < c+8; i++)
	{
/*		if ( (c<<13)+0xFFFF >= SNES.ROMSize) // FIXED ?
			SNESC.Map[i+0x200] = SNESC.Map[i+0x600] = (uint8 *)MAP_NONE;
		else*/
		if ( (c<<13)+8191 < 1024*1024)
		 	SNESC.Map[i+0x200] = SNESC.Map[i+0x600] = &SNESC.ROM[c<<13];
		 else
			SNESC.Map[i+0x200] = SNESC.Map[i+0x600] = NULL;
	  SNES.BlockIsROM[i+0x200] = SNES.BlockIsROM[i+0x600] = TRUE;
	}
    }
  MapRAM();
  WriteProtectROM();
  
  
}

#define ROM_PAGING_SIZE	(1*1024*1024)

uchar	*ROM_paging = NULL;
uint16	*ROM_paging_offs = NULL;
int		ROM_paging_cur = 0;

void	mem_clear_paging()
{
	if (ROM_paging)
	{
		free(ROM_paging);
		free(ROM_paging_offs);
		ROM_paging = NULL;
		ROM_paging_offs = NULL;
	}
}

void	mem_init_paging()
{
	if (ROM_paging)
		mem_clear_paging();
		
	ROM_paging = malloc(ROM_PAGING_SIZE);
	if (!ROM_paging)
	{
		iprintf("Not enough memory for ROM paging.\n");
		while(1);
	}	
	ROM_paging_offs = malloc(ROM_PAGING_SIZE/8192);
	if (!ROM_paging_offs)
	{
		iprintf("Not enough memory for ROM paging (2).\n");
		while(1);
	}	
	memset(ROM_paging_offs, 0xFF, ROM_PAGING_SIZE/8192);
	ROM_paging_cur = 0;
}

void	mem_removeCacheBlock(int block)
{
	if ((block & 7) >= 4)
	{
		SNESC.Map[block] = NULL;
		SNESC.Map[block+0x400] = NULL;		
	}
	if (SNES.BlockIsROM[block+0x200])	
		SNESC.Map[block+0x200] = NULL;
	SNESC.Map[block+0x600] = NULL;	
}

char *	mem_checkReload(int block)
{
	int i;
	uchar *ptr;
	int	ret;
	
	if (!CFG.LargeROM)
		return NULL;
			
	i = block & 0x1FF;
//	LOG("checkReload %x %x\r\n", block, i);
	
	if (ROM_paging_offs[ROM_paging_cur] != 0xFFFF)
	{
//		LOG("remove %d\r\n", ROM_paging_offs[ROM_paging_cur]);
		mem_removeCacheBlock(ROM_paging_offs[ROM_paging_cur]);
	}
	
	
	ROM_paging_offs[ROM_paging_cur] = i;
	ptr = ROM_paging+(ROM_paging_cur*8192);
	
//	LOG("@%d(%d) => blk %d\n", i*8192, SNES.ROMHeader+i*8192, ROM_paging_cur);
	ret = FS_loadROMPage(ptr, SNES.ROMHeader+i*8192, 8192);
//	LOG("ret = %d %x %x %x %x\n", ret, ptr[0], ptr[1], ptr[2], ptr[3]);
	
	if ((i & 7) >= 4)
	{
		SNESC.Map[i] = ptr-((block << 13)&0xFFFF);
		SNESC.Map[i+0x400] = ptr-((block << 13)&0xFFFF);		
	}
	if (SNES.BlockIsROM[i+0x200])
		SNESC.Map[i+0x200] = ptr-((block << 13)&0xFFFF);
	SNESC.Map[i+0x600] = ptr-((block << 13)&0xFFFF);
	
	ROM_paging_cur++;
	if (ROM_paging_cur >= ROM_PAGING_SIZE/8192)
		ROM_paging_cur = 0;
				
	return ptr-((block << 13)&0xFFFF);
}


void	InitMap()
{
  if (CFG.LargeROM)
  {
	  if (SNES.HiROM)
    	InitLargeHiROMMap();
  	else
    	InitLargeLoROMMap();
    	
    mem_init_paging();
    return;
  }
	
  if (SNES.HiROM)
    InitHiROMMap();
  else
    InitLoROMMap();
}



/*inline */uint8	IO_getbyte(int addr, uint32 address)
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
		return *(SNESC.SRAM+((address&SNESC.SRAMMask)));
      case MAP_HIROM_SRAM:
		return *(SNESC.SRAM+(((address&0x7fff)-0x6000+
			     ((address&0xf0000)>>3))&SNESC.SRAMMask));
      default:
        return 0;
    }
	
}

/*inline */void	IO_setbyte(int addr, uint32 address, uint8 byte)
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
		*(SNESC.SRAM+((address&SNESC.SRAMMask))) = byte; return;
      case MAP_HIROM_SRAM:
		*(SNESC.SRAM+(((address&0x7fff)-0x6000+
	 	      ((address&0xf0000)>>3))&SNESC.SRAMMask)) = byte; return;
      case MAP_NONE:
        return;
    }
}

/*inline */uint16	IO_getword(int addr, uint32 address)
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
		return GET_WORD16(SNESC.SRAM+((address&SNESC.SRAMMask)));
      case MAP_HIROM_SRAM:
		return GET_WORD16(SNESC.SRAM+(((address&0x7fff)-0x6000+
			     ((address&0xf0000)>>3))&SNESC.SRAMMask));
      default:
        return 0;
    }
}

/*inline */void	IO_setword(int addr, uint32 address, uint16 word)
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
		SET_WORD16(SNESC.SRAM+((address&SNESC.SRAMMask)), word);
        return;
      case MAP_HIROM_SRAM:
        SET_WORD16(SNESC.SRAM+(((address&0x7fff)-0x6000+
	 	            ((address&0xf0000)>>3))&SNESC.SRAMMask), word);
        return;
      case MAP_NONE:
        return;
    }    
}

//#include "memmap.h"
IN_ITCM
uchar   mem_getbyte(uint32 offset,uchar bank)
{
  int   address = (bank<<16)+offset;
  int   block;
  uchar *addr;

  block = (address>>13)&0x7FF;
  addr = SNESC.Map[block];
  if (addr == NULL)
  	addr = mem_checkReload(block);
  if ((int)addr >= MAP_LAST)
    {
      return *(addr+(offset&0xFFFF));
    }
  else
  	return IO_getbyte((int)addr, address);
}

IN_ITCM2
void	mem_setbyte(uint32 offset, uchar bank, uchar byte)
{
  int   address = (bank<<16)+offset;
  int   block;
  uchar *addr;

  block = (address>>13)&0x7FF;
  addr = SNES.WriteMap[block];
  if (addr == NULL)
  	addr = mem_checkReload(block);  
  if ((int)addr >= MAP_LAST)
    {
      *(addr+(offset&0xFFFF)) = byte;
    }
  else
  	IO_setbyte((int)addr, address, byte);
}

IN_ITCM
ushort  mem_getword(uint32 offset,uchar bank)
{
  int   address = (bank<<16)+offset;
  int   block;
  uchar  *addr;

  block = (address>>13)&0x7FF;
  addr = SNESC.Map[block];
  if (addr == NULL)
  	addr = mem_checkReload(block);  
  if ((int)addr >= MAP_LAST)
    {
      return GET_WORD16(addr+(offset&0xFFFF)); 	
    }
  else
  	return IO_getword((int)addr, address);
}

IN_ITCM2
void    mem_setword(uint32 offset, uchar bank, ushort word)
{
  int   address = (bank<<16)+offset;
  int   block;
  uchar  *addr;

//  CPU.WaitAddress = -1;
  block = (address>>13)&0x7FF;
  addr = SNES.WriteMap[block];
  if (addr == NULL)
  	addr = mem_checkReload(block);  
  if ((int)addr >= MAP_LAST)
    {
      SET_WORD16(addr+(offset&0xFFFF), word); 	
    }
  else
    IO_setword((int)addr, address, word);
}

IN_ITCM2
void	*map_memory(uint16 offset, uchar bank)
{
  int   block;
  int	address = (bank<<16)+offset;
  uchar	*ptr;

  block = (address>>13)&0x7FF;
  ptr = SNESC.Map[block];

  if (ptr == NULL)
  	ptr = mem_checkReload(block);

  if ((int)ptr >= MAP_LAST)
    return ptr+offset;

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