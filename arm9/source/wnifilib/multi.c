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
//project specific extension due to how joypad maps to emulator core

#include "nifi.h"

#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefs.h"


#include "specific_shared.h"
#include "wifi_arm9.h"
#include "multi.h"
#include "dswifi9.h"
#include "wifi_shared.h"
#include "toolchain_utils.h"
#include "main.h"

#include "client_http_handler.h"
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
#include "dswnifi.h"
#include "bios.h"

//emu specific
#include <string.h>
#include <stdlib.h>
#include "cfg.h"
#include "gfx.h"
#include "core.h"
#include "engine.h"
#include "apu.h"
#include "ppu.h"
#include "main.h"
#include "opcodes.h"
#include "conf.h"
#include "fs.h"
#include "memmap.h"
#include "crc32.h"
#include "gui.h"
#include "apu_jukebox.h"
#include "common_shared.h"

int plykeys1 = 0;		//player1
int plykeys2 = 0;		//player2

int host_vcount = 0;		//host generated REG_VCOUNT
int guest_vcount = 0;		//guest generated REG_VCOUNT

int host_framecount = 0;
int guest_framecount = 0;


//memcpy ( void * destination, const void * source, size_t num );
void sendcmd(uint8 * databuf_src)
{
	//0-1-2 ID:
	memcpy((uint8*)(databuf_src + 3), (uint8*)&nifi_cmd, sizeof(nifi_cmd));
	
	int framesize = 0;
	
	if(SpecificIPC->dswifiSrv.dsnwifisrv_mode == dswifi_nifimode){

		if(nifi_stat == 5) {	//host
			host_vcount = (int)(REG_VCOUNT&0xff);
			
			memcpy((uint8*)(databuf_src + 3 + sizeof(nifi_cmd)), (uint32*)&host_vcount, sizeof(host_vcount));
			
			//new: send nifi_keys overwifi , not compatible with stock nesdsnifi
			memcpy((uint8*)(databuf_src + 3 + sizeof(nifi_cmd) + sizeof(host_vcount)), &plykeys1, sizeof(plykeys1));
			
			//new: send host framecount
			memcpy((uint8*)(databuf_src + 3 + sizeof(nifi_cmd) + sizeof(host_vcount) + sizeof(plykeys1)), &host_framecount, sizeof(host_framecount));
			
			framesize = 3 + sizeof(nifi_cmd) + sizeof(host_vcount) + sizeof(plykeys1) + sizeof(host_framecount);
			
		} 
		else if(nifi_stat == 6){
			memcpy((uint8*)(databuf_src + 3 + sizeof(nifi_cmd)), &plykeys2, sizeof(plykeys2));
			
			//update guest VCOUNT here for host sync
			guest_vcount = (int)(REG_VCOUNT&0xff);
			
			memcpy((uint8*)(databuf_src + 3 + sizeof(nifi_cmd) + sizeof(plykeys2)), (uint32*)&guest_vcount, sizeof(guest_vcount));
			
			memcpy((uint8*)(databuf_src + 3 + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount)), &nifi_keys_sync, sizeof(nifi_keys_sync));
			
			//new: send guest framecount
			memcpy((uint8*)(databuf_src + 3 + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount) + sizeof(nifi_keys_sync)), &guest_framecount, sizeof(guest_framecount));
			
			framesize = 3 + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount) + sizeof(nifi_keys_sync) + sizeof(guest_framecount);
		}
		
	}
	else if(SpecificIPC->dswifiSrv.dsnwifisrv_mode == dswifi_wifimode){
		switch(SpecificIPC->dswifiSrv.dsnwifisrv_stat){
			
			//#last:connected!
			case(ds_netplay_host):{
				host_vcount = (int)(REG_VCOUNT&0xff);
			
				memcpy((uint8*)(databuf_src + 3 + sizeof(nifi_cmd)), (uint32*)&host_vcount, sizeof(host_vcount));
				
				//new: send nifi_keys overwifi , not compatible with stock nesdsnifi
				memcpy((uint8*)(databuf_src + 3 + sizeof(nifi_cmd) + sizeof(host_vcount)), &plykeys1, sizeof(plykeys1));
				
				//new: send host framecount
				memcpy((uint8*)(databuf_src + 3 + sizeof(nifi_cmd) + sizeof(host_vcount) + sizeof(plykeys1)), &host_framecount, sizeof(host_framecount));
				
				framesize = 3 + sizeof(nifi_cmd) + sizeof(host_vcount) + sizeof(plykeys1) + sizeof(host_framecount);
				
			}break;
			
			case(ds_netplay_guest):{
				memcpy((uint8*)(databuf_src + 3 + sizeof(nifi_cmd)), &plykeys2, sizeof(plykeys2));
				
				//update guest VCOUNT here for host sync
				guest_vcount = (int)(REG_VCOUNT&0xff);
				
				memcpy((uint8*)(databuf_src + 3 + sizeof(nifi_cmd) + sizeof(plykeys2)), (uint32*)&guest_vcount, sizeof(guest_vcount));
				
				memcpy((uint8*)(databuf_src + 3 + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount)), &nifi_keys_sync, sizeof(nifi_keys_sync));
				
				//new: send guest framecount
				memcpy((uint8*)(databuf_src + 3 + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount) + sizeof(nifi_keys_sync)), &guest_framecount, sizeof(guest_framecount));
				
				framesize = 3 + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount) + sizeof(nifi_keys_sync) + sizeof(guest_framecount);
			}break;
		}
	}
	
	//coto: generate crc per nifi frame so we dont end up with corrupted data.
	volatile uint16 crc16_frame = swiCRC16	(	0xffff, //uint16 	crc,
		databuf_src,
		(framesize)		//cant have this own crc here
		);
	
	*(uint16*)(databuf_src + framesize)	= crc16_frame;
	framesize = framesize + sizeof(crc16_frame);
	
	switch(SpecificIPC->dswifiSrv.dsnwifisrv_mode){
		case dswifi_nifimode:{
			Wifi_RawTxFrame_NIFI(framesize , 0x0014, (unsigned short *)databuf_src);
		}
		break;
		case dswifi_wifimode:{
			Wifi_RawTxFrame_WIFI(framesize , databuf_src);
		}
		break;
	}
	
}


