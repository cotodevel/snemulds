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

//DSWNifi Library 1.4 (update: 3/11/2019)	(dd/mm/yyyy)

// Shared
#include "dswnifi_lib.h"

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#include "wifi_shared.h"

// ARM7
#ifdef ARM7

#endif

// ARM9
#ifdef ARM9

#include "timerTGDS.h"
#include "keyboard.h"
#include "sgIP_sockets.h"
#include "nds_cp15_misc.h"

__attribute__((section(".dtcm")))
struct dsnwifisrvStr dswifiSrv;

volatile 	uint8 data[4096];			//receiver frame, data + frameheader. Used by NIFI Mode
void Handler(int packetID, int readlength){
	switch(getMULTIMode()){
		case (dswifi_localnifimode):{
			Wifi_RxRawReadPacket(packetID, readlength, (unsigned short *)data);
			struct frameBlock * frameHandled = receiveDSWNIFIFrame((uint8*)(data+frame_header_size),readlength);	//sender always cut off 2 bytes for crc16 we use	
			if(frameHandled != NULL){	//LOCAL:Valid Frame?, then User Recv Process here
				TGDSRecvHandler(frameHandled);
			}
		}
		break;
		//case (dswifi_udpnifimode): DSWNIFI uses its own daemon to process input/output frames
	}
}


void initNiFi(){
	Wifi_InitDefault(false);
	Wifi_SetPromiscuousMode(1);
	Wifi_RawSetPacketHandler(Handler);
	Wifi_SetChannel(10);
	
	if(1){
		DisableIrq(IRQ_TIMER3);
		TIMERXDATA(3) = -1310; // 1310 * 256 cycles = ~10ms;
		TIMERXCNT(3) = 0x00C2; // enable, irq, 1/256 clock
		EnableIrq(IRQ_TIMER3);
	}
	Wifi_EnableWifi();
}

/////////////////////////////////UDP DSWNIFI Part////////////////////////////////////
sint8 server_ip[MAX_TGDSFILENAME_LENGTH+1]; //(sint8*)"192.168.43.220";
//SOCK_STREAM = TCP / SOCK_DGRAM = UDP

//true == sends and frees the queued frame / false == didn't send, no frame
bool sendDSWNIFIFame(struct frameBlock * frameInst){
	uint8 * databuf_src = 	frameInst->framebuffer;
	sint32 sizetoSend 		= 	frameInst->frameSize;
	volatile uint16 crc16_frame = swiCRC16	(	0xffff, //uint16 	crc,
		(uint8*)databuf_src,
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
		{
			Wifi_RawTxFrame_WIFI(sizetoSend , databuf_src);
		}
		break;
	}
	return true;
}

__attribute__((section(".dtcm")))
struct frameBlock FrameSenderBlock;	//used by the user sender process, must be valid so the ToolchainGenericDS library sends proper frame data.

__attribute__((section(".dtcm")))
struct frameBlock FrameRecvBlock;	//used by the user receiver process, can be NULL if no data frame was received.

//reads raw packet (and raw read size) and defines if valid frame or not. 
//must be called from either localnifi handler or udp nifi/wifi on-receive-packet handler
struct frameBlock * receiveDSWNIFIFrame(uint8 * databuf_src, int frameSizeRecv){
	struct frameBlock * frameRecvInst =  NULL;
	if(databuf_src != NULL){
		volatile uint16 crc16_recv_frame = 0;
		int frame_hdr_size = 0;	//nifi raw only frame has this header
		switch(getMULTIMode()){
			case(dswifi_localnifimode):
			{
				frame_hdr_size = frame_header_size;	//localframe has this header
				frameSizeRecv-=frame_hdr_size;
			}
			break;
			case(dswifi_udpnifimode):
			{
				frame_hdr_size = 0;					//udp nifi frame has not this header
				frameSizeRecv-=frame_hdr_size;
			}
		}
		crc16_recv_frame = *(uint16*)(databuf_src + frameSizeRecv - (int)sizeof(volatile uint16));
		volatile uint16 crc16_frame_gen = swiCRC16	(	
			0xffff, //uint16 	crc,
			(databuf_src),
			(frameSizeRecv - (int)sizeof(volatile uint16))
		);
		
		//crc-valid dswnifi_frame?
		if(crc16_frame_gen == crc16_recv_frame){
			frameRecvInst = (struct frameBlock *)&FrameRecvBlock;
			frameRecvInst->framebuffer = databuf_src-1;
			frameRecvInst->frameSize = frameSizeRecv;
		}
	}
	return frameRecvInst;
}

//if IDLE: returns true always, 
//if UDPNIFI connect OK: true, otherwise false. 
//if NIFI: returns true always
//if GDBSTUB: connect OK: true, otherwise false.
bool switch_dswnifi_mode(sint32 mode){
	//save last DSWnifiMode
	if(mode != LastDSWnifiMode){
		LastDSWnifiMode = mode;
	}
	//Raw Network Packet Nifi
	if (mode == (sint32)dswifi_localnifimode){
		if(connectDSWIFIAP(DSWNIFI_ENTER_NIFIMODE) == true){	//setWIFISetup set inside
			dswifiSrv.dsnwifisrv_stat	= ds_searching_for_multi_servernotaware;
			setMULTIMode(mode);
			setConnectionStatus(proc_connect);
			OnDSWIFIlocalnifiEnable();
			return true;
		}
		else{
			//Could not connect
			switch_dswnifi_mode(dswifi_idlemode);
			return false;
		}
	}
	//UDP Nifi (wifi AP)
	else if (mode == (sint32)dswifi_udpnifimode){
		dswifiSrv.dsnwifisrv_stat = ds_searching_for_multi_servernotaware;
		bool useWIFIAP = true;
		if(connectDSWIFIAP(DSWNIFI_ENTER_WIFIMODE) == true){	//setWIFISetup set inside
			setConnectionStatus(proc_connect);
			setMULTIMode(mode);
			OnDSWIFIudpnifiEnable();
			return true;
		}
		else{
			//Could not connect
			switch_dswnifi_mode(dswifi_idlemode);
			return false;
		}
	}
	//GDBStub mode
	else if (mode == (sint32)dswifi_gdbstubmode){
		dswifiSrv.dsnwifisrv_stat	= ds_multi_notrunning;
		if (gdbNdsStart() == true){	//setWIFISetup set inside
			setMULTIMode(mode);
			OnDSWIFIGDBStubEnable();
			return true;
		}
		else{
			//Could not connect
			switch_dswnifi_mode(dswifi_idlemode);
			return false;
		}
	}
	//idle mode minimal setup
	else if (mode == (sint32)dswifi_idlemode){
		dswifiSrv.dsnwifisrv_stat	= ds_multi_notrunning;
		setMULTIMode(mode);
		setWIFISetup(false);
		setConnectionStatus(proc_shutdown);
		resetGDBSession();	//Requires all TGDS projects to start as dswifi_idlemode!!
		connectDSWIFIAP(DSWNIFI_ENTER_IDLEMODE);
		OnDSWIFIidlemodeEnable();
	}
	return true;
}


//Used for NIFI server not aware req.
bool sentReq = false;
struct frameBlock * FrameSenderUser = NULL;
__attribute__((section(".dtcm")))
sint32 LastDSWnifiMode = dswifi_idlemode;

//DSWNifi Daemon: Performs connect, execute and disconnect phases of a local/udp session between DS's (for now it's 2 DS)
//ret: dswifi_idlemode (not connected) / dswifi_udpnifimode (UDP NIFI) / dswifi_localnifimode (LOCAL NIFI)

