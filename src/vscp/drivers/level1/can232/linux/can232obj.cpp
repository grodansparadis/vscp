///////////////////////////////////////////////////////////////////////////////
// CAN232Obj.cpp: implementation of the CCAN232Obj class.
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2012 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "can232obj.h"
#include "stdio.h"

// Prototypes
void *workThread(void *p);
bool can232ToCanal(char * p, PCANALMSG pMsg);

////////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////////////////

CCAN232Obj::CCAN232Obj()
{
    m_can232obj.m_version = -1; // Version is unknown
    m_can232obj.m_bAuto = false; // Poll mode is default
    m_can232obj.m_cntRcv = 0; // No chars in receive buffer yet
    m_can232obj.m_state = CAN232_STATE_NONE;
    dll_init(&m_can232obj.m_rcvList, SORT_NONE);
    dll_init(&m_can232obj.m_sndList, SORT_NONE);
}

CCAN232Obj::~CCAN232Obj()
{
    close(); // Close comm channel in case its open
    dll_removeAllNodes(&m_can232obj.m_rcvList);
    dll_removeAllNodes(&m_can232obj.m_sndList);
}


//------------------------------------------------------------------------------
// Open the CAN232 comm port
//
// Parameters are as follows
//
// 	comport;baudrate;mask;filter;bus-speed;btr0;btr1
//
// if btr0 and btr1 is defined they should be used instead of bus-speed which
// in that case can have any value even if zero is recommended.
//
//------------------------------------------------------------------------------

