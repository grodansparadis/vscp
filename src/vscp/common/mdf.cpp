// FILE: mdf.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, the VSCP project
// <info@vscp.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "frmmain.h"
#endif

#ifdef WIN32
#include <pch.h>
#endif

#include <deque>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

#include <canal.h>
#include <mdf.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <vscp.h>
#include <vscphelper.h>

#include <deque>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

#include <expat.h>
#include <json.hpp> // Needs C++11  -std=c++11
#include <mustache.hpp>

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

// https://github.com/nlohmann/json
using json = nlohmann::json;

using namespace kainjow::mustache;

// Bugger for XML parser
#define XML_BUFF_SIZE 512000

// notes
// =====
// All object that can be in different languages default to
// "en". This works by assigning the selected locale ( m_strLocale )
// if found and the default ("en") if the selected language is not found.

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_ValueListValue::CMDF_ValueListValue()
{
  ;
}

CMDF_ValueListValue::~CMDF_ValueListValue()
{
  m_strDescription.clear();
  m_strHelpType.clear();
  m_strHelp.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_RemoteVariable::CMDF_RemoteVariable() {}

CMDF_RemoteVariable::~CMDF_RemoteVariable()
{
  clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  ClearStorage
//

void
CMDF_RemoteVariable::clearStorage(void)
{
  // Clearup value list
  std::deque<CMDF_ValueListValue *>::iterator iterValue;
  for (iterValue = m_list_value.begin(); iterValue != m_list_value.end(); ++iterValue) {
    CMDF_ValueListValue *pRecordValue = *iterValue;
    if (NULL != pRecordValue) {
      delete pRecordValue;
      pRecordValue = nullptr;
    }
  }
  m_list_value.clear();

  m_strDescription.clear();
  m_strHelpType.clear();
  m_strHelp.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  getRemoteVariableValueType
//

std::string
CMDF_RemoteVariable::getRemoteVariableValueType(void)
{
  switch (m_nType) {

    case type_string:
      return (std::string("String"));

    case type_boolval:
      return (std::string("Boolean"));

    case type_bitfield:
      return (std::string("Bitfield"));

    case type_int8_t:
      return (std::string("Signed 8-bit integer"));

    case type_uint8_t:
      return (std::string("Unsigned 8-bit integer"));

    case type_int16_t:
      return (std::string("Signed 16-bit integer"));

    case type_uint16_t:
      return (std::string("Unsigned 16-bit integer"));

    case type_int32_t:
      return (std::string("Signed 32-bit integer"));

    case type_uint32_t:
      return (std::string("Unsigned 32-bit integer"));

    case type_int64_t:
      return (std::string("Signed 64-bit integer"));

    case type_uint64_t:
      return (std::string("Unsigned 64-bit integer"));

    case type_float:
      return (std::string("float"));

    case type_double:
      return (std::string("double"));

    case type_date:
      return (std::string("Date"));

    case type_time:
      return (std::string("Time"));

    case type_guid:
      return (std::string("GUID"));

    case type_unknown:
    default:
      return (std::string("Unknown Type"));
  }
}

///////////////////////////////////////////////////////////////////////////////
//  getRemoteVariableTypeByteCount
//

uint16_t
CMDF_RemoteVariable::getRemoteVariableTypeByteCount(void)
{
  uint16_t width = 0;

  switch (m_nType) {

    case type_string:
      width = m_nWidth;
      break;

    case type_boolval:
      width = 1;
      break;

    case type_bitfield:
      width = m_nWidth;
      break;

    case type_int8_t:
    case type_uint8_t:
      width = 1;
      break;

    case type_int16_t:
    case type_uint16_t:
      width = 2;
      break;

    case type_int32_t:
    case type_uint32_t:
      width = 4;
      break;

    case type_int64_t:
    case type_uint64_t:
      width = 8;
      break;

    case type_float:
      width = 4;
      break;

    case type_double:
      width = 8;
      break;

    case type_date:
      width = 6;
      break;

    case type_time:
      width = 6;
      break;

    case type_guid:
      width = 16;
      break;

    case type_unknown:
      width = 0;
    default:
      break;

  } // switch

  return width;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Bit::CMDF_Bit()
{
  ;
}

CMDF_Bit::~CMDF_Bit()
{
  clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  ClearStorage
//

void
CMDF_Bit::clearStorage(void)
{
  // Clearup value list
  std::deque<CMDF_ValueListValue *>::iterator iterValue;
  for (iterValue = m_list_value.begin(); iterValue != m_list_value.end(); ++iterValue) {
    CMDF_ValueListValue *pRecordValue = *iterValue;
    if (NULL != pRecordValue) {
      delete pRecordValue;
      pRecordValue = nullptr;
    }
  }
  m_list_value.clear();

  m_strDescription.clear();
  m_strHelpType.clear();
  m_strHelp.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Register::CMDF_Register()
{
  ;
}

CMDF_Register::~CMDF_Register()
{
  clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  ClearStorage
//

void
CMDF_Register::clearStorage(void)
{
  // Clear up bit list
  std::deque<CMDF_Bit *>::iterator iterBit;
  for (iterBit = m_list_bit.begin(); iterBit != m_list_bit.end(); ++iterBit) {
    CMDF_Bit *pRecordBit = *iterBit;
    if (NULL != pRecordBit) {
      delete pRecordBit;
      pRecordBit = nullptr;
    }
  }
  m_list_bit.clear();

  // Clear up value list
  std::deque<CMDF_ValueListValue *>::iterator iterValue;
  for (iterValue = m_list_value.begin(); iterValue != m_list_value.end(); ++iterValue) {
    CMDF_ValueListValue *pRecordValue = *iterValue;
    if (NULL != pRecordValue) {
      delete pRecordValue;
      pRecordValue = nullptr;
    }
  }
  m_list_value.clear();

  m_strDescription.clear();
  m_strHelpType.clear();
  m_strHelp.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  setDefault
//

uint8_t
CMDF_Register::setDefault(void)
{
  if (m_strDefault.npos == m_strDefault.find("UNDEF")) {
    m_value = vscp_readStringValue(m_strDefault);
  }
  else {
    m_value = 0;
  }

  return m_value;
}

///////////////////////////////////////////////////////////////////////////////
//  Assignment
//

CMDF_Register &
CMDF_Register::operator=(const CMDF_Register &other)
{
  m_strName        = other.m_strName;
  m_strDescription = other.m_strDescription;
  m_strHelpType    = other.m_strHelpType;
  m_strHelp        = other.m_strHelp;

  m_nPage   = other.m_nPage;
  m_nOffset = other.m_nOffset;
  m_nWidth  = other.m_nWidth;

  m_type = other.m_type;
  m_size = other.m_size;

  m_nMin = other.m_nMin;
  m_nMax = other.m_nMax;

  m_strDefault = other.m_strDefault;

  m_nAccess = other.m_nAccess;

  m_rowInGrid = other.m_rowInGrid;
  m_value     = other.m_value;

  m_fgcolor = other.m_fgcolor;
  m_bgcolor = other.m_bgcolor;

  // Clear up bit list
  std::deque<CMDF_Bit *>::iterator iterBit;
  for (iterBit = m_list_bit.begin(); iterBit != m_list_bit.end(); ++iterBit) {
    CMDF_Bit *pRecordBit = *iterBit;
    if (NULL != pRecordBit) {
      delete pRecordBit;
      pRecordBit = nullptr;
    }
  }
  m_list_bit.clear();

  // Assign
  m_list_bit = other.m_list_bit;

  // Clear up value list
  std::deque<CMDF_ValueListValue *>::iterator iterValue;
  for (iterValue = m_list_value.begin(); iterValue != m_list_value.end(); ++iterValue) {
    CMDF_ValueListValue *pRecordValue = *iterValue;
    if (NULL != pRecordValue) {
      delete pRecordValue;
      pRecordValue = nullptr;
    }
  }
  m_list_value.clear();

  // Assign
  m_list_value = other.m_list_value;

  return *this;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_ActionParameter::CMDF_ActionParameter()
{
  ;
}

CMDF_ActionParameter::~CMDF_ActionParameter()
{
  clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  ClearStorage
//

void
CMDF_ActionParameter::clearStorage(void)
{
  // Clearup bit list
  std::deque<CMDF_Bit *>::iterator iterBit;
  for (iterBit = m_list_bit.begin(); iterBit != m_list_bit.end(); ++iterBit) {
    CMDF_Bit *pRecordBit = *iterBit;
    if (NULL != pRecordBit) {
      delete pRecordBit;
      pRecordBit = nullptr;
    }
  }
  m_list_bit.clear();

  // Clearup value list
  std::deque<CMDF_ValueListValue *>::iterator iterValue;
  for (iterValue = m_list_value.begin(); iterValue != m_list_value.end(); ++iterValue) {
    CMDF_ValueListValue *pRecordValue = *iterValue;
    if (NULL != pRecordValue) {
      delete pRecordValue;
      pRecordValue = nullptr;
    }
  }
  m_list_value.clear();

  m_strDescription.clear();
  m_strHelpType.clear();
  m_strHelp.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Action::CMDF_Action()
{
  ;
}

CMDF_Action::~CMDF_Action()
{
  clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  ClearStorage
//

void
CMDF_Action::clearStorage(void)
{
  // Cleanup action parameter list
  std::deque<CMDF_ActionParameter *>::iterator iterActionParam;
  for (iterActionParam = m_list_ActionParameter.begin(); iterActionParam != m_list_ActionParameter.end();
       ++iterActionParam) {
    CMDF_ActionParameter *pRecordActionParam = *iterActionParam;
    if (NULL != pRecordActionParam) {
      delete pRecordActionParam;
      pRecordActionParam = nullptr;
    }
  }

  // Delete all nodes
  m_list_ActionParameter.clear();

  m_strDescription.clear();
  m_strHelpType.clear();
  m_strHelp.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_DecisionMatrix::CMDF_DecisionMatrix()
{
  m_nLevel       = 1;
  m_nStartPage   = 0;
  m_nStartOffset = 0;
  m_nRowCount    = 0;
  m_nRowSize     = 8;
  m_bIndexed     = false;
}

CMDF_DecisionMatrix::~CMDF_DecisionMatrix()
{
  clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

void
CMDF_DecisionMatrix::clearStorage()
{
  m_nLevel       = 1;
  m_nStartPage   = 0;
  m_nStartOffset = 0;
  m_nRowCount    = 0;
  m_nRowSize     = 8;

  m_list_action.clear();
  m_list_action.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_EventData::CMDF_EventData()
{
  ;
}

CMDF_EventData::~CMDF_EventData()
{
  clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_EventData::clearStorage()
{
  // Clearup bit list
  std::deque<CMDF_Bit *>::iterator iterBit;
  for (iterBit = m_list_bit.begin(); iterBit != m_list_bit.end(); ++iterBit) {
    CMDF_Bit *pRecordBit = *iterBit;
    if (NULL != pRecordBit) {
      delete pRecordBit;
      pRecordBit = nullptr;
    }
  }
  m_list_bit.clear();

  // Clearup value list
  std::deque<CMDF_ValueListValue *>::iterator iterValue;
  for (iterValue = m_list_value.begin(); iterValue != m_list_value.end(); ++iterValue) {
    CMDF_ValueListValue *pRecordValue = *iterValue;
    if (NULL != pRecordValue) {
      delete pRecordValue;
      pRecordValue = nullptr;
    }
  }
  m_list_value.clear();

  m_strDescription.clear();
  m_strHelpType.clear();
  m_strHelp.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Event::CMDF_Event()
{
  ;
}

CMDF_Event::~CMDF_Event()
{
  clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_Event::clearStorage()
{
  // Cleanup event data
  std::deque<CMDF_EventData *>::iterator iterEventData;
  for (iterEventData = m_list_eventdata.begin(); iterEventData != m_list_eventdata.end(); ++iterEventData) {
    CMDF_EventData *pRecordEventData = *iterEventData;
    if (NULL != pRecordEventData) {
      delete pRecordEventData;
      pRecordEventData = nullptr;
    }
  }
  m_list_eventdata.clear();

  m_strDescription.clear();
  m_strHelpType.clear();
  m_strHelp.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Item::CMDF_Item()
{
  ;
}

CMDF_Item::~CMDF_Item()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_BootLoaderInfo::CMDF_BootLoaderInfo()
{
  ;
}

CMDF_BootLoaderInfo::~CMDF_BootLoaderInfo()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_BootLoaderInfo::clearStorage(void)
{
  m_nAlgorithm  = 0;
  m_nBlockSize  = 0;
  m_nBlockCount = 0;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Address::CMDF_Address() {}

CMDF_Address::~CMDF_Address()
{
  clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_Address::clearStorage(void)
{
  m_strStreet.clear();
  m_strTown.clear();
  m_strCity.clear();
  m_strPostCode.clear();
  m_strState.clear();
  m_strRegion.clear();
  m_strCountry.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Manufacturer::CMDF_Manufacturer()
{
  ;
}

CMDF_Manufacturer::~CMDF_Manufacturer()
{
  clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_Manufacturer::clearStorage(void)
{
  // Cleanup Address list
  std::deque<CMDF_Address *>::iterator iterAddress;
  for (iterAddress = m_list_Address.begin(); iterAddress != m_list_Address.end(); ++iterAddress) {
    CMDF_Address *pRecordAddress = *iterAddress;
    if (NULL != pRecordAddress) {
      delete pRecordAddress;
      pRecordAddress = nullptr;
    }
  }
  m_list_Address.clear();

  // Cleanup Phone list
  std::deque<CMDF_Item *>::iterator iterPhone;
  for (iterPhone = m_list_Phone.begin(); iterPhone != m_list_Phone.end(); ++iterPhone) {
    CMDF_Item *pRecordPhone = *iterPhone;
    if (NULL != pRecordPhone) {
      delete pRecordPhone;
      pRecordPhone = nullptr;
    }
  }
  m_list_Phone.clear();

  // Cleanup Fax list
  std::deque<CMDF_Item *>::iterator iterFax;
  for (iterFax = m_list_Fax.begin(); iterFax != m_list_Fax.end(); ++iterFax) {
    CMDF_Item *pRecordFax = *iterFax;
    if (NULL != pRecordFax) {
      delete pRecordFax;
      pRecordFax = nullptr;
    }
  }
  m_list_Fax.clear();

  // Cleanup mail list
  std::deque<CMDF_Item *>::iterator iterEmail;
  for (iterEmail = m_list_Email.begin(); iterEmail != m_list_Email.end(); ++iterEmail) {
    CMDF_Item *pRecordEmail = *iterEmail;
    if (NULL != pRecordEmail) {
      delete pRecordEmail;
    }
  }
  m_list_Email.clear();

  // Cleanup web list
  std::deque<CMDF_Item *>::iterator iterweb;
  for (iterweb = m_list_Web.begin(); iterweb != m_list_Web.end(); ++iterweb) {
    CMDF_Item *pRecordWeb = *iterweb;
    if (NULL != pRecordWeb) {
      delete pRecordWeb;
      pRecordWeb = nullptr;
    }
  }
  m_list_Web.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Firmware::CMDF_Firmware()
{
  clearStorage();
}

CMDF_Firmware::~CMDF_Firmware() {}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_Firmware::clearStorage(void)
{
  m_strPath.clear();
  m_size             = 0;
  m_version_major    = 0;
  m_version_minor    = 0;
  m_version_subminor = 0;

  m_strDescription.clear();
  m_strHelpType.clear();
  m_strHelp.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF::CMDF()
{
  m_strLocale = "en";
}

CMDF::~CMDF()
{
  clearStorage();
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF::clearStorage(void)
{
  // Clean up manufacturer list
  std::deque<CMDF_Manufacturer *>::iterator iterManufacturer;
  for (iterManufacturer = m_list_manufacturer.begin(); iterManufacturer != m_list_manufacturer.end();
       ++iterManufacturer) {
    CMDF_Manufacturer *pRecordManufacturer = *iterManufacturer;
    if (NULL != pRecordManufacturer) {
      delete pRecordManufacturer;
      pRecordManufacturer = nullptr;
    }
  }
  m_list_manufacturer.clear();

  // Cleanup node event list
  std::deque<CMDF_Event *>::iterator iterEvent;
  for (iterEvent = m_list_event.begin(); iterEvent != m_list_event.end(); ++iterEvent) {
    CMDF_Event *pRecordEvent = *iterEvent;
    if (NULL != pRecordEvent) {
      delete pRecordEvent;
      pRecordEvent = nullptr;
    }
  }
  m_list_event.clear();

  // Clean up register list
  std::deque<CMDF_Register *>::iterator iterRegister;
  for (iterRegister = m_list_register.begin(); iterRegister != m_list_register.end(); ++iterRegister) {
    CMDF_Register *pRecordRegister = *iterRegister;
    if (NULL != pRecordRegister) {
      delete pRecordRegister;
      pRecordRegister = nullptr;
    }
  }
  m_list_register.clear();

  // Clean up RemoteVariable list
  std::deque<CMDF_RemoteVariable *>::iterator iterRemoteVariable;
  for (iterRemoteVariable = m_list_remotevar.begin(); iterRemoteVariable != m_list_remotevar.end();
       ++iterRemoteVariable) {
    CMDF_RemoteVariable *pRecordRemoteVariable = *iterRemoteVariable;
    if (NULL != pRecordRemoteVariable) {
      delete pRecordRemoteVariable;
      pRecordRemoteVariable = nullptr;
    }
  }
  m_list_remotevar.clear();

  // Clear alarm bit list
  std::deque<CMDF_Bit *>::iterator iterAlarmBits;
  for (iterAlarmBits = m_list_alarmbits.begin(); iterAlarmBits != m_list_alarmbits.end(); ++iterAlarmBits) {
    CMDF_Bit *pRecordAlarmBits = *iterAlarmBits;
    if (NULL != pRecordAlarmBits) {
      delete pRecordAlarmBits;
      pRecordAlarmBits = nullptr;
    }
  }
  m_list_alarmbits.clear();

  m_mapModule_Name.clear();
  m_strModule_Model.clear();
  m_strModule_Version.clear();
  m_mapModule_Description.clear();
  m_mapModule_InfoURL.clear();
  m_changeDate.clear();
  m_Module_buffersize = 0;
  m_strURL.clear();
  m_list_manufacturer.clear();
  m_dmInfo.clearStorage();
  m_bootInfo.clearStorage();
  m_list_event.clear();
  m_list_register.clear();
  m_list_remotevar.clear();
  m_list_alarmbits.clear();

  // Clean up firmware list
  std::deque<CMDF_Firmware *>::iterator iterFirmware;
  for (iterFirmware = m_firmware.begin(); iterFirmware != m_firmware.end(); ++iterFirmware) {
    CMDF_Firmware *pRecordFirmware = *iterFirmware;
    if (NULL != pRecordFirmware) {
      pRecordFirmware->clearStorage();
      delete pRecordFirmware;
      pRecordFirmware = nullptr;
    }
  }
  m_firmware.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  downLoadMDF
//

bool
CMDF::downLoadMDF(std::string &remoteFile, std::string &tempFileName)
{
  /* TODO
  char buf[ 64000 ];
  size_t cnt;
  xxFile tempFile;

  tempFileName = wxFileName::CreateTempFileName( "mdf", &tempFile );
  if ( 0 == tempFileName.length() ) {
      return false;
  }

  m_tempFileName = tempFileName;

  // Create and open http stream
  xxURL url( remoteFile );
  if ( xxURL_NOERR != url.GetError() ) {
      return false;
  }

  wxInputStream *in_stream;
  if ( NULL == ( in_stream = url.GetInputStream() ) ) {
      return false;
  }

  do  {

      // Read data from server
      in_stream->Read( buf, sizeof( buf ) );

      // get valid size for buffer
      cnt = in_stream->LastRead();

      // Write buffer to file
      tempFile.Write( buf, cnt );

  } while ( cnt == sizeof( buf ) );

  // we are done with the stream object
  if ( NULL != in_stream ) delete in_stream;

  // Close the file
  tempFile.Close();
*/
  return true;
}

///////////////////////////////////////////////////////////////////////////////
//  load
//

bool
CMDF::load(std::string &remoteFile, bool bLocalFile)
{
  std::string localFile = remoteFile;

  if (remoteFile.npos == remoteFile.find("http://")) {
    std::string str;
    str = "http://";
    str += remoteFile;
    remoteFile = str;
  }

  // Get URL from user if not given
  if (!bLocalFile && !remoteFile.length()) {
    return false;
  }
  // Get filename from user if not given
  else if (bLocalFile && !remoteFile.length()) {
    return false;
  }

  // Must have a path at this point
  if (0 == remoteFile.length()) {
    return false;
  }

  if (!bLocalFile) {

    if (!downLoadMDF(remoteFile, localFile)) {
      return false;
    }
  }

  return parseMDF(localFile);
}

///////////////////////////////////////////////////////////////////////////////
//  mdfDescriptionFormat
//

void
CMDF::mdfDescriptionFormat(std::string &str)
{
  bool bEscape = false;
  bool bSpace  = true; // Remove leading space
  std::string strWork;
  unsigned int i;

  for (i = 0; i < str.length(); i++) {

    if (bSpace && (' ' != str[i]))
      bSpace = false;

    if (bEscape) {

      switch (str[i]) {

        case '\\':
          strWork += str[i];
          break;

        case 'n':
          strWork += '\n';
          bSpace = true; // Don't allow a space to start line
          break;
      }

      bEscape = false;
    }
    else {
      switch (str[i]) {

        case '\n':
        case '\r':
        case '\t':
          break;

        case '\\':
          bEscape = true;
          break;

          // We allow only one space
        case ' ':
          if (!bSpace) {
            strWork += str[i];
            bSpace = true;
          }
          break;

        default:
          strWork += str[i];
      }
    }
  }

  str = strWork;

  int cnt = 0;

  strWork.clear();

  // Break apart line that are longer then 80 chars
  for (i = 0; i < str.length(); i++) {

    if ('\n' == str[i]) {
      cnt = 0;
    }

    cnt++;

    if (cnt > 80) {

      if (' ' == str[i]) {

        // Add line break
        strWork += '\n';

        cnt = 0;
      }
      else {
        strWork += str[i];
      }
    }
    else {
      strWork += str[i];
    }
  }

  str = strWork;
}

//////////////////////////////////////////////////////////////////////////////
// getDeviceHtmlStatusInfo
//

std::string
CMDF::vscp_getDeviceHtmlStatusInfo(const uint8_t *registers, CMDF *pmdf)
{
  static std::string strHTML;
  std::string str;

  strHTML = "<html><body>";
  strHTML += "<h4>Clear text node data</h4>";
  strHTML += "<font color=\"#009900\">";

  strHTML += "nodeid = ";
  str = vscp_str_format("%d", registers[0x91]);
  strHTML += str;
  strHTML += "<br>";

  strHTML += "GUID = ";
  vscp_writeGuidArrayToString(str, registers + 0xd0);
  strHTML += str;
  strHTML += "<br>";

  strHTML += "MDF URL = ";
  char url[33];
  memset(url, 0, sizeof(url));
  memcpy(url, registers + 0xe0, 32);
  str = std::string(url);
  strHTML += str;
  strHTML += "<br>";

  strHTML += "Alarm: ";
  if (registers[0x80]) {
    strHTML += "Yes";
  }
  else {
    strHTML += "No";
  }
  strHTML += "<br>";

  strHTML += "Node Control Flags: ";
  if (registers[0x83] & 0x10) {
    strHTML += "[Register Write Protect] ";
  }
  else {
    strHTML += "[Register Read/Write] ";
  }
  switch ((registers[0x83] & 0xC0) >> 6) {
    case 1:
      strHTML += " [Initialized] ";
      break;
    default:
      strHTML += " [Uninitialized] ";
      break;
  }
  strHTML += "<br>";

  strHTML += "Firmware VSCP conformance : ";
  strHTML += vscp_str_format("%d.%d", registers[0x81], registers[0x82]);
  strHTML += "<br>";

  strHTML += "User ID: ";
  strHTML += vscp_str_format("%d.%d.%d.%d.%d",
                             registers[0x84],
                             registers[0x85],
                             registers[0x86],
                             registers[0x87],
                             registers[0x88]);
  strHTML += "<br>";

  strHTML += "Manufacturer device ID: ";
  strHTML += vscp_str_format("%d.%d.%d.%d", registers[0x89], registers[0x8A], registers[0x8B], registers[0x8C]);
  strHTML += "<br>";

  strHTML += "Manufacturer sub device ID: ";
  strHTML += vscp_str_format("%d.%d.%d.%d", registers[0x8d], registers[0x8e], registers[0x8f], registers[0x90]);
  strHTML += "<br>";

  strHTML += "Page select: ";
  strHTML +=
    vscp_str_format("%d (MSB=%d LSB=%d)", registers[0x92] * 256 + registers[0x93], registers[0x92], registers[0x93]);
  strHTML += "<br>";

  strHTML += "Firmware version: ";
  strHTML += vscp_str_format("%d.%d.%d", registers[0x94], registers[0x95], registers[0x96]);
  strHTML += "<br>";

  strHTML += "Boot loader algorithm: ";
  strHTML += vscp_str_format("%d - ", registers[0x97]);
  switch (registers[0x97]) {

    case 0x00:
      strHTML += "VSCP universal algorithm 0";
      break;

    case 0x01:
      strHTML += "Microchip PIC algorithm 0";
      break;

    case 0x10:
      strHTML += "Atmel AVR algorithm 0";
      break;

    case 0x20:
      strHTML += "NXP ARM algorithm 0";
      break;

    case 0x30:
      strHTML += "ST ARM algorithm 0";
      break;

    default:
      strHTML += "Unknown algorithm.";
      break;
  }

  strHTML += "<br>";

  strHTML += "Buffer size: ";
  strHTML += vscp_str_format("%d bytes. ", registers[0x98]);
  if (!registers[0x98]) {
    strHTML += " ( == default size (8 or 487 bytes) )";
  }
  strHTML += "<br>";

  strHTML += "Number of register pages: ";
  strHTML += vscp_str_format("%d", registers[0x99]);
  if (registers[0x99] > 22) {
    strHTML += " (Note: VSCP Works display max 22 pages.) ";
  }
  strHTML += "<br>";

  // Decision matrix info.
  if (nullptr != pmdf) {

    unsigned char data[8];
    memset(data, 0, 8);

    strHTML += "Decison Matrix: Rows=";
    strHTML += vscp_str_format("%d ", pmdf->m_dmInfo.m_nRowCount);
    strHTML += " Offset=";
    strHTML += vscp_str_format("%d ", pmdf->m_dmInfo.m_nStartOffset);
    strHTML += " Page start=";
    strHTML += vscp_str_format("%d ", pmdf->m_dmInfo.m_nStartPage);
    strHTML += " Row Size=";
    strHTML += vscp_str_format("%d ", pmdf->m_dmInfo.m_nRowSize);
    strHTML += " Level=";
    strHTML += vscp_str_format("%d ", pmdf->m_dmInfo.m_nLevel);
    strHTML += " # actions define =";
    strHTML += vscp_str_format("%d ", pmdf->m_dmInfo.m_list_action.size());
    strHTML += "<br>";
  }
  else {
    strHTML += "No Decision Matrix is available on this device.";
    strHTML += "<br>";
  }

  if (nullptr != pmdf) {

    // MDF Info
    strHTML += "<h1>MDF Information</h1>";

    strHTML += "<font color=\"#009900\">";

    // Manufacturer data
    strHTML += "<b>Module name :</b> ";
    strHTML += pmdf->m_Module_Name["en"]; // TODO Make language aware
    strHTML += "<br>";

    strHTML += "<b>Module model:</b> ";
    strHTML += pmdf->m_strModule_Model; 
    strHTML += "<br>";

    strHTML += "<b>Module version:</b> ";
    strHTML += pmdf->m_strModule_Version;

    strHTML += "<b>Module last change:</b> ";
    strHTML += pmdf->m_changeDate;  
    strHTML += "<br>";

    strHTML += "<b>Module description:</b> ";
    strHTML += pmdf->m_mapModule_Description["en"];  // TODO Make language aware
    strHTML += "<br>";

    strHTML += "<b>Module URL</b> : ";
    strHTML += "<a href=\"";
    strHTML += pmdf->m_mapModule_InfoURL["en"]; // TODO Make language aware
    strHTML += "\">";
    strHTML += pmdf->m_mapModule_InfoURL["en"]; // TODO Make language aware
    strHTML += "</a>";
    strHTML += "<br>";

    std::deque<CMDF_Manufacturer *>::iterator iter;
    for (iter = pmdf->m_list_manufacturer.begin(); iter != pmdf->m_list_manufacturer.end(); ++iter) {

      strHTML += "<hr><br>";

      CMDF_Manufacturer *manufacturer = *iter;
      strHTML += "<b>Manufacturer:</b> ";
      strHTML += manufacturer->m_strName;
      strHTML += "<br>";

      std::deque<CMDF_Address *>::iterator iterAddr;
      for (iterAddr = manufacturer->m_list_Address.begin(); iterAddr != manufacturer->m_list_Address.end();
           ++iterAddr) {

        CMDF_Address *address = *iterAddr;
        strHTML += "<h4>Address</h4>";
        strHTML += "<b>Street:</b> ";
        strHTML += address->m_strStreet;
        strHTML += "<br>";
        strHTML += "<b>Town:</b> ";
        strHTML += address->m_strTown;
        strHTML += "<br>";
        strHTML += "<b>City:</b> ";
        strHTML += address->m_strCity;
        strHTML += "<br>";
        strHTML += "<b>Post Code:</b> ";
        strHTML += address->m_strPostCode;
        strHTML += "<br>";
        strHTML += "<b>State:</b> ";
        strHTML += address->m_strState;
        strHTML += "<br>";
        strHTML += "<b>Region:</b> ";
        strHTML += address->m_strRegion;
        strHTML += "<br>";
        strHTML += "<b>Country:</b> ";
        strHTML += address->m_strCountry;
        strHTML += "<br><br>";
      }

      std::deque<CMDF_Item *>::iterator iterPhone;
      for (iterPhone = manufacturer->m_list_Phone.begin(); iterPhone != manufacturer->m_list_Phone.end(); ++iterPhone) {

        CMDF_Item *phone = *iterPhone;
        strHTML += "<b>Phone:</b> ";
        strHTML += phone->m_strItem;
        strHTML += " ";
        strHTML += phone->m_strDescription["en"]; // TODO
        strHTML += "<br>";
      }

      std::deque<CMDF_Item *>::iterator iterFax;
      for (iterFax = manufacturer->m_list_Fax.begin(); iterFax != manufacturer->m_list_Fax.end(); ++iterFax) {

        CMDF_Item *fax = *iterFax;
        strHTML += "<b>Fax:</b> ";
        strHTML += fax->m_strItem;
        strHTML += " ";
        strHTML += fax->m_strDescription["en"]; // TODO
        strHTML += "<br>";
      }

      std::deque<CMDF_Item *>::iterator iterEmail;
      for (iterEmail = manufacturer->m_list_Email.begin(); iterEmail != manufacturer->m_list_Email.end(); ++iterEmail) {

        CMDF_Item *email = *iterEmail;
        strHTML += "<b>Email:</b> <a href=\"";
        strHTML += email->m_strItem;
        strHTML += "\" >";
        strHTML += email->m_strItem;
        strHTML += "</a> ";
        strHTML += email->m_strDescription["en"]; // TODO
        strHTML += "<br>";
      }

      std::deque<CMDF_Item *>::iterator iterWeb;
      for (iterWeb = manufacturer->m_list_Web.begin(); iterWeb != manufacturer->m_list_Web.end(); ++iterWeb) {

        CMDF_Item *web = *iterWeb;
        strHTML += "<b>Web:</b> <a href=\"";
        strHTML += web->m_strItem;
        strHTML += "\">";
        strHTML += web->m_strItem;
        strHTML += "</a> ";
        strHTML += web->m_strDescription["en"]; // TODO
        strHTML += "<br>";
      }

    } // manufacturer
  }
  else {
    strHTML += "No MDF info available.";
    strHTML += "<br>";
  }

  strHTML += "</font>";
  strHTML += "</body></html>";

  return strHTML;
}

///////////////////////////////////////////////////////////////////////////////
// getModuleName
//

std::string 
CMDF::getModuleName(std::string language)
{
  std::string str = m_mapModule_Name[language];
  if (str.length() == 0) {
    str = m_mapModule_Name["en"];
  }
  return str;
}

///////////////////////////////////////////////////////////////////////////////
// getModuleDescription
//

std::string 
CMDF::getModuleDescription(std::string language)
{
  std::string str = m_mapModule_Description[language];
  if (str.length() == 0) {
    str = m_mapModule_Description["en"];
  }
  return str;
}

///////////////////////////////////////////////////////////////////////////////
// getModuleInfoUrl
//

std::string 
CMDF::getModuleInfoUrl(std::string language)
{
  std::string str = m_mapModule_InfoURL[language];
  if (str.length() == 0) {
    str = m_mapModule_InfoURL["en"];
  }
  return str;
}

// ----------------------------------------------------------------------------

/*
    XML Setup
    =========

    <setup interface="A string that identifies the Ethernet device"
            localmac="The mac address used as the outgoing mac address"
            subaddr="sub address for i/f"
            filter="filter for outgoing traffic"
            mask="mask for outgoing traffic"
    />
*/

// ----------------------------------------------------------------------------

std::string gtoken;
std::string gLastLanguage;

int gdepth_xml_parser = 0;

bool gbVscp   = false;
bool gbModule = false;

void
__startSetupMDFParser(void *data, const char *name, const char **attr)
{
  CMDF *pmdf = (CMDF *) data;
  if (NULL == pmdf) {
    return;
  }

  spdlog::trace("ParseMDF: <--- Start: Tag: {0} Depth: {1}", name, gdepth_xml_parser);

  // Default language
  gLastLanguage = "en";

  // Save the token
  gtoken = name;
  vscp_trim(gtoken);

  if ((0 == strcmp(name, "vscp")) && (0 == gdepth_xml_parser)) {
    gbVscp = true;
  }
  else if (gbVscp && (0 == strcmp(name, "redirect")) && (1 == gdepth_xml_parser)) {
    pmdf->m_redirectUrl = "";
  }
  else if (gbVscp && (0 == strcmp(name, "module")) && (1 == gdepth_xml_parser)) {
    gbModule = true;
  }
  else if (gbModule && (0 == strcmp(name, "description")) && (2 == gdepth_xml_parser)) {
    for (int i = 0; attr[i]; i += 2) {

      std::string attribute = attr[i + 1];
      vscp_trim(attribute);
      vscp_makeLower(attribute);

      if (0 == strcasecmp(attr[i], "lang")) {
        if (!attribute.empty()) {
          gLastLanguage = attribute;
        }
      }
    }
  }
  else if (gbModule && (0 == strcmp(name, "infourl")) && (2 == gdepth_xml_parser)) {
    for (int i = 0; attr[i]; i += 2) {

      std::string attribute = attr[i + 1];
      vscp_trim(attribute);
      vscp_makeLower(attribute);

      if (0 == strcasecmp(attr[i], "lang")) {
        if (!attribute.empty()) {
          gLastLanguage = attribute;
        }
      }
    }
  }

  gdepth_xml_parser++;
}

void
__handleMDFParserData(void *data, const XML_Char *content, int length)
{
  // Get the pointer to the CMDF object
  CMDF *pmdf = (CMDF *) data;
  if (NULL == pmdf) {
    return;
  }

  // Must be some content to work on
  if (!content) {
    return;
  }

  // No use to work without the <vscp> tag
  if (!gbVscp) {
    return;
  }

  std::string strContent = std::string(content, length);
  vscp_trim(strContent);
  if (strContent.empty()) {
    return;
  }

  spdlog::trace("ParseMDF: XML Data: {}", strContent);

  if (gbModule && (3 == gdepth_xml_parser)) { // Module

    if (gtoken == "name") {
      spdlog::trace("ParseMDF: Module name: {0} language: {1}", strContent, gLastLanguage);
      pmdf->m_mapModule_Name[gLastLanguage] = strContent;
    }
    else if (gtoken == "model") {
      spdlog::trace("ParseMDF: Module name: {0}", strContent);
      pmdf->m_strModule_Model = strContent;
    }
    else if (gtoken == "version") {
      spdlog::trace("ParseMDF: Module name: {0}", strContent);
      pmdf->m_strModule_Version = strContent;
    }
    else if (gtoken == "changed") {
      spdlog::trace("ParseMDF: Module Changedate: {0}", strContent);
      pmdf->m_changeDate = strContent;
    }
    else if (gtoken == "description") {
      spdlog::trace("ParseMDF: Module Description: {0} language: {1}", strContent, gLastLanguage);
      pmdf->m_mapModule_Description[gLastLanguage] = strContent;
      spdlog::trace("ParseMDF: Module Description size: {0}", pmdf->m_mapModule_Description.size());
    }
    else if (gtoken == "infourl") {
      spdlog::trace("ParseMDF: Module infoUrl: {0} language: {1}", strContent, gLastLanguage);
      pmdf->m_mapModule_InfoURL[gLastLanguage] = strContent;
    }
    else if (gtoken == "buffersize") {
      spdlog::trace("ParseMDF: Module buffer size: {0}", strContent);
      pmdf->m_Module_buffersize = vscp_readStringValue(strContent);
    }
  }
  // manufacturer, picture, files, manual, boot, registers abstractions/remotevar, alarm, dmatrix, events
  else if (4 == gdepth_xml_parser) {
  }
}

void
__endSetupMDFParser(void *data, const char *name)
{
  spdlog::trace("ParseMDF: ---> End: Tag: {0} Depth: {1}", name, gdepth_xml_parser);

  if ((0 == strcmp(name, "vscp")) && (1 == gdepth_xml_parser)) {
    gbVscp = false;
  }
  else if ((0 == strcmp(name, "redirect")) && (2 == gdepth_xml_parser)) {
    // pmdf->m_redirectUrl = "";
  }
  else if (gbVscp && (0 == strcmp(name, "module")) && (2 == gdepth_xml_parser)) {
    gbModule = false;
  }

  gdepth_xml_parser--;
}


// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//  parseMDF_XML
//

int
CMDF::parseMDF_XML(std::ifstream &ifs)
{
  int rv = VSCP_ERROR_SUCCESS;

  // Init. XML parsing globals
  gdepth_xml_parser = 0;
  gbVscp   = false;
  gbModule = false;
  gLastLanguage = "en";

  // Empty old MDF information
  clearStorage();

  // XML setup
  std::string strSetupXML;

  XML_Parser xmlParser = XML_ParserCreate("UTF-8");
  XML_SetUserData(xmlParser, this);
  XML_SetElementHandler(xmlParser, __startSetupMDFParser, __endSetupMDFParser);
  XML_SetCharacterDataHandler(xmlParser, __handleMDFParserData);

  int bytes_read;
  void *buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);

  while (ifs.good()) {
    ifs.read((char *) buf, XML_BUFF_SIZE);
    bytes_read = ifs.gcount();
    if (bytes_read > 0) {
      if (!XML_ParseBuffer(xmlParser, bytes_read, bytes_read == 0)) {
        spdlog::error("ParseXML: Failed parse XML file at line {0} [{1}].",
                      XML_GetCurrentLineNumber(xmlParser),
                      XML_ErrorString(XML_GetErrorCode(xmlParser)));
        rv = VSCP_ERROR_PARSING;
        break;
      }
    }
  }

  XML_ParserFree(xmlParser);

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  parseMDF_JSON
//

int
CMDF::parseMDF_JSON(std::ifstream &ifs)
{
  int rv = VSCP_ERROR_SUCCESS;

  return rv;
}

// static char *
// ltrim(char *s)
// {
//   while (isspace(*s))
//     s++;
//   return s;
// }

///////////////////////////////////////////////////////////////////////////////
//  parseMDF
//

int
CMDF::parseMDF(std::string &path)
{
  int rv = VSCP_ERROR_ERROR;
  std::ifstream ifs;

  // Check if file exists
  if (!vscp_fileExists(path)) {
    return VSCP_ERROR_INVALID_PATH;
  }

  // Check format
  // ------------
  // If the file is a JSON file we will parse it
  // as JSON else we will parse it as XML. The first
  // character determines the type "{" for JSON or "<"
  // for XML. Whitespace is ignored.

  try {
    ifs.open(path);
  }
  catch (...) {
    spdlog::error("parseMDF: Failed to open file {}", path);
    return VSCP_ERROR_NOT_OPEN;
  }

  size_t pos;
  std::string str;

  while (std::getline(ifs, str)) {
    // if (!ifs.gcount()) {
    //   spdlog::error("parseMDF: Failed to read file {}", path);
    //   ifs.close();
    //   return false;
    // }
    vscp_trim(str);
    if ((pos = str.find('{')) != std::string::npos) {
      spdlog::debug("parseMDF: MDF file format is JSON");
      rv = parseMDF_JSON(ifs);
      ifs.close();
      break;
    }
    else if ((pos = str.find('<')) != std::string::npos) {
      spdlog::debug("parseMDF: MDF file format is XML");
      rv = parseMDF_XML(ifs);
      ifs.close();
      break;
    }
    else {
      rv = VSCP_ERROR_INVALID_SYNTAX;
      spdlog::error("parseMDF: MDF file format not supported");
    }
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getNumberOfRegisters
//

uint32_t
CMDF::getNumberOfRegisters(uint32_t page)
{
  uint32_t nregisters = 0;

  std::deque<CMDF_Register *>::iterator iterValue;
  for (iterValue = m_list_register.begin(); iterValue != m_list_register.end(); ++iterValue) {
    CMDF_Register *pRecordValue = *iterValue;
    if (NULL != pRecordValue) {
      if (page == pRecordValue->m_nPage)
        nregisters++;
    }
  }

  return nregisters;
};

///////////////////////////////////////////////////////////////////////////////
//  getNumberOfPages
//

uint32_t
CMDF::getPages(std::set<long> &pages)
{
  // bool bFound;

  std::deque<CMDF_Register *>::iterator iterValue;
  for (iterValue = m_list_register.begin(); iterValue != m_list_register.end(); ++iterValue) {

    CMDF_Register *pRecordValue = *iterValue;
    if (NULL != pRecordValue) {

      /*bFound = false;
      for ( uint32_t i=0; i<array.Count(); i++ ) {
          uint16_t ttt = array.Item( i );
          if ( -1 == ttt ) continue;
          if ( pRecordValue->m_nPage == ttt ) {
              bFound = true;
              break;
          }
      }*/

      // Add page if not already in set
      if (pages.end() == pages.find(pRecordValue->m_nPage)) {
        pages.insert(pRecordValue->m_nPage);
      }
    }
  }

  return (uint32_t) pages.size();
};

///////////////////////////////////////////////////////////////////////////////
//  getMDFRegister
//

CMDF_Register *
CMDF::getMDFRegister(uint8_t reg, uint16_t page)
{
  std::deque<CMDF_Register *>::iterator iter;
  for (iter = m_list_register.begin(); iter != m_list_register.end(); ++iter) {

    CMDF_Register *preg = *iter;
    if ((reg == preg->m_nOffset) && (page == preg->m_nPage)) {
      return preg;
    }
  }

  return NULL;
}
