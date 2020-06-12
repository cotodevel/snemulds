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

#include "busTGDS.h"
#include "utilsTGDS.h"

void SetBusSLOT1SLOT2ARM9(){
	uint16 bits = (GBASLOT_SRAMACCESS_TIME_10CYCLE|GBASLOT_ROMACCESS_TIME_10CYCLE_1STAC|GBASLOT_ROMACCESS_TIME_10CYCLE_2NDAC|GBASLOT_PHY_PIN_FREQ_16MHZ|NDSSLOT_ARM9BUS|GBASLOT_ARM9BUS);
	//allow slot1 & slot2 to be read from ARM9 by default
	Write16bitAddrExtArm(0x04000204, bits);
	setCpuBusAccessPrio(bits);
}	

void SetBusSLOT1SLOT2ARM7(){
	uint16 bits = (GBASLOT_SRAMACCESS_TIME_10CYCLE|GBASLOT_ROMACCESS_TIME_10CYCLE_1STAC|GBASLOT_ROMACCESS_TIME_10CYCLE_2NDAC|GBASLOT_PHY_PIN_FREQ_16MHZ|NDSSLOT_ARM7BUS|GBASLOT_ARM7BUS);
	//allow slot1 & slot2 to be read from ARM9 by default
	Write16bitAddrExtArm(0x04000204, bits);
	setCpuBusAccessPrio(bits);
}	

void SetBusSLOT1ARM7SLOT2ARM9(){
	uint16 bits = (GBASLOT_SRAMACCESS_TIME_10CYCLE|GBASLOT_ROMACCESS_TIME_10CYCLE_1STAC|GBASLOT_ROMACCESS_TIME_10CYCLE_2NDAC|GBASLOT_PHY_PIN_FREQ_16MHZ|NDSSLOT_ARM7BUS|GBASLOT_ARM9BUS);
	//allow slot1 & slot2 to be read from ARM9 by default
	Write16bitAddrExtArm(0x04000204, bits);
	setCpuBusAccessPrio(bits);
}

void SetBusSLOT1ARM9SLOT2ARM7(){
	uint16 bits = (GBASLOT_SRAMACCESS_TIME_10CYCLE|GBASLOT_ROMACCESS_TIME_10CYCLE_1STAC|GBASLOT_ROMACCESS_TIME_10CYCLE_2NDAC|GBASLOT_PHY_PIN_FREQ_16MHZ|NDSSLOT_ARM9BUS|GBASLOT_ARM7BUS);
	//allow slot1 & slot2 to be read from ARM9 by default
	Write16bitAddrExtArm(0x04000204, bits);
	setCpuBusAccessPrio(bits);
}

//Set BUS access rights
void setCpuBusAccessPrio(uint16 bus_assignment){
	EXMEMCNT	= (uint16)(bus_assignment);
}