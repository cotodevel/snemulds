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

//nifi cmds
#define HOSTCMD_FROM_EXT_DS_IDLE (uint32)(0xffff1010)
#define HOSTCMD_FROM_EXT_DS_UPDATESTEP1 (uint32)(0xffff1011)	//host -> client : guest changes
#define HOSTCMD_FROM_EXT_DS_UPDATESTEP2 (uint32)(0xffff1012)	//host -> client-> host: acknowledge guest changes to host
#define HOSTCMD_FROM_EXT_DS_RESETNIFI (uint32)(0xffff1013)		//host -> client reset nifi

//nifi status
#define NIFI_SETUP (int)(0)
#define NIFI_PLAY (int)(1)

struct snemulDSNIFIUserMsg{
	int keys;	
	int DS_VCOUNT;	//REG_VCOUNT
	bool host;	//if host == true, snes vcount syncline is from host (if guest). If host then snes vcount syncline generates here
	int SNES_VCOUNT;
	uint32 cmdIssued;
	struct tm DSEXTTime;
};

extern bool nifiHost;	//true = yes, false = no
extern bool nifiSetup;

#endif


#ifdef __cplusplus
extern "C"{
#endif

//Example Sender Code
//Send This DS Time to External DS through UDP NIFI or Local NIFI:
//volatile uint8 somebuf[128];
//sprintf((char*)somebuf,"DSTime:%d:%d:%d",getTime()->tm_hour,getTime()->tm_min,getTime()->tm_sec);
//if(!FrameSenderUser){
//				FrameSenderUser = HandleSendUserspace((uint8*)somebuf,sizeof(somebuf));	//make room for crc16 frame
//}

extern struct snemulDSNIFIUserMsg thissnemulDSNIFIUserMsg;
extern struct snemulDSNIFIUserMsg * forgeNIFIMsg(int keys, int DS_VCOUNT, bool host, int SNES_VCOUNT, uint32 cmdIssued,struct tm DSEXTTime);
extern void issueISHOSTCmd();
extern void issueISHOSTACKCmd(bool hostorGuest);
extern void resetNifi();
extern void donifi();

#ifdef __cplusplus
}
#endif