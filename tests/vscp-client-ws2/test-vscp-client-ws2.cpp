#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 
#include <vscp_aes.h>
#include <vscphelper.h>
#include "civetweb.h"
#include <json.hpp> // Needs C++11  -std=c++11

#include "vscp_client_ws2.h"

#define WS2_RESPONSE_TIMEOUT	2000	

// for convenience
using json = nlohmann::json;

// Default user/pass
const char *ws2_username = "admin";
const char *ws2_password = "secret";
// VSCP key (this key is secret and shared with server) No it should not be here.
const char *ws2_vscpkey = "A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E";

typedef enum ws2_states {ws2_disconnected,ws2_connected,ws2_login} ws2_states;

typedef enum ws2_substates {ws2_idle,ws2_waiting} ws2_substates;

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

sem_t sem_msg; 

std::deque<std::string> receiveQueue;

struct tclient_data {
	int closed;
	std::deque<std::string> receiveQueue;
};

struct tclient_data client1_data; // = {NULL, 0, 0};
struct mg_connection *newconn1 = NULL;



///////////////////////////////////////////////////////////////////////////////
// ws2_make_auth_pkg
//

int ws2_send_pkg( json& j, 
					std::string& sid, 
					std::string& key, 
					std::string& user, 
					std::string& password)
{
	j["type"] = "cmd";
	j["command"] ="auth";

	std::deque<std::string> args;

	//sid:xxxx
	//crypto;yyyy

	return VSCP_ERROR_SUCCESS;
}

int ws2_encrypt_password(std::string& strout,
							std::string struser,
							std::string strpassword,
							uint8_t *vscpkey,
							uint8_t *iv) 
{
	uint8_t buf[200]; 
	char out[200];
	std::string strCombined;
	strCombined = struser;
	strCombined += ":";
	strCombined += strpassword;
	
	AES_CBC_encrypt_buffer(AES128, 
							buf,
							(uint8_t *)strCombined.c_str(), 
							strCombined.length(), 
							vscpkey, 
							iv);						
	vscp_byteArray2HexStr(out, buf, 16);
	strout = out;

	return VSCP_ERROR_SUCCESS;
}

static int
websocket_client_data_handler(struct mg_connection *conn,
                              int flags,
                              char *data,
                              size_t data_len,
                              void *user_data)
{
	struct mg_context *ctx = mg_get_context(conn);
	
	// struct tclient_data *pclient_datax =
	//     (struct tclient_data *)mg_get_user_data(ctx);
	// printf("-----> Client received data from server: ");
	// std::string strx(data,data_len);
	// //fwrite(data, 1, data_len, stdout);
	// printf("%s\n",strx.c_str());	
	
	struct tclient_data *pclient_data =
	    (struct tclient_data *)mg_get_user_data(ctx);

	printf("Client received data from server: ");
	std::string str(data,data_len);
	//fwrite(data, 1, data_len, stdout);
	printf("%s\n",str.c_str());

	pclient_data->receiveQueue.push_back(str);
	sem_post(&sem_msg);

	//pclient_data->data = malloc(data_len);
	/* assert(pclient_data->data != NULL); ... just sigfault */
	//memcpy(pclient_data->data, data, data_len);
	//pclient_data->len = data_len;

	return 1;
}

static void
websocket_client_close_handler(const struct mg_connection *conn,
                               void *user_data)
{
	struct mg_context *ctx = mg_get_context(conn);
	struct tclient_data *pclient_data =
	    (struct tclient_data *)mg_get_user_data(ctx);

	printf("Client: Close handler\n");
	//pclient_data->closed++;
}

///////////////////////////////////////////////////////////////////////////////
// waitForResponse
//