bool CCAN232Obj::open(const char *pDevice, unsigned long flags)
{
    int cnt;
    bool rv = false;
    const char *p;
    char szCmd[ 80 ];
    char szResponse[ 32 ];
    //----------------------------------------------------------------------
    //	Set default parameters
    //----------------------------------------------------------------------
    char MODEMDEVICE[20] = "/dev/ttyS0";
    char nBaudRate[10] = "57600";
    unsigned long nMask = 0;
    unsigned long nFilter = 0;
    unsigned long busspeed = 125;
    unsigned char btr0 = 0, btr1 = 0;
    unsigned char nSpeed = 4; // 125k


    //----------------------------------------------------------------------
    //	Accure Mutex
    //----------------------------------------------------------------------
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_mutex_init(&m_can232ObjMutex, NULL);

    m_can232obj.m_bRun = true;

    // if open we have noting to do
    if (0 != m_can232obj.m_comm.getFD()) return 0;

    //----------------------------------------------------------------------
    //	Parse given parameters
    //----------------------------------------------------------------------
    // Port
    p = strtok((char *) pDevice, ";");
    if (NULL != p) strncpy(MODEMDEVICE, p, strlen(p));

    // Baudrate
    p = strtok(NULL, ";");
    if (NULL != p) {
        strncpy(nBaudRate, p, strlen(p));
    }

    // Mask
    p = strtok(NULL, ";");
    if (NULL != p) {
        if ((NULL != strstr(p, "0x")) || (NULL != strstr(p, "0X"))) {
            sscanf(p + 2, "%lx", &nMask);
        } else {
            nMask = atol(p);
        }
    }

    // Filter
    p = strtok(NULL, ";");
    if (NULL != p) {
        if ((NULL != strstr(p, "0x")) || (NULL != strstr(p, "0X"))) {
            sscanf(p + 2, "%lx", &nFilter);
        } else {
            nFilter = atol(p);
        }
    }

    // Bus-Speed
    p = strtok(NULL, ";");
    if (NULL != p) busspeed = atol(p);

    // BTR0
    p = strtok(NULL, ";");
    if (NULL != p) btr0 = atoi(p);

    // BTR1
    p = strtok(NULL, ";");
    if (NULL != p) btr1 = atoi(p);

    //----------------------------------------------------------------------
    // Open Serial Port
    //----------------------------------------------------------------------
    if (!m_can232obj.m_comm.open(MODEMDEVICE)) {
        syslog(LOG_CRIT, "can232obj: Open [%s] failed\n", MODEMDEVICE);
        return 0;
    }

    syslog(LOG_CRIT, "can232obj: Open [%s] successful\n", MODEMDEVICE);
    rv = true;

    //----------------------------------------------------------------------
    // Comm::setParam( char *baud, char *parity, char *bits, int HWFlow, int SWFlow )
    //----------------------------------------------------------------------
    m_can232obj.m_comm.setParam(nBaudRate, (char*) "N", (char*) "8", 0, 0);

    //----------------------------------------------------------------------
    //
    //----------------------------------------------------------------------
    m_can232obj.m_receiveBuf[ 0 ] = 0;
    m_can232obj.m_cntRcv = 0;

    // Initiate statistics
    m_can232obj.m_stat.cntReceiveData = 0;
    m_can232obj.m_stat.cntReceiveFrames = 0;
    m_can232obj.m_stat.cntTransmitData = 0;
    m_can232obj.m_stat.cntTransmitFrames = 0;

    m_can232obj.m_stat.cntBusOff = 0;
    m_can232obj.m_stat.cntBusWarnings = 0;
    m_can232obj.m_stat.cntOverruns = 0;

    //----------------------------------------------------------------------
    //
    //----------------------------------------------------------------------
    switch (busspeed) {

    case 10:
        nSpeed = 0;
        break;

    case 20:
        nSpeed = 1;
        break;

    case 50:
        nSpeed = 2;
        break;

    case 100:
        nSpeed = 3;
        break;

    case 125:
        nSpeed = 4;
        break;

    case 250:
        nSpeed = 5;
        break;

    case 500:
        nSpeed = 6;
        break;

    case 800:
        nSpeed = 7;
        break;

    case 1000:
        nSpeed = 8;
        break;

    }


    //----------------------------------------------------------------------
    // Needed in case of not closed by last process.
    //----------------------------------------------------------------------
    sprintf(szCmd, "C\r");

    bool CANClose = FALSE;
    int try_count = 0;
    printf("Trying to close CAN bus first\n");
    while (!CANClose) {
        printf("C Write [");
        for (int i = 0; i < strlen(szCmd); i++) {
            printf(" %02X", szCmd[i]);
        }
        printf("]\n");
        m_can232obj.m_comm.comm_puts(szCmd, strlen(szCmd), true);
        SLEEP(100);
        //*szResponse = 0;
        //for (int i=0; i<sizeof(szResponse); i++) szResponse[i]=0;
        cnt = m_can232obj.m_comm.comm_gets(szResponse, sizeof(szResponse), 10000);
        if (szResponse[0] == '\7' || szResponse[0] == '\r') CANClose = TRUE;
        printf("C Read  [");
        for (int i = 0; i < cnt; i++) {
            printf(" %02X", szResponse[i]);
        }
        printf("]\n");
        SLEEP(100);
        if (try_count++ > 10) {
            syslog(LOG_CRIT, "can232obj: Unable to Close CAN bus before Open!\n");
            m_can232obj.m_bRun = false;
            m_can232obj.m_comm.close();
            return 0;
        }

    }
    printf("CAN bus closed\n");
    //----------------------------------------------------------------------

    SLEEP(100);
    m_can232obj.m_comm.drainInput();

    sprintf(szCmd, "\r");
    m_can232obj.m_comm.comm_puts(szCmd, strlen(szCmd), true);
    SLEEP(100);
    m_can232obj.m_comm.drainInput();

    sprintf(szCmd, "\r");
    m_can232obj.m_comm.comm_puts(szCmd, strlen(szCmd), true);
    SLEEP(100);
    m_can232obj.m_comm.drainInput();

    //----------------------------------------------------------------------
    // Get version
    //----------------------------------------------------------------------
    sprintf(szCmd, "V\r");
    bool GETVersion = FALSE;
    try_count = 0;
    printf("Trying to get version\n");
    while (!GETVersion) {
        printf("V Write [");
        for (int i = 0; i < strlen(szCmd); i++) {
            printf("%02X ", szCmd[i]);
        }
        printf("]\n");
        m_can232obj.m_comm.comm_puts(szCmd, strlen(szCmd), true);
        //*szResponse = 0;
        //for (int i=0; i<sizeof(szResponse); i++) szResponse[i]=0;
        cnt = m_can232obj.m_comm.comm_gets(szResponse, sizeof(szResponse), 10000);
        printf("V Read  [");
        for (int i = 0; i < cnt; i++) {
            printf("%02X ", szResponse[i]);
        }
        printf("]\n");

        if (*szResponse == 'V' && strlen(szResponse) > 5) {
            m_can232obj.m_version = atoi(szResponse + 1);
            GETVersion = TRUE;
        }
        SLEEP(100);
        if (++try_count > 10) {
            syslog(LOG_CRIT, "can232obj: Unable to get can232 dongle version!\n");
            m_can232obj.m_bRun = false;
            m_can232obj.m_comm.close();
            return 0;
        }
    }
    printf("Get Version: [%d]\n", m_can232obj.m_version);
    syslog(LOG_CRIT, "can232obj: Get Version: [%d]\n", m_can232obj.m_version);


    //----------------------------------------------------------------------
    // Set bitrate
    //----------------------------------------------------------------------
    if (btr0 || btr1) {
        sprintf(szCmd, "s%2X%2X\r", btr0, btr1);
        if (!m_can232obj.m_comm.comm_puts(szCmd, strlen(szCmd), true)) {
            m_can232obj.m_bRun = false;
            m_can232obj.m_comm.close();
            return 0;
        }
    } else {
        sprintf(szCmd, "S%u\r", nSpeed);
        if (!m_can232obj.m_comm.comm_puts(szCmd, strlen(szCmd), true)) {
            m_can232obj.m_bRun = false;
            m_can232obj.m_comm.close();
            return 0;
        }
    }
    *szResponse = 0;
    cnt = m_can232obj.m_comm.comm_gets(szResponse, sizeof(szResponse), 10000);
    printf("sS [");
    for (int i = 0; i < cnt; i++) {
        printf("%02X ", szResponse[i]);
    }
    printf("]\n");

    //----------------------------------------------------------------------
    // Open CAN Bus
    //----------------------------------------------------------------------
    sprintf(szCmd, "O\r");
    printf("O Write [");
    for (int i = 0; i < strlen(szCmd); i++) {
        printf("%02X ", szCmd[i]);
    }
    printf("]\n");
    if (!m_can232obj.m_comm.comm_puts(szCmd, strlen(szCmd), true)) {
        m_can232obj.m_bRun = false;
        m_can232obj.m_comm.close();
        return 0;
    }
    *szResponse = 0;
    cnt = m_can232obj.m_comm.comm_gets(szResponse, sizeof(szResponse), 10000);
    printf("O Read  [");
    for (int i = 0; i < cnt; i++) {
        printf("%02X ", szResponse[i]);
    }
    printf("]\n");

    //----------------------------------------------------------------------
    // Check for autopoll mode
    //----------------------------------------------------------------------
    if (m_can232obj.m_version >= 1220) {
        sprintf(szCmd, "P\r");
        printf("P Write [");
        for (int i = 0; i < strlen(szCmd); i++) {
            printf("%02X ", szCmd[i]);
        }
        printf("]\n");
        m_can232obj.m_comm.comm_puts(szCmd, strlen(szCmd), true);
        *szResponse = 0;
        cnt = m_can232obj.m_comm.comm_gets(szResponse, sizeof(szResponse), 10000);
        printf("P Read  [");
        for (int i = 0; i < cnt; i++) {
            printf("%02X ", szResponse[i]);
        }
        printf("]\n");
        if (0x07 == *(szResponse)) {
            m_can232obj.m_bAuto = true;
        }
    }
    printf("Auto pool %s\n", m_can232obj.m_bAuto ? "enabled" : "disabled");
    syslog(LOG_CRIT, "can232obj: Auto pool %s", m_can232obj.m_bAuto ? "enabled" : "disabled");

    //----------------------------------------------------------------------
    // Start thread 
    //----------------------------------------------------------------------
    if (pthread_create(&m_threadId,
            &thread_attr,
            workThread,
            this)) {
        rv = false;
        close();
    }

    //----------------------------------------------------------------------
    // Release the mutex for other threads to use
    //----------------------------------------------------------------------
    pthread_mutex_unlock(&m_can232ObjMutex);

    return rv;
}


