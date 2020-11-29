// ControlObject.cpp: m_path_db_vscp_logimplementation of the CControlObject
// class.
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
//

#define _POSIX

#include <controlobject.h>

#include <arpa/inet.h>
#include <errno.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#ifdef WITH_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>

#include <expat.h>

#include <fastpbkdf2.h>
#include <vscp_aes.h>

#include <actioncodes.h>
#include <automation.h>
#include <canal_macro.h>
#include <configfile.h>
#include <crc.h>
#include <devicelist.h>
#include <devicethread.h>
#include <randpassword.h>
#include <version.h>
#include <vscp.h>
#include <vscp_debug.h>
#include <vscpd_caps.h>
#include <vscphelper.h>
#include <vscpmd5.h>

#define UNUSED(x) (void)(x)
void
foo(const int i)
{
    UNUSED(i);
}

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define XML_BUFF_SIZE 0xffff

// Prototypes
void
createFolderStuct(std::string& rootFolder); // from vscpd.cpp


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CControlObject::CControlObject()
{
    m_bQuit = false;

    // Open syslog
    openlog("vscpd", LOG_CONS, LOG_DAEMON);

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Starting the vscpd daemon");
    }

    if (0 != pthread_mutex_init(&m_mutex_DeviceList, NULL)) {
        syslog(LOG_ERR, "Unable to init m_mutex_DeviceList");
        return;
    }

    m_rootFolder = "/var/lib/vscp/vscpd/";

    m_vscptoken = "Carpe diem quam minimum credula postero";

    // Nill the GUID
    m_guid.clear();

    // Initialize the CRC
    crcInit();
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CControlObject::~CControlObject()
{
    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Cleaning up");
    }

    if (0 != pthread_mutex_destroy(&m_mutex_DeviceList)) {
        syslog(LOG_ERR, "Unable to destroy m_mutex_DeviceList");
        return;
    }

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Terminating the vscpd daemon");
    }

    // Close syslog
    closelog();
}

/////////////////////////////////////////////////////////////////////////////
// init
//