sint32 doMULTIDaemonStage1(){
	sint32 DSWnifiConnectionStatus =	getConnectionStatus();
	if(DSWnifiConnectionStatus == proc_idle){
		return dswifi_idlemode;	//nothing to do for : LOCAL / UDP NIFI
	}
	else{
		return doMULTIDaemonStage2(DSWnifiConnectionStatus);
	}
}

bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

//
sint32 doMULTIDaemonStage2(sint32 ThisConnectionStatus){
	sint32 retDaemonCode = 0;
	switch(ThisConnectionStatus){
		case (proc_connect):{
			//UDP NIFI
			if(getMULTIMode() == dswifi_udpnifimode){
				
				//Ask for Server Input IP here:
				if(globalTGDSCustomConsole == false){
					bool isDirectFramebuffer = false;
					bool disableTSCWhenTGDSConsoleTop = false;
					u8* currentVRAMContext = (u8*)TGDSARM9Malloc(128*1024);
					bool SaveConsoleContext = true; //Save Console context
					TGDSLCDSwap(disableTSCWhenTGDSConsoleTop, isDirectFramebuffer, SaveConsoleContext, currentVRAMContext);
					printf("Connecting to UDP TGDS Server Companion: ");
					printf("Write down the IP then <Enter>, or tap <ESC> to quit. ");
					
					//Start keyboard
					initKeyboard();	
					SETDISPCNT_SUB(MODE_0_2D | DISPLAY_BG0_ACTIVE);	//Keyboard show
					char str[MAX_TGDSFILENAME_LENGTH+1] = "";
					bool validIPv4 = false;
					while(1==1){
						char keyPress = processKeyboard(&str[0], MAX_TGDSFILENAME_LENGTH+1, ECHO_ON, (int)strlen(str) );
						if(keyPress == (char)RET)
						{
							// process input
							validIPv4 = isValidIpAddress(str);
							if(validIPv4 == true){
								break;
							}
							printf("Invalid IP: %s",str);
							strcpy(str, "");	//clean string if invalid
						}
						else if (keyPress == (char)ESC){
							break;
						}
					}
					SaveConsoleContext = false; //Restore Console context
					TGDSLCDSwap(disableTSCWhenTGDSConsoleTop, isDirectFramebuffer, SaveConsoleContext, currentVRAMContext);
					TGDSARM9Free(currentVRAMContext);
					if(validIPv4 == false){					
						printf("Invalid IP address");
						return dswifi_udpnifimodeFailConnectStage;
					}
					strcpy(server_ip,str);
				}
				else{
					//special console code: todo
					bool isTGDSCustomConsole = false;	//set default console or custom console: default console
					GUI_init(isTGDSCustomConsole);
					GUI_clear();
					
					printf("UDP Nifi mode keyboard support is not yet ");
					printf("available for a custom TGDS video setting. ");
					return dswifi_udpnifimodeFailConnectStage;
				}
				
				//UDP: DSClient - Server IP / Desktop Server UDP companion Listener.
				dswifiSrv.client_http_handler_context.socket_id__multi_notconnected=socket(AF_INET,SOCK_DGRAM,0);	
				int i=1;
				i=ioctl(dswifiSrv.client_http_handler_context.socket_id__multi_notconnected,FIONBIO,&i); // set non-blocking port
				memset((uint8*)&dswifiSrv.client_http_handler_context.sain_UDP_PORT, 0, sizeof(dswifiSrv.client_http_handler_context.sain_UDP_PORT));
				dswifiSrv.client_http_handler_context.sain_UDP_PORT.sin_family=AF_INET;
				dswifiSrv.client_http_handler_context.sain_UDP_PORT.sin_addr.s_addr=0;
				dswifiSrv.client_http_handler_context.sain_UDP_PORT.sin_port=htons((int)UDP_PORT);
				if(bind(dswifiSrv.client_http_handler_context.socket_id__multi_notconnected,(struct sockaddr *)&dswifiSrv.client_http_handler_context.sain_UDP_PORT,sizeof(dswifiSrv.client_http_handler_context.sain_UDP_PORT))) {
					//binding ERROR
					close(dswifiSrv.client_http_handler_context.socket_id__multi_notconnected);
					retDaemonCode = dswifi_udpnifimodeFailConnectStage;
				}
				else{
					//binding OK
				}
				//UDP: DSClient - Server IP / Desktop Server UDP companion Sender
				memset((uint8*)&dswifiSrv.client_http_handler_context.server_addr, 0, sizeof(struct sockaddr_in));
				dswifiSrv.client_http_handler_context.server_addr.sin_family = AF_INET;
				dswifiSrv.client_http_handler_context.server_addr.sin_addr.s_addr = inet_addr(server_ip);
				dswifiSrv.client_http_handler_context.server_addr.sin_port = htons((int)UDP_PORT);
				//no binding since we have no control of server port and we should not know it anyway, DGRAM specific.
				
				setConnectionStatus(proc_execution);
				retDaemonCode = dswifi_udpnifimode;
			}
			//LOCAL NIFI: runs on DSWIFI process
			if(getMULTIMode() == dswifi_localnifimode){
				setConnectionStatus(proc_execution);
				retDaemonCode = dswifi_localnifimode;
			}
		}
		break;
		
		case (proc_execution):{
			//////////////////////////////////////////Handle Internal Send/Recv
			//Server UDP Handler listener
			volatile uint8 incomingbuf[frameDSsize] = {0};
			int datalen = 0;
			struct sockaddr_in sender_server;
			int sain_len = 0;
			//UDP NIFI
			if(getMULTIMode() == dswifi_udpnifimode){
				//UDP: (execute RPC from server and process frames)
				switch(dswifiSrv.dsnwifisrv_stat){
					//#1 DS is not connected, serve any upcoming commands related to non connected to multi
					case(ds_searching_for_multi_servernotaware):{
						sain_len = sizeof(struct sockaddr_in);
						memset((uint8*)&sender_server, 0, sizeof(struct sockaddr_in));
						uint8 cmd[12] = {0};
						if(sentReq == false){		
							int k = 0;
							for(k = 0; k < 4; k++){
								char outgoingbuf[frameDSsize] = {0};
								char IP[16];
								sprintf(outgoingbuf,"dsnotaware-NIFINintendoDS-%s-",print_ip((uint32)Wifi_GetIP(), IP));
								sendto(dswifiSrv.client_http_handler_context.socket_id__multi_notconnected,outgoingbuf,strlen(outgoingbuf),0,(struct sockaddr *)&dswifiSrv.client_http_handler_context.server_addr,sizeof(struct sockaddr_in));
								swiDelay(8888);
							}
							sentReq = true;	//acknowledge DS send
						}
						if( (datalen = recvfrom(dswifiSrv.client_http_handler_context.socket_id__multi_notconnected, (uint8*)incomingbuf, sizeof(incomingbuf), 0, (struct sockaddr *)&sender_server,(int*)&sain_len)) > 0 ){
							if(datalen>0) {
								memcpy((uint8*)cmd,(uint8*)incomingbuf,sizeof(cmd));	//cmd recv
								cmd[sizeof(cmd)-1] = '\0';
							}
						}
						
						//Server aware
						if(strncmp((const char *)cmd, (const char *)"srvaware", 8) == 0){
							char * token_hostguest = (char*)&outSplitBuf[2][0];	//host or guest
							char * token_extip = (char*)&outSplitBuf[1][0];	//external NDS ip to connect
							char * cmdRecv = (char*)&outSplitBuf[0][0];	//cmd
							str_split((char*)incomingbuf, "-", NULL);
							int host_mode = strncmp((const char*)token_hostguest, (const char *)"host", 4); 	//host == 0
							int guest_mode = strncmp((const char*)token_hostguest, (const char *)"guest", 5); 	//guest == 0
							
							dswifiSrv.client_http_handler_context.socket_multi_listener=socket(AF_INET,SOCK_DGRAM,0);
							int cmd=1;
							cmd=ioctl(dswifiSrv.client_http_handler_context.socket_multi_listener,FIONBIO,&cmd); // set non-blocking port
							
							dswifiSrv.client_http_handler_context.socket_multi_sender=socket(AF_INET,SOCK_DGRAM,0);
							int optval = 1;
							setsockopt(dswifiSrv.client_http_handler_context.socket_multi_sender, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval));
		
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
							memset((char *)&dswifiSrv.client_http_handler_context.sain_listener, 0, sizeof(struct sockaddr_in));
							dswifiSrv.client_http_handler_context.sain_listener.sin_family = AF_INET;
							dswifiSrv.client_http_handler_context.sain_listener.sin_addr.s_addr = htonl(INADDR_ANY);	//local/any ip listen to desired port
							dswifiSrv.client_http_handler_context.sain_listener.sin_port = htons(LISTENER_PORT); //nds_multi_port
							
							//NDS MULTI IP: No need to bind / connect since it's datagram (UDP)
							memset((char *) &dswifiSrv.client_http_handler_context.sain_sender, 0, sizeof(struct sockaddr_in));
							dswifiSrv.client_http_handler_context.sain_sender.sin_family = AF_INET;
							dswifiSrv.client_http_handler_context.sain_sender.sin_addr.s_addr = inet_addr((char*)token_extip);
							dswifiSrv.client_http_handler_context.sain_sender.sin_port = htons(SENDER_PORT); 
							
							//bind ThisIP(each DS network hardware) against the current DS UDP port
							if(bind(dswifiSrv.client_http_handler_context.socket_multi_listener,(struct sockaddr *)&dswifiSrv.client_http_handler_context.sain_listener,sizeof(struct sockaddr_in))) {
								if(host_mode == 0){
									//[host]binding error
								}
								else if(guest_mode == 0){
									//[guest]binding error
								}
								close(dswifiSrv.client_http_handler_context.socket_multi_listener);
								retDaemonCode = dswifi_udpnifimodeFailExecutionStage;
							}
							else{
								//read IP from sock interface binded
								if(host_mode == 0){
									//stop sending data, server got it already.
									dswifiSrv.dsnwifisrv_stat = ds_netplay_host_servercheck;
								}
								else if(guest_mode == 0){
									//stop sending data, server got it already.
									dswifiSrv.dsnwifisrv_stat = ds_netplay_guest_servercheck;
								}
								sentReq = false;	//prepare next issuer msg
							}	
						}
					}
					break;
					
					//servercheck phase acknow
					case(ds_netplay_host_servercheck):case(ds_netplay_guest_servercheck):{
						if(sentReq == false){
							//check pending receive
							int LISTENER_PORT 	=	0;
							char status[10] = {0};
							if(dswifiSrv.dsnwifisrv_stat == ds_netplay_host_servercheck){
								LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_HOST;
								sprintf(status,"host");
							}
							else if(dswifiSrv.dsnwifisrv_stat == ds_netplay_guest_servercheck){
								LISTENER_PORT 	= 	(int)NDSMULTI_UDP_PORT_GUEST;
								sprintf(status,"guest");
							}
							char buf2[frameDSsize] = {0};
							char IP[16];
							sprintf(buf2,"dsaware-%s-bindOK-%d-%s-", status, LISTENER_PORT, print_ip((uint32)Wifi_GetIP(), IP));
							sendto(dswifiSrv.client_http_handler_context.socket_id__multi_notconnected,buf2,sizeof(buf2),0,(struct sockaddr *)&dswifiSrv.client_http_handler_context.server_addr,sizeof(struct sockaddr_in));
							sentReq = true;	//acknowledge DS send
						}
						uint8 cmd[12] = {0};	//srv->ds command handler
						sain_len = sizeof(struct sockaddr_in);
						memset((uint8*)&sender_server, 0, sizeof(struct sockaddr_in));
						datalen=recvfrom(dswifiSrv.client_http_handler_context.socket_id__multi_notconnected,(uint8*)incomingbuf,sizeof(incomingbuf),0,(struct sockaddr *)&sender_server,(int*)&sain_len);
						if(datalen>0) {
							memcpy((uint8*)cmd,(uint8*)incomingbuf,sizeof(cmd));	//cmd recv
							cmd[sizeof(cmd)-1] = '\0';
						}
						if(strncmp((const char *)cmd, (const char *)"dsconnect", 9) == 0){
							if(dswifiSrv.dsnwifisrv_stat == ds_netplay_host_servercheck){	
								dswifiSrv.dsnwifisrv_stat = ds_netplay_host;
							}
							else if(dswifiSrv.dsnwifisrv_stat == ds_netplay_guest_servercheck){
								dswifiSrv.dsnwifisrv_stat = ds_netplay_guest;
							}
							close(dswifiSrv.client_http_handler_context.socket_id__multi_notconnected); //closer server socket to prevent problems when udp multiplayer
						}
					}
					break;
					
					//#last:connected!
					//logic: recv data: frameDSsize from port
					case(ds_netplay_host):case(ds_netplay_guest):{						
						sain_len = sizeof(struct sockaddr_in);
						memset((uint8*)&sender_server, 0, sizeof(struct sockaddr_in));
						int datalen2=recvfrom(dswifiSrv.client_http_handler_context.socket_multi_listener,(uint8*)incomingbuf,sizeof(incomingbuf),0,(struct sockaddr *)&sender_server,(int*)&sain_len);
						if(datalen2>0) {
							//decide whether to put data in userbuffer and if frame is valid here
							struct frameBlock * frameHandled = receiveDSWNIFIFrame((uint8*)incomingbuf,datalen2);
							//Valid Frame?
							if(frameHandled != NULL){
								//trigger the User Recv Process here
								TGDSRecvHandler(frameHandled);
							}
						}
					}
					break;
				}
				retDaemonCode = dswifi_udpnifimode;
				
				///////////////////////////////////////Handle Send Library
				// Send Frame UserCore
				if(FrameSenderUser){
					sendDSWNIFIFame(FrameSenderUser);
					FrameSenderUser = NULL;
				}
			}
			
			//Local Nifi: runs from within the DSWIFI frame handler itself so ignored here.
			if(getMULTIMode() == dswifi_localnifimode){
				retDaemonCode = dswifi_localnifimode;
			}
		}
		break;
		//shutdown
		case (proc_shutdown):{
			if(LastDSWnifiMode == dswifi_udpnifimode){
				close(dswifiSrv.client_http_handler_context.socket_id__multi_notconnected);
			}
			sentReq = false;
			setConnectionStatus(proc_idle);
			retDaemonCode = dswifi_idlemode;
		}
		break;
	}
	return retDaemonCode;
}