//------------------------------------------------------------------------------
//  close
//------------------------------------------------------------------------------

int CCAN232Obj::close(void)
{
    int rv = 0;

    // Do nothing if already terminated
    if (!m_can232obj.m_bRun) return 1;

    // Terminate the thread
    m_can232obj.m_bRun = false;

    UNLOCK_MUTEX(m_can232ObjMutex);
    LOCK_MUTEX(m_can232ObjMutex);

    char buf[] = "C\r";
    if (m_can232obj.m_comm.comm_puts(buf, strlen(buf), true)) {
        rv = 1;
    }

    // Give the worker thread some time to terminate
#ifdef WIN32
	Sleep( 1000 );
#else
	sleep( 1 );
#endif	
	
    int *trv;
    pthread_join(m_threadId, (void **) &trv);
    pthread_mutex_destroy(&m_can232ObjMutex);

    m_can232obj.m_comm.close();
    syslog(LOG_CRIT, "can232obj: close port\n");

    return rv;
}

//------------------------------------------------------------------------------
//  writeMsg
//------------------------------------------------------------------------------

int CCAN232Obj::writeMsg(PCANALMSG pCanalMsg)
{
    int rv = 0;

    // Must be room for the message
    if (m_can232obj.m_sndList.nCount < CAN232_MAX_SNDMSG) {
        if (NULL != pCanalMsg) {
            dllnode *pNode = new dllnode;
            if (NULL != pNode) {
                canalMsg *pnewMsg = new canalMsg;
                pNode->pObject = pnewMsg;
                pNode->pKey = NULL;
                pNode->pstrKey = NULL;
                if (NULL != pnewMsg) {
                    memcpy(pnewMsg, pCanalMsg, sizeof( canalMsg));
                }

                LOCK_MUTEX(m_can232ObjMutex);

                dll_addNode(&m_can232obj.m_sndList, pNode);

                UNLOCK_MUTEX(m_can232ObjMutex);

                rv = true;
            }
        }
    }
    return rv;
}

