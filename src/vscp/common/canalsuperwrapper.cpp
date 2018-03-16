///////////////////////////////////////////////////////////////////////////////
// CanalSuperWrapper.cpp: implementation of the CCanalSuperWrapper class.
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifdef WIN32
#include <winsock2.h>
#endif

#include "wx/wx.h"
#include "wx/defs.h"
#if wxUSE_GUI != 0
#include <wx/progdlg.h>
#endif
#include <wx/stdpaths.h>

#include "vscp.h"
#include "canalsuperwrapper.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCanalSuperWrapper::CCanalSuperWrapper( void )
{
    init();

    m_itemDevice.id = USE_TCPIP_INTERFACE;
    m_itemDevice.strName = _("Locahost");
    m_itemDevice.strPath = _("TCPIP");
    m_itemDevice.strParameters = _("admin;secret;localhost;9598");
    m_itemDevice.flags = 0; 
    m_itemDevice.filter = 0;
    m_itemDevice.mask = 0;
    m_devid = 0;
}

CCanalSuperWrapper::CCanalSuperWrapper( devItem *pItem )
{	
    if ( NULL == pItem ) return;

    init();

    if ( pItem->strPath.Length() && pItem->strPath.IsSameAs(_("TCPIP") ) ) {

        // TCP/IP interface
        m_itemDevice.id = USE_TCPIP_INTERFACE;

        m_itemDevice.strName = pItem->strName;
        m_itemDevice.strPath = pItem->strPath;
        m_itemDevice.strParameters = pItem->strParameters;
        m_itemDevice.flags = pItem->flags;
        m_itemDevice.filter = pItem->filter;
        m_itemDevice.mask = pItem->mask;
    }
    else if ( NULL != pItem )  {

        // DLL/DL Interface
        m_itemDevice.id = USE_DLL_INTERFACE;

        if ( pItem->strName.Length() ) {
            m_itemDevice.strName = pItem->strName;
        }

        if ( pItem->strPath.Length() ) {
            m_itemDevice.strPath = pItem->strPath;
        }

        if ( pItem->strParameters.Length() ) {
            m_itemDevice.strParameters = pItem->strParameters;
        }	

        m_itemDevice.flags = pItem->flags;
        m_itemDevice.filter = pItem->filter;
        m_itemDevice.mask = pItem->mask;

        m_canalDll.initialize( m_itemDevice.strPath );

    }
    else {

        // pItem == NULL
        // We do some default

        // TCP/IP interface
        m_itemDevice.id = USE_TCPIP_INTERFACE;
        m_itemDevice.strName = _("Locahost");
        m_itemDevice.strPath = _("TCPIP");
        m_itemDevice.strParameters = _("admin;secret;localhost;9598");
        m_itemDevice.flags = 0;
        m_itemDevice.filter = 0;
        m_itemDevice.mask = 0;

    }

}

CCanalSuperWrapper::~CCanalSuperWrapper()
{
    doCmdClose();	
}

///////////////////////////////////////////////////////////////////////////////
// init
//  

void CCanalSuperWrapper::init( void )
{
    // Init. register read parameters
    m_registerReadErrorTimeout = SW_REGISTER_READ_ERROR_TIMEOUT;
    m_registerReadResendTimeout = SW_REGISTER_READ_RESEND_TIMEOUT;
    m_registerReadMaxRetries = SW_REGISTER_READ_MAX_TRIES;
}

///////////////////////////////////////////////////////////////////////////////
// setInterface
//  TCP/IP version

void CCanalSuperWrapper::setInterface( const wxString& host,
                                            const wxString& username,
                                            const wxString& password )
{
    // TCP/IP interface
    m_itemDevice.id = USE_TCPIP_INTERFACE;

    // No name
    m_itemDevice.strName.Empty();

    // No path
    m_itemDevice.strPath.Empty();

    // Build TCP/IP configuration string
    m_itemDevice.strParameters = host;
    m_itemDevice.strParameters += _(";");
    m_itemDevice.strParameters += username;
    m_itemDevice.strParameters += _(";"); 
    m_itemDevice.strParameters += password;
    m_itemDevice.strParameters += _(";");

    m_itemDevice.flags = 0;
    m_itemDevice.filter = 0;
    m_itemDevice.mask = 0;
}

///////////////////////////////////////////////////////////////////////////////
// setInterface - CANAL version
//  

void CCanalSuperWrapper::setInterface( const wxString& name,
    const wxString& path,
    const wxString& parameters,
    const unsigned long flags,
    const unsigned long filter,
    const unsigned long mask)
{
    // DLL/DL Interface
    m_itemDevice.id = USE_DLL_INTERFACE;

    m_itemDevice.strName = name;
    m_itemDevice.strPath = path;
    m_itemDevice.strParameters = parameters;

    m_itemDevice.flags = flags;
    m_itemDevice.filter = filter;
    m_itemDevice.mask = mask;

    m_canalDll.initialize( m_itemDevice.strPath );
}

///////////////////////////////////////////////////////////////////////////////
// open
//

