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
#include "dswnifi.h"

#include "specific_shared.h"
#include "wifi_shared.h"
#include "clock.h"

#ifdef ARM9

#include "nifi.h"

#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefs.h"


#include "specific_shared.h"
#include "wifi_arm9.h"
#include "dswifi9.h"
#include "wifi_shared.h"
#include "toolchain_utils.h"
#include "main.h"

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


#include "nifi.h"
#include "wifi_arm9.h"
#include "dswifi9.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
//#include <types.h>
#include <socket.h>
#include <in.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "typedefs.h"
#include "http_utils.h"

#include "http_utils.h"
#include "nifi.h"
#include "dswifi9.h"

#include "socket.h"
#include "in.h"
#include <netdb.h>
#include <stdio.h>
#include <string.h>  
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "wifi_arm9.h"


#endif

#ifdef ARM9
TdsnwifisrvStr dswifiSrv;
sint8* server_ip = (sint8*)"192.168.43.220";
client_http_handler client_http_handler_context;

//opens port 32123 @ UDP (any IP inbound from sender)
void request_connection(sint8* str_url,int str_url_size)
{   
	//UDP
	
	//for any IP that writes to our own IP @ PORT
	client_http_handler_context.socket_id__multi_notconnected=socket(AF_INET,SOCK_DGRAM,0);
	
	int i=1;
	i=ioctl(client_http_handler_context.socket_id__multi_notconnected,FIONBIO,&i); // set non-blocking port
	client_http_handler_context.sain_UDP_PORT.sin_family=AF_INET;
	client_http_handler_context.sain_UDP_PORT.sin_addr.s_addr=0;
	client_http_handler_context.sain_UDP_PORT.sin_port=htons((int)UDP_PORT);
	
	if(bind(client_http_handler_context.socket_id__multi_notconnected,(struct sockaddr *)&client_http_handler_context.sain_UDP_PORT,sizeof(client_http_handler_context.sain_UDP_PORT))) {
		sint8 buf[64];
		sprintf(buf,"binding ERROR ");
		printf((sint8 *)&buf[0]);
		close(client_http_handler_context.socket_id__multi_notconnected);
		return;
	}
	else{
		sint8 buf[64];
		sprintf(buf,"binding OK: port %d IP: %s ",(int)UDP_PORT, (sint8*)print_ip((uint32)Wifi_GetIP()));//(sint8*)print_ip((uint32)Wifi_GetIP()));
		printf((sint8 *)&buf[0]);
	}
	
	
	//for server IP / used for sending msges
	memset((sint8 *) &client_http_handler_context.server_addr, 0, sizeof(client_http_handler_context.server_addr));
	client_http_handler_context.server_addr.sin_family = AF_INET;
	client_http_handler_context.server_addr.sin_addr.s_addr = inet_addr(server_ip);//SERV_HOST_ADDR);
	client_http_handler_context.server_addr.sin_port = htons((int)UDP_PORT); //SERVER_PORT_ID);
	
}

//0 idle, 1 nifi, 2 dswnifi
void switch_dswnifi_mode(sint32 mode){
	
	//idle mode minimal setup
	if (mode == (sint32)dswifi_idlemode){
		dswifiSrv.dsnwifisrv_stat	= ds_multi_notrunning;
		setMULTIMode(dswifi_idlemode);
		dswifiSrv.dswifi_setup = false;
	}
	//Raw Network Packet Nifi
	else if (mode == (sint32)dswifi_localnifimode){
		//nifi
		dswifiSrv.dsnwifisrv_stat	= ds_searching_for_multi_servernotaware;
		setMULTIMode(dswifi_localnifimode);
		dswifiSrv.dswifi_setup = false;
	}
	//UDP Nifi/WIFI
	else if (
		(mode == (sint32)dswifi_udpnifimode)
		||
		(mode == (sint32)dswifi_tcpnifimode)
	){
		dswifiSrv.dsnwifisrv_stat = ds_searching_for_multi_servernotaware;
		setMULTIMode(mode);
		dswifiSrv.dswifi_setup = false;
	}
	
	
	// Idle
	if(getMULTIMode() == dswifi_idlemode){
		dswifiSrv.dswifi_setup = false;
		setConnectionStatus(proc_idle);
	}
	
	//set NIFI mode
	else if((getMULTIMode() == dswifi_localnifimode) && (dswifiSrv.dswifi_setup == false)){
		initNiFi();
		dswifiSrv.dswifi_setup = true;
		setConnectionStatus(proc_connect);
	}
	
	//set UDP/TCP DSWNIFI
	else if(
		(
		(getMULTIMode() == dswifi_udpnifimode)
		||
		(getMULTIMode() == dswifi_tcpnifimode)
		)
		&& (dswifiSrv.dswifi_setup == false)
	){
		if(Wifi_InitDefault(WFC_CONNECT) == true)
		{
			printf("connected: IP: %s",(char*)print_ip((uint32)Wifi_GetIP()));
			setConnectionStatus(proc_connect);
			dswifiSrv.dswifi_setup = true;
		}
		else{
			//Could not connect
			setConnectionStatus(proc_idle);
		}
	}
	
}

