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

#ifndef __eventsTGDS_h__
#define __eventsTGDS_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

//Seconds before sleepmode takes place
#define SLEEPMODE_SECONDS (int)(15)

//Max Events per TGDS instance
#define TGDS_MAX_EVENTS (int)(16)

//Events
#define TGDS_EVENT_IDLE (u32)(0xF0AA1A00)
#define TGDS_EVENT_PRESS_KEY (u32)(0xF0AA1A01)
#define TGDS_TIMEOUT_EVENT_HANDLE_SLEEPMODE (u32)(0xF0AA1A02)

#ifdef __cplusplus
extern "C"{
#endif

#ifdef ARM7
extern int secondsPassed;
extern int dsvcount;
extern bool eventsTGDSActive;
extern int sleepModeTimeout;	//Value in seconds
extern u32 TGDSEvent;	//for speed and simplicity sake I allow up to 1 event, then after executed it is removed
extern bool sleepModeEnabled;

//If sleepMode is active, these take into effect
extern int getTurnOffScreensTimeout();
#endif

extern void enableTGDSEventHandling();
extern void disableTGDSEventHandling();
extern void TGDSSetEvent(int newEvent);
extern void setTurnOffScreensTimeout(int secondsBeforeEventTrigger);
extern void TurnOnScreens();
extern void TurnOffScreens();
extern void enableSleepMode();
extern void disableSleepMode();
extern void setAndEnableSleepModeInSeconds(int seconds);

#ifdef __cplusplus
}
#endif

#ifdef ARM7

//TGDS Event handler: Can be set manually, or irq driven; thus, project specific.
static inline void TGDSEventHandler(){
	if(eventsTGDSActive == true){
		switch((u32)TGDSEvent){
			case((u32)TGDS_TIMEOUT_EVENT_HANDLE_SLEEPMODE):{
				if(sleepModeEnabled == true){
					if(dsvcount < 60){
						dsvcount++;
					}
					else{
						secondsPassed++;
						if(secondsPassed == SLEEPMODE_SECONDS){
							TurnOffScreens();
						}
						dsvcount = 0;
					}
				}
			}
			break;
		}
	}
}
#endif

#endif
