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

#include "cfg.h"
#include "apu.h"

void ARM7_fifo_init(void)
{
   //activate FIFO
   REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR;
} 

void SendArm7Command(u32 command) {
   while (REG_IPC_FIFO_CR & IPC_FIFO_SEND_FULL) ;
   if (REG_IPC_FIFO_CR & IPC_FIFO_ERROR) {
      REG_IPC_FIFO_CR |= IPC_FIFO_SEND_CLEAR;
   }
   REG_IPC_FIFO_TX = command;
} 

void	APU_reset()
{
	SendArm7Command(1);	
}

void	APU_nice_reset()
{
	APU_stop();
	APU_reset();	
}

void	APU_pause()
{
	SendArm7Command(2);	
}

void	APU_stop()
{
	*APU_ADDR_ANS = 0;
	SendArm7Command(4);
	// Wait the APU disabling
	while (*APU_ADDR_ANS != 0xFF00FF00);	
}

void	APU_playSpc()
{
	SendArm7Command(3);
}

void	APU_saveSpc()
{
	SendArm7Command(6);
}

void	APU_loadSpc()
{
	SendArm7Command(7);
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
	memcpy(APU_RAM_ADDRESS-0x100, data, size);
	SendArm7Command(3); // Put APU in PLAY MODE		
}


void APU_command(uint32 command)
{
	SendArm7Command(command);	
}