/*! 
 *	\file Comm.h
 *  \brief interface for the Linux comm class.
 *  \author Ake Hedman, eurosource, 
 *  \author akhe@users.sourceforge.net 
 *  \date   2002-2004
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

#if !defined(AFX_COMCOMM_H__E2F5A961_FF34_437B_91E1_8D7DEA1E94C9__INCLUDED_)
#define AFX_COMCOMM_H__E2F5A961_FF34_437B_91E1_8D7DEA1E94C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define HANDSHAKE_NONE		0
#define HANDSHAKE_HARDWARE	1
#define HANDSHAKE_SOFTWARE	2

/*! 
 *	\class CCommParams
 *  \brief This class hat encapsulate communication parameters
 *	\brief for a serial communication port.
 *  \author Ake Hedman, eurosource, akhe@eurosource.se
 *  \version 1
 *  \date    2002-2004
 */

class CCommParams
{

public:
	
	/// The port number to use
	unsigned char m_nPort;
	
	/// The baudrate to use
	DWORD m_nBaudRate;
	
	/// # of stopbits to use
	unsigned char m_nDatabits;
	
	/// Patity to use
	unsigned char m_nParity;

	/// # of stopbits to use
	unsigned char m_nStopbits;

};

/*! \class CComm
 *  \brief A class that handles serial port communication.
 *  \author Ake Hedman, eurosource, akhe@eurosource.se
 *  \version 1
 *  \date    2002-2004
 */
 

class CComm
{

public:

	/// Constructor
	CComm();

	/// Destructor
	virtual ~CComm();

	/*! 
		Initialise the serial communication port

		\param nComPort Index for communication port (default to COM1).
		\param nBaudRate Baudrate for the selected port (defaults to 9600).
		\param nDatabits Number of databits ( defeaults to 8 ).
		\param nParity Parity (defaults to no parity ).
		\param nStopbits Number of stopbits (defaults to one stopbit).
		\return True on success.
	*/
	bool init( unsigned char nComPort=1, 
							DWORD nBaudRate = CBR_9600, 
							unsigned char nDatabits = 8, 
							unsigned char nParity= NOPARITY, 
							unsigned char nStopbits=ONESTOPBIT,
							unsigned char nHandshake = HANDSHAKE_NONE );

	/*!
		Read a byte from the port (if one is available).

		\param cnt pointer to variable holding number of characters read. 
		\return Character read from comm port depending on param
	*/
	char readChar( int* cnt );

	/*!
		Write a string of charcters to the communication port.

		@param str String to send.
		@param bCRLF If true a CRLF will be sent after the string has been sent.
	*/
	void write( char *str, bool bCRLF = false );

	/*!
		Write a buffer to the channel

		\param p Pointer to buffer that holds data that should be written.
		\param cnt Number of bytes to send.
		\return If the function succeeds, the return value is nonzero.
		\return	If the function fails, the return value is zero.  
	*/
	BOOL writebuf( unsigned char * p, unsigned short cnt );

	/*!
		Send a byte to the communication port.

		\param b byte to send.
	*/
	void writeChar( unsigned char b );

	/*!
		Drain the input queue
	*/
	void drainInput( void );
	
	/*!
		Close the communication channel.
	*/
	void close( void );

	/*!
		Get the handle to the communication port.

		\return handle to an open communication port or NULL if
		\return	the port is't open.
	*/
	HANDLE getHandle( void ) { return m_hCommPort; };

private :	
	
	/*!
		Handle to the communication port
	*/
	HANDLE m_hCommPort;

};

#endif // !defined(AFX_COMCOMM_H__E2F5A961_FF34_437B_91E1_8D7DEA1E94C9__INCLUDED_)