//------------------------------------------------------------------------------
//  readMsg
//------------------------------------------------------------------------------

int CCAN232Obj::readMsg(canalMsg *pMsg)
{
    int rv = false;

    if ((NULL != m_can232obj.m_rcvList.pHead) &&
            (NULL != m_can232obj.m_rcvList.pHead->pObject)) {

        LOCK_MUTEX(m_can232ObjMutex);

        memcpy(pMsg, m_can232obj.m_rcvList.pHead->pObject, sizeof( canalMsg));
        dll_removeNode(&m_can232obj.m_rcvList, m_can232obj.m_rcvList.pHead);

        UNLOCK_MUTEX(m_can232ObjMutex);

        rv = true;
    }
    
    return rv;
}

//------------------------------------------------------------------------------
//	setFilter
//------------------------------------------------------------------------------

bool CCAN232Obj::setFilter(unsigned long filter, unsigned long mask)
{
    char buf[ 20 ];
    char szCmd[ 80 ];

    LOCK_MUTEX(m_can232ObjMutex);

    // Acceptance Code
    sprintf(buf, "M%8.8lX\r", filter);
    m_can232obj.m_comm.comm_puts(buf, strlen(buf), true);
    *szCmd = 0;
    m_can232obj.m_comm.comm_gets(szCmd, sizeof( szCmd), 100000);
    printf("Open: Response: setFilter: [%s]\n", szCmd);

    // Acceptance Mask
    sprintf(buf, "m%8.8lX\r", mask);
    m_can232obj.m_comm.comm_puts(buf, strlen(buf), true);
    *szCmd = 0;
    m_can232obj.m_comm.comm_gets(szCmd, sizeof( szCmd), 100000);
    printf("Open: Response: setFilter: [%s]\n", szCmd);

    UNLOCK_MUTEX(m_can232ObjMutex);

    return true;
}

//------------------------------------------------------------------------------
//	setFilter
//------------------------------------------------------------------------------

bool CCAN232Obj::setFilter(unsigned long filter)
{
    char buf[ 20 ];
    char szCmd[ 80 ];

    LOCK_MUTEX(m_can232ObjMutex);

    // Acceptance Code
    sprintf(buf, "M%8.8lX\r", filter);
    m_can232obj.m_comm.comm_puts(buf, strlen(buf), true);
    *szCmd = 0;
    m_can232obj.m_comm.comm_gets(szCmd, sizeof( szCmd), 100000);
    printf("Open: Response: setFilter: [%s]\n", szCmd);

    UNLOCK_MUTEX(m_can232ObjMutex);

    return true;
}

