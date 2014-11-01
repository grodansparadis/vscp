// ControlObject.h: interface for the CControlObject class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#if !defined(CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
#define CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_

#ifdef WIN32
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#endif

// NS_ENABLE_SSL		// Enable webserver SSL functionality

#include "wx/wx.h"
#include <wx/thread.h>
#include <wx/filename.h>

#include "../../common/net_skeleton.h"
#include "../../common/mongoose.h"

#include "devicelist.h"
#include "clientlist.h"
#include "interfacelist.h"
#include "userlist.h"
#include "tables.h"
#include "../../vscp/common/vscp.h"
#include "vscpvariable.h"
#include "vscpautomation.h"
#include "tcpipclientthread.h"
#include "udpclientthread.h"
#include "udpthread.h"
#include "daemonvscp.h"
#include "dm.h"
#include "vscp.h"

// Forward declarations
class VSCPWebServerThread;
class VSCPClientThread;
class CVSCPAutomation;

enum {
    DAEMON_LOGMSG_NONE = 0,
    DAEMON_LOGMSG_DEBUG,
    DAEMON_LOGMSG_INFO,
    DAEMON_LOGMSG_NOTICE,
    DAEMON_LOGMSG_WARNING,
    DAEMON_LOGMSG_ERROR,
    DAEMON_LOGMSG_CRITICAL,
    DAEMON_LOGMSG_ALERT,
    DAEMON_LOGMSG_EMERGENCY,    
};

enum {
    DAEMON_LOGTYPE_GENERAL = 0,
    DAEMON_LOGTYPE_SECURITY,
    DAEMON_LOGTYPE_ACCESS
};

// Needed on Linux
#ifndef VSCPMIN
#define VSCPMIN(X,Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef VSCPMAX
 #define VSCPMAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#endif	


#define MAX_ITEMS_RECEIVE_QUEUE                 1021
#define MAX_ITEMS_SEND_QUEUE                    1021
#define MAX_ITEMS_CLIENT_RECEIVE_QUEUE          8191
#define VSCP_TCPIPIF_DEFAULT_PORT				9598


WX_DECLARE_LIST(canalMsg, CanalMsgList);
WX_DECLARE_LIST(vscpEvent, VSCPEventList);
WX_DECLARE_LIST(vscpEventFilter, TRIGGERLIST);
WX_DECLARE_STRING_HASH_MAP( wxString, HashString );


/*!
    This class implement a thread that handles
    client receive events
 */

class clientMsgWorkerThread : public wxThread {
public:

    /// Constructor
    clientMsgWorkerThread();

    /// Destructor
    virtual ~clientMsgWorkerThread();

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
        Termination control
     */
    bool m_bQuit;

    /*!
        Pointer to control object.
     */
    CControlObject *m_pCtrlObject;

};


/*!
	This is the class that does the main work in the daemon.
*/

class CControlObject {
public:

    /*!
        Constructor
     */
    CControlObject(void);

    /*!
        Destructor
     */
    virtual ~CControlObject(void);

    /*!
      Write log message - 
    */ 
    void logMsg(const wxString& wxstr, 
                    const uint8_t level = DAEMON_LOGMSG_INFO, 
                    const uint8_t nType = DAEMON_LOGTYPE_GENERAL );

    /*!
        General initialization
     */
    bool init(wxString& strcfgfile);

    /*!
        Clean up used resources
     */
    bool cleanup(void);

    /*!
        The main worker thread
     */
    bool run(void);

    /*!
        Start worker threads for devices
        @return true on success
     */
    bool startDeviceWorkerThreads(void);

    /*!
        Stop worker threads for devices
        @return true on success	
     */
    bool stopDeviceWorkerThreads(void);

    /*!
        Starting daemon worker thread
        @return true on success
     */
    bool startDaemonWorkerThread(void);

    /*!
        Stop daemon worker thread
        @return true on success
     */
    bool stopDaemonWorkerThread(void);


    /*!
        Starting TCP/IP worker thread
        @return true on success
     */
    bool startTcpWorkerThread(void);

    /*!
        Stop the TCP/IP worker thread
        @return true on success
     */
    bool stopTcpWorkerThread(void);

	/*!
		Start the UDP worker thread
	*/
	bool startUDPWorkerThread(void);

	/*!
		Stop the UDP Workerthread
	*/
	bool stopUDPWorkerThread(void);

	/*!
        Starting WebServer worker thread
        @return true on success
     */
    bool startWebServerThread(void);

    /*!
        Stop the WebServer worker thread
        @return true on success
     */
    bool stopWebServerThread(void);

    /*!
        Starting Client worker thread
        @return true on success
     */
    bool startClientWorkerThread(void);

