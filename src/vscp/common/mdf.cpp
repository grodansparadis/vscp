// FILE: mdf.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2022 Ake Hedman, the VSCP project
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
#include <stdio.h>
#include <string>
#include <memory>

#include <expat.h>
#include <json.hpp>         // Needs C++11  -std=c++11
#include <mustache.hpp>
#include <maddy/parser.h>   // Markdown -> HTML

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

#include <curl/curl.h>

// For older cURL versions you will also need
//#include <curl/types.h>
//#include <curl/easy.h>

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

CMDF_Value::CMDF_Value()
{
  m_name.clear();
  m_strValue.clear();
}

CMDF_Value::~CMDF_Value()
{
  m_mapDescription.clear();
  m_mapInfoURL.clear();
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_RemoteVariable::CMDF_RemoteVariable()
{
  m_strDefault.clear();
  m_type   = remote_variable_type_unknown;
  m_page   = 0;
  m_offset = 0;
  m_bitpos = 0;
  m_size   = 0;
  m_access = MDF_REG_ACCESS_READ_WRITE;

  m_bgcolor   = 0xffffff;
  m_fgcolor   = 0x000000;
}

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
  m_strDefault.clear();
  m_type   = remote_variable_type_unknown;
  m_page   = 0;
  m_offset = 0;
  m_bitpos = 0;
  m_size   = 0;
  m_access = MDF_REG_ACCESS_READ_WRITE;

  m_bgcolor   = 0xffffff;
  m_fgcolor   = 0x000000;

  // Clearup value list
  std::deque<CMDF_Value *>::iterator iterValue;
  for (iterValue = m_list_value.begin(); iterValue != m_list_value.end(); ++iterValue) {
    CMDF_Value *pRecordValue = *iterValue;
    if (nullptr != pRecordValue) {
      delete pRecordValue;
      pRecordValue = nullptr;
    }
  }

  m_list_value.clear();
  m_mapDescription.clear();
  m_mapInfoURL.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  getTypeString
//

std::string
CMDF_RemoteVariable::getTypeString(void)
{
  switch (m_type) {

    case remote_variable_type_string:
      return (std::string("String"));

    case remote_variable_type_boolean:
      return (std::string("Boolean"));

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
      return (std::string("Float"));

    case remote_variable_type_double:
      return (std::string("Double"));

    case remote_variable_type_date:
      return (std::string("Date"));

    case remote_variable_type_time:
      return (std::string("Time"));

    case remote_variable_type_unknown:
    default:
      return (std::string("Unknown Type"));
  }
}

///////////////////////////////////////////////////////////////////////////////
//  getTypeByteCount
//

uint16_t
CMDF_RemoteVariable::getTypeByteCount(void)
{
  uint16_t size = 0;

  switch (m_type) {

    case remote_variable_type_string:
      size = m_size;
      break;

    case remote_variable_type_boolean:
      size = 1;
      break;

    case remote_variable_type_int8_t:
    case remote_variable_type_uint8_t:
      size = 1;
      break;

    case remote_variable_type_int16_t:
    case remote_variable_type_uint16_t:
      size = 2;
      break;

    case remote_variable_type_int32_t:
    case remote_variable_type_uint32_t:
      size = 4;
      break;

    case remote_variable_type_int64_t:
    case remote_variable_type_uint64_t:
      size = 8;
      break;

    case remote_variable_type_float:
      size = 4;
      break;

    case remote_variable_type_double:
      size = 8;
      break;

    case remote_variable_type_date:
      size = 3; // YYMMDD
      break;

    case remote_variable_type_time:
      size = 3; // HHMMSS
      break;

    case remote_variable_type_unknown:
      size = 0;
    default:
      break;

  } // switch

  return size;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Bit::CMDF_Bit()
{
  m_name.clear();
  m_pos     = 0;
  m_width   = 1;
  m_default = 0;
  m_min     = 0;
  m_max     = 1;
  m_access  = MDF_REG_ACCESS_READ_WRITE;
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
  m_name.clear();
  m_pos     = 0;
  m_width   = 1;
  m_default = 0;
  m_min     = 0;
  m_max     = 1;
  m_access  = MDF_REG_ACCESS_READ_WRITE;

  // Clearup value list
  std::deque<CMDF_Value *>::iterator iterValue;
  for (iterValue = m_list_value.begin(); iterValue != m_list_value.end(); ++iterValue) {
    CMDF_Value *pRecordValue = *iterValue;
    if (nullptr != pRecordValue) {
      delete pRecordValue;
      pRecordValue = nullptr;
    }
  }

  m_list_value.clear();
  m_mapDescription.clear();
  m_mapInfoURL.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  setWidth
//

void
CMDF_Bit::setWidth(uint8_t width)
{
  // Zero width is not allowed
  // Set to default
  if (!width) {
    width = 1;
  }
  
  // Max width is a byte width == 8
  if (width > 8) {
    width = 8;
  }

  m_width = width;

  m_mask = 0;
  for (int k=m_pos; k<(m_pos + m_width); k++) {
    m_mask |= (1 << k);
  }

  setMax(0xff);  
}

///////////////////////////////////////////////////////////////////////////////
//  setMin
//

void
CMDF_Bit::setMin(uint8_t min)
{
  m_min = min & m_min;
}

///////////////////////////////////////////////////////////////////////////////
//  setMax
//

void
CMDF_Bit::setMax(uint8_t max)
{
  m_max = max & m_mask;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Register::CMDF_Register()
{
  m_page       = 0;
  m_offset     = 0;
  m_span       = 1;
  m_width      = 8;
  m_min        = 0;
  m_max        = 255;
  m_strDefault = "UNDEF";
  m_access     = MDF_REG_ACCESS_READ_WRITE;
  m_type       = MDF_REG_TYPE_STANDARD;
  m_size       = 1;

  m_bgcolor   = 0xffffff;
  m_fgcolor   = 0x000000;
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
    if (nullptr != pRecordBit) {
      delete pRecordBit;
      pRecordBit = nullptr;
    }
  }

  m_list_bit.clear();

  // Clear up value list
  std::deque<CMDF_Value *>::iterator iterValue;
  for (iterValue = m_list_value.begin(); iterValue != m_list_value.end(); ++iterValue) {
    CMDF_Value *pRecordValue = *iterValue;
    if (nullptr != pRecordValue) {
      delete pRecordValue;
      pRecordValue = nullptr;
    }
  }

  m_list_value.clear();
  m_mapDescription.clear();
  m_mapInfoURL.clear();

  m_page       = 0;
  m_offset     = 0;
  m_span       = 1;
  m_width      = 8;
  m_min        = 0;
  m_max        = 255;
  m_strDefault = "UNDEF";
  m_access     = MDF_REG_ACCESS_READ_WRITE;
  m_type       = MDF_REG_TYPE_STANDARD;
  m_size       = 1;

  m_bgcolor   = 0xffffff;
  m_fgcolor   = 0x000000;
}

///////////////////////////////////////////////////////////////////////////////
//  getDefault
//

bool
CMDF_Register::getDefault(uint8_t &default_value)
{
  if (m_strDefault.compare("UNDEF") == 0) {
    return false;
  }
  default_value = (uint8_t) strtol(m_strDefault.c_str(), nullptr, 0);
  return true;
};

///////////////////////////////////////////////////////////////////////////////
//  Assignment
//

CMDF_Register &
CMDF_Register::operator=(const CMDF_Register &other)
{
  m_name           = other.m_name;
  m_mapDescription = other.m_mapDescription;
  m_mapInfoURL     = other.m_mapInfoURL;

  m_page   = other.m_page;
  m_offset = other.m_offset;
  m_span   = other.m_span;
  m_width  = other.m_width;

  m_type = other.m_type;
  m_size = other.m_size;

  m_min = other.m_min;
  m_max = other.m_max;

  m_strDefault = other.m_strDefault;

  m_access = other.m_access;

  m_fgcolor = other.m_fgcolor;
  m_bgcolor = other.m_bgcolor;

  // Clear up bit list
  std::deque<CMDF_Bit *>::iterator iterBit;
  for (iterBit = m_list_bit.begin(); iterBit != m_list_bit.end(); ++iterBit) {
    CMDF_Bit *pRecordBit = *iterBit;
    if (nullptr != pRecordBit) {
      delete pRecordBit;
      pRecordBit = nullptr;
    }
  }
  m_list_bit.clear();

  // Assign
  m_list_bit = other.m_list_bit;

  // Clear up value list
  std::deque<CMDF_Value *>::iterator iterValue;
  for (iterValue = m_list_value.begin(); iterValue != m_list_value.end(); ++iterValue) {
    CMDF_Value *pRecordValue = *iterValue;
    if (nullptr != pRecordValue) {
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
  m_name.clear();
  m_offset = 0;
  m_min    = 0;
  m_max    = 0;
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
  m_name.clear();
  m_offset = 0;
  m_min    = 0;
  m_max    = 0;

  // Clearup bit list
  std::deque<CMDF_Bit *>::iterator iterBit;
  for (iterBit = m_list_bit.begin(); iterBit != m_list_bit.end(); ++iterBit) {
    CMDF_Bit *pRecordBit = *iterBit;
    if (nullptr != pRecordBit) {
      delete pRecordBit;
      pRecordBit = nullptr;
    }
  }

  m_list_bit.clear();

  // Clearup value list
  std::deque<CMDF_Value *>::iterator iterValue;
  for (iterValue = m_list_value.begin(); iterValue != m_list_value.end(); ++iterValue) {
    CMDF_Value *pRecordValue = *iterValue;
    if (nullptr != pRecordValue) {
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
  m_name.clear();
  m_code = 0;
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
  m_name.clear();
  m_code = 0;

  // Cleanup action parameter list
  std::deque<CMDF_ActionParameter *>::iterator iterActionParam;
  for (iterActionParam = m_list_ActionParameter.begin(); iterActionParam != m_list_ActionParameter.end();
       ++iterActionParam) {
    CMDF_ActionParameter *pRecordActionParam = *iterActionParam;
    if (nullptr != pRecordActionParam) {
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
  m_level       = 1;
  m_startPage   = 0;
  m_startOffset = 0;
  m_rowCount    = 0;
  m_rowSize     = 8;
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
  m_level       = 1;
  m_startPage   = 0;
  m_startOffset = 0;
  m_rowCount    = 0;
  m_rowSize     = 8;

  m_list_action.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_EventData::CMDF_EventData()
{
  m_name.clear();
  m_offset = 0;
}

CMDF_EventData::~CMDF_EventData()
{
  m_name.clear();
  m_offset = 0;

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
    if (nullptr != pRecordBit) {
      delete pRecordBit;
      pRecordBit = nullptr;
    }
  }
  m_list_bit.clear();

  // Clearup value list
  std::deque<CMDF_Value *>::iterator iterValue;
  for (iterValue = m_list_value.begin(); iterValue != m_list_value.end(); ++iterValue) {
    CMDF_Value *pRecordValue = *iterValue;
    if (nullptr != pRecordValue) {
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
  m_name.clear();
  m_class     = 0;
  m_type      = 0;
  m_priority  = 7;
  m_direction = MDF_EVENT_DIR_OUT;
}

CMDF_Event::~CMDF_Event()
{
  m_name.clear();
  m_class     = 0;
  m_type      = 0;
  m_priority  = 7;
  m_direction = MDF_EVENT_DIR_OUT;

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
    if (nullptr != pRecordEventData) {
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
  m_name.clear();
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
  m_nAlgorithm  = 0;
  m_nBlockSize  = 0;
  m_nBlockCount = 0;
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

CMDF_Address::CMDF_Address()
{
  clearStorage();
}

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
  clearStorage();
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
  m_address.clearStorage();

  // Cleanup Phone list
  std::deque<CMDF_Item *>::iterator iterPhone;
  for (iterPhone = m_list_Phone.begin(); iterPhone != m_list_Phone.end(); ++iterPhone) {
    CMDF_Item *pRecordPhone = *iterPhone;
    if (nullptr != pRecordPhone) {
      delete pRecordPhone;
      pRecordPhone = nullptr;
    }
  }
  m_list_Phone.clear();

  // Cleanup Fax list
  std::deque<CMDF_Item *>::iterator iterFax;
  for (iterFax = m_list_Fax.begin(); iterFax != m_list_Fax.end(); ++iterFax) {
    CMDF_Item *pRecordFax = *iterFax;
    if (nullptr != pRecordFax) {
      delete pRecordFax;
      pRecordFax = nullptr;
    }
  }
  m_list_Fax.clear();

  // Cleanup mail list
  std::deque<CMDF_Item *>::iterator iterEmail;
  for (iterEmail = m_list_Email.begin(); iterEmail != m_list_Email.end(); ++iterEmail) {
    CMDF_Item *pRecordEmail = *iterEmail;
    if (nullptr != pRecordEmail) {
      delete pRecordEmail;
    }
  }
  m_list_Email.clear();

  // Cleanup web list
  std::deque<CMDF_Item *>::iterator iterweb;
  for (iterweb = m_list_Web.begin(); iterweb != m_list_Web.end(); ++iterweb) {
    CMDF_Item *pRecordWeb = *iterweb;
    if (nullptr != pRecordWeb) {
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
  m_strName.clear();
  m_strURL.clear();
  m_strFormat.clear();
  m_mapDescription.clear();
  m_mapInfoURL.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Video::CMDF_Video()
{
  clearStorage();
}

CMDF_Video::~CMDF_Video()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_Video::clearStorage(void)
{
  m_strName.clear();
  m_strURL.clear();
  m_strFormat.clear();
  m_mapDescription.clear();
  m_mapInfoURL.clear();
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
  m_strName.clear();
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
  m_mapInfoURL.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Driver::CMDF_Driver()
{
  clearStorage();
}

CMDF_Driver::~CMDF_Driver()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_Driver::clearStorage(void)
{
  m_strName.clear();
  m_strURL.clear();
  m_strName.clear();
  m_strDate.clear();
  m_strOS.clear();
  m_strOSVer.clear();
  m_strMd5.clear();

  m_version_major = 0;
  m_version_minor = 0;
  m_version_patch = 0;

  m_mapDescription.clear();
  m_mapInfoURL.clear();
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
  m_strName.clear();
  m_strURL.clear();
  m_strFormat.clear();
  m_mapDescription.clear();
  m_mapInfoURL.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Setup::CMDF_Setup()
{
  clearStorage();
}

CMDF_Setup::~CMDF_Setup()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_Setup::clearStorage(void)
{
  m_strName.clear();
  m_strURL.clear();
  m_strFormat.clear();
  m_mapDescription.clear();
  m_mapInfoURL.clear();
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
    if (nullptr != pRecordEvent) {
      delete pRecordEvent;
      pRecordEvent = nullptr;
    }
  }
  m_list_event.clear();

  // Clean up register list
  std::deque<CMDF_Register *>::iterator iterRegister;
  for (iterRegister = m_list_register.begin(); iterRegister != m_list_register.end(); ++iterRegister) {
    CMDF_Register *pRecordRegister = *iterRegister;
    if (nullptr != pRecordRegister) {
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
    if (nullptr != pRecordRemoteVariable) {
      delete pRecordRemoteVariable;
      pRecordRemoteVariable = nullptr;
    }
  }
  m_list_remotevar.clear();

  // Clear alarm bit list
  std::deque<CMDF_Bit *>::iterator iterAlarmBits;
  for (iterAlarmBits = m_list_alarm.begin(); iterAlarmBits != m_list_alarm.end(); ++iterAlarmBits) {
    CMDF_Bit *pRecordAlarmBits = *iterAlarmBits;
    if (nullptr != pRecordAlarmBits) {
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
    if (nullptr != pRecordPicture) {
      pRecordPicture->clearStorage();
      delete pRecordPicture;
      pRecordPicture = nullptr;
    }
  }
  m_list_picture.clear();

  // Clean up video list
  std::deque<CMDF_Video *>::iterator iterVideo;
  for (iterVideo = m_list_video.begin(); iterVideo != m_list_video.end(); ++iterVideo) {
    CMDF_Video *pRecordVideo = *iterVideo;
    if (nullptr != pRecordVideo) {
      pRecordVideo->clearStorage();
      delete pRecordVideo;
      pRecordVideo = nullptr;
    }
  }
  m_list_video.clear();

  // Clean up firmware list
  std::deque<CMDF_Firmware *>::iterator iterFirmware;
  for (iterFirmware = m_list_firmware.begin(); iterFirmware != m_list_firmware.end(); ++iterFirmware) {
    CMDF_Firmware *pRecordFirmware = *iterFirmware;
    if (nullptr != pRecordFirmware) {
      pRecordFirmware->clearStorage();
      delete pRecordFirmware;
      pRecordFirmware = nullptr;
    }
  }
  m_list_firmware.clear();

  // Clean up driver list
  std::deque<CMDF_Driver *>::iterator iterDriver;
  for (iterDriver = m_list_driver.begin(); iterDriver != m_list_driver.end(); ++iterDriver) {
    CMDF_Driver *pRecordDriver = *iterDriver;
    if (nullptr != pRecordDriver) {
      pRecordDriver->clearStorage();
      delete pRecordDriver;
      pRecordDriver = nullptr;
    }
  }
  m_list_driver.clear();

  // Clean up manual list
  std::deque<CMDF_Manual *>::iterator iterManual;
  for (iterManual = m_list_manual.begin(); iterManual != m_list_manual.end(); ++iterManual) {
    CMDF_Manual *pRecordManual = *iterManual;
    if (nullptr != pRecordManual) {
      pRecordManual->clearStorage();
      delete pRecordManual;
      pRecordManual = nullptr;
    }
  }
  m_list_manual.clear();

  // Clean up setup list
  std::deque<CMDF_Setup *>::iterator iterSetup;
  for (iterSetup = m_list_setup.begin(); iterSetup != m_list_setup.end(); ++iterSetup) {
    CMDF_Setup *pRecordSetup = *iterSetup;
    if (nullptr != pRecordSetup) {
      pRecordSetup->clearStorage();
      delete pRecordSetup;
      pRecordSetup = nullptr;
    }
  }
  m_list_setup.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  downLoadMDF
//
// https://stackoverflow.com/questions/1636333/download-file-using-libcurl-in-c-c
//

static size_t
write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
}

CURLcode
CMDF::downLoadMDF(std::string &url, std::string &tempFileName)
{
  CURL *curl;
  FILE *fp;
  CURLcode res;
  // char *url = "http://localhost/aaa.txt";
  // char outfilename[FILENAME_MAX] = "C:\\bbb.txt";
  curl = curl_easy_init();
  if (curl) {
    fp = fopen(tempFileName.c_str(), "wb");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    res = curl_easy_perform(curl);
    // always cleanup
    curl_easy_cleanup(curl);
    fclose(fp);
  }
  return res;
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

// * * * Parser Helpers * * *

bool
__getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr)
{
  CMDF_Value *pValue = new CMDF_Value;
  if (nullptr == pValue) {
    spdlog::error("Parse-XML: handleMDFParserData: Failed to allocate memory for value structure");
    return false;
  }

  pvaluelist->push_back(pValue);

  for (int i = 0; attr[i]; i += 2) {

    std::string attribute = attr[i + 1];
    vscp_trim(attribute);
    vscp_makeLower(attribute);
    //std::cout << "Attribute: " << attribute << " size: " << pvaluelist->size() << std::endl;

    if (0 == strcasecmp(attr[i], "name")) {
      pValue->m_name = attribute;
    }
    else if (0 == strcasecmp(attr[i], "value")) {
      pValue->m_strValue = attribute;
    }
  }
  return true;
}

bool
__getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr)
{
  CMDF_Bit *pBits = new CMDF_Bit;
  if (nullptr == pBits) {
    spdlog::error("Parse-XML: handleMDFParserData: Failed to allocate memory for bit structure");
    return false;
  }

  pbitlist->push_back(pBits);

  for (int i = 0; attr[i]; i += 2) {

    std::string attribute = attr[i + 1];
    vscp_trim(attribute);
    vscp_makeLower(attribute);

    if (0 == strcasecmp(attr[i], "name")) {
      vscp_trim(attribute);
      vscp_makeLower(attribute);
      pBits->m_name = attribute;
    }
    else if (0 == strcasecmp(attr[i], "pos")) {
      pBits->m_pos = vscp_readStringValue(attribute);
    }
    else if (0 == strcasecmp(attr[i], "width")) {
      pBits->m_width = vscp_readStringValue(attribute);
    }
    else if (0 == strcasecmp(attr[i], "default")) {
      if (attribute == "true") {
        pBits->m_default = true;
      }
      else if (attribute == "false") {
        pBits->m_default = false;
      }
      else {
        pBits->m_default = vscp_readStringValue(attribute);
      }
    }
    else if (0 == strcasecmp(attr[i], "min")) {
      pBits->m_min = vscp_readStringValue(attribute);
    }
    else if (0 == strcasecmp(attr[i], "max")) {
      pBits->m_max = vscp_readStringValue(attribute);
    }
    else if (0 == strcasecmp(attr[i], "access")) {
      // Register access
      std::string strAccess = attribute;
      vscp_trim(strAccess);
      vscp_makeLower(strAccess);
      pBits->m_access = MDF_REG_ACCESS_NONE;
      if (strAccess == "w") {
        pBits->m_access = MDF_REG_ACCESS_WRITE_ONLY;
        spdlog::debug("Parse-XML: Register access: Read Only");
      }
      else if (strAccess == "r") {
        pBits->m_access = MDF_REG_ACCESS_READ_ONLY;
        spdlog::debug("Parse-XML: Register access: Write Only");
      }
      else if (strAccess == "rw") {
        pBits->m_access = MDF_REG_ACCESS_READ_WRITE;
        spdlog::debug("Parse-XML: Register access: Read/Write");
      }
    }
  }
  return true;
}

// ----------------------------------------------------------------------------

// std::string gToken;        // Current token                                 (gTokenList.front())
// std::string gTokenParent;  // This is the parent to the current token       (gTokenList.at(1))
std::string gLastLanguage; // Last language ISO two diget code (name/description)

std::deque<std::string> gTokenList;

int gdepth_xml_parser = 0;

// bool gbVscp   = false;  (gTokenList.back() == "vscp")
// bool gbModule = false;  (gTokenList.at(gTokenList.size()-2) == "module")

CMDF_Item *gpItemStruct;                    // Holds temporary items
CMDF_Bit *gpBitStruct;                      // Holds temporary bits
CMDF_Value *gpValueStruct;                  // Holds temporary values
CMDF_Picture *gpPictureStruct;              // Holds temporary picture items
CMDF_Video *gpVideoStruct;                  // Holds temporary video items
CMDF_Firmware *gpFirmwareStruct;            // Holds temporary firmware items
CMDF_Driver *gpDriverStruct;                // Holds temporary driver items
CMDF_Manual *gpManualStruct;                // Holds temporary manual items
CMDF_Setup *gpSetupStruct;                  // Holds temporary setup items
CMDF_Register *gpRegisterStruct;            // Holds temporary register items
CMDF_RemoteVariable *gpRvarStruct;          // Holds temporary remote variable items
CMDF_ActionParameter *gpActionParamStruct;  // Holds temporary action parameter items
CMDF_Action *gpActionStruct;                // Holds temporary action items
CMDF_Event *gpEventStruct;                  // Holds temporary event items
CMDF_EventData *gpEventDataStruct;          // Holds temporary event data items

// clang-format off

// ----------------------------------------------------------------------------

void
__startSetupMDFParser(void *data, const char *name, const char **attr)
{
  CMDF *pmdf = (CMDF *) data;
  if (nullptr == pmdf) {
    spdlog::trace("Parse-XML: ---> startSetupMDFParser: Data object is invalid");
    return;
  }

  spdlog::trace("Parse-XML: <--- startSetupMDFParser: Tag: {0} Depth: {1}", name, gdepth_xml_parser);

  // Save token
  std::string currentToken = name;
  vscp_trim(currentToken);
  vscp_makeLower(currentToken);
  gTokenList.push_front(currentToken);

  // Default language
  gLastLanguage = "en";

  // Set language for 'name', 'infourl' and 'description'
  if ((currentToken == "name") || (currentToken == "description") || (currentToken == "infourl")) {
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

  // Verify structure <vscp><module>.....</module></vscp>
  if (gdepth_xml_parser >= 2 && (gTokenList.at(gTokenList.size() - 2) != "module") || (gTokenList.back() != "vscp")) {
    spdlog::error("Parse-XML: startSetupMDFParser: Invalid structure");
    return;
  }

  switch (gdepth_xml_parser) {

    case 0: // Root
      if (currentToken == "vscp") {
        ;
      }
      break;

    case 1:
      if ((gTokenList.back() == "vscp") && (currentToken == "module")) {
        ;
      }
      break;

    case 2:
      if (currentToken == "manufacturer") {}
      // (gTokenList.at(gTokenList.size()-2) == "module")
      else if (currentToken == "files") {
        // picture/firmware/manual etc entries under here
      }
      // * * * NOTE! * * *
      // This is an old deprecated form with one picture element
      // Now under the <files> tag
      else if (currentToken == "picture") {
        gpPictureStruct = new CMDF_Picture;
        if (nullptr == gpPictureStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for picture");
          return;
        }
        pmdf->m_list_picture.push_back(gpPictureStruct);

        for (int i = 0; attr[i]; i += 2) {
          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          //vscp_makeLower(attribute);
          if (0 == strcasecmp(attr[i], "name")) {
            if (!attribute.empty()) {
              gpPictureStruct->m_strName = attribute;
            }
          }
          else if ((0 == strcasecmp(attr[i], "path")) || (0 == strcasecmp(attr[i], "url"))) {
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
      else if (currentToken == "firmware") {
        gpFirmwareStruct = new CMDF_Firmware;
        if (nullptr == gpFirmwareStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for firmware");
          return;
        }

        pmdf->m_list_firmware.push_back(gpFirmwareStruct);

        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          //vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "name")) {
            if (!attribute.empty()) {
              gpFirmwareStruct->m_strName = attribute;
            }
          }
          else if (0 == strcasecmp(attr[i], "target")) {
            // Target for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware target: {0}", attribute);
            gpFirmwareStruct->m_strTarget = attribute;
          }
          else if (0 == strcasecmp(attr[i], "targetcode")) {
            // Target for for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware target code: {0}", attribute);
            gpFirmwareStruct->m_targetCode = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "version_major")) {
            // Version for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware version major: {0}", attribute);
            gpFirmwareStruct->m_version_major = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "version_minor")) {
            // Version for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware version minor: {0}", attribute);
            gpFirmwareStruct->m_version_minor = vscp_readStringValue(attribute);
          }
          else if ((0 == strcasecmp(attr[i], "version_patch")) || (0 == strcasecmp(attr[i], "version_subminor"))) {
            // Version for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware version patch: {0}", attribute);
            gpFirmwareStruct->m_version_patch = vscp_readStringValue(attribute);
          }
          else if ((0 == strcasecmp(attr[i], "url")) || (0 == strcasecmp(attr[i], "path"))) {
            // URL for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware url: {0}", attribute);
            gpFirmwareStruct->m_strURL = attribute;
          }
          else if (0 == strcasecmp(attr[i], "md5sum")) {
            // MD5 for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware md5: {0}", attribute);
            gpFirmwareStruct->m_strMd5 = attribute;
          }
          else if (0 == strcasecmp(attr[i], "size")) {
            // Size for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware size: {0}", attribute);
            gpFirmwareStruct->m_size = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "date")) {
            // Date for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware date: {0}", attribute);
            gpFirmwareStruct->m_strDate = attribute;
          }
          else if (0 == strcasecmp(attr[i], "format")) {
            // Date for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware date: {0}", attribute);
            gpFirmwareStruct->m_strFormat = attribute;
          }
        }
      }
      // * * * NOTE! * * *
      // This is an old deprecated form with one manual element
      // Now under the <files> tag
      else if (currentToken == "manual") {
        gpManualStruct = new CMDF_Manual;
        if (nullptr == gpManualStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for manual");
          return;
        }

        pmdf->m_list_manual.push_back(gpManualStruct);

        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          //vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "name")) {
            if (!attribute.empty()) {
              gpManualStruct->m_strName = attribute;
            }
          }
          else if ((0 == strcasecmp(attr[i], "path")) || (0 == strcasecmp(attr[i], "url"))) {
            // Path/url for manual
            spdlog::trace("Parse-XML: handleMDFParserData: Module manual path/url: {0}", attribute);
            gpManualStruct->m_strURL = attribute;
          }
          else if (0 == strcasecmp(attr[i], "lang")) {
            // Language for manual
            spdlog::trace("Parse-XML: handleMDFParserData: Module manual language: {0}", attribute);
            gpManualStruct->m_strLanguage = attribute;
          }
          else if (0 == strcasecmp(attr[i], "format")) {
            // Format for manual
            spdlog::trace("Parse-XML: handleMDFParserData: Module manual format: {0}", attribute);
            gpManualStruct->m_strFormat = attribute;
          }
        }
      }
      else if (currentToken == "boot") {
        // Nothing to do here
      }
      else if (currentToken == "registers") {
        // Nothing to do here
      }
      else if (currentToken == "register") {
        // Nothing to do here
      }
      else if (currentToken == "abstractions") {
        // Nothing to do here
      }
      else if (currentToken == "remotevar") {
        // Nothing to do here
      }
      else if (currentToken == "dmatrix") {
        // Nothing to do here
      }
      else if (currentToken == "events") {
        // Nothing to do here
      }
      break;

    // ------------------------------------------------------------------------

    case 3:

      if ((currentToken == "address")) {
        ;
      }
      else if ((currentToken == "telephone")) {
        gpItemStruct = new CMDF_Item;
      }
      else if ((currentToken == "fax")) {
        gpItemStruct = new CMDF_Item;
      }
      else if ((currentToken == "email")) {
        gpItemStruct = new CMDF_Item;
      }
      else if ((currentToken == "web")) {
        gpItemStruct = new CMDF_Item;
      }
      // [3] Picture (standard format)
      else if ((currentToken == "picture")) {

        gpPictureStruct = new CMDF_Picture;
        if (nullptr == gpPictureStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for picture");
          return;
        }

        pmdf->m_list_picture.push_back(gpPictureStruct);

        for (int i = 0; attr[i]; i += 2) {
          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "name")) {
            if (!attribute.empty()) {
              gpPictureStruct->m_strName = attribute;
            }
          }
          else if ((0 == strcasecmp(attr[i], "path")) || (0 == strcasecmp(attr[i], "url"))) {
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
      // [3] Video (standard format)
      else if ((currentToken == "video")) {

        gpVideoStruct = new CMDF_Video;
        if (nullptr == gpVideoStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for video");
          return;
        }

        pmdf->m_list_video.push_back(gpVideoStruct);

        for (int i = 0; attr[i]; i += 2) {
          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "name")) {
            if (!attribute.empty()) {
              gpVideoStruct->m_strName = attribute;
            }
          }
          else if ((0 == strcasecmp(attr[i], "path")) || (0 == strcasecmp(attr[i], "url"))) {
            if (!attribute.empty()) {
              gpVideoStruct->m_strURL = attribute;
            }
          }
          else if (0 == strcasecmp(attr[i], "format")) {
            if (!attribute.empty()) {
              gpVideoStruct->m_strFormat = attribute;
            }
          }
        }
      }      
      // [3] Firmware  (standard format)
      else if ((currentToken == "firmware")) {
        gpFirmwareStruct = new CMDF_Firmware;
        if (nullptr == gpFirmwareStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for firmware");
          return;
        }

        pmdf->m_list_firmware.push_back(gpFirmwareStruct);

        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "name")) {
            if (!attribute.empty()) {
              gpFirmwareStruct->m_strName = attribute;
            }
          }
          else if (0 == strcasecmp(attr[i], "target")) {
            // Target for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware target: {0}", attribute);
            gpFirmwareStruct->m_strTarget = attribute;
          }
          else if (0 == strcasecmp(attr[i], "targetcode")) {
            // Target for for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware target code: {0}", attribute);
            gpFirmwareStruct->m_targetCode = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "version_major")) {
            // Version for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware version major: {0}", attribute);
            gpFirmwareStruct->m_version_major = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "version_minor")) {
            // Version for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware version minor: {0}", attribute);
            gpFirmwareStruct->m_version_minor = vscp_readStringValue(attribute);
          }
          else if ((0 == strcasecmp(attr[i], "version_patch")) || (0 == strcasecmp(attr[i], "version_subminor"))) {
            // Version for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware version patch: {0}", attribute);
            gpFirmwareStruct->m_version_patch = vscp_readStringValue(attribute);
          }
          else if ((0 == strcasecmp(attr[i], "url")) || (0 == strcasecmp(attr[i], "path"))) {
            // URL for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware url: {0}", attribute);
            gpFirmwareStruct->m_strURL = attribute;
          }
          else if (0 == strcasecmp(attr[i], "md5sum")) {
            // MD5 for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware md5: {0}", attribute);
            gpFirmwareStruct->m_strMd5 = attribute;
          }
          else if (0 == strcasecmp(attr[i], "size")) {
            // Size for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware size: {0}", attribute);
            gpFirmwareStruct->m_size = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "date")) {
            // Date for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware date: {0}", attribute);
            gpFirmwareStruct->m_strDate = attribute;
          }
          else if (0 == strcasecmp(attr[i], "format")) {
            // Date for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware date: {0}", attribute);
            gpFirmwareStruct->m_strFormat = attribute;
          }
        }
      }
      // [3] Driver (standard format)
      else if ((currentToken == "driver")) {

        gpDriverStruct = new CMDF_Driver;
        if (nullptr == gpDriverStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for driver");
          return;
        }

        pmdf->m_list_driver.push_back(gpDriverStruct);

        for (int i = 0; attr[i]; i += 2) {
          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "name")) {
            if (!attribute.empty()) {
              gpDriverStruct->m_strName = attribute;
            }
          }
          else if (0 == strcasecmp(attr[i], "name")) {
            // Name of driver
            spdlog::trace("Parse-XML: handleMDFParserData: Driver name: {0}", attribute);
            gpDriverStruct->m_strName = attribute;
          }
          else if (0 == strcasecmp(attr[i], "type")) {
            // Driver type
            spdlog::trace("Parse-XML: handleMDFParserData: Driver type: {0}", attribute);
            gpDriverStruct->m_strType = attribute;
          }
          else if (0 == strcasecmp(attr[i], "version_major")) {
            // Version for driver
            spdlog::trace("Parse-XML: handleMDFParserData: Driver version major: {0}", attribute);
            gpDriverStruct->m_version_major = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "version_minor")) {
            // Version for driver
            spdlog::trace("Parse-XML: handleMDFParserData: Driver version minor: {0}", attribute);
            gpDriverStruct->m_version_minor = vscp_readStringValue(attribute);
          }
          else if ((0 == strcasecmp(attr[i], "version_patch")) || (0 == strcasecmp(attr[i], "version_subminor"))) {
            // Version for driver
            spdlog::trace("Parse-XML: handleMDFParserData: Driver version patch: {0}", attribute);
            gpDriverStruct->m_version_patch = vscp_readStringValue(attribute);
          }
          else if ((0 == strcasecmp(attr[i], "url")) || (0 == strcasecmp(attr[i], "path"))) {
            // URL for driver
            spdlog::trace("Parse-XML: handleMDFParserData: Driver url: {0}", attribute);
            gpDriverStruct->m_strURL = attribute;
          }
          else if (0 == strcasecmp(attr[i], "md5sum")) {
            // MD5 for driver file
            spdlog::trace("Parse-XML: handleMDFParserData: Driver md5: {0}", attribute);
            gpDriverStruct->m_strMd5 = attribute;
          }
          else if (0 == strcasecmp(attr[i], "os")) {
            // OS for driver
            spdlog::trace("Parse-XML: handleMDFParserData: Driver OS: {0}", attribute);
            gpDriverStruct->m_strOS = attribute;
          }
          else if (0 == strcasecmp(attr[i], "osver")) {
            // OS versin for driver
            spdlog::trace("Parse-XML: handleMDFParserData: Driver OS: {0}", attribute);
            gpDriverStruct->m_strOSVer = attribute;
          }
          else if (0 == strcasecmp(attr[i], "date")) {
            // Date for firmware
            spdlog::trace("Parse-XML: handleMDFParserData: Module firmware date: {0}", attribute);
            gpDriverStruct->m_strDate = attribute;
          }
        }
      }
      // [3] Manual  (standard format)
      else if ((currentToken == "manual")) {
        gpManualStruct = new CMDF_Manual;
        if (nullptr == gpManualStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for manual");
          return;
        }

        pmdf->m_list_manual.push_back(gpManualStruct);

        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "name")) {
            if (!attribute.empty()) {
              gpManualStruct->m_strName = attribute;
            }
          }
          else if ((0 == strcasecmp(attr[i], "path")) || (0 == strcasecmp(attr[i], "url"))) {
            // Path/url for manual
            spdlog::trace("Parse-XML: handleMDFParserData: Module manual path/url: {0}", attribute);
            gpManualStruct->m_strURL = attribute;
          }
          else if (0 == strcasecmp(attr[i], "lang")) {
            // Language for manual
            spdlog::trace("Parse-XML: handleMDFParserData: Module manual language: {0}", attribute);
            gpManualStruct->m_strLanguage = attribute;
          }
          else if (0 == strcasecmp(attr[i], "format")) {
            // Format for manual
            spdlog::trace("Parse-XML: handleMDFParserData: Module manual format: {0}", attribute);
            gpManualStruct->m_strFormat = attribute;
          }
        }
      }
      // [3] Picture (standard format)
      else if ((currentToken == "setup")) {

        gpSetupStruct = new CMDF_Setup;
        if (nullptr == gpSetupStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for setup");
          return;
        }

        pmdf->m_list_setup.push_back(gpSetupStruct);

        for (int i = 0; attr[i]; i += 2) {
          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "name")) {
            if (!attribute.empty()) {
              gpSetupStruct->m_strName = attribute;
            }
          }
          else if ((0 == strcasecmp(attr[i], "path")) || (0 == strcasecmp(attr[i], "url"))) {
            if (!attribute.empty()) {
              gpSetupStruct->m_strURL = attribute;
            }
          }
          else if (0 == strcasecmp(attr[i], "format")) {
            if (!attribute.empty()) {
              gpSetupStruct->m_strFormat = attribute;
            }
          }
        }
      }
      // Boot  (standard format)
      else if ((currentToken == "boot")) {
        ;
      }
      // [3] reg  (register definitions)
      else if ((currentToken == "reg")) {
        gpRegisterStruct = new CMDF_Register;
        if (nullptr == gpRegisterStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for register");
          return;
        }

        pmdf->m_list_register.push_back(gpRegisterStruct);

        // Get register attributes
        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "name")) {
            // Register name
            spdlog::trace("Parse-XML: handleMDFParserData: Register name: {0}", attribute);
            gpRegisterStruct->m_name = attribute;
          }
          else if (0 == strcasecmp(attr[i], "page")) {
            // Register page
            spdlog::trace("Parse-XML: handleMDFParserData: Register page: {0}", attribute);
            gpRegisterStruct->m_page = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "offset")) {
            // Register offset
            spdlog::trace("Parse-XML: handleMDFParserData: Register offset: {0}", attribute);
            gpRegisterStruct->m_offset = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "span")) {
            // Register span
            spdlog::trace("Parse-XML: handleMDFParserData: Register span: {0}", attribute);
            gpRegisterStruct->m_span = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "width")) {
            // Register width
            spdlog::trace("Parse-XML: handleMDFParserData: Register width: {0}", attribute);
            gpRegisterStruct->m_width = vscp_readStringValue(attribute);
            if (gpRegisterStruct->m_width > 8) {
              gpRegisterStruct->m_width = 8;
            }
          }
          else if (0 == strcasecmp(attr[i], "default")) {
            // Register default
            spdlog::trace("Parse-XML: handleMDFParserData: Register default: {0}", attribute);
            gpRegisterStruct->m_strDefault = attribute;
          }
          else if (0 == strcasecmp(attr[i], "access")) {
            // Register access
            std::string strAccess = attribute;
            vscp_trim(strAccess);
            vscp_makeLower(strAccess);
            gpRegisterStruct->m_access = MDF_REG_ACCESS_NONE;
            if (strAccess == "w") {
              gpRegisterStruct->m_access = MDF_REG_ACCESS_WRITE_ONLY;
              spdlog::debug("Parse-XML: Register access: Read Only");
            }
            else if (strAccess == "r") {
              gpRegisterStruct->m_access = MDF_REG_ACCESS_READ_ONLY;
              spdlog::debug("Parse-XML: Register access: Write Only");
            }
            else if (strAccess == "rw") {
              gpRegisterStruct->m_access = MDF_REG_ACCESS_READ_WRITE;
              spdlog::debug("Parse-XML: Register access: Read/Write");
            }
          }
          else if (0 == strcasecmp(attr[i], "min")) {
            // Register min
            spdlog::trace("Parse-XML: handleMDFParserData: Register min: {0}", attribute);
            gpRegisterStruct->m_min = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "max")) {
            // Register max
            spdlog::trace("Parse-XML: handleMDFParserData: Register max: {0}", attribute);
            gpRegisterStruct->m_max = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "type")) {
            // Register type
            spdlog::trace("Parse-XML: handleMDFParserData: Register type: {0}", attribute);
            std::string strRegType = attribute;
            vscp_trim(strRegType);
            vscp_makeLower(strRegType);
            gpRegisterStruct->m_type = MDF_REG_TYPE_STANDARD;
            if ((strRegType == "standard") || (strRegType == "std")) {
              gpRegisterStruct->m_type = MDF_REG_TYPE_STANDARD;
              spdlog::debug("Parse-XML: Register type: standard");
            }
            else if ((strRegType == "dmatrix") || (strRegType == "dmatrix1") || (strRegType == "dm")) {
              gpRegisterStruct->m_type = MDF_REG_TYPE_DMATRIX1;
              spdlog::debug("Parse-XML: Register type: dmatrix");
            }
            else if ((strRegType == "block") || (strRegType == "blk")) {
              gpRegisterStruct->m_type = MDF_REG_TYPE_BLOCK;
              spdlog::debug("Parse-XML: Register type: dmatrix");
            }
            else {
              gpRegisterStruct->m_type = static_cast<mdf_register_type>(vscp_readStringValue(attribute));
              spdlog::debug("Parse-XML: Register type: {0}", gpRegisterStruct->m_type);
            }
          }
          else if (0 == strcasecmp(attr[i], "fgcolor")) {
            // Register foreground color
            spdlog::trace("Parse-XML: handleMDFParserData: Register fgcolor: {0}", attribute);
            gpRegisterStruct->m_fgcolor = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "bgcolor")) {
            // Register background color
            spdlog::trace("Parse-XML: handleMDFParserData: Register bgcolor: {0}", attribute);
            gpRegisterStruct->m_bgcolor = vscp_readStringValue(attribute);
          }
        }
      }
      // [3] reg  (register definitions)
      else if ((currentToken == "remotevar") || (currentToken == "abstraction")) {
        gpRvarStruct = new CMDF_RemoteVariable;
        if (nullptr == gpRvarStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for remote variable");
          return;
        }

        pmdf->m_list_remotevar.push_back(gpRvarStruct);

        // Get register attributes
        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "name")) {
            // Remote variable name
            spdlog::trace("Parse-XML: handleMDFParserData: Remote variable name: {0}", attribute);
            gpRvarStruct->m_name = attribute;
          }
          else if (0 == strcasecmp(attr[i], "default")) {
            // Remote variable default value
            spdlog::trace("Parse-XML: handleMDFParserData: Remote variable default: {0}", attribute);
            gpRvarStruct->m_strDefault = attribute;
          }
          else if (0 == strcasecmp(attr[i], "bitpos")) {
            // Remote variable bit position for boolean
            spdlog::trace("Parse-XML: handleMDFParserData: Remote variable bitpos: {0}", attribute);
            gpRvarStruct->m_bitpos = vscp_readStringValue(attribute) & 7;
          }
          else if (0 == strcasecmp(attr[i], "size")) {
            // Remote variable string max size
            spdlog::trace("Parse-XML: handleMDFParserData: Remote variable string max size: {0}", attribute);
            gpRvarStruct->m_size = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "access")) {
            // Remote variable access
            spdlog::trace("Parse-XML: handleMDFParserData: Remote variable access: {0}", attribute);
            gpRvarStruct->m_size = vscp_readStringValue(attribute);
            gpRvarStruct->m_access = MDF_REG_ACCESS_NONE;
            if (attribute == "w") {
              gpRvarStruct->m_access = MDF_REG_ACCESS_WRITE_ONLY;
              spdlog::trace("Parse-XML: Register access: Read Only");
            }
            else if (attribute == "r") {
              gpRvarStruct->m_access = MDF_REG_ACCESS_READ_ONLY;
              spdlog::trace("Parse-XML: Register access: Write Only");
            }
            else if (attribute == "rw") {
              gpRvarStruct->m_access = MDF_REG_ACCESS_READ_WRITE;
              spdlog::trace("Parse-XML: Register access: Read/Write");
            }
          }
          else if (0 == strcasecmp(attr[i], "type")) {
            // Remote variable type
            spdlog::trace("Parse-XML: handleMDFParserData: Remote variable fgcolor: {0}", attribute);
            if (attribute == "string") {
              gpRvarStruct->m_type = remote_variable_type_string;
              spdlog::debug("Parse-XML: Remote variable type set to 'string' {0}.", gpRvarStruct->m_type);
            }
            else if (attribute == "bool") {
              gpRvarStruct->m_type = remote_variable_type_boolean;
              spdlog::debug("Parse-XML: Remote variable type set to 'boolena' {0}.", gpRvarStruct->m_type);
            }
            else if ((attribute == "int8") || (attribute == "int8_t") || (attribute == "char")) {
              gpRvarStruct->m_type = remote_variable_type_int8_t;
              spdlog::debug("Parse-XML: Remote variable type set to 'int8_t' {0}.", gpRvarStruct->m_type);
            }
            else if ((attribute == "uint8") || (attribute == "uint8_t") || (attribute == "byte")) {
              gpRvarStruct->m_type = remote_variable_type_uint8_t;
              spdlog::debug("Parse-XML: Remote variable type set to 'uint8_t' {0}.", gpRvarStruct->m_type);
            }
            else if ((attribute == "int16") || (attribute == "int16_t") || (attribute == "short")) {
              gpRvarStruct->m_type = remote_variable_type_int16_t;
              spdlog::debug("Parse-XML: Remote variable type set to 'int16_t' {0}.", gpRvarStruct->m_type);
            }
            else if ((attribute == "uint16") || (attribute == "uint16_t")) {
              gpRvarStruct->m_type = remote_variable_type_uint16_t;
              spdlog::debug("Parse-XML: Remote variable type set to 'uint16_t' {0}.", gpRvarStruct->m_type);
            }
            else if ((attribute == "int32") || (attribute == "int32_t") || (attribute == "long")) {
              gpRvarStruct->m_type = remote_variable_type_int32_t;
              spdlog::debug("Parse-XML: Remote variable type set to 'int32_t' {0}.", gpRvarStruct->m_type);
            }
            else if ((attribute == "uint32") || (attribute == "uint32_t") || (attribute == "unsigned")) {
              gpRvarStruct->m_type = remote_variable_type_uint32_t;
              spdlog::debug("Parse-XML: Remote variable type set to 'uint32_t' {0}.", gpRvarStruct->m_type);
            }
            else if ((attribute == "int64") || (attribute == "int64_t") || (attribute == "longlong")) {
              gpRvarStruct->m_type = remote_variable_type_int64_t;
              spdlog::debug("Parse-XML: Remote variable type set to 'int64_t' {0}.", gpRvarStruct->m_type);
            }
            else if ((attribute == "uint64") || (attribute == "uint64_t")) {
              gpRvarStruct->m_type = remote_variable_type_uint64_t;
              spdlog::debug("Parse-XML: Remote variable type set to 'uint64_t' {0}.", gpRvarStruct->m_type);
            }
            else if (attribute == "float") {
              gpRvarStruct->m_type = remote_variable_type_float;
              spdlog::debug("Parse-XML: Remote variable type set to 'float' {0}.", gpRvarStruct->m_type);
            }
            else if (attribute == "double") {
              gpRvarStruct->m_type = remote_variable_type_double;
              spdlog::debug("Parse-XML: Remote variable type set to 'double' {0}.", gpRvarStruct->m_type);
            }
            else if (attribute == "date") {
              gpRvarStruct->m_type = remote_variable_type_date;
              spdlog::debug("Parse-XML: Remote variable type set to 'date' {0}.", gpRvarStruct->m_type);
            }
            else if (attribute == "time") {
              gpRvarStruct->m_type = remote_variable_type_time;
              spdlog::debug("Parse-XML: Remote variable type set to 'time' {0}.", gpRvarStruct->m_type);
            }
            else {
              gpRvarStruct->m_type = remote_variable_type_unknown;
            }
          }
          else if (0 == strcasecmp(attr[i], "page")) {
            // Register page
            spdlog::trace("Parse-XML: handleMDFParserData: Remote variable fgcolor: {0}", attribute);
            gpRvarStruct->m_page = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "offset")) {
            // Register offset
            spdlog::trace("Parse-XML: handleMDFParserData: Remote variable fgcolor: {0}", attribute);
            gpRvarStruct->m_offset = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "fgcolor")) {
            // Register foreground color
            spdlog::trace("Parse-XML: handleMDFParserData: Remote variable fgcolor: {0}", attribute);
            gpRvarStruct->m_fgcolor = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "bgcolor")) {
            // Register background color
            spdlog::trace("Parse-XML: handleMDFParserData: Remote variable bgcolor: {0}", attribute);
            gpRvarStruct->m_bgcolor = vscp_readStringValue(attribute);
          }
        }
      }
      /*
        <alarm>
          <bit>           <------
            .....
          </bit>
        </alarm>
      */
      else if ((currentToken == "bit") && 
          (gTokenList.at(1) == "alarm")) {

        spdlog::trace("Parse-XML: handleMDFParserData: Bit");

        if (!__getBitAttributes(&pmdf->m_list_alarm, attr)) {
          spdlog::error("Parse-XML: handleMDFParserData: Failed to allocate memory for bit structure");
          return;
        }

        // Set global pointer to added value so other info can be added
        gpBitStruct = pmdf->m_list_alarm.back();

      }
      // Old form for start and page data
      else if ((gTokenList.at(0) == "start") &&
               (gTokenList.at(1) == "dmatrix")) {

        // Get register attributes
        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "page")) {
            // dmatrix action code            
            spdlog::trace("Parse-XML: handleMDFParserData: dmatrix start page code: {0}", attribute);
            pmdf->getDM()->setStartPage(vscp_readStringValue(attribute));
          }
          else if (0 == strcasecmp(attr[i], "offset")) {
            // dmatrix action code            
            spdlog::trace("Parse-XML: handleMDFParserData: dmatrix start offset code: {0}", attribute);
            pmdf->getDM()->setStartOffset(vscp_readStringValue(attribute));
          }
        }         
        //spdlog::trace("Parse-XML: handleMDFParserData: dmatrix: start {0}", strContent);
        //pmdf->getDM()->setRowSize(vscp_readStringValue(strContent));
      }
      /*
        <dmatrix>
          <action>           <------
            .....
          </action>
        </dmatrix>
      */
      else if ((currentToken == "action") && 
          (gTokenList.at(1) == "dmatrix")) {

        gpActionStruct = new CMDF_Action;
        if (nullptr == gpActionStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for dmatrix action");
          return;
        }

        pmdf->getDM()->m_list_action.push_back(gpActionStruct);

        // Get register attributes
        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "code")) {
            // dmatrix action code            
            spdlog::trace("Parse-XML: handleMDFParserData: dmatrix action code: {0}", attribute);
            gpActionStruct->m_code = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "name")) {
            // dmatrix action code
            spdlog::trace("Parse-XML: handleMDFParserData: dmatrix action name: {0}", attribute);
            gpActionStruct->m_name = attribute;
          }
        }    
      }
      /*
        <events>
          <event>           <------
            .....
          </event>
        </events>
      */
      else if ((currentToken == "event") && 
          (gTokenList.at(1) == "events")) {

        gpEventStruct = new CMDF_Event;
        if (nullptr == gpEventStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for event");
          return;
        }

        pmdf->m_list_event.push_back(gpEventStruct);

        // Get register attributes
        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "name")) {           
            spdlog::trace("Parse-XML: handleMDFParserData: event name: {0}", attribute);
            gpEventStruct->m_name = attribute;
          }
          else if (0 == strcasecmp(attr[i], "class")) {           
            spdlog::trace("Parse-XML: handleMDFParserData: event class: {0}", attribute);
            if (attribute == "-") {
              gpEventStruct->m_class = -1;
            }
            else {
              gpEventStruct->m_class = vscp_readStringValue(attribute);
            }
          }
          else if (0 == strcasecmp(attr[i], "type")) {
            spdlog::trace("Parse-XML: handleMDFParserData: event type: {0}", attribute);
            if (attribute == "-") {
              gpEventStruct->m_type = -1;
            }
            else {
              gpEventStruct->m_type = vscp_readStringValue(attribute);
            }
          }
          else if (0 == strcasecmp(attr[i], "direction")) {
            spdlog::trace("Parse-XML: handleMDFParserData: event direction: {0}", attribute);
            gpEventStruct->m_direction = MDF_EVENT_DIR_OUT;
            vscp_trim(attribute);
            vscp_makeLower(attribute);
            if (attribute == "in") {
              gpEventStruct->m_direction = MDF_EVENT_DIR_IN;
            }
            else if (attribute == "out") {
              gpEventStruct->m_direction = MDF_EVENT_DIR_OUT;
            }
            else {
              gpEventStruct->m_direction = MDF_EVENT_DIR_OUT;
            }            
          }            
          else if (0 == strcasecmp(attr[i], "priority")) {
            spdlog::trace("Parse-XML: handleMDFParserData: event priority: {0}", attribute);
            gpEventStruct->m_priority = 3;
            vscp_trim(attribute);
            vscp_makeLower(attribute);
            if (attribute == "low") {
              gpEventStruct->m_priority = VSCP_PRIORITY_LOW >> 5;
              spdlog::debug("Parse-XML: Event priority set to low.");
            }
            else if (attribute == "normal") {
              gpEventStruct->m_priority = VSCP_PRIORITY_NORMAL >> 5;
              spdlog::debug("Parse-XML: Event priority set to medium.");
            }
            else if (attribute == "medium") {
              gpEventStruct->m_priority = VSCP_PRIORITY_MEDIUM >> 5;
              spdlog::debug("Parse-XML: Event priority set to medium.");
            }
            else if (attribute == "high") {
              gpEventStruct->m_priority = VSCP_PRIORITY_HIGH >> 5;
              spdlog::debug("Parse-XML: Event priority set to high.");
            }
            else {
              gpEventStruct->m_priority = vscp_readStringValue(attribute) & 7;
              spdlog::debug("Parse-XML: Event priority set to {0}.", gpEventStruct->m_priority);
            }
          }
        }     
      }
      break;


    case 4:

      /*
        <reg>
          <bit>           <------
            .....
          </bit>
        </reg>
      */
      if ((currentToken == "bit") && 
          (gTokenList.at(1) == "reg") &&
          (gpRegisterStruct != nullptr)) {

        spdlog::trace("Parse-XML: handleMDFParserData: Bit");

        if (!__getBitAttributes(&gpRegisterStruct->m_list_bit, attr)) {
          spdlog::error("Parse-XML: handleMDFParserData: Failed to allocate memory for bit structure");
          return;
        }

        // Set global pointer to added value so other info can be added
        gpBitStruct = gpRegisterStruct->m_list_bit.back();

      }
      /*
        <reg>
          <valuelist>           <------
            <item>....</item>
            <item>....</item>
          </valuelist>
        </reg>
      */
      else if ((currentToken == "valuelist") && 
               (gTokenList.at(1) == "reg") && 
               (gpRegisterStruct != nullptr)) {
        // Noting to do here          
        spdlog::trace("Parse-XML: handleMDFParserData: Valuelist start");
      }

      /*
        <rvar>
          <bit>           <------
            .....
          </bit>
        </rvar>
      */
      if ((currentToken == "bit") && 
          ((gTokenList.at(1) == "remotevar") ||(gTokenList.at(1) == "abstraction") ) &&
          (gpRvarStruct != nullptr)) {

        spdlog::trace("Parse-XML: handleMDFParserData: Bit");

        if (!__getBitAttributes(&gpRvarStruct->m_list_bit, attr)) {
          spdlog::error("Parse-XML: handleMDFParserData: Failed to allocate memory for bit structure");
          return;
        }

        // Set global pointer to added value so other info can be added
        gpBitStruct = gpRvarStruct->m_list_bit.back();

      }
      /*
        <dmatrix>
          <action>
            <param>           <------
              .....
            </param>  
          </action>
        </dmatrix>
      */
      else if ((currentToken == "param") &&
               (gTokenList.at(1) == "action") &&
               (gTokenList.at(2) == "dmatrix") &&
               (gpActionStruct != nullptr)) {

        gpActionParamStruct = new CMDF_ActionParameter;
        if (nullptr == gpActionParamStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for dmatrix action parameter");
          return;
        }

        gpActionStruct->m_list_ActionParameter.push_back(gpActionParamStruct);

        // Get register attributes
        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "offset")) {
            // dmatrix action parameter offset            
            spdlog::trace("Parse-XML: handleMDFParserData: dmatrix action parameter offset: {0}", attribute);
            gpActionParamStruct->m_offset = vscp_readStringValue(attribute);
          }
          if (0 == strcasecmp(attr[i], "min")) {
            // dmatrix action parameter min            
            spdlog::trace("Parse-XML: handleMDFParserData: dmatrix action parameter min: {0}", attribute);
            gpActionParamStruct->m_min = vscp_readStringValue(attribute);
          }
          if (0 == strcasecmp(attr[i], "max")) {
            // dmatrix action parameter max            
            spdlog::trace("Parse-XML: handleMDFParserData: dmatrix action parameter max: {0}", attribute);
            gpActionParamStruct->m_max = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "name")) {
            // dmatrix action parameter name
            spdlog::trace("Parse-XML: handleMDFParserData: dmatrix action paramerter name: {0}", attribute);
            gpActionParamStruct->m_name = attribute;
          }
        }
      }

      /*
        <events>
          <event>           
            <data>....</data>  <------
          </event>
        </events>
      */
      else if ((currentToken == "data") && 
               (gTokenList.at(1) == "event") && 
               (gTokenList.at(2) == "events")) {

        gpEventDataStruct = new CMDF_EventData;
        if (nullptr == gpEventDataStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for event");
          return;
        }

        gpEventStruct->m_list_eventdata.push_back(gpEventDataStruct);

        // Get register attributes
        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          vscp_makeLower(attribute);

          if (0 == strcasecmp(attr[i], "name")) {           
            spdlog::trace("Parse-XML: handleMDFParserData: event data name: {0}", attribute);
            gpEventDataStruct->m_name = attribute;
          }
          else if (0 == strcasecmp(attr[i], "offset")) {           
            spdlog::trace("Parse-XML: handleMDFParserData: event data offset: {0}", attribute);
            gpEventDataStruct->m_offset = vscp_readStringValue(attribute);
          }
        }
      }
      
      break;

    case 5:
      /*
        <reg>
          <valuelist>
            <item>....</item>   <------
            <item>....</item>
          </valuelist>
        </reg>
      */
      if ((currentToken == "item") && 
          (gTokenList.at(1) == "valuelist") && 
          (gTokenList.at(2) == "reg") &&
          (gpRegisterStruct != nullptr)) {

        spdlog::trace("Parse-XML: handleMDFParserData: register value");

        if (!__getValueAttributes(&gpRegisterStruct->m_list_value, attr)) {
          spdlog::error("Parse-XML: handleMDFParserData: Failed to parse register value values");
          return;
        }

        // Set global pointer to added value so other info can be added
        gpValueStruct = gpRegisterStruct->m_list_value.back();
      }
      else if ((currentToken == "valuelist") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "reg") &&
               (gpRegisterStruct != nullptr) &&
               (gpBitStruct != nullptr)) {
        // Noting to do here
        spdlog::trace("Parse-XML: handleMDFParserData: bit value list starts");
      }

      /*
        <remotevar>
          <valuelist>
            <item>....</item>   <------
            <item>....</item>
          </valuelist>
        </remotevar>
      */
      else if ((currentToken == "item") && 
          (gTokenList.at(1) == "valuelist") && 
          ((gTokenList.at(2) == "remotevar") || (gTokenList.at(2) == "abstraction")) &&
          (gpRvarStruct != nullptr)) {

        spdlog::trace("Parse-XML: handleMDFParserData: remote variable value");

        if (!__getValueAttributes(&gpRvarStruct->m_list_value, attr)) {
          spdlog::error("Parse-XML: handleMDFParserData: Failed to parse remote variable value values");
          return;
        }

        // Set global pointer to added value so other info can be added
        gpValueStruct = gpRvarStruct->m_list_value.back();
      }
      else if ((currentToken == "valuelist") && 
               (gTokenList.at(1) == "bit") && 
               ((gTokenList.at(2) == "remotevar") || (gTokenList.at(2) == "abstraction")) &&
               (gpRvarStruct != nullptr) &&
               (gpBitStruct != nullptr)) {
        // Noting to do here
        spdlog::trace("Parse-XML: handleMDFParserData: bit remote variable list starts");
      }

      /*
          <dmatrix>
            <action>
              <param>
                <bit></bit>  <------
              </param>
            </action>
          </dmatrix>
      */
      else if ((currentToken == "bit") && 
               (gTokenList.at(1) == "param") &&
               (gTokenList.at(2) == "action") &&
               (gTokenList.at(3) == "dmatrix") &&
               (gpActionStruct != nullptr) &&
               (gpActionParamStruct != nullptr)) {

        spdlog::trace("Parse-XML: handleMDFParserData: action param Bit");

        //std::cout << "Size: " << gpActionParamStruct->m_list_value.size() << std::endl;
        if (!__getBitAttributes(&gpActionParamStruct->m_list_bit, attr)) {
          spdlog::error("Parse-XML: handleMDFParserData: Failed to parse register bit values");
          return;
        }

        // Set global pointer to added value so other info can be added
        gpBitStruct = gpActionParamStruct->m_list_bit.back();    
      } 

      /*
          <events>
            <event>
              <data>
                <bit></bit>  <------
              </data>
            </event>
          </events>
      */
      else if ((currentToken == "bit") && 
               (gTokenList.at(1) == "data") &&
               (gTokenList.at(2) == "event") &&
               (gTokenList.at(3) == "events") &&
               (gpEventStruct != nullptr) &&
               (gpEventDataStruct != nullptr)) {

        spdlog::trace("Parse-XML: handleMDFParserData: event data Bit");

        //std::cout << "Size: " << gpActionParamStruct->m_list_value.size() << std::endl;
        if (!__getBitAttributes(&gpEventDataStruct->m_list_bit, attr)) {
          spdlog::error("Parse-XML: handleMDFParserData: Failed to parse event data bit");
          return;
        }

        // Set global pointer to added value so other info can be added
        gpBitStruct = gpEventDataStruct->m_list_bit.back();    
      }

      break;

    
    case 6:

      /*
          <reg>
            <bit>
              <valuelist>
                <item></item>
              </valuelist>
            </bit>
          </reg>
      */
      if ((currentToken == "item") && 
          (gTokenList.at(1) == "valuelist") &&
          (gTokenList.at(2) == "bit") &&
          (gTokenList.at(3) == "reg") &&
          (gpRegisterStruct != nullptr) &&
          (gpBitStruct != nullptr)) {

        spdlog::trace("Parse-XML: handleMDFParserData: Value");

        //std::cout << "Size: " << gpBitStruct->m_list_value.size() << std::endl;
        if (!__getValueAttributes(&gpBitStruct->m_list_value, attr)) {
          spdlog::error("Parse-XML: handleMDFParserData: Failed to parse register bit values");
          return;
        }

        // Set global pointer to added value so other info can be added
        gpValueStruct = gpBitStruct->m_list_value.back();
      }
      /*
            <rvar>
              <bit>
                <valuelist>
                  <item></item>
                </valuelist>
              </bit>
            </rvar>
      */
      else if ((currentToken == "item") && 
          (gTokenList.at(1) == "valuelist") &&
          (gTokenList.at(2) == "bit") &&
          ((gTokenList.at(3) == "remotevar") || (gTokenList.at(3) == "abstraction")) &&
          (gpRvarStruct != nullptr) &&
          (gpBitStruct != nullptr)) {

        spdlog::trace("Parse-XML: handleMDFParserData: Value");

        //std::cout << "Size: " << gpBitStruct->m_list_value.size() << std::endl;
        if (!__getValueAttributes(&gpBitStruct->m_list_value, attr)) {
          spdlog::error("Parse-XML: handleMDFParserData: Failed to parse register bit values");
          return;
        }

        // Set global pointer to added value so other info can be added
        gpValueStruct = gpBitStruct->m_list_value.back();
      }

      /*
            <action>
              <param>
                <valuelist>
                  <item>...</item>
                </valuelist>
              </param>
            </action>
      */
      else if ((currentToken == "item") && 
          (gTokenList.at(1) == "valuelist") &&
          (gTokenList.at(2) == "param") &&
          ((gTokenList.at(3) == "action")) &&
          (gpActionStruct != nullptr) &&
          (gpActionParamStruct != nullptr)) {

        spdlog::trace("Parse-XML: handleMDFParserData: Value");

        //std::cout << "Size: " << gpActionParamStruct->m_list_value.size() << std::endl;
        if (!__getValueAttributes(&gpActionParamStruct->m_list_value, attr)) {
          spdlog::error("Parse-XML: handleMDFParserData: Failed to parse register bit values");
          return;
        }

        // Set global pointer to added value so other info can be added
        gpValueStruct = gpActionParamStruct->m_list_value.back();
      }

      /*
            <action>
              <param>
                <bit>
                  <item>...</item>
                </bit>
              </param>
            </action>
      */
      else if ((currentToken == "item") && 
          (gTokenList.at(1) == "valuelist") &&
          (gTokenList.at(2) == "param") &&
          ((gTokenList.at(3) == "action")) &&
          (gpActionStruct != nullptr) &&
          (gpActionParamStruct != nullptr)) {

        spdlog::trace("Parse-XML: handleMDFParserData: Value");

        //std::cout << "Size: " << gpActionParamStruct->m_list_value.size() << std::endl;
        if (!__getValueAttributes(&gpActionParamStruct->m_list_value, attr)) {
          spdlog::error("Parse-XML: handleMDFParserData: Failed to parse register bit values");
          return;
        }

        // Set global pointer to added value so other info can be added
        gpValueStruct = gpActionParamStruct->m_list_value.back();
      }

      /*
          <events>  
            <event>
              <data>
                <valuelist>
                  <item>...</item>  <-----
                </valuelist>
              </data>
            </event>
          </events>  
      */
      else if ((currentToken == "item") && 
          (gTokenList.at(1) == "valuelist") &&
          (gTokenList.at(2) == "data") &&
          (gTokenList.at(3) == "event") &&
          ((gTokenList.at(4) == "events")) &&
          (gpEventStruct != nullptr) &&
          (gpEventDataStruct != nullptr)) {

        spdlog::trace("Parse-XML: handleMDFParserData: Value");

        //std::cout << "Size: " << gpActionParamStruct->m_list_value.size() << std::endl;
        if (!__getValueAttributes(&gpEventDataStruct->m_list_value, attr)) {
          spdlog::error("Parse-XML: handleMDFParserData: Failed to parse register bit values");
          return;
        }

        // Set global pointer to added value so other info can be added
        gpValueStruct = gpEventDataStruct->m_list_value.back();
      }

      break;

    case 7:
      /*
          <dmatrix>
            <action>
              <param>
                <bit>
                  <valuelist>
                    <item></item> <-----
                  </valuelist>
                </bit>
              <param>
            </action>
          </dmatrix>
      */
      if ((currentToken == "item") && 
          (gTokenList.at(1) == "valuelist") &&
          (gTokenList.at(2) == "bit") &&
          (gTokenList.at(3) == "param") &&
          (gTokenList.at(4) == "action") &&
          (gTokenList.at(5) == "dmatrix") &&
          (gpActionStruct != nullptr) &&
          (gpActionParamStruct != nullptr) &&
          (gpBitStruct != nullptr)) {

        spdlog::trace("Parse-XML: handleMDFParserData: Value");

        //std::cout << "Size: " << gpBitStruct->m_list_value.size() << std::endl;
        if (!__getValueAttributes(&gpBitStruct->m_list_value, attr)) {
          spdlog::error("Parse-XML: handleMDFParserData: Failed to parse register bit values");
          return;
        }

        // Set global pointer to added value so other info can be added
        gpValueStruct = gpBitStruct->m_list_value.back();
      }

      /*
          <events>
            <event>
              <data>
                <bit>
                  <valuelist>
                    <item></item> <-----
                  </valuelist>
                </bit>
              <action>
            </event>
          </events>
      */
      if ((currentToken == "item") && 
          (gTokenList.at(1) == "valuelist") &&
          (gTokenList.at(2) == "bit") &&
          (gTokenList.at(3) == "data") &&
          (gTokenList.at(4) == "event") &&
          (gTokenList.at(5) == "events") &&
          (gpEventStruct != nullptr) &&
          (gpEventDataStruct != nullptr) &&
          (gpBitStruct != nullptr)) {

        spdlog::trace("Parse-XML: handleMDFParserData: Value");

        //std::cout << "Size: " << gpBitStruct->m_list_value.size() << std::endl;
        if (!__getValueAttributes(&gpBitStruct->m_list_value, attr)) {
          spdlog::error("Parse-XML: handleMDFParserData: Failed to parse register bit values");
          return;
        }

        // Set global pointer to added value so other info can be added
        gpValueStruct = gpBitStruct->m_list_value.back();
      }      
      
      break;

  } // switch depth

  gdepth_xml_parser++;
}

