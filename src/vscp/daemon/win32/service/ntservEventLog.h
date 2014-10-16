/*
Module : NTSERVEVENTLOG.H
Purpose: Defines the interface for the class CNTEventLog. 
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

#ifndef __NTSERVEVENTLOG_H__
#define __NTSERVEVENTLOG_H__


////////////////////////////// Includes ///////////////////////////////////////

#include "ntservEventLogRecord.h"


////////////////////////////// Classes ////////////////////////////////////////

//An encapsulation of the client side to the NT event log APIs
class CNTSERVICE_EXT_CLASS CNTEventLog
{
public:
//Constructors / Destructors
  CNTEventLog();
  ~CNTEventLog();

//Methods
  operator HANDLE() const;
  BOOL     Attach(HANDLE hEventLog);
  HANDLE   Detach();
  BOOL     Open(LPCTSTR lpUNCServerName, LPCTSTR lpSourceName);
  BOOL     OpenBackup(LPCTSTR lpUNCServerName, LPCTSTR lpFileName);
  BOOL     OpenApplication(LPCTSTR lpUNCServerName);
  BOOL     OpenSystem(LPCTSTR lpUNCServerName);
  BOOL     OpenSecurity(LPCTSTR lpUNCServerName);
  BOOL     Close();
  BOOL     Backup(LPCTSTR lpBackupFileName) const;
  BOOL     Clear(LPCTSTR lpBackupFileName) const;
  BOOL     GetNumberOfRecords(DWORD& dwNumberOfRecords) const;
  BOOL     GetOldestRecord(DWORD& dwOldestRecord) const;
  BOOL     NotifyChange(HANDLE hEvent) const;
  BOOL     ReadNext(CEventLogRecord& record) const;
  BOOL     ReadPrev(CEventLogRecord& record) const;
  BOOL     GetFullInformation(DWORD& dwFull) const;

protected:
  HANDLE m_hEventLog;
};

#endif //__NTSERVEVENTLOG_H__
