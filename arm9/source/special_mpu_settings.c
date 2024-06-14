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

#include "special_mpu_settings.h"
#include "typedefsTGDS.h"
#include "nds_cp15_misc.h"
#include "posixHandleTGDS.h"
#include "linkerTGDS.h"

void enterMPUConfig(){
	asm("mov	r0, #0x1f");   //sys mode
	asm("msr	cpsr, r0");
	MProtectionDisable();
	ICacheDisable();
	DCacheDisable();
	flush_icache_all();
	flush_dcache_all();
	asm("mov r0,#0");
	asm("mcr	p15, 0, r0, c7, c10, 4");	//make cache coherent
}

void leaveMPUConfig(){
	asm("mov	r0, #0x1f");   //sys mode
	asm("msr	cpsr, r0");
	flush_icache_all();
	flush_dcache_all();
	ICacheEnable();
	DCacheEnable();
	asm("mov r0,#0");
	asm("mcr	p15, 0, r0, c7, c10, 4");	//make cache coherent
	MProtectionEnable();
}

void setSnemulDSSpecial0xFFFF0000MPUSettings(){
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[0].regionsettings = (uint32)( PAGE_128M | 0x00000000 | 1);
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[1].regionsettings = (uint32)( PAGE_64K | 0xFFFF0000 | 1);
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[2].regionsettings = (uint32)( PAGE_32K | 0x03000000 | 1); //allow to read/write 0x03000000 @ ARM9 32K Shared WRAM 
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[3].regionsettings = (uint32)( PAGE_128M | 0x08000000 | 1);	//allow to read 0x08000000 ~ 0x0fffffff
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[4].regionsettings = (uint32)((uint32)(&_itcm_start) | PAGE_32K | 1);
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[5].regionsettings = (uint32)((uint32)(&_dtcm_start) | PAGE_16K | 1);
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[6].regionsettings = (uint32)( PAGE_16M 	| 0x02400000 | 1);
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[7].regionsettings = (uint32)( PAGE_4M 	| 0x02000000 | 1);
	mpuSetting[VECTORS_0xFFFF0000_MPU].WriteBufferAvailabilityForRegions = 0b11110101; //EWRAM, DTCM, ITCM are writeable regions. GBA ROM region is NOT allowed to write.
	
	mpuSetting[VECTORS_0xFFFF0000_MPU].DCacheAvailabilityForRegions = 0b10000100;	//DTCM & ITCM caches are enabled for region 2 & 7 to give speedup
	mpuSetting[VECTORS_0xFFFF0000_MPU].ICacheAvailabilityForRegions = 0b10000100;
	
	mpuSetting[VECTORS_0xFFFF0000_MPU].ITCMAccessForRegions = 0x33333333;
	mpuSetting[VECTORS_0xFFFF0000_MPU].DTCMAccessForRegions = 0x33333333;
	
	updateMPUSetting((T_mpuSetting *)&mpuSetting[VECTORS_0xFFFF0000_MPU]);
}