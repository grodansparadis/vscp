// vscp_client_level2.h
//
// VSCP level II driver client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:  (C) 2007-2025
// Ake Hedman, the VSCP project, <info@vscp.org>
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

#ifdef WIN32
#include <pch.h>
#endif

#ifndef WIN32

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
#include <linux/can.h>
#include <linux/can/raw.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>

#include <expat.h>

#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>
#include <vscphelper.h>
#include "vscp-client-level2.h"

#include <string>
#include <list>


// Forward declaration
static void *workerThread(void *pObj);

///////////////////////////////////////////////////////////////////////////////
// C-tor
//

vscpClientLevel2::vscpClientLevel2() 
{
    m_bConnected = false;  // Not connected
    m_tid = 0;
    m_bRun = false;

    setResponseTimeout(3);  // Response timeout 3 ms
    pthread_mutex_init(&m_mutexif,NULL);
}

///////////////////////////////////////////////////////////////////////////////
// D-tor
//

vscpClientLevel2::~vscpClientLevel2() 
{
    disconnect();
    pthread_mutex_destroy(&m_mutexif);
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int vscpClientLevel2::init(const std::string &interface, 
                                unsigned long flags,
                                uint32_t timeout)
{
    m_interface = interface;
    m_flags = flags;
    setResponseTimeout(DEAULT_RESPONSE_TIMEOUT);  // Response timeout 3 ms
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string vscpClientLevel2::getConfigAsJson(void) 
{
    json j;
    std::string rv;
    
    j["interface"] = m_interface;
    j["flags"] = m_flags;
    j["responsetimeout"] = getResponseTimeout();

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool vscpClientLevel2::initFromJson(const std::string& config)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int vscpClientLevel2::connect(void) 
{
    int rv = CANAL_ERROR_SUCCESS;
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int vscpClientLevel2::disconnect(void)
{
    m_bRun = false;
    pthread_join(m_tid, NULL);
    
    m_bConnected = false;
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool vscpClientLevel2::isConnected(void)
{
    return m_bConnected;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientLevel2::send(vscpEvent &ev)
{

    return VSCP_ERROR_WRITE_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientLevel2::send(vscpEventEx &ex)
{
  rv = VSCP_ERROR_WRITE_ERROR;

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientLevel2::receive(vscpEvent &ev)
{
    int rv = VSCP_ERROR_ERROR;

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientLevel2::receive(vscpEventEx &ex)
{
  int rv = VSCP_ERROR_ERROR;

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// setfilter
//

int vscpClientLevel2::setfilter(vscpEventFilter &filter)
{
    int rv;

    uint32_t _filter = ((unsigned long)filter.filter_priority << 26) |
                    ((unsigned long)filter.filter_class << 16) |
                    ((unsigned long)filter.filter_type << 8) | filter.filter_GUID[0];
    if ( CANAL_ERROR_SUCCESS == (rv = m_canalif.CanalSetFilter(_filter))) {
        return rv;
    }

    uint32_t _mask = ((unsigned long)filter.mask_priority << 26) |
                    ((unsigned long)filter.mask_class << 16) |
                    ((unsigned long)filter.mask_type << 8) | filter.mask_GUID[0];
    return m_canalif.CanalSetMask(_mask);
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int vscpClientLevel2::getcount(uint16_t *pcount)
{
    return m_canalif.CanalDataAvailable();
}

//////////////////////////////////////////////////////////////////////////////
// clear
//

int vscpClientLevel2::clear()
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getversion
//

int vscpClientLevel2::getversion(uint8_t *pmajor,
                                    uint8_t *pminor,
                                    uint8_t *prelease,
                                    uint8_t *pbuild)
{

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int vscpClientLevel2::getinterfaces(std::deque<std::string> &iflist)
{
    // No interfaces available
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int vscpClientLevel2::getwcyd(uint64_t &wcyd)
{
    wcyd = VSCP_SERVER_CAPABILITY_NONE;   // No capabilities
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void vscpClientLevel2::setConnectionTimeout(uint32_t timeout)
{
    ;
}

//////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t vscpClientLevel2::getConnectionTimeout(void)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void vscpClientLevel2::setResponseTimeout(uint32_t timeout)
{
    ;
}

//////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t vscpClientLevel2::getResponseTimeout(void)
{
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int vscpClientLevel2::setCallback(LPFNDLL_EV_CALLBACK m_evcallback)
{
    // Can not be called when connected
    if ( m_bConnected ) return VSCP_ERROR_ERROR;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int vscpClientLevel2::setCallback(LPFNDLL_EX_CALLBACK m_excallback)
{
    // Can not be called when connected
    if ( m_bConnected ) return VSCP_ERROR_ERROR;

    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Workerthread
//
// This thread call the appropriate callback when events are received
//


// static void *workerThread(void *pObj)
// {
//     uint8_t guid[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//     vscpClientLevel2 *pClient = (vscpClientLevel2 *)pObj;
//     VscpCanalDeviceIf *pif = (VscpCanalDeviceIf *)&(pClient->m_canalif);

//     if (NULL == pif) return NULL;

//     while (pClient->m_bRun) {

//         //pthread_mutex_lock(&pif->m_mutexif);
        
//         // Check if there are events to fetch
//         int cnt;
//         if ((cnt = pClient->m_canalif.CanalDataAvailable())) {

//             while (cnt) {
//                 canalMsg msg;
//                 if ( CANAL_ERROR_SUCCESS == pClient->m_canalif.CanalReceive(&msg) ) {
//                     if ( NULL != pClient->m_evcallback ) {
//                         vscpEvent ev;
//                         if (vscp_convertCanalToEvent(&ev,
//                                                         &msg,
//                                                         guid) ) {
//                             pClient->m_evcallback(&ev);
//                         }
//                     }
//                     if ( NULL != pClient->m_excallback ) {
//                         vscpEventEx ex;
//                         if (vscp_convertCanalToEventEx(&ex,
//                                                         &msg,
//                                                         guid) ) {
//                             pClient->m_excallback(&ex);
//                         }
//                     }
//                 }
//                 cnt--;
//             }

//         }

//         //pthread_mutex_unlock(&pif->m_mutexif);
//         usleep(200);
//     }

//     return NULL;
// }

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

    vscpClientLevel2 *pObj = (vscpClientLevel2 *)pData;
    if (NULL == pObj) {
        //syslog(LOG_ERR, "No object data supplied for worker thread");
        return NULL;
    }

    strncpy(devname, pObj->m_interface.c_str(), sizeof(devname) - 1);

    while (pObj->m_bRun) {

        sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (sock < 0) {

            if (ENETDOWN == errno) {
                sleep(1);
                continue;   // We try again
            }

            if (pObj->m_flags & pObj->FLAG_ENABLE_DEBUG) {
                syslog(LOG_ERR,
                       "%s",
                    (const char *)"ReadSocketCanTread: Error while "
                                     "opening socket. Terminating!");
            }
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
        while (!pObj->m_bRun && bInnerLoop) {

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
                    pObj->m_bRun = false;
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
                        pObj->m_bRun = false;
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
                    pEvent->vscp_class = vscp_getVscpClassFromCANALid(frame.can_id);

                    // Set VSCP type
                    pEvent->vscp_type = vscp_getVscpTypeFromCANALid(frame.can_id);

                    // Copy data if any
                    pEvent->sizeData = frame.len;
                    if (frame.len) {
                        memcpy(pEvent->pdata, frame.data, frame.len);
                    }

                    // if (vscp_doLevel2Filter(pEvent, &pObj->m_vscpfilter)) {
                    //     pthread_mutex_lock( &pObj->m_mutexReceiveQueue);
                    //     pObj->m_receiveList.push_back(pEvent);
                    //     sem_post(&pObj->m_semReceiveQueue);
                    //     pthread_mutex_unlock( &pObj->m_mutexReceiveQueue);
                    // } else {
                    //     vscp_deleteVscpEvent(pEvent);
                    // }
                }

            } else {

                // Check if there is event(s) to send
                // if (pObj->m_sendList.size()) {

                //     // Yes there are data to send
                //     // So send it out on the CAN bus

                //     pthread_mutex_lock( &pObj->m_mutexSendQueue);
                //     vscpEvent *pEvent = pObj->m_sendList.front();
                //     pObj->m_sendList.pop_front();
                //     pthread_mutex_unlock( &pObj->m_mutexSendQueue);

                //     if (NULL == pEvent) continue;

                //     // Class must be a Level I class or a Level II
                //     // mirror class
                //     if (pEvent->vscp_class < 512) {
                //         frame.can_id = vscp_getCANALidFromEvent(pEvent);
                //         frame.can_id |= CAN_EFF_FLAG; // Always extended
                //         if (0 != pEvent->sizeData) {
                //             frame.len =
                //               (pEvent->sizeData > 8 ? 8 : pEvent->sizeData);
                //             memcpy(frame.data, pEvent->pdata, frame.len);
                //         }
                //     } else if (pEvent->vscp_class < 1024) {
                //         pEvent->vscp_class -= 512;
                //         frame.can_id = vscp_getCANALidFromEvent(pEvent);
                //         frame.can_id |= CAN_EFF_FLAG; // Always extended
                //         if (0 != pEvent->sizeData) {
                //             frame.len = ((pEvent->sizeData - 16) > 8
                //                            ? 8
                //                            : pEvent->sizeData - 16);
                //             memcpy(frame.data, pEvent->pdata + 16, frame.len);
                //         }
                //     }

                //     // Remove the event
                //     pthread_mutex_lock( &pObj->m_mutexSendQueue);
                //     vscp_deleteVSCPevent(pEvent);
                //     pthread_mutex_unlock( &pObj->m_mutexSendQueue);

                //     // Write the data
                //     int nbytes = write(sock, &frame, sizeof(struct can_frame));

                //} // event to send

            } // No data to read

        } // Inner loop

        // Close the socket
        close(sock);

    } // Outer loop

    return NULL;
}

#endif