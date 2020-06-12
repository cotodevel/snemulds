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

#ifndef __nds_bus_h__
#define __nds_bus_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

//0-1   32-pin GBA Slot SRAM Access Time    (0-3 = 10, 8, 6, 18 cycles)  
#define GBASLOT_SRAMACCESS_TIME_10CYCLE	(uint16)(0<<0)
#define GBASLOT_SRAMACCESS_TIME_8CYCLE	(uint16)(1<<0)
#define GBASLOT_SRAMACCESS_TIME_6CYCLE	(uint16)(2<<0)
#define GBASLOT_SRAMACCESS_TIME_18CYCLE	(uint16)(3<<0)

//2-3   32-pin GBA Slot ROM 1st Access Time (0-3 = 10, 8, 6, 18 cycles)
#define GBASLOT_ROMACCESS_TIME_10CYCLE_1STAC	(uint16)(0<<2)
#define GBASLOT_ROMACCESS_TIME_8CYCLE_1STAC	(uint16)(1<<2)
#define GBASLOT_ROMACCESS_TIME_6CYCLE_1STAC	(uint16)(2<<2)
#define GBASLOT_ROMACCESS_TIME_18CYCLE_1STAC	(uint16)(3<<2)

//4     32-pin GBA Slot ROM 2nd Access Time (0-1 = 6, 4 cycles)
#define GBASLOT_ROMACCESS_TIME_10CYCLE_2NDAC	(uint16)(0<<4)
#define GBASLOT_ROMACCESS_TIME_8CYCLE_2NDAC	(uint16)(1<<4)
#define GBASLOT_ROMACCESS_TIME_6CYCLE_2NDAC	(uint16)(2<<4)
#define GBASLOT_ROMACCESS_TIME_18CYCLE_2NDAC	(uint16)(3<<4)

//5-6   32-pin GBA Slot PHI-pin out   (0-3 = Low, 4.19MHz, 8.38MHz, 16.76MHz)
#define GBASLOT_PHY_PIN_FREQ_LOW	(uint16)(0<<5)
#define GBASLOT_PHY_PIN_FREQ_4MHZ	(uint16)(1<<5)
#define GBASLOT_PHY_PIN_FREQ_8MHZ	(uint16)(2<<5)
#define GBASLOT_PHY_PIN_FREQ_16MHZ	(uint16)(3<<5)

//7     32-pin GBA Slot Access Rights     (0=ARM9, 1=ARM7)
#define NDSSLOTBUS_ACCESS_PRIO	(uint16)(1<<11)	//>1 ARM7, 0 ARM9
#define GBASLOTBUS_ACCESS_PRIO	(uint16)(1<<7)	//>1 ARM7, 0 ARM9
#define NDSSLOT_ARM9BUS	(uint16)0
#define NDSSLOT_ARM7BUS	(uint16)NDSSLOTBUS_ACCESS_PRIO
#define GBASLOT_ARM9BUS	(uint16)0
#define GBASLOT_ARM7BUS	(uint16)GBASLOTBUS_ACCESS_PRIO


#endif

#ifdef __cplusplus
extern "C"{
#endif

extern void SetBusSLOT1SLOT2ARM9();
extern void SetBusSLOT1SLOT2ARM7();
extern void SetBusSLOT1ARM7SLOT2ARM9();
extern void SetBusSLOT1ARM9SLOT2ARM7();
extern void setCpuBusAccessPrio(uint16 bus_assignment);

#ifdef __cplusplus
}
#endif