/*
Module : NTSERVEVENTLOGRECORD.H
Purpose: Defines the interface for the class CEventLogRecord. 
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

#ifndef __NTSERVEVENTLOGRECORD_H__
#define __NTSERVEVENTLOGRECORD_H__


/////////////////////////////// Includes //////////////////////////////////////

#include "ntservDefines.h"


////////////////////////////// Classes ////////////////////////////////////////

//A friendlier way of handling EVENTLOGRECORD structures
class CNTSERVICE_EXT_CLASS CEventLogRecord
{
public: 
//Constructors / Destructors
  CEventLogRecord();
  CEventLogRecord(const CEventLogRecord& record);
  CEventLogRecord(const EVENTLOGRECORD* pRecord);

//Methods
  CEventLogRecord& operator=(const CEventLogRecord& record);

//Data
  DWORD                 m_dwRecordNumber;
  DWORD                 m_dwTimeGenerated;
  DWORD                 m_dwTimeWritten;
  DWORD                 m_dwEventID;
  WORD                  m_wEventType;
  WORD                  m_wEventCategory;
  CNTServiceByteArray   m_UserSID;
  CNTServiceStringArray m_Strings;
  CNTServiceByteArray   m_Data;
  CNTServiceString      m_sSourceName;
  CNTServiceString      m_sComputerName;
};

#endif //__NTSERVEVENTLOGRECORD_H__
