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

#ifdef ARM7

#include "typedefsTGDS.h"
#include "InterruptsARMCores_h.h"
#include "ipcfifoTGDS.h"
#include "linkerTGDS.h"
#include "CPUARMTGDS.h"
#include "xmem.h"
#include "posixHandleTGDS.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

uint32 globalVRAMStackStartPtr=0;
uint32 allocVRAMStacks(){
	globalVRAMStackStartPtr = (uint32)TGDSARM7Malloc(1024 * 8);	
	return globalVRAMStackStartPtr;
}


void deallocVRAMStacks(){
	TGDSARM7Free((uint32*)globalVRAMStackStartPtr);
}

#endif