bool
CControlObject::init(std::string& strcfgfile, std::string& rootFolder)
{
    std::string str;

    // Save root folder for later use.
    m_rootFolder = rootFolder;

    // Root folder must exist
    if (!vscp_fileExists(m_rootFolder.c_str())) {
        syslog(LOG_ERR,
               "The specified rootfolder does not exist (%s).",
               (const char*)m_rootFolder.c_str());
        return false;
    }

    // Change locale to get the correct decimal point "."
    setlocale(LC_NUMERIC, "C");

    // A configuration file must be available
    if (!vscp_fileExists(strcfgfile.c_str())) {
        printf("No configuration file. Can't initialize!.");
        syslog(LOG_ERR,
               "No configuration file. Can't initialize!. Path=%s",
               strcfgfile.c_str());
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    //                         Read XML configuration
    ////////////////////////////////////////////////////////////////////////////

    // Read XML configuration
    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Reading configuration file");
    }

    // Read XML configuration
    try {
        if (!readConfiguration(strcfgfile)) {
            syslog(LOG_ERR,
                   "Unable to open/parse configuration file. Can't initialize! "
                   "Path =%s",
                   strcfgfile.c_str());
            return FALSE;
        }
    }
    catch (...) {
        syslog(LOG_ERR, "Exception when reading configuration file");
        return FALSE;
    }

#ifndef WIN32
    if (m_runAsUser.length()) {
        struct passwd* pw;
        if (NULL == (pw = getpwnam(m_runAsUser.c_str()))) {
            syslog(LOG_ERR, "Unknown user.");
        }
        else if (setgid(pw->pw_gid) != 0) {
            syslog(LOG_ERR, "setgid() failed. [%s]", strerror(errno));
        }
        else if (setuid(pw->pw_uid) != 0) {
            syslog(LOG_ERR, "setuid() failed. [%s]", strerror(errno));
        }
    }
#endif

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Using configuration file: %s", strcfgfile.c_str());
    }

    //==========================================================================
    //                           Add driver user
    //==========================================================================

    // Get GUID
    if (m_guid.isNULL()) {
        if (!getMacAddress(m_guid)) {
            // We failed to create GUID from MAC address use
            // 'localhost' IP instead as the base.
            getIPAddress(m_guid);
        }
    }

    str = "VSCP Server started - ";
    str += "Version: ";
    str += VSCPD_DISPLAY_VERSION;
    str += " - ";
    str += VSCPD_COPYRIGHT;
    syslog(LOG_INFO, "%s", str.c_str());

    // Load drivers
    try {
        startDeviceWorkerThreads();
    }
    catch (...) {
        syslog(LOG_ERR, "Exception when loading drivers");
        return FALSE;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// run - Program main loop
//
// Most work is done in the threads at the moment
//

bool
CControlObject::run(void)
{
    std::deque<CClientItem*>::iterator nodeClient;

#ifdef WITH_SYSTEMD
    sd_notify(0, "READY=1");
#endif

    //-------------------------------------------------------------------------
    //                            MAIN - LOOP
    //-------------------------------------------------------------------------

    struct timespec now, old_now;
    clock_gettime(CLOCK_REALTIME, &old_now);
    old_now.tv_sec -= 60;  // Do firts send right away

    while (true) {

        clock_gettime(CLOCK_REALTIME, &now);

        // We send heartbeat every minute
        if ((now.tv_sec-old_now.tv_sec) > 60) {

            // Save time
            clock_gettime(CLOCK_REALTIME, &old_now);

            // if (!automation(pClientItem)) {
            //     syslog(LOG_ERR, "Failed to send automation events!");
            // }
        }

    } // while


    // Clean up is called in main file

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Mainloop ending");
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// automation

// bool
// CControlObject::automation(CClientItem* pClientItem)
// {
//     vscpEventEx ex;

//     // Send VSCP_CLASS1_INFORMATION,
//     // Type=9/VSCP_TYPE_INFORMATION_NODE_HEARTBEAT
//     ex.obid      = 0; // IMPORTANT Must be set by caller before event is sent
//     ex.head      = 0;
//     ex.timestamp = vscp_makeTimeStamp();
//     vscp_setEventExToNow(&ex); // Set time to current time
//     ex.vscp_class = VSCP_CLASS1_INFORMATION;
//     ex.vscp_type  = VSCP_TYPE_INFORMATION_NODE_HEARTBEAT;
//     ex.sizeData   = 3;

//     // GUID
//     memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

//     ex.data[0] = 0; // index
//     ex.data[1] = 0; // zone
//     ex.data[2] = 0; // subzone

//     // if (!sendEvent(pClientItem, &ex)) {
//     //     syslog(LOG_ERR, "Failed to send Class1 heartbeat");
//     // }

//     // Send VSCP_CLASS2_INFORMATION,
//     // Type=2/VSCP2_TYPE_INFORMATION_HEART_BEAT
//     ex.obid      = 0; // IMPORTANT Must be set by caller before event is sent
//     ex.head      = 0;
//     ex.timestamp = vscp_makeTimeStamp();
//     vscp_setEventExToNow(&ex); // Set time to current time
//     ex.vscp_class = VSCP_CLASS2_INFORMATION;
//     ex.vscp_type  = VSCP2_TYPE_INFORMATION_HEART_BEAT;
//     ex.sizeData   = 64;

//     // GUID
//     memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

//     memset(ex.data, 0, sizeof(ex.data));
//     // memcpy(ex.data,
//     //        m_strServerName.c_str(),
//     //        std::min((int)strlen(m_strServerName.c_str()), 64));

//     // if (!sendEvent(pClientItem, &ex)) {
//     //     syslog(LOG_ERR, "Failed to send Class2 heartbeat");
//     // }

//     // Send VSCP_CLASS1_PROTOCOL,
//     // Type=1/VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT
//     ex.obid      = 0; // IMPORTANT Must be set by caller before event is sent
//     ex.head      = 0;
//     ex.timestamp = vscp_makeTimeStamp();
//     vscp_setEventExToNow(&ex); // Set time to current time
//     ex.vscp_class = VSCP_CLASS1_PROTOCOL;
//     ex.vscp_type  = VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT;
//     ex.sizeData   = 5;

//     // GUID
//     memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

//     time_t tnow;
//     time(&tnow);
//     uint32_t time32 = (uint32_t)tnow;

//     ex.data[0] = 0; // 8 - bit crc for VSCP daemon GUID
//     ex.data[1] = (uint8_t)((time32 >> 24) & 0xff); // Time since epoch MSB
//     ex.data[2] = (uint8_t)((time32 >> 16) & 0xff);
//     ex.data[3] = (uint8_t)((time32 >> 8) & 0xff);
//     ex.data[4] = (uint8_t)((time32)&0xff); // Time since epoch LSB

//     // if (!sendEvent(pClientItem, &ex)) {
//     //     syslog(LOG_ERR, "Failed to send segment controller heartbeat");
//     // }

//     // Send VSCP_CLASS2_PROTOCOL,
//     // Type=20/VSCP2_TYPE_PROTOCOL_HIGH_END_SERVER_CAPS
//     ex.obid      = 0; // IMPORTANT Must be set by caller before event is sent
//     ex.head      = 0;
//     ex.timestamp = vscp_makeTimeStamp();
//     vscp_setEventExToNow(&ex); // Set time to current time
//     ex.vscp_class = VSCP_CLASS2_PROTOCOL;
//     ex.vscp_type  = VSCP2_TYPE_PROTOCOL_HIGH_END_SERVER_CAPS;

//     // Fill in data
//     memset(ex.data, 0, sizeof(ex.data));

//     // GUID
//     memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

//     // Server ip address
//     cguid guid;
//     if (getIPAddress(guid)) {
//         ex.data[VSCP_CAPABILITY_OFFSET_IP_ADDR]     = guid.getAt(8);
//         ex.data[VSCP_CAPABILITY_OFFSET_IP_ADDR + 1] = guid.getAt(9);
//         ex.data[VSCP_CAPABILITY_OFFSET_IP_ADDR + 2] = guid.getAt(10);
//         ex.data[VSCP_CAPABILITY_OFFSET_IP_ADDR + 3] = guid.getAt(11);
//     }

//     // Server name
//     // memcpy(ex.data + VSCP_CAPABILITY_OFFSET_SRV_NAME,
//     //        (const char*)m_strServerName.c_str(),
//     //        std::min((int)strlen((const char*)m_strServerName.c_str()), 64));

//     // Capabilities array
//     //getVscpCapabilities(ex.data);

//     // non-standard ports
//     // TODO

//     ex.sizeData = 104;

//     // if (!sendEvent(pClientItem, &ex)) {
//     //     syslog(LOG_ERR, "Failed to send high end server capabilities.");
//     // }

//     return true;
// }

/////////////////////////////////////////////////////////////////////////////
// cleanup

bool
CControlObject::cleanup(void)
{
    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "ControlObject: cleanup - Giving worker threads time to stop "
               "operations...");
    }

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "ControlObject: cleanup - Stopping device worker thread...");
    }

    try {
        stopDeviceWorkerThreads();
    }
    catch (...) {
        syslog(LOG_ERR,
               "REST: Exception occurred when stoping device worker threads");
    }

    if (__VSCP_DEBUG_EXTRA) {
        syslog(
          LOG_DEBUG,
          "ControlObject: cleanup - Stopping VSCP Server worker thread...");
    }

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "ControlObject: cleanup - Stopping client worker thread...");
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// startDeviceWorkerThreads
//