// datalen = size of packet from beginning of 802.11 header to end, but not including CRC.
int Wifi_RawTxFrame_NIFI(sint32 datalen, uint16 rate, uint16 * data) {
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

int Wifi_RawTxFrame_WIFI(sint32 datalen, uint8 * data) {
	switch(dswifiSrv.dsnwifisrv_stat){	//sender phase: only send packets if we are in UDP DSWNifi mode
		case(ds_netplay_host):case(ds_netplay_guest):{
			if(getMULTIMode() == dswifi_udpnifimode){
				sendto(dswifiSrv.client_http_handler_context.socket_multi_sender,data,datalen,0,(struct sockaddr *)&dswifiSrv.client_http_handler_context.sain_sender,sizeof(dswifiSrv.client_http_handler_context.sain_sender));
			}
		}
		break;
	}
	return 0;
}

//UserCodeHandlers that belong to TGDS
struct frameBlock * HandleSendUserspace(uint8 * databuf_src, int bufsize){
	if(!databuf_src){
		return NULL;
	}
	if( (bufsize - (int)sizeof(volatile uint16)) <= 0){
		return NULL;
	}
	bufsize -= sizeof(volatile uint16);
	struct frameBlock * frameSenderInst =  (struct frameBlock *)&FrameSenderBlock;
	frameSenderInst->framebuffer = databuf_src;
	frameSenderInst->frameSize = bufsize;
	if(getMULTIMode() == dswifi_localnifimode){	
		sendDSWNIFIFame(frameSenderInst);
	}
	return frameSenderInst;
}

struct dsnwifisrvStr * getDSWNIFIStr(){
	return (struct dsnwifisrvStr *)&dswifiSrv;
}

__attribute__((section(".itcm")))
bool TGDSRecvHandler(struct frameBlock * frameBlockRecv){
	//frameBlockRecv->framebuffer	//Pointer to received Frame
	//frameBlockRecv->frameSize		//Size of received Frame
	int DSWnifiMode = getMULTIMode();
	switch(DSWnifiMode){
		//single player, has no access to shared buffers.
		case(dswifi_idlemode):{
			//DSWNIFIStatus:SinglePlayer
			
			return false;
		}
		break;
		
		//NIFI local
		case(dswifi_localnifimode):{
			struct dsnwifisrvStr * dsnwifisrvStrInstWireless = (struct dsnwifisrvStr *)frameBlockRecv->framebuffer;	
			//Sender command implementation
			
			switch(dsnwifisrvStrInstWireless->nifiCommand){				
				//TotalDSConnected: Sender cmd
				case(NIFI_SENDER_TOTAL_CONNECTED_DS):{
					//Handle cmd
					int TotalCount = 1;
					u32 * shBuf = (u32*)&dsnwifisrvStrInstWireless->sharedBuffer[0];
					shBuf[0] = (u32)TotalCount;
					dsnwifisrvStrInstWireless->nifiCommand = NIFI_ACK_TOTAL_CONNECTED_DS;
					FrameSenderUser = HandleSendUserspace((uint8*)dsnwifisrvStrInstWireless, frameBlockRecv->frameSize);
				}
				break;
				//TotalDSConnected: Process Recv -> Sender ACK cmd
				case(NIFI_ACK_TOTAL_CONNECTED_DS):{
					struct dsnwifisrvStr * dsnwifisrvStrInst = getDSWNIFIStr();
					memcpy((u8*)&dsnwifisrvStrInst->sharedBuffer[0], (u8*)&dsnwifisrvStrInstWireless->sharedBuffer[0], sizeof(dsnwifisrvStrInst->sharedBuffer));
					dsnwifisrvStrInst->nifiCommand = dsnwifisrvStrInstWireless->nifiCommand;
				}
				break;
				//SendBinary: Sender cmd
				case(NIFI_SENDER_SEND_BINARY):{
					struct dsnwifisrvStr * dsnwifisrvStrInst = getDSWNIFIStr();
					dsnwifisrvStrInst->frameIndex = dsnwifisrvStrInstWireless->frameIndex;
					dsnwifisrvStrInst->BinarySize = dsnwifisrvStrInstWireless->BinarySize;
					memcpy((u8*)&dsnwifisrvStrInst->sharedBuffer[0], (u8*)&dsnwifisrvStrInstWireless->sharedBuffer[0], sizeof(dsnwifisrvStrInst->sharedBuffer));
					dsnwifisrvStrInst->nifiCommand = dsnwifisrvStrInstWireless->nifiCommand;	//wait for recv in main() to copy these arriving packets					
				}
				break;
				//SendBinary: Process Recv -> Sender ACK cmd
				case(NIFI_ACK_SEND_BINARY):{
					struct dsnwifisrvStr * dsnwifisrvStrInst = getDSWNIFIStr();
					dsnwifisrvStrInst->nifiCommand = dsnwifisrvStrInstWireless->nifiCommand;	//SendDSBinary() continue next frame
				}
				break;
				//SendBinary: Process Recv -> Sender Finish transfer cmd
				case(NIFI_ACK_SEND_BINARY_FINISH):{
					struct dsnwifisrvStr * dsnwifisrvStrInst = getDSWNIFIStr();
					dsnwifisrvStrInst->nifiCommand = dsnwifisrvStrInstWireless->nifiCommand;	//wait for recv in main() to copy these arriving packets
				}
				break;
				
				default:{
					return TGDSRecvHandlerUser(frameBlockRecv, DSWnifiMode);
				}
				break;
			}
			return true;
		}
		break;
		
		//UDP NIFI
		case(dswifi_udpnifimode):{
			//Todo: replicate the same stuff from LocalNifi
			return true;
		}
		break;
		
	}
	return false;
}

/////////////////////////////////GDB Server stub Part////////////////////////////////////

// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

int (*remoteSendFnc)(char *, int) = NULL;
int (*remoteRecvFnc)(char *, int) = NULL;
bool (*remoteInitFnc)() = NULL;
void (*remoteCleanUpFnc)() = NULL;

int remotePort = 55555;
int remoteSignal = 5;
int remoteSocket = -1;
int remoteListenSocket = -1;
bool remoteConnected = false;
bool remoteResumed = false;
int reconnectCount = 0;

//this one performs the actual connection and toggles the connected status.
bool connectDSWIFIAP(int DSWNIFI_MODE){
	if(WNifi_InitSafeDefault(DSWNIFI_MODE) == true){
		setWIFISetup(true);
		return true;
	}
	setWIFISetup(false);
	return false;
}

bool gdbNdsStart(){
	dswifiSrv.GDBStubEnable = false;
	if(connectDSWIFIAP(DSWNIFI_ENTER_WIFIMODE) == true){	//GDB Requires the DS Wifi to enter wifi mode
		dswifiSrv.GDBStubEnable = true;
		return true;
	}
	return false;
}

int remoteTcpSend(char *data, int len)
{
	return send(remoteSocket, data, len, 0);
}

int remoteTcpRecv(char *data, int len)
{
	return recv(remoteSocket, data, len, 0);
}

bool remoteTcpInit()
{
	if(remoteSocket == -1) {
		int s = socket(PF_INET, SOCK_STREAM, 0);
		remoteListenSocket = s;
		if(s < 0) {
			//printf("Error opening socket ");
			while(1==1){}
		}
		int tmp = 1;
		setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *) &tmp, sizeof (tmp));
	
		//    char hostname[MAX_TGDSFILENAME_LENGTH+1];
		//    gethostname(hostname, MAX_TGDSFILENAME_LENGTH+1);

		//    hostent *ent = gethostbyname(hostname);
		//    unsigned long a = *((unsigned long *)ent->h_addr);

		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(remotePort);
		addr.sin_addr.s_addr = htonl(0);
		
		if(bind(s, (struct sockaddr *)&addr, sizeof(addr))) {
			addr.sin_port = htons(ntohs(addr.sin_port)+1);
		}
		
		//printf("Listening for a connection at port %d ",ntohs(addr.sin_port));
		if(listen(s, 1)) {
			//printf("Error listening ");
			while(1==1){}
		}
		int len = sizeof(addr);	//socklen_t

		int s2 = accept(s, (struct sockaddr *)&addr, &len);
		if(s2 > 0) {
			//printf("Got a connection from %s %d ",inet_ntoa((in_addr)addr.sin_addr),ntohs(addr.sin_port));
		}
		
		char dummy;
		recv(s2, &dummy, 1, 0);
		if(dummy != '+') {
			//GDB Server: ACK NOT received
		}else{
			//GDB Server: ACK RECEIVED
		}
		
		remoteSocket = s2;
		//    close(s);
	}
	return true;
}

