///////////////////////////////////////////////////////////////////////////////
// DllWrapper.cpp: implementation of the CDllWrapper class.
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//

#if WIN32
#include <windows.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#else
#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <dlfcn.h>
#include <syslog.h>

#include "wx/wx.h"
#include "wx/defs.h"

#endif

#include <vscp_class.h>
#include <vscp_type.h>
#include "dllwrapper.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CDllWrapper::CDllWrapper()
{
	m_bInit = false;
	m_devid = 0;

    // Init. register read parameters
	m_registerReadErrorTimeout = DLL_REGISTER_READ_ERROR_TIMEOUT;
	m_registerReadResendTimeout = DLL_REGISTER_READ_RESEND_TIMEOUT;
	m_registerReadMaxRetries = DLL_REGISTER_READ_MAX_TRIES;
}


CDllWrapper::~CDllWrapper()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// initialize
//

int CDllWrapper::initialize( wxString& strPath )
{
	// Check that the file exists
	if ( !( ::wxFileExists( strPath ) ) ) return CANAL_ERROR_PARAMETER;  
  
	// Save the path
	m_strPath = strPath;

	// Load dynamic library
	if ( ! m_wxdll.Load( strPath, wxDL_LAZY ) ) {
		wxLogDebug( _("Unable to load dynamic library."));
		return CANAL_ERROR_PARAMETER;
	}

	// Now find methods in library

	// * * * * CANAL OPEN * * * *
	if ( NULL == ( m_proc_CanalOpen =
	                   ( LPFNDLL_CANALOPEN ) m_wxdll.GetSymbol ( _T ( "CanalOpen" ) ) ) ) {
		// Free the library
		wxLogDebug(  _( "Unable to get dl entry for CanalOpen." ) );
		return CANAL_ERROR_PARAMETER;
	}

	// * * * * CANAL CLOSE * * * *
	if ( NULL == ( m_proc_CanalClose =
	                   ( LPFNDLL_CANALCLOSE ) m_wxdll.GetSymbol ( _T ( "CanalClose" ) ) ) ) {
		// Free the library
		wxLogDebug( _( "Unable to get dl entry for CanalClose." ) );
		return CANAL_ERROR_PARAMETER;
	}

	// * * * * CANAL GETLEVEL * * * *
	if ( NULL == ( m_proc_CanalGetLevel =
	                   ( LPFNDLL_CANALGETLEVEL ) m_wxdll.GetSymbol ( _T ( "CanalGetLevel" ) ) ) ) {
		// Free the library
		wxLogDebug( _( "Unable to get dl entry for CanalGetLevel." ) );
		return CANAL_ERROR_PARAMETER;
	}

	// * * * * CANAL SEND * * * *
	if ( NULL == ( m_proc_CanalSend =
	                   ( LPFNDLL_CANALSEND ) m_wxdll.GetSymbol ( _T ( "CanalSend" ) ) ) ) {
		// Free the library
		wxLogDebug( _( "Unable to get dl entry for CanalSend." ) );
		return CANAL_ERROR_PARAMETER;
	}

	// * * * * CANAL DATA AVAILABLE * * * *
	if ( NULL == ( m_proc_CanalDataAvailable =
	                   ( LPFNDLL_CANALDATAAVAILABLE ) m_wxdll.GetSymbol ( _T ( "CanalDataAvailable" ) ) ) ) {
		// Free the library
		wxLogDebug( _( "Unable to get dl entry for CanalDataAvailable." ) );
		return CANAL_ERROR_PARAMETER;
	}


	// * * * * CANAL RECEIVE * * * *
	if ( NULL == ( m_proc_CanalReceive =
	                   ( LPFNDLL_CANALRECEIVE ) m_wxdll.GetSymbol ( _T ( "CanalReceive" ) ) ) ) {
		// Free the library
		wxLogDebug( _( "Unable to get dl entry for CanalReceive." ) );
		return CANAL_ERROR_PARAMETER;
	}

	// * * * * CANAL GET STATUS * * * *
	if ( NULL == ( m_proc_CanalGetStatus =
	                   ( LPFNDLL_CANALGETSTATUS ) m_wxdll.GetSymbol ( _T ( "CanalGetStatus" ) ) ) ) {
		// Free the library
		wxLogDebug( _( "Unable to get dl entry for CanalGetStatus." ) );
		return CANAL_ERROR_PARAMETER;
	}

	// * * * * CANAL GET STATISTICS * * * *
	if ( NULL == ( m_proc_CanalGetStatistics =
	                   ( LPFNDLL_CANALGETSTATISTICS ) m_wxdll.GetSymbol ( _T ( "CanalGetStatistics" ) ) ) ) {
		// Free the library
		wxLogDebug( _( "Unable to get dl entry for CanalGetStatistics." ) );
		return CANAL_ERROR_PARAMETER;
	}

	// * * * * CANAL SET FILTER * * * *
	if ( NULL == ( m_proc_CanalSetFilter =
	                   ( LPFNDLL_CANALSETFILTER ) m_wxdll.GetSymbol ( _T ( "CanalSetFilter" ) ) ) ) {
		// Free the library
		wxLogDebug( _( "Unable to get dl entry for CanalSetFilter." ) );
		return CANAL_ERROR_PARAMETER;
	}

	// * * * * CANAL SET MASK * * * *
	if ( NULL == ( m_proc_CanalSetMask =
	                   ( LPFNDLL_CANALSETMASK ) m_wxdll.GetSymbol ( _T ( "CanalSetMask" ) ) ) ) {
		// Free the library
		wxLogDebug( _( "Unable to get dl entry for CanalSetMask." ));
		return CANAL_ERROR_PARAMETER;
	}

	// * * * * CANAL GET VERSION * * * *
	if ( NULL == ( m_proc_CanalGetVersion =
	                   ( LPFNDLL_CANALGETVERSION ) m_wxdll.GetSymbol ( _T ( "CanalGetVersion" ) ) ) ) {
		// Free the library
		wxLogDebug( _( "Unable to get dl entry for CanalGetVersion." ));
		return CANAL_ERROR_PARAMETER;
	}

	// * * * * CANAL GET DLL VERSION * * * *
	if ( NULL == ( m_proc_CanalGetDllVersion =
	                   ( LPFNDLL_CANALGETDLLVERSION ) m_wxdll.GetSymbol ( _T ( "CanalGetDllVersion" ) ) ) ) {
		// Free the library
		wxLogDebug( _( "Unable to get dl entry for CanalGetDllVersion." ));
		return CANAL_ERROR_PARAMETER;
	}

	// * * * * CANAL GET VENDOR STRING * * * *
	if ( NULL == ( m_proc_CanalGetVendorString =
	                   ( LPFNDLL_CANALGETVENDORSTRING ) m_wxdll.GetSymbol ( _T ( "CanalGetVendorString" ) ) ) ) {
		// Free the library
		wxLogDebug( _( "Unable to get dl entry for CanalGetVendorString." ));
		return CANAL_ERROR_PARAMETER;
	}


  wxLogNull logNo;


	// ******************************
	//     Generation 2 Methods
	// ******************************


	// * * * * CANAL BLOCKING SEND * * * *
	if ( NULL == ( m_proc_CanalBlockingSend =
	                   ( LPFNDLL_CANALBLOCKINGSEND ) m_wxdll.GetSymbol ( _T ( "CanalBlockingSend" ) ) ) ) {
		wxLogDebug( _( "Unable to get dl entry for CanalBlockingSend. Probably Generation 1 driver." ) );
    m_proc_CanalBlockingSend = NULL;
	}

	// * * * * CANAL BLOCKING RECEIVE * * * *
	if ( NULL == ( m_proc_CanalBlockingReceive =
	                   ( LPFNDLL_CANALBLOCKINGRECEIVE ) m_wxdll.GetSymbol ( _T ( "CanalBlockingReceive" ) ) ) ) {
		wxLogDebug( _( "Unable to get dl entry for CanalBlockingReceive. Probably Generation 1 driver." ) );
    m_proc_CanalBlockingReceive = NULL;
	}

	// * * * * CANAL GET DRIVER INFO * * * *
	if ( NULL == ( m_proc_CanalGetdriverInfo =
	                   ( LPFNDLL_CANALGETDRIVERINFO ) m_wxdll.GetSymbol ( _T ( "CanalGetDriverInfo" ) ) ) ) {
		wxLogDebug( _( "Unable to get dl entry for CanalGetDriverInfo. Probably Generation 1 driver." ) );
        m_proc_CanalGetdriverInfo = NULL;
	}

  
	// Mark as initialized
	m_bInit = true;
  
	return CANAL_ERROR_SUCCESS;
  
}


