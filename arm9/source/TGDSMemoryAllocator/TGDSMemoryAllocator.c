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

//This file abstracts specific TGDS memory allocator code which allows for either default malloc or custom implementation malloc, by overriding this source code.

#include "posixHandleTGDS.h"
#include "xmem.h"
#include "dldi.h"
#include "dsregs.h"
#include "utilsTGDS.h"

////////[For custom Memory Allocator implementation]:////////
//You need to override getProjectSpecificMemoryAllocatorSetup():
//After that, TGDS project initializes the default/custom allocator automatically.


	////////[Custom Memory implementation ]////////

//Definition that overrides the weaksymbol expected from toolchain to init ARM9's TGDS memory allocation
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O1")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
struct AllocatorInstance * getProjectSpecificMemoryAllocatorSetup(u32 ARM7MallocStartAddress, int ARM7MallocSize, bool isCustomTGDSMalloc, u32 TargetARM7DLDIAddress) {
	struct AllocatorInstance * customMemoryAllocator = &CustomAllocatorInstance;
	memset((u8*)customMemoryAllocator, 0, sizeof(CustomAllocatorInstance));
	customMemoryAllocator->customMalloc = isCustomTGDSMalloc;
	customMemoryAllocator->ARM7MallocStartAddress = ARM7MallocStartAddress;
	customMemoryAllocator->ARM7MallocSize = ARM7MallocSize;
	
	customMemoryAllocator->ARM9MallocStartaddress = (u32)sbrk(0);
	customMemoryAllocator->memoryToAllocate = (3000*1024); 
	customMemoryAllocator->CustomTGDSMalloc9 = (TGDSARM9MallocHandler)&Xmalloc;
	customMemoryAllocator->CustomTGDSCalloc9 = (TGDSARM9CallocHandler)&Xcalloc;
	customMemoryAllocator->CustomTGDSFree9 = (TGDSARM9FreeHandler)&Xfree;
	customMemoryAllocator->CustomTGDSMallocFreeMemory9 = (TGDSARM9MallocFreeMemoryHandler)&XMEM_FreeMem;
	
	//Init XMEM (let's see how good this one behaves...)
	u32 xmemsize = XMEMTOTALSIZE = customMemoryAllocator->memoryToAllocate;
	xmemsize = xmemsize - (xmemsize/XMEM_BS) - 1024;
	xmemsize = xmemsize - (xmemsize%1024);
	XmemSetup(xmemsize, XMEM_BS);
	XmemInit(customMemoryAllocator->ARM9MallocStartaddress, (u32)customMemoryAllocator->memoryToAllocate);
	
	//DLDI
	customMemoryAllocator->DLDI9StartAddress = (u32)&_io_dldi_stub;
	customMemoryAllocator->TargetARM7DLDIAddress = TargetARM7DLDIAddress;
	
	//Memory Setup: ARM7 TGDS 64K = 0x03800000 ~ 0x03810000. TGDS Sound Streaming code: Custom ARM7 Sound
	WRAM_CR = WRAM_32KARM9_0KARM7;
	
	//libutils
	initializeLibUtils(
		NULL, //HandleFifoNotEmptyWeakRefLibUtils_fn: NULL because TGDS project doesn't use extended FIFO calls from libutils
		NULL, //wifiUpdateVBLANKARM7LibUtils_fn: NULL because TGDS project doesn't use extended FIFO calls from libutils
		NULL, //wifiInterruptARM7LibUtils_fn: NULL because TGDS project doesn't use extended FIFO calls from libutils
		NULL //timerWifiInterruptARM9LibUtils_fn: NULL because TGDS project doesn't use extended FIFO calls from libutils
	);
	
	return customMemoryAllocator;
}