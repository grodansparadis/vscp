// can232drv.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
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
//
// Linux
// =====
// device1 = comport;baudrate;mask;filter;bus-speed;btr0;btr1
//
// WIN32
// =====
// device1 = comport;baudrate;mask;filter;bus-speed;btr0;btr1

#include "vscpl1drv-can232.h"
#include "stdio.h"
#include "stdlib.h"

void _init () __attribute__ ((constructor));
void _fini () __attribute__ ((destructor));

void
_init ()
{
  printf ("initializing\n");
}

void
_fini ()
{
  printf ("finishing\n");
}

////////////////////////////////////////////////////////////////////////////
// CCan232drvdllApp

////////////////////////////////////////////////////////////////////////////
// CCan232drvdllApp construction

CCan232drvdllApp::CCan232drvdllApp ()
{
  m_instanceCounter = 0;
  pthread_mutex_init (&m_objMutex, NULL);
  // Init the driver array
  for (int i = 0; i < CANAL_LOGGER_DRIVER_MAX_OPEN; i++)
    {
      m_logArray[ i ] = NULL;
    }

  UNLOCK_MUTEX (m_objMutex);
}

CCan232drvdllApp::~CCan232drvdllApp ()
{
  LOCK_MUTEX (m_objMutex);

  for (int i = 0; i < CANAL_LOGGER_DRIVER_MAX_OPEN; i++)
    {

      if (NULL != m_logArray[ i ])
        {
          CCAN232Obj *pCAN232Obj = getDriverObject (i);

          if (NULL != pCAN232Obj)
            {
              pCAN232Obj->close ();
              delete m_logArray[ i ];
              m_logArray[ i ] = NULL;
            }
        }
    }

  UNLOCK_MUTEX (m_objMutex);
  pthread_mutex_destroy (&m_objMutex);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCan232drvdllApp object

CCan232drvdllApp theApp;




///////////////////////////////////////////////////////////////////////////////
// CreateObject

//extern "C" CCan232drvdllApp* CreateObject( void ) {
//	CCan232drvdllApp *theapp = new CCan232drvdllApp;
//	return ( ( CCan232drvdllApp * ) theapp );
//}

///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

//long CCan232drvdllApp::addDriverObject( CLog *plog )

long
CCan232drvdllApp::addDriverObject (CCAN232Obj *plog)
{
  long h = 0;

  LOCK_MUTEX (m_objMutex);
  for (int i = 0; i < CANAL_LOGGER_DRIVER_MAX_OPEN; i++)
    {

      if (NULL == m_logArray[ i ])
        {

          m_logArray[ i ] = plog;
          h = i + 1681;
          break;
        }
    }
  UNLOCK_MUTEX (m_objMutex);

  return h;
}


///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

//CLog * CCan232drvdllApp::getDriverObject( long h )

CCAN232Obj *
CCan232drvdllApp::getDriverObject (long h)
{
  long idx = h - 1681;

  // Check if valid handle
  if (idx < 0) return NULL;
  if (idx >= CANAL_LOGGER_DRIVER_MAX_OPEN) return NULL;
  return m_logArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void
CCan232drvdllApp::removeDriverObject (long h)
{
  long idx = h - 1681;

  // Check if valid handle
  if (idx < 0) return;
  if (idx >= CANAL_LOGGER_DRIVER_MAX_OPEN) return;

  LOCK_MUTEX (m_objMutex);
  if (NULL != m_logArray[ idx ]) delete m_logArray[ idx ];
  m_logArray[ idx ] = NULL;
  UNLOCK_MUTEX (m_objMutex);
}

///////////////////////////////////////////////////////////////////////////////
// InitInstance

BOOL
CCan232drvdllApp::InitInstance ()
{
  m_instanceCounter++;
  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//                             C A N A L -  A P I
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CanalOpen
//

extern "C" long
CanalOpen (const char *pDevice, unsigned long flags)
{
  long h = 0;

  CCAN232Obj *pdrvObj = new CCAN232Obj ();
  if (NULL != pdrvObj)
    {
      if (pdrvObj->open (pDevice, flags))
        {
          if (!(h = theApp.addDriverObject (pdrvObj)))
            {
              delete pdrvObj;
            }
        }
      else
        {
          delete pdrvObj;
        }
    }

  return h;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalClose
// 

extern "C" int
CanalClose (long handle)
{
  int rv = 0;

  CCAN232Obj *pLog = theApp.getDriverObject (handle);
  if (NULL == pLog) return 0;

  pLog->close ();
  theApp.removeDriverObject (handle);

  rv = 1;
  return CANAL_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalGetLevel
// 

extern "C" unsigned long
CanalGetLevel (long handle)
{
  return CANAL_LEVEL_STANDARD;
}


///////////////////////////////////////////////////////////////////////////////
// CanalSend
//

extern "C" int
CanalSend (long handle, PCANALMSG pCanalMsg)
{
  CCAN232Obj *pdrvObj = theApp.getDriverObject (handle);

  if (NULL == pdrvObj) return 0;
  return ( pdrvObj->writeMsg (pCanalMsg) ? CANAL_ERROR_SUCCESS : CANAL_ERROR_GENERIC);
}

///////////////////////////////////////////////////////////////////////////////
// CanalReceive
//

extern "C" int
CanalReceive (long handle, PCANALMSG pCanalMsg)
{
  CCAN232Obj *pdrvObj = theApp.getDriverObject (handle);

  if (NULL == pdrvObj) return 0;
  return ( pdrvObj->readMsg (pCanalMsg) ? CANAL_ERROR_SUCCESS : CANAL_ERROR_GENERIC);
}

///////////////////////////////////////////////////////////////////////////////
// CanalDataAvailable
//

extern "C" int
CanalDataAvailable (long handle)
{
  CCAN232Obj *pdrvObj = theApp.getDriverObject (handle);

  if (NULL == pdrvObj) return 0;
  return pdrvObj->dataAvailable ();
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatus
//

extern "C" int
CanalGetStatus (long handle, PCANALSTATUS pCanalStatus)
{
  CCAN232Obj *pdrvObj = theApp.getDriverObject (handle);

  if (NULL == pdrvObj) return 0;
  return ( pdrvObj->getStatus (pCanalStatus) ? CANAL_ERROR_SUCCESS : CANAL_ERROR_GENERIC);

}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatistics
//

extern "C" int
CanalGetStatistics (long handle, PCANALSTATISTICS pCanalStatistics)
{
  CCAN232Obj *pdrvObj = theApp.getDriverObject (handle);

  if (NULL == pdrvObj) return 0;
  return ( pdrvObj->getStatistics (pCanalStatistics) ? CANAL_ERROR_SUCCESS : CANAL_ERROR_GENERIC);

}

///////////////////////////////////////////////////////////////////////////////
// CanalSetFilter
//

extern "C" int
CanalSetFilter (long handle, unsigned long filter)
{
  CCAN232Obj *pdrvObj = theApp.getDriverObject (handle);

  if (NULL == pdrvObj) return 0;
  return ( pdrvObj->setFilter (filter) ? CANAL_ERROR_SUCCESS : CANAL_ERROR_GENERIC);
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetMask
//

extern "C" int
CanalSetMask (long handle, unsigned long mask)
{
  CCAN232Obj *pdrvObj = theApp.getDriverObject (handle);

  if (NULL == pdrvObj) return 0;
  return ( pdrvObj->setMask (mask) ? CANAL_ERROR_SUCCESS : CANAL_ERROR_GENERIC);
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetBaudrate
//

extern "C" int
CanalSetBaudrate (long handle, unsigned long baudrate)
{
  // Not supported in this DLL
  return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVersion
//

extern "C" unsigned long
CanalGetVersion (void)
{
  unsigned long version;
  unsigned char *p = (unsigned char *) &version;

  *p = CANAL_MAIN_VERSION;
  *(p + 1) = CANAL_MINOR_VERSION;
  *(p + 2) = CANAL_SUB_VERSION;
  *(p + 3) = 0;
  return version;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetDllVersion
//

extern "C" unsigned long
CanalGetDllVersion (void)
{
  return DLL_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVendorString
//

extern "C" const char *
CanalGetVendorString (void)
{
  return CANAL_DLL_VENDOR;
}



