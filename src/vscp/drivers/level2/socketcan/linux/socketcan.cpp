// socketcan.cpp: implementation of the Csocketcan class.
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

#include <string>
#include <list>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <syslog.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

// Different on Kernel 2.6 and cansocket examples
// currently using locally from can-utils
// TODO remove include from makefile when they are in sync
#include <linux/can.h>
#include <linux/can/raw.h>

#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <net/if.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>

#include <expat.h>

#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>
#include <vscphelper.h>
#include <vscpremotetcpif.h>

#include "vscpl2drv_socketcan.h"
#include "socketcan.h"

// Buffer for XML parser
#define XML_BUFF_SIZE 10000

// Forward declaration 
void *workerThread(void *pData);

//////////////////////////////////////////////////////////////////////
// Csocketcan
//

Csocketcan::Csocketcan()
{
    m_bQuit         = false;
    m_interface     = "vcan0";
    vscp_clearVSCPFilter(&m_vscpfilter); // Accept all events

    // Open syslog channel
    openlog(VSCP_DLL_SONAME, LOG_PID | LOG_CONS, LOG_DAEMON);

    sem_init(&m_semSendQueue, 0, 0);
    sem_init(&m_semReceiveQueue, 0, 0);

    pthread_mutex_init(&m_mutexSendQueue, NULL);
    pthread_mutex_init(&m_mutexReceiveQueue, NULL);
}

//////////////////////////////////////////////////////////////////////
// ~Csocketcan
//

Csocketcan::~Csocketcan()
{
    close();

    sem_destroy(&m_semSendQueue);
    sem_destroy(&m_semReceiveQueue);

    pthread_mutex_destroy(&m_mutexSendQueue);
    pthread_mutex_destroy(&m_mutexReceiveQueue);

    // Close syslog channel
    closelog();
}

// ----------------------------------------------------------------------------

/*
    XML Setup
    =========

    <setup interface=""
            filter = ""
            mask = "" />
*/

// ----------------------------------------------------------------------------

int depth_setup_parser = 0;

