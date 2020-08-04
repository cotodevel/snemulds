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

#ifndef __apu_h__
#define __apu_h__

#include "common.h"

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern void	APU_reset();
extern void	APU_nice_reset();
extern void	APU_pause();
extern void	APU_stop();
extern void	APU_playSpc(u8 * inSPCBuffer);
extern void	APU_saveSpc(u8 * inSPCBuffer);
extern void	APU_loadSpc();
extern void	APU_clear();
extern void APU_command(uint32 command);

#ifdef __cplusplus
}
#endif
