///////////////////////////////////////////////////////////////////////////////
// vscpremotetcpif.h:
//
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



/*!
    \file    vscptcpif.h
    \brief   The VscpTcpIf class encapuslate sthe VSCP tcp/ip interface.
    \details The VscpTcpIf class holds functionality to talk toi the tcp/ip
    of the VSCP daemon and on client that implement the interface. All api's
    of the interface are supported by the class.
 */


#if !defined(VSCPREMOTETCPIF_H__INCLUDED_)
#define VSCPREMOTETCPIF_H__INCLUDED_


#include <canal.h>
#include <vscp.h>
#include <guid.h>

#if defined(_WIN32)
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <stdint.h>
#include <inttypes.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sockettcp.h>
#endif

#include <sockettcp.h>
#include <vscphelper.h>
#include "wx/datetime.h"

//---------------------------------------------------------------------------




/*! 
    @def DEFAULT_RESPONSE_TIMEOUT
    Default response timeout for communication with the
    tcp/ip interface of the daemon in milliseconds.

    This is the overall time to wait for a +OK respose.
 */
#define TCPIP_DEFAULT_RESPONSE_TIMEOUT          2000

/*! 
    @def DEFAULT_INNER_RESPONSE_TIMEOUT
    Default timeout for inner data check. A Read call will
    always wait this long for data.
*/
#define TCPIP_DEFAULT_INNER_RESPONSE_TIMEOUT    0


#define TCPIP_DEFAULT_AFTER_COMMAND_SLEEP       0   // TODO remove !!!!!!!!!

// Default values for read/write register functions
// used in device config and scan.
#define TCPIP_REGISTER_READ_RESEND_TIMEOUT      1000
#define TCPIP_REGISTER_READ_ERROR_TIMEOUT       5000
#define TCPIP_REGISTER_READ_MAX_TRIES           3

#define TCPIP_DEFAULT_CONNECT_TIMEOUT_SECONDS   15   // Seconds
/*!
    @def TCPIP_DLL_VERSION
    Pseudo version string
 */
#define TCPIP_DLL_VERSION                       0x00000010
/*! 
    @def TCPIP_VENDOR_STRING
    Pseudo vendor string
 */
#define TCPIP_VENDOR_STRING                     "Grodans Paradis AB, Sweden"

#define DRIVER_INFO_STRING                      ""

/*!
    Use this macro for unused function parameters.
 */
#define TCPIP_UNUSED(__par)             (void)(__par)

/// Receive queue
WX_DECLARE_LIST(vscpEvent, EVENT_RX_QUEUE);
/// Transmit queue
WX_DECLARE_LIST(vscpEvent, EVENT_TX_QUEUE);

// Forward declarations
class VscpRemoteTcpIf;


/*!
    @brief Class for VSCP daemon tcp/ip interface
 */
class VscpRemoteTcpIf {
public:

    /// Constructor
    VscpRemoteTcpIf();

    /// Destructor
    virtual ~VscpRemoteTcpIf();

public:

    /*!
        Set response timeout.
        @param to Timeout value in milliseconds. 
     */
    void setResponseTimeout( uint32_t to ) {
        if ( to < 100 ) to = 1000;      // To be backward compatible
        if (to) m_responseTimeOut = to;        
    };
    
    /*!
        Set after command sleep time (milliseconds)
        @param to Sleep value in milliseconds.
     */
    void setAfterCommandSleep( uint16_t to ) {
        TCPIP_UNUSED(to);               // For backward compability
    }

    /*!
        Set register read/write timings
     */
    void setRegisterOperationTiming( uint8_t retries, 
                                        uint32_t resendto, 
                                        uint32_t errorto) {
        m_registerOpMaxRetries = retries;
        m_registerOpResendTimeout = resendto;
        m_registerOpErrorTimeout = errorto;
    };
    
    /*!
        Get last response form remote node
        @return Last raw response data from remote node
     */
    wxString& getLastResponse( void ) { return m_strResponse; };

    /*!
        Get last response time
        @return Relative time in milliseconds when last valid received
            data was read.
    */
    wxLongLong getlastResponseTime( void ) { return m_lastResponseTime; };
    
    /*!
        Returns TRUE if we are connected false otherwise. 
     */
    bool isConnected( void ) { return( ( NULL != m_conn ) &&
                                       ( STCP_CONN_STATE_CONNECTED == m_conn->conn_state ) ); };

    /*!
        checkReturnValue
        @param bClear Clear the input bugger before starting to wait for received data.
        @return Return false for "-OK" and true for "+OK"
     */
    bool checkReturnValue( bool bClear = false );

    /*!
        Clear the input queue
     */
    void doClrInputQueue( void );

    /*!
        \brief Creates the input string array from a remote client response
        @return Number of rows in created string array.
    */
    size_t addInputStringArrayFromReply( bool bClear = false );

    /*!
        \brief Send a command to the remote client.
        @param cmd Commad to issue
        @return Returns VSCP_ERROR_SUCCESS if the command could be sent successfully.
     */
    int doCommand( const wxString& cmd );
    int doCommand( const std::string& cmd );

    /*!
        \brief Send a command to the remote client.
        @param cmd Commad to issue
        @return Returns VSCP_ERROR_SUCCESS if the command could be sent successfully.
     */
    int doCommand( const char *cmd );

