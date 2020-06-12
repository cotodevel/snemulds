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

#ifndef __cpuarm_h__
#define __cpuarm_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <unistd.h>

#endif

#ifdef __cplusplus
extern "C"{
#endif

#ifdef ARM7
extern void useARM7VRAMStacks();		
extern uint32 allocVRAMStacks();	//internal: used by useARM7VRAMStacks
extern void deallocVRAMStacks();	//internal: used if you want to abandon VRAM ARM7 stacks
extern uint32 globalVRAMStackStartPtr;
#endif

#ifdef __cplusplus
}
#endif