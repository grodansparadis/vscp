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
  m_mapDescription.clear();
  m_mapInfoURL.clear();
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

  m_mapDescription.clear();
  m_mapInfoURL.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  getRemoteVariableValueType
//

std::string
CMDF_RemoteVariable::getRemoteVariableValueType(void)
{
  switch (m_type) {

    case remote_variable_type_string:
      return (std::string("String"));

    case remote_variable_type_boolval:
      return (std::string("Boolean"));

    case remote_variable_type_bitfield:
      return (std::string("Bitfield"));

    case remote_variable_type_int8_t:
      return (std::string("Signed 8-bit integer"));

    case remote_variable_type_uint8_t:
      return (std::string("Unsigned 8-bit integer"));

    case remote_variable_type_int16_t:
      return (std::string("Signed 16-bit integer"));

    case remote_variable_type_uint16_t:
      return (std::string("Unsigned 16-bit integer"));

    case remote_variable_type_int32_t:
      return (std::string("Signed 32-bit integer"));

    case remote_variable_type_uint32_t:
      return (std::string("Unsigned 32-bit integer"));

    case remote_variable_type_int64_t:
      return (std::string("Signed 64-bit integer"));

    case remote_variable_type_uint64_t:
      return (std::string("Unsigned 64-bit integer"));

    case remote_variable_type_float:
      return (std::string("float"));

    case remote_variable_type_double:
      return (std::string("double"));

    case remote_variable_type_date:
      return (std::string("Date"));

    case remote_variable_type_time:
      return (std::string("Time"));

    case remote_variable_type_guid:
      return (std::string("GUID"));

    case remote_variable_type_unknown:
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

  switch (m_type) {

    case remote_variable_type_string:
      width = m_width;
      break;

    case remote_variable_type_boolval:
      width = 1;
      break;

    case remote_variable_type_bitfield:
      width = m_width;
      break;

    case remote_variable_type_int8_t:
    case remote_variable_type_uint8_t:
      width = 1;
      break;

    case remote_variable_type_int16_t:
    case remote_variable_type_uint16_t:
      width = 2;
      break;

    case remote_variable_type_int32_t:
    case remote_variable_type_uint32_t:
      width = 4;
      break;

    case remote_variable_type_int64_t:
    case remote_variable_type_uint64_t:
      width = 8;
      break;

    case remote_variable_type_float:
      width = 4;
      break;

    case remote_variable_type_double:
      width = 8;
      break;

    case remote_variable_type_date:
      width = 6;
      break;

    case remote_variable_type_time:
      width = 6;
      break;

    case remote_variable_type_guid:
      width = 16;
      break;

    case remote_variable_type_unknown:
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

  m_mapDescription.clear();
  m_mapInfoURL.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Register::CMDF_Register()
{
  m_page = 0;
  m_offset = 0;
  m_width = 8;
  m_min = 0;
  m_max = 255;
  m_strDefault = "UNDEF";
  m_access = MDF_REG_ACCESS_READ_WRITE;
  m_type = REGISTER_TYPE_STANDARD;
  m_size = 1;

  m_rowInGrid = 0;
  m_value = 0;
  m_bgcolor = 0;
  m_fgcolor = 0;
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

  m_mapDescription.clear();
  m_mapInfoURL.clear();
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
  m_mapDescription = other.m_mapDescription;
  m_mapInfoURL     = other.m_mapInfoURL;

  m_page   = other.m_page;
  m_offset = other.m_offset;
  m_width  = other.m_width;

  m_type = other.m_type;
  m_size = other.m_size;

  m_min = other.m_min;
  m_max = other.m_max;

  m_strDefault = other.m_strDefault;

  m_access = other.m_access;

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

  m_mapDescription.clear();
  m_mapInfoURL.clear();
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

  m_mapDescription.clear();
  m_mapInfoURL.clear();
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

  m_mapDescription.clear();
  m_mapInfoURL.clear();
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

  m_mapDescription.clear();
  m_mapInfoURL.clear();
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
  // std::deque<CMDF_Address *>::iterator iterAddress;
  // for (iterAddress = m_list_Address.begin(); iterAddress != m_list_Address.end(); ++iterAddress) {
  //   CMDF_Address *pRecordAddress = *iterAddress;
  //   if (NULL != pRecordAddress) {
  //     delete pRecordAddress;
  //     pRecordAddress = nullptr;
  //   }
  // }
  // m_list_Address.clear();

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

CMDF_Picture::CMDF_Picture()
{
  clearStorage();
}

CMDF_Picture::~CMDF_Picture()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_Picture::clearStorage(void)
{
  m_strURL.clear();
  m_strFormat.clear();
  m_mapDescription.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Firmware::CMDF_Firmware()
{
  clearStorage();
}

CMDF_Firmware::~CMDF_Firmware()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_Firmware::clearStorage(void)
{
  m_strURL.clear();
  m_strTarget.clear();
  m_strDate.clear();
  m_strFormat.clear();
  m_strMd5.clear();

  m_targetCode    = 0;
  m_size          = 0;
  m_version_major = 0;
  m_version_minor = 0;
  m_version_patch = 0;

  m_mapDescription.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Manual::CMDF_Manual()
{
  clearStorage();
}

CMDF_Manual::~CMDF_Manual()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_Manual::clearStorage(void)
{
  m_strURL.clear();
  m_strFormat.clear();
  m_mapDescription.clear();
}

// ----------------------------------------------------------------------------

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
  for (iterAlarmBits = m_list_alarm.begin(); iterAlarmBits != m_list_alarm.end(); ++iterAlarmBits) {
    CMDF_Bit *pRecordAlarmBits = *iterAlarmBits;
    if (NULL != pRecordAlarmBits) {
      delete pRecordAlarmBits;
      pRecordAlarmBits = nullptr;
    }
  }
  m_list_alarm.clear();

  m_name = "";
  m_strModule_Model.clear();
  m_strModule_Version.clear();
  m_mapDescription.clear();
  m_mapInfoURL.clear();
  m_strModule_changeDate.clear();
  m_module_bufferSize = 0;
  m_strURL.clear();
  // m_list_manufacturer.clear();
  m_dmInfo.clearStorage();
  m_bootInfo.clearStorage();
  m_list_event.clear();
  m_list_register.clear();
  m_list_remotevar.clear();
  m_list_alarm.clear();

  // Clean up picture list
  std::deque<CMDF_Picture *>::iterator iterPicture;
  for (iterPicture = m_list_picture.begin(); iterPicture != m_list_picture.end(); ++iterPicture) {
    CMDF_Picture *pRecordPicture = *iterPicture;
    if (NULL != pRecordPicture) {
      pRecordPicture->clearStorage();
      delete pRecordPicture;
      pRecordPicture = nullptr;
    }
  }
  m_list_picture.clear();

  // Clean up firmware list
  std::deque<CMDF_Firmware *>::iterator iterFirmware;
  for (iterFirmware = m_list_firmware.begin(); iterFirmware != m_list_firmware.end(); ++iterFirmware) {
    CMDF_Firmware *pRecordFirmware = *iterFirmware;
    if (NULL != pRecordFirmware) {
      pRecordFirmware->clearStorage();
      delete pRecordFirmware;
      pRecordFirmware = nullptr;
    }
  }
  m_list_firmware.clear();

  // Clean up manual list
  std::deque<CMDF_Manual *>::iterator iterManual;
  for (iterManual = m_list_manual.begin(); iterManual != m_list_manual.end(); ++iterManual) {
    CMDF_Manual *pRecordManual = *iterManual;
    if (NULL != pRecordManual) {
      pRecordManual->clearStorage();
      delete pRecordManual;
      pRecordManual = nullptr;
    }
  }
  m_list_manual.clear();
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

/*  TODO
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
    strHTML += pmdf->m_mapModule_Name["en"]; // TODO Make language aware
    strHTML += "<br>";

    strHTML += "<b>Module model:</b> ";
    strHTML += pmdf->m_strModule_Model;
    strHTML += "<br>";

    strHTML += "<b>Module version:</b> ";
    strHTML += pmdf->m_strModule_Version;

    strHTML += "<b>Module last change:</b> ";
    strHTML += pmdf->m_strModule_changeDate;
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

    //std::deque<CMDF_Manufacturer *>::iterator iter;
    //for (iter = pmdf->m_list_manufacturer.begin(); iter != pmdf->m_list_manufacturer.end(); ++iter) {

    strHTML += "<hr><br>";

    //CMDF_Manufacturer *manufacturer = *iter;
    strHTML += "<b>Manufacturer:</b> ";
    strHTML += m_manufacturer->m_strName;
    strHTML += "<br>";

    //std::deque<CMDF_Address *>::iterator iterAddr;
    // for (iterAddr = pmdf->m_manufacturer->m_address.begin(); iterAddr != manufacturer->m_list_Address.end();
    //      ++iterAddr) {

    //CMDF_Address *address = *iterAddr;
    strHTML += "<h4>Address</h4>";
    strHTML += "<b>Street:</b> ";
    strHTML += m_manufacturer->m_address->m_strStreet;
    strHTML += "<br>";
    strHTML += "<b>Town:</b> ";
    strHTML += m_manufacturer->m_address->m_strTown;
    strHTML += "<br>";
    strHTML += "<b>City:</b> ";
    strHTML += m_manufacturer->m_address->m_strCity;
    strHTML += "<br>";
    strHTML += "<b>Post Code:</b> ";
    strHTML += m_manufacturer->m_address->m_strPostCode;
    strHTML += "<br>";
    strHTML += "<b>State:</b> ";
    strHTML += m_manufacturer->m_address->m_strState;
    strHTML += "<br>";
    strHTML += "<b>Region:</b> ";
    strHTML += m_manufacturer->m_address->m_strRegion;
    strHTML += "<br>";
    strHTML += "<b>Country:</b> ";
    strHTML += m_manufacturer->m_address->m_strCountry;
    strHTML += "<br><br>";
      //}

    std::deque<CMDF_Item *>::iterator iterPhone;
    for (iterPhone = m_manufacturer->m_list_Phone.begin();
          iterPhone != m_manufacturer->m_list_Phone.end();
          ++iterPhone) {

    CMDF_Item *phone = *iterPhone;
    strHTML += "<b>Phone:</b> ";
    strHTML += phone->m_strItem;
    strHTML += " ";
    strHTML += phone->m_mapDescription["en"]; // TODO
    strHTML += "<br>";
      //}

    std::deque<CMDF_Item *>::iterator iterFax;
    for (iterFax = m_manufacturer->m_list_Fax.begin();
          iterFax != m_manufacturer->m_list_Fax.end();
          ++iterFax) {

      CMDF_Item *fax = *iterFax;
      strHTML += "<b>Fax:</b> ";
      strHTML += fax->m_strItem;
      strHTML += " ";
      strHTML += fax->m_mapDescription["en"]; // TODO
      strHTML += "<br>";
    }

    std::deque<CMDF_Item *>::iterator iterEmail;
    for (iterEmail = m_manufacturer->m_list_Email.begin();
          iterEmail != m_manufacturer->m_list_Email.end();
          ++iterEmail) {

      CMDF_Item *email = *iterEmail;
      strHTML += "<b>Email:</b> <a href=\"";
      strHTML += email->m_strItem;
      strHTML += "\" >";
      strHTML += email->m_strItem;
      strHTML += "</a> ";
      strHTML += email->m_mapDescription["en"]; // TODO
      strHTML += "<br>";
    }

    std::deque<CMDF_Item *>::iterator iterWeb;
    for (iterWeb = m_manufacturer->m_list_Web.begin();
          iterWeb != m_manufacturer->m_list_Web.end();
          ++iterWeb) {

      CMDF_Item *web = *iterWeb;
      strHTML += "<b>Web:</b> <a href=\"";
      strHTML += web->m_strItem;
      strHTML += "\">";
      strHTML += web->m_strItem;
      strHTML += "</a> ";
      strHTML += web->m_mapDescription["en"]; // TODO
      strHTML += "<br>";
    }

    //} // manufacturer
  }
  else {
    strHTML += "No MDF info available.";
    strHTML += "<br>";
  }

  strHTML += "</font>";
  strHTML += "</body></html>";

  return strHTML;
}
*/



///////////////////////////////////////////////////////////////////////////////
// getModuleDescription
//

std::string
CMDF::getModuleDescription(std::string language)
{
  std::string str = m_mapDescription[language];
  if (str.length() == 0) {
    str = m_mapDescription["en"];
  }
  return str;
}

///////////////////////////////////////////////////////////////////////////////
// getModuleHelpUrl
//

std::string
CMDF::getModuleHelpUrl(std::string language)
{
  std::string str = m_mapInfoURL[language];
  if (str.length() == 0) {
    str = m_mapInfoURL["en"];
  }
  return str;
}

///////////////////////////////////////////////////////////////////////////////
// getManufacturerName
//

std::string
CMDF::getManufacturerName(uint8_t index)
{
  return m_manufacturer.m_strName;
}

///////////////////////////////////////////////////////////////////////////////
// getManufacturerStreetAddress
//

std::string
CMDF::getManufacturerStreetAddress(uint8_t index)
{
  return m_manufacturer.m_address.m_strStreet;
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

std::string gToken;        // Current token
std::string gTokenParent;  // This is the parent to the current token
std::string gLastLanguage; // Last language ISO two diget code (name/description)

int gdepth_xml_parser = 0;

bool gbVscp   = false;
bool gbModule = false;

CMDF_Item *gpItemStruct;         // Holds temporary items
CMDF_Picture *gpPictureStruct;   // Holds temporary picture items
CMDF_Firmware *gpFirmwareStruct; // Holds temporary firmware items
CMDF_Manual *gpManualStruct;     // Holds temporary manual items

// ----------------------------------------------------------------------------
void
__startSetupMDFParser(void *data, const char *name, const char **attr)
{
  CMDF *pmdf = (CMDF *) data;
  if (NULL == pmdf) {
    spdlog::trace("ParseMDF: ---> startSetupMDFParser: Data object is invalid");
    return;
  }

  spdlog::trace("ParseMDF: <--- startSetupMDFParser: Tag: {0} Depth: {1}", name, gdepth_xml_parser);

  gToken = name;
  vscp_trim(gToken);

  // Default language
  gLastLanguage = "en";

  // Set language for 'name', 'infourl' and 'description'
  if ((0 == strcmp(name, "name")) || (0 == strcmp(name, "description")) || (0 == strcmp(name, "infourl"))) {
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

  switch (gdepth_xml_parser) {

    case 0: // Root
      if (0 == strcmp(name, "vscp")) {
        gbVscp = true;
      }
      break;

    case 1:
      if (gbVscp && (0 == strcmp(name, "module"))) {
        gTokenParent = "module";
        gbModule     = true;
      }
      break;

    case 2:
      if (gbModule && (0 == strcmp(name, "manufacturer"))) {
        gTokenParent = "manufacturer";
      }
      else if (gbModule && (0 == strcmp(name, "files"))) {
        gTokenParent = "files";
        // picture/firmware/manual entries under here
      }
      // * * * NOTE! * * *
      // This is an old deprecated form with one picture element
      // Now under the <files> tag
      else if (gbModule && (0 == strcmp(name, "picture"))) {

        gTokenParent = "picture";

        gpPictureStruct = new CMDF_Picture;
        if (nullptr == gpPictureStruct) {
          spdlog::error("ParseMDF: ---> startSetupMDFParser: Failed to allocate memory for picture");
          return;
        }
        pmdf->m_list_picture.push_back(gpPictureStruct);

        for (int i = 0; attr[i]; i += 2) {
          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);
          if ((0 == strcasecmp(attr[i], "path")) || (0 == strcasecmp(attr[i], "url"))) {
            if (!attribute.empty()) {
              gpPictureStruct->m_strURL = attribute;
            }
          }
          else if (0 == strcasecmp(attr[i], "format")) {
            if (!attribute.empty()) {
              gpPictureStruct->m_strFormat = attribute;
            }
          }
        }
      }
      // * * * NOTE! * * *
      // This is an old deprecated form with one firmware element
      // Now under the <files> tag
      else if (gbModule && (0 == strcmp(name, "firmware"))) {

        gTokenParent     = "firmware";
        gpFirmwareStruct = new CMDF_Firmware;
        if (nullptr == gpFirmwareStruct) {
          spdlog::error("ParseMDF: ---> startSetupMDFParser: Failed to allocate memory for firmware");
          return;
        }

        pmdf->m_list_firmware.push_back(gpFirmwareStruct);

        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "target")) {
            // Target for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware target: {0}", attribute);
            gpFirmwareStruct->m_strTarget = attribute;
          }
          else if (0 == strcasecmp(attr[i], "targetcode")) {
            // Target for for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware target code: {0}", attribute);
            gpFirmwareStruct->m_targetCode = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "version_major")) {
            // Version for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware version major: {0}", attribute);
            gpFirmwareStruct->m_version_major = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "version_minor")) {
            // Version for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware version minor: {0}", attribute);
            gpFirmwareStruct->m_version_minor = vscp_readStringValue(attribute);
          }
          else if ((0 == strcasecmp(attr[i], "version_patch")) || (0 == strcasecmp(attr[i], "version_subminor"))) {
            // Version for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware version patch: {0}", attribute);
            gpFirmwareStruct->m_version_patch = vscp_readStringValue(attribute);
          }
          else if ((0 == strcasecmp(attr[i], "url")) || (0 == strcasecmp(attr[i], "path"))) {
            // URL for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware url: {0}", attribute);
            gpFirmwareStruct->m_strURL = attribute;
          }
          else if (0 == strcasecmp(attr[i], "md5sum")) {
            // MD5 for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware md5: {0}", attribute);
            gpFirmwareStruct->m_strMd5 = attribute;
          }
          else if (0 == strcasecmp(attr[i], "size")) {
            // Size for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware size: {0}", attribute);
            gpFirmwareStruct->m_size = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "date")) {
            // Date for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware date: {0}", attribute);
            gpFirmwareStruct->m_strDate = attribute;
          }
          else if (0 == strcasecmp(attr[i], "format")) {
            // Date for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware date: {0}", attribute);
            gpFirmwareStruct->m_strFormat = attribute;
          }
        }
      }
      // * * * NOTE! * * *
      // This is an old deprecated form with one manual element
      // Now under the <files> tag
      else if (gbModule && (0 == strcmp(name, "manual"))) {
        gTokenParent   = "manual";
        gpManualStruct = new CMDF_Manual;
        if (nullptr == gpManualStruct) {
          spdlog::error("ParseMDF: ---> startSetupMDFParser: Failed to allocate memory for manual");
          return;
        }

        pmdf->m_list_manual.push_back(gpManualStruct);

        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if ((0 == strcasecmp(attr[i], "path")) || (0 == strcasecmp(attr[i], "url"))) {
            // Path/url for manual
            spdlog::trace("ParseMDF: handleMDFParserData: Module manual path/url: {0}", attribute);
            gpManualStruct->m_strURL = attribute;
          }
          else if (0 == strcasecmp(attr[i], "lang")) {
            // Language for manual
            spdlog::trace("ParseMDF: handleMDFParserData: Module manual language: {0}", attribute);
            gpManualStruct->m_strLanguage = attribute;
          }
          else if (0 == strcasecmp(attr[i], "format")) {
            // Format for manual
            spdlog::trace("ParseMDF: handleMDFParserData: Module manual format: {0}", attribute);
            gpManualStruct->m_strFormat = attribute;
          }
        }
      }
      else if (gbModule && (0 == strcmp(name, "boot"))) {
        gTokenParent = "boot";
      }
      else if (gbModule && (0 == strcmp(name, "registers"))) {
        gTokenParent = "register";
      }
      else if (gbModule && (0 == strcmp(name, "abstractions"))) {
        gTokenParent = "remotevar";
      }
      else if (gbModule && (0 == strcmp(name, "remotevar"))) {
        gTokenParent = "remotevar";
      }
      else if (gbModule && (0 == strcmp(name, "dmatrix"))) {
        gTokenParent = "dmatrix";
      }
      else if (gbModule && (0 == strcmp(name, "events"))) {
        gTokenParent = "events";
      }
      break;

    case 3:
      if ((0 == strcmp(name, "address"))) {
        gTokenParent = "address";
      }
      else if ((0 == strcmp(name, "telephone"))) {
        gTokenParent = "telephone";
        gpItemStruct = new CMDF_Item;
      }
      else if ((0 == strcmp(name, "fax"))) {
        gTokenParent = "fax";
        gpItemStruct = new CMDF_Item;
      }
      else if ((0 == strcmp(name, "email"))) {
        gTokenParent = "email";
        gpItemStruct = new CMDF_Item;
      }
      else if ((0 == strcmp(name, "web"))) {
        gTokenParent = "web";
        gpItemStruct = new CMDF_Item;
      }
      // Picture (standard format)
      else if ((0 == strcmp(name, "picture"))) {

        gTokenParent = "picture";

        gpPictureStruct = new CMDF_Picture;
        if (nullptr == gpPictureStruct) {
          spdlog::error("ParseMDF: ---> startSetupMDFParser: Failed to allocate memory for picture");
          return;
        }

        pmdf->m_list_picture.push_back(gpPictureStruct);

        for (int i = 0; attr[i]; i += 2) {
          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);
          if ((0 == strcasecmp(attr[i], "path")) || (0 == strcasecmp(attr[i], "url"))) {
            if (!attribute.empty()) {
              gpPictureStruct->m_strURL = attribute;
            }
          }
          else if (0 == strcasecmp(attr[i], "format")) {
            if (!attribute.empty()) {
              gpPictureStruct->m_strFormat = attribute;
            }
          }
        }
      }
      // Firmware  (standard format)
      else if ((0 == strcmp(name, "firmware"))) {

        gTokenParent     = "firmware";
        gpFirmwareStruct = new CMDF_Firmware;
        if (nullptr == gpFirmwareStruct) {
          spdlog::error("ParseMDF: ---> startSetupMDFParser: Failed to allocate memory for firmware");
          return;
        }

        pmdf->m_list_firmware.push_back(gpFirmwareStruct);

        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "target")) {
            // Target for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware target: {0}", attribute);
            gpFirmwareStruct->m_strTarget = attribute;
          }
          else if (0 == strcasecmp(attr[i], "targetcode")) {
            // Target for for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware target code: {0}", attribute);
            gpFirmwareStruct->m_targetCode = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "version_major")) {
            // Version for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware version major: {0}", attribute);
            gpFirmwareStruct->m_version_major = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "version_minor")) {
            // Version for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware version minor: {0}", attribute);
            gpFirmwareStruct->m_version_minor = vscp_readStringValue(attribute);
          }
          else if ((0 == strcasecmp(attr[i], "version_patch")) || (0 == strcasecmp(attr[i], "version_subminor"))) {
            // Version for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware version patch: {0}", attribute);
            gpFirmwareStruct->m_version_patch = vscp_readStringValue(attribute);
          }
          else if ((0 == strcasecmp(attr[i], "url")) || (0 == strcasecmp(attr[i], "path"))) {
            // URL for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware url: {0}", attribute);
            gpFirmwareStruct->m_strURL = attribute;
          }
          else if (0 == strcasecmp(attr[i], "md5sum")) {
            // MD5 for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware md5: {0}", attribute);
            gpFirmwareStruct->m_strMd5 = attribute;
          }
          else if (0 == strcasecmp(attr[i], "size")) {
            // Size for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware size: {0}", attribute);
            gpFirmwareStruct->m_size = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "date")) {
            // Date for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware date: {0}", attribute);
            gpFirmwareStruct->m_strDate = attribute;
          }
          else if (0 == strcasecmp(attr[i], "format")) {
            // Date for firmware
            spdlog::trace("ParseMDF: handleMDFParserData: Module firmware date: {0}", attribute);
            gpFirmwareStruct->m_strFormat = attribute;
          }
        }
      }
      // Manual  (standard format)
      else if ((0 == strcmp(name, "manual"))) {
        gTokenParent   = "manual";
        gpManualStruct = new CMDF_Manual;
        if (nullptr == gpManualStruct) {
          spdlog::error("ParseMDF: ---> startSetupMDFParser: Failed to allocate memory for manual");
          return;
        }

        pmdf->m_list_manual.push_back(gpManualStruct);

        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if ((0 == strcasecmp(attr[i], "path")) || (0 == strcasecmp(attr[i], "url"))) {
            // Path/url for manual
            spdlog::trace("ParseMDF: handleMDFParserData: Module manual path/url: {0}", attribute);
            gpManualStruct->m_strURL = attribute;
          }
          else if (0 == strcasecmp(attr[i], "lang")) {
            // Language for manual
            spdlog::trace("ParseMDF: handleMDFParserData: Module manual language: {0}", attribute);
            gpManualStruct->m_strLanguage = attribute;
          }
          else if (0 == strcasecmp(attr[i], "format")) {
            // Format for manual
            spdlog::trace("ParseMDF: handleMDFParserData: Module manual format: {0}", attribute);
            gpManualStruct->m_strFormat = attribute;
          }
        }
      }
      // Boot  (standard format)
      else if ((0 == strcmp(name, "boot"))) {
        gTokenParent = "boot";
        // gpBootStruct = new CMDF_Boot;
      }
      break;
  }

  gdepth_xml_parser++;
}

