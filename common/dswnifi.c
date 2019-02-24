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
#include "ipcfifoTGDSUser.h"
#include "wifi_shared.h"
#include "clockTGDS.h"
#include "ipcfifoTGDS.h"

#ifdef ARM9
#include "dswnifi_lib.h"

#include "dswnifi.h"
#include "wifi_arm9.h"
#include "dswifi9.h"
#include "wifi_shared.h"
#include "utilsTGDS.h"
#include <netdb.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <socket.h>
#include <in.h>
#include <assert.h>
#include <string.h>

//emu specific
#include "cfg.h"
#include "gfx.h"
#include "core.h"
#include "snes.h"
#include "engine.h"
#include "apu.h"
#include "ppu.h"
#include "main.h"
#include "opcodes.h"
#include "conf.h"
#include "fs.h"
#include "memmap.h"
#include "crc32.h"
#include "guiTGDS.h"
#include "apu_jukebox.h"

#include "dswnifi.h"
#include "dswnifi_lib.h"

#endif

#ifdef ARM9

int nifi_stat = 0;	//start as idle always
int nifi_cmd = 0;
int nifi_keys = 0;		//holds the keys for players. player1 included
int nifi_keys_sync;	//(guestnifikeys & hostnifikeys)
int plykeys1 = 0;		//player1
int plykeys2 = 0;		//player2
int host_vcount = 0;		//host generated REG_VCOUNT
int guest_vcount = 0;		//guest generated REG_VCOUNT
int host_framecount = 0;
int guest_framecount = 0;


//These methods are template you must override (as defined below), to have an easy DS - DS framework running.

//Example Sender Code
//Send This DS Time to External DS through UDP NIFI or Local NIFI:
//volatile uint8 somebuf[128];
//sprintf((char*)somebuf,"DSTime:%d:%d:%d",getTime()->tm_hour,getTime()->tm_min,getTime()->tm_sec);
//if(!FrameSenderUser){
//				FrameSenderUser = HandleSendUserspace((uint8*)somebuf,sizeof(somebuf));	//make room for crc16 frame
//}

//Example Receiver Code
__attribute__((section(".itcm")))
void HandleRecvUserspace(struct frameBlock * frameBlockRecv){
	//frameBlockRecv->framebuffer	//Pointer to received Frame
	//frameBlockRecv->frameSize		//Size of received Frame
	do_multi(frameBlockRecv);		
}

//Multiplayer key binding/buffer shared code. For DS-DS multiplayer emu core stuff.
__attribute__((section(".itcm")))
bool do_multi(struct frameBlock * frameBlockRecv)
{
	//frameBlockRecv->framebuffer	//Pointer to received Frame
	//frameBlockRecv->frameSize		//Size of received Frame
	
	switch(getMULTIMode()){
		
		//single player, has no access to shared buffers.
		case(dswifi_idlemode):{
			plykeys1 = get_joypad() | 0x80000000;	//bit15 (second half word) is for ready bit, required by emu		
			//joypad1 ready state? start reading from 15th bit backwards.
			if (DMA_PORT[0x00]&1)
			{
				SNES.Joy1_cnt = 16;	//ok send acknowledge bit
				write_joypad1((plykeys1&0xffff)); //only use actual joypad bits
			}
		
			//write joy2
			if (CFG.mouse){
				read_mouse();
			}
			if (CFG.scope){
				read_scope();
			}
			return false;
		}
		break;
		
		//NIFI local
		case(dswifi_localnifimode):{
			struct snemulDSNIFIUserMsg * thissnemulDSNIFIUserMsgReceiver = (struct snemulDSNIFIUserMsg *)frameBlockRecv->framebuffer;
			///////////////////////////////////// emu layer receiver
			if(nifiSetup == true){	
				switch(thissnemulDSNIFIUserMsgReceiver->cmdIssued){
					//actual multiplayer NIFI frame
					case(NIFI_FRAME_EXT):{
						//host/guest logic depending on frame received
						//[host -> guest sent frame]
						if((thissnemulDSNIFIUserMsgReceiver->host == true) && (nifiHost == false)){
							//GUEST:RECV from HOST: plykeys2 is this DS, plykeys1 is ext DS
							host_vcount = thissnemulDSNIFIUserMsgReceiver->DS_VCOUNT;
							
							//port bits ready? feed input
							if(thissnemulDSNIFIUserMsgReceiver->DMA_PORT_EXT > 0){
								plykeys1 = thissnemulDSNIFIUserMsgReceiver->keys;
								SNES.Joy1_cnt = 16;	//ok send acknowledge bit
								//write_joypad1((plykeys1&0xffff)); //only use actual joypad bits
							}
							else{
								SNES.Joy1_cnt = 0;
							}
							
							if((u8)(DMA_PORT[0x00]&1) > 0){			
								plykeys2 = get_joypad() | 0x80000000;	//bit15 (second half word) is for ready bit, required by emu		
								//joypad2 ready state? start reading from 15th bit backwards.
								SNES.Joy2_cnt = 16;	//ok send acknowledge bit
								//write_joypad2((plykeys2&0xffff)); //only use actual joypad bits
							}
							
						}
						
						//[guest -> host sent frame]
						else if((thissnemulDSNIFIUserMsgReceiver->host == false) && (nifiHost == true)){
							//HOST:RECV from GUEST: plykeys1 is this DS, plykeys2 is ext DS
							guest_vcount = thissnemulDSNIFIUserMsgReceiver->DS_VCOUNT;
							
							if(thissnemulDSNIFIUserMsgReceiver->DMA_PORT_EXT > 0){
								plykeys2 = thissnemulDSNIFIUserMsgReceiver->keys;
								SNES.Joy2_cnt = 16;	//ok send acknowledge bit
								//write_joypad2((plykeys2&0xffff)); //only use actual joypad bits
							}
							else{
								SNES.Joy2_cnt = 0;
							}
							
							if((u8)(DMA_PORT[0x00]&1) > 0){
								plykeys1 = get_joypad() | 0x80000000;	//bit15 (second half word) is for ready bit, required by emu		
								//joypad1 ready state? start reading from 15th bit backwards.
								SNES.Joy1_cnt = 16;	//ok send acknowledge bit
								write_joypad1((plykeys1&0xffff)); //only use actual joypad bits
							}	
						}
						
						switch(getMULTIMode()){
							case(dswifi_idlemode):{
								
							}
							break;
							case(dswifi_localnifimode):{
								//DS HW Sync
								int DSScanline = (REG_VCOUNT&0x1ff);
								if(DSScanline >= 202 && DSScanline < 213){
									if(nifiHost == true){
										//REG_VCOUNT= guest_vcount;	//wait for guest
									}
									else{
										REG_VCOUNT = host_vcount;	//sync to host
									}
								}
							}
							break;
						}
						
						
						guestSNESFrameCount = thissnemulDSNIFIUserMsgReceiver->ExtSnesFrameCount;
					}
					break;
				}
			}
			return true;
		}
		break;
		
		//UDP NIFI
		case(dswifi_udpnifimode):{
			//todo
			return true;
		}
		break;
	
	}
	
	return false;
}