// ----------------------------------------------------------------------------

void
__handleMDFParserData(void *data, const XML_Char *content, int length)
{
  // Get the pointer to the CMDF object
  CMDF *pmdf = (CMDF *) data;
  if (nullptr == pmdf) {
    spdlog::error("Parse-XML: ---> handleMDFParserData: Data object is invalid");
    return;
  }

  // Must be some content to work on
  if (!content) {
    spdlog::error("Parse-XML: ---> handleMDFParserData: No content");
    return;
  }

  // No use to work without the <vscp> tag
  if (!(gTokenList.back() == "vscp")) {
    spdlog::error("Parse-XML: ---> handleMDFParserData: No vscp tag");
    return;
  }

  std::string strContent = std::string(content, length);
  vscp_trim(strContent);
  if (strContent.empty()) {
    return;
  }

  spdlog::trace("Parse-XML: XML Data: {0}", strContent);

  switch (gdepth_xml_parser) {

    case 1: // On module level
      if (gTokenList.at(0) == "redirect") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module redirect: {0}", strContent);
        pmdf->m_redirectUrl = strContent;
      }
      else if (gTokenList.at(0) == "name") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module name: {0} language: {1}", strContent);
        vscp_trim(strContent);
        vscp_makeLower(strContent);
        pmdf->m_name = strContent;
      }
      break;

    case 2:

      break;

    case 3:

      //std::cout << "3 - " << gTokenList.at(2) << " " <<  gTokenList.at(1) << " " << gTokenList.at(0) << std::endl;

      if (gTokenList.at(0) == "name") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module name: {0}", strContent);
        vscp_trim(strContent);
        vscp_makeLower(strContent);
        pmdf->m_name = strContent;
      }
      else if (gTokenList.at(0) == "model") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module name: {0}", strContent);
        pmdf->m_strModule_Model = strContent;
      }
      else if (gTokenList.at(0) == "version") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module name: {0}", strContent);
        pmdf->m_strModule_Version = strContent;
      }
      else if (gTokenList.at(0) == "changed") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module Changedate: {0}", strContent);
        pmdf->m_strModule_changeDate = strContent;
      }
      else if (gTokenList.at(0) == "description") {
        if (gTokenList.at(1) == "module") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module Description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          pmdf->m_mapDescription[gLastLanguage] += strContent;
          spdlog::trace("Parse-XML: handleMDFParserData: Module Description size: {0}", pmdf->m_mapDescription.size());
        }
      }
      else if (gTokenList.at(0) == "infourl") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module infoUrl: {0} language: {1}", strContent, gLastLanguage);
        pmdf->m_mapInfoURL[gLastLanguage] += strContent;
      }
      else if (gTokenList.at(0) == "buffersize") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module buffer size: {0}", strContent);
        pmdf->m_module_bufferSize = vscp_readStringValue(strContent);
      }                  

      break;

    case 4: 
    
      //std::cout << "4 - " << gTokenList.at(3) << " " <<  gTokenList.at(2) << " " <<  gTokenList.at(1) << " " << gTokenList.at(0) << std::endl;

      // manufacturer, picture, files, manual, boot, registers abstractions/remotevar, alarm, dmatrix,
      // events
      if ((gTokenList.at(1) == "manufacturer") && (gTokenList.at(0) == "name")) {
        // Name of manufacturer
        spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer name: {0} language: {1}",
                      strContent,
                      gLastLanguage);
        pmdf->m_manufacturer.m_strName = strContent;
      }
      // Old form of picture name
      else if (gTokenList.at(1) == "picture" && (gTokenList.at(0) == "name")) {
        // Picture description
        if (gpPictureStruct != nullptr) {
          spdlog::trace("Parse-XML: handleMDFParserData: Picture name: {0}",
                        strContent );
          vscp_trim(strContent);
          vscp_makeLower(strContent);
          gpPictureStruct->m_strName = strContent;
        }
      }
      // Old form of picture description
      else if (gTokenList.at(1) == "picture" && (gTokenList.at(0) == "description")) {
        // Picture description
        if (gpPictureStruct != nullptr) {
          spdlog::trace("Parse-XML: handleMDFParserData: Picture Description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpPictureStruct->m_mapDescription[gLastLanguage] += strContent;
          spdlog::trace("Parse-XML: handleMDFParserData: Picture  Description size: {0}",
                        gpPictureStruct->m_mapDescription.size());
        }
      }
      // Old form of firmware name
      else if (gTokenList.at(1) == "firmware" && (gTokenList.at(0) == "name")) {
        // Firmware name
        if (gpFirmwareStruct != nullptr) {
          spdlog::trace("Parse-XML: handleMDFParserData: Firmware name: {0}",
                        strContent );
          vscp_trim(strContent);
          vscp_makeLower(strContent);
          gpFirmwareStruct->m_strName = strContent;
        }
      }
      // Old form of firmware description
      else if (gTokenList.at(1) == "firmware" && (gTokenList.at(0) == "description")) {
        // Firmware description
        if (gpFirmwareStruct != nullptr) {
          spdlog::trace("Parse-XML: handleMDFParserData: Firmware Description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpFirmwareStruct->m_mapDescription[gLastLanguage] += strContent;
          spdlog::trace("Parse-XML: handleMDFParserData: Firmware  Description size: {0}",
                        gpFirmwareStruct->m_mapDescription.size());
        }
      }
      // Old form of firmware name
      else if (gTokenList.at(1) == "manual" && (gTokenList.at(0) == "name")) {
        // Namual name
        if (gpManualStruct != nullptr) {
          spdlog::trace("Parse-XML: handleMDFParserData: Manual name: {0}",
                        strContent );
          vscp_trim(strContent);
          vscp_makeLower(strContent);
          gpManualStruct->m_strName = strContent;
        }
      }
      // Old form of manual description
      else if (gTokenList.at(1) == "manual" && (gTokenList.at(0) == "description")) {
        // Manual description
        if (gpManualStruct != nullptr) {
          spdlog::trace("Parse-XML: handleMDFParserData: Manual Description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpManualStruct->m_mapDescription[gLastLanguage] += strContent;
          spdlog::trace("Parse-XML: handleMDFParserData: Manual Description size: {0}",
                        gpManualStruct->m_mapDescription.size());
        }
      }      

      // Decision matrix

      else if ((gTokenList.at(0) == "level") &&
               (gTokenList.at(1) == "dmatrix")) {
        spdlog::trace("Parse-XML: handleMDFParserData: dmatrix: level {0}", strContent);
        pmdf->getDM()->setLevel(vscp_readStringValue(strContent));
      }
      else if ((gTokenList.at(0) == "rowcnt") &&
               (gTokenList.at(1) == "dmatrix")) {
        spdlog::trace("Parse-XML: handleMDFParserData: dmatrix: row count {0}", strContent);
        pmdf->getDM()->setRowCount(vscp_readStringValue(strContent));
      }
      else if ((gTokenList.at(0) == "rowsize") &&
               (gTokenList.at(1) == "dmatrix")) {
        spdlog::trace("Parse-XML: handleMDFParserData: dmatrix: row size {0}", strContent);
        pmdf->getDM()->setRowSize(vscp_readStringValue(strContent));
      }
      

      // Bootloader

      else if ((gTokenList.at(1) == "boot") && (gTokenList.at(0) == "algorithm")) {
        pmdf->m_bootInfo.m_nAlgorithm = vscp_readStringValue(strContent);
      }
      else if ((gTokenList.at(1) == "boot") && (gTokenList.at(0) == "blocksize")) {
        pmdf->m_bootInfo.m_nBlockSize = vscp_readStringValue(strContent);
      }
      else if ((gTokenList.at(1) == "boot") && (gTokenList.at(0) == "blockcount")) {
        pmdf->m_bootInfo.m_nBlockCount = vscp_readStringValue(strContent);
      }
      
      break;

    case 5:

      // std::cout << "5 - " << gTokenList.at(3) << " " <<  gTokenList.at(2) << " " <<  gTokenList.at(1) << " " << currentToken << std::endl;

      // manufacturer: address, telephone, fax, email, web

      // [5] manufacturer: address
      if (gTokenList.at(1) == "address") {
        if (gTokenList.at(0) == "street") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer address street: {0}", strContent);
          pmdf->m_manufacturer.m_address.m_strStreet = strContent;
        }
        else if (gTokenList.at(0) == "city") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer address city: {0}", strContent);
          pmdf->m_manufacturer.m_address.m_strCity = strContent;
        }
        else if (gTokenList.at(0) == "town") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer address town: {0}", strContent);
          pmdf->m_manufacturer.m_address.m_strTown = strContent;
        }
        else if (gTokenList.at(0) == "postcode") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer address postcode: {0}", strContent);
          pmdf->m_manufacturer.m_address.m_strPostCode = strContent;
        }
        else if (gTokenList.at(0) == "country") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer address country: {0}", strContent);
          pmdf->m_manufacturer.m_address.m_strCountry = strContent;
        }
        else if (gTokenList.at(0) == "region") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer address region: {0}", strContent);
          pmdf->m_manufacturer.m_address.m_strRegion = strContent;
        }
      }
      // [5] manufacturer/telephone
      else if (gTokenList.at(1) == "telephone") {
        if ((gTokenList.at(0) == "number") && (gpItemStruct != nullptr)) {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer address telephone number: {0}", strContent);
          vscp_trim(strContent);
          vscp_makeLower(strContent);
          gpItemStruct->m_name = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          spdlog::trace(
            "Parse-XML: handleMDFParserData: Module manufacturer address telephone description: {0} Language: {1}",
            strContent,
            gLastLanguage);
          gpItemStruct->m_mapDescription[gLastLanguage] += strContent;
        }
      }
      // [5] manufacturer/fax
      else if ((gTokenList.at(1) == "fax") && (gpItemStruct != nullptr)) {
        if (gTokenList.at(0) == "number") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer address fax number: {0}", strContent);
          vscp_trim(strContent);
          vscp_makeLower(strContent);
          gpItemStruct->m_name = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer address fax description: {0} Language: {1}",
                        strContent,
                        gLastLanguage);
          gpItemStruct->m_mapDescription[gLastLanguage] += strContent;
        }
      }
      // [5] manufacturer/email
      else if ((gTokenList.at(1) == "email") && (gpItemStruct != nullptr)) {
        if (gTokenList.at(0) == "address") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer email address: {0}", strContent);
          vscp_trim(strContent);
          vscp_makeLower(strContent);
          gpItemStruct->m_name = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          spdlog::trace(
            "Parse-XML: handleMDFParserData: Module manufacturer email address description: {0} Language: {1}",
            strContent,
            gLastLanguage);
          gpItemStruct->m_mapDescription[gLastLanguage] += strContent;
        }
      }
      // [5] manufacturer/web
      else if ((gTokenList.at(1) == "web") && (gpItemStruct != nullptr)) {
        if ((gTokenList.at(0) == "address") || (gTokenList.at(0) == "url")) {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer email address: {0}", strContent);
          vscp_trim(strContent);
          vscp_makeLower(strContent);
          gpItemStruct->m_name = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          spdlog::trace(
            "Parse-XML: handleMDFParserData: Module manufacturer email address description: {0} Language: {1}",
            strContent,
            gLastLanguage);
          gpItemStruct->m_mapDescription[gLastLanguage] += strContent;
        }
      }
      // [5] Picture standard format
      else if ((gTokenList.at(1) == "picture") && (gpPictureStruct != nullptr)) {
        if (gTokenList.at(0) == "name") {
          // Description for picture
          spdlog::trace("Parse-XML: handleMDFParserData: Module picture name: {0}",
                        strContent);
          vscp_trim(strContent);
          vscp_makeLower(strContent);              
          gpPictureStruct->m_strName = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          // Description for picture
          spdlog::trace("Parse-XML: handleMDFParserData: Module picture description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpPictureStruct->m_mapDescription[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "infourl") {
          // Info URL for picture
          spdlog::trace("Parse-XML: handleMDFParserData: Module picture infourl: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpPictureStruct->m_mapInfoURL[gLastLanguage] += strContent;
        }
      }
      // [5] Video standard format
      else if ((gTokenList.at(1) == "video") && (gpVideoStruct != nullptr)) {
        if (gTokenList.at(0) == "name") {
          // Description for picture
          spdlog::trace("Parse-XML: handleMDFParserData: Module video name: {0}",
                        strContent);
          vscp_trim(strContent);
          vscp_makeLower(strContent);              
          gpVideoStruct->m_strName = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          // Description for video
          spdlog::trace("Parse-XML: handleMDFParserData: Module video description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpVideoStruct->m_mapDescription[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "infourl") {
          // Info URL for video
          spdlog::trace("Parse-XML: handleMDFParserData: Module video infourl: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpVideoStruct->m_mapInfoURL[gLastLanguage] += strContent;
        }
      }
      // [5] Firmware standard format
      else if ((gTokenList.at(1) == "firmware") && (gpFirmwareStruct != nullptr)) {
        if (gTokenList.at(0) == "name") {
          // Description for picture
          spdlog::trace("Parse-XML: handleMDFParserData: Module picture description: {0}",
                        strContent);
          vscp_trim(strContent);
          vscp_makeLower(strContent);              
          gpFirmwareStruct->m_strName = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          // Description for firmware
          spdlog::trace("Parse-XML: handleMDFParserData: Module firmware description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpFirmwareStruct->m_mapDescription[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "infourl") {
          // Info URL for firmware
          spdlog::trace("Parse-XML: handleMDFParserData: Module firmware infourl: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpFirmwareStruct->m_mapInfoURL[gLastLanguage] += strContent;
        }
      }
      // [5] Driver standard format
      else if ((gTokenList.at(1) == "driver") && (gpDriverStruct != nullptr)) {
        if (gTokenList.at(0) == "name") {
          // Description for picture
          spdlog::trace("Parse-XML: handleMDFParserData: Module picture description: {0}",
                        strContent);
          vscp_trim(strContent);
          vscp_makeLower(strContent);              
          gpDriverStruct->m_strName = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          // Description for driver
          spdlog::trace("Parse-XML: handleMDFParserData: Module driver description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpDriverStruct->m_mapDescription[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "infourl") {
          // Info URL for driver
          spdlog::trace("Parse-XML: handleMDFParserData: Module driver infourl: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpDriverStruct->m_mapInfoURL[gLastLanguage] += strContent;
        }
      }
      // [5] Manual standard format
      
      else if ((gTokenList.at(1) == "manual") && (gpManualStruct != nullptr)) {
        if (gTokenList.at(0) == "name") {
          // Description for picture
          spdlog::trace("Parse-XML: handleMDFParserData: Module picture description: {0}",
                        strContent);
          vscp_trim(strContent);
          vscp_makeLower(strContent);              
          gpManualStruct->m_strName = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          // Description for manual
          spdlog::trace("Parse-XML: handleMDFParserData: Module manual description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpManualStruct->m_mapDescription[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "infourl") {
          // Info URL for manual
          spdlog::trace("Parse-XML: handleMDFParserData: Module manual infourl: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpManualStruct->m_mapInfoURL[gLastLanguage] += strContent;
        }
      }
      // [5] Setup standard format
      else if ((gTokenList.at(1) == "setup") && (gpSetupStruct != nullptr)) {
        if (gTokenList.at(0) == "name") {
          // Description for picture
          spdlog::trace("Parse-XML: handleMDFParserData: Module picture description: {0}", strContent);
          vscp_trim(strContent);
          vscp_makeLower(strContent);              
          gpSetupStruct->m_strName = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          // Description for setup
          spdlog::trace("Parse-XML: handleMDFParserData: Module setup description: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpSetupStruct->m_mapDescription[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "infourl") {
          // Info URL for setup
          spdlog::trace("Parse-XML: handleMDFParserData: Module picture infourl: {0} language: {1}",
                        strContent,
                        gLastLanguage);
          gpSetupStruct->m_mapInfoURL[gLastLanguage] += strContent;
        }
      }
      // [5] reg
      else if ((gTokenList.at(1) == "reg") && (gpRegisterStruct != nullptr)) {

        // Old form "name"
        if (gTokenList.at(0) == "name") {
          gpRegisterStruct->m_name = strContent;
          vscp_trim(gpRegisterStruct->m_name);
          vscp_makeLower(gpRegisterStruct->m_name);
        }
        else if (gTokenList.at(0) == "access") {
          // Register access
          std::string strAccess = strContent;
          vscp_trim(strAccess);
          vscp_makeLower(strAccess);
          gpRegisterStruct->m_access = MDF_REG_ACCESS_NONE;
          if (strAccess == "w") {
            gpRegisterStruct->m_access = MDF_REG_ACCESS_WRITE_ONLY;
            spdlog::debug("Parse-XML: Register access: Read Only");
          }
          else if (strAccess == "r") {
            gpRegisterStruct->m_access = MDF_REG_ACCESS_READ_ONLY;
            spdlog::debug("Parse-XML: Register access: Write Only");
          }
          else if (strAccess == "rw") {
            gpRegisterStruct->m_access = MDF_REG_ACCESS_READ_WRITE;
            spdlog::debug("Parse-XML: Register access: Read/Write");
          }
        }
        else if (gTokenList.at(0) == "description") {
          gpRegisterStruct->m_mapDescription[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "infourl") {
          gpRegisterStruct->m_mapInfoURL[gLastLanguage] += strContent;
        }
      }
      // [5] reg
      else if (((gTokenList.at(1) == "remotevar") || (gTokenList.at(1) == "abstraction")) && 
               (gpRvarStruct != nullptr)) {

        // Old form "name"
        if (gTokenList.at(0) == "name") {
          gpRvarStruct->m_name = strContent;
          vscp_trim(gpRvarStruct->m_name);
          vscp_makeLower(gpRvarStruct->m_name);
        }
        else if (gTokenList.at(0) == "access") {
          // Register access
          std::string strAccess = strContent;
          vscp_trim(strAccess);
          vscp_makeLower(strAccess);
          gpRvarStruct->m_access = MDF_REG_ACCESS_NONE;
          if (strAccess == "w") {
            gpRvarStruct->m_access = MDF_REG_ACCESS_WRITE_ONLY;
            spdlog::debug("Parse-XML: Register access: Read Only");
          }
          else if (strAccess == "r") {
            gpRvarStruct->m_access = MDF_REG_ACCESS_READ_ONLY;
            spdlog::debug("Parse-XML: Register access: Write Only");
          }
          else if (strAccess == "rw") {
            gpRvarStruct->m_access = MDF_REG_ACCESS_READ_WRITE;
            spdlog::debug("Parse-XML: Register access: Read/Write");
          }
        }
        else if (gTokenList.at(0) == "description") {
          gpRvarStruct->m_mapDescription[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "infourl") {
          gpRvarStruct->m_mapInfoURL[gLastLanguage] += strContent;
        }
      }
      // Event
      else if ((gTokenList.at(1) == "event") && 
               (gpEventStruct != nullptr)) {

        // Old form "name"
        if (gTokenList.at(0) == "name") {
          gpEventStruct->m_name = strContent;
          vscp_trim(gpEventStruct->m_name);
          vscp_makeLower(gpEventStruct->m_name);
        }
        else if (gTokenList.at(0) == "description") {
          gpEventStruct->m_mapDescription[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "infourl") {
          gpEventStruct->m_mapInfoURL[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "priority") {
          spdlog::trace("Parse-XML: handleMDFParserData: event priority: {0}", strContent);
          gpEventStruct->m_priority = 3;
          vscp_trim(strContent);
          vscp_makeLower(strContent);
          if (strContent == "low") {
            gpEventStruct->m_priority = VSCP_PRIORITY_LOW >> 5;
            spdlog::debug("Parse-XML: Event priority set to low.");
          }
          else if (strContent == "normal") {
            gpEventStruct->m_priority = VSCP_PRIORITY_NORMAL >> 5;
            spdlog::debug("Parse-XML: Event priority set to medium.");
          }
          else if (strContent == "medium") {
            gpEventStruct->m_priority = VSCP_PRIORITY_MEDIUM >> 5;
            spdlog::debug("Parse-XML: Event priority set to medium.");
          }
          else if (strContent == "high") {
            gpEventStruct->m_priority = VSCP_PRIORITY_HIGH >> 5;
            spdlog::debug("Parse-XML: Event priority set to high.");
          }
          else {
            gpEventStruct->m_priority = vscp_readStringValue(strContent) & 7;
            spdlog::debug("Parse-XML: Event priority set to {0}.", gpEventStruct->m_priority);
          }
        }
        else if (gTokenList.at(0) == "direction") {
          spdlog::trace("Parse-XML: handleMDFParserData: event direction: {0}", strContent);
          gpEventStruct->m_direction = MDF_EVENT_DIR_OUT;
          vscp_trim(strContent);
          vscp_makeLower(strContent);
          if (strContent == "in") {
            gpEventStruct->m_direction = MDF_EVENT_DIR_IN;
          }
          else if (strContent == "out") {
            gpEventStruct->m_direction = MDF_EVENT_DIR_OUT;
          }
          else {
            gpEventStruct->m_direction = MDF_EVENT_DIR_OUT;
          }    
        }
      }

      // * * * alarm * * *

      // alarm/bit/name  - Not preferred form (Better as attribut) 
      else if ((gTokenList.at(0) == "name") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "alarm") &&
               (gpBitStruct != nullptr)) {
        gpBitStruct->m_name = strContent;
        vscp_trim(gpBitStruct->m_name);
        vscp_makeLower(gpBitStruct->m_name);
      }
      // alarm/bit/description
      else if ((gTokenList.at(0) == "description") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "alarm") &&
               (gpBitStruct != nullptr)) {
        gpBitStruct->m_mapDescription[gLastLanguage] += strContent;
      }
      // alarm/bit/info url
      else if ((gTokenList.at(0) == "infourl") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "alarm") &&
               (gpBitStruct != nullptr)) {
        gpBitStruct->m_mapInfoURL[gLastLanguage] += strContent;
      }      

      // dmatrix

      // dmatrix/action/name
      else if ((gTokenList.at(0) == "name") && 
               (gTokenList.at(1) == "action") && 
               (gTokenList.at(2) == "dmatrix") &&
               (gpActionStruct != nullptr)) {
        vscp_trim(strContent);
        vscp_makeLower(strContent);         
        gpActionStruct->m_name  = strContent;
      }
      // dmatrix/action/description
      else if ((gTokenList.at(0) == "description") && 
               (gTokenList.at(1) == "action") && 
               (gTokenList.at(2) == "dmatrix") &&
               (gpActionStruct != nullptr)) {
        gpActionStruct->m_mapDescription[gLastLanguage] += strContent;
      }
      // dmatrix/action/infourl
      else if ((gTokenList.at(0) == "infourl") && 
               (gTokenList.at(1) == "action") && 
               (gTokenList.at(2) == "dmatrix") &&
               (gpActionStruct != nullptr)) {
        gpActionStruct->m_mapInfoURL[gLastLanguage] += strContent;
      }
      break;

    case 6: // name,description,item,valuelist

      // std::cout << "6 - " << gTokenList.at(3) << " " <<  gTokenList.at(2) << " " <<  gTokenList.at(1) << " " << currentToken << std::endl;

      // reg/bit/name  - Not preferred form (Better as attribut) 
      if ((gTokenList.at(0) == "name") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "reg") &&
               (gpBitStruct != nullptr)) {
        gpBitStruct->m_name = strContent;
        vscp_trim(gpBitStruct->m_name);
        vscp_makeLower(gpBitStruct->m_name);
      }
      // reg/bit/description
      else if ((gTokenList.at(0) == "description") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "reg") &&
               (gpBitStruct != nullptr)) {
        gpBitStruct->m_mapDescription[gLastLanguage] += strContent;
      }
      // reg/bit/info url
      else if ((gTokenList.at(0) == "infourl") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "reg") &&
               (gpBitStruct != nullptr)) {
        gpBitStruct->m_mapInfoURL[gLastLanguage] += strContent;
      }


      // * * * Abstraction * * *


      // rvar/bit/name  - Not preferred form (Better as attribut) 
      else if ((gTokenList.at(0) == "name") && 
               (gTokenList.at(1) == "bit") && 
               ((gTokenList.at(2) == "remotevar") || (gTokenList.at(2) == "abstraction")) &&
               (gpBitStruct != nullptr)) {
        gpBitStruct->m_name = strContent;
        vscp_trim(gpBitStruct->m_name);
        vscp_makeLower(gpBitStruct->m_name);
      }
      // rvar/bit/description
      else if ((gTokenList.at(0) == "description") && 
               (gTokenList.at(1) == "bit") && 
               ((gTokenList.at(2) == "remotevar") || (gTokenList.at(2) == "abstraction")) &&
               (gpBitStruct != nullptr)) {
        gpBitStruct->m_mapDescription[gLastLanguage] += strContent;
      }
      // rvar/bit/info url
      else if ((gTokenList.at(0) == "infourl") && 
               (gTokenList.at(1) == "bit") && 
               ((gTokenList.at(2) == "remotevar") || (gTokenList.at(2) == "abstraction")) &&
               (gpBitStruct != nullptr)) {
        gpBitStruct->m_mapInfoURL[gLastLanguage] += strContent;
      }

      
      // * * * dmatrix -- action param * * *


      else if ((gTokenList.at(0) == "name") && 
               (gTokenList.at(1) == "param") && 
               (gTokenList.at(2) == "action") &&
               (gTokenList.at(3) == "dmatrix") &&
               (gpActionParamStruct != nullptr)) {
        gpActionParamStruct->m_name = strContent;
        vscp_trim(gpActionParamStruct->m_name);
        vscp_makeLower(gpActionParamStruct->m_name);
      }
      else if ((gTokenList.at(0) == "description") && 
               (gTokenList.at(1) == "param") && 
               (gTokenList.at(2) == "action") &&
               (gTokenList.at(3) == "dmatrix") &&
               (gpActionParamStruct != nullptr)) {
        gpActionParamStruct->m_mapDescription[gLastLanguage] += strContent;
      }
      else if ((gTokenList.at(0) == "infourl") &&
               (gTokenList.at(1) == "param") &&
               (gTokenList.at(2) == "action") &&
               (gTokenList.at(3) == "dmatrix") &&
               (gpActionParamStruct != nullptr)) {
        gpActionParamStruct->m_mapInfoURL[gLastLanguage] += strContent;
      }

      // * * * event data * * *

      // event data
      else if ((gTokenList.at(0) == "name") && 
               (gTokenList.at(1) == "data") && 
               (gTokenList.at(2) == "event") &&
               (gpEventDataStruct != nullptr)) {
        gpEventDataStruct->m_name = strContent;
        vscp_trim(gpEventDataStruct->m_name);
        vscp_makeLower(gpEventDataStruct->m_name);
      }
      else if ((gTokenList.at(0) == "description") && 
               (gTokenList.at(1) == "data") && 
               (gTokenList.at(2) == "event") &&
               (gpEventDataStruct != nullptr)) {
        gpEventDataStruct->m_mapDescription[gLastLanguage] += strContent;
      }
      else if ((gTokenList.at(0) == "infourl") && 
               (gTokenList.at(1) == "data") && 
               (gTokenList.at(2) == "event") &&
               (gpEventDataStruct != nullptr)) {
        gpEventDataStruct->m_mapInfoURL[gLastLanguage] += strContent;
      }
      else if ((gTokenList.at(0) == "valuelist") && 
               (gTokenList.at(1) == "data") && 
               (gTokenList.at(2) == "event") &&
               (gpEventDataStruct != nullptr)) {
        int i = 9;
      }

      

      break;

    case 7:

      //std::cout << "7 - " <<  gTokenList.at(3) << " " <<  gTokenList.at(2) << " " <<  gTokenList.at(1) << " " << gTokenList.at(0) << std::endl;

      // * * * registers * * *

      // reg/valuelist/item/name *
      if ((gTokenList.at(0) == "name") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "reg") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_name = strContent;
        vscp_trim(gpValueStruct->m_name);
        vscp_makeLower(gpValueStruct->m_name);
      }
      // reg/valuelist/item/description *
      else if ((gTokenList.at(0) == "description") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "reg") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_mapDescription[gLastLanguage] += strContent;
      }
      // reg/valuelist/item/infourl *
      else if ((gTokenList.at(0) == "infourl") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "reg") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_mapInfoURL[gLastLanguage] += strContent;
      }

      // * * * Remote variables   * * *
      
      // rvar/valuelist/item/name *
      else if ((gTokenList.at(0) == "name") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               ((gTokenList.at(3) == "remotevar") || (gTokenList.at(3) == "abstraction")) && 
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_name = strContent;
        vscp_trim(gpValueStruct->m_name);
        vscp_makeLower(gpValueStruct->m_name);
      }
      // rvar/valuelist/item/description *
      else if ((gTokenList.at(0) == "description") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               ((gTokenList.at(3) == "remotevar") || (gTokenList.at(3) == "abstraction")) &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_mapDescription[gLastLanguage] += strContent;
      }
      // rvar/valuelist/item/infourl *
      else if ((gTokenList.at(0) == "infourl") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               ((gTokenList.at(3) == "remotevar") || (gTokenList.at(3) == "abstraction")) &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_mapInfoURL[gLastLanguage] += strContent;
      } 

      // dmatrix bits

      else if ((gTokenList.at(0) == "name") &&
               (gTokenList.at(1) == "bit") &&
               (gTokenList.at(2) == "param") &&
               (gTokenList.at(3) == "action") &&
               (gTokenList.at(4) == "dmatrix") &&
               (gpActionParamStruct != nullptr) &&
               (gpBitStruct != nullptr)) {
        gpBitStruct->m_name = strContent;
        vscp_trim(gpBitStruct->m_name);
        vscp_makeLower(gpBitStruct->m_name);
      }
      else if ((gTokenList.at(0) == "description") &&
               (gTokenList.at(1) == "bit") &&
               (gTokenList.at(2) == "param") &&
               (gTokenList.at(3) == "action") &&
               (gTokenList.at(4) == "dmatrix") &&
               (gpActionParamStruct != nullptr) &&
               (gpBitStruct != nullptr) ) {
        gpBitStruct->m_mapDescription[gLastLanguage] += strContent;
      }
      else if ((gTokenList.at(0) == "infourl") &&
               (gTokenList.at(1) == "bit") &&
               (gTokenList.at(2) == "param") &&
               (gTokenList.at(3) == "action") &&
               (gTokenList.at(4) == "dmatrix") &&
               (gpActionParamStruct != nullptr) &&
               (gpBitStruct != nullptr)) {
        gpBitStruct->m_mapInfoURL[gLastLanguage] += strContent;
      }

      // event data

      else if ((gTokenList.at(0) == "name") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "data") &&
               (gTokenList.at(3) == "event") &&
               (gTokenList.at(4) == "events") &&
               (gpBitStruct != nullptr)) {
        gpBitStruct->m_name = strContent;
        vscp_trim(gpValueStruct->m_name);
        vscp_makeLower(gpValueStruct->m_name);
      }

      else if ((gTokenList.at(0) == "description") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "data") &&
               (gTokenList.at(3) == "event") &&
               (gTokenList.at(4) == "events") &&
               (gpBitStruct != nullptr)) {
        gpBitStruct->m_mapDescription[gLastLanguage] += strContent;
      }

      else if ((gTokenList.at(0) == "infourl") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "data") &&
               (gTokenList.at(3) == "event") &&
               (gTokenList.at(4) == "events") &&
               (gpBitStruct != nullptr)) {
        gpBitStruct->m_mapInfoURL[gLastLanguage] += strContent;
      }

      break;

    case 8:

      //std::cout << "8 - " <<  gTokenList.at(4) << " " <<  gTokenList.at(3) << " " <<  gTokenList.at(2) << " " <<  gTokenList.at(1) << " " << gTokenList.at(0) << std::endl; 

      // reg/cmatrix action
      //     bit/valuelist/item/value *
      if ((gTokenList.at(0) == "value") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "bit") &&
               (gpValueStruct != nullptr)) { 
        gpValueStruct->m_strValue = strContent;
      }
      // reg/bit/valuelist/item/name *
      else if ((gTokenList.at(0) == "name") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "bit") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_name = strContent;
        vscp_trim(gpValueStruct->m_name);
        vscp_makeLower(gpValueStruct->m_name);
      }
      // bit/valuelist/item/description *
      else if ((gTokenList.at(0) == "description") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "bit") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_mapDescription[gLastLanguage] += strContent;
      }
      // bit/valuelist/item/infourl *
      else if ((gTokenList.at(0) == "infourl") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") && 
               (gTokenList.at(3) == "bit") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_mapInfoURL[gLastLanguage] += strContent;
      }   

      // dmatrix value

      else if ((gTokenList.at(0) == "value") &&
               (gTokenList.at(1) == "item") &&
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "param") &&
               (gTokenList.at(4) == "action") &&
               (gTokenList.at(5) == "dmatrix") &&
               (gpActionParamStruct != nullptr) &&
               (gpValueStruct != nullptr) ) {
        gpValueStruct->m_strValue = strContent;
      }

      else if ((gTokenList.at(0) == "name") &&
               (gTokenList.at(1) == "item") &&
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "param") &&
               (gTokenList.at(4) == "action") &&
               (gTokenList.at(5) == "dmatrix") &&
               (gpActionParamStruct != nullptr) &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_name = strContent;
        vscp_trim(gpValueStruct->m_name);
        vscp_makeLower(gpValueStruct->m_name);
      }
      else if ((gTokenList.at(0) == "description") &&
               (gTokenList.at(1) == "item") &&
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "param") &&
               (gTokenList.at(4) == "action") &&
               (gTokenList.at(5) == "dmatrix") &&
               (gpActionParamStruct != nullptr) &&
               (gpValueStruct != nullptr) ) {
        gpValueStruct->m_mapDescription[gLastLanguage] += strContent;
      }
      else if ((gTokenList.at(0) == "infourl") &&
               (gTokenList.at(1) == "item") &&
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "param") &&
               (gTokenList.at(4) == "action") &&
               (gTokenList.at(5) == "dmatrix") &&
               (gpActionParamStruct != nullptr) &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_mapInfoURL[gLastLanguage] += strContent;
      }

      // event data

      // event/valuelist/item/value *
      else if ((gTokenList.at(0) == "value") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "data") &&
               (gTokenList.at(4) == "event") &&
               (gTokenList.at(5) == "events") &&
               (gpValueStruct != nullptr)) { 
        gpValueStruct->m_strValue = strContent;
      }

      // event/valuelist/item/name *
      else if ((gTokenList.at(0) == "name") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "data") &&
               (gTokenList.at(4) == "event") &&
               (gTokenList.at(5) == "events") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_name = strContent;
        vscp_trim(gpValueStruct->m_name);
        vscp_makeLower(gpValueStruct->m_name);
      }

      // event/valuelist/item/description *
      else if ((gTokenList.at(0) == "description") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "data") &&
               (gTokenList.at(4) == "event") &&
               (gTokenList.at(5) == "events") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_mapDescription[gLastLanguage] += strContent;
      }

      // event/valuelist/item/infourl *
      else if ((gTokenList.at(0) == "infourl") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") && 
               (gTokenList.at(3) == "data") &&
               (gTokenList.at(4) == "event") &&
               (gTokenList.at(5) == "events") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_mapInfoURL[gLastLanguage] += strContent;
      }
      
      break;

    case 9:

      // dmatrix action

      if ((gTokenList.at(0) == "value") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "bit") &&
               (gTokenList.at(4) == "param") &&
               (gTokenList.at(5) == "action") &&
               (gpValueStruct != nullptr)) { 
        gpValueStruct->m_strValue = strContent;
      }
      else if ((gTokenList.at(0) == "name") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "bit") &&
               (gTokenList.at(4) == "param") &&
               (gTokenList.at(5) == "action") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_name = strContent;
      }
      else if ((gTokenList.at(0) == "description") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "bit") &&
               (gTokenList.at(4) == "param") &&
               (gTokenList.at(5) == "action") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_mapDescription[gLastLanguage] += strContent;
      }
      else if ((gTokenList.at(0) == "infourl") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gTokenList.at(3) == "bit") &&
               (gTokenList.at(4) == "param") &&
               (gTokenList.at(5) == "action") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_mapInfoURL[gLastLanguage] += strContent;
      }

      // event data

      else if ((gTokenList.at(0) == "value") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") && 
               (gTokenList.at(3) == "bit") &&
               (gTokenList.at(4) == "data") &&
               (gTokenList.at(5) == "event") &&
               (gTokenList.at(6) == "events") &&
               (gpValueStruct != nullptr)) { 
        gpValueStruct->m_strValue = strContent;
      }

      else if ((gTokenList.at(0) == "name") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") && 
               (gTokenList.at(3) == "bit") &&
               (gTokenList.at(4) == "data") &&
               (gTokenList.at(5) == "event") &&
               (gTokenList.at(6) == "events") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_name = strContent;
        vscp_trim(gpValueStruct->m_name);
        vscp_makeLower(gpValueStruct->m_name);
      }

      else if ((gTokenList.at(0) == "description") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") && 
               (gTokenList.at(3) == "bit") &&
               (gTokenList.at(4) == "data") &&
               (gTokenList.at(5) == "event") &&
               (gTokenList.at(6) == "events") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_mapDescription[gLastLanguage] += strContent;
      }

      else if ((gTokenList.at(0) == "infourl") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") && 
               (gTokenList.at(3) == "bit") &&
               (gTokenList.at(4) == "data") &&
               (gTokenList.at(5) == "event") &&
               (gTokenList.at(6) == "events") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct->m_mapInfoURL[gLastLanguage] += strContent;
      }

      break;  
  }
}

