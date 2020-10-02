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

#include "vscp_client_mqtt.h"

#define XML_BUFF_SIZE 0xffff

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

struct ws2_response {
	enumType type;
	enumCommand cmd;
	std::deque<std::string> args;
	int error_code;
	std::string error_str;
 };

struct ws2_package {
	enumType type;
	std::string cmd;
	std::deque<std::string> args;
};

const char *user = "admin";
const char *password = "secret";
uint8_t vscpkey[] = { 0xA4,0xA8,0x6F,0x7D,0x7E,0x11,0x9B,0xA3,
                   0xF0,0xCD,0x06,0x88,0x1E,0x37,0x1B,0x98,
                   0x9B,0x33,0xB6,0xD6,0x06,0xA8,0x63,0xB6,
                   0x33,0xEF,0x52,0x9D,0x64,0x54,0x4F,0x8E };

sem_t sem_msg; 

struct tclient_data {
	//void *data;
	//size_t len;
	int closed;
	std::deque<std::string> receiveQueue;
};

struct tclient_data client1_data; // = {NULL, 0, 0};
struct mg_connection *newconn1 = NULL;

///////////////////////////////////////////////////////////////////////////////
// ws2_parse_resp
//

int ws2_parse_resp( struct ws2_response *presp, std::string& strws2pkt)
{
	// Check pointer
	if (NULL == presp ) {
		return VSCP_TYPE_ERROR_INVALID_POINTER;
	}

	try {
		json json_pkg = json::parse(strws2pkt.c_str());

		if (json_pkg.find("type") != json_pkg.end()) {
			
			std::string type = json_pkg.at("type").get<std::string>();
			vscp_trim(type);
			vscp_makeUpper(type);
			
			if ( type == "+") {
				presp->type = ws2pos;
			}
			else if ( type == "-" ) {
				presp->type = ws2neg;
			}
		}

		if (json_pkg.find("error_code") != json_pkg.end()) {
			presp->error_code = json_pkg.at("erro_code").get<int>();
		}

		if (json_pkg.find("error_str") != json_pkg.end()) {
			presp->error_str = json_pkg.at("error_str").get<std::string>();
		}

		for (auto it = json_pkg.begin(); it != json_pkg.end(); ++it) {
			if ("args" == it.key()) {	
				std::deque<std::string> queue = it.value();
				presp->args = queue; 									
			}
		}
	}
	catch (...) {
		return VSCP_ERROR_ERROR;
	}

	return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// ws2_create_json
//

int ws2_send_pkg( json& j,struct ws2_package *ppkg )
{
	// Check pointer
	if (NULL == ppkg ) {
		return VSCP_TYPE_ERROR_INVALID_POINTER;
	}

	json templ_cmd = {
		{"type","cmd"},
		{"command","noop"},
		{"args",nullptr}
	};
	templ_cmd["args"]["iv"] = "5a475c082c80dcdf7f2dfbd976253b24";
	templ_cmd["args"]["crypto"] = "89d599aceb8e846dc3daf6cd33ce5e6d";

	switch (ppkg->type) {

		case ws2pos:
			j["type"] = "+";
			break;
		
		case ws2neg:
			j["type"] = "-";
			break;

		case ws2cmd:
			j["type"] = "cmd";
			break;			
	}

	if ( ws2cmd == ppkg->type ) {
		j["type"] = ppkg->cmd;
	}

	j["args"] = ppkg->args;

	return VSCP_ERROR_SUCCESS;
}

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
	pclient_data->closed++;
}

