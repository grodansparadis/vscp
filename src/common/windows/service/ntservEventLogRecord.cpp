/*  
Module : NTSERVEVENTLOGRECORD.CPP
Purpose: Implementation for the class CNTEventLogRecord
Created: PJN / 14-07-1997
History: PJN / 14-08-2005 1. Fixed an issue where the SID and binary values was not being set correctly in the 
                          overloaded CEventLogRecord constructor which takes a EVENTLOGRECORD pointer. Thanks 
                          to Brian Lee for reporting this issue. Also the arrays which this information is 
                          copied into is now preallocated to improve performance.
         PJN / 12-05-2006 1. Minor update to remove now unnecessary include of AfxPriv.h from this module.
         PJN / 25-06-2006 1. Optimized CEventLogRecord constructor
                          2. Code now uses newer C++ style casts instead of C style casts.
         PJN / 01-08-2010 1. CEventLogRecord::m_TimeGenerated and CEventLogRecord::m_TimeWritten values are now 
                          simple DWORD values instead of CTime instances.

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

#include "stdafx.h"
#include "ntservEventLogRecord.h"


/////////////////////////////////  Macros /////////////////////////////////////

#ifdef CNTSERVICE_MFC_EXTENSIONS
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#endif


///////////////////////////////// Implementation //////////////////////////////

CEventLogRecord::CEventLogRecord() : m_dwRecordNumber(0),
                                     m_dwTimeGenerated(0),
                                     m_dwTimeWritten(0),
                                     m_dwEventID(0),
                                     m_wEventType(0),
                                     m_wEventCategory(0)
{
}

CEventLogRecord::CEventLogRecord(const CEventLogRecord& record)
{
  *this = record;
}

CEventLogRecord::CEventLogRecord(const EVENTLOGRECORD* pRecord)
{
  //Validate our parameters
  ATLASSUME(pRecord != NULL);

  //First the easy ones
  m_dwRecordNumber = pRecord->RecordNumber;
  m_dwTimeGenerated = pRecord->TimeGenerated;
  m_dwTimeWritten = pRecord->TimeWritten;
  m_dwEventID = pRecord->EventID;
  m_wEventType = pRecord->EventType;
  m_wEventCategory = pRecord->EventCategory;

  //Copy over the SID
  DWORD i = 0;
  const BYTE* pBeginRecord = reinterpret_cast<const BYTE*>(pRecord);
  ATLASSUME(pBeginRecord != NULL);
  DWORD dwCurOffset = pRecord->UserSidOffset;
  if (pRecord->UserSidLength)
  {
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    m_UserSID.SetSize(0, pRecord->UserSidLength); //Preallocate the array to improve performance
  #else
    m_UserSID.reserve(pRecord->UserSidLength); //Preallocate the array to improve performance
  #endif
  }
  while (i < pRecord->UserSidLength)
  {
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    m_UserSID.Add(pBeginRecord[dwCurOffset + i]);
  #else
    m_UserSID.push_back(pBeginRecord[dwCurOffset + i]);
  #endif
    i++;
  }
  dwCurOffset += pRecord->UserSidLength;

  //Copy over the Binary data
  i = 0;
  dwCurOffset = pRecord->DataOffset;
  if (pRecord->DataLength)
  {
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    m_Data.SetSize(0, pRecord->DataLength); //Preallocate the array to improve performance
  #else
    m_Data.reserve(pRecord->DataLength); //Preallocate the array to improve performance
  #endif
  }
  while (i < pRecord->DataLength)
  {
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    m_Data.Add(pBeginRecord[dwCurOffset + i]);
  #else
    m_Data.push_back(pBeginRecord[dwCurOffset + i]);
  #endif
    i++;
  }
  dwCurOffset += pRecord->DataLength;

  //Copy over the SourceName
  const TCHAR* pszBeginRecord = reinterpret_cast<const TCHAR*>(pBeginRecord + sizeof(EVENTLOGRECORD));
  dwCurOffset = 0;
  DWORD dwStartOffset = dwCurOffset;
  while (pszBeginRecord[dwCurOffset])
    dwCurOffset++;
  m_sSourceName = &pszBeginRecord[dwStartOffset];

  //Skip over the NULL 
  dwCurOffset++;

  //Copy over the ComputerName
  dwStartOffset = dwCurOffset;
  while (pszBeginRecord[dwCurOffset])
    dwCurOffset++;
  m_sComputerName = &pszBeginRecord[dwStartOffset];

  //Copy over the strings array
  int nStringsRead = 0;
  pszBeginRecord = reinterpret_cast<const TCHAR*>(pBeginRecord + pRecord->StringOffset);
  dwCurOffset = 0;
  while (nStringsRead < pRecord->NumStrings)
  {
    //Find the next string
    dwStartOffset = dwCurOffset;
    while (pszBeginRecord[dwCurOffset])
      dwCurOffset++;

    //Add it to the array
    CNTServiceString sText(&pszBeginRecord[dwStartOffset]);
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    m_Strings.Add(sText);
  #else
    m_Strings.push_back(sText);
  #endif

    //Increment the number of strings read
    nStringsRead++;

    //Skip over the NULL
    dwCurOffset++;
  }
}

CEventLogRecord& CEventLogRecord::operator=(const CEventLogRecord& record)
{
  m_dwRecordNumber = record.m_dwRecordNumber;
  m_dwTimeGenerated = record.m_dwTimeGenerated;
  m_dwTimeWritten = record.m_dwTimeWritten;
  m_dwEventID = record.m_dwEventID;
  m_wEventType = record.m_wEventType;
  m_wEventCategory = record.m_wEventCategory;
#ifdef CNTSERVICE_MFC_EXTENSIONS
  m_UserSID.Copy(record.m_UserSID);
  m_Strings.Copy(record.m_Strings);
  m_Data.Copy(record.m_Data);
#else
  m_UserSID = record.m_UserSID;
  m_Strings = record.m_Strings;
  m_Data = record.m_Data;
#endif
  m_sSourceName = record.m_sSourceName;
  m_sComputerName = record.m_sComputerName;

  return *this;
}
