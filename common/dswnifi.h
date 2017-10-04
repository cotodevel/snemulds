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


//coto: nifi & wifi mix support! (:
#define dswifi_wifimode 3
#define dswifi_nifimode 2
#define dswifi_idlemode 1

//special wifi mode
#define manpage_flags 0
#define ds_searching_for_multi_servernotaware 9	//"srvaware" sends back other NDS info (IP,PORT), then we open PORT socket, and bind it to the new IP, then set flag to ds_wait_for_multi, we listen to new port and close the old one here

#define ds_netplay_host_servercheck 12		//ds are binded at this point but we need to know if both are connected each other
#define ds_netplay_guest_servercheck 13

#define ds_netplay_host 14
#define ds_netplay_guest 16


//---------------------------------------------------------------------------------
typedef struct dsnwifisrvStr {
//---------------------------------------------------------------------------------
	uint8 dsnwifisrv_mode;	//0 : idle / 1 : nifi / 2 : (ds)wifi	
	bool dswifi_setup;	//false: not setup / true: setup already
	
	uint8 dsnwifisrv_stat;	//ds_searching_for_multi / ds_multiplay
	

}TdsnwifisrvStr;

extern TdsnwifisrvStr dswifiSrv;


#endif


#ifdef __cplusplus
extern "C"{
#endif

extern void switch_dswnifi_mode(uint8 mode);

#ifdef __cplusplus
}
#endif