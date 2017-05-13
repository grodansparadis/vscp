// bigtest.c 
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../../src/vscp/helperlib/vscphelperlib.h"

// Output text for happy moments if set to one
#define verbose_success		0

// Output text for sad moments if set to one
#define verbose_error		1

// Output text for talky moments if set to 1
#define verbose_info		1

// Standard connection (a VSCP daemon must be running here)
#define INTERFACE1_HOST 	"127.0.0.1:9598"
#define INTERFACE1_USER 	"admin"
#define INTERFACE1_PASSWORD 	"secret"


// Count errors in the test
int error_cnt = 0;

int main(int argc, char* argv[])
{
    int rv;
    long handle1; 

    if ( verbose_info ) {
       printf("VSCP helperlib tables test program\n");
       printf("==================================\n");
    }

    handle1 = vscphlp_newSession();
    if (0 != handle1 ) {
        if (verbose_success) printf( "Handle one OK %ld\n", handle1 );
    }
    else {
        if (verbose_error) printf("\aError: Failed to get handle for channel 1\n");
        error_cnt++;
    }

    // Open Channel 1
    rv=vscphlp_open( handle1, 
                         INTERFACE1_HOST,
                         INTERFACE1_USER,
                         INTERFACE1_PASSWORD ); 
    if ( VSCP_ERROR_SUCCESS == rv ) {
        if (verbose_success) printf("Command success: vscphlp_open on channel 1\n");
    }
    else {
        printf("\aCommand error: vscphlp_open on channel 1  Error code=%d\n", rv);
        error_cnt++;
        return -1;
    }

    // NOOP on handle1
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_noop( handle1 ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_noop on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_noop on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }

    // Get version on handle1
    unsigned char majorVer, minorVer, subminorVer;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getVersion( handle1, &majorVer, &minorVer, &subminorVer ) ) ) {
        if (verbose_info) printf( "channel 1: Major version=%d  Minor version=%d  Sub Minor verion=%d\n", 
                                    majorVer, 
                                    minorVer, 
                                    subminorVer );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getVersion on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    //=========================================================================
    //                             Start of test
    //=========================================================================


    // Create test table
    


    if ( VSCP_ERROR_SUCCESS == vscphlp_close( handle1 ) ) {
        if (verbose_success) printf( "Command success: vscphlp_close on channel 1\n" );
    }

    vscphlp_closeSession( handle1 );
    vscphlp_closeSession( handle2 );

    printf("\n\nHit ENTER to terminate Error Count= %d\n", error_cnt );
    (void)getchar();
   
    return 0;
    
}