    /*!
        Open communication interface.
        @param strInterface should contain "username;password;ip-addr;port" if used. All including 
            port are optional and defaults to no username/password, server as "localhost" and "9598" 
            as default port.
        @param flags are not used at the moment.
        @return CANAL_ERROR_SUCCESS if channel is open or CANAL error code  if error 
        or the channel is already opened or other error occur.
     */
    int doCmdOpen( const wxString& strInterface = (_("")), uint32_t flags = 0L );

    /*!
        Open communication interface.
        @param strHostname Host to connect to + port (host:port).
        @param strUsername Username.
        @param strPassword Username.
        @return CANAL_ERROR_SUCCESS if channel is open or CANAL error code if error 
            or the channel is already opened or other error occur.
     */
    int doCmdOpen( const wxString& strHostname,
                    const wxString& strUsername,
                    const wxString& strPassword );

    /*!
        Open communication interface.
        @param strHostname Host to connect to.
        @param port Port of interface to connec to
        @param strUsername Username.
        @param strPassword Username.
        @return CANAL_ERROR_SUCCESS if channel is open or CANAL error code if error 
            or the channel is already opened or other error occur.
     */
    int doCmdOpen( const wxString& strHostname,
                    short port,
                    const wxString& strUsername,
                    const wxString& strPassword );                

    /*!
        Close communication interface
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdClose( void );

    /*!
        Write NOOP message through pipe. 
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdNOOP( void );

    /*!
        Clear input queue
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdClear( void );

    /*!
        Get the Canal protocol level
        @return VSCP Level
     */
    unsigned long doCmdGetLevel( void ) {
        return VSCP_LEVEL2;
    }

    /*!
        Send a VSCP Level II event through interface. 
        If a GUID sent with all items set to zero the GUID of the
        interface ("-") will be used.
        @param pEvent VSCP Level II event to send.		
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdSend( const vscpEvent *pEvent );

    /*!
        Send a VSCP ex event through interface. 
        @param pEvent VSCP Level II ex event to send.	
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdSendEx( const vscpEventEx *pEvent );

    /*!
        Send a Level I event through interface. 
        @param pMsg VSCP Level I event (==canalMsg).	
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdSendLevel1( const canalMsg *pMsg );

    /*!
        Receive a VSCP event through the interface. 
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdReceive( vscpEvent *pEvent );

    /*!
        Receive an VSCP ex event through the interface. 
        @return true if success false if not.
     */
    int doCmdReceiveEx( vscpEventEx *pEvent );

    /*!
        Receive an VSCP Level I event through the interface.
        For the extended and the RTR bit to be handled the
        CAN message bit in the head byte must be set.
        Low eight bits of the CAN id is fetched from GUID[15]
        that is LSB of GUID.
        @param Pointer to CANAL message
        @return CANAL_ERROR_SUCCESS if success false if not.
     */
    int doCmdReceiveLevel1( canalMsg *pCanalMsg );

    /*!
        This command sets an open interface in the receive loop (RCVLOOP). 
        It does nothing other then putting the interface in the loop mode and 
        checking that it went there.

        Note! The only way to terminate this receive loop is to close the session with 
        doCmdClose (or just close the socket).

        @return CANAL_ERROR_SUCCESS if success CANAL_ERROR_GENERIC if not.
     */
    int doCmdEnterReceiveLoop( void );

    /*!
        Quit the receive loop
     */
    int doCmdQuitReceiveLoop(void);

    /*!
        Receive an event 
        The receiveloop command must have been issued for this method to work as 
        it sets up the ringbuffer used for the blocking receive.
        @param pEvent Pointer to VSCP event.
        @param timeout Timout to wait for data. Default = 500 milliseconds.
        @return CANAL_ERROR_SUCCESS when event is received. CANAL_ERROR_FIFO_EMPTY is
        returned if no event was available. CANAL_ERROR_TIMEOUT on timeout.CANAL_ERROR_COMMUNICATION
        is returned if a socket error is detected.
     */
    int doCmdBlockingReceive( vscpEvent *pEvent, uint32_t timeout = 500 );

    int doCmdBlockingReceive( vscpEventEx *pEventEx, uint32_t timeout = 500 );

    /*!
        Get the number of events in the input queue of this interface
        @return the number of events available or if negative
        an error code.
     */
    int doCmdDataAvailable( void );

    /*!
        Receive CAN status through the interface. 
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdStatus( canalStatus *pStatus );


    /*!
        A VSCP variant of the above
     */
    int doCmdStatus( VSCPStatus *pStatus );

    /*!
        Receive VSCP statistics through the interface. 
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdStatistics( VSCPStatistics *pStatistics );

    /*!
        Receive CAN statistics through the interface. 
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdStatistics( canalStatistics *pStatistics );

    /*!
        Set/Reset a filter through the interface. 
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdFilter( const vscpEventFilter *pFilter );

    /*!
        Set/Reset filter through the interface. 
        @param filter Filter on string form (priority,class,type,guid).
        @param mask Mask on string form (priority,class,type,guid).
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdFilter( const wxString& filter, const wxString& mask );

    /*!
        Get i/f version through the interface. 
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdVersion( uint8_t *pMajorVer,
                        uint8_t *pMinorVer,
                        uint8_t *pSubMinorVer);

    /*!
        Get i/f version including build-version through the interface. 
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdVersion( uint8_t *pMajorVer,
                        uint8_t *pMinorVer,
                        uint8_t *pSubMinorVer,
                        uint16_t *pBuildVer );                        

    /*!
        Get interface version
     */
    unsigned long doCmdDLLVersion( void );


