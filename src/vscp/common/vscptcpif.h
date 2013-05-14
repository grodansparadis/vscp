///////////////////////////////////////////////////////////////////////////////
// vscptcpif.h:
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2013 
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
///////////////////////////////////////////////////////////////////////////////


/*!
	\file vscptcpif.h
	\brief	The VscpTcpIf class encapuslate sthe VSCP tcp/ip interface.
	\details The VscpTcpIf class holds functionality to talk toi the tcp/ip
	of the VSCP daemon and on client that implement the interface. All api's
	of the interface are supported by the class.
*/


#if !defined(AFX_VSCPTCPIF_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)
#define AFX_VSCPTCPIF_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_


#include "canal.h"
#include "vscp.h"
#include "guid.h"
#include "vscphelper.h"

#include "wx/socket.h"
#include "wx/datetime.h"

//---------------------------------------------------------------------------



/*! 
	\def DEFAULT_RESPONSE_TIMEOUT
	Default response timeout for communication with the
	tcp/ip interface
*/
#define DEFAULT_RESPONSE_TIMEOUT 		2

/*!
	\def TCPIP_DLL_VERSION
	Pseudo version string
*/
#define TCPIP_DLL_VERSION				0x00000001
/*! 
	\def TCPIP_VENDOR_STRING
	Pseudo vendor string
*/
#define TCPIP_VENDOR_STRING	  			(_("Grodans Paradis AB, Sweden"))

/// Receive queue
WX_DECLARE_LIST( vscpEvent, EVENT_RX_QUEUE );
/// Transmit queue
WX_DECLARE_LIST( vscpEvent, EVENT_TX_QUEUE );


/*!
  @brief Class for VSCP daemon tcp/ip interface
*/
class VscpTcpIf  
{

public:

    /// Constructor
    VscpTcpIf();

    /// Destructor
    virtual ~VscpTcpIf();

public:

    /*!
        checkReturnValue
        \return Return false for "-OK" and true for "+OK"
    */
    bool checkReturnValue( void );

    /*!
        Do command
    */
    bool doCommand( uint8_t cmd );

    /*!
        Open communication interface.
        \param strInterface should contain "username;password;ip-addr;port" if used. All including 
            port are optional and defaults to no username/password, server as "localhost" and "9598" 
            as default port.
        \param flags are not used at the moment.
        \return CANAL_ERROR_SUCCESS if channel is open or CANAL error code  if error 
        or the channel is already opened or other error occur.
    */
    long doCmdOpen( const wxString& strInterface = (_("")), uint32_t flags = 0L );

    /*!
        Open communication interface.
        \param strHostname Host to connect to.
        \param port TCP/IP port to use.
        \param strUsername Username.
        \param strPassword Username.
        \return CANAL_ERROR_SUCCESS if channel is open or CANAL error code if error 
            or the channel is already opened or other error occur.
    */
    long doCmdOpen( const wxString& strHostname, 
	    				const short port, 
		    			const wxString& strUsername, 
			    		const wxString& strPassword );

    /*!
        Close communication interface
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdClose( void );

    /*!
        Write NOOP message through pipe. 
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdNOOP( void );

	/*!
		Clear input queue
		\return CANAL_ERROR_SUCCESS on success and error code if failure.
	*/
	int doCmdClear( void );


    /*!
        Get the Canal protocol level
        \return VSCP Level
    */
    unsigned long doCmdGetLevel( void ) { return CANAL_LEVEL_STANDARD; }


    /*!
        Send a VSCP Level II event through interface. 
        If a GUID sent with all items set to zero the GUID of the
        interface ("-") will be used.
        \param pEvent VSCP Level II event to send.		
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdSend( const vscpEvent *pEvent );

    /*!
        Send a VSCP ex event through interface. 
        \param pEvent VSCP Level II ex event to send.	
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdSendEx( const vscpEventEx *pEvent );

    /*!
        Send a Level I event through interface. 
        \param pMsg VSCP Level I event (==canalMsg).	
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdSendLevel1( const canalMsg *pMsg );

    /*!
        Receive a VSCP event through the interface. 
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdReceive( vscpEvent *pEvent );

    /*!
        Receive an VSCP ex event through the interface. 
        \return true if success false if not.
    */
    int doCmdReceiveEx( vscpEventEx *pEvent );

