/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

*/

#ifndef __cartheader_h
#define __cartheader_h


typedef struct sDSCARTHEADER {
	uint8 	gametitle[12];	//000h    12    Game Title  (Uppercase ASCII, padded with 00h)
	uint32	gamecode;	//00Ch    4     Gamecode    (Uppercase ASCII, NTR-<code>)        (0=homebrew)
	uint16	makercode;	//010h    2     Makercode   (Uppercase ASCII, eg. "01"=Nintendo) (0=homebrew)
	uint8	unitcode;	//012h    1     Unitcode    (00h=NDS, 02h=NDS+DSi, 03h=DSi) (bit1=DSi)
  uint8 encriptionseedsel;//013h    1     Encryption Seed Select (00..07h, usually 00h)
  uint8 devicecapacity;	//014h    1     Devicecapacity         (Chipsize = 128KB SHL nn) (eg. 7 = 16MB)
  uint8	reserved1[7];	//015h    7     Reserved    (zero filled)
  uint8	reserved2;	//01Ch    1     Reserved    (zero)                      (except, used on DSi)
  uint8	ndsregion;	//01Dh    1     NDS Region  (00h=Normal, 80h=China, 40h=Korea) (other on DSi)
  uint8	romversion;	//01Eh    1     ROM Version (usually 00h)
  uint8	autostart;	//01Fh    1     Autostart (Bit2: Skip "Press Button" after Health and Safety) (Also skips bootmenu, even in Manual mode & even Start pressed)
  uint32	arm9romoffset;	//020h    4     ARM9 rom_offset    (4000h and up, align 1000h)
  uint32	arm9entryaddress;	//024h    4     ARM9 entry_address (2000000h..23BFE00h)
  uint32	arm9ramaddress;	//028h    4     ARM9 ram_address   (2000000h..23BFE00h)
  uint32	arm9size;	//02Ch    4     ARM9 size          (max 3BFE00h) (3839.5KB)
  uint32	arm7romoffset;	//030h    4     ARM7 rom_offset    (8000h and up)
  uint32	arm7entryaddress;	//034h    4     ARM7 entry_address (2000000h..23BFE00h, or 37F8000h..3807E00h)
  uint32 	arm7ramaddress;	//038h    4     ARM7 ram_address   (2000000h..23BFE00h, or 37F8000h..3807E00h)
  uint32	arm7size;	//03Ch    4     ARM7 size          (max 3BFE00h, or FE00h) (3839.5KB, 63.5KB)
  //...rest don't care
}tDSCARTHEADER;


#endif