// test.cpp : 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2019 Ake Hedman,
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
// $RCSfile: test.cpp,v $                                       
// $Date: 2005/09/08 07:56:00 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dlfcn.h>

#include "../../../../common/devicelist.h"

#define NUMBER_OF_THREADS 2

// Prototypes
void *workThread( void *id );

int main( void )
{
	int i;
	pthread_t workthreads[ NUMBER_OF_THREADS ];
	pthread_attr_t thread_attr;

	pthread_attr_init( &thread_attr );
	
	printf("\n\n\n");
	for ( i=0; i<NUMBER_OF_THREADS; i++ ) {
		
		// Create the worker thread.
		if ( pthread_create( 	&workthreads[ i ],
									&thread_attr,
									workThread,
									&i ) ) {	
		}
		
	}
	
	// Give the threads some time to work
	printf("Giving threads some time to work.\n");
	sleep( 5 );
	
	for ( i=0; i<NUMBER_OF_THREADS; i++ ) {
		int *rv;
		pthread_join( workthreads[ i ], (void **)&rv );
	}

	return ( EXIT_SUCCESS );
}

///////////////////////////////////////////////////////////////////////////////
// workThread
//

void *workThread( void *id )
{
	int rv = 0;
	void* handle = 0;
	//parent *(*open)(void) = 0
	//const char* error = 0;
	LPFNDLL_CANALOPEN			        m_proc_CanalOpen;
	LPFNDLL_CANALCLOSE				    m_proc_CanalClose;
	LPFNDLL_CANALGETLEVEL			    m_proc_CanalGetLevel;
	LPFNDLL_CANALSEND				    m_proc_CanalSend;
	LPFNDLL_CANALRECEIVE			    m_proc_CanalReceive;
	LPFNDLL_CANALDATAAVAILABLE		    m_proc_CanalDataAvailable;
	LPFNDLL_CANALGETSTATUS			    m_proc_CanalGetStatus;
	LPFNDLL_CANALGETSTATISTICS		    m_proc_CanalGetStatistics;
	LPFNDLL_CANALSETFILTER			    m_proc_CanalSetFilter;
	LPFNDLL_CANALSETMASK			    m_proc_CanalSetMask;
	LPFNDLL_CANALSETBAUDRATE		    m_proc_CanalSetBaudrate;
	LPFNDLL_CANALGETVERSION			    m_proc_CanalGetVersion;
	LPFNDLL_CANALGETDLLVERSION		    m_proc_CanalGetDllVersion;
	LPFNDLL_CANALGETVENDORSTRING	    m_proc_CanalGetVendorString;
	// Generation 2
	LPFNDLL_CANALBLOCKINGSEND		    m_proc_CanalBlockingSend;
	LPFNDLL_CANALBLOCKINGRECEIVE	    m_proc_CanalBlockingReceive;
	LPFNDLL_CANALGETDRIVERINFO		    m_proc_CanalGetdriverInfo;

    // Level II driver methods
    LPFNDLL_VSCPOPEN			        m_proc_VSCPOpen;
	LPFNDLL_VSCPCLOSE				    m_proc_VSCPClose;
	LPFNDLL_VSCPBLOCKINGSEND			m_proc_VSCPBlockingSend;
	LPFNDLL_VSCPBLOCKINGRECEIVE			m_proc_VSCPBlockingReceive;
    LPFNDLL_VSCPGETLEVEL				m_proc_VSCPGetLevel;
    LPFNDLL_VSCPGETVERSION			    m_proc_VSCPGetVersion;
	LPFNDLL_VSCPGETDLLVERSION		    m_proc_VSCPGetDllVersion;
	LPFNDLL_VSCPGETVENDORSTRING	        m_proc_VSCPGetVendorString;
    LPFNDLL_VSCPGETDRIVERINFO		    m_proc_VSCPGetdriverInfo;
	LPFNDLL_VSCPGETWEBPAGETEMPLATE		m_proc_VSCPGetWebPageTemplate;
	LPFNDLL_VSCPGETWEBPAGEINFO			m_proc_VSCPGetWebPageInfo;
	LPFNDLL_VSCPWEBPAGEUPDATE			m_proc_VSCPWebPageupdate;

	handle = dlopen("./can232drv.so", RTLD_LAZY);
	
	if ( 0 == handle ) {
		fprintf( stderr, "test: Failed to load library %s\n", dlerror() );
		exit ( EXIT_FAILURE );
	}

	// * * * * CANAL OPEN * * * * 
	if ( 0 == ( m_proc_CanalOpen = (LPFNDLL_CANALOPEN)dlsym( handle, "CanalOpen" ) ) )  {
	
		// Free the library	
		dlclose( handle );
		fprintf( stderr, "test: Unable to get dl entry for CanalOpen. error=%s", dlerror() );
		exit ( EXIT_FAILURE );
		
	}
	
	// * * * * CANAL CLOSE * * * * 
	if ( 0 == ( m_proc_CanalClose = (LPFNDLL_CANALCLOSE)dlsym( handle, "CanalClose" ) ) )  {
	
		// Free the library	
		dlclose( handle );
		fprintf( stderr, "test: Unable to get dl entry for CanalClose. error=%s", dlerror() );
		exit ( EXIT_FAILURE );
		
	}
	
	// * * * * CANAL GETLEVEL * * * * 
	if ( 0 == ( m_proc_CanalGetLevel = (LPFNDLL_CANALGETLEVEL)dlsym( handle, "CanalGetLevel" ) ) )  {
	
		// Free the library	
		dlclose( handle );
		fprintf( stderr, "test: Unable to get dl entry for CanalGetLevel. error=%s", dlerror() );
		exit ( EXIT_FAILURE );
		
	}
	
	// * * * * CANAL SEND * * * * 
	if ( 0 == ( m_proc_CanalSend = (LPFNDLL_CANALSEND)dlsym( handle, "CanalSend" ) ) )  {
	
		// Free the library	
		dlclose( handle );
		fprintf( stderr, "test: Unable to get dl entry for CanalSend. error=%s", dlerror() );
		exit ( EXIT_FAILURE );
		
	}
	
	
	// * * * * CANAL DATA AVAILABLE * * * * 
	if ( 0 == ( m_proc_CanalDataAvailable = (LPFNDLL_CANALDATAAVAILABLE)dlsym( handle, "CanalDataAvailable" ) ) )  {
	
		// Free the library	
		dlclose( handle );
		fprintf( stderr, "test: Unable to get dl entry for CanalDataAvailable. error=%s", dlerror() );
		exit ( EXIT_FAILURE );
		
	}
	
	// * * * * CANAL DATA AVAILABLE * * * * 
	if ( 0 == ( m_proc_CanalDataAvailable = (LPFNDLL_CANALDATAAVAILABLE)dlsym( handle, "CanalDataAvailable" ) ) )  {
	
		// Free the library	
		dlclose( handle );
		fprintf( stderr, "test: Unable to get dl entry for CanalDataAvailable. error=%s", dlerror() );
		exit ( EXIT_FAILURE );
		
	}
	
	// * * * * CANAL RECEIVE * * * * 
	if ( 0 == ( m_proc_CanalReceive = (LPFNDLL_CANALRECEIVE)dlsym( handle, "CanalReceive" ) ) )  {
	
		// Free the library	
		dlclose( handle );
		fprintf( stderr, "test: Unable to get dl entry for CanalReceive. error=%s", dlerror() );
		exit ( EXIT_FAILURE );
		
	}
	
	// * * * * CANAL GET STATUS * * * * 
	if ( 0 == ( m_proc_CanalGetStatus = (LPFNDLL_CANALGETSTATUS)dlsym( handle, "CanalGetStatus" ) ) )  {
	
		// Free the library	
		dlclose( handle );
		fprintf( stderr, "test: Unable to get dl entry for CanalGetStatus. error=%s", dlerror() );
		exit ( EXIT_FAILURE );
		
	}
	
	// * * * * CANAL GET STATISTICS * * * * 
	if ( 0 == ( m_proc_CanalGetStatistics = (LPFNDLL_CANALGETSTATISTICS)dlsym( handle, "CanalGetStatistics" ) ) )  {
	
		// Free the library	
		dlclose( handle );
		fprintf( stderr, "test: Unable to get dl entry for CanalGetStatistics. error=%s", dlerror() );
		exit ( EXIT_FAILURE );
		
	}
	
	// * * * * CANAL SET FILTER * * * * 
	if ( 0 == ( m_proc_CanalSetFilter = (LPFNDLL_CANALSETFILTER)dlsym( handle, "CanalSetFilter" ) ) )  {
	
		// Free the library	
		dlclose( handle );
		fprintf( stderr, "test: Unable to get dl entry for CanalSetFilter. error=%s", dlerror() );
		exit ( EXIT_FAILURE );
		
	}
	
	// * * * * CANAL SET MASK * * * * 
	if ( 0 == ( m_proc_CanalSetMask = (LPFNDLL_CANALSETMASK)dlsym( handle, "CanalSetMask" ) ) )  {
	
		// Free the library	
		dlclose( handle );
		fprintf( stderr, "test: Unable to get dl entry for CanalSetMask. error=%s", dlerror() );
		exit ( EXIT_FAILURE );
		
	}
	
	// * * * * CANAL GET VERSION * * * * 
	if ( 0 == ( m_proc_CanalGetVersion = (LPFNDLL_CANALGETVERSION)dlsym( handle, "CanalGetVersion" ) ) )  {
	
		// Free the library	
		dlclose( handle );
		fprintf( stderr, "test: Unable to get dl entry for CanalGetVersion. error=%s", dlerror() );
		exit ( EXIT_FAILURE );
		
	}
	
	// * * * * CANAL GET DLL VERSION * * * *
	if ( 0 == ( m_proc_CanalGetDllVersion = (LPFNDLL_CANALGETDLLVERSION)dlsym( handle, "CanalGetDllVersion" ) ) )  {
	
		// Free the library	
		dlclose( handle );
		fprintf( stderr, "test: Unable to get dl entry for CanalGetDllVersion. error=%s", dlerror() );
		exit ( EXIT_FAILURE );
		
	}
	
	// * * * * CANAL GET VENDOR STRING * * * * 
	if ( 0 == ( m_proc_CanalGetVendorString = (LPFNDLL_CANALGETVENDORSTRING)dlsym( handle, "CanalGetVendorString" ) ) )  {
	
		// Free the library	
		dlclose( handle );
		fprintf( stderr, "test: Unable to get dl entry for CanalGetVendorString. error=%s", dlerror() );
		exit ( EXIT_FAILURE );
		
	}
	
	////////////////////////////////////////////////////////////////////////////
	//                           * * * T E S T S * * *
	///////////////////////////////////////////////////////////////////////////
	
	// Open
	char device[ 32 ];
	char flags[ 32 ];
	long dh;
	sprintf( device, "/dev/ttyS0;19200;0;0;125" );
	dh = (*m_proc_CanalOpen)( device, 1 );

	printf("Sending data.\n");
	
	canalMsg msg;
	
	//for ( int i= 0; i<256; i++ ) {
	for ( int i= 0; i<10; i++ ) {
	
		msg.obid = 1946;
		msg.flags = CANAL_IDFLAG_EXTENDED;		 			 
		msg.id = i;			 
		msg.sizeData = 8;		 
		msg.data[0] = 0x11;		 
		msg.data[1] = 0x22;
		msg.data[2] = 0x33;
		msg.data[3] = 0x44;
		msg.data[4] = 0x55;
		msg.data[5] = 0x66;
		msg.data[6] = 0x77;
		msg.data[7] = 0x88;
		msg.timestamp = 3210;
		(*m_proc_CanalSend)( dh, &msg );
		printf(".");
	
		msg.obid = 2001;
		msg.flags = CANAL_IDFLAG_EXTENDED;		 			 
		msg.id = i;			 
		msg.sizeData = 4;		 
		msg.data[0] = 0xA1;		 
		msg.data[1] = 0xB2;
		msg.data[2] = 0xC3;
		msg.data[3] = 0xD4;
		msg.data[4] = 0xE5;
		msg.data[5] = 0xF6;
		msg.data[6] = 0x07;
		msg.data[7] = 0x18;
		msg.timestamp = 0xcc55;
		(*m_proc_CanalSend)( dh, &msg );
		printf(".");
	}
	
	printf("\n");
	
	printf("===========================================================\n");
	printf("=                       Data for DLL\n");
	printf("===========================================================\n");
	
	printf( "Canal Level = %lu\n", (*m_proc_CanalGetLevel)( dh ) );
	printf( "Canal Version = %lu\n", (*m_proc_CanalGetVersion)() );
	printf( "DLL Version = %lu\n", (*m_proc_CanalGetDllVersion)() );				
	printf( "Vendor String = %s\n", (*m_proc_CanalGetVendorString)() ); 

	printf("===========================================================\n");
	
	printf("Closing Log function\n");
	
	// Give the driver some time to finish writing
	printf("Waiting before close\n\n\n");
	sleep( 5 );
	
	// Close
	(*m_proc_CanalClose)( dh );
	
	printf("Closing DLL\n");

	dlclose( handle );
	
	pthread_exit( &rv );
}