//host/guest decider logic: based on timestamp the oldest time is guest
__attribute__((section(".dtcm")))
bool nifiHost = false;	//true = yes, false = no

__attribute__((section(".dtcm")))
bool nifiSetup = false; //true = yes, false = no

__attribute__((section(".itcm")))
bool SendRawEmuFrame(int keys, int DS_VCOUNT, bool host, int ExtSnesFrameCount, uint32 cmdIssued, u8 DMA_PORT_EXTInst, struct tm DSEXTTime){
	if (getMULTIMode() == dswifi_localnifimode){
		struct snemulDSNIFIUserMsg snemulDSNIFIUserMsgInst;
		snemulDSNIFIUserMsgInst.keys = keys;
		snemulDSNIFIUserMsgInst.DS_VCOUNT = DS_VCOUNT;
		snemulDSNIFIUserMsgInst.host = host;
		snemulDSNIFIUserMsgInst.ExtSnesFrameCount = ExtSnesFrameCount;
		snemulDSNIFIUserMsgInst.cmdIssued = cmdIssued;
		snemulDSNIFIUserMsgInst.DMA_PORT_EXT = DMA_PORT_EXTInst;
		snemulDSNIFIUserMsgInst.DSEXTTime = DSEXTTime;
		int curFrameSize = sizeof(struct snemulDSNIFIUserMsg);
		if(curFrameSize <= frameDSsize){	//use frameDSsize (or less) as the sender buffer size, any other size won't be sent.
			FrameSenderUser = HandleSendUserspace((uint8*)&snemulDSNIFIUserMsgInst, curFrameSize);
			return true;
		}
	}
	return false;
}

//runs in loop. Must be in vblank. (hblank is too slow since we need each snes vcount line to be sync). So a multiplayer is 1 frame render based
//true == nifi running
//false == nifi not running

__attribute__((section(".itcm")))
bool donifi(int DS_VCOUNTER){
	if (getMULTIMode() == dswifi_localnifimode){
		///////////////////////////////////// emu layer sender
		//play/update code
		
		//host logic: plykeys1 is this DS, plykeys2 is ext DS
		if(nifiHost == true){
			SendRawEmuFrame(plykeys1, DS_VCOUNTER, nifiHost, ThisSNESFrameCount, NIFI_FRAME_EXT, (u8)(DMA_PORT[0x00]&1), *getTime());
		}
		//guest logic: plykeys2 is this DS, plykeys1 is ext DS
		else{
			SendRawEmuFrame(plykeys2, DS_VCOUNTER, nifiHost, ThisSNESFrameCount, NIFI_FRAME_EXT, (u8)(DMA_PORT[0x00]&1), *getTime());
		}
		return true;
	}
	return false;
}

//DSWNIFI callbacks. These run only once per DSWNIFI set mode
void OnDSWIFIlocalnifiEnable(){

}

void OnDSWIFIidlemodeEnable(){
	nifiHost = false;
	nifiSetup = false;
	guest_vcount = 0;
	host_vcount = 0;
	
	
	SNES.Joy1_cnt = 0;
	SNES.Joy2_cnt = 0;
	
	plykeys1 = 0;
	plykeys2 = 0;
	
	SNES.V_Count = 0;
}

void OnDSWIFIudpnifiEnable(){

}

void OnDSWIFIGDBStubEnable(){

}

#endif