long CCanalSuperWrapper::doCmdOpen( const wxString& strInterface, 
                                        unsigned long flags )
{
    long rv = false;

    if ( USE_DLL_INTERFACE == m_itemDevice.id ) {

        // *** Open dll/dl interface ***

        if ( strInterface.Length() ) {
            rv = m_canalDll.doCmdOpen( strInterface, flags );
        }
        else {
            rv = m_canalDll.doCmdOpen( m_itemDevice.strParameters, 
                m_itemDevice.flags );
        }   

    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {

        // *** Open remote TCP/IP interface *** 

        if ( strInterface.Length() ) {
            if ( VSCP_ERROR_SUCCESS ==  m_vscptcpif.doCmdOpen( strInterface, flags ) ) {
                rv = 1661; // Pretend to be driver 
            }
        }
        else {
            rv = m_vscptcpif.doCmdOpen( m_itemDevice.strParameters, m_itemDevice.flags );
            if ( VSCP_ERROR_SUCCESS == rv ) {
                // We try to get the interface GUID. If we
                // fail to get it we use the GUID assigned
                // in the constructor
                m_vscptcpif.doCmdGetGUID( m_GUID );

                rv = 1661; // pretend to be driver
            }

        }

    }

    m_devid = rv;

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// close
//

int CCanalSuperWrapper::doCmdClose( void )
{
    m_devid = 0;

    if ( USE_DLL_INTERFACE == m_itemDevice.id) {
        return m_canalDll.doCmdClose();
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        return m_vscptcpif.doCmdClose();
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdNOOP
//

int CCanalSuperWrapper::doCmdNOOP( void )
{
    if ( USE_DLL_INTERFACE == m_itemDevice.id) {
        return m_canalDll.doCmdNOOP();
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        return m_vscptcpif.doCmdNOOP();
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdClear
//

int CCanalSuperWrapper::doCmdClear( void )
{
    if ( USE_DLL_INTERFACE == m_itemDevice.id) {
        canalMsg *pMsg = NULL;
        while ( 0 < doCmdDataAvailable() ) {
            m_canalDll.doCmdReceive( pMsg );
            delete pMsg;
            pMsg = NULL;
        }
        return CANAL_RESPONSE_SUCCESS;
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        return m_vscptcpif.doCmdClear();
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetLevel
//

unsigned long CCanalSuperWrapper::doCmdGetLevel( void )
{ 
    if ( USE_DLL_INTERFACE == m_itemDevice.id) {
        return m_canalDll.doCmdGetLevel();
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        return CANAL_LEVEL_USES_TCPIP; 
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//

int CCanalSuperWrapper::doCmdSend( canalMsg *pMsg )
{	
    if ( USE_DLL_INTERFACE == m_itemDevice.id) {
        return m_canalDll.doCmdSend( pMsg );
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        return m_vscptcpif.doCmdSendLevel1( pMsg );
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//

int CCanalSuperWrapper::doCmdSend( const vscpEvent *pEvent )
{	
    canalMsg msg;

    if ( USE_DLL_INTERFACE == m_itemDevice.id ) {
        vscp_convertEventToCanal( &msg, pEvent );
        return doCmdSend( &msg );
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id ) {
        return m_vscptcpif.doCmdSend( pEvent );
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//

int CCanalSuperWrapper::doCmdSend( const vscpEventEx *pEventEx )
{	
    int rv;

    if ( USE_DLL_INTERFACE == m_itemDevice.id ) {
        vscpEvent *pevent = new vscpEvent;
        vscp_convertVSCPfromEx( pevent, pEventEx );
        rv = doCmdSend( pevent );
        vscp_deleteVSCPevent( pevent );
        return rv;
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id ) {
        return m_vscptcpif.doCmdSendEx( pEventEx );
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

int CCanalSuperWrapper::doCmdReceive( canalMsg *pMsg )
{		
    if ( USE_DLL_INTERFACE == m_itemDevice.id) {
        return m_canalDll.doCmdReceive( pMsg );
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        return m_vscptcpif.doCmdReceiveLevel1( pMsg );
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

int CCanalSuperWrapper::doCmdReceive( vscpEvent *pEvent )
{	
    int rv = 0;

    if ( USE_DLL_INTERFACE == m_itemDevice.id ) {
        canalMsg msg;
        rv = m_canalDll.doCmdReceive( &msg );
        uint8_t guid[16];
        memset( guid, 0, 16 );
        vscp_convertCanalToEvent( pEvent, &msg, guid );
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id ) {
        rv = m_vscptcpif.doCmdReceive( pEvent );
    }

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

int CCanalSuperWrapper::doCmdReceive( vscpEventEx *pEventEx )
{	
    int rv = 0;

    if ( USE_DLL_INTERFACE == m_itemDevice.id ) {
        canalMsg msg;
        rv =  m_canalDll.doCmdReceive( &msg );
        uint8_t guid[16];
        memset( guid, 0, 16 );
        vscp_convertCanalToEventEx( pEventEx, &msg, guid );
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id ) {
        rv =  m_vscptcpif.doCmdReceiveEx( pEventEx );
    }

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdDataAvailable
//

int CCanalSuperWrapper::doCmdDataAvailable( void )
{
    if ( USE_DLL_INTERFACE == m_itemDevice.id ) {
        return m_canalDll.doCmdDataAvailable();
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id ) {
        return m_vscptcpif.doCmdDataAvailable();
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdState
//

int CCanalSuperWrapper::doCmdStatus( canalStatus *pStatus )
{	
    if ( USE_DLL_INTERFACE == m_itemDevice.id) {
        return m_canalDll.doCmdStatus( pStatus );
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        return m_vscptcpif.doCmdStatus( pStatus );
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdStatistics
//

int CCanalSuperWrapper::doCmdStatistics( canalStatistics *pStatistics )
{	
    if ( USE_DLL_INTERFACE == m_itemDevice.id) {
        return m_canalDll.doCmdStatistics( pStatistics );
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        return m_vscptcpif.doCmdStatistics( pStatistics );
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdFilter
//

int CCanalSuperWrapper::doCmdFilter( unsigned long filter )
{	
    if ( USE_DLL_INTERFACE == m_itemDevice.id) {
        return m_canalDll.doCmdFilter( filter );
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        return CANAL_ERROR_NOT_SUPPORTED;
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdMask
//

int CCanalSuperWrapper::doCmdMask( unsigned long mask )
{	
    if ( USE_DLL_INTERFACE == m_itemDevice.id) {
        return m_canalDll.doCmdMask( mask );
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        return CANAL_ERROR_NOT_SUPPORTED;
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdVscpFilter
//

int CCanalSuperWrapper::doCmdVscpFilter( const vscpEventFilter *pFilter )
{
    if ( USE_DLL_INTERFACE == m_itemDevice.id) {
        return CANAL_ERROR_NOT_SUPPORTED;
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        return m_vscptcpif.doCmdFilter( pFilter );
    }

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdBaudrate

int CCanalSuperWrapper::doCmdBaudrate( unsigned long baudrate )
{
    if ( USE_DLL_INTERFACE == m_itemDevice.id) {
        return m_canalDll.doCmdBaudrate( baudrate );
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        return CANAL_ERROR_SUCCESS;
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdVersion
//

unsigned long CCanalSuperWrapper::doCmdVersion( void )
{
    if ( USE_DLL_INTERFACE == m_itemDevice.id) {
        return m_canalDll.doCmdVersion();
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        uint8_t v1=0,v2=0,v3=0;
        m_vscptcpif.doCmdVersion( &v2, &v2, &v3 );
        return ((v1<<16) + (v2<<8) + v3);
    }

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdDLLVersion
//

unsigned long CCanalSuperWrapper::doCmdDLLVersion( void )
{
    if ( USE_DLL_INTERFACE == m_itemDevice.id) {
        return m_canalDll.doCmdDLLVersion();
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        return m_vscptcpif.doCmdDLLVersion();
    }

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdVendorString
//

const char * CCanalSuperWrapper::doCmdVendorString( void )
{
    if ( USE_DLL_INTERFACE == m_itemDevice.id ) {
        return m_canalDll.doCmdVendorString();
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id ) {
        return m_vscptcpif.doCmdVendorString();
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdShutDown
//

int CCanalSuperWrapper::doCmdShutDown( void )
{
    if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
        return m_vscptcpif.doCmdShutDown();
    }

    return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetDriverInfo
//

const char * CCanalSuperWrapper::doCmdGetDriverInfo( void )
{
    if ( USE_DLL_INTERFACE == m_itemDevice.id ) {
        return m_canalDll.doCmdGetDriverInfo();
    }
    else if ( USE_TCPIP_INTERFACE == m_itemDevice.id ) {
        return m_vscptcpif.doCmdGetDriverInfo();
    }

    return NULL;
}




//****************************************************************************
//                  R e g i s t e r  r o u t i n e s
//****************************************************************************




// We don't want the graphical UI on apps. that don't use it 
#if ( wxUSE_GUI != 0 )


//////////////////////////////////////////////////////////////////////////////
// readLevel1Register
//

bool CCanalSuperWrapper::_readLevel1Register( uint8_t nodeid, 
                                                uint8_t reg, 
                                                uint8_t *pcontent,
                                                wxProgressDialog *pdlg )
{
    bool rv = true;
    uint32_t errors = 0;
    bool bResend;
    wxString strBuf;
    canalMsg canalEvent;

    // Check pointer
    if ( NULL == pcontent ) return false;

    canalEvent.flags = CANAL_IDFLAG_EXTENDED;
    canalEvent.obid = 0;
    canalEvent.id = 0x0900;             // CLASS1.PROTOCOL Read register
    canalEvent.sizeData = 2;
    canalEvent.data[ 0 ] = nodeid;      // Node to read from
    canalEvent.data[ 1 ] = reg;         // Register to read

    bResend = false;
    doCmdSend( &canalEvent );

    wxLongLong startTime = ::wxGetLocalTimeMillis();

    while ( true ) {

        if ( NULL != pdlg ) pdlg->Pulse();

        if ( 0 < doCmdDataAvailable() ) {                               // Message available
            if ( CANAL_ERROR_SUCCESS == doCmdReceive( &canalEvent ) ) {	// Valid event
                if ( (unsigned short)( canalEvent.id & 0xffff ) ==
                    ( 0x0a00 + nodeid ) ) {                             // Read reply?
                        if ( canalEvent.data[ 0 ] == reg ) {            // Requested register?

                            if ( NULL != pcontent ) {
                                *pcontent = canalEvent.data[ 1 ];
                            }
                            break;

                        }       // Check for correct node
                }               // Check for correct reply event 
            }
        }
        else {
            //wxMilliSleep( 1 );
            ::wxSafeYield();
        }

        // Check for read error timeout
        if ( ( ::wxGetLocalTimeMillis() - startTime ) > m_registerReadErrorTimeout ) {
            errors++;
        }
        // Should we resend?
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
// writeLevel1Register
//

bool CCanalSuperWrapper::_writeLevel1Register( uint8_t nodeid, 
                                                    uint8_t reg, 
                                                    uint8_t *pval,
                                                    wxProgressDialog *pdlg )
{
    bool rv = true;
    uint32_t errors = 0;
    bool bResend;
    wxString strBuf;
    canalMsg canalEvent;

    canalEvent.flags = CANAL_IDFLAG_EXTENDED;
    canalEvent.obid = 0;
    canalEvent.id = 0x0B00;             // CLASS1.PROTOCOL Write register
    canalEvent.sizeData = 3;
    canalEvent.data[ 0 ] = nodeid;      // Node to read from
    canalEvent.data[ 1 ] = reg;         // Register to write
    canalEvent.data[ 2 ] = *pval;       // value to write

    bResend = false;
    doCmdSend( &canalEvent );

    wxLongLong startTime = ::wxGetLocalTimeMillis();

    while ( true ) {

        if ( NULL != pdlg ) pdlg->Pulse();

        if ( 0 < doCmdDataAvailable() ) {                               // Message available
            if ( CANAL_ERROR_SUCCESS == doCmdReceive( &canalEvent ) ) { // Valid event
                if ( (unsigned short)( canalEvent.id & 0xffff ) ==
                    ( 0x0a00 + nodeid ) ) {         // Read reply?
                        if ( canalEvent.data[ 0 ] == reg ) {            // Requested register?

                            if ( *pval != canalEvent.data[ 1 ] ) rv = false;
                            // Save read value
                            *pval = canalEvent.data[ 1 ];
                            break;
                        } // Check for correct node

                        // Save read value
                        *pval = canalEvent.data[ 1 ];

                } // Check for correct reply event 
            }
        }
        else {
            //wxMilliSleep( 1 );
            ::wxSafeYield();
        }

        if ( ( ::wxGetLocalTimeMillis() - startTime ) > m_registerReadErrorTimeout ) {
                errors++;
        }
        else if ( ( ::wxGetLocalTimeMillis() - startTime ) >  m_registerReadResendTimeout ) {
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

bool CCanalSuperWrapper::readLevel1Registers( wxWindow *pwnd,
                                                uint8_t *pregisters,
                                                uint8_t nodeid,
                                                uint8_t startreg,
                                                uint16_t count,
                                                wxProgressDialog *pdlg )
{
    int i;
    uint8_t val;
    bool rv = true;
    int errors = 0;
    wxString strBuf;

    wxProgressDialog progressDlg( _("VSCP Works"),
        _("Reading Registers"),
        count, 
        pwnd,
        wxPD_ELAPSED_TIME | 
        wxPD_AUTO_HIDE | 
        wxPD_APP_MODAL | 
        wxPD_CAN_ABORT );

    progressDlg.Pulse( _("Reading registers!") );


    // *********************
    // Read register content
    // *********************
    for ( i = startreg; i < (startreg + count); i++ ) {

        if ( NULL != pdlg ) pdlg->Pulse();

        if ( !progressDlg.Update( i-startreg ) ) {
            rv = false;
            break;   // User aborted
        }

        progressDlg.Pulse( wxString::Format(_("Reading register %d"), i) );

        if ( _readLevel1Register( nodeid, i, &val ) ) {
            pregisters[ i-startreg ] = val;
        }
        else {
            errors++;
        }

        if ( errors > 2 ) {
            wxMessageBox( _("No node present or problems when communicating with node. Aborting!") );
            rv = false;
            break;
        }

    } // for

    return rv;
}

//////////////////////////////////////////////////////////////////////////////
// readLevel2Register
//

bool CCanalSuperWrapper::readLevel2Register( cguid& ifGUID, 
                                                uint32_t reg, 
                                                uint8_t *pcontent,
                                                cguid *pdestGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2 )
{
    bool rv = true;
    uint32_t errors = 0;
    bool bResend;
    wxString strBuf;
    vscpEventEx e;

    // Check pointers
    if ( NULL == pcontent ) return false; 
    if (NULL == pdestGUID ) return false;
    
    e.head = VSCP_PRIORITY_NORMAL;
    e.timestamp = 0;
    e.obid = 0;

    // Check if a specific interface is used
    if ( !ifGUID.isNULL() ) {

        // Event should be sent to a specific interface
        e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
        e.vscp_type = VSCP_TYPE_PROTOCOL_READ_REGISTER;

        memset( e.GUID, 0, 16 );            // We use GUID for interface 
        e.sizeData = 16 + 2;                // Interface GUID + nodeid + register to read

        ifGUID.writeGUID( e.data );

        e.data[ 16 ] = pdestGUID->getLSB(); // nodeid
        e.data[ 17 ] = reg;                 // Register to read

    }
    else {

        // Event should be sent to all interfaces

        if ( bLevel2 ) {

            // True real Level II event

            e.head = VSCP_PRIORITY_NORMAL;
            e.vscp_class = VSCP_CLASS2_PROTOCOL;
            e.vscp_type = VSCP2_TYPE_PROTOCOL_READ_REGISTER;

            memset( e.GUID, 0, 16 );                // We use GUID for interface 

            e.sizeData = 22;                        // nodeid + register to read

            pdestGUID->writeGUID( e.data );	        // Destination GUID
            e.data[ 16 ] = ( reg >> 24 ) & 0xff;    // Register to read
            e.data[ 17 ] = ( reg >> 16 ) & 0xff;
            e.data[ 18 ] = ( reg >> 8 ) & 0xff;
            e.data[ 19 ] = reg & 0xff;
            e.data[ 20 ] = 0x00;                    // Read one register
            e.data[ 21 ] = 0x01;

        }
        else {

            // Level I over CLASS2 to all interfaces 
            e.head = VSCP_PRIORITY_NORMAL;
            e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
            e.vscp_type = VSCP_TYPE_PROTOCOL_READ_REGISTER;

            memset( e.GUID, 0, 16 );            // We use GUID for interface 
            e.sizeData = 16 + 2;                // nodeid + register to read
            pdestGUID->writeGUID( e.data );     // Destination GUID

            e.data[16] = pdestGUID->getLSB();   // nodeid
            e.data[17] = reg;                   // Register to read

        }
    }

    bResend = false;

    // Send the event
    doCmdClear();
    e.timestamp = 0;
    doCmdSend( &e );

    wxLongLong resendTime = ::wxGetLocalTimeMillis();

    while ( true ) {
        
        if ( NULL != pdlg ) pdlg->Pulse();

        if ( 0 < doCmdDataAvailable() ) {                       // Message available

            if ( CANAL_ERROR_SUCCESS == doCmdReceive( &e ) ) {  // Valid event

                // Check for correct reply event
                {
                    wxString str;
                    str = wxString::Format(_("Received Event: class=%d type=%d size=%d data=%d %d"), 
                            e.vscp_class, e.vscp_type, e.sizeData, e.data[16], e.data[17] );
                    wxLogDebug(str);
                }

                // Level I Read reply?
                if ( /*ifGUID.isNULL() &&*/ ( VSCP_CLASS1_PROTOCOL == e.vscp_class ) && 
                    ( VSCP_TYPE_PROTOCOL_RW_RESPONSE == e.vscp_type ) ) {   
                    
                    if ( e.data[ 0 ] == reg ) {	// Requested register?
                        
                        if ( pdestGUID->isSameGUID( e.GUID ) ) { // From correct node?
                            if ( NULL != pcontent ) {
                                *pcontent = e.data[ 1 ];
                            }
                            break;
                        }
                        
                    } // Check for correct node
                }
                // Level II 512 Read reply?
                else if ( !ifGUID.isNULL() && !bLevel2 && 
                    ( VSCP_CLASS2_LEVEL1_PROTOCOL == e.vscp_class ) && 
                    ( VSCP_TYPE_PROTOCOL_RW_RESPONSE == e.vscp_type ) ) { 

                    if ( pdestGUID->isSameGUID( e.GUID ) ) {
                        
                        // Is this the register we requested?
                        if ( e.data[ 16 ] == reg ) {
                            // OK get the data
                            if ( NULL != pcontent ) {
                                *pcontent = e.data[ 17 ];
                                break;
                            }
                        }
                    }
                
                }
                // Level II Read reply?
                else if ( ifGUID.isNULL() && bLevel2 && 
                    ( VSCP_CLASS2_PROTOCOL == e.vscp_class ) && 
                    ( VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE == e.vscp_type ) ) { 

                    // from us
                    if ( pdestGUID->isSameGUID( e.GUID ) ) {	

                        uint32_t retreg = ( e.data[ 0 ]  << 24 ) +
                                (	e.data[ 1 ]  << 16 ) +
                                (	e.data[ 2 ]  << 8 ) +
                                e.data[ 3 ];

                        // Reg we requested?
                        if ( retreg == reg ) {
                                
                            // OK get the data
                            if ( NULL != pcontent ) {
                                *pcontent = e.data[ 4 ];
                                break;
                            }
                        }
                    }
                }

            } // valid event
            
            
        }

        if ( ( ::wxGetLocalTimeMillis() - resendTime ) > 
            m_registerReadResendTimeout ) {
                
                errors++;

                // Send again
                e.timestamp = 0;
                wxLongLong resendTime = ::wxGetLocalTimeMillis();
                doCmdSend( &e );

        }   

        if ( errors > m_registerReadMaxRetries ) {
            rv = false;
            break;
        }
        
        wxMilliSleep( 10 );
        //::wxSafeYield();

    } // while

    return rv;
}


//////////////////////////////////////////////////////////////////////////////
// writeLevel2Register
//

bool CCanalSuperWrapper::_writeLevel2Register( cguid& ifGUID, 
                                                uint32_t reg, 
                                                uint8_t *pcontent,
                                                cguid& destGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2 )
{
    bool rv = true;
    bool bInterface = false;  // No specific interface set
    uint32_t errors = 0;
    bool bResend;
    wxString strBuf;
    vscpEventEx event;

    // Check pointers
    if ( NULL == pcontent ) return false;

    if ( !ifGUID.isNULL() ) {

        event.head = VSCP_PRIORITY_NORMAL;
        event.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
        event.vscp_type = VSCP_TYPE_PROTOCOL_WRITE_REGISTER;

        memset( event.GUID, 0, 16 );        // We use interface GUID

        event.sizeData = 16 + 3;            // Interface GUID + nodeid + register to read + valied

        destGUID.writeGUID( event.data );   // Destination GUID
        event.data[16] = destGUID.getLSB(); // nodeid
        event.data[17] = reg;               // Register to write
        event.data[18] = *pcontent;         // value to write

    }
    else {

        if ( bLevel2 ) {

            event.head = VSCP_PRIORITY_NORMAL;
            event.vscp_class = VSCP_CLASS2_PROTOCOL;
            event.vscp_type = VSCP2_TYPE_PROTOCOL_WRITE_REGISTER;

            memset( event.GUID, 0, 16 );        // We use interface GUID

            event.sizeData = 21;                // nodeid + register to read

            destGUID.writeGUID( event.data );   // Destination GUID

            event.data[ 16 ] = ( reg >> 24 ) & 0xff;    // Register to write
            event.data[ 17 ] = ( reg >> 16 ) & 0xff;
            event.data[ 18 ] = ( reg >> 8 ) & 0xff;
            event.data[ 19 ] = reg & 0xff;
            event.data[ 20 ] = *pcontent;       // Data to write

        }
        else {

            // Level I over CLASS2 to all interfaces 

            event.head = VSCP_PRIORITY_NORMAL;
            event.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
            event.vscp_type = VSCP_TYPE_PROTOCOL_WRITE_REGISTER;

            memset( event.GUID, 0, 16 );        // We use interface GUID

            event.sizeData = 16 + 3;

            destGUID.writeGUID( event.data );   // Destination GUID

            event.data[16] = destGUID.getLSB();	// nodeid
            event.data[17] = reg;               // Register to write
            event.data[18] = *pcontent;	        // value to write

        }

    }

    bResend = false;
    
    // Send the event
    doCmdSend( &event );

    wxLongLong startTime = ::wxGetLocalTimeMillis();

    while ( true ) {

        if ( NULL != pdlg ) pdlg->Pulse();

        if ( 0 < doCmdDataAvailable() ) {                             // Message available
            if ( CANAL_ERROR_SUCCESS == doCmdReceive( &event ) ) {    // Valid event
                
                if ( /*ifGUID.isNULL() &&*/ ( VSCP_CLASS1_PROTOCOL == event.vscp_class ) && 
                    ( VSCP_TYPE_PROTOCOL_RW_RESPONSE == event.vscp_type ) ) {   // Read reply?
                    if ( event.data[ 0 ] == reg ) {	// Requested register?
                        
                        if ( destGUID.isSameGUID( event.GUID ) ) { // From correct node? Correct node?

                            // We go a rw reply from the correct node is
                            // the written data same as we expect.
                            if ( *pcontent != event.data[ 1 ] ) rv = false;
                            break;

                        }
                    }   
                } 

                // Level II 512 Read reply?
                else if ( !ifGUID.isNULL() && !bLevel2 && 
                    ( VSCP_CLASS2_LEVEL1_PROTOCOL == event.vscp_class ) && 
                    ( VSCP_TYPE_PROTOCOL_RW_RESPONSE == event.vscp_type ) ) { 

                    //if ( destGUID.isSameGUID( event.GUID ) ) {
                    if (destGUID.getLSB() == event.data[15] ) {
                        // Reg we requested?
                        if ( event.data[ 16 ] == reg ) {
                            // OK get the data
                            if ( NULL != pcontent ) {
                                // We go a rw reply from the correct node is
                                // the written data same as we expect.
                                if ( *pcontent != event.data[ 17 ] ) rv = false;
                                break;
                            }
                        }
                    }

                }
                // Level II Read reply?
                else if ( !bInterface && bLevel2 && 
                    ( VSCP_CLASS2_PROTOCOL == event.vscp_class ) && 
                    ( VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE == event.vscp_type ) ) { 

                    // from us
                    if ( destGUID.isSameGUID( event.GUID ) ) {	

                        uint32_t retreg = ( event.data[ 0 ]  << 24 ) +
                                (	event.data[ 1 ]  << 16 ) +
                                (	event.data[ 2 ]  << 8 ) +
                                event.data[ 3 ];

                        // Reg we requested?
                        if ( retreg == reg ) {
                                
                            // OK get the data
                            if ( NULL != pcontent ) {
                                // We go a rw reply from the correct node is
                                // the written data same as we expect.
                                if ( *pcontent != event.data[ 4 ] ) rv = false;
                                break;
                            }
                        }
                    }
                }
            }
        }
        else {
            //wxMilliSleep( 1 );
            ::wxSafeYield();
        }

        if ( ( ::wxGetLocalTimeMillis() - startTime ) > m_registerReadErrorTimeout ) {
                errors++;
        }
        else if ( ( ::wxGetLocalTimeMillis() - startTime ) > m_registerReadResendTimeout ) {
                // Send again
                if ( !bResend) {
                    doCmdSend( &event );
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
// readLevel2Registers
//

bool CCanalSuperWrapper::readLevel2Registers( wxWindow *pwnd,
                                                uint8_t *pregisters,
                                                cguid& ifGUID,
                                                uint32_t startreg,
                                                uint32_t count,
                                                cguid *pdestGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2 )
{
    unsigned char val;
    bool rv = true;
    int errors = 0;
    wxString strBuf;

    wxProgressDialog progressDlg( _("VSCP Works"),
                                    _("Reading Registers"),
                                    count, 
                                    pwnd,
                                    wxPD_ELAPSED_TIME | 
                                    wxPD_AUTO_HIDE | 
                                    wxPD_APP_MODAL |
                                    wxPD_CAN_ABORT );

    progressDlg.Pulse( _("Reading registers") );

    // *********************
    // Read register content
    // *********************
    for ( unsigned int i=startreg; i<( startreg + count ); i++ ) {

        if ( NULL != pdlg ) pdlg->Pulse();

        if ( !progressDlg.Update( i-startreg ) ) {
            rv = false;
            break;
        }

        progressDlg.Pulse( wxString::Format(_("Reading register %04X:%02X"), 0, i) );

        if ( readLevel2Register( ifGUID, 
                                    i, 
                                    &val,
                                    pdestGUID,
                                    &progressDlg,
                                    bLevel2 ) ) {
            pregisters[ i - startreg ] = val;
        }
        else {
            wxMessageBox( _("No node present or problems when communicating with node. Aborting!") );
            rv = false;
            break;
        }


    } // for


    return rv;
}


//////////////////////////////////////////////////////////////////////////////
// getMDFfromLevel1Device
//

wxString CCanalSuperWrapper::getMDFfromLevel1Device( uint8_t id, bool bSilent )
{
    wxString strWrk;
    char url[ 33 ];
    bool rv = true;

    uint8_t *p = (uint8_t *)url;
    for ( int i=0; i<32; i++ ) {
        if ( !getDllInterface()->readLevel1Register( 0,
                                                        id, 
                                                        0xE0 + i, 
                                                        p++ ) ) {
                if ( !bSilent ) {
                    ::wxMessageBox( _("Unable to read register."), _("VSCP Works"), wxICON_ERROR );
                }
                rv = false;
                goto error;
        }
    }

    strWrk = strWrk.From8BitData( url );
    if ( wxNOT_FOUND == strWrk.Find( _("http://") ) ) {
        wxString str;
        str = _("http://");
        str += strWrk;
        strWrk = str;
    }

error:

    if ( !rv ) strWrk.Empty();

    return strWrk;
}

//////////////////////////////////////////////////////////////////////////////
// getMDFfromDevice2
//

wxString CCanalSuperWrapper::getMDFfromLevel2Device( wxProgressDialog& progressDlg,
                                                        cguid& ifGUID,
                                                        cguid& destGUID, 
                                                        bool bLevel2, 
                                                        bool bSilent )
{
    wxString strWrk;
    char url[ 33 ];
    bool rv = true;

    memset( url, 0, sizeof( url ) );

    if ( bLevel2 ) {

        progressDlg.Pulse( _("Reading MDF URL from Level II device.") );

        // Level 2 device
        uint8_t *p = (uint8_t *)url;
        for ( int i=0; i<32; i++ ) {

            if ( !progressDlg.Pulse( wxString::Format(_("Reading MDF register %08X"), 0xe0 + i ) ) ) {
                break;
            }

            if ( !readLevel2Register( ifGUID, 
                                        0xFFFFFFE0 + i, 
                                        p++,
                                        &destGUID,
                                        &progressDlg,
                                        true ) ) {
                    if ( !bSilent ) {
                        ::wxMessageBox( _("Unable to read register."), _("VSCP Works"), wxICON_ERROR );
                    }
                    rv = false;
                    goto error;
            }

        }

    }
    else {

        progressDlg.Pulse( _("Reading MDF URL from Level I device over Server.") );

        // Level 1 device
        uint8_t *p = (uint8_t *)url;
        for ( int i=0; i<32; i++ ) {

            if ( !progressDlg.Pulse( wxString::Format(_("Reading MDF register %02X"), 0xe0 +i ) ) ) {
                break;
            }

            if ( !readLevel2Register( ifGUID, 
                                        0xE0 + i, 
                                        p++,
                                        &destGUID ) ) {
                    if ( !bSilent ) {
                        ::wxMessageBox( _("Unable to read register."), _("VSCP Works"), wxICON_ERROR );
                    }
                    rv = false;
                    goto error;
            }

        }

    }

    strWrk = strWrk.From8BitData( url );
    if ( wxNOT_FOUND == strWrk.Find( _("http://") ) ) {
        wxString str;
        str = _("http://");
        str += strWrk;
        strWrk = str;
    }

error:

    if ( !rv ) strWrk.Empty();

    return strWrk;
}


//////////////////////////////////////////////////////////////////////////////
// loadMDF
//

bool CCanalSuperWrapper::loadMDF( wxWindow *pwnd,
                                    uint8_t *preg_url,
                                    wxString& url,
                                    CMDF *pmdf )
{
    bool rv = true;
    //wxStandardPaths stdpaths;
    wxString remoteFile;
    //uint8_t mdf_url[33];

    // Check pointers
    if ( NULL == pmdf ) return false;
    //if ( NULL == pmdf_url ) return false;

    // If length of url is zero registers should be read
    // from device to get mdf url
    if ( NULL != preg_url ) {

        remoteFile = _("http://") + wxString::From8BitData( (const char *)preg_url );
        //wxString remoteFile = _("http://www.grodansparadis.com/smart2_001.mdf");

    }
    else {
        remoteFile = url;
    }

    wxString localFile;

    wxProgressDialog progressDlg(_("VSCP Module Description File"),
            _("Load and parse MDF"),
            100,
            pwnd,
            wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE | wxPD_APP_MODAL);
    

    wxDateTime now = wxDateTime::Now();

    if ( wxNOT_FOUND == remoteFile.Find( _("://") ) ) {

        // If no UI no way to ask
        if (NULL != pwnd)  return false;
        
        localFile = remoteFile;

        // Load MDF from local file
        wxFileDialog dlg( pwnd,
                            _("Choose file to load MDF from "),
                            wxStandardPaths::Get().GetUserDataDir(),
                            _(""),
                            _("MSF Files (*.mdf)|*.mdf|XML Files (*.xml)|*.xml|All files (*.*)|*.*") );
        if ( wxID_OK == dlg.ShowModal() ) {
            localFile = dlg.GetPath();
        }
        else {
            return false;
        }
    }
    else {
        // Download the MDF
        progressDlg.Update( 30, _("Download MDF.") );

        if ( !pmdf->downLoadMDF( remoteFile, localFile ) ) {
            wxMessageBox( _("Failed to download MDF.") );
            progressDlg.Update( 100 );
            return false;
        }
    }

    progressDlg.Update( 60, _("Parsing MDF.") );

    pmdf->clearStorage();

    if ( !pmdf->parseMDF( localFile ) ) {
        wxMessageBox( _("Failed to parse MDF.") );
        progressDlg.Update( 100 );
        return false;
    }

    return rv;
}


//////////////////////////////////////////////////////////////////////////////
// getLevel1DmInfo
//

bool CCanalSuperWrapper::getLevel1DmInfo( const uint8_t nodeid, 
                                            uint8_t *pdata )
{
    bool rv = true;
    bool bResend;
    wxString strBuf;
    canalMsg canalEvent;

    // Check pointer
    if ( NULL == pdata ) return false;

    canalEvent.flags = CANAL_IDFLAG_EXTENDED;
    canalEvent.obid = 0;
    canalEvent.id = 0x2000;             // CLASS1.PROTOCOL Get decision matrix info
    canalEvent.sizeData = 1;
    canalEvent.data[ 0 ] = nodeid;      // Node to read from

    bResend = false;
    doCmdSend( &canalEvent );

    wxDateTime start = wxDateTime::Now();

    while ( true ) {

        if ( 0 < doCmdDataAvailable() ) {                       // Message available
            if ( doCmdReceive( &canalEvent ) ) {                // Valid message
                if ( (unsigned short)( canalEvent.id & 0xffff ) ==
                    ( 0x2100 + nodeid ) ) {                     // DM info reply?
                        // Copy in response data
                        memcpy( pdata, canalEvent.data, 8 );
                        break;
                }
            }
        }

        if ( (wxDateTime::Now() - start).GetSeconds() > 2 ) {
            rv = false;
            break;
        }
        else if ( (wxDateTime::Now() - start).GetSeconds() > 1 ) {
            // Send again
            if ( !bResend) {
                doCmdSend( &canalEvent );
            }
            bResend = true;
        }
    } // while

    return rv;
}


//////////////////////////////////////////////////////////////////////////////
// getLevel2DmInfo
//

bool CCanalSuperWrapper::getLevel2DmInfo( cguid& ifGUID, 
                                            uint8_t *pdata,
    bool bLevel2 )
{
    bool rv = true;
    bool bResend;
    wxString strBuf;
    vscpEventEx event;

    // Check pointer
    if ( NULL == pdata ) return false;

    event.head = 0;
    event.vscp_class = 512;             // CLASS2.PROTOCOL1
    event.vscp_type = 9;                // Get decision matrix info
    memset( event.GUID, 0, 16 );        // We use interface GUID
    event.sizeData = 16 + 1;            // Interface GUID + nodeid 
    ifGUID.writeGUID( event.data );     // Address node
    event.data[16] = ifGUID.getLSB();   // nodeid

    bResend = false;
    doCmdSend( &event );

    wxDateTime start = wxDateTime::Now();

    while ( true ) {

        if ( 0 < doCmdDataAvailable() ) {                           // Message available
            if ( CANAL_ERROR_SUCCESS == doCmdReceive( &event ) ) {  // Valid event
                if ( ( 0 == event.vscp_class ) && 
                    ( 0x21 == event.vscp_type ) ) {                 // DM reply?
                        memcpy( pdata, event.data, 8 );
                        break;
                }
            }
        }

        if ( (wxDateTime::Now() - start).GetSeconds() > 2 ) {
            rv = false;
            break;
        }
        else if ( (wxDateTime::Now() - start).GetSeconds() > 1 ) {
            // Send again
            if ( !bResend) {
                doCmdSend( &event );
            }
            bResend = true;
        }
    } // while

    return rv;
}


//////////////////////////////////////////////////////////////////////////////
// setRegisterPage
//

bool CCanalSuperWrapper::setRegisterPage( uint8_t nodeid, 
                                            uint16_t page, 
                                            cguid *pifGUID,
                                            cguid *pdestGUID,
                                            bool bLevel2 )
{
    uint8_t val;

    if ( NULL == pdestGUID || pifGUID->isNULL() ) {

        val = ( page >> 8 ) & 0xff;
        if ( !_writeLevel1Register( nodeid, 
                                    0x92, 
                                    &val ) ) {
                return false;
        }

        val = page & 0xff;
        if ( !_writeLevel1Register( nodeid, 
                                    0x93, 
                                    &val ) ) {
                return false;
        }

    }
    else {

        val = ( page >> 8 ) & 0xff;
        if ( _writeLevel2Register( *pifGUID, 
                                    bLevel2 ? 0xffffff92 : 0x92, 
                                    &val,
                                    *pdestGUID ) ) {
                return false;
        }

        val = page & 0xff;
        if ( _writeLevel2Register( *pifGUID, 
                                    bLevel2 ? 0xffffff93 : 0x93, 
                                    &val,
                                    *pdestGUID ) ) {
                return false;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// getRegisterPage
//

uint32_t CCanalSuperWrapper::getRegisterPage( wxWindow *pwnd, 
                                                uint8_t nodeid, 
                                                cguid *pifGUID,
                                                cguid *pdestGUID,
                                                bool bLevel2 )
{
    uint32_t page = 0;

    if ( NULL == pdestGUID || pifGUID->isNULL() ) {

        uint8_t regs[ 2 ];
        if ( readLevel1Registers( pwnd,
                                    regs,
                                    nodeid,
                                    0x92,
                                    2 ) ) {
                page = ( regs[ 0 ] << 8 ) + regs[ 1 ];
        }
        else {
            return false;
        }

    }
    else {
        uint8_t regs[ 2 ];
        if ( readLevel2Registers( pwnd,
                                    regs,
                                    *pifGUID,
                                    bLevel2 ? 0xffffff92 : 0x92,
                                    2,
                                    pdestGUID, 
                                    NULL,
                                    bLevel2 ) ) {
            page = ( regs[ 0 ] << 8 ) + regs[ 1 ];
        }
        else {
            return false;
        }

    }

    return page;

}


///////////////////////////////////////////////////////////////////////////////
//  getDMRow
// 

bool CCanalSuperWrapper::getDMRow( wxWindow *pwnd,
                                    uint8_t nodeid, 
                                    CMDF_DecisionMatrix *pdm, 
                                    uint32_t row, 
                                    uint8_t *pRow,
                                    cguid *pifGUID,
                                    cguid *pdestGUID,
                                    bool bLevel2,
                                    bool bSilent )
{
    bool rv = true;
    uint8_t val;
    uint8_t *p = pRow;
    uint16_t savepage;

    // Can't load row that is larger than the availabel rows
    if ( row >= pdm->m_nRowCount ) return false;

    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bSilent );
        if ( savepage != pdm->m_nStartPage ) {
            if ( !setRegisterPage( nodeid, pdm->m_nStartPage, pifGUID, pdestGUID, bSilent ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for decision matrix!") );
                return false;
            }
        }
    }

    if ( pdm->m_bIndexed ) {

        for ( int i=0; i<pdm->m_nRowSize; i++ ) {

            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {
            
                // Set index
                val = pdm->m_nStartOffset + row * pdm->m_nRowSize + i; // Indexed pos		
                if ( !_writeLevel1Register( nodeid, pdm->m_nStartOffset, &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to set register index for decision matrix!") );
                    rv = false;
                    goto error;
                }

                // Read DM byte
                if ( !_readLevel1Register( nodeid, pdm->m_nStartOffset + 1, p++ ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to read register in indexed decision matrix") );
                    rv = false;
                    goto error;
                }

            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            pdm->m_nStartOffset, 
                                            &val,
                                            *pdestGUID ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                if ( !readLevel2Register( *pifGUID, 
                                            pdm->m_nStartOffset + 1 , 
                                            p++,
                                            pdestGUID ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }	

            }
            
        } // for
    } // Indexed
    else {

        if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

            // Read row
            if ( !readLevel1Registers( pwnd, 
                                        pRow, 
                                        nodeid, 
                                        pdm->m_nStartOffset + row * pdm->m_nRowSize, 
                                        pdm->m_nRowSize ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read decision matrix row!") );
                rv = false;
                goto error;
            }

        }
        else {

            // Level II.
            if ( !readLevel2Registers( pwnd,
                                        p,
                                        *pifGUID,
                                        pdm->m_nStartOffset + row * pdm->m_nRowSize,
                                        pdm->m_nRowSize,
                                        pdestGUID,
                                        NULL,
                                        bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }
    }

error:

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != pdm->m_nStartPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bSilent ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for decision matrix!") );
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionString
// 

bool CCanalSuperWrapper::getAbstractionString( wxWindow *pwnd,
                                                uint8_t nodeid,
                                                CMDF_Abstraction *abstraction,
                                                wxString& retstr, 
                                                cguid *pifGUID,
                                                cguid *pdestGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2,
                                                bool bSilent )
{
    bool rv = true;
    wxString str;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction string!") );
                return false;
            }
        }
    }

    uint8_t *p;
    p = new uint8_t[ abstraction->m_nWidth + 1 ];
    memset( p, 0, abstraction->m_nWidth + 1 );

    if ( abstraction->m_bIndexed ) {

        for ( uint8_t i=0; i<abstraction->m_nWidth; i++ ) {

            uint8_t val = i;
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                // Write index to string
                
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset, 
                                            &val ) ) {
                        if ( !bSilent ) wxMessageBox( _("Failed to write abstraction string index!") );
                        break;
                }

                // Read value
                if ( !_readLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            (p+i) ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction string value!") );
                    break;
                }

            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &val,
                                            *pdestGUID ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                if ( !readLevel2Register( *pifGUID,
                                            abstraction->m_nOffset + 1,
                                            (p+i),
                                            pdestGUID, 
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read abstraction data!") );
                    rv = false;
                    goto error;
                }	

            }
        }
    }
    else {

        // Read string from linear storage.
        if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

            if ( !readLevel1Registers( pwnd, 
                                        p, 
                                        nodeid, 
                                        abstraction->m_nOffset, 
                                        abstraction->m_nWidth ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
            }

        }
        else {

            // Level II.
            if ( !readLevel2Registers( pwnd,
                                        p,
                                        *pifGUID,
                                        abstraction->m_nOffset,
                                        2,
                                        pdestGUID, 
                                        pdlg,
                                        bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }

    }

    retstr.From8BitData( (const char *)p );

error:

    if ( NULL != p ) delete [] p;

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction string!") );
                return false;
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstractionString
//

bool CCanalSuperWrapper::writeAbstractionString( wxWindow *pwnd,
                                                    uint8_t nodeid,
                                                    CMDF_Abstraction *abstraction,
                                                    wxString& strvalue,
                                                    cguid *pifGUID,
                                                    cguid *pdestGUID,
                                                    wxProgressDialog *pdlg,
                                                    bool bLevel2,
                                                    bool bSilent )
{
    bool rv = true;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    uint8_t *p;
    p = new uint8_t[ abstraction->m_nWidth + 1 ];
    memset( p, 0, abstraction->m_nWidth + 1 );
    strcpy( (char *)p, (const char*)strvalue.mb_str( wxConvUTF8 ) );

    // Save page
    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction string!") );
                return false;
            }
        }
    }


    if ( abstraction->m_bIndexed ) {
        
        for ( uint8_t i=0; i<abstraction->m_nWidth; i++ ) {

            
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                // Write index
                uint8_t val = i;
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction string index!") );
                    rv = false;
                    goto error;;
                }

                // Read value
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            (p+i) ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction string value!") );
                    rv = false;
                    goto error;
                }
            }
            else {
            
                // Write index Level II
                uint8_t idx = i;
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &idx,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                // Write data Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset + i, 
                                            (p+i),
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }	

            }

        }

    }
    else {

        for ( uint8_t i=0; i<abstraction->m_nWidth; i++ ) {

            // Write string to linear storage.
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + i, 
                                            p++ ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction string!") );
                    rv = false;
                    goto error;
                }
            }
            else {
    
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            p++,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

            }

        }

    }

error:

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction string!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionBitField
// 

bool CCanalSuperWrapper::getAbstractionBitField( wxWindow *pwnd,
                                                    uint8_t nodeid,
                                                    CMDF_Abstraction *abstraction,
                                                    wxString& retstr, 
                                                    cguid *pifGUID,
                                                    cguid *pdestGUID,
                                                    wxProgressDialog *pdlg,
                                                    bool bLevel2,
                                                    bool bSilent )
{
    bool rv = true;
    wxString strvalue;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction BitField!") );
                return false;
            }
        }
    }

    // Octet width is the number of bytes needed to store the bits
    uint8_t octetwidth = abstraction->m_nWidth / 8 + 
                            ( ( abstraction->m_nWidth % 8 ) ? 1 : 0 );

    uint8_t *p;
    p = new uint8_t[ octetwidth ];
    memset( p, 0, octetwidth );

    if ( abstraction->m_bIndexed ) {

        for ( uint8_t i=0; i<octetwidth; i++ ) {

            uint8_t val = i;
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                // Write index
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction BitField index!") );
                    rv = false;
                    goto error;
                }

                // Read value
                if ( !_readLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            (p+i) ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction BitField value!") );
                    rv = false;
                    goto error;
                }
            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &val,
                                            *pdestGUID ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                if ( !readLevel2Register( *pifGUID,
                                            abstraction->m_nOffset + 1,
                                            (p+i),
                                            pdestGUID, 
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read abstraction data!") );
                    rv = false;
                    goto error;
                }	

            }
        }
    }
    else {

        // Read string from linear storage.
        if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

            if ( !readLevel1Registers( pwnd, 
                                        p, 
                                        nodeid, 
                                        abstraction->m_nOffset, 
                                        octetwidth ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction BitField!") );
                rv = false;
                goto error;
            }

        }
        else {

            // Level II.
            if ( !readLevel2Registers( pwnd,
                                        p,
                                        *pifGUID,
                                        abstraction->m_nOffset,
                                        2,
                                        pdestGUID, 
                                        pdlg,
                                        bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }

    }

    for ( int i=0; i<abstraction->m_nWidth; i++ ) {
        for ( int j=7; j>0; j-- ) {
            if ( *(p + i) & (1 << j) ) {
                strvalue += _("1");
            }
            else {
                strvalue += _("0");
            }
        }
    }
    

error:


    if ( NULL != p ) delete [] p;

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction BitField!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstractionBitField
//

bool CCanalSuperWrapper::writeAbstractionBitField( wxWindow *pwnd,
                                                    uint8_t nodeid,
                                                    CMDF_Abstraction *abstraction,
                                                    wxString& strBitField,
                                                    cguid *pifGUID,
                                                    cguid *pdestGUID,
                                                    wxProgressDialog *pdlg,
                                                    bool bLevel2,
                                                    bool bSilent )
{
    bool rv = true;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    // Octet width is the number of bytes needed to store the bits
    uint8_t octetwidth = abstraction->m_nWidth / 8 + 
                            ( ( abstraction->m_nWidth % 8 ) ? 1 : 0 );

    uint8_t *p;
    p = new uint8_t[ octetwidth ];
    memset( p, 0, octetwidth );

    // Build byte array
    wxString str = strBitField;
    for ( int i=0; i<abstraction->m_nWidth; i++ ) {
        for ( int j=7; j>0; j-- ) {
            if ( !str.Length() ) break; // Must be digits left
            if ( _("1") == str.Left( 1 ) ) {
                *(p+1) += (1 << j);
            }
            str = str.Right( str.Length() - 1 );
        }
    }

    // Save page
    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction BitField!") );
                return false;
            }
        }
    }


    if ( abstraction->m_bIndexed ) {
        
        for ( uint8_t i=0; i<octetwidth; i++ ) {

            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                // Write index to bitfield
                uint8_t val = i;
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction BitField index!") );
                    rv = false;
                    goto error;
                }

                // Read value
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            (p+i) ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction BitField value!") );
                    rv = false;
                    goto error;
                }

            }
            else {
            
                // Write index Level II
                uint8_t idx = i;
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &idx,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                // Write data Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset + i, 
                                            (p+i),
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }	

            }

        }

    }
    else {
        // Write bitfield to linear storage.
        for ( uint8_t i=0; i<octetwidth; i++ ) {

            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {
            
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + i, 
                                            p++ ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction BitField!") );
                    rv = false;
                    goto error;
                }

            }
            else {
    
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            p++,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

            }
        }

    }


error:


    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction BitField!") );
                return false;
            }
        }
    }
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionBool
//

bool CCanalSuperWrapper::getAbstractionBool( wxWindow *pwnd,
                                                uint8_t nodeid,
                                                CMDF_Abstraction *abstraction,
                                                bool *bval,
                                                cguid *pifGUID,
                                                cguid *pdestGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2,
                                                bool bSilent )
{
    bool rv = true;
    uint8_t val;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction bool!") );
                return false;
            }
        }
    }

    
    if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() )) {

        // Read value
        if ( !_readLevel1Register( nodeid, 
                                    abstraction->m_nOffset, 
                                    &val ) ) {
            if ( !bSilent ) wxMessageBox( _("Failed to read abstraction boolean value!") );
            rv = false;
            goto error;
        }

    }
    else {
            
        if ( !readLevel2Register( *pifGUID,
                                    abstraction->m_nOffset,
                                    &val,
                                    pdestGUID, 
                                    pdlg,
                                            bLevel2 ) ) {
            if ( !bSilent ) wxMessageBox( _("Failed to read abstraction data!") );
            rv = false;
            goto error;
        }
    
    }

error:

    *bval = val ? true : false;

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction bool!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstractionBool
//

bool CCanalSuperWrapper::writeAbstractionBool( wxWindow *pwnd,
                                                uint8_t nodeid,
                                                CMDF_Abstraction *abstraction,
                                                bool& bval,
                                                cguid *pifGUID,
                                                cguid *pdestGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2,
                                                bool bSilent )
{
    bool rv = true;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    uint8_t val = bval ? true : false;

    // Save page
    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction boolean!") );
                return false;
            }
        }
    }

    val = bval ? true : false;
    if ( !_writeLevel1Register( nodeid, 
                                abstraction->m_nOffset, 
                                &val ) ) {
        if ( !bSilent ) wxMessageBox( _("Unable to write abstraction boolean!") );
        rv = false;
        goto error;
    }
    else {
            
        // Write index Level II
        if ( !_writeLevel2Register( *pifGUID, 
                                    abstraction->m_nOffset, 
                                    &val,
                                    *pdestGUID ) ) {
            if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
            rv = false;
            goto error;
        }

    }

error:


    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction string!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstraction8bitinteger
//

bool CCanalSuperWrapper::getAbstraction8bitinteger( wxWindow *pwnd,
                                                        uint8_t nodeid,
                                                        CMDF_Abstraction *abstraction,
                                                        uint8_t *pval,
                                                        cguid *pifGUID,
                                                        cguid *pdestGUID,
                                                        wxProgressDialog *pdlg,
                                                        bool bLevel2,
                                                        bool bSilent )
{
    bool rv = true;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 8-bit integer!") );
                return false;
            }
        }
    }

    if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

        // Read value
        if ( !_readLevel1Register( nodeid, 
                                        abstraction->m_nOffset, 
                                        pval ) ) {
            if ( !bSilent ) wxMessageBox( _("Failed to read abstraction 8-bit integer value!") );
            rv = false;
            goto error;
        }

    }
    else {
            
        if ( !readLevel2Register( *pifGUID,
                                    abstraction->m_nOffset,
                                    pval,
                                    pdestGUID, 
                                    pdlg,
                                    bLevel2 ) ) {
            if ( !bSilent ) wxMessageBox( _("Failed to read abstraction data!") );
            rv = false;
            goto error;
        }
    
    }

error:

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid,savepage ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 8-bit integer!") );
                return false;
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstraction8bitinteger
//

bool CCanalSuperWrapper::writeAbstraction8bitinteger( wxWindow *pwnd,
                                                        uint8_t nodeid,
                                                        CMDF_Abstraction *abstraction,
                                                        uint8_t& val,
                                                        cguid *pifGUID,
                                                        cguid *pdestGUID,
                                                        wxProgressDialog *pdlg,
                                                        bool bLevel2,
                                                        bool bSilent )
{
    bool rv = true;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    // Save page
    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 8-bit integer!") );
                return false;
            }
        }
    }

    if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {
    
        if ( !_writeLevel1Register( nodeid, 
                                    abstraction->m_nOffset, 
                                    &val ) ) {
            if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 8-bit integer!") );
            rv = false;
            goto error;
        }
    }
    else {
            
        // Write index Level II
        if ( !_writeLevel2Register( *pifGUID, 
                                    abstraction->m_nOffset, 
                                    &val,
                                    *pdestGUID ) ) {
            if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
            rv = false;
            goto error;
    
        }

    }


error:


    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 8-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstraction16bitinteger
//

bool CCanalSuperWrapper::getAbstraction16bitinteger( wxWindow *pwnd,
                                                        uint8_t nodeid,
                                                        CMDF_Abstraction *abstraction,
                                                        uint16_t *pval,
                                                        cguid *pifGUID,
                                                        cguid *pdestGUID,
                                                        wxProgressDialog *pdlg,
                                                        bool bLevel2,
                                                        bool bSilent )
{
    bool rv = true;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }	
        }
    }

    uint8_t *p = 0;
    p = new uint8_t[ 2 ];

    if ( NULL == p ) {
        return false;
    }

    if ( abstraction->m_bIndexed ) {

        for ( uint8_t i=0; i<2; i++ ) {

            uint8_t idx = i;

            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {
                
                // Write index Level I
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset, 
                                            &idx ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                // Read value
                if ( !_readLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            (p+i) ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
                    rv = false;
                    goto error;
                }

            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &idx,
                                            *pdestGUID ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                if ( !readLevel2Register( *pifGUID,
                                            abstraction->m_nOffset + 1,
                                            (p+i),
                                            pifGUID, 
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read abstraction data!") );
                    rv = false;
                    goto error;
                }	

            }
            
        }
    }
    else {

        // Read string from linear storage.
        if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {
        
            // Level I.
            if ( !readLevel1Registers( pwnd, 
                                        p, 
                                        nodeid, 
                                        abstraction->m_nOffset, 
                                        2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }
        else {

            // Level II.
            if ( !readLevel2Registers( pwnd,
                                        p,
                                        *pifGUID,
                                        abstraction->m_nOffset,
                                        2,
                                        pdestGUID,
                                        pdlg,
                                        bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }
    }

error:

    *pval = ( p[0] << 8 ) + p[1];
    delete [] p;
    p = NULL;

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstraction16bitinteger
//

bool CCanalSuperWrapper::writeAbstraction16bitinteger( wxWindow *pwnd,
                                                        uint8_t nodeid,
                                                        CMDF_Abstraction *abstraction,
                                                        uint16_t& val16,
                                                        cguid *pifGUID,
                                                        cguid *pdestGUID,
                                                        wxProgressDialog *pdlg,
                                                        bool bLevel2,
                                                        bool bSilent )
{
    bool rv = false;
    val16 = wxINT16_SWAP_ON_LE( val16 );
    uint8_t *p = (uint8_t *)&val16;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    // Save page
    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    if ( abstraction->m_bIndexed ) {

        for ( uint8_t i=0; i<2; i++ ) {

            uint8_t idx = i;

            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {
    
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset, 
                                            &idx ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
        
                // Write MSB
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            (p+i) ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }

            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &idx,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                // Write data Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset + i, 
                                            (p+i),
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }	

            }
        }

    }
    else {

        // Write string from linear storage.
        if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {
        
            // Write MSB
            if ( !_writeLevel1Register( nodeid, 
                                        abstraction->m_nOffset, 
                                        p ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                rv = false;
                goto error;
            }

            // Write LSB
            if ( !_writeLevel1Register( nodeid, 
                                        abstraction->m_nOffset + 1, 
                                        (p + 1) ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                rv = false;
                goto error;
            }

        }
        else {
        
            // Write MSB
            if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            p,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                rv = false;
                goto error;
            }

            // Write LSB
            if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset + 1 , 
                                            (p + 1),
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                rv = false;
                goto error;
            }
        
        }
    }

error:

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }	
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstraction32bitinteger
//

bool CCanalSuperWrapper::getAbstraction32bitinteger( wxWindow *pwnd,
                                                        uint8_t nodeid,
                                                        CMDF_Abstraction *abstraction,
                                                        uint32_t *pval,
                                                        cguid *pifGUID,
                                                        cguid *pdestGUID,
                                                        wxProgressDialog *pdlg,
                                                        bool bLevel2,
                                                        bool bSilent )
{
    bool rv = true;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    uint8_t *p;
    p = new uint8_t[ 4 ];

    if ( abstraction->m_bIndexed ) {

        for ( uint8_t i=0; i<4; i++ ) {

            uint8_t idx = i;
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                // Write index to string
                uint8_t idx = i;
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset, 
                                            &idx ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                // Read value
                if ( !_readLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            (p+i) ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
                    rv = false;
                    goto error;
                }
            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &idx,
                                            *pdestGUID ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                if ( !readLevel2Register( *pifGUID,
                                            abstraction->m_nOffset + 1,
                                            (p+i),
                                            pdestGUID, 
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read abstraction data!") );
                    rv = false;
                    goto error;
                }	

            }

        }
    }
    else {

        // Read string from linear storage.
        if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

            if ( !readLevel1Registers( pwnd, 
                                        p, 
                                        nodeid, 
                                        abstraction->m_nOffset, 
                                        4 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }
        else {

            // Level II.
            if ( !readLevel2Registers( pwnd,
                                        p,
                                        *pifGUID,
                                        abstraction->m_nOffset,
                                        2,
                                        pdestGUID, 
                                        pdlg,
                                        bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }
    }

    *pval = ( p[0] << 24 ) + ( p[1] << 16 ) + ( p[2] << 8 ) + p[3];

error:

    if ( NULL != p ) delete [] p;

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstraction32bitinteger
//

bool CCanalSuperWrapper::writeAbstraction32bitinteger( wxWindow *pwnd,
                                                        uint8_t nodeid,
                                                        CMDF_Abstraction *abstraction,
                                                        uint32_t& val32,
                                                        cguid *pifGUID,
                                                        cguid *pdestGUID,
                                                        wxProgressDialog *pdlg,
                                                        bool bLevel2,
                                                        bool bSilent )
{
    bool rv = true;
    uint8_t val;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    // Save page
    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid );
        if ( savepage != abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    if ( abstraction->m_bIndexed ) {

        for ( int i=0; i<4; i++ ) {
    
            uint8_t idx = i;
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            &idx ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
        
                // Write data
                val = ( ( val32 >> (8 * (3-i) ) ) & 0xff );
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &idx,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                // Write data Level II
                val = ( ( val32 >> (8 * (3-i) ) ) & 0xff );
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset + i, 
                                            &val,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }	

            }

        }

    }
    else {

        for ( int i=0; i<4; i++ ) {

            val = ( ( val32 >> (8 * (3-i) ) ) & 0xff );
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                // Write data
                
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + i, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }

            }
            else {

                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &val,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

            }
        }
    }

error:

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstraction64bitinteger
//
bool CCanalSuperWrapper::getAbstraction64bitinteger( wxWindow *pwnd,
                                                        uint8_t nodeid,
                                                        CMDF_Abstraction *abstraction,
                                                        uint64_t *pval,
                                                        cguid *pifGUID,
                                                        cguid *pdestGUID,
                                                        wxProgressDialog *pdlg,
                                                        bool bLevel2,
                                                        bool bSilent )
{
    bool rv = true;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    uint8_t *p;
    p = new uint8_t[ 8 ];

    if ( abstraction->m_bIndexed ) {

        for ( uint8_t i=0; i<8; i++ ) {

            uint8_t idx = i;
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                // Write index to string
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset, 
                                            &idx ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }
                
                // Read value
                if ( !_readLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            (p+i) ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
                    rv = false;
                    goto error;
                }
            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &idx,
                                            *pdestGUID ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                if ( !readLevel2Register( *pifGUID,
                                            abstraction->m_nOffset + 1,
                                            (p+i),
                                            pdestGUID, 
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read abstraction data!") );
                    rv = false;
                    goto error;
                }	

            }
        }

    }
    else {

        // Read string from linear storage.
        if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

            if ( !readLevel1Registers( pwnd, 
                                        p, 
                                        nodeid, 
                                        abstraction->m_nOffset, 
                                        8 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }
        else {

            // Level II.
            if ( !readLevel2Registers( pwnd,
                                        p,
                                        *pifGUID,
                                        abstraction->m_nOffset,
                                        2,
                                        pdestGUID, 
                                        pdlg,
                                        bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }

    }

    *pval = *( (uint64_t *)p );
    //( p[0] << 56 ) + ( p[1] << 48 ) + ( p[2] << 40 ) + ( p[3] << 32 ) +
    //		( p[4] << 24 ) + ( p[5] << 16 ) + ( p[6] << 8 ) + p[7];

error:

    if ( NULL != p ) delete [] p;

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstraction64bitinteger
//

bool CCanalSuperWrapper::writeAbstraction64bitinteger( wxWindow *pwnd,
                                                        uint8_t nodeid,
                                                        CMDF_Abstraction *abstraction,
                                                        uint64_t& val64,
                                                        cguid *pifGUID,
                                                        cguid *pdestGUID,
                                                        wxProgressDialog *pdlg,
                                                        bool bLevel2,
                                                        bool bSilent )
{
    bool rv = true;
    uint8_t val;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    // Save page
    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    if ( abstraction->m_bIndexed ) {

        for ( int i=0; i<8; i++ ) {

            uint8_t idx = i;
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {
    
                // Index = 0
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            &idx ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
            
                // Write data
                val = ( ( val64 >> (8 * (3-i) ) ) & 0xff );
                    if ( !_writeLevel1Register( nodeid, 
                                                abstraction->m_nOffset + 1, 
                                                &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &idx,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                // Write data Level II
                val = ( ( val64 >> (8 * (3-i) ) ) & 0xff );
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset + i, 
                                            &val,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }	

            }

        }

    }
    else {

        for ( int i=0; i<8; i++ ) {

            val = ( ( val64 >> (8 * (3-i) ) ) & 0xff );
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {
                    
                // Write data
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + i, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
            }
            else {

                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &val,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

            }
        }
    }

error:

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionFloat
//
bool CCanalSuperWrapper::getAbstractionFloat( wxWindow *pwnd,
                                                uint8_t nodeid,
                                                CMDF_Abstraction *abstraction,
                                                float *pval,
                                                cguid *pifGUID,
                                                cguid *pdestGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2,
                                                bool bSilent )
{
    bool rv = true;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    uint8_t *p;
    p = new uint8_t[ 4 ];

    if ( abstraction->m_bIndexed ) {

        for ( uint8_t i=0; i<4; i++ ) {

            uint8_t idx = i;
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() )  ) {

                // Write index to string
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset, 
                                            &idx ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }
            
                // Read value
                if ( !_readLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            (p+i) ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
                    rv = false;
                    goto error;
                }
            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &idx,
                                            *pdestGUID ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                if ( !readLevel2Register( *pifGUID,
                                            abstraction->m_nOffset + 1,
                                            (p+i),
                                            pdestGUID, 
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read abstraction data!") );
                    rv = false;
                    goto error;
                }	

            }

        }
    }
    else {

        // Read float from linear storage.
        if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

            if ( !readLevel1Registers( pwnd, 
                                        p, 
                                        nodeid, 
                                        abstraction->m_nOffset, 
                                        4 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }
        else {

            // Level II.
            if ( !readLevel2Registers( pwnd,
                                        p,
                                        *pifGUID,
                                        abstraction->m_nOffset,
                                        2,
                                        pdestGUID, 
                                        pdlg,
                                        bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }

    }

    *pval = wxINT32_SWAP_ON_BE( *((float *)p) );

error:

    if ( NULL != p ) delete [] p;

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstractionFloat
//

bool CCanalSuperWrapper::writeAbstractionFloat( wxWindow *pwnd,
                                                uint8_t nodeid,
                                                CMDF_Abstraction *abstraction,
                                                float& valfloat,
                                                cguid *pifGUID,
                                                cguid *pdestGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2,
                                                bool bSilent )
{
    bool rv = true;
    uint8_t val;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    uint8_t *p = (uint8_t *)&valfloat;

    // Save page
    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    if ( abstraction->m_bIndexed ) {

        for ( int i=0; i<4; i++ ) {

            uint8_t val = i;
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {
    
                // Index = 0
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
        
                // Write data
                val = *(p+i);
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }

            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &val,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                // Write data Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset + i, 
                                            (p+i),
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }	

            }

        }

    }
    else {

        for ( int i=0; i<4; i++ ) {

            val = val = *(p+i);
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() )  ) {

                // Write data
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + i, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }

            }
            else {

                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &val,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

            }
        }

    }


error:


    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionDouble
//

bool CCanalSuperWrapper::getAbstractionDouble( wxWindow *pwnd,
                                                uint8_t nodeid,
                                                CMDF_Abstraction *abstraction,
                                                double *pval,
                                                cguid *pifGUID,
                                                cguid *pdestGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2,
                                                bool bSilent )
{
    bool rv = true;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    uint8_t *p;
    p = new uint8_t[ 8 ];

    if ( abstraction->m_bIndexed ) {

        for ( uint8_t i=0; i<8; i++ ) {

            uint8_t idx = i;
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                // Write index to string	
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset, 
                                            &idx ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                // Read value
                if ( !_readLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            (p+i) ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
                    rv = false;
                    goto error;
                }
            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &idx,
                                            *pdestGUID ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                if ( !readLevel2Register( *pifGUID,
                                            abstraction->m_nOffset + 1,
                                            (p+i),
                                            pdestGUID, 
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read abstraction data!") );
                    rv = false;
                    goto error;
                }	

            }

        }
    }
    else {

        // Read string from linear storage.
        if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

            if ( !readLevel1Registers( pwnd, 
                                        p, 
                                        nodeid, 
                                        abstraction->m_nOffset, 
                                        8 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;	
            }
        }
        else {

            // Level II.
            if ( !readLevel2Registers( pwnd,
                                        p,
                                        *pifGUID,
                                        abstraction->m_nOffset,
                                        2,
                                        pdestGUID, 
                                        pdlg,
                                        bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }

    }

    *pval = *((double *)p);

error:

    if ( NULL != p ) delete [] p;

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  writetAbstractionDouble
//

bool CCanalSuperWrapper::writetAbstractionDouble( wxWindow *pwnd,
                                                    uint8_t nodeid,
                                                    CMDF_Abstraction *abstraction,
                                                    double& valdouble,
                                                    cguid *pifGUID,
                                                    cguid *pdestGUID,
                                                    wxProgressDialog *pdlg,
                                                    bool bLevel2,
                                                    bool bSilent )
{
    bool rv = true;
    uint8_t val;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    uint8_t *p = (uint8_t *)&valdouble;

    // Save page
    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    if ( abstraction->m_bIndexed ) {

        for ( int i=0; i<8; i++ ) {

            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() )  ) {
    
                // Index = 0
                val = i;
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
        
                // Write data
                val = *(p+i);
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
            }
            else {
            
                // Write index Level II
                val = i;
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &val,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                // Write data Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset + i, 
                                            (p+i),
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }	

            }

        }

    }
    else {

        for ( int i=0; i<8; i++ ) {

            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                // Write data
                val = val = *(p+i);
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + i, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
            }
            else {

                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &val,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

            }
        }
        
    }

error:

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionDate
//

bool CCanalSuperWrapper::getAbstractionDate( wxWindow *pwnd,
                                                uint8_t nodeid,
                                                CMDF_Abstraction *abstraction,
                                                wxDateTime *pval,
                                                cguid *pifGUID,
                                                cguid *pdestGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2,
                                                bool bSilent )
{
    bool rv = true;
    uint16_t savepage;
        uint8_t year;

    // Check pointers
    if ( NULL == abstraction) return false;

    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }	
        }
    }

    uint8_t *p;
    p = new uint8_t[ 4 ];

    if ( abstraction->m_bIndexed ) {

        for ( uint8_t i=0; i<4; i++ ) {

            uint8_t idx = i;
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                // Write index to string
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset, 
                                            &idx ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }
            
                // Read value
                if ( !_readLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            (p+i) ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
                    rv = false;
                    goto error;
                }
            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &idx,
                                            *pdestGUID ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                if ( !readLevel2Register( *pifGUID,
                                            abstraction->m_nOffset + 1,
                                            (p+i),
                                            pdestGUID, 
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read abstraction data!") );
                    rv = false;
                    goto error;
                }	

            }

        }
    }
    else {

        // Read string from linear storage.
        if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

            if ( !readLevel1Registers( pwnd, 
                                        p, 
                                        nodeid, 
                                        abstraction->m_nOffset, 
                                        4 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }
        }
        else {

            // Level II.
            if ( !readLevel2Registers( pwnd,
                                        p,
                                        *pifGUID,
                                        abstraction->m_nOffset,
                                        2,
                                        pdestGUID, 
                                        pdlg,
                                        bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }

    }

    year = ( p[ 0 ] << 8 ) + p[ 1 ];
    pval->SetYear( year );
    pval->SetMonth( wxDateTime::Month( p[ 2 ] ) );
    pval->SetDay( p[ 3 ] );

error:

    if ( NULL != p ) delete [] p;

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstractionDate
//

bool CCanalSuperWrapper::writeAbstractionDate( wxWindow *pwnd,
                                                uint8_t nodeid,
                                                CMDF_Abstraction *abstraction,
                                                wxDateTime& valdate,
                                                cguid *pifGUID,
                                                cguid *pdestGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2,
                                                bool bSilent )
{
    bool rv = true;
    uint8_t val;
    uint8_t buf[ 4 ];
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    uint16_t year = valdate.GetYear();
    buf[ 0 ] = ( ( year >> 8 ) & 0xff );
    buf[ 1 ] = ( year & 0xff ); 
    buf[ 2 ] = valdate.GetMonth();
    buf[ 3 ] = valdate.GetDay();

    uint8_t *p = buf;

    // Save page
    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    if ( abstraction->m_bIndexed ) {

        for ( int i=0; i<5; i++ ) {

            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {
    
                // Index = 0
                val = i;
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
        
                // Write data
                val = *(p+i);
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
            }
            else {
            
                // Write index Level II
                val = i;
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &val,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                // Write data Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset + i, 
                                            (p+i),
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }	

            }

        }

    }
    else {

        for ( int i=0; i<5; i++ ) {

            val = val = *(p+i);
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                // Write data
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + i, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }

            }
            else {

                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &val,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

            }
        }
    }

error:

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                    return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionTime
//

bool CCanalSuperWrapper::getAbstractionTime( wxWindow *pwnd,
                                                uint8_t nodeid,
                                                CMDF_Abstraction *abstraction,
                                                wxDateTime *pval,
                                                cguid *pifGUID,
                                                cguid *pdestGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2,
                                                bool bSilent )
{
    bool rv = true;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                    return false;
            }
        }
    }

    uint8_t *p;
    p = new uint8_t[ 3 ];

    if ( abstraction->m_bIndexed ) {

        for ( uint8_t i=0; i<3; i++ ) {

            uint8_t idx = i;
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                // Write index to string
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset, 
                                            &idx ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                // Read value
                if ( !_readLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            (p+i) ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
                    rv = false;
                    goto error;
                }
            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &idx,
                                            *pdestGUID ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                if ( !readLevel2Register( *pifGUID,
                                            abstraction->m_nOffset + 1,
                                            (p+i),
                                            pdestGUID, 
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read abstraction data!") );
                    rv = false;
                    goto error;
                }	

            }

        }
    }
    else {

        // Read string from linear storage.
        if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() )) {

            if ( !readLevel1Registers( pwnd, 
                                        p, 
                                        nodeid, 
                                        abstraction->m_nOffset, 
                                        3 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }
        }
        else {

            // Level II.
            if ( !readLevel2Registers( pwnd,
                                        p,
                                        *pifGUID,
                                        abstraction->m_nOffset,
                                        2,
                                        pdestGUID, 
                                        pdlg,
                                        bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }

    }

    pval->SetHour( p[ 0 ] );
    pval->SetMinute( p[ 1 ] );
    pval->SetSecond( p[ 2 ] );

error:

    if ( NULL != p ) delete [] p;

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstractionTime
//

bool CCanalSuperWrapper::writeAbstractionTime( wxWindow *pwnd,
                                                uint8_t nodeid,
                                                CMDF_Abstraction *abstraction,
                                                wxDateTime& valtime,
                                                cguid *pifGUID,
                                                cguid *pdestGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2,
                                                bool bSilent )
{
    bool rv = true;
    uint8_t val;
    uint8_t buf[ 3 ];
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    buf[ 0 ] = valtime.GetHour();
    buf[ 1 ] = valtime.GetMinute();
    buf[ 2 ] = valtime.GetSecond();

    uint8_t *p = buf;

    // Save page
    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    if ( abstraction->m_bIndexed ) {

        for ( int i=0; i<3; i++ ) {

            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {
    
                // Index = 0
                val = i;
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
        
                // Write data
                val = *(p+i);
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
            }
            else {
            
                // Write index Level II
                val = i;
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &val,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                // Write data Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset + i, 
                                            (p+i),
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }	

            }

        }

    }
    else {

        for ( int i=0; i<3; i++ ) {

            val = val = *(p+i);
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                // Write data
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + i, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }

            }
            else {

                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &val,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

            }

        }
    }

    
error:


    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionGUID
//

bool CCanalSuperWrapper::getAbstractionGUID( wxWindow *pwnd,
                                                uint8_t nodeid,
                                                CMDF_Abstraction *abstraction,
                                                cguid *pval,
                                                cguid *pifGUID,
                                                cguid *pdestGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2,
                                                bool bSilent )
{
    bool rv = true;
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    uint8_t *p;
    p = new uint8_t[ 16 ];

    if ( abstraction->m_bIndexed ) {

        for ( uint8_t i=0; i<16; i++ ) {

            uint8_t idx = i;
            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

                // Write index to string
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset, 
                                            &idx ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                // Read value
                if ( !_readLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            (p+i) ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
                    rv = false;
                    goto error;
                }
            }
            else {
            
                // Write index Level II
                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &idx,
                                            *pdestGUID ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

                if ( !readLevel2Register( *pifGUID,
                                            abstraction->m_nOffset + 1,
                                            (p+i),
                                            pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to read abstraction data!") );
                    rv = false;
                    goto error;
                }	

            }

        }
    }
    else {

        // Read string from linear storage.
        if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {

            if ( !readLevel1Registers( pwnd, 
                                        p, 
                                        nodeid, 
                                        abstraction->m_nOffset, 
                                        16 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }
        }
        else {

            // Level II.
            if ( !readLevel2Registers( pwnd,
                                        p,
                                        *pifGUID,
                                        abstraction->m_nOffset,
                                        2,
                                        pdestGUID, 
                                        pdlg,
                                        bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
                rv = false;
                goto error;
            }

        }

    }

    pval->getFromArray( p );
    
error:

    if ( NULL != p ) delete [] p;

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstractionGUID
//

bool CCanalSuperWrapper::writeAbstractionGUID( wxWindow *pwnd,
                                                uint8_t nodeid,
                                                CMDF_Abstraction *abstraction,
                                                cguid& valguid,
                                                cguid *pifGUID,
                                                cguid *pdestGUID,
                                                wxProgressDialog *pdlg,
                                                bool bLevel2,
                                                bool bSilent )
{
    bool rv = true;
    uint8_t val;
    uint8_t buf[ 16 ];
    uint16_t savepage;

    // Check pointers
    if ( NULL == abstraction) return false;

    memcpy( buf, valguid.getGUID(), 16 );
    uint8_t *p = buf;

    // Save page
    if ( !bLevel2 ) {
        savepage = getRegisterPage( pwnd, nodeid, pifGUID, pdestGUID, bLevel2 );
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, abstraction->m_nPage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    if ( abstraction->m_bIndexed ) {

        for ( int i=0; i<16; i++ ) {

            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {
    
                // Index = 0
                val = i;
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
        
                // Write data
                val = *(p+i);
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + 1, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
            }

        }

    }
    else {

        for ( int i=0; i<16; i++ ) {

            if ( !bLevel2 && ( NULL == pifGUID || pifGUID->isNULL() ) ) {
                
                // Write data
                val = val = *(p+i);
                if ( !_writeLevel1Register( nodeid, 
                                            abstraction->m_nOffset + i, 
                                            &val ) ) {
                    if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
                    rv = false;
                    goto error;
                }
            }
            else {

                if ( !_writeLevel2Register( *pifGUID, 
                                            abstraction->m_nOffset, 
                                            &val,
                                            *pdestGUID,
                                            pdlg,
                                            bLevel2 ) ) {
                    if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
                    rv = false;
                    goto error;
                }

            }

        }
        
    }

error:

    // Restore page
    if ( !bLevel2 ) {
        if ( savepage != abstraction->m_nPage ) {
            if ( !setRegisterPage( nodeid, savepage, pifGUID, pdestGUID, bLevel2 ) ) {
                if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
                return false;
            }
        }
    }

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
//  getAbstractionValueAsString
//

wxString CCanalSuperWrapper::getAbstractionValueAsString( wxWindow *pwnd,
                                                            uint8_t nodeid,
                                                            CMDF_Abstraction *abstraction,
                                                            cguid *pifGUID,
                                                            cguid *pdestGUID,
                                                            wxProgressDialog *pdlg,
                                                            bool bLevel2,
                                                            bool bSilent )
{
    bool rv = false;
    wxString strValue;

    switch ( abstraction->m_nType ) {

    case type_string: 
        rv = getAbstractionString( pwnd,
                                nodeid,
                                abstraction,
                                strValue,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent ); 
        break;

    case type_boolval:
        {
            bool bval;
            rv = getAbstractionBool( pwnd,
                                nodeid,
                                abstraction,
                                &bval,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
            strValue = (bval? _("true") : _("false") );
        }
        break;

    case type_bitfield:
        rv = getAbstractionBitField( pwnd,
                                nodeid,
                                abstraction,
                                strValue,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
        break;

    case type_int8_t:
        {
            uint8_t val;
            rv = getAbstraction8bitinteger( pwnd,
                                nodeid,
                                abstraction,
                                &val,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
            strValue.Printf( _("0x%02x"), val );
        }
        break;

    case type_uint8_t:
        {
            uint8_t val;
            rv = getAbstraction8bitinteger( pwnd,
                                nodeid,
                                abstraction,
                                &val,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
            strValue.Printf( _("0x%02x"), val );
        }
        break;

    case type_int16_t:
        {
            uint16_t val;
            rv = getAbstraction16bitinteger( pwnd,
                                nodeid,
                                abstraction,
                                &val,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
            strValue.Printf( _("0x%04x"), val );
        }
        break;

    case type_uint16_t:
        {
            uint16_t val;
            rv = getAbstraction16bitinteger( pwnd,
                                nodeid,
                                abstraction,
                                &val,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
            strValue.Printf( _("0x%04x"), val );
        }
        break;

    case type_int32_t:
        {
            uint32_t val;
            rv = getAbstraction32bitinteger( pwnd,
                                nodeid,
                                abstraction,
                                &val,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
            strValue.Printf( _("0x%04x"), val );
        }
        break;

    case type_uint32_t:
        {
            uint32_t val;
            rv = getAbstraction32bitinteger( pwnd,
                                nodeid,
                                abstraction,
                                &val,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
            strValue.Printf( _("0x%08x"), val );
        }
        break;

    case type_int64_t:
        {
            uint64_t val;
            rv = getAbstraction64bitinteger( pwnd,
                                nodeid,
                                abstraction,
                                &val,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
            strValue.Printf( _("0x%08x"), val );
        }
        break;

    case type_uint64_t:
        {
            uint64_t val;
            rv = getAbstraction64bitinteger( pwnd,
                                nodeid,
                                abstraction,
                                &val,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
            strValue.Printf( _("0x%08x"), val );
        }
        break;

    case type_float:
        {
            float val;
            rv = getAbstractionFloat( pwnd,
                                nodeid,
                                abstraction,
                                &val,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
            strValue.Printf( _("%f"), val );
        }
        break;

    case type_double:
        {
            double val;
            rv = getAbstractionDouble( pwnd,
                                nodeid,
                                abstraction,
                                &val,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
            strValue.Printf( _("%f"), val );
        }
        break;

    case type_date:
        {
            wxDateTime val;
            rv = getAbstractionDate( pwnd,
                                nodeid,
                                abstraction,
                                &val,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
            if ( val.IsValid() ) {
                strValue = val.FormatISODate();
            }
            else {
                strValue = _( "00-00-00T00:00:00" );
            }
        }
        break;

    case type_time:
        {
            wxDateTime val;
            rv = getAbstractionTime( pwnd,
                                nodeid,
                                abstraction,
                                &val,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
            strValue = val.FormatISOTime();
        }
        break;

    case type_guid:
        {
            cguid val;
            rv = getAbstractionGUID( pwnd,
                                nodeid,
                                abstraction,
                                &val,
                                pifGUID,
                                pdestGUID,
                                pdlg,
                                bLevel2,
                                bSilent  );
            val.toString( strValue );
        }
        break;

    case type_unknown:
    default:
        strValue = _("Unknown Value");
        break;
    }

    if ( !rv ) strValue = _("ERROR");

    return strValue;
}





#endif
