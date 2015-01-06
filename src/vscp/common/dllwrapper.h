///////////////////////////////////////////////////////////////////////////////
// dllwrapper.h: interface for the CDllWrapper class
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
//
//

#if !defined(AFX_DLLWRAPPER_H__66E4FA3F_1CA1_405D_AAF1_5F9B1272D75A__INCLUDED_)
#define AFX_DLLWRAPPER_H__66E4FA3F_1CA1_405D_AAF1_5F9B1272D75A__INCLUDED_

#include "wx/wx.h"
#include <wx/dynlib.h>
#include "canaldlldef.h"	// Holds the function declarations

/*!
	\file dllwrapper.h
	\brief This class encapsulates the CANAL dll/dl(so) interface. 
	\details The class have all 
	methods of the CANAL interface specification implemented in an easy to handle form.
	Use the class if you want to talk directly to CANAL dll/dl(so) drivers. A better
	choice may be to use CanalSuperWrapper wich can talk to both a dll/dl(so) and to a tvp/ip
	interface.
*/


// Default values for read/write register functions
// used in device config and scan.
#define DLL_REGISTER_READ_RESEND_TIMEOUT	5000
#define DLL_REGISTER_READ_ERROR_TIMEOUT		2000
#define DLL_REGISTER_READ_MAX_TRIES			3


/*!
	\class CDllWrapper
	\brief Encapsulates the CANAL dll/dl(so) interface.
	\details The class have all 
	methods of the CANAL interface specification implemented in an easy to handle form.
	Use the class if you want to talk directly to CANAL dll/dl(so) drivers. A better
	choice may be to use CanalSuperWrapper wich can talk to both a dll/dl(so) and to a tvp/ip
	interface.
*/


class CDllWrapper  
{

public:

	/// Constructor
    CDllWrapper();

	/// Destructor
    virtual ~CDllWrapper();
  
    /*!
        initialize the dll wrapper
    
        @param strPath to the canal dll
        @return	true on success
    */
    int initialize( wxString& strPath ); 
  
    /*!
        Open communication channel.
     
        @param strConfiguration is name of channel.
	    @param flags CANAL flags for the channel.
        @return Channel handler on success. -1 on error.
    */
    long doCmdOpen( const wxString& strConfiguration = (_("")), unsigned long flags = 0L );
  
  
    /*!
        Close communication channel
        @return CANAL_ERROR_SUCCESS on success or CANAL error if failure.
    */
    int doCmdClose( void );
  
  
    /*!
        Do a no operation test command. 
        @return CANAL_ERROR_SUCCESS on success or CANAL error if failure.
    */
    bool doCmdNOOP( void ) { return true; };
  
  
    /*!
        Get the Canal protocol level
        @return CANAL_ERROR_SUCCESS on success or CANAL error if failure.
    */
    unsigned long doCmdGetLevel( void );
  
    /*!
        Send a CAN message 
        @param pMsg Ponter to CAN message to send
        @return CANAL_ERROR_SUCCESS on success or CANAL error if failure.
    */
    int doCmdSend( canalMsg *pMsg );
  
	/*!
		Send a CAN message and block if it can't
		be sent right away.
		@param pMsg Ponter to CAN message to send
		@param timeout Time to wait in milliseconds or zero to wait forever.
		@return CANAL_ERROR_SUCCESS on success or CANAL error if failure.
					Return CANAL_ERROR_NOT_SUPPORTED if blocking operations is not
					supported.
	*/
	int doCmdBlockingSend( canalMsg *pMsg, unsigned long timeout );
  
    /*!
        Receive a CAN message. 
        @param pMsg Poniter to CAN message that receive received message.
        @return CANAL_ERROR_SUCCESS on success or CANAL error if failure.
    */
    int doCmdReceive( canalMsg *pMsg );
  
