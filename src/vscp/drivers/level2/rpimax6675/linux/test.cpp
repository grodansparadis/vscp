// test.cpp : 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP Project (http://www.vscp.org) 
//
// Copyright (C) 2000-2019 Ake Hedman, 
// Grodans Paradis AB, <akhe@grodansparadis.com>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include <iostream>

#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>
 
#include "max6675.h"
#include "max6675obj.h"

#ifdef USE_PIGPIOD
#include <pigpiod_if2.h>
#else
    #include <pigpio.h>
#endif

#define TEST_HOST		"192.168.1.9"
#define TEST_PORT		9598
#define TEST_USER		"admin"
#define TEST_PASSWORD	"secret"
#define TEST_PREFIX		"max6675"
#define TEST_CONFIG		""

void sig_handler(int signo)
{
    if (signo == SIGSEGV) printf("received SIGSEGV\n");
    if (signo == SIGTERM) printf("received SIGTERM\n");
    exit( -1 );
}

///////////////////////////////////////////////////////////////////////////////
//  BlockingSend
// 

int
BlockingSend( CRpiMax6675 *pmax6675, const vscpEvent *pEvent, unsigned long timeout )
{
	int rv = 0;
	if ( NULL == pmax6675 ) return CANAL_ERROR_MEMORY;
    pmax6675->addEvent2SendQueue( pEvent );
	return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  BlockingReceive
// 

int
BlockingReceive( CRpiMax6675 *pmax6675, vscpEvent *pEvent, unsigned long timeout )
{
    int rv = 0;
 
    // Check pointers
    if ( NULL == pEvent) return CANAL_ERROR_PARAMETER;
	if (NULL == pmax6675) return CANAL_ERROR_MEMORY;
    
	if ( 0 == pmax6675->m_receiveList.size() ) return CANAL_ERROR_FIFO_EMPTY;

	/*
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = timeout * 1e6;
	if ( ETIMEDOUT == sem_timedwait( &pmax6675->m_semaphore_ReceiveQueue, &ts ) ) {
        return CANAL_ERROR_TIMEOUT;
    }
	*/
    
	pthread_mutex_lock( &pmax6675->m_mutex_ReceiveQueue );
    vscpEvent *pLocalEvent = pmax6675->m_receiveList.front();
    pmax6675->m_receiveList.pop_front();
	pthread_mutex_unlock( &pmax6675->m_mutex_ReceiveQueue );
    if ( NULL == pLocalEvent ) return CANAL_ERROR_MEMORY;
    
    vscp_copyVSCPEvent( pEvent, pLocalEvent );
    vscp_deleteVSCPevent( pLocalEvent );
	
	return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  getCountInqueue
// 

int getCountInqueue( CRpiMax6675 *pmax6675 ) 
{
	return pmax6675->m_receiveList.size();
}

///////////////////////////////////////////////////////////////////////////////
//  main
// 

int main( int argc, const char* argv[] )
{
	int i,j,k;		// General indexes
	int cnt;
	
    CRpiMax6675 max6675;
	int ntest = 1;	// Default do test 1
	bool bQuit = false;
	char c;



	if ( SIG_ERR == signal(SIGSEGV, sig_handler) ) printf("Could not catch SIGSEGV\n");
	if ( SIG_ERR == signal(SIGTERM, sig_handler) ) printf("Could not catch SIGSEGV\n");

	printf( "%s arg = %s\n", argv[0], argv[1] );

	if ( argc > 1 ) {
		ntest = atoi( argv[1] );
	}
	 
#ifdef USE_PGPIOD
	printf("Using if2\n\n");
#endif	

if ( 1 == ntest ) {

	int nSent = 0;

	printf("RUNNING TEST 1 - Ten temperature events\n");
	printf("---------------------------------------\n\n");

	printf("Open max6675\n");
    max6675.open( TEST_USER,
		        	TEST_PASSWORD,
		        	TEST_HOST,
		        	TEST_PORT,
		        	"max6675",
		        	TEST_CONFIG );

	for ( i=0; i<20; i++ ) {

		// CLASS1.CONTROL, Type=5, TURN-ON
		/*printf("Send CLASS1.CONTROL, Type=5,TURN-ON\n");
		vscpEvent *pEventOn = new vscpEvent;
		pEventOn->pdata = new uint8_t[3];
		pEventOn->pdata[0] = 0; // index
		pEventOn->pdata[1] = 0; // zone
		pEventOn->pdata[2] = 0; // subzone
		pEventOn->sizeData = 3;
		pEventOn->obid = 0;
		pEventOn->timestamp = 0;	
		pEventOn->vscp_class = VSCP_CLASS1_CONTROL;
		pEventOn->vscp_type = VSCP_TYPE_CONTROL_TURNON;

		if ( CANAL_ERROR_SUCCESS == BlockingSend( &max6675, pEventOn, 500 ) ) {
			nSent++;
		}
		usleep(100000);

		// CLASS1.CONTROL, Type=6, TURN_OFF
		printf("Send CLASS1.CONTROL, Type=6,TURN-OFF\n");
		vscpEvent *pEventOff = new vscpEvent;
		pEventOff->pdata = new uint8_t[3];
		pEventOff->pdata[0] = 0; // index
		pEventOff->pdata[1] = 0; // zone
		pEventOff->pdata[2] = 0; // subzone
		pEventOff->sizeData = 3;
		pEventOff->obid = 0;
		pEventOff->timestamp = 0;	
		pEventOff->vscp_class = VSCP_CLASS1_CONTROL;
		pEventOff->vscp_type = VSCP_TYPE_CONTROL_TURNOFF;

		if ( CANAL_ERROR_SUCCESS == BlockingSend( &max6675, pEventOn, 500 ) ) {
			nSent++;
		}
		usleep(100000);*/
	}

	int cnt = getCountInqueue( &max6675 );
	printf("Number of events sent = %d\n", nSent );
	printf("Number of events in in-queue = %d\n", cnt );

	
	for ( int i=0; i<cnt; i++ ) {
		vscpEvent *pEvent = new vscpEvent;
		if ( CANAL_ERROR_SUCCESS == BlockingReceive( &max6675, pEvent, 1000 ) ) {
			if ( NULL != pEvent ) {
				printf("%d - Fetching event with class=%d, type=%d, data size = %d\n", 
						i,
						pEvent->vscp_class,
						pEvent->vscp_type,
						pEvent->sizeData );
						
			}
		}

		if ( NULL != pEvent ) {			
			vscp_deleteVSCPevent_v2( &pEvent );
		}
	}

	printf("Close max6675\n");
	max6675.close();
}
/*
else if ( 2 == ntest ) {

	printf("RUNNING TEST 2 - Monitor test on pin 27 (ctrl+D to terminate)\n");
	printf("-------------------------------------------------------------\n\n");

	cnt = 0;

	printf("Open max6675\n");
    max6675.open( TEST_USER,
		        TEST_PASSWORD,
		        TEST_HOST,
		        TEST_PORT,
		        "rpi2",
		        TEST_CONFIG);
	
	printf("Waiting for eight rising flanks on BCM 27.\n");
	printf("w 22 1 w 22 0w 22 1 w 22 0 w 22 1 w 22 0 w 22 1 w 22 0\n");

	cnt = 0;
 	while ( cnt < 8 ) {

		vscpEvent *pEvent;
		
		if ( !vscp_newVSCPevent( &pEvent ) ) {
			printf("Failed to create event!\n");
			exit( -1 );
		}

		if ( CANAL_ERROR_SUCCESS == BlockingReceive( &max6675, pEvent, 1000 ) ) {
			if ( NULL != pEvent ) {
				cnt++;
				printf("%d - Got event with class=%d, type=%d, data size = %d\n", 
						cnt,
						pEvent->vscp_class,
						pEvent->vscp_type,
						pEvent->sizeData );		
			}
		}

		if ( NULL != pEvent ) {			
			vscp_deleteVSCPevent_v2( &pEvent );
		}

	}

	printf("Close max6675\n");
	max6675.close();
}
else if ( 3 == ntest ) {

	printf("RUNNING TEST 3 - Reports test on pin 27 (ctrl+C to terminate)\n");
	printf("-------------------------------------------------------------\n\n");

	cnt = 0;

	printf("Open max6675\n");
    max6675.open( TEST_USER,
		        TEST_PASSWORD,
		        TEST_HOST,
		        TEST_PORT,
		        "rpi3",
		        TEST_CONFIG);
	
	printf("Waiting for ever for report events with status for on BCM 27.\n");
	printf("w 22 1 w 22 0w 22 1 w 22 0 w 22 1 w 22 0 w 22 1 w 22 0\n");

	cnt = 0;
 	while ( true ) {

		vscpEvent *pEvent;
		
		if ( !vscp_newVSCPevent( &pEvent ) ) {
			printf("Failed to create event!\n");
			exit( -1 );
		}

		if ( CANAL_ERROR_SUCCESS == BlockingReceive( &max6675, pEvent, 1000 ) ) {
			if ( NULL != pEvent ) {
				cnt++;
				printf("%d - Got event with class=%d, type=%d, size=%d\n", 
						cnt,
						pEvent->vscp_class,
						pEvent->vscp_type,
						pEvent->sizeData );		
			}
		}

		if ( NULL != pEvent ) {			
			vscp_deleteVSCPevent_v2( &pEvent );
		}

	}

	printf("Close max6675\n");
	max6675.close();
}

else if ( 4 == ntest ) {

	printf("RUNNING TEST 4 - PWM test on pin 27 (ctrl+C to terminate)\n");
	printf("---------------------------------------------------------\n\n");

	cnt = 0;

	printf("Open max6675\n");
    max6675.open( TEST_USER,
		        TEST_PASSWORD,
		        TEST_HOST,
		        TEST_PORT,
		        "rpi4",
		        TEST_CONFIG);
	
	printf("0.3098 ms 800Hz, 25%% duty cycle on BCM 26.  Press any key to change duty cycle tp 50\n");
	char c = getchar();

	printf("0.3098 ms 800Hz, 50%% duty cycle on BCM 26.  Press any key to change duty cycle tp 75\n");
	c = getchar();

	printf("0.3098 ms 800Hz, 75%% duty cycle on BCM 26.  Press any key to change duty cycle tp 75\n");
	c = getchar();

	printf("BCM 26.  Press any key to change duty cycle tp 75%%\n");
	c = getchar();

	cnt = 0;
 	while ( true ) {

		vscpEvent *pEvent;
		
		if ( !vscp_newVSCPevent( &pEvent ) ) {
			printf("Failed to create event!\n");
			exit( -1 );
		}

		if ( CANAL_ERROR_SUCCESS == BlockingReceive( &max6675, pEvent, 1000 ) ) {
			if ( NULL != pEvent ) {
				cnt++;
				printf("%d - Got event with class=%d, type=%d, size=%d\n", 
						cnt,
						pEvent->vscp_class,
						pEvent->vscp_type,
						pEvent->sizeData );		
			}
		}

		if ( NULL != pEvent ) {			
			vscp_deleteVSCPevent_v2( &pEvent );
		}

	}

	printf("Close max6675\n");
	max6675.close();
}
*/

    return 0;
}