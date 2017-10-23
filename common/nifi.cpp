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

#ifdef ARM9

//DSWNIFI Library revision: 1.1
#include "nifi.h"

#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefs.h"

#include "specific_shared.h"
#include "wifi_arm9.h"
#include "dswnifi.h"
#include "dswifi9.h"
#include "wifi_shared.h"
#include "wifi_arm9.h"
#include "toolchain_utils.h"

#include "http_utils.h"
#include "socket.h"
#include "in.h"
#include <netdb.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "InterruptsARMCores_h.h"
#include "common_shared.h"
#include "bios.h"
#include "timer.h"


#ifdef WIFI_USE_TCP_SGIP
#include "sgIP.h"
#endif

//nifi_stat: 0 not ready, 1 act as a host and waiting, 2 act as a guest and waiting, 3 connecting, 4 connected, 5 host ready, 6 guest ready

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



//frames
//
//Read-Only
volatile	const uint8 nifitoken[32]		= {0xB2, 0xD1, 'n', 'i', 'f', 'i', 'd', 's'};
volatile 	const uint8 nificonnect[32]	= {0xB2, 0xD1, 'c', 'o', 'n', 'n', 'e', 'c', 't'};

//Read-Write
volatile 	uint8 nificrc[32]				= {0xB2, 0xD1, (uint8)CRC_CRC_STAGE, 0, 0, 0};
volatile 	uint8 data[4096];			//receiver frame, data + frameheader is recv TX'd frame nfdata[128]. Used by NIFI Mode
volatile 	uint8 nfdata[128]			= {0xB2, 0xD1, (uint8)CRC_OK_SAYS_HOST, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};	//sender frame, recv as data[4096], see above. all valid frames have CRC_OK_SAYS_HOST sent.


// datalen = size of packet from beginning of 802.11 header to end, but not including CRC.
int Wifi_RawTxFrame_NIFI(uint16 datalen, uint16 rate, uint16 * data) {
	int base,framelen, hdrlen, writelen;
	int copytotal, copyexpect;
	
	uint16 framehdr[arm9_header_framesize + 2];	//+2 for arm7 header section
	framelen=datalen + 8 + (WifiData->wepmode7 ? 4 : 0);

	if(framelen + 40>Wifi_TxBufferWordsAvailable()*2) { // error, can't send this much!
		SGIP_DEBUG_MESSAGE(("Transmit:err_space"));
		return -1; //?
	}

	framehdr[0]=0;
	framehdr[1]=0;
	framehdr[2]=0;
	framehdr[3]=0;
	framehdr[4]=0; // rate, will be filled in by the arm7.
	hdrlen= arm9_header_framesize; //18;
	framehdr[6]=0x0208;
	framehdr[7]=0;

	// MACs.
	memset((uint8*)(framehdr + 8), 0xFF, 18);

	if(WifiData->wepmode7)
	{
		framehdr[6] |=0x4000;
		hdrlen=20;
	}
	framehdr[17] = 0;
	framehdr[18] = 0; // wep IV, will be filled in if needed on the arm7 side.
	framehdr[19] = 0;

	framehdr[5]=framelen+hdrlen * 2 - 12 + 4;
	copyexpect= ((framelen+hdrlen * 2 - 12 + 4) + 12 - 4 + 1)/2;
	copytotal=0;

	WifiData->stats[WSTAT_TXQUEUEDPACKETS]++;
	WifiData->stats[WSTAT_TXQUEUEDBYTES] += framelen + hdrlen * 2;

	base = WifiData->txbufOut;
	Wifi_TxBufferWrite(base,hdrlen,framehdr);
	base += hdrlen;
	copytotal += hdrlen;
	if(base >= (WIFI_TXBUFFER_SIZE / 2)) base -= WIFI_TXBUFFER_SIZE / 2;

	// add LLC header
	framehdr[0]=0xAAAA;
	framehdr[1]=0x0003;
	framehdr[2]=0x0000;
	unsigned short protocol = 0x08FE;
	framehdr[3] = ((protocol >> 8) & 0xFF) | ((protocol << 8) & 0xFF00);

	Wifi_TxBufferWrite(base, 4, framehdr);
	base += 4;
	copytotal += 4;
	if(base>=(WIFI_TXBUFFER_SIZE/2)) base -= WIFI_TXBUFFER_SIZE/2;

	writelen = datalen;
	if(writelen) {
		Wifi_TxBufferWrite(base,(writelen+1)/2,data);
		base += (writelen + 1) / 2;
		copytotal += (writelen + 1) / 2;
		if(base>=(WIFI_TXBUFFER_SIZE/2)) base -= WIFI_TXBUFFER_SIZE/2;
	}
	if(WifiData->wepmode7)
	{ // add required extra bytes
		base += 2;
		copytotal += 2;
		if(base >= (WIFI_TXBUFFER_SIZE / 2)) base -= WIFI_TXBUFFER_SIZE / 2;
	}
	WifiData->txbufOut = base; // update fifo out pos, done sending packet.

	if(copytotal!=copyexpect)
	{
		SGIP_DEBUG_MESSAGE(("Tx exp:%i que:%i",copyexpect,copytotal));
	}
	if(synchandler) {
		synchandler();
	}
	return 0;
}

