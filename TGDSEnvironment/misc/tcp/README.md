Coto:

This covers the UDP NIFI protocol, to connect DS - DS through internet.

This is a (windows only) UDP Server called "UDP TGDS Server Companion Program", 

if you have VS2012+ installed you can edit sources through .sln file.



Steps:

1) use some router/old phone that supports either OPEN or WEP connections, setup a WIFI hotspot in that device. 
Use any DS game that supports NIFI to setup the DS to connect to the device you use as WIFI hotspot.

2) The computer that acts as server must use the SAME IP as in toolchaingenericds/src/arm9/source/arm9_driver/dswnifi_lib.c , specifically the symbol:

2a)
sint8* server_ip = (sint8*)"192.168.43.221";	// <-- here add your desktop IP

2b)

//add the header
#include "dswnifi_lib.c"

//the callback switch_dswnifi_mode(argument), where argument can be: dswifi_idlemode , dswifi_localnifimode or dswifi_udpnifimode (only these)
//So in your app you can switch between these functions to enter multiplayer local or multiplayer through internet 
//from:
switch_dswnifi_mode(dswifi_idlemode);

//to:
switch_dswnifi_mode(dswifi_localnifimode);

//or:
switch_dswnifi_mode(dswifi_udpnifimode);	//<- this guide uses UDP through internet so this is what you need if you are reading this README

When your TGDS project is ready, recompile ToolchainGenericDS AND your ToolchainGenericDS project to make changes effective.
The Server Companion uses the first network IP it finds (so make sure you reflect back the IP you wrote earlier in the dswnifi library, to the host PC, through static IP)


3) head to /Release folder, run TGDS-UDPCompanion.exe, a console with yellow letters will pop-up saying to connect two (yes 2) DSs in the same network.If you have a firewall
warning, please check public and private networks and click accept.

As soon as the client in the DS connects (by using the above callbacks), the server companion will detect 1 DS connected. Do the same steps for a 2nd DS...
When two DSes are connected, they will connect each other! Press L to send a UDP nifi message to the other DS!

Check the project https://bitbucket.org/Coto88/toolchaingenericds-multiplayer-example to know how to implement the callbacks:
override userCode sender and userCode ReceiveHandler required, so your project has nifi through local or UDP internet!


Coto.