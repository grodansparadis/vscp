// example1.c 
//

#include "stdio.h"
#include "string.h"
#include "vscphelperlib.h"

/*!
 * Sends n events to remote server.
 *
 *
 *
 * */


//#define HOST "demo.vscp.org:9598"
//#define HOST "185.144.156.45:9598"
//#define HOST "192.168.1.6:9598"
#define HOST "127.0.0.1:9598"


#define COUNT 100

int main(int argc, char* argv[])
{
    long handle;
    long rv;
    unsigned char v1,v2,v3;
    
    // New session
    printf("Starting a new session to [" HOST "] ...\n");
    handle = vscphlp_newSession();
    if ( handle ) {
        printf("vscphlp_newSession: Success!\n");
    }
    else  {
	printf("vscphlp_newSession: Failed. handle = %ld\n", handle );
	vscphlp_closeSession( handle );
	return -1;
    }

    //vscphlp_setAfterCommandSleep( handle, 200 );

    printf("Open channel...\n");
    if ( VSCP_ERROR_SUCCESS == vscphlp_open( handle, 
           					HOST,
                				"admin",
    	           				"secret" ) ) {
		
	printf("vscphlp_open: Success!\n");
	
	if ( VSCP_ERROR_SUCCESS == vscphlp_noop( handle ) ) {
		printf("vscphlp_noop: 1 Success!\n");
	}   
	else {
		printf( "vscphlp_noop: 1 Failure %ld\n", rv );
		vscphlp_close( handle );
		vscphlp_closeSession( handle );
		return -1;
	}
    
	if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getVersion( handle, &v1, &v2, &v3 ) ) ) {
		printf( "vscphlp_getVersion: Success\n" );
		printf( "Version for VSCP daemon is %d.%d.%d\n", v1,v2,v3 );
	}
	else {
		printf( "vscphlp_getVersion: Failure %ld\n", rv );
		vscphlp_close( handle );
		vscphlp_closeSession( handle );
		return -1;
	}
    
	// Send event on channel 1
	vscpEventEx ex;
    	ex.vscp_class = 10;  // CLASS1.MEASUREMENT
    	ex.vscp_type = 1;    // Count
    	ex.head = 0;
    	ex.year = 1956;
    	ex.month = 11;
    	ex.day = 2;
    	ex.hour = 12;
    	ex.minute = 10;
    	ex.second = 3;
    	ex.sizeData = 2;
    	ex.timestamp = 0;
    	ex.data[0] = 0;  
    	ex.data[1] = 0;
    	memset(ex.GUID, 0, sizeof(ex.GUID) ); // Setting GUID to all zero tell interface to use it's own GUID

	for ( int i=0; i<COUNT; i++ ) {

		ex.data[0] = (i>>8) & 0xff;
    		ex.data[1] = i & 0xff;

    		if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEventEx( handle, &ex ) ) ) {
        		printf( "vscphlp_sendEvent: Success. %d\n", i );
    		}
    		else {
        		printf("vscphlp_sendEvent: Failure - rv = %ld\n", rv );
        		vscphlp_close( handle );
        		vscphlp_closeSession( handle );
        		return -1;
    		}
	}




	if ( VSCP_ERROR_SUCCESS == vscphlp_close( handle ) ) {
		printf("vscphlp_close: Success!\n");
	}
	else {
		printf( "vscphlp_close: 2 Failure %ld\n", rv );
	}
	
	vscphlp_closeSession( handle );
	
    } 
    else {
        printf( "vscphlp_open: Failure. host %s\n", HOST );
		vscphlp_closeSession( handle ); 
        return -1;
    }
   
	return 0;
}