void remoteTcpCleanUp()
{
	if(remoteSocket > 0) {
		//Closing remote socket
		close(remoteSocket);
		remoteSocket = -1;
	}
	if(remoteListenSocket > 0) {
		//Closing listen socket
		close(remoteListenSocket);
		remoteListenSocket = -1;
	}
}

int remotePipeSend(char *data, int len)
{
	int res = write(1, data, len);
	return res;
}

int remotePipeRecv(char *data, int len)
{
	int res = read(0, data, len);
	return res;
}

bool remotePipeInit()
{
	char dummy;
	read(0, &dummy, 1);
	if(dummy != '+') {
		//GDB Server: ACK not received
		while(1==1);
	}
	return true;
}

void remotePipeCleanUp()
{
}

void remoteSetPort(int port)
{
	remotePort = port;
}

void remoteSetProtocol(int p)
{
  if(p == 0) {
    remoteSendFnc = remoteTcpSend;
    remoteRecvFnc = remoteTcpRecv;
    remoteInitFnc = remoteTcpInit;
    remoteCleanUpFnc = remoteTcpCleanUp;
  } else {
    remoteSendFnc = remotePipeSend;
    remoteRecvFnc = remotePipeRecv;
    remoteInitFnc = remotePipeInit;
    remoteCleanUpFnc = remotePipeCleanUp;
  }
}

