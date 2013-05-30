// libvscphelper.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2013
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
//
// Linux
// =====
// device1 = logger,/tmp/canal_log,txt,/usr/local/lib/canallogger.so,64,64,1
//
// WIN32
// =====
// device1 = logger,c:\canal_log,txt,d:\winnr\system32\canallogger.dll,64,64,1

#ifdef __GNUG__
//#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include  "wx/ownerdrw.h"
#endif

#include "wx/tokenzr.h"

#include "libvscphelper.h"
#include "stdio.h"
#include "stdlib.h"


void _init() __attribute__((constructor));
void _fini() __attribute__((destructor));

void _init() {printf("initializing\n");}
void _fini() {printf("finishing\n");}



////////////////////////////////////////////////////////////////////////////
// CVSCPLApp construction

CVSCPLApp::CVSCPLApp()
{
	m_instanceCounter = 0;
	pthread_mutex_init( &m_objMutex, NULL );

	// Init the driver array
	for ( int i = 0; i<VSCP_INTERFACE_MAX_OPEN; i++ ) {
		m_pvscpifArray[ i ] = NULL;
	}

	UNLOCK_MUTEX( m_objMutex );
}


CVSCPLApp::~CVSCPLApp()
{
	LOCK_MUTEX( m_objMutex );
	
	for ( int i = 0; i<VSCP_INTERFACE_MAX_OPEN; i++ ) {
		
		if ( NULL == m_pvscpifArray[ i ] ) {
			
			VscpTcpIf *pvscpif =  getDriverObject( i );
			if ( NULL != pvscpif ) { 
				pvscpif->doCmdClose();	
				delete m_pvscpifArray[ i ];
				m_pvscpifArray[ i ] = NULL; 
			}
		}
	}

	UNLOCK_MUTEX( m_objMutex );	
	pthread_mutex_destroy( &m_objMutex );
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLoggerdllApp object

CVSCPLApp theApp;




///////////////////////////////////////////////////////////////////////////////
// CreateObject


///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long CVSCPLApp::addDriverObject( VscpTcpIf *pvscpif )
{
	long h = 0;

	LOCK_MUTEX( m_objMutex );
	for ( int i=0; i<VSCP_INTERFACE_MAX_OPEN; i++ ) {
	
		if ( NULL == m_pvscpifArray[ i ] ) {
		
			m_pvscpifArray[ i ] = pvscpif;	
			h = i + 1681; 
			break;

		}

	}

	UNLOCK_MUTEX( m_objMutex );

	return h;
}


///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

VscpTcpIf *CVSCPLApp::getDriverObject( long h )
{
	long idx = h - 1681;

	// Check if valid handle
	if ( idx < 0 ) return NULL;
	if ( idx >= VSCP_INTERFACE_MAX_OPEN ) return NULL;
	return m_pvscpifArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void CVSCPLApp::removeDriverObject( long h )
{
	long idx = h - 1681;

	// Check if valid handle
	if ( idx < 0 ) return;
	if ( idx >= VSCP_INTERFACE_MAX_OPEN  ) return;

	LOCK_MUTEX( m_objMutex );
	if ( NULL != m_pvscpifArray[ idx ] ) delete m_pvscpifArray[ idx ];
	m_pvscpifArray[ idx ] = NULL;
	UNLOCK_MUTEX( m_objMutex );
}

///////////////////////////////////////////////////////////////////////////////
// InitInstance

BOOL CVSCPLApp::InitInstance() 
{
	m_instanceCounter++;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//                      V S C P   H E L P E R -  A P I
///////////////////////////////////////////////////////////////////////////////

/*!
    \fn long vscp_readStringValue( const char * pStrValue )
    \brief Read a value (decimal or hex) from a string.
	\return The converted number.
*/

extern "C" unsigned long  vscp_readStringValue( const char * pStrValue )
{
    wxString strVal;
    strVal.FromAscii( pStrValue );

    return readStringValue( strVal );
}



/*!
    \fn unsigned char vscp_getVscpPriority( const vscpEvent *pEvent )
    \brief Get VSCP priority.
*/

extern "C" unsigned char  vscp_getVscpPriority( const vscpEvent *pEvent )
{
    return getVscpPriority( pEvent );
}

/*!
    \fn void vscp_setVscpPriority( vscpEvent *pEvent, unsigned char priority )
    \brief Set VSCP priority.
*/

extern "C" void  vscp_setVscpPriority( vscpEvent *pEvent, unsigned char priority )
{
    setVscpPriority( pEvent, priority );
}


/*!
    \fn vscp_getVSCPheadFromCANid( const unsigned long id )
    \brief Get the VSCP head from a CANAL message id (CAN id).
*/

extern "C" unsigned char  vscp_getVSCPheadFromCANid( const unsigned long id )
{
    return  getVSCPheadFromCANid( id );
}

/*!
    \fn vscp_getVSCPclassFromCANid( const unsigned long id )
    \brief Get the VSCP class from a CANAL message id (CAN id).
*/

extern "C" unsigned short  vscp_getVSCPclassFromCANid( const unsigned long id )
{
    return getVSCPclassFromCANid( id );
}


/*!
    \fn unsigned short vscp_getVSCPtypeFromCANid( const unsigned long id )
    \brief Get the VSCP type from a a CANAL message id (CAN id).
*/

extern "C" unsigned short  vscp_getVSCPtypeFromCANid( const unsigned long id )
{
    return getVSCPtypeFromCANid( id );
}

/*!
    \fn unsigned short vscp_getVSCPnicknameFromCANid( const unsigned long id )
    \brief Get the VSCP nickname from a a CANAL message id (CAN id).
*/

extern "C" unsigned short  vscp_getVSCPnicknameFromCANid( const unsigned long id )
{
    return getVSCPnicknameFromCANid( id );
}

/*!
    \fn unsigned long vscp_getCANidFromVSCPdata( const unsigned char priority, 
                                                    const unsigned short vscp_class, 
                                                    const unsigned short vscp_type )
    \brief Construct a CANAL id (CAN id ) from VSCP.
*/

extern "C" unsigned long  vscp_getCANidFromVSCPdata( const unsigned char priority, 
                                                                    const unsigned short vscp_class, 
                                                                    const unsigned short vscp_type )
{
    return  getCANidFromVSCPdata( priority, vscp_class, vscp_type );
}

/*!
    \fn unsigned long vscp_getCANidFromVSCPevent( const vscpEvent *pEvent )
    \brief Get CANAL id (CAN id) from VSCP event.
*/

extern "C" unsigned long  vscp_getCANidFromVSCPevent( const vscpEvent *pEvent )
{
    return getCANidFromVSCPevent( pEvent );
}

/*!
    \fn unsigned short vscp_calcCRC( vscpEvent *pEvent, short bSet )
    \brief Calculate VSCP crc.
*/

extern "C" unsigned short  vscp_calcCRC( vscpEvent *pEvent, short bSet )
{
    return vscp_calc_crc( pEvent, bSet );
}


/*!
    \fn bool vscp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
    \brief Write GUID into VSCP event from string.
*/

extern "C" bool  vscp_getGuidFromString( vscpEvent *pEvent, const char * pGUID )
{
    wxString strGUID = wxString::FromAscii( pGUID );
    return  getGuidFromString( pEvent, strGUID );
}

/*!
    \fn bool vscp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr )
    \brief Write GUID from string into array.
*/

extern "C" bool  vscp_getGuidFromStringToArray( uint8_t *pGUID, const char * pStr )
{
    wxString strGUID = wxString::FromAscii( pStr );
    return getGuidFromStringToArray( pGUID, strGUID );
}

/*!
    \fn bool vscp_writeGuidToString( const vscpEvent *pEvent, char * pStr )
    \brief Write GUID froom VSCP event to string.
*/

extern "C" bool  vscp_writeGuidToString( const vscpEvent *pEvent, char * pStr )
{
    bool rv;

    wxString strGUID;
    rv = writeGuidToString( pEvent, strGUID );
    strcpy( pStr, strGUID.ToAscii() );
    return rv;
}


/*!
    \fn bool vscp_writeGuidToString4Rows( const vscpEvent *pEvent, 
                                            wxString& strGUID )
    \brief Write GUID from VSCP event to string with four bytes on each
    row seperated by \r\n. 
*/

extern "C" bool  vscp_writeGuidToString4Rows( const vscpEvent *pEvent, 
                                                                wxString& strGUID )
{
    return writeGuidToString4Rows( pEvent, strGUID );
}

/*!
    \fn bool vscp_writeGuidArrayToString( const unsigned char * pGUID, 
                                            wxString& strGUID )
    \brief Write GUID from byte array to string.
*/

extern "C" bool  vscp_writeGuidArrayToString( const unsigned char * pGUID, 
                                                                wxString& strGUID )
{
    return writeGuidArrayToString( pGUID, strGUID );
}

/*!
    \fn bool vscp_isGUIDEmpty( unsigned char *pGUID )
    \brief Check if GUID is empty (all nills).
*/

extern "C" bool  vscp_isGUIDEmpty( unsigned char *pGUID )
{
    return isGUIDEmpty( pGUID );
}

/*!
    \fn bool vscp_isSameGUID( const unsigned char *pGUID1, 
                                const unsigned char *pGUID2 )
    \brief Check if two GUID's is equal to each other.
*/

extern "C" bool  vscp_isSameGUID( const unsigned char *pGUID1, 
                                                const unsigned char *pGUID2 )
{
    return isSameGUID( pGUID1, pGUID2 );
}

/*!
    \fn bool vscp_convertVSCPtoEx( vscpEventEx *pEventEx, 
                                    const vscpEvent *pEvent )
    \brief Convert VSCP standard event form to ex. form.
*/

extern "C" bool  vscp_convertVSCPtoEx( vscpEventEx *pEventEx, 
                                                        const vscpEvent *pEvent )
{
    return convertVSCPtoEx( pEventEx, pEvent );
}

/*!
    \fn bool vscp_convertVSCPfromEx( vscpEvent *pEvent, 
                                        const vscpEventEx *pEventEx )
    \brief Convert VSCP ex. event form to standard form.
*/

extern "C" bool  vscp_convertVSCPfromEx( vscpEvent *pEvent, 
                                                        const vscpEventEx *pEventEx )
{
    return convertVSCPfromEx( pEvent, pEventEx );
}


/*!
    \fn void vscp_deleteVSCPevent( vscpEvent *pEvent )
    \brief Delete VSCP event.
*/

extern "C" void  vscp_deleteVSCPevent( vscpEvent *pEvent )
{
    return deleteVSCPevent( pEvent );
}

/*!
    \fn void vscp_deleteVSCPeventEx( vscpEventEx *pEventEx )
    \brief Delete VSCP event ex.
*/

extern "C" void  vscp_deleteVSCPeventEx( vscpEventEx *pEventEx )
{
    return deleteVSCPeventEx( pEventEx );
}

/*!
    \fn void vscp_clearVSCPFilter( vscpEventFilter *pFilter )
    \brief Clear VSCP filter.
*/

extern "C" void  vscp_clearVSCPFilter( vscpEventFilter *pFilter )
{
    return clearVSCPFilter( pFilter );
}

/*!
    \fn bool readFilterFromString( vscpEventFilter *pFilter, wxString& strFilter )
    \brief Read a filter from a string
	\param pFilter Filter structure to write filter to.
	\param strFilter Filter in string form 
				filter-priority, filter-class, filter-type, filter-GUID
	\return true on success, fals eon failure.
*/

extern "C" bool  vscp_readFilterFromString( vscpEventFilter *pFilter, wxString& strFilter )
{
	return readFilterFromString( pFilter, strFilter );
}

/*!
    \fn bool readMaskFromString( vscpEventFilter *pFilter, wxString& strMask )
    \brief Read a mask from a string
	\param pFilter Filter structure to write mask to.
	\param strMask Mask in string form 
				mask-priority, mask-class, mask-type, mask-GUID
	\return true on success, fals eon failure.
*/

extern "C" bool  vscp_readMaskFromString( vscpEventFilter *pFilter, wxString& strMask )
{
	return readMaskFromString( pFilter, strMask );
}

/*!
    \fn bool vscp_doLevel2Filter( const vscpEvent *pEvent,
                                    const vscpEventFilter *pFilter )
    \brief Check VSCP filter condition.
*/

extern "C" bool  vscp_doLevel2Filter( const vscpEvent *pEvent,
                                                    const vscpEventFilter *pFilter )
{
    return  doLevel2Filter( pEvent, pFilter );
}


/*!
    \fn bool vscp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                            const canalMsg *pcanalMsg,
                                            unsigned char *pGUID,
                                            bool bCAN )
    \brief Convert CANAL message to VSCP event.
*/

extern "C" bool  vscp_convertCanalToEvent( vscpEvent *pvscpEvent,
                                                            const canalMsg *pcanalMsg,
                                                            unsigned char *pGUID,
                                                            bool bCAN )
{
    return convertCanalToEvent( pvscpEvent,
                                    pcanalMsg,
                                    pGUID,
                                    bCAN );
}


/*!
    \fn bool vscp_convertEventToCanal( canalMsg *pcanalMsg,
                                        const vscpEvent *pvscpEvent )
    \brief Convert VSCP event to CANAL message.
*/

extern "C" bool  vscp_convertEventToCanal( canalMsg *pcanalMsg,
                                                            const vscpEvent *pvscpEvent )
{
    return  convertEventToCanal( pcanalMsg,
                                    pvscpEvent );
}


/*!
    \fn bool vscp_convertEventExToCanal( canalMsg *pcanalMsg,
                                            const vscpEventEx *pvscpEventEx )
    \brief Convert VSCP event ex. to CANAL message.
*/

extern "C" bool  vscp_convertEventExToCanal( canalMsg *pcanalMsg,
                                                            const vscpEventEx *pvscpEventEx )
{
    return  convertEventExToCanal( pcanalMsg,
                                    pvscpEventEx );
}

/*!
    \fn unsigned long vscp_getTimeStamp( void )
    \brief Get VSCP timestamp.
*/

extern "C" unsigned long  vscp_getTimeStamp( void )
{
    return makeTimeStamp();
}

/*!
    \fn bool vscp_copyVSCPEvent( vscpEvent *pEventTo, 
                                    const vscpEvent *pEventFrom )
    \brief Copy VSCP event.
*/

extern "C" bool  vscp_copyVSCPEvent( vscpEvent *pEventTo, 
                                                    const vscpEvent *pEventFrom )
{
    return copyVSCPEvent( pEventTo, pEventFrom );
}

/*!
    \fn bool vscp_writeVscpDataToString( const vscpEvent *pEvent, 
                                            wxString& str, 
                                            bool bUseHtmlBreak )
    \brief Write VSCP data in readable form to a (multiline) string.
*/

extern "C" bool  vscp_writeVscpDataToString( const vscpEvent *pEvent, 
                                                            wxString& str, 
                                                            bool bUseHtmlBreak )
{
    return writeVscpDataToString( pEvent, 
                                    str, 
                                    bUseHtmlBreak );
}


/*!
    \fn bool vscp_getVscpDataFromString( vscpEvent *pEvent, 
                                            const wxString& str )
    \brief Set data in VSCP event from a string.
*/
extern "C" bool  vscp_getVscpDataFromString( vscpEvent *pEvent, 
                                                                const wxString& str )
{
    return getVscpDataFromString( pEvent, str );
}

/*!
    \fn bool vscp_writeVscpEventToString( vscpEvent *pEvent, 
                                            char *p )
    \brief Write VSCP data to a string.
*/

extern "C" bool  vscp_writeVscpEventToString( vscpEvent *pEvent, 
                                                                char *p )
{
    bool rv;

    wxString str = wxString::FromAscii( p );
    if ( ( rv =  writeVscpEventToString( pEvent, str ) ) ) {
        strcpy( p, str.ToAscii() );
    }
    return rv;
}

/*!
    \fn bool vscp_getVscpEventFromString( vscpEvent *pEvent, 
                                            const char *p )
    \brief Get VSCP event from string.
*/

extern "C" bool  vscp_getVscpEventFromString( vscpEvent *pEvent, 
                                                                const char *p )
{
    wxString str = wxString::FromAscii( p );
    return getVscpEventFromString( pEvent, str ); 
}


//-------------------------------------------------------------------------


/*!
    \fn bool vscp_getVariableString( const char *pName, char *pValue ) 
    \brief Get variable value from string variable
    \param name of variable
    \param pointer to string that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_getVariableString( const char *pName, char *pValue ) 
{ 
    bool rv;

    wxString name = wxString::FromAscii( pName );
    wxString strValue;
    if ( ( rv = theApp.m_vscpif.getVariableString( name, &strValue ) ) ) {
        strcpy( pValue, strValue.ToAscii() );
    }

    return rv;
}

/*!
    \fn bool vscp_setVariableString( const char *pName, char *pValue ) 
    \brief set string variable from a pointer to a string
    \param name of variable
    \param pointer to string that contains the string.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_setVariableString( const char *pName, char *pValue ) 
{ 
    bool rv;

    // Check pointers
    if ( NULL == pName ) return false;
    if ( NULL == pValue ) return false;

    wxString name = wxString::FromAscii( pName );
    wxString strValue = wxString::FromAscii( pValue );
    return theApp.m_vscpif.setVariableString( name, strValue );

    return rv;
}

/*!
    \fn bool vscp_getVariableBool( const char *pName, bool *bValue )
    \brief Get variable value from boolean variable
    \param name of variable
    \param pointer to boolean variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_getVariableBool( const char *pName, bool *bValue )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableBool( name, bValue ); 
};


/*!
    \fn bool vscp_setVariableBool( const char *pName, bool bValue )
    \brief Get variable value from boolean variable
    \param name of variable
    \param pointer to boolean variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_setVariableBool( const char *pName, bool bValue )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableBool( name, bValue ); 
};


/*!
    \fn bool vscp_getVariableInt( const char *pName, int *value )
    \brief Get variable value from integer variable
    \param name of variable
    \param pointer to integer variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_getVariableInt( const char *pName, int *value )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableInt( name, value ); 
};


/*!
    \fn bool vscp_setVariableInt( const char *pName, int value )
    \brief Get variable value from integer variable
    \param name of variable
    \param pointer to integer variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_setVariableInt( const char *pName, int value )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableInt( name, value ); 
};

/*!
    \fn bool vscp_getVariableLong( const char *pName, long *value )
    \brief Get variable value from long variable
    \param name of variable
    \param pointer to long variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_getVariableLong( const char *pName, long *value )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableLong( name, value ); 
};

/*!
    \fn bool vscp_setVariableLong( const char *pName, long value )
    \brief Get variable value from long variable
    \param name of variable
    \param pointer to long variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_setVariableLong( const char *pName, long value )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableLong( name, value ); 
};

/*!
    \fn bool vscp_getVariableDouble( const char *pName, double *value )
    \brief Get variable value from double variable
    \param name of variable
    \param pointer to double variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_getVariableDouble( const char *pName, double *value )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableDouble( name, value ); 
};

/*!
    \fn bool vscp_setVariableDouble( const char *pName, double value )
    \brief Get variable value from double variable
    \param name of variable
    \param pointer to double variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_setVariableDouble( const char *pName, double value )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableDouble( name, value ); 
};

/*!
    \fn bool vscp_getVariableMeasurement( const char *pName, char *pValue )
    \brief Get variable value from measurement variable
    \param name of variable
    \param String that get that get the 
    value of the measurement.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_getVariableMeasurement( const char *pName, char *pValue )
{ 
    bool rv;

    wxString name = wxString::FromAscii( pName );
    wxString strValue;
    if ( rv = theApp.m_vscpif.getVariableMeasurement( name, strValue ) ) {
        strcpy( pValue, strValue.ToAscii() );
    }

    return rv;
};

/*!
    \fn bool vscp_setVariableMeasurement( const char *pName, char *pValue )
    \brief Get variable value from measurement variable
    \param name of variable
    \param String that get that get the 
    value of the measurement.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_setVariableMeasurement( const char *pName, char *pValue )
{ 
    bool rv;

    wxString name = wxString::FromAscii( pName );
    wxString strValue;
    return theApp.m_vscpif.setVariableMeasurement( name, strValue );

    return rv;
};

/*!
    \fn bool vscp_getVariableEvent( const char *pName, vscpEvent *pEvent )
    \breif Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_getVariableEvent( const char *pName, vscpEvent *pEvent )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableEvent( name, pEvent ); 
}

/*!
    \fn bool vscp_setVariableEvent( const char *pName, vscpEvent *pEvent )
    \breif Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_setVariableEvent( const char *pName, vscpEvent *pEvent )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableEvent( name, pEvent ); 
}

/*!
    \fn bool vscp_getVariableEventEx( const char *pName, vscpEventEx *pEvent )
    \brief Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_getVariableEventEx( const char *pName, vscpEventEx *pEvent )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableEventEx( name, pEvent ); 
}

/*!
    \fn bool vscp_setVariableEventEx( const char *pName, vscpEventEx *pEvent )
    \brief Get variable value from event variable
    \param name of variable
    \param pointer to event variable that get the value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_setVariableEventEx( const char *pName, vscpEventEx *pEvent )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableEventEx( name, pEvent ); 
}

/*!
    \fn bool vscp_getVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_getVariableGUID( const char *pName, cguid& GUID )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableGUID( name, GUID ); 
}

/*!
    \fn bool vscp_setVariableGUID( const char *pName, cguid& GUID )
    \brief Get variable value from GUID variable
    \param name of variable
    \param pointer to event variable that get the value of the GUID variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_setVariableGUID( const char *pName, cguid& GUID )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableGUID( name, GUID ); 
}

/*!
    \fn bool vscp_getVariableVSCPdata( const char *pName, uint16_t *psizeData, uint8_t *pData )
    \brief Get variable value from VSCP data variable
    \param name of variable
    \param Pointer to variable that will hold the size of the data array
    \param pointer to VSCP data array variable (unsigned char [8] ) that get the 
    value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_getVariableVSCPdata( const char *pName, uint16_t *psizeData, uint8_t *pData )
{ 
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableVSCPdata( name, psizeData, pData ); 
}

/*!
    \fn bool vscp_setVariableVSCPdata( const char *pName, uint16_t sizeData, uint8_t *pData )
    \brief Get variable value from VSCP data variable
    \param name of variable
    \param Pointer to variable that will hold the size of the data array
    \param pointer to VSCP data array variable (unsigned char [8] ) that get the 
    value of the string variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_setVariableVSCPdata( const char *pName, uint16_t sizeData, uint8_t *pData )
{ 
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableVSCPdata( name, sizeData, pData ); 
}

/*!
    \fn bool vscp_getVariableVSCPclass( const char *pName, uint16_t *vscp_class )
    \brief Get variable value from class variable
    \param name of variable
    \param pointer to int that get the value of the class variable.
    \return true if the variable is of type string.
*/
extern "C"  bool  vscp_getVariableVSCPclass( const char *pName, uint16_t *vscp_class )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableVSCPclass( name, vscp_class ); 
}

/*!
    \fn bool vscp_setVariableVSCPclass( const char *pName, uint16_t vscp_class )
    \brief Get variable value from class variable
    \param name of variable
    \param pointer to int that get the value of the class variable.
    \return true if the variable is of type string.
*/
extern "C"  bool  vscp_setVariableVSCPclass( const char *pName, uint16_t vscp_class )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableVSCPclass( name, vscp_class ); 
}

/*!
    \fn bool vscp_getVariableVSCPtype( const char *pName, uint8_t *vscp_type )
    \brief Get variable value from type variable
    \param name of variable
    \param pointer to int that get the value of the type variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_getVariableVSCPtype( const char *pName, uint8_t *vscp_type )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.getVariableVSCPtype( name, vscp_type ); 
}

/*!
    \fn bool  vscp_setVariableVSCPtype( const char *pName, uint8_t vscp_type )
    \brief Get variable value from type variable
    \param name of variable
    \param pointer to int that get the value of the type variable.
    \return true if the variable is of type string.
*/
extern "C" bool  vscp_setVariableVSCPtype( const char *pName, uint8_t vscp_type )
{
    wxString name = wxString::FromAscii( pName );
    return theApp.m_vscpif.setVariableVSCPtype( name, vscp_type ); 
}