// ----------------------------------------------------------------------------

void
__endSetupMDFParser(void *data, const char *name)
{
  spdlog::trace("Parse-XML: ---> End: Tag: {0} Depth: {1}", name, gdepth_xml_parser);

  // Get the pointer to the CMDF object
  CMDF *pmdf = (CMDF *) data;
  if (nullptr == pmdf) {
    spdlog::trace("Parse-XML: ---> endSetupMDFParser: Data object is invalid");
    return;
  }

  std::string currentToken = name;
  vscp_trim(currentToken);
  vscp_makeLower(currentToken);

  switch (gdepth_xml_parser) {

    case 1:
      if (currentToken == "vscp") {
        ;
      }
      break;

    case 2:
      if (currentToken == "module") {
        ;
      }
      else if (currentToken == "redirect") {
        ;
      }
      break;

    case 3:
      if (currentToken == "manufacturer") {
        ; // Nothing to do
      }
      else if (currentToken == "firmware") {
        gpFirmwareStruct = nullptr;
      }
      else if (currentToken == "driver") {
        gpDriverStruct = nullptr;
      }
      // This is the deprecated form
      else if (currentToken == "pictures") {
        gpPictureStruct = nullptr;
      }
      else if (currentToken == "picture") {
        gpPictureStruct = nullptr;
      }
      else if (currentToken == "video") {
        gpVideoStruct = nullptr;
      }
      else if (currentToken == "driver") {
        gpDriverStruct = nullptr;
      }
      else if (currentToken == "setup") {
        gpSetupStruct = nullptr;
      }
      else if (currentToken == "manual") {
        gpManualStruct = nullptr;
      }
      else if (currentToken == "registers") {
        gpRegisterStruct = nullptr;
      }
      else if (currentToken == "register") {
        gpRegisterStruct = nullptr;
      }      
      else if (currentToken == "alarm") {        
        // Noting to do here
      }   
      else if (currentToken == "dmatrix") {        
        // Noting to do here
      }    
      break;

    case 4:
      if (currentToken == "address") {
        ; // Nothing to do
      }
      else if ((currentToken == "bit") && 
               (gTokenList.at(1) == "alarm") &&
               (gpBitStruct != nullptr)) {
        gpBitStruct = nullptr;
      } 
      else if ((currentToken == "action") && 
               (gTokenList.at(1) == "dmatrix") &&
               (gpActionStruct != nullptr)) {
        gpActionStruct = nullptr;
      }
      else if ((currentToken == "telephone") && 
               (gpItemStruct != nullptr)) {
        // Save address data in list
        pmdf->m_manufacturer.m_list_Phone.push_back(gpItemStruct);
        gpItemStruct = nullptr;
      }
      else if ((currentToken == "fax") && 
               (gpItemStruct != nullptr)) {
        // Save address data in list
        pmdf->m_manufacturer.m_list_Fax.push_back(gpItemStruct);
        gpItemStruct = nullptr;
      }
      else if ((currentToken == "email") && 
               (gpItemStruct != nullptr)) {
        // Save address data in list
        pmdf->m_manufacturer.m_list_Email.push_back(gpItemStruct);
        gpItemStruct = nullptr;
      }
      else if ((currentToken == "web") && 
               (gpItemStruct != nullptr)) {
        // Save address data in list
        pmdf->m_manufacturer.m_list_Web.push_back(gpItemStruct);
        gpItemStruct = nullptr;
      }
      else if ((currentToken == "reg") && 
               (gpRegisterStruct != nullptr)) {
        // Nothing to do
        gpRegisterStruct = nullptr;
      }
      else if ((currentToken == "abstraction") && 
               (gpRvarStruct != nullptr)) {
        gpRvarStruct = nullptr;
      }
      else if ((currentToken == "remotevar") && 
               (gpRvarStruct != nullptr)) {
        gpRvarStruct = nullptr;
      }      
      break;

    case 5: // name,description,access

      //std::cout << "5 - " << gTokenList.at(2) << " " << gTokenList.at(1) << " " << currentToken << std::endl;

      // Manufacturer
      if ((currentToken == "street") && 
               (gTokenList.at(1) == "address") &&
               (gTokenList.at(2) == "manufacturer") ) {
        ;
      }
      else if ((currentToken == "city") && 
               (gTokenList.at(1) == "address") &&
               (gTokenList.at(2) == "manufacturer") ) {
        ;
      }
      else if ((currentToken == "town") && 
               (gTokenList.at(1) == "address") &&
               (gTokenList.at(2) == "manufacturer") ) {
        ;
      }
      else if ((currentToken == "postcode") && 
               (gTokenList.at(1) == "address") &&
               (gTokenList.at(2) == "manufacturer") ) {
        ;
      }
      else if ((currentToken == "country") && 
               (gTokenList.at(1) == "address") &&
               (gTokenList.at(2) == "manufacturer") ) {
        ;
      }
      else if ((currentToken == "number") && 
               (gTokenList.at(1) == "telephone") &&
               (gTokenList.at(2) == "manufacturer") ) {
        ;
      }
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "telephone") &&
               (gTokenList.at(2) == "manufacturer") ) {
        ;
      }
      else if ((currentToken == "number") && 
               (gTokenList.at(1) == "fax") &&
               (gTokenList.at(2) == "manufacturer") ) {
        ;
      }
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "fax") &&
               (gTokenList.at(2) == "manufacturer") ) {
        ;
      }
      else if ((currentToken == "number") && 
               (gTokenList.at(1) == "email") &&
               (gTokenList.at(2) == "manufacturer") ) {
        ;
      }
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "email") &&
               (gTokenList.at(2) == "manufacturer") ) {
        ;
      }
      else if ((currentToken == "number") && 
               (gTokenList.at(1) == "web") &&
               (gTokenList.at(2) == "manufacturer") ) {
        ;
      }
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "web") &&
               (gTokenList.at(2) == "manufacturer") ) {
        ;
      }
      // files
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "picture") &&
               (gTokenList.at(2) == "files") ) {
        ;
      }
      else if ((currentToken == "infourl") && 
               (gTokenList.at(1) == "picture") &&
               (gTokenList.at(2) == "files") ) {
        ;
      }
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "firmware") &&
               (gTokenList.at(2) == "files") ) {
        ;
      }
      else if ((currentToken == "infourl") && 
               (gTokenList.at(1) == "firmware") &&
               (gTokenList.at(2) == "files") ) {
        ;
      }
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "manual") &&
               (gTokenList.at(2) == "files") ) {
        ;
      }
      else if ((currentToken == "infourl") && 
               (gTokenList.at(1) == "manual") &&
               (gTokenList.at(2) == "files") ) {
        ;
      }
      // registers
      else if ((currentToken == "name") && 
               (gTokenList.at(1) == "reg") &&
               (gTokenList.at(2) == "registers") ) {
        // No need to do anything here
      }
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "reg") &&
               (gTokenList.at(2) == "registers") ) {
        // No need to do anything here
      }
      else if ((currentToken == "infourl") && 
               (gTokenList.at(1) == "reg") &&
               (gTokenList.at(2) == "registers") ) {
        // No need to do anything here
      }
      else if ((currentToken == "access") && 
               (gTokenList.at(1) == "reg") &&
               (gTokenList.at(2) == "registers") ) {
        // No need to do anything here
      }
      else if ((currentToken == "valuelist") && 
               (gTokenList.at(1) == "reg") &&
               (gTokenList.at(2) == "registers") ) {
        // No need to do anything here
      }
      else if ((currentToken == "bit") && 
               (gTokenList.at(1) == "reg") &&
               (gTokenList.at(2) == "registers") &&
               (gpBitStruct != nullptr) ) {
        gpBitStruct = nullptr;
      }

      // remotevar
      else if ((currentToken == "name") && 
               (gTokenList.at(1) == "remotevar") &&
               (gTokenList.at(2) == "remotevars") ) {
        // No need to do anything here
      }
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "remotevar") &&
               (gTokenList.at(2) == "remotevars") ) {
        // No need to do anything here
      }
      else if ((currentToken == "infourl") && 
               (gTokenList.at(1) == "remotevar") &&
               (gTokenList.at(2) == "remotevars") ) {
        // No need to do anything here
      }
      else if ((currentToken == "access") && 
               (gTokenList.at(1) == "remotevar") &&
               (gTokenList.at(2) == "remotevars") ) {
        // No need to do anything here
      }
      else if ((currentToken == "valuelist") && 
               (gTokenList.at(1) == "remotevar") &&
               (gTokenList.at(2) == "remotevars") ) {
        // No need to do anything here
      }
      else if ((currentToken == "bit") && 
               (gTokenList.at(1) == "remotevar") &&
               (gTokenList.at(2) == "remotevars") &&
               (gpBitStruct != nullptr) ) {
        gpBitStruct = nullptr;
      }

      // Abstractions
      else if ((currentToken == "name") && 
               (gTokenList.at(1) == "abstraction") &&
               (gTokenList.at(2) == "abstractions") && 
               (gpRvarStruct != nullptr)) {
        ;
      }
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "abstraction") &&
               (gTokenList.at(2) == "abstractions") && 
               (gpRvarStruct != nullptr)) {
        ;
      }
      else if ((currentToken == "infourl") && 
               (gTokenList.at(1) == "abstraction") &&
               (gTokenList.at(2) == "abstractions") && 
               (gpRvarStruct != nullptr)) {
        ;
      }
      else if ((currentToken == "access") && 
               (gTokenList.at(1) == "abstraction") &&
               (gTokenList.at(2) == "abstractions") && 
               (gpRvarStruct != nullptr)) {
        ;
      }
      else if ((currentToken == "valuelist") && 
               (gTokenList.at(1) == "abstraction") &&
               (gTokenList.at(2) == "abstractions") ) {
        // No need to do anything here
      }
      else if ((currentToken == "bit") && 
               (gTokenList.at(1) == "abstraction") &&
               (gTokenList.at(2) == "abstractions") &&
               (gpBitStruct != nullptr) ) {
        gpBitStruct = nullptr;
      }      
      break;

    case 6: // name,description,item,valuelist

      // std::cout << "6 - " gTokenList.at(3) << " " <<  gTokenList.at(2) << " " <<  gTokenList.at(1) << " " << currentToken << std::endl;

      // registers

      if ((currentToken == "item") && 
          (gTokenList.at(1) == "valuelist") && 
          (gTokenList.at(2) == "reg") &&
          (gpValueStruct != nullptr)) {
        gpValueStruct = nullptr;
      }
      // reg bit
      else if ((currentToken == "name") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "reg") &&
               (gpBitStruct != nullptr)) {
        // No need to do anything here
      }
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "reg") &&
               (gpBitStruct != nullptr)) {
        // No need to do anything here
      }
      else if ((currentToken == "infourl") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "reg") &&
               (gpBitStruct != nullptr)) {
        // No need to do anything here
      }
      else if ((currentToken == "valuelist") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "reg") &&
               (gpBitStruct != nullptr)) {
        // No need to do anything here
      }

      // remotevar

      if ((currentToken == "item") && 
          (gTokenList.at(1) == "valuelist") && 
          (gTokenList.at(2) == "remotevar") &&
          (gpValueStruct != nullptr)) {
        gpValueStruct = nullptr;
      }
      // reg bit
      else if ((currentToken == "name") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "remotevar") &&
               (gpBitStruct != nullptr)) {
        // No need to do anything here
      }
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "remotevar") &&
               (gpBitStruct != nullptr)) {
        // No need to do anything here
      }
      else if ((currentToken == "infourl") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "remotevar") &&
               (gpBitStruct != nullptr)) {
        // No need to do anything here
      }
      else if ((currentToken == "valuelist") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "remotevar") &&
               (gpBitStruct != nullptr)) {
        // No need to do anything here
      }

      // abstraction

      if ((currentToken == "item") && 
          (gTokenList.at(1) == "valuelist") && 
          (gTokenList.at(2) == "abstraction") &&
          (gpValueStruct != nullptr)) {
        gpValueStruct = nullptr;
      }
      // reg bit
      else if ((currentToken == "name") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "abstraction") &&
               (gpBitStruct != nullptr)) {
        // No need to do anything here
      }
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "abstraction") &&
               (gpBitStruct != nullptr)) {
        // No need to do anything here
      }
      else if ((currentToken == "infourl") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "abstraction") &&
               (gpBitStruct != nullptr)) {
        // No need to do anything here
      }
      else if ((currentToken == "valuelist") && 
               (gTokenList.at(1) == "bit") && 
               (gTokenList.at(2) == "abstraction") &&
               (gpBitStruct != nullptr)) {
        // No need to do anything here
      }

      // action param

      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "param") && 
               (gTokenList.at(2) == "action") &&
               (gpBitStruct != nullptr)) {
        ;
      }
      else if ((currentToken == "infourl") && 
               (gTokenList.at(1) == "param") && 
               (gTokenList.at(2) == "action") &&
               (gpBitStruct != nullptr)) {
        ;
      }
      else if ((currentToken == "valuelist") && 
               (gTokenList.at(1) == "parm") && 
               (gTokenList.at(2) == "action") &&
               (gpActionStruct != nullptr)) {
        int i = 0;
      }
      // event data
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "data") && 
               (gTokenList.at(2) == "event") &&
               (gpBitStruct != nullptr)) {
        ;
      }
      else if ((currentToken == "infourl") && 
               (gTokenList.at(1) == "data") && 
               (gTokenList.at(2) == "event") &&
               (gpBitStruct != nullptr)) {
        ;
      }
      else if ((currentToken == "valuelist") && 
               (gTokenList.at(1) == "data") && 
               (gTokenList.at(2) == "event") &&
               (gpBitStruct != nullptr)) {
        ;
      }
      break;

    case 7:

      // std::cout << "7 - " gTokenList.at(3) << " " <<  gTokenList.at(2) << " " <<  gTokenList.at(1) << " " << currentToken << std::endl;

      // param/valuelist/item
      if ((currentToken == "item") && 
          (gTokenList.at(1) == "valuelist") && 
          (gTokenList.at(2) == "param") &&
          (gpItemStruct != nullptr)) {
        gpItemStruct = nullptr;
      }
      // bit/valuelist/item
      else if ((currentToken == "item") && 
               (gTokenList.at(1) == "valuelist") && 
               (gTokenList.at(2) == "bit") &&
               (gpItemStruct != nullptr)) {
        gpItemStruct = nullptr;
      }
      // valuelist/item
      else if ((currentToken == "name") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gpValueStruct != nullptr)) {
        ;
      }
      else if ((currentToken == "description") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gpValueStruct != nullptr)) {
        ;
      }
      else if ((currentToken == "infourl") && 
               (gTokenList.at(1) == "item") && 
               (gTokenList.at(2) == "valuelist") &&
               (gpValueStruct != nullptr)) {
        ;
      }
      // dmatrix/action/param/valuelist/item
      else if ((currentToken == "item") && 
          (gTokenList.at(1) == "valuelist") && 
          (gTokenList.at(2) == "param") &&
          (gTokenList.at(3) == "action") &&
          (gTokenList.at(4) == "dmatrix") &&
          (gpValueStruct != nullptr)) {
        gpValueStruct = nullptr;
      }
      break;

    case 8:
      if ( (gTokenList.at(0) == "item") && 
               (gTokenList.at(1) == "valuelist") &&
               (gTokenList.at(2) == "bit") &&
               (gTokenList.at(3) == "reg") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct = nullptr;
      }        
      // dmatrix/action/param/bit/valuelist/item
      else if ((currentToken == "item") && 
               (gTokenList.at(1) == "valuelist") && 
               (gTokenList.at(2) == "bit") &&
               (gTokenList.at(3) == "param") &&
               (gTokenList.at(4) == "action") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct = nullptr;
      }
  }

  // Reset language for 'name' and 'description' to default
  if ((gTokenList.at(0) == "name") || (gTokenList.at(0) == "description") || (gTokenList.at(0) == "infourl")) {
    gLastLanguage = "en";
  }

  gTokenList.pop_front(); // remove current token
  gdepth_xml_parser--;
}

