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

#ifndef	_cp15_misc_
#define _cp15_misc_

#include "dsregs.h"
#include "typedefsTGDS.h"
#include "dsregs_asm.h"

//Each MPU setting
#define VECTORS_0x00000000_MPU	0
#define VECTORS_0xFFFF0000_MPU	1

typedef struct s_regionSetting {
	//format: (Same as CP15Opcodes.S)
	//addr = IO address
	//PAGE_X = Region size in Page Size units to cover, starting addr
	//(1<<0) & (region_enable_bit<<0) = bit setting for enable(1) or disable(0) such Region.
	//example: ( PAGE_64M | addr | 1)
	uint32 regionsettings;
}T_regionSetting;

typedef struct s_mpuSetting {
	T_regionSetting inst_regionSetting[8];
	
	//Defines regions ( 0 -- 7) where the AHB (Drain) Write Buffer is Enabled.
	//http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0201d/I1033977.html <-- good explanation.
	//Basically those regions are allowed to write-miss / write-back without causing external access (to whatever memory is connected to caches).
	// If write-miss, data (changes) is buffered locally. If write-back happens the cache-line is marked as dirty ONLY (no external access), causing two scenarios:
	//a) A read happens, and ALSO causes a cache-hit, no external accesses happen. Speeding up code.
	//b) Otherwise, a read cache-miss will happen, a write-back happens, then the old cache-line dirty is updated.
	//So: Access/Writes external memory, ONLY when reads cause a cache-miss. Otherwise Access/Writes are "cached".
	
	//DrainWrite buffer Opcode makes sure all writes buffered HAPPEN!
	uint32 WriteBufferAvailabilityForRegions;
		
	uint32 DCacheAvailabilityForRegions;
	uint32 ICacheAvailabilityForRegions;
	
	uint32 ITCMAccessForRegions;
	uint32 DTCMAccessForRegions;
	
}T_mpuSetting;

#endif
 
