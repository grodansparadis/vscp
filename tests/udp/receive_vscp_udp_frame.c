/*
    Simple udp server
    http://www.binarytides.com/programming-udp-sockets-c-linux/
*/

#include <unistd.h> 
#include <stdio.h>   
#include <string.h>  
#include <stdlib.h>  
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <crc.h>
#include <vscp.h>
 
#define BUFLEN  1024   // Max length of buffer
#define PORT    9999   // The port on which to listen for incoming data
 
void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main(void)
{
    struct sockaddr_in si_me, si_other;
     
    int s, i, slen = sizeof(si_other) , recv_len;
    unsigned char buf[BUFLEN];
     
    // create a UDP socket
    if ( -1 == ( s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) ) ) {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    // bind socket to port
    if ( -1 == bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) ) {
        die("bind");
    }
     
    // keep listening for data
    while ( 1 ) {
        printf("Waiting for VSCP UDP frames...");
        fflush(stdout);
         
        // try to receive some data, this is a blocking call
        memset( buf, 0, sizeof( buf ) );
        if ( -1 == ( recv_len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &si_other, &slen) ) ) {
            die("recvfrom()");
        }
         
        // print details of the client/peer and the data received
        printf( "Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port) );
        printf( "Frame: " );
        for ( int i=0; i<recv_len; i++ ) {
            printf("%02X ", buf[i] );
        }
        printf("\n");
         
        //now reply the client with the same data
        /*if ( -1 == sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen ) ) {
            die("sendto()");
        }*/
    }
 
    close(s);
    return 0;
}