int waitForResponse( uint32_t timeout )
{	
	time_t ts;
	time(&ts);

	struct timespec to;
	to.tv_nsec = 0;
	to.tv_sec = ts + timeout/1000;

	if ( -1 == sem_timedwait(&sem_msg,&to) ) {
		printf("!!!!!!!!!!!!!!!!! Error = %d\n",errno);
		switch(errno) {

			case EINTR:
				return VSCP_ERROR_INTERUPTED;

			case EINVAL:
				return VSCP_ERROR_PARAMETER;

			case EAGAIN:
				return VSCP_ERROR_ERROR;

			case ETIMEDOUT:	
			default:
				return VSCP_ERROR_TIMEOUT;
		}

	}

	return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// _startWebsocket
//

void _startWebsocket()
{
	char ebuf[100] = {0};
	const char *path = "/ws2";
	newconn1 = mg_connect_websocket_client( "192.168.1.7"/*"echo.websocket.org"*/, // host
	                                       8884 /*443*/,     // port
	                                       0 /*1*/,       	 // ssl
	                                       ebuf,
	                                       sizeof(ebuf),
	                                       path,
	                                       NULL,
	                                       websocket_client_data_handler,
	                                       websocket_client_close_handler,
	                                       &client1_data);

	if (newconn1 == NULL) {
		printf("Error: %s\n", ebuf);
	}
	else {

		// Response will be something like
		/*
			{    
    			"type" : "+",
    			"command": "AUTH0", 
    			"args" : {
        			"sid" : "3ded39018dbf0e8e4512a7cac79fd487"
    			}
			}

			where sid is a random session id that should be used to 
			encrypt admin:password together with the vscpkey that is
			a secret key between the VSCP daemon and a client
		*/

		// struct timespec to;
		// time_t ts;
		// time(&ts);
		// to.tv_nsec = 0;
		// to.tv_sec = ts + WS2_RESPONSE_TIMEOUT;
		// if ( -1 == sem_timedwait(&sem_msg,&to) ) {
		// 	printf("!!!!!!!!!!!!!!!!! Error = %d\n",errno);
		// 	switch(errno) {

		// 	}
		// 	return;
		// }

		if ( VSCP_ERROR_SUCCESS != waitForResponse( WS2_RESPONSE_TIMEOUT ) ) {
			printf("ERROR or TIMEOUT\n");
			return;
		}
		std::string strws2pkt = client1_data.receiveQueue.front();		
		client1_data.receiveQueue.pop_front();
		printf("strWsPkt = %s\n",strws2pkt.c_str());

		json j;
		try {
			j = json::parse(strws2pkt.c_str());
		}
		catch (...) {
			printf("Parsing error\n");
			return;
		}

		std::string sidstr;
		uint8_t iv[16];
		if ( j["type"] == "+") {
			printf("------------------------------------> type\n");
			std::deque<std::string> args = j["args"];
			printf("%ld\n",args.size());
			std::string token = j["args"][0];
			sidstr = j["args"][1];
			printf("%s\n",token.c_str());
			if ( (2 == args.size()) &&
			     ("AUTH0" == j["args"][0] )  ) {
				printf("------------------------------------> auth\n");
				printf("------------------------------------> sid   %s\n",sidstr.c_str());
				vscp_hexStr2ByteArray(iv,16,sidstr.c_str());
			}
		}
		else {
			return;
		}

		// login

		uint8_t buf[200]; 
		char out[200];
		// uint8_t ttt[] = "admin:secret";
		// AES_CBC_encrypt_buffer(AES128, buf, ttt, 12, vscpkey, iv);
		// vscp_byteArray2HexStr(out, buf, 16);
		// printf("%s\n", out);

		// //sprintf(buf,template_command, )

		// const char *text = "{\"type\": \"cmd\","\
        //     "\"command\": \"auth\","\
        //     "\"args\": {"\
        //        "\"iv\":\"5a475c082c80dcdf7f2dfbd976253b24\","\
        //        "\"crypto\": \"89d599aceb8e846dc3daf6cd33ce5e6d\""\
        //     "}}";

		json authcmd;
		// //  = {
		// // {"type","cmd"},
		// // {"command","noop"},
		// // {"args",nullptr}
		// // };
		authcmd["type"] = "cmd";
		authcmd["command"] = "auth";
		authcmd["args"]["iv"] = "5a475c082c80dcdf7f2dfbd976253b24";
		authcmd["args"]["crypto"] = "89d599aceb8e846dc3daf6cd33ce5e6d";

		// printf("%s\n",tttt.dump().c_str());

		std::string strout;
		ws2_encrypt_password(strout,
								"admin",
								"secret",
								vscpkey,
								iv);
		printf("Function: %s\n",strout.c_str());
		
		authcmd["args"]["iv"] = sidstr;
		authcmd["args"]["crypto"] = strout;	

		mg_websocket_client_write(newconn1,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          authcmd.dump().c_str(),
		                          authcmd.dump().length());
		
		if ( VSCP_ERROR_SUCCESS != waitForResponse( WS2_RESPONSE_TIMEOUT ) ) {
			printf("ERROR or TIMEOUT\n");
			return;
		}

		// We are in the connected state

		json cmd;
		cmd["type"] = "cmd";
		cmd["command"] = "close";
		cmd["args"] = nullptr;

		mg_websocket_client_write(newconn1,
									MG_WEBSOCKET_OPCODE_TEXT,
									cmd.dump().c_str(),
									cmd.dump().length());

		if ( VSCP_ERROR_SUCCESS != waitForResponse( WS2_RESPONSE_TIMEOUT ) ) {
			printf("ERROR or TIMEOUT\n");
			return;
		}

		mg_close_connection( newconn1 );
		
		// states
		// ------
		// CONNECTED 	- Waiting for AUTH0. If not received within x seconds
		//				  send CHALLENGE
		// LOGIN     	- Send credentials. Wait for positive response.
		// CONNECTED 	- Send events/receive events. 
		// DISCONNECTED - Do nothing
		// enum ws2_states {ws2_disconnected,ws2_connected,ws2_login}

		// substates
		// ---------
		// idle         - Response is received.
		// waiting 		- Waiting for response from remote server.
		// enum ws2_substates {ws2_idle,ws2_waiting}

		sleep(1);
		sleep(1);
		sleep(1);								
	}
}

///////////////////////////////////////////////////////////////////////////////
// startWebsocket
//

void startWebsocket()
{
	vscpClientWs2 c;
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
	sem_init(&sem_msg, 0, 0); 

    printf("Websockets\n");
    mg_init_library(MG_FEATURES_SSL);
	startWebsocket();

	sem_destroy(&sem_msg); 
    return 0;
}