//------------------------------------------------------------------------------
//	setMask
//------------------------------------------------------------------------------

bool CCAN232Obj::setMask(unsigned long mask)
{
    char buf[ 20 ];
    char szCmd[ 80 ];

    LOCK_MUTEX(m_can232ObjMutex);

    // Acceptance Mask
    sprintf(buf, "m%8.8lX\r", mask);
    m_can232obj.m_comm.comm_puts(buf, strlen(buf), true);
    *szCmd = 0;
    m_can232obj.m_comm.comm_gets(szCmd, sizeof( szCmd), 100000);
    printf("Open: Response: setFilter: [%s]\n", szCmd);

    UNLOCK_MUTEX(m_can232ObjMutex);

    return true;
}

//------------------------------------------------------------------------------
//	dataAvailable
//------------------------------------------------------------------------------

int CCAN232Obj::dataAvailable(void)
{
    int cnt;

    LOCK_MUTEX(m_can232ObjMutex);
    cnt = dll_getNodeCount(&m_can232obj.m_rcvList);
    UNLOCK_MUTEX(m_can232ObjMutex);

    return cnt;
}

//------------------------------------------------------------------------------
//	getStatistics
//------------------------------------------------------------------------------

bool CCAN232Obj::getStatistics(PCANALSTATISTICS pCanalStatistics)
{
    pCanalStatistics = &m_can232obj.m_stat;
    return true;
}

//------------------------------------------------------------------------------
//	getStatus
//------------------------------------------------------------------------------

bool CCAN232Obj::getStatus(PCANALSTATUS pCanalStatus)
{
    // Get flags
    //      B0 - CAN receive FIFO full.
    //      B1 - CAN transmit FIFO full.
    //      B2 - Error Warning (EI).
    //      B3 - Data Overrun (DOI).
    //      B4 - Not used.
    //      B5 - Error Passive (EPI).
    //      B6 - Arbitration Lost (ALI).
    //      B7 - Bus Error (BEI)

    return true;
}

//------------------------------------------------------------------------------
// workThread
//
// The workThread do most of the acual work such as send and receive.
//------------------------------------------------------------------------------

