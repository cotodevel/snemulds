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

//DSWNifi Library 1.4 (update: 2/18/2020)	(mm/dd/yyyy)

#ifndef __dswnifi_lib_h__
#define __dswnifi_lib_h__

// Shared
#include "wifi_shared.h"

#define dswifi_udpnifimode (sint32)(5)	//UDP Nifi
#define dswifi_localnifimode (sint32)(6)	//Raw Network Packet Nifi
#define dswifi_localadhocmode (sint32)(7)	//Raw Network Packet DS <-> DS syncronous transfer. WIP
#define dswifi_idlemode (sint32)(8)	//Idle
#define dswifi_gdbstubmode (sint32)(9)	//GDB Stub mode

//ARM7
#ifdef ARM7
#endif

//ARM9
#ifdef ARM9
#include <in.h>
#include <netdb.h>
#include "fatfslayerTGDS.h"
#include "wifi_arm9.h"

//GDB Server. Shared for GDBFile and GDBBuffer map modes.
#define minGDBMapFileAddress	(uint32)(0x00000000)
#define maxGDBMapFileAddress	(uint32)(0xF0000000)

//NIFI defs
#define arm7_header_framesize 	(12 + 2)									//arm7 
#define arm9_header_framesize	(12 + 6)
#define frame_header_size 		(sint32)(arm9_header_framesize + arm7_header_framesize)		//32

#define CRC_CRC_STAGE 		0x81
#define CRC_OK_SAYS_HOST	0x88

//WIFI UDP defs
#define UDP_PORT (sint32)(8888)				//used for UDP Server - NDS Companion connecting
//used for UDP transfers between NDS Multi mode
#define NDSMULTI_UDP_PORT_HOST (sint32)(8889)			//host 	listener - listener is local - sender is multi IP NDS
#define NDSMULTI_UDP_PORT_GUEST (sint32)(8890)		//guest listener - 

//WIFI TCP defs:	7777 to 7788, 8080, 8777, 9777, 27900, 42292
#define TCP_PORT (sint32)(7777)		//used for TCP Server - NDS Companion connecting
#define NDSMULTI_TCP_PORT_HOST (sint32)(7778)			//host 	listener - listener is local - sender is multi IP NDS
#define NDSMULTI_TCP_PORT_GUEST (sint32)(7779)		//guest listener - 

//flag extension 
//connect stage
#define dswifi_udpnifimodeFailConnectStage (sint32)(18)
#define dswifi_localnifimodeFailConnectStage (sint32)(19)
//execution stage
#define dswifi_udpnifimodeFailExecutionStage (sint32)(20)
#define dswifi_localnifimodeFailExecutionStage (sint32)(21)

//special udp nifi/wifi mode
#define ds_multi_notrunning (sint32)(8)
#define ds_searching_for_multi_servernotaware (sint32)(9)	//"srvaware" sends back other NDS info (IP,PORT), then we open PORT socket, and bind it to the new IP, then set flag to ds_wait_for_multi, we listen to new port and close the old one here

#define ds_netplay_host_servercheck (sint32)(10)		//ds are binded at this point but we need to know if both are connected each other
#define ds_netplay_guest_servercheck (sint32)(11)

#define ds_netplay_host (sint32)(12)
#define ds_netplay_guest (sint32)(13)

//Standard Frame Size
#define frameDSsize (sint32)((512)+sizeof(volatile uint16))	//512 bytes
#define frameDSBufferSize (sint32)(256 + 104)	//remaining struct dsnwifisrvStr size within a frameDSsize

//remoteStubMain retcodes
#define remoteStubMainWIFINotConnected (sint32)(-1)	
#define remoteStubMainWIFIConnectedNoGDB (sint32)(16)	//Wifi alone connected, GDB not running
#define remoteStubMainWIFIConnectedGDBRunning (sint32)(17)	//Wifi & GDB running
#define remoteStubMainWIFIConnectedGDBDisconnected (sint32)(18)	//Wifi running, GDB disconnected

//DSWNIFI:

//process status
#define proc_idle (sint32)(0)
#define proc_connect (sint32)(1)
#define proc_execution (sint32)(2)
#define proc_shutdown (sint32)(3)

//DSWNIFI_MODE flags: switch between nifi/wifi/idle mode
#define DSWNIFI_ENTER_IDLEMODE (int)(1)
#define DSWNIFI_ENTER_NIFIMODE (int)(2)
#define DSWNIFI_ENTER_WIFIMODE (int)(3)