//coto: wifi udp netplay code (:
// datalen = size of packet from beginning of 802.11 header to end, but not including CRC.

int Wifi_RawTxFrame_WIFI(uint8 datalen, uint8 * data) {	
	
	//sender phase
	{
		
		switch(dswifiSrv.dsnwifisrv_stat){
		
			//#1 DS is not connected, wait until server acknowledges this info
			case(ds_searching_for_multi_servernotaware):{		
				//NDS MAC Address
				//volatile uint8 macbuf[6];
				//Wifi_GetData(WIFIGETDATA_MACADDRESS, sizeof(macbuf), (uint8*)macbuf);
				
				volatile unsigned long available_ds;
				ioctl(client_http_handler_context.socket_id__multi_notconnected, FIONREAD, (uint8*)&available_ds);
				
				if(available_ds == 0){
					char outgoingbuf[256];
					sprintf(outgoingbuf,"dsnotaware-NIFINintendoDS-%s-",(char*)print_ip((uint32)Wifi_GetIP()));	//DS udp ports on server inherit the logic from "//Server aware" handler
					sendto(client_http_handler_context.socket_id__multi_notconnected,outgoingbuf,strlen(outgoingbuf),0,(struct sockaddr *)&client_http_handler_context.server_addr,sizeof(client_http_handler_context.server_addr));
				}
			}
			break;
			
			
			//servercheck phase. DS's are binded each other. safe to send data between DSes
			case(ds_netplay_host_servercheck):case(ds_netplay_guest_servercheck):{
				
				volatile unsigned long available_ds;
				ioctl(client_http_handler_context.socket_id__multi_notconnected, FIONREAD, (uint8*)&available_ds);
				
				if(available_ds == 0){
					//check pending receive
					int LISTENER_PORT 	=	0;
					int SENDER_PORT		=	0;
					char status[10];
					if(dswifiSrv.dsnwifisrv_stat == ds_netplay_host_servercheck){
						LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_HOST;
						SENDER_PORT		=	(int)NDSMULTI_UDP_PORT_GUEST;
						sprintf(status,"host");
					}
					else if(dswifiSrv.dsnwifisrv_stat == ds_netplay_guest_servercheck){
						LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_GUEST;
						SENDER_PORT		=	(int)NDSMULTI_UDP_PORT_HOST;
						sprintf(status,"guest");
					}
					
					char buf2[64];
					sprintf(buf2,"dsaware-%s-bindOK-%d-%s-",status,LISTENER_PORT,(char*)print_ip((uint32)Wifi_GetIP()));
					//consoletext(64*2-32,(char *)&buf2[0],0);
					sendto(client_http_handler_context.socket_id__multi_notconnected,buf2,sizeof(buf2),0,(struct sockaddr *)&client_http_handler_context.server_addr,sizeof(client_http_handler_context.server_addr));
					
				}
			}
			break;
					
			
			//#last:connected!
			case(ds_netplay_host):case(ds_netplay_guest):{
				//send NIFI Frame here
				//int i=1;
				//i=ioctl(client_http_handler_context.socket_multi_sender,FIONBIO,&i); // set non-blocking port
				//data[0] = (uint8)(rand()&0xff);
				sendto(client_http_handler_context.socket_multi_sender,data,datalen,0,(struct sockaddr *)&client_http_handler_context.sain_sender,sizeof(client_http_handler_context.sain_sender));
			}
			break;
		}
		
	}
	return 0;
}

