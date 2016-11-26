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

#include <nds.h>
#include <nds/dma.h>

#include "cfg.h"
#include "apu.h"


void SendArm7Command(u32 command) {
   fifoSendValue32(FIFO_USER_01,command);
   
   //wait for command to complete
   while(!fifoCheckValue32(FIFO_USER_01));
   if(fifoGetValue32(FIFO_USER_01)!=1) while(1);
} 

void	APU_reset()
{
	SendArm7Command(1);	
}


void	APU_nice_reset()
{
#ifndef IN_EMULATOR	
	APU_stop();
	APU_reset();	
#endif
	
	swiWaitForVBlank();
	swiWaitForVBlank();
}

void	APU_pause()
{
	SendArm7Command(2);	
}

void	APU_stop()
{
#ifndef IN_EMULATOR	
	SendArm7Command(4);
#endif	
}

void	APU_playSpc()
{
	SendArm7Command(3);
}

void	APU_saveSpc()
{
	SendArm7Command(6);
	
	// Wait the ARM7 to save the SPC
	// FIXME : replace this with a variable check
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();	
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();	
	swiWaitForVBlank();
	swiWaitForVBlank();		
}

void	APU_loadSpc()
{
	SendArm7Command(7);

	// Wait the ARM7 to load the SPC
	// FIXME : replace this with a variable check
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();	
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();	
	swiWaitForVBlank();
	swiWaitForVBlank();		
}


void	APU_clear()
{
	SendArm7Command(5);
	*APU_ADDR_CNT = 0;
}


void APU_playSong(uint8 *data, int size)
{
	CFG.Sound_output = 0; // Disable Sound emulation
	if (size > 0x10000 + 0x100 + 0x100)
		return;
	
	SendArm7Command(4); // Disable APU	
	
	//memcpy(APU_RAM_ADDRESS-0x100, data, size);
	dmaCopyHalfWords(3,(void *)(data), (void *)(APU_RAM_ADDRESS-0x100), (int)(size/2)); // /2

	
	SendArm7Command(3); // Put APU in PLAY MODE		
}


void APU_command(uint32 command)
{
	SendArm7Command(command);	
}