void remoteInit()
{
	remoteSetProtocol(0);	//use TCP
	if(remoteInitFnc){
		remoteInitFnc();
	}
}

void remotePutPacket(char *packet)
{
  char *hex = "0123456789abcdef";
  char * buffer = (char *)TGDSARM9Malloc(1024);
	
  int count = strlen(packet);
  unsigned char csum = 0;
  char *p = buffer;
  *p++ = '$';
	
	int i = 0;
  for(i = 0 ;i < count; i++) {
    csum += packet[i];
    *p++ = packet[i];
  }
  *p++ = '#';
  *p++ = hex[csum>>4];
  *p++ = hex[csum & 15];
  *p++ = 0;
  //  //printf("Sending %s\n", buffer);
  remoteSendFnc(buffer, count + 4);

  char c = 0;
  remoteRecvFnc(&c, 1);
  /*
  if(c == '+')
    //printf("ACK\n");
  else if(c=='-')
    //printf("NACK\n");
  */
  TGDSARM9Free(buffer);
}



void remoteOutput(char *s, u32 addr)
{
  char * buffer = (char *)TGDSARM9Malloc(16384);
  char *d = buffer;
  *d++ = 'O';

  if(s) {
    char c = *s++;
    while(c) {
      sprintf(d, "%02x", c);
      d += 2;
      c = *s++;
    }
  } else {
    char c= debuggerReadByte(addr);
    addr++;
    while(c) {
      sprintf(d, "%02x", c);
      d += 2;
      c = debuggerReadByte(addr);
      addr++;
    }
  }
  remotePutPacket(buffer);
  TGDSARM9Free(buffer);
}

void remoteSendSignal()
{
    char * buffer = (char *)TGDSARM9Malloc(1024);
	sprintf(buffer, "S%02x", remoteSignal);
	remotePutPacket(buffer);
	TGDSARM9Free(buffer);
}

void remoteSendStatus()
{
  char * buffer = (char *)TGDSARM9Malloc(1024);
  sprintf(buffer, "T%02x", remoteSignal);
  char *s = buffer;
  s += 3;
  int i = 0;
  for( i = 0; i < 15; i++) {
    u32 v = i+(0x08000000);	//mockup ARM Core Registers
    sprintf(s, "%02x:%02x%02x%02x%02x;",i,
            (v & 255),
            (v >> 8) & 255,
            (v >> 16) & 255,
            (v >> 24) & 255);
    s += 12;
  }
  u32 v = (0x08000000);	//Next ARM PC
  sprintf(s, "0f:%02x%02x%02x%02x;", (v & 255),
          (v >> 8) & 255,
          (v >> 16) & 255,
          (v >> 24) & 255);
  s += 12;
  //CPUUpdateCPSR();
  v = 0;	//reg[16].I;	//read CPSR
  sprintf(s, "19:%02x%02x%02x%02x;", (v & 255),
          (v >> 8) & 255,
          (v >> 16) & 255,
          (v >> 24) & 255);
  s += 12;
  *s = 0;
  //  //printf("Sending %s\n", buffer);
  remotePutPacket(buffer);
  TGDSARM9Free(buffer);
}

void remoteBinaryWrite(char *p)
{
  u32 address = 0;
  int count = 0;
  sscanf(p,"%x,%d:", &address, &count);
  //  //printf("Binary write for %08x %d\n", address, count);

  p = strchr(p, ':');
  p++;
  int i = 0 ;
  for( i = 0; i < count; i++) {
    u8 b = *p++;
    switch(b) {
    case 0x7d:
      b = *p++;
      debuggerWriteByte(address, (b^0x20));
      address++;
      break;
    default:
      debuggerWriteByte(address, b);
      address++;
      break;
    }
  }
  //  //printf("ROM is %08x\n", debuggerReadMemory(0x8000254));
  remotePutPacket("OK");
}

void remoteMemoryWrite(char *p)
{
  u32 address = 0;
  int count = 0;
  sscanf(p,"%x,%d:", &address, &count);
  //  //printf("Memory write for %08x %d\n", address, count);

  p = strchr(p, ':');
  p++;
  int i = 0;
  for( i = 0; i < count; i++) {
    u8 v = 0;
    char c = *p++;
    if(c <= '9')
      v = (c - '0') << 4;
    else
      v = (c + 10 - 'a') << 4;
    c = *p++;
    if(c <= '9')
      v += (c - '0');
    else
      v += (c + 10 - 'a');
    debuggerWriteByte(address, v);
    address++;
  }
  //  //printf("ROM is %08x\n", debuggerReadMemory(0x8000254));
  remotePutPacket("OK");
}

void remoteMemoryRead(char *p)
{
  u32 address = 0;
  int count = 0;
  sscanf(p,"%x,%d:", &address, &count);
  
  char * buffer = (char *)TGDSARM9Malloc(1024);
  char *s = buffer;
  int i = 0;
  for( i = 0; i < count; i++) {
    u8 b = debuggerReadByte(address);
    sprintf(s, "%02x", b);
    address++;
    s += 2;
  }
  *s = 0;
  remotePutPacket(buffer);
  TGDSARM9Free(buffer);
}

void remoteStepOverRange(char *p)
{
  u32 address = 0;
  u32 final = 0;
  sscanf(p, "%x,%x", &address, &final);
  remotePutPacket("OK");
  remoteResumed = true;
  
  /*
  do {
    CPULoop(1);
    if(debugger)
      break;
  } while(armNextPC >= address && armNextPC < final);
	*/
  remoteResumed = false;
  remoteSendStatus();
}

void remoteWriteWatch(char *p, bool active)
{
  u32 address = 0;
  int count = 0;
  sscanf(p, ",%x,%d#", &address, &count);

  //printf("Write watch for %08x %d ", address, count);

  if(address < 0x2000000 || address > 0x3007fff) {
    remotePutPacket("E01");
    return;
  }

  if(address > 0x203ffff && address < 0x3000000) {
    remotePutPacket("E01");
    return;
  }

  u32 final = address + count;

  if(address < 0x2040000 && final > 0x2040000) {
    remotePutPacket("E01");
    return;
  } else if(address < 0x3008000 && final > 0x3008000) {
    remotePutPacket("E01");
    return;
  }
	int i = 0;
  for( i = 0; i < count; i++) {
    if((address >> 24) == 2)
      WorkRAM[address & 0x3ffff] = active;
    else
      InternalRAM[address & 0x7fff] = active;
    address++;
  }

  remotePutPacket("OK");
}

