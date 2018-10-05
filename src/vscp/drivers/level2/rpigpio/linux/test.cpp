// test.cpp : 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2007 Johan Hedlund,  <kungjohan@gmail.com>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
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

#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>
 
#include "rpigpio.h"
#ifdef USE_PIGPIOD
#include <pigpiod_if2.h>
#else
    #include <pigpio.h>
#endif


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
BlockingSend( CRpiGpio *pgpio, const vscpEvent *pEvent, unsigned long timeout )
{
	int rv = 0;
	if ( NULL == pgpio ) return CANAL_ERROR_MEMORY;
    pgpio->addEvent2SendQueue( pEvent );
	return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  BlockingReceive
// 

int
BlockingReceive( CRpiGpio *pgpio, vscpEvent *pEvent, unsigned long timeout )
{
	int rv = 0;
 
    // Check pointer
    if ( NULL == pEvent) return CANAL_ERROR_PARAMETER;
    
	if (NULL == pgpio) return CANAL_ERROR_MEMORY;
    
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = timeout * 1e6;
	if ( ETIMEDOUT == sem_timedwait( &pgpio->m_semaphore_ReceiveQueue, &ts ) ) {
        return CANAL_ERROR_TIMEOUT;
    }
    
	pthread_mutex_lock( &pgpio->m_mutex_ReceiveQueue );
    vscpEvent *pLocalEvent = pgpio->m_receiveList.front();
    pgpio->m_receiveList.pop_front();
	pthread_mutex_unlock( &pgpio->m_mutex_ReceiveQueue );
    if ( NULL == pLocalEvent ) return CANAL_ERROR_MEMORY;
    
    vscp_copyVSCPEvent( pEvent, pLocalEvent );
    vscp_deleteVSCPevent( pLocalEvent );
	
	return CANAL_ERROR_SUCCESS;
}

 
int main(void)
{
	int i;
    CRpiGpio gpio;

	if ( SIG_ERR == signal(SIGSEGV, sig_handler) ) printf("Could not catch SIGSEGV\n");
	if ( SIG_ERR == signal(SIGTERM, sig_handler) ) printf("Could not catch SIGSEGV\n");

    //::wxInitialize();

	printf("Open gpio\n");
    gpio.open( "admin",
		        "secret",
		        "192.168.1.9",
		        9598,
		        "rpi",
		        "t1t2t3t4t5t");

	for ( i=0; i<50; i++ ) {

		// CLASS1.INFORMATION, Type=3, ON
		printf("Send CLASS1.INFORMATION, Type=3,ON\n");
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

		BlockingSend( &gpio, pEventOn, 500 );
		usleep(100000);

		// CLASS1.INFORMATION, Type=4, OFF
		printf("Send CLASS1.INFORMATION, Type=4,OFF\n");
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

		BlockingSend( &gpio, pEventOff, 500 );
		usleep(100000);
	}

	printf("Close gpio\n");
	gpio.close();

	//::wxUninitialize();

    return 0;
}