// bigtest.c 
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include "../../vscphelperlib.h"

#define USER            "admin"
#define PASSWORD        "secret"
#define PORT            9598

//#define HOST            "127.0.0.1"
#define HOST            "192.168.1.6"
//#define HOST            "185.144.156.45"
#define HOST		"192.168.1.30"   // pi4

//#define HOST_PLUS_PORT  "127.0.0.1:9598"
#define HOST_PLUS_PORT  "tcp://192.168.1.6:9598"
//#define HOST_PLUS_PORT  "185.144.156.45:9598"
#define HOST_PLUS_PORT  "tcp://192.168.1.30:9598"

//#define INTERFACE       "127.0.0.1:9598;admin;secret"
#define INTERFACE       "tcp://192.168.1.6:9598;admin;secret"
//#define INTERFACE       "185.144.156.45:9598;admin;secret"
//#define INTERFACE       "tcp://192.168.1.30:9598;admin;secret"

// Count for number of events sent in burst
#define BURST_SEND_COUNT    200

// If TEST_RECEIVE_LOOP is uncommented the rcvloop commands
// will be tested. Must send five events externally to test
#define TEST_RECEIVE_LOOP 

// Uncomment to test variable handling
#define TEST_VARIABLE_HANDLING 

// Uncomment to test helpers
#define TEST_HELPERS

// Uncomment to test measurement functionality
#define TEST_MEASUREMENT


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

///////////////////////////////////////////////////////////////////////////////
// closeAll
//

void closeAll( int h1, int h2 )
{
    if ( VSCP_ERROR_SUCCESS == vscphlp_close( h1 ) ) {
        printf( "vscphlp_close: Success on channel 1\n" );
    }
    else {
        printf( "vscphlp_close: Error on channel 1\n" );
    }

    vscphlp_closeSession( h1 );
    
    if ( VSCP_ERROR_SUCCESS == vscphlp_close( h2 ) ) {
        printf( "vscphlp_close: Success on channel 2\n" );
    }
    else {
        printf( "vscphlp_close: Error on channel 2\n" );
    }

    vscphlp_closeSession( h2 );    
}

///////////////////////////////////////////////////////////////////////////////
// main
//