///////////////////////////////////////////////////////////////////////////////
// open
//

long CDllWrapper::doCmdOpen( const wxString& strConfiguration, unsigned long flags )
{
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;
  
#ifdef WIN32
  char szCfg[ MAX_PATH ];
  strncpy( szCfg, strConfiguration.mb_str(), MAX_PATH );
#else
  char szCfg[ PATH_MAX ];
  strncpy( szCfg, strConfiguration.mb_str(), PATH_MAX );
#endif
  //memcpy( szInterface, strInterface.ToAscii(), strInterface.Length() ); TODO
  
  
	return ( m_devid = m_proc_CanalOpen( szCfg, flags ) );
}


///////////////////////////////////////////////////////////////////////////////
// close
//

int CDllWrapper::doCmdClose( void )
{
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;
  
	return m_proc_CanalClose( m_devid );
}


///////////////////////////////////////////////////////////////////////////////
// doCmdGetLevel
//

unsigned long CDllWrapper::doCmdGetLevel( void )
{ 
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;
  
	return m_proc_CanalGetLevel( m_devid );
}


///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//

int CDllWrapper::doCmdSend( canalMsg * pMsg )
{	
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;
  
	return m_proc_CanalSend( m_devid, pMsg );
}

///////////////////////////////////////////////////////////////////////////////
// doCmdBlockingSend
//