bool
CControlObject::startDeviceWorkerThreads(void)
{
    CDeviceItem* pDeviceItem;
    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "[Controlobject][Driver] - Starting drivers...");
    }

    std::deque<CDeviceItem*>::iterator it;
    for (it = m_deviceList.m_devItemList.begin();
         it != m_deviceList.m_devItemList.end();
         ++it) {

        pDeviceItem = *it;
        if (NULL != pDeviceItem) {

            if (__VSCP_DEBUG_EXTRA) {
                syslog(LOG_DEBUG,
                       "Controlobject: [Driver] - Preparing: %s ",
                       pDeviceItem->m_strName.c_str());
            }

            // Just start if enabled
            if (!pDeviceItem->m_bEnable)
                continue;

            if (__VSCP_DEBUG_EXTRA) {
                syslog(LOG_DEBUG,
                       "Controlobject: [Driver] - Starting: %s ",
                       pDeviceItem->m_strName.c_str());
            }

            // Start  the driver logic
            pDeviceItem->startDriver(this);

        } // Valid device item
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// stopDeviceWorkerThreads
//

bool
CControlObject::stopDeviceWorkerThreads(void)
{
    CDeviceItem* pDeviceItem;

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "[Controlobject][Driver] - Stopping drivers...");
    }
    std::deque<CDeviceItem*>::iterator iter;
    for (iter = m_deviceList.m_devItemList.begin();
         iter != m_deviceList.m_devItemList.end();
         ++iter) {

        pDeviceItem = *iter;
        if (NULL != pDeviceItem) {
            if (__VSCP_DEBUG_EXTRA) {
                syslog(LOG_DEBUG,
                       "Controlobject: [Driver] - Stopping: %s ",
                       pDeviceItem->m_strName.c_str());
            }
            pDeviceItem->stopDriver();
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getMacAddress
//

bool
CControlObject::getMacAddress(cguid& guid)
{
#ifdef WIN32

    bool rv = false;
    NCB Ncb;
    UCHAR uRetCode;
    LANA_ENUM lenum;
    int i;

    // Clear the GUID
    guid.clear();

    memset(&Ncb, 0, sizeof(Ncb));
    Ncb.ncb_command = NCBENUM;
    Ncb.ncb_buffer  = (UCHAR*)&lenum;
    Ncb.ncb_length  = sizeof(lenum);
    uRetCode        = Netbios(&Ncb);
    // printf( "The NCBENUM return code is: 0x%x ", uRetCode );

    for (i = 0; i < lenum.length; i++) {
        memset(&Ncb, 0, sizeof(Ncb));
        Ncb.ncb_command  = NCBRESET;
        Ncb.ncb_lana_num = lenum.lana[i];

        uRetCode = Netbios(&Ncb);

        memset(&Ncb, 0, sizeof(Ncb));
        Ncb.ncb_command  = NCBASTAT;
        Ncb.ncb_lana_num = lenum.lana[i];

        strcpy((char*)Ncb.ncb_callname, "*               ");
        Ncb.ncb_buffer = (unsigned char*)&Adapter;
        Ncb.ncb_length = sizeof(Adapter);

        uRetCode = Netbios(&Ncb);

        if (uRetCode == 0) {
            guid.setAt(0, 0xff);
            guid.setAt(1, 0xff);
            guid.setAt(2, 0xff);
            guid.setAt(3, 0xff);
            guid.setAt(4, 0xff);
            guid.setAt(5, 0xff);
            guid.setAt(6, 0xff);
            guid.setAt(7, 0xfe);
            guid.setAt(8, Adapter.adapt.adapter_address[0]);
            guid.setAt(9, Adapter.adapt.adapter_address[1]);
            guid.setAt(10, Adapter.adapt.adapter_address[2]);
            guid.setAt(11, Adapter.adapt.adapter_address[3]);
            guid.setAt(12, Adapter.adapt.adapter_address[4]);
            guid.setAt(13, Adapter.adapt.adapter_address[5]);
            guid.setAt(14, 0);
            guid.setAt(15, 0);
#ifdef DEBUG__
            char buf[256];
            sprintf(buf,
                    "The Ethernet MAC Address: %02x:%02x:%02x:%02x:%02x:%02x",
                    guid.getAt(2),
                    guid.getAt(3),
                    guid.getAt(4),
                    guid.getAt(5),
                    guid.getAt(6),
                    guid.getAt(7));

            std::string str = std::string(buf);
#endif

            rv = true;
        }
    }

    return rv;

#else
    // cat /sys/class/net/eth0/address
    bool rv = true;
    struct ifreq s;
    int fd;

    // Clear the GUID
    guid.clear();

    fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (-1 == fd)
        return false;

    memset(&s, 0, sizeof(s));
    strcpy(s.ifr_name, "eth0");

    if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {

        // ptr = (unsigned char *)&s.ifr_ifru.ifru_hwaddr.sa_data[0];
        if (__VSCP_DEBUG_EXTRA) {
            syslog(LOG_DEBUG,
                   "Ethernet MAC address: %02X:%02X:%02X:%02X:%02X:%02X",
                   (uint8_t)s.ifr_addr.sa_data[0],
                   (uint8_t)s.ifr_addr.sa_data[1],
                   (uint8_t)s.ifr_addr.sa_data[2],
                   (uint8_t)s.ifr_addr.sa_data[3],
                   (uint8_t)s.ifr_addr.sa_data[4],
                   (uint8_t)s.ifr_addr.sa_data[5]);
        }

        guid.setAt(0, 0xff);
        guid.setAt(1, 0xff);
        guid.setAt(2, 0xff);
        guid.setAt(3, 0xff);
        guid.setAt(4, 0xff);
        guid.setAt(5, 0xff);
        guid.setAt(6, 0xff);
        guid.setAt(7, 0xfe);
        guid.setAt(8, s.ifr_addr.sa_data[0]);
        guid.setAt(9, s.ifr_addr.sa_data[1]);
        guid.setAt(10, s.ifr_addr.sa_data[2]);
        guid.setAt(11, s.ifr_addr.sa_data[3]);
        guid.setAt(12, s.ifr_addr.sa_data[4]);
        guid.setAt(13, s.ifr_addr.sa_data[5]);
        guid.setAt(14, 0);
        guid.setAt(15, 0);
    }
    else {
        syslog(LOG_ERR, "Failed to get hardware address (must be root?).");
        rv = false;
    }

    return rv;

#endif
}

///////////////////////////////////////////////////////////////////////////////
//  getIPAddress
//

bool
CControlObject::getIPAddress(cguid& guid)
{
    // Clear the GUID
    guid.clear();

    guid.setAt(0, 0xff);
    guid.setAt(1, 0xff);
    guid.setAt(2, 0xff);
    guid.setAt(3, 0xff);
    guid.setAt(4, 0xff);
    guid.setAt(5, 0xff);
    guid.setAt(6, 0xff);
    guid.setAt(7, 0xfd);

    char szName[128];
    gethostname(szName, sizeof(szName));
#if defined(_WIN32)
    LPHOSTENT lpLocalHostEntry;
#else
    struct hostent* lpLocalHostEntry;
#endif
    lpLocalHostEntry = gethostbyname(szName);
    if (NULL == lpLocalHostEntry) {
        return false;
    }

    // Get all local addresses
    int idx = -1;
    void* pAddr;
    unsigned long localaddr[16]; // max 16 local addresses
    do {
        idx++;
        localaddr[idx] = 0;
        pAddr          = lpLocalHostEntry->h_addr_list[idx];
        if (NULL != pAddr)
            localaddr[idx] = *((unsigned long*)pAddr);
    } while ((NULL != pAddr) && (idx < 16));

    guid.setAt(8, (localaddr[0] >> 24) & 0xff);
    guid.setAt(9, (localaddr[0] >> 16) & 0xff);
    guid.setAt(10, (localaddr[0] >> 8) & 0xff);
    guid.setAt(11, localaddr[0] & 0xff);

    guid.setAt(12, 0);
    guid.setAt(13, 0);
    guid.setAt(14, 0);
    guid.setAt(15, 0);

    return true;
}


// ----------------------------------------------------------------------------
// FULL XML configuration callbacks
// ----------------------------------------------------------------------------

static int depth_full_config_parser = 0;
static int bVscpConfigFound         = 0;
static int bGeneralConfigFound      = 0;
static int bMQTTConfigFound         = 0;
static int bLevel1DriverConfigFound = 0;
static int bLevel2DriverConfigFound = 0;

static void
startFullConfigParser(void* data, const char* name, const char** attr)
{
    CControlObject* pObj = (CControlObject*)data;
    if (NULL == data)
        return;

    // fprintf(stderr, "%s\n", name);

    if ((0 == depth_full_config_parser) &&
        (0 == vscp_strcasecmp(name, "vscpconfig"))) {
        bVscpConfigFound = TRUE;
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "general"))) {
        bGeneralConfigFound = TRUE;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "runasuser")) {
                vscp_trim(attribute);
                pObj->m_runAsUser = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                pObj->m_guid.getFromString(attribute);
            }

        }
    }
    else if (bVscpConfigFound && bGeneralConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "debug"))) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "byte1")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[0] = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "byte2")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[1] = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "byte3")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[2] = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "byte4")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[3] = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "byte5")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[4] = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "byte6")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[5] = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "byte7")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[6] = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "byte8")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[7] = vscp_readStringValue(attribute);
                }
            }
        }
    }
    else if (bVscpConfigFound && bGeneralConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "security"))) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "vscptoken")) {
                pObj->m_vscptoken = attribute;
            }

        }
    }

    
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "mqtt"))) {
        
        bVscpConfigFound = TRUE;

        std::string host = "127.0.0.1";
        short port = 1883;
        std::string clientid;
        std::string user = "vscp";
        std::string password = "secret";
        int qos = 0;
        bool bCleanSession = false;
        bool bRetain = false;
        int keepalive = -1;
        std::string topic_subscribe = "vscp/#";
        std::string topic_publish = "vscp/%guid%/%class%/%type%/";
        std::string cafile = "";
    	std::string capath = "";
    	std::string certfile = "";
    	std::string keyfile = "";
    	std::string xpassword = "";

        for (int i=0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "host")) {
                vscp_startsWith(attribute, "tcp://", &attribute);
                vscp_startsWith(attribute, "stcp://", &attribute);
                vscp_trim(attribute);
                host = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "port")) {
                port = vscp_readStringValue(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "user")) {
                user = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "password")) {
                password = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "qos")) {
                qos = vscp_readStringValue(attribute);
                if (qos>3) {
                    syslog(LOG_ERR,"Config: MQTT qos > 3. Set to 0.");
                    qos = 0;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "bcleansession")) {  // bCleanSession
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bCleanSession = true;
                }
                else {
                    bCleanSession = false;
                }
            }                        
            else if (0 == vscp_strcasecmp(attr[i], "bretain")) {        // bRetain
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bRetain = true;
                }
                else {
                    bRetain = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "keepalive")) {   
                keepalive = vscp_readStringValue(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "topic-subscribe")) {
                topic_subscribe = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "topic-publish")) {
                topic_publish = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "cafile")) {
                cafile = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "capath")) {
                capath = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "certfile")) {
                certfile = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "keyfile")) {
                keyfile = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "xpassword")) {
                xpassword = attribute;
            }

        }

        if ( VSCP_ERROR_SUCCESS != pObj->m_mqtt.init( host,
                                                    port,
                                                    topic_subscribe,
                                                    topic_publish,
                                                    clientid,
                                                    user,
                                                    password,
                                                    bCleanSession,
                                                    qos ) ) {
            syslog(LOG_ERR,"Failed to initialize MQTT broker.");
            syslog(LOG_ERR,"host: %s Port %d",host.c_str(), (int)port );
            syslog(LOG_ERR,"user %s ", user.c_str() );
            syslog(LOG_ERR,"topic subscribe %s ", topic_subscribe.c_str() );
            syslog(LOG_ERR,"topic publish %s ", topic_publish.c_str() );
            syslog(LOG_ERR,"clinet id %s ", clientid.c_str() );
            syslog(LOG_ERR,"bCleanSession: %s ", bCleanSession ? "true" : "false" );
            syslog(LOG_ERR,"qos: %d ", qos );
        }

        if ( bRetain ) {
            if ( VSCP_ERROR_SUCCESS != pObj->m_mqtt.setRetain(bRetain) ) {
                syslog(LOG_ERR,"Failed to set MQTT broker retain");
            }
        }

        if ( -1 != keepalive ) {
            if ( VSCP_ERROR_SUCCESS != pObj->m_mqtt.setKeepAlive(keepalive) ) {
                syslog(LOG_ERR,"Failed to set MQTT broker keepalive (%d)", keepalive);
            }
        }

        if (cafile.length()) {
           if ( VSCP_ERROR_SUCCESS != pObj->m_mqtt.set_tls(cafile,
                                                            capath,
                                                            certfile,
                                                            keyfile,
                                                            xpassword ) ) {
               syslog(LOG_ERR,"Failed to set MQTT broker tls");
           }
        }

        // if (bEnable) {
        //     pObj->addMqttClient(interface,
		// 	                        user,
		// 	                        password,
		// 	                        filter,
		// 	                        guid,
		// 	                        qos,
		// 	                        bCleanSession,
		// 	                        bRetain,
		// 	                        keepalive,
		// 	                        topic_subscribe,
		// 	                        topic_publish,
		// 	                        cafile,
		// 	                        capath,
		// 	                        certfile,
		// 	                        keyfile,
		// 	                        xpassword );
        // }
        
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             ((0 == vscp_strcasecmp(name, "level1driver")) ||
              (0 == vscp_strcasecmp(name, "canal1driver")))) {
        bLevel1DriverConfigFound = TRUE;
    }
    else if (bVscpConfigFound && bLevel1DriverConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "driver"))) {

        std::string strName;
        std::string strConfig;
        std::string strPath;
        unsigned long flags  = 0;
        uint32_t translation = 0;
        cguid guid;
        bool bEnabled = false;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bEnabled = true;
                }
                else {
                    bEnabled = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "name")) {
                strName = attribute;
                // Replace possible spaces in name with underscore
                std::string::size_type found;
                while (std::string::npos !=
                       (found = strName.find_first_of(" "))) {
                    strName[found] = '_';
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "config")) {
                strConfig = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i],
                                          "parameter")) { // deprecated
                strConfig = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "path")) {
                strPath = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "flags")) {
                flags = vscp_readStringValue(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                guid.getFromString(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "translation")) {
                translation = vscp_readStringValue(attribute);
            }
        } // for

        if (bEnabled) {
            // Add the level I device
            if (!pObj->m_deviceList.addItem(strName,
                                            strConfig,
                                            strPath,
                                            flags,
                                            guid,
                                            VSCP_DRIVER_LEVEL1,
                                            bEnabled,
                                            translation)) {
                syslog(LOG_ERR,
                       "Level I driver not added name=%s. "
                       "Path does not exist. - [%s]",
                       strName.c_str(),
                       strPath.c_str());
            }
            else {
                if (__VSCP_DEBUG_DRIVER1) {
                    syslog(LOG_DEBUG,
                           "Level I driver added. name = %s - [%s]",
                           strName.c_str(),
                           strPath.c_str());
                }
            }
        }
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             ((0 == vscp_strcasecmp(name, "level2driver")))) {
        bLevel2DriverConfigFound = TRUE;
    }
    else if (bVscpConfigFound && bLevel2DriverConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "driver"))) {

        std::string strName;
        std::string strConfig;
        std::string strPath;
        cguid guid;
        bool bEnabled = false;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bEnabled = true;
                }
                else {
                    bEnabled = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "name")) {
                strName = attribute;
                // Replace spaces in name with underscore
                std::string::size_type found;
                while (std::string::npos !=
                       (found = strName.find_first_of(" "))) {
                    strName[found] = '_';
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "path-config")) {
                strConfig = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i],
                                          "parameter")) { // deprecated
                strConfig = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "path-driver")) {
                strPath = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                guid.getFromString(attribute);
            }
        } // for

        // Add the level II device
        if (bEnabled) {
            if (!pObj->m_deviceList.addItem(strName,
                                            strConfig,
                                            strPath,
                                            0,
                                            guid,
                                            VSCP_DRIVER_LEVEL2,
                                            bEnabled)) {
                if (__VSCP_DEBUG_DRIVER2) {
                    syslog(LOG_ERR,
                           "Level II driver was not added. name = %s"
                           "Path does not exist. - [%s]",
                           strName.c_str(),
                           strPath.c_str());
                }
            }
            else {
                if (__VSCP_DEBUG_DRIVER2) {
                    syslog(LOG_DEBUG,
                           "Level II driver added. name = %s- [%s]",
                           strName.c_str(),
                           strPath.c_str());
                }
            }
        }
    }

    depth_full_config_parser++;
}

