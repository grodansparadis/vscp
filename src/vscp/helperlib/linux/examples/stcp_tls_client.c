
// stcp_tcl_client.c 
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <stdint.h>
#include <inttypes.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sockettcp.h>
#include <vscphelperlib.h>

#ifndef UNUSED
# define UNUSED(x) ((void)(x))
#endif

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif


char errbuf[200];
char buf[128000];
struct stcp_connection *conn;

///////////////////////////////////////////////////////////////////////////////
// current_timestamp
//

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}


int main(int argc, char* argv[])
{
    long handle;
    long long t1,t2;

    UNUSED(argc); UNUSED(argv);

    t1 = t2 = current_timestamp();

    printf("Raw TLS test with stcp\n");
    printf("----------------------\n");

    struct stcp_secure_options opts;
    opts.client_cert_path = NULL;
    opts.server_cert_path = NULL;
    opts.pem = "civitweb_client.pem";
    opts.chain = NULL;
    //"kEECDH:kEDH:kRSA:AESGCM:AES256:AES128:3DES:SHA256:SHA84:SHA1:!aNULL:!eNULL:!EXP:!LOW:!MEDIUM!ADH:!AECDH";
    opts.ca_path = NULL;
    opts.ca_file = NULL;

    //conn = stcp_connect_remote( "localhost", 4433, errbuf, sizeof( errbuf ), 5 );
    conn = stcp_connect_remote_secure( "localhost", 4433, &opts, 5 );
    if ( NULL != conn ) {

        printf( "Connect OK\n");

        char buf[8192];
        memset( buf, 0, sizeof( buf ) );
        if ( stcp_read( conn, buf, sizeof( buf ), 5000 ) > 0 ) {
            printf( "%s\n", buf );
        }
        

        stcp_close_connection( conn );
        conn = NULL;

        stcp_uninit_ssl();
    }
    else {
        printf("Failed to open connection\n");
    }

    return 0;
}