int CDllWrapper::doCmdBlockingSend( canalMsg * pMsg, unsigned long timeout )
{	
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;

	if ( NULL == m_proc_CanalBlockingSend ) return CANAL_ERROR_NOT_SUPPORTED;
  
	return m_proc_CanalBlockingSend( m_devid, pMsg, timeout );
}

///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

int CDllWrapper::doCmdReceive( canalMsg *pMsg )
{		
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;
  
	return m_proc_CanalReceive( m_devid, pMsg );
}

///////////////////////////////////////////////////////////////////////////////
// doCmdBlockingReceive
//

int CDllWrapper::doCmdBlockingReceive( canalMsg *pMsg, unsigned long timeout )
{		
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;
	
	if ( NULL == m_proc_CanalBlockingReceive ) return CANAL_ERROR_NOT_SUPPORTED;  

	return m_proc_CanalBlockingReceive( m_devid, pMsg, timeout );
}

///////////////////////////////////////////////////////////////////////////////
// doCmdDataAvailable
//

int CDllWrapper::doCmdDataAvailable( void )
{
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;
  
	return m_proc_CanalDataAvailable( m_devid );
}

///////////////////////////////////////////////////////////////////////////////
// doCmdState
//

int CDllWrapper::doCmdStatus( canalStatus *pStatus )
{	
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;
  
	return m_proc_CanalGetStatus( m_devid, pStatus );
}



///////////////////////////////////////////////////////////////////////////////
// doCmdStatistics
//

int CDllWrapper::doCmdStatistics( canalStatistics *pStatistics )
{	
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;
  
	return m_proc_CanalGetStatistics( m_devid, pStatistics );
}



///////////////////////////////////////////////////////////////////////////////
// doCmdFilter
//

int CDllWrapper::doCmdFilter( unsigned long filter )
{	
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;
  
	return m_proc_CanalSetFilter( m_devid, filter );
}



///////////////////////////////////////////////////////////////////////////////
// doCmdMask
//

int CDllWrapper::doCmdMask( unsigned long mask )
{	
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;
  
	return m_proc_CanalSetMask( m_devid, mask );
}

///////////////////////////////////////////////////////////////////////////////
// doCmdBaudrate

int CDllWrapper::doCmdBaudrate( unsigned long baudrate )
{
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;
  
	return m_proc_CanalSetBaudrate( m_devid, baudrate );
}


///////////////////////////////////////////////////////////////////////////////
// doCmdVersion
//

unsigned long CDllWrapper::doCmdVersion( void )
{
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;
  
	return m_proc_CanalGetVersion();
}


///////////////////////////////////////////////////////////////////////////////
// doCmdDLLVersion
//

unsigned long CDllWrapper::doCmdDLLVersion( void )
{
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return CANAL_ERROR_INIT_FAIL;
  
	return m_proc_CanalGetDllVersion();
}

///////////////////////////////////////////////////////////////////////////////
// doCmdVendorString
//

