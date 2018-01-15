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

#if !defined(AFX_COMCOMM_H__E2F5A961_FF34_437B_91E1_8D7DEA1E94C9__INCLUDED_)
#define AFX_COMCOMM_H__E2F5A961_FF34_437B_91E1_8D7DEA1E94C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define HANDSHAKE_NONE      0
#define HANDSHAKE_HARDWARE  1
#define HANDSHAKE_SOFTWARE  2

/*! 
 *	\class CCommParams
 *  \brief This class hat encapsulate communication parameters
 *	\brief for a serial communication port.
 *  \author Ake Hedman, eurosource, akhe@eurosource.se
 *  \version 1
 *  \date    2002-2005
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
 *  \date    2002-2005
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
        Read a number of bytes from the port (if available) within a 
        certain time.

        \param  Pointer to buffer that receives data.
        \param	Size for buffer.
        \param  cntRead nr of character we want or if a negative number return
                when CR is found.
        \return Number of characters read or zero on timeout.
    */
    unsigned short readBuf( char *p, 
                            unsigned short size, 
                            short cntRead = -1, 
                            unsigned long timeout = 500 );

    /*!
        Write a string of charcters to the communication port.

        @param str String to send.
        @param bCRLF If true a CRLF will be sent after the string has been sent.
        @param bNoLF If true only CR is sent instead of CRLF.
    */
    void write( char *str, bool bCRLF = false, bool bNoLF = false );

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
        sendCmd

    */
    bool sendCommand( char * pCmd, char *pResponse,  unsigned long timeout = 500 );

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
