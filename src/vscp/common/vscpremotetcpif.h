///////////////////////////////////////////////////////////////////////////////
// vscpremotetcpif.h:
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
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

#include <arpa/inet.h>
#include <inttypes.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <semaphore.h>
#include <sockettcp.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#include <canal.h>
#include <guid.h>
#include <sockettcp.h>
#include <vscp.h>
#include <vscpdatetime.h>
#include <vscphelper.h>

//---------------------------------------------------------------------------

/*!
    @def DEFAULT_RESPONSE_TIMEOUT
    Default response timeout for communication with the
    tcp/ip interface of the daemon in milliseconds.

    This is the overall time to wait for a +OK respose.
 */
#define TCPIP_DEFAULT_RESPONSE_TIMEOUT 2000

/*!
    @def DEFAULT_INNER_RESPONSE_TIMEOUT
    Default timeout for inner data check. A Read call will
    always wait this long for data.
*/
#define TCPIP_DEFAULT_INNER_RESPONSE_TIMEOUT 0

#define TCPIP_DEFAULT_AFTER_COMMAND_SLEEP 0 // TODO remove !!!!!!!!!

// Default values for read/write register functions
// used in device config and scan.
#define TCPIP_REGISTER_READ_RESEND_TIMEOUT 1000
#define TCPIP_REGISTER_READ_ERROR_TIMEOUT 5000
#define TCPIP_REGISTER_READ_MAX_TRIES 3

#define TCPIP_DEFAULT_CONNECT_TIMEOUT_SECONDS 15 // Seconds
/*!
    @def TCPIP_DLL_VERSION
    Pseudo version string
 */
#define TCPIP_DLL_VERSION 0x00000010
/*!
    @def TCPIP_VENDOR_STRING
    Pseudo vendor string
 */
#define TCPIP_VENDOR_STRING "Grodans Paradis AB, Sweden"

#define DRIVER_INFO_STRING ""

/*!
    Use this macro for unused function parameters.
 */
#define TCPIP_UNUSED(__par) (void)(__par)

// Forward declarations
class VscpRemoteTcpIf;

/*!
    @brief Class for VSCP daemon tcp/ip interface
 */
class VscpRemoteTcpIf
{
  public:
    /// Constructor
    VscpRemoteTcpIf();

    /// Destructor
    virtual ~VscpRemoteTcpIf();

  public:

    /*!
        Set connect timeout.
        @param to Timeout value in **seconds**.
     */
    void setConnectTimeout(uint16_t to = TCPIP_DEFAULT_CONNECT_TIMEOUT_SECONDS)
    {
        m_connectionTimeOut = to;
    };

    /*!
        Get connection timeout (in seconds)
        @return Connection timeout in **seconds**
    */
    uint16_t getConnectTimeout(void) { return m_connectionTimeOut; };

    /*!
        Set response timeout.
        @param to Timeout value in milliseconds.
     */
    void setResponseTimeout(uint32_t to = TCPIP_DEFAULT_RESPONSE_TIMEOUT)
    {
        if (to < 100) to = 1000; // To be backward compatible
        if (to) m_responseTimeOut = to;
    };

    /*!
        Get response timeout
        @return Response timeout in milliseconds
    */
    uint32_t getResponseTimeout(void)  { return m_responseTimeOut; };

    /*!
        Set after command sleep time (milliseconds)
        @param to Sleep value in milliseconds.
     */
    void setAfterCommandSleep(uint16_t to)
    {
        TCPIP_UNUSED(to); // For backward compability
    }

    /*!
        Set register read/write timings
     */
    void setRegisterOperationTiming(uint8_t retries,
                                    uint32_t resendto,
                                    uint32_t errorto)
    {
        m_registerOpMaxRetries    = retries;
        m_registerOpResendTimeout = resendto;
        m_registerOpErrorTimeout  = errorto;
    };

    /*!
        Get last response form remote node
        @return Last raw response data from remote node
     */
    std::string &getLastResponse(void) { return m_strResponse; };

    /*!
        Get last response time
        @return Relative time in milliseconds when last valid received
            data was read.
    */
    uint32_t getlastResponseTime(void) { return m_lastResponseTime; };

    /*!
        Returns TRUE if we are connected false otherwise.
     */
    bool isConnected(void)
    {
        return ((NULL != m_conn) &&
                (STCP_CONN_STATE_CONNECTED == m_conn->conn_state));
    };