    /*!
        Get vendor string
     */
    const char *doCmdVendorString( void );

    /*!
        Get Driver information string.
     */
    const char *doCmdGetDriverInfo( void );

    /*!
        Get GUID for this interface.  
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdGetGUID( unsigned char *pGUID );

    /*!
        Get GUID for this interface.  
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdGetGUID( cguid& ifguid );

    /*!
        Set GUID for this interface.  
        @param pGUID Array with uint8_t GUID's. (Note! Not a string).
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdSetGUID( const unsigned char *pGUID );
    
    /*!
        Set GUID for this interface.  
        @param ifguid GUID to set.
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdSetGUID( cguid& ifguid );

    /*!
        Get information about a channel..  
        @param pChannelInfo The stucture that will get the information
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdGetChannelInfo( VSCPChannelInfo *pChannelInfo );

    /*!
        Get Channel ID for the open channel
        @param pChannelID Channel ID for channel
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdGetChannelID( uint32_t *pChannelID );

    /*!
        Get available interfaces
        @param array A string array that will get interface list
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdInterfaceList( wxArrayString& array );

    /*!
        Dummy for Baudrate setting
     */
    int doCmdSetBaudrate( uint32_t baudrate ) {
        TCPIP_UNUSED( baudrate );
        return CANAL_ERROR_SUCCESS;
    };

    /*!
        Dummy for filter setting
     */
    int doCmdFilter( uint32_t filter ) {
        TCPIP_UNUSED( filter );
        return CANAL_ERROR_SUCCESS;
    };

    /*!
        Dummy for mask setting
     */
    int doCmdMask( uint32_t mask ) {
        TCPIP_UNUSED( mask );
        return CANAL_ERROR_SUCCESS;
    };

    /*!
    Shutdown the VSCP daemon  
     */
    int doCmdShutDown(void);

    /*!
        Get a VSCP event from a line of data from the server
        @param pEvent Pointer to VSCP event.
        @param strLine String with server event data line        
        @return true on success.
     */
    bool getEventFromLine( vscpEvent *pEvent, 
                            const wxString& strLine );






    // ------------------------------------------------------------------------
    //                    V A R I A B L E  H A N D L I N G
    // ------------------------------------------------------------------------
    
    
    
    

    /*!
        List variables on remote server
     
        @param array Will be filed with names for remote variables.
        @param regexp Regular expression used to select variables. Leave
            blank to return all variables.
        @param type Variable type that should be listed or 0 for all types.
        @return VSCP_ERROR_SUCCESS on success
     */
    int getRemoteVariableList( wxArrayString& array, 
                                    const wxString regexp = _(""),
                                    const int type = 0 );
    
    /*!
     * Save variables to external disk
     * 
     * @param Path to external storage.
     * @param select Which is “1” for only writing persistent variables, 
     *          “2” only write non-persistent variables and “3” or “0” 
     *          for writing both types.
     * @param regexp Optional regular expression that selects which variables 
     *          to write. Default is all.
     * @return VSCP_ERROR_SUCCESS on success
     */
    int saveRemoteVariablesToDisk( const wxString& path, 
                                        const int select=0, 
                                        const wxString& regExp=_("") );
    
    /*!
     * Load variables from external disk
     * 
     * @param Path to external storage.
     * @return VSCP_ERROR_SUCCESS on success.
     */
    int loadRemoteVariablesFromDisk( const wxString& path );

    /*!
        Create a variable
     
        @param name Name of variable to create.
        @param strType of variable either in string or numeric form
        @param bPersistet True if the variable should be persistent (default=false)
        @param strUser Owner of variable. Can be left empty in which case the 
                            logged in user will be used.
        @param rights Access rights for variable.
        @param strValue Value for the variable.
        @param strNote Optional note for variable.
        
        @return VSCP_ERROR_SUCCESS on success
     */
    int createRemoteVariable( const wxString& name, 
                                const wxString& strType,
                                const bool bPersistent,
                                const wxString& strUser,
                                const uint32_t rights,
                                const wxString& strValue,
                                const wxString& strNote = _("") );
    
    /*!
        Delete a named variable
        @param name Name of variable to delete.
        @return VSCP_ERROR_SUCCESS on success
     */
    int deleteRemoteVariable( const wxString& name );

    /*!
        Get remote variable on string form
        Format is: "variable name";"type";"persistence";"user";"rights";"value";"note"
     
        @param name of variable.
        @param strValue pointer to string that get the variable on string form.
        @return VSCP_ERROR_SUCCESS if the variable is of type string and the operation
                    was successful.
     */
    int getRemoteVariableAsString( const wxString& name, 
                                    wxString& strVariable );

    /*!
        Set variable from string 
     
        @param name of variable.
        @param strValue Variable on string form.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableFromString( const wxString& name, 
                                        const wxString& strValue,
                                        const bool bPersistent = false,
                                        const uint32_t rights = 0x744 );
    
    /*!
     * Get last change date/time for remote variable.
     * 
     * @param name Name of variable.
     * @return VSCP_ERROR_SUCCESS on success
     */
    int getRemoteVariableLastChange( const wxString& name, 
                                        wxDateTime& lastChange );
    
    
    /*!
     * Get type on numeric form for variable.
     * 
     * @param name Name of variable.
     * @return VSCP_ERROR_SUCCESS on success
     * 
     */
    int getRemoteVariableType( const wxString& name, 
                                    uint8_t *pType );
    
