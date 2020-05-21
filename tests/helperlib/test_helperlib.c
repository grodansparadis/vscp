// bigtest.c 
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vscphelper.h>

// Output text for happy moments if set to one
#define verbose_success		0

// Output text for sad moments if set to one
#define verbose_error		1

// Output text for talky moments if set to 1
#define verbose_info		1

// Standard connection (a VSCP daemon must be running here)
#define INTERFACE1_HOST 	"127.0.0.1:9598"
#define INTERFACE1_USER 	"admin"
#define INTERFACE1_PASSWORD "secret"

// ALternative connection form (a VSCP daemon must be running here)
#define INTERFACE2 		    "127.0.0.1:9598;admin;secret"
#define INTERFACE2_FLAGS	0

// If TEST_RECEIVE_LOOP is uncommented the rcvloop commands
// will be tested. Must send five events externally to test
//#define TEST_RECEIVE_LOOP 

// Uncomment to test variable handling
#define TEST_VARIABLE_HANDLING 

// Uncomment to test helpers
#define TEST_HELPERS

// Uncomment to test measurement functionality
#define TEST_MEASUREMENT

// Count errors in the test
int error_cnt = 0;

int main(int argc, char* argv[])
{
    int rv;
    long handle1, handle2; 

    if ( verbose_info ) {
       printf("VSCP helperlib test program\n");
       printf("===========================\n");
    }

    handle1 = vscphlp_newSession();
    if (0 != handle1 ) {
        if (verbose_success) printf( "Handle one OK %ld\n", handle1 );
    }
    else {
        if (verbose_error) printf("\aError: Failed to get handle for channel 1\n");
        error_cnt++;
    }

    handle2 = vscphlp_newSession();
    if (0 != handle2 ) {
        if (verbose_success) printf( "Handle two OK %ld\n", handle2 );
    }
    else {
        if (verbose_error) printf("\aError: Failed to get handle for channel 2\n");
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

    // OPEN channel 2
    rv=vscphlp_openInterface( handle2, INTERFACE2, INTERFACE2_FLAGS ); 
    if ( VSCP_ERROR_SUCCESS == rv ) {
        if (verbose_success) printf("Command success: vscphlp_openInterface on channel 2\n");
    }
    else {
        printf("\aCommand error: vscphlp_openInterface on channel 2  Error code=%d\n", rv);
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

    // NOOP on handle2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_noop( handle2 ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_noop on channel 2\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_noop on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }

    // Get version on handle1
    unsigned char majorVer, minorVer, subminorVer;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getVersion( handle1, &majorVer, &minorVer, &subminorVer ) ) ) {
        if (verbose_info) printf( "channel 1: Major version = %d  Minor version = %d  Sub Minor verion = %d\n", 
                                    majorVer, 
                                    minorVer, 
                                    subminorVer );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getVersion on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


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
    e.pdata = malloc( sizeof( unsigned char[3] ) );
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


    // Two events should now have been received on handle2
    unsigned int count;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_isDataAvailable on handle2\n" );
        if ( 2 == count ) {
            if (verbose_success) printf( "Two events waiting to be fetched on channel 2.\n" );
        }
        else if ( count > 2 ) {
            if (verbose_success) printf( "%u events waiting to be fetched on channel 2.\n", count );
        }
        else {
            if (verbose_success) printf( "%u events waiting to be fetched on channel 2 [Other source is also sending events].\n", count );
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_isDataAvailable on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }


    // Clear the event queue on the demon
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_clearDaemonEventQueue( handle2 ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_clearDaemonEventQueue input queue on channel 2\n");
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_clearDaemonEventQueue on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }


    // We should now have an empty queue on channel2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_isDataAvailable on handle2\n" );
            printf( "count = %u\n", count );
            if ( !count ) printf("Which is correct.\n");
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_isDataAvailable on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }

    vscpEventEx ex;
    ex.vscp_class = 10; // CLASS1.MEASUREMENT
    ex.vscp_type = 6;   // Temperature
    ex.head = 0;
    e.timestamp = 0;
    e.year = 2018;
    e.month = 3;
    e.day = 17;
    e.hour = 12;
    e.minute = 13;
    e.second = 1;
    ex.sizeData = 3;
    ex.data[0] = 138;   // 6 degrees Celsius from sensor 2
    ex.data[1] = 0;
    ex.data[2] = 6;
    memset(ex.GUID, 0, sizeof(e.GUID) ); // Setting GUID to all zero tell interface to use it's own GUID

    // Send event again
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEventEx( handle1, &ex ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_sendEventEx on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_sendEventEx on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }

    ex.data[0] = 138;  // 1.11 degrees Celsius from sensor 2
    ex.data[1] = 2;
    ex.data[2] = 111; 

    // Send event again
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEventEx( handle1, &ex ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_sendEventEx on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_sendEventEx on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    ex.data[0] = 138;  // -1 degrees Celsius from sensor 2
    ex.data[1] = 0;
    ex.data[2] = 255; 

    // Send event again
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEventEx( handle1, &ex ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_sendEventEx on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_sendEventEx on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }

    // We should now have three events in the queue on channel2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_isDataAvailable on handle2\n" );
            printf( "count = %u\n", count );
            if ( 3 == count ) printf("Which is correct.\n");
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_isDataAvailable on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }


    // Read event1
    vscpEvent *pEvent = malloc( sizeof( vscpEvent ) );
    pEvent->pdata = NULL;   // A must for a successful delete
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_receiveEvent( handle2, pEvent ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_receiveEvent on handle2\n" );
            printf( "VSCP class=%d VSCP type=%d sizeData=%d\n", 
                        pEvent->vscp_class,
                        pEvent->vscp_type,
                        pEvent->sizeData );
            printf("Data = ");
            for ( int i=0; i<pEvent->sizeData; i++ ) {
                printf("%d ", pEvent->pdata[i] );
            }
            printf("\n");
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_receiveEvent on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }


    vscphlp_deleteVSCPevent( pEvent );  // This helper is the same as the above two commented lines
    pEvent = NULL;
    
    
    
    // Read event2
    pEvent = malloc( sizeof( vscpEvent ) );
    pEvent->pdata = NULL;   // A must for a successful delete
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_receiveEvent( handle2, pEvent ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_receiveEvent on handle2\n" );
            printf( "VSCP class=%d VSCP type=%d sizeData=%d\n", 
                        pEvent->vscp_class,
                        pEvent->vscp_type,
                        pEvent->sizeData );
            printf("Data = ");
            for ( int i=0; i<pEvent->sizeData; i++ ) {
                printf("%d ", pEvent->pdata[i] );
            }
            printf("\n");
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_receiveEvent on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }

    // Free the event
    vscphlp_deleteVSCPevent( pEvent );
    pEvent = NULL;

    // Read event3   -  Use vscpEventEx
    vscpEventEx ex2;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_receiveEventEx( handle2, &ex2 ) ) ) {
        if (verbose_success) { 
            printf( "Command success: vscphlp_receiveEventEx on handle2\n" );
            printf( "VSCP class=%d VSCP type=%d sizeData=%d\n", 
                        ex2.vscp_class,
                        ex2.vscp_type,
                        ex2.sizeData );
            printf("Data = ");
            for ( int i=0; i<ex2.sizeData; i++ ) {
                printf("%d ", ex2.data[i] );
            }
            printf("\n");
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_receiveEventEx on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }
    

    // Get status
    VSCPStatus status;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getStatus( handle2, &status ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_getStatus on channel 2\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getStatus on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }



    // Get statistics
    VSCPStatistics stat;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getStatistics( handle2, &stat ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_getStatistics on channel 2\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getStatistics on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }

    // Set VSCP filter
    vscpEventFilter filter;
    filter.filter_class = 22;   // We are interested  in events with VSCP class=22 only
    filter.mask_class = 0xffff;
    filter.mask_type = 0;               // Any type
    filter.mask_priority = 0;           // Any priority
    memset( filter.mask_GUID, 0, 16 );  // Any GUID
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_setFilter( handle2, &filter ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setFilter on channel 2\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setFilter on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }


    // Display # events in the queue on channel2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_isDataAvailable on handle2\n" );
            printf( "count before sending two events = %u\n", count );
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_isDataAvailable on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }


    // Send event that should not be received
    e.vscp_class = 10;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEvent( handle1, &e ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_sendEvent on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_sendEvent on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    // Send event that should be received
    e.vscp_class = 22;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEvent( handle1, &e ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_sendEvent on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_sendEvent on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    // Display # events in the queue on channel2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_isDataAvailable on handle2\n" );
            printf( "count after sending two events (+1) = %u\n", count );
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_isDataAvailable on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }


    // Clear the filter
    memset( &filter, 0, sizeof( vscpEventFilter ) );
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_setFilter( handle2, &filter ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setFilter on channel 2\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setFilter on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }

    // Get server version
    unsigned char v1,v2,v3;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getVersion( handle2, &v1, &v2, &v3 ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_getVersion on channel 2\n" );
            printf( "Version for VSCP daemon on channel 2 is %d.%d.%d\n", v1,v2,v3 );
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getVersion on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }


    // Get DLL version
    unsigned long dllversion;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getDLLVersion( handle2, &dllversion ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_getDLLVersion on channel 2\n" );
            printf( "DL(L) version is %08lX\n", dllversion );
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getDLLVersion on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }


    // Get vendorstring
    char buf[120];
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getVendorString( handle2, buf, sizeof(buf) ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_getVendorString on channel 2\n" );
            printf( "Vendorstring = \"%s\"\n", buf );
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getVendorString on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }

    // Get driver info
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getDriverInfo( handle2, buf, sizeof(buf) ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_getDriverInfo on channel 2\n" );
            printf( "Driver info = \"%s\"\n", buf );
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getDriverInfo on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }

    
#ifdef TEST_RECEIVE_LOOP

    if (verbose_info) printf("\n* * * * * Receive loop  * * * * *\n");

    // Enter receiveloop
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_enterReceiveLoop( handle2 ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_enterReceiveLoop on channel 2\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_enterReceiveLoop on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }

    if (verbose_info) {
        printf("* * * * Waiting for five received events on channel 2 * * * * *\n");
        printf("You should send them. For example \"send 0,20,3,,,,0:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15,0,1,35\" in the tcp/ip interface will work \n");
    }

    int cntEvents = 0;
    while ( cntEvents < 5 ) {
        pEvent = malloc( sizeof( vscpEvent ) );
        pEvent->pdata = NULL;   // A must for a successful delete
        if ( VSCP_ERROR_SUCCESS == vscphlp_blockingReceiveEvent( handle2, pEvent, 1000 ) ) {
            if (verbose_success) {
                printf( "Command success: vscphlp_blockingReceiveEvent on channel 2\n" );
                printf(" Event: class=%d Type=%d sizeData=%d\n", 
                            pEvent->vscp_class,
                            pEvent->vscp_type,
                            pEvent->sizeData );            
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    printf("Data = ");
                    for ( int i=0; i<pEvent->sizeData; i++ ) {
                        printf("%d ", pEvent->pdata[i] );
                    }
                    printf("\n");
                }
            }
            cntEvents++;
        }
        vscphlp_deleteVSCPevent( pEvent );
        pEvent = NULL;
    }


    // Quit receiveloop
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_quitReceiveLoop( handle2 ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_quitReceiveLoop on channel 2\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_quitReceiveLoop on channel 2  Error code=%d\n", rv);
        error_cnt++;
    }