void remoteReadRegisters(char *p)
{
  char * buffer = (char *)TGDSARM9Malloc(1024);
  char *s = buffer;
  int i;
  // regular registers
  for(i = 0; i < 15; i++) {
    u32 v = i + (0x08000000);//reg[i].I;
    sprintf(s, "%02x%02x%02x%02x",  v & 255, (v >> 8) & 255,
            (v >> 16) & 255, (v >> 24) & 255);
    s += 8;
  }
  // PC
  u32 pc = i;	//armNextPC;
  sprintf(s, "%02x%02x%02x%02x", pc & 255, (pc >> 8) & 255,
          (pc >> 16) & 255, (pc >> 24) & 255);
  s += 8;

  // floating point registers (24-bit)
  for(i = 0; i < 8; i++) {
    sprintf(s, "000000000000000000000000");
    s += 24;
  }

  // FP status register
  sprintf(s, "00000000");
  s += 8;
  // CPSR
  //CPUUpdateCPSR();
  u32 v = 0;	//reg[16].I;
  sprintf(s, "%02x%02x%02x%02x",  v & 255, (v >> 8) & 255,
          (v >> 16) & 255, (v >> 24) & 255);
  s += 8;
  *s = 0;
  remotePutPacket(buffer);
  TGDSARM9Free(buffer);
}

void remoteWriteRegister(char *p)
{
	//todo
  /*
  int r = 0;
  sscanf(p, "%d=", &r);
  p = strchr(p, '=');
  p++;
  char c = *p++;
  u32 v = 0;
  u8 data[4] = {0,0,0,0};

  int i = 0;
  while(c != '#') {
    u8 b = 0;
    if(c <= '9')
      b = (c - '0') << 4;
    else
      b = (c + 10 - 'a') << 4;
    c = *p++;
    if(c <= '9')
      b += (c - '0');
    else
      b += (c + 10 - 'a');
    data[i++] = b;
    c = *p++;
  }
	
  v = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);

  //  //printf("Write register %d=%08x\n", r, v);
  
  reg[r].I = v;
  if(r == 15) {
    armNextPC = v;
    if(armState)
      reg[15].I = v + 4;
    else
      reg[15].I = v + 2;
  }
  */
  
  remotePutPacket("OK");
}

sint32 remoteStubMain(){

	if( (getMULTIMode() == dswifi_gdbstubmode) && (getWIFISetup() == true) && (dswifiSrv.GDBStubEnable == true) ){
		sint32 remoteStubMainStatus = 0;
		if(remoteResumed) {
			remoteSendStatus();
			remoteResumed = false;
		}
		char * buffer = (char *)TGDSARM9Malloc(1024);
		int res = remoteRecvFnc(buffer, 1024);
		//if DSWIFI connection is lost, re-connect and init GDB Server if external logic says so.
		if(res > 0){
			char *p = buffer;
			char c = *p++;
			char pp = '+';
			remoteSendFnc(&pp, 1);
			if(c != '$'){
				//try next time
			}
			else{
				c= *p++;
				switch(c) {
				case '?':
				  remoteSendSignal();
				  break;
				case 'D':
				  remotePutPacket("OK");
				  remoteResumed = true;
				  return dswifi_gdbstubmode;
				case 'e':
				  remoteStepOverRange(p);
				  break;
				case 'k':
				  remotePutPacket("OK");
				  return dswifi_gdbstubmode;
				case 'C':
				  remoteResumed = true;
				  return dswifi_gdbstubmode;
				case 'c':
				  remoteResumed = true;
				  return dswifi_gdbstubmode;
				case 's':
				  remoteResumed = true;
				  remoteSignal = 5;
				  //CPULoop(1);
				  if(remoteResumed) {
					remoteResumed = false;
					remoteSendStatus();
				  }
				  break;
				case 'g':
				  remoteReadRegisters(p);
				  break;
				case 'P':
				  remoteWriteRegister(p);
				  break;
				case 'M':
				  remoteMemoryWrite(p);
				  break;
				case 'm':
				  remoteMemoryRead(p);
				  break;
				case 'X':
				  remoteBinaryWrite(p);
				  break;
				case 'H':
				  remotePutPacket("OK");
				  break;
				case 'q':
				  remotePutPacket("");
				  break;
				case 'Z':
				  if(*p++ == '2') {
					remoteWriteWatch(p, true);
				  } else
					remotePutPacket("");
				  break;
				case 'z':
				  if(*p++ == '2') {
				remoteWriteWatch(p, false);
				  } else
				remotePutPacket("");
				  break;
				default:
				  {
					*(strchr(p, '#') + 3) = 0;
					//printf("Unknown packet %s ", --p);
					remotePutPacket("");
				  }
				  break;
				}
			}	
			remoteStubMainStatus = remoteStubMainWIFIConnectedGDBRunning;	//WIFI connected and GDB running
		}
		else if(res == 0){
			remoteStubMainStatus = remoteStubMainWIFIConnectedGDBRunning;	//WIFI connected and GDB running but no data was recv
		}
		else{
			remoteCleanUp();
			remoteStubMainStatus = remoteStubMainWIFIConnectedGDBDisconnected;	//Socket closed
		}
		TGDSARM9Free(buffer);
		return remoteStubMainStatus;
	}
	else{
		if(getWIFISetup() == true){
			return remoteStubMainWIFIConnectedNoGDB;	//gdb not running, WIFI connected	(retry connection here)
		}
	}
	return remoteStubMainWIFINotConnected;				//gdb not running, WIFI not connected (first time connection)
}

//Shared for GDBFile and GDBBuffer map modes.
uint32 GDBMapFileAddress = (uint32)0x00000000;
void setCurrentRelocatableGDBFileAddress(uint32 addrInput){
	GDBMapFileAddress = addrInput;
}
uint32 getCurrentRelocatableGDBFileAddress(){
	return GDBMapFileAddress;
}

void remoteStubSignal(int sig, int number)
{
  remoteSignal = sig;
  remoteResumed = false;
  remoteSendStatus();
}

void remoteCleanUp()
{
  if(remoteCleanUpFnc)
    remoteCleanUpFnc();
	dswifiSrv.GDBStubEnable = false;
}

u32 debuggerReadMemory(u32 addr){
	if(gdbStubMapMethod == GDBSTUB_METHOD_GDBBUFFER){
		return (u32)readu32GDBMapBuffer(addr);
	}
	else if(gdbStubMapMethod == GDBSTUB_METHOD_GDBFILE){
		return (u32)readu32GDBMapFile(addr);
	}
	else if(isValidMap(addr) == true){
		coherent_user_range_by_size((uint32)addr, (int)4);
		return (*(u32*)addr);
	}
	return (u32)(0xffffffff);
}

u16 debuggerReadHalfWord(u32 addr){
	if(gdbStubMapMethod == GDBSTUB_METHOD_GDBBUFFER){
		return (u16)(readu32GDBMapBuffer(addr)&0xffff);
	}
	else if(gdbStubMapMethod == GDBSTUB_METHOD_GDBFILE){
		return (u16)(readu32GDBMapFile(addr) & 0xffff);
	}
	else if(isValidMap(addr) == true){
		coherent_user_range_by_size((uint32)addr, (int)4);
		return (*(u16*)addr);
	}
	return (u16)(0xffff);
}

