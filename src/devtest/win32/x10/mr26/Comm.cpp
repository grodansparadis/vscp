/*! 
 *	\file COMComm.cpp
 *  \brief interface for the Linux comm class.
 *  \author Ake Hedman, eurosource, 
 *  \author akhe@users.sourceforge.net 
 *  \date    2002-2004
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 * 
 * This file is part of M.U.M.I.N. Multi Unit Microcontroler 
 * Interface Network which you can download from 
 *	http://www.eurosource.se
 *
 * Copyright (C) 2000-2004 Ake Hedman, eurosource, <akhe@users.sourceforge.net>
 * 
 * mumin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with M.U.M.I.N. see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * This class wraps and extends Minicom.
 * see ftp://ftp.sunsite.unc.edu/pub/Linux/apps/serialcomm/dialout
 *
 * Some changes by akhe@eurosource.se ( http://www.eurosource.se ) done for the
 * M.U.M.I.N. protocol.
 * 
 * - Sorry no more sysdep. Works only on Linux systems ( maybe on others ).
 *
 */

/*! 
 *  \file COMComm.cpp
 *  \brief  Implementation of the CCOMComm class..
 *  \author Ake Hedman, eurosource, akhe@eurosource.se
 *  \date   2002-2004
 */
 
#include "stdafx.h"
#include "Comm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

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
	DCB dcbCommPort;

	// Check com-port data
	if ( nComPort!= 0 ) {
		sprintf( szComPort, "COM%d", nComPort );
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
	ct.ReadIntervalTimeout = 200;		
	ct.ReadTotalTimeoutMultiplier = 0;		
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
	dcbCommPort.fDsrSensitivity = false;
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

	return true;
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
	if ( 0 == result ) {	
		int err = GetLastError();
		dwCount = 0;
	}

	*cnt = dwCount;
	return c;
}


/////////////////////////////////////////////////////////////////////////////
// write
//

void CComm::write( char* String, bool bCRLF )
{
	DWORD dwCount;
	char c;
	
	for (int i = 0;i < ((signed)strlen(String)); i++) {
		WriteFile( m_hCommPort, &String[i], 1, &dwCount, NULL );
	}

	if ( bCRLF ) {
		c = 13;
		WriteFile( m_hCommPort, &c, 1, &dwCount, NULL );
	
		c = 10;
		WriteFile( m_hCommPort, &c, 1, &dwCount, NULL );
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
// close
//

void CComm::close()
{
	CloseHandle( m_hCommPort );
	m_hCommPort = NULL;
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