    /*!
        checkReturnValue
        @param bClear Clear the input bugger before starting to wait for
       received data.
        @return Return false for "-OK" and true for "+OK"
     */
    bool checkReturnValue(bool bClear = false);

    /*!
        Clear the input queue
     */
    void doClrInputQueue(void);

    /*!
        \brief Creates the input string array from a remote client response
        @return Number of rows in created string array.
    */
    size_t addInputStringArrayFromReply(bool bClear = false);

    /*!
        \brief Send a command to the remote client.
        @param cmd Commad to issue
        @return Returns VSCP_ERROR_SUCCESS if the command could be sent
       successfully.
     */
    int doCommand(const std::string &cmd);

    /*!
        \brief Send a command to the remote client.
        @param cmd Commad to issue
        @return Returns VSCP_ERROR_SUCCESS if the command could be sent
       successfully.
     */
    int doCommand(const char *cmd);

    /*!
        Open communication interface.
        @param strInterface should contain "username;password;ip-addr;port" if
       used. All including port are optional and defaults to no
       username/password, server as "localhost" and "9598" as default port.
        @param flags are not used at the moment.
        @return CANAL_ERROR_SUCCESS if channel is open or CANAL error code  if
       error or the channel is already opened or other error occur.
     */
    int doCmdOpen(const std::string &strInterface = std::string(""),
                  uint32_t flags                  = 0L);

    /*!
        Open communication interface.
        @param strHostname Host to connect to + port (host:port).
        @param strUsername Username.
        @param strPassword Username.
        @return CANAL_ERROR_SUCCESS if channel is open or CANAL error code if
       error or the channel is already opened or other error occur.
     */
    int doCmdOpen(const std::string &strHostname,
                  const std::string &strUsername,
                  const std::string &strPassword);

    /*!
        Open communication interface.
        @param strHostname Host to connect to.
        @param port Port of interface to connec to
        @param strUsername Username.
        @param strPassword Username.
        @return CANAL_ERROR_SUCCESS if channel is open or CANAL error code if
       error or the channel is already opened or other error occur.
     */
    int doCmdOpen(const std::string &strHostname,
                  short port,
                  const std::string &strUsername,
                  const std::string &strPassword);

    /*!
        Close communication interface
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdClose(void);

    /*!
        Write NOOP message through pipe.
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdNOOP(void);

    /*!
        Clear input queue
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdClear(void);

    /*!
        Get the Canal protocol level
        @return VSCP Level
     */
    unsigned long doCmdGetLevel(void) { return VSCP_LEVEL2; }

    /*!
        Send a VSCP Level II event through interface.
        If a GUID sent with all items set to zero the GUID of the
        interface ("-") will be used.
        @param pEvent VSCP Level II event to send.
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdSend(const vscpEvent *pEvent);

    /*!
        Send a VSCP ex event through interface.
        @param pEvent VSCP Level II ex event to send.
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdSendEx(const vscpEventEx *pEvent);

    /*!
        Send a Level I event through interface.
        @param pMsg VSCP Level I event (==canalMsg).
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdSendLevel1(const canalMsg *pMsg);

    /*!
        Receive a VSCP event through the interface.
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdReceive(vscpEvent *pEvent);

    /*!
        Receive an VSCP ex event through the interface.
        @return true if success false if not.
     */
    int doCmdReceiveEx(vscpEventEx *pEvent);

    /*!
        Receive an VSCP Level I event through the interface.
        For the extended and the RTR bit to be handled the
        CAN message bit in the head byte must be set.
        Low eight bits of the CAN id is fetched from GUID[15]
        that is LSB of GUID.
        @param Pointer to CANAL message
        @return CANAL_ERROR_SUCCESS if success false if not.
     */
    int doCmdReceiveLevel1(canalMsg *pCanalMsg);

    /*!
        This command sets an open interface in the receive loop (RCVLOOP).
        It does nothing other then putting the interface in the loop mode and
        checking that it went there.

        Note! The only way to terminate this receive loop is to close the
       session with doCmdClose (or just close the socket).

        @return CANAL_ERROR_SUCCESS if success CANAL_ERROR_GENERIC if not.
     */
    int doCmdEnterReceiveLoop(void);

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
        @return CANAL_ERROR_SUCCESS when event is received.
       CANAL_ERROR_FIFO_EMPTY is returned if no event was available.
       CANAL_ERROR_TIMEOUT on timeout.CANAL_ERROR_COMMUNICATION is returned if a
       socket error is detected.
     */
    int doCmdBlockingReceive(vscpEvent *pEvent, uint32_t timeout = 500);

