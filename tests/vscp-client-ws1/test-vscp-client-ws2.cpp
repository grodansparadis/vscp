
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "civetweb.h"

#include "vscp_client_mqtt.h"

struct tclient_data {
	void *data;
	size_t len;
	int closed;
};

struct tclient_data client1_data = {NULL, 0, 0};
struct mg_connection *newconn1 = NULL;

static int
websocket_client_data_handler(struct mg_connection *conn,
                              int flags,
                              char *data,
                              size_t data_len,
                              void *user_data)
{
	struct mg_context *ctx = mg_get_context(conn);
	struct tclient_data *pclient_data =
	    (struct tclient_data *)mg_get_user_data(ctx);

	printf("Client received data from server: ");
	fwrite(data, 1, data_len, stdout);
	printf("\n");

	pclient_data->data = malloc(data_len);
	/* assert(pclient_data->data != NULL); ... just sigfault */
	memcpy(pclient_data->data, data, data_len);
	pclient_data->len = data_len;

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
	const char *path = "/ws1";
	newconn1 = mg_connect_websocket_client( "127.0.0.1"/*"echo.websocket.org"*/, // host
	                                       8884 /*443*/,     // port
	                                       0 /*1*/,       // ssl
	                                       ebuf,
	                                       sizeof(ebuf),
	                                       path,
	                                       NULL,
	                                       websocket_client_data_handler,
	                                       websocket_client_close_handler,
	                                       &client1_data);

	if (newconn1 == NULL) {
		printf("Error: %s\n", ebuf);
	} else {
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
    uint64_t wcyd;
    std::string strLine = "00-00-00-00-00-00-9E-1F";
    
    wcyd = 0;
    size_t pos;
    for ( int i=0; i<8; i++) {
        wcyd = wcyd << 8;
        printf("%s\n",strLine.c_str());
        wcyd += std::stoul(strLine,&pos,16);
        if ( 7 != i) {
            strLine = strLine.substr(pos+1);  // Take away first part of str "zz:"
        }
    }
    printf( "%" PRIx64 "\n",wcyd);

    printf("Websockets\n");
    mg_init_library(MG_FEATURES_SSL);
	startWebsocket();

    return 0;
}
