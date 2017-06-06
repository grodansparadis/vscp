/*
    Simple udp client
*/

#include <unistd.h> 
#include <stdio.h>   
#include <string.h>  
#include <stdlib.h>  
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#define SERVER  "127.0.0.1" // Server to connect to
#define BUFLEN  512         // Max length of buffer
#define PORT    33333       // The port on which to send data
 
void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main(void)
{
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
 
    if ( -1 == ( s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) ) ) {
        die("socket");
    }
 
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
     
    if ( 0 == inet_aton(SERVER , &si_other.sin_addr) )  {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
 
    while( 1 ) {
        printf("Enter message : ");
        fgets(message, sizeof(message), stdin);
         
        //send the message
        if ( -1 == sendto( s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen ) ) {
            die("sendto()");
        }
         
        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data
        memset(buf,'\0', BUFLEN);
        //try to receive some data, this is a blocking call
        if ( -1 == recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen ) ) {
            die("recvfrom()");
        }
         
        puts( buf );
    }
 
    close(s);
    return 0;
}