#endif



#ifdef TEST_VARIABLE_HANDLING

    if (verbose_info) printf("\n\n\n* * * * * Variables  * * * * *\n\n\a");
    
    
    if (verbose_info) {
        printf("\n\n********************************************************************\n");
        printf("               Test string variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    
    

    // Create a string variable
    if (verbose_info) printf("\n\n===== vscphlp_createRemoteVariable =====\n");
    // Value = dGhpcyBpcyB0aGUgdmFsdWUgb2YgdGhlIHN0cmluZyB2YXJpYWJsZSE= = "this is the value of the string variable!"
    // note = SnVzdCBhIG5vdGU= = "Just a note"
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_string_variable", 
                        "string",
                        0,
                        "",
                        0x744,
                        "dGhpcyBpcyB0aGUgdmFsdWUgb2YgdGhlIHN0cmluZyB2YXJpYWJsZSE=",
                        "SnVzdCBhIG5vdGU=")  ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    // Read a value from a string variable
    {
        if (verbose_success) printf("\n\n===== vscphlp_getRemoteVariableString =====\n");
        char strBuf[2000];
        memset( strBuf, 0, sizeof( strBuf ) );
        if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableString( handle1, "test_string_variable", strBuf, sizeof( strBuf )-1 ) ) ) {
            if (verbose_success) {
                printf( "Command success: vscphlp_getRemoteVariableString on channel 1\n" );
                printf(" Value = [%s] \n", strBuf );
            }
            if ( 0 != strcmp( strBuf, "dGhpcyBpcyB0aGUgdmFsdWUgb2YgdGhlIHN0cmluZyB2YXJpYWJsZSE=" ) ) {
                printf("\aError (vscphlp_getRemoteVariableString): Written value does not match read value!\n");
            }
            else {
                if (verbose_success) printf("Written value match read value!\n");
            }
        }
        else {
            if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableString on channel 1  Error code=%d\n", rv);
            error_cnt++;
        }
    }

    // Set a value from a string variable
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableString =====\n");    

    // Set value to "Q2FycGUgRGllbQ==" = Carpe Diem
    if ( VSCP_ERROR_SUCCESS == 
        (rv = vscphlp_setRemoteVariableString( handle1, "test_string_variable", "Q2FycGUgRGllbQ==" ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableString on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableString on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }

    // Read a value from a string variable
    {
        if (verbose_info) printf("\n\n===== vscphlp_getRemoteVariableString =====\n");
        char strBuf[2000];
        memset( strBuf, 0, sizeof( strBuf ) );
        if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableString( handle1, "test_string_variable", strBuf, sizeof( strBuf )-1 ) ) ) {
            if (verbose_success) {
                printf( "Command success: vscphlp_getRemoteVariableString on channel 1\n" );
                printf(" Value = [%s]\n", strBuf );
            }
            if ( 0 != strcmp( strBuf, "Q2FycGUgRGllbQ==" ) ) {
                printf("\aError (vscphlp_getRemoteVariableString): Written value does not match read value!\n");
            }
            else {
                if (verbose_success) printf("Written value match read value!\n");
            }
        }
        else {
            if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableString on channel 1  Error code=%d\n", rv);
            error_cnt++;
        }
    }
    
    
    if (verbose_info) {
        printf("\n\n********************************************************************\n");
        printf("               Test delete variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    

    // Delete a variable
    if (verbose_info) printf("\n\n===== vscphlp_deleteRemoteVariable =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_deleteRemoteVariable( handle1, 
                                             "test_string_variable" ) ) )  {
        if (verbose_info) printf( "Command success: vscphlp_deleteRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_deleteRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    
    if (verbose_info) { 
        printf("\n\n********************************************************************\n");
        printf("               Test boolean variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    

    
    // Create a boolean variable
    if (verbose_info) printf("\n\n===== vscphlp_createRemoteVariable =====\n");

    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_bool_variable", 
                        "bool",
                        0,
                        "",
                        0x744,
                        "true",
                        "VGhpcyBpcyBhIG5vdGU=")  ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }

    // Write a value (false) to a boolean variable
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableBool =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableBool( handle1, "test_bool_variable", 0 )  ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableBool on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableBool on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    // Read a value from a boolean variable
    if (verbose_info) printf("\n\n===== vscphlp_getRemoteariableBool =====\n");
    int valBool;
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableBool( handle1, "test_bool_variable", &valBool ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableBool on channel 1\n" );
            printf(" Value = %s\n", valBool ? "true" : "false" );
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableBool on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    if (verbose_success) {
        printf("\n\n********************************************************************\n");
        printf("               Test integer variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    
    


    // Create an int variable
    if (verbose_success) printf("\n\n===== vscphlp_createRemoteVariable =====\n");

    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_integer_variable", 
                        "integer",
                        0,
                        "",
                        0x744,
                        "1234",
                        "VGhpcyBpcyBhIG5vdGU=")  ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    // Write a value to an int variable
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableInt =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableInt( handle1, "test_integer_variable", 18666 )  ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableInt on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableInt on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    // Read a value from a int variable
    if (verbose_info) printf("\n\n===== vscphlp_getRemoteVariableInt =====\n");
    int intValue;
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableInt( handle1, "test_integer_variable", &intValue ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableInt on channel 1\n" );
            printf(" Value = %d\n", intValue );
        }
        if ( intValue == 18666 ) {
            if (verbose_success) printf("\aRead value is correct!\n");
        }
        else {
            if (verbose_error) printf("\aError (vscphlp_getRemoteVariableInt):Read value is NOT correct!\n");
            error_cnt++;
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableInt on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }



    if (verbose_info) {
        printf("\n\n********************************************************************\n");
        printf("               Test long variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    
    
    


    // Create a  long variable
    if (verbose_info) printf("\n\n===== vscphlp_createRemoteVariable =====\n");

    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_long_variable", 
                        "long",
                        0,
                        "",
                        0x744,
                        "1234567",
                        "VGhpcyBpcyBhIG5vdGU=")  ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    // Write a value to an long variable
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableLong =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableLong( handle1, "test_long_variable", 123456780 )  ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableLong on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableLong on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    // Read a value from a long variable
    if (verbose_info) printf("\n\n===== vscphlp_getRemoteVariableLong =====\n");
    long longValue;
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableLong( handle1, "test_long_variable", &longValue ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableLong on channel 1\n" );
            printf(" Value = %ld\n", longValue );
        }
        if ( longValue == 123456780 ) {
            if (verbose_success) printf("\aRead value is correct!\n");
        }
        else {
            if (verbose_error) printf("\aError (vscphlp_getRemoteVariableLong):Read value is NOT correct!\n");
            error_cnt++;
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableLong on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    if (verbose_info) { 
        printf("\n\n********************************************************************\n");
        printf("               Test floating point variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    
    

    // Create a float variable
    if (verbose_info) printf("\n\n===== vscphlp_createRemoteVariable =====\n");

    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_float_variable", 
                        "float",
                        0,
                        "",
                        0x744,
                        "1.2345",
                        "VGhpcyBpcyBhIG5vdGU=")  ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    // Write a value to an float variable
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableDouble =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableDouble( handle1, "test_float_variable", 1.2345001 )  ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableDouble on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableDouble on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    // Read a value from a float variable
    if (verbose_info) printf("\n\n===== vscphlp_getRemoteVariableDouble =====\n");
    double floatValue;
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableDouble( handle1, "test_float_variable", &floatValue ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableDouble on channel 1\n" );
            printf(" Value = %f\n", floatValue );
        }
        if ( (round( 10000*floatValue )/10000 ) == 1.2345 ) {
            if (verbose_success) printf("\aRead value is correct (checked four decimals)!\n");
        }
        else {
            if (verbose_error) printf("\aError (vscphlp_getRemoteVariableDouble):Read value is NOT correct (checked four decimals)!\n");
            error_cnt++;
        }
    }
    else {
        if (verbose_success) printf("\aCommand error: vscphlp_getRemoteVariableDouble on channel 1  Error code=%d\n", rv);
    }


    if (verbose_info) {
        printf("\n\n********************************************************************\n");
        printf("               Test measurement variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    
    

    // Create measurement variable
   if (verbose_info)  printf("\n----------------- vscphlp_createRemoteVariable -----------------\n");
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_measurement_variable", 
                        "measurement",
                        0,
                        "",
                        0x744,
                        "13.82,1,0,255,255",
                        "VGhpcyBpcyBhIG5vdGU=")  ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    // Write a value to an measurement variable
    if (verbose_info) printf("\n----------------- vscphlp_setRemoteVariableMeasurement -----------------\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableMeasurement( handle1, 
                                                            "test_measurement_variable", 
                                                            1.234,
                                                            2,
                                                            7,
                                                            11,
                                                            22 ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableMeasurement on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableMeasurement on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    // Read a value from a measurement variable 
    if (verbose_info) printf("\n----------------- vscphlp_getRemoteVariableMeasurement -----------------\n");
    {
        double value;
        int unit,sensoridx,zone,subzone;

        if ( VSCP_ERROR_SUCCESS == 
                ( rv = vscphlp_getRemoteVariableMeasurement( handle1, 
                                                                "test_measurement_variable",
                                                                &value,
                                                                &unit,
                                                                &sensoridx,
                                                                &zone,
                                                                &subzone ) ) ) {
            if (verbose_success) {
                printf( "Command success: vscphlp_getRemoteVariableMeasurement on channel 1\n" );
                printf(" Value = %f, unit = %d, sensoridx = %d, zone = %d, subzone = %d\n", 
                        value, unit, sensoridx, zone, subzone  );
            }
            if ( ((round( 10000*value )/10000 ) == 1.2340 ) &&
                    ( 2 == unit ) &&
                    ( 7 == sensoridx ) && 
                    ( 11 == zone ) &&
                    ( 22 == subzone ) ) {
                if (verbose_success) printf("\aRead values is correct (value checked for four decimals)!\n");
            }
            else {
                if (verbose_error) printf("\aError (vscphlp_getRemoteVariableMeasurement): Read values is NOT correct (value checked four decimals)!\n");
                error_cnt++;
            }
        }
        else {
            if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableMeasurement on channel 1  Error code=%d\n", rv);
            error_cnt++;
        }
    }


    if (verbose_info) { 
        printf("\n\n********************************************************************\n");
        printf("               Test event variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    


    // Create event variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_event_variable", 
                        "event",
                        0,
                        "",
                        0x744,
                        "",     /* Empty value is reset value */
                        "VGhpcyBpcyBhIG5vdGU=")  ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    // Write a value to an event variable
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableEvent =====\n");
    
    pEvent = malloc( sizeof( vscpEvent ) );
    pEvent->head = 0;
    pEvent->vscp_class = 10;
    pEvent->vscp_type = 6;
    pEvent->obid = 0;
    pEvent->timestamp = 0;
    memset( pEvent->GUID, 0, 16 );
    pEvent->sizeData = 4;
    pEvent->pdata = malloc( sizeof( unsigned char[4] ) );
    pEvent->pdata[ 0 ] = 10;
    pEvent->pdata[ 1 ] = 20;
    pEvent->pdata[ 2 ] = 30;
    pEvent->pdata[ 3 ] = 40;
    
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableEvent( handle1, "test_event_variable", pEvent )  ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableEvent on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableEvent on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    // Read a value from a event variable 
    if (verbose_info) printf("\n\n==== vscphlp_getRemoteVariableString =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableEvent( handle1, "test_event_variable", pEvent  ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableEvent on channel 1\n" );
            printf(" Event: class=%d Type=%d sizeData=%d\n", 
                            pEvent->vscp_class,
                            pEvent->vscp_type,
                            pEvent->sizeData );
            if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                printf("Data = ");
                for ( int i=0; i<pEvent->sizeData; i++ ) {
                    printf("%d ", pEvent->pdata[i] );
                }
                printf("\n");
            }
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableEvent on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    // Free the event
    vscphlp_deleteVSCPevent( pEvent );
    pEvent = NULL;

    
    if (verbose_info) {
        printf("\n\n********************************************************************\n");
        printf("               Test eventx variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    
    
    
    // Create eventex variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_eventex_variable", 
                        "event",
                        0,
                        "",
                        0x744,
                        "",     /* Empty value is reset value */
                        "VGhpcyBpcyBhIG5vdGU=")  ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    // Write a value to an event variable
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableEventEx =====\n");
    vscpEventEx ex1;
    ex1.head = 0;
    ex1.vscp_class = 50;
    ex1.vscp_type = 22;
    ex1.obid = 0;
    ex1.timestamp = 0;
    memset( ex1.GUID, 0, 16 );
    ex1.sizeData = 4;
    ex1.data[ 0 ] = 40;
    ex1.data[ 1 ] = 30;
    ex1.data[ 2 ] = 20;
    ex1.data[ 3 ] = 10;
    
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableEventEx( handle1, "test_eventex_variable", &ex1 )  ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableEventEx on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableEventEx on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    // Read a value from a event variable 
    if (verbose_info) printf("\n\n===== vscphlp_getRemoteVariableEventEx =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableEventEx( handle1, "test_eventex_variable", &ex1  ) ) ) {
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableEventEx on channel 1\n" );
            printf(" Event: class=%d Type=%d sizeData=%d\n", 
                            ex1.vscp_class,
                            ex1.vscp_type,
                            ex1.sizeData );
            if ( ex1.sizeData ) {
                printf("Data = ");
                for ( int i=0; i<ex1.sizeData; i++ ) {
                    printf("%d ", ex1.data[i] );
                }
                printf("\n");
            }
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableEvent on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }

    
    if (verbose_info) {
        printf("\n\n********************************************************************\n");
        printf("               Test GUID [string] variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    
    
    // Create GUID variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_guidstr_variable", 
                        "guid",
                        0,
                        "",
                        0x744,
                        "",     /* Empty value is reset value */
                        "VGhpcyBpcyBhIG5vdGU=") ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }

    // Write a value to an GUID variable - string type
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableGUIDString =====\n");
    char strGUID[64];
    strcpy( strGUID, "FF:FF:FF:FF:FF:FF:FF:00:00:00:00:7F:00:01:01:FD" );

    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableGUIDString( handle1, "test_guidstr_variable", strGUID ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableGUIDString on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableGUIDString on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }

    memset( strGUID, 0, sizeof(strGUID) );
   
    // Read a value from a GUID variable - string type
    if (verbose_info) printf("\n\n===== vscphlp_getRemoteVariableGUIDString =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableGUIDString( handle1, "test_guidstr_variable", strGUID, sizeof(strGUID)-1 )  ) )  {        
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableGUIDString on channel 1\n" );
            printf(" Value = %s\n", strGUID );
        }
        if ( 0 == strcmp( strGUID, "FF:FF:FF:FF:FF:FF:FF:00:00:00:00:7F:00:01:01:FD") ) {
            if (verbose_success) printf(" GUID strings are the same.\n");
        }
        else {
            if (verbose_error) printf("Error (vscphlp_getRemoteVariableGUIDString): Read GUID string are NOT the same.\n");
            error_cnt++;
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableGUIDString on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    
    
    if (verbose_info) {
        printf("\n\n********************************************************************\n");
        printf("               Test GUID [array] variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    



    // Create GUID variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_guidarray_variable", 
                        "eventguid",    /* Alternative name */
                        0,
                        "",
                        0x744,
                        "",     /* Empty value is reset value */
                        "VGhpcyBpcyBhIG5vdGU=") ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    // Write a value to an GUID variable - array type
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableGUIDArray =====\n");
    unsigned char GUID[16];
    memset( GUID, 0, 16 );
    for ( int i=0;i<16; i++ ) {
        GUID[i] = i;
    }

    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableGUIDArray( handle1, "test_guidarray_variable", GUID ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableGUIDArray on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableGUIDArray on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }

    memset( GUID, 0, 16 );
   
    // Read a value from a GUID variable - array type
    if (verbose_info) printf("\n\n===== vscphlp_getRemoteVariableGUIDArray =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableGUIDArray( handle1, "test_guidarray_variable", GUID  ) ) )  {
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableGUIDArray on channel 1\n" );
            printf(" Value = " );
            for ( int i=0; i<16; i++ ) {
                printf("%d ", GUID[i] );    
            }
            printf("\n");
        }
        int err = 0;
        for ( int i=0; i<16; i++ ) {
            if ( i != GUID[i] ) {
                if (verbose_error) printf("Error (vscphlp_getRemoteVariableGUIDArray): GUID's are NOT the same\n " );
                err = 1;
                break;
            } 
        }
        if ( !err && verbose_success ) printf(" GUID's are the same\n " );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableGUIDArray on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }

    
    
    if (verbose_info) {
        printf("\n\n********************************************************************\n");
        printf("              Test VCSP Data variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    
    


    // Create fata array variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_dataarray_variable", 
                        "eventdata",    /* Alternative name */
                        0,
                        "",
                        0x744,
                        "",     /* Empty value is reset value */
                        "VGhpcyBpcyBhIG5vdGU=") ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    unsigned char dataArray[10];
    memset( dataArray, 0, sizeof(dataArray) );
    for ( int i=0; i<sizeof(dataArray); i++ ) {
        dataArray[ i ] = i;    
    }

    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableVSCPData =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableVSCPData( handle1, "test_dataarray_variable", dataArray, sizeof( dataArray ) ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableVSCPData on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableVSCPData on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }

    unsigned short size;
    memset( dataArray, 0, sizeof( dataArray ) );
   
    // Read a value from a data variable 
    if (verbose_info) printf("\n\n===== vscphlp_getRemoteVariableVSCPData =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableVSCPData( handle1, "test_dataarray_variable", dataArray, &size  ) ) )  {
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableVSCPData on channel 1\n" );
            printf(" Value = " );
            for ( int i=0; i<size; i++ ) {
                printf("%d ", dataArray[i] );    
            }
            printf("\n");
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableVSCPData on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }


    
    

    if (verbose_success) {
        printf("\n\n********************************************************************\n");
        printf("               Test VSCP class variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    
    

    
    // Create event class variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_vscp_class_variable", 
                        "eventclass",    /* Alternative name */
                        0,
                        "",
                        0x744,
                        "",     /* Empty value is reset value */
                        "VGhpcyBpcyBhIG5vdGU=") ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
    }
    
    // Write a value for VSCP class type
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableVSCPClass =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableVSCPClass( handle1, "test_vscp_class_variable", 10 ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableVSCPClass on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableVSCPClass on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
   
    unsigned short vscpclass;

    // Read a value from aVSCP class type
    if (verbose_info) printf("\n\n===== vscphlp_getRemoteVariableVSCPClass =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableVSCPClass( handle1, "test_vscp_class_variable", &vscpclass ) ) )  {
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableVSCPClass on channel 1\n" );
            printf(" Value = %d\n", vscpclass );
        }
        if ( 10 == vscpclass ) {
            if (verbose_success) printf("Read value is the same as written\n");
        }
        else {
            if (verbose_error) printf("\aError (vscphlp_getRemoteVariableVSCPClass): Read value is NOT the same as written\n");
            error_cnt++;
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableVSCPClass on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }

    
    if (verbose_info) {
        printf("\n\n********************************************************************\n");
        printf("               Test VSCP type variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    
    
    

    // Create event type variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_vscp_type_variable", 
                        "eventtype",    /* Alternative name */
                        0,
                        "",
                        0x744,
                        "",     /* Empty value is reset value */
                        "VGhpcyBpcyBhIG5vdGU=") ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    // Write a value for VSCP type type
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableVSCPType =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableVSCPType( handle1, "test_vscp_type_variable", 22 ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableVSCPType on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableVSCPType on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
   
    unsigned short vscptype;

    // Read a value from aVSCP type type
    if (verbose_info) printf("\n\n===== vscphlp_getRemoteVariableVSCPType =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableVSCPType( handle1, "test_vscp_type_variable", &vscptype ) ) )  {
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableVSCPType on channel 1\n" );
            printf(" Value = %d\n", vscptype );
        }
        if ( 22 == vscptype ) {
            if (verbose_success) printf("Read value is the same as written\n");
        }
        else {
            if (verbose_error) printf("\aError: Read value is NOT the same as written\n");
            error_cnt++;
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableVSCPType on channel 1  Error code=%d\n", rv);
        error_cnt++;        
    }


    
    if (verbose_info) {
        printf("\n\n********************************************************************\n");
        printf("               Test VSCP timestamp variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    
    
    

    // Create variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_vscp_timestamp_variable", 
                        "timestamp",    
                        0,
                        "",
                        0x744,
                        "",     /* Empty value is reset value */
                        "VGhpcyBpcyBhIG5vdGU=") ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    // Write a value 
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableVSCPTimestamp =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableVSCPTimestamp( handle1, "test_vscp_timestamp_variable", 223344 ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableVSCPTimestamp on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableVSCPTimestamp on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
   
    unsigned long vscptimestamp;

    // Read a value from aVSCP type type
    if (verbose_info) printf("\n\n===== vscphlp_getRemoteVariableVSCPTimestamp =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableVSCPTimestamp( handle1, 
                                                            "test_vscp_timestamp_variable", 
                                                            &vscptimestamp ) ) )  {
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableVSCPTimestamp on channel 1\n" );
            printf(" Value = %lu\n", vscptimestamp );
        }
        if ( 223344 == vscptimestamp ) {
            if (verbose_success) printf("Read value is the same as written\n");
        }
        else {
            if (verbose_error) printf("\aError (vscphlp_getRemoteVariableVSCPTimestamp): Read value is NOT the same as written\n");
            error_cnt++;
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableVSCPTimestamp on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }







    
    if (verbose_info) {
        printf("\n\n********************************************************************\n");
        printf("               Test DateTime variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    
    
    

    // Create variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_datetime_variable", 
                        "datetime", 
                        0,
                        "",
                        0x744,
                        "",     /* Empty value is reset value */
                        "VGhpcyBpcyBhIG5vdGU=") ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    // Write a value 
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableDateTime =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableDateTime( handle1, "test_datetime_variable", "1956-11-02T17:12:01" ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableDateTime on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableDateTime on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
   
    char datetimebuf[80];
    memset( datetimebuf, 0, sizeof( datetimebuf) );

    // Read a value from a VSCP type type
    if (verbose_info) printf("\n\n===== vscphlp_getRemoteVariableDateTime =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableDateTime( handle1, 
                                                            "test_datetime_variable", 
                                                            datetimebuf,
                                                            sizeof( datetimebuf ) ) ) )  {
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableDateTime on channel 1\n" );
            printf(" Value = %s\n", datetimebuf );
        }
        if ( 0 == strcmp( datetimebuf, "1956-11-02T17:12:01" ) ) {
            if (verbose_success) printf("Read value is the same as written\n");
        }
        else {
            if (verbose_success) printf("\aError (vscphlp_getRemoteVariableDateTime): Read value is NOT the same as written\n");
            error_cnt++;
        }
    }
    else {
        if (verbose_success) printf("\aCommand error: vscphlp_getRemoteVariableDate on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }










    
    if (verbose_info) {
        printf("\n\n********************************************************************\n");
        printf("               Test Date variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    
    
    

    // Create variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_date_variable", 
                        "date", 
                        0,
                        "",
                        0x744,
                        "",     /* Empty value is reset value */
                        "VGhpcyBpcyBhIG5vdGU=") ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    // Write a value 
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableDate =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableDate( handle1, "test_date_variable", "1956-11-02" ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableDate on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableDate on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
   
    char datebuf[80];
    memset( datebuf, 0, sizeof( datebuf) );

    // Read a value from a VSCP type type
    if (verbose_info) printf("\n\n===== vscphlp_getRemoteVariableDate =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableDate( handle1, 
                                                            "test_date_variable", 
                                                            datebuf,
                                                            sizeof( datebuf ) ) ) )  {
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableDate on channel 1\n" );
            printf(" Value = %s\n", datebuf );
        }
        if ( 0 == strcmp( datebuf, "1956-11-02" ) ) {
            if (verbose_success) printf("Read value is the same as written\n");
        }
        else {
            if (verbose_success) printf("\aError (vscphlp_getRemoteVariableDate): Read value is NOT the same as written\n");
            error_cnt++;
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableDate on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }













    
    if (verbose_info) {
        printf("\n\n********************************************************************\n");
        printf("               Test Time variable reading/writing \n");
        printf("********************************************************************\n\n");
    }
    
    
    

    // Create variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_time_variable", 
                        "time", 
                        0,
                        "",
                        0x744,
                        "",     /* Empty value is reset value */
                        "VGhpcyBpcyBhIG5vdGU=") ) ) {
        if (verbose_success) printf( "Command success: vscphlp_createRemoteVariable on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_createRemoteVariable on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    
    // Write a value 
    if (verbose_info) printf("\n\n===== vscphlp_setRemoteVariableTime =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableTime( handle1, "test_time_variable", "19:22:11" ) ) ) {
        if (verbose_success) printf( "Command success: vscphlp_setRemoteVariableTime on channel 1\n" );
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_setRemoteVariableTime on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
   
    char timebuf[80];
    memset( timebuf, 0, sizeof( timebuf) );

    // Read a value from a VSCP type type
    if (verbose_info) printf("\n\n===== vscphlp_getRemoteVariableTime =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableTime( handle1, 
                                                            "test_time_variable", 
                                                            timebuf,
                                                            sizeof( timebuf ) ) ) )  {
        if (verbose_success) {
            printf( "Command success: vscphlp_getRemoteVariableTime on channel 1\n" );
            printf(" Value = %s\n", timebuf );
        }
        if ( 0 == strcmp( timebuf, "19:22:11" ) ) {
            if (verbose_success) printf("Read value is the same as written\n");
        }
        else {
            if (verbose_error) printf("\aError (vscphlp_getRemoteVariableTime): Read value is NOT the same as written\n");
            error_cnt++;
        }
    }
    else {
        if (verbose_error) printf("\aCommand error: vscphlp_getRemoteVariableTime on channel 1  Error code=%d\n", rv);
        error_cnt++;
    }
    // Save variables marked as persistent
    /*printf("\n\n===== vscphlp_saveRemoteVariablesToDisk =====\n");
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_saveRemoteVariablesToDisk( handle1, "download", 0, "" ) ) )  {
        printf( "Command success: vscphlp_saveRemoteVariablesToDisk on channel 1\n" );
    }
    else {
        printf("\aCommand error: vscphlp_saveRemoteVariablesToDisk on channel 1  Error code=%d\n", rv);
    }*/
    
#endif








// -------------------------------------------------------------------------------------------------








#ifdef TEST_HELPERS

    if (verbose_info) { 
        printf("\n\n");
        printf("\n\nTesting helpers\n");
        printf("===============\n");
    }


    pEvent = malloc( sizeof( vscpEvent ) );
    pEvent->head = 0;
    pEvent->vscp_class = 10;
    pEvent->vscp_type = 6;
    pEvent->obid = 0;
    pEvent->timestamp = 0;
    memset( pEvent->GUID, 0, 16 );
    pEvent->sizeData = 3;
    pEvent->pdata = malloc( sizeof( unsigned char[3] ) );
    pEvent->pdata[ 0 ] = 138;
    pEvent->pdata[ 1 ] = 0;
    pEvent->pdata[ 2 ] = 30;

    vscpEventEx ex3;
    ex3.head = 0;
    ex3.vscp_class = 10;
    ex3.vscp_type = 6;
    ex3.obid = 0;
    ex3.timestamp = 0;
    memset( ex3.GUID, 0, 16 );
    ex3.sizeData = 3;
    ex3.data[ 0 ] = 138;
    ex3.data[ 1 ] = 0;
    ex3.data[ 2 ] = 30;
 
    long readValue = vscphlp_readStringValue("0x22");
    if ( 0x22 == readValue ) {
        if (verbose_success) printf("readStringValue correct (0x22 )= %ld\n", readValue );
    }
    else {
        if (verbose_error) printf("\aError:  readStringValue (0x22) = %ld\n", readValue );
        error_cnt++;
    }


    readValue = (long)vscphlp_readStringValue("-00000000099");
    if ( -99 == readValue ) {
        if (verbose_success) printf("readStringValue correct (-99)= %ld\n", readValue );
    }
    else {
        if (verbose_error) printf("\aError:  readStringValue (-99) = %ld\n", readValue );
        error_cnt++;
    }
    
    


    unsigned char eventPriority;
    if ( 0 == ( eventPriority = vscphlp_getVscpPriority( pEvent ) ) ) {
        if (verbose_success) printf("Event priority = %d\n", eventPriority );
    }
    else {
        if (verbose_error) printf("\aError (vscphlp_getVscpPriority): Event priority = %d\n", eventPriority );
        error_cnt++;
    }


    if ( 0 == ( eventPriority = vscphlp_getVscpPriorityEx( &ex3 ) ) ) {
        if (verbose_success) printf("EventEx priority = %d\n", eventPriority );
    }
    else {
        if (verbose_error) printf("\aError (vscphlp_getVscpPriorityEx): Get EventEx priority = %d\n", eventPriority );
        error_cnt++;
    }


    vscphlp_setVscpPriority( pEvent, 3 );
    if ( 3 == ( eventPriority = vscphlp_getVscpPriority( pEvent ) ) ) {
        if (verbose_success) printf("Event priority = %d\n", eventPriority );
    }
    else {
        if (verbose_error) printf("\aError: vscphlp_setVscpPriority = %d\n", eventPriority );
        error_cnt++;
    }


    vscphlp_setVscpPriorityEx( &ex3, 7 );
    if ( 7 == ( eventPriority = vscphlp_getVscpPriorityEx( &ex3 ) ) ) {
        if (verbose_success) printf("Event priority = %d\n", eventPriority );
    }
    else {
        if (verbose_error) printf("\aError: vscphlp_setVscpPriorityEx = %d\n", eventPriority );
        error_cnt++;
    }

    unsigned char vscphead;
    unsigned long canalid = 0x0c0a0601;
    vscphead = vscphlp_getVSCPheadFromCANALid( canalid );
    if ( 96 == vscphead ) {
        if (verbose_success) printf("VSCP head = %d\n", vscphead );
    }
    else {
        if (verbose_error) printf("\aError: vscphlp_getVSCPheadFromCANALid = %d\n", vscphead );
        error_cnt++;
    }


    unsigned short canal_vscpclass = vscphlp_getVSCPclassFromCANALid( canalid );
    if ( 10 == canal_vscpclass ) {
        if (verbose_success) printf("VSCP Class = %d\n", canal_vscpclass );
    }
    else {
        if (verbose_error) printf("\aError: vscphlp_getVSCPclassFromCANALid = %d\n", canal_vscpclass );
        error_cnt++;
    }


    unsigned short canal_vscptype = vscphlp_getVSCPtypeFromCANALid( canalid );
    if ( 6 == canal_vscptype ) {
        if (verbose_success) printf("VSCP Type = %d\n", canal_vscptype );
    }
    else {
        if (verbose_error) printf("\aError: vscphlp_getVSCPtypeFromCANALid = %d\n", canal_vscptype );
        error_cnt++;
    }

    unsigned char canal_nickname = vscphlp_getVSCPnicknameFromCANALid( canalid );
    if ( 1 == canal_nickname ) {
        if (verbose_success) printf("Nickname = %d\n", canal_nickname );
    }
    else {
        if (verbose_error) printf("\aError: vscphlp_getVSCPnicknameFromCANALid = %d\n", canal_nickname );
        error_cnt++;
    }

    unsigned long constr_canal_id2 = vscphlp_getCANALidFromVSCPdata( 3, 10, 6 ); 
    if ( 0x0c0a0600 == constr_canal_id2 ) {
        if (verbose_success) printf("Nickname = %08lX\n", constr_canal_id2 );
    }
    else {
        if (verbose_error) printf("\aError: vscphlp_getVSCPnicknameFromCANALid = %08lX\n", constr_canal_id2 );
        error_cnt++;
    }

    constr_canal_id2 = vscphlp_getCANALidFromVSCPevent( pEvent ); 
    if ( 0x0c0a0600 == constr_canal_id2 ) {
        if (verbose_success) printf("Nickname = %08lX\n", constr_canal_id2 );
    }
    else {
        if (verbose_error) printf("\aError: vscphlp_getCANALidFromVSCPevent = %08lX\n", constr_canal_id2 );
        error_cnt++;
    }  

    constr_canal_id2 = vscphlp_getCANALidFromVSCPeventEx( &ex3 ); 
    if ( 0x1c0a0600 == constr_canal_id2 ) {
        if (verbose_success) printf("Nickname = %08lX\n", constr_canal_id2 );
    }
    else {
        if (verbose_error) printf("\aError: vscphlp_getCANALidFromVSCPeventEx = %08lX\n", constr_canal_id2 );
        error_cnt++;
    } 


    // Calculate CRC for event
    unsigned short crc = vscphlp_calc_crc_Event( pEvent, 0 );
    if (verbose_success) printf("CRC = %04X\n", crc );

    // Calculate CRC for event
    crc = vscphlp_calc_crc_EventEx( pEvent, 0 );
    if (verbose_success) printf("CRC = %04X\n", crc );

    // Calculate CRC for GUID array
    unsigned char GUID2[16];
    memset( GUID2, 0, 16 );
    for ( int i=0;i<16; i++ ) {
        GUID2[i] = i;
    }
    unsigned char crc8 = vscphlp_calcCRC4GUIDArray( GUID2 );
    if (verbose_success) printf("CRC = %02X\n", crc8 );

    // Calculate GUID for GUID string
    char strguid[64], strguid2[64];
    strcpy( strguid, "FF:FF:FF:FF:FF:FF:FF:00:00:00:00:7F:00:01:01:FD" );
    crc8 = vscphlp_calcCRC4GUIDString( strguid);
    if (verbose_success) printf("CRC = %02X\n", crc8 );


    if ( VSCP_ERROR_SUCCESS == vscphlp_getGuidFromString( pEvent, strguid ) ) {
        vscphlp_writeGuidToString( pEvent, strguid2, sizeof( strguid2 )-1 );
        if (verbose_success) printf( "GUID=%s\n", strguid2 );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_getGuidFromString\n");
        error_cnt++;
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_getGuidFromStringEx( &ex3, strguid ) ) {
        vscphlp_writeGuidToStringEx( &ex3, strguid2, sizeof( strguid2 )-1 );
        if (verbose_success) printf( "GUID=%s\n", strguid2 );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_getGuidFromStringEx\n");
        error_cnt++;
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_writeGuidToString4Rows( pEvent, strguid2, sizeof( strguid2 )-1 ) ) {
        if (verbose_success) printf( "GUID\n%s\n", strguid2 );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_writeGuidToString4Rows\n");
        error_cnt++;
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_writeGuidToString4RowsEx( &ex3, strguid2, sizeof( strguid2 )-1 ) ) {
        if (verbose_success) printf( "GUID\n%s\n", strguid2 );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_writeGuidToString4RowsEx\n");
        error_cnt++;
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_writeGuidArrayToString( GUID2, strguid2, sizeof( strguid2 )-1 ) ) {
        if (verbose_success) printf( "GUID=%s\n", strguid2 );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_writeGuidArrayToString\n");
        error_cnt++;
    }

    unsigned char emptyGUID[16];
    memset( emptyGUID,0, 16 );
    if ( vscphlp_isGUIDEmpty( emptyGUID ) ) {
        if (verbose_success) printf( "vscphlp_isGUIDEmpty  - GUID is detected as empty as it should be\n" );    
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_isGUIDEmpty\n");
        error_cnt++;
    }

    if ( vscphlp_isGUIDEmpty( GUID2 ) ) {
        if (verbose_error) printf( "\aError: vscphlp_isGUIDEmpty\n");
        error_cnt++;
    }
    else {
        if (verbose_success) printf( "vscphlp_isGUIDEmpty  - GUID is detected as NOT empty as it should be\n" );
        
    }

    if ( vscphlp_isSameGUID( emptyGUID, GUID2) ) {
        if (verbose_error) printf( "\aError: vscphlp_isSameGUID\n");
        error_cnt++;
    }
    else {
        if (verbose_success) printf( "vscphlp_isSameGUID  - Correct, GUIDs are not the same.\n" );
    }

    vscphlp_writeGuidArrayToString( GUID2, strguid2, sizeof( strguid2 )-1 );
    if (verbose_success) printf( "GUID before reverse = %s\n", strguid2 );
    if ( VSCP_ERROR_SUCCESS == vscphlp_reverseGUID( GUID2 ) ) {
        vscphlp_writeGuidArrayToString( GUID2, strguid2, sizeof( strguid2 )-1 );
        if (verbose_success) printf( "GUID  after reverse = %s\n", strguid2 );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_reverseGUID\n");
        error_cnt++;
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_getGuidFromStringToArray( GUID2, strguid ) ) {
        vscphlp_writeGuidArrayToString( GUID2, strguid2, sizeof( strguid2 )-1 );
        if (verbose_success) printf( "GUID  after reverse = %s\n", strguid2 );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_getGuidFromStringToArray\n");
        error_cnt++;
    }

    vscpEventEx ex4;
    if ( VSCP_ERROR_SUCCESS != vscphlp_convertVSCPtoEx( &ex4, pEvent ) ) {
        if (verbose_error) printf( "\aError: vscphlp_getGuidFromStringToArray\n");
        error_cnt++;
    }

    vscpEvent *pEvent2 = malloc( sizeof( vscpEvent ) );
    pEvent2->pdata = NULL;
    if ( VSCP_ERROR_SUCCESS != vscphlp_convertVSCPfromEx( pEvent2, &ex4 ) ) {
        if (verbose_error) printf( "\aError: vscphlp_convertVSCPfromEx\n");
        error_cnt++;
    }
    vscphlp_deleteVSCPevent( pEvent2 );
    pEvent2 = NULL;

    //vscpEventFilter filter;
    vscphlp_clearVSCPFilter( &filter );

    if ( VSCP_ERROR_SUCCESS != vscphlp_readFilterFromString( &filter, 
                "1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00" ) ) {
        if (verbose_error) printf( "\aError: vscphlp_readFilterFromString\n");   
        error_cnt++;
    }
    else {
        if (verbose_success) printf( "OK: vscphlp_readFilterFromString\n");    
    }
    
    if ( VSCP_ERROR_SUCCESS != vscphlp_readMaskFromString( &filter, 
                "1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00" ) ) {
        if (verbose_error) printf( "\aError: vscphlp_readMaskFromString\n");   
        error_cnt++;
    }
    else {
        if (verbose_success) printf( "OK: vscphlp_readMaskFromString\n");    
    }

    
    if ( vscphlp_doLevel2Filter( pEvent, &filter ) ) {
        if (verbose_success) printf( "Event pass:  vscphlp_doLevel2Filter\n");
    }
    else {
        if (verbose_error) printf( "Event does NOT pass:  vscphlp_doLevel2Filter\n");
        error_cnt++;
    }

    vscpEvent *pEvent3 = malloc( sizeof( vscpEvent ) );
    pEvent3->pdata = NULL;
    canalMsg canalMsg = { 0 };
    canalMsg.id = 0x0c0a0601;
    canalMsg.sizeData = 3;
    canalMsg.data[0] = 138;
    canalMsg.data[1] = 0;
    canalMsg.data[2] = 30;
    if ( VSCP_ERROR_SUCCESS == vscphlp_convertCanalToEvent( pEvent3,
                                                               &canalMsg,
                                                               GUID2 ) ) {
        if (verbose_success) printf( "OK vscphlp_convertCanalToEvent VSCP class=%d Type=%d\n", pEvent3->vscp_class, pEvent3->vscp_type );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_convertCanalToEvent\n");
        error_cnt++;
    }

    // Free the event
    vscphlp_deleteVSCPevent( pEvent3 );
    pEvent3 = NULL;

    vscpEventEx ex5;
    if ( VSCP_ERROR_SUCCESS == vscphlp_convertCanalToEventEx( &ex5,
                                                                &canalMsg,
                                                                GUID2 ) ) {
        if (verbose_success) printf( "OK vscphlp_convertCanalToEventEx VSCP class=%d Type=%d\n", ex5.vscp_class, ex5.vscp_type );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_convertCanalToEvent\n");
        error_cnt++;
    }

    
    if ( VSCP_ERROR_SUCCESS == vscphlp_convertEventToCanal( &canalMsg, pEvent ) ) {
        if (verbose_success) printf( "OK vscphlp_convertEventToCanal id=%08lX\n", canalMsg.id );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_convertEventToCanal\n");
        error_cnt++;
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_convertEventExToCanal( &canalMsg, &ex5 ) ) {
        if (verbose_success) printf( "OK vscphlp_convertEventExToCanal id=%08lX\n", canalMsg.id );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_convertEventExToCanal\n");
        error_cnt++;
    }


    // Can this one be wrong (if so add fault case)
    if (verbose_success) printf( ">>> OK vscphlp_makeTimeStamp  %04lX\n", vscphlp_makeTimeStamp() );

    
    vscpEvent *pEventFrom = malloc( sizeof( vscpEvent ) );
    vscpEvent *pEventTo = malloc( sizeof( vscpEvent ) );
    pEventFrom->head = 0;
    pEventFrom->vscp_class = 10;
    pEventFrom->vscp_type = 6;
    pEventFrom->obid = 0;
    pEventFrom->timestamp = 0;
    memset( pEventFrom->GUID, 0, 16 );
    pEventFrom->sizeData = 2;
    pEventFrom->pdata = malloc( sizeof( unsigned char[2] ) );
    pEventFrom->pdata[ 0 ] = 0xAA;
    pEventFrom->pdata[ 1 ] = 0x55;

    if ( VSCP_ERROR_SUCCESS == vscphlp_copyVSCPEvent( pEventTo, pEventFrom ) ) {
        if (verbose_success) printf( "OK vscphlp_copyVSCPEvent %02X %02X \n", pEventTo->pdata[0], pEventTo->pdata[1] );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_copyVSCPEvent\n");
        error_cnt++;
    }



    // Free the events
    vscphlp_deleteVSCPevent( pEventFrom );
    pEventFrom = NULL;
    
    vscphlp_deleteVSCPevent( pEventTo );
    pEventTo = NULL;

    char dataBuf[80];
    if ( VSCP_ERROR_SUCCESS == vscphlp_writeVscpDataToString( pEvent, 
                                                                dataBuf, 
                                                                sizeof( dataBuf )-1,
                                                                0 ) ) {
       if (verbose_success) printf( "OK vscphlp_writeVscpDataToString \n%s \n", dataBuf );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_writeVscpDataToString\n");
        error_cnt++;
    }


    unsigned char dataArray2[32];
    unsigned short sizeData;
    if ( VSCP_ERROR_SUCCESS == 
             vscphlp_setVscpDataArrayFromString( dataArray2, 
                                       &sizeData,
                                       "1,2,3,4,5,6,0x07,0x55,3,4,0xaa,0xff,0xff" ) ) {
        if (verbose_success) printf( "OK vscphlp_setVscpDataArrayFromString size=%d Data = \n", sizeData );
        for ( int i=0; i<sizeData; i++ ) {
            if (verbose_success) printf("%d ", dataArray2[i] );
        }
        printf("\n");
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_setVscpDataArrayFromString\n");
        error_cnt++;
    }
    
    
    if ( VSCP_ERROR_SUCCESS == 
             vscphlp_setVscpDataFromString( &e, "1,2,3,4,5,6,0x07,0x55,3,4,0xaa,0xff,0xff" ) ) {
        if (verbose_success) printf( "OK vscphlp_setVscpDataFromString size=%d Data = \n", e.sizeData );
        for ( int i=0; i<e.sizeData; i++ ) {
            if (verbose_success) printf("%d ", e.pdata[i] );
        }
        if (verbose_success) printf("\n");
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_setVscpDataFromString\n");
        error_cnt++;
    }


    char eventBuf[128];
    if ( VSCP_ERROR_SUCCESS == vscphlp_writeVscpEventToString( pEvent, eventBuf, sizeof( eventBuf )-1 ) ) {
        if (verbose_success) printf( "OK vscphlp_writeVscpEventToString Event = %s\n", eventBuf );    
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_writeVscpEventToString\n");
        error_cnt++;
    }


    if ( VSCP_ERROR_SUCCESS == vscphlp_writeVscpEventExToString( &ex3, eventBuf, sizeof( eventBuf )-1 ) ) {
        if (verbose_success) printf( "OK vscphlp_writeVscpEventToString Event = %s\n", eventBuf );    
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_writeVscpEventToString\n");
        error_cnt++;
    }


    vscpEvent *pEventString1 = malloc( sizeof( vscpEvent ) );
    pEventString1->pdata = NULL;

    if ( VSCP_ERROR_SUCCESS == vscphlp_setVscpEventFromString( pEventString1, 
                             "0,10,6,0,0,FF:FF:FF:FF:FF:FF:FF:00:00:00:00:7F:00:01:01:FD,0x8A,0x00,0x1E" ) ) {
        if (verbose_success) printf( "OK vscphlp_setVscpEventFromString class=%d Type=%d\n", 
                   pEventString1->vscp_class, pEventString1->vscp_type );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_setVscpEventFromString\n");
        error_cnt++;
    }

    // Free the events
    vscphlp_deleteVSCPevent( pEventString1 );
    pEventString1 = NULL;


    vscpEventEx ex6;
    if ( VSCP_ERROR_SUCCESS == vscphlp_setVscpEventExFromString( &ex6, 
                             "0,10,6,0,0,FF:FF:FF:FF:FF:FF:FF:00:00:00:00:7F:00:01:01:FD,0x8A,0x00,0x1E" ) ) {
        if (verbose_success) printf( "OK vscphlp_setVscpEventExFromString class=%d Type=%d\n", 
                   ex6.vscp_class, ex6.vscp_type );
    }
    else {
        if (verbose_error) printf( "\aError: vscphlp_setVscpEventExFromString\n");
        error_cnt++;
    }
    
    vscphlp_deleteVSCPevent( pEvent );
    pEvent = NULL;


#endif











// -------------------------------------------------------------------------------------------------











#ifdef TEST_MEASUREMENT

    if (verbose_info) {
        printf("\n\n");
        printf("\n\nMeasurement helpers\n");
        printf("===================\n");
    }

    pEvent = malloc( sizeof( vscpEvent ) );
    pEvent->head = 0;
    pEvent->vscp_class = 10;
    pEvent->vscp_type = 6;
    pEvent->obid = 0;
    pEvent->timestamp = 0;
    memset( pEvent->GUID, 0, 16 );
    pEvent->sizeData = 3;
    pEvent->pdata = malloc( sizeof( unsigned char[3] ) );
    pEvent->pdata[ 0 ] = 138;
    pEvent->pdata[ 1 ] = 0;
    pEvent->pdata[ 2 ] = 30;

    unsigned char dataCoding = vscphlp_getMeasurementDataCoding( pEvent );
    if ( 138 == dataCoding  ) {
        if (verbose_success) printf("OK vscphlp_getMeasurementDataCoding - Data Coding = %d\n", dataCoding );
    }
    else {
        if (verbose_error) printf("\aError: vscphlp_getMeasurementDataCoding - Data Coding = %d\n", dataCoding );
        error_cnt++;
    }


    vscphlp_deleteVSCPevent( pEvent );
    pEvent = NULL;


    unsigned char bitarry[3];
    bitarry[0] = VSCP_DATACODING_BIT; // Data cding byte. Default unit, sensoridx=0
    bitarry[1] = 0x55;
    bitarry[2] = 0xAA;
    unsigned long long bitarray64 = vscphlp_getDataCodingBitArray( bitarry, sizeof( bitarry ) );
    if ( bitarray64 ==  0x55AA ) {
        if (verbose_success) printf("OK - vscphlp_getDataCodingBitArray \n");
    }
    else {
        if (verbose_success) printf("\aError: vscphlp_getDataCodingBitArray [%d]\n", dataCoding );
        error_cnt++;
    }

    unsigned char normarry[4];
    normarry[0] = 0x89; // Data coding byte: Normalized integer, unit=1, sensoridx=1
    normarry[1] = 0x82; // Decimal point two steps to the left
    normarry[2] = 0x01;
    normarry[3] = 0x36;
    double value =  vscphlp_getDataCodingNormalizedInteger( normarry, sizeof( normarry ) );
    if ( 3.1 == value ) {
        if (verbose_success) printf("OK - vscphlp_getDataCodingNormalizedInteger value = %f \n", value );
    }
    else {
        if (verbose_error) printf("Error - vscphlp_getDataCodingNormalizedInteger value = %f \n", value );
        error_cnt++;
    }

    normarry[0] = 0x60; // Data coding byte: Integer, unit=0, sensoridx=0
    normarry[1] = 0xFF;
    normarry[2] = 0xFF;
    normarry[3] = 0xFF;
    unsigned long long val64 = vscphlp_getDataCodingInteger( normarry, sizeof( normarry ) );
    if (verbose_success) printf("OK - vscphlp_getDataCodingInteger value = %llu \n", val64 );

    unsigned char stringarry[6];
    stringarry[0] = VSCP_DATACODING_STRING; // Data cding byte. Default unit, sensoridx=0
    stringarry[1] = 0x32;
    stringarry[2] = 0x33;
    stringarry[3] = 0x34;
    stringarry[4] = 0x2E;
    stringarry[5] = 0x35;
    char stringbuf[2048];
    if ( VSCP_ERROR_SUCCESS == vscphlp_getDataCodingString( stringarry,
                                                              sizeof( stringarry ), 
                                                              stringbuf,
                                                              sizeof( stringbuf ) ) ) {
        if (verbose_success) printf("OK - vscphlp_getDataCodingString value = %s \n", stringbuf );
    }
    else {
        if (verbose_error) printf("Error - vscphlp_getDataCodingString value = %s \n", stringbuf );
        error_cnt++;
    }



    vscpEvent *pEventMeasurement = malloc( sizeof( vscpEvent ) );
    pEventMeasurement->head = 0;
    pEventMeasurement->vscp_class = 10;
    pEventMeasurement->vscp_type = 6;
    pEventMeasurement->obid = 0;
    pEventMeasurement->timestamp = 0;
    memset( pEventMeasurement->GUID, 0, 16 );
    pEventMeasurement->sizeData = 4;
    pEventMeasurement->pdata = malloc( sizeof( unsigned char[ pEventMeasurement->sizeData ] ) );
    pEventMeasurement->pdata[0] = 0x89;
    pEventMeasurement->pdata[1] = 0x02;
    pEventMeasurement->pdata[2] = 0x00;
    pEventMeasurement->pdata[3] = 0xF1;

    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsString( pEventMeasurement, 
                                                                    stringbuf, 
                                                                    sizeof( stringbuf ) ) ) {
        if (verbose_success) printf("OK - vscphlp_getVSCPMeasurementAsString value = %s \n", stringbuf );
    }
    else {
        if (verbose_error) printf("Error - vscphlp_getVSCPMeasurementAsString value = %s \n", stringbuf );
        error_cnt++;
    }




    pEventMeasurement->pdata[0] = 0x89;
    pEventMeasurement->pdata[1] = 0x02;
    pEventMeasurement->pdata[2] = 0xFF;
    pEventMeasurement->pdata[3] = 0xF1;

    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsString( pEventMeasurement, 
                                                                    stringbuf, 
                                                                    sizeof( stringbuf ) ) ) {
        if (verbose_success) printf("OK - vscphlp_getVSCPMeasurementAsString value = %s \n", stringbuf );
    }
    else {
        if (verbose_error) printf("Error - vscphlp_getVSCPMeasurementAsString value = %s \n", stringbuf );
        error_cnt++;
    }




    pEventMeasurement->pdata[0] = VSCP_DATACODING_BIT;
    pEventMeasurement->pdata[1] = 0x55;
    pEventMeasurement->pdata[2] = 0xAA;
    pEventMeasurement->pdata[3] = 0x55;

    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsString( pEventMeasurement, 
                                                                    stringbuf, 
                                                                    sizeof( stringbuf ) ) ) {
        if (verbose_success) printf("OK - vscphlp_getVSCPMeasurementAsString value = %s \n", stringbuf );
    }
    else {
        if (verbose_error) printf("Error - vscphlp_getVSCPMeasurementAsString value = %s \n", stringbuf );
        error_cnt++;
    }



    pEventMeasurement->pdata[0] = VSCP_DATACODING_BYTE;
    pEventMeasurement->pdata[1] = 0x55;
    pEventMeasurement->pdata[2] = 0xAA;
    pEventMeasurement->pdata[3] = 0x55;

    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsString( pEventMeasurement, 
                                                                    stringbuf, 
                                                                    sizeof( stringbuf ) ) ) {
        if (verbose_success) printf("OK - vscphlp_getVSCPMeasurementAsString value = %s \n", stringbuf );
    }
    else {
        if (verbose_error) printf("Error - vscphlp_getVSCPMeasurementAsString value = %s \n", stringbuf );
        error_cnt++;
    }


    pEventMeasurement->pdata[0] = VSCP_DATACODING_STRING;
    pEventMeasurement->pdata[1] = 0x33;
    pEventMeasurement->pdata[2] = 0x31;
    pEventMeasurement->pdata[3] = 0x34;

    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsString( pEventMeasurement, 
                                                                    stringbuf, 
                                                                    sizeof( stringbuf ) ) ) {
        if (verbose_success) printf("OK - vscphlp_getVSCPMeasurementAsString value = %s \n", stringbuf );
    }
    else {
        if (verbose_error) printf("Error - vscphlp_getVSCPMeasurementAsString value = %s \n", stringbuf );
        error_cnt++;
    }


    pEventMeasurement->pdata[0] = VSCP_DATACODING_INTEGER;
    pEventMeasurement->pdata[1] = 0x00;
    pEventMeasurement->pdata[2] = 0xFF;
    pEventMeasurement->pdata[3] = 0xFF;

    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsString( pEventMeasurement, 
                                                                    stringbuf, 
                                                                    sizeof( stringbuf ) ) ) {
        if (verbose_success) printf("OK - vscphlp_getVSCPMeasurementAsString value = %s \n", stringbuf );
    }
    else {
        if (verbose_error) printf("Error - vscphlp_getVSCPMeasurementAsString value = %s \n", stringbuf );
        error_cnt++;
    }



    pEventMeasurement->pdata[0] = VSCP_DATACODING_INTEGER;
    pEventMeasurement->pdata[1] = 0xFF;
    pEventMeasurement->pdata[2] = 0xFF;
    pEventMeasurement->pdata[3] = 0xFF;

    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsString( pEventMeasurement, 
                                                                    stringbuf, 
                                                                    sizeof( stringbuf ) ) ) {
        if (verbose_success) printf("OK - vscphlp_getVSCPMeasurementAsString value = %s \n", stringbuf );
    }
    else {
        if (verbose_error) printf("Error - vscphlp_getVSCPMeasurementAsString value = %s \n", stringbuf );
        error_cnt++;
    }



    pEventMeasurement->pdata[0] = VSCP_DATACODING_INTEGER;
    pEventMeasurement->pdata[1] = 0xFF;
    pEventMeasurement->pdata[2] = 0xFF;
    pEventMeasurement->pdata[3] = 0xFF;
    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsDouble( pEventMeasurement, &value ) ) {
        if (verbose_success) printf("OK - vscphlp_getVSCPMeasurementAsDouble value = %lf\n", value );
    }
    else {
        if (verbose_error) printf("Error - vscphlp_getVSCPMeasurementAsDouble value = %lf \n", value );
        error_cnt++;
    }

    vscpEvent *pEventfloat = malloc( sizeof( vscpEvent ) );
    pEventfloat->head = 0;
    pEventfloat->vscp_class = 10;
    pEventfloat->vscp_type = 6;
    pEventfloat->obid = 0;
    pEventfloat->timestamp = 0;
    memset( pEventfloat->GUID, 0, 16 );
    pEventfloat->sizeData = 8;
    pEventfloat->pdata = malloc( sizeof( unsigned char[ pEventfloat->sizeData ] ) );
    pEventfloat->pdata[0] = 234;
    pEventfloat->pdata[1] = 46;
    pEventfloat->pdata[2] = 68;
    pEventfloat->pdata[3] = 84;
    pEventfloat->pdata[4] = 251;
    pEventfloat->pdata[5] = 33;
    pEventfloat->pdata[6] = 9;
    pEventfloat->pdata[7] = 64;

    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementFloat64AsString( pEventfloat, 
                                                                            stringbuf, 
                                                                            sizeof( stringbuf ) ) ) {
        if (verbose_success) printf("OK - vscphlp_getVSCPMeasurementFloat64AsString value = %s \n", stringbuf );
    }
    else {
        if (verbose_error) printf("Error - vscphlp_getVSCPMeasurementFloat64AsString value = %s \n", stringbuf );
        error_cnt++;
    }

    // Free the event
    vscphlp_deleteVSCPevent( pEventfloat );
    pEventfloat = NULL;

    // Free the event
    vscphlp_deleteVSCPevent( pEventMeasurement );
    pEventMeasurement = NULL;    

#endif


    // -------------------------------------------------------------------------------------------------
    

    // Free the event (should be NULL here)
    if ( NULL == pEvent ) {
        if (verbose_success) printf("pEvent is NULL.\n");
    }
    else {
        if (verbose_error) printf("Error: pEvent working event Not NULL.\n");
    }
    if (verbose_info) printf(">>> Delete pEvent working event.\n");
    vscphlp_deleteVSCPevent( pEvent );
    pEvent = NULL;

    if (verbose_info) printf("\n\n\n");


    // free data
    if (verbose_info) printf(">>> Free 'e' working event data.\n");
    if ( NULL != pEvent) free( e.pdata );

    if ( VSCP_ERROR_SUCCESS == vscphlp_close( handle1 ) ) {
        if (verbose_success) printf( "Command success: vscphlp_close on channel 1\n" );
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_close( handle2 ) ) {
        if (verbose_success) printf( "Command success: vscphlp_close on channel 2\n" );
    }

    vscphlp_closeSession( handle1 );
    vscphlp_closeSession( handle2 );

    // ---------------------------------------------------------------------------------

    uint8_t iv1[5] = {0xae,0x41,0x83,0x80,0x00};
    float fval = vscp_getMeasurementAsFloat(iv1,5);
    printf("Value is %f [16.437500]", fvalue );

    printf("\n\nHit ENTER to terminate Error Count= %d\n", error_cnt );
    (void)getchar();
   
    return 0;
    
}
