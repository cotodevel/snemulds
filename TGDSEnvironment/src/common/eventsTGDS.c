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

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "eventsTGDS.h"
#include "ipcfifoTGDS.h"
#ifdef ARM7
#include "wifi_arm7.h"
#endif
#include "spifwTGDS.h"

//TGDS events implementation (useful for stuff like press input / play sound / power handling given some event)

//ARM7 part because it's mostly unused, and keeping stuff in the IWRAM does not affect ARM9 at all.
#ifdef ARM7
int secondsPassed = 0;
int dsvcount = 0;
bool eventsTGDSActive = false;
int sleepModeTimeout = 0;	//Value in seconds
u32 TGDSEvent = TGDS_EVENT_IDLE;	//for speed and simplicity sake I allow up to 1 event, then after executed it is removed
bool sleepModeEnabled = false;

int getTurnOffScreensTimeout(){
	return (int)sleepModeTimeout;
}

#endif

//Shared
void enableTGDSEventHandling(){
	#ifdef ARM7
	eventsTGDSActive = true;
	#endif
	#ifdef ARM9
	SendFIFOWords(TGDS_ARM7_ENABLE_EVENT_HANDLING, 0);
	#endif
}

void TGDSSetEvent(int newEvent){
	#ifdef ARM7
	TGDSEvent = newEvent;
	#endif
	#ifdef ARM9
	SendFIFOWords(TGDS_ARM7_SET_EVENT_HANDLING, (u32)newEvent);
	#endif
}

void disableTGDSEventHandling(){
	#ifdef ARM7
	eventsTGDSActive = false;
	#endif
	#ifdef ARM9
	SendFIFOWords(TGDS_ARM7_DISABLE_EVENT_HANDLING, 0);
	#endif
}

//These bits are part of sleep mode, because sleep mode powers off most hardware. These just turn off the screens
void setTurnOffScreensTimeout(int secondsBeforeEventTrigger){
	#ifdef ARM7
	sleepModeTimeout = secondsBeforeEventTrigger;
	#endif
	#ifdef ARM9
	SendFIFOWords(TGDS_ARM7_ENABLE_SLEEPMODE_TIMEOUT, (u32)secondsBeforeEventTrigger);
	#endif
}

void TurnOnScreens(){
	#ifdef ARM7
	secondsPassed = 0;
	dsvcount = 0;
	setBacklight(POWMAN_BACKLIGHT_TOP_BIT | POWMAN_BACKLIGHT_BOTTOM_BIT);	//both lit screens
	SetLedState(LED_ON);
	#endif
	
	#ifdef ARM9
	SendFIFOWords(TGDS_ARM7_TURNON_BACKLIGHT, 0);
	#endif
}

void TurnOffScreens(){
	#ifdef ARM7
	setBacklight(0);
	SetLedState(LED_LONGBLINK);
	#endif
	
	#ifdef ARM9
	SendFIFOWords(TGDS_ARM7_TURNOFF_BACKLIGHT, 0);
	#endif
}

void setAndEnableSleepModeInSeconds(int seconds){
	//Enable TGDS Event handling
	enableTGDSEventHandling();
	TGDSSetEvent(TGDS_TIMEOUT_EVENT_HANDLE_SLEEPMODE);
	setTurnOffScreensTimeout(seconds);
	
	//SleepMode enable
	enableSleepMode();
}

void enableSleepMode(){
	#ifdef ARM7
	sleepModeEnabled = true;
	#endif
	#ifdef ARM9
	SendFIFOWords(TGDS_ARM7_ENABLE_SLEEPMODE, 0);
	#endif
}

void disableSleepMode(){
	#ifdef ARM7
	sleepModeEnabled = false;
	//Turn on screens if sleep mode was triggered before disabling it
	TurnOnScreens();
	#endif
	#ifdef ARM9
	SendFIFOWords(TGDS_ARM7_DISABLE_SLEEPMODE, 0);
	#endif
}
