/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* Free for non-commercial use                             */
/***********************************************************/

#include <nds.h>

#include "cfg.h"
#include "apu.h"

void ARM7_fifo_init(void)
{
 
} 

void SendArm7Command(u32 command) {
   REG_IPC_FIFO_TX = command;
} 

void	APU_reset()
{
	SendArm7Command(1);	
}

void	APU_pause()
{
	SendArm7Command(2);	
}

void	APU_stop()
{
	*APU_ADDR_ANS = 0;
	SendArm7Command(4);
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