//UDP Descriptor
struct client_http_handler{
	//dswifi socket @ PORT 8888
    struct sockaddr_in sain_UDP_PORT;	//UDP: Listener sockaddr_in
	struct sockaddr_in server_addr;		//UDP Sender sockaddr_in: Desktop Server UDP companion Sender
    int socket_id__multi_notconnected;	//UDP/TCP: Listener FD. For handshake with server so both DS can connect each other. sendto uses for sending to PC
    
	//(this)DS Server Companion Listener: TCP 7777
	struct sockaddr_in sain_listener;	//UDP: Unused, TCP: DS sockaddr_in Server Companion Listen Socket: TCP 7777
	int socket_multi_listener;			//TCP: DS Server Listener. For handshake with server so both DS can connect each other/ DS-DS Multiplay
    
	//(The other) DS Multi: TCP NDSMULTI_TCP_PORT_HOST || NDSMULTI_TCP_PORT_GUEST
	struct sockaddr_in sain_sender;	//UDP/TCP: Client DS sockaddr_in assigned by the server
	int socket_multi_sender;		//UDP/TCP: Client DS File Descriptor assigned by the server to send messages to.
    
	//(this)DS Server Multi: TCP NDSMULTI_TCP_PORT_HOST || NDSMULTI_TCP_PORT_GUEST
	struct sockaddr_in sain_listenerNetplay;	//UDP: Unused, TCP: (this)DS Server Multi sockaddr_in TCP NDSMULTI_TCP_PORT_HOST || NDSMULTI_TCP_PORT_GUEST
	int socket_multi_listenerNetplay;			//UDP: Unused, TCP: (this)DS Server Multi SocketDescriptor TCP NDSMULTI_TCP_PORT_HOST || NDSMULTI_TCP_PORT_GUEST
	
	//host socket entry
    struct hostent myhost;    
    bool wifi_enabled;
};

//Note: Each Sender command requires an user-defined implementation in 2 sides:

//Synchronous Bi-directional NIFI commands (here, trigger)
//dswnifi.c in TGDS-project (action, reply)

//Sender command
#define NIFI_SENDER_TOTAL_CONNECTED_DS (u32)(0xffff4440)
#define NIFI_SENDER_SEND_BINARY (u32)(0xffff4442)

//Ack command (reply to Sender command)
#define NIFI_ACK_TOTAL_CONNECTED_DS (u32)(0xffff4441)
#define NIFI_ACK_SEND_BINARY (u32)(0xffff4443)
#define NIFI_ACK_SEND_BINARY_FINISH (u32)(0xffff4445)

//LOCAL/IDLE/GDB/UDP
struct dsnwifisrvStr {
	struct client_http_handler client_http_handler_context;	//Handles UDP DSWNIFI
	sint32 dsnwifisrv_mode;	//dswifi_idlemode / dswifi_localnifimode / dswifi_udpnifimode / dswifi_gdbstubmode		//used by setMULTIMode() getMULTIMode()
	sint32	connectionStatus;	//proc_idle / proc_connect / proc_execution / proc_shutdown	//used by getConnectionStatus() setConnectionStatus()
	sint32 	dsnwifisrv_stat;	//MULTI: inter DS Connect status: ds_multi_notrunning / ds_searching_for_multi / (ds_multiplay): ds_netplay_host ds_netplay_guest
	bool dswifi_setup;	//false: not setup / true: setup already	//used by getWIFISetup() / setWIFISetup()
	bool incoming_packet;	//when any of the above methods received a packet == true / no == false
	bool GDBStubEnable;
	
	//Session bits
	int DSIndexInNetwork;
	u32 nifiCommand;
	
	//Send Binary bits
	int frameIndex;		//	/NIFI SEND BINARY only
	int BinarySize;		//	/
	
	u8 sharedBuffer[frameDSBufferSize];
};

//returned by HandleSendUserspace. Converts the user buffer and size into a struct the ToolchainGenericDS library understands.
struct frameBlock{
    uint8 * framebuffer;
	sint32	frameSize;
};

