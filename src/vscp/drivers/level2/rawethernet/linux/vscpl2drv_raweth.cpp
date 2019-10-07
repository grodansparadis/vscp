// rawethernet.cpp: implementation of the CRawEthernet class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP Project (http://www.vscp.org)
//
// Copyright (C) 2000-2019 Ake Hedman,
// Grodans Paradis AB, <akhe@grodansparadis.com>
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
// http://www.opensourceforu.com/2011/02/capturing-packets-c-program-libpcap/
//

#include <list>
#include <map>
#include <string>

#include "limits.h"
#include "stdlib.h"
#include "syslog.h"
#include "unistd.h"
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <ctype.h>
#include <libgen.h>
#include <net/if.h>
#include <pcap.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>

#include <expat.h>

#include <vscp_class.h>
#include <vscp_type.h>
#include <vscphelper.h>
#include <vscpremotetcpif.h>

#include "vscpl2drv_raweth.h"

#define XML_BUFF_SIZE   10000

// Worker threads
void *writeWorkerThread(void *pData);
void *readWorkerThread(void *pData);

void
_init() __attribute__((constructor));
void
_fini() __attribute__((destructor));

// This map holds driver handles/objects
static std::map<long, CRawEthernet *> g_ifMap;

// Mutex for the map object
static pthread_mutex_t g_mapMutex;

////////////////////////////////////////////////////////////////////////////
// DLL constructor
//

void
_init()
{
    pthread_mutex_init(&g_mapMutex, NULL);
}

////////////////////////////////////////////////////////////////////////////
// DLL destructor
//

void
_fini()
{
    // If empty - nothing to do
    if (g_ifMap.empty()) return;

    // Remove orphan objects

    LOCK_MUTEX(g_mapMutex);

    for (std::map<long, CRawEthernet *>::iterator it = g_ifMap.begin();
         it != g_ifMap.end();
         ++it) {
        // std::cout << it->first << " => " << it->second << '\n';

        CRawEthernet *pif = it->second;
        if (NULL != pif) {
            pif->m_srv.doCmdClose();
            delete pif;
            pif = NULL;
        }
    }

    g_ifMap.clear(); // Remove all items

    UNLOCK_MUTEX(g_mapMutex);
    pthread_mutex_destroy(&g_mapMutex);
}

///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long
addDriverObject(CRawEthernet *pif)
{
    std::map<long, CRawEthernet *>::iterator it;
    long h = 0;

    LOCK_MUTEX(g_mapMutex);

    // Find free handle
    while (true) {
        if (g_ifMap.end() == (it = g_ifMap.find(h))) break;
        h++;
    };

    g_ifMap[h] = pif;
    h += 1681;

    UNLOCK_MUTEX(g_mapMutex);

    return h;
}

///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