    int doCmdBlockingReceive(vscpEventEx *pEventEx, uint32_t timeout = 500);

    /*!
        Get the number of events in the input queue of this interface
        @return the number of events available or if negative
        an error code.
     */
    int doCmdDataAvailable(void);

    /*!
        Receive CAN status through the interface.
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdStatus(canalStatus *pStatus);

    /*!
        A VSCP variant of the above
     */
    int doCmdStatus(VSCPStatus *pStatus);

    /*!
        Receive VSCP statistics through the interface.
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdStatistics(VSCPStatistics *pStatistics);

    /*!
        Receive CAN statistics through the interface.
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdStatistics(canalStatistics *pStatistics);

    /*!
        Set/Reset a filter through the interface.
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdFilter(const vscpEventFilter *pFilter);

    /*!
        Set/Reset filter through the interface.
        @param filter Filter on string form (priority,class,type,guid).
        @param mask Mask on string form (priority,class,type,guid).
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdFilter(const std::string &filter, const std::string &mask);

    /*!
        Get i/f version through the interface.
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdVersion(uint8_t *pMajorVer,
                     uint8_t *pMinorVer,
                     uint8_t *pSubMinorVer);

    /*!
        Get i/f version including build-version through the interface.
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdVersion(uint8_t *pMajorVer,
                     uint8_t *pMinorVer,
                     uint8_t *pSubMinorVer,
                     uint16_t *pBuildVer);

    /*!
        Get interface version
     */
    unsigned long doCmdDLLVersion(void);

    /*!
        Get vendor string
     */
    const char *doCmdVendorString(void);

    /*!
        Get Driver information string.
     */
    const char *doCmdGetDriverInfo(void);

    /*!
        Get GUID for this interface.
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdGetGUID(unsigned char *pGUID);

    /*!
        Get GUID for this interface.
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdGetGUID(cguid &ifguid);

    /*!
        Set GUID for this interface.
        @param pGUID Array with uint8_t GUID's. (Note! Not a string).
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdSetGUID(const unsigned char *pGUID);

    /*!
        Set GUID for this interface.
        @param ifguid GUID to set.
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdSetGUID(cguid &ifguid);

    /*!
        Get information about a channel..
        @param pChannelInfo The stucture that will get the information
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdGetChannelInfo(VSCPChannelInfo *pChannelInfo);

    /*!
        Get Channel ID for the open channel
        @param pChannelID Channel ID for channel
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdGetChannelID(uint32_t *pChannelID);

    /*!
        Get available interfaces
        @param array A string array that will get interface list
        @return CANAL_ERROR_SUCCESS on success and error code if failure.
     */
    int doCmdInterfaceList(std::deque<std::string> &array);

    /*!
        Get server capabilities WCYD
        WCYD (What Can You Do)
        @param pwcyd Pointer to uint64_t that receive capability code
        @return VSCP_ERROR_SUCCESS if OK, else failure code
    */
    int doCmdWcyd(uint64_t *pwcyd);

    /*!
        Dummy for Baudrate setting
     */
    int doCmdSetBaudrate(uint32_t baudrate)
    {
        TCPIP_UNUSED(baudrate);
        return CANAL_ERROR_SUCCESS;
    };

    /*!
        Dummy for filter setting
     */
    int doCmdFilter(uint32_t filter)
    {
        TCPIP_UNUSED(filter);
        return CANAL_ERROR_SUCCESS;
    };

