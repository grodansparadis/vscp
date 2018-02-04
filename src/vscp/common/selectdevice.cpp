///////////////////////////////////////////////////////////////////////////////
// SelectDevice.cpp: implementation of the CSelectDevice class.
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2017 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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

#include <wx/valgen.h>
#include <wx/config.h>
#include "wx/wfstream.h"
#include "wx/fileconf.h"

#include "SelectDevice.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSelectDevice::CSelectDevice( wxListBox* plist )
{
    m_plistBox = plist;

    for ( int i=0; i < 256; i++ ) {
        m_deviceList[ i ] = NULL;
    }

    if ( NULL != m_plistBox ) fillListBox();
}

CSelectDevice::~CSelectDevice()
{
    for ( int i=0; i < 256; i++ ) {
        if ( NULL != m_deviceList[ i ] ) {
            delete m_deviceList[ i ];
            m_deviceList[ i ] = NULL;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////
// fillListBox
//

void CSelectDevice::fillListBox( void )
{
    int i;

    if ( NULL == m_plistBox ) return;

    for ( i = 0; i< 256; i++ ) {
        m_deviceList[ i ] = NULL;	
    }


    m_plistBox->Append( _T("canal daemon interface") );

    loadRegistryDeviceData();

    for ( i = 0; i< 256; i++ ) {
        if ( NULL != m_deviceList[ i ] ) {
            m_plistBox->Append( m_deviceList[ i ]->strName );	
        }
    }

    m_plistBox->SetSelection( 0 );

}


//////////////////////////////////////////////////////////////////////////////
// getDataValue
//

unsigned long CSelectDevice::getDataValue( const char *szData )
{
    unsigned long val;
    char *nstop;

    if ( ( NULL != strchr( szData, 'x' ) ) ||
        ( NULL != strchr( szData, 'X' ) ) ) {
            val = strtoul( szData, &nstop, 16 );
    }
    else {
        val = strtoul( szData, &nstop, 10 );
    }

    return val;	
}


/////////////////////////////////////////////////////////////////////////////
// getSelectedDevice
//

void CSelectDevice::getSelectedDevice( int idx, devItem** pItem )
{
    if ( ( 0 == idx ) || ( idx < 0) || ( idx > 255 ) ) {
        *pItem = NULL;
    }
    else {
        *pItem = m_deviceList[ idx -1 ];
    }
}


/////////////////////////////////////////////////////////////////////////////
// getDeviceProfile

bool CSelectDevice::getDeviceProfile( int idx, devItem *pDev )
{
    bool rv = true;

    if ( NULL == pDev ) return false;

    if ( ( 0 == idx ) || ( idx < 0) || ( idx > 255 ) ) {
        return false;
    }
    else if ( 0 == idx ) {
        pDev->id = 0;		
    }
    else {
        memcpy( pDev, m_deviceList[ idx - 1 ], sizeof( devItem ) );	
    }

    return rv;
}

/////////////////////////////////////////////////////////////////////////////
// loadRegistryDeviceData

void CSelectDevice::loadRegistryDeviceData()	
{
    /*
    int i, idx;
    HKEY hk;
    DWORD lv;
    DWORD type;
    char szDeviceKey[ 32 ];
    unsigned char buf[2048];

    idx = 0;

    if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
    "software\\canal\\CanalWorks\\",
    NULL,
    KEY_ALL_ACCESS,
    &hk ) ) {

    // A maximum of 256 devices
    for( i=0; i<256; i++ ) {

    sprintf( szDeviceKey, "device%i", i );
    lv = sizeof( buf );
    if ( ERROR_SUCCESS == RegQueryValueEx(	hk,
    szDeviceKey,
    NULL,
    &type,
    buf,
    &lv ) ) {
    if ( ( REG_SZ == type ) && ( 1 != lv ) ) {
    char *p;
    int inBufSize = 0;
    int outBufSize = 0;
    unsigned long flags = 0;
    unsigned long filter = 0;
    unsigned long mask = 0;

    char *pName = strtok( (char *)buf, "," );
    if ( NULL == pName ) continue;

    char *pDevice = strtok( NULL, "," );

    char *pDLL = strtok( NULL, "," );
    if ( NULL == pDLL ) continue;

    // inbufsize
    p = strtok( NULL, "," );
    if ( NULL != p ) inBufSize = atoi( p );

    // outbufsize
    p = strtok( NULL, "," );
    if ( NULL != p ) outBufSize = atoi( p );

    // flags
    p = strtok( NULL, "," );
    if ( NULL != p ) flags = atol( p );

    // filter
    p = strtok( NULL, "," );
    if ( NULL != p ) filter = atol( p );

    // mask
    p = strtok( NULL, "," );
    if ( NULL != p ) mask = atol( p );

    // Add the device
    devItem *pDev = new devItem;
    if ( NULL != pDev ) {

    pDev->id = idx; // Offset +1
    pDev->regid = i; // Reg idx
    strncpy( (char *)pDev->name, pName, sizeof( pDev->name ) ); 
    if ( NULL != pDevice ) {
    strncpy( (char *)pDev->deviceStr, pDevice, MAX_PATH ); 
    }
    else {
    *pDev->deviceStr = 0;	
    }
    strncpy( (char *)pDev->path, pDLL, MAX_PATH ); 
    pDev->flags = flags;
    pDev->filter = filter;
    pDev->mask = mask;
    pDev->inbufsize = inBufSize;
    pDev->outbufsize = outBufSize;

    m_deviceList[ idx++ ] = pDev;
    }				
    }
    }
    }

    RegCloseKey( hk );

    }
    */

    int i, idx;
    wxChar szDeviceKey[ 32 ];
    wxChar buf[ 2048 ];
    wxString str;

#ifdef WIN32
    char szPathSystemDir[ MAX_PATH ];
    GetSystemDirectory( szPathSystemDir, sizeof( szPathSystemDir ) );
    strcat( szPathSystemDir, "\\canalworks.conf" );
#else
    wxChar szPathSystemDir[ PATH_MAX ];
    strcpy( szPathSystemDir, _("/etc/canalworks.conf") );
#endif 

    wxFFileInputStream cfgstream( szPathSystemDir );

    if ( !cfgstream.Ok() ) {
        wxMessageBox("Failed to open configuration file!");
        return;
    }

    wxFileConfig *pconfig = new wxFileConfig( cfgstream );

    pconfig->SetPath(_("/DRIVERS"));

    idx = 0;

    // A maximum of 256 devices
    for( i=0; i<256; i++ ) {

        sprintf( szDeviceKey, _("device%i"), i );
        if ( pconfig->Read( szDeviceKey, &str ) ) {

            wxChar *p;
            int inBufSize = 0;
            int outBufSize = 0;
            unsigned long flags = 0;
            unsigned long filter = 0;
            unsigned long mask = 0;

            strcpy( (wxChar *)buf, str );
            wxChar *pName = strtok( buf, _(",") );
            if ( NULL == pName ) continue;

            wxChar *pDevice = strtok( NULL, _(",") );

            wxChar *pDLL = strtok( NULL, _(",") );
            if ( NULL == pDLL ) continue;

            // inbufsize
            p = strtok( NULL, _(",") );
            if ( NULL != p ) inBufSize = atoi( p );

            // outbufsize
            p = strtok( NULL, _(",") );
            if ( NULL != p ) outBufSize = atoi( p );

            // flags
            p = strtok( NULL, _(",") );
            if ( NULL != p ) flags = atol( p );

            // filter
            p = strtok( NULL, _(",") );
            if ( NULL != p ) filter = atol( p );

            // mask
            p = strtok( NULL, _(",") );
            if ( NULL != p ) mask = atol( p );

            // Add the device
            devItem *pDev = new devItem;
            if ( NULL != pDev ) {

                pDev->id = idx; // Offset +1
                pDev->regid = i; // Reg idx
                pDev->strName = ( wxChar *)pName;
                pDev->strParameters = ( wxChar *)pDevice;
                pDev->strPath = ( wxChar *)pDLL;
                pDev->flags = flags;
                pDev->filter = filter;
                pDev->mask = mask;

                m_deviceList[ idx++ ] = pDev;
            }	

        }

    }

    if ( NULL!= pconfig ) delete pconfig;

}



/////////////////////////////////////////////////////////////////////////////
// OnButtonEditDevice

void CSelectDevice::OnButtonEditDevice() 
{
    if ( NULL == m_plistBox ) return;	
    /*
    int idx = m_plistBox->GetSelection();

    if ( 0 == idx ) {
    wxMessageBox("Noting to edit for this built in interface!");	
    }
    else if ( -1 != idx ) {

    CDocumentProperties dlg;

    dlg.m_strName = m_deviceList[ idx - 1 ]->name;
    dlg.m_strPath = m_deviceList[ idx - 1 ]->path;
    dlg.m_strDeviceString = m_deviceList[ idx - 1 ]->deviceStr;
    _ultoa( m_deviceList[ idx - 1 ]->flags, dlg.m_strDeviceFlags.GetBufferSetLength( 32 ), 10 );
    itoa( m_deviceList[ idx - 1 ]->inbufsize, dlg.m_strInBufSize.GetBufferSetLength( 32 ), 10 );
    itoa( m_deviceList[ idx - 1 ]->outbufsize, dlg.m_strOutBufSize.GetBufferSetLength( 32 ), 10 );

    if ( IDOK == dlg.DoModal() ) {

    // Get Data
    strncpy( m_deviceList[ idx - 1 ]->name, dlg.m_strName, sizeof( m_deviceList[ idx - 1 ]->name ) );
    strncpy( m_deviceList[ idx - 1 ]->path, dlg.m_strPath, MAX_PATH );
    strncpy( m_deviceList[ idx - 1 ]->deviceStr, dlg.m_strDeviceString, MAX_PATH );

    m_deviceList[ idx - 1 ]->inbufsize = atoi( dlg.m_strOutBufSize ); 
    m_deviceList[ idx - 1 ]->outbufsize = atoi( dlg.m_strInBufSize );
    m_deviceList[ idx - 1 ]->flags = atol( dlg.m_strDeviceFlags  );

    // Fix listbox
    m_ctrlListInterfaces.DeleteString( idx );
    m_ctrlListInterfaces.InsertString( idx, m_deviceList[ idx - 1 ]->name );
    m_ctrlListInterfaces.SetCurSel( idx );

    // Fix registry
    char buf[ 2048 ];
    char key[ 80 ];
    sprintf( buf, 
    "%s,%s,%s,%u,%u,%u", 
    m_deviceList[ idx - 1 ]->name, 
    m_deviceList[ idx - 1 ]->deviceStr, 
    m_deviceList[ idx - 1 ]->path, 
    m_deviceList[ idx - 1 ]->inbufsize, 
    m_deviceList[ idx - 1 ]->outbufsize, 
    m_deviceList[ idx - 1 ]->flags );  

    sprintf( key, "device%i", m_deviceList[ idx - 1 ]->id );

    HKEY hk;
    if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
    "software\\canal\\canalworks\\",
    NULL,
    KEY_ALL_ACCESS,				
    &hk ) ) {				
    RegSetValueEx( hk,
    key,
    NULL,
    REG_SZ,
    (CONST BYTE *)buf,
    sizeof( buf )		
    );

    RegCloseKey( hk );
    }
    }
    }	
    */
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonAddDevice

void CSelectDevice::OnButtonAddDevice() 
{
    /*	if ( NULL == m_plistBox ) return;

    int highRegIdx = 0;
    int highLbIdx = 0;

    CDocumentProperties dlg;

    if ( IDOK == dlg.DoModal() ) {

    // Find unused device devicelist entry
    bool bfoundDevId = false;
    for ( int i=0; i < 256; i++ ) {
    if ( !bfoundDevId && ( NULL == m_deviceList[ i ] ) ) {
    highLbIdx = i;
    bfoundDevId = true;
    }

    if ( NULL != m_deviceList[ i ] ) {
    if ( m_deviceList[ i ]->id > highRegIdx ) {
    highRegIdx = m_deviceList[ i ]->id;
    }
    }
    }

    // Next available is old highest + one
    if ( 0 != highRegIdx )  highRegIdx++;

    devItem *pDev = new devItem;

    if ( NULL != pDev ) {

    m_deviceList[ highLbIdx ] = pDev;
    m_deviceList[ highLbIdx ]->id = highRegIdx;		

    // Get Data
    strncpy( m_deviceList[ highLbIdx ]->name, dlg.m_strName, sizeof( m_deviceList[ highLbIdx ]->name ) );
    strncpy( m_deviceList[ highLbIdx ]->path, dlg.m_strPath, MAX_PATH );
    strncpy( m_deviceList[ highLbIdx ]->deviceStr, dlg.m_strDeviceString, MAX_PATH );
    m_deviceList[ highLbIdx ]->inbufsize = atoi( dlg.m_strOutBufSize );
    m_deviceList[ highLbIdx ]->outbufsize = atoi( dlg.m_strInBufSize );
    m_deviceList[ highLbIdx ]->flags = atol( dlg.m_strDeviceFlags  );

    // Fix listbox
    int idx = m_ctrlListInterfaces.AddString( m_deviceList[ highRegIdx ]->name );
    if ( -1 != idx ) {
    m_ctrlListInterfaces.SetCurSel( idx );
    }

    // Fix registry
    char buf[ 2048 ];
    char key[ 80 ];
    sprintf( buf, 
    "%s,%s,%s,%u,%u,%u", 
    m_deviceList[ highRegIdx ]->name, 
    m_deviceList[ highRegIdx ]->deviceStr, 
    m_deviceList[ highRegIdx ]->path, 
    m_deviceList[ highRegIdx ]->inbufsize, 
    m_deviceList[ highRegIdx ]->outbufsize, 
    m_deviceList[ highRegIdx ]->flags );  

    sprintf( key, "device%i", m_deviceList[ highRegIdx ]->id );

    HKEY hk;
    if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
    "software\\canal\\canalworks\\",
    NULL,
    KEY_ALL_ACCESS,				
    &hk ) ) {				
    RegSetValueEx( hk,
    key,
    NULL,
    REG_SZ,
    (CONST BYTE *)buf,
    sizeof( buf ) );

    RegCloseKey( hk );

    }
    }
    else {
    AfxMessageBox("Unable to add new data. Possible memory problem!");
    }
    }		
    */
}

/////////////////////////////////////////////////////////////////////////////
// OnButtonRemoveDevice

void CSelectDevice::OnButtonRemoveDevice() 
{
    /*	if ( NULL == m_plistBox ) return;

    int idx = m_ctrlListInterfaces.GetCurSel();

    if ( 0 == idx ) {
    AfxMessageBox("This item can not be removed!");	
    }
    else if ( -1 != idx ) {
    if ( IDYES == AfxMessageBox("Are you sure that this item should be deleted?", 
    MB_YESNO ) ) {			
    devItem *pDev = m_deviceList[ idx - 1 ];
    m_deviceList[ idx - 1 ] = NULL;	

    m_ctrlListInterfaces.DeleteString( idx );

    if ( NULL != pDev ) {
    char key[ 80 ];

    sprintf( key, "device%i", pDev->id );

    HKEY hk;
    if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
    "software\\canal\\canalworks\\",
    NULL,
    KEY_ALL_ACCESS,				
    &hk ) ) {				
    RegDeleteValue( hk, key );
    RegCloseKey( hk );

    }

    delete pDev;
    }
    }
    }
    */
}
