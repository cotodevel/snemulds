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

#include "snes.h"
#include "cpu.h"
#include "apu.h"
#include "gfx.h"
#include "cfg.h"

//IN_DTCM
struct s_cpu	CPU;
struct s_apu	APU;
struct s_gfx	GFX;
struct s_cfg	CFG;
struct s_snes	SNES;
IN_DTCM
struct s_snescore	SNESC;

struct s_apu2 *APU2 = ((struct s_apu2 *)(0x27ED000));


IN_DTCM
uint16	PPU_PORT[0x90]; // 2100 -> 2183
IN_DTCM
uint16	DMA_PORT[0x180]; // 4200 -> 437F

