// vscp_client_socketcan.h
//
// CANAL client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:   © 2007-2021
// Ake Hedman, Grodans Paradis AB, <akhe@vscp.org>
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

#include <unistd.h>
#include <pthread.h> 

#include "vscp_client_socketcan.h"
#include "vscphelper.h"

// Forward declaration
static void *workerThread(void *pObj);

///////////////////////////////////////////////////////////////////////////////
// C-tor
//

vscpClientSocketCan::vscpClientSocketCan() 
{
    m_type = CVscpClient::connType::CANAL;
    m_bConnected = false;  // Not connected
    m_tid = 0;
    m_bRun = false;
    pthread_mutex_init(&m_mutexif,NULL);
}

///////////////////////////////////////////////////////////////////////////////
// D-tor
//

vscpClientSocketCan::~vscpClientSocketCan() 
{
    disconnect();
    pthread_mutex_destroy(&m_mutexif);
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int vscpClientSocketCan::init(const std::string &strPath,
                const std::string &strParameters,
                unsigned long flags,
                unsigned long baudrate)
{
    m_canalif.CanalSetBaudrate(baudrate);   // Unusual implemention and return value may differ if
                                            // not implemented so we do not check
                                            // A standard implemention should return 
                                            // CANAL_ERROR_NOT_SUPPORTED if this method is not
                                            // implemented
    return m_canalif.init(strPath,strParameters,flags);  
}

///////////////////////////////////////////////////////////////////////////////
// toJSON
//

std::string vscpClientSocketCan::toJSON(void) 
{
    std::string rv;

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// fromJSON
//

bool vscpClientSocketCan::fromJSON(const std::string& config)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int vscpClientSocketCan::connect(void) 
{
    int rv = m_canalif.CanalOpen();
    if ( VSCP_ERROR_SUCCESS == rv ) {
        m_bConnected = true;
    }

    // Start worker thread if a callback has been defined
    if ( (NULL != m_evcallback) || (NULL != m_excallback) ) {
        int rv = pthread_create(&m_tid, NULL, workerThread, this); 
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int vscpClientSocketCan::disconnect(void)
{
    m_bRun = false;
    pthread_join(m_tid, NULL); 
    
    m_bConnected = false;
    return m_canalif.CanalClose();
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool vscpClientSocketCan::isConnected(void)
{
    return m_bConnected;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientSocketCan::send(vscpEvent &ev)
{
    canalMsg canalMsg;
    if ( !vscp_convertEventToCanal(&canalMsg, &ev ) ) {
        return VSCP_ERROR_PARAMETER;    
    }

    return m_canalif.CanalSend(&canalMsg);
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientSocketCan::send(vscpEventEx &ex)
{
    canalMsg canalMsg;
    if ( !vscp_convertEventExToCanal(&canalMsg, &ex ) ) {
        return VSCP_ERROR_PARAMETER;    
    }

    return m_canalif.CanalSend(&canalMsg);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientSocketCan::receive(vscpEvent &ev)
{
    int rv;
    canalMsg canalMsg;
    uint8_t guid[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    
    if ( CANAL_ERROR_SUCCESS != (rv = m_canalif.CanalReceive(&canalMsg) ) ) {
        return rv;
    }

    return vscp_convertCanalToEvent(&ev,
                                    &canalMsg,
                                    guid);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientSocketCan::receive(vscpEventEx &ex)
{
    int rv;
    canalMsg canalMsg;
    uint8_t guid[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    
    if ( CANAL_ERROR_SUCCESS != (rv = m_canalif.CanalReceive(&canalMsg) ) ) {
        return rv;
    }

    return vscp_convertCanalToEventEx(&ex,
                                        &canalMsg,
                                        guid);
}

///////////////////////////////////////////////////////////////////////////////
// setfilter
//

int vscpClientSocketCan::setfilter(vscpEventFilter &filter)
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

int vscpClientSocketCan::getcount(uint16_t *pcount)
{
    return m_canalif.CanalDataAvailable();
}

//////////////////////////////////////////////////////////////////////////////
// clear
//

int vscpClientSocketCan::clear()
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getversion
//

int vscpClientSocketCan::getversion(uint8_t *pmajor,
                                    uint8_t *pminor,
                                    uint8_t *prelease,
                                    uint8_t *pbuild)
{
    uint32_t ver = m_canalif.CanalGetDllVersion();

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int vscpClientSocketCan::getinterfaces(std::deque<std::string> &iflist)
{
    // No interfaces available
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int vscpClientSocketCan::getwcyd(uint64_t &wcyd)
{
    wcyd = VSCP_SERVER_CAPABILITY_NONE;   // No capabilities
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void vscpClientSocketCan::setConnectionTimeout(uint32_t timeout)
{
    ;
}

//////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t vscpClientSocketCan::getConnectionTimeout(void)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void vscpClientSocketCan::setResponseTimeout(uint32_t timeout)
{
    ;
}

//////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t vscpClientSocketCan::getResponseTimeout(void)
{
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int vscpClientSocketCan::setCallback(LPFNDLL_EV_CALLBACK m_evcallback)
{
    // Can not be called when connected
    if ( m_bConnected ) return VSCP_ERROR_ERROR;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int vscpClientSocketCan::setCallback(LPFNDLL_EX_CALLBACK m_excallback)
{
    // Can not be called when connected
    if ( m_bConnected ) return VSCP_ERROR_ERROR;

    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Callback workerthread
//
// This thread call the appropriate callback when events are received
//


static void *workerThread(void *pObj)
{
    uint8_t guid[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    vscpClientSocketCan *pClient = (vscpClientSocketCan *)pObj;
    VscpCanalDeviceIf *pif = (VscpCanalDeviceIf *)&(pClient->m_canalif);

    if (NULL == pif) return NULL;

    while (pClient->m_bRun) {

        //pthread_mutex_lock(&pif->m_mutexif);
        
        // Check if there are events to fetch
        int cnt;
        if ((cnt = pClient->m_canalif.CanalDataAvailable())) {

            while (cnt) {
                canalMsg msg;
                if ( CANAL_ERROR_SUCCESS == pClient->m_canalif.CanalReceive(&msg) ) {
                    if ( NULL != pClient->m_evcallback ) {
                        vscpEvent ev;
                        if (vscp_convertCanalToEvent(&ev,
                                                        &msg,
                                                        guid) ) {
                            pClient->m_evcallback(&ev);
                        }
                    }
                    if ( NULL != pClient->m_excallback ) {
                        vscpEventEx ex;
                        if (vscp_convertCanalToEventEx(&ex,
                                                        &msg,
                                                        guid) ) {
                            pClient->m_excallback(&ex);
                        }
                    }
                }
                cnt--;
            }

        }

        //pthread_mutex_unlock(&pif->m_mutexif);
        usleep(200);
    }

    return NULL;
}
