// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include "../../vscphelperlib.h"

// If TEST_RECEIVE_LOOP is uncommented the rcvloop commands
// will be tested. Must send five events externally to test
//#define TEST_RECEIVE_LOOP 

// Uncomment to test variable handling
#define TEST_VARIABLE_HANDLING 

int _tmain(int argc, _TCHAR* argv[])
{
    int rv;
    long handle1, handle2;

    printf("VSCP helperlib test program\n");
    printf("===========================\n");

    handle1 = vscphlp_newSession();
    if (0 != handle1 ) {
        printf( "Handle one OK %d\n", handle1 );
    }
    else {
        printf("\aError: Failed to get handle for channel 1\n");
    }

    handle2 = vscphlp_newSession();
    if (0 != handle2 ) {
        printf( "Handle two OK %d\n", handle2 );
    }
    else {
        printf("\aError: Failed to get handle for channel 2\n");
    }

    // Open Channel 1
    rv=vscphlp_open( handle1, 
                         "192.168.1.9:9598",
                         "admin",
                        "secret" ); 
    if ( VSCP_ERROR_SUCCESS == rv ) {
        printf("Command success: vscphlp_open on channel 1\n");
    }
    else {
        printf("\aCommand error: vscphlp_open on channel 1  Error code=%d\n", rv);
    }

    // OPEN channel 2
    rv=vscphlp_openInterface( handle2, "192.168.1.9:9598;admin;secret", 0 ); 
    if ( VSCP_ERROR_SUCCESS == rv ) {
        printf("Command success: vscphlp_openInterface on channel 2\n");
    }
    else {
        printf("\aCommand error: vscphlp_openInterface on channel 2  Error code=%d\n", rv);
    }

    // NOOP on handle1
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_noop( handle1 ) ) ) {
        printf( "Command success: vscphlp_noop on channel 1\n" );
    }
    else {
        printf("\aCommand error: vscphlp_noop on channel 1  Error code=%d\n", rv);
    }

    // NOOP on handle2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_noop( handle2 ) ) ) {
        printf( "Command success: vscphlp_noop on channel 2\n" );
    }
    else {
        printf("\aCommand error: vscphlp_noop on channel 2  Error code=%d\n", rv);
    }

    // Get version on handle1
    unsigned char majorVer, minorVer, subminorVer;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getVersion( handle1, &majorVer, &minorVer, &subminorVer ) ) ) {
        printf( "channel 1: Major version=%d  Minor version=%d  Sub Minor verion=%d\n", 
                majorVer, 
                minorVer, 
                subminorVer );
    }
    else {
        printf("\aCommand error: vscphlp_getVersion on channel 1  Error code=%d\n", rv);
    }


    // Send event on channel 1
    vscpEvent e;
    e.vscp_class = 10;  // CLASS1.MEASUREMENT
    e.vscp_type = 6;    // Temperature
    e.head = 0;
    e.sizeData = 3;
    e.pdata = new unsigned char[3];
    e.pdata[0] = 138;  // Six degrees Celsius from sensor 2
    e.pdata[1] = 0;
    e.pdata[2] = 6;
    memset(e.GUID, 0, sizeof(e.GUID) ); // Setting GUID to all zero tell interface to use it's own GUID

    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEvent( handle1, &e ) ) ) {
        printf( "Command success: vscphlp_sendEvent on channel 1\n" );
    }
    else {
        printf("\aCommand error: vscphlp_sendEvent on channel 1  Error code=%d\n", rv);
    }

    // Do it again
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEvent( handle1, &e ) ) ) {
        printf( "Command success: vscphlp_sendEvent on channel 1\n" );
    }
    else {
        printf("\aCommand error: vscphlp_sendEvent on channel 1  Error code=%d\n", rv);
    }


    // Two events should now have been received on handle2
    unsigned int count;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        printf( "Command success: vscphlp_isDataAvailable on handle2\n" );
        if ( 2 == count ) {
            printf( "Two events waiting to be fetched on channel 2." );
        }
        else if ( count > 2 ) {
            printf( "%d events waiting to be fetched on channel 2.", count );
        }
        else {
            printf( "%d events waiting to be fetched on channel 2 [Other source is also sending events].", count );
        }
    }
    else {
        printf("\aCommand error: vscphlp_isDataAvailable on channel 2  Error code=%d\n", rv);
    }


    // Clear the event queue on the demon
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_clearDaemonEventQueue( handle2 ) ) ) {
        printf( "Command success: vscphlp_clearDaemonEventQueue input queue on channel 2\n", 
                majorVer, 
                minorVer, 
                subminorVer );
    }
    else {
        printf("\aCommand error: vscphlp_clearDaemonEventQueue on channel 2  Error code=%d\n", rv);
    }


    // We should now have an empty queue on channel2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        printf( "Command success: vscphlp_isDataAvailable on handle2\n" );
        printf( "count = %d\n", count );
        if ( !count ) printf("Which is correct.\n");
    }
    else {
        printf("\aCommand error: vscphlp_isDataAvailable on channel 2  Error code=%d\n", rv);
    }

    vscpEventEx ex;
    ex.vscp_class = 10; // CLASS1.MEASUREMENT
    ex.vscp_type = 6;   // Temperature
    ex.head = 0;
    ex.sizeData = 3;
    ex.data[0] = 138;   // 6 degrees Celsius from sensor 2
    ex.data[1] = 0;
    ex.data[2] = 6;
    memset(ex.GUID, 0, sizeof(e.GUID) ); // Setting GUID to all zero tell interface to use it's own GUID

    // Send event again
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEventEx( handle1, &ex ) ) ) {
        printf( "Command success: vscphlp_sendEventEx on channel 1\n" );
    }
    else {
        printf("\aCommand error: vscphlp_sendEventEx on channel 1  Error code=%d\n", rv);
    }

    ex.data[0] = 138;  // 1.11 degrees Celsius from sensor 2
    ex.data[1] = 2;
    ex.data[2] = 111; 

    // Send event again
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEventEx( handle1, &ex ) ) ) {
        printf( "Command success: vscphlp_sendEventEx on channel 1\n" );
    }
    else {
        printf("\aCommand error: vscphlp_sendEventEx on channel 1  Error code=%d\n", rv);
    }


    ex.data[0] = 138;  // -1 degrees Celsius from sensor 2
    ex.data[1] = 0;
    ex.data[2] = 255; 

    // Send event again
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEventEx( handle1, &ex ) ) ) {
        printf( "Command success: vscphlp_sendEventEx on channel 1\n" );
    }
    else {
        printf("\aCommand error: vscphlp_sendEventEx on channel 1  Error code=%d\n", rv);
    }

    // We should now have three events in the queue on channel2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        printf( "Command success: vscphlp_isDataAvailable on handle2\n" );
        printf( "count = %d\n", count );
        if ( 3 == count ) printf("Which is correct.\n");
    }
    else {
        printf("\aCommand error: vscphlp_isDataAvailable on channel 2  Error code=%d\n", rv);
    }


    // Read event1
    vscpEvent *pEvent = new vscpEvent;
    pEvent->pdata = NULL;   // A must for a successful delete
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_receiveEvent( handle2, pEvent ) ) ) {
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
    else {
        printf("\aCommand error: vscphlp_receiveEvent on channel 2  Error code=%d\n", rv);
    }

    //delete pEvent->pdata;
    //delete pEvent;
    vscphlp_deleteVSCPevent( pEvent );  // This helper is the same as the above two commented lines

    
    
    
    // Read event2
    pEvent = new vscpEvent;
    pEvent->pdata = NULL;   // A must for a successful delete
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_receiveEvent( handle2, pEvent ) ) ) {
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
    else {
        printf("\aCommand error: vscphlp_receiveEvent on channel 2  Error code=%d\n", rv);
    }

    // Free the event
    vscphlp_deleteVSCPevent( pEvent );


    // Read event3   -  Use vscpEventEx
    vscpEventEx ex2;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_receiveEventEx( handle2, &ex2 ) ) ) {
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
    else {
        printf("\aCommand error: vscphlp_receiveEventEx on channel 2  Error code=%d\n", rv);
    }
    

    // Get status
    VSCPStatus status;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getStatus( handle2, &status ) ) ) {
        printf( "Command success: vscphlp_getStatus on channel 2\n" );
    }
    else {
        printf("\aCommand error: vscphlp_getStatus on channel 2  Error code=%d\n", rv);
    }


    // Get statistics
    VSCPStatistics stat;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getStatistics( handle2, &stat ) ) ) {
        printf( "Command success: vscphlp_getStatistics on channel 2\n" );
    }
    else {
        printf("\aCommand error: vscphlp_getStatistics on channel 2  Error code=%d\n", rv);
    }

    // Set VSCP filter
    vscpEventFilter filter;
    filter.filter_class = 22;   // We are interested  in events with VSCP class=22 only
    filter.mask_class = 0xffff;
    filter.mask_type = 0;               // Any type
    filter.mask_priority = 0;           // Any priority
    memset( filter.mask_GUID, 0, 16 );  // Any GUID
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_setFilter( handle2, &filter ) ) ) {
        printf( "Command success: vscphlp_setFilter on channel 2\n" );
    }
    else {
        printf("\aCommand error: vscphlp_setFilter on channel 2  Error code=%d\n", rv);
    }


    // Display # events in the queue on channel2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        printf( "Command success: vscphlp_isDataAvailable on handle2\n" );
        printf( "count before sending two events = %d\n", count );
    }
    else {
        printf("\aCommand error: vscphlp_isDataAvailable on channel 2  Error code=%d\n", rv);
    }


    // Send event that should not be received
    e.vscp_class = 10;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEvent( handle1, &e ) ) ) {
        printf( "Command success: vscphlp_sendEvent on channel 1\n" );
    }
    else {
        printf("\aCommand error: vscphlp_sendEvent on channel 1  Error code=%d\n", rv);
    }


    // Send event that should be received
    e.vscp_class = 22;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEvent( handle1, &e ) ) ) {
        printf( "Command success: vscphlp_sendEvent on channel 1\n" );
    }
    else {
        printf("\aCommand error: vscphlp_sendEvent on channel 1  Error code=%d\n", rv);
    }


    // Display # events in the queue on channel2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        printf( "Command success: vscphlp_isDataAvailable on handle2\n" );
        printf( "count after sending two events (+1) = %d\n", count );
    }
    else {
        printf("\aCommand error: vscphlp_isDataAvailable on channel 2  Error code=%d\n", rv);
    }


    // Clear the filter
    memset( &filter, 0, sizeof( vscpEventFilter ) );
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_setFilter( handle2, &filter ) ) ) {
        printf( "Command success: vscphlp_setFilter on channel 2\n" );
    }
    else {
        printf("\aCommand error: vscphlp_setFilter on channel 2  Error code=%d\n", rv);
    }

    // Get server version
    unsigned char v1,v2,v3;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getVersion( handle2, &v1, &v2, &v3 ) ) ) {
        printf( "Command success: vscphlp_getVersion on channel 2\n" );
        printf( "Version for VSCP daemon on channel 2 is %d.%d.%d\n", v1,v2,v3 );
    }
    else {
        printf("\aCommand error: vscphlp_getVersion on channel 2  Error code=%d\n", rv);
    }


    // Get DLL version
    unsigned long dllversion;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getDLLVersion( handle2, &dllversion ) ) ) {
        printf( "Command success: vscphlp_getDLLVersion on channel 2\n" );
        printf( "DL(L) version is %08X\n", dllversion );
    }
    else {
        printf("\aCommand error: vscphlp_getDLLVersion on channel 2  Error code=%d\n", rv);
    }


    // Get vendorstring
    char buf[120];
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getVendorString( handle2, buf, sizeof(buf) ) ) ) {
        printf( "Command success: vscphlp_getVendorString on channel 2\n" );
        printf( "Vendorstring = \"%s\"\n", buf );
    }
    else {
        printf("\aCommand error: vscphlp_getVendorString on channel 2  Error code=%d\n", rv);
    }

    // Get driver info
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getDriverInfo( handle2, buf, sizeof(buf) ) ) ) {
        printf( "Command success: vscphlp_getDriverInfo on channel 2\n" );
        printf( "Driver info = \"%s\"\n", buf );
    }
    else {
        printf("\aCommand error: vscphlp_getDriverInfo on channel 2  Error code=%d\n", rv);
    }

    