    /*!
        Receive an VSCP Level I event through the interface.
        For the extended and the RTR bit to be handled the
        CAN message bit in the head byte must be set.
        Low eight bits of the CAN id is fetched from GUID[15]
        that is LSB of GUID.
        \return CANAL_ERROR_SUCCESS if success false if not.
    */
    int doCmdReceiveLevel1( canalMsg *pCanalMsg );

    /*!
        This command sets an open interface in the receive loop (RCVLOOP). 
        It does nothing other then putting the interface in the loop mode and 
        checking that it went there.

        Note! The only way to terminate this receive loop is to close the session with 
        doCmdClose (or just close the socket).

        \return CANAL_ERROR_SUCCESS if success CANAL_ERROR_GENERIC if not.
    */
    int doCmdEnterReceiveLoop( void );

    /*!
        Receive an event 
        The receiveloop command must have been issued for this method to work as 
        it sets up the ringbuffer used for the blocking receive.
        \param pEvent Pointer to VSCP event.
        \param timeout Timout to wait for data. Default = 500 milliseconds.
        \return CANAL_ERROR_SUCCESS when event is received. CANAL_ERROR_FIFO_EMPTY is
        returned if no event was available. CANAL_ERROR_TIMEOUT on timeout.CANAL_ERROR_COMMUNICATION
        is returned if a socket error is detected.
    */
    int doCmdBlockReceive( vscpEvent *pEvent, uint32_t timeout = 500 );

    /*!
        Get the number of events in the input queue of this interface
        \return the number of events available or if negative
        an error code.
    */
    int doCmdDataAvailable( void );

    /*!
        Receive CAN status through the interface. 
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdStatus( canalStatus *pStatus );

    /*!
        Receive CAN statistics through the interface. 
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdStatistics( canalStatistics *pStatistics );

    /*!
        Set/Reset a filter through the interface. 
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdFilter( const vscpEventFilter *pFilter );

    /*!
        Set/Reset filter through the interface. 
        \param filter Filter on string form (priority,class,type,guid).
        \param mask Mask on string form (priority,class,type,guid).
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdFilter( const wxString& filter, const wxString& mask );

    /*!
        Get i/f version through the interface. 
        \return version number.
    */
    unsigned long doCmdVersion( void );

    /*!
        Get interface version
    */
    unsigned long doCmdDLLVersion( void );


    /*!
        Get vendor string
    */
    const char * doCmdVendorString( void );

    /*!
        Get Driver information string.
    */
    const char * doCmdGetDriverInfo( void );

    /*!
        Get GUID for this interface.  
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdGetGUID( char *pGUID );

    /*!
        Set GUID for this interface.  
        \param pGUID Array with uint8_t GUID's. (Note! Not a string).
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdSetGUID( const char *pGUID );

    /*!
        Get information about a channel..  
        \param pChannelInfo The stucture that will get the information
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdGetChannelInfo( VSCPChannelInfo *pChannelInfo );

    /*!
        Get Channel ID for the open channel
        \param pChannelID Channel ID for channel
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdGetChannelID( uint32_t *pChannelID );

    /*!
        Get available interfaces
        \param array A string array that will get interface list
        \return CANAL_ERROR_SUCCESS on success and error code if failure.
    */
    int doCmdInterfaceList( wxArrayString& array );

    /*!
        Dummy for Baudrate setting
    */
    int doCmdSetBaudrate(  uint32_t baudrate ) 
                { baudrate = baudrate; return CANAL_ERROR_SUCCESS; };

    /*!
        Dummy for filter setting
    */
    int doCmdFilter( uint32_t filter )
                { filter= filter; return CANAL_ERROR_SUCCESS; };

    /*!
        Dummy for mask setting
    */
    int doCmdMask( uint32_t mask )
                        { mask = mask; return CANAL_ERROR_SUCCESS; };

	/*!
        Shutdown the VSCP daemon  
    */
    int doCmdShutDown( void );


    // ------------------------------------------------------------------------
    //                    V A R I A B L E  H A N D L I N G
    // ------------------------------------------------------------------------



    /*!
        Get variable value from string variable
        \param name of variable
        \param strValue pointer to string that get the value of the string variable.
        \return true if the variable is of type string and the operation
        was successfull.
    */
    bool getVariableString( wxString& name, wxString *strValue );

    /*!
        Set variable value from string variable
        \param name of variable
        \param strValue value to set string variable to.
        \return true if the variable is of type string and the operation
        was successfull.
    */
    bool setVariableString( wxString& name, const wxString& strValue );