const char * CDllWrapper::doCmdVendorString( void )
{
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return NULL;
  
	return m_proc_CanalGetVendorString();
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetDriverInfo
//

const char * CDllWrapper::doCmdGetDriverInfo( void )
{
	// If not initialized there is nothing we can do
	if ( !m_bInit ) return NULL;

	if ( NULL == m_proc_CanalGetdriverInfo ) return NULL;
  
	return m_proc_CanalGetdriverInfo();
}

///////////////////////////////////////////////////////////////////////////////
// isBlockingSupported
//

bool CDllWrapper::isBlockingSupported( bool bRead, bool bWrite )
{
	bool bReceiveBlockingSupported = true;
	bool bTransmitBlockingSupported = true;

	if ( bRead && ( NULL == m_proc_CanalBlockingReceive ) ) {
		bReceiveBlockingSupported = false;
	}

	if ( bWrite && ( NULL == m_proc_CanalBlockingSend ) ) {
		bTransmitBlockingSupported = false;
	}

	return ( bReceiveBlockingSupported && bTransmitBlockingSupported );
}


//////////////////////////////////////////////////////////////////////////////
// readLevel1Register
//

int CDllWrapper::readLevel1Register( unsigned char nodeid, 
                                        unsigned short page,
								        unsigned char reg, 
								        unsigned char *pcontent )
{
	int rv = CANAL_ERROR_SUCCESS;
	unsigned long errors = 0;
	bool bResend;
	wxString strBuf;
	canalMsg canalEvent;

	// Check pointer
	if ( NULL == pcontent ) return false;

	canalEvent.flags = CANAL_IDFLAG_EXTENDED;
	canalEvent.obid = 0;
	canalEvent.id = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ << 8;
	canalEvent.sizeData = 4;
	canalEvent.data[ 0 ] = nodeid;              // Node to read from
    canalEvent.data[ 1 ] = (page>>8) & 0xff;    // MSB of page
    canalEvent.data[ 2 ] = page & 0xff;         // LSB of page
	canalEvent.data[ 3 ] = reg;                 // Register to write

	bResend = false;
	doCmdSend( &canalEvent );

	wxLongLong startTime = ::wxGetLocalTimeMillis();

	while ( true ) {

        if ( doCmdDataAvailable() ) {									// Message available
            if ( CANAL_ERROR_SUCCESS == doCmdReceive( &canalEvent ) ) {	// Valid event
                if ( 5 != canalEvent.sizeData ) continue;
                if ( (unsigned short)( canalEvent.id & 0xffff ) ==
                    ( ( VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE << 8 ) + nodeid ) ) {  // Read reply?
                        if ( ( 0 == canalEvent.data[ 0 ] ) &&
                                ( (page >> 8) == canalEvent.data[ 1 ] ) &&
                                ( (page & 0xff ) == canalEvent.data[ 2 ] ) &&
                                ( canalEvent.data[ 3 ] == reg ) ) {	    // Requested register?

                            if ( NULL != pcontent ) {
                                *pcontent = canalEvent.data[ 4 ];
                            }
							break;
                        }

				} // Check for correct reply event 
			}
		}
		else {
			wxMilliSleep( 2 );
		}

		// Check for read error timeout
		if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
			m_registerReadErrorTimeout ) {
				errors++;
		}
		// Should we resend?
		else if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
			m_registerReadResendTimeout ) {
				// Send again
				if ( !bResend) {
					doCmdSend( &canalEvent );
				}
				bResend = true;
		}

		if ( errors > m_registerReadMaxRetries ) {
			rv = false;
			break;
		}

	} // while

	return rv;
}


//////////////////////////////////////////////////////////////////////////////
// writeLevel1Register
//