    /*!
     * Get persistence for variable.
     * 
     * @param name Name of variable.
     * @return VSCP_ERROR_SUCCESS on success
     * 
     */
    int getRemoteVariablePersistence( const wxString& name, 
                                        bool *pPersistent );
    
    /*!
     * Get owner for remote variable.
     * 
     * @param name Name of variable.
     * @return VSCP_ERROR_SUCCESS on success
     * 
     */
    int getRemoteVariableOwner( const wxString& name, 
                                    uint32_t *pOwner );
    
    /*!
     * Get access rights for remote variable.
     * 
     * @param name Name of variable.
     * @return VSCP_ERROR_SUCCESS on success
     * 
     */
    int getRemoteVariableAccessRights( const wxString& name, 
                                        uint16_t *pRights );
    
    /*!
     * Get value on string form for variable.
     * 
     * @param name Name of variable.
     * @param strValue Value in string form
     * @param bDecode Decode (if true) from BASE64 if value is stored decoded.
     * @return VSCP_ERROR_SUCCESS on success
     * 
     */
    int getRemoteVariableValue( const wxString& name, 
                                    wxString& strValue, 
                                    bool bDecode=false );

    int getRemoteVariableValue( const std::string& name, 
                                    std::string& strValue, 
                                    bool bDecode=false );                                        
    
    /*!
     * Set value for variable from string.
     * 
     * @param name Name of variable.
     * @return VSCP_ERROR_SUCCESS on success
     * 
     */
    int setRemoteVariableValue( const wxString& name, 
                                    wxString& strValue );
    
    /*!
     * Get note for remote variable.
     * 
     * @param name Name of variable.
     * @return VSCP_ERROR_SUCCESS on success
     * 
     */
    int getRemoteVariableNote( const wxString& name, 
                                wxString& strNote );


    /*!
        Get variable value from boolean variable
     
        @param name of variable.
        @param bValue pointer to boolean variable that get the value of the string variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type bool and the operation
        was successful.
     */
    int getRemoteVariableBool( const wxString& name, 
                                bool *bValue);

    /*!
        Set variable value from boolean variable
     
        @param name of variable.
        @param bValue boolean variable with the value to set.
        @return VSCP_ERROR_SUCCESS if the variable is of type bool and the operation
        was successful.
     */
    int setRemoteVariableBool( const wxString& name, 
                                const bool bValue);

    /*!
        Get variable value from integer variable
     
        @param name of variable.
        @param value pointer to integer variable that get the value of the string variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type integer and the operation
        was successful.
     */
    int getRemoteVariableInt( const wxString& name, 
                                int *value);

    /*!
        Set variable value from integer variable
     
        @param name of variable. 
        @param value integer variable with the value to set.
        @return VSCP_ERROR_SUCCESS if the variable is of type integer and the operation
        was successful.
     */
    int setRemoteVariableInt( const wxString& name, 
                                int value);

    /*!
        Get variable value from long variable
     
        @param name of variable.
        @param value pointer to long variable that get the value of the string variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type long and the operation
        was successful.
     */
    int getRemoteVariableLong( const wxString& name, 
                                long *value);

    /*!
        Set variable value from long variable
     
        @param name of variable.
        @param value long variable with the value to set.
        @return VSCP_ERROR_SUCCESS if the variable is of type long and the operation
        was successful.
     */
    int setRemoteVariableLong( const wxString& name, 
                                long value);

    /*!
        Get variable value from double variable
     
        @param name of variable.
        @param value double variable with the value to set.
        @return VSCP_ERROR_SUCCESS if the variable is of type double and the operation
        was successful.
     */
    int getRemoteVariableDouble( const wxString& name, 
                                    double *value);

    /*!
        Set variable value from double variable
     
        @param name of variable.
        @param value The double value to set.
        @return VSCP_ERROR_SUCCESS if the variable is of type double and the operation
        was successful.
     */
    int setRemoteVariableDouble( const wxString& name, 
                                    double value );

    /*!
        Get variable value from measurement variable
     
        @param name of variable.
        @param pvalue Pointer to a double that will get the value for the measurement.
        @param punit Pointer to uint8_t that will get the unit for the measurement.
        @param psensoridx Pointer to uint8_t that will get the sensor index for the measurement.
        @param pzone Pointer to a uint8_t that will get the zone for the measurement.
        @param psubzone Pointer to a unit8_t that will get the sub zone for the measurement.
        @return VSCP_ERROR_SUCCESS if the variable is of type measurement and the operation
                was successful.
     */
    int getRemoteVariableMeasurement( const wxString& name, 
                                        double *pvalue,
                                        uint8_t *punit,
                                        uint8_t *psensoridx,
                                        uint8_t *pzone,
                                        uint8_t *psubzone );

    /*!
        set variable value from double variable
     
        @param name of variable.
        @param value Double containing the value for the measurement.
        @param unit The unit for the measurement.
        @param sensoridx The sensor index for the measurement.
        @param zone The zone for the measurement.
        @param subzone The sub zone for the measurement.
        @return VSCP_ERROR_SUCCESS if the variable is of type double and the operation
        was successful.
     */
    int setRemoteVariableMeasurement( const wxString& name, 
                                        double value,
                                        uint8_t unit = 0,
                                        uint8_t sensoridx = 0,
                                        uint8_t zone = 255,
                                        uint8_t subzone = 255 );