    /*!
        Get variable value from boolean variable
        \param name of variable
        \param bValue pointer to boolean variable that get the value of the string variable.
        \return true if the variable is of type bool and the operation
        was successfull.
    */
    bool getVariableBool( wxString& name, bool *bValue );

    /*!
        Set variable value from boolean variable
        \param name of variable
        \param bValue boolean variable with the value to set.
        \return true if the variable is of type bool and the operation
        was successfull.
    */
    bool setVariableBool( wxString& name, const bool bValue );

    /*!
        Get variable value from integer variable
        \param name of variable
        \param value pointer to integer variable that get the value of the string variable.
        \return true if the variable is of type integer and the operation
        was successfull.
    */
    bool getVariableInt( wxString& name, int *value );

    /*!
        Set variable value from integer variable
        \param name of variable. 
        \param value integer variable with the value to set.
        \return true if the variable is of type integer and the operation
        was successfull.
    */
    bool setVariableInt( wxString& name, int value );

    /*!
        Get variable value from long variable
        \param name of variable
        \param value pointer to long variable that get the value of the string variable.
        \return true if the variable is of type long and the operation
        was successfull.
    */
    bool getVariableLong( wxString& name, long *value );

    /*!
        Set variable value from long variable
        \param name of variable
        \param value long variable with the value to set.
        \return true if the variable is of type long and the operation
        was successfull.
    */
    bool setVariableLong( wxString& name, long value );

    /*!
        Get variable value from double variable
        \param name of variable
        \param value double variable with the value to set.
        \return true if the variable is of type double and the operation
        was successfull.
    */
    bool getVariableDouble( wxString& name, double *value );

    /*!
        Set variable value from double variable
        \param name of variable
        \param value The double value to set.
        \return true if the variable is of type double and the operation
        was successfull.
    */
    bool setVariableDouble( wxString& name, double value );

    /*!
        Get variable value from measurement variable
        \param name of variable
        \param strValue String that get that get the 
        value of the measurement.
        \return true if the variable is of type measurement and the operation
        was successfull.
    */
    bool getVariableMeasurement( wxString& name, wxString& strValue );

    /*!
        set variable value from double variable
        \param name of variable
        \param strValue pointer to double variable that get the value of the string variable.
        \return true if the variable is of type double and the operation
        was successfull.
    */
    bool setVariableMeasurement( wxString& name, wxString& strValue );

    /*!
        Get variable value from event variable
        \param name of variable
        \param pEvent pointer to event variable that get the value of the string variable.
        \return true if the variable is of type VSCP event and the operation
        was successfull.
    */
    bool getVariableEvent( wxString& name, vscpEvent *pEvent );

    /*!
        set variable value from VSCP event
        \param name of variable
        \param pEvent pointer to event that is used to set the variable.
        \return true if the operation was successfull.
    */
    bool setVariableEvent( wxString& name, vscpEvent *pEvent );

    /*!
        Get variable value from event variable
        \param name of variable
        \param pEvent pointer to event variable that get the value of the string variable.
        \return true if the variable is of type VSCP event and the operation
        was successfull.
    */
    bool getVariableEventEx( wxString& name, vscpEventEx *pEvent );

    /*!
        set variable value from VSCP event
        \param name of variable
        \param pEvent pointer to event that is used to set the variable.
        \return true if the operation was successfull.
    */
    bool setVariableEventEx( wxString& name, vscpEventEx *pEvent );

    /*!
        Get variable value from GUID variable
        \param name of variable
        \param pGUID pointer to event variable that get the value of the GUID variable.
        \return true if the variable is of type VSCP GUID and the operation
        was successfull.
    */
    bool getVariableGUID( wxString& name, cguid& pGUID );

    /*!
        set variable value from GUID
        \param name of variable
        \param pGUID pointer to GUID that is used to set the variable.
        \return true if the operation was successfull.
    */
    bool setVariableGUID( wxString& name, cguid& pGUID );

    /*!
        Get variable value from VSCP data variable
        \param name of variable
        \param psizeData pointer to variable that will hold the size of the data array
        \param pData pointer to VSCP data array variable (unsigned char [8] ) that get the 
        value of the string variable.
        \return true if the variable is of type VSCP data and the operation
        was successfull.
    */
    bool getVariableVSCPdata( wxString& name, uint16_t *psizeData, uint8_t *pData );

