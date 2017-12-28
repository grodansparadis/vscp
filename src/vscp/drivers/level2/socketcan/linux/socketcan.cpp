// socketcan.cpp: implementation of the Csocketcan class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP Project (http://www.vscp.org) 
//
// Copyright (C) 2000-2017 Ake Hedman, 
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

#include <stdio.h>
#include "unistd.h"
#include "stdlib.h"
#include <string.h>
#include "limits.h"
#include "syslog.h"
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

// Different on Kernel 2.6 and cansocket examples
// currently using locally from can-utils
// TODO remove include form makefile when they are in sync
#include <linux/can.h>
#include <linux/can/raw.h>

#include <signal.h>
#include <ctype.h>
#include <libgen.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>
#include <errno.h>

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>

#include "../../../../common/vscp.h"
#include "../../../../common/vscphelper.h"
#include "../../../../common/vscpremotetcpif.h"
#include "../../../../common/vscp_type.h"
#include "../../../../common/vscp_class.h"
#include "socketcan.h"

// queues
//WX_DEFINE_LIST(VSCPEVENTLIST_SEND);
//WX_DEFINE_LIST(VSCPEVENTLIST_RECEIVE);

//////////////////////////////////////////////////////////////////////
// Csocketcan
//

Csocketcan::Csocketcan() {
    m_bQuit = false;
    m_pthreadWorker = NULL;
    m_interface = _("vcan0");
    vscp_clearVSCPFilter(&m_vscpfilter); // Accept all events
    ::wxInitialize();
}

//////////////////////////////////////////////////////////////////////
// ~Csocketcan
//

Csocketcan::~Csocketcan() {
    close();
    ::wxUninitialize();
}


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
    bool rv = true;
    wxString wxstr = wxString::FromAscii(pConfig);

    m_username = wxString::FromAscii(pUsername);
    m_password = wxString::FromAscii(pPassword);
    m_host = wxString::FromAscii(pHost);
    m_port = port;
    m_prefix = wxString::FromAscii(pPrefix);

    // Parse the configuration string. It should
    // have the following form
    // path
    // 
    wxStringTokenizer tkz(wxString::FromAscii(pConfig), _(";\n"));

    // Check for socketcan interface in configuration string
    if (tkz.HasMoreTokens()) {
        // Interface
        m_interface = tkz.GetNextToken();
    }


    // First log on to the host and get configuration 
    // variables

    if (VSCP_ERROR_SUCCESS != m_srv.doCmdOpen( m_host,
                                                    m_username,
                                                    m_password ) ) {
        syslog( LOG_ERR,
                "%s",
                (const char *) "Unable to connect to "
                "VSCP TCP/IP interface. Terminating!");
        return false;
    }

    // Find the channel id
    uint32_t ChannelID;
    m_srv.doCmdGetChannelID(&ChannelID);

    //m_srv.doCmdGetGUID( m_ifguid );

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

    wxString str;
    wxString strName = m_prefix +
            wxString::FromAscii("_interface");
    m_srv.getRemoteVariableValue(strName, m_interface);

    strName = m_prefix +
            wxString::FromAscii("_filter");
    if (m_srv.getRemoteVariableValue(strName, str)) {
        vscp_readFilterFromString(&m_vscpfilter, str);
    }

    strName = m_prefix +
            wxString::FromAscii("_mask");
    if (m_srv.getRemoteVariableValue(strName, str)) {
        vscp_readMaskFromString(&m_vscpfilter, str);
    }

    m_srv.doClrInputQueue();

    // start the workerthread
    m_pthreadWorker = new CSocketCanWorkerTread();
    if (NULL != m_pthreadWorker) {
        m_pthreadWorker->m_pObj = this;
        m_pthreadWorker->Create();
        m_pthreadWorker->Run();
    } else {
        rv = false;
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
    wxSleep(1); // Give the thread some time to terminate

}

//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool
Csocketcan::addEvent2SendQueue(const vscpEvent *pEvent) 
{
    m_mutexSendQueue.Lock();
    //m_sendQueue.Append((vscpEvent *)pEvent);
    m_sendList.push_back((vscpEvent *) pEvent);
    m_semSendQueue.Post();
    m_mutexSendQueue.Unlock();
    return true;
}



//////////////////////////////////////////////////////////////////////
//                Workerthread - CSocketCanWorkerTread
//////////////////////////////////////////////////////////////////////

CSocketCanWorkerTread::CSocketCanWorkerTread() 
{
    m_pObj = NULL;
}

