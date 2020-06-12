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

#include "initNDSTGDS.h"
#include "utilsTGDS.h"
#include "dsregs.h"
#include "typedefsTGDS.h"
#include "InterruptsARMCores_h.h"
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "dmaTGDS.h"
#include "timerTGDS.h"
#include "spifwTGDS.h"
#include "powerTGDS.h"
#include "ipcfifoTGDS.h"
#include "soundTGDS.h"
#include "global_settings.h"
#include "eventsTGDS.h"
#include "posixHandleTGDS.h"

#ifdef ARM9
#include "devoptab_devices.h"
#include "videoTGDS.h"
#include "wifi_arm9.h"
#include "fatfslayerTGDS.h"
#include "dldi.h"
#endif

void initHardware(void) {
//---------------------------------------------------------------------------------
	//Reset Both Cores
	resetMemory_ARMCores();
	
	
	#ifdef ARM7
	//Init Shared Address Region and get NDS Header
	memset((uint32*)TGDSIPC, 0, TGDSIPCSize);
	memcpy((u8*)&TGDSIPC->DSHeader,(u8*)0x027FFE00, sizeof(TGDSIPC->DSHeader));
	
	//Read DHCP settings (in order)
	LoadFirmwareSettingsFromFlash();
	
	//Init SoundSampleContext
	initSoundSampleContext();
	initSound();
	
	#endif
	
	#ifdef ARM9
	powerON(POWER_2D_A | POWER_2D_B | POWER_SWAP_LCDS);
	setBacklight(POWMAN_BACKLIGHT_TOP_BIT|POWMAN_BACKLIGHT_BOTTOM_BIT);
	setupDefaultExceptionHandler();	//ARM9 TGDS Exception Handler
	
	//PPU Engines Default
	SETDISPCNT_MAIN(0); 
	SETDISPCNT_SUB(0);
	REG_BG0CNT = REG_BG1CNT = REG_BG2CNT = REG_BG3CNT = 0;
	
	//Library init code
	
	//Newlib init
	//Stream Devices Init: see devoptab_devices.c
	//setbuf(stdin, NULL);
	setbuf(stdout, NULL);	//iprintf directs to DS Framebuffer (printf already does that)
	//setbuf(stderr, NULL);
	
	printf7Setup();
	TryToDefragmentMemory();
	
	#ifdef ARM9_DLDI
	//ARM9DLDI
	setDLDIARM7Address((u32 *)&_dldi_start);
	#endif
	
	//Allocate various TGDS objects
	wifi_connect_point = (Wifi_AccessPoint*)malloc(sizeof(Wifi_AccessPoint));
	WifiData = (Wifi_MainStruct *)malloc(sizeof(Wifi_MainStruct));
	files = (struct fd*)malloc(sizeof(struct fd)*OPEN_MAXTGDS);
	
	//Enable TSC
	setTouchScreenEnabled(true);	
	
	//Enable TGDS Event handling + Set timeout to turn off screens if idle.
	setAndEnableSleepModeInSeconds(SLEEPMODE_SECONDS);
	
	//TGDS Sound stream: Default volume
	setVolume(4);
	
	#endif
	
}
