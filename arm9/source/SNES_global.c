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

#include "gfx.h"
#include "apu.h"
#include "cfg.h"

#include "cpu.h"
#include "core.h"
#include "engine.h"
#include "apu.h"
#include "ppu.h"
#include "main.h"
#include "conf.h"
#include "fs.h"
#include "memmap.h"
#include "guiTGDS.h"
#include "opcodes.h"
//#include "common.h"
#include "specific_shared.h"
#include "nds_cp15_misc.h"
#include "fsfatlayerTGDS.h"
#include "about.h"
#include "utilsTGDS.h"
#include "clockTGDS.h"
#include "snes.h"


__attribute__((section(".dtcm")))
struct s_cpu	CPU;
__attribute__((section(".arm9sharedwram")))
struct s_gfx	GFX;
struct s_cfg	CFG;
__attribute__((section(".arm9sharedwram")))
struct s_snes	SNES;
__attribute__((section(".dtcm")))
struct s_snescore	SNESC;


__attribute__((section(".dtcm")))
uint16	PPU_PORT[0x90]; // 2100 -> 2183
__attribute__((section(".dtcm")))
uint16	DMA_PORT[0x180]; // 4200 -> 437F