	/*!
        Receive a CAN message
	    @param pMsg Poniter to CAN message that receive received message.
        @param timeout Time to wait in milliseconds or zero to wait forever.
        @return CANAL_ERROR_SUCCESS on success or CANAL error if failure. 
					Return CANAL_ERROR_NOT_SUPPORTED if blocking operations is not
					supported.
    */
    int doCmdBlockingReceive( canalMsg *pMsg, unsigned long timeout );
  
    /*!
        Get the number of messages in the input queue
        @return the number of messages available or if negative
        an error code.
    */
    int doCmdDataAvailable( void );
  
    /*!
        Receive CAN status. 
        @return Return number of messages in inqueue.
    */
    int doCmdStatus( canalStatus *pStatus );
  
    /*!
        Receive CAN statistics. 
        @return CANAL_ERROR_SUCCESS on success or CANAL error if failure.
    */
    int doCmdStatistics( canalStatistics *pStatistics );
  
    /*!
        Set/Reset a filter. 
        @return CANAL_ERROR_SUCCESS on success or CANAL error if failure.
    */
    int doCmdFilter( const unsigned long filter );
  
    /*!
        Set/Reset a mask. 
        @return CANAL_ERROR_SUCCESS on success or CANAL error if failure.
    */  
    int doCmdMask( const unsigned long mask );
  
    /*!
        Set baudrate. 
        @return CANAL_ERROR_SUCCESS on success or CANAL error if failure.
    */
    int doCmdBaudrate( const unsigned long baudrate );
  
    /*!
        Get interface version. 
        @return Driver version.
    */
    unsigned long doCmdVersion( void );
  
    /*!
        Get dll version. 
        @return DLL version.
    */
    unsigned long doCmdDLLVersion( void );
  
    /*!
        Get vendorstring. 
        @return Pointer to vendor string..
    */
    const char *doCmdVendorString( void );

	/*!
		Get driver information
		@return Pointer to driver information string or NULL
			if no driver info is available.
	*/
	const char *doCmdGetDriverInfo( void );


	// Extended functionality


	/*!
		Check if blocking is supported
		@param bRead Set to true to check receive blocking
		@param bWrite Set to true to check send blocking
		@return Returns true if blocking is supported.
	*/
	bool isBlockingSupported( bool bRead = true, bool bWrite = true );
	
	
	
	///////////////////////////////////////////////////////////////////////////
	//                         V S C P  S p e c i f i c
	///////////////////////////////////////////////////////////////////////////
	
	
    /*!
	    Set read timeout
	    @param to Read timeout in milliseconds
	*/
	void setReadTimeout( unsigned long to ) { m_registerReadErrorTimeout = to; };

    /*!
	    Get read timeout
	    @return Read timeout in milliseconds
	*/
	unsigned long getReadTimeout( void) { return m_registerReadErrorTimeout; };

	/*!
	    Set read resend timeout
	    @param to Read resend timeout in milliseconds
	*/
	void setReadResendTimeout( unsigned long to ) { m_registerReadResendTimeout = to; };

    /*!
	    Get resend timeout
	    @return Resend timeout in milliseconds
	*/
	unsigned long getReadResendTimeout( void) { return m_registerReadResendTimeout; };

	/*!
	    Set max read retries
	    @param n Number of retries before giving up
	*/
	void setMaxRetries( unsigned short n ) { m_registerReadMaxRetries = n; };

    /*!
	    Get max read retries
	    @return Number of retries before giving up
	*/
	unsigned short getMaxRetries( void ) { return m_registerReadMaxRetries; };

	/*!
        Read level I register
        @param nodeid Nickname for node to read register on
        @param page Page to use
	    @param reg Register to read.
	    @param pval Pointer to value read
	    @return CANAL_ERROR_SUCCESS on success, errocode on failure.
    */
	int readLevel1Register( unsigned char nodeid,
                                unsigned short page,
                                unsigned char reg, 
                                unsigned char *pcontent );

    /*!
	    Write level I register
	    @param nodeid Nickname for node to write register on
        @param page Page to use
	    @param reg Register to write to.
	    @param val Register value to write
	    @return CANAL_ERROR_SUCCESS on success, errocode on failure.
	*/
	int writeLevel1Register( unsigned char nodeid,
                                unsigned short page,
								unsigned char reg, 
								unsigned char *pval );