    /*!
        set variable value from VSCP data
        \param name of variable.
        \param sizeData Size of data.
        \param pData Pointer to data array to set data from.
        \return true if the operation was successfull.
    */
    bool setVariableVSCPdata( wxString& name, uint16_t sizeData, uint8_t *pData );

    /*!
        Get variable value from class variable
        \param name of variable
        \param vscp_class pointer to int that get the value of the class variable.
        \return true if the variable is of type VSCP class and the operation
        was successfull.
    */
    bool getVariableVSCPclass( wxString& name, uint16_t *vscp_class );

    /*!
        set variable value from vscp_class.
        \param name of variable.
        \param vscp_class to write to variable.
        \return true if the operation was successfull.
    */
    bool setVariableVSCPclass( wxString& name, uint16_t vscp_class );

    /*!
        Get variable value from type variable
        \param name of variable
        \param vscp_type pointer to int that get the value of the type variable.
        \return true if the variable is of type VSCP type and the operation
        was successfull.
    */
    bool getVariableVSCPtype( wxString& name, uint8_t *vscp_type );


    /*!
        set variable value from vscp_type.
        \param name of variable.
        \param vscp_type to write to variable.
        \return true if the operation was successfull.
    */
    bool setVariableVSCPtype( wxString& name, uint16_t vscp_type );



 // ------------------------------------------------------------------------
 //                           B I N A R Y  M O D E
 // ------------------------------------------------------------------------



    /*!
        Go to binary mode
        \return true if success false if not.
    */
    int doCmdBinary( void );

    /*!
        Send a VSCP Level II event through binary interface. 
        \param pEvent VSCP Level II event to send.		
        \return true if success false if not.
    */
    int doBinarySend( vscpEvent *pEvent );

    /*!
        Send a VSCP Level II ex event through binary interface. 
        \param pEvent VSCP Level II ex event.	
        \return true if success false if not.
    */
    int doBinarySendEx( vscpEventEx *pEvent );

    /*!
        Get a VSCP event from a line of data from the server
        \param strLine String with server event data line
        \param pEvent Pointer to VSCP event.
        \return true on success.
    */
    bool getEventFromLine( const wxString& strLine, vscpEvent *pEvent );

    /*!
        Set response timeout
        \param to Timeout value in seconds. (Default = 2 seconds.)
    */
    void setResponseTimeout( uint8_t to ) { if ( to ) m_responseTimeOut = to; };

    /*!
        Receive a VSCP Level II event through binary interface. 
        \param pEvent VSCP Level II event.	
        \return true if success false if no event available.
    */
    int doBinaryReceive( vscpEvent *pEvent );

    /*!
        Receive a VSCP Level II event through binary interface. 
        \param pEvent VSCP Level II ex event.	
        \return true if success false if no event available.
    */
    int doBinaryReceiveEx( vscpEventEx *pEvent );


    /*!
        Write a binary VSCP event
        \param type Frame type.
        \param buffer data to send.
        \param nbytes Number of bytes to send.
        \return true on success, false on failure
    */
    int Binary_WriteMsg( uint8_t type, const void *buffer, wxUint32 nbytes );

    /*!
        Read a binary VSCP event
        \param pType Receives the frame type.
        \param buffer to receive data in 
        \param nbytes buffer size.
        \param pnRead Number of data bytes actually received.
        \return true on success, false on failure
    */
    int Binary_ReadMsg( uint8_t *pType, void* buffer, wxUint32 nbytes, uint16_t *pnRead );

    /*!
        Send binary error frame
        \param error_code Code to send out
    */
    void sendBinaryErrorFrame( uint8_t error_code );

    /*!
        Do a binary NOOP command.
        \return True on success, false on failure
    */
    int doBinaryNOOP( void );

    /*!
        Do a binary close of the channel
        \return True on success, false on failure
    */
    int doBinaryClose( void );


// ------------------------------------------------------------------------
//                   T H R E A D E D  I N T E R F A C E
// ------------------------------------------------------------------------




// ------------------------------------------------------------------------


protected:

    /// Socket
    wxSocketClient* m_psock;

    /// Server address
    wxIPV4address m_addr;

    /// Response string
    wxString m_strReply;

    /// Error storage
    uint32_t m_err;

    /// Last binary error
    uint8_t m_lastBinaryError;

    /// Flag for active receive loop
    bool m_bModeReceiveLoop;

    /// Server response timeout
    uint8_t m_responseTimeOut;

};




// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