    /*!
        Stop Client worker thread
        @return true on success
     */
    bool stopClientWorkerThread(void);

    /*!
        Add a new client to the clinet list

        @param Pointer to client that should be added.
        @param Normally not used but can be used to set a special 
        client id.
     */
    void addClient(CClientItem *pClientItem, uint32_t id = 0);

    /*!
        Remove a new client from the clinet list

        @param pClientItem Pointer to client that should be added.
     */
    void removeClient(CClientItem *pClientItem);


    /*!
        Get device address for primary etehernet adapter

        @param guid class
     */
    bool getMacAddress(cguid& guid);


    /*!
        Get the first IP address computer is known under

        @param pGUID Pointer to GUID class
     */
    bool getIPAddress(cguid& guid);

    /*!
        Read configuration data
        @param strcfgfile path to configuration file.
        @return Returns true on success false on failure.
     */
    bool readConfiguration(wxString& strcfgfile);

	/*!
         Save configuration data
     */
    bool saveConfiguration(void);


    /*!
        send level II message to all clients
     */
    void sendEventToClient(CClientItem *pClientItem, vscpEvent *pEvent);

    /*!
        Send Level II event to all clients witch exception
     */
    void sendEventAllClients(vscpEvent *pEvent, uint32_t excludeID = 0);


    /*!
        Get clientmap index from a client id
     */
    uint32_t getClientMapFromId(uint32_t clid);

    /*!
        Get a client id from a clinet map index
     */
    uint32_t getClientMapFromIndex(uint32_t idx);

    /*!
        Add a client id to the clientmap
     */
    uint32_t addIdToClientMap(uint32_t clid);

    /*!
        Remove a client id to the clientmap
		@return Tfrue on success
     */
    bool removeIdFromClientMap(uint32_t clid);

	/*!
		Get the webserver thread
		@return Pointer to the webserver thread
	*/
	VSCPWebServerThread *getWebServer( void ) { return m_pwebServerThread; };

	/*!
		Get the VSCP TCP/IP thread thread
		@return Pointer to the VSCP TCP/IP thread thread
	*/ 
	VSCPClientThread *getTCPIPServer( void ) { return m_pVSCPClientThread; };

public:

#ifdef BUILD_VSCPD_SERVICE
    HANDLE m_hEvntSource;
#endif

    /*!
        true if we should quit
     */
    bool m_bQuit;

    /*!
        User to run asa for Unix
        if not ""
    */
    wxString m_runAsUser;

    /*!
        Maximum number of items in receiveq ueue for clients
     */
    uint32_t m_maxItemsInClientReceiveQueue;


    /*!
        Server GUID
        This is the GUID for the server
     */
    //uint8_t m_GUID[ 16 ];
	cguid m_guid;

    /*!
        ClientMap
        This structure maps client id's and
        The client map structure
        Maps unsigned log client id's to
        unsigned char id's for the
        GUID
     */
    uint32_t m_clientMap[ VSCP_MAX_CLIENTS ];

    /////////////////////////////////////////////////////////
	//                      Logging
	/////////////////////////////////////////////////////////

    wxMutex m_mutexLogWrite;

    /*!
        Log Level
     */
    uint8_t m_logLevel;

    /*!
        Path to general log file
    */
    bool m_bLogGeneralEnable;   // Enable general logfile
    wxFileName m_logGeneralFileName;
    wxFile m_fileLogGeneral;

    /*!
        Path to security log file
    */
    bool m_bLogSecurityEnable; // Enable security logfile
    wxFileName m_logSecurityFileName;
    wxFile m_fileLogSecurity;

    /*!
        Path to access log file
    */
    bool m_bLogAccessEnable; // Enable security logfile
    wxFileName m_logAccessFileName;
    wxFile m_fileLogAccess;


	/////////////////////////////////////////////////////////
	//              Enable/disable switches
	/////////////////////////////////////////////////////////

    /*!
        Enable control (TCP/IP) interface
     */
    bool m_bTCPInterface;

	/*!
		Enable UDP interface
	*/
	bool m_bUDPInterface;

	/*!
        Enable DM functionality
     */
    bool m_bDM;

	/*!
        Enable CANAL Driver functionality
     */
    bool m_bEnableLevel1Drivers;

    /*!
        Enable Level2 Driver functionality
    */
    bool m_bEnableLevel2Drivers;

    /*!
        Enable VSCP Daemon functionality
     */
    bool m_bVSCPDaemon;

    /*!
        Enable variable usage
     */
    bool m_bVariables;


	/////////////////////////////////////////////////////////
	//                  Communication
	/////////////////////////////////////////////////////////

	/// net_skeleton structure
	struct ns_mgr m_mgrTcpIpServer;