// clang-format on

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

  gLastLanguage = "en";

  gpItemStruct        = nullptr;
  gpPictureStruct     = nullptr;
  gpVideoStruct       = nullptr;
  gpFirmwareStruct    = nullptr;
  gpDriverStruct      = nullptr;
  gpManualStruct      = nullptr;
  gpRegisterStruct    = nullptr;
  gpBitStruct         = nullptr;
  gpValueStruct       = nullptr;
  gpActionParamStruct = nullptr;
  gpActionStruct      = nullptr;
  gpEventStruct       = nullptr;
  gpEventDataStruct   = nullptr;

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

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// getDescriptionList
//

int
CMDF::getDescriptionList(json &j, std::map<std::string, std::string> &map)
{
  // Register description
  if (j.contains("description")) {

    if (j["description"].is_string()) {
      map["en"] = j["description"];
      spdlog::debug("Parse-JSON: Description: {0} language: 'en' ", j["description"]);
    }
    else if (j["description"].is_object()) {
      for (auto &item : j["description"].items()) {
        map[item.key()] = item.value();
        spdlog::debug("Parse-JSON: Description: {0} language: '{1}'", item.value(), item.key());
      }
    }
    else {
      spdlog::trace("Parse-JSON: No Description (not string, not object)");
    }
  }
  else {
    spdlog::trace("Parse-JSON: No Description.");
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getInfoUrlList
//

int
CMDF::getInfoUrlList(json &j, std::map<std::string, std::string> &map)
{
  // Info URL
  if (j.contains("infourl")) {

    if (j["infourl"].is_string()) {
      map["en"] = j["infourl"];
      spdlog::debug("Parse-JSON: InfoURL: {0} language: 'en' ", j["infourl"]);
    }
    else if (j["infourl"].is_object()) {
      for (auto &item : j["infourl"].items()) {
        map[item.key()] = item.value();
        spdlog::debug("Parse-JSON: InfoURL: {0} language: '{1}'", item.value(), item.key());
      }
    }
    else {
      spdlog::trace("Parse-JSON: No InfoURL (not string, not object)");
    }
  }
  else {
    spdlog::trace("Parse-JSON: No InfoURL.");
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getValueList
//

int
CMDF::getValueList(json &j, std::deque<CMDF_Value *> &list)
{
  // Register valuelist
  if (j.contains("valuelist") && j["valuelist"].is_array()) {
    for (auto &item : j["valuelist"].items()) {
      if (item.value().is_object()) {

        json jvalue(item.value());

        CMDF_Value *pvalue = new CMDF_Value();
        if (pvalue == nullptr) {
          spdlog::error("Parse-JSON: Failed to allocate memory for register value item.");
          return VSCP_ERROR_PARSING;
        }

        list.push_back(pvalue);

        if (jvalue.contains("name") && jvalue["name"].is_string()) {
          pvalue->m_name = jvalue["name"];
          vscp_trim(pvalue->m_name);
          vscp_makeLower(pvalue->m_name);
          spdlog::debug("Parse-JSON: Value list name: {0}", pvalue->m_name);
        }
        else {
          pvalue->m_name = "";
          spdlog::error("Parse-JSON: No register valuelist name defined (defaults to empty).");
        }

        if (jvalue.contains("value") && jvalue["value"].is_string()) {
          pvalue->m_strValue = jvalue["value"];
          spdlog::debug("Parse-JSON: Value list value: {0}", pvalue->m_strValue);
        }
        else {
          spdlog::error("Parse-JSON: No register valuelist value defined (defaults to zero).");
        }

        if (getDescriptionList(jvalue, pvalue->m_mapDescription) != VSCP_ERROR_SUCCESS) {
          spdlog::error("Parse-JSON: Failed to get register bit description.");
          return VSCP_ERROR_PARSING;
        }

        if (getInfoUrlList(jvalue, pvalue->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
          spdlog::error("Parse-JSON: Failed to get register bit infourl.");
          return VSCP_ERROR_PARSING;
        }
      }
      else {
        spdlog::trace("Parse-JSON: No register valuelist value is is not object");
      }
    }
  } // valuelist

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  getBitList
//

int
CMDF::getBitList(json &j, std::deque<CMDF_Bit *> &list)
{
  // Register bit
  if (j.contains("bit") && j["bit"].is_array()) {
    for (auto &item : j["bit"].items()) {
      if (item.value().is_object()) {

        json j(item.value());

        CMDF_Bit *pbit = new CMDF_Bit();
        if (pbit == nullptr) {
          spdlog::error("Parse-JSON: Failed to allocate memory for register bit item.");
          return VSCP_ERROR_PARSING;
        }

        list.push_back(pbit);

        if (j.contains("name") && j["name"].is_string()) {
          pbit->m_name = j["name"];
          vscp_trim(pbit->m_name);
          vscp_makeLower(pbit->m_name);
          spdlog::debug("Parse-JSON: Bitlist name: {0}", pbit->m_name);
        }
        else {
          pbit->m_name = "";
          spdlog::error("Parse-JSON: No register bit name defined (defaults to empty).");
        }

        if (j.contains("pos") && j["pos"].is_number()) {
          pbit->m_pos = j["pos"];
          spdlog::debug("Parse-JSON: Bitlist pos: {0}", pbit->m_pos);
        }
        else if (j.contains("pos") && j["pos"].is_string()) {
          pbit->m_pos = vscp_readStringValue(j["pos"]);
          spdlog::debug("Parse-JSON: Bitlist pos: {0}", pbit->m_pos);
        }
        else {
          pbit->m_pos = 0;
          spdlog::error("Parse-JSON: No register bit pos defined (defaults to zero).");
        }

        if (j.contains("width") && j["width"].is_number()) {
          pbit->m_width = j["width"];
          spdlog::debug("Parse-JSON: Bitlist width: {0}", pbit->m_width);
        }
        else if (j.contains("width") && j["width"].is_string()) {
          pbit->m_width = vscp_readStringValue(j["width"]);
          spdlog::debug("Parse-JSON: Bitlist width: {0}", pbit->m_width);
        }
        else {
          pbit->m_width = 1;
          spdlog::trace("Parse-JSON: No register bit width defined (defaults to one).");
        }

        if (j.contains("default") && j["default"].is_number()) {
          pbit->m_default = j["default"];
          spdlog::debug("Parse-JSON: Bitlist default: {0}", pbit->m_default);
        }
        else if (j.contains("default") && j["default"].is_boolean()) {
          pbit->m_default = j["default"] ? 1 : 0;
          spdlog::debug("Parse-JSON: Bitlist default: {0}", pbit->m_default);
        }
        else if (j.contains("default") && j["default"].is_string()) {
          pbit->m_default = vscp_readStringValue(j["default"]);
          spdlog::debug("Parse-JSON: Bitlist default: {0}", pbit->m_default);
        }
        else {
          pbit->m_default = 1;
          spdlog::trace("Parse-JSON: No register bit default defined (defaults to zero).");
        }

        if (j.contains("min") && j["min"].is_number()) {
          pbit->m_min = j["min"];
          spdlog::debug("Parse-JSON: Bitlist min: {0}", pbit->m_min);
        }
        else if (j.contains("min") && j["min"].is_string()) {
          pbit->m_min = vscp_readStringValue(j["min"]);
          spdlog::debug("Parse-JSON: Bitlist min: {0}", pbit->m_min);
        }
        else {
          pbit->m_min = 0;
          spdlog::trace("Parse-JSON: No register bit min defined (defaults to zero).");
        }

        if (j.contains("max") && j["max"].is_number()) {
          pbit->m_max = j["max"];
          spdlog::debug("Parse-JSON: Bitlist max: {0}", pbit->m_max);
        }
        else if (j.contains("max") && j["max"].is_string()) {
          pbit->m_max = vscp_readStringValue(j["max"]);
          spdlog::debug("Parse-JSON: Bitlist max: {0}", pbit->m_max);
        }
        else {
          pbit->m_min = 255;
          spdlog::trace("Parse-JSON: No register bit max defined (defaults to 255).");
        }

        if (j.contains("access") && j["access"].is_string()) {
          std::string strAccess = j["access"];
          vscp_trim(strAccess);
          vscp_makeLower(strAccess);
          pbit->m_access = MDF_REG_ACCESS_NONE;
          if (strAccess == "w") {
            pbit->m_access = MDF_REG_ACCESS_WRITE_ONLY;
            spdlog::debug("Parse-JSON: Bitlist access: Read Only");
          }
          else if (strAccess == "r") {
            pbit->m_access = MDF_REG_ACCESS_READ_ONLY;
            spdlog::debug("Parse-JSON: Bitlist access: Write Only");
          }
          else if (strAccess == "rw") {
            pbit->m_access = MDF_REG_ACCESS_READ_WRITE;
            spdlog::debug("Parse-JSON: Bitlist access: Read/Write");
          }
        }
        else {
          pbit->m_access = MDF_REG_ACCESS_READ_WRITE;
          spdlog::info("Parse-JSON: No register access defined (defaults to 'rw').");
        }

        // Get description
        if (getDescriptionList(j, pbit->m_mapDescription) != VSCP_ERROR_SUCCESS) {
          spdlog::error("Parse-JSON: Failed to get register bit description.");
          return VSCP_ERROR_PARSING;
        }

        // Get info URL
        if (getInfoUrlList(j, pbit->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
          spdlog::error("Parse-JSON: Failed to get register bit infourl.");
          return VSCP_ERROR_PARSING;
        }

        // Get valuelist
        if (getValueList(j, pbit->m_list_value) != VSCP_ERROR_SUCCESS) {
          spdlog::warn("Parse-JSON: Failed to get register bit valuelist.");
        }

      } // is object
    }   // for
  }     // bit

  return VSCP_ERROR_SUCCESS;
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

  // REDIRECT  <-----
  if (j.contains("redirect") && j["redirect"].is_string()) {
    spdlog::trace("Parse-JSON: REDIRECT found '{0}', parsing aborted.", m_name);
    m_redirectUrl = j["redirect"];
    return VSCP_ERROR_SUCCESS;
  }

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
        vscp_trim(m_name);
        vscp_makeLower(m_name);
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
      spdlog::debug("Parse-JSON: Module model: {0}", m_strModule_Model);
    }
    else {
      spdlog::debug("Parse-JSON: There is no module model.");
    }

    // Module version - not mandatory
    if (j["module"].contains("version") && j["module"]["version"].is_string()) {
      m_strModule_Version = j["module"]["version"];
      spdlog::debug("Parse-JSON: Module version: {0}", m_strModule_Version);
    }
    else {
      spdlog::debug("Parse-JSON: There is no module version.");
    }

    // Module changed - not mandatory
    if (j["module"].contains("changed") && j["module"]["changed"].is_string()) {
      m_strModule_changeDate = j["module"]["changed"];
      spdlog::debug("Parse-JSON: Module changed: {0}", m_strModule_changeDate);
    }
    else {
      spdlog::debug("Parse-JSON: There is no module changed.");
    }

    // Module buffersize - not mandatory
    m_module_bufferSize = 8;
    if (j["module"].contains("buffersize") && j["module"]["buffersize"].is_number_integer()) {
      m_module_bufferSize = j["module"]["buffersize"];
      spdlog::debug("Parse-JSON: Module buffersize: {0}", m_module_bufferSize);
    }
    else {
      spdlog::debug("Parse-JSON: There is no module buffersize.");
    }

    // Module description - not mandatory
    // Can either be string or object

    if (getDescriptionList(j["module"], m_mapDescription) != VSCP_ERROR_SUCCESS) {
      spdlog::warn("Parse-JSON: Failed to get module description.");
    }

    if (getInfoUrlList(j["module"], m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
      spdlog::warn("Parse-JSON: Failed to get module infourl.");
    }

    // ------------------------------------------------------------------------
    //                                  Module
    // ------------------------------------------------------------------------

    // Module infourl - not mandatory
    // Can either be string or object
    if (j["module"].contains("manufacturer") && j["module"]["manufacturer"].is_object()) {

      json jsub = j["module"]["manufacturer"];

      if (jsub.contains("name") && jsub["name"].is_string()) {
        m_manufacturer.m_strName = jsub["name"];
        spdlog::debug("Parse-JSON: Module manufacturer name: {0}", m_manufacturer.m_strName);
      }
      else {
        spdlog::warn("Parse-JSON: Failed to read module manufacturer name (not string)");
      }

      // Adress - Is always object
      if (jsub.contains("address") && jsub["address"].is_object()) {

        json jsub2 = jsub["address"];
        if (jsub2.contains("street") && jsub2["street"].is_string()) {
          m_manufacturer.m_address.m_strStreet = jsub2["street"];
          spdlog::debug("Parse-JSON: Module manufacturer address: {0}", m_manufacturer.m_address.m_strStreet);
        }

        if (jsub2.contains("town") && jsub2["town"].is_string()) {
          m_manufacturer.m_address.m_strTown = jsub2["town"];
          spdlog::debug("Parse-JSON: Module manufacturer town: {0}", m_manufacturer.m_address.m_strTown);
        }

        if (jsub2.contains("city") && jsub2["city"].is_string()) {
          m_manufacturer.m_address.m_strCity = jsub2["city"];
          spdlog::debug("Parse-JSON: Module manufacturer city: {0}", m_manufacturer.m_address.m_strCity);
        }

        if (jsub2.contains("postcode") && jsub2["postcode"].is_string()) {
          m_manufacturer.m_address.m_strPostCode = jsub2["postcode"];
          spdlog::debug("Parse-JSON: Module manufacturer post code: {0}", m_manufacturer.m_address.m_strPostCode);
        }

        if (jsub2.contains("state") && jsub2["state"].is_string()) {
          m_manufacturer.m_address.m_strState = jsub2["state"];
          spdlog::debug("Parse-JSON: Module manufacturer state: {0}", m_manufacturer.m_address.m_strState);
        }

        if (jsub2.contains("region") && jsub2["region"].is_string()) {
          m_manufacturer.m_address.m_strRegion = jsub2["region"];
          spdlog::debug("Parse-JSON: Module manufacturer region: {0}", m_manufacturer.m_address.m_strRegion);
        }

        if (jsub2.contains("country") && jsub2["country"].is_string()) {
          m_manufacturer.m_address.m_strCountry = jsub2["country"];
          spdlog::debug("Parse-JSON: Module manufacturer country: {0}", m_manufacturer.m_address.m_strCountry);
        }

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
              m_manufacturer.m_list_Phone.push_back(ptel);

              json jsub2 = phone.value();

              if (jsub2.contains("number") && jsub2["number"].is_string()) {
                ptel->m_name = jsub2["number"];
                spdlog::debug("Parse-JSON: Module manufacturer telephone: {0}", ptel->m_name);
              }

              // Description is language specific. Can be string or object
              if (getDescriptionList(jsub2, ptel->m_mapDescription) != VSCP_ERROR_SUCCESS) {
                spdlog::warn("Parse-JSON: Failed to get tel description.");
              }

              if (getInfoUrlList(jsub2, ptel->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
                spdlog::warn("Parse-JSON: Failed to tel bit infourl.");
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
              m_manufacturer.m_list_Fax.push_back(pfax);

              json jsub2 = fax.value();

              if (jsub2.contains("number") && jsub2["number"].is_string()) {
                pfax->m_name = jsub2["number"];
                spdlog::debug("Parse-JSON: Module manufacturer fax: {0}", pfax->m_name);
              }

              // Description is language specific. Can be string or object
              if (getDescriptionList(jsub2, pfax->m_mapDescription) != VSCP_ERROR_SUCCESS) {
                spdlog::warn("Parse-JSON: Failed to get fax description.");
              }

              if (getInfoUrlList(jsub2, pfax->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
                spdlog::warn("Parse-JSON: Failed to get fax infourl.");
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
              m_manufacturer.m_list_Email.push_back(pemail);

              json jsub2 = email.value();

              if (jsub2.contains("address") && jsub2["address"].is_string()) {
                pemail->m_name = jsub2["address"];
                spdlog::debug("Parse-JSON: Module manufacturer email: {0}", pemail->m_name);
              }

              // Description is language specific. Can be string or object
              if (getDescriptionList(jsub2, pemail->m_mapDescription) != VSCP_ERROR_SUCCESS) {
                spdlog::warn("Parse-JSON: Failed to get address description.");
              }

              if (getInfoUrlList(jsub2, pemail->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
                spdlog::warn("Parse-JSON: Failed to get address infourl.");
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
              m_manufacturer.m_list_Email.push_back(pweb);

              json jsub2 = web.value();

              if (jsub2.contains("url") && jsub2["url"].is_string()) {
                pweb->m_name = jsub2["url"];
                spdlog::debug("Parse-JSON: Module manufacturer web: {0}", pweb->m_name);
              }
              else if (jsub2.contains("address") && jsub2["address"].is_string()) {
                pweb->m_name = jsub2["address"];
                spdlog::debug("Parse-JSON: Module manufacturer web: {0}", pweb->m_name);
              }
              else {
                spdlog::warn("Parse-JSON: No web url/address.");
              }

              // Description is language specific. Can be string or object
              if (getDescriptionList(jsub2, pweb->m_mapDescription) != VSCP_ERROR_SUCCESS) {
                spdlog::warn("Parse-JSON: Failed to get web description.");
              }

              if (getInfoUrlList(jsub2, pweb->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
                spdlog::warn("Parse-JSON: Failed to get web infourl.");
              }
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

        if (jsub.contains("algorithm") && jsub["algorithm"].is_string()) {
          m_bootInfo.m_nAlgorithm = vscp_readStringValue(jsub["algorithm"]);
          spdlog::debug("Parse-JSON: Boot loader algorithm: {0} ", m_bootInfo.m_nAlgorithm);
        }
        else if (jsub.contains("algorithm") && jsub["algorithm"].is_number()) {
          m_bootInfo.m_nAlgorithm = jsub["algorithm"];
          spdlog::debug("Parse-JSON: Boot loader algorithm: {0} ", m_bootInfo.m_nAlgorithm);
        }

        if (jsub.contains("blocksize") && jsub["blocksize"].is_string()) {
          m_bootInfo.m_nBlockSize = vscp_readStringValue(jsub["blocksize"]);
          spdlog::debug("Parse-JSON: Boot loader block size: {0} ", m_bootInfo.m_nBlockSize);
        }
        else if (jsub.contains("blocksize") && jsub["blocksize"].is_number()) {
          m_bootInfo.m_nBlockSize = jsub["blocksize"];
          spdlog::debug("Parse-JSON: Boot loader block size: {0} ", m_bootInfo.m_nBlockSize);
        }

        if (jsub.contains("blockcount") && jsub["blockcount"].is_string()) {
          m_bootInfo.m_nBlockCount = vscp_readStringValue(jsub["blockcount"]);
          spdlog::debug("Parse-JSON: MBoot loader block count: {0} ", m_bootInfo.m_nBlockCount);
        }
        else if (jsub.contains("blockcount") && jsub["blockcount"].is_number()) {
          m_bootInfo.m_nBlockCount = jsub["blockcount"];
          spdlog::debug("Parse-JSON: MBoot loader block count: {0} ", m_bootInfo.m_nBlockCount);
        }
      }

      // ------------------------------------------------------------------------
      //                                Registers
      // ------------------------------------------------------------------------

      if (j["module"].contains("register") && j["module"]["register"].is_array()) {
        for (auto &reg : j["module"]["register"].items()) {
          // std::cout << "key: " << reg.key() << ", value:" << reg.value() << '\n';
          if (reg.value().is_object()) {

            CMDF_Register *preg = new CMDF_Register();
            if (preg == nullptr) {
              spdlog::error("Parse-JSON: Failed to allocate memory for register item.");
              return VSCP_ERROR_PARSING;
            }

            m_list_register.push_back(preg);

            json jreg(reg.value());
            // std::cout << "REG: " << jreg.dump() << '\n';

            // Register Page
            if (jreg.contains("page") && jreg["page"].is_number()) {
              preg->m_page = jreg["page"];
              spdlog::debug("Parse-JSON: Module register page: {0}", preg->m_page);
            }
            else if (jreg.contains("page") && jreg["page"].is_string()) {
              preg->m_page = vscp_readStringValue(jreg["page"]);
              spdlog::debug("Parse-JSON: Module register page: {0}", preg->m_page);
            }
            else {
              preg->m_page = 0;
              spdlog::warn("Parse-JSON: No register page defined (defaults to zero).");
            }

            // Register offset
            if (jreg.contains("offset") && jreg["offset"].is_number()) {
              preg->m_offset = jreg["offset"];
              spdlog::debug("Parse-JSON: Module register offset: {0}", preg->m_offset);
            }
            else if (jreg.contains("offset") && jreg["offset"].is_string()) {
              preg->m_offset = vscp_readStringValue(jreg["offset"]);
              spdlog::debug("Parse-JSON: Module register offset: {0}", preg->m_offset);
            }
            else {
              preg->m_offset = 0;
              spdlog::error("Parse-JSON: No register offset defined (defaults to zero).");
            }

            // Register span
            if (jreg.contains("span") && jreg["span"].is_number()) {
              preg->m_span = jreg["span"];
              spdlog::debug("Parse-JSON: Module register span: {0}", preg->m_span);
              if ((preg->m_span == 0) || (preg->m_span > 8)) {
                spdlog::warn("Parse-JSON: Register span is greater than eight or zero. This is not supported. "
                             "Default set (1)");
              }
            }
            else if (jreg.contains("span") && jreg["span"].is_string()) {
              preg->m_span = vscp_readStringValue(jreg["span"]);
              spdlog::debug("Parse-JSON: Module register span: {0}", preg->m_span);
              if ((preg->m_span == 0) || (preg->m_span > 8)) {
                spdlog::warn("Parse-JSON: Register span is greater than eight or zero. This is not supported. "
                             "Default set (1)");
              }
            }
            else {
              preg->m_span = 1;
              spdlog::info("Parse-JSON: No register span defined (defaults to one byte).");
            }

            // Register width
            if (jreg.contains("width") && jreg["width"].is_number()) {
              preg->m_width = jreg["width"];
              spdlog::debug("Parse-JSON: Module register width: {0}", preg->m_width);
            }
            else if (jreg.contains("width") && jreg["width"].is_string()) {
              preg->m_width = vscp_readStringValue(jreg["width"]);
              spdlog::debug("Parse-JSON: Module register width: {0}", preg->m_width);
            }
            else {
              preg->m_width = 8;
              spdlog::info("Parse-JSON: No register width defined (defaults to eight bits).");
            }

            // Register type
            if (jreg.contains("type") && jreg["type"].is_string()) {
              std::string strType = jreg["type"];
              vscp_trim(strType);
              vscp_lower(strType);
              if (strType == "std") {
                preg->m_type = MDF_REG_TYPE_STANDARD;
                spdlog::debug("Parse-JSON: Module register type: standard {0}", preg->m_type);
              }
              else if ((strType == "dmatrix1") || (strType == "dm") || (strType == "dmatrix")) {
                preg->m_type = MDF_REG_TYPE_DMATRIX1;
                spdlog::debug("Parse-JSON: Module register type: dmatrix1 {0}", preg->m_type);
              }
              else if (strType == "block") {
                preg->m_type = MDF_REG_TYPE_BLOCK;
                spdlog::debug("Parse-JSON: Module register type: block {0}", preg->m_type);
              }
              else {
                spdlog::warn("Invalid block type: {0} - will be set to standard", strType);
                preg->m_type = MDF_REG_TYPE_STANDARD;
                spdlog::debug("Parse-JSON: Module register type: standard {0}", preg->m_type);
              }
            }
            else {
              preg->m_type = MDF_REG_TYPE_STANDARD;
              spdlog::info("Parse-JSON: No register type defined (Set to standard).");
            }

            // Register min
            if (jreg.contains("min") && jreg["min"].is_number()) {
              preg->m_min = jreg["min"];
              spdlog::debug("Parse-JSON: Module register min: {0}", preg->m_min);
            }
            else if (jreg.contains("min") && jreg["min"].is_string()) {
              preg->m_min = vscp_readStringValue(jreg["min"]);
              spdlog::debug("Parse-JSON: Module register min: {0}", preg->m_min);
            }
            else {
              preg->m_min = 0;
              spdlog::info("Parse-JSON: No register min defined (defaults to zero).");
            }

            // Register max
            if (jreg.contains("max") && jreg["max"].is_number()) {
              preg->m_max = jreg["max"];
              spdlog::debug("Parse-JSON: Module register max: {0}", preg->m_max);
            }
            else if (jreg.contains("max") && jreg["max"].is_string()) {
              preg->m_max = vscp_readStringValue(jreg["max"]);
              spdlog::debug("Parse-JSON: Module register max: {0}", preg->m_max);
            }
            else {
              preg->m_max = 0;
              spdlog::info("Parse-JSON: No register max defined (defaults to 255).");
            }

            if (jreg.contains("default") && jreg["default"].is_string()) {
              std::string default_value = jreg["default"];
              vscp_trim(default_value);
              vscp_makeUpper(default_value);
              preg->m_strDefault = default_value;
              spdlog::debug("Parse-JSON: Module register default: {0}", preg->m_strDefault);
            }
            else if (jreg.contains("default") && jreg["default"].is_number()) {
              preg->m_strDefault = std::to_string((int) jreg["default"]);
              spdlog::debug("Parse-JSON: Module register value and default set to defined default: {0}", preg->m_strDefault);
            }
            else if (jreg.contains("default") && jreg["default"].is_boolean()) {
              preg->m_strDefault = jreg["default"] ? "true" : "false";
              spdlog::debug("Parse-JSON: Module register value and default set to defined default: {0}", preg->m_strDefault);
            }

            if (jreg.contains("access") && jreg["access"].is_string()) {
              std::string strAccess = jreg["access"];
              vscp_trim(strAccess);
              vscp_makeLower(strAccess);
              preg->m_access = MDF_REG_ACCESS_NONE;
              if (strAccess == "rw") {
                preg->m_access = MDF_REG_ACCESS_READ_WRITE;
                spdlog::debug("Parse-JSON: Module register access: Read/Write");
              }
              else if (strAccess == "r") {
                preg->m_access = MDF_REG_ACCESS_READ_ONLY;
                spdlog::debug("Parse-JSON: Module register access: Read Only");
              }
              else if (strAccess == "w") {
                preg->m_access = MDF_REG_ACCESS_WRITE_ONLY;
                spdlog::debug("Parse-JSON: Module register access: Write Only");
              }
            }
            else {
              preg->m_max = 0;
              spdlog::info("Parse-JSON: No register max defined (defaults to 255).");
            }

            // Register name
            if (jreg.contains("name") && jreg["name"].is_string()) {
              preg->m_name = jreg["name"];
              vscp_trim(preg->m_name);
              vscp_makeLower(preg->m_name);
              spdlog::info("Parse-JSON: Register name set to {0}.", preg->m_name);
            }
            else {
              spdlog::info("Parse-JSON: No register name defined (defaults to empty string).");
            }

            // Foreground color (VSCP Works)
            if (jreg.contains("fgcolor") && jreg["fgcolor"].is_string()) {
              preg->m_fgcolor = vscp_readStringValue(jreg["fgcolor"]);
              spdlog::debug("Parse-JSON: Remote variable foreground color set to {}.", preg->m_fgcolor);
            }
            else if (jreg.contains("fgcolor") && jreg["fgcolor"].is_number()) {
              preg->m_fgcolor = jreg["fgcolor"];
              spdlog::debug("Parse-JSON: Remote variable foreground color to {}.", preg->m_fgcolor);
            }
            else {
              preg->m_fgcolor = 0;
              spdlog::trace("Parse-JSON: No foreground color defined (set to 0).");
            }

            // Backround color (VSCP Works)
            if (jreg.contains("bgcolor") && jreg["bgcolor"].is_string()) {
              preg->m_bgcolor = vscp_readStringValue(jreg["bgcolor"]);
              spdlog::debug("Parse-JSON: Remote variable background color set to {}.", preg->m_bgcolor);
            }
            else if (jreg.contains("bgcolor") && jreg["bgcolor"].is_number()) {
              preg->m_bgcolor = jreg["bgcolor"];
              spdlog::debug("Parse-JSON: Remote variable background color to {}.", preg->m_bgcolor);
            }
            else {
              preg->m_bgcolor = 0;
              spdlog::trace("Parse-JSON: No background color not defined (set to 0).");
            }

            if (getDescriptionList(jreg, preg->m_mapDescription) != VSCP_ERROR_SUCCESS) {
              spdlog::warn("Parse-JSON: Failed to get register bit description.");
            }

            if (getInfoUrlList(jreg, preg->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
              spdlog::warn("Parse-JSON: Failed to get register bit infourl.");
            }

            // Register valuelist
            if (getValueList(jreg, preg->m_list_value) != VSCP_ERROR_SUCCESS) {
              spdlog::warn("Parse-JSON: Failed to get register valuelist.");
            }

            // Register bitlist
            if (getBitList(jreg, preg->m_list_bit) != VSCP_ERROR_SUCCESS) {
              spdlog::warn("Parse-JSON: Failed to get register bitlist.");
            }
          }
        }
      }
      else {
        spdlog::warn("Parse-JSON: Failed to read module registers");
      }

      //------------------------------------------------------------------------
      //                             Remote variable
      //------------------------------------------------------------------------

      if (j["module"].contains("remotevar") && j["module"]["remotevar"].is_array()) {
        for (auto &rvar : j["module"]["remotevar"].items()) {
          // std::cout << "key: " << rvar.key() << ", value:" << rvar.value() << '\n';
          spdlog::trace("Parse-JSON: Remote variable key = {0} type = {1}.", rvar.key(), rvar.value());
          if (rvar.value().is_object()) {

            CMDF_RemoteVariable *prvar = new CMDF_RemoteVariable();
            if (prvar == nullptr) {
              spdlog::error("Parse-JSON: Failed to allocate memory for remote variable item.");
              return VSCP_ERROR_PARSING;
            }

            m_list_remotevar.push_back(prvar);

            json jrvar = rvar.value();

            // Remote variable name
            if (jrvar.contains("name") && jrvar["name"].is_string()) {
              prvar->m_name = jrvar["name"];
              vscp_trim(prvar->m_name);
              vscp_makeLower(prvar->m_name);
              spdlog::debug("Parse-JSON: Remote variable name set to {}.", prvar->m_name);
            }
            else {
              spdlog::error("Parse-JSON: No remote variable name defined.");
              return VSCP_ERROR_PARSING;
            }

            // Remote variable type
            if (jrvar.contains("type") && jrvar["type"].is_string()) {
              std::string str = jrvar["type"];
              vscp_trim(str);
              vscp_makeLower(str);
              if (str == "string") {
                prvar->m_type = remote_variable_type_string;
                spdlog::debug("Parse-JSON: Remote variable type set to 'string' {}.", prvar->m_type);
              }
              else if (str == "bool") {
                prvar->m_type = remote_variable_type_boolean;
                spdlog::debug("Parse-JSON: Remote variable type set to 'boolena' {}.", prvar->m_type);
              }
              else if ((str == "int8") || (str == "int8_t") || (str == "char")) {
                prvar->m_type = remote_variable_type_int8_t;
                spdlog::debug("Parse-JSON: Remote variable type set to 'int8_t' {}.", prvar->m_type);
              }
              else if ((str == "uint8") || (str == "uint8_t") || (str == "byte")) {
                prvar->m_type = remote_variable_type_uint8_t;
                spdlog::debug("Parse-JSON: Remote variable type set to 'uint8_t' {}.", prvar->m_type);
              }
              else if ((str == "int16") || (str == "int16_t") || (str == "short")) {
                prvar->m_type = remote_variable_type_int16_t;
                spdlog::debug("Parse-JSON: Remote variable type set to 'int16_t' {}.", prvar->m_type);
              }
              else if ((str == "uint16") || (str == "uint16_t")) {
                prvar->m_type = remote_variable_type_uint16_t;
                spdlog::debug("Parse-JSON: Remote variable type set to 'uint16_t' {}.", prvar->m_type);
              }
              else if ((str == "int32") || (str == "int32_t") || (str == "long")) {
                prvar->m_type = remote_variable_type_int32_t;
                spdlog::debug("Parse-JSON: Remote variable type set to 'int32_t' {}.", prvar->m_type);
              }
              else if ((str == "uint32") || (str == "uint32_t") || (str == "unsigned")) {
                prvar->m_type = remote_variable_type_uint32_t;
                spdlog::debug("Parse-JSON: Remote variable type set to 'uint32_t' {}.", prvar->m_type);
              }
              else if ((str == "int64") || (str == "int64_t") || (str == "longlong")) {
                prvar->m_type = remote_variable_type_int64_t;
                spdlog::debug("Parse-JSON: Remote variable type set to 'int64_t' {}.", prvar->m_type);
              }
              else if ((str == "uint64") || (str == "uint64_t")) {
                prvar->m_type = remote_variable_type_uint64_t;
                spdlog::debug("Parse-JSON: Remote variable type set to 'uint64_t' {}.", prvar->m_type);
              }
              else if (str == "float") {
                prvar->m_type = remote_variable_type_float;
                spdlog::debug("Parse-JSON: Remote variable type set to 'float' {}.", prvar->m_type);
              }
              else if (str == "double") {
                prvar->m_type = remote_variable_type_double;
                spdlog::debug("Parse-JSON: Remote variable type set to 'double' {}.", prvar->m_type);
              }
              else if (str == "date") {
                prvar->m_type = remote_variable_type_date;
                spdlog::debug("Parse-JSON: Remote variable type set to 'date' {}.", prvar->m_type);
              }
              else if (str == "time") {
                prvar->m_type = remote_variable_type_time;
                spdlog::debug("Parse-JSON: Remote variable type set to 'time' {}.", prvar->m_type);
              }
              else {
                prvar->m_type = remote_variable_type_unknown;
              }
            }
            else {
              // Must be a type if a remote variable is there
              spdlog::error("Parse-JSON: No remote variable type defined.");
              return VSCP_ERROR_PARSING;
            }

            // Default value. Always a string. Convert to the correct type
            // when default is set.
            if (jrvar.contains("default") && jrvar["default"].is_string()) {
              prvar->m_strDefault = jrvar["default"];
              spdlog::debug("Parse-JSON: Remote variable default set to {}.", prvar->m_strDefault);
            }
            else if (jrvar.contains("default") && jrvar["default"].is_number()) {
              prvar->m_strDefault = std::to_string((uint32_t) jrvar["default"]);
              spdlog::debug("Parse-JSON: Remote variable default set to {}.", prvar->m_strDefault);
            }
            else if (jrvar.contains("default") && jrvar["default"].is_number_integer()) {
              prvar->m_strDefault = std::to_string((int32_t) jrvar["default"]);
              spdlog::debug("Parse-JSON: Remote variable default set to {}.", prvar->m_strDefault);
            }
            else if (jrvar.contains("default") && jrvar["default"].is_number_unsigned()) {
              prvar->m_strDefault = std::to_string((uint32_t) jrvar["default"]);
              spdlog::debug("Parse-JSON: Remote variable default set to {}.", prvar->m_strDefault);
            }
            else if (jrvar.contains("default") && jrvar["default"].is_number_float()) {
              prvar->m_strDefault = std::to_string((double) jrvar["default"]);
              spdlog::debug("Parse-JSON: Remote variable default set to {}.", prvar->m_strDefault);
            }
            else if (jrvar.contains("default") && jrvar["default"].is_boolean()) {
              prvar->m_strDefault = jrvar["default"] ? "true" : "false";
              spdlog::debug("Parse-JSON: Remote variable default set to {}.", prvar->m_strDefault);
            }
            else {
              prvar->m_strDefault = "";
              spdlog::debug("Parse-JSON: No remote variable default defined (set to empty).");
            }

            // Page where the remote variable is located
            if (jrvar.contains("page") && jrvar["page"].is_string()) {
              prvar->m_page = vscp_readStringValue(jrvar["page"]);
              spdlog::debug("Parse-JSON: Remote variable page set to {}.", prvar->m_page);
            }
            else if (jrvar.contains("page") && jrvar["page"].is_number()) {
              prvar->m_page = jrvar["page"];
              spdlog::debug("Parse-JSON: Remote variable page set to {}.", prvar->m_page);
            }
            else {
              prvar->m_page = 0;
              spdlog::warn("Parse-JSON: No remote variable page defined (set to 0).");
            }

            // Offset where the remote variable is located
            if (jrvar.contains("offset") && jrvar["offset"].is_string()) {
              prvar->m_offset = vscp_readStringValue(jrvar["offset"]);
              spdlog::debug("Parse-JSON: Remote variable offset set to {}.", prvar->m_offset);
            }
            else if (jrvar.contains("offset") && jrvar["offset"].is_number()) {
              prvar->m_offset = jrvar["offset"];
              spdlog::debug("Parse-JSON: Remote variable offset set to {}.", prvar->m_offset);
            }
            else {
              // Must be an offset if a remote variable is there, but we accept
              // a default value. Flag as error because it is an error.
              prvar->m_offset = 0;
              spdlog::error("Parse-JSON: No remote variable offset defined (set to 0).");
            }

            // Bit position in byte where the boolean variable is located (0-7)
            // If -1 not used.
            if (jrvar.contains("bitpos") && jrvar["bitpos"].is_string()) {
              prvar->m_bitpos = vscp_readStringValue(jrvar["bitpos"]) & 0x07;
              spdlog::debug("Parse-JSON: Remote variable bitpos set to {}.", prvar->m_bitpos);
            }
            else if (jrvar.contains("bitpos") && jrvar["bitpos"].is_number()) {
              prvar->m_bitpos = jrvar["bitpos"];
              prvar->m_bitpos &= 7;
              spdlog::debug("Parse-JSON: Remote variable bitpos set to {}.", prvar->m_bitpos);
            }
            else {
              prvar->m_bitpos = -1;
              spdlog::trace("Parse-JSON: No remote variable bitpos defined (set to -1).");
            }

            // Size for a string (ignored for all other types)
            if (jrvar.contains("size") && jrvar["size"].is_string()) {
              prvar->m_size = vscp_readStringValue(jrvar["size"]);
              spdlog::debug("Parse-JSON: Remote variable size set to {}.", prvar->m_size);
            }
            else if (jrvar.contains("size") && jrvar["size"].is_number()) {
              prvar->m_size = jrvar["size"];
              spdlog::debug("Parse-JSON: Remote variable size set to {}.", prvar->m_size);
            }
            else {
              prvar->m_bitpos = 0;
              spdlog::trace("Parse-JSON: No remote variable size defined (set to 0).");
            }

            // Access rights for remote variable
            if (jrvar.contains("access") && jrvar["access"].is_string()) {
              std::string strAccess = jrvar["access"];
              vscp_trim(strAccess);
              vscp_makeLower(strAccess);
              prvar->m_access = MDF_REG_ACCESS_NONE;
              if (strAccess == "w") {
                spdlog::debug("Parse-JSON: Remote variable access set to write only.");
                prvar->m_access = MDF_REG_ACCESS_WRITE_ONLY;
              }
              else if (strAccess == "r") {
                spdlog::debug("Parse-JSON: Remote variable access set to read only.");
                prvar->m_access = MDF_REG_ACCESS_READ_ONLY;
              }
              else if (strAccess == "rw") {
                spdlog::debug("Parse-JSON: Remote variable access set to read/write.");
                prvar->m_access = MDF_REG_ACCESS_READ_WRITE;
              }
            }
            else {
              prvar->m_access = MDF_REG_ACCESS_READ_WRITE;
              spdlog::debug("Parse-JSON: No remote variable access defined (defaults to 'rw').");
            }

            // Foreground color (VSCP Works)
            if (jrvar.contains("fgcolor") && jrvar["fgcolor"].is_string()) {
              prvar->m_fgcolor = vscp_readStringValue(jrvar["fgcolor"]);
              spdlog::debug("Parse-JSON: Remote variable fore ground color set to {}.", prvar->m_fgcolor);
            }
            else if (jrvar.contains("fgcolor") && jrvar["fgcolor"].is_number()) {
              prvar->m_fgcolor = jrvar["fgcolor"];
              spdlog::debug("Parse-JSON: Remote variable fore ground color to {}.", prvar->m_fgcolor);
            }
            else {
              prvar->m_fgcolor = 0;
              spdlog::trace("Parse-JSON: No remote variable size defined (set to 0).");
            }

            // Backround color (VSCP Works)
            if (jrvar.contains("bgcolor") && jrvar["bgcolor"].is_string()) {
              prvar->m_bgcolor = vscp_readStringValue(jrvar["bgcolor"]);
              spdlog::debug("Parse-JSON: Remote variable background color set to {}.", prvar->m_bgcolor);
            }
            else if (jrvar.contains("bgcolor") && jrvar["bgcolor"].is_number()) {
              prvar->m_bgcolor = jrvar["bgcolor"];
              spdlog::debug("Parse-JSON: Remote variable background color to {}.", prvar->m_bgcolor);
            }
            else {
              prvar->m_bgcolor = 0;
              spdlog::trace("Parse-JSON: No remote variable bgcolor not defined (set to 0).");
            }

            if (getDescriptionList(jrvar, prvar->m_mapDescription) != VSCP_ERROR_SUCCESS) {
              spdlog::warn("Parse-JSON: Failed to get remote variable description.");
            }

            if (getInfoUrlList(jrvar, prvar->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
              spdlog::warn("Parse-JSON: Failed to get remote variable infourl.");
            }

            // Register valuelist
            if (getValueList(jrvar, prvar->m_list_value) != VSCP_ERROR_SUCCESS) {
              spdlog::warn("Parse-JSON: Failed to get remot evariable valuelist.");
            }

            // Register bitlist
            if (getBitList(jrvar, prvar->m_list_bit) != VSCP_ERROR_SUCCESS) {
              spdlog::warn("Parse-JSON: Failed to get remote variable bitlist.");
            }
          }
        }
      }
      else {
        spdlog::warn("Parse-JSON: Failed to read module remote variable");
      }

      //------------------------------------------------------------------------
      //                            Decision Matrix
      //------------------------------------------------------------------------

      if (j["module"].contains("dmatrix") && j["module"]["dmatrix"].is_object()) {

        json jdmatrix = j["module"]["dmatrix"];
        // std::cout << "DM: " << jdmatrix.dump() << '\n';

        // Level
        if (jdmatrix.contains("level")) {
          if (jdmatrix["level"].is_number()) {
            m_dmInfo.m_level = jdmatrix["level"];
            spdlog::debug("Parse-JSON: DM level set to {0}.", m_dmInfo.m_level);
          }
          else if (jdmatrix["level"].is_string()) {
            m_dmInfo.m_level = vscp_readStringValue(jdmatrix["level"]);
            spdlog::debug("Parse-JSON: DM level set to {0}.", m_dmInfo.m_level);
          }
          else {
            m_dmInfo.m_level = 1;
            spdlog::warn("DM level is of strange type (not number or string). Set to default (1).");
          }
        }
        else {
          m_dmInfo.m_level = 1;
          spdlog::warn("Parse-JSON: No DM level defined. Set to default (1).");
        }

        // start page
        if (jdmatrix.contains("start-page")) {
          if (jdmatrix["start-page"].is_number()) {
            m_dmInfo.m_startPage = jdmatrix["start-page"];
            spdlog::debug("Parse-JSON: DM start page set to {0}.", m_dmInfo.m_startPage);
          }
          else if (jdmatrix["start-page"].is_string()) {
            m_dmInfo.m_startPage = vscp_readStringValue(jdmatrix["start-page"]);
            spdlog::debug("Parse-JSON: DM start page set to {0}.", m_dmInfo.m_startPage);
          }
          else {
            m_dmInfo.m_startPage = 0;
            spdlog::warn("Parse-JSON: DM start-page is of strange type (not number or string). Set to default (0).");
          }
        }
        else {
          m_dmInfo.m_startPage = 0;
          spdlog::warn("Parse-JSON: No DM start page defined. Set to default (0).");
        }

        // start offset
        if (jdmatrix.contains("start-offset")) {
          if (jdmatrix["start-offset"].is_number()) {
            m_dmInfo.m_startOffset = jdmatrix["start-offset"];
            spdlog::debug("Parse-JSON: DM start offset set to {0}.", m_dmInfo.m_startOffset);
          }
          else if (jdmatrix["start-offset"].is_string()) {
            m_dmInfo.m_startOffset = vscp_readStringValue(jdmatrix["start-offset"]);
            spdlog::debug("Parse-JSON: DM start offset set to {0}.", m_dmInfo.m_startOffset);
          }
          else {
            m_dmInfo.m_startOffset = 0;
            spdlog::warn("Parse-JSON: DM start-offset is of strange type (not number or string). Set to default (0).");
          }
        }
        else {
          m_dmInfo.m_startOffset = 0;
          spdlog::warn("Parse-JSON: No DM start offset defined. Set to default (0).");
        }

        // Row Count
        if (jdmatrix.contains("rowcount")) {
          if (jdmatrix["rowcount"].is_number()) {
            m_dmInfo.m_rowCount = jdmatrix["rowcount"];
            spdlog::debug("Parse-JSON: DM row count set to {0}.", m_dmInfo.m_rowCount);
          }
          else if (jdmatrix["rowcount"].is_string()) {
            m_dmInfo.m_rowCount = vscp_readStringValue(jdmatrix["rowcount"]);
            spdlog::debug("Parse-JSON: DM row count set to {0}.", m_dmInfo.m_rowCount);
          }
          else {
            m_dmInfo.m_rowCount = 0;
            spdlog::warn("DM row count is of strange type (not number or string). Set to default (0).");
          }
        }
        else {
          m_dmInfo.m_rowCount = 0;
          spdlog::warn("Parse-JSON: No DM row count defined. Set to default (0).");
        }

        // Row Size
        if (jdmatrix.contains("rowsize")) {
          if (jdmatrix["rowsize"].is_number()) {
            m_dmInfo.m_rowSize = jdmatrix["rowsize"];
            spdlog::debug("Parse-JSON: DM row size set to {0}.", m_dmInfo.m_rowSize);
          }
          else if (jdmatrix["rowsize"].is_string()) {
            m_dmInfo.m_rowSize = vscp_readStringValue(jdmatrix["rowsize"]);
            spdlog::debug("Parse-JSON: DM row size set to {0}.", m_dmInfo.m_rowSize);
          }
          else {
            m_dmInfo.m_rowSize = 8;
            spdlog::warn("DM row size is of strange type (not number or string). Set to default (8).");
          }
        }
        else {
          m_dmInfo.m_rowSize = 8;
          spdlog::warn("Parse-JSON: No DM row size defined. Set to default (8).");
        }

        if (jdmatrix.contains("action") && jdmatrix["action"].is_array()) {
          for (auto &action : jdmatrix["action"].items()) {
            // std::cout << "key: " << action.key() << ", value:" << action.value() << '\n';
            if (action.value().is_object()) {

              CMDF_Action *paction = new CMDF_Action();
              if (paction == nullptr) {
                spdlog::error("Parse-JSON: Failed to allocate memory for DM action item.");
                return VSCP_ERROR_PARSING;
              }

              m_dmInfo.m_list_action.push_back(paction);

              json jaction = action.value();

              if (jaction.contains("name") && jaction["name"].is_string()) {
                paction->m_name = jaction["name"];
                vscp_trim(paction->m_name);
                vscp_makeLower(paction->m_name);
                spdlog::debug("Parse-JSON: DM action name set to {0}.", paction->m_name);
              }
              else {
                spdlog::error("Parse-JSON: No DM action name defined.");
                return VSCP_ERROR_PARSING;
              }

              if (jaction.contains("code") && jaction["name"].is_number()) {
                paction->m_code = jaction["code"];
                spdlog::debug("Parse-JSON: DM action code set to {0}.", paction->m_name);
              }
              else if (jaction.contains("code") && jaction["name"].is_string()) {
                paction->m_code = vscp_readStringValue(jaction["code"]);
                spdlog::debug("Parse-JSON: DM action code set to {0}.", paction->m_code);
              }
              else {
                spdlog::error("Parse-JSON: No DM action code defined.");
                return VSCP_ERROR_PARSING;
              }

              if (getDescriptionList(jaction, paction->m_mapDescription) != VSCP_ERROR_SUCCESS) {
                spdlog::warn("Parse-JSON: Failed to get register bit description.");
              }

              if (getInfoUrlList(jaction, paction->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
                spdlog::warn("Parse-JSON: Failed to get register bit infourl.");
              }

              // Action parameters (For level I just one parameter)
              if (jaction.contains("param") && jaction["param"].is_array()) {

                for (auto &param : jaction["param"].items()) {

                  CMDF_ActionParameter *pparam = new CMDF_ActionParameter();
                  if (pparam == nullptr) {
                    spdlog::error("Parse-JSON: Failed to allocate memory for DM action parameter item.");
                    return VSCP_ERROR_PARSING;
                  }

                  paction->m_list_ActionParameter.push_back(pparam);

                  json jparam = param.value();

                  // std::cout << "key: " << param.key() << ", value:" << param.value() << '\n';
                  if (param.value().is_object()) {

                    // name
                    if (jparam.contains("name") && jparam["name"].is_string()) {
                      pparam->m_name = jparam["name"];
                      vscp_trim(pparam->m_name);
                      vscp_makeLower(pparam->m_name);
                      spdlog::debug("Parse-JSON: DM action parameter name set to {0}.", pparam->m_name);
                    }
                    else {
                      spdlog::warn("Parse-JSON: No DM action parameter name defined. Set to default (0).");
                    }

                    // offset
                    if (jparam.contains("offset") && jparam["offset"].is_string()) {
                      pparam->m_offset = vscp_readStringValue(jparam["offset"]);
                      spdlog::debug("Parse-JSON: DM action parameter offset set to {0}.", pparam->m_offset);
                    }
                    else if (jparam.contains("offset") && jparam["offset"].is_number()) {
                      pparam->m_offset = jparam["offset"];
                      spdlog::debug("Parse-JSON: DM action parameter offset set to {0}.", pparam->m_offset);
                    }
                    else {
                      pparam->m_offset = 0;
                      spdlog::warn("Parse-JSON: No DM action parameter offset defined. Set to default (0).");
                    }

                    // min
                    if (jparam.contains("min") && jparam["min"].is_string()) {
                      pparam->m_min = vscp_readStringValue(jparam["min"]);
                      spdlog::debug("Parse-JSON: DM action parameter min set to {0}.", pparam->m_min);
                    }
                    else if (jparam.contains("min") && jparam["min"].is_number()) {
                      pparam->m_min = jparam["min"];
                      spdlog::debug("Parse-JSON: DM action parameter min set to {0}.", pparam->m_min);
                    }
                    else {
                      pparam->m_min = 0;
                      spdlog::warn("Parse-JSON: No DM action parameter min defined. Set to default (0).");
                    }

                    // max
                    if (jparam.contains("max") && jparam["max"].is_string()) {
                      pparam->m_max = vscp_readStringValue(jparam["max"]);
                      spdlog::debug("Parse-JSON: DM action parameter max set to {0}.", pparam->m_max);
                    }
                    else if (jparam.contains("max") && jparam["max"].is_string()) {
                      pparam->m_max = jparam["max"];
                      spdlog::debug("Parse-JSON: DM action parameter max set to {0}.", pparam->m_max);
                    }
                    else {
                      pparam->m_max = 0;
                      spdlog::warn("Parse-JSON: No DM action parameter max defined. Set to default (0).");
                    }

                    if (getDescriptionList(jparam, pparam->m_mapDescription) != VSCP_ERROR_SUCCESS) {
                      spdlog::warn("Parse-JSON: Failed to get DM parameter bit description.");
                    }

                    if (getInfoUrlList(jparam, pparam->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
                      spdlog::warn("Parse-JSON: Failed to get DM parameter bit infourl.");
                    }

                    // Register valuelist
                    if (getValueList(jparam, pparam->m_list_value) != VSCP_ERROR_SUCCESS) {
                      spdlog::warn("Parse-JSON: Failed to get DM parameter valuelist.");
                    }

                    // Register bitlist
                    if (getBitList(jparam, pparam->m_list_bit) != VSCP_ERROR_SUCCESS) {
                      spdlog::warn("Parse-JSON: Failed to get DM parameter bitlist.");
                    }
                  }
                }
              }
            }
            else {
              spdlog::warn("Parse-JSON: No DM action parameters defined.");
            }
          }
        }
        else {
          spdlog::warn("Parse-JSON: Failed to read DM actions");
        }
      }
      else {
        spdlog::warn("Parse-JSON: Failed to read module decision matrix");
      }

      // ------------------------------------------------------------------------
      //                                  Events
      // ------------------------------------------------------------------------

      if (j["module"].contains("events") && j["module"]["events"].is_array()) {
        for (auto &ev : j["module"]["events"].items()) {
          // std::cout << "key: " << ev.key() << ", value:" << ev.value() << '\n';
          if (ev.value().is_object()) {

            CMDF_Event *pev = new CMDF_Event();
            if (pev == nullptr) {
              spdlog::error("Parse-JSON: Failed to allocate memory for remote variable item.");
              return VSCP_ERROR_PARSING;
            }

            m_list_event.push_back(pev);

            json jev = ev.value();

            // name
            if (jev.contains("name") && jev["name"].is_string()) {
              pev->m_name = jev["name"];
              vscp_trim(pev->m_name);
              vscp_makeLower(pev->m_name);
              spdlog::debug("Parse-JSON: Event name set to {0}.", pev->m_name);
            }

            // Class
            if (jev.contains("class") && jev["class"].is_string()) {
              if (jev["class"]) {
                pev->m_class = -1;
              }
              else {
                pev->m_class = vscp_readStringValue(jev["class"]);
              }
              spdlog::debug("Parse-JSON: Event class set to {0}.", pev->m_class);
            }
            else if (jev.contains("class") && jev["class"].is_number()) {
              pev->m_class = jev["class"];
              spdlog::debug("Parse-JSON: Event class set to {0}.", pev->m_class);
            }
            else {
              spdlog::error("Parse-JSON: No event class defined.");
              return VSCP_ERROR_PARSING;
            }

            // Type
            if (jev.contains("type") && jev["type"].is_string()) {
              if (jev["type"]) {
                pev->m_type = -1;
              }
              else {
                pev->m_type = vscp_readStringValue(jev["type"]);
              }
              spdlog::debug("Parse-JSON: Event type set to {0}.", pev->m_type);
            }
            else if (jev.contains("type") && jev["type"].is_number()) {
              pev->m_type = jev["type"];
              spdlog::debug("Parse-JSON: Event type set to {0}.", pev->m_type);
            }
            else {
              spdlog::error("Parse-JSON: No event type defined.");
              return VSCP_ERROR_PARSING;
            }

            // Direction ('in'/'out')
            if (jev.contains("direction") && jev["direction"].is_string()) {

              std::string str = jev["direction"];
              vscp_trim(str);
              vscp_makeLower(str);

              if (str == "in") {
                pev->m_direction = MDF_EVENT_DIR_IN;
                spdlog::debug("Parse-JSON: Event direction set to in.");
              }
              else if (str == "out") {
                pev->m_direction = MDF_EVENT_DIR_OUT;
                spdlog::debug("Parse-JSON: Event direction set to out.");
              }
              else {
                pev->m_direction = MDF_EVENT_DIR_OUT;
                spdlog::error("Parse-JSON: Event direction not 'in' or 'out'. Set to out.");
              }
            }
            else {
              pev->m_direction = MDF_EVENT_DIR_OUT;
              spdlog::warn("Parse-JSON: No event direction defined. Set to out.");
            }

            // Priority
            if (jev.contains("priority") && jev["priority"].is_string()) {

              std::string str = jev["priority"];
              vscp_trim(str);
              vscp_makeLower(str);

              if (str == "low") {
                pev->m_priority = VSCP_PRIORITY_LOW >> 5;
                spdlog::debug("Parse-JSON: Event priority set to low.");
              }
              else if (str == "normal") {
                pev->m_priority = VSCP_PRIORITY_NORMAL >> 5;
                spdlog::debug("Parse-JSON: Event priority set to normal.");
              }
              else if (str == "medium") {
                pev->m_priority = VSCP_PRIORITY_MEDIUM >> 5;
                spdlog::debug("Parse-JSON: Event priority set to medium.");
              }
              else if (str == "high") {
                pev->m_priority = VSCP_PRIORITY_HIGH >> 5;
                spdlog::debug("Parse-JSON: Event priority set to high.");
              }
              else {
                pev->m_priority = vscp_readStringValue(jev["priority"]) & 7;
                spdlog::debug("Parse-JSON: Event priority set to {0}.", pev->m_priority);
              }
            }
            else if (jev.contains("priority") && jev["priority"].is_number()) {
              pev->m_priority = (uint8_t) jev["priority"] & 7;
              spdlog::debug("Parse-JSON: Event priority set to {0}.", pev->m_priority);
            }
            else {
              pev->m_priority = 3;
              spdlog::warn("Parse-JSON: No event priority defined (set to 3.");
            }

            if (getDescriptionList(jev, pev->m_mapDescription) != VSCP_ERROR_SUCCESS) {
              spdlog::warn("Parse-JSON: Failed to get event description.");
            }

            if (getInfoUrlList(jev, pev->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
              spdlog::warn("Parse-JSON: Failed to get event infourl.");
            }

            if (jev.contains("data") && jev["data"].is_array()) {
              for (auto &data : jev["data"].items()) {
                // std::cout << "key: " << ev.key() << ", value:" << ev.value() << '\n';
                if (data.value().is_object()) {

                  CMDF_EventData *pevdata = new CMDF_EventData();
                  if (pevdata == nullptr) {
                    spdlog::error("Parse-JSON: Failed to allocate memory for event data item.");
                    return VSCP_ERROR_PARSING;
                  }

                  pev->m_list_eventdata.push_back(pevdata);
                  json jevdata = data.value();

                  // name
                  if (jevdata.contains("name") && jevdata["name"].is_string()) {
                    pevdata->m_name = jevdata["name"];
                    vscp_trim(pevdata->m_name);
                    vscp_makeLower(pevdata->m_name);
                    spdlog::debug("Parse-JSON: Event name set to {0}.", pevdata->m_name);
                  }

                  if (getDescriptionList(jevdata, pevdata->m_mapDescription) != VSCP_ERROR_SUCCESS) {
                    spdlog::warn("Parse-JSON: Failed to get event data description.");
                  }

                  if (getInfoUrlList(jevdata, pevdata->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
                    spdlog::warn("Parse-JSON: Failed to get event data infourl.");
                  }

                  // Register valuelist
                  if (getValueList(jevdata, pevdata->m_list_value) != VSCP_ERROR_SUCCESS) {
                    spdlog::warn("Parse-JSON: Failed to get event data valuelist.");
                  }

                  // Register bitlist
                  if (getBitList(jevdata, pevdata->m_list_bit) != VSCP_ERROR_SUCCESS) {
                    spdlog::warn("Parse-JSON: Failed to get event data bitlist.");
                  }

                } // data object
              }   // for
            }     // data array

          } // Event object
        }   // for
      }     // Events array
      else {
        spdlog::warn("Parse-JSON: Failed to read module events");
      }

      // ------------------------------------------------------------------------
      //                               Alarm bits
      // ------------------------------------------------------------------------

      if (j["module"].contains("alarms") && j["module"]["alarms"].is_array()) {
        for (auto &alarm : j["module"]["alarms"].items()) {
          // std::cout << "key: " << alarm.key() << ", value:" << alarm.value() << '\n';
          if (alarm.value().is_object()) {

            CMDF_Bit *palarm = new CMDF_Bit();
            if (palarm == nullptr) {
              spdlog::error("Parse-JSON: Failed to allocate memory for alarm item.");
              return VSCP_ERROR_PARSING;
            }

            m_list_alarm.push_back(palarm);

            json jalarm = alarm.value();

            // name
            if (jalarm.contains("name") && jalarm["name"].is_string()) {
              palarm->m_name = jalarm["name"];
              vscp_trim(palarm->m_name);
              vscp_makeLower(palarm->m_name);
              spdlog::debug("Parse-JSON: Alarm name set to {0}.", palarm->m_name);
            }

            // pos
            if (jalarm.contains("pos") && jalarm["pos"].is_string()) {
              palarm->m_pos = vscp_readStringValue(jalarm["pos"]);
              spdlog::debug("Parse-JSON: Alarm pos set to {0}.", palarm->m_pos);
            }
            else if (jalarm.contains("pos") && jalarm["pos"].is_number()) {
              palarm->m_pos = jalarm["pos"];
              spdlog::debug("Parse-JSON: Alarm pos set to {0}.", palarm->m_pos);
            }
            else {
              palarm->m_pos = 0;
              spdlog::warn("Parse-JSON: No alarm pos defined (set to 0.");
            }

            if (getDescriptionList(jalarm, palarm->m_mapDescription) != VSCP_ERROR_SUCCESS) {
              spdlog::warn("Parse-JSON: Failed to get event data description.");
            }

            if (getInfoUrlList(jalarm, palarm->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
              spdlog::warn("Parse-JSON: Failed to get event data infourl.");
            }
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

          if (jpic.contains("name") && jpic["name"].is_string()) {
            ppicture->m_strName = jpic["name"];
            spdlog::debug("Parse-JSON: Picture name: {0} ", jpic["name"]);
          }
          else {
            spdlog::warn("Parse-JSON: No picture name.");
          }

          if (jpic.contains("url") && jpic["url"].is_string()) {
            ppicture->m_strURL = jpic["url"];
            spdlog::debug("Parse-JSON: Picture URL: {0} ", jpic["url"]);
          }
          else {
            spdlog::warn("Parse-JSON: No picture url.");
          }

          if (jpic.contains("format") && jpic["format"].is_string()) {
            ppicture->m_strFormat = jpic["format"];
            spdlog::debug("Parse-JSON: Picture Format: {0} ", jpic["url"]);
          }
          else {
            spdlog::warn("Parse-JSON: No picture format.");
          }

          if (getDescriptionList(jpic, ppicture->m_mapDescription) != VSCP_ERROR_SUCCESS) {
            spdlog::warn("Parse-JSON: Failed to get picture description.");
          }

          if (getInfoUrlList(jpic, ppicture->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
            spdlog::warn("Parse-JSON: Failed to get picture infourl.");
          }
        }
      }

      // * * * Video * * *
      if (jsub.contains("video") && jsub["video"].is_array()) {
        for (auto &item : jsub["video"].items()) {

          CMDF_Video *pvideo = new CMDF_Video();
          if (pvideo == nullptr) {
            spdlog::error("Parse-JSON: Failed to allocate memory for video item.");
            return VSCP_ERROR_PARSING;
          }

          m_list_video.push_back(pvideo);

          json jvideo = item.value();

          if (jvideo.contains("name") && jvideo["name"].is_string()) {
            pvideo->m_strName = jvideo["name"];
            spdlog::debug("Parse-JSON: Video name: {0} ", jvideo["name"]);
          }
          else {
            spdlog::warn("Parse-JSON: No video name.");
          }

          if (jvideo.contains("url") && jvideo["url"].is_string()) {
            pvideo->m_strURL = jvideo["url"];
            spdlog::debug("Parse-JSON: Video URL: {0} ", jvideo["url"]);
          }
          else {
            spdlog::warn("Parse-JSON: No video url.");
          }

          if (jvideo.contains("format") && jvideo["format"].is_string()) {
            pvideo->m_strFormat = jvideo["format"];
            spdlog::debug("Parse-JSON: Video Format: {0} ", jvideo["url"]);
          }
          else {
            spdlog::warn("Parse-JSON: No video format.");
          }

          if (getDescriptionList(jvideo, pvideo->m_mapDescription) != VSCP_ERROR_SUCCESS) {
            spdlog::warn("Parse-JSON: Failed to get video description.");
          }

          if (getInfoUrlList(jvideo, pvideo->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
            spdlog::warn("Parse-JSON: Failed to get video infourl.");
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

          if (jfirmware.contains("name") && jfirmware["name"].is_string()) {
            pfirmware->m_strName = jfirmware["name"];
            spdlog::debug("Parse-JSON: Fiirmware name: {0} ", jfirmware["name"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware name.");
          }

          if (jfirmware.contains("url") && jfirmware["url"].is_string()) {
            pfirmware->m_strURL = jfirmware["url"];
            spdlog::debug("Parse-JSON: Firmware URL: {0} ", jfirmware["url"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware url.");
          }

          if (jfirmware.contains("target") && jfirmware["target"].is_string()) {
            pfirmware->m_strTarget = jfirmware["target"];
            spdlog::debug("Parse-JSON: Firmware target: {0} ", jfirmware["target"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware target.");
          }

          if (jfirmware.contains("targetcode") && jfirmware["targetcode"].is_number()) {
            pfirmware->m_targetCode = jfirmware["targetcode"];
            spdlog::debug("Parse-JSON: Firmware target: {0} ", jfirmware["targetcode"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware target code.");
          }

          // Target code string value
          if (jfirmware.contains("targetcode") && jfirmware["targetcode"].is_string()) {
            pfirmware->m_targetCode = vscp_readStringValue(jfirmware["targetcode"]);
            spdlog::debug("Parse-JSON: Firmware target: {0} ", jfirmware["targetcode"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware target code.");
          }

          if (jfirmware.contains("targetcode") && jfirmware["targetcode"].is_number()) {
            pfirmware->m_targetCode = jfirmware["targetcode"];
            spdlog::debug("Parse-JSON: Firmware target: {0} ", jfirmware["targetcode"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware target code.");
          }

          if (jfirmware.contains("format") && jfirmware["format"].is_string()) {
            pfirmware->m_strFormat = jfirmware["format"];
            spdlog::debug("Parse-JSON: Firmware format: {0} ", jfirmware["format"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware format.");
          }

          if (jfirmware.contains("date") && jfirmware["date"].is_string()) {
            pfirmware->m_strDate = jfirmware["date"];
            spdlog::debug("Parse-JSON: Firmware date: {0} ", jfirmware["date"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware date");
          }

          if (jfirmware.contains("version_major") && jfirmware["version_major"].is_number()) {
            pfirmware->m_version_major = jfirmware["version_major"];
            spdlog::debug("Parse-JSON: Firmware version major: {0} ", jfirmware["version_major"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version_major");
          }

          if (jfirmware.contains("version_major") && jfirmware["version_major"].is_string()) {
            pfirmware->m_version_major = vscp_readStringValue(jfirmware["version_major"]);
            spdlog::debug("Parse-JSON: Firmware version major: {0} ", jfirmware["version_major"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version_major");
          }

          if (jfirmware.contains("version_minor") && jfirmware["version_minor"].is_number()) {
            pfirmware->m_version_minor = jfirmware["version_minor"];
            spdlog::debug("Parse-JSON: Firmware version minor: {0} ", jfirmware["version_minor"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version_minor");
          }

          if (jfirmware.contains("version_minor") && jfirmware["version_minor"].is_string()) {
            pfirmware->m_version_minor = vscp_readStringValue(jfirmware["version_minor"]);
            spdlog::debug("Parse-JSON: Firmware version minor: {0} ", jfirmware["version_minor"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version_minor");
          }

          if (jfirmware.contains("version_patch") && jfirmware["version_patch"].is_number()) {
            pfirmware->m_version_patch = jfirmware["version_patch"];
            spdlog::debug("Parse-JSON: Firmware version patch: {0} ", jfirmware["version_patch"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version patch");
          }

          if (jfirmware.contains("version_patch") && jfirmware["version_patch"].is_string()) {
            pfirmware->m_version_patch = vscp_readStringValue(jfirmware["version_patch"]);
            spdlog::debug("Parse-JSON: Firmware version patch: {0} ", jfirmware["version_patch"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version patch");
          }

          if (jfirmware.contains("size") && jfirmware["size"].is_number()) {
            pfirmware->m_size = jfirmware["size"];
            spdlog::debug("Parse-JSON: Firmware version size: {0} ", jfirmware["size"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version size");
          }

          if (jfirmware.contains("size") && jfirmware["size"].is_string()) {
            pfirmware->m_size = vscp_readStringValue(jfirmware["size"]);
            spdlog::debug("Parse-JSON: Firmware version size: {0} ", jfirmware["size"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version size");
          }

          if (jfirmware.contains("md5sum") && jfirmware["md5sum"].is_string()) {
            pfirmware->m_strMd5 = jfirmware["md5sum"];
            spdlog::debug("Parse-JSON: Firmware version md5: {0} ", jfirmware["md5sum"]);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version md5sum");
          }

          if (getDescriptionList(jfirmware, pfirmware->m_mapDescription) != VSCP_ERROR_SUCCESS) {
            spdlog::warn("Parse-JSON: Failed to get event data description.");
          }

          if (getInfoUrlList(jfirmware, pfirmware->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
            spdlog::warn("Parse-JSON: Failed to get event data infourl.");
          }
        }
      }

      // * * * Driver * * *
      if (jsub.contains("driver") && jsub["driver"].is_array()) {
        for (auto &item : jsub["driver"].items()) {

          CMDF_Driver *pdriver = new CMDF_Driver();
          if (pdriver == nullptr) {
            spdlog::error("Parse-JSON: Failed to allocate memory for firmware item.");
            return VSCP_ERROR_PARSING;
          }

          m_list_driver.push_back(pdriver);

          json jdriver = item.value();

          if (jdriver.contains("name") && jdriver["name"].is_string()) {
            pdriver->m_strName = jdriver["name"];
            spdlog::debug("Parse-JSON: Driver name: {0} ", jdriver["name"]);
          }
          else {
            spdlog::warn("Parse-JSON: No driver name.");
          }

          if (jdriver.contains("url") && jdriver["url"].is_string()) {
            pdriver->m_strURL = jdriver["url"];
            spdlog::debug("Parse-JSON: Driver URL: {0} ", jdriver["url"]);
          }
          else {
            spdlog::warn("Parse-JSON: No driver url.");
          }

          if (jdriver.contains("name") && jdriver["name"].is_string()) {
            pdriver->m_strName = jdriver["name"];
            spdlog::debug("Parse-JSON: Driver name: {0} ", jdriver["name"]);
          }
          else {
            spdlog::warn("Parse-JSON: No driver name.");
          }

          // Type
          if (jdriver.contains("type") && jdriver["type"].is_string()) {
            pdriver->m_strType = jdriver["type"];
            spdlog::debug("Parse-JSON: driver type: {0} ", jdriver["type"]);
          }
          else {
            spdlog::warn("Parse-JSON: No driver type.");
          }

          // OS
          if (jdriver.contains("os") && jdriver["os"].is_string()) {
            pdriver->m_strOS = jdriver["os"];
            spdlog::debug("Parse-JSON: driver OS: {0} ", jdriver["os"]);
          }
          else {
            spdlog::warn("Parse-JSON: No driver OS.");
          }

          // OS version
          if (jdriver.contains("osver") && jdriver["osver"].is_string()) {
            pdriver->m_strOSVer = jdriver["osver"];
            spdlog::debug("Parse-JSON: driver OS version: {0} ", jdriver["osver"]);
          }
          else {
            spdlog::warn("Parse-JSON: No driver OS version.");
          }

          if (jdriver.contains("date") && jdriver["date"].is_string()) {
            pdriver->m_strDate = jdriver["date"];
            spdlog::debug("Parse-JSON: Driver date: {0} ", jdriver["date"]);
          }
          else {
            spdlog::warn("Parse-JSON: No driver date");
          }

          if (jdriver.contains("version_major") && jdriver["version_major"].is_number()) {
            pdriver->m_version_major = jdriver["version_major"];
            spdlog::debug("Parse-JSON: Driver version major: {0} ", jdriver["version_major"]);
          }
          else {
            spdlog::warn("Parse-JSON: No driver version_major");
          }

          if (jdriver.contains("version_major") && jdriver["version_major"].is_string()) {
            pdriver->m_version_major = vscp_readStringValue(jdriver["version_major"]);
            spdlog::debug("Parse-JSON: Driver version major: {0} ", jdriver["version_major"]);
          }
          else {
            spdlog::warn("Parse-JSON: No driver version_major");
          }

          if (jdriver.contains("version_minor") && jdriver["version_minor"].is_number()) {
            pdriver->m_version_minor = jdriver["version_minor"];
            spdlog::debug("Parse-JSON: Driver version minor: {0} ", jdriver["version_minor"]);
          }
          else {
            spdlog::warn("Parse-JSON: No driver version_minor");
          }

          if (jdriver.contains("version_minor") && jdriver["version_minor"].is_string()) {
            pdriver->m_version_minor = vscp_readStringValue(jdriver["version_minor"]);
            spdlog::debug("Parse-JSON: Driver version minor: {0} ", jdriver["version_minor"]);
          }
          else {
            spdlog::warn("Parse-JSON: No driver version_minor");
          }

          if (jdriver.contains("version_patch") && jdriver["version_patch"].is_number()) {
            pdriver->m_version_patch = jdriver["version_patch"];
            spdlog::debug("Parse-JSON: Driver version patch: {0} ", jdriver["version_patch"]);
          }
          else {
            spdlog::warn("Parse-JSON: No driver version patch");
          }

          if (jdriver.contains("version_patch") && jdriver["version_patch"].is_string()) {
            pdriver->m_version_patch = vscp_readStringValue(jdriver["version_patch"]);
            spdlog::debug("Parse-JSON: Driver version patch: {0} ", jdriver["version_patch"]);
          }
          else {
            spdlog::warn("Parse-JSON: No driver version patch");
          }

          if (jdriver.contains("md5sum") && jdriver["md5sum"].is_string()) {
            pdriver->m_strMd5 = jdriver["md5sum"];
            spdlog::debug("Parse-JSON: Driver version md5: {0} ", jdriver["md5sum"]);
          }
          else {
            spdlog::warn("Parse-JSON: No driver version md5sum");
          }

          if (getDescriptionList(jdriver, pdriver->m_mapDescription) != VSCP_ERROR_SUCCESS) {
            spdlog::warn("Parse-JSON: Failed to get event data description.");
          }

          if (getInfoUrlList(jdriver, pdriver->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
            spdlog::warn("Parse-JSON: Failed to get event data infourl.");
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

          if (jmanual.contains("name") && jmanual["name"].is_string()) {
            pmanual->m_strName = jmanual["name"];
            spdlog::debug("Parse-JSON: Manual name: {0} ", jmanual["name"]);
          }
          else {
            spdlog::warn("Parse-JSON: No manual name.");
          }

          if (jmanual.contains("url") && jmanual["url"].is_string()) {
            pmanual->m_strURL = jmanual["url"];
            spdlog::debug("Parse-JSON: Manual URL: {0} ", jmanual["url"]);
          }
          else {
            spdlog::warn("Parse-JSON: No manual url.");
          }

          if (jmanual.contains("format") && jmanual["format"].is_string()) {
            pmanual->m_strFormat = jmanual["format"];
            spdlog::debug("Parse-JSON: Manual Format: {0} ", jmanual["format"]);
          }
          else {
            spdlog::warn("Parse-JSON: No manual format.");
          }

          if (jmanual.contains("lang") && jmanual["lang"].is_string()) {
            pmanual->m_strLanguage = jmanual["lang"];
            spdlog::debug("Parse-JSON: Manual language: {0} ", jmanual["lang"]);
          }
          else {
            spdlog::warn("Parse-JSON: No manual language.");
          }

          if (getDescriptionList(jmanual, pmanual->m_mapDescription) != VSCP_ERROR_SUCCESS) {
            spdlog::warn("Parse-JSON: Failed to get event data description.");
          }

          if (getInfoUrlList(jmanual, pmanual->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
            spdlog::warn("Parse-JSON: Failed to get event data infourl.");
          }
        }
      }

      // * * * setup * * *
      if (jsub.contains("setup") && jsub["setup"].is_array()) {
        for (auto &item : jsub["setup"].items()) {

          CMDF_Setup *psetup = new CMDF_Setup();
          if (psetup == nullptr) {
            spdlog::error("Parse-JSON: Failed to allocate memory for setup item.");
            return VSCP_ERROR_PARSING;
          }

          m_list_setup.push_back(psetup);

          json jsetup = item.value();

          if (jsetup.contains("name") && jsetup["name"].is_string()) {
            psetup->m_strName = jsetup["name"];
            spdlog::debug("Parse-JSON: Setup name: {0} ", jsetup["name"]);
          }
          else {
            spdlog::warn("Parse-JSON: No setup name.");
          }

          if (jsetup.contains("url") && jsetup["url"].is_string()) {
            psetup->m_strURL = jsetup["url"];
            spdlog::debug("Parse-JSON: Setup URL: {0} ", jsetup["url"]);
          }
          else {
            spdlog::warn("Parse-JSON: No setup url.");
          }

          if (jsetup.contains("format") && jsetup["format"].is_string()) {
            psetup->m_strFormat = jsetup["format"];
            spdlog::debug("Parse-JSON: Setup Format: {0} ", jsetup["url"]);
          }
          else {
            spdlog::warn("Parse-JSON: No setup format.");
          }

          if (getDescriptionList(jsetup, psetup->m_mapDescription) != VSCP_ERROR_SUCCESS) {
            spdlog::warn("Parse-JSON: Failed to get setup description.");
          }

          if (getInfoUrlList(jsetup, psetup->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
            spdlog::warn("Parse-JSON: Failed to get setup infourl.");
          }
        }
      }
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

  spdlog::debug("Parse-XML: Parsing MDF file {}", path);

  // Check if file exists
  if (!vscp_fileExists(path)) {
    spdlog::debug("Parse-XML: MDF file {} does not exist", path);
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
    spdlog::error("Parse-XML: Failed to open file {}", path);
    return VSCP_ERROR_NOT_OPEN;
  }

  size_t pos;
  std::string str;

  while (std::getline(ifs, str)) {
    // if (!ifs.gcount()) {
    //   spdlog::error("Parse-XML: Failed to read file {}", path);
    //   ifs.close();
    //   return false;
    // }
    vscp_trim(str);
    if ((pos = str.find('{')) != std::string::npos) {
      spdlog::debug("Parse-XML: MDF file format is JSON");
      ifs.close();
      rv = parseMDF_JSON(path);
      break;
    }
    else if ((pos = str.find('<')) != std::string::npos) {
      spdlog::debug("Parse-XML: MDF file format is XML");
      rv = parseMDF_XML(ifs);
      ifs.close();
      break;
    }
    else {
      rv = VSCP_ERROR_INVALID_SYNTAX;
      spdlog::error("Parse-XML: MDF file format not supported");
    }
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getNumberOfRegisters
//

size_t
CMDF::getRegisterCount(uint32_t page)
{
  size_t nregisters = 0;

  std::deque<CMDF_Register *>::iterator iterValue;
  for (iterValue = m_list_register.begin(); iterValue != m_list_register.end(); ++iterValue) {
    CMDF_Register *pRecordValue = *iterValue;
    if (nullptr != pRecordValue) {
      if (page == pRecordValue->m_page)
        nregisters++;
    }
  }

  return nregisters;
};

///////////////////////////////////////////////////////////////////////////////
//  getPages
//

uint32_t
CMDF::getPages(std::set<uint16_t> &pages)
{
  std::deque<CMDF_Register *>::iterator iterValue;
  for (iterValue = m_list_register.begin(); iterValue != m_list_register.end(); ++iterValue) {

    CMDF_Register *pRecordValue = *iterValue;
    if (nullptr != pRecordValue) {
      // Add page if not already in set
      if (pages.end() == pages.find(pRecordValue->m_page)) {
        pages.insert(pRecordValue->m_page);
      }
    }
  }

  return (uint32_t) pages.size();
};

///////////////////////////////////////////////////////////////////////////////
//  getRegister
//

CMDF_Register *
CMDF::getRegister(uint32_t reg, uint16_t page)
{
  std::deque<CMDF_Register *>::iterator iter;
  for (iter = m_list_register.begin(); iter != m_list_register.end(); ++iter) {
    CMDF_Register *preg = *iter;
    if ((reg == preg->m_offset) && (page == preg->m_page)) {
      return preg;
    }
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//  getRemoteVariable
//

CMDF_RemoteVariable *
CMDF::getRemoteVariable(std::string name)
{
  vscp_trim(name);
  vscp_makeLower(name);

  std::deque<CMDF_RemoteVariable *>::iterator iter;
  for (iter = m_list_remotevar.begin(); iter != m_list_remotevar.end(); ++iter) {
    CMDF_RemoteVariable *prvar = *iter;
    std::string rname          = prvar->m_name;
    vscp_trim(rname);
    vscp_makeLower(rname);
    if (rname == name) {
      return prvar;
    }
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// getRegisterList
//

void
CMDF::getRegisterMap(uint16_t page, std::map<uint32_t, CMDF_Register *> &mapRegs)
{
  //std::map<uint32_t, CMDF_Register> mapRegs;
  std::set<uint32_t> regset;

  // Make a sorted set of registers
  for (auto reg : m_list_register) {
    if (page == reg->m_page) {
      regset.insert(reg->m_offset);
    }
  }

  for (auto reg : regset) {
    mapRegs[reg] = getRegister(reg, page);
  }

  return;
}

///////////////////////////////////////////////////////////////////////////////
// getRegisterList
//

std::string&
CMDF::format(std::string& docs)
{
  int idx = 0;

  // If first character is a # the string is coded as a
  // markddown document. If not it is coded as a HTML document.

  vscp_trim(docs);

  if ('#' == docs.at(0)) {
    // Markdown

    // Replace escapes
    // "\n" -> \n    
    do {
      idx = docs.find("\\n", idx);
      if (idx == std::string::npos) break;
      docs.replace(idx, 2, "\n");
    } while (true);

    std::stringstream markdownInput(docs);
    std::shared_ptr<maddy::Parser> parser = std::make_shared<maddy::Parser>();
    docs = parser->Parse(markdownInput);
  }
  else {
    // HTML

    // Replace escapes
    // "/n" -> <br>    
    do {
      idx = docs.find("\\n", idx);
      if (idx == std::string::npos) break;
      docs.replace(idx, 2, "<br>");
    } while (true);
  }
  return docs;
}

  