    /*!
        Get variable value from event variable
     
        @param name of variable.
        @param pEvent pointer to event variable that get the value of the string variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP event and the operation
        was successful.
     */
    int getRemoteVariableEvent( const wxString& name, 
                                    vscpEvent *pEvent );

    /*!
        set variable value from VSCP event
     
        @param name of variable.
        @param pEvent pointer to event that is used to set the variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableEvent( const wxString& name, 
                                    vscpEvent *pEvent );

    /*!
        Get variable value from event variable
     
        @param name of variable.
        @param pEvent pointer to event variable that get the value of the string variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP event and the operation
        was successful.
     */
    int getRemoteVariableEventEx( const wxString& name, 
                                    vscpEventEx *pEvent );

    /*!
        set variable value from VSCP event
     
        @param name of variable.
        @param pEvent pointer to event that is used to set the variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableEventEx( const wxString& name, 
                                    vscpEventEx *pEvent );

    /*!
        Get variable value from GUID variable
     
        @param name of variable.
        @param pGUID pointer to event variable that get the value of the GUID variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP GUID and the operation
        was successful.
     */
    int getRemoteVariableGUID( const wxString& name, cguid& pGUID );

    /*!
        set variable value from GUID
     
        @param name of variable.
        @param pGUID pointer to GUID that is used to set the variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableGUID( const wxString& name, cguid& pGUID );

    /*!
        Get variable value from VSCP data variable
     
        @param name of variable.
        @param pData Pointer to data array to get data from.
        @param pSize Pointer to size of data.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP data and the operation
        was successful.
     */
    int getRemoteVariableVSCPdata( const wxString& name, 
                                        uint8_t *pData, 
                                        uint16_t *pSize );

    /*!
        set variable value from VSCP data
     
        @param name of variable.
        @param pData Pointer to data array to set data from.
        @param size Size of data.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableVSCPdata( const wxString& name, 
                                    uint8_t *pData, 
                                    uint16_t size );

    /*!
        Get variable value from class variable
     
        @param name of variable.
        @param vscp_class pointer to int that get the value of the class variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP class and the operation
        was successful.
     */
    int getRemoteVariableVSCPclass( const wxString& name, 
                                        uint16_t *vscp_class );