#ifdef __cplusplus
extern "C"{
#endif

// Shared

//Handles DSWNIFI service
extern struct dsnwifisrvStr dswifiSrv;

#ifdef ARM9

//NIFI Part
//DSWNIFI: NIFI
extern void Handler(int packetID, int readlength);
extern void initNiFi();

//DSWNIFI: nifi buffer IO
extern volatile uint8	 data[4096];

//TCP UDP DSWNIFI Part
extern int Wifi_RawTxFrame_WIFI(sint32 datalen, uint8 * data);
extern int Wifi_RawTxFrame_NIFI(sint32 datalen, uint16 rate, uint16 * data);

extern bool switch_dswnifi_mode(sint32 mode);
extern struct frameBlock * FrameSenderUser;	//if !NULL, then must sendFrame. HandleSendUserspace(); generates this one

//the process that runs on vblank and ensures DS - DS Comms
//code that runs from ITCM
extern sint32 doMULTIDaemonStage1();
//code can't run from ITCM
extern sint32 doMULTIDaemonStage2(sint32 ThisConnectionStatus);

extern struct frameBlock FrameSenderBlock;	//used by the user sender process, must be valid so the ToolchainGenericDS library sends proper frame data.
extern struct frameBlock FrameRecvBlock;	//used by the user receiver process, can be NULL if no data frame was received.

//frame receiver implementation, has all receiver-like modes here. Returns true if correct frame received from TCP/UDP
extern struct frameBlock * 	receiveDSWNIFIFrame(uint8 * databuf_src,int frameSizeRecv);	//framesize is calculated inside (crc over udp requires framesize previously to here calculated anyway)
extern bool sendDSWNIFIFame(struct frameBlock * frameInst);
extern sint8 server_ip[MAX_TGDSFILENAME_LENGTH+1];
extern bool isValidIpAddress(char *ipAddress);

//Send a frame to the other connected DS
extern struct frameBlock * HandleSendUserspace(uint8 * databuf_src, int bufsize);

//userCode must override, provide these functions.

//As long you define this ReceiveHandler, everytime the outter connected DS to this DS sends a packet, it will be received here.
extern bool TGDSRecvHandler(struct frameBlock * frameBlockRecv);	//called by receiveDSWNIFIFrame(); when a frame is valid. TGDS layer. //Returns: Current DSWnifi mode
extern __attribute__((weak))	bool TGDSRecvHandlerUser(struct frameBlock * frameBlockRecv, int DSWnifiMode);	//called by TGDSRecvHandler when cmd is User implemented. TGDS User layer

//Callback that runs upon setting DSWNIFI mode to dswifi_localnifimode
extern __attribute__((weak))	void OnDSWIFIlocalnifiEnable();

//Callback that runs upon setting DSWNIFI mode to dswifi_idlemode
extern __attribute__((weak))	void OnDSWIFIidlemodeEnable();

//Callback that runs upon setting DSWNIFI mode to dswifi_udpnifimode
extern __attribute__((weak))	void OnDSWIFIudpnifiEnable();

//Callback that runs upon setting DSWNIFI mode to dswifi_gdbstubmode
extern __attribute__((weak))	void OnDSWIFIGDBStubEnable();

extern bool sentReq;
extern sint32 LastDSWnifiMode;
extern struct dsnwifisrvStr * getDSWNIFIStr();
extern bool connectDSWIFIAP(int DSWNIFI_MODE);

//GDB stub
extern bool gdbNdsStart();
extern int remotePort;
extern int remoteSignal;
extern int remoteSocket;
extern int remoteListenSocket;
extern bool remoteConnected;
extern bool remoteResumed;
extern int reconnectCount;

extern int remoteTcpSend(char *data, int len);
extern int remoteTcpRecv(char *data, int len);
extern bool remoteTcpInit();
extern void remoteTcpCleanUp();
extern int remotePipeSend(char *data, int len);
extern int remotePipeRecv(char *data, int len);
extern bool remotePipeInit();
extern void remotePipeCleanUp();
extern void remoteSetPort(int port);
extern void remoteSetProtocol(int p);
extern void remoteInit();
extern void remotePutPacket(char *packet);
extern void remoteOutput(char *s, u32 addr);
extern void remoteSendSignal();
extern void remoteSendStatus();
extern void remoteBinaryWrite(char *p);
extern void remoteMemoryWrite(char *p);
extern void remoteMemoryRead(char *p);
extern void remoteStepOverRange(char *p);
extern void remoteWriteWatch(char *p, bool active);
extern void remoteReadRegisters(char *p);
extern void remoteWriteRegister(char *p);
extern sint32 remoteStubMain();
extern void remoteStubSignal(int sig, int number);
extern void remoteCleanUp();

extern u8 debuggerReadByte(u32 addr);
extern u16 debuggerReadHalfWord(u32 addr);
extern u32 debuggerReadMemory(u32 addr);

//GDBMap: File impl.
extern uint32 GDBMapFileAddress;
extern void setCurrentRelocatableGDBFileAddress(uint32 addrInput);
extern uint32 getCurrentRelocatableGDBFileAddress();
extern struct gdbStubMapFile globalGdbStubMapFile;
extern struct gdbStubMapFile * getGDBMapFile();
extern bool initGDBMapFile(char * filename, uint32 newRelocatableAddr);
extern void closeGDBMapFile();
extern uint32 readu32GDBMapFile(uint32 address);
extern void resetGDBSession();

//DSWNIFI synchronous commands
extern int getTotalConnectedDSinNetwork();
extern int SendDSBinary(u8 * binBuffer, int binSize);
extern int ReceiveDSBinary(u8 * inBuffer, int * inBinSize);

//Example : 
//	These methods are used to Connect asynchronously to a server.
/*
// Let's send a simple HTTP request to a server and print the results!
void getHttp(char* url) {
//---------------------------------------------------------------------------------
    // store the HTTP request for later
    const char * request_text = 
        "GET /dswifi/example1.php HTTP/1.1\r\n"
        "Host: www.akkit.org\r\n"
        "User-Agent: Nintendo DS\r\n\r\n";
	
	struct sockaddr_in sain;	//structure holding the server IP/Port DS connects to.
	int serverSocket = openAsyncConn(url, 80, &sain);
	bool connStatus = connectAsync(serverSocket, &sain);
	
    if((serverSocket >= 0) && (connStatus ==true)){
		printf("Connected to server! ");
	}
	else{
		printf("Could not connect. ");
		return;
	}
	
    // send our request
    send( serverSocket, request_text, strlen(request_text), 0 );
    printf("Sent our request! ");

    // Print incoming data
    printf("Printing incoming data: ");

    int recvd_len;
    char incoming_buffer[256];

    while( ( recvd_len = recv(serverSocket, incoming_buffer, 255, 0 ) ) != 0 ) { // if recv returns 0, the socket has been closed.
        if(recvd_len>0) { // data was received!
            incoming_buffer[recvd_len] = 0; // null-terminate
            printf(incoming_buffer);
		}
	}

	printf("Server closed connection! Please power Off NDS. ");
	disconnectAsync(serverSocket);
}
*/

//Client:
extern int openAsyncConn(char * dnsOrIpAddr, int asyncPort, struct sockaddr_in * sain);
extern bool connectAsync(int sock, struct sockaddr_in * sain);
extern bool disconnectAsync(int sock);

//Server:

//opens a port,and begins to listen through it. Then an accept() call (blocking), through the earlier port, will give client connection context.
extern int openServerSyncConn(int SyncPort, struct sockaddr_in * sain);
#endif


#ifdef __cplusplus
}
#endif

