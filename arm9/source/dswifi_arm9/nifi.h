//this extends properties gathered from some nifi projects which should be used across nifi projects

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


#include <nds.h>

#endif

#ifdef __cplusplus
extern "C"{
#endif

extern int Wifi_RawTxFrame_NIFI(u16 datalen, u16 rate, u16 * data);
extern u32 dswifi_arm9status();

extern bool NiFiHandler(int packetID, int readlength, u8 * data);

//nifi buffer IO
extern volatile u8	 data[4096];		//data[32] + is recv TX'd frame nfdata[128]
extern volatile u8	 nfdata[128];	//sender frame, recv as data[4096]

//message for nifi beacons
extern volatile const u8 nifitoken[32];
extern volatile const u8 nificonnect[32];
extern volatile u8 nificrc[32];

extern void Handler(int packetID, int readlength);

//WIFI specific
extern int Wifi_RawTxFrame_WIFI(u8 datalen, u8 * data);
extern volatile u8 wifi_block[512 * 1024];
extern volatile u8 data_udp[256];

extern void switch_dswnifi_mode(u8 mode);

#ifdef __cplusplus
}
#endif
