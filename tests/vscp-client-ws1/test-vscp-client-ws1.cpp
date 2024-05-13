#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 
#include <vscp_aes.h>
#include <vscphelper.h>
#include "civetweb.h"
#include <nlohmann/json.hpp> // Needs C++11  -std=c++11

#include "vscp-client-ws1.h"

#define WS2_RESPONSE_TIMEOUT	2000	

// for convenience
using json = nlohmann::json;

// Default user/pass
const char *ws1_username = "admin";
const char *ws1_password = "secret";
// VSCP key (this key is secret and shared with server) No it should not be here.
const char *ws1_vscpkey = "A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E";

// typedef enum enumType {ws2pos,  /* Positive reply */
// 						ws2neg, /* Negative reply */
// 						ws2cmd	/* Command send/reply */} enumType;
// typedef enum enumCommand {ws2auth,ws2open,ws2close} enumComman;

const char *user = "admin";
const char *password = "secret";
uint8_t vscpkey[] = { 0xA4,0xA8,0x6F,0x7D,0x7E,0x11,0x9B,0xA3,
                   0xF0,0xCD,0x06,0x88,0x1E,0x37,0x1B,0x98,
                   0x9B,0x33,0xB6,0xD6,0x06,0xA8,0x63,0xB6,
                   0x33,0xEF,0x52,0x9D,0x64,0x54,0x4F,0x8E };

///////////////////////////////////////////////////////////////////////////////
// startWebsocket
//

void startWebsocket()
{
	vscpClientWs1 c;
	c.init("127.0.0.1",
					8884,
					false,
					"admin",
					"secret",
					vscpkey );

	if (VSCP_ERROR_SUCCESS == c.connect()) {
		printf("Connected\n");
	}	

	for ( int i=0;i<120;i++) {
		printf("Sending PING\n" );
		mg_websocket_client_write(c.m_conn,
		                          MG_WEBSOCKET_OPCODE_PING,
		                          (const char *)&i,
		                          sizeof(int));
		sleep(1);				
	}

	if (VSCP_ERROR_SUCCESS == c.disconnect()) {
		printf("Disconnected\n");
	}

	printf("The END!\n");
}

int main()
{
	//sem_init(&sem_msg, 0, 0); 

    printf("Websockets\n");
    mg_init_library(MG_FEATURES_SSL);
	startWebsocket();

	//sem_destroy(&sem_msg); 
    return 0;
}
