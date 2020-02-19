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


//DSWNIFI Library revision: 1.2
#ifndef __dswnifi_h__
#define __dswnifi_h__

#define NIFI_FRAME_EXT (uint32)(0xffff1010)

struct snemulDSNIFIUserMsg{
	int keys;	//plykeys1 / plykeys2
	int Joy_cnt;	//Joy1_cnt / Joy2_cnt
	int DS_VCOUNT;	//REG_VCOUNT
	bool host;	//if host == true, snes vcount syncline is from host (if guest). If host then snes vcount syncline generates here
	int ExtSnesFrameCount;
	uint32 cmdIssued;
	u8 DMA_PORT_EXT; 
	struct tm DSEXTTime;
};

extern bool nifiHost;	//true = yes, false = no
extern bool nifiSetup;

#endif


#ifdef __cplusplus
extern "C"{
#endif

#ifdef ARM9

//DSWNIFI: message for nifi beacons
extern volatile const uint8 nifitoken[32];
extern volatile const uint8 nificonnect[32];
extern volatile uint8 nificrc[32];

//DSWNIFI: WIFI specific
extern int nifi_stat;
extern int nifi_cmd;
extern int nifi_keys;		//holds the keys for players.
extern int nifi_keys_sync;	//(guestnifikeys & hostnifikeys)

extern int plykeys1;		//player1
extern int plykeys2;		//player2
extern int guest_framecount;	//used by the guest for syncing.
extern int host_framecount;		//emulator framecount:host
extern int guest_framecount;	//emulator framecount:guest
extern int host_vcount;		//host generated REG_VCOUNT
extern int guest_vcount;		//guest generated REG_VCOUNT


#endif

//Example Sender Code
//Send This DS Time to External DS through UDP NIFI or Local NIFI:
//volatile uint8 somebuf[128];
//sprintf((char*)somebuf,"DSTime:%d:%d:%d",getTime()->tm_hour,getTime()->tm_min,getTime()->tm_sec);
//if(!FrameSenderUser){
//				FrameSenderUser = HandleSendUserspace((uint8*)somebuf,sizeof(somebuf));	
//}

extern bool SendRawEmuFrame(int keys, int DS_VCOUNT, bool host, int ExtSnesFrameCount, uint32 cmdIssued,u8 DMA_PORT_EXTInst, struct tm DSEXTTime);
extern bool donifi(int DS_VCOUNTER);

extern bool waitforhblank;
#ifdef __cplusplus
}
#endif