//coto: just for nifi mode (not wifi)
//true 	== 	nifi frame
//false ==	other frame
bool NiFiHandler(int packetID, int readlength, uint8 * data){
	
	bool valid_nifi_frame = false;
	
	//#1: nifi-handshake  
	//accept a valid NIFI frame (no crc nifi frame will be used)
	if((data[frame_header_size + 0] == nifitoken[0]) && (data[frame_header_size + 1] == nifitoken[1])){
	
		switch(nifi_stat) {
			case 0:{
				valid_nifi_frame = false;
				return valid_nifi_frame;
			}
			break;
			case 1:			
				if(strncmp((const char *)(data + frame_header_size + 2), (const char *)(nifitoken + 2), 6) == 0) {	//token "nifids"
					valid_nifi_frame = true;
					nifi_stat = 3;	
				}
				break;
			case 2:			
				if(strncmp((const char *)(data + frame_header_size + 2), (const char *)(nificonnect + 2), 7) == 0) {	//token "connect"
					valid_nifi_frame = true;
					nifi_stat = 4;
				}
				break;
			case 3:
				if(data[frame_header_size + 2] == CRC_CRC_STAGE) {		//Check the CRC (cmd from MP). Make sure that both players are using the same game.
					/*
					int remotecrc = (data[frame_header_size + 3] | (data[frame_header_size + 4] << 8));
					if(debuginfo[17] == remotecrc) {	//ok. same game
						valid_nifi_frame = true;
						nifi_stat = 5;
						nifi_cmd |= MP_CONN;
						sendcmd((uint8*)&nfdata[0]);
						hideconsole();
						NES_reset();
						nifi_keys = 0;
						plykeys1 = 0;
						plykeys2 = 0;
						guest_framecount = 0;
						global_playcount = 0;
						joyflags &= ~AUTOFIRE;
						__af_st = __af_start;
						menu_game_reset();	//menu is closed.
					}
					else {		//bad crc. disconnect the comm.
						nifi_stat = 0;
						nifi_cmd &= ~MP_CONN;
						sendcmd((uint8*)&nfdata[0]);
					}
					*/
				}
				break;
			case 4:
				if(data[frame_header_size + 2] == CRC_OK_SAYS_HOST) {
					receiveDSWNIFIFrame(data,readlength);
					/*
					if(nifi_cmd & MP_CONN) {	//CRC ok, get ready for multi-play.
						valid_nifi_frame = true;
						nifi_stat = 6;
						hideconsole();
						NES_reset();
						nifi_keys = 0;
						plykeys1 = 0;
						plykeys2 = 0;
						guest_framecount = 0;
						global_playcount = 0;
						joyflags &= ~AUTOFIRE;
						__af_st = __af_start;
						menu_game_reset();	//menu is closed.
					}
					else {					//CRC error, the both sides should choose the some game.
						nifi_stat = 0;
					}
					*/
				}
				break;
		}
	}
	//discard all non-nifi frames
	else{
		return valid_nifi_frame;
	}
	
	//#2: crc nifi-frame
	if((nifi_stat == 5) || (nifi_stat == 6)){ //validate frames only if we are past the nifi handshake
		
		int framesize = 0;
		
		if(nifi_stat == 5) {	//host receives from guest
			framesize = 3 + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount) + sizeof(nifi_keys_sync) + sizeof(guest_framecount);
		} 
		else if(nifi_stat == 6){//guest receives from host
			framesize = 3 + sizeof(nifi_cmd) + sizeof(host_vcount) + sizeof(plykeys1) + sizeof(host_framecount);
		}
		
		//coto:
		//read crc from nifi frame so we dont end up with corrupted data.(read from caller buffer, aligned)
		volatile uint16 crc16_recv_frame = (uint16)*(uint16*)(data + frame_header_size + framesize);
		
		//(read from handler buffer directly, unaligned)
		//(uint16)(*(uint8*)(data + frame_header_size + framesize) | (*(uint8*)(data + frame_header_size + framesize + 1)<<8));
		
		//generate crc per nifi frame so we dont end up with corrupted data.
		volatile uint16 crc16_frame_gen = swiCRC16	(	0xffff, //uint16 	crc,
			(uint8*)(data + frame_header_size),
			(framesize)		//cant have this own crc here
			);
		
		
		if(crc16_frame_gen == crc16_recv_frame){		//works stable
			valid_nifi_frame = true;
			receiveDSWNIFIFrame(data,readlength);
			//discard frame used contents. Prevent likely frames
			memset(data ,0,frame_header_size + framesize);
		}
		else{
			//NIFI FRAME CRC CORRUPTED
		}
		
	}
	
	return valid_nifi_frame;
}


void Handler(int packetID, int readlength)
{
	//coto
	switch(getMULTIMode()){
		case (dswifi_localnifimode):{
			Wifi_RxRawReadPacket(packetID, readlength, (unsigned short *)data);
			NiFiHandler(packetID, readlength, (uint8*)(&data[0]));	
		}
		break;
		
		//cant be here, eats too many cycles lagging network services	
		/*
		case (dswifi_udpnifimode):
		case (dswifi_tcpnifimode):
		{
			
		}
		break;
		*/
		
		
	}
}


void Timer_10ms(void) {
	Wifi_Timer(10);
}

void initNiFi()
{
	Wifi_InitDefault(false);
	Wifi_SetPromiscuousMode(1);
	//Wifi_EnableWifi();
	Wifi_RawSetPacketHandler(Handler);
	Wifi_SetChannel(10);

	if(1) {
		//for secial configuration for wifi
		DisableIrq(IRQ_TIMER3);
		//ori:irqSet(IRQ_TIMER3, Timer_10ms); // replace timer IRQ
		//irqSet(IRQ_TIMER3, Timer_50ms); // replace timer IRQ
		//ori: TIMER3_DATA = -(6553 / 5); // 6553.1 * 256 / 5 cycles = ~10ms;
		TIMERXDATA(3) = -6553; // 6553.1 * 256 cycles = ~50ms;
		TIMERXCNT(3) = 0x00C2; // enable, irq, 1/256 clock
		EnableIrq(IRQ_TIMER3);
	}
}


#endif