void *workThread(void *pObject)
{
    int rv = 0;
    int cnt;
    bool bActivity = true;
    short nPollCnt = 0;
    char szResponse[ 32 ];

    printf("can232obj in workThread\n");

    CCAN232Obj * pcan232obj = (CCAN232Obj *) pObject;
    if (NULL == pcan232obj) {
        pthread_exit(&rv);
    }

    while (pcan232obj->m_can232obj.m_bRun) {

        ///////////////////////////////////////////////////////////////////////
        //                                                              Receive 
        ///////////////////////////////////////////////////////////////////////

        LOCK_MUTEX(pcan232obj->m_can232ObjMutex);

        // Noting to do if we should end...
        if (!pcan232obj->m_can232obj.m_bRun) continue;

        int cnt;
        unsigned char c;
        //printf("workThread - Receive\n");

        c = pcan232obj->m_can232obj.m_comm.readChar(&cnt);

        while (-1 != cnt) {

            bActivity = true;

            if (CAN232_STATE_NONE == pcan232obj->m_can232obj.m_state) {
                if (('t' == c) || ('T' == c) || ('r' == c) || ('R' == c)) {
                    pcan232obj->m_can232obj.m_state = CAN232_STATE_MSG;
                    pcan232obj->m_can232obj.m_receiveBuf[ 0 ] = c;
                    pcan232obj->m_can232obj.m_cntRcv = 1;
                }
            } else if (CAN232_STATE_MSG == pcan232obj->m_can232obj.m_state) {
                pcan232obj->m_can232obj.m_receiveBuf[ pcan232obj->m_can232obj.m_cntRcv++ ] = c;
                if (0x0d == c) {
                    // One full message received If there is place in the queue 
                    // add message to it
                    if (pcan232obj->m_can232obj.m_rcvList.nCount < CAN232_MAX_RCVMSG) {
                        PCANALMSG pMsg = new canalMsg;
                        pMsg->flags = 0;
                        if (NULL != pMsg) {
                            dllnode *pNode = new dllnode;
                            if (NULL != pNode) {
                                printf("workThread R - m_receiveBuf = [%s]\n", pcan232obj->m_can232obj.m_receiveBuf);
                                int cnt = pcan232obj->m_can232obj.m_cntRcv;
                                printf("workThread R - m_receiveBuf [");
                                for (int i = 0; i < cnt; i++) {
                                    printf("%02X ", pcan232obj->m_can232obj.m_receiveBuf[i]);
                                }
                                printf("]\n");
                                if (!can232ToCanal(pcan232obj->m_can232obj.m_receiveBuf, pMsg)) {
                                    pNode->pObject = pMsg;
                                    dll_addNode(&pcan232obj->m_can232obj.m_rcvList, pNode);
                                    // Update statistics
                                    pcan232obj->m_can232obj.m_stat.cntReceiveData += pMsg->sizeData;
                                    pcan232obj->m_can232obj.m_stat.cntReceiveFrames += 1;
                                    printf("workThread R - RcvFrames = [%ld]\n\n", pcan232obj->m_can232obj.m_stat.cntReceiveFrames);
                                } else {
                                    // Failed to translate message
                                    printf("workThread R - Receive Failed to translate message\n");
                                    delete pMsg;
                                    delete pNode;
                                }
                            } else {
                                delete pMsg;
                            }
                        }
                    }
                    pcan232obj->m_can232obj.m_state = CAN232_STATE_NONE;
                }
                if (pcan232obj->m_can232obj.m_cntRcv > sizeof( pcan232obj->m_can232obj.m_receiveBuf)) {
                    // Problems start all over again
                    pcan232obj->m_can232obj.m_state = CAN232_STATE_NONE;
                }
            }
            c = pcan232obj->m_can232obj.m_comm.readChar(&cnt);
        } // while ( 0 != cnt )

        UNLOCK_MUTEX(pcan232obj->m_can232ObjMutex);

        ///////////////////////////////////////////////////////////////////////
        //                                                              Transmit
        ///////////////////////////////////////////////////////////////////////

        //printf("workThread - Transmit\n");
        LOCK_MUTEX(pcan232obj->m_can232ObjMutex);

        // Is there something to transmit
        //                while ( ( NULL != pcan232obj->m_can232obj.m_sndList.pHead ) &&
        //                                ( NULL != pcan232obj->m_can232obj.m_sndList.pHead->pObject ) ) {

        if ((NULL != pcan232obj->m_can232obj.m_sndList.pHead) &&
                (NULL != pcan232obj->m_can232obj.m_sndList.pHead->pObject)) {

            char buf[ 80 ];
            canalMsg msg;
            bActivity = true;

            memcpy(&msg, pcan232obj->m_can232obj.m_sndList.pHead->pObject, sizeof( canalMsg));
            dll_removeNode(&pcan232obj->m_can232obj.m_sndList, pcan232obj->m_can232obj.m_sndList.pHead);

            // Must be a valid standard id
            if (!(msg.flags & CANAL_IDFLAG_EXTENDED) && (msg.id > 0x7ff)) {
                msg.id &= 0x7ff;
            };

            // Must be a valid extended id
            if ((msg.flags & CANAL_IDFLAG_EXTENDED) && (msg.id > 0x1fffffff)) {
                msg.id &= 0x1fffffff;
            }

            // Currently there is a bug in ice old asm can232 tranceiver, and allow only small hex digits
            if (msg.flags & CANAL_IDFLAG_EXTENDED) {
                sprintf(buf, "T%8.8lx%i", msg.id, msg.sizeData);
            } else {
                sprintf(buf, "t%3.3lx%i", msg.id, msg.sizeData);
            }

            if (msg.sizeData) {
                char hex[5];

                for (int i = 0; i < msg.sizeData; i++) {
                    sprintf(hex, "%2.2X", msg.data[i]);
                    //sprintf( hex, "%02.2x", msg.data[i] );
                    strcat(buf, hex);
                }
                strcat(buf, "\r");
            }

            // Send the data
            pcan232obj->m_can232obj.m_comm.comm_puts(buf, strlen(buf), true);
            printf("workThread T - Write [");
            for (int i = 0; i < strlen(buf); i++) {
                if (buf[i] == 0x0D) {
                    printf("[CR]");
                } else {
                    printf("%c", buf[i]);
                }
            }
            printf("]\n");
            pcan232obj->m_can232obj.m_comm.comm_gets(szResponse, sizeof( szResponse), 10000);
            printf("workThread T - Read  [");
            for (int i = 0; i < strlen(szResponse); i++) {
                if (szResponse[i] == 0x0D) {
                    printf("[CR]");
                } else {
                    printf("%c", szResponse[i]);
                }
            }
            printf("]\n\n");
            // needed !! At least at 19200 baud
            SLEEP(100);

            // Update statistics
            pcan232obj->m_can232obj.m_stat.cntTransmitData += msg.sizeData;
            pcan232obj->m_can232obj.m_stat.cntTransmitFrames += 1;

            //} // while there is something to transmit
        } // if there is something to transmit

        // If not in autopoll mode we do a poll for all frames first
        nPollCnt++;

        if (!pcan232obj->m_can232obj.m_bAuto && (nPollCnt > 5)) {
            char szCmd[5];
            sprintf(szCmd, "A\r");
            pcan232obj->m_can232obj.m_comm.comm_puts(szCmd, strlen(szCmd), true);
            nPollCnt = 0;
        }

        UNLOCK_MUTEX(pcan232obj->m_can232ObjMutex);

        if (!bActivity) SLEEP(100);
        bActivity = false;

    } // while( pcan232obj->m_can232obj.m_bRun )
    pthread_exit(&rv);
}


