

#include "vscpl2drv_lmsensors.h"
#include "lmsensors.h"
#include "stdio.h"
#include "stdlib.h"

void
_init() __attribute__((constructor));
void
_fini() __attribute__((destructor));

#include <map>
#include <string>

#include "semaphore.h"
#include "stdio.h"
#include "stdlib.h"

// This map holds driver handles/objects
static std::map<long, Clmsensors *> g_ifMap;

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

    for (std::map<long, Clmsensors *>::iterator it = g_ifMap.begin();
         it != g_ifMap.end();
         ++it) {
        // std::cout << it->first << " => " << it->second << '\n';

        Clmsensors *pif = it->second;
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
addDriverObject(Clmsensors *pif)
{
    std::map<long, Clmsensors *>::iterator it;
    long h = 0;

    LOCK_MUTEX(g_mapMutex);

    // Find free handle
    while (true) {
        if (g_ifMap.end() != (it = g_ifMap.find(h))) break;
        h++;
    };

    g_ifMap[h] = new Clmsensors;
    h += 1681;

    UNLOCK_MUTEX(g_mapMutex);

    return h;
}

///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

Clmsensors *
getDriverObject(long h)
{
    std::map<long, Clmsensors *>::iterator it;
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return NULL;

    it = g_ifMap.find(h);
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
    std::map<long, Clmsensors *>::iterator it;
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return;

    LOCK_MUTEX(g_mapMutex);
    it = g_ifMap.find(h);
    if (it != g_ifMap.end()) {
        Clmsensors *pObj = it->second;
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

    Clmsensors *pdrvObj = new Clmsensors();
    if (NULL != pdrvObj) {

        if (pdrvObj->open(
              pUsername, pPassword, pHost, port, pPrefix, pParameter)) {

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

extern "C" int
VSCPClose(long handle)
{
    int rv = 0;

    Clmsensors *pdrvObj = getDriverObject(handle);
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

    Clmsensors *pdrvObj = getDriverObject(handle);
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

    uint16_t secs = timeout / 1000;
    uint32_t ns   = (timeout - secs * 1000) * 1000;

    // Check pointer
    if (NULL == pEvent) return CANAL_ERROR_PARAMETER;

    Clmsensors *pdrvObj = getDriverObject(handle);
    if (NULL == pdrvObj) return CANAL_ERROR_MEMORY;

    struct timespec ts;
    ts.tv_sec  = secs;
    ts.tv_nsec = ns;
    if (ETIMEDOUT == sem_timedwait(&pdrvObj->m_semReceiveQueue, &ts)) {
        return CANAL_ERROR_TIMEOUT;
    }

    pthread_mutex_lock(&pdrvObj->m_mutexReceiveQueue);
    // nodeClient = pdrvObj->m_receiveQueue.GetFirst();
    // vscpEvent *pLocalEvent = nodeClient->GetData();
    vscpEvent *pLocalEvent = pdrvObj->m_receiveList.front();
    pdrvObj->m_receiveList.pop_front();
    pthread_mutex_unlock(&pdrvObj->m_mutexReceiveQueue);
    if (NULL == pLocalEvent) return CANAL_ERROR_MEMORY;

    vscp_copyVSCPEvent(pEvent, pLocalEvent);
    // pdrvObj->m_receiveQueue.DeleteNode(nodeClient);
    vscp_deleteVSCPevent(pLocalEvent);

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPGetLevel
//

extern "C" unsigned long
VSCPGetLevel(void)
{
    return CANAL_LEVEL_USES_TCPIP;
}

///////////////////////////////////////////////////////////////////////////////
// VSCPGetDllVersion
//

extern "C" unsigned long
VSCPGetDllVersion(void)
{
    return VSCP_DLL_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// VSCPGetVendorString
//

extern "C" const char *
VSCPGetVendorString(void)
{
    return VSCP_DLL_VENDOR;
}

///////////////////////////////////////////////////////////////////////////////
// VSCPGetDriverInfo
//

extern "C" const char *
VSCPGetDriverInfo(void)
{
    return VSCP_LMSENSORS_DRIVERINFO;
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