CSocketCanWorkerTread::~CSocketCanWorkerTread() 
{
    ;
}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *
CSocketCanWorkerTread::Entry() 
{
    int sock;
    char devname[IFNAMSIZ + 1];
    fd_set rdfs;
    struct timeval tv;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct cmsghdr *cmsg;
    struct canfd_frame frame;
    char ctrlmsg[CMSG_SPACE(sizeof (struct timeval)) + CMSG_SPACE(sizeof (__u32))];
    const int canfd_on = 1;

    ::wxInitialize();

    // Check pointers
    if (NULL == m_pObj) return NULL;

    strncpy(devname, m_pObj->m_interface.ToAscii(), sizeof (devname) - 1);
#if DEBUG 
    syslog(LOG_ERR, "CWriteSocketCanTread: Interface: %s\n", ifname);
#endif 

    while (!TestDestroy() && !m_pObj->m_bQuit) {

        sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (sock < 0) {

            if (ENETDOWN == errno) {
                sleep(1);
                continue;
            }

            syslog(LOG_ERR,
                    "%s",
                    (const char *) "CReadSocketCanTread: Error while "
                    "opening socket. Terminating!");

            m_pObj->m_bQuit;
            continue;
        }

        strcpy(ifr.ifr_name, devname);
        ioctl(sock, SIOCGIFINDEX, &ifr);

        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

#ifdef DEBUG
        printf("using interface name '%s'.\n", ifr.ifr_name);
#endif

        // try to switch the socket into CAN FD mode 
        setsockopt(sock,
                SOL_CAN_RAW,
                CAN_RAW_FD_FRAMES,
                &canfd_on,
                sizeof (canfd_on));

        if (bind(sock, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
            syslog(LOG_ERR,
                    "%s",
                    (const char *) "CReadSocketCanTread: Error in socket bind. "
                    "Terminating!");
            close(sock);
            sleep(2);
            continue;
        }

        bool bInnerLoop = true;
        while (!TestDestroy() && !m_pObj->m_bQuit && bInnerLoop) {

            FD_ZERO(&rdfs);
            FD_SET(sock, &rdfs);

            tv.tv_sec = 0;
            tv.tv_usec = 5000; // 5ms timeout 

            int ret;
            if ((ret = select(sock + 1, &rdfs, NULL, NULL, &tv)) < 0) {
                // Error
                if (ENETDOWN == errno) {
                    // We try to get contact with the net
                    // again if it goes down
                    bInnerLoop = false;
                } else {
                    m_pObj->m_bQuit = true;
                }
                continue;
            }

            if (ret) {

                // There is data to read

                ret = read(sock, &frame, sizeof (struct can_frame));
                if (ret < 0) {
                    if (ENETDOWN == errno) {
                        // We try to get contact with the net
                        // again if it goes down
                        bInnerLoop = false;
                        sleep(2);
                    } else {
                        m_pObj->m_bQuit = true;
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
                    pEvent->vscp_class = vscp_getVSCPclassFromCANALid(frame.can_id);

                    // Set VSCP type
                    pEvent->vscp_type = vscp_getVSCPtypeFromCANALid(frame.can_id);

                    // Copy data if any
                    pEvent->sizeData = frame.len;
                    if (frame.len) {
                        memcpy(pEvent->pdata, frame.data, frame.len);
                    }

                    if (vscp_doLevel2Filter(pEvent, &m_pObj->m_vscpfilter)) {
                        m_pObj->m_mutexReceiveQueue.Lock();
                        //m_pObj->m_receiveQueue.Append(pEvent);
                        m_pObj->m_receiveList.push_back(pEvent);
                        m_pObj->m_semReceiveQueue.Post();
                        m_pObj->m_mutexReceiveQueue.Unlock();
                    } else {
                        vscp_deleteVSCPevent(pEvent);
                    }
                }

            } else {

                // Check if there is event(s) to send
                if (m_pObj->m_sendList.size() /*m_pObj->m_sendQueue.GetCount()*/) {

                    // Yes there are data to send
                    // So send it out on the CAN bus

                    //VSCPEVENTLIST_SEND::compatibility_iterator nodeClient;
                    m_pObj->m_mutexSendQueue.Lock();
                    //nodeClient = m_pObj->m_sendQueue.GetFirst();
                    vscpEvent *pEvent = m_pObj->m_sendList.front();
                    m_pObj->m_sendList.pop_front();
                    m_pObj->m_mutexSendQueue.Unlock();

                    if (NULL == pEvent) continue;

                    // Class must be a Level I class or a Level II
                    // mirror class
                    if (pEvent->vscp_class < 512) {
                        frame.can_id = vscp_getCANALidFromVSCPevent(pEvent);
                        frame.can_id |= CAN_EFF_FLAG; // Always extended
                        if (0 != pEvent->sizeData) {
                            frame.len = (pEvent->sizeData > 8 ? 8 : pEvent->sizeData);
                            memcpy(frame.data, pEvent->pdata, frame.len);
                        }
                    } 
                    else if (pEvent->vscp_class < 1024) {
                        pEvent->vscp_class -= 512;
                        frame.can_id = vscp_getCANALidFromVSCPevent(pEvent);
                        frame.can_id |= CAN_EFF_FLAG; // Always extended
                        if (0 != pEvent->sizeData) {
                            frame.len = 
                                    ((pEvent->sizeData - 16) > 8 ? 8 : pEvent->sizeData - 16);
                            memcpy(frame.data, pEvent->pdata + 16, frame.len);
                        }
                    }

                    // Remove the event
                    m_pObj->m_mutexSendQueue.Lock();
                    //m_pObj->m_sendQueue.DeleteNode(nodeClient);
                    vscp_deleteVSCPevent(pEvent);
                    m_pObj->m_mutexSendQueue.Unlock();


                    // Write the data
                    int nbytes = write(sock, &frame, sizeof (struct can_frame));

                } // event to send
            } // No data to read
        } // Inner loop

        // Close the socket
        close(sock);

    } // Outer loop

    return NULL;

}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void
CSocketCanWorkerTread::OnExit() 
{
    ;
}



