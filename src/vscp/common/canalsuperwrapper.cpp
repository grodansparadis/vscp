///////////////////////////////////////////////////////////////////////////////
// CanalSuperWrapper.cpp: implementation of the CCanalSuperWrapper class.
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
// $RCSfile: canalsuperwrapper.cpp,v $                                       
// $Date: 2005/08/30 11:00:04 $                                  
// $Author: akhe $                                              
// $Revision: 1.6 $ 
///////////////////////////////////////////////////////////////////////////////

#include "wx/wx.h"
#include "wx/defs.h"

#include "vscp.h"
#include "canalsuperwrapper.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCanalSuperWrapper::CCanalSuperWrapper( void )
{	
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
// setInterface
//  TCP/IP version

void CCanalSuperWrapper::setInterface( const wxString& host, 
                                        const short port,
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
    m_itemDevice.strParameters = username;
    m_itemDevice.strParameters += _(";"); 
	m_itemDevice.strParameters += password;
	m_itemDevice.strParameters += _(";");
	m_itemDevice.strParameters += host;
	m_itemDevice.strParameters += _(";");
	m_itemDevice.strParameters += wxString::Format( _("%d"), port );
    
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

long CCanalSuperWrapper::doCmdOpen( const wxString& strInterface, unsigned long flags )
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
            rv = m_vscptcpif.doCmdOpen( strInterface, flags );
        }
        else {
            rv = m_vscptcpif.doCmdOpen( m_itemDevice.strParameters, m_itemDevice.flags );
            if ( CANAL_ERROR_SUCCESS == rv ) {
                // We try to get the interface GUID. If we
                // fail to get it we use the GUID assigned
                // in the constructor
                m_vscptcpif.doCmdGetGUID( (char *)m_GUID.m_id );
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
		convertEventToCanal( &msg, pEvent );
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
		convertVSCPfromEx( pevent, pEventEx );
		rv = doCmdSend( pevent );
		deleteVSCPevent( pevent );
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
  return m_vscptcpif.doCmdReceive( pEvent );
}


///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

int CCanalSuperWrapper::doCmdReceive( vscpEventEx *pEventEx )
{	
  return m_vscptcpif.doCmdReceiveEx( pEventEx );
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
		return m_vscptcpif.doCmdVersion();
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

