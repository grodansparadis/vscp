/*
Module : NTSERV.H
Purpose: Defines the interface for an MFC class framework which encapsulate the whole area of Windows services
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

#ifndef __NTSERV_H__
#define __NTSERV_H__


////////////////////////////// Includes ///////////////////////////////////////

#include "ntservDefines.h"
#include "ntservCmdLineInfo.h"
#include "ntservEventLogSource.h"
#include "ntservServiceControlManager.h"


////////////////////////////// Classes ////////////////////////////////////////

//Class which carries the exit code information when you call CNTService::TerminateService
class CNTSERVICE_EXT_CLASS CNTServiceTerminateException
{
public:
//Constructors / Destructors
  CNTServiceTerminateException(DWORD dwWin32ExitCode, DWORD dwServiceSpecificExitCode);
  
//member variables
  DWORD m_dwWin32ExitCode;
  DWORD m_dwServiceSpecificExitCode;
};

//An MFC framework encapsulation of an NT service. You are meant to derive your 
//own class from this and override its functions to implement your own service 
//specific functionality.
class CNTSERVICE_EXT_CLASS CNTService
{
public:
//Defines
  #define STATE_NO_CHANGE 0xffffffff

  //Used in SetServiceToStatus
  enum ServiceAction 
  {
    ActionStartService,
    ActionPauseService,
    ActionContinueService,
    ActionStopService
  };

  enum UIErrorLoggingDetail
  {
    UI_ErrorCode,
    UI_ErrorCodeAndErrorDescription,
    UI_ErrorDescription,
    UI_StringAndErrorCode,
    UI_StringAndErrorCodeAndErrorDescription,
    UI_StringAndErrorDescription
  };

  enum ELErrorLoggingDetail
  {
    EL_ErrorCode,
    EL_ErrorCodeAndErrorDescription,
    EL_ErrorDescription
  };

//Constructors / Destructors
  CNTService();
  CNTService(LPCTSTR lpszServiceName, LPCTSTR lpszDisplayName, DWORD dwControlsAccepted, LPCTSTR lpszDescription = NULL); 
  virtual ~CNTService() {};

//Accessors / Mutators
  CNTServiceString GetServiceName() const { return m_sServiceName; };
  CNTServiceString GetInstanceServiceName() const { return m_sServiceName; };
  CNTServiceString GetMasterServiceName() const { return m_sMasterServiceName; }
  void             SetServiceName(LPCTSTR sServiceName);
  void             SetInstanceServiceName(LPCTSTR sServiceName) { m_sServiceName = sServiceName; };
  void             SetMasterServiceName(LPCTSTR sServiceName) { m_sMasterServiceName = sServiceName; };
  CNTServiceString GetDisplayName() const { return m_sDisplayName; };
  CNTServiceString GetInstanceDisplayName() const { return m_sDisplayName; };
  CNTServiceString GetMasterDisplayName() const { return m_sMasterDisplayName; };
  void             SetDisplayName(LPCTSTR sDisplayName);
  void             SetInstanceDisplayName(LPCTSTR sDisplayName) { m_sDisplayName = sDisplayName; };
  void             SetMasterDisplayName(LPCTSTR sDisplayName) { m_sMasterDisplayName = sDisplayName; };
  CNTServiceString GetDescription() const { return m_sDescription; };
  CNTServiceString GetInstanceDescription() const { return m_sDescription; };
  CNTServiceString GetMasterDescription() const { return m_sMasterDescription; };
  void             SetDescription(LPCTSTR sDescription);
  void             SetInstanceDescription(LPCTSTR sDescription) { m_sDescription = sDescription; };
  void             SetMasterDescription(LPCTSTR sDescription) { m_sMasterDescription = sDescription; };
  void             SetAllowCommandLine(BOOL bCommandLine) { m_bAllowCommandLine = bCommandLine; };
  BOOL             GetAllowCommandLine() const { return m_bAllowCommandLine; };
  void             SetAllowNameChange(BOOL bNameChange) { m_bAllowNameChange = bNameChange; };
  BOOL             GetAllowNameChange() const { return m_bAllowNameChange; };
  void             SetAllowDescriptionChange(BOOL bDescriptionChange) { m_bAllowDescriptionChange = bDescriptionChange; };
  BOOL             GetAllowDescriptionChange() const { return m_bAllowDescriptionChange; };
  void             SetAllowDisplayNameChange(BOOL bDisplayNameChange) { m_bAllowDisplayNameChange = bDisplayNameChange; };
  BOOL             GetAllowDisplayNameChange() const { return m_bAllowDisplayNameChange; };
  void             SetUseConsole(BOOL bUseConsole) { m_bUseConsole = bUseConsole; };
  BOOL             GetUseConsole() const { return m_bUseConsole; };
  void             SetControlsAccepted(DWORD dwControlsAccepted) { m_dwControlsAccepted = dwControlsAccepted; };
  DWORD            GetControlsAccepted() const { return m_dwControlsAccepted; };

//Persistance support
  //Allows saving and restoring of a services settings to the 
  //"HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\ServiceName\Parameters"
  //location in the registry
  virtual BOOL    WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
  virtual BOOL    WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);
  virtual BOOL    WriteProfileStringArray(LPCTSTR lpszSection, LPCTSTR lpszEntry, const CNTServiceStringArray& array);
  virtual BOOL    WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes);

  virtual CNTServiceString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL, DWORD* pLastError = NULL);
  virtual UINT    GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault, DWORD* pLastError = NULL);
  virtual BOOL    GetProfileStringArray(LPCTSTR lpszSection, LPCTSTR lpszEntry, CNTServiceStringArray& array); 
  virtual BOOL    GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE* ppData, ULONG* pnBytes);

  //These versions of the persistence functions allow control over the "ServiceName" to write to as described above as well as control over the bFlush value 
  static BOOL     WriteServiceProfileString(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue, BOOL bFlush, DWORD* pLastError = NULL);
  static BOOL     WriteServiceProfileInt(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue, BOOL bFlush, DWORD* pLastError = NULL);
  static BOOL     WriteServiceProfileStringArray(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, const CNTServiceStringArray& array, BOOL bFlush, DWORD* pLastError = NULL);
  static BOOL     WriteServiceProfileBinary(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, ULONG nBytes, BOOL bFlush, DWORD* pLastError = NULL);
  static CNTServiceString  GetServiceProfileString(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL, DWORD* pLastError = NULL);
  static UINT     GetServiceProfileInt(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault, DWORD* pLastError = NULL);
  static BOOL     GetServiceProfileStringArray(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, CNTServiceStringArray& array, DWORD* pLastError = NULL);
  static BOOL     GetServiceProfileBinary(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE* ppData, ULONG* pnBytes, DWORD* pLastError = NULL);

  //Allows client apps to decide if the above settings should be flushed to the registry
  //or allow them to be written to the registry in a lazy fashion
  void SetProfileWriteFlush(BOOL bFlush) { m_bProfileWriteFlush = bFlush; };
  BOOL GetProfileWriteFlush() const { return m_bProfileWriteFlush; };

//Other Methods
  //Helpful functions to parse the command line and execute the results
  virtual void ParseCommandLine(CNTServiceCommandLineInfo& rCmdInfo);
  virtual DWORD ProcessShellCommand(CNTServiceCommandLineInfo& rCmdInfo);

  //Reports the status of this service back to the SCM
  virtual BOOL ReportStatus();
  virtual BOOL ReportStatus(DWORD dwCurrentState, DWORD dwCheckPoint = STATE_NO_CHANGE,
                            DWORD dwWaitHint = STATE_NO_CHANGE, DWORD dwControlsAccepted = STATE_NO_CHANGE);

  //Installs the callback funtion by calling RegisterServiceCtrlHandler
  virtual BOOL RegisterCtrlHandler();

  //Member function which does the job of responding to SCM requests
  virtual DWORD WINAPI ServiceCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData);

  //The ServiceMain function for this service
  virtual void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);

  //Called in reponse to a shutdown request
  virtual void OnStop();

  //Called in reponse to a pause request
  virtual void OnPause();

  //Called in reponse to a continue request
  virtual void OnContinue();

  //Called in reponse to a Interrogate request
  virtual void OnInterrogate();

  //Called in reponse to a Shutdown request
  virtual void OnShutdown();

  //Called in response to a "param change"
  virtual void OnParamChange();

  //Called in response to a "hardware profile change"
  virtual DWORD OnHardwareProfileChange(DWORD dwEventType, LPVOID lpEventData);

  //Called in response to a "power event"
  virtual DWORD OnPowerEvent(DWORD dwEventType, LPVOID lpEventData);

  //Called in response to a something being added to the services network binding
  virtual void OnNetBindAdd();

  //Called in response to a something being removed from the services network binding
  virtual void OnNetBindRemove();

  //Called in response to a something being enabled in a services network binding
  virtual void OnNetBindEnable();

  //Called in response to a something being disabled in a services network binding
  virtual void OnNetBindDisable();

  //Called in response to a SERVICE_CONTROL_DEVICEEVENT control request
  virtual DWORD OnDeviceEvent(DWORD dwEventType, LPVOID lpEventData);

  //Called in reponse to a user defined request
  virtual void OnUserDefinedRequest(DWORD dwControl);

  //Called when a session change event occurs
  virtual void OnSessionChange(DWORD dwEventType, WTSSESSION_NOTIFICATION* lpEventData);

  //Called in response to a PreShutdown request
  virtual void OnPreShutdown();

  //Called when a time change event occurs
  virtual void OnTimeChange(SERVICE_TIMECHANGE_INFO* lpEventData);

  //Called in response to a trigger event
  virtual void OnTriggerEvent();

  //Kicks off the Service. You would normally call this
  //some where in your main/wmain or InitInstance
  //a standard process rather than as a service. If you are
  //using the CNTServiceCommandLineInfo class, then internally
  //it will call this function for you.
  virtual BOOL Run();

  //Puts a service to a certain status
  virtual BOOL SetServiceToStatus(CNTService::ServiceAction esaStatusWanted, DWORD& dwError, DWORD dwTimeout = 5000, DWORD dwPollingInterval = 250);

  //Installs the service
  virtual BOOL Install(CNTServiceString& sErrorMsg, DWORD& dwError);

  //Uninstalls the service
  virtual BOOL Uninstall(CNTServiceString& sErrorMsg, DWORD& dwError, DWORD dwTimeToWaitForStop = 5000);

  //Runs the service as a normal function as opposed
  //to a service
  virtual void Debug();

  //Displays help for this service
  virtual void ShowHelp();

  //Called to display a message for the service to the end-user
  virtual void DisplayMessage(const CNTServiceString& sMessage);

  //Called to display a message to the debugger output window
  virtual void TraceMessage(LPCTSTR pszMessage);
  virtual void TraceMessage(const CNTServiceString& sMessage);

  //Equivalent to the AfxFormatStringX functions
  virtual void FormatString(CNTServiceString& sMsg, UINT nIDS, LPCTSTR psz1);
  virtual void FormatString(CNTServiceString& sMsg, UINT nIDS, LPCTSTR psz1, LPCTSTR psz2);

  //Enumerate all instances of this service installed on the local machine
  virtual BOOL EnumerateInstances(CNTServiceStringArray& ServiceNames, DWORD& dwError);

  //QueryServiceDynamicInformation API wrapper
  virtual BOOL QueryServiceDynamicInformation(DWORD dwInfoLevel, PVOID* ppDynamicInfo);

protected:
//Methods
  //These three static functions are used internally to
  //go from the SDK functions to the C++ member functions
  static void WINAPI _ServiceCtrlHandler(DWORD dwControl);	
  static DWORD WINAPI _ServiceCtrlHandlerEx(DWORD dwControl, DWORD dwEventType,  
                                            LPVOID lpEventData, LPVOID lpContext);
  static void WINAPI _SDKServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);

  //Actual C++ implementation of ServiceMain
  virtual void _ServiceMain(DWORD dwArgv, LPTSTR* lpszArgv);

  //Function used by constructors to set the initial state of our variables.
  void Initialize(LPCTSTR lpszServiceName, LPCTSTR lpszDisplayName, DWORD dwControlsAccepted, LPCTSTR lpszDescription);

  //Used to setup the function pointer(s)
  void InitializeFunctionPointers();

  //Helper functions to convert a Win32 Error code to a string and handle resource id and message id mapping
  virtual CNTServiceString Win32ErrorToString(UINT nStringID, DWORD dwError, BOOL bEventLog);
  virtual UINT MapResourceID(UINT nID);
  virtual UINT MapMessageID(UINT nID);
  virtual CNTServiceString GetErrorMessage(DWORD dwError);

  //Used internally by the persistance functions
  static BOOL GetSectionKey(ATL::CRegKey& sectionKey, LPCTSTR lpszServiceName, LPCTSTR lpszSection, BOOL bReadOnly = FALSE, DWORD* pLastError = NULL);
  static BOOL GetServiceParametersKey(ATL::CRegKey& serviceKey, LPCTSTR lpszServiceName, BOOL bReadOnly = FALSE, DWORD* pLastError = NULL);

  //Actual implementation of the function to report status to the SCM
  virtual BOOL _ReportStatus(DWORD dwCurrentState, DWORD dwCheckPoint = STATE_NO_CHANGE,
                             DWORD dwWaitHint = STATE_NO_CHANGE, DWORD dwControlsAccepted = STATE_NO_CHANGE,
                             DWORD dwWin32ExitCode = NO_ERROR, DWORD dwServiceSpecificExitCode = 0);

  //Used during the Install procedure
  virtual BOOL GetDependencies(ATL::CHeapPtr<TCHAR>& mszDependencies);

  //Used by EnumerateInstances
  static BOOL CALLBACK _EnumerateInstances(void* pData, ENUM_SERVICE_STATUS& ss);

  //Terminate the service
  void TerminateService(DWORD dwWin32ExitCode = NO_ERROR, DWORD dwServiceSpecificExitCode = 0);

  //Console Control handler
  static BOOL WINAPI _ConsoleCtrlHandler(DWORD dwCtrlType);
  virtual BOOL ConsoleCtrlHandler(DWORD dwCtrlType);

  //Simple helper function which securely nukes the contents of a CNTServiceString
  FORCEINLINE static void SecureEmptyString(CNTServiceString& sVal);

  //Used during Install method
  virtual BOOL InstallEventLogSource(CNTServiceString& sErrorMsg, DWORD& dwError);
  virtual BOOL InstallServiceConfiguration(CNTScmService& service, CNTServiceString& sErrorMsg, DWORD& dwError);

//typedefs of the function pointers
  typedef SERVICE_STATUS_HANDLE (WINAPI REGISTERSERVICECTRLHANDLEREX)(LPCTSTR, LPHANDLER_FUNCTION_EX, LPVOID);
  typedef REGISTERSERVICECTRLHANDLEREX* LPREGISTERSERVICECTRLHANDLEREX;
  typedef BOOL (WINAPI QUERYSERVICEDYNAMICINFORMATION)(SERVICE_STATUS_HANDLE, DWORD, PVOID);
  typedef QUERYSERVICEDYNAMICINFORMATION* LPQUERYSERVICEDYNAMICINFORMATION;

//Member variables
  DWORD                            m_dwServiceType;
  DWORD                            m_dwStartType;
  DWORD                            m_dwErrorControl;
  CNTServiceString                 m_sLoadOrderGroup;
  CNTServiceString                 m_sUserName;
  CNTServiceString                 m_sPassword;
  DWORD                            m_dwEventCategoryCount;
  CNTServiceString                 m_sEventCategoryMessageFile;
  CNTServiceString                 m_sEventMessageFile;
  CNTServiceString                 m_sEventParameterMessageFile;
  DWORD                            m_dwEventTypesSupported;
  UIErrorLoggingDetail             m_UILoggingDetail;
  ELErrorLoggingDetail             m_ELLoggingDetail;
  CNTServiceStringArray            m_sDependencies;
  BOOL                             m_bEventLogSource;                    //Is this service also an event log source
  SERVICE_STATUS_HANDLE            m_hStatus;
  DWORD                            m_dwControlsAccepted;                 //What Control requests will this service repond to
  DWORD                            m_dwCheckPoint;                    
  DWORD                            m_dwWaitHint;                      
  DWORD                            m_dwCurrentState;                     //Current Status of the service
  DWORD                            m_dwRequestedControl;                 //The most recent value sent into ServiceCtrlHandler
  CNTServiceString                 m_sServiceName;                       //Name of the service
  CNTServiceString                 m_sMasterServiceName;                 //The name of the service (unaffected by calls to change it via /SN)
  CNTServiceString                 m_sDisplayName;                       //Display name for the service
  CNTServiceString                 m_sMasterDisplayName;                 //The display name of the service (unaffected by calls to change it via /SDN)
  CNTServiceString                 m_sDescription;                       //The description text for the service
  CNTServiceString                 m_sMasterDescription;                 //THe description text for the service (unaffected by calls to change it via /SD)
  BOOL                             m_bDelayedAutoStart;                  //TRUE enables a delayed Auto start service
  DWORD                            m_dwServiceSidType;                   //The service sid type
  CNTServiceStringArray            m_sPrivileges;                        //The required privileges for this service
  DWORD                            m_dwPreshutdownTimeout;               //The pre shutdown timeout for this service
  DWORD                            m_dwLaunchProtected;                  //The launch protected setting for this service
  CNTEventLogSource                m_EventLogSource;                     //For reporting to the event log
  static CNTService*               sm_lpService;						             //Static which contains the this pointer
  ATL::CComAutoCriticalSection     m_CritSect;                           //Protects changes to any member variables from multiple threads
  BOOL                             m_bProfileWriteFlush;                 //Should calls to WriteProfile... be synchronous or left to be written by the lazy writer
  BOOL                             m_bAllowCommandLine;                  //Should /SCL command line settings be observed
  BOOL                             m_bAllowNameChange;                   //Should /SN command line settings be observed 
  BOOL                             m_bAllowDescriptionChange;            //Should /SD command line settings be observed
  BOOL                             m_bAllowDisplayNameChange;            //Should /SDN command line settings be observed
  BOOL                             m_bUseConsole;                        //TRUE if console output should be used, FALSE for GUI
  CNTServiceString                 m_sBinaryPathName;                    //The binary path we use for installing
  HINSTANCE                        m_hAdvapi32;                          //Instance handle of the "ADVAPI32.DLL" which houses all of the NT Service functions
  LPREGISTERSERVICECTRLHANDLEREX   m_lpfnRegisterServiceCtrlHandlerEx;   //RegisterServiceCtrlHandlerEx function pointer
  LPQUERYSERVICEDYNAMICINFORMATION m_lpfnQueryServiceDynamicInformation; //QueryServiceDynamicInformation function pointer
};

#endif //__NTSERV_H__