void getcmd(uint8 * databuf_src)
{
	
	switch(SpecificIPC->dswifiSrv.dsnwifisrv_mode){
		case dswifi_nifimode:{
			int frame_hdr_detected_size = frame_header_size;	//nifi raw only frame has this header
			int offset_shared = frame_hdr_detected_size + 3;	//recv data is past first 32bytes of data[]
			
			//data buffer has come from RX handler already
			memcpy((uint8*)&nifi_cmd, (uint8*)(databuf_src + offset_shared), sizeof(nifi_cmd));
			
			if(nifi_stat == 5) {	//host
			
				memcpy((uint8*)&plykeys2, (uint8*)(databuf_src + offset_shared + sizeof(nifi_cmd)), sizeof(plykeys2));
				
				//get guest VCOUNT
				memcpy((uint8*)&guest_vcount, (uint8*)(databuf_src + offset_shared + sizeof(nifi_cmd) + sizeof(plykeys2)), sizeof(guest_vcount));
				
				//new: get nifi_sync keys from guest
				memcpy((uint8*)&nifi_keys_sync, (uint8*)(databuf_src + offset_shared + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount)), sizeof(nifi_keys_sync));
				
				//new: get guest_framecount from guest
				memcpy((uint8*)&guest_framecount, (uint8*)(databuf_src + offset_shared + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount) + sizeof(nifi_keys_sync)), sizeof(guest_framecount));
				
				offset_shared = offset_shared + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount) + sizeof(nifi_keys_sync) + sizeof(guest_framecount);
				
			} 
			else {	
				//get host VCOUNT
				memcpy((uint8*)&host_vcount, (uint8*)(databuf_src + offset_shared + sizeof(nifi_cmd)), sizeof(host_vcount));
				
				//new: recv plykeys1 overwifi , not compatible with stock nesdsnifi
				memcpy((uint8*)&plykeys1, (uint8*)(databuf_src + offset_shared + sizeof(nifi_cmd) + sizeof(host_vcount)), sizeof(plykeys1));
				
				//new: get host framecount
				memcpy((uint32*)&host_framecount, (uint8*)(databuf_src + offset_shared + sizeof(nifi_cmd) + sizeof(host_vcount) + sizeof(plykeys1)), sizeof(host_framecount));
				//debuginfo[VBLS] = nesds_framecount = host_framecount;
				
				offset_shared = offset_shared + sizeof(nifi_cmd) + sizeof(host_vcount) + sizeof(plykeys1) + sizeof(host_framecount);
			}
			
			//shared (guest & host vars here)
			//uint16 crc16_recv_frame = 0;
			//memcpy(&crc16_recv_frame, data + offset_shared, sizeof(crc16_recv_frame));
			
		}
		break;
		case dswifi_wifimode:{
			int frame_hdr_detected_size = 0;					//udp nifi frame has not this header
			int offset_shared = frame_hdr_detected_size + 3;	//recv data is past first 32bytes of data[]
			
			//data buffer has come from RX handler already
			memcpy((uint8*)&nifi_cmd, (uint8*)(databuf_src + offset_shared), sizeof(nifi_cmd));
			
			switch(SpecificIPC->dswifiSrv.dsnwifisrv_stat){
				case(ds_netplay_host):{
					memcpy((uint8*)&plykeys2, (uint8*)(databuf_src + offset_shared + sizeof(nifi_cmd)), sizeof(plykeys2));
				
					//get guest VCOUNT
					memcpy((uint8*)&guest_vcount, (uint8*)(databuf_src + offset_shared + sizeof(nifi_cmd) + sizeof(plykeys2)), sizeof(guest_vcount));
					
					//new: get nifi_sync keys from guest
					memcpy((uint8*)&nifi_keys_sync, (uint8*)(databuf_src + offset_shared + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount)), sizeof(nifi_keys_sync));
					
					//new: get guest_framecount from guest
					memcpy((uint8*)&guest_framecount, (uint8*)(databuf_src + offset_shared + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount) + sizeof(nifi_keys_sync)), sizeof(guest_framecount));
					
					offset_shared = offset_shared + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount) + sizeof(nifi_keys_sync) + sizeof(guest_framecount);
					
				}break;
				
				case(ds_netplay_guest):{
					//get host VCOUNT
					memcpy((uint8*)&host_vcount, (uint8*)(databuf_src + offset_shared + sizeof(nifi_cmd)), sizeof(host_vcount));
					
					//new: recv plykeys1 overwifi , not compatible with stock nesdsnifi
					memcpy((uint8*)&plykeys1, (uint8*)(databuf_src + offset_shared + sizeof(nifi_cmd) + sizeof(host_vcount)), sizeof(plykeys1));
					
					//new: get host framecount
					memcpy((uint32*)&host_framecount, (uint8*)(databuf_src + offset_shared + sizeof(nifi_cmd) + sizeof(host_vcount) + sizeof(plykeys1)), sizeof(host_framecount));
					//debuginfo[VBLS] = nesds_framecount = host_framecount;
					
					offset_shared = offset_shared + sizeof(nifi_cmd) + sizeof(host_vcount) + sizeof(plykeys1) + sizeof(host_framecount);
					
				}break;
				
			}
		}
		break;
	}
	
	
}


