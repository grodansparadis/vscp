/*  
Module : NTSERVEVENTLOG.CPP
Purpose: Implementation for the class CNTEventLog
Created: PJN / 14-07-1997
History: PJN / 25-06-2006 1. Code now uses newer C++ style casts instead of C style casts.
         PJN / 02-02-2007 1. Optimized CNTEventLog constructor code.
         PJN / 01-08-2010 1. Added support for GetEventLogInformation

Copyright (c) 1996 - 2014 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


///////////////////////////////// Includes  ///////////////////////////////////

#include "stdafx.h"
#include "ntservEventlog.h"


///////////////////////////////// Macros //////////////////////////////////////

#ifdef CNTSERVICE_MFC_EXTENSIONS
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#endif


///////////////////////////////// Implementation //////////////////////////////

CNTEventLog::CNTEventLog() : m_hEventLog(NULL)
{
}

CNTEventLog::~CNTEventLog()
{
  Close();
}

CNTEventLog::operator HANDLE() const
{
  return m_hEventLog;
}

BOOL CNTEventLog::Attach(HANDLE hEventLog)
{
  if (m_hEventLog != hEventLog)
    Close();

  m_hEventLog = hEventLog;
  return TRUE;
}

HANDLE CNTEventLog::Detach()
{
  HANDLE hReturn = m_hEventLog;
  m_hEventLog = NULL;
  return hReturn;
}

BOOL CNTEventLog::Open(LPCTSTR lpUNCServerName, LPCTSTR lpSourceName)
{
  Close();
  m_hEventLog = OpenEventLog(lpUNCServerName, lpSourceName);
  return (m_hEventLog != NULL);
}

BOOL CNTEventLog::OpenApplication(LPCTSTR lpUNCServerName)
{
  return Open(lpUNCServerName, _T("Application"));
}

BOOL CNTEventLog::OpenSystem(LPCTSTR lpUNCServerName)
{
  return Open(lpUNCServerName, _T("System"));
}

BOOL CNTEventLog::OpenSecurity(LPCTSTR lpUNCServerName)
{
  return Open(lpUNCServerName, _T("Security"));
}

BOOL CNTEventLog::OpenBackup(LPCTSTR lpUNCServerName, LPCTSTR lpFileName)
{
  Close();
  m_hEventLog = OpenBackupEventLog(lpUNCServerName, lpFileName);
  return (m_hEventLog != NULL);
}

BOOL CNTEventLog::Close()
{
  BOOL bSuccess = TRUE;
  if (m_hEventLog != NULL)
  {
    bSuccess = CloseEventLog(m_hEventLog);
    m_hEventLog = NULL;
  }

  return bSuccess;
}

BOOL CNTEventLog::Backup(LPCTSTR lpBackupFileName) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);
  
  return BackupEventLog(m_hEventLog, lpBackupFileName);
}

BOOL CNTEventLog::Clear(LPCTSTR lpBackupFileName) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);
  
  return ClearEventLog(m_hEventLog, lpBackupFileName);
}

BOOL CNTEventLog::GetNumberOfRecords(DWORD& dwNumberOfRecords) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);
  
  return GetNumberOfEventLogRecords(m_hEventLog, &dwNumberOfRecords);
}

BOOL CNTEventLog::GetOldestRecord(DWORD& dwOldestRecord) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);
  
  return GetOldestEventLogRecord(m_hEventLog, &dwOldestRecord);
}

BOOL CNTEventLog::NotifyChange(HANDLE hEvent) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);
  
  return NotifyChangeEventLog(m_hEventLog, hEvent);
}

BOOL CNTEventLog::ReadNext(CEventLogRecord& record) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);

  DWORD dwBytesRead;
  DWORD dwBytesNeeded;
  EVENTLOGRECORD el;
  BOOL bSuccess = ReadEventLog(m_hEventLog, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ, 0, &el, sizeof(EVENTLOGRECORD), &dwBytesRead, &dwBytesNeeded);
  if (bSuccess)
    record = CEventLogRecord(&el);
  else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    //Allocate some memory for the API
    ATL::CHeapPtr<BYTE> lpBuffer;
    if (!lpBuffer.Allocate(dwBytesNeeded))
    {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }

    bSuccess = ReadEventLog(m_hEventLog, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ, 0, lpBuffer, dwBytesNeeded, &dwBytesRead, &dwBytesNeeded);
    if (bSuccess)
      record = CEventLogRecord(reinterpret_cast<EVENTLOGRECORD*>(lpBuffer.m_pData));
  }

  return bSuccess;
}

BOOL CNTEventLog::ReadPrev(CEventLogRecord& record) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);

  DWORD dwBytesRead;
  DWORD dwBytesNeeded;
  EVENTLOGRECORD el;
  BOOL bSuccess = ReadEventLog(m_hEventLog, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ, 0, &el, sizeof(EVENTLOGRECORD), &dwBytesRead, &dwBytesNeeded);
  if (bSuccess)
    record = CEventLogRecord(&el);
  else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    //Allocate some memory for the API
    ATL::CHeapPtr<BYTE> lpBuffer;
    if (!lpBuffer.Allocate(dwBytesNeeded))
    {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }
  
    bSuccess = ReadEventLog(m_hEventLog, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ, 0, lpBuffer, dwBytesNeeded, &dwBytesRead, &dwBytesNeeded);
    if (bSuccess)
      record = CEventLogRecord(reinterpret_cast<EVENTLOGRECORD*>(lpBuffer.m_pData));
  }

  return bSuccess;
}

BOOL CNTEventLog::GetFullInformation(DWORD& dwFull) const
{
  //Validate our parameters
  ATLASSUME(m_hEventLog != NULL);

  //Call through the function pointer
  EVENTLOG_FULL_INFORMATION efi;
  DWORD dwBytesNeeded = 0;
  BOOL bSuccess = GetEventLogInformation(m_hEventLog, EVENTLOG_FULL_INFO, &efi, sizeof(efi), &dwBytesNeeded);
  if (bSuccess)
    dwFull = efi.dwFull;

  return bSuccess;
}