int CDllWrapper::writeLevel1Register( unsigned char nodeid, 
                                        unsigned short page,
                                        unsigned char reg, 
                                        unsigned char *pval )
{
	int rv = CANAL_ERROR_SUCCESS;
	unsigned long errors = 0;
	bool bResend;
	wxString strBuf;
	canalMsg canalEvent;

	canalEvent.flags = CANAL_IDFLAG_EXTENDED;
	canalEvent.obid = 0;
    canalEvent.id = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE << 8;
	canalEvent.sizeData = 5;
	canalEvent.data[ 0 ] = nodeid;              // Node to read from
    canalEvent.data[ 1 ] = (page>>8) & 0xff;    // MSB of page
    canalEvent.data[ 2 ] = page & 0xff;         // LSB of page
	canalEvent.data[ 3 ] = reg;                 // Register to write
    canalEvent.data[ 4 ] = *pval;               // Value

	bResend = false;
	doCmdSend( &canalEvent );

	wxLongLong startTime = ::wxGetLocalTimeMillis();

	while ( true ) {

		if ( doCmdDataAvailable() ) {									// Message available
			if ( CANAL_ERROR_SUCCESS == doCmdReceive( &canalEvent ) ) { // Valid event
                if ( 5 != canalEvent.sizeData ) continue;
				if ( (unsigned short)( canalEvent.id & 0xffff ) ==
					( ( VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE << 8 ) + nodeid ) ) {  // Read reply?
						if ( ( 0 == canalEvent.data[ 0 ] ) &&
                            ( (page >> 8) == canalEvent.data[ 1 ] ) &&
                            ( (page & 0xff ) == canalEvent.data[ 2 ] ) &&
                            ( canalEvent.data[ 3 ] == reg ) ) {			// Requested register?

							if ( *pval != canalEvent.data[ 4 ] ) rv = CANAL_ERROR_REGISTER;
							
                            // Save read value
							*pval = canalEvent.data[ 4 ];
							break;

						} // Check for correct node

						// Save read value
						*pval = canalEvent.data[ 4 ];

				} // Check for correct reply event 
			}
		}
        else {
			wxMilliSleep( 2 );
        }

        if ( ( ::wxGetLocalTimeMillis() - startTime ) > m_registerReadErrorTimeout ) {
            errors++;
		}
        else if ( ( ::wxGetLocalTimeMillis() - startTime ) > m_registerReadResendTimeout ) {
            // Send again
            if ( !bResend) {
                doCmdSend( &canalEvent );
            }
            bResend = true;
        }

        if ( errors > m_registerReadMaxRetries ) {
            rv = false;
            break;
        }

    } // while

    return rv;
}


//////////////////////////////////////////////////////////////////////////////
// readLevel1Registers
//

int CDllWrapper::readLevel1Registers( unsigned short page,
												unsigned char *pregisters,
												unsigned char nodeid,
												unsigned char startreg,
												unsigned short count,
                                                unsigned long idCallback,
												canalreginfo_callback_t *pcallback )
{
	int i;
	unsigned char val;
	int rv = CANAL_ERROR_SUCCESS;
	int errors = 0;
	wxString strBuf;


	// *********************
	// Read register content
	// *********************
	for ( i = startreg; i < (startreg + count); i++ ) {

		if ( NULL != pcallback ) {
            // !progressDlg.Update( i-startreg )
			//progressDlg.Pulse( wxString::Format(_("Reading register %d"), i) );
		}

		

		if ( readLevel1Register( page, nodeid, i, &val ) ) {
			pregisters[ i-startreg ] = val;
		}
		else {
			errors++;
		}

		if ( errors > 2 ) {
            if ( NULL != pcallback ) {
			    //wxMessageBox( _("No node present or problems when communicating with node. Aborting!") );
            }
			rv = false;
			break;
		}

	} // for

	return rv;
}

//////////////////////////////////////////////////////////////////////////////
// getMDFfromLevel1Device
//