//
// The following is helper code to make a threaded interface to the above
// function calls.
// The thread should be created JOINABLE and be terminated by setting 
// to true. Before starting the thread the m_pCtrlObject must be set up 
// to point to an initialized ctrlObjVscpTcpIf
//


//	Receive tread states
		
/*!
	\def RX_TREAD_STATE_NONE 					
	Idle state
*/
#define RX_TREAD_STATE_NONE					0
/*!	
	\def RX_TREAD_STATE_CONNECTED				
	Connected state
*/
#define RX_TREAD_STATE_CONNECTED			1
/*!	
	\def RX_TREAD_STATE_FAIL_DISCONNECTED		
	Fail state
*/
#define RX_TREAD_STATE_FAIL_DISCONNECTED	2
/*!	
	\def RX_TREAD_STATE_DISCONNECTED			
	Disconnected state
*/
#define RX_TREAD_STATE_DISCONNECTED			3	


/// Maximum number of events isn receive queue
#define MAX_TREAD_RECEIVE_EVENTS			8192



/*!
	\class ctrlObjVscpTcpIf
	\brief Shared VSCP tcp/ip object among worker threads.
*/
class ctrlObjVscpTcpIf
{

public:

    /// Constructor
    ctrlObjVscpTcpIf();

    /// Destructor
    ~ctrlObjVscpTcpIf();

    /// Username for VSCP serer
    wxString m_strUsername;

    /// Password for VSCP server
    wxString m_strPassword;

    /// Host address for VSCP server
    wxString m_strHost;

    /// Port for VSCP server
    int m_port;

    /// Thread run control (set to true to terminate thread)
    bool m_bQuit;

    /// Thread error return code
    long m_error;

    /// RX Thread run state
    uint8_t m_rxState;

    /*! 
        Channel id for receive channel
    */
    uint32_t m_rxChannelID;

    /*! 
        Channel id for transmit channel
        If bFilterOwnTx is true events 
        with this obid will not be added
        to the queue.
    */
    uint32_t m_txChannelID;

    /*!
        Filter our own tx events identified by
        obid set in m_txChannelID
    */
    bool m_bFilterOwnTx;

    /*!
        Send wxWidgets receive event instead of writing
        event to RX queue. 
        m_pWnd must point to a valid window and
        the id for that window must be in m_wndID
        Also TX events will be activated if thread is
        started.
    */
    bool m_bUseRXTXEvents;

    /*!
        Pointer to window that receive events.
        NULL if not used.
    */
    wxWindow *m_pWnd;

    /// Id for owner window (0 if not used).
    uint32_t m_wndID;

    /// Event Input queue
    EVENT_RX_QUEUE m_rxQueue;

	//@{
	/// Protection for input queue
	wxMutex m_mutexRxQueue;
	wxSemaphore m_semRxQueue;
	//@}
	
	/// Max events in queue
	uint32_t m_maxRXqueue;

	/// Event output queue
	EVENT_TX_QUEUE m_txQueue;

	//@{
	/// Protection for output queue
	wxMutex m_mutexTxQueue;
	wxSemaphore m_semTxQueue;
	//@}
};





/*!
	\class VSCPTCPIP_RX_WorkerThread
	\brief This class implement a thread that handles
	receive events for a link to the VSCP daemon.
*/

class VSCPTCPIP_RX_WorkerThread : public wxThread
{

public:

    /// Constructor
    VSCPTCPIP_RX_WorkerThread();

    /// Destructor
    virtual ~VSCPTCPIP_RX_WorkerThread();

    /*!
	    Thread code entry point
    */
    virtual void *Entry();


    /*! 
	    called when the thread exits - whether it terminates normally or is
	    stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();

    /*!
	    Pointer to control object.
    */
    ctrlObjVscpTcpIf *m_pCtrlObject;

};

/*!
	\class VSCPTCPIP_TX_WorkerThread
	\brief This class implement a thread that handles
	transmit events for a link to the VSCP daemon.
*/

class VSCPTCPIP_TX_WorkerThread : public wxThread
{

public:

    /// Constructor
    VSCPTCPIP_TX_WorkerThread();

    /// Destructor
    virtual ~VSCPTCPIP_TX_WorkerThread();

    /*!
	    Thread code entry point
    */
    virtual void *Entry();


    /*! 
	    called when the thread exits - whether it terminates normally or is
	    stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();

    /*!
	    Pointer to the daemon main control object.
    */
    ctrlObjVscpTcpIf *m_pCtrlObject;

};

#endif // !defined(AFX_VSCPTCPIF_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)


