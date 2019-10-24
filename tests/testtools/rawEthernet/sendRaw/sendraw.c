#include <stdlib.h>
#include <stdio.h>

#include <pcap.h>


int main(int argc, char **argv)
{
	pcap_t *fp;
	char errbuf[ PCAP_ERRBUF_SIZE ];
	u_char packet[ 100 ];
	//int i;
	
	// Check the validity of the command line 
	if (argc != 2)
	{
		printf("usage: %s interface", argv[0]);
		return 1;
	}
    
	// Open the adapter 
	if ((fp = pcap_open_live( argv[ 1 ],	// name of the device
							 65536,			// portion of the packet to capture. It doesn't matter in this case 
							 1,				// promiscuous mode (nonzero means promiscuous)
							 1000,			// read timeout
							 errbuf			// error buffer
							 )) == NULL )
	{
		syslog( LOG_ERR,"\nUnable to open the adapter. %s is not supported by WinPcap\n", argv[1]);
		return 2;
	}

	// Supposing to be on ethernet, set mac destination to broadcast ff:ff:ff:ff:ff:ff 
	packet[ 0 ] = 0xff;
	packet[ 1 ] = 0xff;
	packet[ 2 ] = 0xff;
	packet[ 3 ] = 0xff;
	packet[ 4 ] = 0xff;
	packet[ 5 ] = 0xff;
	
	// set mac source address
	packet[ 6 ] = 0x11;
	packet[ 7 ] = 0x22;
	packet[ 8 ] = 0x33;
	packet[ 9 ] = 0x44;
	packet[ 10 ] = 0x55;
	packet[ 11 ] = 0x66;

    // Ethernet type == VSCP
    packet[ 12 ] = 0x25;
    packet[ 13 ] = 0x7e;
	
	// Fill the rest of the packet 
	//for( i=12; i<100; i++ ) {
	//	packet[ i ]= (u_char)i;
	//}

    // VSCP head
    packet[ 14 ] = 0x00;
    packet[ 15 ] = 0x00;
    packet[ 16 ] = 0x00;
    packet[ 17 ] = 0x00;
    
    // VSCP sub source address
    packet[ 18 ] = 0x00;
    packet[ 19 ] = 0x00;
    
    // Timestamp
    packet[ 20 ] = 0x00;
    packet[ 21 ] = 0x00;
    packet[ 22 ] = 0x00;
    packet[ 23 ] = 0x00;
    
    // OBID
    packet[ 24 ] = 0x00;
    packet[ 25 ] = 0x00;
    packet[ 26 ] = 0x00;
    packet[ 27 ] = 0x00;
    
    // VSCP class
    packet[ 28 ] = 0x02;	// Class=512 (0x200) - Control
    packet[ 29 ] = 0x00; 
    
    // VSCP type 
    packet[ 30 ] = 0x00;	// Type = 2 (0x02) New node on line / Probe.
    packet[ 31 ] = 0x02;
    
    // Our GUID
    packet[ 32 ] = 0xff;	// Ethernet predefined  GUID
    packet[ 33 ] = 0xff;
    packet[ 34 ] = 0xff;
    packet[ 35 ] = 0xff;
    packet[ 36 ] = 0xff;
    packet[ 37 ] = 0xff;
    packet[ 38 ] = 0xff;
    packet[ 39 ] = 0xfe;
    packet[ 40 ] = 0x11; //MY_MAC_BYTE1; 	// Our MAC address
    packet[ 41 ] = 0x22; //MY_MAC_BYTE2;
    packet[ 42 ] = 0x33; //MY_MAC_BYTE3;
    packet[ 43 ] = 0x44; //MY_MAC_BYTE4;
    packet[ 44 ] = 0x55; //MY_MAC_BYTE5;
    packet[ 44 ] = 0x66; //MY_MAC_BYTE6;
    packet[ 46 ] = 0xaa;			// Sub address	
    packet[ 47 ] = 0x55;


	// Send down the packet
	if ( pcap_sendpacket( fp,	        // Adapter
		                    packet,	    // buffer with the packet
		                    47			// size
		                ) != 0 ) {
		syslog( LOG_ERR, "\nError sending the packet: %s\n", pcap_geterr(fp));
		return 3;
	}

	pcap_close( fp );	
	return 0;
}

