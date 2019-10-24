// rawReceive.cpp : Defines the entry point for the console application.
//
// winpcap is used for this packet capture. See http://www.winpcap.org
// Include directory should point to pcap.h
// Library should point to wcap.lib
// Both available in the wpdback (binary)
//
// http://www.winpcap.org/docs/docs_40_2/html/group__wpcap__tut8.html
// http://www.ecst.csuchico.edu/~beej/guide/net/
// http://mixter.void.ru/rawip.html
//

#ifdef _MSC_VER
/*
 * we do not want the warnings about the old deprecated and unsecure CRT functions
 * since these examples can be compiled under *nix as well
 */
#define _CRT_SECURE_NO_WARNINGS
#endif

//#include "stdafx.h"
//#include <winsock.h>
#include "pcap.h"


/* prototype of the packet handler */
void packet_handler( u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data );


//int _tmain(int argc, _TCHAR* argv[])
int main()
{
    pcap_if_t *alldevs;
    pcap_if_t *d;
    int inum;
    int i=0;
    pcap_t *adhandle;
    char errbuf[ PCAP_ERRBUF_SIZE ];

/*
    WSADATA wsaData; // if this doesn't work
    //WSAData wsaData; // then try this instead
    // MAKEWORD(1,1) for Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0:
    if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(1);
    }

    int fd = socket( PF_INET, SOCK_RAW, IPPROTO_TCP );
    char buffer[8192]; // single packets are usually not bigger than 8192 bytes 
    while ( read (fd, buffer, 8192) > 0 ) {
        printf ("Caught tcp packet: %s\n", 
                    buffer + sizeof( struct iphdr ) + sizeof( struct tcphdr ) );
    }

    WSACleanup();
*/

    printf("This programs dumps VSCP type Ethernet frames on a selected  interface\n");
    printf("======================================================================\n");
    
    // Retrieve the device list on the local machine 
    if ( pcap_findalldevs_ex( PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf ) == -1 ) {
        syslog( LOG_ERR, "Error in pcap_findalldevs: %s\n", errbuf );
        exit( 1 );
    }
    
    // Print the list 
    for ( d = alldevs; d; d = d->next ) {
        printf( "%d. %s", ++i, d->name );
        if ( d->description ) {
            printf(" (%s)\n", d->description );
        }
        else {
            printf( " (No description available)\n" );
        }
    }
    
    if ( i==0 ) {
        printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
        return -1;
    }
    
    printf("Enter the interface number (1-%d):", i );
    scanf( "%d", &inum );
    
    if ( inum < 1 || inum > i ) {
        printf("\nInterface number out of range.\n");
        // Free the device list
        pcap_freealldevs( alldevs );
        return -1;
    }
    
    // Jump to the selected adapter 
    for( d=alldevs, i=0; i< inum-1 ;d=d->next, i++ );
    
    // Open the device 
    if ( ( adhandle= pcap_open( d->name,        // name of the device
                                65536,          // portion of the packet to capture
                                                // 65536 guarantees that the whole packet will be captured on all the link layers
                              PCAP_OPENFLAG_PROMISCUOUS,    // promiscuous mode
                              1000,             // read timeout
                              NULL,             // authentication on the remote machine
                              errbuf            // error buffer
                              ) ) == NULL ) {
        syslog( LOG_ERR, "\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
        // Free the device list 
        pcap_freealldevs( alldevs );
        return -1;
    }
    
    printf("\nlistening on %s...\n", d->description );
    
    // At this point, we don't need any more the device list. Free it 
    pcap_freealldevs( alldevs );
    
    // start the capture 
    pcap_loop( adhandle, 0, packet_handler, NULL );
    
    return 0;
}


/* Callback function invoked by libpcap for every incoming packet */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
    struct tm *ltime;
    char timestr[16];
    time_t local_tv_sec;
    int i;
    u_int etherType = ( pkt_data[ 12 ]  << 8 ) + pkt_data[ 13 ];
    
    if ( 0x257e == etherType ) {

        // convert the timestamp to readable format 
        local_tv_sec = header->ts.tv_sec;
        ltime=localtime(&local_tv_sec);
        strftime( timestr, sizeof timestr, "%H:%M:%S", ltime);
    
        printf("%s,%.6d len:%d type:%02x%02x\n", 
                    timestr, 
                    header->ts.tv_usec, 
                    header->len,
                 pkt_data[ 12 ],
                    pkt_data[ 13 ] );

        /*
        for ( i=0; i<header->len; i++ ) {
            printf( "%02x ", pkt_data[ i ] );
        }
        printf("\n");
        */

        printf("Destination MAC address: " );
        for ( i=0; i<6; i++ ) {
            printf( "%02x ", pkt_data[ i ] );
        }
        printf("\n");

        printf("Source MAC address: " );
        for ( i=6; i<12; i++ ) {
            printf( "%02x ", pkt_data[ i ] );
        }
        printf("\n");

        printf("Ethernet type:  %02x%02x\n", 
                pkt_data[ 12 ],
                pkt_data[ 13 ] );

        printf("VSCP data: ");
        for ( i=14; i<header->len; i++ ) {
            printf( "%02x ", pkt_data[ i ] );
        }
        printf("\n\n");
    }
}