u8 debuggerReadByte(u32 addr){
	if(gdbStubMapMethod == GDBSTUB_METHOD_GDBBUFFER){
		return (u8)(readu32GDBMapBuffer(addr)&0xff);
	}
	else if(gdbStubMapMethod == GDBSTUB_METHOD_GDBFILE){
		return (u8)(readu32GDBMapFile(addr)&0xff);	//correct format: (value 32bit) & 0xff
	}
	else if(isValidMap(addr) == true){
		coherent_user_range_by_size((uint32)addr, (int)4);
		return (*(u8*)addr);
	}
	return (u8)(0xff);
}


int gdbStubMapMethod = 0;
//GDBMap: File impl.
struct gdbStubMapFile globalGdbStubMapFile;
struct gdbStubMapFile * getGDBMapFile(){
	return (struct gdbStubMapFile *)&globalGdbStubMapFile;
}
bool initGDBMapFile(char * filename, uint32 newRelocatableAddr){
	closeGDBMapFile();
	switch_dswnifi_mode(dswifi_idlemode);
	gdbStubMapMethod = GDBSTUB_METHOD_GDBFILE;
	struct gdbStubMapFile * gdbStubMapFileInst = getGDBMapFile();
	memset((uint8*)gdbStubMapFileInst, 0, sizeof(struct gdbStubMapFile));
	FILE * fh = fopen(filename, "r");
	if(fh){
		fseek(fh,0,SEEK_END);
		int fileSize = ftell(fh);
		fseek(fh,0,SEEK_SET);
		if(
			((uint32)newRelocatableAddr >= (uint32)minGDBMapFileAddress)
			&&
			((uint32)newRelocatableAddr < (uint32)maxGDBMapFileAddress)
		){
			gdbStubMapFileInst->GDBFileHandle = fh;
			gdbStubMapFileInst->GDBMapFileSize = fileSize;
			setCurrentRelocatableGDBFileAddress(newRelocatableAddr);
			return true;
		}
		else{
			fclose(fh);
		}
	}
	return false;
}
void closeGDBMapFile(){
	struct gdbStubMapFile * gdbStubMapFileInst = getGDBMapFile();
	if(gdbStubMapFileInst->GDBFileHandle != NULL){ 
		fclose(gdbStubMapFileInst->GDBFileHandle);
	}
}
uint32 readu32GDBMapFile(uint32 address){
	u32 readVal = 0;
	struct gdbStubMapFile * gdbStubMapFileInst = getGDBMapFile();
	if(gdbStubMapFileInst->GDBFileHandle != NULL){
		int FSize = gdbStubMapFileInst->GDBMapFileSize;
		if(
			(FSize > 0)
			&&
			(address >= minGDBMapFileAddress)
			&&
			(address < maxGDBMapFileAddress)
			&&
			(address >= GDBMapFileAddress)
			&&
			(address < (GDBMapFileAddress + FSize))
		){
			int offst = (address & ((uint32)(FSize -1)));
			fseek(gdbStubMapFileInst->GDBFileHandle, offst, SEEK_SET);
			fread((uint32*)&readVal, 1, 4, gdbStubMapFileInst->GDBFileHandle);
			return readVal;
		}
	}
	return (uint32)0xffffffff;
}

//GDBMap: Buffer impl.
struct gdbStubMapBuffer globalGdbStubMapBuffer;
struct gdbStubMapBuffer * getGDBMapBuffer(){
	return (struct gdbStubMapBuffer *)&globalGdbStubMapBuffer;
}

bool initGDBMapBuffer(u32 * bufferStart, int GDBMapBufferSize, uint32 newRelocatableAddr){
	closeGDBMapBuffer();
	switch_dswnifi_mode(dswifi_idlemode);
	gdbStubMapMethod = GDBSTUB_METHOD_GDBBUFFER;
	struct gdbStubMapBuffer * gdbStubMapBufferInst = getGDBMapBuffer();
	if(
		((uint32)newRelocatableAddr >= (uint32)minGDBMapFileAddress)
		&&
		((uint32)newRelocatableAddr < (uint32)maxGDBMapFileAddress)
	){
		gdbStubMapBufferInst->bufferStart = bufferStart;
		gdbStubMapBufferInst->GDBMapBufferSize = GDBMapBufferSize;
		setCurrentRelocatableGDBFileAddress(newRelocatableAddr);
		return true;
	}
	return false;
}
void closeGDBMapBuffer(){
	struct gdbStubMapBuffer * gdbStubMapBufferInst = getGDBMapBuffer();
	memset((uint8*)gdbStubMapBufferInst, 0, sizeof(struct gdbStubMapBuffer));
}
uint32 readu32GDBMapBuffer(uint32 address){
	struct gdbStubMapBuffer * gdbStubMapBufferInst = getGDBMapBuffer();
	int FSize = gdbStubMapBufferInst->GDBMapBufferSize;
	if(
		(FSize > 0)
		&&
		(address >= minGDBMapFileAddress)
		&&
		(address < maxGDBMapFileAddress)
		&&
		(address >= GDBMapFileAddress)
		&&
		(address < (GDBMapFileAddress + FSize))
	){
		int offst = (address & ((uint32)(FSize -1)));
		return *(u32*)(gdbStubMapBufferInst->bufferStart + offst);
	}
	return (uint32)0xffffffff;
}

void resetGDBSession(){
	setCurrentRelocatableGDBFileAddress(0);
	gdbStubMapMethod = GDBSTUB_METHOD_DEFAULT;
	setWIFISetup(false);
}


//misc socket related

//Client:
//opens and returns a new socket >= 0 ready to be used with connectAsync(); method, writes to sockaddr_in the current client (NDS) settings so it needs a valid sockaddr_in structure.
//Otherwise -1 if an error happened, such as server could not be resolved (ie: using custom DNS server), or if the AP isn't set in the firmware.

