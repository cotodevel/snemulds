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

#include "usrsettingsTGDS.h"
#include "ipcfifoTGDS.h"
#include "biosTGDS.h"
#include "dmaTGDS.h"
#include "utilsTGDS.h"

#ifdef ARM9
#include "nds_cp15_misc.h"
#endif

#ifdef ARM7
#include "spifwTGDS.h"

void LoadFirmwareSettingsFromFlash(){
	
	//Load firmware from FLASH
	struct sDSFWHEADER DSFWHEADERInst;
	
	readFirmwareSPI((uint32)0x20, (uint8*)&DSFWHEADERInst.usersettings_offset[0], 0x2);	//020h 2    User Settings Offset (div8) (usually last 200h flash bytes)
	
	//Get User Settings : Offset (div8) accounted
	uint32 usersetting_offset0 = (DSFWHEADERInst.usersettings_offset[1]<<8) | DSFWHEADERInst.usersettings_offset[0];
	usersetting_offset0 = (usersetting_offset0 * 8);
	
	uint32 usersetting_offset1 = usersetting_offset0 + (sint32)DS_FW_USERSETTINGS_SIZE;
	
	//Load DS Firmware User Settings
	struct sDSFWSETTINGS UserSettings0;
	struct sDSFWSETTINGS UserSettings1;
	
	//Load UserSettings
	readFirmwareSPI((uint32)usersetting_offset0, (uint8*)&UserSettings0, sizeof(struct sDSFWSETTINGS));
	readFirmwareSPI((uint32)usersetting_offset1, (uint8*)&UserSettings1, sizeof(struct sDSFWSETTINGS));
	
	//getCRC and validate them
	uint16 crcreadUserSet0 = swiCRC16( 0xffff, (uint8*)&UserSettings0, 0x70);	//CRC16 of entries 00h..6Fh (70h bytes)
	uint16 crcreadUserSet1 = swiCRC16( 0xffff, (uint8*)&UserSettings1, 0x70);	//CRC16 of entries 00h..6Fh (70h bytes)
	
	uint16 crcslot0 = (UserSettings0.crc16fwsetting[1]<<8) | UserSettings0.crc16fwsetting[0];
	uint16 crcslot1 = (UserSettings1.crc16fwsetting[1]<<8) | UserSettings1.crc16fwsetting[0];
	
	//Slot0 Valid
	if(crcslot0 == crcreadUserSet0){
		ParseFWSettings(usersetting_offset0);
	}
	//Slot1 Valid
	else if(crcslot1 == crcreadUserSet1){
		ParseFWSettings(usersetting_offset1);
	}
	else{
		//Proceed, but invalid NVRAM settings
	}
	
}

void ParseFWSettings(uint32 usersetting_offset){
	
	readFirmwareSPI((uint32)usersetting_offset, (uint8*)&TGDSIPC->DSFWSETTINGSInst, sizeof(struct sDSFWSETTINGS));
	readFirmwareSPI((uint32)usersetting_offset+0x01D, (uint8*)&TGDSIPC->consoletype, sizeof(TGDSIPC->consoletype));
	ucs2tombs((uint8*)&TGDSIPC->nickname_schar8[0],(unsigned short*)&TGDSIPC->DSFWSETTINGSInst.nickname_utf16[0],32);
	int nicknameLength = (int)(TGDSIPC->DSFWSETTINGSInst.nickname_length_chars[0] | TGDSIPC->DSFWSETTINGSInst.nickname_length_chars[1] << 8);
	TGDSIPC->nickname_schar8[nicknameLength] = '\0'; 
	readFirmwareSPI((uint32)usersetting_offset+0x64, (uint8*)&TGDSIPC->lang_flags[0], 0x2);
}

#endif