    /*!
        set variable value from vscp_class.
     
        @param name of variable.
        @param vscp_class to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableVSCPclass( const wxString& name, 
                                        uint16_t vscp_class );

    /*!
        Get variable value from type variable
     
        @param name of variable.
        @param vscp_type pointer to int that get the value of the type variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableVSCPtype( const wxString& name, 
                                    uint16_t *vscp_type );


    /*!
        set variable value from vscp_type.
     
        @param name of variable.
        @param vscp_type to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableVSCPtype( const wxString& name, 
                                    uint16_t vscp_type );
    
    // -------------------------------------------------------------------------
    
    /*!
        Get variable value from timestamp variable
     
        @param name of variable.
        @param vscp_timestamp pointer to uint32_t that get the value of the timestamp variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableVSCPTimestamp( const wxString& name, 
                                            uint32_t *vscp_timestamp );


    /*!
        Set variable value from vscp_timestamp.
     
        @param name of variable.
        @param vscp_timestamp as uint32_t to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableVSCPTimestamp( const wxString& name, 
                                            uint32_t vscp_timestamp );

    /*!
        Get variable value from datetime variable
     
        @param name of variable.
        @param datetime that get the value of the datetime variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableDateTime( const wxString& name, 
                                        wxDateTime& datetime );


    /*!
        Set variable value from datetime.
     
        @param name of variable.
        @param datetime to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableDateTime( const wxString& name, 
                                    wxDateTime& datetimes );
    
    /*!
        Get variable value from a date variable
     
        @param name of variable.
        @param date that get the value of the date variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableDate( const wxString& name, 
                                wxDateTime& date );


    /*!
        Set variable value from date.
     
        @param name of variable.
        @param date to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableDate( const wxString& name, 
                                wxDateTime& date );
    
    /*!
        Get variable value from a time variable
     
        @param name of variable.
        @param time that get the value of the time variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableTime( const wxString& name, 
                                wxDateTime& time );


    /*!
        Set variable value from time.
     
        @param name of variable.
        @param time to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableTime( const wxString& name, 
                                wxDateTime& time );
    
    
    /*!
        Get variable value from a base64 ENCODED blob variable
     
        @param name of variable.
        @param blob that get the value of the BASE64 encoded blob variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableBlob( const wxString& name, 
                                wxString& blob );


    /*!
        Set variable value from BASE64 encoded  blob.
     
        @param name of variable.
        @param BASE64 encoded blob to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableBlob( const wxString& name, 
                                wxString& blob );
    
    
    /*!
        Get variable value from a base64 ENCODED MIME variable
     
        @param name of variable.
        @param MIME data that get the value of the BASE64 encoded MIME variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableMIME( const wxString& name, 
                                wxString& mime );


    /*!
        Set variable value from BASE64 encoded MIME data.
     
        @param name of variable.
        @param BASE64 encoded MIME data to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableMIME( const wxString& name, wxString& mime );
    
    /*!
        Get variable value from a base64 ENCODED HTML variable
     
        @param name of variable.
        @param HTML data that get the value of the BASE64 encoded HTML variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableHTML( const wxString& name, wxString& html );


    /*!
        Set variable value from BASE64 encoded HTML data.
     
        @param name of variable.
        @param BASE64 encoded HTML data to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableHTML( const wxString& name, wxString& html );
    
   /*!
        Get variable value from a base64 ENCODED JavaScript variable
     
        @param name of variable.
        @param JavaScript data that get the value of the BASE64 encoded JavaScript variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableJavaScript( const wxString& name, wxString& js );


    /*!
        Set variable value from BASE64 encoded JavaScript data.
     
        @param name of variable.
        @param BASE64 encoded JavaScript data to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableJavaScript( const wxString& name, wxString& js );    
    
    /*!
        Get variable value from a base64 ENCODED LUA variable
     
        @param name of variable.
        @param LUA data that get the value of the BASE64 encoded LUA variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableLUA( const wxString& name, wxString& lua );


    /*!
        Set variable value from BASE64 encoded LUA data.
     
        @param name of variable.
        @param BASE64 encoded LUA data to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableLUA( const wxString& name, wxString& lua );
    
    /*!
        Get variable value from a base64 ENCODED LUARES variable
     
        @param name of variable.
        @param LUARES data that get the value of the BASE64 encoded LUARES variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableLUARES( const wxString& name, wxString& luares );


    /*!
        Set variable value from BASE64 encoded LUARES data.
     
        @param name of variable.
        @param BASE64 encoded LUARES data to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableLUARES( const wxString& name, wxString& luares );    
    
    
    /*!
        Get variable value from a base64 ENCODED UX1 variable
     
        @param name of variable.
        @param UX1 data that get the value of the BASE64 encoded UX1 variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableUX1( const wxString& name, wxString& ux1 );


    /*!
        Set variable value from BASE64 encoded UX1 data.
     
        @param name of variable.
        @param BASE64 encoded UX1 data to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableUX1( const wxString& name, wxString& ux1 ); 
    
    
    /*!
        Get variable value from a base64 ENCODED DM row variable
     
        @param name of variable.
        @param DM row data that get the value of the BASE64 encoded DM row variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableDMROW( const wxString& name, wxString& dmrow );


    /*!
        Set variable value from BASE64 encoded DM row data.
     
        @param name of variable.
        @param BASE64 encoded DM row data to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableDMROW( const wxString& name, wxString& dmrow );
    
    /*!
        Get variable value from a base64 ENCODED driver variable
     
        @param name of variable.
        @param Driver data that get the value of the BASE64 encoded driver variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableDriver( const wxString& name, wxString& drv );


    /*!
        Set variable value from BASE64 encoded driver data.
     
        @param name of variable.
        @param BASE64 encoded driver data to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableDriver( const wxString& name, wxString& drv );
    
    /*!
        Get variable value from a base64 ENCODED user variable
     
        @param name of variable.
        @param User data that get the value of the BASE64 encoded user variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableUser( const wxString& name, wxString& user );


    /*!
        Set variable value from BASE64 encoded user data.
     
        @param name of variable.
        @param BASE64 encoded user data to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableUser( const wxString& name, wxString& user );
    
    /*!
        Get variable value from a base64 ENCODED filter variable
     
        @param name of variable.
        @param Filter data that get the value of the BASE64 encoded filter variable.
        @return VSCP_ERROR_SUCCESS if the variable is of type VSCP type and the operation
        was successful.
     */
    int getRemoteVariableFilter( const wxString& name, wxString& filter );


    /*!
        Set variable value from BASE64 encoded filter data.
     
        @param name of variable.
        @param BASE64 encoded filter data to write to variable.
        @return VSCP_ERROR_SUCCESS if the operation was successful.
     */
    int setRemoteVariableFilter( const wxString& name, wxString& filter );
    
    
    
    
    
    // ------------------------------------------------------------------------
    //                       T A B L E  H A N D L I N G
    // ------------------------------------------------------------------------
    
    
    
    
    

    /*
     * Create table from XML data
     * 
     * @param defTable XML definition of table.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableCreate( const wxString& defTable );
    
    /*!
     * Delete named table
     * 
     * @param tblName Name of an existing table.
     * @param bDeleteFile If true the database file will be deleted.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     * 
     */
    int tableDelete( const wxString &tblName, bool bDeleteFile=false );
    
