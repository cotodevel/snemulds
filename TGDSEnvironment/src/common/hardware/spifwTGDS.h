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

#ifndef __spifw_h
#define __spifw_h

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#define FWCMD_WREN 0x06
#define FWCMD_WRDI 0x04
#define FWCMD_RDID 0x9F
#define FWCMD_RDSR 0x05
#define FWCMD_READ 0x03
#define FWCMD_FAST 0x0B
#define FWCMD_PW   0x0A
#define FWCMD_PP   0x02
#define FWCMD_PE   0xDB
#define FWCMD_SE   0xD8
#define FWCMD_DP   0xB9
#define FWCMD_RDP  0xAB

#define	FWCMD_PAGESIZE	256	//fixed

//Bit0    WIP Write/Program/Erase in Progess (0=No, 1=Busy)
#define	BIT_FWCMD_WIP	(1 << 0)

//Bit1    WEL Write Enable Latch             (0=No, 1=Enable)
#define	BIT_FWCMD_WEL	(1 << 1)


//PowerMan bits
#define 	POWMAN_CONTROL_BIT   0
#define 	POWMAN_BATTERY_BIT   1
#define 	POWMAN_AMPLIFIER_BIT   2
#define 	POWMAN_READ_BIT   (1<<7)
#define 	POWMAN_WRITE_BIT   (0<<7)
#define 	POWMAN_SOUND_AMP_BIT   (1<<0)
#define 	POWMAN_SOUND_MUTE_BIT   (1<<1)
#define 	POWMAN_BACKLIGHT_BOTTOM_BIT   (1<<2)	//#define	PM_BACKLIGHT_BOTTOM	(1<<2)
#define 	POWMAN_BACKLIGHT_TOP_BIT   (1<<3)		//#define	PM_BACKLIGHT_TOP	(1<<3)
#define 	POWMAN_SYSTEM_PWR_BIT   (1<<6)			//#define	PM_SYSTEM_PWR		(1<<6)
#define 	POWMAN_SOUND_PWR_BIT   (1<<0)			//#define 	PM_SOUND_PWR 		(1<<0)
#define 	POWMAN_LED_CONTROL(m)_BIT   ((m)<<4)
#define 	POWMAN_LED_ON_BIT   (0<<4)
#define 	POWMAN_LED_SLEEP_BIT   (1<<4)
#define 	POWMAN_LED_BLINK_BIT   (3<<4)
#define 	POWMAN_AMP_OFFSET_BIT   2
#define 	POWMAN_AMP_ON_BIT   1
#define 	POWMAN_AMP_OFF_BIT   0

#endif


#ifdef __cplusplus
extern "C"{
#endif

#ifdef ARM7
//weak symbols : the implementation of these is project-defined. This callback performs custom button mapping logic in a per-TGDS project basis.
extern __attribute__((weak))	void CustomInputMappingHandler(uint32 readKeys);
#endif

extern void writeFirmwareSPIPage(uint32 address, uint8 * buffer);
extern void readFirmwareSPI(uint32 address, uint8 * destination, uint32 size);
extern int writeFirmwareSPI(uint32 address, void * source, uint32 size);

//powerManagement SPI
extern int PowerManagementDeviceWrite(int reg, int command) ;
extern int PowerManagementDeviceRead(int reg);

#ifdef __cplusplus
}
#endif