//coto: (sender) make do_multi return frame received
bool do_multi()
{
	//single player
	if(SpecificIPC->dswifiSrv.dsnwifisrv_mode == dswifi_idlemode){
		
		plykeys1 = get_joypad() | 0x80000000;	//bit15 (second half word) is for ready bit, required by emu
		
		//joypad1 ready state? start reading from 15th bit backwards.
		if (CPU.DMA_PORT[0x00]&1)
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
	
	//warning:this dswnifi lib is old, update the latest one from nesds
	//detect if WIFI mode is enabled, give priority then when ready (wifi set up between ds consoles is done) continue
	if(SpecificIPC->dswifiSrv.dsnwifisrv_mode == dswifi_wifimode){
		
		//Handle sender (WIFI TX)
		switch(SpecificIPC->dswifiSrv.dsnwifisrv_stat){		
			
			case(ds_netplay_host):{	//host should refresh the global keys.	
				//plykeys1 = IPC_KEYS & MP_KEY_MSK;
			}break;
			
			case(ds_netplay_guest):{				//guest...
				//plykeys2 = IPC_KEYS & MP_KEY_MSK;
				//nifi_keys_sync	=	(plykeys1 & MP_KEY_MSK) | ((plykeys2 & MP_KEY_MSK) << 16);
			}break;
			
		}
		//requires all stages of dswifi_wifimode for init
		sendcmd((uint8*)&nfdata[0]);
		
		//Handle recv 
		switch(SpecificIPC->dswifiSrv.dsnwifisrv_stat){
			//#1 DS is not connected, serve any upcoming commands related to non connected to multi
			//ds_searching_for_multi_servernotaware -> ds_wait_for_multi here
			case(ds_searching_for_multi_servernotaware):{
				
				//Server UDP Handler listener
				volatile unsigned long available_ds_server;
				ioctl (client_http_handler_context.socket_id__multi_notconnected, FIONREAD, (uint8*)&available_ds_server);
				
				volatile char incomingbuf[256];
				volatile int datalen;
				volatile int sain_len;
				volatile struct sockaddr_in sender_server;
				sain_len=sizeof(sender_server);
				volatile char cmd[12];	//srv->ds command handler
				if(available_ds_server > 0){
					datalen=recvfrom(client_http_handler_context.socket_id__multi_notconnected,(uint8*)incomingbuf,sizeof(incomingbuf),0,(struct sockaddr *)&sender_server,(int*)&sain_len);
					if(datalen>0) {
						//incomingbuf[datalen]=0;
						memcpy((uint8*)cmd,(uint8*)incomingbuf,sizeof(cmd));	//cmd recv
					}
				}
		
				
				//Server aware
				if(strncmp((const char *)cmd, (const char *)"srvaware", 8) == 0){
					
					//server send other NDS ip format: cmd-ip-multi_mode- (last "-" goes as well!)
					char **tokens;
					int count, i;
					//const char *str = "JAN,FEB,MAR,APR,MAY,JUN,JUL,AUG,SEP,OCT,NOV,DEC";
					volatile char str[256];
					memcpy ( (uint8*)str, (uint8*)incomingbuf, 256);

					count = split ((const char*)str, '-', &tokens);
					for (i = 0; i < count; i++) {
						//then send back "dsaware"
						//char outgoingbuf[64];
						//sprintf(outgoingbuf,"%s",tokens[i]);
						//sendto(client_http_handler_context.socket_id__multi_notconnected,outgoingbuf,strlen(outgoingbuf),0,(struct sockaddr *)&client_http_handler_context.server_addr,sizeof(client_http_handler_context.server_addr));
					}
					
					//tokens[0];	//cmd
					//tokens[1];	//external NDS ip to connect
					//tokens[2];	//host or guest
					
					int host_mode = strncmp((const char*)tokens[2], (const char *)"host", 4); //host == 0
					int guest_mode = strncmp((const char*)tokens[2], (const char *)"guest", 5); //guest == 0
					
					client_http_handler_context.socket_multi_listener=socket(AF_INET,SOCK_DGRAM,0);
					int cmd=1;
					cmd=ioctl(client_http_handler_context.socket_multi_listener,FIONBIO,&cmd); // set non-blocking port
					
					client_http_handler_context.socket_multi_sender=socket(AF_INET,SOCK_DGRAM,0);
					
					int optval = 1, len;
					setsockopt(client_http_handler_context.socket_multi_sender, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval));
	
					int LISTENER_PORT 	=	0;
					int SENDER_PORT		=	0;
					if(host_mode == 0){
						LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_HOST;
						SENDER_PORT		=	(int)NDSMULTI_UDP_PORT_GUEST;
					}
					else if(guest_mode == 0){
						LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_GUEST;
						SENDER_PORT		=	(int)NDSMULTI_UDP_PORT_HOST;
					}
					
					//bind conn to LOCAL ip-port listener
					memset((char *) &client_http_handler_context.sain_listener, 0, sizeof(client_http_handler_context.sain_listener));
					client_http_handler_context.sain_listener.sin_family = AF_INET;
					client_http_handler_context.sain_listener.sin_addr.s_addr=INADDR_ANY;	//local/any ip listen to desired port
					//int atoi ( const char * str );
					//int nds_multi_port = atoi((const char*)tokens[2]);
					client_http_handler_context.sain_listener.sin_port = htons(LISTENER_PORT); //nds_multi_port
					
					
					//NDS MULTI IP: No need to bind / sendto use
					memset((char *) &client_http_handler_context.sain_sender, 0, sizeof(client_http_handler_context.sain_sender));
					client_http_handler_context.sain_sender.sin_family = AF_INET;
					client_http_handler_context.sain_sender.sin_addr.s_addr = INADDR_BROADCAST;//((const char*)"191.161.23.11");// //ip was reversed 
					client_http_handler_context.sain_sender.sin_port = htons(SENDER_PORT); 
					
					struct sockaddr_in *addr_in2= (struct sockaddr_in *)&client_http_handler_context.sain_sender;
					char *IP_string_sender = inet_ntoa(addr_in2->sin_addr);
					
					
					//bind ThisIP(each DS network hardware) against the current DS UDP port
					if(bind(client_http_handler_context.socket_multi_listener,(struct sockaddr *)&client_http_handler_context.sain_listener,sizeof(client_http_handler_context.sain_listener))) {
						if(host_mode == 0){
							printf("%s \n","[host]binding error");
						}
						else if(guest_mode == 0){
							printf("%s \n","[guest]binding error");
						}
						
						close(client_http_handler_context.socket_multi_listener);
						return 0;
					}
					else{
						char buf[96];
						char id[16];
						//read IP from sock interface binded
						struct sockaddr_in *addr_in= (struct sockaddr_in *)&client_http_handler_context.sain_listener;	//0.0.0.0 == (char*)print_ip((uint32)Wifi_GetIP()) 
						char *IP_string = inet_ntoa(addr_in->sin_addr);
						
						if(host_mode == 0){
							sprintf(buf,"[host]binding OK MULTI: port [%d] IP: [%s]  \n",LISTENER_PORT, (const char*)print_ip((uint32)Wifi_GetIP()));//(char*)print_ip((uint32)Wifi_GetIP()));
							sprintf(id,"[host]");
							
							//stop sending data, server got it already.
							SpecificIPC->dswifiSrv.dsnwifisrv_stat = ds_netplay_host_servercheck;
						}
						else if(guest_mode == 0){
							sprintf(buf,"[guest]binding OK MULTI: port [%d] IP: [%s]  \n",LISTENER_PORT, (const char*)print_ip((uint32)Wifi_GetIP()));//(char*)print_ip((uint32)Wifi_GetIP()));
							sprintf(id,"[guest]");
							//stop sending data, server got it already.
							SpecificIPC->dswifiSrv.dsnwifisrv_stat = ds_netplay_guest_servercheck;
						}
						
						//note: bind UDPsender?: does not work with UDP Datagram socket format (UDP basically)
					}
					
				}
				
			}
			break;
			
			//servercheck phase acknow
			case(ds_netplay_host_servercheck):case(ds_netplay_guest_servercheck):{
				
				//Server UDP Handler listener
				volatile unsigned long available_ds_server;
				ioctl (client_http_handler_context.socket_id__multi_notconnected, FIONREAD, (uint8*)&available_ds_server);
				
				volatile char incomingbuf[256];
				volatile int datalen;
				volatile int sain_len;
				volatile struct sockaddr_in sender_server;
				sain_len=sizeof(sender_server);
				volatile char cmd[12];	//srv->ds command handler
				if(available_ds_server > 0){
					datalen=recvfrom(client_http_handler_context.socket_id__multi_notconnected,(uint8*)incomingbuf,sizeof(incomingbuf),0,(struct sockaddr *)&sender_server,(int*)&sain_len);
					if(datalen>0) {
						//incomingbuf[datalen]=0;
						memcpy((uint8*)cmd,(uint8*)incomingbuf,sizeof(cmd));	//cmd recv
					}
				}
				
				if(strncmp((const char *)cmd, (const char *)"dsconnect", 9) == 0){
					
					int LISTENER_PORT 	=	0;
					int SENDER_PORT		=	0;
					if(SpecificIPC->dswifiSrv.dsnwifisrv_stat == ds_netplay_host_servercheck){
						LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_HOST;
						SENDER_PORT		=	(int)NDSMULTI_UDP_PORT_GUEST;
					}
					else if(SpecificIPC->dswifiSrv.dsnwifisrv_stat == ds_netplay_guest_servercheck){
						LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_GUEST;
						SENDER_PORT		=	(int)NDSMULTI_UDP_PORT_HOST;
					}
					
					if(SpecificIPC->dswifiSrv.dsnwifisrv_stat == ds_netplay_host_servercheck){	
						printf("//////////DSCONNECTED[HOST]-PORT:%d",LISTENER_PORT);
						SpecificIPC->dswifiSrv.dsnwifisrv_stat = ds_netplay_host;
						nifi_stat = 5;
					}
					else if(SpecificIPC->dswifiSrv.dsnwifisrv_stat == ds_netplay_guest_servercheck){
						printf("//////////DSCONNECTED[GUEST]-PORT:%d",LISTENER_PORT);
						SpecificIPC->dswifiSrv.dsnwifisrv_stat = ds_netplay_guest;
						nifi_stat = 6;
					}
					
					close(client_http_handler_context.socket_id__multi_notconnected); //closer server socket to prevent problems
					
				}
			}
			break;
			
			//#last:connected!
			//logic: recv data(256byte buf) from port
			case(ds_netplay_host):case(ds_netplay_guest):{
				
				//DS-DS UDP Handler listener
				volatile unsigned long available_ds_ds;
				ioctl (client_http_handler_context.socket_multi_listener, FIONREAD, (uint8*)&available_ds_ds);
				
				volatile int datalen2;
				volatile uint8 inputbuf[512];
				volatile int sain_len2;
				volatile struct sockaddr_in sender_ds;
				sain_len2=sizeof(sender_ds);
				
				if(available_ds_ds > 0){
				
					datalen2=recvfrom(client_http_handler_context.socket_multi_listener,(uint8*)inputbuf,sizeof(inputbuf),0,(struct sockaddr *)&sender_ds,(int*)&sain_len2);
					if(datalen2>0) {
						inputbuf[datalen2-1]=0;
						
						int framesize = 0;
						int frame_hdr_size = 0;	//nifi has this header, wifi no
						
						switch(SpecificIPC->dswifiSrv.dsnwifisrv_stat){
							//#last:connected!
							case(ds_netplay_host):{	//host receives from guest
								framesize = 3 + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount) + sizeof(nifi_keys_sync) + sizeof(guest_framecount);
							}break;
							
							case(ds_netplay_guest):{ //guest receives from host
								framesize = 3 + sizeof(nifi_cmd) + sizeof(host_vcount) + sizeof(plykeys1) + sizeof(host_framecount);
							}break;
						}
						
						
						//coto:
						//read crc from nifi frame so we dont end up with corrupted data.
						volatile uint16 crc16_recv_frame = (uint16)*(uint16*)(inputbuf + frame_hdr_size + framesize);
						
						//generate crc per nifi frame so we dont end up with corrupted data.
						volatile uint16 crc16_frame_gen = swiCRC16	(	0xffff, //uint16 	crc,
							(uint8*)(inputbuf + frame_hdr_size),
							(framesize)		//cant have this own crc here
							);
						
						//char buf[64];
						
						//data is now nifi frame
						if(crc16_frame_gen == crc16_recv_frame){			
							getcmd((uint8*)&inputbuf[0]);	
							
							//sprintf(buf,"[FRAME CRC OK]");
							//consoletext(64*2-32,(char *)&buf[0],0);
							
							return true;	//exit. dswnifi_frame valid
						}
						else{
							//sprintf(buf,"[%xFRAME CRC CORRUPTED]",(uint8)inputbuf[0]);
							//consoletext(64*2-32,(char *)&buf[0],0);
						}
						
					}
				}
			}
			break;
		}
		
		return false;	//exit. dswnifi_frame invalid
	}
	
	else if(SpecificIPC->dswifiSrv.dsnwifisrv_mode == dswifi_nifimode){	//detect nifi_stat here if nifi_stat = 0 for disconnect when nifi was issued
		static int count = 0;
		switch(nifi_stat) {
			case 0:{
				/*
				if(nifi_cmd & MP_NFEN) {
					Wifi_DisableWifi();
					nifi_cmd &= ~MP_NFEN;
					
					switch_dswnifi_mode((uint8)dswifi_idlemode); //self disconnect
				}
				*/
				return false;
			}
			break;
			case 1:		//act as a host, waiting for another player.
				/*
				if(!(nifi_cmd & MP_NFEN)){
					Wifi_EnableWifi();
				}
				nifi_cmd = MP_HOST | MP_NFEN;
				*/
				break;	//waiting for interrupt.
			case 2:		//act as a guest.
				/*
				if(!(nifi_cmd & MP_NFEN)){
					Wifi_EnableWifi();
				}
				nifi_cmd = MP_NFEN;
				if(count++ > 30) {			//send a flag every second to search a host.
					switch(SpecificIPC->dswifiSrv.dsnwifisrv_mode){
						case dswifi_nifimode:{
							Wifi_RawTxFrame_NIFI(sizeof(nifitoken), 0x0014, (unsigned short *)nifitoken);
						}
						break;
						//case dswifi_wifimode:{
						//	Wifi_RawTxFrame_WIFI(sizeof(nifitoken), (uint8*)nifitoken);
						//}
						//break;
					}
		
					count = 0;
				}
				*/
				break;
			case 3:							//tell the guest that he should send the PRGCRC to the host
				/*
				if(!(nifi_cmd & MP_NFEN)){
					Wifi_EnableWifi();
				}
				if(count++ > 30) {			//send a connected flag.
					
					switch(SpecificIPC->dswifiSrv.dsnwifisrv_mode){
						case dswifi_nifimode:{
							Wifi_RawTxFrame_NIFI(sizeof(nificonnect), 0x0014, (unsigned short *)nificonnect);
						}
						break;
						
						//case dswifi_wifimode:{
						//	Wifi_RawTxFrame_WIFI(sizeof(nificonnect), (uint8*)nificonnect);
						//}
						//break;
						
					}
					
					count = 0;
				}
				*/
				break;
			case 4:
				/*
				if(!(nifi_cmd & MP_NFEN)){
					Wifi_EnableWifi();
				}
				if(count++ > 30) {			//send a connected flag.
					nificrc[3] = debuginfo[17] &0xFF;
					nificrc[4] = (debuginfo[17] >> 8 )&0xFF;
					
					switch(SpecificIPC->dswifiSrv.dsnwifisrv_mode){
						case dswifi_nifimode:{
							Wifi_RawTxFrame_NIFI(6, 0x0014, (unsigned short *)nificrc);
						}
						break;
						
						//case dswifi_wifimode:{
						//	Wifi_RawTxFrame_WIFI(6, (uint8*)nificrc);
						//}
						//break;
					}
					
					count = 0;
				}
				*/
				break;
			case 5:				//host should refresh the global keys.
			{	
				plykeys1 = (uint16)get_joypad();
				sendcmd((uint8*)&nfdata[0]);
			}	
				break;
			case 6:				//guest...
			{	
				plykeys2 = (uint16)get_joypad();	//plugged bit is sent also
				nifi_keys_sync	=	(plykeys1 ) | ((plykeys2) << 16);
				
				//todo: when nifi_keys_sync is acknowledged (written to IO map), set each plugged bit (0x80000000) as well.
				
				sendcmd((uint8*)&nfdata[0]);
			}	break;
			
			default:
			break;
			
		}
		
		return true;
	}
	
	else{
		return false;
	}
	
}