#ifdef TEST_RECEIVE_LOOP

    printf("\n* * * * * Receive loop  * * * * *\n");

    // Enter receiveloop
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_enterReceiveLoop( handle2 ) ) ) {
        printf( "Command success: vscphlp_enterReceiveLoop on channel 2\n" );
    }
    else {
        printf("\aCommand error: vscphlp_enterReceiveLoop on channel 2  Error code=%d\n", rv);
    }

    printf("* * * * Waiting for five received events on channel 2 * * * * *\n", rv);

    int cntEvents = 0;
    while ( cntEvents < 5 ) {
        pEvent = new vscpEvent;
        pEvent->pdata = NULL;   // A must for a successfull delete
        if ( VSCP_ERROR_SUCCESS == vscphlp_blockingReceiveEvent( handle2, pEvent ) ) {
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
            cntEvents++;
        }
        vscphlp_deleteVSCPevent( pEvent );
    }


    // Quit receiveloop
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_quitReceiveLoop( handle2 ) ) ) {
        printf( "Command success: vscphlp_quitReceiveLoop on channel 2\n" );
    }
    else {
        printf("\aCommand error: vscphlp_quitReceiveLoop on channel 2  Error code=%d\n", rv);
    }


#endif



#ifdef TEST_VARIABLE_HANDLING

    printf("\n* * * * * Variables  * * * * *\n");

    // Write a value to a string variable
    if ( VSCP_ERROR_SUCCESS == 
        (rv = vscphlp_setVariableString( handle1, "test_string_variable", "this is the value of the string variable!" )  ) ) {
        printf( "Command success: vscphlp_setVariableString on channel 1\n" );
    }
    else {
        printf("\aCommand error: vscphlp_setVariableString on channel 1  Error code=%d\n", rv);
    }


    // Read a value from a string variable
    char strBuf[32];
    if ( VSCP_ERROR_SUCCESS == 
        (rv = vscphlp_getVariableString( handle1, "test_string_variable", strBuf, sizeof( strBuf ) ) ) ) {
        printf( "Command success: vscphlp_getVariableString on channel 1\n" );
        printf(" Value = %d\n");
    }
    else {
        printf("\aCommand error: vscphlp_getVariableString on channel 1  Error code=%d\n", rv);
    }



    // Write a value to a string variable
    if ( VSCP_ERROR_SUCCESS == 
        (rv = vscphlp_setVariableBool( handle1, "test_bool_variable", -1 )  ) ) {
        printf( "Command success: vscphlp_setVariableBool on channel 1\n" );
    }
    else {
        printf("\aCommand error: vscphlp_setVariableBool on channel 1  Error code=%d\n", rv);
    }


    // Read a value from a boolean variable
    int valBool;
    if ( VSCP_ERROR_SUCCESS == 
        (rv = vscphlp_getVariableBool( handle1, "test_boolean_variable", &valBool ) ) ) {
        printf( "Command success: vscphlp_getVariableBool on channel 1\n" );
        printf(" Value = %s\n", valBool ? "true" : "false" );
    }
    else {
        printf("\aCommand error: vscphlp_getVariableBool on channel 1  Error code=%d\n", rv);
    }


#endif



    // ------------------------------------------------------------------------

    printf("\n\n\n");


    // free data
    delete e.pdata;


    if ( VSCP_ERROR_SUCCESS == vscphlp_close( handle1 ) ) {
        printf( "Command success: vscphlp_close on channel 1\n" );
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_close( handle2 ) ) {
        printf( "Command success: vscphlp_close on channel 2\n" );
    }

    vscphlp_closeSession( handle1 );
    vscphlp_closeSession( handle2 );

    printf("\n\nHit ENTER to terminate\n");
    int c = getchar();
   
	return 0;
}