#ifdef __cplusplus
extern "C"{
#endif

extern uint32 EWRAMUncached(uint32 address);
extern uint32 EWRAMCached(uint32 address);
//Each region listed.
//Use one for MPU settings : vectors @0x00000000
//Use another for MPU settings : vectors @0xffff0000 
extern T_mpuSetting mpuSetting[2];

extern void updateMPUSetting(T_mpuSetting * mpuSetting_inst);

//proc-arm946.S
extern void cpu_arm946_proc_init();
extern void cpu_arm946_proc_fin();
extern void cpu_arm946_reset(uint32 loc);
extern void flush_icache_all();
extern void flush_dcache_all();	//same as flush_dcache_area(uint32 * addr, sint32 size); + full dtcm size

//same opcodes
//extern void flush_kern_cache_all();	//bugged
//extern void flush_user_cache_all();	//bugged, replaced with flush_dcache_area(uint32 * addr, sint32 size);

extern void flush_user_cache_range(uint32 start, uint32 end, uint32 flags);
extern void coherent_kern_range(uint32 start,uint32 end);
extern void coherent_user_range(uint32 start,uint32 end);
extern void coherent_user_range_by_size(uint32 start,sint32 size);

extern void flush_kern_dcache_area(uint32 * addr, sint32 size);	//DCache + ICache Invalidate and Clean Range
extern void flush_dcache_area(uint32 * addr, sint32 size);		//DCache Invalidate and Clean Range

extern void dma_inv_range(uint32 start,uint32  end);
extern void dma_clean_range(uint32 start,uint32 end);
extern void dma_flush_range(uint32 start,uint32 end);
extern void dma_unmap_area(uint32 start,sint32 size,uint32 dir);

//format:
//addr = IO address
//PAGE_X = Region size in Page Size units to cover, starting addr
//(1<<0) & (region_enable_bit<<0) = bit setting for enable(1) or disable(0) such Region.
//example: ( PAGE_64M | addr | 1)
extern void MProtectionRegionBase0(uint32 region);
extern void MProtectionRegionBase1(uint32 region);
extern void MProtectionRegionBase2(uint32 region);
extern void MProtectionRegionBase3(uint32 region);
extern void MProtectionRegionBase4(uint32 region);
extern void MProtectionRegionBase5(uint32 region);
extern void MProtectionRegionBase6(uint32 region);
extern void MProtectionRegionBase7(uint32 region);

extern void WritebufferControl(uint32 regionSettings); //pu_GetWriteBufferability(uint32 v);
extern void RegionDCacheEnable(uint32 regionSettings); //void pu_SetDataCachability(uint32 v);
extern void RegionICacheEnable(uint32 regionSettings); //void pu_SetCodeCachability(uint32 v);
extern void RegionIAccessPermission(uint32 regionSettings); //void pu_SetCodePermissions(uint32 v);
extern void RegionDAccessPermission(uint32 regionSettings); //pu_SetDataPermissions(uint32 v);

extern void MProtectionDisable();
extern void MProtectionEnable();

extern void ICacheDisable();
extern void DCacheDisable();

extern void ICacheEnable();
extern void DCacheEnable();

extern void ITCMEnable();
extern void DTCMEnable();

extern void ITCMDisable();
extern void DTCMDisable();

//C1,C0,0 - Control Register (R/W, or R=Fixed)
//  0  MMU/PU Enable         (0=Disable, 1=Enable) (Fixed 0 if none)
//  1  Alignment Fault Check (0=Disable, 1=Enable) (Fixed 0/1 if none/always on)
//  2  Data/Unified Cache    (0=Disable, 1=Enable) (Fixed 0/1 if none/always on)
//  3  Write Buffer          (0=Disable, 1=Enable) (Fixed 0/1 if none/always on)
//  4  Exception Handling    (0=26bit, 1=32bit)    (Fixed 1 if always 32bit)
//  5  26bit-address faults  (0=Enable, 1=Disable) (Fixed 1 if always 32bit)
//  6  Abort Model (pre v4)  (0=Early, 1=Late Abort) (Fixed 1 if ARMv4 and up)
//  7  Endian                (0=Little, 1=Big)     (Fixed 0/1 if fixed)
//  8  System Protection bit (MMU-only)
//  9  ROM Protection bit    (MMU-only)
//  10 Implementation defined
//  11 Branch Prediction     (0=Disable, 1=Enable)
//  12 Instruction Cache     (0=Disable, 1=Enable) (ignored if Unified cache)
//  13 Exception Vectors     (0=00000000h, 1=FFFF0000h)
//  14 Cache Replacement     (0=Normal/PseudoRandom, 1=Predictable/RoundRobin)
//  15 Pre-ARMv5 Mode        (0=Normal, 1=Pre ARMv5; LDM/LDR/POP_PC.Bit0/Thumb)
//  16 DTCM Enable           (0=Disable, 1=Enable)
//  17 DTCM Load Mode        (0=R/W, 1=DTCM Write-only)
//  18 ITCM Enable           (0=Disable, 1=Enable)
//  19 ITCM Load Mode        (0=R/W, 1=ITCM Write-only)
//  20-31 Reserved           (keep these bits unchanged) (usually zero)
  
extern void CP15ControlRegisterEnable(uint32 ControlRegisterBits); //extern void cpu_SetCP15Cnt(uint32 v); 
extern void CP15ControlRegisterDisable(uint32 ControlRegisterBits); //extern uint32 cpu_GetCP15Cnt(); 
extern void DrainWriteBuffer();

#ifdef EXCEPTION_VECTORS_0xffff0000
//Sets default MPU Settings to use anytime. Uses vectors @ 0xffff0000
extern void set0xFFFF0000FastMPUSettings();
#endif

extern void setitcm();
extern void setdtcm();

//todo

//instruction cache CP15
extern void setitcmbase(); //@ ITCM base = 0 , size = 32 MB
extern void icacheenable(int);

//data cache CP15
extern void setdtcmbase(); //@ DTCM base = __dtcm_start, size = 16 KB
extern void drainwrite();
extern void dcacheenable(int); //Cachability Bits for Data/Unified Protection Region (R/W)
extern uint32 getdtcmbase();
extern uint32 getitcmbase();


#ifdef __cplusplus
}
#endif