bool CDllWrapper::getMDFfromLevel1Device( unsigned char nodeid, wxString &strurl  )
{
	char url[ 33 ];
    unsigned char *p = (unsigned char *)url;
	bool rv = true;

    canalMsg canalEvent;

	canalEvent.flags = CANAL_IDFLAG_EXTENDED;
	canalEvent.obid = 0;
    canalEvent.id = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ << 8;
	canalEvent.sizeData = 5;
	canalEvent.data[ 0 ] = nodeid;  // Node to read from
    canalEvent.data[ 1 ] = 0;       // MSB of page
    canalEvent.data[ 2 ] = 0;       // LSB of page
	canalEvent.data[ 3 ] = 0xE0;    // Register to write
    canalEvent.data[ 4 ] = 32;      // All 32 bytes

    // Send it away
	doCmdSend( &canalEvent );

    wxLongLong startTime = ::wxGetLocalTimeMillis();

    // We should get eight response frames back
    unsigned char receive_flags = 0;
    while ( 255 != receive_flags ) {

        if ( doCmdDataAvailable() ) {	// Message available
			
            if ( CANAL_ERROR_SUCCESS == doCmdReceive( &canalEvent ) ) { // Valid event
                
                if ( 8 != canalEvent.sizeData ) continue;
				
                if ( (unsigned short)( canalEvent.id & 0xffff ) ==
					( ( VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE << 8 ) + nodeid ) ) {  // Read reply?
                        
                        // Check bounds
                        if ( canalEvent.data[ 0 ] > 7 ) continue;

                        // Mark frame as received
                        receive_flags |= ( 1 << canalEvent.data[ 0 ] );

                        for ( int i=0;i<4; i++ ) {
                            url[ canalEvent.data[ 0 ]*4 + i] = canalEvent.data[ 4 + i ];
                        }

				} // Check for correct reply event 
			}
		}
        else {
			wxMilliSleep( 2 );
        }

        if ( ( ::wxGetLocalTimeMillis() - startTime ) > m_registerReadErrorTimeout ) {
            rv = false;
            goto error;
		}
    }

	strurl = strurl.From8BitData( url );
	if ( wxNOT_FOUND == strurl.Find( _("http://") ) ) {
		wxString str;
		str = _("http://");
		str += strurl;
		strurl = str;
	}

error:

	return rv;
}


//////////////////////////////////////////////////////////////////////////////
// getRegistersfromLevel1Device
//
// 1,3,7,15,31,63,127,255,  (2^n)-1  ((2^nPages)-1)
//

int CDllWrapper::getRegistersfromLevel1Device( unsigned char nodeid, 
                                                    unsigned short reg,
                                                    unsigned short page,
                                                    unsigned char count,
                                                    unsigned char *pRegs )
{
	bool rv = CANAL_ERROR_SUCCESS;
    int fcnt = 0;
    unsigned char data[256];    // This makes range checking simpler

    // Max 128 bytes can be read
    if ( count > 128 ) return false;

    canalMsg canalEvent;

	canalEvent.flags = CANAL_IDFLAG_EXTENDED;
	canalEvent.obid = 0;
    canalEvent.id = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ << 8;
	canalEvent.sizeData = 5;
	canalEvent.data[ 0 ] = nodeid;      // Node to read from
    canalEvent.data[ 1 ] = page>>8;     // MSB of page
    canalEvent.data[ 2 ] = page & 0xff; // LSB of page
	canalEvent.data[ 3 ] = reg;         // Start register to read
    canalEvent.data[ 4 ] = count;       // All 32 bytes

    // Send it away
	doCmdSend( &canalEvent );

    wxLongLong startTime = ::wxGetLocalTimeMillis();

    // We should get eight response frames back
    unsigned long receive_flags = 0;
    unsigned char nPages = count/4;
    unsigned char lastpageCnt = count%4;
    if ( lastpageCnt ) nPages++;
    unsigned long allRcvValue = pow(2.0,nPages) - 1;

    while ( allRcvValue != receive_flags ) {

        if ( doCmdDataAvailable() ) {									    // Message available
			
            if ( CANAL_ERROR_SUCCESS == doCmdReceive( &canalEvent ) ) {     // Valid event
                
                //if ( 8 != canalEvent.sizeData ) continue;
				
                if ( (unsigned short)( canalEvent.id & 0xffff ) ==
					( ( VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE << 8 ) + nodeid ) ) {  // Read reply?
                        
                        fcnt++;

                        // Mark frame as received
                        receive_flags |= ( 1 << canalEvent.data[ 0 ] );

                        if ( lastpageCnt && (count - 1) == canalEvent.data[ 0 ] ) {
                            // Last frame
                            for ( int i=0;i<lastpageCnt; i++ ) {
                                data[ canalEvent.data[ 0 ]*4 + i] = canalEvent.data[ 4 + i ];
                            }
                        }
                        else {
                            for ( int i=0;i<4; i++ ) {
                                data[ canalEvent.data[ 0 ]*4 + i] = canalEvent.data[ 4 + i ];
                            }
                        }

				} // Check for correct reply event 
			}
		}
        else {
			wxMilliSleep( 2 );
        }  

        if ( ( ::wxGetLocalTimeMillis() - startTime ) > m_registerReadErrorTimeout ) {
            rv = false;
            goto error;
		}

    }

error:

    // Copy data on success
    if ( CANAL_ERROR_SUCCESS == rv ) {
        memcpy( pRegs, data, count );
    }

	return rv;

}