    /*!
        Dummy for mask setting
     */
    int doCmdMask(uint32_t mask)
    {
        TCPIP_UNUSED(mask);
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
    bool getEventFromLine(vscpEvent *pEvent, const std::string &strLine);

    /*!
        Get server capabilities WCYD
        WCYD (What Can You Do)
        @param pwcyd Pointer to uint64_t that receive capability code
        @return VSCP_ERROR_SUCCESS if OK, else failure code
    */
    int doCmdWcyd(uint64_t &pwcyd);

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

    int readLevel2Register(uint32_t reg,
                           uint16_t page,
                           uint8_t *pval,
                           cguid &ifGUID,
                           cguid *pdestGUID = NULL,
                           bool bLevel2     = false);

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

    int readLevel2Registers(uint32_t reg,
                            uint16_t page,
                            uint8_t count,
                            uint8_t *pval,
                            cguid &ifGUID,
                            cguid *pdestGUID = NULL,
                            bool bLevel2     = false);

    /*!
        Write a level 2 register

        @param reg Register to write.
        @param pval Pointer to data to write. Return read data.
        @param interfaceGUID GUID for interface where devices sits whos register
                    should be read with byte 0 set to nickname id for the
       device.
        @param pdestGUID GUID for remote node.
        @return CANAL_ERROR_SUCCESS on success, errocode on failure.
     */
    int writeLevel2Register(uint32_t reg,
                            uint16_t page,
                            uint8_t *pval,
                            cguid &ifGUID,
                            cguid *pdestGUID = NULL,
                            bool bLevel2     = false);

    /*!
        Get MDf file from device registers

        @param pdestGUID Pointer to guid of node.
        @param strurl URL to MDF if call is successful.
        @param bLevel2 Set to true if this is a level II devive
        @return true on success, false on failure.
     */
    bool getMDFUrlFromLevel2Device(cguid &ifGUID,
                                   cguid &destGUID,
                                   std::string &strurl,
                                   bool bLevel2 = false);

    /*!
        Fetch guid for a tcp/ip interface from it's name.

        @param ifName Interface name
        @param guid Returnd GUID for interface
        @param Returns true on success.
     */
    int fetchIterfaceGUID(const std::string &ifName, cguid &guid);

    // ------------------------------------------------------------------------

  public:
  protected:
    /*!
        Array that gets filled with input lines
     */
    std::deque<std::string> m_inputStrArray;

    // Last read valid response in milliseconds
    // Used to detect dead (rcvloop) link.
    uint32_t m_lastResponseTime;

  private:
    /// Flag for active receive loop
    bool m_bModeReceiveLoop;

    /// Server connection timeout in **seconds**
    uint16_t m_connectionTimeOut;

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
    size_t getInputQueueCount(void);

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
    std::string m_strResponse;
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
#define RX_TREAD_STATE_NONE 0
/*!
        @def RX_TREAD_STATE_CONNECTED
        Connected state
 */
#define RX_TREAD_STATE_CONNECTED 1
/*!
        @def RX_TREAD_STATE_FAIL_DISCONNECTED
        Fail state
 */
#define RX_TREAD_STATE_FAIL_DISCONNECTED 2
/*!
        @def RX_TREAD_STATE_DISCONNECTED
        Disconnected state
 */
#define RX_TREAD_STATE_DISCONNECTED 3

/// Maximum number of events in receive queue
#define MAX_TREAD_RECEIVE_EVENTS 8192

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

    /// Username for VSCP server
    std::string m_strUsername;

    /// Password for VSCP server
    std::string m_strPassword;

    /// Host address for VSCP server
    std::string m_strHost;

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
        Send receive event instead of writing
        event to RX queue.
        m_pWnd must point to a valid window and
        the id for that window must be in m_wndID
        Also TX events will be activated if thread is
        started.
     */
    bool m_bUseRXTXEvents;

    /// Id for owner window (0 if not used).
    uint32_t m_wndID;

    /// Event Input queue
    std::deque<vscpEvent *> m_rxQueue;

    /// Protection for input queue
    pthread_mutex_t m_mutexRxQueue;
    sem_t m_semRxQueue;

    /// Max events in queue
    uint32_t m_maxRXqueue;

    /// Event output queue
    std::deque<vscpEvent *> m_txQueue;

    //@{
    /// Protection for output queue
    pthread_mutex_t m_mutexTxQueue;
    sem_t m_semTxQueue;
    //@}
};

/*!
        \class remotetcpipRxObj
        \brief This class implement a thread that handles
        receive events for a link to the VSCP daemon.
 */

class remotetcpipRxObj
{
  public:
    /// Constructor
    remotetcpipRxObj();

    /// Destructor
    virtual ~remotetcpipRxObj();

    /*!
            Pointer to control object.
     */
    ctrlObjVscpTcpIf *m_pCtrlObject;
};

/*!
        \class remotetcpipTxObj
        \brief This class implement a thread that handles
        transmit events for a link to the VSCP daemon.
 */

class remotetcpipTxObj
{
  public:
    /// Constructor
    remotetcpipTxObj();

    /// Destructor
    virtual ~remotetcpipTxObj();

    /*!
            Pointer to the daemon main control object.
     */
    ctrlObjVscpTcpIf *m_pCtrlObject;
};

#endif // !defined(VSCPREMOTETCPIF_H__INCLUDED_)
