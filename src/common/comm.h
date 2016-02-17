/*! 
 *	\file Comm.h
 *  \brief interface for the Linux comm class.
 *  \author Ake Hedman, Grodans Paradis AB, 
 *  \author <akhe@grodansparadis.com>
 *  \date   2002-2014
 *
*/

///////////////////////////////////////////////////////////////////////////////
// Comm.h
//
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2014 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
// $RCSfile: Comm.h,v $                                       
// $Date: 2005/08/30 11:00:13 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 
///////////////////////////////////////////////////////////////////////////////

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