void
startSetupParser( void *data, const char *name, const char **attr ) 
{
    Csocketcan *pObj = (Csocketcan *)data;
    if (NULL == pObj) return;

    if ((0 == strcmp(name, "setup")) && (0 == depth_setup_parser)) {


        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == strcasecmp(attr[i], "interface")) {
                if (!attribute.empty()) {
                    pObj->m_interface = attribute;
                }
            } else if (0 == strcasecmp(attr[i], "filter")) {
                if (!attribute.empty()) {
                    if (!vscp_readFilterFromString(&pObj->m_vscpfilter,
                                                   attribute)) {
                        syslog(LOG_ERR, "Unable to read event receive filter.");
                    }
                }
            } else if (0 == strcasecmp(attr[i], "mask")) {
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
endSetupParser( void *data, const char *name ) 
{
    depth_setup_parser--;
}

// ----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////
// open
//
//

bool
Csocketcan::open(const char *pUsername,
                 const char *pPassword,
                 const char *pHost,
                 short port,
                 const char *pPrefix,
                 const char *pConfig)
{
    bool rv           = true;
    std::string wxstr = std::string(pConfig);

    m_username = std::string(pUsername);
    m_password = std::string(pPassword);
    m_host     = std::string(pHost);
    m_port     = port;
    m_prefix   = std::string(pPrefix);

    // Parse the configuration string. It should
    // have the following form path
    
    std::deque<std::string> tokens;
    vscp_split( tokens, std::string(pConfig), ";" );


    // Check for socketcan interface in configuration string
    if (!tokens.empty()) {
        // Interface
        m_interface = tokens.front();
        tokens.pop_front();
    }

    // First log on to the host and get configuration
    // variables

    if (VSCP_ERROR_SUCCESS != m_srv.doCmdOpen(m_host,
                                              m_port,
                                              m_username,
                                              m_password)) {
        syslog(LOG_ERR,
               "%s",
               (const char *)"Unable to connect to "
                             "VSCP TCP/IP interface. Terminating!");
        return false;
    }

    // Find the channel id
    uint32_t ChannelID;
    m_srv.doCmdGetChannelID(&ChannelID);

    // m_srv.doCmdGetGUID( m_ifguid );

    // The server should hold configuration data for each sensor
    // we want to monitor.
    //
    // We look for
    //
    //	 _interface - The socketcan interface to use. Typically this
    //	 is “can0, can0, can1...
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
    // <setup interface="vcan0"
    //          filter=""
    //          mask="" />
    //

    std::string str;
    std::string strName = m_prefix + std::string("_interface");
    m_srv.getRemoteVariableValue(strName, m_interface);

    strName = m_prefix + std::string("_filter");
    if (m_srv.getRemoteVariableValue(strName, str)) {
        vscp_readFilterFromString(&m_vscpfilter, str);
    }

    strName = m_prefix + std::string("_mask");
    if (m_srv.getRemoteVariableValue(strName, str)) {
        vscp_readMaskFromString(&m_vscpfilter, str);
    }

    m_srv.doClrInputQueue();

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

    // start the workerthread
    if ( pthread_create( &m_threadWork, 
                            NULL, 
                            workerThread, 
                            this ) ) {
 
        syslog( LOG_CRIT, "Unable to start worker thread." );
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
Csocketcan::close(void)
{
    // Do nothing if already terminated
    if (m_bQuit) return;

    m_bQuit = true; // terminate the thread
    sleep(1);     // Give the thread some time to terminate
}

//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool
Csocketcan::addEvent2SendQueue(const vscpEvent *pEvent)
{
    pthread_mutex_lock( &m_mutexSendQueue);
    m_sendList.push_back((vscpEvent *)pEvent);
    sem_post(&m_semSendQueue);
    pthread_mutex_unlock( &m_mutexSendQueue);
    return true;
}

//////////////////////////////////////////////////////////////////////
//                Workerthread - CSocketCanWorkerTread
//////////////////////////////////////////////////////////////////////



void *workerThread(void *pData)
{
    int sock;
    char devname[IFNAMSIZ + 1];
    fd_set rdfs;
    struct timeval tv;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct cmsghdr *cmsg;
    struct canfd_frame frame;
    char
      ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
    const int canfd_on = 1;

    Csocketcan *pObj = (Csocketcan *)pData;
    if (NULL == pObj) {
        syslog(LOG_CRIT, "No object data supplied for worker thread");
        return NULL;
    }

    strncpy(devname, pObj->m_interface.c_str(), sizeof(devname) - 1);
#if DEBUG
    syslog(LOG_ERR, "CWriteSocketCanTread: Interface: %s\n", ifname);
#endif

    while (!pObj->m_bQuit) {

        sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (sock < 0) {

            if (ENETDOWN == errno) {
                sleep(1);
                continue;
            }

            syslog(LOG_ERR,
                   "%s",
                   (const char *)"CReadSocketCanTread: Error while "
                                 "opening socket. Terminating!");

            break;
        }

        strcpy(ifr.ifr_name, devname);
        ioctl(sock, SIOCGIFINDEX, &ifr);

        addr.can_family  = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

#ifdef DEBUG
        printf("using interface name '%s'.\n", ifr.ifr_name);
#endif

        // try to switch the socket into CAN FD mode
        setsockopt(
          sock, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canfd_on, sizeof(canfd_on));

        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            syslog(LOG_ERR,
                   "%s",
                   (const char *)"CReadSocketCanTread: Error in socket bind. "
                                 "Terminating!");
            close(sock);
            sleep(2);
            continue;
        }

        bool bInnerLoop = true;
        while (pObj->m_bQuit && bInnerLoop) {

            FD_ZERO(&rdfs);
            FD_SET(sock, &rdfs);

            tv.tv_sec  = 0;
            tv.tv_usec = 5000; // 5ms timeout

            int ret;
            if ((ret = select(sock + 1, &rdfs, NULL, NULL, &tv)) < 0) {
                // Error
                if (ENETDOWN == errno) {
                    // We try to get contact with the net
                    // again if it goes down
                    bInnerLoop = false;
                } else {
                    pObj->m_bQuit = true;
                }
                continue;
            }

            if (ret) {

                // There is data to read

                ret = read(sock, &frame, sizeof(struct can_frame));
                if (ret < 0) {
                    if (ENETDOWN == errno) {
                        // We try to get contact with the net
                        // again if it goes down
                        bInnerLoop = false;
                        sleep(2);
                    } else {
                        pObj->m_bQuit = true;
                    }
                    continue;
                }

                // Must be Extended
                if (!(frame.can_id & CAN_EFF_FLAG)) continue;

                // Mask of control bits
                frame.can_id &= CAN_EFF_MASK;

                vscpEvent *pEvent = new vscpEvent();
                if (NULL != pEvent) {

                    pEvent->pdata = new uint8_t[frame.len];
                    if (NULL == pEvent->pdata) {
                        delete pEvent;
                        continue;
                    }

                    // GUID will be set to GUID of interface
                    // by driver interface with LSB set to nickname
                    memset(pEvent->GUID, 0, 16);
                    pEvent->GUID[VSCP_GUID_LSB] = frame.can_id & 0xff;

                    // Set VSCP class
                    pEvent->vscp_class =
                      vscp_getVSCPclassFromCANALid(frame.can_id);

                    // Set VSCP type
                    pEvent->vscp_type =
                      vscp_getVSCPtypeFromCANALid(frame.can_id);

                    // Copy data if any
                    pEvent->sizeData = frame.len;
                    if (frame.len) {
                        memcpy(pEvent->pdata, frame.data, frame.len);
                    }

                    if (vscp_doLevel2Filter(pEvent, &pObj->m_vscpfilter)) {
                        pthread_mutex_lock( &pObj->m_mutexReceiveQueue);
                        pObj->m_receiveList.push_back(pEvent);
                        sem_post(&pObj->m_semReceiveQueue);
                        pthread_mutex_unlock( &pObj->m_mutexReceiveQueue);
                    } else {
                        vscp_deleteVSCPevent(pEvent);
                    }
                }

            } else {

                // Check if there is event(s) to send
                if (pObj->m_sendList.size()) {

                    // Yes there are data to send
                    // So send it out on the CAN bus

                    pthread_mutex_lock( &pObj->m_mutexSendQueue);
                    vscpEvent *pEvent = pObj->m_sendList.front();
                    pObj->m_sendList.pop_front();
                    pthread_mutex_unlock( &pObj->m_mutexSendQueue);

                    if (NULL == pEvent) continue;

                    // Class must be a Level I class or a Level II
                    // mirror class
                    if (pEvent->vscp_class < 512) {
                        frame.can_id = vscp_getCANALidFromVSCPevent(pEvent);
                        frame.can_id |= CAN_EFF_FLAG; // Always extended
                        if (0 != pEvent->sizeData) {
                            frame.len =
                              (pEvent->sizeData > 8 ? 8 : pEvent->sizeData);
                            memcpy(frame.data, pEvent->pdata, frame.len);
                        }
                    } else if (pEvent->vscp_class < 1024) {
                        pEvent->vscp_class -= 512;
                        frame.can_id = vscp_getCANALidFromVSCPevent(pEvent);
                        frame.can_id |= CAN_EFF_FLAG; // Always extended
                        if (0 != pEvent->sizeData) {
                            frame.len = ((pEvent->sizeData - 16) > 8
                                           ? 8
                                           : pEvent->sizeData - 16);
                            memcpy(frame.data, pEvent->pdata + 16, frame.len);
                        }
                    }

                    // Remove the event
                    pthread_mutex_lock( &pObj->m_mutexSendQueue);
                    vscp_deleteVSCPevent(pEvent);
                    pthread_mutex_unlock( &pObj->m_mutexSendQueue);

                    // Write the data
                    int nbytes = write(sock, &frame, sizeof(struct can_frame));

                } // event to send

            } // No data to read

        } // Inner loop

        // Close the socket
        close(sock);

    } // Outer loop

    return NULL;
}


