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

#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "utilsTGDS.h"

#ifdef ARM7
#include <string.h>
#include "wifi_arm7.h"
#include "spifwTGDS.h"
#endif

#ifdef ARM9
#include <stdbool.h>

#include "dsregs.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"
#include "wifi_arm9.h"

#endif

//Turn On the BG/3D engines if ARM9
//Turn On Sound Speakers / Wifi if ARM7
void powerON(uint32 values){
	#ifdef ARM7
	REG_POWERCNT |= values;
	#endif
	
	#ifdef ARM9
	if(!(values & POWERMAN_ARM9)){
		SendFIFOWords(FIFO_POWERCNT_ON, (uint32)values);
	}
	else{
		REG_POWERCNT |= values;
	}
	#endif
}

//Turn Off the BG/3D engines if ARM9
//Turn Off Sound Speakers / Wifi if ARM7
void powerOFF(uint32 values){
	#ifdef ARM7
	REG_POWERCNT &= ~values;
	#endif
	
	#ifdef ARM9
	if(!(values & POWERMAN_ARM9)){
		SendFIFOWords(FIFO_POWERCNT_OFF, (uint32)values);
	}
	else{
		REG_POWERCNT &= ~values;
	}
	#endif
}

#ifdef ARM7
void SoundPowerON(u8 volume){
	powerON(POWER_SOUND);
	PowerManagementDeviceWrite(PM_CONTROL_REG, ( PowerManagementDeviceRead(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
	REG_SOUNDCNT = SOUND_ENABLE;
	REG_MASTER_VOLUME = volume;
}
#endif