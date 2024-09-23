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
#include "dldi.h"
#include "dsregs.h"
#include "xmem.h"

	////////[Default Memory implementation (newlib's malloc)]////////

//Definition that overrides the weaksymbol expected from toolchain to init ARM9's TGDS memory allocation
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
struct AllocatorInstance * getProjectSpecificMemoryAllocatorSetup(bool isCustomTGDSMalloc) {
	struct AllocatorInstance * customMemoryAllocator = &CustomAllocatorInstance;
	memset((u8*)customMemoryAllocator, 0, sizeof(CustomAllocatorInstance));
	customMemoryAllocator->customMalloc = isCustomTGDSMalloc;
	
	customMemoryAllocator->ARM9MallocStartaddress = (u32)sbrk(0);
	customMemoryAllocator->memoryToAllocate = (480*1024); //240K of free memory :(, this means GUI won't segfault now but ROM probably will (3MB or less) and will require a reload everytime GUI is tweaked 
	customMemoryAllocator->CustomTGDSMalloc9 = (TGDSARM9MallocHandler)&malloc;
	customMemoryAllocator->CustomTGDSCalloc9 = (TGDSARM9CallocHandler)&calloc;
	customMemoryAllocator->CustomTGDSFree9 = (TGDSARM9FreeHandler)&free;
	customMemoryAllocator->CustomTGDSMallocFreeMemory9 = (TGDSARM9MallocFreeMemoryHandler)&mallocGetFreeMemoryInBytes;
	
	//Memory Setup: ARM7 TGDS 64K = 0x03800000 ~ 0x03810000. TGDS Sound Streaming code: Custom ARM7 Sound
	WRAM_CR = WRAM_32KARM9_0KARM7;

	return customMemoryAllocator;
}


//


	////////[Custom Memory implementation: WoopsiSDK TGDS-multiboot interoperability]////////

//Definition that overrides the weaksymbol expected from toolchain to init ARM9's TGDS memory allocation
struct AllocatorInstance * getWoopsiSDKToolchainGenericDSMultibootMemoryAllocatorSetup(bool isCustomTGDSMalloc){
	struct AllocatorInstance * customMemoryAllocator = &CustomAllocatorInstance;
	memset((u8*)customMemoryAllocator, 0, sizeof(CustomAllocatorInstance));
	customMemoryAllocator->customMalloc = isCustomTGDSMalloc;
	
	extern u32 __vma_stub_end__;
	customMemoryAllocator->ARM9MallocStartaddress = (u32)&__vma_stub_end__;
	customMemoryAllocator->memoryToAllocate = (480*1024); //240K of free memory :(, this means GUI won't segfault now but ROM probably will (3MB or less) and will require a reload everytime GUI is tweaked
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
	
	//Memory Setup: ARM7 TGDS 64K = 0x03800000 ~ 0x03810000. TGDS Sound Streaming code: Custom ARM7 Sound
	WRAM_CR = WRAM_32KARM9_0KARM7;
	
	return customMemoryAllocator;
}