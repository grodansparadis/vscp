#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 
#include <vscphelper.h>
#include "civetweb.h"
#include <json.hpp> // Needs C++11  -std=c++11

#include "vscp_client_mqtt.h"

#define XML_BUFF_SIZE 0xffff

// for convenience
using json = nlohmann::json;

// A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E

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
	newconn1 = mg_connect_websocket_client( "127.0.0.1"/*"echo.websocket.org"*/, // host
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
		json json_pkg = json::parse(strWsPkt.c_str());
		if (json_pkg.find("type") != json_pkg.end()) {

            std::string str = json_pkg.at("type").get<std::string>();
            vscp_trim(str);
            vscp_makeUpper(str);
			printf("Type = %s\n", str.c_str());
		}

		for (auto it = json_pkg.begin(); it != json_pkg.end();
                         ++it) {
			if ("args" == it.key()) {	
				std::deque<std::string> ll = it.value();
				printf("ll = %ld\n",ll.size());
				std::map<std::string, std::string> argmap;
				for ( int i = 0; i <= ll.size(); i++) {
					std::string str = ll.front();
					ll.pop_front();
					printf("arg %d = %s\n",i, str.c_str());
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

		mg_websocket_client_write(newconn1,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          text,
		                          strlen(text));
		sleep(1);
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
