/*
Module : NTSERVSERVICECONTROLMANAGER.H
Purpose: Defines the interface for the class CNTServiceControlManager. 
Created: PJN / 14-07-1998

Copyright (c) 1997 - 2014 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


////////////////////////////// Macros / Defines ////////////////////////////////

#pragma once

#ifndef __NTSERVSERVICECONTROLMANAGER_H__
#define __NTSERVSERVICECONTROLMANAGER_H__


////////////////////////////// Includes ///////////////////////////////////////

#include "ntservDefines.h"
#include "ntservScmService.h"


////////////////////////////// Classes ////////////////////////////////////////

//An encapsulation of the NT Service Control Manager
class CNTSERVICE_EXT_CLASS CNTServiceControlManager
{
public:
//typedefs
  typedef BOOL (CALLBACK* ENUM_SERVICES_PROC)(void* pData, ENUM_SERVICE_STATUS& ss);
  typedef BOOL (CALLBACK* ENUM_SERVICES_PROC2)(void* pData, ENUM_SERVICE_STATUS_PROCESS& ssp);

//Constructors / Destructors
  CNTServiceControlManager();
  ~CNTServiceControlManager();

//Methods
  //Allows access to the underlying SC_HANDLE representing the SCM
  operator SC_HANDLE() const;

  //Attach / Detach support from an SDK SC_HANDLE
  BOOL Attach(SC_HANDLE hSCM);
  SC_HANDLE Detach();

  //Opens a connection to the SCM
  BOOL Open(LPCTSTR pszMachineName, DWORD dwDesiredAccess);
  
  //Close the connection to the SCM
  void Close();																							 

  //Get the SCM Status
  BOOL QueryLockStatus(LPQUERY_SERVICE_LOCK_STATUS& lpLockStatus) const; 

  //Enumerates the specified services
  BOOL EnumServices(DWORD dwServiceType, DWORD dwServiceState, void* pUserData, ENUM_SERVICES_PROC lpEnumServicesFunc) const;

  //Opens the specified service
  BOOL OpenService(LPCTSTR lpServiceName,	DWORD dwDesiredAccess, CNTScmService& service) const;

  //Lock the SCM database
  BOOL Lock();

  //Unlocks the SCM database
  BOOL Unlock();

  //Windows 2000 specific functions
  BOOL EnumServices(DWORD dwServiceType, DWORD dwServiceState, LPCTSTR pszGroupName, void* pUserData, ENUM_SERVICES_PROC2 lpEnumServicesFunc) const;

protected:
//typedefs
  typedef SERVICE_STATUS_HANDLE (WINAPI REGISTERSERVICECTRLHANDLEREX)(LPCTSTR, LPHANDLER_FUNCTION_EX, LPVOID);
  typedef REGISTERSERVICECTRLHANDLEREX* LPREGISTERSERVICECTRLHANDLEREX;
  typedef BOOL (WINAPI ENUMSERVICESSTATUSEX)(SC_HANDLE, SC_ENUM_TYPE, DWORD, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD, LPDWORD, LPCTSTR);
  typedef ENUMSERVICESSTATUSEX* LPENUMSERVICESSTATUSEX;

//Member variables
  SC_HANDLE                      m_hSCM;	                           //Handle to the SCM
  SC_LOCK		                     m_hLock;                            //Handle of any lock on the Database
  HINSTANCE                      m_hAdvapi32;                        //Instance handle of the "ADVAPI32.DLL" which houses all of the NT Service functions
  LPREGISTERSERVICECTRLHANDLEREX m_lpfnRegisterServiceCtrlHandlerEx; //RegisterServiceCtrlHandler function pointer
  LPENUMSERVICESSTATUSEX         m_lpfnEnumServicesStatusEx;         //EnumServicesStatusEx function pointer
};

#endif //__NTSERVSERVICECONTROLMANAGER_H__
