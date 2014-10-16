/*
Module : NTSERVEVENTLOGSOURCE.H
Purpose: Defines the interface for the class CNTEventLogSource.
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


////////////////////////////// Macros / Defines ///////////////////////////////

#pragma once

#ifndef __NTSERVEVENTLOGSOURCE_H__
#define __NTSERVEVENTLOGSOURCE_H__


////////////////////////////// Includes ///////////////////////////////////////

#include "ntservDefines.h"


////////////////////////////// Classes ////////////////////////////////////////

//An encapsulation of the APIs used to register, unregister, write, install and uninstall Event log entries 
//i.e. the server side to the Event log APIs
class CNTSERVICE_EXT_CLASS CNTEventLogSource
{
public:
//Constructors / Destructors
  CNTEventLogSource();
  CNTEventLogSource(LPCTSTR lpUNCServerName, LPCTSTR lpSourceName, LPCTSTR lpLogName);
  CNTEventLogSource(LPCTSTR lpUNCServerName, LPCTSTR lpSourceName);
  ~CNTEventLogSource();

//Accessors / Mutators
  void             SetServerName(LPCTSTR pszServerName) { m_sServerName = pszServerName; };
  CNTServiceString GetServerName() const { return m_sServerName; };
  void             SetSourceName(LPCTSTR pszSourceName) { m_sSourceName = pszSourceName; };
  CNTServiceString GetSourceName() const { return m_sSourceName; };
  void             SetLogName(LPCTSTR pszLogName) { m_sLogName = pszLogName; };
  CNTServiceString GetLogName() const { return m_sLogName; };

//Methods
  operator HANDLE() const;
  BOOL Attach(HANDLE hEventSource);
  HANDLE Detach();
  BOOL Register(LPCTSTR lpUNCServerName, LPCTSTR lpSourceName);
  BOOL Deregister();
  BOOL Report(WORD wType, WORD wCategory, DWORD dwEventID, PSID lpUserSid,	   
              WORD wNumStrings, DWORD dwDataSize, LPCTSTR* lpStrings, LPVOID lpRawData);
  BOOL Report(WORD wType, DWORD dwEventID);
  BOOL Report(WORD wType, LPCTSTR lpszString);
  BOOL Report(WORD wType, DWORD dwEventID, LPCTSTR lpszString);
  BOOL Report(WORD wType, DWORD dwEventID, LPCTSTR lpszString1, LPCTSTR lpszString2);
  BOOL Report(WORD wType, DWORD dwEventID, LPCTSTR lpszString1, LPCTSTR lpszString2, LPCTSTR lpszString3);
  BOOL Report(WORD wType, DWORD dwEventID, LPCTSTR lpszString1, LPCTSTR lpszString2, DWORD dwCode, BOOL bReportAsHex = FALSE);
  BOOL Report(WORD wType, DWORD dwEventID, LPCTSTR lpszString, DWORD dwCode, BOOL bReportAsHex = FALSE);
  BOOL Report(WORD wType, DWORD dwEventID, DWORD dwCode, BOOL bReportAsHex = FALSE);

  static BOOL Install(LPCTSTR lpszSourceName, LPCTSTR lpszEventMessageFile,LPCTSTR pszEventCategoryMessageFile, LPCTSTR pszEventParameterMessageFile, DWORD dwTypesSupported, DWORD dwCategoryCount);
  static BOOL Install(LPCTSTR lpszLogName, LPCTSTR lpszSourceName, LPCTSTR lpszEventMessageFile, LPCTSTR pszEventCategoryMessageFile, LPCTSTR pszEventParameterMessageFile, DWORD dwTypesSupported, DWORD dwCategoryCount);
  static BOOL Uninstall(LPCTSTR lpSourceName);
  static BOOL Uninstall(LPCTSTR lpszLogName, LPCTSTR lpszSourceName);
  static BOOL GetStringArrayFromRegistry(ATL::CRegKey& key, LPCTSTR lpszEntry, CNTServiceStringArray& array, DWORD* pLastError = NULL);
  static BOOL SetStringArrayIntoRegistry(ATL::CRegKey& key, LPCTSTR lpszEntry, const CNTServiceStringArray& array, DWORD* pLastError = NULL);
  
protected:
  HANDLE                       m_hEventSource;
  CNTServiceString             m_sServerName;
  CNTServiceString             m_sSourceName;
  CNTServiceString             m_sLogName;
  ATL::CComAutoCriticalSection m_csReport; //Critical section to protect multiple threads calling Report at the one time

  friend class CNTService;
};

#endif //__NTSERVEVENTLOGSOURCE_H__
