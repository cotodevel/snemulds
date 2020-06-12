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

#ifndef __usrsettings_h
#define __usrsettings_h

#include "typedefsTGDS.h"
#include "dsregs.h"
#include <stdbool.h>


/* Firmware Header */
//Firmware Header (00000h-001FFh)

struct sDSFWHEADER {
  //Addr Size Expl.
  uint8	romaddrarm9guicode[2];	//000h 2    part3 romaddr/8 (arm9 gui code) (LZ/huffman compression)
  uint8	romaddrarm7wificode[2];	//002h 2    part4 romaddr/8 (arm7 wifi code) (LZ/huffman compression)
  uint8	crc16arm97guiwificode[2];	//004h 2    part3/4 CRC16 arm9/7 gui/wifi code
  uint8	crc16arm97bootcode[2];	//006h 2    part1/2 CRC16 arm9/7 boot code
  uint8	firmwareidentifier[4];	//008h 4    firmware identifier (usually nintendo "MAC",nn) (or nocash "XBOO") the 4th byte (nn) occassionally changes in different versions
  uint8	arm9bootcode[2];	//00Ch 2    part1 arm9 boot code romaddr/2^(2+shift1) (LZSS compressed)
  uint8	arm9bootcode0x02800000[2];	//00Eh 2    part1 arm9 boot code 2800000h-ramaddr/2^(2+shift2)
  uint8	arm7bootcode[2];	//010h 2    part2 arm7 boot code romaddr/2^(2+shift3) (LZSS compressed)
  uint8	arm7bootcode0x03810000[2];	//012h 2    part2 arm7 boot code 3810000h-ramaddr/2^(2+shift4)
  uint8	shiftammount[2];	//014h 2    shift amounts, bit0-2=shift1, bit3-5=shift2, bit6-8=shift3, bit9-11=shift4, bit12-15=firmware_chipsize/128K
  uint8	datagfxromaddr[2];	//016h 2    part5 data/gfx romaddr/8 (LZ/huffman compression)
  uint8	key1encrypted[8];//018h 8    Optional KEY1-encrypted "enPngOFF"=Cartridge KEY2 Disable (feature isn't used in any consoles, instead contains timestamp)
  uint8	firmwareversiontimestamp[5];	//018h 5    Firmware version built timestamp (BCD minute,hour,day,month,year)
  uint8	consoletype;	//01Dh 1    Console type
  //            FFh=Nintendo DS
  //            20h=Nintendo DS-lite
  //            57h=Nintendo DSi
  //            43h=iQueDS
  //            63h=iQueDS-lite
  //          The entry was unused (FFh) in older NDS, ie. replace FFh by 00h)
  //            Bit0   seems to be DSi/iQue related
  //            Bit1   seems to be DSi/iQue related
  //            Bit2   seems to be DSi related
  //            Bit3   zero
  //            Bit4   seems to be DSi related
  //            Bit5   seems to be DS-Lite related
  //            Bit6   indicates presence of "extended" user settings (DSi/iQue)
  //            Bit7   zero
  uint8	unk1[2];	//01Eh 2    Unused (FFh-filled)
  uint8	usersettings_offset[2];	//020h 2    User Settings Offset (div8) (usually last 200h flash bytes)
  uint8 unk2[2];	//022h 2    Unknown (7EC0h or 0B51h)
  uint8	unk3[2];	//024h 2    Unknown (7E40h or 0DB3h)
  uint8	crc16datagfx[2];	//026h 2    part5 CRC16 data/gfx
  uint8	unk4[2];	//028h 2    unused (FFh-filled)
  //02Ah-1FFh Wifi Calibration Data ignored
  uint8	stub[206+256];
  
};	//must match DS_FW_HEADER_SIZE

#define DS_FW_HEADER_ADDRESS (uint32)(0x00000000)
#define DS_FW_HEADER_SIZE (sint32)(sizeof(struct sDSFWHEADER))

//DS Firmware Wifi Internet Access Points