    /*!
     * Get a list of all defined tables
     * 
     * @param tablesArray List that will get filled with table names.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableList( wxArrayString &tablesArray );
    
    /*!
     * Get information about a named table
     * 
     * @param tblName Name of table to fetch infor about.,
     * @param tblInfo Parameter that wil get information about table.
     * @param bXML If true info will be returnd as BASE64 encoded
     *          XML data.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableListInfo( const wxString &tblName, 
                        wxString &tableInfo, 
                        bool bXML = true );
    
    
    
    /*!
     * Get information about a named table
     * 
     * @param tblName Name of table to fetch infor about.,
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param resultArray String array holding result.
     * @param bFull True if all fields of the table row should be listed.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGet( const wxString& tblName, 
                        const wxDateTime& from, 
                        const wxDateTime& to,
                        wxArrayString& resultArray,
                        bool bFull = false  );
    
    /*!
     * Get raw information about a named table
     * 
     * @param tblName Name of table to fetch infor about.,
     * @param from Date/time from which resulting data should be fetched.
     * @param tp Date/time to which resulting data should be fetched.
     * @param resultArray String array holding result.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGetRaw( const wxString& tblName, 
                        const wxDateTime& from, 
                        const wxDateTime& to,
                        wxArrayString& resultArray );
    
    
    /*!
     * Log data to a named table
     * 
     * @param tblName Name of table,
     * @param value Double to log..
     * @param pdt Pointer to datetime. If NULL the current date/time will be used.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableLog( const wxString& tblName, 
                        double value, 
                        wxDateTime *pdt = NULL );
    
    /*!
     * Log data to with SQL expression
     * 
     * @param tblName Name of table.
     * @param sql Log SQL expression.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableLogSQL( const wxString& tblName, const wxString& sql );
    
    
    /*!
     * Get number of records for an interval.
     * 
     * @param tblName Name of table.
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param pRecords Filled with number of records in the interval on success.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGetNumRecords( const wxString& tblName, 
                                const wxDateTime& from, 
                                const wxDateTime& to,
                                size_t *pRecords );
    
    /*!
     * Get first date/time for an interval-
     * 
     * @param tblName Name of table.
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param first First date/time in the interval on success.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGetFirstDate( const wxString& tblName, 
                                const wxDateTime& from, 
                                const wxDateTime& to,
                                wxDateTime& first );
    
    /*!
     * Get last date/time for an interval-
     * 
     * @param tblName Name of table.
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param last Last date/time in the interval on success.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGetLastDate( const wxString& tblName, 
                                const wxDateTime& from, 
                                const wxDateTime& to,
                                wxDateTime& last );
    
    /*!
     * Get last sum of values for an interval-
     * 
     * @param tblName Name of table.
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param sum Sum of values in the interval on success.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGetSum( const wxString& tblName, 
                        const wxDateTime& from, 
                        const wxDateTime& to,
                        double *pSum );
    
    /*!
     * Get min of values for an interval-
     * 
     * @param tblName Name of table.
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param min Min of values in the interval on success.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGetMin( const wxString& tblName, 
                        const wxDateTime& from, 
                        const wxDateTime& to,
                        double *pMin );
    
    
    /*!
     * Get max of values for an interval-
     * 
     * @param tblName Name of table.
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param max Max of values in the interval on success.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGetMax( const wxString& tblName, 
                        const wxDateTime& from, 
                        const wxDateTime& to,
                        double *pMax );
    
    /*!
     * Get average of values for an interval-
     * 
     * @param tblName Name of table.
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param average Average of values in the interval on success.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGetAverage( const wxString& tblName, 
                            const wxDateTime& from, 
                            const wxDateTime& to,
                            double *pAverage );
    
    /*!
     * Get median of values for an interval-
     * 
     * @param tblName Name of table.
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param median Median of values in the interval on success.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGetMedian( const wxString& tblName, 
                            const wxDateTime& from, 
                            const wxDateTime& to,
                            double *pMedian );
    
    /*!
     * Get stdev of values for an interval-
     * 
     * @param tblName Name of table.
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param stddev Standard deviation of values in the interval on success.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGetStdDev( const wxString& tblName, 
                            const wxDateTime& from, 
                            const wxDateTime& to,
                            double *pStdDev );
    
    
    /*!
     * Get variance of values for an interval-
     * 
     * @param tblName Name of table.
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param variance Variance of values in the interval on success.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGetVariance( const wxString& tblName, 
                            const wxDateTime& from, 
                            const wxDateTime& to,
                            double *pVariance );
    
    /*!
     * Get mode of values for an interval-
     * 
     * @param tblName Name of table.
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param mode Mode of values in the interval on success.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGetMode( const wxString& tblName, 
                            const wxDateTime& from, 
                            const wxDateTime& to,
                            double *pMode );
    
    /*!
     * Get lower Q of values for an interval-
     * 
     * @param tblName Name of table.
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param lowerq Lower Q of values in the interval on success.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGetLowerQ( const wxString& tblName, 
                            const wxDateTime& from, 
                            const wxDateTime& to,
                            double *pLowerQ );
    
    /*!
     * Get Upper Q of values for an interval-
     * 
     * @param tblName Name of table.
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param upperq Upper Q of values in the interval on success.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableGetUpperQ( const wxString& tblName, 
                            const wxDateTime& from, 
                            const wxDateTime& to,
                            double *pUpperQ );
    
    
    /*!
     * Clear table over an interval-
     * 
     * @param tblName Name of table.
     * @param from Date/time from which resulting data should be fetched.
     * @param to Date/time to which resulting data should be fetched.
     * @param upperq Upper Q of values in the interval on success.
     * @return return VSCP_ERROR_SUCCESS; on success, error code on failure.
     */
    int tableClear( const wxString& tblName, 
                        const wxDateTime& from, 
                        const wxDateTime& to  );
    
    
    
    
    
    // ------------------------------------------------------------------------
    //                           R E G I S T E R S
    // ------------------------------------------------------------------------

    
    
    
    
    /*!
        Read a level II register
     
        @param reg Register to read (32-bit value for true Level II)
        @param page Page to read from.
        @param pval Pointer to eight bit value that will get read value.
        @param pdestGUID GUID for remote node.
        @param ifGUID GUID for interface this events hsould be sent on. This is
                used to read registers of Level I devices.
        @param bLevel2 If true this is a true Level II read operation.
        @return CANAL_ERROR_SUCCESS on success, errocode on failure.
     */