///////////////////////////////////////////////////////////////////////////////
// can232ToCanal
//

bool can232ToCanal(char * p, PCANALMSG pMsg)
{
    bool rv = false;
    int val;
    short data_offset; // Offset to dlc byte
    char save;

    if ('t' == *p) {
        // Standard frame
        pMsg->flags = 0;
        data_offset = 5;
        pMsg->sizeData = p[ 4 ] - '0';
        p[ 4 ] = 0;
        sscanf(p + 1, "%lx", &pMsg->id);
    } else if ('r' == *p) {
        // Standard remote  frame
        pMsg->sizeData = 0;
        pMsg->flags = CANAL_IDFLAG_RTR;
        data_offset = 5 - 1; // To make timestamp work
        save = p[ 4 ];
        p[ 4 ] = 0;
        sscanf(p + 1, "%lx", &pMsg->id);
        p[ 4 ] = save;
    } else if ('T' == *p) {
        // Extended frame
        pMsg->flags = CANAL_IDFLAG_EXTENDED;
        data_offset = 10;
        pMsg->sizeData = p[ 9 ] - '0';
        p[ 9 ] = 0;
        sscanf(p + 1, "%lx", &pMsg->id);
    } else if ('R' == *p) {
        // Extended remote frame
        pMsg->sizeData = 0;
        pMsg->flags = CANAL_IDFLAG_EXTENDED | CANAL_IDFLAG_RTR;
        data_offset = 10 - 1; // To make timestamp work
        save = p[ 9 ];
        p[ 9 ] = 0;
        sscanf(p + 1, "%lx", &pMsg->id);
        p[ 9 ] = save;
    }

    save = *(p + data_offset + 2 * pMsg->sizeData);

    if (!(pMsg->flags & CANAL_IDFLAG_RTR)) {
        for (int i = pMsg->sizeData; i > 0; i--) {
            *(p + data_offset + 2 * (i - 1) + 2) = 0;
            sscanf(p + data_offset + 2 * (i - 1), "%x", &val);
            pMsg->data[ i - 1 ] = val;
        }
    }

    *(p + data_offset + 2 * pMsg->sizeData) = save;

    // If timestamp is actve - fetch it
    if (0x0d != *(p + data_offset + 2 * pMsg->sizeData)) {
        p[ data_offset + 2 * (pMsg->sizeData) + 4 ] = 0;
        sscanf((p + data_offset + 2 * (pMsg->sizeData)), "%x", &val);
        pMsg->timestamp = val * 1000; // microseconds 
    } else {
        pMsg->timestamp = 0;
    }

    return rv;
}