    /*!
	    Load level I register content into an array
	    @param pwnd Pointer to window (owner usually this) that called this method.
	    @param pregisters Pointer to an array of 256 8-bit registers.
	    @param nodeid nodeid The node whos registers should be read.
	    @param bQuite No progress information if sett to true. (default is false)
	    @param startreg First register to read. Default is 0.
	    @param count Number of registers to read. Default is 256.
        @param canalreginfo_callback_t Gives dynamic  information about operation
	    @return true on success, false on failure.
	*/
	int readLevel1Registers( unsigned short page,
								unsigned char *pregisters,
								unsigned char nodeid,
								unsigned char startreg = 0,
								unsigned short count = 256,
								unsigned long idCallback = 0,
								canalreginfo_callback_t *pcallback = NULL );

    /*!
        Read MDF information from a Level I device
        @param id Nickname id for device  
        @param strurl MDF URL or empty string on failure.
        @return true on success.
    */
    bool getMDFfromLevel1Device( unsigned char nodeid, wxString &strurl );

    /*!
        Read a number of registers
        @param nodeid Node to read registers from
        @param reg Start register to start to read from.
        @param count Number of registers to read. Note: Max 128 bytes can be read!!!
        @param pRegs Pointer to array rhat will get result
        @return CANAL_ERROR_SUCCESS on success, error code on failure.
    */
    int getRegistersfromLevel1Device( unsigned char nodeid, 
                                            unsigned short reg,
                                            unsigned short page,
                                            unsigned char count,
                                            unsigned char *pRegs );

protected:

	/*!
		Path to CANAL driver
	*/
	wxString m_strPath;
  
	/// dl/dll handler
	wxDynamicLibrary m_wxdll;
  
	/// device id from open call
	long m_devid;
  
	/*!
		Flag that indicates that a sucessfull initialization
		has been performed.
	*/
	bool m_bInit;

    /// Error timeout for register read/write operations
    unsigned long m_registerReadErrorTimeout;

    /// Resend timeout for read/write operations
    unsigned long m_registerReadResendTimeout;

    /// Man number of read/write retries
    unsigned short m_registerReadMaxRetries;
  
	//@{
	/// CANAL methods
	LPFNDLL_CANALOPEN				m_proc_CanalOpen;
	LPFNDLL_CANALCLOSE				m_proc_CanalClose;
	LPFNDLL_CANALGETLEVEL			m_proc_CanalGetLevel;
	LPFNDLL_CANALSEND				m_proc_CanalSend;
	LPFNDLL_CANALRECEIVE			m_proc_CanalReceive;
	LPFNDLL_CANALDATAAVAILABLE		m_proc_CanalDataAvailable;
	LPFNDLL_CANALGETSTATUS			m_proc_CanalGetStatus;
	LPFNDLL_CANALGETSTATISTICS		m_proc_CanalGetStatistics;
	LPFNDLL_CANALSETFILTER			m_proc_CanalSetFilter;
	LPFNDLL_CANALSETMASK			m_proc_CanalSetMask;
	LPFNDLL_CANALSETBAUDRATE		m_proc_CanalSetBaudrate;
	LPFNDLL_CANALGETVERSION			m_proc_CanalGetVersion;
	LPFNDLL_CANALGETDLLVERSION		m_proc_CanalGetDllVersion;
	LPFNDLL_CANALGETVENDORSTRING	m_proc_CanalGetVendorString;
	// Generation 2
	LPFNDLL_CANALBLOCKINGSEND		m_proc_CanalBlockingSend;
	LPFNDLL_CANALBLOCKINGRECEIVE	m_proc_CanalBlockingReceive;
	LPFNDLL_CANALGETDRIVERINFO	m_proc_CanalGetdriverInfo;
	//@}
};

#endif // !defined(AFX_DLLWRAPPER_H__66E4FA3F_1CA1_405D_AAF1_5F9B1272D75A__INCLUDED_)