typedef struct sDSFWInternetAP {
//  Addr Siz Expl.
uint8	unk1[64];	//  000h 64  Unknown (usually 00h-filled) (no Proxy supported on NDS)
uint8	ssid_ascii[32];	//  040h 32  SSID (ASCII name of the access point) (padded with 00h's)
uint8	ssid_aoss[32];	//  060h 32  SSID for WEP64 on AOSS router (each security level has its own SSID)
uint8	wepkey1[16];	//  080h 16  WEP Key 1 (for type/size, see entry E6h)
uint8	wepkey2[16];	//  090h 16  WEP Key 2  ;
uint8	wepkey3[16];	//  0A0h 16  WEP Key 3  ; (usually 00h-filled)
uint8	wepkey4[16];	//  0B0h 16  WEP Key 4  ;
uint8	ip_address[4];	//  0C0h 4   IP Address           (0=Auto/DHCP)
uint8	gateway[4];		//  0C4h 4   Gateway              (0=Auto/DHCP)
uint8	primary_dns_server[4];	//  0C8h 4   Primary DNS Server   (0=Auto/DHCP)
uint8	secondary_dns_server[4];	//  0CCh 4   Secondary DNS Server (0=Auto/DHCP)
uint8	subnet_mask;	//  0D0h 1   Subnet Mask (0=Auto/DHCP, 1..1Ch=Leading Ones) (eg. 6 = FC.00.00.00)
uint8	unk2[0x15];	//  0D1h ..  Unknown (usually 00h-filled)
uint8	wep_mode;	//  0E6h 1   WEP Mode (0=None, 1/2/3=5/13/16 byte hex, 5/6/7=5/13/16 byte ascii)
uint8	status;	//  0E7h 1   Status (00h=Normal, 01h=AOSS, FFh=connection not configured/deleted)
uint8	unk3;	//  0E8h 1   Zero (not SSID Length, ie. unlike as entry 4,5,6 on DSi)
uint8	unk4;	//  0E9h 1   Unknown (usually 00h)
uint8	dsiMTU[2];	//  0EAh 2   DSi only: MTU (Max transmission unit) (576..1500, usually 1400)
uint8	unk5[3];	//  0ECh 3   Unknown (usually 00h-filled)
uint8	connection_configured;	//  0EFh 1   bit0/1/2 - connection 1/2/3 (1=Configured, 0=Not configured)
uint8	NWC_USERID[6];	//  0F0h 6   Nintendo Wifi Connection (WFC) 43bit User ID  (ID=([F0h] AND 07FFFFFFFFFFFFh)*1000, shown as decimal string NNNN-NNNN-NNNN-N000) (the upper 5bit of the last byte are containing additional/unknown nonzero data)
uint8	unk6[8];	//  0F6h 8   Unknown (nonzero stuff !?!)
uint8	crc16WinternenAP[2];	//  0FEh 2   CRC16 for Entries 00h..FDh (with initial value 0000h) 

}tDSFWInternetAP;



/* User Settings */

/*
	http://problemkaputt.de/gbatek.htm
  
  DS Firmware User Settings

Current Settings (RAM 27FFC80h-27FFCEFh)
User Settings 0 (Firmware 3FE00h-3FEFFh) ;(DSi & iQue use different address,
User Settings 1 (Firmware 3FF00h-3FFFFh) ;see Firmware Header [020h])  
*/  

#define DS_FW_USERSETTINGS_SIZE 0x100
#define DS_FW_USERSETTINGS0ADDR (uint32)(0x0003FE00)
#define DS_FW_USERSETTINGS1ADDR (uint32)(0x0003FF00)