CRawEthernet *
getDriverObject(long h)
{
    std::map<long, CRawEthernet *>::iterator it;
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return NULL;

    it = g_ifMap.find(idx);
    if (it != g_ifMap.end()) {
        return it->second;
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void
removeDriverObject(long h)
{
    std::map<long, CRawEthernet *>::iterator it;
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return;

    LOCK_MUTEX(g_mapMutex);
    it = g_ifMap.find(idx);
    if (it != g_ifMap.end()) {
        CRawEthernet *pObj = it->second;
        if (NULL != pObj) {
            delete pObj;
            pObj = NULL;
        }
        g_ifMap.erase(it);
    }
    UNLOCK_MUTEX(g_mapMutex);
}

///////////////////////////////////////////////////////////////////////////////
//                         V S C P   D R I V E R -  A P I
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// VSCPOpen
//

extern "C" long
VSCPOpen(const char *pUsername,
         const char *pPassword,
         const char *pHost,
         short port,
         const char *pPrefix,
         const char *pParameter,
         unsigned long flags)
{
    long h = 0;

    CRawEthernet *pdrvObj = new CRawEthernet();
    if (NULL != pdrvObj) {

        if (pdrvObj->open(
              pUsername, pPassword, pHost, port, pPrefix, pParameter, flags)) {

            if (!(h = addDriverObject(pdrvObj))) {
                delete pdrvObj;
            }

        } else {
            delete pdrvObj;
        }
    }

    return h;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPClose
//

#ifdef WIN32
extern "C" int WINAPI EXPORT
VSCPClose(long handle)
#else

extern "C" int
VSCPClose(long handle)
#endif
{
    int rv = 0;

    CRawEthernet *pdrvObj = getDriverObject(handle);
    if (NULL == pdrvObj) return 0;
    pdrvObj->close();
    removeDriverObject(handle);
    rv = 1;
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPBlockingSend
//

extern "C" int
VSCPBlockingSend(long handle, const vscpEvent *pEvent, unsigned long timeout)
{
    int rv = 0;

    CRawEthernet *pdrvObj = getDriverObject(handle);
    if (NULL == pdrvObj) return CANAL_ERROR_MEMORY;

    pdrvObj->addEvent2SendQueue(pEvent);

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPBlockingReceive
//

extern "C" int
VSCPBlockingReceive(long handle, vscpEvent *pEvent, unsigned long timeout)
{
    int rv = 0;

    // Check pointer
    if (NULL == pEvent) return CANAL_ERROR_PARAMETER;

    CRawEthernet *pdrvObj = getDriverObject(handle);
    if (NULL == pdrvObj) return CANAL_ERROR_MEMORY;

    struct timespec ts;
    ts.tv_sec  = 0;
    ts.tv_nsec = timeout * 1000;
    if (ETIMEDOUT == sem_timedwait(&pdrvObj->m_semReceiveQueue, &ts)) {
        return CANAL_ERROR_TIMEOUT;
    }

    pthread_mutex_lock(&pdrvObj->m_mutexReceiveQueue);
    vscpEvent *pLocalEvent = pdrvObj->m_receiveList.front();
    pdrvObj->m_receiveList.pop_front();
    pthread_mutex_unlock(&pdrvObj->m_mutexReceiveQueue);
    if (NULL == pLocalEvent) return CANAL_ERROR_MEMORY;

    vscp_copyVSCPEvent(pEvent, pLocalEvent);
    vscp_deleteVSCPevent(pLocalEvent);

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPGetLevel
//

#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT
VSCPGetLevel(void)
#else

extern "C" unsigned long
VSCPGetLevel(void)
#endif
{
    return CANAL_LEVEL_USES_TCPIP;
}

///////////////////////////////////////////////////////////////////////////////
// VSCPGetDllVersion
//

#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT
VSCPGetDllVersion(void)
#else

extern "C" unsigned long
VSCPGetDllVersion(void)
#endif
{
    return VSCP_DLL_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// VSCPGetVendorString
//

#ifdef WIN32
extern "C" const char *WINAPI EXPORT
VSCPGetVendorString(void)
#else

extern "C" const char *
VSCPGetVendorString(void)
#endif
{
    return VSCP_DLL_VENDOR;
}

///////////////////////////////////////////////////////////////////////////////
// VSCPGetDriverInfo
//

#ifdef WIN32
extern "C" const char *WINAPI EXPORT
VSCPGetDriverInfo(void)
#else

extern "C" const char *
VSCPGetDriverInfo(void)
#endif
{
    return VSCP_RAWETH_DRIVERINFO;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPGetVSCPGetWebPageTemplate
//

extern "C" long
VSCPGetWebPageTemplate(long handle, const char *url, char *page)
{
    page = NULL;

    // Not implemented
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPGetVSCPWebPageInfo
//

extern "C" int
VSCPGetWebPageInfo(long handle, const struct vscpextwebpageinfo *info)
{
    // Not implemented
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPWebPageupdate
//

extern "C" int
VSCPWebPageupdate(long handle, const char *url)
{
    // Not implemented
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
//                            T H E  C O D E
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CRawEthernet
//

CRawEthernet::CRawEthernet()
{
    m_bQuit            = false;
    m_preadWorkThread  = NULL;
    m_pwriteWorkThread = NULL;
    m_interface        = "eth0";
    memset(m_localMac, 0, 16);
    m_subaddr = 0;

    vscp_clearVSCPFilter(&m_vscpfilter); // Accept all events
}

//////////////////////////////////////////////////////////////////////
// ~CRawEthernet
//

CRawEthernet::~CRawEthernet()
{
    close();
}

//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool
CRawEthernet::addEvent2SendQueue(const vscpEvent *pEvent)
{
    pthread_mutex_lock(&m_mutexSendQueue);
    m_sendList.push_back((vscpEvent *)pEvent);
    sem_post(&m_semSendQueue);
    pthread_mutex_unlock(&m_mutexSendQueue);
    return true;
}

// ----------------------------------------------------------------------------

/*
    XML Setup
    =========

    <setup interface="A string that identifies the Ethernet device"
            localmac="The mac address used as the outgoing mac address"
            subaddr="sub address for i/f"
            filter="filter for outgoing traffic"
            mask="mask for outgoing traffic"
    />
*/

// ----------------------------------------------------------------------------

int depth_setup_parser = 0;

void
startSetupParser(void *data, const char *name, const char **attr)
{
    CRawEthernet *pObj = (CRawEthernet *)data;
    if (NULL == pObj) return;

    if ((0 == strcmp(name, "setup")) && (0 == depth_setup_parser)) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == strcasecmp(attr[i], "interface")) {
                if (!attribute.empty()) {
                    pObj->m_interface = attribute;
                }
            } else if (0 == strcasecmp(attr[i], "localmac")) {
                if (!attribute.empty()) {
                    vscp_makeUpper(attribute);
                    std::deque<std::string> tokens_mac;
                    vscp_split(tokens_mac, attribute, ":");
                    for (int i = 0; i < 6; i++) {
                        if (tokens_mac.empty()) break;
                        std::string str = "0X" + tokens_mac.front();
                        tokens_mac.pop_front();
                        pObj->m_localMac[i] = vscp_readStringValue(str);
                    }
                }
            } else if (0 == strcasecmp(attr[i], "subaddr")) {
                if (!attribute.empty()) {
                    pObj->m_subaddr = vscp_readStringValue(attribute);
                }
            }
            else if (0 == strcasecmp(attr[i], "filter")) {
                if (!attribute.empty()) {
                    if (!vscp_readFilterFromString(&pObj->m_vscpfilter,
                                                   attribute)) {
                        syslog(LOG_ERR, "Unable to read event receive filter.");
                    }
                }
            }
            else if (0 == strcasecmp(attr[i], "mask")) {
                if (!attribute.empty()) {
                    if (!vscp_readMaskFromString(&pObj->m_vscpfilter,
                                                 attribute)) {
                        syslog(LOG_ERR, "Unable to read event receive mask.");
                    }
                }
            }
        }
    }

    depth_setup_parser++;
}

void
endSetupParser(void *data, const char *name)
{
    depth_setup_parser--;
}

// ----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////
// open
//
//

bool
CRawEthernet::open(const char *pUsername,
                   const char *pPassword,
                   const char *pHost,
                   short port,
                   const char *pPrefix,
                   const char *pConfig,
                   unsigned long flags)
{
    bool rv         = true;
    std::string str = std::string(pConfig);

    m_username = std::string(pUsername);
    m_password = std::string(pPassword);
    m_host     = std::string(pHost);
    m_port     = port;
    m_prefix   = std::string(pPrefix);

    // Parse the configuration string. It should
    // have the following form path

    std::deque<std::string> tokens;
    vscp_split(tokens, std::string(pConfig), ";");

    // Look for raw Ethernet interface in configuration string
    if (!tokens.empty()) {
        // Interface
        m_interface = tokens.front();
        tokens.pop_front();
    }

    // Local Mac
    std::string localMac;
    if (!tokens.empty()) {
        localMac = tokens.front();
        tokens.pop_front();
        vscp_makeUpper(localMac);
        std::deque<std::string> tokens_mac;
        vscp_split(tokens_mac, localMac, ":");
        for (int i = 0; i < 6; i++) {
            if (tokens_mac.empty()) break;
            std::string str = "0X" + tokens_mac.front();
            tokens_mac.pop_front();
            m_localMac[i] = vscp_readStringValue(str);
        }
    }

    // First log on to the host and get configuration
    // variables

    if (VSCP_ERROR_SUCCESS !=
        m_srv.doCmdOpen(m_host, m_port, m_username, m_password)) {
        syslog(LOG_ERR,
               "%s",
               (const char
                  *)"Unable to connect to VSCP TCP/IP interface. Terminating!");
        return false;
    }

    // Find the channel id
    uint32_t ChannelID;
    m_srv.doCmdGetChannelID(&ChannelID);

    // The server should hold configuration data for each sensor
    // we want to monitor.
    //
    // We look for
    //
    //	 _interface - The ethernet interface to use. Typically this
    //					is “eth0, eth0, eth1...
    //
    //   _localmac - The MAC address for our outgoing frames.
    //					Typically on the form 00:26:55:CA:1F:DA
    //
    //   _filter - Standard VSCP filter in string form.
    //				   1,0x0000,0x0006,
    //				   ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
    //				as priority,class,type,GUID
    //				Used to filter what events that is received from
    //				the socketcan interface. If not give all events
    //				are received.
    //	 _mask - Standard VSCP mask in string form.
    //				   1,0x0000,0x0006,
    //				   ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
    //				as priority,class,type,GUID
    //				Used to filter what events that is received from
    //				the socketcan interface. If not give all events
    //				are received.
    //
    //	 _subaddr - Normally the subaddr of the computer the rawtherent
    //              driver is on is 0x0000 this can be changed with this
    //              vaeiable
    //

    std::string strName = m_prefix + std::string("_interface");
    if (VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue(strName, str)) {
        m_interface = str;
    }

    // Local Mac
    strName = m_prefix + std::string("_localmac");

    if (VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue(strName, str)) {

        localMac = str;

        if (!tokens.empty()) {
            localMac = tokens.front();
            tokens.pop_front();
            vscp_makeUpper(localMac);
            std::deque<std::string> tokens_mac;
            vscp_split(tokens_mac, localMac, ":");
            for (int i = 0; i < 6; i++) {
                if (tokens_mac.empty()) break;
                std::string str = "0X" + tokens_mac.front();
                tokens_mac.pop_front();
                m_localMac[i] = vscp_readStringValue(str);
            }
        }
    }

    strName = m_prefix + std::string("_filter");
    if (VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue(strName, str)) {
        vscp_readFilterFromString(&m_vscpfilter, str);
    }

    strName = m_prefix + std::string("_mask");
    if (VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue(strName, str)) {
        vscp_readMaskFromString(&m_vscpfilter, str);
    }

    // subaddr
    strName = m_prefix + std::string("_subaddr");
    if (VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue(strName, str)) {
        m_subaddr = vscp_readStringValue(str);
    }

    // start the read workerthread
    if (pthread_create(m_readWrkThread, NULL, readWorkerThread, this)) {

        syslog(LOG_ERR, "Unable to start read worker thread.");
        return false;
    }

    // XML setup
    std::string strSetupXML;

    strName = m_prefix + std::string("_setup");
    if (VSCP_ERROR_SUCCESS ==
        m_srv.getRemoteVariableValue(strName, strSetupXML, true)) {
        XML_Parser xmlParser = XML_ParserCreate("UTF-8");
        XML_SetUserData(xmlParser, this);
        XML_SetElementHandler(xmlParser, startSetupParser, endSetupParser);

        int bytes_read;
        void *buff = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);

        strncpy((char *)buff, strSetupXML.c_str(), strSetupXML.length());

        bytes_read = strSetupXML.length();
        if (!XML_ParseBuffer(xmlParser, bytes_read, bytes_read == 0)) {
            syslog(LOG_ERR, "Failed parse XML setup.");
        }

        XML_ParserFree(xmlParser);
    }

    // start the write workerthread
    // start the read workerthread
    if (pthread_create(m_writeWrkThread, NULL, writeWorkerThread, this)) {

        syslog(LOG_ERR, "Unable to start read worker thread.");
        return false;
    }

    // Close the channel
    m_srv.doCmdClose();

    return rv;
}

//////////////////////////////////////////////////////////////////////
// close
//

void
CRawEthernet::close(void)
{
    // Do nothing if already terminated
    if (m_bQuit) return;

    close();

    m_bQuit = true; // terminate the thread
    sleep(1);       // Give the thread some time to terminate
}

//////////////////////////////////////////////////////////////////////
//                      CWrkReadThreadObj
//////////////////////////////////////////////////////////////////////

CWrkReadThreadObj::CWrkReadThreadObj()
{
    m_pObj = NULL;
}

CWrkReadThreadObj::~CWrkReadThreadObj()
{
    ;
}

//////////////////////////////////////////////////////////////////////
// readWorkerThread
//

void *
readWorkerThread(void *pData)
{
    pcap_t *fp;
    char errbuf[PCAP_ERRBUF_SIZE];

    CRawEthernet *pObj = (CRawEthernet *)pData;
    if (NULL == pObj) {
        syslog(LOG_ERR, "No data object supplied for read thread.");
        return NULL;
    }

    // Open the adapter
    if (NULL ==
        (fp = pcap_open_live(
           (const char *)pObj->m_interface.c_str(), // name of the device
           65536, // portion of the packet to capture. It doesn't matter in this
                  // case
           1,     // promiscuous mode (nonzero means promiscuous)
           1000,  // read timeout
           errbuf // error buffer
           ))) {
        syslog(LOG_ERR,
               "RawEthDrv: Unable to open the adapter or %s is not supported "
               "by pcap. Err=%s",
               (const char *)pObj->m_interface.c_str(),
               errbuf);
        return NULL;
    }

    int rv;
    struct pcap_pkthdr *header;
    const u_char *pkt_data;

    while (!pObj->m_bQuit) {

        rv = pcap_next_ex(fp, &header, &pkt_data);

        // Check for timeout
        if (0 == rv) continue;

        // Check for error
        if (rv < 0) {

            syslog(LOG_ERR,
                   "RawEthDrv: Error while getting packet. Err=%s",
                   pcap_geterr(fp));
        }

        // If from our own interface we skip
        if (0 == memcmp(pkt_data + 6, pObj->m_localMac, 6)) continue;

        // Check if this is VSCP
        if ((0x25 == pkt_data[12]) && (0x7e == pkt_data[13])) {

            // We have a packet - send it as a VSCP event
            vscpEventEx eventex;

            eventex.head = pkt_data[15] & 0xe0; // Priority

            eventex.GUID[0]  = 0xff; // Ethernet predefined  GUID
            eventex.GUID[1]  = 0xff;
            eventex.GUID[2]  = 0xff;
            eventex.GUID[3]  = 0xff;
            eventex.GUID[4]  = 0xff;
            eventex.GUID[5]  = 0xff;
            eventex.GUID[6]  = 0xff;
            eventex.GUID[7]  = 0xfe;
            eventex.GUID[8]  = pkt_data[6]; // Source MAC address
            eventex.GUID[9]  = pkt_data[7];
            eventex.GUID[10] = pkt_data[8];
            eventex.GUID[11] = pkt_data[9];
            eventex.GUID[12] = pkt_data[10];
            eventex.GUID[13] = pkt_data[11];
            eventex.GUID[14] = pkt_data[19]; // Device sub address
            eventex.GUID[15] = pkt_data[20];

            eventex.timestamp = (pkt_data[21] << 24) + (pkt_data[22] << 16) +
                                (pkt_data[23] << 8) + pkt_data[24];

            eventex.obid = (pkt_data[25] << 24) + (pkt_data[26] << 16) +
                           (pkt_data[27] << 8) + pkt_data[28];

            eventex.vscp_class = (pkt_data[29] << 8) + pkt_data[30];

            eventex.vscp_type = (pkt_data[31] << 8) + pkt_data[32];

            eventex.sizeData = (pkt_data[33] << 8) + pkt_data[34];

            // Validate data size
            if (eventex.vscp_class < 512) {
                if (eventex.sizeData > 8) eventex.sizeData = 8;
            } else if (eventex.vscp_class < 512) {
                if (eventex.sizeData > (16 + 8)) eventex.sizeData = 24;
            } else {
                if (eventex.sizeData > VSCP_MAX_DATA)
                    eventex.sizeData = VSCP_MAX_DATA;
            }

            // If the packet is smaller then the set datasize just
            // disregard it
            if ((eventex.sizeData + 35) > (uint16_t)header->len) continue;

            memcpy(eventex.data, pkt_data + 35, eventex.sizeData);

            vscpEvent *pEvent = new vscpEvent;
            if (NULL != pEvent) {

                vscp_convertVSCPfromEx(pEvent, &eventex);

                if (vscp_doLevel2FilterEx(&eventex, &pObj->m_vscpfilter)) {
                    pthread_mutex_lock(&pObj->m_mutexReceiveQueue);
                    pObj->m_receiveList.push_back(pEvent);
                    sem_post(&pObj->m_semReceiveQueue);
                    pthread_mutex_unlock(&pObj->m_mutexReceiveQueue);
                } else {
                    vscp_deleteVSCPevent(pEvent);
                }
            }
        }

    } // work loop

    // Close listner
    pcap_close(fp);

    return NULL;
}

//////////////////////////////////////////////////////////////////////
//                Workerthread - CWrkWriteThreadObj
//////////////////////////////////////////////////////////////////////

CWrkWriteThreadObj::CWrkWriteThreadObj()
{
    m_pObj = NULL;
}

CWrkWriteThreadObj::~CWrkWriteThreadObj()
{
    ;
}

//////////////////////////////////////////////////////////////////////
// writeWorkerThread
//

void *
writeWorkerThread(void *pData)
{
    pcap_t *fp;
    char errbuf[PCAP_ERRBUF_SIZE];
    uint8_t packet[512];

    CRawEthernet *pObj = (CRawEthernet *)pData;
    if (NULL == pObj) {
        syslog(LOG_ERR, "No data object supplied for write thread.");
        return NULL;
    }

    // Open the adapter
    if ((fp = pcap_open_live(
           (const char *)pObj->m_interface.c_str(), // name of the device
           65536, // portion of the packet to capture. It doesn't matter in this
                  // case
           1,     // promiscuous mode (nonzero means promiscuous)
           1000,  // read timeout
           errbuf // error buffer
           )) == NULL) {
        syslog(LOG_ERR,
               "RawEthDrv: Unable to open the adapter or %s is not supported "
               "by WinPcap. Err=%s",
               (const char *)pObj->m_interface.c_str(),
               errbuf);
        return NULL;
    }

    int rv;

    while (!pObj->m_bQuit) {

        if ((-1 == vscp_sem_wait(&pObj->m_semSendQueue, 300)) &&
            errno == ETIMEDOUT) {
            continue;
        }

        // Check if there is event(s) to send
        if (pObj->m_sendList.size()) {

            // Yes there are data to send
            pthread_mutex_lock(&pObj->m_mutexSendQueue);
            vscpEvent *pEvent = pObj->m_sendList.front();
            pObj->m_sendList.pop_front();
            pthread_mutex_unlock(&pObj->m_mutexSendQueue);

            if (NULL == pEvent) continue;

            // Set mac destination to broadcast ff:ff:ff:ff:ff:ff
            packet[0] = 0xff;
            packet[1] = 0xff;
            packet[2] = 0xff;
            packet[3] = 0xff;
            packet[4] = 0xff;
            packet[5] = 0xff;

            // set mac source to configured value - 6..11
            memcpy(packet + 6, pObj->m_localMac, 6);

            // Set the type - always 0x2574 (9598)
            packet[12] = 0x25;
            packet[13] = 0x7e;

            // rawEthernet frame version
            packet[14] = RAW_ETHERNET_FRAME_VERSION;

            // Head
            packet[15] = (pEvent->head & VSCP_HEADER_PRIORITY_MASK);
            packet[16] = 0x00;
            packet[17] = 0x00;
            packet[18] = 0x00; // LSB

            // VSCP sub source address
            packet[19] = (pObj->m_subaddr >> 8) & 0xff;
            packet[20] = pObj->m_subaddr & 0xff;

            // Timestamp
            uint32_t timestamp = pEvent->timestamp;
            packet[21]         = (timestamp & 0xff000000) >> 24;
            packet[22]         = (timestamp & 0x00ff0000) >> 16;
            packet[23]         = (timestamp & 0x0000ff00) >> 8;
            packet[24]         = (timestamp & 0x000000ff);

            // obid
            uint32_t obid = pEvent->obid;
            packet[25]    = (obid & 0xff000000) >> 24;
            packet[26]    = (obid & 0x00ff0000) >> 16;
            packet[27]    = (obid & 0x0000ff00) >> 8;
            packet[28]    = (obid & 0x000000ff);

            // VSCP Class
            uint16_t vscp_class = pEvent->vscp_class;
            packet[29]          = (vscp_class & 0xff00) >> 8;
            packet[30]          = (vscp_class & 0xff);

            // VSCP Type
            uint16_t vscp_type = pEvent->vscp_type;
            packet[31]         = (vscp_type & 0xff00) >> 8;
            packet[32]         = (vscp_type & 0xff);

            // Size
            packet[33] = pEvent->sizeData >> 8;
            packet[34] = pEvent->sizeData & 0xff;

            // VSCP Data
            memcpy(packet + 35, pEvent->pdata, pEvent->sizeData);

            // syslog( LOG_INFO,
            //			"RawEthDrv2: Class: %d Type: %d DataSize: %d",
            // pEvent->vscp_class, pEvent->vscp_type, pEvent->sizeData );

            // Send the packet
            if (0 != pcap_sendpacket(fp, packet, 35 + pEvent->sizeData)) {
                syslog(LOG_ERR,
                       "RawEthDrv: Error when sending the packet: %s\n",
                       pcap_geterr(fp));
            }

            // Remove the event
            pthread_mutex_lock(&pObj->m_mutexSendQueue);
            vscp_deleteVSCPevent(pEvent);
            pthread_mutex_unlock(&pObj->m_mutexSendQueue);

        } // Event received

    } // work loop

    // Close the Ethernet interface
    pcap_close(fp);

    // Close the channel
    pObj->m_srv.doCmdClose();

    return NULL;
}
