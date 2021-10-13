/*  
Module : NTSERVSCMSERVICE.CPP
Purpose: Implementation for the class CNTScmService
Created: PJN / 14-07-1997
History: PJN / 09-01-2006 1. When registering the service, the quoting of the path name to use for the service 
                          is now done in CNTService::ProcessShellCommand instead of CNTScmService::Create.
         PJN / 18-05-2006 1. Minor update to rename the local variable of type _NTSERV_SCMSERVICE_DATA which is 
                          used to hold function pointers. Thanks to Frederic Metrich for reporting this issue.
         PJN / 25-06-2006 1. Combined the functionality of the _NTSERV_SCMSERVICE_DATA class into the 
                          main CNTScmService class.
                          2. Code now uses newer C++ style casts instead of C style casts.
                          3. Made the item data parameter to CNTScmService::EnumDependents a void* instead of 
                          the current DWORD.
         PJN / 01-08-2010 1. Added support for NotifyServiceStatusChange
                          2. Added support for ControlServiceEx
                          3. Added support for SERVICE_CONFIG_DELAYED_AUTO_START_INFO
                          4. Added support for SERVICE_CONFIG_FAILURE_ACTIONS_FLAG
                          5. Added support for SERVICE_CONFIG_SERVICE_SID_INFO
                          6. Added support for SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO
                          7. Added support for SERVICE_CONFIG_PRESHUTDOWN_INFO
                          8. Added support for SERVICE_CONFIG_TRIGGER_INFO
                          9. Added support for SERVICE_CONFIG_PREFERRED_NODE
         PJN / 10-11-2012 1. Reworked CNTScmService::WaitForServiceStatus method to avoid the need for calling
                          GetTickCount

Copyright (c) 1996 - 2014 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////////////////  Includes  //////////////////////////////////

#include <pch.h>
#include "ntservDefines.h"
#include "ntservScmService.h"
#include "ntservServiceControlManager.h"


////////////////////////////////// Macros / Defines ////////////////////////////

#ifdef CNTSERVICE_MFC_EXTENSIONS
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#endif


///////////////////////////////// Implementation //////////////////////////////

CNTScmService::CNTScmService() : m_hService(NULL),
                                 m_lpfnQueryServiceConfig2(NULL),
                                 m_lpfnChangeServiceConfig2(NULL),
                                 m_lpfnQueryServiceStatusEx(NULL),
                                 m_lpfnNotifyServiceStatusChange(NULL),
                                 m_lpfnControlServiceEx(NULL)
{
  m_hAdvapi32 = GetModuleHandle(_T("ADVAPI32.DLL"));
  if (m_hAdvapi32 != NULL)
  {
  #ifdef _UNICODE
    m_lpfnQueryServiceConfig2 = reinterpret_cast<LPQUERYSERVICECONFIG2>(GetProcAddress(m_hAdvapi32, "QueryServiceConfig2W"));
    m_lpfnChangeServiceConfig2 = reinterpret_cast<LPCHANGESERVICECONFIG2>(GetProcAddress(m_hAdvapi32, "ChangeServiceConfig2W"));
    m_lpfnNotifyServiceStatusChange = reinterpret_cast<LPNOTIFYSERVICESTATUSCHANGE>(GetProcAddress(m_hAdvapi32, "NotifyServiceStatusChangeW"));
    m_lpfnControlServiceEx = reinterpret_cast<LPCONTROLSERVICEEX>(GetProcAddress(m_hAdvapi32, "ControlServiceExW"));
  #else
    m_lpfnQueryServiceConfig2 = reinterpret_cast<LPQUERYSERVICECONFIG2>(GetProcAddress(m_hAdvapi32, "QueryServiceConfig2A"));
    m_lpfnChangeServiceConfig2 = reinterpret_cast<LPCHANGESERVICECONFIG2>(GetProcAddress(m_hAdvapi32, "ChangeServiceConfig2A"));
    m_lpfnNotifyServiceStatusChange = reinterpret_cast<LPNOTIFYSERVICESTATUSCHANGE>(GetProcAddress(m_hAdvapi32, "NotifyServiceStatusChangeA"));
    m_lpfnControlServiceEx = reinterpret_cast<LPCONTROLSERVICEEX>(GetProcAddress(m_hAdvapi32, "ControlServiceExA"));
  #endif
    m_lpfnQueryServiceStatusEx = reinterpret_cast<LPQUERYSERVICESTATUSEX>(GetProcAddress(m_hAdvapi32, "QueryServiceStatusEx"));
  }
}

CNTScmService::~CNTScmService()
{
  Close();
}

void CNTScmService::Close()
{
  if (m_hService)
  {
    CloseServiceHandle(m_hService);
    m_hService = NULL;
  }
}

CNTScmService::operator SC_HANDLE() const
{
  return m_hService;
}

BOOL CNTScmService::Attach(SC_HANDLE hService)
{
  if (m_hService != hService)
    Close();

  m_hService = hService;
  return TRUE;
}

SC_HANDLE CNTScmService::Detach()
{
  SC_HANDLE hReturn = m_hService;
  m_hService = NULL;
  return hReturn;
}

BOOL CNTScmService::ChangeConfig(DWORD dwServiceType,	DWORD dwStartType,
                                 DWORD dwErrorControl, LPCTSTR lpBinaryPathName,
                                 LPCTSTR lpLoadOrderGroup, LPDWORD lpdwTagId,
                                 LPCTSTR lpDependencies, LPCTSTR lpServiceStartName,
                                 LPCTSTR lpPassword, LPCTSTR lpDisplayName) const
{
  //Validate our parameters
  ATLASSUME(m_hService != NULL);
  
  return ChangeServiceConfig(m_hService, dwServiceType, dwStartType,
                             dwErrorControl, lpBinaryPathName, lpLoadOrderGroup, lpdwTagId,
                             lpDependencies, lpServiceStartName, lpPassword, lpDisplayName);
}

BOOL CNTScmService::Control(DWORD dwControl)
{
  //Validate our parameters
  ATLASSUME(m_hService != NULL);
  
  SERVICE_STATUS ServiceStatus;
  return ControlService(m_hService, dwControl, &ServiceStatus);
}

BOOL CNTScmService::Stop() const
{
  //Validate our parameters
  ATLASSUME(m_hService != NULL);
  
  SERVICE_STATUS ServiceStatus;
  return ControlService(m_hService, SERVICE_CONTROL_STOP, &ServiceStatus);
}

BOOL CNTScmService::WaitForStop(DWORD dwTimeout)
{
  return WaitForServiceStatus(dwTimeout, SERVICE_STOPPED);
}

BOOL CNTScmService::WaitForServiceStatus(DWORD dwTimeout, DWORD dwWaitForStatus, DWORD dwPollingInterval)
{
  //wait for our desired status
  DWORD dwTimeWaited = 0;
  SERVICE_STATUS ss;
  for (;;)
  {
    if (!QueryStatus(ss))
      break;

    //if this is the status we were waiting for then we're done
    if (ss.dwCurrentState == dwWaitForStatus)
      return TRUE;

    //Do the wait
    Sleep(dwPollingInterval);

    //check for timeout
    dwTimeWaited += dwPollingInterval;
    if ((dwTimeout != INFINITE) && (dwTimeWaited > dwTimeout))
    {
      SetLastError(ERROR_TIMEOUT);
      break;
    }
  }
  return FALSE;
}

BOOL CNTScmService::Pause() const
{
  //Validate our parameters
  ATLASSUME(m_hService != NULL);
  
  SERVICE_STATUS ServiceStatus;
  return ControlService(m_hService, SERVICE_CONTROL_PAUSE, &ServiceStatus);
}

BOOL CNTScmService::Continue() const
{
  //Validate our parameters
  ATLASSUME(m_hService != NULL);
  
  SERVICE_STATUS ServiceStatus;
  return ControlService(m_hService, SERVICE_CONTROL_CONTINUE, &ServiceStatus);
}

BOOL CNTScmService::Interrogate() const
{
  //Validate our parameters
  ATLASSUME(m_hService != NULL);
  
  SERVICE_STATUS ServiceStatus;
  return ControlService(m_hService, SERVICE_CONTROL_INTERROGATE, &ServiceStatus);
}

BOOL CNTScmService::Start(DWORD dwNumServiceArgs,	LPCTSTR* lpServiceArgVectors) const
{
  //Validate our parameters
  ATLASSUME(m_hService != NULL);
  
  return StartService(m_hService, dwNumServiceArgs, lpServiceArgVectors);
}

BOOL CNTScmService::AcceptStop(BOOL& bStop)
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);
  
  SERVICE_STATUS ServiceStatus;
  BOOL bSuccess = QueryStatus(ServiceStatus);
  if (bSuccess)
    bStop = ((ServiceStatus.dwControlsAccepted & SERVICE_ACCEPT_STOP) != 0);

  return bSuccess;
}

BOOL CNTScmService::AcceptPauseContinue(BOOL& bPauseContinue)
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);
  
  SERVICE_STATUS ServiceStatus;
  BOOL bSuccess = QueryStatus(ServiceStatus);
  if (bSuccess)
    bPauseContinue = ((ServiceStatus.dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE) != 0);

  return bSuccess;
}

BOOL CNTScmService::AcceptShutdown(BOOL& bShutdown)
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);
  
  SERVICE_STATUS ServiceStatus;
  BOOL bSuccess = QueryStatus(ServiceStatus);
  if (bSuccess)
    bShutdown = ((ServiceStatus.dwControlsAccepted & SERVICE_ACCEPT_SHUTDOWN) != 0);

  return bSuccess;
}

BOOL CNTScmService::QueryStatus(SERVICE_STATUS& ServiceStatus) const
{
  //Validate our parameters
  ATLASSUME(m_hService != NULL);
  
  return QueryServiceStatus(m_hService, &ServiceStatus);
}

BOOL CNTScmService::QueryStatus(SERVICE_STATUS_PROCESS& ssp) const
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnQueryServiceStatusEx == NULL)
  {
    ATLTRACE(_T("CNTScmService::QueryStatus, QueryServiceStatusEx function is not supported on this OS. You need to be running at least Windows 2000 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  DWORD dwBytesNeeded;
  return m_lpfnQueryServiceStatusEx(m_hService, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&ssp), sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);
}

BOOL CNTScmService::QueryConfig(LPQUERY_SERVICE_CONFIG& lpServiceConfig) const
{
  //Validate our parameters
  ATLASSUME(m_hService != NULL);
  ATLASSERT(lpServiceConfig == NULL); //To prevent double overwrites, this function
                                      //asserts if you do not send in a NULL pointer

  DWORD dwBytesNeeded;
  BOOL bSuccess = QueryServiceConfig(m_hService, NULL, 0, &dwBytesNeeded);
  if (!bSuccess && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    lpServiceConfig = reinterpret_cast<LPQUERY_SERVICE_CONFIG>(new BYTE[dwBytesNeeded]);
    DWORD dwSize;
    bSuccess = QueryServiceConfig(m_hService, lpServiceConfig, dwBytesNeeded, &dwSize);
  }
  return bSuccess;
}

BOOL CNTScmService::Create(CNTServiceControlManager& Manager, LPCTSTR lpServiceName, LPCTSTR lpDisplayName,	DWORD dwDesiredAccess,	DWORD dwServiceType, DWORD dwStartType,	DWORD dwErrorControl,	    
                           LPCTSTR lpBinaryPathName, LPCTSTR lpLoadOrderGroup, LPDWORD lpdwTagId,	LPCTSTR lpDependencies, LPCTSTR lpServiceStartName, LPCTSTR lpPassword)
{
  Close();

  m_hService = CreateService(Manager, lpServiceName, lpDisplayName,	dwDesiredAccess, dwServiceType, dwStartType,	dwErrorControl,	lpBinaryPathName, lpLoadOrderGroup, 
                             lpdwTagId,	lpDependencies, lpServiceStartName, lpPassword);
  return (m_hService != NULL);
}

BOOL CNTScmService::Delete() const
{
  //Validate our parameters
  ATLASSUME(m_hService != NULL);
  
  return DeleteService(m_hService);
}

BOOL CNTScmService::SetObjectSecurity(SECURITY_INFORMATION dwSecurityInformation,
                                      PSECURITY_DESCRIPTOR lpSecurityDescriptor) const

{
  //Validate our parameters
  ATLASSUME(m_hService != NULL);
  
  return SetServiceObjectSecurity(m_hService, dwSecurityInformation, lpSecurityDescriptor);
}

BOOL CNTScmService::QueryObjectSecurity(SECURITY_INFORMATION dwSecurityInformation,
                                        PSECURITY_DESCRIPTOR& lpSecurityDescriptor) const
{
  //Validate our parameters
  ATLASSUME(m_hService != NULL);
  ATLASSERT(lpSecurityDescriptor == NULL); //To prevent double overwrites, this function
                                        //asserts if you do not send in a NULL pointer

  DWORD dwBytesNeeded;
  BOOL bSuccess = QueryServiceObjectSecurity(m_hService, dwSecurityInformation, NULL, 0, &dwBytesNeeded);
  if (!bSuccess && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    lpSecurityDescriptor = static_cast<PSECURITY_DESCRIPTOR>(new BYTE[dwBytesNeeded]);
    DWORD dwSize;
    bSuccess = QueryServiceObjectSecurity(m_hService, dwSecurityInformation, lpSecurityDescriptor, dwBytesNeeded, &dwSize);
  }

  return bSuccess;
}

BOOL CNTScmService::EnumDependents(DWORD dwServiceState, void* pUserData, ENUM_SERVICES_PROC lpEnumServicesFunc) const
{
  //Validate our parameters
  ATLASSUME(m_hService != NULL);

  DWORD dwBytesNeeded;
  DWORD dwServices;
  BOOL bSuccess = EnumDependentServices(m_hService, dwServiceState, NULL, 0, &dwBytesNeeded, &dwServices);
  DWORD dwLastError = GetLastError();
  if (!bSuccess && (dwLastError == ERROR_MORE_DATA) || (dwLastError == ERROR_INSUFFICIENT_BUFFER)) //Note we use ERROR_INSUFFICIENT_BUFFER here even though it is not documented as a legal return value from EnumDependentServices here
  {
    //Allocate some memory for the API
    ATL::CHeapPtr<ENUM_SERVICE_STATUS> lpServices;
    if (!lpServices.AllocateBytes(dwBytesNeeded))
    {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }

    DWORD dwSize;
    bSuccess = EnumDependentServices(m_hService, dwServiceState, lpServices.m_pData, dwBytesNeeded, &dwSize, &dwServices);
    if (bSuccess)
    {
      BOOL bContinue = TRUE;
      for (DWORD i=0; i<dwServices && bContinue; i++)
        bContinue = lpEnumServicesFunc(pUserData, lpServices[i]);
    }
  }
  return bSuccess;
}

BOOL CNTScmService::ChangeDescription(const CNTServiceString& sDescription)
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnChangeServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::ChangeDescription, ChangeServiceConfig2 function is not supported on this OS. You need to be running at least Windows 2000 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  SERVICE_DESCRIPTION sd;
  CNTServiceString sTemp(sDescription);
#ifdef CNTSERVICE_MFC_EXTENSIONS
  sd.lpDescription = sTemp.GetBuffer(sTemp.GetLength());
#else
  sd.lpDescription = &(sTemp[0]);
#endif
  BOOL bSuccess = m_lpfnChangeServiceConfig2(m_hService, SERVICE_CONFIG_DESCRIPTION, &sd);
#ifdef CNTSERVICE_MFC_EXTENSIONS
  sTemp.ReleaseBuffer();
#endif
  return bSuccess;
}

BOOL CNTScmService::QueryDescription(CNTServiceString& sDescription) const
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnQueryServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::QueryDescription, QueryServiceConfig2 function is not supported on this OS. You need to be running at least Windows 2000 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  DWORD dwBytesNeeded = 0;
  BOOL bSuccess = m_lpfnQueryServiceConfig2(m_hService, SERVICE_CONFIG_DESCRIPTION, NULL, 0, &dwBytesNeeded);
  if (!bSuccess && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    //Allocate some memory for the API
    ATL::CHeapPtr<BYTE> lpData;
    if (!lpData.Allocate(dwBytesNeeded))
    {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }

    bSuccess = m_lpfnQueryServiceConfig2(m_hService, SERVICE_CONFIG_DESCRIPTION, lpData, dwBytesNeeded, &dwBytesNeeded);
    if (bSuccess)
    {
      LPCTSTR pszDescription = reinterpret_cast<LPSERVICE_DESCRIPTION>(lpData.m_pData)->lpDescription;
    #ifdef CNTSERVICE_MFC_EXTENSIONS
      sDescription = pszDescription;
    #else
      if (pszDescription != NULL)
        sDescription = pszDescription;
      else
        sDescription.clear();
    #endif
    }
  }

  return bSuccess;
}

BOOL CNTScmService::ChangeFailureActions(LPSERVICE_FAILURE_ACTIONS pFailureActions)
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnChangeServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::ChangeFailureActions, ChangeServiceConfig2 function is not supported on this OS. You need to be running at least Windows 2000 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  return m_lpfnChangeServiceConfig2(m_hService, SERVICE_CONFIG_FAILURE_ACTIONS, pFailureActions);
}

BOOL CNTScmService::QueryFailureActions(LPSERVICE_FAILURE_ACTIONS& pActions) const
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);
  ATLASSERT(pActions == NULL); //To prevent double overwrites, this function
                            //asserts if you do not send in a NULL pointer

  //Check to see if the function pointer is available
  if (m_lpfnQueryServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::QueryFailureActions, QueryServiceConfig2 function is not supported on this OS. You need to be running at least Windows 2000 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  DWORD dwBytesNeeded = 0;
  BOOL bSuccess = m_lpfnQueryServiceConfig2(m_hService, SERVICE_CONFIG_FAILURE_ACTIONS, NULL, 0, &dwBytesNeeded);
  if (!bSuccess && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    pActions = reinterpret_cast<LPSERVICE_FAILURE_ACTIONS>(new BYTE[dwBytesNeeded]);
    bSuccess = m_lpfnQueryServiceConfig2(m_hService, SERVICE_CONFIG_FAILURE_ACTIONS, reinterpret_cast<LPBYTE>(pActions), dwBytesNeeded, &dwBytesNeeded);
  }

  return bSuccess;
}

BOOL CNTScmService::ChangeDelayAutoStart(BOOL bDelayedAutoStart)
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnChangeServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::ChangeDelayAutoStart, ChangeServiceConfig2 function is not supported on this OS. You need to be running at least Windows Vista / Windows 2008 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  SERVICE_DELAYED_AUTO_START_INFO sdasi;
  sdasi.fDelayedAutostart = bDelayedAutoStart;
  return m_lpfnChangeServiceConfig2(m_hService, SERVICE_CONFIG_DELAYED_AUTO_START_INFO, &sdasi);
}

BOOL CNTScmService::QueryDelayAutoStart(BOOL& bDelayedAutoStart) const
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnQueryServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::QueryDelayAutoStart, QueryServiceConfig2 function is not supported on this OS. You need to be running at least Windows Vista / Windows 2008 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  SERVICE_DELAYED_AUTO_START_INFO sdasi;
  DWORD dwBytesNeeded = 0;
  BOOL bSuccess = m_lpfnQueryServiceConfig2(m_hService, SERVICE_CONFIG_DELAYED_AUTO_START_INFO, reinterpret_cast<LPBYTE>(&sdasi), sizeof(sdasi), &dwBytesNeeded);
  if (bSuccess)
    bDelayedAutoStart = sdasi.fDelayedAutostart;

  return bSuccess;
}

BOOL CNTScmService::ChangeFailureActionsFlag(BOOL bFailureActionsOnNonCrashFailures)
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnChangeServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::ChangeFailureActionsFlag, ChangeServiceConfig2 function is not supported on this OS. You need to be running at least Windows Vista / Windows 2008 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  SERVICE_FAILURE_ACTIONS_FLAG sfaf;
  sfaf.fFailureActionsOnNonCrashFailures = bFailureActionsOnNonCrashFailures;
  return m_lpfnChangeServiceConfig2(m_hService, SERVICE_CONFIG_FAILURE_ACTIONS_FLAG, &sfaf);
}

BOOL CNTScmService::QueryFailureActionsFlag(BOOL& bFailureActionsOnNonCrashFailures) const
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnQueryServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::QueryFailureActionsFlag, QueryServiceConfig2 function is not supported on this OS. You need to be running at least Windows Vista / Windows 2008 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  SERVICE_FAILURE_ACTIONS_FLAG sfaf;
  DWORD dwBytesNeeded = 0;
  BOOL bSuccess = m_lpfnQueryServiceConfig2(m_hService, SERVICE_CONFIG_FAILURE_ACTIONS_FLAG, reinterpret_cast<LPBYTE>(&sfaf), sizeof(sfaf), &dwBytesNeeded);
  if (bSuccess)
    bFailureActionsOnNonCrashFailures = sfaf.fFailureActionsOnNonCrashFailures;

  return bSuccess;
}

BOOL CNTScmService::ChangeSidInfo(DWORD dwServiceSidType)
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnChangeServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::ChangeSidInfo, ChangeServiceConfig2 function is not supported on this OS. You need to be running at least Windows Vista / Windows 2008 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  SERVICE_SID_INFO ssi;
  ssi.dwServiceSidType = dwServiceSidType;
  return m_lpfnChangeServiceConfig2(m_hService, SERVICE_CONFIG_SERVICE_SID_INFO, &ssi);
}

BOOL CNTScmService::QuerySidInfo(DWORD& dwServiceSidType) const
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnQueryServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::QuerySidInfo, QueryServiceConfig2 function is not supported on this OS. You need to be running at least Windows Vista / Windows 2008 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  SERVICE_SID_INFO ssi;
  DWORD dwBytesNeeded = 0;
  BOOL bSuccess = m_lpfnQueryServiceConfig2(m_hService, SERVICE_CONFIG_SERVICE_SID_INFO, reinterpret_cast<LPBYTE>(&ssi), sizeof(ssi), &dwBytesNeeded);
  if (bSuccess)
    dwServiceSidType = ssi.dwServiceSidType;

  return bSuccess;
}

BOOL CNTScmService::ChangeRequiredPrivileges(const CNTServiceStringArray& privileges)
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnChangeServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::ChangeRequiredPrivileges, ChangeServiceConfig2 function is not supported on this OS. You need to be running at least Windows Vista / Windows 2008 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Work out the size of the buffer we will need
#ifdef CNTSERVICE_MFC_EXTENSIONS
  int nSize = 0;
  INT_PTR nStrings = privileges.GetSize();
  for (INT_PTR i=0; i<nStrings; i++)
    nSize += privileges.GetAt(i).GetLength() + 1; //1 extra for each NULL terminator
#else
  CNTServiceStringArray::size_type nSize = 0;
  CNTServiceStringArray::size_type nStrings = privileges.size();
  for (CNTServiceStringArray::size_type i=0; i<nStrings; i++)
    nSize += privileges[i].length() + 1; //1 extra for each NULL terminator
#endif

  //Need one second NULL for the double NULL at the end
  nSize++;

  //Allocate some memory for the API
  ATL::CHeapPtr<TCHAR> lpBuffer;
  if (!lpBuffer.Allocate(nSize))
  {
    SetLastError(ERROR_OUTOFMEMORY);
    return FALSE;
  }

  //Now copy the strings into the buffer
  LPTSTR lpszString = lpBuffer.m_pData;
#ifdef CNTSERVICE_MFC_EXTENSIONS
  int nCurOffset = 0;
  for (INT_PTR i=0; i<nStrings; i++)
  {
    const CNTServiceString& sText = privileges.GetAt(i);
    int nCurrentStringLength = sText.GetLength();
    _tcscpy_s(&lpszString[nCurOffset], nCurrentStringLength+1, sText);
    nCurOffset += (nCurrentStringLength + 1);
  }
#else
  CNTServiceStringArray::size_type nCurOffset = 0;
  for (CNTServiceStringArray::size_type i=0; i<nStrings; i++)
  {
    const CNTServiceString& sText = privileges[i];
    CNTServiceString::size_type nCurrentStringLength = sText.length();
    _tcscpy_s(&lpszString[nCurOffset], nCurrentStringLength+1, sText.c_str());
    nCurOffset += (nCurrentStringLength + 1);
  }
#endif

  //Don't forgot to doubly NULL terminate
  lpszString[nCurOffset] = _T('\0');

  //Call through the function pointer
  SERVICE_REQUIRED_PRIVILEGES_INFO srpi;
  srpi.pmszRequiredPrivileges = lpBuffer.m_pData;
  return m_lpfnChangeServiceConfig2(m_hService, SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO, &srpi);
}

BOOL CNTScmService::QueryRequiredPrivileges(CNTServiceStringArray& privileges) const
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Empty our the array
#ifdef CNTSERVICE_MFC_EXTENSIONS
  privileges.RemoveAll();
#else
  privileges.clear();
#endif

  //Check to see if the function pointer is available
  if (m_lpfnQueryServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::QueryRequiredPrivileges, QueryServiceConfig2 function is not supported on this OS. You need to be running at least Windows Vista / Windows 2008 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  DWORD dwBytesNeeded = 0;
  BOOL bSuccess = m_lpfnQueryServiceConfig2(m_hService, SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO, NULL, 0, &dwBytesNeeded);
  if (!bSuccess && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    //Allocate some memory for the API
    ATL::CHeapPtr<BYTE> lpBuffer;
    if (!lpBuffer.Allocate(dwBytesNeeded))
    {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }

    bSuccess = m_lpfnQueryServiceConfig2(m_hService, SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO, lpBuffer, dwBytesNeeded, &dwBytesNeeded);
    if (bSuccess)
    {
      LPTSTR lpszStrings = reinterpret_cast<LPTSTR>(lpBuffer.m_pData);
      while (lpszStrings[0] != 0)
      {
      #ifdef CNTSERVICE_MFC_EXTENSIONS
        privileges.Add(lpszStrings);
      #else
        privileges.push_back(lpszStrings);
      #endif
        lpszStrings += (_tcslen(lpszStrings ) + 1);
      }
    }
  }

  return bSuccess;
}

BOOL CNTScmService::ChangePreShutdown(DWORD dwPreshutdownTimeout)
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnChangeServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::ChangePreShutdown, ChangeServiceConfig2 function is not supported on this OS. You need to be running at least Windows Vista / Windows 2008 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  SERVICE_PRESHUTDOWN_INFO spsi;
  spsi.dwPreshutdownTimeout = dwPreshutdownTimeout;
  return m_lpfnChangeServiceConfig2(m_hService, SERVICE_CONFIG_PRESHUTDOWN_INFO, &spsi);
}

BOOL CNTScmService::QueryPreShutdown(DWORD& dwPreshutdownTimeout) const
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnQueryServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::QueryPreShutdown, QueryServiceConfig2 function is not supported on this OS. You need to be running at least Windows Vista / Windows 2008 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  SERVICE_PRESHUTDOWN_INFO spsi;
  DWORD dwBytesNeeded = 0;
  BOOL bSuccess = m_lpfnQueryServiceConfig2(m_hService, SERVICE_CONFIG_PRESHUTDOWN_INFO, reinterpret_cast<LPBYTE>(&spsi), sizeof(spsi), &dwBytesNeeded);
  if (bSuccess)
    dwPreshutdownTimeout = spsi.dwPreshutdownTimeout;

  return bSuccess;
}

BOOL CNTScmService::ChangeTrigger(PSERVICE_TRIGGER_INFO pTriggerInfo)
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnChangeServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::ChangeTrigger, ChangeServiceConfig2 function is not supported on this OS. You need to be running at least Windows 7 / Windows 2008 R2 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  return m_lpfnChangeServiceConfig2(m_hService, SERVICE_CONFIG_TRIGGER_INFO, pTriggerInfo);
}

BOOL CNTScmService::QueryTrigger(PSERVICE_TRIGGER_INFO& pTriggerInfo) const
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);
  ATLASSERT(pTriggerInfo == NULL); //To prevent double overwrites, this function
                                   //asserts if you do not send in a NULL pointer

  //Check to see if the function pointer is available
  if (m_lpfnQueryServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::QueryTrigger, QueryServiceConfig2 function is not supported on this OS. You need to be running at least Windows 7 / Windows 2008 R2 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  DWORD dwBytesNeeded = 0;
  BOOL bSuccess = m_lpfnQueryServiceConfig2(m_hService, SERVICE_CONFIG_TRIGGER_INFO, NULL, 0, &dwBytesNeeded);
  if (!bSuccess && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    pTriggerInfo = reinterpret_cast<PSERVICE_TRIGGER_INFO>(new BYTE[dwBytesNeeded]);
    bSuccess = m_lpfnQueryServiceConfig2(m_hService, SERVICE_CONFIG_TRIGGER_INFO, reinterpret_cast<LPBYTE>(pTriggerInfo), dwBytesNeeded, &dwBytesNeeded);
  }

  return bSuccess;
}

BOOL CNTScmService::ChangePreferredNode(USHORT usPreferredNode, BOOL bDelete)
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnChangeServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::ChangePreferredNode, ChangeServiceConfig2 function is not supported on this OS. You need to be running at least Windows 7 / Windows 2008 R2 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  SERVICE_PREFERRED_NODE_INFO spni;
  spni.usPreferredNode = usPreferredNode;
  spni.fDelete = bDelete ? 1 : 0;
  return m_lpfnChangeServiceConfig2(m_hService, SERVICE_CONFIG_PREFERRED_NODE, &spni);
}

BOOL CNTScmService::QueryPreferredNode(USHORT& usPreferredNode, BOOL& bDelete) const
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnQueryServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::QueryPreferredNode, QueryServiceConfig2 function is not supported on this OS. You need to be running at least Windows 7 / Windows 2008 R2 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  SERVICE_PREFERRED_NODE_INFO spni;
  DWORD dwBytesNeeded = 0;
  BOOL bSuccess = m_lpfnQueryServiceConfig2(m_hService, SERVICE_CONFIG_PREFERRED_NODE, reinterpret_cast<LPBYTE>(&spni), sizeof(spni), &dwBytesNeeded);
  if (bSuccess)
  {
    usPreferredNode = spni.usPreferredNode;
    bDelete = spni.fDelete;
  }

  return bSuccess;
}

BOOL CNTScmService::ChangeLaunchProtected(DWORD dwLaunchProtected)
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnChangeServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::ChangeLaunchProtected, ChangeServiceConfig2 function is not supported on this OS. You need to be running at least Windows 8.1 / Windows 2012 R2 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  _SERVICE_LAUNCH_PROTECTED_INFO slpi;
  slpi.dwLaunchProtected = dwLaunchProtected;
  return m_lpfnChangeServiceConfig2(m_hService, SERVICE_CONFIG_LAUNCH_PROTECTED, &slpi);
}

BOOL CNTScmService::QueryLaunchProtected(DWORD& dwLaunchProtected) const
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnQueryServiceConfig2 == NULL)
  {
    ATLTRACE(_T("CNTScmService::QueryLaunchProtected, QueryServiceConfig2 function is not supported on this OS. You need to be running at least Windows 8.1 / Windows 2012 R2 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  _SERVICE_LAUNCH_PROTECTED_INFO slpi;
  DWORD dwBytesNeeded = 0;
  BOOL bSuccess = m_lpfnQueryServiceConfig2(m_hService, SERVICE_CONFIG_LAUNCH_PROTECTED, reinterpret_cast<LPBYTE>(&slpi), sizeof(slpi), &dwBytesNeeded);
  if (bSuccess)
    dwLaunchProtected = slpi.dwLaunchProtected;

  return bSuccess;
}

DWORD CNTScmService::NotifyStatusChange(DWORD dwNotifyMask, PSERVICE_NOTIFY pNotifyBuffer) const
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnNotifyServiceStatusChange == NULL)
  {
    ATLTRACE(_T("CNTScmService::NotifyStatusChange, NotifyServiceStatusChange function is not supported on this OS. You need to be running at least Windows Vista / Windows 2008 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  return m_lpfnNotifyServiceStatusChange(m_hService, dwNotifyMask, pNotifyBuffer);
}

DWORD CNTScmService::Control(DWORD dwControl, DWORD dwInfoLevel, PVOID pControlParams)
{
  //Validate our parameters
  ATLASSERT(m_hService != NULL);

  //Check to see if the function pointer is available
  if (m_lpfnControlServiceEx == NULL)
  {
    ATLTRACE(_T("CNTScmService::Control, ControlServiceEx function is not supported on this OS. You need to be running at least Windows Vista / Windows 2008 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  return m_lpfnControlServiceEx(m_hService, dwControl, dwInfoLevel, pControlParams);
}

