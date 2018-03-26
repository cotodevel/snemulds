
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

#ifdef ARM9
#include "nifi.h"
#include "wifi_arm9.h"
#include "dswifi9.h"
#include "client_http_handler.h"
#include <string.h>
#endif

#ifdef ARM9

//0 idle, 1 nifi, 2 dswnifi
void switch_dswnifi_mode(uint8 mode){

	//wifi minimal setup
	if(mode == (uint8)dswifi_wifimode){
		SpecificIPC->dswifiSrv.dsnwifisrv_mode = dswifi_wifimode;
		SpecificIPC->dswifiSrv.dsnwifisrv_stat = ds_searching_for_multi_servernotaware;
		SpecificIPC->dswifiSrv.dswifi_setup = false;	//set for RPC services
	}
	//nifi minimal setup
	else if (mode == (uint8)dswifi_nifimode){
		//nifi
		SpecificIPC->dswifiSrv.dsnwifisrv_mode = dswifi_nifimode;
		SpecificIPC->dswifiSrv.dswifi_setup = false;
	}
	
	//idle mode minimal setup
	else if (mode == (uint8)dswifi_idlemode){
		//nifi
		SpecificIPC->dswifiSrv.dsnwifisrv_mode = dswifi_idlemode;
		SpecificIPC->dswifiSrv.dswifi_setup = false;
	}
	
	
	//set NIFI mode
	if( (SpecificIPC->dswifiSrv.dsnwifisrv_mode == dswifi_nifimode) && (SpecificIPC->dswifiSrv.dswifi_setup == false)){
		initNiFi();
		SpecificIPC->dswifiSrv.dswifi_setup = true;
	}
	
	//set tcp dswifi
	else if((SpecificIPC->dswifiSrv.dsnwifisrv_mode == dswifi_wifimode) && (SpecificIPC->dswifiSrv.dswifi_setup == false)){
		if(Wifi_InitDefault(WFC_CONNECT) == true)
		{
			//char buf[64];
			//sprintf(buf,"connected: IP: %s",(char*)print_ip((uint32)Wifi_GetIP()));
			//consoletext(64*2-32,(char *)&buf[0],0);
			
			//works fine both
			//new connection
			request_connection(server_ip,strlen(server_ip));
			
			//this must be called on send tx frame
			//send_response((char *)"<html><body><form action='http://192.168.43.108/'>First name:<br><input type='text' name='firstname' value='Mickey'><br>Last name:<br><input type='text' name='lastname' value='Mouse'><br><br><input type='submit' value='Submit'></form> </body></html>");
			//send_response((char *)"GET /dswifi/example1.php HTTP/1.1\r\n""Host: www.akkit.org\r\n""User-Agent: Nintendo DS\r\n\r\n");
			SpecificIPC->dswifiSrv.dswifi_setup = true;
		}
		else{
			//printf("     phailed conn! ________________________________");
		}
	}
	
}

#endif
