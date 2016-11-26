/*---------------------------------------------------------------------------------
	Touch screen control for the ARM7

	Copyright (C) 2005
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)
		put in pocketSPC by Nicolas Lannier (archeide)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
			must not claim that you wrote the original software. If you use
			this software in a product, an acknowledgment in the product
			documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and
			must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source
			distribution.

---------------------------------------------------------------------------------*/

#include "ipc_libnds_extended.h"

#include <nds/ndstypes.h>
#include <nds/system.h>
#include <nds/arm7/touch.h>
#include <nds/input.h>
#include <nds/interrupts.h>

#include <nds/bios.h>
#include <nds/arm7/clock.h>
#include <nds/ipc.h>

#include "touch.h"
#include <stdlib.h>

#define TSC_MEASURE_TEMP1    0x84
//#define TSC_MEASURE_Y        0x91
#define TSC_MEASURE_BATTERY  0xA4
//#define TSC_MEASURE_Z1       0xB1
//#define TSC_MEASURE_Z2       0xC1
//#define TSC_MEASURE_X        0xD1
//#define TSC_MEASURE_AUX      0xE4
#define TSC_MEASURE_TEMP2    0xF4

//coto: original libnds touchscreen usage.
void updateMyIPC()
{
	uint16 but=0, batt=0;
	int t1=0, t2=0;
	uint32 temp=0;
	uint8 ct[sizeof(MyIPC->curtime)];
	u32 i;
	
    touchPosition tempPos;
    touchReadXY(&tempPos);
    
	// Read the touch screen
	but = REG_KEYXY;
	batt = touchRead(TSC_MEASURE_BATTERY);
 
	// Read the time
	rtcGetTime((uint8 *)ct);
	BCDToInteger((uint8 *)&(ct[1]), 7);
 
	// Read the temperature
	temp = touchReadTemperature(&t1, &t2);
 
	// Update the MyIPC struct
	MyIPC->buttons	            	= REG_KEYINPUT;
    MyIPC->buttons_xy_folding		= but;
	MyIPC->touched                  = (u8)CheckStylus();
    MyIPC->touch_pendown           = (u8)touchPenDown();
    
    //raw x/y
    MyIPC->touchX    = tempPos.rawx;
    MyIPC->touchY    = tempPos.rawy;
    
    //TFT x/y pixel
    MyIPC->touchXpx = tempPos.px;
    MyIPC->touchYpx = tempPos.py;    
    
	MyIPC->touchZ1 = tempPos.z1;
	MyIPC->touchZ2 = tempPos.z2;
	MyIPC->battery		= batt;
 
	for(i=0; i<sizeof(ct); i++) {
		MyIPC->curtime[i] = ct[i];
	}
 
	MyIPC->temperature = temp;
	MyIPC->tdiode1 = t1;
	MyIPC->tdiode2 = t2;	
}