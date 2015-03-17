// ControlObject.cpp: implementation of the CControlObject class.
//
//////////////////////////////////////////////////////////////////////

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

#include  "wx/log.h"

#include  "wx/sizer.h"
#include  "wx/menuitem.h"
#include  "wx/checklst.h"

#include "ControlObject.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CControlObject::CControlObject()
{

}

CControlObject::~CControlObject()
{

}


//////////////////////////////////////////////////////////////////////////////
// convertToASCII

void 
CControlObject::convertToASCII( unsigned char *p )
{
	unsigned char *pWrite = p;

	if ( NULL == p ) return;

	while ( *p ) {		

		if ( 0xc3 == *p ) {
			
			p++;
			
			switch( *p ) {

				case 0:
					return;	// Done
					break;

				case 0xa5:	// å
					*pWrite = 0xe5;
					break;

				case 0xa4:	// ä
					*pWrite = 0xe4;
					break;
					
				case 0xb6:	// ö
					*pWrite = 0xf6;
					break;

				case 0x85:	// Å
					*pWrite = 0xc5;
					break;

				case 0x84:	// Ä
					*pWrite = 0xc4;
					break;
					
				case 0x96:	// Ö
					*pWrite = 0xd6;
					break;

				default:
					break;
			}
		}
		else {
			*pWrite = *p;
		}
		
		p++;
		pWrite++;

	}

	*pWrite = 0;

}


///////////////////////////////////////////////////////////////////////////////
// startElement

void
CControlObject::startElement( char *name, char **atts )
{

	m_depth += 1;

	// lrfdata
	if ( 0 == strcmp( name, _T("lrfdata") ) ) {
		
		// Start tag
		m_bStartTag = true;	
	
		// Empty list
		m_cntList = 0;

	}

	// object
	else if ( 0 == strcmp( name, _T("object") ) ) {

		if ( m_cntList < MAX_ELEMENTS ) {

			// Object tag - A new object should be added
			m_bObjectTag = true;
			m_list[ m_cntList ].m_id = 0;
			m_list[ m_cntList ].m_catcnt = 0;
			m_list[ m_cntList ].m_name[ 0 ] = 0;
			m_list[ m_cntList ].m_longitude = 0;
			m_list[ m_cntList ].m_latitude = 0;
		}	

	}

	// information tags
	else {
		
		// all other tags
		strncpy( m_curtagname, name, sizeof( m_curtagname ) );
		memset( m_xmlbuf, 0, sizeof( m_xmlbuf ) );

	}
}


///////////////////////////////////////////////////////////////////////////////
// endElement


void
CControlObject::endElement( char *name )
{

	// xtur
	if ( 0 == strcmp( name, _T("xtur") ) ) {
		m_bStartTag = false;
	}

	// object - next object
	else if ( 0 == strcmp( name, _T("object") ) ) {

		m_bObjectTag = false;
		if ( m_cntList < MAX_ELEMENTS ) m_cntList++;
		
	}

	// information tags
	else if ( ( m_depth >= 3 ) && m_bStartTag && m_bObjectTag ) {

		// Must be room in the list
		if ( m_cntList < MAX_ELEMENTS ) {

			// Convert special chars 
			//convertFromHtmlSpecialChars( m_xmlbuf );
			
			// id
			if ( 0 == strcmp( m_curtagname, _T("id") ) ) { 
				m_list[ m_cntList ].m_id = atol( m_xmlbuf );					
			}
			
			// name
			else if ( 0 == strcmp( m_curtagname, _T("name") ) ) {
				
				if ( strlen( m_xmlbuf ) > 0 ) {					
					strncpy( m_list[ m_cntList ].m_name, m_xmlbuf, LIST_MAX_NAME_LEN );		 					 
				}				
			}			
			
			// typegroup
			else if ( 0 == strcmp( m_curtagname, _T("typegroup") ) ) {

				if ( m_list[ m_cntList ].m_catcnt < MAX_TYPE_GROUP ) {
					m_list[ m_cntList ].m_catcnt++;
				}
			}
			
			// cathegory

			else if ( 0 == strcmp( m_curtagname, _T("cat") ) ) {

				if ( m_list[ m_cntList ].m_catcnt < MAX_TYPE_GROUP ) {
					m_list[ m_cntList ].m_cathegory[ m_list[ m_cntList ].m_catcnt ] 
						= atol( m_xmlbuf );
				}
			}
			
			// subcathegory
			else if ( 0 == strcmp( m_curtagname, _T("subcat") ) ) {

				if ( m_list[ m_cntList ].m_catcnt < MAX_TYPE_GROUP ) {
					m_list[ m_cntList ].m_subcategory[ m_list[ m_cntList ].m_catcnt ] 
						= atol( m_xmlbuf );
				}
			}
						
			// longitude
			else if ( 0 == strcmp( m_curtagname, _T("lon") ) ) {
				char tmpbuf[ 32 ];
				m_list[ m_cntList ].m_longitude = atof( tmpbuf );
			}
			
			// latitude
			else if ( 0 == strcmp( m_curtagname, _T("lat") ) ) {

				char tmpbuf[ 32 ];
				m_list[ m_cntList ].m_latitude = atof( tmpbuf );
			}
			
			// distance 
			else if ( 0 == strcmp( m_curtagname, _T("dist") ) ) {
				m_list[ m_cntList ].m_distance = atol( m_xmlbuf );					
			}		
		}
	}

	*m_curtagname = 0;
	*m_xmlbuf;
	m_depth -= 1;
}


