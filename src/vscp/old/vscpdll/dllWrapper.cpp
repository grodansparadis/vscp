///////////////////////////////////////////////////////////////////////////////
// DllWrapper.cpp: implementation of the CDllWrapper class.
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
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
// $RCSfile: dllWrapper.cpp,v $                                       
// $Date: 2005/02/16 15:02:37 $                                  
// $Author: akhe $                                              
// $Revision: 1.1 $ 
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "dllwrapper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CDllWrapper::CDllWrapper()
{
	m_hinst = NULL;
	m_devid = 0;
}



CDllWrapper::~CDllWrapper()
{
	if ( NULL != m_hinst ) {
		FreeLibrary( m_hinst );
	}
}

///////////////////////////////////////////////////////////////////////////////
// initialize
//

bool CDllWrapper::initialize( char * path )
{
	if ( NULL == ( m_hinst = LoadLibrary( path ) ) ) {
		return false;
	}

	if ( NULL == ( m_proc_vscpopen = 
					(LPFNDLL_VSCPOPEN)GetProcAddress( m_hinst, "vscpOpen" ) ) ) {
		FreeLibrary( m_hinst );
		m_hinst = NULL;
		return false;
	}

	if ( NULL == ( m_proc_vscpclose = 
					(LPFNDLL_VSCPCLOSE)GetProcAddress( m_hinst, "vscpClose" ) ) ) {
		FreeLibrary( m_hinst );
		m_hinst = NULL;
		return false;
	}

	if ( NULL == ( m_proc_vscpsend = 
					(LPFNDLL_VSCPSEND)GetProcAddress( m_hinst, "vscpSend" ) ) ) {
		FreeLibrary( m_hinst );
		m_hinst = NULL;
		return false;
	}

	if ( NULL == ( m_proc_vscpreceive = 
					(LPFNDLL_VSCPRECEIVE)GetProcAddress( m_hinst, "vscpReceive" ) ) ) {
		FreeLibrary( m_hinst );
		m_hinst = NULL;
		return false;
	}

	if ( NULL == ( m_proc_vscpdataavailable = 
					(LPFNDLL_VSCPDATAAVAILABLE)GetProcAddress( m_hinst, "vscpDataAvailable" ) ) ) {
		FreeLibrary( m_hinst );
		m_hinst = NULL;
		return false;
	}
	 
	if ( NULL == ( m_proc_vscpsetfilter = 
					(LPFNDLL_VSCPSETFILTER)GetProcAddress( m_hinst, "vscpSetFilter" ) ) ) {
		FreeLibrary( m_hinst );
		m_hinst = NULL;
		return false;
	}
	
	return true;
	
}


///////////////////////////////////////////////////////////////////////////////
// open
//

long CDllWrapper::doCmdOpen( char *szInterface, unsigned long flags )
{
	return ( m_devid = m_proc_vscpopen( szInterface, flags ) );
}

 
///////////////////////////////////////////////////////////////////////////////
// close
//

bool CDllWrapper::doCmdClose( void )
{
	bool rv = m_proc_vscpclose( m_devid ) ? true : false ;
	m_devid = 0;
	return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//

bool CDllWrapper::doCmdSend( PVSCPMSG pMsg )
{	
	return ( ( m_proc_vscpsend( m_devid, pMsg ) ) ? true : false );
}


///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

bool CDllWrapper::doCmdReceive(  PVSCPMSG pMsg )
{		
	return( ( m_proc_vscpreceive( m_devid, pMsg ) ) ? true : false );
}


///////////////////////////////////////////////////////////////////////////////
// doCmdDataAvailable
//

int CDllWrapper::doCmdDataAvailable( void )
{
	return m_proc_vscpdataavailable( m_devid );
}



///////////////////////////////////////////////////////////////////////////////
// doCmdFilter
//

bool CDllWrapper::doCmdFilter( vscpMsgFilter *pFilter )
{	
	return ( m_proc_vscpsetfilter( m_devid, pFilter ) ? true : false );
}