// ----------------------------------------------------------------------------

void
__handleMDFParserData(void *data, const XML_Char *content, int length)
{
  // Get the pointer to the CMDF object
  CMDF *pmdf = (CMDF *) data;
  if (NULL == pmdf) {
    spdlog::error("ParseMDF: ---> handleMDFParserData: Data object is invalid");
    return;
  }

  // Must be some content to work on
  if (!content) {
    spdlog::error("ParseMDF: ---> handleMDFParserData: No content");
    return;
  }

  // No use to work without the <vscp> tag
  if (!gbVscp) {
    spdlog::error("ParseMDF: ---> handleMDFParserData: No vscp tag");
    return;
  }

  std::string strContent = std::string(content, length);
  vscp_trim(strContent);
  if (strContent.empty()) {
    return;
  }

  spdlog::trace("ParseMDF: XML Data: {}", strContent);

  switch (gdepth_xml_parser) {

    case 1: // On module level
      if (gToken == "redirect") {
        spdlog::trace("ParseMDF: handleMDFParserData: Module redirect: {0}", strContent);
        pmdf->m_redirectUrl = strContent;
      }
      else if (gToken == "name") {
        spdlog::trace("ParseMDF: handleMDFParserData: Module name: {0} language: {1}", strContent);
        pmdf->m_name = strContent;
      }
      break;

    case 2:
      break;

    case 3:
      if (gToken == "name") {
        spdlog::trace("ParseMDF: handleMDFParserData: Module name: {0} language: {1}", strContent);
        pmdf->m_name = strContent;
      }
      else if (gToken == "model") {
        spdlog::trace("ParseMDF: handleMDFParserData: Module name: {0}", strContent);
        pmdf->m_strModule_Model = strContent;
      }
      else if (gToken == "version") {
        spdlog::trace("ParseMDF: handleMDFParserData: Module name: {0}", strContent);
        pmdf->m_strModule_Version = strContent;
      }
      else if (gToken == "changed") {
        spdlog::trace("ParseMDF: handleMDFParserData: Module Changedate: {0}", strContent);
        pmdf->m_strModule_changeDate = strContent;
      }
      else if (gToken == "description") {
        if (gTokenParent == "module") {
          spdlog::trace("ParseMDF: handleMDFParserData: Module Description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          pmdf->m_mapDescription[gLastLanguage] = strContent;
          spdlog::trace("ParseMDF: handleMDFParserData: Module Description size: {0}",
                        pmdf->m_mapDescription.size());
        }
      }
      else if (gToken == "infourl") {
        spdlog::trace("ParseMDF: handleMDFParserData: Module infoUrl: {0} language: {1}", strContent, gLastLanguage);
        pmdf->m_mapInfoURL[gLastLanguage] = strContent;
      }
      else if (gToken == "buffersize") {
        spdlog::trace("ParseMDF: handleMDFParserData: Module buffer size: {0}", strContent);
        pmdf->m_module_bufferSize = vscp_readStringValue(strContent);
      }
      break;

    case 4: // manufacturer, picture, files, manual, boot, registers abstractions/remotevar, alarm, dmatrix, events
      if ((gTokenParent == "manufacturer") && (gToken == "name")) {
        // Name of manufacturer
        spdlog::trace("ParseMDF: handleMDFParserData: Module manufacturer name: {0} language: {1}",
                      strContent,
                      gLastLanguage);
        pmdf->m_manufacturer.m_strName = strContent;
      }
      // Old form of picture
      else if (gTokenParent == "picture" && (gToken == "description")) {
        // Picture description
        if (gpPictureStruct != nullptr) {
          spdlog::trace("ParseMDF: handleMDFParserData: Picture Description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpPictureStruct->m_mapDescription[gLastLanguage] = strContent;
          spdlog::trace("ParseMDF: handleMDFParserData: Picture  Description size: {0}",
                        gpPictureStruct->m_mapDescription.size());
        }
      }
      // Old form of firmware
      else if (gTokenParent == "firmware" && (gToken == "description")) {
        // Picture description
        if (gpFirmwareStruct != nullptr) {
          spdlog::trace("ParseMDF: handleMDFParserData: Firmware Description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpFirmwareStruct->m_mapDescription[gLastLanguage] = strContent;
          spdlog::trace("ParseMDF: handleMDFParserData: Firmware  Description size: {0}",
                        gpFirmwareStruct->m_mapDescription.size());
        }
      }
      // Old form of manual
      else if (gTokenParent == "manual" && (gToken == "description")) {
        // Picture description
        if (gpManualStruct != nullptr) {
          spdlog::trace("ParseMDF: handleMDFParserData: Manual Description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpManualStruct->m_mapDescription[gLastLanguage] = strContent;
          spdlog::trace("ParseMDF: handleMDFParserData: Manual Description size: {0}",
                        gpManualStruct->m_mapDescription.size());
        }
      }
      else if (gTokenParent == "boot" && (gToken == "algorithm")) {
        pmdf->m_bootInfo.m_nAlgorithm = vscp_readStringValue(strContent);
      }
      else if (gTokenParent == "boot" && (gToken == "blocksize")) {
        pmdf->m_bootInfo.m_nBlockSize = vscp_readStringValue(strContent);
      }
      else if (gTokenParent == "boot" && (gToken == "blockcount")) {
        pmdf->m_bootInfo.m_nBlockCount = vscp_readStringValue(strContent);
      }
      break;

    case 5:

      // manufacturer: address, telephone, fax, email, web

      // manufacturer: address
      if (gTokenParent == "address") {
        if (gToken == "street") {
          spdlog::trace("ParseMDF: handleMDFParserData: Module manufacturer address street: {0}", strContent);
          pmdf->m_manufacturer.m_address.m_strStreet = strContent;
        }
        else if (gToken == "city") {
          spdlog::trace("ParseMDF: handleMDFParserData: Module manufacturer address city: {0}", strContent);
          pmdf->m_manufacturer.m_address.m_strCity = strContent;
        }
        else if (gToken == "town") {
          spdlog::trace("ParseMDF: handleMDFParserData: Module manufacturer address town: {0}", strContent);
          pmdf->m_manufacturer.m_address.m_strTown = strContent;
        }
        else if (gToken == "postcode") {
          spdlog::trace("ParseMDF: handleMDFParserData: Module manufacturer address postcode: {0}", strContent);
          pmdf->m_manufacturer.m_address.m_strPostCode = strContent;
        }
        else if (gToken == "country") {
          spdlog::trace("ParseMDF: handleMDFParserData: Module manufacturer address country: {0}", strContent);
          pmdf->m_manufacturer.m_address.m_strCountry = strContent;
        }
        else if (gToken == "region") {
          spdlog::trace("ParseMDF: handleMDFParserData: Module manufacturer address region: {0}", strContent);
          pmdf->m_manufacturer.m_address.m_strRegion = strContent;
        }
      }
      // manufacturer/telephone
      else if (gTokenParent == "telephone") {
        if ((gToken == "number") && (gpItemStruct != nullptr)) {
          spdlog::trace("ParseMDF: handleMDFParserData: Module manufacturer address telephone number: {0}", strContent);
          gpItemStruct->m_strItem = strContent;
        }
        else if (gToken == "description") {
          spdlog::trace(
            "ParseMDF: handleMDFParserData: Module manufacturer address telephone description: {0} Language: {1}",
            strContent,
            gLastLanguage);
          gpItemStruct->m_mapDescription[gLastLanguage] = strContent;
        }
      }
      // manufacturer/fax
      else if ((gTokenParent == "fax") && (gpItemStruct != NULL)) {
        if (gToken == "number") {
          spdlog::trace("ParseMDF: handleMDFParserData: Module manufacturer address fax number: {0}", strContent);
          gpItemStruct->m_strItem = strContent;
        }
        else if (gToken == "description") {
          spdlog::trace("ParseMDF: handleMDFParserData: Module manufacturer address fax description: {0} Language: {1}",
                        strContent,
                        gLastLanguage);
          gpItemStruct->m_mapDescription[gLastLanguage] = strContent;
        }
      }
      // manufacturer/email
      else if ((gTokenParent == "email") && (gpItemStruct != NULL)) {
        if (gToken == "address") {
          spdlog::trace("ParseMDF: handleMDFParserData: Module manufacturer email address: {0}", strContent);
          gpItemStruct->m_strItem = strContent;
        }
        else if (gToken == "description") {
          spdlog::trace(
            "ParseMDF: handleMDFParserData: Module manufacturer email address description: {0} Language: {1}",
            strContent,
            gLastLanguage);
          gpItemStruct->m_mapDescription[gLastLanguage] = strContent;
        }
      }
      // manufacturer/web
      else if ((gTokenParent == "email") && (gpItemStruct != NULL)) {
        if ((gToken == "address") || (gToken == "url")) {
          spdlog::trace("ParseMDF: handleMDFParserData: Module manufacturer email address: {0}", strContent);
          gpItemStruct->m_strItem = strContent;
        }
        else if (gToken == "description") {
          spdlog::trace(
            "ParseMDF: handleMDFParserData: Module manufacturer email address description: {0} Language: {1}",
            strContent,
            gLastLanguage);
          gpItemStruct->m_mapDescription[gLastLanguage] = strContent;
        }
      }
      // Picture standard format
      else if ((gTokenParent == "picture") && (gpPictureStruct != NULL)) {
        if (gToken == "description") {
          // Description for firmware
          spdlog::trace("ParseMDF: handleMDFParserData: Module picture description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpPictureStruct->m_mapDescription[gLastLanguage] = strContent;
        }
      }
      // Firmware standard format
      else if ((gTokenParent == "firmware") && (gpFirmwareStruct != NULL)) {
        if (gToken == "description") {
          // Description for firmware
          spdlog::trace("ParseMDF: handleMDFParserData: Module firmware description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpFirmwareStruct->m_mapDescription[gLastLanguage] = strContent;
        }
      }
      // Manual standard format
      else if ((gTokenParent == "manual") && (gpManualStruct != NULL)) {
        if (gToken == "description") {
          // Description for manual
          spdlog::trace("ParseMDF: handleMDFParserData: Module manual description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpManualStruct->m_mapDescription[gLastLanguage] = strContent;
        }
      }
      break;
  }
}

// ----------------------------------------------------------------------------

void
__endSetupMDFParser(void *data, const char *name)
{
  spdlog::trace("ParseMDF: ---> End: Tag: {0} Depth: {1}", name, gdepth_xml_parser);

  // Get the pointer to the CMDF object
  CMDF *pmdf = (CMDF *) data;
  if (NULL == pmdf) {
    spdlog::trace("ParseMDF: ---> endSetupMDFParser: Data object is invalid");
    return;
  }

  switch (gdepth_xml_parser) {

    case 1:
      if (0 == strcasecmp(name, "vscp")) {
        gbVscp = false;
      }
      break;

    case 2:
      if (0 == strcasecmp(name, "module")) {
        gbModule = false;
      }
      else if (0 == strcasecmp(name, "redirect")) {
      }
      break;

    case 3:
      if (0 == strcasecmp(name, "manufacturer")) {
        // Nothing to do
      }
      else if (0 == strcasecmp(name, "firmware")) {
        gpFirmwareStruct = nullptr;
      }
      // This is the deprecated form
      else if (0 == strcasecmp(name, "pictures")) {
        gpPictureStruct = nullptr;
      }
      else if (0 == strcasecmp(name, "picture")) {
        gpPictureStruct = nullptr;
      }
      else if (0 == strcasecmp(name, "manual")) {
        gpManualStruct = nullptr;
      }
      break;

    case 4:
      if (0 == strcasecmp(name, "address")) {
        // Nothing to do
      }
      else if (0 == strcasecmp(name, "telephone") && (gpItemStruct != nullptr)) {
        // Save address data in list
        pmdf->m_manufacturer.m_list_Phone.push_back(gpItemStruct);
        gpItemStruct = nullptr;
      }
      else if (0 == strcasecmp(name, "fax") && (gpItemStruct != nullptr)) {
        // Save address data in list
        pmdf->m_manufacturer.m_list_Fax.push_back(gpItemStruct);
        gpItemStruct = nullptr;
      }
      else if (0 == strcasecmp(name, "email") && (gpItemStruct != nullptr)) {
        // Save address data in list
        pmdf->m_manufacturer.m_list_Email.push_back(gpItemStruct);
        gpItemStruct = nullptr;
      }
      else if (0 == strcasecmp(name, "web") && (gpItemStruct != nullptr)) {
        // Save address data in list
        pmdf->m_manufacturer.m_list_Web.push_back(gpItemStruct);
        gpItemStruct = nullptr;
      }
      break;
  }

  // Reset language for 'name' and 'description' to default
  if ((0 == strcmp(name, "name")) || (0 == strcmp(name, "description"))) {
    gLastLanguage = "en";
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
  gbVscp            = false;
  gbModule          = false;
  gLastLanguage     = "en";

  gpItemStruct     = nullptr;
  gpPictureStruct  = nullptr;
  gpFirmwareStruct = nullptr;
  gpManualStruct   = nullptr;

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
CMDF::parseMDF_JSON(std::string &path)
{
  int rv = VSCP_ERROR_SUCCESS;

  spdlog::debug("Reading JSON MDF {0}", path);

  json j;

  try {
    std::ifstream ifs(path, std::ifstream::in);
    ifs >> j;
    ifs.close();
  }
  catch (...) {
    spdlog::error("Parse-JSON: Failed to parse JSON configuration.");
    return false;
  }

  spdlog::trace("Parse-JSON: <<{}>>", j.dump());

  // ********************************************************************************
  //                                  Module
  // ********************************************************************************

  if (!(j.contains("module") && j["module"].is_object())) {
    spdlog::error("Parse-JSON: module info is not found. <<{}>>", j.dump());
    return VSCP_ERROR_PARSING;
  }
  else {

    // Module name - Mandatory
    // Can either be string or object
    if (j["module"].contains("name")) {

      if (j["module"]["name"].is_string()) {
        m_name = j["module"]["name"];
        spdlog::trace("Parse-JSON: Module name: {0} language: 'en' ", m_name);
      }
      else {
        spdlog::error("Parse-JSON: Failed to read module name (not string, not object)");
        return VSCP_ERROR_PARSING;
      }
    }
    else {
      spdlog::debug("Parse-JSON: Failed to read module name.");
      return VSCP_ERROR_PARSING;
    }

    // Module model - not mandatory
    if (j["module"].contains("model") && j["module"]["model"].is_string()) {
      m_strModule_Model = j["module"]["model"];
      spdlog::trace("Parse-JSON: Module model: {0}", m_strModule_Model);
    }
    else {
      spdlog::trace("Parse-JSON: There is no module model.");
    }

    // Module version - not mandatory
    if (j["module"].contains("version") && j["module"]["version"].is_string()) {
      m_strModule_Version = j["module"]["version"];
      spdlog::trace("Parse-JSON: Module version: {0}", m_strModule_Version);
    }
    else {
      spdlog::trace("Parse-JSON: There is no module version.");
    }

    // Module changed - not mandatory
    if (j["module"].contains("changed") && j["module"]["changed"].is_string()) {
      m_strModule_changeDate = j["module"]["changed"];
      spdlog::trace("Parse-JSON: Module changed: {0}", m_strModule_changeDate);
    }
    else {
      spdlog::trace("Parse-JSON: There is no module changed.");
    }

    // Module buffersize - not mandatory
    m_module_bufferSize = 8;
    if (j["module"].contains("buffersize") && j["module"]["buffersize"].is_number_integer()) {
      m_module_bufferSize = j["module"]["buffersize"];
      spdlog::trace("Parse-JSON: Module buffersize: {0}", m_module_bufferSize);
    }
    else {
      spdlog::trace("Parse-JSON: There is no module buffersize.");
    }

    // Module description - not mandatory
    // Can either be string or object
    if (j["module"].contains("description")) {

      if (j["module"]["description"].is_string()) {
        m_mapDescription["en"] = j["module"]["description"];
        spdlog::trace("Parse-JSON: Module description: {0} language: 'en' ", m_mapDescription["en"]);
      }
      else if (j["module"]["description"].is_object()) {
        for (auto &item : j["module"]["description"].items()) {
          m_mapDescription[item.key()] = item.value();
          spdlog::trace("Parse-JSON: Module description: {0} language: '{1}'", item.value(), item.key());
        }
      }
      else {
        spdlog::trace("Parse-JSON: No module description (not string, not object)");
      }
    }
    else {
      spdlog::trace("Parse-JSON: No module description.");
    }

    // Module infourl - not mandatory
    // Can either be string or object
    if (j["module"].contains("infourl")) {
      if (j["module"]["infourl"].is_string()) {
        m_mapInfoURL["en"] = j["module"]["infourl"];
        spdlog::trace("Parse-JSON: Module info URL: {0} language: 'en' ", m_mapInfoURL["en"]);
      }
      else if (j["module"]["infourl"].is_object()) {
        for (auto &item : j["module"]["infourl"].items()) {
          m_mapInfoURL[item.key()] = item.value();
          spdlog::trace("Parse-JSON: Module info URL: {0} language: '{1}'", item.value(), item.key());
        }
      }
      else {
        spdlog::trace("Parse-JSON: No module info URL (not string, not object)");
      }
    }
    else {
      spdlog::trace("Parse-JSON: No module info URL.");
    }

    // Module infourl - not mandatory
    // Can either be string or object
    if (j["module"].contains("manufacturer") && j["module"]["manufacturer"].is_object()) {

      json jsub = j["module"]["manufacturer"];

      // Create manufacurer object
      // CMDF_Manufacturer *pmf = new CMDF_Manufacturer();
      // if (pmf == nullptr) {
      //   spdlog::error("Parse-JSON: Failed to allocate memory for manufacturer.");
      //   return VSCP_ERROR_PARSING;
      // }
      // m_manufacturer.push_back(pmf);

      if (jsub.contains("name") && jsub["name"].is_string()) {
        m_manufacturer.m_strName = jsub["name"];
        spdlog::trace("Parse-JSON: Module manufacturer name: {0}", m_manufacturer.m_strName);
      }
      else {
        spdlog::warn("Parse-JSON: Failed to read module manufacturer name (not string)");
      }

      // Adress - Is always object
      if (jsub.contains("address") && jsub["address"].is_object()) {

        // for (auto& addr : jsub["address"].items()) {
        // std::cout << "key: " << addr.key() << ", value:" << addr.value() << '\n';
        // if (addr.value().is_object()) {

        // CMDF_Address *paddr = new CMDF_Address();
        // if (paddr == nullptr) {
        //   spdlog::error("Parse-JSON: Failed to allocate memory for address.");
        //   return VSCP_ERROR_PARSING;
        // }
        // m_manufacturer.m_address.push_back(paddr);

        json jsub2 = jsub["address"];
        if (jsub2.contains("street") && jsub2["street"].is_string()) {
          m_manufacturer.m_address.m_strStreet = jsub2["street"];
        }

        if (jsub2.contains("town") && jsub2["town"].is_string()) {
          m_manufacturer.m_address.m_strTown = jsub2["town"];
        }

        if (jsub2.contains("city") && jsub2["city"].is_string()) {
          m_manufacturer.m_address.m_strCity = jsub2["city"];
        }

        if (jsub2.contains("postcode") && jsub2["postcode"].is_string()) {
          m_manufacturer.m_address.m_strPostCode = jsub2["postcode"];
        }

        if (jsub2.contains("state") && jsub2["state"].is_string()) {
          m_manufacturer.m_address.m_strState = jsub2["state"];
        }

        if (jsub2.contains("region") && jsub2["region"].is_string()) {
          m_manufacturer.m_address.m_strRegion = jsub2["region"];
        }

        if (jsub2.contains("country") && jsub2["country"].is_string()) {
          m_manufacturer.m_address.m_strCountry = jsub2["country"];
        }
        //   }
        // }

        // Phone - Is always array
        if (jsub.contains("telephone") && jsub["telephone"].is_array()) {

          for (auto &phone : jsub["telephone"].items()) {
            // std::cout << "key: " << phone.key() << ", value:" << phone.value() << '\n';
            if (phone.value().is_object()) {

              CMDF_Item *ptel = new CMDF_Item();
              if (ptel == nullptr) {
                spdlog::error("Parse-JSON: Failed to allocate memory for telephone.");
                return VSCP_ERROR_PARSING;
              }
              // m_manufacturer.m_address.m_list_Phone.push_back(ptel);

              json jsub2 = phone.key();

              if (jsub2.contains("description") && jsub2["description"].is_string()) {
                ptel->m_strItem = jsub2["number"];
              }

              // Description is language specific. Can be string or object
              if (jsub2.contains("description")) {

                if (jsub2["description"].is_string()) {
                  ptel->m_mapDescription["en"] = jsub2["description"];
                  spdlog::trace("Parse-JSON: Phone description: {0} language: 'en' ", jsub2["description"]);
                }
                else if (jsub2["description"].is_object()) {
                  for (auto &item : jsub2["description"].items()) {
                    ptel->m_mapDescription[item.key()] = item.value();
                    spdlog::trace("Parse-JSON: Phone description: {0} language: '{1}'", item.value(), item.key());
                  }
                }
                else {
                  spdlog::trace("Parse-JSON: No phone description (not string, not object)");
                }
              }
              else {
                spdlog::trace("Parse-JSON: No phone description.");
              }
            }
          }
        }

        // Fax - Is always array
        if (jsub.contains("fax") && jsub["fax"].is_array()) {

          for (auto &fax : jsub["fax"].items()) {
            // std::cout << "key: " << fax.key() << ", value:" << fax.value() << '\n';
            if (fax.value().is_object()) {

              CMDF_Item *pfax = new CMDF_Item();
              if (pfax == nullptr) {
                spdlog::error("Parse-JSON: Failed to allocate memory for fax.");
                return VSCP_ERROR_PARSING;
              }
              // m_manufacturer.m_address.m_list_Fax.push_back(pfax);

              json jsub2 = fax.key();

              if (jsub2.contains("description") && jsub2["description"].is_string()) {
                pfax->m_strItem = jsub2["number"];
              }

              // Description is language specific. Can be string or object
              if (jsub2.contains("description")) {

                if (jsub2["description"].is_string()) {
                  pfax->m_mapDescription["en"] = jsub2["description"];
                  spdlog::trace("Parse-JSON: Fax description: {0} language: 'en' ", jsub2["description"]);
                }
                else if (jsub2["description"].is_object()) {
                  for (auto &item : jsub2["description"].items()) {
                    pfax->m_mapDescription[item.key()] = item.value();
                    spdlog::trace("Parse-JSON: Fax description: {0} language: '{1}'", item.value(), item.key());
                  }
                }
                else {
                  spdlog::trace("Parse-JSON: No phone description (not string, not object)");
                }
              }
              else {
                spdlog::trace("Parse-JSON: No phone description.");
              }
            }
          }
        }

        // Email - Is always array
        if (jsub.contains("email") && jsub["email"].is_array()) {

          for (auto &email : jsub["email"].items()) {
            // std::cout << "key: " << email.key() << ", value:" << email.value() << '\n';
            if (email.value().is_object()) {

              CMDF_Item *pemail = new CMDF_Item();
              if (pemail == nullptr) {
                spdlog::error("Parse-JSON: Failed to allocate memory for email item.");
                return VSCP_ERROR_PARSING;
              }
              // m_manufacturer.m_address.m_list_Email.push_back(pemail);

              json jsub2 = email.key();

              if (jsub2.contains("address") && jsub2["address"].is_string()) {
                pemail->m_strItem = jsub2["address"];
              }

              // Description is language specific. Can be string or object
              if (jsub2.contains("description")) {

                if (jsub2["description"].is_string()) {
                  pemail->m_mapDescription["en"] = jsub2["description"];
                  spdlog::trace("Parse-JSON: Email description: {0} language: 'en' ", jsub2["description"]);
                }
                else if (jsub2["description"].is_object()) {
                  for (auto &item : jsub2["description"].items()) {
                    pemail->m_mapDescription[item.key()] = item.value();
                    spdlog::trace("Parse-JSON: Email description: {0} language: '{1}'", item.value(), item.key());
                  }
                }
                else {
                  spdlog::trace("Parse-JSON: No email description (not string, not object)");
                }
              }
              else {
                spdlog::trace("Parse-JSON: No email description.");
              }
            }
          }
        }

        // Web - Is always array
        if (jsub.contains("web") && jsub["web"].is_array()) {

          for (auto &web : jsub["web"].items()) {
            // std::cout << "key: " << web.key() << ", value:" << web.value() << '\n';
            if (web.value().is_object()) {

              CMDF_Item *pweb = new CMDF_Item();
              if (pweb == nullptr) {
                spdlog::error("Parse-JSON: Failed to allocate memory for web item.");
                return VSCP_ERROR_PARSING;
              }
              // m_manufacturer.m_address.m_list_Email.push_back(pweb);

              json jsub2 = web.key();

              if (jsub2.contains("url") && jsub2["url"].is_string()) {
                pweb->m_strItem = jsub2["url"];
              }
              else if (jsub2.contains("address") && jsub2["address"].is_string()) {
                pweb->m_strItem = jsub2["address"];
              }
              else {
                spdlog::warn("Parse-JSON: No web url/address.");
              }

              // Description is language specific. Can be string or object
              if (jsub2.contains("description")) {

                if (jsub2["description"].is_string()) {
                  pweb->m_mapDescription["en"] = jsub2["description"];
                  spdlog::trace("Parse-JSON: Web description: {0} language: 'en' ", jsub2["description"]);
                }
                else if (jsub2["description"].is_object()) {
                  for (auto &item : jsub2["description"].items()) {
                    pweb->m_mapDescription[item.key()] = item.value();
                    spdlog::trace("Parse-JSON: Web description: {0} language: '{1}'", item.value(), item.key());
                  }
                }
                else {
                  spdlog::trace("Parse-JSON: No Web description (not string, not object)");
                }
              }
              else {
                spdlog::trace("Parse-JSON: No Web description.");
              }
            }
          }
        }
      }

      if (j["module"].contains("register") && j["module"]["register"].is_array()) {
        for (auto &reg : j["module"]["register"].items()) {
          //std::cout << "key: " << reg.key() << ", value:" << reg.value() << '\n';
          if (reg.value().is_object()) {

            CMDF_Register *preg = new CMDF_Register();
            if (preg == nullptr) {
              spdlog::error("Parse-JSON: Failed to allocate memory for register item.");
              return VSCP_ERROR_PARSING;
            }

            m_list_register.push_back(preg);

            json jreg(reg.value());
            //std::cout << "REG: " << jreg.dump() << '\n';

            // Register Page
            if (jreg.contains("page") && jreg["page"].is_number()) {
              preg->m_page = jreg["page"];
            }
            else if (jreg.contains("page") && jreg["page"].is_string()) {
              preg->m_page = vscp_readStringValue(jreg["page"]);
            }
            else {
              preg->m_page = 0;
              spdlog::warn("Parse-JSON: No register page defined (defaults to zero).");
            }

            // Register offset
            if (jreg.contains("offset") && jreg["offset"].is_number()) {
              preg->m_offset = jreg["offset"];
            }
            else if (jreg.contains("offset") && jreg["offset"].is_string()) {
              preg->m_offset = vscp_readStringValue(jreg["offset"]);
            }
            else {
              preg->m_offset = 0;
              spdlog::error("Parse-JSON: No register offset defined (defaults to zero).");
            }

            // Register width
            if (jreg.contains("width") && jreg["width"].is_number()) {
              preg->m_width = jreg["width"];
            }
            else if (jreg.contains("width") && jreg["width"].is_string()) {
              preg->m_width = vscp_readStringValue(jreg["width"]);
            }
            else {
              preg->m_width = 8;
              spdlog::info("Parse-JSON: No register width defined (defaults to eight bits).");
            }

            // Register min
            if (jreg.contains("min") && jreg["min"].is_number()) {
              preg->m_min = jreg["min"];
            }
            else if (jreg.contains("min") && jreg["min"].is_string()) {
              preg->m_min = vscp_readStringValue(jreg["min"]);
            }
            else {
              preg->m_min = 0;
              spdlog::info("Parse-JSON: No register min defined (defaults to zero).");
            }

            // Register max
            if (jreg.contains("max") && jreg["max"].is_number()) {
              preg->m_max = jreg["max"];
            }
            else if (jreg.contains("max") && jreg["max"].is_string()) {
              preg->m_max = vscp_readStringValue(jreg["max"]);
            }
            else {
              preg->m_max = 0;
              spdlog::info("Parse-JSON: No register max defined (defaults to 255).");
            }

            if (jreg.contains("access") && jreg["access"].is_string()) {
              std::string strAccess = jreg["access"];
              vscp_trim(strAccess);
              vscp_makeLower(strAccess);
              preg->m_access = MDF_REG_ACCESS_NONE;
              if (strAccess == "w") {
                preg->m_access = MDF_REG_ACCESS_READ_ONLY;
              }
              else if (strAccess == "r") {
                preg->m_access = MDF_REG_ACCESS_WRITE_ONLY;
              }
              else if (strAccess == "rw") {
                preg->m_access = MDF_REG_ACCESS_READ_WRITE;
              }
            }
            else {
              preg->m_max = 0;
              spdlog::info("Parse-JSON: No register max defined (defaults to 255).");
            }

            // Register description
            if (jreg.contains("description")) {

                if (jreg["description"].is_string()) {
                  preg->m_mapDescription["en"] = jreg["description"];
                  spdlog::trace("Parse-JSON: Register description: {0} language: 'en' ", jreg["description"]);
                }
                else if (jreg["description"].is_object()) {
                  for (auto &item : jreg["description"].items()) {
                    preg->m_mapDescription[item.key()] = item.value();
                    spdlog::trace("Parse-JSON: Register description: {0} language: '{1}'", item.value(), item.key());
                  }
                }
                else {
                  spdlog::trace("Parse-JSON: No register description (not string, not object)");
                }
            }
            else {
              spdlog::trace("Parse-JSON: No register description.");
            }

            // Register infourl
            if (jreg.contains("infourl")) {

                if (jreg["infourl"].is_string()) {
                  preg->m_mapDescription["en"] = jreg["infourl"];
                  spdlog::trace("Parse-JSON: Register description: {0} language: 'en' ", jreg["infourl"]);
                }
                else if (jreg["infourl"].is_object()) {
                  for (auto &item : jreg["infourl"].items()) {
                    preg->m_mapInfoURL[item.key()] = item.value();
                    spdlog::trace("Parse-JSON: Register infourl: {0} language: '{1}'", item.value(), item.key());
                  }
                }
                else {
                  spdlog::trace("Parse-JSON: No register infourl (not string, not object)");
                }
            }
            else {
              spdlog::trace("Parse-JSON: No register infourl.");
            }

            // Register valuelist
            if (jreg.contains("valuelist") && jreg["valuelist"].is_array()) {
              for (auto &item : jreg["valuelist"].items()) {
                if (item.value().is_object()) {

                  json jvalue(item.value());

                  CMDF_ValueListValue *pvalue = new CMDF_ValueListValue();
                  if (pvalue == nullptr) {
                    spdlog::error("Parse-JSON: Failed to allocate memory for register value item.");
                    return VSCP_ERROR_PARSING;
                  }

                  preg->m_list_value.push_back(pvalue);

                  if (jvalue.contains("name") && jvalue["name"].is_string()) {
                    pvalue->m_name = jreg["name"];
                  }
                  else {
                    pvalue->m_name = "";
                    spdlog::error("Parse-JSON: No register valuelist name defined (defaults to empty).");
                  }

                  if (jvalue.contains("value") && jvalue["value"].is_string()) {
                    pvalue->m_strValue = jvalue["value"];
                  }
                  else {
                    spdlog::error("Parse-JSON: No register valuelist value defined (defaults to zero).");
                  }

                  // Register description
                  if (jvalue.contains("description")) {

                      if (jvalue["description"].is_string()) {
                        pvalue->m_mapDescription["en"] = jvalue["description"];
                        spdlog::trace("Parse-JSON: Register description: {0} language: 'en' ", jvalue["description"]);
                      }
                      else if (jvalue["description"].is_object()) {
                        for (auto &item : jvalue["description"].items()) {
                          pvalue->m_mapDescription[item.key()] = item.value();
                          spdlog::trace("Parse-JSON: Register valuelist description: {0} language: '{1}'", item.value(), item.key());
                        }
                      }
                      else {
                        spdlog::trace("Parse-JSON: No register valuelist description (not string, not object)");
                      }
                  }
                  else {
                    spdlog::trace("Parse-JSON: No registervaluelist  description.");
                  }

                  // Register infourl
                  if (jvalue.contains("infourl")) {

                      if (jvalue["infourl"].is_string()) {
                        pvalue->m_mapDescription["en"] = jvalue["infourl"];
                        spdlog::trace("Parse-JSON: Register valuelist infourl: {0} language: 'en' ", jvalue["infourl"]);
                      }
                      else if (jvalue["infourl"].is_object()) {
                        for (auto &item : jvalue["infourl"].items()) {
                          pvalue->m_mapInfoURL[item.key()] = item.value();
                          spdlog::trace("Parse-JSON: Register valuelist infourl: {0} language: '{1}'", item.value(), item.key());
                        }
                      }
                      else {
                        spdlog::trace("Parse-JSON: No register valuelist infourl (not string, not object)");
                      }
                  }
                  else {
                    spdlog::trace("Parse-JSON: No register valuelist infourl.");
                  }

                }
                else {
                  spdlog::trace("Parse-JSON: No register valuelist value is is not object");
                }
              }              
            } // valuelist

            // Register bit
            if (jreg.contains("bit") && jreg["bit"].is_array()) {
              for (auto &item : jreg["bit"].items()) {
                if (item.value().is_object()) {

                  json jbit(item.value());

                  CMDF_Bit *pbit = new CMDF_Bit();
                  if (pbit == nullptr) {
                    spdlog::error("Parse-JSON: Failed to allocate memory for register bit item.");
                    return VSCP_ERROR_PARSING;
                  }

                  preg->m_list_bit.push_back(pbit);

                  if (jbit.contains("name") && jbit["name"].is_string()) {
                    pbit->m_name = jbit["name"];
                  }
                  else {
                    pbit->m_name = "";
                    spdlog::error("Parse-JSON: No register bit name defined (defaults to empty).");
                  }

                  if (jbit.contains("pos") && jbit["pos"].is_number()) {
                    pbit->m_pos = jbit["pos"];
                  }
                  else if (jbit.contains("pos") && jbit["pos"].is_string()) {
                    pbit->m_pos = vscp_readStringValue(jbit["pos"]);
                  }
                  else {
                    pbit->m_pos = 0;
                    spdlog::error("Parse-JSON: No register bit pos defined (defaults to zero).");
                  }

                  if (jbit.contains("width") && jbit["width"].is_number()) {
                    pbit->m_width = jbit["width"];
                  }
                  else if (jbit.contains("width") && jbit["width"].is_string()) {
                    pbit->m_width = vscp_readStringValue(jbit["width"]);
                  }
                  else {
                    pbit->m_width = 1;
                    spdlog::trace("Parse-JSON: No register bit width defined (defaults to one).");
                  }

                  if (jbit.contains("default") && jbit["default"].is_number()) {
                    pbit->m_default = jbit["default"];
                  }
                  else if (jbit.contains("default") && jbit["default"].is_string()) {
                    pbit->m_default = vscp_readStringValue(jbit["default"]);
                  }
                  else {
                    pbit->m_default = 1;
                    spdlog::trace("Parse-JSON: No register bit default defined (defaults to zero).");
                  }

                  if (jbit.contains("min") && jbit["min"].is_number()) {
                    pbit->m_min = jbit["min"];
                  }
                  else if (jbit.contains("min") && jbit["min"].is_string()) {
                    pbit->m_min = vscp_readStringValue(jbit["min"]);
                  }
                  else {
                    pbit->m_min = 0;
                    spdlog::trace("Parse-JSON: No register bit min defined (defaults to zero).");
                  }

                  if (jbit.contains("max") && jbit["max"].is_number()) {
                    pbit->m_max = jbit["max"];
                  }
                  else if (jbit.contains("max") && jbit["max"].is_string()) {
                    pbit->m_max = vscp_readStringValue(jbit["max"]);
                  }
                  else {
                    pbit->m_min = 255;
                    spdlog::trace("Parse-JSON: No register bit max defined (defaults to 255).");
                  }

                  if (jbit.contains("access") && jbit["access"].is_string()) {
                    std::string strAccess = jbit["access"];
                    vscp_trim(strAccess);
                    vscp_makeLower(strAccess);
                    pbit->m_access = MDF_REG_ACCESS_NONE;
                    if (strAccess == "w") {
                      pbit->m_access = MDF_REG_ACCESS_READ_ONLY;
                    }
                    else if (strAccess == "r") {
                      pbit->m_access = MDF_REG_ACCESS_WRITE_ONLY;
                    }
                    else if (strAccess == "rw") {
                      pbit->m_access = MDF_REG_ACCESS_READ_WRITE;
                    }
                  }
                  else {
                    pbit->m_access = MDF_REG_ACCESS_READ_WRITE;
                    spdlog::info("Parse-JSON: No register access defined (defaults to 'rw').");
                  }      

                  // Register description
                  if (jbit.contains("description")) {

                      if (jbit["description"].is_string()) {
                        pbit->m_mapDescription["en"] = jbit["description"];
                        spdlog::trace("Parse-JSON: Register description: {0} language: 'en' ", jreg["description"]);
                      }
                      else if (jbit["description"].is_object()) {
                        for (auto &item : jbit["description"].items()) {
                          pbit->m_mapDescription[item.key()] = item.value();
                          spdlog::trace("Parse-JSON: Register bit description: {0} language: '{1}'", item.value(), item.key());
                        }
                      }
                      else {
                        spdlog::trace("Parse-JSON: No register bit description (not string, not object)");
                      }
                  }
                  else {
                    spdlog::trace("Parse-JSON: No register bit  description.");
                  }

                  // Register infourl
                  if (jbit.contains("infourl")) {

                      if (jbit["infourl"].is_string()) {
                        pbit->m_mapDescription["en"] = jbit["infourl"];
                        spdlog::trace("Parse-JSON: Register bit: {0} language: 'en' ", jbit["infourl"]);
                      }
                      else if (jbit["infourl"].is_object()) {
                        for (auto &item : jbit["infourl"].items()) {
                          pbit->m_mapInfoURL[item.key()] = item.value();
                          spdlog::trace("Parse-JSON: Register bit: {0} language: '{1}'", item.value(), item.key());
                        }
                      }
                      else {
                        spdlog::trace("Parse-JSON: No register bit (not string, not object)");
                      }
                  }
                  else {
                    spdlog::trace("Parse-JSON: No bit infourl.");
                  }

                } // is object
              } // for 
            } // bit
           
          }
        }
      }
      else {
        spdlog::warn("Parse-JSON: Failed to read module registers");
      }

      if (j["module"].contains("remotevar") && j["module"]["remotevar"].is_array()) {
        for (auto &rvar : j["module"]["remotevar"].items()) {
          //std::cout << "key: " << rvar.key() << ", value:" << rvar.value() << '\n';
          if (rvar.value().is_object()) {

            CMDF_RemoteVariable *prvar = new CMDF_RemoteVariable();
            if (prvar == nullptr) {
              spdlog::error("Parse-JSON: Failed to allocate memory for remote variable item.");
              return VSCP_ERROR_PARSING;
            }

            m_list_remotevar.push_back(prvar);

            json jrvar = rvar.value();

            if (jrvar.contains("name") && jrvar["name"].is_string()) {
              prvar->m_name = jrvar["name"];
            }
            else {
              spdlog::error("Parse-JSON: No remote variable name defined.");
              return VSCP_ERROR_PARSING;
            }
          }
        }
      }
      else {
        spdlog::warn("Parse-JSON: Failed to read module remote variable");
      }

      if (j["module"].contains("dmatrix") && j["module"]["dmatrix"].is_object()) {}
      else {
        spdlog::warn("Parse-JSON: Failed to read module decision matrix");
      }

      if (j["module"].contains("events") && j["module"]["events"].is_array()) {
        for (auto &ev : j["module"]["events"].items()) {
          //std::cout << "key: " << ev.key() << ", value:" << ev.value() << '\n';
          if (ev.value().is_object()) {

            CMDF_Event *pev = new CMDF_Event();
            if (pev == nullptr) {
              spdlog::error("Parse-JSON: Failed to allocate memory for remote variable item.");
              return VSCP_ERROR_PARSING;
            }

            m_list_event.push_back(pev);

            json jev = ev.key();
          }
        }
      }
      else {
        spdlog::warn("Parse-JSON: Failed to read module events");
      }

      if (j["module"].contains("alarm") && j["module"]["alarm"].is_array()) {
        for (auto &alarm : j["module"]["alarm"].items()) {
          //std::cout << "key: " << alarm.key() << ", value:" << alarm.value() << '\n';
          if (alarm.value().is_object()) {

            CMDF_Bit *palarm = new CMDF_Bit();
            if (palarm == nullptr) {
              spdlog::error("Parse-JSON: Failed to allocate memory for alarm item.");
              return VSCP_ERROR_PARSING;
            }

            m_list_alarm.push_back(palarm);

            json jalarm = alarm.key();
          }
        }
      }
      else {
        spdlog::warn("Parse-JSON: Failed to read module alarm bits");
      }
    }
    else {
      spdlog::warn("Parse-JSON: No manufacturer defined");
    }

    ///////////////////////////////////////////////////////////////////////////
    //                                Files
    ///////////////////////////////////////////////////////////////////////////

    // 'files' - Is always object
    if (j["module"].contains("files") && j["module"]["files"].is_object()) {

      json jsub = j["module"]["files"];

      // * * * Pictures * * *
      if (jsub.contains("picture") && jsub["picture"].is_array()) {
        for (auto &item : jsub["picture"].items()) {

          CMDF_Picture *ppicture = new CMDF_Picture();
          if (ppicture == nullptr) {
            spdlog::error("Parse-JSON: Failed to allocate memory for picture item.");
            return VSCP_ERROR_PARSING;
          }

          m_list_picture.push_back(ppicture);

          json jpic = item.value();

          if (jpic.contains("url") && jpic["url"].is_string()) {
            ppicture->m_strURL = jpic["url"];
            spdlog::trace("Parse-JSON: Picture URL: {0} ", jpic["url"]);
          }
          else {
            spdlog::warn("Parse-JSON: No picture url.");
          }

          if (jpic.contains("format") && jpic["format"].is_string()) {
            ppicture->m_strFormat = jpic["format"];
            spdlog::trace("Parse-JSON: Picture Format: {0} ", jpic["url"]);
          }
          else {
            spdlog::warn("Parse-JSON: No picture format.");
          }

          if (jpic.contains("description") && jpic["description"].is_object()) {

            for (auto &item : jpic["description"].items()) {
              ppicture->m_mapDescription[item.key()] = item.value();
              spdlog::trace("Parse-JSON: Picture description: {0} language: '{1}'", item.value(), item.key());
            }
          }
          else {
            spdlog::trace("Parse-JSON: No picture description.");
          }
        }
      }

      // * * * Firmware * * *
      if (jsub.contains("firmware") && jsub["firmware"].is_array()) {
        for (auto &item : jsub["firmware"].items()) {

          CMDF_Firmware *pfirmware = new CMDF_Firmware();
          if (pfirmware == nullptr) {
            spdlog::error("Parse-JSON: Failed to allocate memory for firmware item.");
            return VSCP_ERROR_PARSING;
          }

          m_list_firmware.push_back(pfirmware);

          json jfirmware = item.value();

          if (jfirmware.contains("url") && jfirmware["url"].is_string()) {
            pfirmware->m_strURL = jfirmware["url"];
            spdlog::trace("Parse-JSON: Picture URL: {0} ", jfirmware["url"]);
          }
          else {
            spdlog::warn("Parse-JSON: No picture url.");
          }

          if (jfirmware.contains("target") && jfirmware["target"].is_string()) {
            pfirmware->m_strTarget = jfirmware["target"];
            spdlog::trace("Parse-JSON: Firmware target: {0} ", jfirmware["target"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware target.");
          }

          if (jfirmware.contains("targetcode") && jfirmware["targetcode"].is_number()) {
            pfirmware->m_targetCode = jfirmware["targetcode"];
            spdlog::trace("Parse-JSON: Firmware target: {0} ", jfirmware["targetcode"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware target code.");
          }

          // Target code string value
          if (jfirmware.contains("targetcode") && jfirmware["targetcode"].is_string()) {
            pfirmware->m_targetCode = vscp_readStringValue(jfirmware["targetcode"]);
            spdlog::trace("Parse-JSON: Firmware target: {0} ", jfirmware["targetcode"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware target code.");
          }

          if (jfirmware.contains("targetcode") && jfirmware["targetcode"].is_number()) {
            pfirmware->m_targetCode = jfirmware["targetcode"];
            spdlog::trace("Parse-JSON: Firmware target: {0} ", jfirmware["targetcode"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware target code.");
          }

          if (jfirmware.contains("format") && jfirmware["format"].is_string()) {
            pfirmware->m_strFormat = jfirmware["format"];
            spdlog::trace("Parse-JSON: Firmware format: {0} ", jfirmware["format"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware format.");
          }

          if (jfirmware.contains("date") && jfirmware["date"].is_string()) {
            pfirmware->m_strDate = jfirmware["date"];
            spdlog::trace("Parse-JSON: Firmware date: {0} ", jfirmware["date"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware date");
          }

          if (jfirmware.contains("version_major") && jfirmware["version_major"].is_number()) {
            pfirmware->m_version_major = jfirmware["version_major"];
            spdlog::trace("Parse-JSON: Firmware version major: {0} ", jfirmware["version_major"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version_major");
          }

          if (jfirmware.contains("version_major") && jfirmware["version_major"].is_string()) {
            pfirmware->m_version_major = vscp_readStringValue(jfirmware["version_major"]);
            spdlog::trace("Parse-JSON: Firmware version major: {0} ", jfirmware["version_major"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version_major");
          }

          if (jfirmware.contains("version_minor") && jfirmware["version_minor"].is_number()) {
            pfirmware->m_version_minor = jfirmware["version_minor"];
            spdlog::trace("Parse-JSON: Firmware version minor: {0} ", jfirmware["version_minor"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version_minor");
          }

          if (jfirmware.contains("version_minor") && jfirmware["version_minor"].is_string()) {
            pfirmware->m_version_minor = vscp_readStringValue(jfirmware["version_minor"]);
            spdlog::trace("Parse-JSON: Firmware version minor: {0} ", jfirmware["version_minor"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version_minor");
          }

          if (jfirmware.contains("version_patch") && jfirmware["version_patch"].is_number()) {
            pfirmware->m_version_patch = jfirmware["version_patch"];
            spdlog::trace("Parse-JSON: Firmware version patch: {0} ", jfirmware["version_patch"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version patch");
          }

          if (jfirmware.contains("version_patch") && jfirmware["version_patch"].is_string()) {
            pfirmware->m_version_patch = vscp_readStringValue(jfirmware["version_patch"]);
            spdlog::trace("Parse-JSON: Firmware version patch: {0} ", jfirmware["version_patch"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version patch");
          }

          if (jfirmware.contains("size") && jfirmware["size"].is_number()) {
            pfirmware->m_size = jfirmware["size"];
            spdlog::trace("Parse-JSON: Firmware version size: {0} ", jfirmware["size"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version size");
          }

          if (jfirmware.contains("size") && jfirmware["size"].is_string()) {
            pfirmware->m_size = vscp_readStringValue(jfirmware["size"]);
            spdlog::trace("Parse-JSON: Firmware version size: {0} ", jfirmware["size"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version size");
          }

          if (jfirmware.contains("md5sum") && jfirmware["md5sum"].is_string()) {
            pfirmware->m_strMd5 = jfirmware["md5sum"];
            spdlog::trace("Parse-JSON: Firmware version md5: {0} ", jfirmware["md5sum"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version md5sum");
          }

          if (jfirmware.contains("description") && jfirmware["description"].is_object()) {

            for (auto &item : jfirmware["description"].items()) {
              pfirmware->m_mapDescription[item.key()] = item.value();
              spdlog::trace("Parse-JSON: Picture description: {0} language: '{1}'", item.value(), item.key());
            }
          }
          else {
            spdlog::trace("Parse-JSON: No picture description.");
          }
        }
      }

      // * * * Manual * * *
      if (jsub.contains("manual") && jsub["manual"].is_array()) {
        for (auto &item : jsub["manual"].items()) {

          CMDF_Manual *pmanual = new CMDF_Manual();
          if (pmanual == nullptr) {
            spdlog::error("Parse-JSON: Failed to allocate memory for manual item.");
            return VSCP_ERROR_PARSING;
          }

          m_list_manual.push_back(pmanual);

          json jmanual = item.value();

          if (jmanual.contains("url") && jmanual["url"].is_string()) {
            pmanual->m_strURL = jmanual["url"];
            spdlog::trace("Parse-JSON: Manual URL: {0} ", jmanual["url"]);
          }
          else {
            spdlog::warn("Parse-JSON: No manual url.");
          }

          if (jmanual.contains("format") && jmanual["format"].is_string()) {
            pmanual->m_strFormat = jmanual["format"];
            spdlog::trace("Parse-JSON: Manual Format: {0} ", jmanual["format"]);
          }
          else {
            spdlog::warn("Parse-JSON: No manual format.");
          }

          if (jmanual.contains("lang") && jmanual["lang"].is_string()) {
            pmanual->m_strLanguage = jmanual["lang"];
            spdlog::trace("Parse-JSON: Manual language: {0} ", jmanual["lang"]);
          }
          else {
            spdlog::warn("Parse-JSON: No manual language.");
          }

          if (jmanual.contains("description") && jmanual["description"].is_object()) {

            for (auto &item : jmanual["description"].items()) {
              pmanual->m_mapDescription[item.key()] = item.value();
              spdlog::trace("Parse-JSON: Manual description: {0} language: '{1}'", item.value(), item.key());
            }
          }
          else {
            spdlog::trace("Parse-JSON: No manual description.");
          }
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    //                              Bootloader
    ///////////////////////////////////////////////////////////////////////////

    // 'boot' - Is always object
    if (j["module"].contains("boot") && j["module"]["boot"].is_object()) {

      json jsub = j["module"]["boot"];
      if (jsub.contains("algorithm") && jsub["algorithm"].is_number()) {
        m_bootInfo.m_nAlgorithm = jsub["algorithm"];
      }

      if (jsub.contains("blocksize") && jsub["blocksize"].is_number()) {
        m_bootInfo.m_nBlockSize = jsub["blocksize"];
      }

      if (jsub.contains("blockcount") && jsub["blockcount"].is_number()) {
        m_bootInfo.m_nBlockCount = jsub["blockcount"];
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    //                               Registers
    ///////////////////////////////////////////////////////////////////////////

    // 'registers' - Is always array
    if (j["module"].contains("registers") && j["module"]["registers"].is_array()) {

      json jsub = j["module"]["registers"];
    }

    ///////////////////////////////////////////////////////////////////////////
    //                     Remote variables/Abstractions
    ///////////////////////////////////////////////////////////////////////////

    // 'remotevar' - Is always array
    if ((j["module"].contains("remotevar") && j["module"]["remotevar"].is_array())) {

      json jsub = j["module"]["remotevar"];
    }

    ///////////////////////////////////////////////////////////////////////////
    //                               Alarm
    ///////////////////////////////////////////////////////////////////////////

    // 'files' - Is always object
    if (j["module"].contains("alarm") && j["module"]["alarm"].is_object()) {

      json jsub = j["module"]["alarm"];
    }

    ///////////////////////////////////////////////////////////////////////////
    //                          Decision Matrix
    ///////////////////////////////////////////////////////////////////////////

    // 'dmatrix' - Is always object
    if (j["module"].contains("dmatrix") && j["module"]["dmatrix"].is_object()) {

      json jsub = j["module"]["dmatrix"];
    }

    ///////////////////////////////////////////////////////////////////////////
    //                               Events
    ///////////////////////////////////////////////////////////////////////////

    // 'files' - Is always object
    if (j["module"].contains("events") && j["module"]["events"].is_object()) {

      json jsub = j["module"]["events"];
    }

  } // Module

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

  spdlog::debug("parseMDF: Parsing MDF file {}", path);

  // Check if file exists
  if (!vscp_fileExists(path)) {
    spdlog::debug("parseMDF: MDF file {} does not exist", path);
    return VSCP_ERROR_INVALID_PATH;
  }

  // Check format
  // ------------
  // If the file is a JSON file we will parse it
  // as JSON else we will parse it as XML. The first
  // character determines the type "{" for JSON or "<"
  // for XML. Whitespace is ignored.

  try {
    ifs.open(path, std::ifstream::in);
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
      ifs.close();
      rv = parseMDF_JSON(path);
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
      if (page == pRecordValue->m_page)
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
      if (pages.end() == pages.find(pRecordValue->m_page)) {
        pages.insert(pRecordValue->m_page);
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
    if ((reg == preg->m_offset) && (page == preg->m_page)) {
      return preg;
    }
  }

  return NULL;
}
