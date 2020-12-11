// ControlObject.h: interface for the CControlObject class.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2020 Ake Hedman, Grodans Paradis AB
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
//

#if !defined(CONTROLOBJECT_H__INCLUDED_)
#define CONTROLOBJECT_H__INCLUDED_

#include <sqlite3.h>

#include <devicelist.h>
#include <mqtt.h>
#include <vscpmqtt.h>
#include <vscp.h>

#include <set>
#include <map>

#include <json.hpp>  // Needs C++11  -std=c++11
#include <mustache.hpp>

// https://github.com/nlohmann/json
using json = nlohmann::json;

using namespace kainjow::mustache;

// Needed on Linux
#ifndef VSCPMIN
#define VSCPMIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef VSCPMAX
#define VSCPMAX(a, b)                                                          \
    ({                                                                         \
        __typeof__(a) _a = (a);                                                \
        __typeof__(b) _b = (b);                                                \
        _a > _b ? _a : _b;                                                     \
    })
#endif

#define VSCP_MAX_DEVICES 1024 // abs. max. is 0xffff


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
        Get server capabilities (64-bit array)
        @param pCapability Pointer to 64 bit capabilities array
        @return True on success.
     */
    bool getVscpCapabilities(uint8_t* pCapability);

    /*!
        General initialisation
     */
    bool init(std::string& strcfgfile, std::string& rootFolder);

    /*!
        Clean up used resources
     */
    bool cleanup(void);

    /*!
        The main worker thread
     */
    bool run(void);

    /*!
        Send event on all publishing channels
        @param pex Pointer to VSCP event ex
        @return true on success, false on failure
    */
    bool sendEvent(vscpEventEx *pex);

    /*!
        Handle and send periodic events 
        (previous automation)
        @return true on success, false on failure
    */
    bool periodicEvents(void);


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
        Get device address for primary ethernet adapter

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
    bool readConfiguration(const std::string& strcfgfile);


    /*!
        Read the loaded JSON configuration
        @param j JSON object
        @return Returns true on success false on failure.
    */
    bool readJSON(const json& j);

    /*!
        Read the encryption key from a safe location
        @param path Path to file containg the +128 byte key
        @return true on success, false on failure.
    */
    bool readEncryptionKey(const std::string& path);

    /*!
        Discovery routine

        Collects GUID's in a database and allow them
        to be named.

        @param pev Pointer to VSCP event
    */
    void discovery(vscpEvent *pev);

    /*!
        Get VSCP class token from id

        @param id VSCP class 
        @return VSCP class token as string
    */
    std::string getTokenFromClassId(uint16_t id) { return m_map_class_id2Token[id]; }

    /*!
        Get VSCP type token from class and type id

        @param cid VSCP class 
        @param tid VSCP class 
        @return VSCP class token as string
    */
    std::string getTokenFromTypeId(uint16_t cid, uint16_t tid) { return m_map_type_id2Token[((cid << 16) + tid)]; }

 public:

    // Will quit if set to true
    bool m_bQuit;

    // User configurable server name
    std::string m_strServerName;

    // VSCP daemon root folder
    std::string m_rootFolder;

    // Secret key
    uint8_t m_systemKey[32];

    // VSCP encryption token
    std::string m_vscptoken;

    /*!
        User to run as for Unix
        if not ""
    */
    std::string m_runAsUser;

    // GUID for this server
    // Set to MAC address if not set explicit
    cguid m_guid;


    /*!
        Enabling bEventOutLookup will add clear
        text VSCP event information to events in
        the following fields

        vscpClassToken
        vscpTypeToken

        This is not standard.
    */
    bool bEventOutLookup;


    //**************************************************************************
    //                                  DEBUG
    //**************************************************************************

    /*!
     * Debug flags
     * See vscp_debug.h for possible flags.
     */
    uint64_t m_debugFlags;


    //**************************************************************************
    //                                DATABASE
    //**************************************************************************


    /*!
        Path to class/type definition database
    */
    std::string m_pathClassTypeDefinitionDb;

    std::map<uint16_t, std::string> m_map_class_id2Token;   // vscp_class -> class_token
    std::map<std::string, uint16_t> m_map_class_token2Id;   // class_token -> vscp_class

    std::map<uint32_t, std::string> m_map_type_id2Token;    // ((vscp_class << 16) + vscp_type) -> type_token
    std::map<std::string, uint32_t> m_map_type_token2Id;    // type_token -> ((vscp_class << 16) + vscp_type)

    /*!
        Path to discovery database
        Set empty to disable functionality
    */
    std::string m_pathMainDb;
    sqlite3 *m_db_vscp_daemon;

    std::map<std::string, std::string> m_map_discoveryGuidToName;   // key = GUID, value = name

    //**************************************************************************
    //                                 DRIVERS
    //**************************************************************************

    // The list with available devices.
    CDeviceList m_deviceList;
    pthread_mutex_t m_mutex_DeviceList;

    // Mutex for device queue
    pthread_mutex_t m_mutex_deviceList;



    //**************************************************************************
    //                                  MQTT 
    //*************************************************************************

    std::string m_mqtt_strHost;     // MQTT broker
    unsigned short m_mqtt_port;     // MQTT broker port    
    std::string m_mqtt_strClientId;    // Client id
    std::string m_mqtt_strUserName; // Username
    std::string m_mqtt_strPassword; // Password
    int m_mqtt_qos;                 // Quality of service (0/1/2)
    bool m_mqtt_bRetain;            // Enable retain
    int m_mqtt_keepalive;           // Keep alive in seconds
    bool m_mqtt_bCleanSession;      // Clean session on disconnect if true
    uint16_t m_mqtt_reconnect_delay;           // reconnect delay
    uint16_t m_mqtt_reconnect_delay_max;       // max time between reconnects
    bool m_mqtt_reconnect_exponential_backoff; // Use exponential backoff

    // SSL/TSL
    bool m_mqtt_bTLS;               // True of a TLS/SSL connection should be done

    std::string m_mqtt_cafile;	    // path to a file containing the PEM encoded trusted CA certificate files.  
                                    // Either cafile or capath must not be NULL.
    std::string m_mqtt_capath;	    // path to a directory containing the PEM encoded trusted CA certificate files.  
                                    // See mosquitto.conf for more details on configuring this directory.  
                                    // Either cafile or capath must not be NULL.
    std::string m_mqtt_certfile;	// path to a file containing the PEM encoded certificate file for this client.  
                                    // If NULL, keyfile must also be NULL and no client certificate will be used.
    std::string m_mqtt_keyfile;      

    std::string m_mqtt_pwKeyfile;    // Password for keyfile (set only if it is encrypted on disc)

    std::list<std::string> m_mqtt_subscriptions;    // Subscribe topic templates
    std::list<std::string> m_mqtt_publish;          // Publish topic templates

    int m_mqtt_id;                   // Message id - the send function will set this to the message id of this particular 
                                     // message.  This can then be used with the subscribe callback to determine when 
                                     // the message has been sent.
    enumMqttMsgFormat m_mqtt_format; // Format for mqtt events (JSON/XML)

    std::string m_topicInterfaces;   // Topic on which the dameon publish it's interfaces
                                     // Default is vscp/{{guid}}/interfaces

  private:
           
    struct mosquitto *m_mosq;   // Handel for MQTT connection
};

#endif // !defined(CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
