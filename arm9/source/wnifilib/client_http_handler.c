#include "client_http_handler.h"
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

sint8* server_ip = (sint8*)"192.168.43.220";

//coworker that handles wifi/nifi commands in background

//__attribute__((section(".dtcm")))
client_http_handler client_http_handler_context;


//opens port 32123 @ UDP (any IP inbound from sender)
void request_connection(sint8* str_url,int str_url_size)
{   
	//TCP
	/*
    int safe_length = str_url_size + 1;
    sint8 str_url_safe[safe_length];
	
    //void * memcpy ( void * destination, const void * source, size_t num );
    memcpy(str_url_safe,str_url,safe_length);
    
    // Create a TCP socket
    
    // Find the IP address of the server, with gethostbyname
    struct hostent * myhost = gethostbyname( str_url_safe );
    
    //void * memcpy ( void * destination, const void * source, size_t num );
    memcpy((uint8*)&client_http_handler_context.myhost,(uint8*)myhost,sizeof(client_http_handler_context.myhost));
    
    //iprintf("Found Server IP Address! %s \n",str_url_safe);
	
	// Create a TCP socket
    client_http_handler_context.socket_id = socket( AF_INET, SOCK_STREAM, 0 );
    //iprintf("Created Socket!\n");

    // Tell the socket to connect to the IP address we found, on port 80 (HTTP)
    client_http_handler_context.sain.sin_family = AF_INET;
    client_http_handler_context.sain.sin_port = htons(80);
	
	if(bind(client_http_handler_context.socket_id,(struct sockaddr *)&client_http_handler_context.sain,sizeof(client_http_handler_context.sain))) {
		sint8 buf[64];
		sprintf(buf,"binding failed \n");
		consoletext(64*2-32,(sint8 *)&buf[0],0);
		
		close(client_http_handler_context.socket_id);
	}
	else{
		sint8 buf[64];
		sprintf(buf,"binding success \n");
		consoletext(64*2-32,(sint8 *)&buf[0],0);
	}
	
    client_http_handler_context.sain.sin_addr.s_addr= *( (unsigned long *)(client_http_handler_context.myhost.h_addr_list[0]) );
    connect( client_http_handler_context.socket_id,(struct sockaddr *)&client_http_handler_context.sain, sizeof(client_http_handler_context.sain) );
    //iprintf("Connected to server!\n");
	
	//sint8 buf[64];
	//sprintf(buf,"ClientIP:%s ->Server IP Address: %s ok \n",*( (unsigned long *)(client_http_handler_context.myhost.h_addr_list[0]) ));
	//consoletext(64*2-32,(sint8 *)&buf[0],0);
	*/
	
	
	//BLOCK NON BLOCK EXAMPLE
	// #include "sys/ioctl.h"
    //int on =1;
    //int off =0;

    //Enable non-blocking
    //ioctl (mySocket, FIONBIO, &(on));
    //Disable non-blocking
    //ioctl (mySocket, FIONBIO, (sint8 *) &(off));
	
	
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
		sprintf(buf,"binding ERROR\n");
		printf((sint8 *)&buf[0]);
		close(client_http_handler_context.socket_id__multi_notconnected);
		return;
	}
	else{
		sint8 buf[64];
		sprintf(buf,"binding OK: port %d IP: %s \n",(int)UDP_PORT, (sint8*)print_ip((uint32)Wifi_GetIP()));//(sint8*)print_ip((uint32)Wifi_GetIP()));
		printf((sint8 *)&buf[0]);
	}
	
	
	//for server IP / used for sending msges
	memset((sint8 *) &client_http_handler_context.server_addr, 0, sizeof(client_http_handler_context.server_addr));
	client_http_handler_context.server_addr.sin_family = AF_INET;
	client_http_handler_context.server_addr.sin_addr.s_addr = inet_addr(server_ip);//SERV_HOST_ADDR);
	client_http_handler_context.server_addr.sin_port = htons((int)UDP_PORT); //SERVER_PORT_ID);
	
	
	/*//works, but enable for testing since its set to BLOCKING (upcoming sends are idle)
	sint8 msg[12];
	sprintf(msg,  "Hello world");
	int retcode = sendto(client_http_handler_context.socket_id__multi_notconnected,msg,12,0,(struct sockaddr *) &client_http_handler_context.server_addr,sizeof(client_http_handler_context.server_addr));
	if (retcode <= -1){
		sint8 buf[64];
		sprintf(buf,"server send phail\n");
		consoletext(64*2-32,(sint8 *)&buf[0],0);
	}
	else{
		sint8 buf[64];
		sprintf(buf,"server send OK \n");
		consoletext(64*2-32,(sint8 *)&buf[0],0);
	}
	*/
	
	
}

/*
//send a form that issues data passed as string
bool send_response(sint8 * str_params)
{
    // send our request
    send( client_http_handler_context.socket_id, str_params, strlen(str_params), 0 );
    //iprintf("Sent our request!\n");

    // Print incoming data
    //iprintf("Printing incoming data:\n");

    
    //recv causes freezes
    static int recvd_len;
    static sint8 incoming_buffer[256];
    
    while( ( recvd_len = recv( client_http_handler_context.socket_id, incoming_buffer, 255, 0 ) ) != 0 ) { // if recv returns 0, the socket has been closed.
        if(recvd_len>0) { // data was received!
            incoming_buffer[recvd_len] = 0; // null-terminate
            //iprintf(incoming_buffer);
            break;//when get-response was acquired, exit.
		}
	}
    
	//iprintf("Other side closed connection!");
    return true;
}
*/

//libnds
/*
//---------------------------------------------------------------------------------
void getHttp(sint8* url) {
//---------------------------------------------------------------------------------
 // Let's send a simple HTTP request to a server and print the results!
 // store the HTTP request for later
 const sint8 * request_text = 
 "GET /dswifi/example1.php HTTP/1.1\r\n"
 "Host: www.akkit.org\r\n"
 "User-Agent: Nintendo DS\r\n\r\n";
 // Find the IP address of the server, with gethostbyname
 struct hostent * myhost = gethostbyname( url );
 iprintf("Found IP Address!\n");
 
 // Create a TCP socket
 int my_socket;
 my_socket = socket( AF_INET, SOCK_STREAM, 0 );
 iprintf("Created Socket!\n");
 // Tell the socket to connect to the IP address we found, on port 80 (HTTP)
 struct sockaddr_in sain;
 sain.sin_family = AF_INET;
 sain.sin_port = htons(80);
 sain.sin_addr.s_addr= *( (unsigned long *)(myhost->h_addr_list[0]) );
 connect( my_socket,(struct sockaddr *)&sain, sizeof(sain) );
 iprintf("Connected to server!\n");
 // send our request
 send( my_socket, request_text, strlen(request_text), 0 );
 iprintf("Sent our request!\n");
 // Print incoming data
 iprintf("Printing incoming data:\n");
 int recvd_len;
 sint8 incoming_buffer[256];
 while( ( recvd_len = recv( my_socket, incoming_buffer, 255, 0 ) ) != 0 ) { // if recv returns 0, the socket has been closed.
 if(recvd_len>0) { // data was received!
 incoming_buffer[recvd_len] = 0; // null-terminate
 iprintf(incoming_buffer);
 }
 }
 iprintf("Other side closed connection!");
 shutdown(my_socket,0); // good practice to shutdown the socket.
 closesocket(my_socket); // remove the socket.
}
*/