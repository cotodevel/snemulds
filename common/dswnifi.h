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
#ifndef __dswnifi_h__
#define __dswnifi_h__

#include "typedefs.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#ifdef ARM9
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <socket.h>
#include <in.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#endif

//process status
#define proc_idle (sint32)(0)
#define proc_connect (sint32)(1)
#define proc_execution (sint32)(2)
#define proc_shutdown (sint32)(3)

//coto: nifi & wifi support. todo: test in order
#define dswifi_tcpnifimode (sint32)(4)	//TCP
#define dswifi_udpnifimode (sint32)(5)	//UDP Nifi
#define dswifi_localnifimode (sint32)(6)	//Raw Network Packet Nifi
#define dswifi_idlemode (sint32)(7)	//Idle

//special udp nifi/wifi mode
#define ds_multi_notrunning (sint32)(8)
#define ds_searching_for_multi_servernotaware (sint32)(9)	//"srvaware" sends back other NDS info (IP,PORT), then we open PORT socket, and bind it to the new IP, then set flag to ds_wait_for_multi, we listen to new port and close the old one here

#define ds_netplay_host_servercheck (sint32)(10)		//ds are binded at this point but we need to know if both are connected each other
#define ds_netplay_guest_servercheck (sint32)(11)

#define ds_netplay_host (sint32)(12)
#define ds_netplay_guest (sint32)(13)



#include "in.h"
#include "netdb.h"

//struct that spinlocks a current request
typedef struct {
    //dswifi socket @ PORT 8888
    struct sockaddr_in sain_UDP_PORT;			//local nds, any IP takes 127.0.0.1 or current IP @ port 8888 // really used for sending udp packets
	struct sockaddr_in server_addr;		//server: APP UDP companion address	/ unused
    int socket_id__multi_notconnected;	//initial stage required PORT (server -- DS)	/used for ds - server
    
	//listener IP-port
	struct sockaddr_in sain_listener;			//local nds, any IP takes 127.0.0.1 or current IP @ port 8889 (this IP, which is special for each ds console connected)
	int socket_multi_listener;		//multi NDS UDP PORT reserved / used for ds - ds comms
    
	//sender IP-port
	struct sockaddr_in sain_sender;	//ndsmulti IP (second DS ip): UDP multiplayer stores other NDS IP / used from guest mode -> socket_id__multi_connected 
	int socket_multi_sender;		//multi NDS UDP PORT reserved / used for ds - ds comms
    	
	//host socket entry
    struct hostent myhost;
    
    bool wifi_enabled;
    uint8 http_buffer[4 * 1024];
    

} client_http_handler;



//shared memory cant use #ifdef ARMX since corrupts both definition sides for each ARM Core
//---------------------------------------------------------------------------------
typedef struct dsnwifisrvStr {
//---------------------------------------------------------------------------------
	sint32 dsnwifisrv_mode;	//dswifi_idlemode / dswifi_localnifimode / dswifi_udpnifimode / dswifi_tcpnifimode				//used by setMULTIMode() getMULTIMode()
	
	sint32	connectionStatus;	//proc_idle / proc_connect / proc_execution / proc_shutdown	//used by getConnectionStatus() setConnectionStatus()
	sint32 	dsnwifisrv_stat;	//MULTI: inter DS Connect status: ds_multi_notrunning / ds_searching_for_multi / (ds_multiplay): ds_netplay_host ds_netplay_guest
	
	bool dswifi_setup;	//false: not setup / true: setup already
	
	bool incoming_packet;	//when any of the above methods received a packet == true / no == false
}TdsnwifisrvStr;

extern TdsnwifisrvStr dswifiSrv;



#endif


#ifdef __cplusplus
extern "C"{
#endif

extern void switch_dswnifi_mode(sint32 mode);

extern void setMULTIMode(sint32 flag);	//idle(dswifi_idlemode) / raw packet(dswifi_localnifimode) / UDP nifi(dswifi_udpnifimode) / TCP wifi(dswifi_wifimode)
extern sint32 getMULTIMode();			//idle(dswifi_idlemode) / raw packet(dswifi_localnifimode) / UDP nifi(dswifi_udpnifimode) / TCP wifi(dswifi_wifimode)
extern bool getWIFISetup();
extern void setConnectionStatus(sint32 flag);
extern void getConnectionStatus(sint32 flag);

//the process that runs on vblank and ensures DS - DS Comms
extern sint32 doMULTIDaemon();

extern struct sockaddr_in stSockAddrServer;
extern int SocketFDLocal;
extern int SocketFDServer;
extern int port;

extern void request_connection(sint8* str_url,int str_url_size);
extern bool do_multi();

//below calls are internal, used by DSWNIFI library
//frame sender implementation, has all sender-like modes here
extern void sendDSWNIFIFame(uint8 * databuf_src,int sizetoSend);

//frame receiver implementation, has all receiver-like modes here. Returns true if correct frame received from TCP/UDP
extern void receiveDSWNIFIFrame(uint8 * databuf_src,int frameSize);	//framesize is calculated inside (crc over udp requires framesize previously to here calculated anyway)


extern sint8* server_ip;
extern client_http_handler client_http_handler_context;

//below calls are user code
//buffers ToolchainGenericDS offer, so project can read them anytime.
extern bool receivedValid;	//true = yes, false = no
extern bool sentNow;	//true = yes, false = no
extern uint8 recvbufferuser[512];

//these are process the specific project can use to queue (ie: read from non looped program flow, but once a time, to send/receive frames)
//these methods are used for UDP NIFI/WIFI or local NIFI,
extern void HandleSendUserspace(uint8 * buf, int bufsize);
extern bool HandleRecvUserspace(uint8 * buf, int bufsize);


#ifdef __cplusplus
}
#endif