int main(int argc, char* argv[])
{
    int rv;
    long handle1, handle2; 
    long long t1,t2;

    printf("VSCP helperlib test program\n");
    printf("VSCP helperlib test program\n");
    printf("===========================\n");

    handle1 = vscphlp_newSession();
    if (0 != handle1 ) {
        printf( "vscphlp_newSession: Success. handle 1 = %ld\n", handle1 );
    }
    else {
        printf("\avscphlp_newSession: Failure - channel 1\n");
        return -1;
    }

    handle2 = vscphlp_newSession();
    if (0 != handle2 ) {
        printf( "vscphlp_newSession: Success. handle 2 = %ld\n", handle2 );
    }
    else {
        printf("\avscphlp_newSession: Failure - channel 2\n");
        vscphlp_closeSession( handle1 );
        return -1;
    }

    // Open Channel 1
    rv=vscphlp_open( handle1, 
                         HOST_PLUS_PORT,
                         USER,
                         PASSWORD ); 
    if ( VSCP_ERROR_SUCCESS == rv ) {
        printf( "vscphlp_open: Success. - Channel 1\n" );
    }
    else {
        printf("vscphlp_open: Failure - Channel 1, rv = %d\n", rv );
        vscphlp_closeSession( handle1 );
        vscphlp_closeSession( handle2 );
        return -1;
    }

    // OPEN channel 2
    rv=vscphlp_openInterface( handle2, INTERFACE, 0 ); 
    if ( VSCP_ERROR_SUCCESS == rv ) {
        printf( "vscphlp_open: Success. - Channel 2\n" );
    }
    else {
        printf("vscphlp_open: Failure - Channel 2, rv = %d\n", rv );
        vscphlp_close( handle1 );
        vscphlp_closeSession( handle1 );
        vscphlp_closeSession( handle2 );
        return -1;
    }

    // NOOP on handle1
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_noop( handle1 ) ) ) {
        printf( "vscphlp_noop: Success. - Channel 1\n" );
    }
    else {
        printf("vscphlp_noop: Failure - Channel 1, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }

    // NOOP on handle2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_noop( handle2 ) ) ) {
        printf( "vscphlp_noop: Success. - Channel 2\n" );
    }
    else {
        printf("vscphlp_noop: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }

    // Get version on handle1
    unsigned char majorVer, minorVer, subminorVer;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getVersion( handle1, &majorVer, &minorVer, &subminorVer ) ) ) {
        printf( "vscphlp_getVersion: Success. - Channel 1\n" );
        printf( "channel 1: Major version=%d  Minor version=%d  Sub Minor version=%d\n", 
                majorVer, 
                minorVer, 
                subminorVer );
    }
    else {
        printf("vscphlp_getVersion: Failure - Channel 1, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }


    // Get version on handle2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getVersion( handle2, &majorVer, &minorVer, &subminorVer ) ) ) {
        printf( "vscphlp_getVersion: Success. - Channel 2\n" );
        printf( "channel 1: Major version=%d  Minor version=%d  Sub Minor version=%d\n", 
                majorVer, 
                minorVer, 
                subminorVer );
    }
    else {
        printf("vscphlp_getVersion: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }


    // Send event on channel 1
    vscpEvent e;
    e.vscp_class = 10;  // CLASS1.MEASUREMENT
    e.vscp_type = 6;    // Temperature
    e.head = 0;
    e.year = 1956;
    e.month = 11;
    e.day = 2;
    e.hour = 12;
    e.minute = 10;
    e.second = 3;
    e.sizeData = 3;
    e.timestamp = 0;
    e.pdata = malloc( sizeof( unsigned char[3] ) );
    e.pdata[0] = 138;  // Six degrees Celsius from sensor 2
    e.pdata[1] = 0;
    e.pdata[2] = 6;
    memset(e.GUID, 0, sizeof(e.GUID) ); // Setting GUID to all zero tell interface to use it's own GUID

    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEvent( handle1, &e ) ) ) {
        printf( "vscphlp_sendEvent: Success. - Try 1, Channel 1\n" );
    }
    else {
        printf("vscphlp_sendEvent: Failure - Try 1, Channel 1, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }

    // Do it again
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEvent( handle1, &e ) ) ) {
        printf( "vscphlp_sendEvent: Success. - Try 2, Channel 1\n" );
    }
    else {
        printf("vscphlp_sendEvent: Failure - Try 2, Channel 1, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }

    printf("Giving events time to be deliverd.\n");
    sleep( 1 );

    // Two events should now have been received on handle2
    unsigned int count;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        printf( "vscphlp_isDataAvailable: Success. - Channel 2\n" );
        if ( 2 == count ) {
            printf( "Two events waiting to be fetched on channel 2.\n" );
        }
        else if ( count < 2 ) {
            printf( "%u events waiting to be fetched on channel 2.\n", count );
            printf("vscphlp_isDataAvailable: Failure - Number of events in queue should be 2 or greater.\n" );
            closeAll( handle1, handle2 );
            return -1;
        }
        else {
            printf( "%u events waiting to be fetched on channel 2 [Other source is also sending events].\n", count );
        }
    }
    else {
        printf("vscphlp_isDataAvailable: Failure - Channel 1, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }


    // Clear the event queue on the demon channel 2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_clearDaemonEventQueue( handle2 ) ) ) {
        printf( "vscphlp_clearDaemonEventQueue: Success. - Channel 2\n" );
    }
    else {
        printf("vscphlp_clearDaemonEventQueue: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }


    // We should now have an empty queue on channel2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        printf( "vscphlp_isDataAvailable: Success. - Channel 2\n" );
        printf( "count = %u\n", count );
        if ( !count ) {
            printf("Which is correct.\n");
        }
        else {
            printf("Which is wrong (can be due to other event source).\n");
        }
    }
    else {
        printf("vscphlp_isDataAvailable: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }

    vscpEventEx ex;
    ex.vscp_class = 10; // CLASS1.MEASUREMENT
    ex.vscp_type = 6;   // Temperature
    ex.head = 0;
    ex.year = 1956;
    ex.month = 11;
    ex.day = 2;
    ex.hour = 12;
    ex.minute = 10;
    ex.second = 3;
    ex.timestamp = 0;   // Let interface set it
    ex.sizeData = 3;
    ex.data[0] = 138;   // 6 degrees Celsius from sensor 2
    ex.data[1] = 0;
    ex.data[2] = 6;
    memset(ex.GUID, 0, sizeof(e.GUID) ); // Setting GUID to all zero tell interface to use it's own GUID

    // Send event again
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEventEx( handle1, &ex ) ) ) {
        printf( "vscphlp_sendEventEx: Success. - 1 Channel 1\n" );
    }
    else {
        printf("vscphlp_sendEventEx: Failure - 1 Channel 1, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }

    ex.data[0] = 138;  // 1.11 degrees Celsius from sensor 2
    ex.data[1] = 2;
    ex.data[2] = 111; 

    // Send event again
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEventEx( handle1, &ex ) ) ) {
        printf( "vscphlp_sendEventEx: Success. - 2 Channel 1\n" );
    }
    else {
        printf("vscphlp_sendEventEx: Failure - 2 Channel 1, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }


    ex.data[0] = 138;  // -1 degrees Celsius from sensor 2
    ex.data[1] = 0;
    ex.data[2] = 255; 

    // Send event again
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEventEx( handle1, &ex ) ) ) {
        printf( "vscphlp_sendEventEx: Success. - 3 Channel 1\n" );
    }
    else {
        printf("vscphlp_sendEventEx: Failure - 3 Channel 1, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }

    printf("Giving events time to be delivered.\n");
    sleep( 1 );

    // We should now have three events in the queue on channel2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        printf( "vscphlp_isDataAvailable: Success. - Channel 2\n" );
        printf( "count = %u\n", count );
        if ( 3 == count ) printf("Which is correct.\n");
    }
    else {
        printf("vscphlp_isDataAvailable: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }


    // Read event1
    vscpEvent *pEvent = malloc( sizeof( vscpEvent ) );
    pEvent->pdata = NULL;   // A must for a successful delete
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_receiveEvent( handle2, pEvent ) ) ) {
        printf( "vscphlp_receiveEvent: Success. - Channel 2\n" );
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
        printf("vscphlp_receiveEvent: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }

    vscphlp_deleteVSCPevent( pEvent );  
    pEvent = NULL;
    
    
    
    // Read event2
    pEvent = malloc( sizeof( vscpEvent ) );
    pEvent->pdata = NULL;   // A must for a successful delete
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_receiveEvent( handle2, pEvent ) ) ) {
        printf( "vscphlp_receiveEvent: Success. - Channel 2\n" );
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
        printf("vscphlp_receiveEvent: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }

    // Free the event
    vscphlp_deleteVSCPevent( pEvent );
    pEvent = NULL;




    // Read event3   -  Use vscpEventEx
    vscpEventEx ex2;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_receiveEventEx( handle2, &ex2 ) ) ) {
        printf("vscphlp_receiveEventEx: Success - Channel 2, rv = %d\n", rv );
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
        printf("vscphlp_receiveEventEx: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }
    

    // Get status
    VSCPStatus status;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getStatus( handle2, &status ) ) ) {
        printf( "vscphlp_getStatus: Success. - Channel 2\n" );
    }
    else {
        printf("vscphlp_getStatus: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }


    // Get statistics
    VSCPStatistics stat;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getStatistics( handle2, &stat ) ) ) {
        printf( "vscphlp_getStatistics: Success. - Channel 2\n" );
    }
    else {
        printf("vscphlp_getStatistics: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }

    // Set VSCP filter
    vscpEventFilter filter;
    filter.filter_class = 22;           // We are interested  in events with VSCP class=22 only
    filter.mask_class = 0xffff;
    filter.mask_type = 0;               // Any type
    filter.mask_priority = 0;           // Any priority
    memset( filter.mask_GUID, 0, 16 );  // Any GUID
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_setFilter( handle2, &filter ) ) ) {
        printf( "vscphlp_setFilter: Success. - Channel 2\n" );
    }
    else {
        printf("vscphlp_setFilter: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }


    // Display # events in the queue on channel2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        printf( "vscphlp_isDataAvailable: Success. - Channel 2\n" );
        printf( "count before sending two events = %u\n", count );
    }
    else {
        printf("vscphlp_isDataAvailable: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }


    // Send event that should not be received
    e.vscp_class = 10;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEvent( handle1, &e ) ) ) {
        printf( "vscphlp_sendEvent: Success. - Channel 1\n" );
    }
    else {
        printf("vscphlp_sendEvent: Failure - Channel 1, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }


    // Send event that should be received
    e.vscp_class = 22;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEvent( handle1, &e ) ) ) {
        printf( "vscphlp_sendEvent: Success. - Channel 1\n" );
    }
    else {
        printf("vscphlp_sendEvent: Failure - Channel 1, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }


    // Display # events in the queue on channel2
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_isDataAvailable( handle2, &count ) ) ) {
        printf( "vscphlp_isDataAvailable: Success. - Channel 2\n" );
        printf( "count after sending two events (+1) = %u\n", count );
    }
    else {
        printf("vscphlp_isDataAvailable: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }


    // Clear the filter
    memset( &filter, 0, sizeof( vscpEventFilter ) );
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_setFilter( handle2, &filter ) ) ) {
        printf( "vscphlp_setFilter: Success. - Channel 2\n" );
    }
    else {
        printf("vscphlp_setFilter: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }

    // Get server version
    unsigned char v1,v2,v3;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getVersion( handle2, &v1, &v2, &v3 ) ) ) {
        printf( "vscphlp_getVersion: Success. - Channel 2\n" );
        printf( "Version for VSCP daemon on channel 2 is %d.%d.%d\n", v1,v2,v3 );
    }
    else {
        printf("vscphlp_getVersion: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }


    // Get DLL version
    unsigned long dllversion;
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getDLLVersion( handle2, &dllversion ) ) ) {
        printf( "vscphlp_getDLLVersion: Success. - Channel 2\n" );
        printf( "DL(L) version is %08lX\n", dllversion );
    }
    else {
        printf("vscphlp_getDLLVersion: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }


    // Get vendorstring
    char buf[120];
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getVendorString( handle2, buf, sizeof(buf) ) ) ) {
        printf( "vscphlp_getVendorString: Success. - Channel 2\n" );
        printf( "Vendorstring = \"%s\"\n", buf );
    }
    else {
        printf("vscphlp_getVendorString: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }

    // Get driver info
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_getDriverInfo( handle2, buf, sizeof(buf) ) ) ) {
        printf( "vscphlp_getDriverInfo: Success. - Channel 2\n" );
        printf( "Driver info = \"%s\"\n", buf );
    }
    else {
        printf("vscphlp_getDriverInfo: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }



// ****************************************************************************
//                            TEST RECEIVE LOOP
// ****************************************************************************


    
#ifdef TEST_RECEIVE_LOOP

    printf("\n* * * * * Receive loop  * * * * *\n");

    // Enter receiveloop
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_enterReceiveLoop( handle2 ) ) ) {
        printf( "vscphlp_enterReceiveLoop: Success. - Channel 2\n" );
    }
    else {
        printf("vscphlp_enterReceiveLoop: Failure - Channel 2, rv = %d\n", rv );
        closeAll( handle1, handle2 );
        return -1;
    }

    printf("* * * * Waiting for 200 received events on channel 2 * * * * *\n");

    // ----

    // Send event on channel 1
	vscpEventEx exa;
    exa.vscp_class = 10;    // CLASS1.MEASUREMENT
    exa.vscp_type = 1;      // Count
    exa.head = 0;
    exa.year = 1867;        // Marie Curie
    exa.month = 11;         
    exa.day = 7;
    exa.hour = 12;
    exa.minute = 10;
    exa.second = 3;
    exa.sizeData = 2;
    exa.timestamp = 0;
    exa.data[0] = 0;  
    exa.data[1] = 0;
    memset(exa.GUID, 0, sizeof(exa.GUID) ); // Setting GUID to all zero tell interface to use it's own GUID

    t1 = t2 = current_timestamp();
	for ( int i=0; i<BURST_SEND_COUNT; i++ ) {

		exa.data[0] = (i>>8) & 0xff;
    	exa.data[1] = i & 0xff;

    	if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_sendEventEx( handle1, &exa ) ) ) {
        	printf( "vscphlp_sendEvent: Success. %d\n", i );
    	}
    	else {
        	printf("vscphlp_sendEvent: Failure - rv = %d\n", rv );
        	closeAll( handle1, handle2 );
            return -1;
    	}
	}

    t2 = current_timestamp();
    printf("Burst send time %d ms\n", (int)(t2-t1) );

    // ----    

    int cntEvents = 0;
    int blockIteration = 0;

    t1 = t2 = current_timestamp();
    while ( cntEvents < BURST_SEND_COUNT ) {
        pEvent = malloc( sizeof( vscpEvent ) );
        pEvent->pdata = NULL;   // NULL a must for a successful delete
        if ( VSCP_ERROR_SUCCESS == ( rv = vscphlp_blockingReceiveEvent( handle2, pEvent, 30000 ) ) ) {
            printf( "vscphlp_blockingReceiveEvent: Success. - Channel 2 " );
            printf(" Event: class=%d Type=%d sizeData=%d ", 
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
	    else  
	    {
            printf("vscphlp_blockingReceiveEvent: %d Failure - Channel 2, rv = %d\n", blockIteration, rv );
            closeAll( handle1, handle2 );
        return -1;
	    }

        vscphlp_deleteVSCPevent( pEvent );
        pEvent = NULL;
        blockIteration++;

    }

    t2 = current_timestamp();
    printf("Burst receive time %d ms\n", (int)(t2-t1) );

    // Quit receiveloop
    if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_quitReceiveLoop( handle2 ) ) ) {
        printf( "vscphlp_quitReceiveLoop: Success. - Channel 2\n" );
    }
    else {
        printf("vscphlp_quitReceiveLoop: %d Failure - Channel 2, rv = %d\n", blockIteration, rv );
        closeAll( handle1, handle2 );
        return -1;
    }


#endif

    

//                               Variables



#ifdef TEST_VARIABLE_HANDLING



    printf("\n\n********************************************************************\n");
    printf("                          Variables \n");
    printf("********************************************************************\n\n");
    
    

    // Create a string variable
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
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    // Read a value from a string variable
    {
        char strBuf[2000];
        memset( strBuf, 0, sizeof( strBuf ) );
        if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableString( handle1, "test_string_variable", strBuf, sizeof( strBuf )-1 ) ) ) {
            printf( "vscphlp_getRemoteVariableString: Success on channel 1\n" );
            printf(" Value = [%s] \n", strBuf );
            if ( 0 != strcmp( strBuf, "dGhpcyBpcyB0aGUgdmFsdWUgb2YgdGhlIHN0cmluZyB2YXJpYWJsZSE=" ) ) {
                printf("vscphlp_getRemoteVariableString: Error - Written value does not match read value!\n");
                closeAll( handle1, handle2 );
                return -1;
            }
            else {
                printf("vscphlp_getRemoteVariableString: Success - Written value match read value!\n");
            }
        }
        else {
            printf("vscphlp_getRemoteVariableString: Error on channel 1  Error code=%d\n", rv);
            closeAll( handle1, handle2 );
            return -1;
        }
    }
    

    printf("Set value to \"Q2FycGUgRGllbQ==\" = 'Carpe Diem'\n");
    if ( VSCP_ERROR_SUCCESS == 
        (rv = vscphlp_setRemoteVariableString( handle1, "test_string_variable", "Q2FycGUgRGllbQ==" ) ) ) {
        printf( "vscphlp_getRemoteVariableString: Success - on channel 1\n" );
    }
    else {
        printf("vscphlp_getRemoteVariableString: Error - on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    

    // Read a value from a string variable
    {
        char strBuf[2000];
        memset( strBuf, 0, sizeof( strBuf ) );
        if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableString( handle1, "test_string_variable", strBuf, sizeof( strBuf )-1 ) ) ) {
            printf( "vscphlp_getRemoteVariableString: Success on channel 1\n" );
            printf(" Value = [%s]\n", strBuf );
            if ( 0 != strcmp( strBuf, "Q2FycGUgRGllbQ==" ) ) {
                printf("vscphlp_getRemoteVariableString: Error - Written value does not match read value!\n");
                closeAll( handle1, handle2 );
                return -1;
            }
            else {
                printf("vscphlp_getRemoteVariableString: Written value match read value!\n");
            }
        }
        else {
            printf("vscphlp_getRemoteVariableString: Error on channel 1  Error code=%d\n", rv);
            closeAll( handle1, handle2 );
            return -1;
        }
    }
        
    
    // Delete a variable
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_deleteRemoteVariable( handle1, 
                                             "test_string_variable" ) ) )  {
        printf( "vscphlp_deleteRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_deleteRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    

    // Create a boolean variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_bool_variable", 
                        "bool",
                        0,
                        "",
                        0x744,
                        "true",
                        "VGhpcyBpcyBhIG5vdGU=")  ) ) {
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }



    // Write a value (false) to a boolean variable
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableBool( handle1, "test_bool_variable", 0 )  ) ) {
        printf( "vscphlp_setRemoteVariableBool: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableBool: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }



    // Read a value from a boolean variable
    int valBool;
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableBool( handle1, "test_bool_variable", &valBool ) ) ) {
        printf( "vscphlp_getRemoteVariableBool: Success on channel 1\n" );
        printf(" Value = %s\n", valBool ? "true" : "false" );
    }
    else {
        printf("vscphlp_getRemoteVariableBool: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }


    // Create an int variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_integer_variable", 
                        "integer",
                        0,
                        "",
                        0x744,
                        "1234",
                        "VGhpcyBpcyBhIG5vdGU=")  ) ) {
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    // Write a value to an int variable
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableInt( handle1, "test_integer_variable", 18666 )  ) ) {
        printf( "vscphlp_setRemoteVariableInt: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableInt: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }


    // Read a value from a int variable
    int intValue;
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableInt( handle1, "test_integer_variable", &intValue ) ) ) {
        printf( "vscphlp_getRemoteVariableInt: Success on channel 1\n" );
        printf(" Value = %d\n", intValue );
        if ( intValue == 18666 ) {
            printf("vscphlp_getRemoteVariableInt: Success - Read value is correct!\n");
        }
        else {
            printf("vscphlp_getRemoteVariableInt: Error - Read value is NOT correct!\n");
            closeAll( handle1, handle2 );
            return -1;
        }
    }
    else {
        printf("vscphlp_getRemoteVariableInt: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }



    // Create a  long variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_long_variable", 
                        "long",
                        0,
                        "",
                        0x744,
                        "1234567",
                        "VGhpcyBpcyBhIG5vdGU=")  ) ) {
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    // Write a value to an long variable
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableLong( handle1, "test_long_variable", 123456780 )  ) ) {
        printf( "vscphlp_setRemoteVariableLong: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableLong: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }


    // Read a value from a long variable
    long longValue;
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableLong( handle1, "test_long_variable", &longValue ) ) ) {
        printf( "vscphlp_getRemoteVariableLong: Success on channel 1\n" );
        printf(" Value = %ld\n", longValue );
        if ( longValue == 123456780 ) {
            printf("vscphlp_getRemoteVariableLong: Success - Read value is correct!\n");
        }
        else {
            printf("vscphlp_getRemoteVariableLong: Error - Read value is NOT correct!\n");
            closeAll( handle1, handle2 );
            return -1;
        }
    }
    else {
        printf("vscphlp_getRemoteVariableLong: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    

    // Create a float variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_float_variable", 
                        "float",
                        0,
                        "",
                        0x744,
                        "1.2345",
                        "VGhpcyBpcyBhIG5vdGU=")  ) ) {
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    // Write a value to an float variable
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableDouble( handle1, "test_float_variable", 1.2345001 )  ) ) {
        printf( "vscphlp_setRemoteVariableDouble: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableDouble: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }


    // Read a value from a float variable
    double floatValue;
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableDouble( handle1, "test_float_variable", &floatValue ) ) ) {
        printf( "vscphlp_getRemoteVariableDouble: Success on channel 1\n" );
        printf(" Value = %f\n", floatValue );
        if ( (round( 10000*floatValue )/10000 ) == 1.2345 ) {
            printf("vscphlp_getRemoteVariableDouble: Success - Read value is correct (checked four decimals)!\n");
        }
        else {
            printf("vscphlp_getRemoteVariableDouble: Error - Read value is NOT correct (checked four decimals)!\n");
            closeAll( handle1, handle2 );
            return -1;
        }
    }
    else {
        printf("vscphlp_getRemoteVariableDouble: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }

    

    // Create measurement variable
    if ( VSCP_ERROR_SUCCESS == 
        ( rv = vscphlp_createRemoteVariable( handle1, 
                        "test_measurement_variable", 
                        "measurement",
                        0,
                        "",
                        0x744,
                        "13.82,1,0,255,255",
                        "VGhpcyBpcyBhIG5vdGU=")  ) ) {
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    // Write a value to an measurement variable
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableMeasurement( handle1, 
                                                            "test_measurement_variable", 
                                                            1.234,
                                                            2,
                                                            7,
                                                            11,
                                                            22 ) ) ) {
        printf( "vscphlp_setRemoteVariableMeasurement: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableMeasurement: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }


    // Read a value from a measurement variable 
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
            printf( "vscphlp_getRemoteVariableMeasurement: Success on channel 1\n" );
            printf(" Value = %f, unit = %d, sensoridx = %d, zone = %d, subzone = %d\n", 
                    value, unit, sensoridx, zone, subzone  );
            if ( ((round( 10000*value )/10000 ) == 1.2340 ) &&
                    ( 2 == unit ) &&
                    ( 7 == sensoridx ) && 
                    ( 11 == zone ) &&
                    ( 22 == subzone ) ) {
                printf("vscphlp_getRemoteVariableMeasurement: Success - Read values is correct (value checked for four decimals)!\n");
            }
            else {
                printf("vscphlp_getRemoteVariableMeasurement: Error - Read values is NOT correct (value checked four decimals)!\n");
                closeAll( handle1, handle2 );
                return -1;
            }
        }
        else {
            printf("vscphlp_getRemoteVariableMeasurement: Error on channel 1  Error code=%d\n", rv);
            closeAll( handle1, handle2 );
            return -1;
        }
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
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    // Write a value to an event variable
    pEvent = malloc( sizeof( vscpEvent ) );
    if ( NULL == pEvent ) {
        printf("Failed to allocate storage for event.\n");
        closeAll( handle1, handle2 );
        return -1;
    }

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
        printf( "vscphlp_setRemoteVariableEvent: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableEvent: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }


    // Read a value from a event variable 
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableEvent( handle1, "test_event_variable", pEvent  ) ) ) {
        printf( "vscphlp_getRemoteVariableEvent: Success on channel 1\n" );
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
    else {
        printf("vscphlp_getRemoteVariableEvent: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }


    // Free the event
    vscphlp_deleteVSCPevent( pEvent );
    pEvent = NULL;

    
    
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
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    // Write a value to an event variable
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
        printf( "vscphlp_setRemoteVariableEventEx: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableEventEx: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }


    // Read a value from a event variable 
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableEventEx( handle1, "test_eventex_variable", &ex1  ) ) ) {
        printf( "vscphlp_getRemoteVariableEventEx: Success on channel 1\n" );
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
    else {
        printf("vscphlp_getRemoteVariableEventEx: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
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
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }

    // Write a value to an GUID variable - string type
    char strGUID[64];
    strcpy( strGUID, "FF:FF:FF:FF:FF:FF:FF:00:00:00:00:7F:00:01:01:FD" );

    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableGUIDString( handle1, "test_guidstr_variable", strGUID ) ) ) {
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }

    memset( strGUID, 0, sizeof(strGUID) );
   
    // Read a value from a GUID variable - string type
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableGUIDString( handle1, "test_guidstr_variable", strGUID, sizeof(strGUID)-1 )  ) )  {
        printf( "vscphlp_getRemoteVariableGUIDString: Success on channel 1\n" );
        printf(" Value = %s\n", strGUID );
        if ( 0 == strcmp( strGUID, "FF:FF:FF:FF:FF:FF:FF:00:00:00:00:7F:00:01:01:FD") ) {
            printf(" GUID strings are the same.\n");
        }
        else {
            printf("vscphlp_getRemoteVariableGUIDString: Error - Read GUID string are NOT the same.\n");
            closeAll( handle1, handle2 );
            return -1;
        }
    }
    else {
        printf("vscphlp_getRemoteVariableGUIDString: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
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
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    // Write a value to an GUID variable - array type
    unsigned char GUID[16];
    memset( GUID, 0, 16 );
    for ( int i=0;i<16; i++ ) {
        GUID[i] = i;
    }

    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableGUIDArray( handle1, "test_guidarray_variable", GUID ) ) ) {
        printf( "vscphlp_setRemoteVariableGUIDArray: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableGUIDArray: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }

    memset( GUID, 0, 16 );
   
    // Read a value from a GUID variable - array type
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableGUIDArray( handle1, "test_guidarray_variable", GUID  ) ) )  {
        printf( "vscphlp_getRemoteVariableGUIDArray: Success on channel 1\n" );
        printf(" Value = " );
        for ( int i=0; i<16; i++ ) {
            printf("%d ", GUID[i] );    
        }
        printf("\n");
        int err = 0;
        for ( int i=0; i<16; i++ ) {
            if ( i != GUID[i] ) {
                printf("Error: GUID's are NOT the same\n " );
                err = 1;
                break;
            } 
        }
        if ( !err ) printf(" GUID's are the same\n " );
    }
    else {
        printf("vscphlp_getRemoteVariableGUIDArray: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
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
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    unsigned char dataArray[10];
    memset( dataArray, 0, sizeof(dataArray) );
    for ( int i=0; i<sizeof(dataArray); i++ ) {
        dataArray[ i ] = i;    
    }

    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableVSCPData( handle1, "test_dataarray_variable", dataArray, sizeof( dataArray ) ) ) ) {
        printf( "vscphlp_setRemoteVariableVSCPData: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableVSCPData: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }

    unsigned short size;
    memset( dataArray, 0, sizeof( dataArray ) );
   
    // Read a value from a data variable 
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableVSCPData( handle1, "test_dataarray_variable", dataArray, &size  ) ) )  {
        printf( "vscphlp_getRemoteVariableVSCPData: Success on channel 1\n" );
        printf(" Value = " );
        for ( int i=0; i<size; i++ ) {
            printf("%d ", dataArray[i] );    
        }
        printf("\n");
    }
    else {
        printf("vscphlp_getRemoteVariableVSCPData: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
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
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    // Write a value for VSCP class type
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableVSCPClass( handle1, "test_vscp_class_variable", 10 ) ) ) {
        printf( "vscphlp_setRemoteVariableVSCPClass: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableVSCPClass: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
   
    unsigned short vscpclass;

    // Read a value from aVSCP class type
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableVSCPClass( handle1, "test_vscp_class_variable", &vscpclass ) ) )  {
        printf( "vscphlp_getRemoteVariableVSCPClass: Success on channel 1\n" );
        printf(" Value = %d\n", vscpclass );
        if ( 10 == vscpclass ) {
            printf("Read value is the same as written\n");
        }
        else {
            printf("\aError: Read value is NOT the same as written\n");
        }
    }
    else {
        printf("vscphlp_getRemoteVariableVSCPClass: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
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
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    // Write a value for VSCP type type
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableVSCPType( handle1, "test_vscp_type_variable", 22 ) ) ) {
        printf( "vscphlp_setRemoteVariableVSCPType: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableVSCPType: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
   
    unsigned short vscptype;

    // Read a value from aVSCP type type
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableVSCPType( handle1, "test_vscp_type_variable", &vscptype ) ) )  {
        printf( "vscphlp_getRemoteVariableVSCPType: Success on channel 1\n" );
        printf(" Value = %d\n", vscptype );
        if ( 22 == vscptype ) {
            printf("vscphlp_getRemoteVariableVSCPType: Success - Read value is the same as written\n");
        }
        else {
            printf("vscphlp_getRemoteVariableVSCPType: Error - Read value is NOT the same as written\n");
            closeAll( handle1, handle2 );
            return -1;
        }
    }
    else {
        printf("vscphlp_getRemoteVariableVSCPType: Success on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
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
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    // Write a value 
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableVSCPTimestamp( handle1, "test_vscp_timestamp_variable", 223344 ) ) ) {
        printf( "vscphlp_setRemoteVariableVSCPTimestamp: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableVSCPTimestamp: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
   
    unsigned long vscptimestamp;

    // Read a value from aVSCP type type
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableVSCPTimestamp( handle1, 
                                                            "test_vscp_timestamp_variable", 
                                                            &vscptimestamp ) ) )  {
        printf( "vscphlp_getRemoteVariableVSCPTimestamp: Success on channel 1\n" );
        printf(" Value = %lu\n", vscptimestamp );
        if ( 223344 == vscptimestamp ) {
            printf("vscphlp_getRemoteVariableVSCPTimestamp: Success - Read value is the same as written\n");
        }
        else {
            printf("vscphlp_getRemoteVariableVSCPTimestamp: Error - Read value is NOT the same as written\n");
            closeAll( handle1, handle2 );
            return -1;
        }
    }
    else {
        printf("vscphlp_getRemoteVariableVSCPTimestamp: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
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
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    // Write a value 
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableDateTime( handle1, "test_datetime_variable", "1956-11-02T17:12:01" ) ) ) {
        printf( "vscphlp_setRemoteVariableDateTime: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableDateTime: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
   
    char datetimebuf[80];
    memset( datetimebuf, 0, sizeof( datetimebuf) );

    // Read a value from a VSCP type type
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableDateTime( handle1, 
                                                            "test_datetime_variable", 
                                                            datetimebuf,
                                                            sizeof( datetimebuf ) ) ) )  {
        printf( "vscphlp_getRemoteVariableDateTime: Success on channel 1\n" );
        printf(" Value = %s\n", datetimebuf );
        if ( 0 == strcmp( datetimebuf, "1956-11-02T17:12:01" ) ) {
            printf("vscphlp_getRemoteVariableDateTime: Success - Read value is the same as written\n");
        }
        else {
            printf("vscphlp_getRemoteVariableDateTime: Error - Read value is NOT the same as written\n");
            closeAll( handle1, handle2 );
            return -1;
        }
    }
    else {
        printf("vscphlp_getRemoteVariableDateTime: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
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
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    // Write a value 
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableDate( handle1, "test_date_variable", "1956-11-02" ) ) ) {
        printf( "vscphlp_setRemoteVariableDate: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableDate: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
   
    char datebuf[80];
    memset( datebuf, 0, sizeof( datebuf) );

    // Read a value from a VSCP type type
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableDate( handle1, 
                                                            "test_date_variable", 
                                                            datebuf,
                                                            sizeof( datebuf ) ) ) )  {
        printf( "vscphlp_getRemoteVariableDate: Success on channel 1\n" );
        printf(" Value = %s\n", datebuf );
        if ( 0 == strcmp( datebuf, "1956-11-02" ) ) {
            printf("vscphlp_getRemoteVariableDate: Success - Read value is the same as written\n");
        }
        else {
            printf("vscphlp_getRemoteVariableDate: Error - Read value is NOT the same as written\n");
            closeAll( handle1, handle2 );
            return -1;
        }
    }
    else {
        printf("vscphlp_getRemoteVariableDate: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
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
        printf( "vscphlp_createRemoteVariable: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_createRemoteVariable: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
    
    // Write a value 
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_setRemoteVariableTime( handle1, "test_time_variable", "19:22:11" ) ) ) {
        printf( "vscphlp_setRemoteVariableTime: Success on channel 1\n" );
    }
    else {
        printf("vscphlp_setRemoteVariableTime: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }
   
    char timebuf[80];
    memset( timebuf, 0, sizeof( timebuf) );

    // Read a value from a VSCP type type
    if ( VSCP_ERROR_SUCCESS == 
            (rv = vscphlp_getRemoteVariableTime( handle1, 
                                                    "test_time_variable", 
                                                    timebuf,
                                                    sizeof( timebuf ) ) ) )  {
        printf( "vscphlp_getRemoteVariableTime: Success on channel 1\n" );
        printf(" Value = %s\n", timebuf );
        if ( 0 == strcmp( timebuf, "19:22:11" ) ) {
            printf("vscphlp_getRemoteVariableTime: Success - Read value is the same as written\n");
        }
        else {
            printf("vscphlp_getRemoteVariableTime: Error - Read value is NOT the same as written\n");
            closeAll( handle1, handle2 );
            return -1;
        }
    }
    else {
        printf("vscphlp_getRemoteVariableTime: Error on channel 1  Error code=%d\n", rv);
        closeAll( handle1, handle2 );
        return -1;
    }


    
#endif








// -------------------------------------------------------------------------------------------------




//return 0;



#ifdef TEST_HELPERS

    printf("\n\n");
    printf("\n\nTesting helpers\n");
    printf("===============\n");


    pEvent = malloc( sizeof( vscpEvent ) );
    if ( NULL == pEvent ) {
        printf("Allocation of event failed!\n");
        closeAll( handle1, handle2 );
        return -1;
    }
    pEvent->head = 0;
    pEvent->vscp_class = 10;
    pEvent->vscp_type = 6;
    pEvent->obid = 0;
    pEvent->timestamp = 0;
    memset( pEvent->GUID, 0, 16 );
    pEvent->sizeData = 3;
    pEvent->pdata = malloc( sizeof( unsigned char[3] ) );
    if ( NULL == pEvent->pdata ) {
        printf("Allocation of event data failed!\n");
        free( pEvent );
        closeAll( handle1, handle2 );
        return -1;
    }
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
        printf("vscphlp_readStringValue: Success correct = %ld\n", readValue );
    }
    else {
        printf("vscphlp_readStringValue: Error - readStringValue = %ld\n", readValue );
        closeAll( handle1, handle2 );
        return -1;
    }


    readValue = vscphlp_readStringValue("-00000000099");
    if ( -99 == readValue ) {
        printf("vscphlp_readStringValue: Success  correct = %ld\n", readValue );
    }
    else {
        printf("vscphlp_readStringValue: Error - readStringValue = %ld\n", readValue );
        closeAll( handle1, handle2 );
        return -1;
    }
    
    


    unsigned char eventPriority;
    if ( 0 == ( eventPriority = vscphlp_getVscpPriority( pEvent ) ) ) {
        printf("vscphlp_getVscpPriority: Success - Event priority = %d\n", eventPriority );
    }
    else {
        printf("vscphlp_getVscpPriority: Error - Event priority = %d\n", eventPriority );
        closeAll( handle1, handle2 );
        return -1;
    }


    if ( 0 == ( eventPriority = vscphlp_getVscpPriorityEx( &ex3 ) ) ) {
        printf("vscphlp_getVscpPriorityEx: Success - EventEx priority = %d\n", eventPriority );
    }
    else {
        printf("vscphlp_getVscpPriorityEx: Error - Get EventEx priority = %d\n", eventPriority );
        closeAll( handle1, handle2 );
        return -1;
    }


    vscphlp_setVscpPriority( pEvent, 3 );
    if ( 3 == ( eventPriority = vscphlp_getVscpPriority( pEvent ) ) ) {
        printf("vscphlp_setVscpPriority: Success - Event priority = %d\n", eventPriority );
    }
    else {
        printf("vscphlp_setVscpPriority - Error - vscphlp_setVscpPriority = %d\n", eventPriority );
        closeAll( handle1, handle2 );
        return -1;
    }


    vscphlp_setVscpPriorityEx( &ex3, 7 );
    if ( 7 == ( eventPriority = vscphlp_getVscpPriorityEx( &ex3 ) ) ) {
        printf("vscphlp_setVscpPriorityEx: Success - Event priority = %d\n", eventPriority );
    }
    else {
        printf("vscphlp_setVscpPriorityEx: Error - vscphlp_setVscpPriorityEx = %d\n", eventPriority );
        closeAll( handle1, handle2 );
        return -1;
    }

    unsigned char vscphead;
    unsigned long canalid = 0x0c0a0601;
    vscphead = vscphlp_getVSCPheadFromCANALid( canalid );
    if ( 96 == vscphead ) {
        printf("vscphlp_getVSCPheadFromCANALid: Success - VSCP head = %d\n", vscphead );
    }
    else {
        printf("vscphlp_getVSCPheadFromCANALid: Error - vscphlp_getVSCPheadFromCANALid = %d\n", vscphead );
        closeAll( handle1, handle2 );
        return -1;
    }


    unsigned short canal_vscpclass = vscphlp_getVSCPclassFromCANALid( canalid );
    if ( 10 == canal_vscpclass ) {
        printf("vscphlp_getVSCPclassFromCANALid: Success - VSCP Class = %d\n", canal_vscpclass );
    }
    else {
        printf("vscphlp_getVSCPclassFromCANALid: Error - vscphlp_getVSCPclassFromCANALid = %d\n", canal_vscpclass );
        closeAll( handle1, handle2 );
        return -1;
    }


    unsigned short canal_vscptype = vscphlp_getVSCPtypeFromCANALid( canalid );
    if ( 6 == canal_vscptype ) {
        printf("vscphlp_getVSCPtypeFromCANALid: Success - VSCP Type = %d\n", canal_vscptype );
    }
    else {
        printf("vscphlp_getVSCPtypeFromCANALid: Error - vscphlp_getVSCPtypeFromCANALid = %d\n", canal_vscptype );
        closeAll( handle1, handle2 );
        return -1;
    }

    unsigned char canal_nickname = vscphlp_getVSCPnicknameFromCANALid( canalid );
    if ( 1 == canal_nickname ) {
        printf("vscphlp_getVSCPnicknameFromCANALid: Success - Nickname = %d\n", canal_nickname );
    }
    else {
        printf("vscphlp_getVSCPnicknameFromCANALid: - Error - vscphlp_getVSCPnicknameFromCANALid = %d\n", canal_nickname );
        closeAll( handle1, handle2 );
        return -1;
    }

    unsigned long constr_canal_id2 = vscphlp_getCANALidFromVSCPdata( 3, 10, 6 ); 
    if ( 0x0c0a0600 == constr_canal_id2 ) {
        printf("vscphlp_getCANALidFromVSCPdata: Success - Nickname = %08lX\n", constr_canal_id2 );
    }
    else {
        printf("vscphlp_getCANALidFromVSCPdata: Error - vscphlp_getVSCPnicknameFromCANALid = %08lX\n", constr_canal_id2 );
        closeAll( handle1, handle2 );
        return -1;
    }

    constr_canal_id2 = vscphlp_getCANALidFromVSCPevent( pEvent ); 
    if ( 0x0c0a0600 == constr_canal_id2 ) {
        printf("vscphlp_getCANALidFromVSCPevent: Success - Nickname = %08lX\n", constr_canal_id2 );
    }
    else {
        printf("vscphlp_getCANALidFromVSCPevent: Error - vscphlp_getCANALidFromVSCPevent = %08lX\n", constr_canal_id2 );
        closeAll( handle1, handle2 );
        return -1;
    }  

    constr_canal_id2 = vscphlp_getCANALidFromVSCPeventEx( &ex3 ); 
    if ( 0x1c0a0600 == constr_canal_id2 ) {
        printf("vscphlp_getCANALidFromVSCPeventEx: Success - Nickname = %08lX\n", constr_canal_id2 );
    }
    else {
        printf("vscphlp_getCANALidFromVSCPeventEx: Error - vscphlp_getCANALidFromVSCPeventEx = %08lX\n", constr_canal_id2 );
        closeAll( handle1, handle2 );
        return -1;
    } 


    // Calculate CRC for event
    unsigned short crc = vscphlp_calc_crc_Event( pEvent, 0 );
    printf("vscphlp_calc_crc_Event: CRC = %04X\n", crc );

    // Calculate CRC for event
    crc = vscphlp_calc_crc_EventEx( pEvent, 0 );
    printf("vscphlp_calc_crc_EventEx: CRC = %04X\n", crc );

    // Calculate CRC for GID array
    unsigned char GUID2[16];
    memset( GUID2, 0, 16 );
    for ( int i=0;i<16; i++ ) {
        GUID2[i] = i;
    }
    unsigned char crc8 = vscphlp_calcCRC4GUIDArray( GUID2 );
    printf("vscphlp_calcCRC4GUIDArray: CRC = %02X\n", crc8 );

    // Calculate GUID for GUID string
    char strguid[64], strguid2[64];
    strcpy( strguid, "FF:FF:FF:FF:FF:FF:FF:00:00:00:00:7F:00:01:01:FD" );
    crc8 = vscphlp_calcCRC4GUIDString( strguid);
    printf("vscphlp_calcCRC4GUIDString: CRC = %02X\n", crc8 );


    if ( VSCP_ERROR_SUCCESS == vscphlp_getGuidFromString( pEvent, strguid ) ) {
        vscphlp_writeGuidToString( pEvent, strguid2, sizeof( strguid2 )-1 );
        printf( "vscphlp_writeGuidToString: Success - GUID=%s\n", strguid2 );
    }
    else {
        printf( "vscphlp_writeGuidToString: Error - vscphlp_writeGuidArrayToString\n");
        closeAll( handle1, handle2 );
        return -1;
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_getGuidFromStringEx( &ex3, strguid ) ) {
        vscphlp_writeGuidToStringEx( &ex3, strguid2, sizeof( strguid2 )-1 );
        printf( "vscphlp_getGuidFromStringEx: Success -  GUID=%s\n", strguid2 );
    }
    else {
        printf( "vscphlp_getGuidFromStringEx: Error - vscphlp_writeGuidArrayToString\n");
        closeAll( handle1, handle2 );
        return -1;
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_writeGuidToString4Rows( pEvent, strguid2, sizeof( strguid2 )-1 ) ) {
        printf( "vscphlp_writeGuidToString4Rows: Success - GUID\n%s\n", strguid2 );
    }
    else {
        printf( "vscphlp_writeGuidToString4Rows: Error - vscphlp_writeGuidArrayToString\n");
        closeAll( handle1, handle2 );
        return -1;
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_writeGuidToString4RowsEx( &ex3, strguid2, sizeof( strguid2 )-1 ) ) {
        printf( "vscphlp_writeGuidToString4Rows: Success - GUID\n%s\n", strguid2 );
    }
    else {
        printf( "vscphlp_writeGuidToString4Rows: Error - vscphlp_writeGuidArrayToString\n");
        closeAll( handle1, handle2 );
        return -1;
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_writeGuidArrayToString( GUID2, strguid2, sizeof( strguid2 )-1 ) ) {
        printf( "vscphlp_writeGuidToString4Rows: Successs - GUID=%s\n", strguid2 );
    }
    else {
        printf( "vscphlp_writeGuidToString4Rows: Error - vscphlp_writeGuidArrayToString\n");
        closeAll( handle1, handle2 );
        return -1;
    }

    unsigned char emptyGUID[16];
    memset( emptyGUID,0, 16 );
    if ( vscphlp_isGUIDEmpty( emptyGUID ) ) {
        printf( "vscphlp_isGUIDEmpty: Success - GUID is detected as empty as it should be\n" );    
    }
    else {
        printf( "vscphlp_isGUIDEmpty: Error -  vscphlp_isGUIDEmpty\n");
        closeAll( handle1, handle2 );
        return -1;
    }

    // GUID that is not empty
    if ( vscphlp_isGUIDEmpty( GUID2 ) ) {
        printf( "vscphlp_isGUIDEmpty: Error - Detected as empty but should NOT be.\n");  
        closeAll( handle1, handle2 );
        return -1;
    }
    else {
        printf( "vscphlp_isGUIDEmpty: Success  - GUID is detected as NOT empty as it should be.\n" );     
    }

    // GUID's are not the same
    if ( vscphlp_isSameGUID( emptyGUID, GUID2) ) {
        printf( "vscphlp_isSameGUID: Error - vscphlp_isSameGUID\n");
        closeAll( handle1, handle2 );
        return -1;           
    }
    else {
        printf( "vscphlp_isSameGUID: Success  - Correct, GUIDs are not the same.\n" ); 
    }

    vscphlp_writeGuidArrayToString( GUID2, strguid2, sizeof( strguid2 )-1 );
    printf( "vscphlp_writeGuidArrayToString: GUID before reverse = %s\n", strguid2 );
    if ( VSCP_ERROR_SUCCESS == vscphlp_reverseGUID( GUID2 ) ) {
        vscphlp_writeGuidArrayToString( GUID2, strguid2, sizeof( strguid2 )-1 );
        printf( "vscphlp_writeGuidArrayToString: GUID after reverse = %s\n", strguid2 );
    }
    else {
        printf( "vscphlp_writeGuidArrayToString: Error - vscphlp_reverseGUID\n");
        closeAll( handle1, handle2 );
        return -1;
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_getGuidFromStringToArray( GUID2, strguid ) ) {
        vscphlp_writeGuidArrayToString( GUID2, strguid2, sizeof( strguid2 )-1 );
        printf( "vscphlp_writeGuidArrayToString: GUID  after reverse = %s\n", strguid2 );
    }
    else {
        printf( "vscphlp_writeGuidArrayToString: Error - vscphlp_getGuidFromStringToArray\n");
        closeAll( handle1, handle2 );
        return -1;
    }

    vscpEventEx ex4;
    if ( VSCP_ERROR_SUCCESS != vscphlp_convertVSCPtoEx( &ex4, pEvent ) ) {
        printf( "vscphlp_convertVSCPtoEx: Error - vscphlp_getGuidFromStringToArray\n");
        closeAll( handle1, handle2 );
        return -1;
    }

    vscpEvent *pEvent2 = malloc( sizeof( vscpEvent ) );
    pEvent2->pdata = NULL;
    if ( VSCP_ERROR_SUCCESS != vscphlp_convertVSCPfromEx( pEvent2, &ex4 ) ) {
        printf( "vscphlp_convertVSCPfromEx: Error - vscphlp_convertVSCPfromEx\n");
        closeAll( handle1, handle2 );
        return -1;
    }
    vscphlp_deleteVSCPevent( pEvent2 );
    pEvent2 = NULL;

    //vscpEventFilter filter;
    vscphlp_clearVSCPFilter( &filter );

    if ( VSCP_ERROR_SUCCESS != vscphlp_readFilterFromString( &filter, 
                "1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00" ) ) {
        printf( "vscphlp_readFilterFromString: Error - vscphlp_readFilterFromString\n");   
        closeAll( handle1, handle2 );
        return -1;
    }
    else {
        printf( "vscphlp_readFilterFromString: Success.\n");    
    }
    
    if ( VSCP_ERROR_SUCCESS != vscphlp_readMaskFromString( &filter, 
                "1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00" ) ) {
        printf( "vscphlp_readMaskFromString: Error - vscphlp_readMaskFromString\n");   
        closeAll( handle1, handle2 );
        return -1;
    }
    else {
        printf( "vscphlp_readMaskFromString: Success.\n");    
    }

    
    if ( vscphlp_doLevel2Filter( pEvent, &filter ) ) {
        printf( "vscphlp_doLevel2Filter: Event pass: \n");
    }
    else {
        printf( "vscphlp_doLevel2Filter: Event does NOT pass: \n");
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
        printf( "vscphlp_convertCanalToEvent: Success - VSCP class=%d Type=%d\n", pEvent3->vscp_class, pEvent3->vscp_type );
    }
    else {
        printf( "vscphlp_convertCanalToEvent: Error\n");
        closeAll( handle1, handle2 );
        return -1;
    }

    // Free the event
    vscphlp_deleteVSCPevent( pEvent3 );
    pEvent3 = NULL;

    vscpEventEx ex5;
    if ( VSCP_ERROR_SUCCESS == vscphlp_convertCanalToEventEx( &ex5,
                                                                &canalMsg,
                                                                GUID2 ) ) {
        printf( "vscphlp_convertCanalToEventEx: Success - VSCP class=%d Type=%d\n", ex5.vscp_class, ex5.vscp_type );
    }
    else {
        printf( "vscphlp_convertCanalToEvent: Error\n");
        closeAll( handle1, handle2 );
        return -1;
    }

    
    if ( VSCP_ERROR_SUCCESS == vscphlp_convertEventToCanal( &canalMsg, pEvent ) ) {
        printf( "vscphlp_convertEventToCanal: Success - id=%08lX\n", canalMsg.id );
    }
    else {
        printf( "vscphlp_convertEventToCanal: Error: \n");
        closeAll( handle1, handle2 );
        return -1;        
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_convertEventExToCanal( &canalMsg, &ex5 ) ) {
        printf( "vscphlp_convertEventExToCanal: Success - id=%08lX\n", canalMsg.id );
    }
    else {
        printf( "vscphlp_convertEventExToCanal: Error: \n");
        closeAll( handle1, handle2 );
        return -1;
    }


    printf( "vscphlp_makeTimeStamp  %04lX\n", vscphlp_makeTimeStamp() );

    
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
        printf( "vscphlp_copyVSCPEvent: Success - %02X %02X \n", pEventTo->pdata[0], pEventTo->pdata[1] );
    }
    else {
        printf( "vscphlp_copyVSCPEvent: Error\n");
        closeAll( handle1, handle2 );
        return -1;
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
       printf( "vscphlp_writeVscpDataToString: Success - \n%s \n", dataBuf );
    }
    else {
        printf( "vscphlp_writeVscpDataToString: Error: \n");
        closeAll( handle1, handle2 );
        return -1;
    }


    unsigned char dataArray2[32];
    unsigned short sizeData;
    if ( VSCP_ERROR_SUCCESS == 
             vscphlp_setVscpDataArrayFromString( dataArray2, 
                                       &sizeData,
                                       "1,2,3,4,5,6,0x07,0x55,3,4,0xaa,0xff,0xff" ) ) {
        printf( "vscphlp_setVscpDataArrayFromString: Success - size=%d Data = \n", sizeData );
        for ( int i=0; i<sizeData; i++ ) {
            printf("%d ", dataArray2[i] );
        }
        printf("\n");
    }
    else {
        printf( "vscphlp_setVscpDataArrayFromString: Error: \n");
        closeAll( handle1, handle2 );
        return -1;
    }
    
    
    if ( VSCP_ERROR_SUCCESS == 
             vscphlp_setVscpDataFromString( &e, "1,2,3,4,5,6,0x07,0x55,3,4,0xaa,0xff,0xff" ) ) {
        printf( "vscphlp_setVscpDataFromString: Success -  size=%d Data = \n", e.sizeData );
        for ( int i=0; i<e.sizeData; i++ ) {
            printf("%d ", e.pdata[i] );
        }
        printf("\n");
    }
    else {
        printf( "vscphlp_setVscpDataFromString: Error: \n");
        closeAll( handle1, handle2 );
        return -1;
    }


    char eventBuf[128];
    if ( VSCP_ERROR_SUCCESS == vscphlp_writeVscpEventToString( pEvent, eventBuf, sizeof( eventBuf )-1 ) ) {
        printf( "vscphlp_writeVscpEventToString: Success - Event = %s\n", eventBuf );    
    }
    else {
        printf( "vscphlp_writeVscpEventToString: Error: \n");
        closeAll( handle1, handle2 );
        return -1;
    }


    if ( VSCP_ERROR_SUCCESS == vscphlp_writeVscpEventExToString( &ex3, eventBuf, sizeof( eventBuf )-1 ) ) {
        printf( "vscphlp_writeVscpEventToString: Success -  Event = %s\n", eventBuf );    
    }
    else {
        printf( "vscphlp_writeVscpEventToString: Error: \n");
        closeAll( handle1, handle2 );
        return -1;
    }


    vscpEvent *pEventString1 = malloc( sizeof( vscpEvent ) );
    pEventString1->pdata = NULL;

    if ( VSCP_ERROR_SUCCESS == vscphlp_setVscpEventFromString( pEventString1, 
                             "0,10,6,,,,FF:FF:FF:FF:FF:FF:FF:00:00:00:00:7F:00:01:01:FD,0x8A,0x00,0x1E" ) ) {
        printf( "vscphlp_setVscpEventFromString: Success class=%d Type=%d\n", 
                   pEventString1->vscp_class, pEventString1->vscp_type );
    }
    else {
        printf( "vscphlp_setVscpEventFromString: Error: \n");
        closeAll( handle1, handle2 );
        return -1;
    }

    // Free the events
    vscphlp_deleteVSCPevent( pEventString1 );
    pEventString1 = NULL;


    vscpEventEx ex6;
    if ( VSCP_ERROR_SUCCESS == vscphlp_setVscpEventExFromString( &ex6, 
                             "0,10,6,,,,FF:FF:FF:FF:FF:FF:FF:00:00:00:00:7F:00:01:01:FD,0x8A,0x00,0x1E" ) ) {
        printf( "vscphlp_setVscpEventExFromString: Success - class=%d Type=%d\n", 
                   ex6.vscp_class, ex6.vscp_type );
    }
    else {
        printf( "vscphlp_setVscpEventExFromString: Error: \n");
        closeAll( handle1, handle2 );
        return -1;
    }
    
    vscphlp_deleteVSCPevent( pEvent );
    pEvent = NULL;


#endif











// -------------------------------------------------------------------------------------------------











#ifdef TEST_MEASUREMENT


    pEvent = malloc( sizeof( vscpEvent ) );
    if ( NULL == pEvent ) {
        printf("Allocation of event failed!\n");
        closeAll( handle1, handle2 );
        return -1;
    }
    pEvent->head = 0;
    pEvent->vscp_class = 10;
    pEvent->vscp_type = 6;
    pEvent->obid = 0;
    pEvent->timestamp = 0;
    memset( pEvent->GUID, 0, 16 );
    pEvent->sizeData = 3;
    pEvent->pdata = malloc( sizeof( unsigned char[3] ) );
    if ( NULL == pEvent->pdata ) {
        printf("Allocation of event data failed!\n");
        free( pEvent );
        closeAll( handle1, handle2 );
        return -1;
    }
    pEvent->pdata[ 0 ] = 138;
    pEvent->pdata[ 1 ] = 0;
    pEvent->pdata[ 2 ] = 30;




    printf("\n\nMeasurement helpers\n");
    printf("===================\n");

    unsigned char dataCoding = vscphlp_getMeasurementDataCoding( pEvent );
    if ( 138 == dataCoding  ) {
        printf("vscphlp_getMeasurementDataCoding: Success - Data Coding = %d\n", dataCoding );
    }
    else {
        printf("vscphlp_getMeasurementDataCoding: Error - Data Coding = %d\n", dataCoding );
        closeAll( handle1, handle2 );
        return -1;
    }


    unsigned char bitarry[3];
    bitarry[0] = VSCP_DATACODING_BIT; // Data coding byte. Default unit, sensoridx=0
    bitarry[1] = 0x55;
    bitarry[2] = 0xAA;
    unsigned long long bitarray64 = vscphlp_getDataCodingBitArray( bitarry, sizeof( bitarry ) );
    if ( bitarray64 ==  0x55AA ) {
        printf("vscphlp_getDataCodingBitArray: Success.\n");
    }
    else {
        printf("vscphlp_getDataCodingBitArray: Error -  [%d]\n", dataCoding );
        closeAll( handle1, handle2 );
        return -1;
    }

    unsigned char normarry[4];
    normarry[0] = 0x89; // Data coding byte: Normalized integer, unit=1, sensoridx=1
    normarry[1] = 0x02;
    normarry[2] = 0x01;
    normarry[3] = 0x36;
    double value =  vscphlp_getDataCodingNormalizedInteger( normarry, sizeof( normarry ) );
    if ( 31000.0 == value ) {
        printf("vscphlp_getDataCodingNormalizedInteger: Success -  Value = %f \n", value );
    }
    else {
        printf("vscphlp_getDataCodingNormalizedInteger: Error - value = %f \n", value );
        closeAll( handle1, handle2 );
        return -1;
    }

    normarry[0] = 0x60; // Data coding byte: Integer, unit=0, sensoridx=0
    normarry[1] = 0xFF;
    normarry[2] = 0xFF;
    normarry[3] = 0xFF;
    unsigned long long val64 = vscphlp_getDataCodingInteger( normarry, sizeof( normarry ) );
    printf("vscphlp_getDataCodingInteger: Success -  value = %llu \n", val64 );

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
        printf("vscphlp_getDataCodingString: Success -  value = %s \n", stringbuf );
    }
    else {
        printf("vscphlp_getDataCodingString: Error -  value = %s \n", stringbuf );
        closeAll( handle1, handle2 );
        return -1;
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
        printf("vscphlp_getVSCPMeasurementAsString: Success -  value = %s \n", stringbuf );
    }
    else {
        printf("vscphlp_getVSCPMeasurementAsString: Error -  value = %s \n", stringbuf );
        closeAll( handle1, handle2 );
        return -1;
    }




    pEventMeasurement->pdata[0] = 0x89;
    pEventMeasurement->pdata[1] = 0x02;
    pEventMeasurement->pdata[2] = 0xFF;
    pEventMeasurement->pdata[3] = 0xF1;

    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsString( pEventMeasurement, 
                                                                    stringbuf, 
                                                                    sizeof( stringbuf ) ) ) {
        printf("vscphlp_getVSCPMeasurementAsString: Success -  value = %s \n", stringbuf );
    }
    else {
        printf("vscphlp_getVSCPMeasurementAsString: Error -  value = %s \n", stringbuf );
        closeAll( handle1, handle2 );
        return -1;
    }




    pEventMeasurement->pdata[0] = VSCP_DATACODING_BIT;
    pEventMeasurement->pdata[1] = 0x55;
    pEventMeasurement->pdata[2] = 0xAA;
    pEventMeasurement->pdata[3] = 0x55;

    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsString( pEventMeasurement, 
                                                                    stringbuf, 
                                                                    sizeof( stringbuf ) ) ) {
        printf("vscphlp_getVSCPMeasurementAsString: Success -  value = %s \n", stringbuf );
    }
    else {
        printf("vscphlp_getVSCPMeasurementAsString: Error -  value = %s \n", stringbuf );
        closeAll( handle1, handle2 );
        return -1;
    }



    pEventMeasurement->pdata[0] = VSCP_DATACODING_BYTE;
    pEventMeasurement->pdata[1] = 0x55;
    pEventMeasurement->pdata[2] = 0xAA;
    pEventMeasurement->pdata[3] = 0x55;

    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsString( pEventMeasurement, 
                                                                    stringbuf, 
                                                                    sizeof( stringbuf ) ) ) {
        printf("vscphlp_getVSCPMeasurementAsString: Success -  value = %s \n", stringbuf );
    }
    else {
        printf("vscphlp_getVSCPMeasurementAsString: Error -  value = %s \n", stringbuf );
        closeAll( handle1, handle2 );
        return -1;
    }


    pEventMeasurement->pdata[0] = VSCP_DATACODING_STRING;
    pEventMeasurement->pdata[1] = 0x33;
    pEventMeasurement->pdata[2] = 0x31;
    pEventMeasurement->pdata[3] = 0x34;

    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsString( pEventMeasurement, 
                                                                    stringbuf, 
                                                                    sizeof( stringbuf ) ) ) {
        printf("vscphlp_getVSCPMeasurementAsString: Success -  value = %s \n", stringbuf );
    }
    else {
        printf("vscphlp_getVSCPMeasurementAsString: Error -  value = %s \n", stringbuf );
        closeAll( handle1, handle2 );
        return -1;
    }


    pEventMeasurement->pdata[0] = VSCP_DATACODING_INTEGER;
    pEventMeasurement->pdata[1] = 0x00;
    pEventMeasurement->pdata[2] = 0xFF;
    pEventMeasurement->pdata[3] = 0xFF;

    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsString( pEventMeasurement, 
                                                                    stringbuf, 
                                                                    sizeof( stringbuf ) ) ) {
        printf("vscphlp_getVSCPMeasurementAsString: Success -  value = %s \n", stringbuf );
    }
    else {
        printf("vscphlp_getVSCPMeasurementAsString: Error -  value = %s \n", stringbuf );
        closeAll( handle1, handle2 );
        return -1;
    }



    pEventMeasurement->pdata[0] = VSCP_DATACODING_INTEGER;
    pEventMeasurement->pdata[1] = 0xFF;
    pEventMeasurement->pdata[2] = 0xFF;
    pEventMeasurement->pdata[3] = 0xFF;

    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsString( pEventMeasurement, 
                                                                    stringbuf, 
                                                                    sizeof( stringbuf ) ) ) {
        printf("vscphlp_getVSCPMeasurementAsString: Success -  value = %s \n", stringbuf );
    }
    else {
        printf("vscphlp_getVSCPMeasurementAsString: Error -  value = %s \n", stringbuf );
        closeAll( handle1, handle2 );
        return -1;
    }



    pEventMeasurement->pdata[0] = VSCP_DATACODING_INTEGER;
    pEventMeasurement->pdata[1] = 0xFF;
    pEventMeasurement->pdata[2] = 0xFF;
    pEventMeasurement->pdata[3] = 0xFF;
    if ( VSCP_ERROR_SUCCESS == vscphlp_getVSCPMeasurementAsDouble( pEventMeasurement, &value ) ) {
        printf("vscphlp_getVSCPMeasurementAsDouble: Success -  value = %lf\n", value );
    }
    else {
        printf("vscphlp_getVSCPMeasurementAsDouble: Error -  value = %lf \n", value );
        closeAll( handle1, handle2 );
        return -1;
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
        printf("vscphlp_getVSCPMeasurementFloat64AsString: Success -  value = %s \n", stringbuf );
    }
    else {
        printf("vscphlp_getVSCPMeasurementFloat64AsString: Error -  value = %s \n", stringbuf );
        closeAll( handle1, handle2 );
        return -1;
    }

    // Free the event
    vscphlp_deleteVSCPevent( pEventfloat );
    pEventfloat = NULL;

    // Free the event
    vscphlp_deleteVSCPevent( pEventMeasurement );
    pEventMeasurement = NULL;    