int openAsyncConn(char * dnsOrIpAddr, int asyncPort, struct sockaddr_in * sain){
	// Find the IP address of the server, with gethostbyname
    struct hostent * myhost = gethostbyname( dnsOrIpAddr );
	if(myhost != NULL){
		struct in_addr **address_list = (struct in_addr **)myhost->h_addr_list;
		if((address_list != NULL) && (address_list[0] != NULL)){
			printf("Server WAN IP Address! %s", inet_ntoa(*address_list[0]));
		}
		else{
			return -1;
		}
	}
	else{
		return -1;
	}
    // Create a TCP socket
    int my_socket = socket( AF_INET, SOCK_STREAM, 0 );
    if(my_socket < 0){
		forceclosesocket(my_socket); // remove the socket.
		return -1;
	}
	int enable = 1;
	if (setsockopt(my_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){	//socket can be respawned ASAP if it's dropped
		forceclosesocket(my_socket); // remove the socket.
		return -1;
	}
	memset(sain, 0, sizeof(struct sockaddr_in)); 
	int i=1;
	i=ioctl(my_socket, FIONBIO,&i);	//set non-blocking
    sain->sin_family = AF_INET;
    sain->sin_port = htons(asyncPort);
    sain->sin_addr.s_addr= *( (unsigned long *)(myhost->h_addr_list[0]) );
    
	return my_socket;
}

bool connectAsync(int sock, struct sockaddr_in * sain){
	//Connect
	int retVal = 0;
	if ((retVal = connect(sock,(struct sockaddr *)sain, sizeof(struct sockaddr_in))) < 0){
		if (errno != EINPROGRESS){
			return false;
		}
	}
	return true;
}

//Server:
//Open a port and listen through it. Synchronous/blocking.
int openServerSyncConn(int SyncPort, struct sockaddr_in * sain){
	int srv_len = sizeof(struct sockaddr_in);
	memset(sain, 0, srv_len);
	sain->sin_port = htons(SyncPort);//default listening port
	sain->sin_addr.s_addr = INADDR_ANY;	//the socket will be bound to all local interfaces (and we just have one up to this point, being the DS Client IP acquired from the DHCP server).
	
	int my_socket = socket(AF_INET, SOCK_STREAM, 0);
	int enable = 1;
	if (setsockopt(my_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){	//socket can be respawned ASAP if it's dropped
		forceclosesocket(my_socket); // remove the socket.
		return -1;
	}
	if(my_socket == -1){
		return -1;
	}
	int retVal = bind(my_socket,(struct sockaddr*)sain, srv_len);
	if(retVal == -1){
		disconnectAsync(my_socket);
		return -1;
	}
	int MAXCONN = 20;
	retVal = listen(my_socket, MAXCONN);
	if(retVal == -1){
		disconnectAsync(my_socket);
		return -1;
	}
	return my_socket;
}


bool disconnectAsync(int sock){
	shutdown(sock,0); // good practice to shutdown the socket.
	forceclosesocket(sock); // remove the socket.
	return true;
}


//Synchronous Bi-directional NIFI commands: The DS Sender waits until the command was executed in Remote DS.

//Host gets total of connected DSes. 
int getTotalConnectedDSinNetwork(){
	int TotalCount = 0;
	switch(getMULTIMode()){
		case (dswifi_localnifimode):
		case (dswifi_udpnifimode):{
			int retryCount = 0;
			struct dsnwifisrvStr * dsnwifisrvStrInst = getDSWNIFIStr();
			dsnwifisrvStrInst->DSIndexInNetwork = 0;
			dsnwifisrvStrInst->nifiCommand = NIFI_SENDER_TOTAL_CONNECTED_DS;
			char frame[frameDSsize];	//use frameDSsize as the sender buffer size, any other size won't be sent.
			memcpy(frame, (u8*)dsnwifisrvStrInst, sizeof(struct dsnwifisrvStr));
			FrameSenderUser = HandleSendUserspace((uint8*)frame, sizeof(frame));
			
			//wait until host sends us a response
			while(dsnwifisrvStrInst->nifiCommand == NIFI_SENDER_TOTAL_CONNECTED_DS){
				swiDelay(1);
				
				if(retryCount == 10000){
					return getTotalConnectedDSinNetwork();
				}
				retryCount++;
			}
			
			//Process ACK
			if(dsnwifisrvStrInst->nifiCommand == NIFI_ACK_TOTAL_CONNECTED_DS){
				u32 * shBuf = (u32*)&dsnwifisrvStrInst->sharedBuffer[0];
				TotalCount = shBuf[0];
			}
			TotalCount++;
		}
		break;
	}
	return TotalCount;
}


//Send a Binary over DS Wireless: Returns frames sent
int SendDSBinary(u8 * binBuffer, int binSize){
	int s = 0;
	switch(getMULTIMode()){
		case (dswifi_localnifimode):
		case (dswifi_udpnifimode):{
			//defaults
			int retryCount = 0;
			struct dsnwifisrvStr * dsnwifisrvStrInst = getDSWNIFIStr();
			dsnwifisrvStrInst->nifiCommand = NIFI_SENDER_SEND_BINARY;
			dsnwifisrvStrInst->frameIndex = 0;
			dsnwifisrvStrInst->BinarySize = binSize;
			for(s = 0; s < binSize/frameDSBufferSize; s++){	
				//Update packet
				dsnwifisrvStrInst->nifiCommand = NIFI_SENDER_SEND_BINARY;
				dsnwifisrvStrInst->frameIndex = s;
				memcpy((u8*)&dsnwifisrvStrInst->sharedBuffer[0], (u8*)&binBuffer[s*frameDSBufferSize], sizeof(dsnwifisrvStrInst->sharedBuffer));
				char frame[frameDSsize];	//use frameDSsize as the sender buffer size, any other size won't be sent.
				memcpy(frame, (u8*)dsnwifisrvStrInst, sizeof(struct dsnwifisrvStr));
				FrameSenderUser = HandleSendUserspace((uint8*)frame, sizeof(frame));
				
				//wait until host sends us a response
				while(dsnwifisrvStrInst->nifiCommand == NIFI_SENDER_SEND_BINARY){
					swiDelay(1);
					if(retryCount == 10000){
						s = -1;
						retryCount = 0;
						break;
					}
					retryCount++;
				}
				
				if(dsnwifisrvStrInst->nifiCommand == NIFI_ACK_SEND_BINARY_FINISH){	//finish, exit.
					break;
				}
			}
		}
		break;
	}
	return s;
}

//Receive a Binary over DS Wireless: Returns frames received
int ReceiveDSBinary(u8 * inBuffer, int * inBinSize){
	int receiveCount = 0;
	switch(getMULTIMode()){
		case (dswifi_localnifimode):
		case (dswifi_udpnifimode):{
			struct dsnwifisrvStr * dsnwifisrvStrInst = getDSWNIFIStr();			
			//defaults
			int binSize = 0;
			dsnwifisrvStrInst->nifiCommand = NIFI_ACK_SEND_BINARY;
			dsnwifisrvStrInst->frameIndex = -1;
			memset((u8*)&dsnwifisrvStrInst->sharedBuffer[0], 0, sizeof(dsnwifisrvStrInst->sharedBuffer));
				
			while(dsnwifisrvStrInst->nifiCommand != NIFI_SENDER_SEND_BINARY){	//todo: add timeout
				swiDelay(1);
			}
			//copy
			int frameIndex = dsnwifisrvStrInst->frameIndex;	//0
			memcpy((u8*)inBuffer + (frameIndex*frameDSBufferSize), &dsnwifisrvStrInst->sharedBuffer[0], sizeof(dsnwifisrvStrInst->sharedBuffer));
			
			binSize = dsnwifisrvStrInst->BinarySize;
			*inBinSize = binSize;
			printf("receive start... size: %d ", binSize);
			
			dsnwifisrvStrInst->nifiCommand = NIFI_ACK_SEND_BINARY;	//SendDSBinary() continue next frame
			char frame[frameDSsize];
			memcpy(frame, (u8*)dsnwifisrvStrInst, sizeof(struct dsnwifisrvStr));
			FrameSenderUser = HandleSendUserspace((uint8*)frame, sizeof(frame));
			
			int r = frameIndex + 1;	//1
			for(r = 1; r < binSize/frameDSBufferSize; r++){
				while(dsnwifisrvStrInst->nifiCommand != NIFI_SENDER_SEND_BINARY){	//todo: add timeout
					swiDelay(1);
				}
				
				//copy next
				int frameIndex = dsnwifisrvStrInst->frameIndex;
				if(frameIndex == 0){	//means the session got lost, retry from the beginning
					r = 0;
				}
				memcpy((u8*)inBuffer + (frameIndex*frameDSBufferSize), &dsnwifisrvStrInst->sharedBuffer[0], sizeof(dsnwifisrvStrInst->sharedBuffer));
				
				dsnwifisrvStrInst->nifiCommand = NIFI_ACK_SEND_BINARY;	//SendDSBinary() continue next frame
				char frame[frameDSsize];
				memcpy(frame, (u8*)dsnwifisrvStrInst, sizeof(struct dsnwifisrvStr));
				FrameSenderUser = HandleSendUserspace((uint8*)frame, sizeof(frame));
			}
			receiveCount = r;
			dsnwifisrvStrInst->nifiCommand = NIFI_ACK_SEND_BINARY_FINISH;
			memcpy(frame, (u8*)dsnwifisrvStrInst, sizeof(struct dsnwifisrvStr));
			FrameSenderUser = HandleSendUserspace((uint8*)frame, sizeof(frame));
		}
		break;
	}
	return receiveCount;
}

#endif //ARM9 end