    int readLevel2Register( uint32_t reg,
                                uint16_t page,
                                uint8_t *pval,
                                cguid& ifGUID,
                                cguid *pdestGUID = NULL,
                                bool bLevel2 = false );

    /*!
        Load level II register content into an array
     
        @param reg First register to read..
        @param count Number of registers to read (max 128 bytes).
        @param page Page to read from.
        @param pregisters Pointer to an array of count 8-bit registers.
        @param pinterfaceGUID GUID for interface to do read on.
        @param pdestGUID GUID for remote node.	
        @param bLevel2 True for a true Level II device.
        @return CANAL_ERROR_SUCCESS on success, errocode on failure.
     */

    int readLevel2Registers( uint32_t reg,
                                uint16_t page,
                                uint8_t count,
                                uint8_t *pval,
                                cguid& ifGUID,
                                cguid *pdestGUID = NULL,
                                bool bLevel2 = false );

    /*!
        Write a level 2 register
     
        @param reg Register to write.
        @param pval Pointer to data to write. Return read data.
        @param interfaceGUID GUID for interface where devices sits whos register
                    should be read with byte 0 set to nickname id for the device.
        @param pdestGUID GUID for remote node.	
        @return CANAL_ERROR_SUCCESS on success, errocode on failure.
     */
    int writeLevel2Register(uint32_t reg,
                                uint16_t page,
                                uint8_t *pval,
                                cguid& ifGUID,
                                cguid *pdestGUID = NULL,
                                bool bLevel2 = false );

    /*!
        Get MDf file from device registers
     
        @param pdestGUID Pointer to guid of node.
        @param strurl URL to MDF if call is successful.
        @param bLevel2 Set to true if this is a level II devive 
        @return true on success, false on failure.
     */
    bool getMDFUrlFromLevel2Device( cguid& ifGUID,
                                        cguid& destGUID,
                                        wxString &strurl,
                                        bool bLevel2 = false );


    /*!
        Fetch guid for a tcp/ip interface from it's name.
     
        @param ifName Interface name
        @param guid Returnd GUID for interface
        @param Returns true on success.
     */
    int fetchIterfaceGUID( const wxString& ifName, cguid& guid );




    // ------------------------------------------------------------------------
    
    

public:



protected:

    /*! 
        Array that gets filled with input lines
     */
    wxArrayString m_inputStrArray;

    // Last read valid response in milliseconds
    // Used to detect dead (rcvloop) link.  
    wxLongLong m_lastResponseTime;

private:

    /// Flag for active receive loop
    bool m_bModeReceiveLoop;

    /// Server response timeout in milliseconds
    uint32_t m_responseTimeOut;

    /// Error timeout for register read/write operations
    uint32_t m_registerOpErrorTimeout;

    /// Resend timeout for read/write operations
    uint32_t m_registerOpResendTimeout;

    /// Number of read/write retries
    uint8_t m_registerOpMaxRetries;

    /*! 
     * The tcp inner timout. Will always wait this amount of time for data
     * if set to -1 default value 2000 ms will be used.
     * Min value is 0 == return directly.
    */
    int m_innerResponseTimeout;

    /*!
        Version information is stored whenever the 
        version command is used (always done when done on 'open').
        Initialized to the same version as this system on startup.
    */
    uint8_t m_version_major;
    uint8_t m_version_minor;
    uint8_t m_version_release;
    uint16_t m_version_build;

    /*!
        Get input queue count
        @return Number of messages in the queue
     */
    size_t getInputQueueCount( void );

    /*!
        The connection structure
        Not NULL if connected.
    */
    struct stcp_connection *m_conn;

    
    /*!
     * This is the last response from a remote node.
     * It can contain multiple response lines (separated
     * with \r\n) and will end with a line containg +OK
     * or -ERR if an error response was receved.
     */            
    wxString m_strResponse;
};




// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

//
// The following is helper code to make a threaded interface to the above
// function calls.
// The thread should be created DETACHABLE and be terminated by setting 
// to true. Before starting the thread the m_pCtrlObject must be set up 
// to point to an initialized ctrlObjVscpTcpIf
//


//	Receive tread states

/*!
        @def RX_TREAD_STATE_NONE 					
        Idle state
 */
#define RX_TREAD_STATE_NONE                             0
/*!	
        @def RX_TREAD_STATE_CONNECTED				
        Connected state
 */
#define RX_TREAD_STATE_CONNECTED                        1
/*!	
        @def RX_TREAD_STATE_FAIL_DISCONNECTED		
        Fail state
 */
#define RX_TREAD_STATE_FAIL_DISCONNECTED                2
/*!	
        @def RX_TREAD_STATE_DISCONNECTED			
        Disconnected state
 */
#define RX_TREAD_STATE_DISCONNECTED                     3	


/// Maximum number of events in receive queue
#define MAX_TREAD_RECEIVE_EVENTS                        8192

/*!
        \class ctrlObjVscpTcpIf
        \brief Shared VSCP tcp/ip object among worker threads.
 */
class ctrlObjVscpTcpIf {
public:

    /// Constructor
    ctrlObjVscpTcpIf();

    /// Destructor
    ~ctrlObjVscpTcpIf();

    /// Username for VSCP server
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

class VSCPTCPIP_RX_WorkerThread : public wxThread {
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

class VSCPTCPIP_TX_WorkerThread : public wxThread {
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


