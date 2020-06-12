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

//Source http://problemkaputt.de/gbatek.htm#dsfirmwareserialflashmemory

#include "spifwTGDS.h"
#include "spiTGDS.h"
#include <stdio.h>
#include <string.h>

#ifdef ARM7

void readFirmwareSPI(uint32 address, uint8 * destination, uint32 size){

	//Set Chip Select LOW to invoke the command & Transmit the instruction byte
	REG_SPI_CR = BIT_SPICNT_ENABLE | BIT_SPICNT_BYTETRANSFER | BIT_SPICNT_CSHOLDENABLE | BIT_SPICNT_FWCNT | BIT_SPICLK_4MHZ;
	RWSPICNT(FWCMD_READ);
	
	//Transmit any parameter bytes
	RWSPICNT((address>>16) & 0xFF);
	RWSPICNT((address>> 8) & 0xFF);
	RWSPICNT((address) & 0xFF);
	
	//Transmit/receive any data bytes
	//read fw back from dataport
	sint32 offset = 0;
	
	// Read the data
	for(offset=0;offset<size;offset++) {
		destination[offset] = RWSPICNT(0);
	}
	
	//Set Chip Select HIGH to finish the command
	SPICSHIGH();
	REG_SPI_CR = 0;
}

//todo: untested. When I have a swapable DS Lite NVRAM chip I will fix this
/*
//WEL is set by WREN instruction (which must be issued before any write/program/erase instructions)
void writeFirmwareSPIPage(uint32 address, uint8 * buffer){
	//Set Chip Select LOW to invoke the command & Transmit the instruction byte (write enable)
	REG_SPI_CR = BIT_SPICNT_ENABLE|BIT_SPICNT_CSHOLDENABLE|BIT_SPICNT_NVRAMCNT| BIT_SPICLK_4MHZ;
	RWSPICNT(FWCMD_WREN);
	//Set Chip Select HIGH to finish the command
	SPICSHIGH();

	//Set Chip Select LOW to invoke the command & Transmit the instruction byte (wait for write latch enable)
	REG_SPI_CR = BIT_SPICNT_ENABLE|BIT_SPICNT_CSHOLDENABLE|BIT_SPICNT_NVRAMCNT| BIT_SPICLK_4MHZ;
	RWSPICNT(FWCMD_RDSR);
	while(!(RWSPICNT(0)&BIT_FWCMD_WEL));
	//Set Chip Select HIGH to finish the command
	SPICSHIGH();
	
	//Set Chip Select LOW to invoke the command & Transmit the instruction byte
	REG_SPI_CR = BIT_SPICNT_ENABLE|BIT_SPICNT_CSHOLDENABLE|BIT_SPICNT_NVRAMCNT| BIT_SPICLK_4MHZ;
	RWSPICNT(FWCMD_PW);
	
	// Set the address
	RWSPICNT((address>>16) & 0xFF);
	RWSPICNT((address>> 8) & 0xFF);
	RWSPICNT((address) & 0xFF);
	
	//Transmit/receive any data bytes
	//write fw back to dataport
	sint32 offset = 0;
	while(offset<(sint32)FWCMD_PAGESIZE){
		RWSPICNT(buffer[offset]);
		offset++;
	}
	
	//Set Chip Select HIGH to finish the command
	SPICSHIGH();

	//Set Chip Select LOW to invoke the command & Transmit the instruction byte: wait until work in progress write bit is released ( 0 )
	REG_SPI_CR = BIT_SPICNT_ENABLE|BIT_SPICNT_CSHOLDENABLE|BIT_SPICNT_NVRAMCNT| BIT_SPICLK_4MHZ;
	RWSPICNT(FWCMD_RDSR);
	while(RWSPICNT(0)&BIT_FWCMD_WIP);
	//Set Chip Select HIGH to finish the command
	SPICSHIGH();
	
	//Set Chip Select LOW to invoke the command & Transmit the instruction byte: Ensure write is disabled when leaving write firmware.
	REG_SPI_CR = BIT_SPICNT_ENABLE|BIT_SPICNT_CSHOLDENABLE|BIT_SPICNT_NVRAMCNT| BIT_SPICLK_4MHZ;
	RWSPICNT(FWCMD_WRDI);
	while(RWSPICNT(0)&BIT_FWCMD_WEL);
	//Set Chip Select HIGH to finish the command
	SPICSHIGH();
}

//Order is MSB (Big Endian) for SPI firmware Reads
int writeFirmwareSPI(uint32 address, void * source, uint32 size){
	//Page aligned to FWCMD_PAGESIZE
	address &= ~(FWCMD_PAGESIZE);
	size &= ~(FWCMD_PAGESIZE);
	
	volatile uint8 pagebuf[FWCMD_PAGESIZE];
	
	sint32 loopwrite = size;
	while(loopwrite > 0){
		loopwrite-=FWCMD_PAGESIZE;
		writeFirmwareSPIPage((address + loopwrite), (uint8 *)(source + loopwrite));
		
		//checks FW segment
		readFirmwareSPI((address + loopwrite), (uint8*)&pagebuf[0], FWCMD_PAGESIZE);
		if (!(memcmp((uint8*)&pagebuf[0],(uint8 *)(source + loopwrite),FWCMD_PAGESIZE) == 0)){
			return -1;
		}
	}
	
	return 0;
}
*/

int PowerManagementDeviceRead(int reg) {	//aka readPowerManagement
	return PowerManagementDeviceWrite((reg)|POWMAN_READ_BIT, 0);
}


int PowerManagementDeviceWrite(int reg, int command) {	//aka writePowerManagement
//---------------------------------------------------------------------------------
	uint32 OLD_IME = REG_IME;
	REG_IME = 0;
	
	// Write the register / access mode (bit 7 sets access mode)
	SPIWAITCNT();
	
	REG_SPI_CR = BIT_SPICNT_ENABLE | BIT_SPICLK_1MHZ | BIT_SPICNT_BYTETRANSFER | BIT_SPICNT_CSHOLDENABLE | BIT_SPICNT_POWERCNT;
	REG_SPI_DATA = reg;
	
	// Write the command / start a read
	SPIWAITCNT(); 
	REG_SPI_CR = BIT_SPICNT_ENABLE | BIT_SPICLK_1MHZ | BIT_SPICNT_BYTETRANSFER | BIT_SPICNT_POWERCNT;
	REG_SPI_DATA = command; 
	SPIWAITCNT();

	REG_IME = OLD_IME;

	return REG_SPI_DATA  & BITMASK_SPI_DATA;
}

#endif