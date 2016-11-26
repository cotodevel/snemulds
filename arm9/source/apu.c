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
#include "ipc_libnds_extended.h"

//Coto: deprecated, new hardware IPC fifo is better
/*
void SendArm7Command(u32 command) {
   fifoSendValue32(FIFO_USER_01,command);
   
   //wait for command to complete
   while(!fifoCheckValue32(FIFO_USER_01));
   if(fifoGetValue32(FIFO_USER_01)!=1) while(1);
} 
*/

void	APU_reset()
{
    APU_command(0x00000001);
}


void	APU_nice_reset()
{
#ifndef IN_EMULATOR	
	APU_stop();
	APU_reset();	
#endif
	
}

void	APU_pause()
{
    APU_command(0x00000002);
}

void	APU_stop()
{
#ifndef IN_EMULATOR	
	APU_command(0x00000004);
#endif	
}

void	APU_playSpc()
{
    APU_command(0x00000003);
}

void	APU_saveSpc()
{
	APU_command(0x00000006);
	// Wait the ARM7 to save the SPC
	// FIXME : replace this with a variable check
}

void	APU_loadSpc()
{
	APU_command(0x00000007);
	// Wait the ARM7 to load the SPC
	// FIXME : replace this with a variable check
}


void	APU_clear()
{
	APU_command(0x00000005);
	MyIPC->APU_ADDR_CNT = 0;
}


void APU_playSong(uint8 *data, int size)
{
	CFG.Sound_output = 0; // Disable Sound emulation
	if (size > 0x10000 + 0x100 + 0x100)
		return;
	
    APU_command(0x00000004);    // Disable APU
	
    
    memcpy(APU_RAM_ADDRESS-0x100, data, size);
    APU_command(0x00000003);    // Put APU in PLAY MODE	
}


void APU_command(uint32 command)
{
	SendArm7Command(command,0x00000000,0x00000000,0x00000000);
    swiWaitForVBlank();
}