#endif


    // -------------------------------------------------------------------------------------------------
    

    // Free the event
    if ( NULL == pEvent ) printf("pEvent is NULL.\n");
    printf("Delete pEvent working event.\n");
    vscphlp_deleteVSCPevent( pEvent );
    pEvent = NULL;

    printf("\n\n\n");




     t1 = t2 = current_timestamp();
	for ( int i=0; i<BURST_SEND_COUNT; i++ ) {

		exa.data[0] = (i>>8) & 0xff;
    	exa.data[1] = i & 0xff;

    	if ( VSCP_ERROR_SUCCESS == (rv = vscphlp_noop( handle1 ) ) ) {
        	printf( "vscphlp_noop: Success. %d\n", i );
    	}
    	else {
        	printf("vscphlp_noop: Failure - rv = %d\n", rv );
        	closeAll( handle1, handle2 );
            return -1;
    	}
	}

    t2 = current_timestamp();
    printf("Burst NOOP time %d ms\n", (int)(t2-t1) );






    // free data
    printf("Free 'e' working event data.\n");
    if ( NULL != pEvent) free( e.pdata );

    if ( VSCP_ERROR_SUCCESS == vscphlp_close( handle1 ) ) {
        printf( "Command success: vscphlp_close on channel 1\n" );
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_close( handle2 ) ) {
        printf( "Command success: vscphlp_close on channel 2\n" );
    }

    vscphlp_closeSession( handle1 );
    vscphlp_closeSession( handle2 );

    //printf("\n\nHit ENTER to terminate Error Count= %d\n", error_cnt );
    //(void)getchar();

   
   
    return 0;
}