void
startWebsocket()
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

		sem_wait(&sem_msg);
		std::string strWsPkt = client1_data.receiveQueue.front();		
		client1_data.receiveQueue.pop_front();

		printf("strWsPkt = %s\n",strWsPkt.c_str());

		struct ws2_response *presponse = (struct ws2_response *)new struct ws2_response;
		json json_pkg = json::parse(strWsPkt.c_str());
		if (json_pkg.find("type") != json_pkg.end()) {

            std::string type = json_pkg.at("type").get<std::string>();
            vscp_trim(type);
            vscp_makeUpper(type);
			printf("Type = %s\n", type.c_str());
			if ( type == "+") {
				presponse->type = ws2pos;
			}
			else if ( type == "-" ) {
				presponse->type = ws2neg;
			}
			else if ( type == "CMD" ) {
				presponse->type = ws2cmd;
			}
		}

		if (json_pkg.find("command") != json_pkg.end()) {

            // presponse->command = json_pkg.at("command").get<std::string>();
            // vscp_trim(presponse->command);
            // vscp_makeUpper(presponse->command);
			// printf("Type = %s\n", presponse->command.c_str());
		}

		if (json_pkg.find("error_code") != json_pkg.end()) {

            presponse->error_code = json_pkg.at("erro_code").get<int>();
			printf("error-code = %d\n", presponse->error_code);
		}

		if (json_pkg.find("error_str") != json_pkg.end()) {

            presponse->error_str = json_pkg.at("error_str").get<std::string>();
            vscp_trim(presponse->error_str);
            vscp_makeUpper(presponse->error_str);
			printf("error-str = %s\n", presponse->error_str.c_str());
		}

		uint8_t sid[16];
		for (auto it = json_pkg.begin(); it != json_pkg.end(); ++it) {
			if ("args" == it.key()) {	
				std::deque<std::string> ll = it.value();
				presponse->args = ll; //it.value();
				printf("args size = %ld\n",presponse->args.size());
				std::map<std::string, std::string> argmap;
				for ( int i = 0; i <= presponse->args.size(); i++) {
					std::string str = presponse->args.front();
					presponse->args.pop_front();
					printf("arg %d = %s\n",i, str.c_str());
					if ( 1 == i) {
						printf("size = %ld\n",vscp_hexStr2ByteArray(sid,16,str.c_str()));
					}
				}
				// try {
				// 	for (auto it = json_obj.begin(); it != json_obj.end(); ++it) {
				// 		if (it.value().is_string()) {
				// 			argmap[it.key()] = it.value();
				// 			printf("%s = %s\n",it.key().c_str(),argmap[it.key()].c_str());
				// 		}
				// 	}
				// }
				// catch (...) {
				// 	printf("ERROR\n");
				// }					
			}
		}

		const char *text = "Can you hear me, Major Tom?";
		printf("OK ... sending text\n");

		uint8_t iv[16];
		std::string result;
		std::string str_iv = "5a475c082c80dcdf7f2dfbd976253b24";
		printf("size = %ld\n",vscp_hexStr2ByteArray(iv,16,str_iv.c_str()));
		vscp_makePasswordHash(result,"admin:secret",iv);
		printf("encrypted: %s %s\n",result.c_str(),
			vscp_isPasswordValid(result,"admin:secret") ? "OK" : "BAD");

		uint8_t buf[200]; 
		char out[200];
		uint8_t ttt[] = "admin:secret";
		AES_CBC_encrypt_buffer(AES128, buf, ttt, 12, vscpkey, iv);
		vscp_byteArray2HexStr(out, buf, 16);
		printf("%s\n", out);

		//sprintf(buf,template_command, )

		text = "{\"type\": \"cmd\","\
            "\"command\": \"auth\","\
            "\"args\": {"\
               "\"iv\":\"5a475c082c80dcdf7f2dfbd976253b24\","\
               "\"crypto\": \"89d599aceb8e846dc3daf6cd33ce5e6d\""\
            "}}";

		json tttt;
		//  = {
		// {"type","cmd"},
		// {"command","noop"},
		// {"args",nullptr}
		// };
		tttt["type"] = "cmd";
		tttt["command"] = "auth";
		tttt["args"]["iv"] = "5a475c082c80dcdf7f2dfbd976253b24";
		tttt["args"]["crypto"] = "89d599aceb8e846dc3daf6cd33ce5e6d";

		printf("%s\n",tttt.dump().c_str());

		std::string strout;
		ws2_encrypt_password(strout,
								"admin",
								"secret",
								vscpkey,
								iv);
		printf("Function: %s\n",strout.c_str());
		
		tttt["args"]["crypto"] = strout;	

		mg_websocket_client_write(newconn1,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          tttt.dump().c_str(),
		                          tttt.dump().length());
		sleep(1);

		
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
										
	}
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
