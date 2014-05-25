///////////////////////////////////////////////////////////////////////////////
// COMComm.cpp:  
//
// Copyright (C) 2000-2014 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "stdio.h"
#include "com_win32.h"
 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Constructor
//

CComm::CComm()
{
	m_hCommPort = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
//

CComm::~CComm()
{
	if ( NULL != m_hCommPort ) {
		CloseHandle( m_hCommPort );
	}
}


/////////////////////////////////////////////////////////////////////////////
// init
//

bool CComm::init( unsigned char nComPort, 
						DWORD nBaudRate, 
						unsigned char nDatabits, 
						unsigned char nParity, 
						unsigned char nStopbits,
						unsigned char nHandshake )
{
	char szComPort[10];
	COMMTIMEOUTS ct;
	DCB dcbCommPort;	// ONESTOPBIT

	// Check com-port data
	if ( nComPort!= 0 ) {
		sprintf( szComPort, "\\\\.\\COM%d", nComPort );
	}
	else {
		return false;	
	}

	if ( INVALID_HANDLE_VALUE == 
			( m_hCommPort = CreateFile( szComPort, 
										GENERIC_READ | GENERIC_WRITE,
										0, 
										NULL, 
										OPEN_EXISTING, 
										FILE_ATTRIBUTE_NORMAL, 
										NULL ) ) ) {
		return false;
	}
	
	memset( &ct, 0, sizeof( ct ) );

	// Set timeouts
	ct.ReadIntervalTimeout = MAXDWORD;			// AKHE Changed fron 200
	ct.ReadTotalTimeoutMultiplier = MAXDWORD;	// AKHE Changed from 0
	ct.ReadTotalTimeoutConstant = 200;		
	ct.WriteTotalTimeoutMultiplier = 0;		
	ct.WriteTotalTimeoutConstant = 200;	
	SetCommTimeouts( m_hCommPort, &ct );
	
	// Comm state
	if ( !GetCommState( m_hCommPort, &dcbCommPort ) ) {
		CloseHandle( m_hCommPort );
		return false;
	}
	
	dcbCommPort.DCBlength = sizeof( DCB );
	dcbCommPort.BaudRate = nBaudRate;	
	if ( nParity != NOPARITY ) {
		dcbCommPort.fParity = true;
	}
	else {
		dcbCommPort.fParity = false;
	}

	dcbCommPort.Parity = nParity;
	dcbCommPort.fDtrControl = DTR_CONTROL_ENABLE;
	dcbCommPort.fDtrControl = DTR_CONTROL_DISABLE;
	//dcbCommPort.fDsrSensitivity = DTR_CONTROL_DISABLE;
	dcbCommPort.ByteSize = nDatabits;
	dcbCommPort.StopBits = nStopbits;

	if (  HANDSHAKE_HARDWARE == nHandshake ) {
		
		// Software flow control
		dcbCommPort.fOutX = FALSE;
		dcbCommPort.fInX = FALSE;
	
		// Hardware flow control
		dcbCommPort.fOutxCtsFlow = TRUE;
		dcbCommPort.fOutxDsrFlow = TRUE;	

	}
	else if (  HANDSHAKE_SOFTWARE == nHandshake ) {
		
		// Software flow control
		dcbCommPort.fOutX = TRUE;
		dcbCommPort.fInX = TRUE;
	
		// Hardware flow control
		dcbCommPort.fOutxCtsFlow = FALSE;
		dcbCommPort.fOutxDsrFlow = FALSE;
		
	}
	else { //  HANDSHAKE_NONE 

		// Software flow control
		dcbCommPort.fOutX = FALSE;
		dcbCommPort.fInX = FALSE;
	
		// Hardware flow control
		dcbCommPort.fOutxCtsFlow = FALSE;
		dcbCommPort.fOutxDsrFlow = FALSE;
	}
	
	if ( !SetCommState( m_hCommPort, &dcbCommPort ) )  {
		CloseHandle( m_hCommPort );
		return false;
	}

	//COMMPROP prop;
	//GetCommProperties( m_hCommPort, &prop );

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// close
//

void CComm::close()
{
	CloseHandle( m_hCommPort );
	m_hCommPort = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// readChar
//

char CComm::readChar( int* cnt )
{
	char c = 0;
	DWORD dwCount;

	int result = 0;

	result = ReadFile( m_hCommPort, &c, 1, &dwCount, NULL );
	if (result == 0) {	
		int err = GetLastError();
	}

	*cnt = dwCount;
	return c;
}





/////////////////////////////////////////////////////////////////////////////
// write
//

void CComm::write( char* String, bool bCRLF, bool bNoLF )
{
	DWORD dwCount;
	char c;
	
	//for (int i = 0;i < ((signed)strlen(String)); i++) {
	WriteFile( m_hCommPort, String, strlen( String ), &dwCount, NULL );
	//}

	if ( bCRLF ) {
		c = 13;
		WriteFile( m_hCommPort, &c, 1, &dwCount, NULL );
	
		if ( !bNoLF ) {
			c = 10;
			WriteFile( m_hCommPort, &c, 1, &dwCount, NULL );
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// write
//

BOOL CComm::writebuf( unsigned char * p, unsigned short cnt )
{
	DWORD dwCount;
	return WriteFile( m_hCommPort, p, cnt, &dwCount, NULL ); 
}


/////////////////////////////////////////////////////////////////////////////
// writeChar
//

void CComm::writeChar( unsigned char b )
{
	DWORD dwCount;
	 
	WriteFile( m_hCommPort, &b, 1, &dwCount, NULL );
}






/////////////////////////////////////////////////////////////////////////////
// drainInput
//

void CComm::drainInput()
{
	int cnt;
	unsigned char c;
	
	do {
		c = readChar( &cnt );
	
	} while ( cnt );
}


/////////////////////////////////////////////////////////////////////////////
// sendCommand
//

bool CComm::sendCommand( char * pCmd, char *pResponse,  unsigned long timeout )
{
	bool rv = false;
	DWORD tstart;	
	int cnt;
	char buf[256];
	unsigned short pos = 0;

	memset( buf, 0, sizeof( buf )  );	

	drainInput();
	write( pCmd, true, true );
	tstart = GetTickCount();
	while ( ( GetTickCount() - tstart ) < timeout ) {
		buf[ pos ] = readChar ( &cnt );
		if ( cnt ) {			
			if ( NULL != strstr( buf, pResponse ) ) return true;
			pos++;
			if ( pos > sizeof( buf ) ) break;
		}
	}

	return rv;
}

/////////////////////////////////////////////////////////////////////////////
// readBuf
//

unsigned short CComm::readBuf( char *pBuf, 
								unsigned short size, 
								short cntRead, 
								unsigned long timeout )
{
	char *p = pBuf;
	DWORD tstart;	
	int cnt;
	unsigned short pos = 0;

	memset( pBuf, 0, size );
	tstart = GetTickCount();
	while ( ( GetTickCount() - tstart ) < timeout ) {
		p[ pos ]  = readChar ( &cnt );
		if ( cnt ) {			
			if ( ( -1 == cntRead ) && ( 0x0d == p[ pos ] ) ) return true;
			pos++;
			if ( pos > size ) break;			
		}
	}

	return pos;
}	