    /// Interface(s) used for TCP/IP connection
    wxString m_strTcpInterfaceAddress;

	/// INterface(s) to use for UDP
	wxString m_strUDPInterfaceAddress;


	/////////////////////////////////////////////////////////
	//                     VARIABLES
	/////////////////////////////////////////////////////////

    /*!
        Hash table for variables
     */
    CVariableStorage m_VSCP_Variables;

    /*!
        Mutex to protect variables
     */
    wxMutex m_variableMutex;

    /*!
        Event source for NT event reporting
     */
#ifdef BUILD_VSCPD_SERVICE
    HANDLE m_hEventSource;
#endif


    /*!
        Mutex for client queue
     */
    wxMutex m_wxClientMutex;


    /*!
        Mutex for device queue
     */
    wxMutex m_wxDeviceMutex;


  
    /// Daemon Decision Matrix Object
    CDM m_dm;


    /// Automation Object
    CVSCPAutomation m_automation;
   

    /*!
        Username for level II divers
     */
    wxString m_driverUsername;

    /*!
        Password for Level II drivers
     */
    wxString m_driverPassword;


    //*****************************************************
    //            websocket/webserver interface
    //*****************************************************

    // Enable/disable full webserver
    bool m_bWebServer;

	struct mg_server *m_pwebserver;

	/// Extra mime types on the form "extension1=type1,extension2=type2,..."
    wxString m_extraMimeTypes;
	
    // Path to web root
    wxString m_pathWebRoot;

    // Domain for webserver and other net services
	wxString m_authDomain;

    // webserver port as port "8080" or address + port "127.0.0.1:8080"
    // If only port will bind to all interfaces,
    wxString m_portWebServer;	// defaults to "8080"

    // Path to SSL certificate
    wxString m_pathCert;

    // CGI interpreter to use
    wxString m_cgiInterpreter;

    // CGI Pattern
    wxString m_cgiPattern;

    // Enable directory listing
    bool bEnableDirectoryListing;

    // Hide file pattern
    wxString m_hideFilePatterns;

    // Index files
    wxString m_indexFiles;

    // URL rewrites
    wxString m_urlRewrites;

    // Run as user
    wxString m_runAsUserWeb;

    // * * Websockets * *

    // Enable disable web socket interface
    bool m_bWebSockets;

	// websocket authentivcation is needed  (if true)
	bool m_bAuthWebsockets;
	

    //*****************************************************
    //                      Lists
    //*****************************************************
    /*!
        The list with available devices.
     */
    CDeviceList m_deviceList;
    wxMutex m_mutexDeviceList;

    /*!
        The list with active clients.
     */
    CClientList m_clientList;
    wxMutex m_mutexClientList;


    /*!
        The list of users
     */
    CUserList m_userList;
    wxMutex m_mutexUserList;

	/*!
		This is a list with defined tables
	*/
	listVSCPTables m_listTables;
	wxMutex m_mutexTableList;


    // *************************************************************************


    /*!
        Send queue

        This is the send queue for all clients attached to the system. A client
	 *  place events here and the system distribute it to all other clients.
     */
    VSCPEventList m_clientOutputQueue;

    /*!
        Event object to indicate that there is an event in the client output queue.
     */
    wxSemaphore m_semClientOutputQueue;

    /*!
        Mutex for Level II message send queue
     */
    wxMutex m_mutexClientOutputQueue;

    // *************************************************************************


private:

    //*****************************************************
    //                          Threads
    //*****************************************************

    /*!
        controlobject device thread
     */
    clientMsgWorkerThread *m_pclientMsgWorkerThread;
    wxMutex m_mutexclientMsgWorkerThread;

    /*!
        The server thread for the TCP connection interface
     */
    VSCPClientThread *m_pVSCPClientThread;
    wxMutex m_mutexTcpClientListenThread;

    /*!
        The server thread for the VSCP daemon
     */
    daemonVSCPThread *m_pdaemonVSCPThread;
    wxMutex m_mutexdaemonVSCPThread;

	/*!
        The server thread for the VSCP daemon
     */
	VSCPUDPClientThread *m_pVSCPClientUDPThread;
	wxMutex m_mutexVSCPClientnUDPThread;

	/*!
		Webserver
	*/
	VSCPWebServerThread *m_pwebServerThread;
	wxMutex m_mutexwebServerThread;

    /*!
        UDP Worker threads
     */
    //UDPSendThread *m_pudpSendThread;
    //UDPReceiveThread *m_pudpReceiveThread;

    //wxMutex m_mutexudpSendThread;
    //wxMutex m_mutexudpReceiveThread;
};




#endif // !defined(AFX_CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
