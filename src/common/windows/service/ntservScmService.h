/*
Module : NTSERVSCMSERVICE.H
Purpose: Defines the interface for the class CNTScmService.
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


/////////////////////////////// Macros / Defines //////////////////////////////

#pragma once

#ifndef __NTSERVSCMSERVICE_H__
#define __NTSERVSCMSERVICE_H__


////////////////////////////// Includes ///////////////////////////////////////

#include "ntservDefines.h"


////////////////////////////// Classes ////////////////////////////////////////

//Forward declaration
class CNTServiceControlManager;

//An encapsulation of a service as returned from querying the SCM (i.e. an SC_HANDLE)
class CNTSERVICE_EXT_CLASS CNTScmService
{
public:
//typedefs
  typedef BOOL (CALLBACK* ENUM_SERVICES_PROC)(void* pUserData, ENUM_SERVICE_STATUS& ss);

//Constructors / Destructors
  CNTScmService();
  ~CNTScmService();

//Methods
  //Releases the underlying SC_HANDLE
  void Close();

  //Allows access to the underlying SC_HANDLE representing the service
  operator SC_HANDLE() const;

  //Attach / Detach support from an SDK SC_HANDLE
  BOOL Attach(SC_HANDLE hService);
  SC_HANDLE Detach();

  //Changes the configuration of this service
  BOOL ChangeConfig(DWORD dwServiceType, DWORD dwStartType, DWORD dwErrorControl,
                    LPCTSTR lpBinaryPathName, LPCTSTR lpLoadOrderGroup, LPDWORD lpdwTagId,
                    LPCTSTR lpDependencies, LPCTSTR lpServiceStartName, LPCTSTR lpPassword,
                    LPCTSTR lpDisplayName) const;

  //Send a command to the service
  BOOL Control(DWORD dwControl);
  DWORD Control(DWORD dwControl, DWORD dwInfoLevel, PVOID pControlParams);

  //These functions call Control() with the standard predefined control codes
  BOOL Stop() const;			  //Ask the service to stop
  BOOL Pause() const;			  //Ask the service to pause
  BOOL Continue() const;	  //Ask the service to continue
  BOOL Interrogate() const; //Ask the service to update its status to the SCM

  //Waits for a service to stop with a configurable timeout
  BOOL WaitForStop(DWORD dwTimeout);

  //Waits for a service to have a certain status (with a configurable timeout)
  BOOL WaitForServiceStatus(DWORD dwTimeout, DWORD dwWaitForStatus, DWORD dwPollingInterval = 250);

  //Start the execution of the service
  BOOL Start(DWORD dwNumServiceArgs, LPCTSTR* lpServiceArgVectors) const;	

  //Determines what Control codes this service supports
  BOOL AcceptStop(BOOL& bStop);                   //Ask the service can it stop
  BOOL AcceptPauseContinue(BOOL& bPauseContinue);	//Ask the service can it pause continue
  BOOL AcceptShutdown(BOOL& bShutdown);           //Ask the service if it is notified of shutdowns

  //Get the most return status of the service reported to the SCM by this service
  BOOL QueryStatus(SERVICE_STATUS& ServiceStatus) const;

  //Get the configuration parameters of this service from the SCM
  BOOL QueryConfig(LPQUERY_SERVICE_CONFIG& lpServiceConfig) const;

  //Add a new service to the SCM database
  BOOL Create(CNTServiceControlManager& Manager, LPCTSTR lpServiceName,	LPCTSTR lpDisplayName,	           
              DWORD dwDesiredAccess, DWORD dwServiceType, DWORD dwStartType, DWORD dwErrorControl,	             
              LPCTSTR lpBinaryPathName, LPCTSTR lpLoadOrderGroup, LPDWORD lpdwTagId, LPCTSTR lpDependencies,	           
              LPCTSTR lpServiceStartName,	LPCTSTR lpPassword);

  //Mark this service as to be deleted from the SCM.
  BOOL Delete() const;

  //Enumerate the services that this service depends upon
  BOOL EnumDependents(DWORD dwServiceState, void* pUserData, ENUM_SERVICES_PROC lpEnumServicesFunc) const;

  //Get the security information associated with this service
  BOOL QueryObjectSecurity(SECURITY_INFORMATION dwSecurityInformation, PSECURITY_DESCRIPTOR& lpSecurityDescriptor) const;

  //Set the security descriptor associated with this service
  BOOL SetObjectSecurity(SECURITY_INFORMATION dwSecurityInformation, PSECURITY_DESCRIPTOR lpSecurityDescriptor) const;

  //Windows 2000+ specific functions
  BOOL QueryDescription(CNTServiceString& sDescription) const;
  BOOL ChangeDescription(const CNTServiceString& sDescription);
  BOOL QueryFailureActions(LPSERVICE_FAILURE_ACTIONS& pActions) const;
  BOOL ChangeFailureActions(LPSERVICE_FAILURE_ACTIONS pActions);
  BOOL QueryStatus(SERVICE_STATUS_PROCESS& ssp) const;

  //Windows Vista / 2008+ specific functions
  DWORD NotifyStatusChange(DWORD dwNotifyMask, PSERVICE_NOTIFY pNotifyBuffer) const;
  BOOL  QueryDelayAutoStart(BOOL& bDelayedAutoStart) const;
  BOOL  ChangeDelayAutoStart(BOOL bDelayedAutoStart);
  BOOL  QueryFailureActionsFlag(BOOL& bFailureActionsOnNonCrashFailures) const;
  BOOL  ChangeFailureActionsFlag(BOOL bFailureActionsOnNonCrashFailures);
  BOOL  QuerySidInfo(DWORD& dwServiceSidType) const;
  BOOL  ChangeSidInfo(DWORD dwServiceSidType);
  BOOL  QueryRequiredPrivileges(CNTServiceStringArray& privileges) const;
  BOOL  ChangeRequiredPrivileges(const CNTServiceStringArray& priviledges);
  BOOL  QueryPreShutdown(DWORD& dwPreshutdownTimeout) const;
  BOOL  ChangePreShutdown(DWORD dwPreshutdownTimeout);

  //Windows 7 / 2008 RC+ specific functions
  BOOL ChangeTrigger(PSERVICE_TRIGGER_INFO pTriggerInfo);
  BOOL QueryTrigger(PSERVICE_TRIGGER_INFO& pTriggerInfo) const;
  BOOL ChangePreferredNode(USHORT usPreferredNode, BOOL bDelete);
  BOOL QueryPreferredNode(USHORT& usPreferredNode, BOOL& bDelete) const;

  //Windows 8.1+ specific functions
  BOOL ChangeLaunchProtected(DWORD dwLaunchProtected);
  BOOL QueryLaunchProtected(DWORD& dwLaunchProtected) const;

protected:
//typedefs
  typedef BOOL (WINAPI QUERYSERVICECONFIG2)(SC_HANDLE, DWORD, LPBYTE, DWORD, LPDWORD);
  typedef QUERYSERVICECONFIG2* LPQUERYSERVICECONFIG2;
  typedef BOOL (WINAPI CHANGESERVICECONFIG2)(SC_HANDLE, DWORD, LPVOID);
  typedef CHANGESERVICECONFIG2* LPCHANGESERVICECONFIG2;
  typedef BOOL (WINAPI QUERYSERVICESTATUSEX)(SC_HANDLE, SC_STATUS_TYPE, LPBYTE, DWORD, LPDWORD);
  typedef QUERYSERVICESTATUSEX* LPQUERYSERVICESTATUSEX;
  typedef BOOL (WINAPI NOTIFYSERVICESTATUSCHANGE)(SC_HANDLE, DWORD, PSERVICE_NOTIFY);
  typedef NOTIFYSERVICESTATUSCHANGE* LPNOTIFYSERVICESTATUSCHANGE;
  typedef BOOL (WINAPI CONTROLSERVICEEX)(SC_HANDLE, DWORD, DWORD, PVOID);
  typedef CONTROLSERVICEEX* LPCONTROLSERVICEEX;

//Member variables
  SC_HANDLE                   m_hService;
  HINSTANCE                   m_hAdvapi32;                     //Instance handle of the "ADVAPI32.DLL" which houses all of the NT Service functions
  LPQUERYSERVICECONFIG2       m_lpfnQueryServiceConfig2;       //QueryServiceConfig2 function pointer
  LPCHANGESERVICECONFIG2      m_lpfnChangeServiceConfig2;      //ChangeServiceConfig2 function pointer
  LPQUERYSERVICESTATUSEX      m_lpfnQueryServiceStatusEx;      //QueryServiceStatusEx function pointer
  LPNOTIFYSERVICESTATUSCHANGE m_lpfnNotifyServiceStatusChange; //NotifyServiceStatusChange function pointer
  LPCONTROLSERVICEEX          m_lpfnControlServiceEx;          //ControlServiceEx function pointer
};

#endif //__NTSERVSCMSERVICE_H__