///////////////////////////////////////////////////////////////////////////////
//	characterHandler

void 
CControlObject::characterHandler( char *data, int len )
{
	// Calculate room left in buffer
	int sz  = sizeof( m_xmlbuf ) -  strlen( m_xmlbuf );

	if ( len < sizeof( m_xmlbuf ) ) {

		if ( sz > len ) {
			strncat( m_xmlbuf, data, len );
		}
		else {
			strncat( m_xmlbuf, data, sz );
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// parseXML
//

bool 
CControlObject::parseXML( char *pData, unsigned int len )
{
	bool rv = true;
	unsigned int pos = 0;
	int xml_state = XML_STATE_NONE; 
	char *pTag = NULL;
	char *pChar = NULL;
	wxString strStatus;
	int loopcnt = 0;

	// Must be valid pointers
	if ( NULL == pData ) return false;

	// Must be something to parse
	if ( 0 == len )  return false;

	// Must be something in buffer
	if ( 0 == *pData ) return false;

	while( pos < len ) {
		
		loopcnt++;
	 
		if ( loopcnt > 500 ) {
			//m_objectInfoWnd.GetWindowText( strStatus );
			strStatus += _T(".");
			//m_objectInfoWnd.SetWindowText( strStatus );
			loopcnt = 0;
		}

		switch ( xml_state ) {
		
		case XML_STATE_START:		// Start tag

			if ( pData[ pos ] == _T('>') ) {
				
				// We have parsed a full start tag
				pData[ pos ] = 0;
				startElement( pTag, NULL );
				xml_state = XML_STATE_DATA;
				pChar = pData + pos + 1;

			}
			break;

		case XML_STATE_DATA:		// data

			if ( ( pos < ( len - 1 ) ) && 
						pData[ pos ] == _T('<') && 
						pData[ pos + 1] == _T('/')) {
				// We have parsed a full data object
				xml_state = XML_STATE_END;
				pData[ pos ] = 0;
				pData++; // Move past '/'
				characterHandler( pChar,  strlen( pChar ) );
			}
			else if ( ( pos < ( len - 1 ) ) && 
						pData[ pos ] == _T('<') && 
						pData[ pos + 1] != _T('?') &&
						pData[ pos + 1] != _T('&') ) {
				// Nested start tag
				xml_state = XML_STATE_START;
				pTag = pData + pos + 1;
			}
			break;

		case XML_STATE_END:			// end tag

			if ( pData[ pos ] == _T('>') ) {
				// We have parsed a full end tag
				pData[ pos ] = 0;
				endElement( pTag );
				xml_state = XML_STATE_NONE;
				pChar = pData + pos + 1;
			}
			break;

		default: // no state
			if ( pData[ pos ] == _T('<') ) {

				if ( pos < ( len - 1 ) ) {
				
					if ( pData[ pos + 1 ] == _T('/') ) {
						xml_state = XML_STATE_END;
						pTag = pData + pos + 2;
						pData++;
					}
					else if ( ( pData[ pos + 1 ] != _T('!') ) &&
						( pData[ pos + 1 ] != _T('?') ) ) {
						// Not a comment, new state "start tag"
						xml_state = XML_STATE_START;
						pTag = pData + pos + 1;
					}

				}
				
			}
			break;
		}

		// Next character
		pos++;
	}

	return rv;
}

//////////////////////////////////////////////////////////////////////////////
// getXMLValue
//


bool 
CControlObject::getXMLValue( char *pBuf, 
								char *pTag, 
								char *pValue, 
								unsigned int maxlen )
{
	bool rv = false;
	char *pStart = NULL;			// Start tag
	char *pEnd = NULL;				// End tag
	char startTag[ TAG_MAX_SIZE ];
	char endTag[ TAG_MAX_SIZE ];

	if ( NULL == pBuf ) return false;
	if ( NULL == pTag ) return false;
	if ( NULL == pValue ) return false;
	if ( 0 >= maxlen ) return false;

	if ( strlen( pTag ) > TAG_MAX_SIZE - 3 ) return false;

	*startTag = 0;
	*endTag = 0;
	*pValue = 0;

	printf( startTag, _T("<%s>"), pTag );
	printf( endTag, _T("</%s>"), pTag );
	
	pStart = strstr( pBuf, startTag );
	pEnd = strstr( pBuf, endTag );

	if ( ( NULL != pStart ) && ( NULL != pEnd ) ) {
		
		char c;

		pStart += strlen( startTag );
		c = *pEnd;
		*pEnd = 0;
		
		if ( strlen( pStart ) <= maxlen ) {

			strcpy( pValue, pStart );
			rv = true;

		}

		*pEnd = c;

	}

	return rv;
}


//////////////////////////////////////////////////////////////////////////////
// getDataFromServer
//

bool 
CControlObject::getDataFromServer( void )
{
	//int nRetCode = 0;
	//DWORD errorCode = 0;
	bool rv = true;

	char szURL[ 256 ];
	//char wrkbuf[ 25 ];
	
	strcpy( szURL, m_szServerURL );
	strcat( szURL, _T("/lrfobjects.php?cmd=list1" ) );

	
	// Max distance
	//if ( 0 != m_maxDistance ) {
	//	wcscat( szURL, _T("&max=") );
	//	swprintf( wrkbuf, _T("%i"), m_maxDistance * 1000 );
	//	wcscat( szURL, wrkbuf );
	//}

	// Max items to display 
	//wcscat( szURL, _T("&limit=") );
	//swprintf( wrkbuf, _T("%d"), MAX_ITEMS_LISTED );
	//wcscat( szURL, wrkbuf );

	// Maximum number of objects to display m_maxDisplayItems
	//wcscat( szURL, _T("&limit=") );
	//swprintf( wrkbuf, _T("%d"), m_maxDisplayItems );
	//wcscat( szURL, wrkbuf );

 
	char* szXMLPage = new char[ MAX_WEBPAGE_SIZE + 1 ];
	DWORD cnt = 0;
	
	if ( NULL != szXMLPage ) { 
		
		if ( 0 < ( cnt = doHtmlPageFetch( szURL, szXMLPage, MAX_WEBPAGE_SIZE ) ) ) {

			// Initialize the parse object
			m_bObjectTag = false;
			m_bStartTag = false;
			m_depth = 0;
			memset (m_xmlbuf, 0, sizeof( m_xmlbuf ) );
			*m_curtagname = 0;

			parseXML( (char *)szXMLPage, cnt );		 

		} // Fetchpage
		
		delete [] szXMLPage;
	}

	return rv;

}



//////////////////////////////////////////////////////////////////////////////
// reportObjectToServer


bool 
CControlObject::reportObjectToServer( wxString *strName,  wxString *strDescription )
{
	//int nRetCode = 0;
	//DWORD errorCode = 0;
	//bool bFilter = false;
	bool rv = false;

/*	// Show the wait cursor
	CWaitCursor cw;
	

	char szURL[ 256 ];
	char wrkbuf[ 25 ];

	DWORD   dwAccessType = PRE_CONFIG_INTERNET_ACCESS;

	DWORD dwHttpRequestFlags =
		INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_NO_AUTO_REDIRECT;

	const char szHeaders[] =
		_T("Accept: text/*\r\nUser-Agent: XTUR Client\r\n");
	
 
	wcscpy( szURL, m_szServerURL );
	wcscat( szURL, _T("/pdanewobject.php?cmd=add" ) );

	if ( ( 0 != m_curLatitude ) && ( 0 != m_curLongitude ) ) {
		// Latitude
		wcscat( szURL, _T("&lat=") );
		swprintf( wrkbuf, _T("%f"), m_curLatitude );
		wcscat( szURL, wrkbuf );

		//Longitude
 		wcscat( szURL, _T("&lon=") );
		swprintf( wrkbuf, _T("%f"), m_curLongitude );
		wcscat( szURL, wrkbuf );
	}
	
	// Username
	wcscat( szURL, _T("&username=") );
	wcscat( szURL, m_UserName );
 
	// Password
	wcscat( szURL, _T("&password=") );
	wcscat( szURL, m_UserPassword );

	// Objectname
	wcscat( szURL, _T("&obname=") );
	wcscat( szURL, strName );

	// Objectdescription
	wcscat( szURL, _T("&obdescription=") );
	wcscat( szURL, strDescription );

	//AfxMessageBox( szURL );

	char* szHTMLResultPage = new char[ MAX_WEBPAGE_SIZE + 1 ];

	if ( NULL != szHTMLResultPage ) {

		if ( doHtmlPageFetch( szURL, szHTMLResultPage, MAX_WEBPAGE_SIZE ) ) {
			// OK we got a page - Check the result
			char *p;
			if ( NULL != ( p = wcsstr( szHTMLResultPage, _T("<result>OK</result>" ) ) ) ) {
				// Success
				m_objectInfoWnd.SetWindowText( _T("Det nya objektet har lagts till.") );
				rv = true;
			}
			else {
				// Failure
				char strResult[80];
				char strInfo[ 256 ];

				getXMLValue( szHTMLResultPage, 
								_T("reason"), 
								strResult, 
								sizeof( strResult ) );
 
				swprintf( strInfo, _T("Error: Misslyckades med att addera objekt.\r\n%s"), strResult );
 
				m_objectInfoWnd.SetWindowText( strInfo  );
				rv = false;
			}
		}
		delete [] szHTMLResultPage;
	}
	else {
		m_objectInfoWnd.SetWindowText(_T("Errro: To low memory to allocate HTML result buffer.") );	
		return false;
	}
*/		
	return rv;
}

/////////////////////////////////////////////////////////////////////////////
// doHtmlPageFetch
//

long 
CControlObject::doHtmlPageFetch( char *pszURL, char *pBuf, long bufsize )
{	
	// Validate arguments
	if ( NULL == pszURL ) return false;
	if ( NULL == pBuf ) return false;
	if ( bufsize < 1 ) return false;

/*	
	wxHtmlParser *parser = new wxHtmlParser();

	DWORD   dwAccessType = PRE_CONFIG_INTERNET_ACCESS;

	DWORD dwHttpRequestFlags =
		INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_NO_AUTO_REDIRECT;

	const char szHeaders[] =
		_T("Accept: text/*\r\nUser-Agent: XTUR Client\r\n");

	CClientSession session( _T("XTUR - Client"), dwAccessType );
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;

	// check to see if this is a reasonable URL
	CString strServerName;
	CString strObject;
	INTERNET_PORT nPort;
	DWORD dwServiceType;

	if ( !AfxParseURL( pszURL, dwServiceType, strServerName, strObject, nPort ) ||
			dwServiceType != INTERNET_SERVICE_HTTP ) {
		m_objectInfoWnd.SetWindowText(_T("URL för server skall inledas med http://.") );
		return -1;
	}

    // !!!

	pServer = session.GetHttpConnection( strServerName, nPort );

	pFile = pServer->OpenRequest( CHttpConnection::HTTP_VERB_GET,
									strObject, 
									NULL, 
									1, 
									NULL, 
									NULL, 
									dwHttpRequestFlags );

	m_objectInfoWnd.SetWindowText(_T("4 Kontaktar X-Tur Server...") );

	// !!!

	pFile->AddRequestHeaders( szHeaders );
	pFile->SendRequest();
	 
	m_objectInfoWnd.SetWindowText(_T("5 Verifierar uppkoppling...") );

	DWORD dwRet;
	pFile->QueryInfoStatusCode(dwRet);

 	m_objectInfoWnd.SetWindowText(_T("6 Access kontroll...") );

	// if access was denied, prompt the user for the password
	if ( dwRet == HTTP_STATUS_DENIED ) {
#ifdef _WIN32_WCE
		m_objectInfoWnd.SetWindowText(_T("Access denied: Invalid password\n") );
		return -1;			
#else
		DWORD dwPrompt;
		dwPrompt = pFile->ErrorDlg(NULL, ERROR_INTERNET_INCORRECT_PASSWORD,
						FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | 
						FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS, NULL );

		// if the user cancelled the dialog, bail out
		if ( dwPrompt != ERROR_INTERNET_FORCE_RETRY ) {
			m_objectInfoWnd.SetWindowText(_T("Access denied: Invalid password\n") );
			return -1;
		}

		pFile->SendRequest();
		pFile->QueryInfoStatusCode(dwRet);
#endif
	}


	CString strNewLocation;
	pFile->QueryInfo( HTTP_QUERY_RAW_HEADERS_CRLF, strNewLocation );

	m_objectInfoWnd.SetWindowText(_T("7 Begär information från server...") );

	// were we redirected?
	// these response status codes come from WININET.H

	if ( dwRet == HTTP_STATUS_MOVED ||
		 dwRet == HTTP_STATUS_REDIRECT ||
		 dwRet == HTTP_STATUS_REDIRECT_METHOD ) {
		
		CString strNewLocation;
		pFile->QueryInfo( HTTP_QUERY_RAW_HEADERS_CRLF, strNewLocation );

		m_objectInfoWnd.SetWindowText(_T("8 Flyttad till annan server...") );

		int nPlace = strNewLocation.Find(_T("Location: ") );
		if ( nPlace == -1 ) {
			m_objectInfoWnd.SetWindowText(_T("Error: Site redirects with no new location\n") );
			return -1;
		}

		strNewLocation = strNewLocation.Mid( nPlace + 10 );
		nPlace = strNewLocation.Find('\n');
		if ( nPlace > 0 ) {
			strNewLocation = strNewLocation.Left( nPlace );
		}

		// Close up the redirected site
		pFile->Close();
		delete pFile;
		pServer->Close();
		delete pServer;

		// Figure out what the old place was
		if ( !AfxParseURL( strNewLocation, 
							dwServiceType, 
							strServerName, 
							strObject, nPort)) {
			m_objectInfoWnd.SetWindowText(_T("Error: the redirected URL could not be parsed.\n") );
			return -1;
		}

		if ( dwServiceType != INTERNET_SERVICE_HTTP ) {
			m_objectInfoWnd.SetWindowText( _T("Error: the redirected URL does not reference a HTTP resource.\n") );
			return -1;
		}

		// try again at the new location
		pServer = session.GetHttpConnection( strServerName, nPort );
		pFile = pServer->OpenRequest( CHttpConnection::HTTP_VERB_GET,
										strObject, 
										NULL, 
										1, 
										NULL, 
										NULL, 
										dwHttpRequestFlags );

		pFile->AddRequestHeaders( szHeaders );
		pFile->SendRequest();

		pFile->QueryInfoStatusCode(dwRet);
		if ( dwRet != HTTP_STATUS_OK ) {
			m_objectInfoWnd.SetWindowText(_T("Error: Got status code .\n") );
			return -1;
		}

	 	m_objectInfoWnd.SetWindowText(_T("9 Kontakt med ny server...") );
	}

 	m_objectInfoWnd.SetWindowText(_T("10 Hämtar data...") );

	//
	// Receive the file contents and check the result
	//

	long cnt = 0; // Nothing fetched yet
	pBuf[0] = 0;
	char * sz = new char[ BUFFER_SIZE + 1 ];
    char *szwBuf = new char[ 1 * (BUFFER_SIZE + 1) ];
 	
 
	DWORD rcnt;
	CString strStatus;

	memset( pBuf, 0, bufsize );

	while ( rcnt = pFile->Read( sz, BUFFER_SIZE ) ) {

		memset( szwBuf, 0, 1 * sizeof( szwBuf ) );
			
		// Must have room for the data
		if ( ( cnt + rcnt ) >= (unsigned long)bufsize ) {
			break;
		}

		//memcpy( ( pBuf + cnt ), sz, rcnt );
		
		// Convert to Unicode
		long n;
		if ( !( n = MultiByteToWideChar( CP_ACP, 
								MB_PRECOMPOSED, 
								sz, 
								rcnt,
								szwBuf, 
								bufsize ) ) ) {
			AfxMessageBox( _T("Failure") );
		}

		//memcpy( ( pBuf + cnt ), szwBuf, rcnt );
		wcsncat( pBuf, szwBuf, n );
		cnt += rcnt;

		n = wcslen( pBuf );
		
		memset( sz, 0, sizeof( sz ) );

		// Give the user some status
		m_objectInfoWnd.GetWindowText( strStatus );
		strStatus += _T(".");
		m_objectInfoWnd.SetWindowText( strStatus );
	}

	//AfxMessageBox( pBuf );

	//char *pp;
	//pp = wcsstr( pBuf, _T("</xtur>") );
		
	if ( NULL != sz )  delete [] sz;
	if ( NULL != szwBuf )  delete [] szwBuf;

	// The page has been fetched clean up	
	pFile->Close();
	pServer->Close();

	if ( pFile != NULL ) {
		delete pFile;
	}

	if ( pServer != NULL ) {
		delete pServer;
	}
		
	session.Close();
		
	return cnt;
*/
	return 0;
} 