static void
handleFullConfigData(void* data, const char* content, int length)
{
}

static void
endFullConfigParser(void* data, const char* name)
{
    depth_full_config_parser--;

    if (1 == depth_full_config_parser &&
        (0 == vscp_strcasecmp(name, "vscpconfig"))) {
        bVscpConfigFound = FALSE;
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "mqtt"))) {
        bMQTTConfigFound = FALSE;
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
        ((0 == vscp_strcasecmp(name, "level1driver")) ||
         (0 == vscp_strcasecmp(name, "canal1driver")))) {
        bLevel1DriverConfigFound = FALSE;
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "level2driver"))) {
        bLevel2DriverConfigFound = FALSE;
    }
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// readConfiguration
//
// Read the configuration XML file
//

bool
CControlObject::readConfiguration(const std::string& strcfgfile)
{
    FILE* fp;

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "Reading full XML configuration from [%s]",
               (const char*)strcfgfile.c_str());
    }

    fp = fopen(strcfgfile.c_str(), "r");
    if (NULL == fp) {
        syslog(LOG_ERR,
               "Failed to open configuration file [%s]",
               strcfgfile.c_str());
        return false;
    }

    XML_Parser xmlParser = XML_ParserCreate("UTF-8");
    XML_SetUserData(xmlParser, this);
    XML_SetElementHandler(xmlParser,
                          startFullConfigParser,
                          endFullConfigParser);
    XML_SetCharacterDataHandler(xmlParser, handleFullConfigData);

    void* buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);
    if (NULL == buf) {
        XML_ParserFree(xmlParser);
        fclose(fp);
        syslog(LOG_ERR,
               "Failed to allocate buffer for configuration file [%s]",
               strcfgfile.c_str());
        return false;
    }

    size_t file_size = 0;
    file_size        = fread(buf, sizeof(char), XML_BUFF_SIZE, fp);

    if (!XML_ParseBuffer(xmlParser, file_size, file_size == 0)) {
        syslog(LOG_ERR, "Failed parse XML configuration file.");
        fclose(fp);
        XML_ParserFree(xmlParser);
        return false;
    }

    fclose(fp);
    XML_ParserFree(xmlParser);

    return true;
} // XML config




