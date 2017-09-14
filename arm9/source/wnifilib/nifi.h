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

//DSWNIFI Library revision: 1.1

#ifndef nifi_dswifi_extension
#define nifi_dswifi_extension

//NIFI
#define arm7_header_framesize 	(12 + 2)									//arm7 
#define arm9_header_framesize	(12 + 6)
#define frame_header_size 		(arm9_header_framesize + arm7_header_framesize)	

#define CRC_CRC_STAGE 		0x81
#define CRC_OK_SAYS_HOST	0x88
//coto
//WIFI UDP
#define UDP_PORT 8888				//used for UDP Server - NDS Companion connecting
//used for UDP transfers between NDS Multi mode
#define NDSMULTI_UDP_PORT_HOST 8889			//host 	listener - listener is local - sender is multi IP NDS
#define NDSMULTI_UDP_PORT_GUEST 8890		//guest listener - 


#include "typedefs.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#endif

#ifdef __cplusplus
extern "C"{
#endif

//DSWNIFI: NIFI
extern void Handler(int packetID, int readlength);
extern int Wifi_RawTxFrame_NIFI(uint16 datalen, uint16 rate, uint16 * data);
extern bool NiFiHandler(int packetID, int readlength, uint8 * data);
extern void initNiFi();
extern void Timer_10ms(void);
extern void initNiFi();

//DSWNIFI: nifi buffer IO
extern volatile uint8	 data[4096];		//data[32] + is recv TX'd frame nfdata[128]
extern volatile uint8	 nfdata[128];	//sender frame, recv as data[4096]

//DSWNIFI: message for nifi beacons
extern volatile const uint8 nifitoken[32];
extern volatile const uint8 nificonnect[32];
extern volatile uint8 nificrc[32];

//DSWNIFI: WIFI specific
extern int Wifi_RawTxFrame_WIFI(uint8 datalen, uint8 * data);
extern int nifi_stat;
extern int nifi_cmd;
extern int nifi_keys;		//holds the keys for players.
extern int nifi_keys_sync;	//(guestnifikeys & hostnifikeys)

#ifdef __cplusplus
}
#endif