static inline void setMULTIMode(sint32 flag){
	dswifiSrv.dsnwifisrv_mode = (sint32)flag;
}

static inline sint32 getMULTIMode(){
	return (sint32)dswifiSrv.dsnwifisrv_mode;
}

static inline bool getWIFISetup(){
	return (bool)dswifiSrv.dswifi_setup;
}

static inline void setWIFISetup(bool flag){
	dswifiSrv.dswifi_setup = (bool)flag;
}

static inline void setConnectionStatus(sint32 flag){
	dswifiSrv.connectionStatus = (sint32)flag;
}

static inline sint32 getConnectionStatus(){
	return (sint32)dswifiSrv.connectionStatus;
}


//GDB Stub part
#define debuggerWriteMemory(addr, value) \
  *(u32*)addr = (value)

#define debuggerWriteHalfWord(addr, value) \
  *(u16*)addr = (value)

#define debuggerWriteByte(addr, value) \
  *(u8*)addr = (value)

#define InternalRAM ((u8*)0x03000000)
#define WorkRAM ((u8*)0x02000000)

struct gdbStubMapFile {
	int GDBMapFileSize;
	FILE * GDBFileHandle;
};

//GDBMap: Buffer impl.
struct gdbStubMapBuffer {
	u8 * bufferStart;
	int GDBMapBufferSize;
};

#define GDBSTUB_METHOD_DEFAULT (int)(-1)
#define GDBSTUB_METHOD_GDBFILE (int)(1)
#define GDBSTUB_METHOD_GDBBUFFER (int)(2)

extern int gdbStubMapMethod;
extern struct gdbStubMapBuffer globalGdbStubMapBuffer;
extern struct gdbStubMapBuffer * getGDBMapBuffer();

extern bool initGDBMapBuffer(u32 * bufferStart, int GDBMapBufferSize, uint32 newRelocatableAddr);
extern void closeGDBMapBuffer();
extern uint32 readu32GDBMapBuffer(uint32 address);

#endif //ARM9 end

#endif
