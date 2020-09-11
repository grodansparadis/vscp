// test_connections.c 
//

// This test do many, many, many connections to the localhost interface,
// stressing it.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vscphelperlib.h>
#include <gtest/gtest.h>

// Output text for happy moments if set to one
#define verbose_success		0

// Output text for sad moments if set to one
#define verbose_error		1

// Output text for talky moments if set to 1
#define verbose_info		1

// Standard connection (a VSCP daemon must be running here)
//#define INTERFACE1_HOST         "127.0.0.1:9598"
//#define INTERFACE1_HOST         "192.168.1.44:9598"  // pi11
#define INTERFACE1_HOST         "192.168.1.34:9598"  // pi6
#define INTERFACE1_USER 	"admin"
#define INTERFACE1_PASSWORD "secret"

TEST(tcpif, Connect_to_VSCP_daemon) { 

    int rv;
    long handle1; 
    long error_cnt = 0;
    
    handle1 = vscphlp_newSession();
    ASSERT_NE(handle1, 0);


    // Open Channel 1
    rv=vscphlp_open( handle1, 
                         INTERFACE1_HOST,
                         INTERFACE1_USER,
                         INTERFACE1_PASSWORD ); 
    ASSERT_EQ(rv, VSCP_ERROR_SUCCESS);

    // NOOP on handle1
    rv = vscphlp_noop( handle1 );
    ASSERT_EQ(rv, VSCP_ERROR_SUCCESS);

    // Get version on handle1
    unsigned char majorVer, minorVer, subminorVer;
    rv = vscphlp_getVersion( handle1, &majorVer, &minorVer, &subminorVer );
    ASSERT_EQ(rv, VSCP_ERROR_SUCCESS);

    rv= vscphlp_close( handle1 );
    ASSERT_EQ(rv, VSCP_ERROR_SUCCESS);


    vscphlp_closeSession( handle1 );    

    return;

    //vscpEvent *pEvent;

    // Send event on channel 1
    vscpEvent e;
    e.vscp_class = 10;  // CLASS1.MEASUREMENT
    e.vscp_type = 6;    // Temperature
    e.head = 0;
    e.obid = 0;
    e.timestamp = 0;
    e.year = 2018;
    e.month = 3;
    e.day = 17;
    e.hour = 12;
    e.minute = 13;
    e.second = 1;
    e.sizeData = 3;
    e.pdata = (uint8_t *)malloc( sizeof( unsigned char[3] ) );
    e.pdata[0] = 138;  // Six degrees Celsius from sensor 2
    e.pdata[1] = 0;
    e.pdata[2] = 6;
    memset(e.GUID, 0, sizeof(e.GUID) ); // Setting GUID to all zero tell interface to use it's own GUID

    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEvent( handle1, &e ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_sendEvent on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_sendEvent on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }

    // Do it again
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEvent( handle1, &e ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_sendEvent on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_sendEvent on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }

    // Free the event (should be NULL here)
    // if ( NULL == pEvent ) {
    //     if (verbose_success) printf("pEvent is NULL.\n");
    // }
    // else {
    //     if (verbose_error) printf("Error: pEvent working event Not NULL.\n");
    // }
    // if (verbose_info) printf(">>> Delete pEvent working event.\n");
    // vscphlp_deleteVSCPevent( pEvent );
    // pEvent = NULL;

    // if (verbose_info) printf("\n\n\n");


    // free data
    // if (verbose_info) printf(">>> Free 'e' working event data.\n");
    // if ( NULL != pEvent) free( e.pdata );

    if ( VSCP_ERROR_SUCCESS == vscphlp_close( handle1 ) ) {
        if (verbose_success) printf( "Command success: vscphlp_close on channel 1\n" );
    }    

    vscphlp_closeSession( handle1 );

    ASSERT_EQ(0, 0);

}

int main(int argc, char* argv[])
{
    int count = 10000;

    if ( argc > 1 ) {
        count = atoi(argv[1]);
    }
    testing::InitGoogleTest(&argc, argv);
    for ( int i=0; i<count; i++ ) {
        if ( 1 == RUN_ALL_TESTS() ) {
            printf( "\nFailed at count = %d(%d)\n\n", i, count);
            return 1;
        }
    }
   
}