struct sDSFWSETTINGS {
	uint8 Version[2];	//000h  2   Version (5) (Always 5, for all NDS/DSi Firmware versions)
	uint8	color;	//002h  1   Favorite color (0..15) (0=Gray, 1=Brown, etc.)
	uint8	birthday_month;	//003h  1   Birthday month (1..12) (Binary, non-BCD)
	uint8	birthday_day;	//004h  1   Birthday day   (1..31) (Binary, non-BCD)
	uint8	unused1;	//005h  1   Not used (zero)
	uint8	nickname_utf16[20];	//006h  20  Nickname string in UTF-16 format
	uint8	nickname_length_chars[2];	//01Ah  2   Nickname length in characters    (0..10)	-> to int (signed 32 bit): int nicknameLength = (int)(TGDSIPC->DSFWSETTINGSInst.nickname_length_chars[0] | TGDSIPC->DSFWSETTINGSInst.nickname_length_chars[1] << 8);
 	uint8	message_utf16[52];	//01Ch  52  Message string in UTF-16 format
	uint8	message_length_chars[2];	//050h  2   Message length in characters     (0..26)
	uint8	alarm_hour;	//052h  1   Alarm hour     (0..23) (Binary, non-BCD)
	uint8	alarm_minute;	//053h  1   Alarm minute   (0..59) (Binary, non-BCD)
	uint8	unk1[2];	//054h  2
	uint8	enable_alarm;	//056h  1   80h=enable alarm (huh?), bit 0..6=enable?
	uint8	unk2;	//057h  1   Zero (1 byte)
	uint8	tsc_adcposx1y112bit[4];	//058h  2x2 Touch-screen calibration point (adc.x1,y1) 12bit ADC-position
	uint8	tsc_tsccalx1y18bit[2];	//05Ch  2x1 Touch-screen calibration point (scr.x1,y1) 8bit pixel-position
	uint8	tsc_adcposx2y212bit[4];	//05Eh  2x2 Touch-screen calibration point (adc.x2,y2) 12bit ADC-position
	uint8	tsc_tsccalx2y28bit[2];	//062h  2x1 Touch-screen calibration point (scr.x2,y2) 8bit pixel-position
	uint8	fw_language[2];	//064h  2   Language and Flags (see below)
	uint8	fw_year;	//066h  1   Year (2000..2255) (when having entered date in the boot menu)
	uint8	unk3;	//067h  1   Unknown (usually 00h...08h or 78h..7Fh or so)
	uint8	rtc_offset[4];	//068h  4   RTC Offset (difference in seconds when RTC time/date was changed)
	uint8	unk4[4];	//06Ch  4   Not used (FFh-filled, sometimes 00h-filled) (=MSBs of above?)
	//Below not stored in RAM (found only in FLASH memory)...
	uint8	updatecounter[2];	//070h  2   update counter (used to check latest) (must be 0000h..007Fh)
	uint8	crc16fwsetting[2];	//072h  2   CRC16 of entries 00h..6Fh (70h bytes)
	uint8	unk5[0x8c];	//074h  8Ch Not used (FFh-filled) (or extended data, see below)
	//Below extended data was invented for iQue DS (for adding the chinese language setting), and is also included in Nintendo DSi models. Presence of extended data is indicated in Firmware Header entry [1Dh].Bit6.
	//074h  1   Unknown (01h) (maybe version?)
	//075h  1   Extended Language (0..5=Same as Entry 064h, plus 6=Chinese)
    //        (for language 6, entry 064h defaults to english; for compatibility)
    //        (for language 0..5, both entries 064h and 075h have same value)
	//076h  2   Bitmask for Supported Languages (Bit0..6)
    //        (007Eh for iQue DS, ie. with chinese, but without japanese)
    //        (003Eh for DSi/EUR, ie. without chinese, and without japanese)
	//078h  86h Not used (FFh-filled on iQue DS, 00h-filled on DSi)
	//0FEh  2   CRC16 of entries 74h..FDh (8Ah bytes)
	
};

//language flags
/*
	Language and Flags (Entry 064h)
	Bit
	0..2 Language (0=Japanese, 1=English, 2=French, 3=German,
		4=Italian, 5=Spanish, 6..7=Reserved) (for Chinese see Entry 075h)
		(the language setting also implies time/data format)
	3    GBA mode screen selection (0=Upper, 1=Lower)
	4-5  Backlight Level    (0..3=Low,Med,High,Max) (DS-Lite only)
	6    Bootmenu Disable   (0=Manual/bootmenu, 1=Autostart Cartridge)
	9    Settings Lost (1=Prompt for User Info, and Language, and Calibration)
	10   Settings Okay (0=Prompt for User Info)
	11   Settings Okay (0=Prompt for User Info) (Same as Bit10)
	12   No function
	13   Settings Okay (0=Prompt for User Info, and Language)
	14   Settings Okay (0=Prompt for User Info) (Same as Bit10)
	15   Settings Okay (0=Prompt for User Info) (Same as Bit10)
	The Health and Safety message is skipped if Bit9=1, or if one or more of the following bits is zero: Bits 10,11,13,14,15. However, as soon as entering the bootmenu, the Penalty-Prompt occurs.
*/

#define language_mask (0x7)

#endif

#ifdef __cplusplus
extern "C"{
#endif

#ifdef ARM7
extern void LoadFirmwareSettingsFromFlash();
#endif

extern void ParseFWSettings(uint32 usersetting_offset);

#ifdef __cplusplus
}
#endif