__attribute__((section(".itcm")))
void setMULTIMode(sint32 flag){
	dswifiSrv.dsnwifisrv_mode = (sint32)flag;
}

__attribute__((section(".itcm")))
sint32 getMULTIMode(){
	return (sint32)dswifiSrv.dsnwifisrv_mode;
}

__attribute__((section(".itcm")))
bool getWIFISetup(){
	return (bool)dswifiSrv.dswifi_setup;
}

__attribute__((section(".itcm")))
void setConnectionStatus(sint32 flag){
	dswifiSrv.connectionStatus = (sint32)flag;
}

__attribute__((section(".itcm")))
sint32 getConnectionStatus(){
	return (sint32)dswifiSrv.connectionStatus;
}

//these cant be in shared memory, gets stuck
int port = 8888; 	//gdb stub port
//SOCK_STREAM = TCP / SOCK_DGRAM = UDP
struct sockaddr_in stSockAddrServer;

int SocketFDLocal = -1;
int SocketFDServer = -1;

//ret: -1 not connected, 0 UDP NIFI ok, 1 TCP NIFI ok, 2 LOCAL NIFI ok (connect, execute and disconnect)
__attribute__((section(".itcm")))
sint32 doMULTIDaemon(){
	
	switch(getConnectionStatus()){
		case (proc_idle):{
			//nothing to do for : LOCAL / UDP NIFI / TCP NIFI
			return -1;
		}
		break;
		
		case (proc_connect):{
			
			//UDP NIFI
			if(getMULTIMode() == dswifi_udpnifimode){
				
				//DSWNIFI library uses this for UDP handshake
				request_connection(server_ip,strlen(server_ip));
				
				//Default UDP method for connecting to server, DSWNIFI library does not use this.
				/*
				memset(&stSockAddrClient, 0, sizeof(stSockAddrClient));
				SocketFDLocal = socket(AF_INET, SOCK_DGRAM, 0);
				if(-1 == SocketFDLocal)
				{
					return -1;
				}
				
				int enable = 1;
				if (setsockopt(SocketFDLocal, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
					
				}
				
				int i=1;
				i=ioctl(SocketFDLocal,FIONBIO,&i); // set non-blocking port
				
				
				stSockAddrClient.sin_family = AF_INET;
				stSockAddrClient.sin_port = htons(port);
				stSockAddrClient.sin_addr.s_addr = INADDR_ANY;	//local/any ip listen to desired port //inet_addr((sint8*)"192.168.43.220");
				
				if(bind(SocketFDLocal,(struct sockaddr *)&stSockAddrClient, sizeof(stSockAddrClient)))
				{
					return -1;
				}
				
				//Server Setup
				memset((uint8*)&stSockAddrServer, 0, sizeof(stSockAddrServer));
				
				SocketFDServer = socket(PF_INET, SOCK_DGRAM, 0);
				
				stSockAddrServer.sin_family = AF_INET;
				stSockAddrServer.sin_port = htons(port);
				stSockAddrServer.sin_addr.s_addr = inet_addr(server_ip);
				*/
				
				
				//no binding since we have no control of server port and we should not know it anyway
				setConnectionStatus(proc_execution);
				return 0;
			}
			//TCP NIFI
			if(getMULTIMode() == dswifi_tcpnifimode){
				SocketFDLocal = socket(AF_INET, SOCK_STREAM, 0);
				if(-1 == SocketFDLocal)
				{
					return -1;
				}
				int i=1;
				i=ioctl(SocketFDLocal,FIONBIO,&i); // set non-blocking port (otherwise emulator blocks)
				
				int optval = 1;
				setsockopt(SocketFDLocal, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
				//Server Setup
				memset((uint8*)&stSockAddrServer, 0, sizeof(stSockAddrServer));
				
				/* this is an Internet address */
				stSockAddrServer.sin_family = AF_INET;
				/* let the system figure out our IP address */
				stSockAddrServer.sin_addr.s_addr = INADDR_ANY;
				/* this is the port we will listen on */
				stSockAddrServer.sin_port = htons(port);
				//connect wont work since we want the DS to open TCP at 8888 port
				/*
				if (connect(SocketFDLocal,&stSockAddrServer,sizeof(stSockAddrServer)) < 0){
					printf("ERROR connecting");
					close(SocketFDLocal);
				}
				else{
					printf("OK connecting");
				}
				*/
				if(bind(SocketFDLocal,(struct sockaddr *)&stSockAddrServer, sizeof(stSockAddrServer)))
				{
					return -1;
				}
				
				listen(SocketFDLocal,5);	//DS Acts as server at desired port
				setConnectionStatus(proc_execution);
				return 1;
			}
			
			
			//LOCAL NIFI: runs on DSWIFI process
			if(getMULTIMode() == dswifi_localnifimode){
				setConnectionStatus(proc_execution);
				return 2;
			}
		}
		break;
		
		case (proc_execution):{
			
			///////////////////////////////////////Handle Send (case dependency handled inside)
			HandleSendUserspace((uint8*)&nfdata[0],sizeof(nfdata));	//use the nfdata as send buffer
			
			
			//////////////////////////////////////////Handle Recv
			
			//UDP NIFI
			if(getMULTIMode() == dswifi_udpnifimode){
				
				/* works, default template but dswnifi use its own 
				int clilen = 0;
				int srvlen = 0;
				struct sockaddr_in cli_addr;
				
				clilen = sizeof(cli_addr);
				srvlen = sizeof(stSockAddrServer);
				memset((uint8*)&cli_addr, 0, clilen);
				volatile char incomingbuf[256];
				volatile char sendbuf[256];
				
				int read_size = 0;
				//Receive a message from client
				if( (read_size = recvfrom(SocketFDLocal, (uint8*)incomingbuf, sizeof(incomingbuf), 0, (struct sockaddr*)&cli_addr, (int*)&clilen)) > 0 )
				{
					//Handle Remote Procedure Commands
					sprintf((char*)sendbuf,"DSTime:%d:%d:%d",getTime()->tm_hour,getTime()->tm_min,getTime()->tm_sec);
					
					//Send reply data
					if( sendto(SocketFDLocal,(uint8*)sendbuf, sizeof(sendbuf),0,(struct sockaddr *)&stSockAddrServer,srvlen) < 0)
					{
						//printf("Send failed");
					}
					else{
						//printf("Send ok");
					}
					
					//only after we accepted we can close Server socket.
					//setConnectionStatus(proc_shutdown);
				}
				*/
				
				//UDP: (execute RPC from server and process frames)
				switch(dswifiSrv.dsnwifisrv_stat){
					//#1 DS is not connected, serve any upcoming commands related to non connected to multi
					//ds_searching_for_multi_servernotaware -> ds_wait_for_multi here
					case(ds_searching_for_multi_servernotaware):{
						
						//Server UDP Handler listener
						volatile unsigned long available_ds_server;
						ioctl (client_http_handler_context.socket_id__multi_notconnected, FIONREAD, (uint8*)&available_ds_server);
						
						volatile char incomingbuf[256];
						volatile int datalen = 0;
						volatile int sain_len = 0;
						
						volatile struct sockaddr_in sender_server;
						sain_len=sizeof(struct sockaddr_in);
						memset((uint8*)&sender_server, 0, sain_len);
						volatile char cmd[12];	//srv->ds command handler
						if(available_ds_server > 0){
							if( (datalen = recvfrom(client_http_handler_context.socket_id__multi_notconnected, (uint8*)incomingbuf, sizeof(incomingbuf), 0, (struct sockaddr *)&sender_server,(int*)&sain_len)) > 0 ){
								if(datalen>0) {
									//incomingbuf[datalen]=0;
									memcpy((uint8*)cmd,(uint8*)incomingbuf,sizeof(cmd));	//cmd recv
								}
							}
						}
				
						//add frame receive here and detect if valid frame, if not, run the below
						
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
									//printf("%s ","[host]binding error");
								}
								else if(guest_mode == 0){
									//printf("%s ","[guest]binding error");
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
									sprintf(buf,"[host]binding OK MULTI: port [%d] IP: [%s]  ",LISTENER_PORT, (const char*)print_ip((uint32)Wifi_GetIP()));//(char*)print_ip((uint32)Wifi_GetIP()));
									sprintf(id,"[host]");
									printf("%s",buf);
									//stop sending data, server got it already.
									dswifiSrv.dsnwifisrv_stat = ds_netplay_host_servercheck;
								}
								else if(guest_mode == 0){
									sprintf(buf,"[guest]binding OK MULTI: port [%d] IP: [%s]  ",LISTENER_PORT, (const char*)print_ip((uint32)Wifi_GetIP()));//(char*)print_ip((uint32)Wifi_GetIP()));
									sprintf(id,"[guest]");
									printf("%s",buf);
									//stop sending data, server got it already.
									dswifiSrv.dsnwifisrv_stat = ds_netplay_guest_servercheck;
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
						volatile char cmd[12];	//srv->ds command handler
						volatile struct sockaddr_in sender_server;
						sain_len=sizeof(struct sockaddr_in);
						memset((uint8*)&sender_server, 0, sain_len);
						
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
							if(dswifiSrv.dsnwifisrv_stat == ds_netplay_host_servercheck){
								LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_HOST;
								SENDER_PORT		=	(int)NDSMULTI_UDP_PORT_GUEST;
							}
							else if(dswifiSrv.dsnwifisrv_stat == ds_netplay_guest_servercheck){
								LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_GUEST;
								SENDER_PORT		=	(int)NDSMULTI_UDP_PORT_HOST;
							}
							
							if(dswifiSrv.dsnwifisrv_stat == ds_netplay_host_servercheck){	
								clrscr();
								printf("//////////DSCONNECTED[HOST]-PORT:%d",LISTENER_PORT);
								dswifiSrv.dsnwifisrv_stat = ds_netplay_host;
								nifi_stat = 5;
							}
							else if(dswifiSrv.dsnwifisrv_stat == ds_netplay_guest_servercheck){
								clrscr();
								printf("//////////DSCONNECTED[GUEST]-PORT:%d",LISTENER_PORT);
								dswifiSrv.dsnwifisrv_stat = ds_netplay_guest;
								nifi_stat = 6;
							}
							
							close(client_http_handler_context.socket_id__multi_notconnected); //closer server socket to prevent problems when udp multiplayer
							
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
						volatile uint8 inputbuf[256];
						volatile int sain_len2;
						volatile struct sockaddr_in sender_ds;
						sain_len2=sizeof(struct sockaddr_in);
						
						memset((uint8*)&sender_ds, 0, sain_len2);
						
						if(available_ds_ds > 0){
						
							datalen2=recvfrom(client_http_handler_context.socket_multi_listener,(uint8*)inputbuf,sizeof(inputbuf),0,(struct sockaddr *)&sender_ds,(int*)&sain_len2);
							if(datalen2>0) {
								inputbuf[datalen2-1]=0;
								
								//decide whether to put data in userbuffer and if frame is valid here
								receiveDSWNIFIFrame((uint8 *)&inputbuf[0],datalen2);
							}
						}
					}
					break;
				}
				
				//only after we accepted we can close Server socket.
				//setConnectionStatus(proc_shutdown);
				
				return 0;
			}
			
			//TCP NIFI
			if(getMULTIMode() == dswifi_tcpnifimode){
				
				int connectedSD = -1;
				int read_size = 0;
				struct sockaddr_in stSockAddrClient;
				struct timeval timeout;
				timeout.tv_sec = 0;
				timeout.tv_usec = 0 * 1000;
				int stSockAddrClientSize = sizeof(struct sockaddr_in);
				memset((uint8*)&stSockAddrClient, 0, sizeof(struct sockaddr_in));
				
				volatile uint8 sendbuf[512];
				volatile uint8 recvbuf[512];

				if(!(-1 == SocketFDLocal))
				{
					//TCP
					if ((connectedSD = accept(SocketFDLocal, (struct sockaddr *)(&stSockAddrClient), &stSockAddrClientSize)) == -1)
					{ 
						//perror("accept"); exit(1); 
					}
					else{
						/* Read from the socket */
						//Receive a message from client (only data > 0 ), socket will be closed anyway
						while( (read_size = recv(connectedSD , (uint8*)recvbuf , sizeof(recvbuf) , 0)) > 0 )
						{
							
						}
						recvbuf[read_size] = '\0';
						
						//Handle Remote Procedure Commands
						//sprintf((char*)sendbuf,"DSTime:%d:%d:%d",getTime()->tm_hour,getTime()->tm_min,getTime()->tm_sec);
						
						//Send some data
						if( send(connectedSD , (uint8*)sendbuf , sizeof(sendbuf) , 0) > 0)
						{
							//Send ok
						}
						else{
							//Send error
						}
						
						//only after we accepted we can close Server socket.
						setConnectionStatus(proc_shutdown);
					}
					close(connectedSD);
				}
		
				
				return 1;
			}
			
			//LOCAL NIFI
			if(getMULTIMode() == dswifi_localnifimode){
				
				return 2;
			}
			
		}
		break;
		
		//shutdown
		case (proc_shutdown):{
			close(SocketFDLocal);
			setMULTIMode(proc_connect);
		}
		break;
		
	}
	
}


//below calls are internal, used by DSWNIFI library. Not for user code
void sendDSWNIFIFame(uint8 * databuf_src,int sizetoSend)
{
	//prevent resending the same packet
	if(sentNow == false){
		//coto: generate crc per nifi frame so we dont end up with corrupted data.
		volatile uint16 crc16_frame = swiCRC16	(	0xffff, //uint16 	crc,
			databuf_src,
			(sizetoSend)		//cant have this own crc here
			);
		
		*(uint16*)(databuf_src + sizetoSend)	= crc16_frame;
		sizetoSend = sizetoSend + sizeof(crc16_frame);
		
		switch(getMULTIMode()){
			case (dswifi_localnifimode):{
				Wifi_RawTxFrame_NIFI(sizetoSend , 0x0014, (unsigned short *)databuf_src);
			}
			break;
			case(dswifi_udpnifimode):
			case(dswifi_tcpnifimode):
			{
				Wifi_RawTxFrame_WIFI(sizetoSend , databuf_src);
			}
			break;
		}
		
		sentNow = true;
	}
}

//reads raw packet (and raw read size) and defines if valid frame or not. 
//must be called from either localnifi or udp nifi/wifi
void receiveDSWNIFIFrame(uint8 * databuf_src,int frameSizeRecv)	//the idea is that we append DSWNIFI Frame + extended frame. so copy the extra data to buffer
{
	//only receive a packet when user code wants
	if(receivedValid == false){
		if(frameSizeRecv > sizeof(recvbufferuser)){
			return ;
		}
		
		//1: check the header localnifi appends, udp nifi/wifi does not append this header
		int frame_hdr_size = 0;	//nifi raw only frame has this header
		int framesize = 0;	//calculated frame size , different from frameSizeRecv
		switch(getMULTIMode()){
			case(dswifi_localnifimode):
			{
				frame_hdr_size = frame_header_size;	//localframe has this header
			}
			break;
			
			case(dswifi_udpnifimode):
			case(dswifi_tcpnifimode):
			{
				frame_hdr_size = 0;					//udp nifi frame has not this header
			}
		}
		//2: calculate frame size
		switch(dswifiSrv.dsnwifisrv_stat){
			//#last:connected!
			case(ds_netplay_host):{	//host receives from guest
				framesize = 3 + sizeof(nifi_cmd) + sizeof(plykeys2) + sizeof(guest_vcount) + sizeof(nifi_keys_sync) + sizeof(guest_framecount);
			}break;
			
			case(ds_netplay_guest):{ //guest receives from host
				framesize = 3 + sizeof(nifi_cmd) + sizeof(host_vcount) + sizeof(plykeys1) + sizeof(host_framecount);
			}break;
		}	
		
		//read crc from nifi frame so we dont end up with corrupted data.
		volatile uint16 crc16_recv_frame = (uint16)*(uint16*)(databuf_src + frame_hdr_size + framesize);
		
		//generate crc per nifi frame so we dont end up with corrupted data.
		volatile uint16 crc16_frame_gen = swiCRC16	(	0xffff, //uint16 	crc,
			(uint8*)(databuf_src + frame_hdr_size),
			(framesize)		//cant have this own crc here
			);
		
		//do crc calc here. if valid, set receivedValid = true; and copy contents to recvbufferuser
		
		//data is now nifi frame
		if(crc16_frame_gen == crc16_recv_frame){
			receivedValid = true;
			
			//only copy the real frame contents
			memcpy ((uint8*)&recvbufferuser[0], (uint8*)(databuf_src + frame_hdr_size), framesize);
			HandleRecvUserspace((uint8*)&recvbufferuser[0], framesize);
			//valid dswnifi_frame
		}
		else{
			//invalid dswnifi_frame
		}
	}
	
}


__attribute__((section(".dtcm")))
uint8 recvbufferuser[512];

__attribute__((section(".dtcm")))
bool receivedValid;	//true = yes, false = no

__attribute__((section(".dtcm")))
bool sentNow;	//true = yes, false = no

//These methods are weak since those are overriden by each project definition.
//they are here, until all the ds - ds comms library goes upstream ToolchainGenericDS
//The project only implements DS - DS Comms here and the rest is handled automatically by ToolchainGenericDS
void HandleSendUserspace(uint8 * databuf_src, int bufsize){
	
	//0-1-2 ID:
	memcpy((uint8*)(databuf_src + 3), (uint8*)&nifi_cmd, sizeof(nifi_cmd));
	int framesize = 0;
	//Sender
	switch(getMULTIMode()){
		case(dswifi_localnifimode):
		{
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
		break;
		
		case(dswifi_udpnifimode):
		case(dswifi_tcpnifimode):
		{
			switch(dswifiSrv.dsnwifisrv_stat){			
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
		break;
		
	}
	
	
	sendDSWNIFIFame(databuf_src,framesize);
	
}

//project specific. todo, copy a whole buffer besides the NIFI variables so below logic can be used
//to receive from usercode: if(receivedValid == true){ copy recvbufferuser to dest }
bool HandleRecvUserspace(uint8 * databuf_src, int bufsize){
	
	int frame_hdr_detected_size = 0;	
	int offset_shared = frame_hdr_detected_size + 3;
			
	switch(getMULTIMode()){
		case(dswifi_localnifimode):
		{
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
		
		case(dswifi_udpnifimode):
		case(dswifi_tcpnifimode):
		{
			//data buffer has come from RX handler already
			memcpy((uint8*)&nifi_cmd, (uint8*)(databuf_src + offset_shared), sizeof(nifi_cmd));
			
			switch(dswifiSrv.dsnwifisrv_stat){
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




//multiplayer key binding/buffer shared code, must run after send/recv packets
bool do_multi()
{
	switch(getMULTIMode()){
		
		//single player
		case(dswifi_idlemode):{
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
		break;
		
		//NIFI local
		case(dswifi_localnifimode):{	//detect nifi_stat here if nifi_stat = 0 for disconnect when nifi was issued
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
						switch(dswifiSrv.dsnwifisrv_mode){
							case dswifi_nifimode:{
								Wifi_RawTxFrame_NIFI(sizeof(nifitoken), 0x0014, (unsigned short *)nifitoken);
							}
							break;
							//case dswifi_tcpnifimode:{
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
						
						switch(dswifiSrv.dsnwifisrv_mode){
							case dswifi_nifimode:{
								Wifi_RawTxFrame_NIFI(sizeof(nificonnect), 0x0014, (unsigned short *)nificonnect);
							}
							break;
							
							//case dswifi_tcpnifimode:{
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
						
						switch(dswifiSrv.dsnwifisrv_mode){
							case dswifi_nifimode:{
								Wifi_RawTxFrame_NIFI(6, 0x0014, (unsigned short *)nificrc);
							}
							break;
							
							//case dswifi_tcpnifimode:{
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
				}	
					break;
				case 6:				//guest...
				{	
					plykeys2 = (uint16)get_joypad();	//plugged bit is sent also
					nifi_keys_sync	=	(plykeys1 ) | ((plykeys2) << 16);
					
					//todo: when nifi_keys_sync is acknowledged (written to IO map), set each plugged bit (0x80000000) as well.
					
				}	break;
			}
			
			return true;
			
		}
		break;
	}
	
	
	return false;
}


#endif