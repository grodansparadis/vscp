// FILE: mdf.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project
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

#include <canal.h>
#include <mdf.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <vscp.h>
#include <vscphelper.h>

#include <deque>
#include <fstream>
#include <iostream>
#include <memory>
#include <cstring>
#include <set>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>

#include <expat.h>
#ifdef VSCP_HAVE_MADDY
#include <maddy/parser.h> // Markdown -> HTML
#endif
#include <mustache.hpp>
#include <nlohmann/json.hpp> 

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

#include <curl/curl.h>

// For older cURL versions you will also need
// #include <curl/types.h>
// #include <curl/easy.h>

// https://github.com/nlohmann/json
using json = nlohmann::json;

using namespace kainjow::mustache;

// Register/remote variable default values are stored as written in the MDF.
// Only the "not set" keyword is normalized (any case) to "UNDEF", which is
// what CMDF_Register::getDefault() and applications compare against.
// Access rights as written in an MDF. The specification uses "r", "w" and
// "rw"; other spellings seen in MDF files ("readwrite", "read only", ...)
// are understood too. Anything unknown gives read/write (the specification
// default) rather than no access at all.
static mdf_access_mode
mdf_parseAccess(std::string str)
{
  vscp_trim(str);
  vscp_makeLower(str);
  std::string s;
  for (char c : str) {
    if (!isspace((unsigned char) c) && ('/' != c) && ('-' != c) && ('_' != c)) {
      s += c;
    }
  }
  if (s.empty()) {
    return MDF_REG_ACCESS_READ_WRITE;
  }
  if (("r" == s) || ("ro" == s) || ("read" == s) || ("readonly" == s)) {
    return MDF_REG_ACCESS_READ_ONLY;
  }
  if (("w" == s) || ("wo" == s) || ("write" == s) || ("writeonly" == s)) {
    return MDF_REG_ACCESS_WRITE_ONLY;
  }
  if (("rw" == s) || ("wr" == s) || ("readwrite" == s) || ("writeread" == s) || ("rws" == s)) {
    return MDF_REG_ACCESS_READ_WRITE;
  }
  spdlog::warn("MDF: Unknown access '{}', read/write used.", str);
  return MDF_REG_ACCESS_READ_WRITE;
}

static std::string
mdf_normalizeDefault(std::string str)
{
  vscp_trim(str);
  if (0 == strcasecmp(str.c_str(), "undef")) {
    return "UNDEF";
  }
  return str;
}

// The module level in an MDF file is 1 (Level I) or 2 (Level II); internally
// it is VSCP_LEVEL1 (0) / VSCP_LEVEL2 (1). Anything else is taken as Level I.
static int
mdf_levelFromFile(int64_t level)
{
  if (2 == level) {
    return VSCP_LEVEL2;
  }
  if (1 != level) {
    spdlog::warn("MDF: Module level {} is not 1 or 2. Level I is used.", level);
  }
  return VSCP_LEVEL1;
}

// The level written to an MDF file
static int
mdf_levelToFile(int level)
{
  return (VSCP_LEVEL2 == level) ? 2 : 1;
}

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

CMDF_Object::CMDF_Object(mdf_record_type type)
{
  m_type = type;
}

CMDF_Object::~CMDF_Object() {}

///////////////////////////////////////////////////////////////////////////////
// getObjectTypeString
//

std::string
CMDF_Object::getObjectTypeString()
{
  switch (m_type) {
    case mdf_type_mdf:
      return std::string("MDF top level");
    default:
      return std::string("Unknown");
  }
}

///////////////////////////////////////////////////////////////////////////////
// getMdfObjectType
//

mdf_record_type
CMDF_Object::getMdfObjectType()
{
  return m_type;
};

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Value::CMDF_Value()
  : CMDF_Object(mdf_type_value)
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
  : CMDF_Object(mdf_type_remotevar)
{
  m_strDefault = "UNDEF";
  m_type       = remote_variable_type_unknown;
  m_page       = 0;
  m_offset     = 0;
  m_bitpos     = 0;
  m_size       = 0;
  m_access     = MDF_REG_ACCESS_READ_WRITE;

  m_bgcolor = 0xffffff;
  m_fgcolor = 0x000000;
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
  m_strDefault = "UNDEF";
  m_strMin.clear();
  m_strMax.clear();
  m_type       = remote_variable_type_unknown;
  m_page       = 0;
  m_offset     = 0;
  m_bitpos     = 0;
  m_size       = 0;
  m_access     = MDF_REG_ACCESS_READ_WRITE;

  m_bgcolor = 0xffffff;
  m_fgcolor = 0x000000;

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

  // Clear up bit list (the bits were never freed)
  for (auto pbit : m_list_bit) {
    delete pbit;
  }
  m_list_bit.clear();

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
// getAccessStr
//

std::string
CMDF_RemoteVariable::getAccessStr(void)
{
  std::string str = "";
  // MDF_REG_ACCESS_READ_ONLY = 1, MDF_REG_ACCESS_WRITE_ONLY = 2, READ_WRITE = 3
  if (MDF_REG_ACCESS_READ_ONLY & getAccess()) {
    str += "r";
  }
  if (MDF_REG_ACCESS_WRITE_ONLY & getAccess()) {
    str += "w";
  }
  return str;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Bit::CMDF_Bit()
  : CMDF_Object(mdf_type_bit)
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
  if (width > 7) {
    width = 8;
  }

  // Save calculated
  m_width = width;

  m_mask = 0;
  for (int k = m_pos; k < (m_pos + m_width); k++) {
    m_mask |= (1 << k);
  }

  // m_min/m_max/m_default are values of the bit field (not shifted to its
  // position) and are kept as they are given
}

///////////////////////////////////////////////////////////////////////////////
//  setPos
//

void
CMDF_Bit::setPos(uint8_t pos)
{
  m_pos = (pos & 7);

  // Make sure width get masked
  setWidth(m_width);
}

///////////////////////////////////////////////////////////////////////////////
//  setMin
//

void
CMDF_Bit::setMin(uint8_t min)
{
  m_min = min;
}

///////////////////////////////////////////////////////////////////////////////
//  setMax
//

void
CMDF_Bit::setMax(uint8_t max)
{
  m_max = max;
}

///////////////////////////////////////////////////////////////////////////////
// getAccessStr
//

std::string
CMDF_Bit::getAccessStr(void)
{
  std::string str = "";
  // MDF_REG_ACCESS_READ_ONLY = 1, MDF_REG_ACCESS_WRITE_ONLY = 2, READ_WRITE = 3
  if (MDF_REG_ACCESS_READ_ONLY & getAccess()) {
    str += "r";
  }
  if (MDF_REG_ACCESS_WRITE_ONLY & getAccess()) {
    str += "w";
  }
  return str;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Register::CMDF_Register()
  : CMDF_Object(mdf_type_register)
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
  // m_size       = 1;  // removed used span

  m_bgcolor = 0xffffff;
  m_fgcolor = 0x000000;

  // All null means not used
  m_fgeven = 0x000000;
  m_fgodd  = 0x000000;
  m_bgeven = 0x000000;
  m_bgodd  = 0x000000;

  m_fgcolorMdf = m_fgcolor;
  m_bgcolorMdf = m_bgcolor;
  m_spanIndex  = -1;
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
  // m_size       = 1; // Removed used span instead

  m_bgcolor = 0xffffff;
  m_fgcolor = 0x000000;

  m_fgeven = 0x000000;
  m_fgodd  = 0x000000;
  m_bgeven = 0x000000;
  m_bgodd  = 0x000000;

  m_fgcolorMdf = m_fgcolor;
  m_bgcolorMdf = m_bgcolor;
  m_spanIndex  = -1;
}

///////////////////////////////////////////////////////////////////////////////
//  getDefault
//

bool
CMDF_Register::getDefault(uint8_t &default_value)
{
  if ((0 == strcasecmp(m_strDefault.c_str(), "undef")) || m_strDefault.empty()) {
    return false;
  }
  // vscp_readStringValue handles decimal, 0x, 0o and 0b (strtol does not do 0b)
  default_value = (uint8_t) vscp_readStringValue(m_strDefault);
  return true;
};

///////////////////////////////////////////////////////////////////////////////
//  Assignment
//

CMDF_Register &
CMDF_Register::operator=(const CMDF_Register &other)
{
  // Check for self-assignment!
  if (this == &other) { // Same object?
    return *this;       // Yes, so skip assignment, and just return *this.
  }

  m_name           = other.m_name;
  m_mapDescription = other.m_mapDescription;
  m_mapInfoURL     = other.m_mapInfoURL;

  m_page   = other.m_page;
  m_offset = other.m_offset;
  m_span   = other.m_span;
  m_width  = other.m_width;

  m_type = other.m_type;
  // m_size = other.m_size;  // Removed used span instead

  m_min = other.m_min;
  m_max = other.m_max;

  m_strDefault = other.m_strDefault;

  m_access = other.m_access;

  m_fgcolor = other.m_fgcolor;
  m_bgcolor = other.m_bgcolor;

  m_fgeven = other.m_fgeven;
  m_fgodd  = other.m_fgodd;
  m_bgeven = other.m_bgeven;
  m_bgodd  = other.m_bgodd;

  m_fgcolorMdf = other.m_fgcolorMdf;
  m_bgcolorMdf = other.m_bgcolorMdf;
  m_spanIndex  = other.m_spanIndex;

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

  // Deep copy of the bit definitions (and their value lists) so copies of
  // DMATRIX1/BLOCK registers show the same bits. Each register owns its own
  // objects so there is no double free.
  for (auto pbitOther : other.m_list_bit) {
    if (nullptr == pbitOther) {
      continue;
    }
    CMDF_Bit *pbit = new CMDF_Bit(*pbitOther); // Copies scalars and maps
    pbit->getListValues()->clear();            // Value pointers belong to 'other'
    for (auto pvalueOther : *pbitOther->getListValues()) {
      if (nullptr != pvalueOther) {
        pbit->getListValues()->push_back(new CMDF_Value(*pvalueOther));
      }
    }
    m_list_bit.push_back(pbit);
  }

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

  // Deep copy of the value list
  for (auto pvalueOther : other.m_list_value) {
    if (nullptr != pvalueOther) {
      m_list_value.push_back(new CMDF_Value(*pvalueOther));
    }
  }

  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// getAccessStr
//

std::string
CMDF_Register::getAccessStr(void)
{
  std::string str = "";
  // MDF_REG_ACCESS_READ_ONLY = 1, MDF_REG_ACCESS_WRITE_ONLY = 2, READ_WRITE = 3
  if (MDF_REG_ACCESS_READ_ONLY & getAccess()) {
    str += "r";
  }
  if (MDF_REG_ACCESS_WRITE_ONLY & getAccess()) {
    str += "w";
  }
  return str;
}

///////////////////////////////////////////////////////////////////////////////
// getTypeStr
//

std::string
CMDF_Register::getTypeStr(void)
{
  std::string str;

  switch (m_type) {
    case MDF_REG_TYPE_STANDARD:
      str = "std";
      break;
    case MDF_REG_TYPE_DMATRIX1:
      str = "dmatrix1";
      break;
    case MDF_REG_TYPE_BLOCK:
      str = "block";
      break;

    default:
      str = "invalid";
      break;
  }

  return str;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_ActionParameter::CMDF_ActionParameter()
  : CMDF_Object(mdf_type_action_param)
{
  m_name.clear();
  m_offset = 0;
  m_min    = 0;
  m_max    = 255;
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
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Action::CMDF_Action()
  : CMDF_Object(mdf_type_action)
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
//  getActionParam
//

CMDF_ActionParameter *
CMDF_Action::getActionParam(uint16_t offset)
{
  std::deque<CMDF_ActionParameter *>::iterator it;
  for (it = m_list_ActionParameter.begin(); it != m_list_ActionParameter.end(); ++it) {
    CMDF_ActionParameter *pActionParam = *it;
    printf("offset=%d\n", pActionParam->getOffset());
    if ((nullptr != pActionParam) && (offset == pActionParam->getOffset())) {
      return pActionParam;
    }
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//  addActionParam
//

bool
CMDF_Action::addActionParam(CMDF_ActionParameter *pactionparam)
{
  // Check pointer
  if (nullptr == pactionparam) {
    return false;
  }

  // Offset must be unique
  if (nullptr != getActionParam(pactionparam->getOffset())) {
    return false;
  }

  m_list_ActionParameter.push_back(pactionparam);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
//  deleteActionParam
//

bool
CMDF_Action::deleteActionParam(CMDF_ActionParameter *pactionparam)
{
  if (nullptr == pactionparam) {
    return false;
  }

  std::deque<CMDF_ActionParameter *>::iterator it;
  for (auto it = m_list_ActionParameter.begin(); it != m_list_ActionParameter.end(); ++it) {
    if (pactionparam->getOffset() == (*it)->getOffset()) {
      m_list_ActionParameter.erase(it);
      delete pactionparam;
      return true;
    }
  }

  return false;
}

bool
CMDF_Action::deleteActionParam(uint8_t offset)
{
  for (auto it = m_list_ActionParameter.begin(); it != m_list_ActionParameter.end(); ++it) {
    if (offset == (*it)->getOffset()) {
      m_list_ActionParameter.erase(it);
      delete *it;
      return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_DecisionMatrix::CMDF_DecisionMatrix()
  : CMDF_Object(mdf_type_decision_matrix)
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

  // Cleanup action list
  std::deque<CMDF_Action *>::iterator iterAction;
  for (iterAction = m_list_action.begin(); iterAction != m_list_action.end(); ++iterAction) {
    CMDF_Action *pRecordAction = *iterAction;
    if (nullptr != pRecordAction) {
      delete pRecordAction;
      pRecordAction = nullptr;
    }
  }

  m_list_action.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  getAction
//

CMDF_Action *
CMDF_DecisionMatrix::getAction(uint16_t code)
{
  std::deque<CMDF_Action *>::iterator it;
  for (it = m_list_action.begin(); it != m_list_action.end(); ++it) {
    CMDF_Action *pAction = *it;
    printf("code=%d\n", pAction->getCode());
    if ((nullptr != pAction) && (code == pAction->getCode())) {
      return pAction;
    }
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//  addAction
//

bool
CMDF_DecisionMatrix::addAction(CMDF_Action *paction)
{
  // Check pointer
  if (nullptr == paction) {
    return false;
  }

  // Offset must be unique
  if (nullptr != getAction(paction->getCode())) {
    return false;
  }

  m_list_action.push_back(paction);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
//  deleteAction
//

bool
CMDF_DecisionMatrix::deleteAction(CMDF_Action *paction)
{
  if (nullptr == paction) {
    return false;
  }

  std::deque<CMDF_ActionParameter *>::iterator it;
  for (auto it = m_list_action.begin(); it != m_list_action.end(); ++it) {
    if (paction->getCode() == (*it)->getCode()) {
      m_list_action.erase(it);
      delete paction;
      return true;
    }
  }

  return false;
}

bool
CMDF_DecisionMatrix::deleteAction(uint8_t code)
{
  for (auto it = m_list_action.begin(); it != m_list_action.end(); ++it) {
    if (code == (*it)->getCode()) {
      m_list_action.erase(it);
      delete *it;
      return true;
    }
  }

  return false;
}

//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_EventData::CMDF_EventData()
  : CMDF_Object(mdf_type_event_data)
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
  : CMDF_Object(mdf_type_event)
{
  m_name.clear();
  m_class     = 0;
  m_type      = 0;
  m_priority  = 3; // Specification default
  m_direction = MDF_EVENT_DIR_OUT;
}

CMDF_Event::~CMDF_Event()
{
  m_name.clear();
  m_class     = 0;
  m_type      = 0;
  m_priority  = 3; // Specification default
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
//  getEventData
//

CMDF_EventData *
CMDF_Event::getEventData(uint8_t offset)
{
  for (auto it = m_list_eventdata.cbegin(); it != m_list_eventdata.cend(); ++it) {
    if (offset == (*it)->getOffset()) {
      return *it;
    }
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//  addEventData
//

bool
CMDF_Event::addEventData(CMDF_EventData *pEventData)
{
  if (nullptr != pEventData) {
    m_list_eventdata.push_back(pEventData);
    return true;
  }
  else {
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////////
//  deleteEventData
//

bool
CMDF_Event::deleteEventData(CMDF_EventData *pEventData)
{
  // Check pointer
  if (nullptr == pEventData) {
    return false;
  }

  for (auto it = m_list_eventdata.cbegin(); it != m_list_eventdata.cend(); ++it) {
    if (pEventData == *it) {
      m_list_eventdata.erase(it);
      delete pEventData;
      return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
//  isEventDataOffsetUnique
//

bool
CMDF_Event::isEventDataOffsetUnique(uint8_t offset)
{
  if (nullptr == getEventData(offset)) {
    return true;
  }

  return false;
}

bool
CMDF_Event::isEventDataOffsetUnique(CMDF_EventData *pEventData)
{
  // Check pointer
  if (nullptr == pEventData) {
    return false;
  }

  for (auto it = m_list_eventdata.cbegin(); it != m_list_eventdata.cend(); ++it) {
    // We don't check ourself
    if (pEventData == *it) {
      continue;
    }

    if (pEventData->getOffset() == (*it)->getOffset()) {
      return false;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Item::CMDF_Item()
  : CMDF_Object(mdf_type_value_item)
{
  m_value.clear();
}

CMDF_Item::~CMDF_Item()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_BootLoaderInfo::CMDF_BootLoaderInfo()
  : CMDF_Object(mdf_type_bootloader)
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
  : CMDF_Object(mdf_type_address)
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
  : CMDF_Object(mdf_type_manufacturer)
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

  // Cleanup social list
  std::deque<CMDF_Item *>::iterator itersocial;
  for (itersocial = m_list_Social.begin(); itersocial != m_list_Social.end(); ++itersocial) {
    CMDF_Item *pRecordSocial = *itersocial;
    if (nullptr != pRecordSocial) {
      delete pRecordSocial;
      pRecordSocial = nullptr;
    }
  }
  m_list_Social.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Picture::CMDF_Picture()
  : CMDF_Object(mdf_type_picture)
{
  clearStorage();
  m_strName = ""; // default name
}

CMDF_Picture::~CMDF_Picture() {}

///////////////////////////////////////////////////////////////////////////////
//  clearStorage
//

void
CMDF_Picture::clearStorage(void)
{
  m_strVersion.clear();
  m_strName.clear();
  m_strURL.clear();
  m_strFormat.clear();
  m_strDate.clear();
  m_mapDescription.clear();
  m_mapInfoURL.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Video::CMDF_Video()
  : CMDF_Object(mdf_type_video)
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
  m_strVersion.clear();
  m_strName.clear();
  m_strURL.clear();
  m_strFormat.clear();
  m_strDate.clear();
  m_mapDescription.clear();
  m_mapInfoURL.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Firmware::CMDF_Firmware()
  : CMDF_Object(mdf_type_firmware)
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
  m_strFormat = "INTELHEX";
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
//  setVersion
//

void
CMDF_Firmware::setVersion(const std::string &ver)
{
  std::deque<std::string> vector;
  vscp_split(vector, ver, ".");

  if (vector.size() >= 1) {
    m_version_major = vscp_readStringValue(vector[0]);
  }

  if (vector.size() >= 2) {
    m_version_minor = vscp_readStringValue(vector[1]);
  }

  if (vector.size() >= 3) {
    m_version_patch = vscp_readStringValue(vector[2]);
  }
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Driver::CMDF_Driver()
  : CMDF_Object(mdf_type_driver)
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
  m_strType.clear();
  m_strArchitecture.clear();

  m_version_major = 0;
  m_version_minor = 0;
  m_version_patch = 0;

  m_mapDescription.clear();
  m_mapInfoURL.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  setVersion
//

void
CMDF_Driver::setVersion(const std::string &ver)
{
  std::deque<std::string> vector;
  vscp_split(vector, ver, ".");

  if (vector.size() >= 1) {
    m_version_major = vscp_readStringValue(vector[0]);
  }

  if (vector.size() >= 2) {
    m_version_minor = vscp_readStringValue(vector[1]);
  }

  if (vector.size() >= 3) {
    m_version_patch = vscp_readStringValue(vector[2]);
  }
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Manual::CMDF_Manual()
  : CMDF_Object(mdf_type_manual)
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
  m_strVersion.clear();
  m_strName.clear();
  m_strURL.clear();
  m_strFormat.clear();
  m_strDate.clear();
  m_mapDescription.clear();
  m_mapInfoURL.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF_Setup::CMDF_Setup()
  : CMDF_Object(mdf_type_setup)
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
  m_strDate.clear();
  m_mapDescription.clear();
  m_mapInfoURL.clear();
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//  Constructor/Destructor
//

CMDF::CMDF()
  : CMDF_Object(mdf_type_mdf)
{
  m_strLocale         = "en";
  m_vscpLevel         = VSCP_LEVEL1;
  m_module_bufferSize = 8; // Specification default
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

  m_vscpLevel = VSCP_LEVEL1;
  m_name      = "";
  m_copyright.clear();
  m_redirectUrl.clear();
  m_manufacturer.clearStorage();
  m_strModule_Model.clear();
  m_strModule_Version.clear();
  m_mapDescription.clear();
  m_mapInfoURL.clear();
  m_strModule_changeDate.clear();
  m_module_bufferSize = 8; // Specification default
  m_strURL.clear();
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

// Limits for downloaded MDF files
#define MDF_DOWNLOAD_MAX_SIZE        (16 * 1024 * 1024) // Bytes
#define MDF_DOWNLOAD_CONNECT_TIMEOUT 10L                 // Seconds
#define MDF_DOWNLOAD_TIMEOUT         60L                 // Seconds
#define MDF_DOWNLOAD_MAX_REDIRS      5L

struct mdfDownloadState {
  FILE *fp;
  size_t total;
};

static size_t
write_data(void *ptr, size_t size, size_t nmemb, void *userdata)
{
  mdfDownloadState *pstate = static_cast<mdfDownloadState *>(userdata);
  const size_t bytes       = size * nmemb;

  // Abort (returning less than asked for) if the file gets too big
  if ((nullptr == pstate) || (nullptr == pstate->fp) || (pstate->total + bytes > MDF_DOWNLOAD_MAX_SIZE)) {
    spdlog::error("MDF download: File is larger than {} bytes. Aborted.", MDF_DOWNLOAD_MAX_SIZE);
    return 0;
  }

  size_t written = fwrite(ptr, 1, bytes, pstate->fp);
  pstate->total += written;
  return written;
}

CURLcode
CMDF::downLoadMDF(const std::string &url, const std::string &tempFileName)
{
  CURLcode res = CURLE_FAILED_INIT;

  // Only plain or secure HTTP. (Redirects to e.g. file:// are refused.)
  if ((0 != url.compare(0, 7, "http://")) && (0 != url.compare(0, 8, "https://"))) {
    spdlog::error("MDF download: Unsupported URL scheme {}", url);
    return CURLE_UNSUPPORTED_PROTOCOL;
  }

  CURL *curl = curl_easy_init();
  if (nullptr == curl) {
    return res;
  }

  FILE *fp = fopen(tempFileName.c_str(), "wb");
  if (nullptr == fp) {
    spdlog::error("MDF download: Failed to open temporary file {}", tempFileName);
    curl_easy_cleanup(curl);
    return CURLE_WRITE_ERROR;
  }

  mdfDownloadState state = { fp, 0 };

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, MDF_DOWNLOAD_MAX_REDIRS);
#if LIBCURL_VERSION_NUM >= 0x075500 // 7.85.0
  curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, "http,https");
  curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS_STR, "http,https");
#else
  curl_easy_setopt(curl, CURLOPT_PROTOCOLS, (long) (CURLPROTO_HTTP | CURLPROTO_HTTPS));
  curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, (long) (CURLPROTO_HTTP | CURLPROTO_HTTPS));
#endif
  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L); // HTTP >= 400 is an error, not an MDF
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, MDF_DOWNLOAD_CONNECT_TIMEOUT);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, MDF_DOWNLOAD_TIMEOUT);
  curl_easy_setopt(curl, CURLOPT_MAXFILESIZE_LARGE, (curl_off_t) MDF_DOWNLOAD_MAX_SIZE);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &state);

  res = curl_easy_perform(curl);

  // always cleanup
  curl_easy_cleanup(curl);
  if ((0 != fclose(fp)) && (CURLE_OK == res)) {
    res = CURLE_WRITE_ERROR;
  }

  if (CURLE_OK != res) {
    spdlog::error("MDF download: Failed to download {} ({})", url, curl_easy_strerror(res));
    remove(tempFileName.c_str()); // Do not leave a partial file behind
  }

  return res;
}

///////////////////////////////////////////////////////////////////////////////
//  mdfCreateTempFile
//
// Create a new, empty, uniquely named temporary file that only the current
// user can access.
//

static bool
mdfCreateTempFile(std::string &path)
{
#ifdef WIN32
  char dir[MAX_PATH + 1];
  char name[MAX_PATH + 1];
  DWORD len = GetTempPathA(sizeof(dir), dir);
  if ((0 == len) || (len > sizeof(dir))) {
    return false;
  }
  // Creates the file (unique name)
  if (0 == GetTempFileNameA(dir, "mdf", 0, name)) {
    return false;
  }
  path = name;
  return true;
#else
  const char *dir = getenv("TMPDIR");
  std::string templ = std::string((nullptr != dir && *dir) ? dir : "/tmp") + "/vscp-mdf-XXXXXX";
  std::vector<char> buf(templ.begin(), templ.end());
  buf.push_back('\0');
  int fd = mkstemp(buf.data()); // Creates the file with mode 0600
  if (-1 == fd) {
    return false;
  }
  close(fd);
  path = buf.data();
  return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
//  load
//

bool
CMDF::load(const std::string &file, bool bLocalFile)
{
  std::string path = file;
  vscp_trim(path);
  m_strLastError.clear();

  // Must have a path or URL
  if (path.empty()) {
    m_strLastError = "No MDF path or URL given";
    return false;
  }

  if (bLocalFile) {
    // parseMDF returns VSCP_ERROR_SUCCESS (0) on success
    return (VSCP_ERROR_SUCCESS == parseMDF(path));
  }

  // Remote file. Keep an explicit http:// or https:// scheme (https used
  // to become "http://https://..."). URLs without a scheme (as stored in
  // devices) get http:// as before.
  std::string url = path;
  if (std::string::npos == url.find("://")) {
    url = "http://" + url;
  }

  // Download to a private temporary file (the URL itself was used as the
  // local file name before)
  std::string tempFile;
  if (!mdfCreateTempFile(tempFile)) {
    spdlog::error("MDF load: Failed to create temporary file");
    m_strLastError = "Failed to create a temporary file for the download";
    return false;
  }

  // downLoadMDF returns CURLE_OK (0) on success
  CURLcode curlrv = downLoadMDF(url, tempFile);
  if (CURLE_OK != curlrv) {
    remove(tempFile.c_str());
    m_strLastError = "Failed to download " + url + " (" + curl_easy_strerror(curlrv) + ")";
    return false;
  }

  bool rv = (VSCP_ERROR_SUCCESS == parseMDF(tempFile));
  remove(tempFile.c_str());
  if (!rv) {
    return false;
  }

  // A redirect tells that the real MDF is somewhere else (a few hops at most
  // so a redirect loop ends)
  static thread_local int redirectDepth = 0;
  if (!m_redirectUrl.empty() && (m_redirectUrl != path)) {
    if (redirectDepth >= 5) {
      m_strLastError = "Too many MDF redirects";
      return false;
    }
    CMDF target;
    target.m_strLocale = m_strLocale;
    redirectDepth++;
    bool bOk = target.load(m_redirectUrl, false);
    redirectDepth--;
    if (!bOk) {
      m_strLastError = "Failed to load redirected MDF " + m_redirectUrl + ": " + target.m_strLastError;
      return false;
    }
    swapContent(target);
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
//  Escaping for saved MDF files
//
// Text from an MDF (possibly downloaded) is written back when saving. It
// must be escaped so it can not break or add to the XML/JSON structure.
// Numbers and other non string types are written as they are.
//

template<typename T>
static const T &
mdfXml(const T &value)
{
  return value;
}

static std::string
mdfXml(const std::string &str)
{
  std::string out;
  out.reserve(str.size());
  for (char c : str) {
    switch (c) {
      case '&':
        out += "&amp;";
        break;
      case '<':
        out += "&lt;";
        break;
      case '>':
        out += "&gt;";
        break;
      case '"':
        out += "&quot;";
        break;
      case '\'':
        out += "&apos;";
        break;
      // Written as character references so XML attribute value
      // normalization does not turn them into spaces
      case '\t':
        out += "&#9;";
        break;
      case '\n':
        out += "&#10;";
        break;
      case '\r':
        out += "&#13;";
        break;
      default:
        out += c;
    }
  }
  return out;
}

static std::string
mdfXml(const char *str)
{
  return mdfXml(std::string((nullptr != str) ? str : ""));
}

template<typename T>
static const T &
mdfJson(const T &value)
{
  return value;
}

static std::string
mdfJson(const std::string &str)
{
  // dump() gives a quoted and escaped JSON string - drop the quotes
  std::string quoted = json(str).dump(-1, ' ', false, json::error_handler_t::replace);
  return quoted.substr(1, quoted.size() - 2);
}

static std::string
mdfJson(const char *str)
{
  return mdfJson(std::string((nullptr != str) ? str : ""));
}

///////////////////////////////////////////////////////////////////////////////
//  writeMap_xml
//

void
CMDF::writeMap_xml(std::ofstream &fout, std::map<std::string, std::string> *pmap, const std::string &tag)
{
  if ((nullptr != pmap) && pmap->size()) {
    for (auto it = pmap->begin(); it != pmap->end(); it++) {
      fout << "<" << mdfXml(tag) << " lang=\"" << mdfXml(it->first) << "\">" << mdfXml(it->second) << "</" << mdfXml(tag) << ">" << std::endl;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//  writeMap_json
//

void
CMDF::writeMap_json(std::ofstream &fout,
                    std::map<std::string, std::string> *pmap,
                    const std::string &tag,
                    bool /*bCommaAtEnd*/)
{
  if ((nullptr != pmap) && pmap->size()) {
    fout << "," << std::endl;
    fout << "\"" << mdfJson(tag) << "\": [" << std::endl;
    size_t pos = 0;
    for (auto it = pmap->begin(); it != pmap->end(); it++) {
      fout << "{";
      fout << "\"" << mdfJson(it->first) << "\": \"" << mdfJson(it->second) << "\"";
      fout << "}";
      if ((++pos) < pmap->size()) {
        fout << ",";
      }
      fout << std::endl << std::endl;
    }
    fout << "]";
  }
}

///////////////////////////////////////////////////////////////////////////////
//  Helpers for save_xml / save_json
//
// The writers must keep everything the parsers read, so that loading a saved
// MDF gives the same CMDF content as the one that was saved. Everything is
// written in the forms parseMDF_XML()/parseMDF_JSON() read, in model order.
//

// Keyword for a remote variable type, as read by both parsers
static std::string
mdf_remoteVarTypeKeyword(vscp_remote_variable_type type)
{
  switch (type) {
    case remote_variable_type_string:
      return "string";
    case remote_variable_type_boolean:
      return "bool";
    case remote_variable_type_int8_t:
      return "int8_t";
    case remote_variable_type_uint8_t:
      return "uint8_t";
    case remote_variable_type_int16_t:
      return "int16_t";
    case remote_variable_type_uint16_t:
      return "uint16_t";
    case remote_variable_type_int32_t:
      return "int32_t";
    case remote_variable_type_uint32_t:
      return "uint32_t";
    case remote_variable_type_int64_t:
      return "int64_t";
    case remote_variable_type_uint64_t:
      return "uint64_t";
    case remote_variable_type_float:
      return "float";
    case remote_variable_type_double:
      return "double";
    case remote_variable_type_date:
      return "date";
    case remote_variable_type_time:
      return "time";
    default:
      return "unknown";
  }
}

// A file version string as written: "1.2.3" as version_major/version_minor/
// version_subminor (the specification form), anything else as "version".
static bool
mdf_versionParts(const std::string &version, std::string &major, std::string &minor, std::string &patch)
{
  std::deque<std::string> parts;
  vscp_split(parts, version, ".");
  if (3 != parts.size()) {
    return false;
  }
  for (auto &part : parts) {
    if (part.empty() || (part.size() > 5) || (std::string::npos != part.find_first_not_of("0123456789")) ||
        ((part.size() > 1) && ('0' == part[0])) || (std::stoul(part) > 65535)) {
      return false;
    }
  }
  major = parts[0];
  minor = parts[1];
  patch = parts[2];
  return true;
}

static std::string
mdf_xmlVersionAttrs(const std::string &version)
{
  std::string major, minor, patch;
  if (version.empty()) {
    return "";
  }
  if (mdf_versionParts(version, major, minor, patch)) {
    return " version_major=\"" + major + "\" version_minor=\"" + minor + "\" version_subminor=\"" + patch + "\"";
  }
  return " version=\"" + mdfXml(version) + "\"";
}

// Event class/type as written in an MDF ("-" means all)
static std::string
mdf_classTypeToFile(int value)
{
  return (value < 0) ? std::string("-") : std::to_string(value);
}

static std::string
mdf_hexColor(uint32_t color)
{
  char buf[16];
  snprintf(buf, sizeof(buf), "0x%06x", (unsigned) color);
  return buf;
}

// ----------------------------------------------------------------------------
//                                  XML
// ----------------------------------------------------------------------------

static void
mdf_xmlTexts(std::ostream &fout, CMDF_Object *pobj)
{
  std::map<std::string, std::string> *pmap;
  if (nullptr != (pmap = pobj->getMapDescription())) {
    for (auto &kv : *pmap) {
      fout << "<description lang=\"" << mdfXml(kv.first) << "\">" << mdfXml(kv.second) << "</description>"
           << std::endl;
    }
  }
  if (nullptr != (pmap = pobj->getMapInfoUrl())) {
    for (auto &kv : *pmap) {
      fout << "<infourl lang=\"" << mdfXml(kv.first) << "\">" << mdfXml(kv.second) << "</infourl>" << std::endl;
    }
  }
}

static void
mdf_xmlValues(std::ostream &fout, std::deque<CMDF_Value *> *pvalues)
{
  if ((nullptr == pvalues) || pvalues->empty()) {
    return;
  }
  fout << "<valuelist>" << std::endl;
  for (CMDF_Value *pvalue : *pvalues) {
    fout << "<item name=\"" << mdfXml(pvalue->getName()) << "\" value=\"" << mdfXml(pvalue->getValue()) << "\">"
         << std::endl;
    mdf_xmlTexts(fout, pvalue);
    fout << "</item>" << std::endl;
  }
  fout << "</valuelist>" << std::endl;
}

static void
mdf_xmlBits(std::ostream &fout, std::deque<CMDF_Bit *> *pbits)
{
  if (nullptr == pbits) {
    return;
  }
  for (CMDF_Bit *pbit : *pbits) {
    fout << "<bit name=\"" << mdfXml(pbit->getName()) << "\" ";
    fout << "pos=\"" << (int) pbit->getPos() << "\" ";
    fout << "width=\"" << (int) pbit->getWidth() << "\" ";
    fout << "default=\"" << (int) pbit->getDefault() << "\" ";
    fout << "min=\"" << (int) pbit->getMin() << "\" ";
    fout << "max=\"" << (int) pbit->getMax() << "\" ";
    fout << "access=\"" << mdfXml(pbit->getAccessStr()) << "\">" << std::endl;
    mdf_xmlTexts(fout, pbit);
    mdf_xmlValues(fout, pbit->getListValues());
    fout << "</bit>" << std::endl;
  }
}

static void
mdf_xmlItems(std::ostream &fout, std::deque<CMDF_Item *> *pitems, const char *tag, const char *valueTag)
{
  for (CMDF_Item *pitem : *pitems) {
    fout << "<" << tag << ">" << std::endl;
    fout << "<" << valueTag << ">" << mdfXml(pitem->getValue()) << "</" << valueTag << ">" << std::endl;
    mdf_xmlTexts(fout, pitem);
    fout << "</" << tag << ">" << std::endl;
  }
}

/*
  A register to save. The parsers expand a DMATRIX1/BLOCK register with a
  span into one register per byte ("<name> - DM 0", "<name> - DM 1", ...).
  When saving, such a group is written as the original single register, but
  only if every register of the group is still exactly what the expansion
  made (an editor may have changed, renamed or removed one of them). If not,
  each register is written on its own (span 1) so no change is lost.
*/
struct mdf_saveRegister {
  CMDF_Register *preg;
  std::string name; // Name to write
  uint32_t span;    // Span to write
  bool bGroup;      // true for a collapsed DMATRIX1/BLOCK group
};

// Everything of a register except name, offset and display colors
static std::string
mdf_registerSignature(CMDF_Register *preg)
{
  std::ostringstream ss;
  ss << preg->getPage() << "|" << preg->getWidth() << "|" << (int) preg->getType() << "|" << preg->getAccessStr()
     << "|" << preg->getDefault() << "|" << preg->getMin() << "|" << preg->getMax() << "|"
     << preg->getForegroundColorEven() << "|" << preg->getForegroundColorOdd() << "|"
     << preg->getBackgroundColorEven() << "|" << preg->getBackgroundColorOdd() << "|"
     << preg->getForegroundColorMdf() << "|" << preg->getBackgroundColorMdf() << "|" << preg->getSpan() << "\n";
  mdf_xmlTexts(ss, preg);
  mdf_xmlBits(ss, preg->getListBits());
  mdf_xmlValues(ss, preg->getListValues());
  return ss.str();
}

// Display colors expandRegisterSpan() gives register 'pos' of a group
static bool
mdf_spanColorsAsExpanded(CMDF_Register *pfirst, CMDF_Register *preg, uint32_t pos)
{
  const bool bPyjamas = pfirst->getForegroundColorEven() || pfirst->getForegroundColorOdd() ||
                        pfirst->getBackgroundColorEven() || pfirst->getBackgroundColorOdd();
  uint32_t fg = pfirst->getForegroundColorMdf();
  uint32_t bg = pfirst->getBackgroundColorMdf();
  if (bPyjamas) {
    fg = (pos % 2) ? pfirst->getForegroundColorOdd() : pfirst->getForegroundColorEven();
    bg = (pos % 2) ? pfirst->getBackgroundColorOdd() : pfirst->getBackgroundColorEven();
  }
  return (preg->getForegroundColor() == fg) && (preg->getBackgroundColor() == bg);
}

static std::deque<mdf_saveRegister>
mdf_registersToSave(std::deque<CMDF_Register *> *pregs)
{
  std::deque<mdf_saveRegister> out;
  std::set<CMDF_Register *> skip;

  for (size_t i = 0; i < pregs->size(); i++) {
    CMDF_Register *preg = (*pregs)[i];
    if (skip.count(preg)) {
      continue;
    }

    // Not made by expansion: written as it is (a DMATRIX1/BLOCK register
    // added in an editor is expanded when the file is read again)
    if (preg->getSpanIndex() < 0) {
      out.push_back({ preg, preg->getName(), preg->getSpan(), false });
      continue;
    }

    const char *label = (MDF_REG_TYPE_DMATRIX1 == preg->getType()) ? " - DM " : " - BLOCK ";
    std::string first = std::string(label) + "0";
    std::string name  = preg->getName();

    bool bIntact = (0 == preg->getSpanIndex()) && (preg->getSpan() > 1) &&
                   ((MDF_REG_TYPE_DMATRIX1 == preg->getType()) || (MDF_REG_TYPE_BLOCK == preg->getType())) &&
                   (name.size() >= first.size()) &&
                   (0 == name.compare(name.size() - first.size(), first.size(), first)) &&
                   mdf_spanColorsAsExpanded(preg, preg, 0);

    std::deque<CMDF_Register *> members;
    std::string base;
    if (bIntact) {
      // First register of an expanded group. The group is written as the
      // original register only if all its registers are as expanded.
      base            = name.substr(0, name.size() - first.size());
      std::string sig = mdf_registerSignature(preg);
      members.push_back(preg);
      for (uint32_t pos = 1; (pos < preg->getSpan()) && bIntact; pos++) {
        std::string copyName  = base + label + std::to_string(pos);
        CMDF_Register *pfound = nullptr;
        for (size_t j = i + 1; j < pregs->size(); j++) {
          CMDF_Register *pcopy = (*pregs)[j];
          if (!skip.count(pcopy) && ((int) pos == pcopy->getSpanIndex()) && (pcopy->getType() == preg->getType()) &&
              (pcopy->getPage() == preg->getPage()) && (pcopy->getOffset() == preg->getOffset() + pos)) {
            pfound = pcopy;
            break;
          }
        }
        if ((nullptr == pfound) || (pfound->getName() != copyName) || (mdf_registerSignature(pfound) != sig) ||
            !mdf_spanColorsAsExpanded(preg, pfound, pos)) {
          bIntact = false;
        }
        else {
          members.push_back(pfound);
        }
      }
    }

    if (bIntact) {
      for (CMDF_Register *pm : members) {
        skip.insert(pm);
      }
      out.push_back({ preg, base, preg->getSpan(), true });
    }
    else {
      // A register made by expansion that was changed (or whose group was
      // changed): written on its own, one byte
      out.push_back({ preg, name, 1, false });
    }
  }

  return out;
}

///////////////////////////////////////////////////////////////////////////////
//  toXmlString
//

std::string
CMDF::toXmlString(void)
{
  std::ostringstream fout;

  fout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  fout << "<!-- MDF file created with VSCP Works + -->" << std::endl;
  fout << "<vscp>" << std::endl;
  if (!m_redirectUrl.empty()) {
    fout << "<redirect mdf-path=\"" << mdfXml(m_redirectUrl) << "\" />" << std::endl;
  }
  fout << "<module>" << std::endl;

  fout << "<name>" << mdfXml(getModuleName()) << "</name>" << std::endl;
  fout << "<copyright>" << mdfXml(getModuleCopyright()) << "</copyright>" << std::endl;
  fout << "<level>" << mdf_levelToFile(getModuleLevel()) << "</level>" << std::endl;
  fout << "<model>" << mdfXml(getModuleModel()) << "</model>" << std::endl;
  fout << "<version>" << mdfXml(getModuleVersion()) << "</version>" << std::endl;
  fout << "<changed>" << mdfXml(getModuleChangeDate()) << "</changed>" << std::endl;
  fout << "<buffersize>" << getModuleBufferSize() << "</buffersize>" << std::endl;
  mdf_xmlTexts(fout, this);

  // Manufacturer
  CMDF_Manufacturer *pman = getManufacturer();
  CMDF_Address *paddr     = pman->getAddressObj();
  fout << "<manufacturer>" << std::endl;
  fout << "<name>" << mdfXml(pman->getName()) << "</name>" << std::endl;
  fout << "<address>" << std::endl;
  fout << "<street>" << mdfXml(paddr->getStreet()) << "</street>" << std::endl;
  fout << "<town>" << mdfXml(paddr->getTown()) << "</town>" << std::endl;
  fout << "<city>" << mdfXml(paddr->getCity()) << "</city>" << std::endl;
  fout << "<postcode>" << mdfXml(paddr->getPostCode()) << "</postcode>" << std::endl;
  fout << "<state>" << mdfXml(paddr->getState()) << "</state>" << std::endl;
  fout << "<region>" << mdfXml(paddr->getRegion()) << "</region>" << std::endl;
  fout << "<country>" << mdfXml(paddr->getCountry()) << "</country>" << std::endl;
  fout << "</address>" << std::endl;
  mdf_xmlItems(fout, pman->getPhoneContactList(), "telephone", "number");
  mdf_xmlItems(fout, pman->getFaxContactList(), "fax", "number");
  mdf_xmlItems(fout, pman->getEmailContactList(), "email", "address");
  mdf_xmlItems(fout, pman->getWebContactList(), "web", "address");
  mdf_xmlItems(fout, pman->getSocialContactList(), "social", "address");
  fout << "</manufacturer>" << std::endl;

  // Boot loader
  CMDF_BootLoaderInfo *pboot = getBootLoaderObj();
  fout << "<boot>" << std::endl;
  fout << "<algorithm>" << (int) pboot->getAlgorithm() << "</algorithm>" << std::endl;
  fout << "<blocksize>" << pboot->getBlockSize() << "</blocksize>" << std::endl;
  fout << "<blockcount>" << pboot->getBlockCount() << "</blockcount>" << std::endl;
  fout << "</boot>" << std::endl;

  // Files
  fout << "<files>" << std::endl;
  for (CMDF_Picture *p : *getPictureObjList()) {
    fout << "<picture name=\"" << mdfXml(p->getName()) << "\" url=\"" << mdfXml(p->getUrl()) << "\" format=\""
         << mdfXml(p->getFormat()) << "\" date=\"" << mdfXml(p->getDate()) << "\""
         << mdf_xmlVersionAttrs(p->getVersion()) << ">" << std::endl;
    mdf_xmlTexts(fout, p);
    fout << "</picture>" << std::endl;
  }
  for (CMDF_Video *p : *getVideoObjList()) {
    fout << "<video name=\"" << mdfXml(p->getName()) << "\" url=\"" << mdfXml(p->getUrl()) << "\" format=\""
         << mdfXml(p->getFormat()) << "\" date=\"" << mdfXml(p->getDate()) << "\""
         << mdf_xmlVersionAttrs(p->getVersion()) << ">" << std::endl;
    mdf_xmlTexts(fout, p);
    fout << "</video>" << std::endl;
  }
  for (CMDF_Manual *p : *getManualObjList()) {
    fout << "<manual name=\"" << mdfXml(p->getName()) << "\" url=\"" << mdfXml(p->getUrl()) << "\" format=\""
         << mdfXml(p->getFormat()) << "\" lang=\"" << mdfXml(p->getLanguage()) << "\" date=\""
         << mdfXml(p->getDate()) << "\"" << mdf_xmlVersionAttrs(p->getVersion()) << ">" << std::endl;
    mdf_xmlTexts(fout, p);
    fout << "</manual>" << std::endl;
  }
  for (CMDF_Firmware *p : *getFirmwareObjList()) {
    fout << "<firmware name=\"" << mdfXml(p->getName()) << "\" url=\"" << mdfXml(p->getUrl()) << "\" format=\""
         << mdfXml(p->getFormat()) << "\" date=\"" << mdfXml(p->getDate()) << "\" target=\""
         << mdfXml(p->getTarget()) << "\" targetcode=\"" << p->getTargetCode() << "\" version_major=\""
         << p->getVersionMajor() << "\" version_minor=\"" << p->getVersionMinor() << "\" version_subminor=\""
         << p->getVersionPatch() << "\" size=\"" << p->getSize() << "\" md5=\"" << mdfXml(p->getMd5())
         << "\">" << std::endl;
    mdf_xmlTexts(fout, p);
    fout << "</firmware>" << std::endl;
  }
  for (CMDF_Driver *p : *getDriverObjList()) {
    fout << "<driver name=\"" << mdfXml(p->getName()) << "\" url=\"" << mdfXml(p->getUrl()) << "\" type=\""
         << mdfXml(p->getType()) << "\" architecture=\"" << mdfXml(p->getArchitecture()) << "\" os=\""
         << mdfXml(p->getOS()) << "\" osver=\"" << mdfXml(p->getOSVer()) << "\" date=\"" << mdfXml(p->getDate())
         << "\" version_major=\"" << p->getVersionMajor() << "\" version_minor=\"" << p->getVersionMinor()
         << "\" version_subminor=\"" << p->getVersionPatch() << "\" md5=\"" << mdfXml(p->getMd5()) << "\">"
         << std::endl;
    mdf_xmlTexts(fout, p);
    fout << "</driver>" << std::endl;
  }
  for (CMDF_Setup *p : *getSetupObjList()) {
    fout << "<setup name=\"" << mdfXml(p->getName()) << "\" url=\"" << mdfXml(p->getUrl()) << "\" format=\""
         << mdfXml(p->getFormat()) << "\" date=\"" << mdfXml(p->getDate()) << "\""
         << mdf_xmlVersionAttrs(p->getVersion()) << ">" << std::endl;
    mdf_xmlTexts(fout, p);
    fout << "</setup>" << std::endl;
  }
  fout << "</files>" << std::endl;

  // Registers (model order, expanded DMATRIX1/BLOCK registers collapsed)
  fout << "<registers>" << std::endl;
  for (auto &sr : mdf_registersToSave(getRegisterObjList())) {
    CMDF_Register *preg = sr.preg;
    fout << "<reg name=\"" << mdfXml(sr.name) << "\" ";
    fout << "page=\"" << preg->getPage() << "\" ";
    fout << "offset=\"" << preg->getOffset() << "\" ";
    fout << "span=\"" << sr.span << "\" ";
    fout << "width=\"" << preg->getWidth() << "\" ";
    fout << "type=\"" << preg->getTypeStr() << "\" ";
    fout << "access=\"" << mdfXml(preg->getAccessStr()) << "\" ";
    fout << "default=\"" << mdfXml(preg->getDefault()) << "\" ";
    fout << "min=\"" << (int) preg->getMin() << "\" ";
    fout << "max=\"" << (int) preg->getMax() << "\" ";
    fout << "fgcolor=\"" << mdf_hexColor(sr.bGroup ? preg->getForegroundColorMdf() : preg->getForegroundColor())
         << "\" ";
    fout << "bgcolor=\"" << mdf_hexColor(sr.bGroup ? preg->getBackgroundColorMdf() : preg->getBackgroundColor())
         << "\" ";
    fout << "evenfg=\"" << mdf_hexColor(preg->getForegroundColorEven()) << "\" ";
    fout << "oddfg=\"" << mdf_hexColor(preg->getForegroundColorOdd()) << "\" ";
    fout << "evenbg=\"" << mdf_hexColor(preg->getBackgroundColorEven()) << "\" ";
    fout << "oddbg=\"" << mdf_hexColor(preg->getBackgroundColorOdd()) << "\">" << std::endl;
    mdf_xmlTexts(fout, preg);
    mdf_xmlBits(fout, preg->getListBits());
    mdf_xmlValues(fout, preg->getListValues());
    fout << "</reg>" << std::endl;
  }
  fout << "</registers>" << std::endl;

  // Remote variables
  fout << "<remotevars>" << std::endl;
  for (CMDF_RemoteVariable *prvar : *getRemoteVariableList()) {
    fout << "<remotevar name=\"" << mdfXml(prvar->getName()) << "\" ";
    fout << "type=\"" << mdf_remoteVarTypeKeyword(prvar->getType()) << "\" ";
    fout << "default=\"" << mdfXml(prvar->getDefault()) << "\" ";
    fout << "page=\"" << prvar->getPage() << "\" ";
    fout << "offset=\"" << prvar->getOffset() << "\" ";
    fout << "bitpos=\"" << prvar->getBitPos() << "\" ";
    fout << "size=\"" << prvar->getStringSize() << "\" ";
    if (!prvar->getMin().empty()) {
      fout << "min=\"" << mdfXml(prvar->getMin()) << "\" ";
    }
    if (!prvar->getMax().empty()) {
      fout << "max=\"" << mdfXml(prvar->getMax()) << "\" ";
    }
    fout << "access=\"" << mdfXml(prvar->getAccessStr()) << "\" ";
    fout << "fgcolor=\"" << mdf_hexColor(prvar->getForegroundColor()) << "\" ";
    fout << "bgcolor=\"" << mdf_hexColor(prvar->getBackgroundColor()) << "\">" << std::endl;
    mdf_xmlTexts(fout, prvar);
    mdf_xmlBits(fout, prvar->getListBits());
    mdf_xmlValues(fout, prvar->getListValues());
    fout << "</remotevar>" << std::endl;
  }
  fout << "</remotevars>" << std::endl;

  // Alarm bits
  fout << "<alarm>" << std::endl;
  mdf_xmlBits(fout, getAlarmList());
  fout << "</alarm>" << std::endl;

  // Decision matrix
  CMDF_DecisionMatrix *pdm = getDM();
  fout << "<dmatrix>" << std::endl;
  fout << "<level>" << (int) pdm->getLevel() << "</level>" << std::endl;
  fout << "<start page=\"" << pdm->getStartPage() << "\" offset=\"" << pdm->getStartOffset() << "\" />"
       << std::endl;
  fout << "<rowcnt>" << pdm->getRowCount() << "</rowcnt>" << std::endl;
  fout << "<rowsize>" << pdm->getRowSize() << "</rowsize>" << std::endl;
  for (CMDF_Action *paction : *pdm->getActionList()) {
    fout << "<action code=\"" << paction->getCode() << "\" name=\"" << mdfXml(paction->getName()) << "\">"
         << std::endl;
    mdf_xmlTexts(fout, paction);
    for (CMDF_ActionParameter *pparam : *paction->getListActionParameter()) {
      fout << "<param name=\"" << mdfXml(pparam->getName()) << "\" offset=\"" << pparam->getOffset() << "\" min=\""
           << (int) pparam->getMin() << "\" max=\"" << (int) pparam->getMax() << "\">" << std::endl;
      mdf_xmlTexts(fout, pparam);
      mdf_xmlBits(fout, pparam->getListBits());
      mdf_xmlValues(fout, pparam->getListValues());
      fout << "</param>" << std::endl;
    }
    fout << "</action>" << std::endl;
  }
  fout << "</dmatrix>" << std::endl;

  // Events
  fout << "<events>" << std::endl;
  for (CMDF_Event *pevent : *getEventList()) {
    fout << "<event name=\"" << mdfXml(pevent->getName()) << "\" ";
    fout << "class=\"" << mdf_classTypeToFile(pevent->getClass()) << "\" ";
    fout << "type=\"" << mdf_classTypeToFile(pevent->getType()) << "\" ";
    fout << "priority=\"" << (int) pevent->getPriority() << "\" ";
    fout << "direction=\"" << ((MDF_EVENT_DIR_IN == pevent->getDirection()) ? "in" : "out") << "\">" << std::endl;
    mdf_xmlTexts(fout, pevent);
    for (CMDF_EventData *pdata : *pevent->getListEventData()) {
      fout << "<data name=\"" << mdfXml(pdata->getName()) << "\" offset=\"" << pdata->getOffset() << "\">"
           << std::endl;
      mdf_xmlTexts(fout, pdata);
      mdf_xmlBits(fout, pdata->getListBits());
      mdf_xmlValues(fout, pdata->getListValues());
      fout << "</data>" << std::endl;
    }
    fout << "</event>" << std::endl;
  }
  fout << "</events>" << std::endl;

  fout << "</module>" << std::endl;
  fout << "</vscp>" << std::endl;

  return fout.str();
}

// ----------------------------------------------------------------------------
//                                  JSON
// ----------------------------------------------------------------------------

using ojson = nlohmann::ordered_json;

static void
mdf_jsonTexts(ojson &j, CMDF_Object *pobj)
{
  std::map<std::string, std::string> *pmap;
  if ((nullptr != (pmap = pobj->getMapDescription())) && !pmap->empty()) {
    j["description"] = ojson::object();
    for (auto &kv : *pmap) {
      j["description"][kv.first] = kv.second;
    }
  }
  if ((nullptr != (pmap = pobj->getMapInfoUrl())) && !pmap->empty()) {
    j["infourl"] = ojson::object();
    for (auto &kv : *pmap) {
      j["infourl"][kv.first] = kv.second;
    }
  }
}

static void
mdf_jsonVersion(ojson &j, const std::string &version)
{
  std::string major, minor, patch;
  if (version.empty()) {
    return;
  }
  if (mdf_versionParts(version, major, minor, patch)) {
    j["version_major"]    = std::stoul(major);
    j["version_minor"]    = std::stoul(minor);
    j["version_subminor"] = std::stoul(patch);
  }
  else {
    j["version"] = version;
  }
}

static void
mdf_jsonValues(ojson &j, std::deque<CMDF_Value *> *pvalues)
{
  if ((nullptr == pvalues) || pvalues->empty()) {
    return;
  }
  j["valuelist"] = ojson::array();
  for (CMDF_Value *pvalue : *pvalues) {
    ojson jv;
    jv["name"]  = pvalue->getName();
    jv["value"] = pvalue->getValue();
    mdf_jsonTexts(jv, pvalue);
    j["valuelist"].push_back(jv);
  }
}

static ojson
mdf_jsonBitList(std::deque<CMDF_Bit *> *pbits)
{
  ojson jbits = ojson::array();
  for (CMDF_Bit *pbit : *pbits) {
    ojson jb;
    jb["name"]    = pbit->getName();
    jb["pos"]     = pbit->getPos();
    jb["width"]   = pbit->getWidth();
    jb["default"] = pbit->getDefault();
    jb["min"]     = pbit->getMin();
    jb["max"]     = pbit->getMax();
    jb["access"]  = pbit->getAccessStr();
    mdf_jsonTexts(jb, pbit);
    mdf_jsonValues(jb, pbit->getListValues());
    jbits.push_back(jb);
  }
  return jbits;
}

static void
mdf_jsonBits(ojson &j, std::deque<CMDF_Bit *> *pbits)
{
  if ((nullptr != pbits) && !pbits->empty()) {
    j["bit"] = mdf_jsonBitList(pbits);
  }
}

static ojson
mdf_jsonItems(std::deque<CMDF_Item *> *pitems, const char *valueKey)
{
  ojson jitems = ojson::array();
  for (CMDF_Item *pitem : *pitems) {
    ojson ji;
    ji[valueKey] = pitem->getValue();
    mdf_jsonTexts(ji, pitem);
    jitems.push_back(ji);
  }
  return jitems;
}

///////////////////////////////////////////////////////////////////////////////
//  toJsonString
//

std::string
CMDF::toJsonString(void)
{
  ojson jmod;

  jmod["name"]       = getModuleName();
  jmod["copyright"]  = getModuleCopyright();
  jmod["level"]      = mdf_levelToFile(getModuleLevel());
  jmod["model"]      = getModuleModel();
  jmod["version"]    = getModuleVersion();
  jmod["changed"]    = getModuleChangeDate();
  jmod["buffersize"] = getModuleBufferSize();
  mdf_jsonTexts(jmod, this);

  // Manufacturer
  CMDF_Manufacturer *pman = getManufacturer();
  CMDF_Address *paddr     = pman->getAddressObj();
  ojson jman;
  jman["name"]                = pman->getName();
  jman["address"]["street"]   = paddr->getStreet();
  jman["address"]["town"]     = paddr->getTown();
  jman["address"]["city"]     = paddr->getCity();
  jman["address"]["postcode"] = paddr->getPostCode();
  jman["address"]["state"]    = paddr->getState();
  jman["address"]["region"]   = paddr->getRegion();
  jman["address"]["country"]  = paddr->getCountry();
  jman["telephone"]           = mdf_jsonItems(pman->getPhoneContactList(), "number");
  jman["fax"]                 = mdf_jsonItems(pman->getFaxContactList(), "number");
  jman["email"]               = mdf_jsonItems(pman->getEmailContactList(), "address");
  jman["web"]                 = mdf_jsonItems(pman->getWebContactList(), "address");
  jman["social"]              = mdf_jsonItems(pman->getSocialContactList(), "address");
  jmod["manufacturer"]        = jman;

  // Boot loader
  CMDF_BootLoaderInfo *pboot      = getBootLoaderObj();
  jmod["boot"]["algorithm"]       = pboot->getAlgorithm();
  jmod["boot"]["blocksize"]       = pboot->getBlockSize();
  jmod["boot"]["blockcount"]      = pboot->getBlockCount();

  // Files
  ojson jfiles;
  jfiles["picture"] = ojson::array();
  for (CMDF_Picture *p : *getPictureObjList()) {
    ojson jf;
    jf["name"]   = p->getName();
    jf["url"]    = p->getUrl();
    jf["format"] = p->getFormat();
    jf["date"]   = p->getDate();
    mdf_jsonVersion(jf, p->getVersion());
    mdf_jsonTexts(jf, p);
    jfiles["picture"].push_back(jf);
  }
  jfiles["video"] = ojson::array();
  for (CMDF_Video *p : *getVideoObjList()) {
    ojson jf;
    jf["name"]   = p->getName();
    jf["url"]    = p->getUrl();
    jf["format"] = p->getFormat();
    jf["date"]   = p->getDate();
    mdf_jsonVersion(jf, p->getVersion());
    mdf_jsonTexts(jf, p);
    jfiles["video"].push_back(jf);
  }
  jfiles["manual"] = ojson::array();
  for (CMDF_Manual *p : *getManualObjList()) {
    ojson jf;
    jf["name"]   = p->getName();
    jf["url"]    = p->getUrl();
    jf["format"] = p->getFormat();
    jf["lang"]   = p->getLanguage();
    jf["date"]   = p->getDate();
    mdf_jsonVersion(jf, p->getVersion());
    mdf_jsonTexts(jf, p);
    jfiles["manual"].push_back(jf);
  }
  jfiles["firmware"] = ojson::array();
  for (CMDF_Firmware *p : *getFirmwareObjList()) {
    ojson jf;
    jf["name"]          = p->getName();
    jf["url"]           = p->getUrl();
    jf["target"]        = p->getTarget();
    jf["targetcode"]    = p->getTargetCode();
    jf["format"]        = p->getFormat();
    jf["date"]          = p->getDate();
    jf["version_major"] = p->getVersionMajor();
    jf["version_minor"] = p->getVersionMinor();
    jf["version_subminor"] = p->getVersionPatch();
    jf["size"]          = p->getSize();
    jf["md5"]           = p->getMd5();
    mdf_jsonTexts(jf, p);
    jfiles["firmware"].push_back(jf);
  }
  jfiles["driver"] = ojson::array();
  for (CMDF_Driver *p : *getDriverObjList()) {
    ojson jf;
    jf["name"]          = p->getName();
    jf["url"]           = p->getUrl();
    jf["type"]          = p->getType();
    jf["os"]            = p->getOS();
    jf["osver"]         = p->getOSVer();
    jf["architecture"]  = p->getArchitecture();
    jf["date"]          = p->getDate();
    jf["version_major"] = p->getVersionMajor();
    jf["version_minor"] = p->getVersionMinor();
    jf["version_subminor"] = p->getVersionPatch();
    jf["md5"]           = p->getMd5();
    mdf_jsonTexts(jf, p);
    jfiles["driver"].push_back(jf);
  }
  jfiles["setup"] = ojson::array();
  for (CMDF_Setup *p : *getSetupObjList()) {
    ojson jf;
    jf["name"]    = p->getName();
    jf["url"]     = p->getUrl();
    jf["format"]  = p->getFormat();
    jf["date"]    = p->getDate();
    mdf_jsonVersion(jf, p->getVersion());
    mdf_jsonTexts(jf, p);
    jfiles["setup"].push_back(jf);
  }
  jmod["files"] = jfiles;

  // Registers (model order, expanded DMATRIX1/BLOCK registers collapsed)
  jmod["registers"] = ojson::array();
  for (auto &sr : mdf_registersToSave(getRegisterObjList())) {
    CMDF_Register *preg = sr.preg;
    ojson jr;
    jr["name"]    = sr.name;
    jr["page"]    = preg->getPage();
    jr["offset"]  = preg->getOffset();
    jr["span"]    = sr.span;
    jr["width"]   = preg->getWidth();
    jr["type"]    = preg->getTypeStr();
    jr["access"]  = preg->getAccessStr();
    jr["default"] = preg->getDefault();
    jr["min"]     = preg->getMin();
    jr["max"]     = preg->getMax();
    jr["fgcolor"] = mdf_hexColor(sr.bGroup ? preg->getForegroundColorMdf() : preg->getForegroundColor());
    jr["bgcolor"] = mdf_hexColor(sr.bGroup ? preg->getBackgroundColorMdf() : preg->getBackgroundColor());
    jr["evenfg"]  = mdf_hexColor(preg->getForegroundColorEven());
    jr["oddfg"]   = mdf_hexColor(preg->getForegroundColorOdd());
    jr["evenbg"]  = mdf_hexColor(preg->getBackgroundColorEven());
    jr["oddbg"]   = mdf_hexColor(preg->getBackgroundColorOdd());
    mdf_jsonTexts(jr, preg);
    mdf_jsonBits(jr, preg->getListBits());
    mdf_jsonValues(jr, preg->getListValues());
    jmod["registers"].push_back(jr);
  }

  // Remote variables
  jmod["remotevars"] = ojson::array();
  for (CMDF_RemoteVariable *prvar : *getRemoteVariableList()) {
    ojson jr;
    jr["name"]    = prvar->getName();
    jr["type"]    = mdf_remoteVarTypeKeyword(prvar->getType());
    jr["default"] = prvar->getDefault();
    if (!prvar->getMin().empty()) {
      jr["min"] = prvar->getMin();
    }
    if (!prvar->getMax().empty()) {
      jr["max"] = prvar->getMax();
    }
    jr["page"]    = prvar->getPage();
    jr["offset"]  = prvar->getOffset();
    jr["bitpos"]  = prvar->getBitPos();
    jr["size"]    = prvar->getStringSize();
    jr["access"]  = prvar->getAccessStr();
    jr["fgcolor"] = mdf_hexColor(prvar->getForegroundColor());
    jr["bgcolor"] = mdf_hexColor(prvar->getBackgroundColor());
    mdf_jsonTexts(jr, prvar);
    mdf_jsonBits(jr, prvar->getListBits());
    mdf_jsonValues(jr, prvar->getListValues());
    jmod["remotevars"].push_back(jr);
  }

  // Alarm bits
  jmod["alarm"] = mdf_jsonBitList(getAlarmList());

  // Decision matrix
  CMDF_DecisionMatrix *pdm = getDM();
  ojson jdm;
  jdm["level"]        = pdm->getLevel();
  jdm["start-page"]   = pdm->getStartPage();
  jdm["start-offset"] = pdm->getStartOffset();
  jdm["rowcount"]     = pdm->getRowCount();
  jdm["rowsize"]      = pdm->getRowSize();
  jdm["action"]       = ojson::array();
  for (CMDF_Action *paction : *pdm->getActionList()) {
    ojson ja;
    ja["name"] = paction->getName();
    ja["code"] = paction->getCode();
    mdf_jsonTexts(ja, paction);
    ja["param"] = ojson::array();
    for (CMDF_ActionParameter *pparam : *paction->getListActionParameter()) {
      ojson jp;
      jp["name"]   = pparam->getName();
      jp["offset"] = pparam->getOffset();
      jp["min"]    = pparam->getMin();
      jp["max"]    = pparam->getMax();
      mdf_jsonTexts(jp, pparam);
      mdf_jsonBits(jp, pparam->getListBits());
      mdf_jsonValues(jp, pparam->getListValues());
      ja["param"].push_back(jp);
    }
    jdm["action"].push_back(ja);
  }
  jmod["dmatrix"] = jdm;

  // Events
  jmod["events"] = ojson::array();
  for (CMDF_Event *pevent : *getEventList()) {
    ojson je;
    je["name"]      = pevent->getName();
    if (pevent->getClass() < 0) {
      je["class"] = "-";
    }
    else {
      je["class"] = pevent->getClass();
    }
    if (pevent->getType() < 0) {
      je["type"] = "-";
    }
    else {
      je["type"] = pevent->getType();
    }
    je["priority"]  = pevent->getPriority();
    je["direction"] = (MDF_EVENT_DIR_IN == pevent->getDirection()) ? "in" : "out";
    mdf_jsonTexts(je, pevent);
    je["data"] = ojson::array();
    for (CMDF_EventData *pdata : *pevent->getListEventData()) {
      ojson jd;
      jd["name"]   = pdata->getName();
      jd["offset"] = pdata->getOffset();
      mdf_jsonTexts(jd, pdata);
      mdf_jsonBits(jd, pdata->getListBits());
      mdf_jsonValues(jd, pdata->getListValues());
      je["data"].push_back(jd);
    }
    jmod["events"].push_back(je);
  }

  ojson jroot;
  if (!m_redirectUrl.empty()) {
    jroot["redirect"] = m_redirectUrl;
  }
  jroot["module"] = jmod;

  // Invalid UTF-8 in texts is replaced instead of throwing
  return jroot.dump(2, ' ', false, ojson::error_handler_t::replace) + "\n";
}

///////////////////////////////////////////////////////////////////////////////
//  save_xml / save_json
//

int
CMDF::save_xml(const std::string &path)
{
  return save(path, MDF_FORMAT_XML);
}

int
CMDF::save_json(const std::string &path)
{
  return save(path, MDF_FORMAT_JSON);
}

// ----------------------------------------------------------------------------
//                          Safe file replacement
// ----------------------------------------------------------------------------

#ifdef WIN32
static std::wstring
mdf_widePath(const std::string &path)
{
  int len = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
  if (len <= 0) {
    return std::wstring();
  }
  std::wstring wpath(len, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &wpath[0], len);
  wpath.resize(len - 1);
  return wpath;
}
#endif

/*
  Write content to a new temporary file in the same folder as path (so it
  can be renamed over path). The file is flushed to disk before it is closed.
  On success tempPath holds its name.
*/
static bool
mdf_writeTempFile(const std::string &path, const std::string &content, std::string &tempPath, std::string &err)
{
#ifdef WIN32
  for (int attempt = 0; attempt < 100; attempt++) {
    tempPath = path + ".tmp" + std::to_string(GetCurrentProcessId()) + "_" + std::to_string(GetTickCount() + attempt);
    // CREATE_NEW: never touch an existing file
    HANDLE h = CreateFileW(mdf_widePath(tempPath).c_str(),
                           GENERIC_WRITE,
                           0,
                           nullptr,
                           CREATE_NEW,
                           FILE_ATTRIBUTE_NORMAL,
                           nullptr);
    if (INVALID_HANDLE_VALUE == h) {
      if (ERROR_FILE_EXISTS == GetLastError()) {
        continue;
      }
      err = "Unable to create temporary file " + tempPath;
      return false;
    }
    const char *p = content.data();
    size_t left   = content.size();
    bool bOk      = true;
    while (left > 0) {
      DWORD chunk = (DWORD) ((left > 0x10000000) ? 0x10000000 : left);
      DWORD written = 0;
      if (!WriteFile(h, p, chunk, &written, nullptr) || (0 == written)) {
        bOk = false;
        break;
      }
      p += written;
      left -= written;
    }
    if (bOk && !FlushFileBuffers(h)) {
      bOk = false;
    }
    if (!CloseHandle(h)) {
      bOk = false;
    }
    if (!bOk) {
      DeleteFileW(mdf_widePath(tempPath).c_str());
      err = "Unable to write temporary file " + tempPath;
      return false;
    }
    return true;
  }
  err = "Unable to create a unique temporary file for " + path;
  return false;
#else
  // Temporary file in the same folder: ".<name>.XXXXXX"
  std::string dir;
  std::string name = path;
  size_t slash     = path.find_last_of('/');
  if (std::string::npos != slash) {
    dir  = path.substr(0, slash + 1);
    name = path.substr(slash + 1);
  }
  std::string templ = dir + "." + name + ".XXXXXX";
  std::vector<char> buf(templ.begin(), templ.end());
  buf.push_back('\0');
  int fd = mkstemp(buf.data());
  if (-1 == fd) {
    err = "Unable to create temporary file in the folder of " + path + " (" + strerror(errno) + ")";
    return false;
  }
  tempPath = buf.data();

  // Keep the permissions of the file that is replaced. A new file gets the
  // permissions a normally created file would get.
  struct stat st;
  if (0 == stat(path.c_str(), &st)) {
    fchmod(fd, st.st_mode & 07777);
  }
  else {
    mode_t mask = umask(0);
    umask(mask);
    fchmod(fd, 0666 & ~mask);
  }

  const char *p = content.data();
  size_t left   = content.size();
  while (left > 0) {
    ssize_t written = write(fd, p, left);
    if (written < 0) {
      if (EINTR == errno) {
        continue;
      }
      err = "Unable to write temporary file " + tempPath + " (" + strerror(errno) + ")";
      close(fd);
      unlink(tempPath.c_str());
      return false;
    }
    p += written;
    left -= (size_t) written;
  }

  if (0 != fsync(fd)) {
    err = "Unable to flush temporary file " + tempPath + " (" + strerror(errno) + ")";
    close(fd);
    unlink(tempPath.c_str());
    return false;
  }
  if (0 != close(fd)) {
    err = "Unable to close temporary file " + tempPath + " (" + strerror(errno) + ")";
    unlink(tempPath.c_str());
    return false;
  }
  return true;
#endif
}

// Replace path with tempPath in one step (the old content or the new, never a
// mix or an empty file).
static bool
mdf_replaceFile(const std::string &tempPath, const std::string &path, std::string &err)
{
#ifdef WIN32
  if (!MoveFileExW(mdf_widePath(tempPath).c_str(),
                   mdf_widePath(path).c_str(),
                   MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
    err = "Unable to replace " + path + " (Windows error " + std::to_string(GetLastError()) + ")";
    return false;
  }
  return true;
#else
  if (0 != rename(tempPath.c_str(), path.c_str())) {
    err = "Unable to replace " + path + " (" + strerror(errno) + ")";
    return false;
  }
  // Make the rename itself durable
  std::string dir = ".";
  size_t slash    = path.find_last_of('/');
  if (std::string::npos != slash) {
    dir = (0 == slash) ? "/" : path.substr(0, slash);
  }
  int dfd = open(dir.c_str(), O_RDONLY);
  if (dfd >= 0) {
    fsync(dfd);
    close(dfd);
  }
  return true;
#endif
}

static void
mdf_removeFile(const std::string &path)
{
#ifdef WIN32
  DeleteFileW(mdf_widePath(path).c_str());
#else
  unlink(path.c_str());
#endif
}

// First line where two texts differ (for error messages)
static std::string
mdf_firstDifference(const std::string &a, const std::string &b)
{
  std::istringstream sa(a);
  std::istringstream sb(b);
  std::string la, lb;
  int line = 0;
  while (true) {
    bool ba = (bool) std::getline(sa, la);
    bool bb = (bool) std::getline(sb, lb);
    line++;
    if (!ba && !bb) {
      return "";
    }
    if (!ba || !bb || (la != lb)) {
      vscp_trim(la);
      vscp_trim(lb);
      return "line " + std::to_string(line) + ": expected '" + la + "', got '" + lb + "'";
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//  save
//

int
CMDF::save(const std::string &path, mdf_format format, bool bVerify)
{
  m_strLastError.clear();

  if (path.empty()) {
    m_strLastError = "No file name given";
    return VSCP_ERROR_INVALID_PATH;
  }

  std::string content;
  try {
    if (MDF_FORMAT_XML == format) {
      content = toXmlString();
    }
    else if (MDF_FORMAT_JSON == format) {
      content = toJsonString();
    }
    else {
      m_strLastError = "Unknown MDF format";
      return VSCP_ERROR_INVALID_SYNTAX;
    }
  }
  catch (const std::exception &ex) {
    m_strLastError = std::string("Failed to create MDF content: ") + ex.what();
    return VSCP_ERROR_ERROR;
  }

  // The new content goes to a temporary file first. The file that is saved
  // over is only replaced when everything has been written (and verified),
  // so a failure never leaves it empty or half written.
  std::string tempPath;
  if (!mdf_writeTempFile(path, content, tempPath, m_strLastError)) {
    spdlog::error("MDF save: {}", m_strLastError);
    return VSCP_ERROR_WRITE;
  }

  if (bVerify) {
    // Read the written file back. It must give exactly the content we have.
    CMDF check;
    int rv = check.parseMDF(tempPath);
    std::string diff;
    if (VSCP_ERROR_SUCCESS != rv) {
      m_strLastError = "The saved file could not be read back (" + check.getLastError() + "). " + path +
                       " has not been changed.";
    }
    else {
      std::string expected;
      std::string got;
      try {
        expected = toJsonString();
        got      = check.toJsonString();
      }
      catch (const std::exception &ex) {
        got = std::string("<error: ") + ex.what() + ">";
      }
      if (expected != got) {
        m_strLastError = "The saved file does not read back the same (" + mdf_firstDifference(expected, got) +
                         "). " + path + " has not been changed.";
      }
    }
    if (!m_strLastError.empty()) {
      spdlog::error("MDF save: {}", m_strLastError);
      mdf_removeFile(tempPath);
      return VSCP_ERROR_VERIFY;
    }
  }

  if (!mdf_replaceFile(tempPath, path, m_strLastError)) {
    spdlog::error("MDF save: {}", m_strLastError);
    mdf_removeFile(tempPath);
    return VSCP_ERROR_WRITE;
  }

  return VSCP_ERROR_SUCCESS;
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

  str     = strWork;
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
// getModuleInfoUrl
//

std::string
CMDF::getModuleInfoUrl(std::string language)
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
    // std::cout << "Attribute: " << attribute << " size: " << pvaluelist->size() << std::endl;

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

  bool bMax = false; // max given?
  for (int i = 0; attr[i]; i += 2) {

    std::string attribute = attr[i + 1];
    vscp_trim(attribute);

    if (0 == strcasecmp(attr[i], "name")) {
      vscp_trim(attribute);
      pBits->m_name = attribute;
    }
    else if (0 == strcasecmp(attr[i], "pos")) {
      // pBits->m_pos = vscp_readStringValue(attribute);
      pBits->setPos(vscp_readStringValue(attribute));
    }
    else if (0 == strcasecmp(attr[i], "width")) {
      // pBits->m_width = vscp_readStringValue(attribute);
      pBits->setWidth(vscp_readStringValue(attribute));
    }
    else if (0 == strcasecmp(attr[i], "default")) {
      if (0 == strcasecmp(attribute.c_str(), "true")) {
        pBits->m_default = true;
      }
      else if (0 == strcasecmp(attribute.c_str(), "false")) {
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
      bMax         = true;
    }
    else if (0 == strcasecmp(attr[i], "access")) {
      // Register access
      std::string strAccess = attribute;
      vscp_trim(strAccess);
      vscp_makeLower(strAccess);
      pBits->m_access = mdf_parseAccess(strAccess);
    }
  }

  // Without a max the largest value the bit field can hold is the max
  if (!bMax) {
    pBits->m_max = (uint8_t) ((1u << pBits->m_width) - 1);
  }
  return true;
}

// ----------------------------------------------------------------------------

// std::string gToken;        // Current token                                 (gTokenList.front())
// std::string gTokenParent;  // This is the parent to the current token       (gTokenList.at(1))
// XML parser state. Per thread (so MDF files can be parsed in different
// threads at the same time) and reset at the start of every parse.
static thread_local std::string gLastLanguage; // Last language ISO two diget code (name/description)

static thread_local std::deque<std::string> gTokenList;

static thread_local int gdepth_xml_parser = 0;

// Character data collected for the current element. Expat may deliver the
// text of one element in several pieces (buffer boundaries, entities like
// &amp;) so it is collected and handled once.
static thread_local std::string gCharData;

// Set at the <reg> start tag when the register type given by the
// attributes is DMATRIX1 or BLOCK. The register is expanded at its end tag.
static thread_local bool gbExpandRegister = false;

// Set when <vscp> is the root element / <vscp><module> has been seen. A file
// without them is not an MDF (an HTML error page for example).
static thread_local bool gbSawVscpRoot = false;
static thread_local bool gbSawModule   = false;

// bool gbVscp   = false;  (gTokenList.back() == "vscp")
// bool gbModule = false;  (gTokenList.at(gTokenList.size()-2) == "module")

static thread_local CMDF_Item *gpItemStruct = nullptr;                   // Holds temporary items
static thread_local CMDF_Bit *gpBitStruct = nullptr;                     // Holds temporary bits
static thread_local CMDF_Value *gpValueStruct = nullptr;                 // Holds temporary values
static thread_local CMDF_Picture *gpPictureStruct = nullptr;             // Holds temporary picture items
static thread_local CMDF_Video *gpVideoStruct = nullptr;                 // Holds temporary video items
static thread_local CMDF_Firmware *gpFirmwareStruct = nullptr;           // Holds temporary firmware items
static thread_local CMDF_Driver *gpDriverStruct = nullptr;               // Holds temporary driver items
static thread_local CMDF_Manual *gpManualStruct = nullptr;               // Holds temporary manual items
static thread_local CMDF_Setup *gpSetupStruct = nullptr;                 // Holds temporary setup items
static thread_local CMDF_Register *gpRegisterStruct = nullptr;           // Holds temporary register items
static thread_local CMDF_RemoteVariable *gpRvarStruct = nullptr;         // Holds temporary remote variable items
static thread_local CMDF_ActionParameter *gpActionParamStruct = nullptr; // Holds temporary action parameter items
static thread_local CMDF_Action *gpActionStruct = nullptr;               // Holds temporary action items
static thread_local CMDF_Event *gpEventStruct = nullptr;                 // Holds temporary event items
static thread_local CMDF_EventData *gpEventDataStruct = nullptr;         // Holds temporary event data items

void
__handleMDFParserData(void *data, const XML_Char *content, int length);

// Reset all XML parser state
static void
__resetMDFParserState(void)
{
  gdepth_xml_parser = 0;
  gLastLanguage     = "en";
  gTokenList.clear();
  gCharData.clear();
  gbExpandRegister = false;
  gbSawVscpRoot    = false;
  gbSawModule      = false;

  gpItemStruct        = nullptr;
  gpBitStruct         = nullptr;
  gpValueStruct       = nullptr;
  gpPictureStruct     = nullptr;
  gpVideoStruct       = nullptr;
  gpFirmwareStruct    = nullptr;
  gpDriverStruct      = nullptr;
  gpManualStruct      = nullptr;
  gpSetupStruct       = nullptr;
  gpRegisterStruct    = nullptr;
  gpRvarStruct        = nullptr;
  gpActionParamStruct = nullptr;
  gpActionStruct      = nullptr;
  gpEventStruct       = nullptr;
  gpEventDataStruct   = nullptr;
}

// Collect character data (handled in __flushMDFParserData)
static void
__collectMDFParserData(void * /*data*/, const XML_Char *content, int length)
{
  if ((nullptr != content) && (length > 0)) {
    gCharData.append(content, (size_t) length);
  }
}

// Handle collected character data for the element that is current now
// (called when a child element starts and when the element ends).
// The text is handed over one line at a time as expat did before, so
// multi line descriptions are joined exactly as earlier. Pieces of one
// line (split by expat at entities or buffer boundaries) are kept together.
static void
__flushMDFParserData(void *data)
{
  if (gCharData.empty()) {
    return;
  }

  std::string text;
  text.swap(gCharData);

  // Text spread over several lines (indented in the file) is one text: the
  // lines are trimmed and joined with a space (they used to be joined
  // without one, which glued the last word of a line to the first word of
  // the next).
  std::string joined;
  size_t start = 0;
  while (start <= text.size()) {
    size_t end = text.find('\n', start);
    if (std::string::npos == end) {
      end = text.size();
    }
    std::string line = text.substr(start, end - start);
    vscp_trim(line);
    if (!line.empty()) {
      if (!joined.empty()) {
        joined += " ";
      }
      joined += line;
    }
    start = end + 1;
  }
  if (!joined.empty()) {
    __handleMDFParserData(data, joined.data(), (int) joined.size());
  }
}

// Defaults for a remote variable embedded in a register: everything not
// given for the remote variable comes from the register.
static void
mdf_remoteVarFromRegister(CMDF_RemoteVariable *prvar, CMDF_Register *preg)
{
  prvar->setName(preg->getName());
  prvar->setType(remote_variable_type_uint8_t);
  prvar->setPage(preg->getPage());
  prvar->setOffset(preg->getOffset());
  prvar->setAccess(preg->getAccess());
  prvar->setDefault(preg->getDefault());
  prvar->setForegroundColor(preg->getForegroundColor());
  prvar->setBackgroundColor(preg->getBackgroundColor());
  if (nullptr != preg->getMapDescription()) {
    *prvar->getMapDescription() = *preg->getMapDescription();
  }
  if (nullptr != preg->getMapInfoUrl()) {
    *prvar->getMapInfoUrl() = *preg->getMapInfoUrl();
  }
}

// Version of a file item: a "version" attribute, or "major.minor.subminor"
// from version_major/version_minor/version_subminor (or version_patch).
static void
mdf_xmlFileVersion(const char **attr, std::string &version)
{
  std::string major, minor, patch;
  bool bParts = false;
  for (int i = 0; attr[i]; i += 2) {
    std::string attribute = attr[i + 1];
    vscp_trim(attribute);
    if (0 == strcasecmp(attr[i], "version")) {
      version = attribute;
      return;
    }
    else if (0 == strcasecmp(attr[i], "version_major")) {
      major  = std::to_string(vscp_readStringValue(attribute));
      bParts = true;
    }
    else if (0 == strcasecmp(attr[i], "version_minor")) {
      minor  = std::to_string(vscp_readStringValue(attribute));
      bParts = true;
    }
    else if ((0 == strcasecmp(attr[i], "version_subminor")) || (0 == strcasecmp(attr[i], "version_patch"))) {
      patch  = std::to_string(vscp_readStringValue(attribute));
      bParts = true;
    }
  }
  if (bParts) {
    version = (major.empty() ? "0" : major) + "." + (minor.empty() ? "0" : minor) + "." + (patch.empty() ? "0" : patch);
  }
}

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

  // Text of the parent element seen so far belongs to the parent
  __flushMDFParserData(data);

  // The end handler always pops a token and decrements the depth, so the
  // depth must be incremented on every return path from here on (the
  // early returns below used to skip it and the parser state went bad).
  struct DepthGuard {
    ~DepthGuard() { gdepth_xml_parser++; }
  } depthGuard;

  // Attributes of a remote variable (in <remotevars> or embedded in a <reg>)
  auto xmlRvarAttributes = [&](CMDF_RemoteVariable *prv) {
    for (int i = 0; attr[i]; i += 2) {

      std::string attribute = attr[i + 1];
      vscp_trim(attribute);

      if (0 == strcasecmp(attr[i], "name")) {
        // Remote variable name
        spdlog::trace("Parse-XML: handleMDFParserData: Remote variable name: {0}", attribute);
        prv->m_name = attribute;
      }
      else if (0 == strcasecmp(attr[i], "default")) {
        // Remote variable default value
        spdlog::trace("Parse-XML: handleMDFParserData: Remote variable default: {0}", attribute);
        prv->m_strDefault = mdf_normalizeDefault(attribute);
      }
      else if (0 == strcasecmp(attr[i], "min")) {
        prv->m_strMin = attribute;
      }
      else if (0 == strcasecmp(attr[i], "max")) {
        prv->m_strMax = attribute;
      }
      else if (0 == strcasecmp(attr[i], "bitpos")) {
        // Remote variable bit position for boolean
        spdlog::trace("Parse-XML: handleMDFParserData: Remote variable bitpos: {0}", attribute);
        prv->m_bitpos = vscp_readStringValue(attribute) & 7;
      }
      else if (0 == strcasecmp(attr[i], "size")) {
        // Remote variable string max size
        spdlog::trace("Parse-XML: handleMDFParserData: Remote variable string max size: {0}", attribute);
        prv->m_size = vscp_readStringValue(attribute);
      }
      else if ((0 == strcasecmp(attr[i], "access")) || (0 == strcasecmp(attr[i], "acess"))) {
        // Remote variable access ("acess" is a misspelling in the specification examples)
        spdlog::trace("Parse-XML: handleMDFParserData: Remote variable access: {0}", attribute);
        prv->m_access = mdf_parseAccess(attribute);
      }
      else if (0 == strcasecmp(attr[i], "type")) {
        // Remote variable type
        spdlog::trace("Parse-XML: handleMDFParserData: Remote variable type: {0}", attribute);
        vscp_makeLower(attribute); // keyword compare only, not stored
        if (attribute == "string") {
          prv->m_type = remote_variable_type_string;
          spdlog::trace("Parse-XML: Remote variable type set to 'string' {0}.", (int)prv->m_type);
        }
        else if (attribute == "bool") {
          prv->m_type = remote_variable_type_boolean;
          spdlog::trace("Parse-XML: Remote variable type set to 'boolena' {0}.", (int)prv->m_type);
        }
        else if ((attribute == "int8") || (attribute == "int8_t") || (attribute == "char")) {
          prv->m_type = remote_variable_type_int8_t;
          spdlog::trace("Parse-XML: Remote variable type set to 'int8_t' {0}.", (int)prv->m_type);
        }
        else if ((attribute == "uint8") || (attribute == "uint8_t") || (attribute == "byte")) {
          prv->m_type = remote_variable_type_uint8_t;
          spdlog::trace("Parse-XML: Remote variable type set to 'uint8_t' {0}.", (int)prv->m_type);
        }
        else if ((attribute == "int16") || (attribute == "int16_t") || (attribute == "short")) {
          prv->m_type = remote_variable_type_int16_t;
          spdlog::trace("Parse-XML: Remote variable type set to 'int16_t' {0}.", (int)prv->m_type);
        }
        else if ((attribute == "uint16") || (attribute == "uint16_t")) {
          prv->m_type = remote_variable_type_uint16_t;
          spdlog::trace("Parse-XML: Remote variable type set to 'uint16_t' {0}.", (int)prv->m_type);
        }
        else if ((attribute == "int32") || (attribute == "int32_t") || (attribute == "long")) {
          prv->m_type = remote_variable_type_int32_t;
          spdlog::trace("Parse-XML: Remote variable type set to 'int32_t' {0}.", (int)prv->m_type);
        }
        else if ((attribute == "uint32") || (attribute == "uint32_t") || (attribute == "unsigned")) {
          prv->m_type = remote_variable_type_uint32_t;
          spdlog::trace("Parse-XML: Remote variable type set to 'uint32_t' {0}.", (int)prv->m_type);
        }
        else if ((attribute == "int64") || (attribute == "int64_t") || (attribute == "longlong")) {
          prv->m_type = remote_variable_type_int64_t;
          spdlog::trace("Parse-XML: Remote variable type set to 'int64_t' {0}.", (int)prv->m_type);
        }
        else if ((attribute == "uint64") || (attribute == "uint64_t")) {
          prv->m_type = remote_variable_type_uint64_t;
          spdlog::trace("Parse-XML: Remote variable type set to 'uint64_t' {0}.", (int)prv->m_type);
        }
        else if (attribute == "float") {
          prv->m_type = remote_variable_type_float;
          spdlog::trace("Parse-XML: Remote variable type set to 'float' {0}.", (int)prv->m_type);
        }
        else if (attribute == "double") {
          prv->m_type = remote_variable_type_double;
          spdlog::trace("Parse-XML: Remote variable type set to 'double' {0}.", (int)prv->m_type);
        }
        else if (attribute == "date") {
          prv->m_type = remote_variable_type_date;
          spdlog::trace("Parse-XML: Remote variable type set to 'date' {0}.", (int)prv->m_type);
        }
        else if (attribute == "time") {
          prv->m_type = remote_variable_type_time;
          spdlog::trace("Parse-XML: Remote variable type set to 'time' {0}.", (int)prv->m_type);
        }
        else {
          prv->m_type = remote_variable_type_unknown;
        }
      }
      else if (0 == strcasecmp(attr[i], "page")) {
        // Register page
        spdlog::trace("Parse-XML: handleMDFParserData: Remote variable fgcolor: {0}", attribute);
        prv->m_page = vscp_readStringValue(attribute);
      }
      else if (0 == strcasecmp(attr[i], "offset")) {
        // Register offset
        spdlog::trace("Parse-XML: handleMDFParserData: Remote variable fgcolor: {0}", attribute);
        prv->m_offset = vscp_readStringValue(attribute);
      }
      else if (0 == strcasecmp(attr[i], "fgcolor")) {
        // Register foreground color
        spdlog::trace("Parse-XML: handleMDFParserData: Remote variable fgcolor: {0}", attribute);
        prv->m_fgcolor = vscp_readStringValue(attribute);
      }
      else if (0 == strcasecmp(attr[i], "bgcolor")) {
        // Register background color
        spdlog::trace("Parse-XML: handleMDFParserData: Remote variable bgcolor: {0}", attribute);
        prv->m_bgcolor = vscp_readStringValue(attribute);
      }
    }
  };

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
      if (0 == strcasecmp(attr[i], "lang")) {
        if (!attribute.empty()) {
          gLastLanguage = attribute;
        }
      }
    }
  }

  // Verify structure <vscp><module>.....</module></vscp>
  if (((gdepth_xml_parser >= 2) && (gTokenList.at(gTokenList.size() - 2) != "module")) || 
      (gTokenList.back() != "vscp")) {
    spdlog::error("Parse-XML: startSetupMDFParser: Invalid structure");
    return;
  }

  switch (gdepth_xml_parser) {

    case 0: // Root
      if (currentToken == "vscp") {
        gbSawVscpRoot = true;
      }
      break;

    case 1:
      if ((gTokenList.back() == "vscp") && (currentToken == "module")) {
        gbSawModule = true;
      }
      // <vscp><redirect mdf-path="url"/>
      else if ((gTokenList.back() == "vscp") && (currentToken == "redirect")) {
        for (int i = 0; attr[i]; i += 2) {
          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          if ((0 == strcasecmp(attr[i], "mdf-path")) || (0 == strcasecmp(attr[i], "url")) ||
              (0 == strcasecmp(attr[i], "path"))) {
            pmdf->m_redirectUrl = attribute;
          }
        }
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
        mdf_xmlFileVersion(attr, gpPictureStruct->m_strVersion);

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
          else if (0 == strcasecmp(attr[i], "date")) {
            // Date for picture
            spdlog::trace("Parse-XML: handleMDFParserData: Module picture date: {0}", attribute);
            gpPictureStruct->m_strDate = attribute;
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
          else if ((0 == strcasecmp(attr[i], "md5sum")) || (0 == strcasecmp(attr[i], "md5"))) {
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
        mdf_xmlFileVersion(attr, gpManualStruct->m_strVersion);

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
          else if ((0 == strcasecmp(attr[i], "lang")) || (0 == strcasecmp(attr[i], "language"))) {
            // Language for manual
            spdlog::trace("Parse-XML: handleMDFParserData: Module manual language: {0}", attribute);
            gpManualStruct->m_strLanguage = attribute;
          }
          else if (0 == strcasecmp(attr[i], "format")) {
            // Format for manual
            spdlog::trace("Parse-XML: handleMDFParserData: Module manual format: {0}", attribute);
            gpManualStruct->m_strFormat = attribute;
          }
          else if (0 == strcasecmp(attr[i], "date")) {
            // Date for manula
            spdlog::trace("Parse-XML: handleMDFParserData: Module manialö date: {0}", attribute);
            gpManualStruct->m_strDate = attribute;
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
        // Attribute form: <dmatrix level="1" start-page="3" start-offset="0"
        //                          rowcnt="4" rowsize="8">
        // (the child element form is handled in the data handler)
        for (int i = 0; attr[i]; i += 2) {
          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          if (0 == strcasecmp(attr[i], "level")) {
            pmdf->m_dmInfo.m_level = vscp_readStringValue(attribute);
          }
          else if ((0 == strcasecmp(attr[i], "start-page")) || (0 == strcasecmp(attr[i], "startpage")) ||
                   (0 == strcasecmp(attr[i], "page"))) {
            pmdf->m_dmInfo.m_startPage = vscp_readStringValue(attribute);
          }
          else if ((0 == strcasecmp(attr[i], "start-offset")) || (0 == strcasecmp(attr[i], "startoffset")) ||
                   (0 == strcasecmp(attr[i], "offset"))) {
            pmdf->m_dmInfo.m_startOffset = vscp_readStringValue(attribute);
          }
          else if ((0 == strcasecmp(attr[i], "rowcnt")) || (0 == strcasecmp(attr[i], "rowcount"))) {
            pmdf->m_dmInfo.m_rowCount = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "rowsize")) {
            pmdf->m_dmInfo.m_rowSize = vscp_readStringValue(attribute);
          }
        }
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
      else if (((currentToken == "telephone") || (currentToken == "fax") || (currentToken == "email") ||
                (currentToken == "web") || (currentToken == "social")) &&
               (gTokenList.at(1) == "manufacturer")) {
        // An item that was started but never ended (damaged file) is dropped
        // here instead of leaking
        delete gpItemStruct;
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
        mdf_xmlFileVersion(attr, gpPictureStruct->m_strVersion);

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
          else if (0 == strcasecmp(attr[i], "date")) {
            // Date for picture
            spdlog::trace("Parse-XML: handleMDFParserData: Module picture date: {0}", attribute);
            gpPictureStruct->m_strDate = attribute;
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
        mdf_xmlFileVersion(attr, gpVideoStruct->m_strVersion);

        for (int i = 0; attr[i]; i += 2) {
          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          //vscp_makeLower(attribute);

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
          else if (0 == strcasecmp(attr[i], "date")) {
            // Date for video
            spdlog::trace("Parse-XML: handleMDFParserData: Module video date: {0}", attribute);
            gpVideoStruct->m_strDate = attribute;
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
          else if ((0 == strcasecmp(attr[i], "md5sum")) || (0 == strcasecmp(attr[i], "md5"))) {
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
          //vscp_makeLower(attribute);

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
          else if ((0 == strcasecmp(attr[i], "md5sum")) || (0 == strcasecmp(attr[i], "md5"))) {
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
          else if (0 == strcasecmp(attr[i], "architecture")) {
            // Processor for driver
            spdlog::trace("Parse-XML: handleMDFParserData: Driver architecture: {0}", attribute);
            gpDriverStruct->m_strArchitecture = attribute;
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
        mdf_xmlFileVersion(attr, gpManualStruct->m_strVersion);

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
          else if ((0 == strcasecmp(attr[i], "lang")) || (0 == strcasecmp(attr[i], "language"))) {
            // Language for manual
            spdlog::trace("Parse-XML: handleMDFParserData: Module manual language: {0}", attribute);
            gpManualStruct->m_strLanguage = attribute;
          }
          else if (0 == strcasecmp(attr[i], "format")) {
            // Format for manual
            spdlog::trace("Parse-XML: handleMDFParserData: Module manual format: {0}", attribute);
            gpManualStruct->m_strFormat = attribute;
          }
          else if (0 == strcasecmp(attr[i], "date")) {
            // Date for manual
            spdlog::trace("Parse-XML: handleMDFParserData: Module manual date: {0}", attribute);
            gpManualStruct->m_strDate = attribute;
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
        mdf_xmlFileVersion(attr, gpSetupStruct->m_strVersion);

        for (int i = 0; attr[i]; i += 2) {
          std::string attribute = attr[i + 1];
          vscp_trim(attribute);
          //vscp_makeLower(attribute);

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
          else if (0 == strcasecmp(attr[i], "date")) {
            // Date for setup
            spdlog::trace("Parse-XML: handleMDFParserData: Module setup date: {0}", attribute);
            gpSetupStruct->m_strDate = attribute;
          }
          else if (0 == strcasecmp(attr[i], "version")) {
            gpSetupStruct->m_strVersion = attribute;
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

        // Get register attributes
        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);

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
          else if ((0 == strcasecmp(attr[i], "span")) || (0 == strcasecmp(attr[i], "size"))) {
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
            gpRegisterStruct->m_strDefault = mdf_normalizeDefault(attribute);
          }
          else if (0 == strcasecmp(attr[i], "access")) {
            // Register access
            std::string strAccess = attribute;
            vscp_trim(strAccess);
            vscp_makeLower(strAccess);
            gpRegisterStruct->m_access = mdf_parseAccess(strAccess);
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
              spdlog::trace("Parse-XML: Register type: standard");
            }
            else if ((strRegType == "dmatrix") || (strRegType == "dmatrix1") || (strRegType == "dm")) {
              gpRegisterStruct->m_type = MDF_REG_TYPE_DMATRIX1;
              spdlog::trace("Parse-XML: Register type: dmatrix");
            }
            else if ((strRegType == "block") || (strRegType == "blk")) {
              gpRegisterStruct->m_type = MDF_REG_TYPE_BLOCK;
              spdlog::trace("Parse-XML: Register type: dmatrix");
            }
            else {
              gpRegisterStruct->m_type = static_cast<mdf_register_type>(vscp_readStringValue(attribute));
              spdlog::trace("Parse-XML: Register type: {0}", (int)gpRegisterStruct->m_type);
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
          else if (0 == strcasecmp(attr[i], "evenfg")) {
            // Register even foreground color
            spdlog::trace("Parse-XML: handleMDFParserData: Register evenfg: {0}", attribute);
            gpRegisterStruct->m_fgeven = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "evenbg")) {
            // Register even background color
            spdlog::trace("Parse-XML: handleMDFParserData: Register evenbg: {0}", attribute);
            gpRegisterStruct->m_bgeven = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "oddfg")) {
            // Register odd foreground color
            spdlog::trace("Parse-XML: handleMDFParserData: Register oddfg: {0}", attribute);
            gpRegisterStruct->m_fgodd = vscp_readStringValue(attribute);
          }
          else if (0 == strcasecmp(attr[i], "oddbg")) {
            // Register odd background color
            spdlog::trace("Parse-XML: handleMDFParserData: Register oddbg: {0}", attribute);
            gpRegisterStruct->m_bgodd = vscp_readStringValue(attribute);
          }
        } // Register attributes

        // Store the register. DMATRIX1/BLOCK registers (type given as an
        // attribute, as before) are expanded to one register per byte of the
        // span at the end tag, when the bit and value definitions are known
        // (copies made here had none).
        gbExpandRegister = (MDF_REG_TYPE_DMATRIX1 == gpRegisterStruct->m_type) ||
                           (MDF_REG_TYPE_BLOCK == gpRegisterStruct->m_type);
        pmdf->m_list_register.push_back(gpRegisterStruct);
      }
      // [3] remotevar  (rvar definitions)
      else if ((currentToken == "remotevar") || (currentToken == "abstraction")) {
        gpRvarStruct = new CMDF_RemoteVariable;
        if (nullptr == gpRvarStruct) {
          spdlog::error("Parse-XML: ---> startSetupMDFParser: Failed to allocate memory for remote variable");
          return;
        }

        pmdf->m_list_remotevar.push_back(gpRvarStruct);

        // Get register attributes
        xmlRvarAttributes(gpRvarStruct);
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
          else if ((0 == strcasecmp(attr[i], "direction")) || (0 == strcasecmp(attr[i], "dir"))) {
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
              spdlog::trace("Parse-XML: Event priority set to low.");
            }
            else if (attribute == "normal") {
              gpEventStruct->m_priority = VSCP_PRIORITY_NORMAL >> 5;
              spdlog::trace("Parse-XML: Event priority set to medium.");
            }
            else if (attribute == "medium") {
              gpEventStruct->m_priority = VSCP_PRIORITY_MEDIUM >> 5;
              spdlog::trace("Parse-XML: Event priority set to medium.");
            }
            else if (attribute == "high") {
              gpEventStruct->m_priority = VSCP_PRIORITY_HIGH >> 5;
              spdlog::trace("Parse-XML: Event priority set to high.");
            }
            else {
              gpEventStruct->m_priority = vscp_readStringValue(attribute) & 7;
              spdlog::trace("Parse-XML: Event priority set to {0}.", gpEventStruct->m_priority);
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
        <reg name="..." page="0" offset="10">
          <remotevar name="..." type="int16_t"/>     <------
        </reg>
        A remote variable embedded in a register. What is not given is taken
        from the register (specification).
      */
      else if (((currentToken == "remotevar") || (currentToken == "abstraction")) &&
               (gTokenList.at(1) == "reg") &&
               (gpRegisterStruct != nullptr)) {
        gpRvarStruct = new CMDF_RemoteVariable;
        mdf_remoteVarFromRegister(gpRvarStruct, gpRegisterStruct);
        pmdf->m_list_remotevar.push_back(gpRvarStruct);
        xmlRvarAttributes(gpRvarStruct);
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
        // Without an offset attribute the position is the offset
        gpEventDataStruct->m_offset = (uint16_t) (gpEventStruct->m_list_eventdata.size() - 1);

        // Get register attributes
        for (int i = 0; attr[i]; i += 2) {

          std::string attribute = attr[i + 1];
          vscp_trim(attribute);

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

  // gdepth_xml_parser is incremented by depthGuard
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
  if (gTokenList.empty() || !(gTokenList.back() == "vscp")) {
    spdlog::error("Parse-XML: ---> handleMDFParserData: No vscp tag");
    return;
  }

  std::string strContent = std::string(content, length);
  vscp_trim(strContent);
  if (strContent.empty()) {
    return;
  }

  spdlog::trace("Parse-XML: XML Data: {0}", strContent);

  // <vscp><redirect>url</redirect></vscp>
  if ((2 == gTokenList.size()) && (gTokenList.at(0) == "redirect")) {
    pmdf->m_redirectUrl = strContent;
    return;
  }

  switch (gdepth_xml_parser) {

    case 1: // On module level
      if (gTokenList.at(0) == "redirect") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module redirect: {0}", strContent);
        pmdf->m_redirectUrl = strContent;
      }
      else if (gTokenList.at(0) == "name") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module name: {0}", strContent);
        pmdf->m_name = strContent;
      }
      else if (gTokenList.at(0) == "level") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module level: {0}", strContent);
        pmdf->m_vscpLevel = mdf_levelFromFile(vscp_readStringValue(strContent));
      }
      break;

    case 2:

      break;

    case 3:

      //std::cout << "3 - " << gTokenList.at(2) << " " <<  gTokenList.at(1) << " " << gTokenList.at(0) << std::endl;

      if (gTokenList.at(0) == "name") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module name: {0}", strContent);
        vscp_trim(strContent);
        pmdf->m_name = strContent;
      }
      else if (gTokenList.at(0) == "level") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module level: {0}", strContent);
        pmdf->m_vscpLevel = mdf_levelFromFile(vscp_readStringValue(strContent));
      }
      else if (gTokenList.at(0) == "model") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module name: {0}", strContent);
        pmdf->m_strModule_Model = strContent;
      }
      else if (gTokenList.at(0) == "copyright") {
        spdlog::trace("Parse-XML: handleMDFParserData: Module copyright: {0}", strContent);
        pmdf->m_copyright = strContent;
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
      else if (((gTokenList.at(0) == "rowcnt") || (gTokenList.at(0) == "rowcount")) &&
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
        else if (gTokenList.at(0) == "state") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer address state: {0}", strContent);
          pmdf->m_manufacturer.m_address.m_strState = strContent;
        }
      }
      // [5] manufacturer/telephone
      else if ((gTokenList.at(1) == "telephone") && (gpItemStruct != nullptr)) {
        if (gTokenList.at(0) == "number") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer address telephone number: {0}", strContent);
          vscp_trim(strContent);
          gpItemStruct->m_value = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          spdlog::trace(
            "Parse-XML: handleMDFParserData: Module manufacturer address telephone description: {0} Language: {1}",
            strContent,
            gLastLanguage);
          gpItemStruct->m_mapDescription[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "infourl") {
          gpItemStruct->m_mapInfoURL[gLastLanguage] += strContent;
        }
      }
      // [5] manufacturer/fax
      else if ((gTokenList.at(1) == "fax") && (gpItemStruct != nullptr)) {
        if (gTokenList.at(0) == "number") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer address fax number: {0}", strContent);
          vscp_trim(strContent);
          gpItemStruct->m_value = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer address fax description: {0} Language: {1}",
                        strContent,
                        gLastLanguage);
          gpItemStruct->m_mapDescription[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "infourl") {
          gpItemStruct->m_mapInfoURL[gLastLanguage] += strContent;
        }
      }
      // [5] manufacturer/email
      else if ((gTokenList.at(1) == "email") && (gpItemStruct != nullptr)) {
        if (gTokenList.at(0) == "address") {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer email address: {0}", strContent);
          vscp_trim(strContent);
          gpItemStruct->m_value = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          spdlog::trace(
            "Parse-XML: handleMDFParserData: Module manufacturer email address description: {0} Language: {1}",
            strContent,
            gLastLanguage);
          gpItemStruct->m_mapDescription[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "infourl") {
          gpItemStruct->m_mapInfoURL[gLastLanguage] += strContent;
        }
      }
      // [5] manufacturer/web
      else if ((gTokenList.at(1) == "web") && (gpItemStruct != nullptr)) {
        if ((gTokenList.at(0) == "address") || (gTokenList.at(0) == "url")) {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer web address: {0}", strContent);
          vscp_trim(strContent);
          gpItemStruct->m_value = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          spdlog::trace(
            "Parse-XML: handleMDFParserData: Module manufacturer email web description: {0} Language: {1}",
            strContent,
            gLastLanguage);
          gpItemStruct->m_mapDescription[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "infourl") {
          gpItemStruct->m_mapInfoURL[gLastLanguage] += strContent;
        }
      }
      // [5] manufacturer/social
      else if ((gTokenList.at(1) == "social") && (gpItemStruct != nullptr)) {
        if ((gTokenList.at(0) == "address") || (gTokenList.at(0) == "url")) {
          spdlog::trace("Parse-XML: handleMDFParserData: Module manufacturer social address: {0}", strContent);
          vscp_trim(strContent);
          gpItemStruct->m_value = strContent;
        }
        else if (gTokenList.at(0) == "description") {
          spdlog::trace(
            "Parse-XML: handleMDFParserData: Module manufacturer email social description: {0} Language: {1}",
            strContent,
            gLastLanguage);
          gpItemStruct->m_mapDescription[gLastLanguage] += strContent;
        }
        else if (gTokenList.at(0) == "infourl") {
          gpItemStruct->m_mapInfoURL[gLastLanguage] += strContent;
        }
      }
      // [5] Picture standard format
      else if ((gTokenList.at(1) == "picture") && (gpPictureStruct != nullptr)) {
        if (gTokenList.at(0) == "name") {
          // Description for picture
          spdlog::trace("Parse-XML: handleMDFParserData: Module picture name: {0}",
                        strContent);
          vscp_trim(strContent);
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
        }
        else if (gTokenList.at(0) == "access") {
          // Register access
          std::string strAccess = strContent;
          vscp_trim(strAccess);
          vscp_makeLower(strAccess);
          gpRegisterStruct->m_access = mdf_parseAccess(strAccess);
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
        }
        else if (gTokenList.at(0) == "access") {
          // Register access
          std::string strAccess = strContent;
          vscp_trim(strAccess);
          vscp_makeLower(strAccess);
          gpRvarStruct->m_access = mdf_parseAccess(strAccess);
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
            spdlog::trace("Parse-XML: Event priority set to low.");
          }
          else if (strContent == "normal") {
            gpEventStruct->m_priority = VSCP_PRIORITY_NORMAL >> 5;
            spdlog::trace("Parse-XML: Event priority set to medium.");
          }
          else if (strContent == "medium") {
            gpEventStruct->m_priority = VSCP_PRIORITY_MEDIUM >> 5;
            spdlog::trace("Parse-XML: Event priority set to medium.");
          }
          else if (strContent == "high") {
            gpEventStruct->m_priority = VSCP_PRIORITY_HIGH >> 5;
            spdlog::trace("Parse-XML: Event priority set to high.");
          }
          else {
            gpEventStruct->m_priority = vscp_readStringValue(strContent) & 7;
            spdlog::trace("Parse-XML: Event priority set to {0}.", gpEventStruct->m_priority);
          }
        }
        else if ((gTokenList.at(0) == "direction") || (gTokenList.at(0) == "dir")) {
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
        //int i = 9;
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

  // Handle the text of the element that ends now
  __flushMDFParserData(data);

  // Every start pushed a token. Never pop from an empty list.
  if (gTokenList.empty()) {
    spdlog::error("Parse-XML: endSetupMDFParser: Unbalanced element {0}", name);
    return;
  }

  std::string currentToken = name;
  vscp_trim(currentToken);
  vscp_makeLower(currentToken);

  // End of a remote variable embedded in a register
  if (((currentToken == "remotevar") || (currentToken == "abstraction")) && (gTokenList.size() >= 2) &&
      (gTokenList.at(1) == "reg")) {
    gpRvarStruct = nullptr;
  }

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
      else if ((currentToken == "social") && 
               (gpItemStruct != nullptr)) {
        // Save address data in list
        pmdf->m_manufacturer.m_list_Social.push_back(gpItemStruct);
        gpItemStruct = nullptr;
      }
      else if ((currentToken == "reg") && 
               (gpRegisterStruct != nullptr)) {
        // Register is complete - expand DMATRIX1/BLOCK registers
        if (gbExpandRegister) {
          pmdf->expandRegisterSpan(gpRegisterStruct);
        }
        gbExpandRegister = false;
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
        //int i = 0;
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
          (gpValueStruct != nullptr)) {
        gpValueStruct = nullptr;
      }
      // bit/valuelist/item
      else if ((currentToken == "item") && 
               (gTokenList.at(1) == "valuelist") && 
               (gTokenList.at(2) == "bit") &&
               (gpValueStruct != nullptr)) {
        gpValueStruct = nullptr;
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
CMDF::parseMDF_XML_raw(std::ifstream &ifs)
{
  int rv = VSCP_ERROR_SUCCESS;

  // Init. XML parsing state (per thread)
  __resetMDFParserState();

  // Empty old MDF information
  clearStorage();

  XML_Parser xmlParser = XML_ParserCreate("UTF-8");
  if (nullptr == xmlParser) {
    spdlog::error("ParseXML: Failed to create XML parser.");
    return VSCP_ERROR_MEMORY;
  }
  XML_SetUserData(xmlParser, this);
  XML_SetElementHandler(xmlParser, __startSetupMDFParser, __endSetupMDFParser);
  XML_SetCharacterDataHandler(xmlParser, __collectMDFParserData);

  // A new buffer must be requested from expat before every parse call and
  // the last call must be marked final so a truncated file is detected.
  bool bFinal = false;
  while (!bFinal) {
    void *buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);
    if (nullptr == buf) {
      spdlog::error("ParseXML: Failed to get XML parser buffer.");
      rv = VSCP_ERROR_MEMORY;
      break;
    }

    ifs.read((char *) buf, XML_BUFF_SIZE);
    std::streamsize bytes_read = ifs.gcount();
    if (ifs.bad()) {
      spdlog::error("ParseXML: Failed to read MDF file.");
      rv = VSCP_ERROR_READ;
      break;
    }
    bFinal = !ifs.good(); // EOF reached

    if (XML_STATUS_OK != XML_ParseBuffer(xmlParser, (int) bytes_read, bFinal ? 1 : 0)) {
#if XML_MAJOR_VERSION > 2 || (XML_MAJOR_VERSION == 2 && XML_MINOR_VERSION >= 9)
      const auto lineNumber = XML_GetCurrentLineNumber64(xmlParser);
#else
      const auto lineNumber = XML_GetCurrentLineNumber(xmlParser);
#endif
      spdlog::error("ParseXML: Failed parse XML file at line {0} [{1}].",
                    lineNumber,
                    XML_ErrorString(XML_GetErrorCode(xmlParser)));
      m_strLastError = "XML error at line " + std::to_string((unsigned long long) lineNumber) + ": " +
                       XML_ErrorString(XML_GetErrorCode(xmlParser));
      rv = VSCP_ERROR_PARSING;
      break;
    }
  }

  XML_ParserFree(xmlParser);

  // Items are stored when their end tag is seen. One that was started but
  // not stored (parse error) would leak.
  if (nullptr != gpItemStruct) {
    delete gpItemStruct;
    gpItemStruct = nullptr;
  }

  // A well formed XML file that is not an MDF
  if (VSCP_ERROR_SUCCESS == rv) {
    if (!gbSawVscpRoot) {
      m_strLastError = "Not an MDF file (the root element must be <vscp>)";
      rv             = VSCP_ERROR_PARSING;
    }
    else if (!gbSawModule && m_redirectUrl.empty()) {
      m_strLastError = "Not an MDF file (no <module> or <redirect> in <vscp>)";
      rv             = VSCP_ERROR_PARSING;
    }
  }

  // Do not keep pointers into this MDF after parsing
  __resetMDFParserState();

  return rv;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// getDescriptionList
//

int
CMDF::getDescriptionList(json &j, std::map<std::string, std::string> &map)
{
  auto toJsonString = [](const json &value) -> std::string {
    if (value.is_string()) {
      return value.get<std::string>();
    }
    else if (value.is_number_integer()) {
      return std::to_string(value.get<int64_t>());
    }
    else if (value.is_number_unsigned()) {
      return std::to_string(value.get<uint64_t>());
    }
    else if (value.is_number_float()) {
      return std::to_string(value.get<double>());
    }
    else if (value.is_boolean()) {
      return value.get<bool>() ? "true" : "false";
    }

    return value.dump();
  };

  // Register description
  if (j.contains("description")) {

    if (j["description"].is_string()) {
      map["en"] = j["description"];
      spdlog::debug("Parse-JSON: Description: {0} language: 'en' ", map["en"]);
    }
    else if (j["description"].is_object()) {
      for (auto &item : j["description"].items()) {
        map[item.key()] = toJsonString(item.value());
        spdlog::debug("Parse-JSON: Description: {0} language: '{1}'", map[item.key()], (std::string)item.key());
      }
    }
    // Array of {"lang": "text"} objects. This is the form save_json writes,
    // so files saved by this library can be read back.
    else if (j["description"].is_array()) {
      for (auto &entry : j["description"]) {
        if (!entry.is_object()) {
          continue;
        }
        for (auto &item : entry.items()) {
          map[item.key()] = toJsonString(item.value());
        }
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
  auto toJsonString = [](const json &value) -> std::string {
    if (value.is_string()) {
      return value.get<std::string>();
    }
    else if (value.is_number_integer()) {
      return std::to_string(value.get<int64_t>());
    }
    else if (value.is_number_unsigned()) {
      return std::to_string(value.get<uint64_t>());
    }
    else if (value.is_number_float()) {
      return std::to_string(value.get<double>());
    }
    else if (value.is_boolean()) {
      return value.get<bool>() ? "true" : "false";
    }

    return value.dump();
  };

  // Info URL
  if (j.contains("infourl")) {

    if (j["infourl"].is_string()) {
      map["en"] = j["infourl"];
      spdlog::debug("Parse-JSON: InfoURL: {0} language: 'en' ", map["en"]);
    }
    else if (j["infourl"].is_object()) {
      for (auto &item : j["infourl"].items()) {
        map[item.key()] = toJsonString(item.value());
        spdlog::debug("Parse-JSON: InfoURL: {0} language: '{1}'", map[item.key()], (std::string)item.key());
      }
    }
    // Array of {"lang": "text"} objects. This is the form save_json writes,
    // so files saved by this library can be read back.
    else if (j["infourl"].is_array()) {
      for (auto &entry : j["infourl"]) {
        if (!entry.is_object()) {
          continue;
        }
        for (auto &item : entry.items()) {
          map[item.key()] = toJsonString(item.value());
        }
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
          spdlog::debug("Parse-JSON: Value list name: {0}", pvalue->m_name);
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
          spdlog::debug("Parse-JSON: Bitlist name: {0}", pbit->m_name);
        }

        if (j.contains("pos") && j["pos"].is_number()) {
          // pbit->m_pos = j["pos"];
          pbit->setPos(j["pos"]);
          spdlog::debug("Parse-JSON: Bitlist pos: {0}", pbit->m_pos);
        }
        else if (j.contains("pos") && j["pos"].is_string()) {
          // pbit->m_pos = vscp_readStringValue(j["pos"]);
          pbit->setPos(vscp_readStringValue(j["pos"]));
          spdlog::debug("Parse-JSON: Bitlist pos: {0}", pbit->m_pos);
        }

        if (j.contains("width") && j["width"].is_number()) {
          // pbit->m_width = j["width"];
          pbit->setWidth(j["width"]);
          spdlog::debug("Parse-JSON: Bitlist width: {0}", pbit->m_width);
        }
        else if (j.contains("width") && j["width"].is_string()) {
          // pbit->m_width = vscp_readStringValue(j["width"]);
          pbit->setWidth(vscp_readStringValue(j["width"]));
          spdlog::debug("Parse-JSON: Bitlist width: {0}", pbit->m_width);
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
          std::string str = j["default"];
          vscp_trim(str);
          if (0 == strcasecmp(str.c_str(), "true")) {
            pbit->m_default = 1;
          }
          else if (0 == strcasecmp(str.c_str(), "false")) {
            pbit->m_default = 0;
          }
          else {
            pbit->m_default = vscp_readStringValue(str);
          }
          spdlog::debug("Parse-JSON: Bitlist default: {0}", pbit->m_default);
        }

        if (j.contains("min") && j["min"].is_number()) {
          pbit->m_min = j["min"];
          spdlog::debug("Parse-JSON: Bitlist min: {0}", pbit->m_min);
        }
        else if (j.contains("min") && j["min"].is_string()) {
          pbit->m_min = vscp_readStringValue(j["min"]);
          spdlog::debug("Parse-JSON: Bitlist min: {0}", pbit->m_min);
        }

        if (j.contains("max") && j["max"].is_number()) {
          pbit->m_max = j["max"];
          spdlog::debug("Parse-JSON: Bitlist max: {0}", pbit->m_max);
        }
        else if (j.contains("max") && j["max"].is_string()) {
          pbit->m_max = vscp_readStringValue(j["max"]);
          spdlog::debug("Parse-JSON: Bitlist max: {0}", pbit->m_max);
        }

        if (j.contains("access") && j["access"].is_string()) {
          std::string strAccess = j["access"];
          vscp_trim(strAccess);
          vscp_makeLower(strAccess);
          pbit->m_access = mdf_parseAccess(strAccess);
        }

        // Get description
        // Without a max the largest value the bit field can hold is the max
        if (!j.contains("max") || j["max"].is_null()) {
          pbit->m_max = (uint8_t) ((1u << pbit->m_width) - 1);
        }

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
//  mdf_normalizeJson
//
// The JSON parser reads one spelling of each key. MDF files (and the
// specification itself) use others: keys in other letter case ("Region",
// "Name"), "event" for "events", "rowcnt" for "rowcount", "dir" for
// "direction", "version_subminor" for "version_patch", "md5" for "md5sum",
// "path" for "url" and "language" for "lang". They are renamed here (only if
// the parser's spelling is not also present) so no information is skipped.
//

// A JSON scalar as text, without losing sign or decimals
static std::string
mdf_jsonScalarString(const json &value)
{
  if (value.is_string()) {
    return value.get<std::string>();
  }
  if (value.is_boolean()) {
    return value.get<bool>() ? "true" : "false";
  }
  if (value.is_number_unsigned()) {
    return std::to_string(value.get<uint64_t>());
  }
  if (value.is_number_integer()) {
    return std::to_string(value.get<int64_t>());
  }
  if (value.is_number_float()) {
    return value.dump();
  }
  return value.dump();
}

static void
mdf_normalizeJsonNode(json &node, bool bModule)
{
  static const std::set<std::string> knownKeys = {
    "redirect", "module", "name", "model", "version", "changed", "description", "infourl", "buffersize",
    "level", "copyright", "manufacturer", "address", "street", "town", "city", "postcode", "state", "region",
    "country", "telephone", "fax", "email", "web", "social", "number", "url", "path", "boot", "algorithm",
    "blocksize", "blockcount", "files", "picture", "video", "firmware", "driver", "manual", "setup", "format",
    "date", "target", "targetcode", "size", "md5", "md5sum", "version_major", "version_minor", "version_patch",
    "version_subminor", "type", "os", "osver", "architecture", "lang", "language", "register", "registers",
    "page", "offset", "span", "width", "min", "max", "access", "default", "fgcolor", "bgcolor", "evenfg",
    "oddfg", "evenbg", "oddbg", "valuelist", "value", "bit", "bits", "pos", "remotevar", "remotevars",
    "bitpos", "alarm", "alarms", "dmatrix", "start-page", "start-offset", "rowcnt", "rowcount", "rowsize",
    "action", "code", "param", "event", "events", "class", "priority", "direction", "dir", "data", "start_page",
    "start_offset"
  };

  static const std::map<std::string, std::string> aliases = {
    { "dir", "direction" },   { "rowcnt", "rowcount" }, { "version_subminor", "version_patch" },
    { "md5", "md5sum" },      { "path", "url" },        { "language", "lang" },
    { "start_page", "start-page" }, { "start_offset", "start-offset" }
  };

  if (node.is_array()) {
    for (auto &item : node) {
      mdf_normalizeJsonNode(item, false);
    }
    return;
  }

  if (!node.is_object()) {
    return;
  }

  // Collect renames first (the object can not be changed while iterating)
  std::vector<std::pair<std::string, std::string>> renames;
  for (auto it = node.begin(); it != node.end(); ++it) {
    std::string key   = it.key();
    std::string lower = key;
    vscp_makeLower(lower);
    if (!knownKeys.count(lower)) {
      continue;
    }
    std::string target = lower;
    auto alias         = aliases.find(lower);
    if (aliases.end() != alias) {
      target = alias->second;
    }
    if (bModule && ("event" == lower)) {
      target = "events";
    }
    if ((target != key) && !node.contains(target)) {
      renames.push_back({ key, target });
    }
  }
  for (auto &r : renames) {
    if (node.contains(r.second)) {
      continue; // Two spellings of one key: keep the first one renamed
    }
    node[r.second] = node[r.first];
    node.erase(r.first);
  }

  // A version given as version_major/version_minor/version_patch (the
  // form the specification uses for files) is also made available as a
  // "major.minor.patch" string for the file types that store it as a string.
  if (!bModule && !node.contains("version") &&
      (node.contains("version_major") || node.contains("version_minor") || node.contains("version_patch"))) {
    auto part = [&node](const char *key) -> std::string {
      if (!node.contains(key)) {
        return "0";
      }
      if (node[key].is_number_unsigned()) {
        return std::to_string(node[key].get<uint64_t>());
      }
      if (node[key].is_number_integer()) {
        return std::to_string(node[key].get<int64_t>());
      }
      if (node[key].is_string()) {
        return std::to_string(vscp_readStringValue(node[key].get<std::string>()));
      }
      return "0";
    };
    node["version"] = part("version_major") + "." + part("version_minor") + "." + part("version_patch");
  }

  for (auto it = node.begin(); it != node.end(); ++it) {
    // Language keys in texts are left as they are
    if (("description" == it.key()) || ("infourl" == it.key())) {
      continue;
    }
    mdf_normalizeJsonNode(it.value(), ("module" == it.key()));
  }
}

static void
mdf_normalizeJson(json &j)
{
  mdf_normalizeJsonNode(j, false);

  if (!(j.contains("module") && j["module"].is_object())) {
    return;
  }
  json &jmod = j["module"];

  // A remote variable embedded in a register ("remotevar": {...} in the
  // register object) becomes a normal remote variable. What it does not give
  // is taken from the register (specification). They are put first, in
  // register order (where they are in an XML file).
  const char *regKey = jmod.contains("register") ? "register" : (jmod.contains("registers") ? "registers" : nullptr);
  if ((nullptr != regKey) && jmod[regKey].is_array()) {
    json embedded = json::array();
    for (auto &jreg : jmod[regKey]) {
      if (!jreg.is_object() || !jreg.contains("remotevar") || !jreg["remotevar"].is_object()) {
        continue;
      }
      json jrv = jreg["remotevar"];
      for (const char *key : { "name", "page", "offset", "access", "default", "fgcolor", "bgcolor", "description",
                               "infourl" }) {
        if (!jrv.contains(key) && jreg.contains(key)) {
          jrv[key] = jreg[key];
        }
      }
      if (!jrv.contains("type")) {
        jrv["type"] = "uint8_t";
      }
      embedded.push_back(jrv);
      jreg.erase("remotevar");
    }
    if (!embedded.empty()) {
      const char *rvKey = jmod.contains("remotevar") ? "remotevar" : (jmod.contains("remotevars") ? "remotevars" : "remotevar");
      if (!jmod.contains(rvKey) || !jmod[rvKey].is_array()) {
        jmod[rvKey] = json::array();
      }
      for (auto &jrv : jmod[rvKey]) {
        embedded.push_back(jrv);
      }
      jmod[rvKey] = embedded;
    }
  }

  // Event data without an offset: the position in the list is the offset
  for (const char *evKey : { "events", "event" }) {
    if (!jmod.contains(evKey) || !jmod[evKey].is_array()) {
      continue;
    }
    for (auto &jev : jmod[evKey]) {
      if (!jev.is_object() || !jev.contains("data") || !jev["data"].is_array()) {
        continue;
      }
      size_t idx = 0;
      for (auto &jdata : jev["data"]) {
        if (jdata.is_object() && !jdata.contains("offset")) {
          jdata["offset"] = idx;
        }
        idx++;
      }
    }
  }

  // Module name: the specification asks for a string, older files have a
  // language object. Use English (or the first language).
  if (j.contains("module") && j["module"].is_object() && j["module"].contains("name") &&
      j["module"]["name"].is_object() && !j["module"]["name"].empty()) {
    json jname = j["module"]["name"];
    if (jname.contains("en") && jname["en"].is_string()) {
      j["module"]["name"] = jname["en"];
    }
    else if (jname.begin().value().is_string()) {
      j["module"]["name"] = jname.begin().value();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//  parseMDF_JSON
//

int
CMDF::parseMDF_JSON_raw(const std::string &path)
{
  int rv = VSCP_ERROR_SUCCESS;

  spdlog::debug("Reading JSON MDF {0}", path);

  json j;

  try {
    std::ifstream ifs(path, std::ifstream::in | std::ifstream::binary);
    if (!ifs.is_open()) {
      m_strLastError = "Unable to open " + path;
      return VSCP_ERROR_NOT_OPEN;
    }
    ifs >> j;
    ifs.close();
  }
  catch (const std::exception &ex) {
    spdlog::error("Parse-JSON: Failed to parse JSON MDF: {}", ex.what());
    m_strLastError = std::string("JSON error: ") + ex.what();
    return VSCP_ERROR_PARSING;
  }
  catch (...) {
    spdlog::error("Parse-JSON: Failed to parse JSON MDF.");
    m_strLastError = "JSON error";
    return VSCP_ERROR_PARSING;
  }

  spdlog::trace("Parse-JSON: <<{}>>", j.dump());

  // Accept the spellings the specification uses (and older ones) by
  // bringing them to the form the parser below reads.
  mdf_normalizeJson(j);

  try {

  // REDIRECT  <-----
  if (j.contains("redirect") && j["redirect"].is_string()) {
    spdlog::trace("Parse-JSON: REDIRECT found '{0}'.", (std::string) j["redirect"]);
    m_redirectUrl = j["redirect"];
    // A redirect only file has nothing more to read. If there is module
    // content too it is read as well (so it is not lost).
    if (!(j.contains("module") && j["module"].is_object())) {
      return VSCP_ERROR_SUCCESS;
    }
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
        spdlog::trace("Parse-JSON: Module name: {0} language: 'en' ", m_name);
      }
      else {
        spdlog::error("Parse-JSON: Failed to read module name (not string, not object)");
        return VSCP_ERROR_PARSING;
      }
    }
    else {
      spdlog::trace("Parse-JSON: Failed to read module name.");
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
    if (j["module"].contains("copyright") && j["module"]["copyright"].is_string()) {
      m_copyright = j["module"]["copyright"];
      spdlog::trace("Parse-JSON: Module copyright: {0}", m_copyright);
    }
    else {
      spdlog::trace("Parse-JSON: There is no module copyright.");
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

    // Module level: 1 = Level I, 2 = Level II (as for XML)
    if (j["module"].contains("level")) {
      if (j["module"]["level"].is_number_integer()) {
        m_vscpLevel = mdf_levelFromFile(j["module"]["level"].get<int64_t>());
      }
      else if (j["module"]["level"].is_string()) {
        m_vscpLevel = mdf_levelFromFile(vscp_readStringValue(j["module"]["level"].get<std::string>()));
      }
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
    {
      json jsub = (j["module"].contains("manufacturer") && j["module"]["manufacturer"].is_object())
                    ? j["module"]["manufacturer"]
                    : json::object();

      if (jsub.contains("name") && jsub["name"].is_string()) {
        m_manufacturer.m_strName = jsub["name"];
        spdlog::trace("Parse-JSON: Module manufacturer name: {0}", m_manufacturer.m_strName);
      }
      else {
        spdlog::warn("Parse-JSON: Failed to read module manufacturer name (not string)");
      }

      // Adress - Is always object
      if (jsub.contains("address") && jsub["address"].is_object()) {

        json jsub2 = jsub["address"];
        if (jsub2.contains("street") && jsub2["street"].is_string()) {
          m_manufacturer.m_address.m_strStreet = jsub2["street"];
          spdlog::trace("Parse-JSON: Module manufacturer address: {0}", m_manufacturer.m_address.m_strStreet);
        }

        if (jsub2.contains("town") && jsub2["town"].is_string()) {
          m_manufacturer.m_address.m_strTown = jsub2["town"];
          spdlog::trace("Parse-JSON: Module manufacturer town: {0}", m_manufacturer.m_address.m_strTown);
        }

        if (jsub2.contains("city") && jsub2["city"].is_string()) {
          m_manufacturer.m_address.m_strCity = jsub2["city"];
          spdlog::trace("Parse-JSON: Module manufacturer city: {0}", m_manufacturer.m_address.m_strCity);
        }

        if (jsub2.contains("postcode") && jsub2["postcode"].is_string()) {
          m_manufacturer.m_address.m_strPostCode = jsub2["postcode"];
          spdlog::trace("Parse-JSON: Module manufacturer post code: {0}", m_manufacturer.m_address.m_strPostCode);
        }

        if (jsub2.contains("state") && jsub2["state"].is_string()) {
          m_manufacturer.m_address.m_strState = jsub2["state"];
          spdlog::trace("Parse-JSON: Module manufacturer state: {0}", m_manufacturer.m_address.m_strState);
        }

        if (jsub2.contains("region") && jsub2["region"].is_string()) {
          m_manufacturer.m_address.m_strRegion = jsub2["region"];
          spdlog::trace("Parse-JSON: Module manufacturer region: {0}", m_manufacturer.m_address.m_strRegion);
        }

        if (jsub2.contains("country") && jsub2["country"].is_string()) {
          m_manufacturer.m_address.m_strCountry = jsub2["country"];
          spdlog::trace("Parse-JSON: Module manufacturer country: {0}", m_manufacturer.m_address.m_strCountry);
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
                ptel->m_value = jsub2["number"];
                spdlog::trace("Parse-JSON: Module manufacturer telephone: {0}", ptel->m_value);
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
                pfax->m_value = jsub2["number"];
                spdlog::trace("Parse-JSON: Module manufacturer fax: {0}", pfax->m_value);
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
                pemail->m_value = jsub2["address"];
                spdlog::trace("Parse-JSON: Module manufacturer email: {0}", pemail->m_value);
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
              m_manufacturer.m_list_Web.push_back(pweb);

              json jsub2 = web.value();

              if (jsub2.contains("url") && jsub2["url"].is_string()) {
                pweb->m_value = jsub2["url"];
                spdlog::trace("Parse-JSON: Module manufacturer web: {0}", pweb->m_value);
              }
              else if (jsub2.contains("address") && jsub2["address"].is_string()) {
                pweb->m_value = jsub2["address"];
                spdlog::trace("Parse-JSON: Module manufacturer web: {0}", pweb->m_value);
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

        // Social - Is always array
        if (jsub.contains("social") && jsub["social"].is_array()) {

          for (auto &social : jsub["social"].items()) {
            // std::cout << "key: " << social.key() << ", value:" << social.value() << '\n';
            if (social.value().is_object()) {

              CMDF_Item *psocial = new CMDF_Item();
              if (psocial == nullptr) {
                spdlog::error("Parse-JSON: Failed to allocate memory for social item.");
                return VSCP_ERROR_PARSING;
              }
              m_manufacturer.m_list_Social.push_back(psocial);

              json jsub2 = social.value();

              if (jsub2.contains("url") && jsub2["url"].is_string()) {
                psocial->m_value = jsub2["url"];
                spdlog::trace("Parse-JSON: Module manufacturer web: {0}", psocial->m_value);
              }
              else if (jsub2.contains("address") && jsub2["address"].is_string()) {
                psocial->m_value = jsub2["id"];
                spdlog::trace("Parse-JSON: Module manufacturer web: {0}", psocial->m_value);
              }
              else {
                spdlog::warn("Parse-JSON: No social url/address.");
              }

              // Description is language specific. Can be string or object
              if (getDescriptionList(jsub2, psocial->m_mapDescription) != VSCP_ERROR_SUCCESS) {
                spdlog::warn("Parse-JSON: Failed to get social description.");
              }

              if (getInfoUrlList(jsub2, psocial->m_mapInfoURL) != VSCP_ERROR_SUCCESS) {
                spdlog::warn("Parse-JSON: Failed to get social infourl.");
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
          spdlog::trace("Parse-JSON: Boot loader algorithm: {0} ", m_bootInfo.m_nAlgorithm);
        }
        else if (jsub.contains("algorithm") && jsub["algorithm"].is_number()) {
          m_bootInfo.m_nAlgorithm = jsub["algorithm"];
          spdlog::trace("Parse-JSON: Boot loader algorithm: {0} ", m_bootInfo.m_nAlgorithm);
        }

        if (jsub.contains("blocksize") && jsub["blocksize"].is_string()) {
          m_bootInfo.m_nBlockSize = vscp_readStringValue(jsub["blocksize"]);
          spdlog::trace("Parse-JSON: Boot loader block size: {0} ", m_bootInfo.m_nBlockSize);
        }
        else if (jsub.contains("blocksize") && jsub["blocksize"].is_number()) {
          m_bootInfo.m_nBlockSize = jsub["blocksize"];
          spdlog::trace("Parse-JSON: Boot loader block size: {0} ", m_bootInfo.m_nBlockSize);
        }

        if (jsub.contains("blockcount") && jsub["blockcount"].is_string()) {
          m_bootInfo.m_nBlockCount = vscp_readStringValue(jsub["blockcount"]);
          spdlog::trace("Parse-JSON: MBoot loader block count: {0} ", m_bootInfo.m_nBlockCount);
        }
        else if (jsub.contains("blockcount") && jsub["blockcount"].is_number()) {
          m_bootInfo.m_nBlockCount = jsub["blockcount"];
          spdlog::trace("Parse-JSON: MBoot loader block count: {0} ", m_bootInfo.m_nBlockCount);
        }
      }

      // ------------------------------------------------------------------------
      //                                Registers
      // ------------------------------------------------------------------------

      const json *pRegisters = nullptr;
      if (j["module"].contains("register") && j["module"]["register"].is_array()) {
        pRegisters = &j["module"]["register"];
      }
      else if (j["module"].contains("registers") && j["module"]["registers"].is_array()) {
        pRegisters = &j["module"]["registers"];
      }

      if (nullptr != pRegisters) {
        for (auto &reg : pRegisters->items()) {
          // std::cout << "key: " << reg.key() << ", value:" << reg.value() << '\n';
          if (reg.value().is_object()) {

            CMDF_Register *preg = new CMDF_Register();
            if (preg == nullptr) {
              spdlog::error("Parse-JSON: Failed to allocate memory for register item.");
              return VSCP_ERROR_PARSING;
            }

            // push back at end -->

            json jreg(reg.value());
            // std::cout << "REG: " << jreg.dump() << '\n';

            // Register Page
            if (jreg.contains("page") && jreg["page"].is_number()) {
              preg->m_page = jreg["page"];
              spdlog::trace("Parse-JSON: Module register page: {0}", preg->m_page);
            }
            else if (jreg.contains("page") && jreg["page"].is_string()) {
              preg->m_page = vscp_readStringValue(jreg["page"]);
              spdlog::trace("Parse-JSON: Module register page: {0}", preg->m_page);
            }

            // Register offset
            if (jreg.contains("offset") && jreg["offset"].is_number()) {
              preg->m_offset = jreg["offset"];
              spdlog::trace("Parse-JSON: Module register offset: {0}", preg->m_offset);
            }
            else if (jreg.contains("offset") && jreg["offset"].is_string()) {
              preg->m_offset = vscp_readStringValue(jreg["offset"]);
              spdlog::trace("Parse-JSON: Module register offset: {0}", preg->m_offset);
            }

            // Register span
            if (jreg.contains("span") && jreg["span"].is_number()) {
              preg->m_span = jreg["span"];
              spdlog::trace("Parse-JSON: Module register span: {0}", preg->m_span);
              if ((preg->m_span == 0) || (preg->m_span > 8)) {
                spdlog::warn("Parse-JSON: Register span is greater than eight or zero. This is not supported. "
                             "Default set (1)");
              }
            }
            else if (jreg.contains("span") && jreg["span"].is_string()) {
              preg->m_span = vscp_readStringValue(jreg["span"]);
              spdlog::trace("Parse-JSON: Module register span: {0}", preg->m_span);
              if ((preg->m_span == 0) || (preg->m_span > 8)) {
                spdlog::warn("Parse-JSON: Register span is greater than eight or zero. This is not supported. "
                             "Default set (1)");
              }
            }

            // Register size (synonym to span)
            if (jreg.contains("size") && jreg["size"].is_number()) {
              preg->m_span = jreg["size"];
              spdlog::trace("Parse-JSON: Module register span: {0}", preg->m_span);
              if (preg->m_span == 0) {
                spdlog::warn("Parse-JSON: Register span is zero. This is not supported. "
                             "Default set (1)");
              }
            }
            else if (jreg.contains("size") && jreg["size"].is_string()) {
              preg->m_span = vscp_readStringValue(jreg["size"]);
              spdlog::trace("Parse-JSON: Module register span: {0}", preg->m_span);
              if (preg->m_span == 0) {
                spdlog::warn("Parse-JSON: Register span is zero. This is not supported. "
                             "Default set (1)");
              }
            }

            // Register width
            if (jreg.contains("width") && jreg["width"].is_number()) {
              preg->m_width = jreg["width"];
              spdlog::trace("Parse-JSON: Module register width: {0}", preg->m_width);
            }
            else if (jreg.contains("width") && jreg["width"].is_string()) {
              preg->m_width = vscp_readStringValue(jreg["width"]);
              spdlog::trace("Parse-JSON: Module register width: {0}", preg->m_width);
            }

            // Register width
            if (jreg.contains("width") && jreg["width"].is_number()) {
              preg->m_width = jreg["width"];
              spdlog::trace("Parse-JSON: Module register width: {0}", preg->m_width);
            }
            else if (jreg.contains("width") && jreg["width"].is_string()) {
              preg->m_width = vscp_readStringValue(jreg["width"]);
              spdlog::trace("Parse-JSON: Module register width: {0}", preg->m_width);
            }

            // Register type
            if (jreg.contains("type") && jreg["type"].is_string()) {
              std::string strType = jreg["type"];
              vscp_trim(strType);
              vscp_lower(strType);
              if (strType == "std") {
                preg->m_type = MDF_REG_TYPE_STANDARD;
                spdlog::trace("Parse-JSON: Module register type: standard {0}", (int) preg->m_type);
              }
              else if ((strType == "dmatrix1") || (strType == "dm") || (strType == "dmatrix")) {
                preg->m_type = MDF_REG_TYPE_DMATRIX1;
                spdlog::trace("Parse-JSON: Module register type: dmatrix1 {0}", (int) preg->m_type);
              }
              else if (strType == "block") {
                preg->m_type = MDF_REG_TYPE_BLOCK;
                spdlog::trace("Parse-JSON: Module register type: block {0}", (int) preg->m_type);
              }
            }

            // Register min
            if (jreg.contains("min") && jreg["min"].is_number()) {
              preg->m_min = jreg["min"];
              spdlog::trace("Parse-JSON: Module register min: {0}", preg->m_min);
            }
            else if (jreg.contains("min") && jreg["min"].is_string()) {
              preg->m_min = vscp_readStringValue(jreg["min"]);
              spdlog::trace("Parse-JSON: Module register min: {0}", preg->m_min);
            }

            // Register max
            if (jreg.contains("max") && jreg["max"].is_number()) {
              preg->m_max = jreg["max"];
              spdlog::trace("Parse-JSON: Module register max: {0}", preg->m_max);
            }
            else if (jreg.contains("max") && jreg["max"].is_string()) {
              preg->m_max = vscp_readStringValue(jreg["max"]);
              spdlog::trace("Parse-JSON: Module register max: {0}", preg->m_max);
            }

            if (jreg.contains("default") && jreg["default"].is_string()) {
              std::string default_value = jreg["default"];
              preg->m_strDefault = mdf_normalizeDefault(default_value);
              spdlog::trace("Parse-JSON: Module register default: {0}", preg->m_strDefault);
            }
            else if (jreg.contains("default") && jreg["default"].is_number()) {
              preg->m_strDefault = mdf_jsonScalarString(jreg["default"]);
              spdlog::trace("Parse-JSON: Module register value and default set to defined default: {0}",
                            preg->m_strDefault);
            }
            else if (jreg.contains("default") && jreg["default"].is_boolean()) {
              preg->m_strDefault = jreg["default"] ? "true" : "false";
              spdlog::trace("Parse-JSON: Module register value and default set to defined default: {0}",
                            preg->m_strDefault);
            }

            if (jreg.contains("access") && jreg["access"].is_string()) {
              std::string strAccess = jreg["access"];
              vscp_trim(strAccess);
              vscp_makeLower(strAccess);
              preg->m_access = mdf_parseAccess(strAccess);
            }

            // Register name
            if (jreg.contains("name") && jreg["name"].is_string()) {
              preg->m_name = jreg["name"];
              vscp_trim(preg->m_name);
              spdlog::info("Parse-JSON: Register name set to {0}.", preg->m_name);
            }
            else {
              spdlog::info("Parse-JSON: No register name defined (defaults to empty string).");
            }

            // Foreground color (VSCP Works)
            if (jreg.contains("fgcolor") && jreg["fgcolor"].is_string()) {
              preg->m_fgcolor = vscp_readStringValue(jreg["fgcolor"]);
              spdlog::trace("Parse-JSON: Remote variable foreground color set to {}.", preg->m_fgcolor);
            }
            else if (jreg.contains("fgcolor") && jreg["fgcolor"].is_number()) {
              preg->m_fgcolor = jreg["fgcolor"];
              spdlog::trace("Parse-JSON: Remote variable foreground color to {}.", preg->m_fgcolor);
            }

            // Backround color (VSCP Works)
            if (jreg.contains("bgcolor") && jreg["bgcolor"].is_string()) {
              preg->m_bgcolor = vscp_readStringValue(jreg["bgcolor"]);
              spdlog::trace("Parse-JSON: Remote variable background color set to {}.", preg->m_bgcolor);
            }
            else if (jreg.contains("bgcolor") && jreg["bgcolor"].is_number()) {
              preg->m_bgcolor = jreg["bgcolor"];
              spdlog::trace("Parse-JSON: Remote variable background color to {}.", preg->m_bgcolor);
            }

            // Even foreground color (VSCP Works)
            if (jreg.contains("evenfg") && jreg["evenfg"].is_string()) {
              preg->m_fgeven = vscp_readStringValue(jreg["evenfg"]);
              spdlog::trace("Parse-JSON: Even foreground color set to {}.", preg->m_fgeven);
            }
            else if (jreg.contains("evenfg") && jreg["evenfg"].is_number()) {
              preg->m_fgeven = jreg["evenfg"];
              spdlog::trace("Parse-JSON: Even foreground color to {}.", preg->m_fgeven);
            }

            // Odd foreground color (VSCP Works)
            if (jreg.contains("oddfg") && jreg["oddfg"].is_string()) {
              preg->m_fgodd = vscp_readStringValue(jreg["oddfg"]);
              spdlog::trace("Parse-JSON: Odd foreground color set to {}.", preg->m_fgodd);
            }
            else if (jreg.contains("oddfg") && jreg["oddfg"].is_number()) {
              preg->m_fgodd = jreg["oddfg"];
              spdlog::trace("Parse-JSON: Odd foreground color to {}.", preg->m_fgodd);
            }

            // Even background color (VSCP Works)
            if (jreg.contains("evenbg") && jreg["evenbg"].is_string()) {
              preg->m_bgeven = vscp_readStringValue(jreg["evenbg"]);
              spdlog::trace("Parse-JSON: Even background color set to {}.", preg->m_bgeven);
            }
            else if (jreg.contains("evenbg") && jreg["evenbg"].is_number()) {
              preg->m_bgeven = jreg["evenbg"];
              spdlog::trace("Parse-JSON: Even background color to {}.", preg->m_bgeven);
            }

            // Odd background color (VSCP Works)
            if (jreg.contains("oddbg") && jreg["oddbg"].is_string()) {
              preg->m_bgodd = vscp_readStringValue(jreg["oddbg"]);
              spdlog::trace("Parse-JSON: Odd background color set to {}.", preg->m_bgodd);
            }
            else if (jreg.contains("oddbg") && jreg["oddbg"].is_number()) {
              preg->m_bgodd = jreg["oddbg"];
              spdlog::trace("Parse-JSON: Odd background color to {}.", preg->m_bgodd);
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

            // Store and expand DMATRIX1/BLOCK registers to one register per
            // byte of the span
            m_list_register.push_back(preg);
            expandRegisterSpan(preg);
          }
        }
      }
      else {
        spdlog::warn("Parse-JSON: Failed to read module registers");
      }

      //------------------------------------------------------------------------
      //                             Remote variable
      //------------------------------------------------------------------------

      const json *pRemoteVars = nullptr;
      if (j["module"].contains("remotevar") && j["module"]["remotevar"].is_array()) {
        pRemoteVars = &j["module"]["remotevar"];
      }
      else if (j["module"].contains("remotevars") && j["module"]["remotevars"].is_array()) {
        pRemoteVars = &j["module"]["remotevars"];
      }

      if (nullptr != pRemoteVars) {
        for (auto &rvar : pRemoteVars->items()) {
          // std::cout << "key: " << rvar.key() << ", value:" << rvar.value() << '\n';
          spdlog::trace("Parse-JSON: Remote variable key = {0} type = {1}.", (std::string)rvar.key(), rvar.value().dump());
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
              spdlog::trace("Parse-JSON: Remote variable name set to {}.", prvar->m_name);
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
                spdlog::trace("Parse-JSON: Remote variable type set to 'string' {}.", (int) prvar->m_type);
              }
              else if (str == "bool") {
                prvar->m_type = remote_variable_type_boolean;
                spdlog::trace("Parse-JSON: Remote variable type set to 'boolena' {}.", (int) prvar->m_type);
              }
              else if ((str == "int8") || (str == "int8_t") || (str == "char")) {
                prvar->m_type = remote_variable_type_int8_t;
                spdlog::trace("Parse-JSON: Remote variable type set to 'int8_t' {}.", (int) prvar->m_type);
              }
              else if ((str == "uint8") || (str == "uint8_t") || (str == "byte")) {
                prvar->m_type = remote_variable_type_uint8_t;
                spdlog::trace("Parse-JSON: Remote variable type set to 'uint8_t' {}.", (int) prvar->m_type);
              }
              else if ((str == "int16") || (str == "int16_t") || (str == "short")) {
                prvar->m_type = remote_variable_type_int16_t;
                spdlog::trace("Parse-JSON: Remote variable type set to 'int16_t' {}.", (int) prvar->m_type);
              }
              else if ((str == "uint16") || (str == "uint16_t")) {
                prvar->m_type = remote_variable_type_uint16_t;
                spdlog::trace("Parse-JSON: Remote variable type set to 'uint16_t' {}.", (int) prvar->m_type);
              }
              else if ((str == "int32") || (str == "int32_t") || (str == "long")) {
                prvar->m_type = remote_variable_type_int32_t;
                spdlog::trace("Parse-JSON: Remote variable type set to 'int32_t' {}.", (int) prvar->m_type);
              }
              else if ((str == "uint32") || (str == "uint32_t") || (str == "unsigned")) {
                prvar->m_type = remote_variable_type_uint32_t;
                spdlog::trace("Parse-JSON: Remote variable type set to 'uint32_t' {}.", (int) prvar->m_type);
              }
              else if ((str == "int64") || (str == "int64_t") || (str == "longlong")) {
                prvar->m_type = remote_variable_type_int64_t;
                spdlog::trace("Parse-JSON: Remote variable type set to 'int64_t' {}.", (int) prvar->m_type);
              }
              else if ((str == "uint64") || (str == "uint64_t")) {
                prvar->m_type = remote_variable_type_uint64_t;
                spdlog::trace("Parse-JSON: Remote variable type set to 'uint64_t' {}.", (int) prvar->m_type);
              }
              else if (str == "float") {
                prvar->m_type = remote_variable_type_float;
                spdlog::trace("Parse-JSON: Remote variable type set to 'float' {}.", (int) prvar->m_type);
              }
              else if (str == "double") {
                prvar->m_type = remote_variable_type_double;
                spdlog::trace("Parse-JSON: Remote variable type set to 'double' {}.", (int) prvar->m_type);
              }
              else if (str == "date") {
                prvar->m_type = remote_variable_type_date;
                spdlog::trace("Parse-JSON: Remote variable type set to 'date' {}.", (int) prvar->m_type);
              }
              else if (str == "time") {
                prvar->m_type = remote_variable_type_time;
                spdlog::trace("Parse-JSON: Remote variable type set to 'time' {}.", (int) prvar->m_type);
              }
            }
            else {
              // Must be a type if a remote variable is there
              spdlog::error("Parse-JSON: No remote variable type defined.");
              return VSCP_ERROR_PARSING;
            }

            // Default value. Always a string. Convert to the correct type
            // when default is set.
            if (jrvar.contains("default") && !jrvar["default"].is_null()) {
              prvar->m_strDefault = mdf_normalizeDefault(mdf_jsonScalarString(jrvar["default"]));
              spdlog::trace("Parse-JSON: Remote variable default set to {}.", prvar->m_strDefault);
            }

            // Min/max (kept as text, the type decides how they are read)
            if (jrvar.contains("min") && !jrvar["min"].is_null()) {
              prvar->m_strMin = mdf_jsonScalarString(jrvar["min"]);
            }
            if (jrvar.contains("max") && !jrvar["max"].is_null()) {
              prvar->m_strMax = mdf_jsonScalarString(jrvar["max"]);
            }

            // Page where the remote variable is located
            if (jrvar.contains("page") && jrvar["page"].is_string()) {
              prvar->m_page = vscp_readStringValue(jrvar["page"]);
              spdlog::trace("Parse-JSON: Remote variable page set to {}.", prvar->m_page);
            }
            else if (jrvar.contains("page") && jrvar["page"].is_number()) {
              prvar->m_page = jrvar["page"];
              spdlog::trace("Parse-JSON: Remote variable page set to {}.", prvar->m_page);
            }

            // Offset where the remote variable is located
            if (jrvar.contains("offset") && jrvar["offset"].is_string()) {
              prvar->m_offset = vscp_readStringValue(jrvar["offset"]);
              spdlog::trace("Parse-JSON: Remote variable offset set to {}.", prvar->m_offset);
            }
            else if (jrvar.contains("offset") && jrvar["offset"].is_number()) {
              prvar->m_offset = jrvar["offset"];
              spdlog::trace("Parse-JSON: Remote variable offset set to {}.", prvar->m_offset);
            }

            // Bit position in byte where the boolean variable is located (0-7)
            // If -1 not used.
            if (jrvar.contains("bitpos") && jrvar["bitpos"].is_string()) {
              prvar->m_bitpos = vscp_readStringValue(jrvar["bitpos"]) & 0x07;
              spdlog::trace("Parse-JSON: Remote variable bitpos set to {}.", prvar->m_bitpos);
            }
            else if (jrvar.contains("bitpos") && jrvar["bitpos"].is_number()) {
              prvar->m_bitpos = jrvar["bitpos"];
              prvar->m_bitpos &= 7;
              spdlog::trace("Parse-JSON: Remote variable bitpos set to {}.", prvar->m_bitpos);
            }

            // Size for a string (ignored for all other types)
            if (jrvar.contains("size") && jrvar["size"].is_string()) {
              prvar->m_size = vscp_readStringValue(jrvar["size"]);
              spdlog::trace("Parse-JSON: Remote variable size set to {}.", prvar->m_size);
            }
            else if (jrvar.contains("size") && jrvar["size"].is_number()) {
              prvar->m_size = jrvar["size"];
              spdlog::trace("Parse-JSON: Remote variable size set to {}.", prvar->m_size);
            }

            // Access rights for remote variable
            if (jrvar.contains("access") && jrvar["access"].is_string()) {
              std::string strAccess = jrvar["access"];
              vscp_trim(strAccess);
              vscp_makeLower(strAccess);
              prvar->m_access = mdf_parseAccess(strAccess);
            }

            // Foreground color (VSCP Works)
            if (jrvar.contains("fgcolor") && jrvar["fgcolor"].is_string()) {
              prvar->m_fgcolor = vscp_readStringValue(jrvar["fgcolor"]);
              spdlog::trace("Parse-JSON: Remote variable fore ground color set to {}.", prvar->m_fgcolor);
            }
            else if (jrvar.contains("fgcolor") && jrvar["fgcolor"].is_number()) {
              prvar->m_fgcolor = jrvar["fgcolor"];
              spdlog::trace("Parse-JSON: Remote variable fore ground color to {}.", prvar->m_fgcolor);
            }

            // Backround color (VSCP Works)
            if (jrvar.contains("bgcolor") && jrvar["bgcolor"].is_string()) {
              prvar->m_bgcolor = vscp_readStringValue(jrvar["bgcolor"]);
              spdlog::trace("Parse-JSON: Remote variable background color set to {}.", prvar->m_bgcolor);
            }
            else if (jrvar.contains("bgcolor") && jrvar["bgcolor"].is_number()) {
              prvar->m_bgcolor = jrvar["bgcolor"];
              spdlog::trace("Parse-JSON: Remote variable background color to {}.", prvar->m_bgcolor);
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
            spdlog::trace("Parse-JSON: DM level set to {0}.", m_dmInfo.m_level);
          }
          else if (jdmatrix["level"].is_string()) {
            m_dmInfo.m_level = vscp_readStringValue(jdmatrix["level"]);
            spdlog::trace("Parse-JSON: DM level set to {0}.", m_dmInfo.m_level);
          }
        }

        // start page
        if (jdmatrix.contains("start-page")) {
          if (jdmatrix["start-page"].is_number()) {
            m_dmInfo.m_startPage = jdmatrix["start-page"];
            spdlog::trace("Parse-JSON: DM start page set to {0}.", m_dmInfo.m_startPage);
          }
          else if (jdmatrix["start-page"].is_string()) {
            m_dmInfo.m_startPage = vscp_readStringValue(jdmatrix["start-page"]);
            spdlog::trace("Parse-JSON: DM start page set to {0}.", m_dmInfo.m_startPage);
          }
        }

        // start offset
        if (jdmatrix.contains("start-offset")) {
          if (jdmatrix["start-offset"].is_number()) {
            m_dmInfo.m_startOffset = jdmatrix["start-offset"];
            spdlog::trace("Parse-JSON: DM start offset set to {0}.", m_dmInfo.m_startOffset);
          }
          else if (jdmatrix["start-offset"].is_string()) {
            m_dmInfo.m_startOffset = vscp_readStringValue(jdmatrix["start-offset"]);
            spdlog::trace("Parse-JSON: DM start offset set to {0}.", m_dmInfo.m_startOffset);
          }
        }

        // Row Count
        if (jdmatrix.contains("rowcount")) {
          if (jdmatrix["rowcount"].is_number()) {
            m_dmInfo.m_rowCount = jdmatrix["rowcount"];
            spdlog::trace("Parse-JSON: DM row count set to {0}.", m_dmInfo.m_rowCount);
          }
          else if (jdmatrix["rowcount"].is_string()) {
            m_dmInfo.m_rowCount = vscp_readStringValue(jdmatrix["rowcount"]);
            spdlog::trace("Parse-JSON: DM row count set to {0}.", m_dmInfo.m_rowCount);
          }
        }

        // Row Size
        if (jdmatrix.contains("rowsize")) {
          if (jdmatrix["rowsize"].is_number()) {
            m_dmInfo.m_rowSize = jdmatrix["rowsize"];
            spdlog::trace("Parse-JSON: DM row size set to {0}.", m_dmInfo.m_rowSize);
          }
          else if (jdmatrix["rowsize"].is_string()) {
            m_dmInfo.m_rowSize = vscp_readStringValue(jdmatrix["rowsize"]);
            spdlog::trace("Parse-JSON: DM row size set to {0}.", m_dmInfo.m_rowSize);
          }
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
                spdlog::trace("Parse-JSON: DM action name set to {0}.", paction->m_name);
              }
              else {
                spdlog::error("Parse-JSON: No DM action name defined.");
                return VSCP_ERROR_PARSING;
              }

              if (jaction.contains("code") && jaction["code"].is_number()) {
                paction->m_code = jaction["code"];
                spdlog::trace("Parse-JSON: DM action code set to {0}.", paction->m_name);
              }
              else if (jaction.contains("code") && jaction["code"].is_string()) {
                paction->m_code = vscp_readStringValue(jaction["code"]);
                spdlog::trace("Parse-JSON: DM action code set to {0}.", paction->m_code);
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
                      spdlog::trace("Parse-JSON: DM action parameter name set to {0}.", pparam->m_name);
                    }
                    else {
                      spdlog::warn("Parse-JSON: No DM action parameter name defined. Set to default (0).");
                    }

                    // offset
                    if (jparam.contains("offset") && jparam["offset"].is_string()) {
                      pparam->m_offset = vscp_readStringValue(jparam["offset"]);
                      spdlog::trace("Parse-JSON: DM action parameter offset set to {0}.", pparam->m_offset);
                    }
                    else if (jparam.contains("offset") && jparam["offset"].is_number()) {
                      pparam->m_offset = jparam["offset"];
                      spdlog::trace("Parse-JSON: DM action parameter offset set to {0}.", pparam->m_offset);
                    }

                    // min
                    if (jparam.contains("min") && jparam["min"].is_string()) {
                      pparam->m_min = vscp_readStringValue(jparam["min"]);
                      spdlog::trace("Parse-JSON: DM action parameter min set to {0}.", pparam->m_min);
                    }
                    else if (jparam.contains("min") && jparam["min"].is_number()) {
                      pparam->m_min = jparam["min"];
                      spdlog::trace("Parse-JSON: DM action parameter min set to {0}.", pparam->m_min);
                    }

                    // max
                    if (jparam.contains("max") && jparam["max"].is_string()) {
                      pparam->m_max = vscp_readStringValue(jparam["max"]);
                      spdlog::trace("Parse-JSON: DM action parameter max set to {0}.", pparam->m_max);
                    }
                    else if (jparam.contains("max") && jparam["max"].is_number()) {
                      pparam->m_max = jparam["max"];
                      spdlog::trace("Parse-JSON: DM action parameter max set to {0}.", pparam->m_max);
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
              spdlog::trace("Parse-JSON: Event name set to {0}.", pev->m_name);
            }

            // Class
            if (jev.contains("class") && jev["class"].is_string()) {
              std::string str = jev["class"];
              vscp_trim(str);
              if ("-" == str) {
                pev->m_class = -1; // All
              }
              else {
                pev->m_class = vscp_readStringValue(str);
              }
              spdlog::trace("Parse-JSON: Event class set to {0}.", pev->m_class);
            }
            else if (jev.contains("class") && jev["class"].is_number()) {
              pev->m_class = jev["class"];
              spdlog::trace("Parse-JSON: Event class set to {0}.", pev->m_class);
            }
            else {
              spdlog::error("Parse-JSON: No event class defined.");
              return VSCP_ERROR_PARSING;
            }

            // Type
            if (jev.contains("type") && jev["type"].is_string()) {
              std::string str = jev["type"];
              vscp_trim(str);
              if ("-" == str) {
                pev->m_type = -1; // All
              }
              else {
                pev->m_type = vscp_readStringValue(str);
              }
              spdlog::trace("Parse-JSON: Event type set to {0}.", pev->m_type);
            }
            else if (jev.contains("type") && jev["type"].is_number()) {
              pev->m_type = jev["type"];
              spdlog::trace("Parse-JSON: Event type set to {0}.", pev->m_type);
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
                spdlog::trace("Parse-JSON: Event direction set to in.");
              }
              else if (str == "out") {
                pev->m_direction = MDF_EVENT_DIR_OUT;
                spdlog::trace("Parse-JSON: Event direction set to out.");
              }
            }

            // Priority
            if (jev.contains("priority") && jev["priority"].is_string()) {

              std::string str = jev["priority"];
              vscp_trim(str);
              vscp_makeLower(str);

              if (str == "low") {
                pev->m_priority = VSCP_PRIORITY_LOW >> 5;
                spdlog::trace("Parse-JSON: Event priority set to low.");
              }
              else if (str == "normal") {
                pev->m_priority = VSCP_PRIORITY_NORMAL >> 5;
                spdlog::trace("Parse-JSON: Event priority set to normal.");
              }
              else if (str == "medium") {
                pev->m_priority = VSCP_PRIORITY_MEDIUM >> 5;
                spdlog::trace("Parse-JSON: Event priority set to medium.");
              }
              else if (str == "high") {
                pev->m_priority = VSCP_PRIORITY_HIGH >> 5;
                spdlog::trace("Parse-JSON: Event priority set to high.");
              }
              else {
                pev->m_priority = vscp_readStringValue(jev["priority"]) & 7;
                spdlog::trace("Parse-JSON: Event priority set to {0}.", pev->m_priority);
              }
            }
            else if (jev.contains("priority") && jev["priority"].is_number()) {
              pev->m_priority = (uint8_t) jev["priority"] & 7;
              spdlog::trace("Parse-JSON: Event priority set to {0}.", pev->m_priority);
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
                    spdlog::trace("Parse-JSON: Event name set to {0}.", pevdata->m_name);
                  }

                  // offset
                  if (jevdata.contains("offset") && jevdata["offset"].is_number()) {
                    pevdata->m_offset = jevdata["offset"];
                  }
                  else if (jevdata.contains("offset") && jevdata["offset"].is_string()) {
                    pevdata->m_offset = vscp_readStringValue(jevdata["offset"]);
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

      bool alarmParsed = false;
      bool alarmSeen   = false;

      auto parseAlarmEntry = [this](const json &jentry) -> bool {
        if (jentry.is_array()) {
          json jalarmwrap;
          jalarmwrap["bit"] = jentry;
          return (getBitList(jalarmwrap, m_list_alarm) == VSCP_ERROR_SUCCESS);
        }

        if (jentry.is_object()) {
          json jalarmobj = jentry;
          if (!jalarmobj.contains("bit") && jalarmobj.contains("bits")) {
            jalarmobj["bit"] = jalarmobj["bits"];
          }
          return (getBitList(jalarmobj, m_list_alarm) == VSCP_ERROR_SUCCESS);
        }

        return false;
      };

      if (j["module"].contains("alarm")) {
        alarmSeen = true;
        if (parseAlarmEntry(j["module"]["alarm"])) {
          alarmParsed = true;
        }
      }

      if ((!alarmParsed) && j["module"].contains("alarms")) {
        alarmSeen = true;
        if (parseAlarmEntry(j["module"]["alarms"])) {
          alarmParsed = true;
        }
      }

      if ((!alarmParsed) && j.contains("alarm")) {
        alarmSeen = true;
        if (parseAlarmEntry(j["alarm"])) {
          alarmParsed = true;
        }
      }

      if ((!alarmParsed) && j.contains("alarms")) {
        alarmSeen = true;
        if (parseAlarmEntry(j["alarms"])) {
          alarmParsed = true;
        }
      }

      if (!alarmParsed) {
        if (alarmSeen) {
          spdlog::warn("Parse-JSON: Failed to parse module alarm bits");
        }
        else {
          spdlog::warn("Parse-JSON: Failed to read module alarm bits");
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    //                                Files
    ///////////////////////////////////////////////////////////////////////////

    // 'files' - Is always object
    // "files" is normally an object {"picture": [...], "manual": [...], ...}.
    // A list of such objects [{"picture": [...]}, {"manual": [...]}, ...] is
    // also accepted (older files and earlier save_json() output used it);
    // it is merged into the object form.
    json jfiles;
    if (j["module"].contains("files") && j["module"]["files"].is_array()) {
      jfiles = json::object();
      for (auto &group : j["module"]["files"]) {
        if (!group.is_object()) {
          continue;
        }
        for (auto &kv : group.items()) {
          if (!jfiles.contains(kv.key())) {
            jfiles[kv.key()] = json::array();
          }
          if (kv.value().is_array()) {
            for (auto &entry : kv.value()) {
              jfiles[kv.key()].push_back(entry);
            }
          }
        }
      }
    }
    else if (j["module"].contains("files")) {
      jfiles = j["module"]["files"];
    }

    // Accept the alternative key names used by older files and the XML form:
    // "path" for "url", "md5" for "md5sum", "language" for "lang"
    if (jfiles.is_object()) {
      for (auto &kv : jfiles.items()) {
        if (!kv.value().is_array()) {
          continue;
        }
        for (auto &entry : kv.value()) {
          if (!entry.is_object()) {
            continue;
          }
          for (auto alias : { std::make_pair("path", "url"),
                              std::make_pair("md5", "md5sum"),
                              std::make_pair("language", "lang") }) {
            if (entry.contains(alias.first) && !entry.contains(alias.second)) {
              entry[alias.second] = entry[alias.first];
            }
          }
        }
      }
    }

    if (jfiles.is_object()) {

      json jsub = jfiles;

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
            spdlog::trace("Parse-JSON: Picture name: {0} ", ppicture->m_strName);
          }
          else {
            spdlog::warn("Parse-JSON: No picture name.");
          }

          if (jpic.contains("url") && jpic["url"].is_string()) {
            ppicture->m_strURL = jpic["url"];
            spdlog::trace("Parse-JSON: Picture URL: {0} ", ppicture->m_strURL);
          }
          else {
            spdlog::warn("Parse-JSON: No picture url.");
          }

          if (jpic.contains("format") && jpic["format"].is_string()) {
            ppicture->m_strFormat = jpic["format"];
            spdlog::trace("Parse-JSON: Picture Format: {0} ", ppicture->m_strFormat);
          }
          else {
            spdlog::warn("Parse-JSON: No picture format.");
          }

          if (jpic.contains("version") && jpic["version"].is_string()) {
            ppicture->m_strVersion = jpic["version"];
          }

          if (jpic.contains("date") && jpic["date"].is_string()) {
            ppicture->m_strDate = jpic["date"];
            spdlog::trace("Parse-JSON: Picture date: {0} ", ppicture->m_strDate);
          }
          else {
            spdlog::warn("Parse-JSON: No picture date");
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
            spdlog::trace("Parse-JSON: Video name: {0} ", pvideo->m_strName);
          }
          else {
            spdlog::warn("Parse-JSON: No video name.");
          }

          if (jvideo.contains("url") && jvideo["url"].is_string()) {
            pvideo->m_strURL = jvideo["url"];
            spdlog::trace("Parse-JSON: Video URL: {0} ", pvideo->m_strURL);
          }
          else {
            spdlog::warn("Parse-JSON: No video url.");
          }

          if (jvideo.contains("format") && jvideo["format"].is_string()) {
            pvideo->m_strFormat = jvideo["format"];
            spdlog::trace("Parse-JSON: Video Format: {0} ", pvideo->m_strFormat);
          }
          else {
            spdlog::warn("Parse-JSON: No video format.");
          }

          if (jvideo.contains("version") && jvideo["version"].is_string()) {
            pvideo->m_strVersion = jvideo["version"];
          }

          if (jvideo.contains("date") && jvideo["date"].is_string()) {
            pvideo->m_strDate = jvideo["date"];
            spdlog::trace("Parse-JSON: Video date: {0} ", pvideo->m_strDate);
          }
          else {
            spdlog::warn("Parse-JSON: No video date");
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
            spdlog::trace("Parse-JSON: Fiirmware name: {0} ", pfirmware->m_strName);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware name.");
          }

          if (jfirmware.contains("url") && jfirmware["url"].is_string()) {
            pfirmware->m_strURL = jfirmware["url"];
            spdlog::trace("Parse-JSON: Firmware URL: {0} ", pfirmware->m_strURL);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware url.");
          }

          if (jfirmware.contains("target") && jfirmware["target"].is_string()) {
            pfirmware->m_strTarget = jfirmware["target"];
            spdlog::trace("Parse-JSON: Firmware target: {0} ", pfirmware->m_strTarget);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware target.");
          }

          if (jfirmware.contains("targetcode") && jfirmware["targetcode"].is_number()) {
            pfirmware->m_targetCode = jfirmware["targetcode"];
            spdlog::trace("Parse-JSON: Firmware target: {0} ", pfirmware->m_targetCode);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware target code.");
          }

          // Target code string value
          if (jfirmware.contains("targetcode") && jfirmware["targetcode"].is_string()) {
            pfirmware->m_targetCode = vscp_readStringValue(jfirmware["targetcode"]);
            spdlog::trace("Parse-JSON: Firmware target: {0} ", pfirmware->m_targetCode);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware target code.");
          }

          if (jfirmware.contains("targetcode") && jfirmware["targetcode"].is_number()) {
            pfirmware->m_targetCode = jfirmware["targetcode"];
            spdlog::trace("Parse-JSON: Firmware target: {0} ", pfirmware->m_targetCode);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware target code.");
          }

          if (jfirmware.contains("format") && jfirmware["format"].is_string()) {
            pfirmware->m_strFormat = jfirmware["format"];
            spdlog::trace("Parse-JSON: Firmware format: {0} ", pfirmware->m_strFormat);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware format.");
          }

          if (jfirmware.contains("date") && jfirmware["date"].is_string()) {
            pfirmware->m_strDate = jfirmware["date"];
            spdlog::trace("Parse-JSON: Firmware date: {0} ", pfirmware->m_strDate);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware date");
          }

          // "major.minor.patch" string form (the separate keys below win)
          if (jfirmware.contains("version") && jfirmware["version"].is_string() && !jfirmware.contains("version_major")) {
            pfirmware->setVersion(jfirmware["version"].get<std::string>());
          }

          if (jfirmware.contains("version_major") && jfirmware["version_major"].is_number()) {
            pfirmware->m_version_major = jfirmware["version_major"];
            spdlog::trace("Parse-JSON: Firmware version major: {0} ", pfirmware->m_version_major);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version_major");
          }

          if (jfirmware.contains("version_major") && jfirmware["version_major"].is_string()) {
            pfirmware->m_version_major = vscp_readStringValue(jfirmware["version_major"]);
            spdlog::trace("Parse-JSON: Firmware version major: {0} ", pfirmware->m_version_major);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version_major");
          }

          if (jfirmware.contains("version_minor") && jfirmware["version_minor"].is_number()) {
            pfirmware->m_version_minor = jfirmware["version_minor"];
            spdlog::trace("Parse-JSON: Firmware version minor: {0} ", pfirmware->m_version_minor);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version_minor");
          }

          if (jfirmware.contains("version_minor") && jfirmware["version_minor"].is_string()) {
            pfirmware->m_version_minor = vscp_readStringValue(jfirmware["version_minor"]);
            spdlog::trace("Parse-JSON: Firmware version minor: {0} ", pfirmware->m_version_minor);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version_minor");
          }

          if (jfirmware.contains("version_patch") && jfirmware["version_patch"].is_number()) {
            pfirmware->m_version_patch = jfirmware["version_patch"];
            spdlog::trace("Parse-JSON: Firmware version patch: {0} ", pfirmware->m_version_patch);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version patch");
          }

          if (jfirmware.contains("version_patch") && jfirmware["version_patch"].is_string()) {
            pfirmware->m_version_patch = vscp_readStringValue(jfirmware["version_patch"]);
            spdlog::trace("Parse-JSON: Firmware version patch: {0} ", pfirmware->m_version_patch );
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version patch");
          }

          if (jfirmware.contains("size") && jfirmware["size"].is_number()) {
            pfirmware->m_size = jfirmware["size"];
            spdlog::trace("Parse-JSON: Firmware version size: {0} ", pfirmware->m_size);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version size");
          }

          if (jfirmware.contains("size") && jfirmware["size"].is_string()) {
            pfirmware->m_size = vscp_readStringValue(jfirmware["size"]);
            spdlog::trace("Parse-JSON: Firmware version size: {0} ", pfirmware->m_size);
          }
          else {
            spdlog::warn("Parse-JSON: No firmware version size");
          }

          if (jfirmware.contains("md5sum") && jfirmware["md5sum"].is_string()) {
            pfirmware->m_strMd5 = jfirmware["md5sum"];
            spdlog::trace("Parse-JSON: Firmware version md5: {0} ", pfirmware->m_strMd5);
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
            spdlog::trace("Parse-JSON: Driver name: {0} ", pdriver->m_strName);
          }
          else {
            spdlog::warn("Parse-JSON: No driver name.");
          }

          if (jdriver.contains("url") && jdriver["url"].is_string()) {
            pdriver->m_strURL = jdriver["url"];
            spdlog::trace("Parse-JSON: Driver URL: {0} ", pdriver->m_strURL);
          }
          else {
            spdlog::warn("Parse-JSON: No driver url.");
          }

          if (jdriver.contains("name") && jdriver["name"].is_string()) {
            pdriver->m_strName = jdriver["name"];
            spdlog::trace("Parse-JSON: Driver name: {0} ", pdriver->m_strName);
          }
          else {
            spdlog::warn("Parse-JSON: No driver name.");
          }

          // Type
          if (jdriver.contains("type") && jdriver["type"].is_string()) {
            pdriver->m_strType = jdriver["type"];
            spdlog::trace("Parse-JSON: driver type: {0} ", pdriver->m_strType);
          }
          else {
            spdlog::warn("Parse-JSON: No driver type.");
          }

          // OS
          if (jdriver.contains("os") && jdriver["os"].is_string()) {
            pdriver->m_strOS = jdriver["os"];
            spdlog::trace("Parse-JSON: driver OS: {0} ", pdriver->m_strOS);
          }
          else {
            spdlog::warn("Parse-JSON: No driver OS.");
          }

          // OS version
          if (jdriver.contains("osver") && jdriver["osver"].is_string()) {
            pdriver->m_strOSVer = jdriver["osver"];
            spdlog::trace("Parse-JSON: driver OS version: {0} ", pdriver->m_strOSVer);
          }
          else {
            spdlog::warn("Parse-JSON: No driver OS version.");
          }

          //  Architecture
          if (jdriver.contains("architecture") && jdriver["architecture"].is_string()) {
            pdriver->m_strArchitecture = jdriver["architecture"];
            spdlog::trace("Parse-JSON: driver OS architecture: {0} ", pdriver->m_strArchitecture);
          }
          else {
            spdlog::warn("Parse-JSON: No driver architecture.");
          }

          if (jdriver.contains("date") && jdriver["date"].is_string()) {
            pdriver->m_strDate = jdriver["date"];
            spdlog::trace("Parse-JSON: Driver date: {0} ", pdriver->m_strDate);
          }
          else {
            spdlog::warn("Parse-JSON: No driver date");
          }

          // "major.minor.patch" string form (the separate keys below win)
          if (jdriver.contains("version") && jdriver["version"].is_string() && !jdriver.contains("version_major")) {
            pdriver->setVersion(jdriver["version"].get<std::string>());
          }

          if (jdriver.contains("version_major") && jdriver["version_major"].is_number()) {
            pdriver->m_version_major = jdriver["version_major"];
            spdlog::trace("Parse-JSON: Driver version major: {0} ", pdriver->m_version_major);
          }
          else {
            spdlog::warn("Parse-JSON: No driver version_major");
          }

          if (jdriver.contains("version_major") && jdriver["version_major"].is_string()) {
            pdriver->m_version_major = vscp_readStringValue(jdriver["version_major"]);
            spdlog::trace("Parse-JSON: Driver version major: {0} ", pdriver->m_version_major);
          }
          else {
            spdlog::warn("Parse-JSON: No driver version_major");
          }

          if (jdriver.contains("version_minor") && jdriver["version_minor"].is_number()) {
            pdriver->m_version_minor = jdriver["version_minor"];
            spdlog::trace("Parse-JSON: Driver version minor: {0} ", pdriver->m_version_minor);
          }
          else {
            spdlog::warn("Parse-JSON: No driver version_minor");
          }

          if (jdriver.contains("version_minor") && jdriver["version_minor"].is_string()) {
            pdriver->m_version_minor = vscp_readStringValue(jdriver["version_minor"]);
            spdlog::trace("Parse-JSON: Driver version minor: {0} ", pdriver->m_version_minor);
          }
          else {
            spdlog::warn("Parse-JSON: No driver version_minor");
          }

          if (jdriver.contains("version_patch") && jdriver["version_patch"].is_number()) {
            pdriver->m_version_patch = jdriver["version_patch"];
            spdlog::trace("Parse-JSON: Driver version patch: {0} ", pdriver->m_version_patch);
          }
          else {
            spdlog::warn("Parse-JSON: No driver version patch");
          }

          if (jdriver.contains("version_patch") && jdriver["version_patch"].is_string()) {
            pdriver->m_version_patch = vscp_readStringValue(jdriver["version_patch"]);
            spdlog::trace("Parse-JSON: Driver version patch: {0} ", pdriver->m_version_patch);
          }
          else {
            spdlog::warn("Parse-JSON: No driver version patch");
          }

          if (jdriver.contains("md5sum") && jdriver["md5sum"].is_string()) {
            pdriver->m_strMd5 = jdriver["md5sum"];
            spdlog::trace("Parse-JSON: Driver version md5: {0} ", pdriver->m_strMd5);
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
            spdlog::trace("Parse-JSON: Manual name: {0} ", pmanual->m_strName);
          }
          else {
            spdlog::warn("Parse-JSON: No manual name.");
          }

          if (jmanual.contains("url") && jmanual["url"].is_string()) {
            pmanual->m_strURL = jmanual["url"];
            spdlog::trace("Parse-JSON: Manual URL: {0} ", pmanual->m_strURL);
          }
          else {
            spdlog::warn("Parse-JSON: No manual url.");
          }

          if (jmanual.contains("format") && jmanual["format"].is_string()) {
            pmanual->m_strFormat = jmanual["format"];
            spdlog::trace("Parse-JSON: Manual Format: {0} ", pmanual->m_strFormat);
          }
          else {
            spdlog::warn("Parse-JSON: No manual format.");
          }

          if (jmanual.contains("lang") && jmanual["lang"].is_string()) {
            pmanual->m_strLanguage = jmanual["lang"];
            spdlog::trace("Parse-JSON: Manual language: {0} ", pmanual->m_strLanguage);
          }
          else {
            spdlog::warn("Parse-JSON: No manual language.");
          }

          if (jmanual.contains("version") && jmanual["version"].is_string()) {
            pmanual->m_strVersion = jmanual["version"];
          }

          if (jmanual.contains("date") && jmanual["date"].is_string()) {
            pmanual->m_strDate = jmanual["date"];
            spdlog::trace("Parse-JSON: Manual date: {0} ", pmanual->m_strDate);
          }
          else {
            spdlog::warn("Parse-JSON: No manual date");
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
            spdlog::trace("Parse-JSON: Setup name: {0} ", psetup->m_strName);
          }
          else {
            spdlog::warn("Parse-JSON: No setup name.");
          }

          if (jsetup.contains("url") && jsetup["url"].is_string()) {
            psetup->m_strURL = jsetup["url"];
            spdlog::trace("Parse-JSON: Setup URL: {0} ", psetup->m_strURL);
          }
          else {
            spdlog::warn("Parse-JSON: No setup url.");
          }

          if (jsetup.contains("format") && jsetup["format"].is_string()) {
            psetup->m_strFormat = jsetup["format"];
            spdlog::trace("Parse-JSON: Setup Format: {0} ", psetup->m_strFormat);
          }
          else {
            spdlog::warn("Parse-JSON: No setup format.");
          }

          if (jsetup.contains("date") && jsetup["date"].is_string()) {
            psetup->m_strDate = jsetup["date"];
            spdlog::trace("Parse-JSON: Setup date: {0} ", psetup->m_strDate);
          }
          else {
            spdlog::warn("Parse-JSON: No setup date");
          }

          if (jsetup.contains("version") && jsetup["version"].is_string()) {
            psetup->m_strVersion = jsetup["version"];
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
  }
  catch (const json::exception &ex) {
    spdlog::error("Parse-JSON: JSON type/parsing error while reading module data: {}", ex.what());
    return VSCP_ERROR_PARSING;
  }
  catch (const std::exception &ex) {
    spdlog::error("Parse-JSON: Unexpected error while reading module data: {}", ex.what());
    return VSCP_ERROR_PARSING;
  }
  catch (...) {
    spdlog::error("Parse-JSON: Unknown error while reading module data.");
    return VSCP_ERROR_PARSING;
  }

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
CMDF::parseMDF_raw(const std::string &path)
{
  int rv = VSCP_ERROR_ERROR;
  std::ifstream ifs;

  spdlog::trace("Parse-XML: Parsing MDF file {}", path);

  // Check if file exists
  if (!vscp_fileExists(path)) {
    spdlog::debug("Parse-XML: MDF file {} does not exist", path);
    m_strLastError = "File " + path + " does not exist";
    return VSCP_ERROR_INVALID_PATH;
  }

  // Check format
  // ------------
  // If the file is a JSON file we will parse it
  // as JSON else we will parse it as XML. The first
  // character determines the type "{" for JSON or "<"
  // for XML. Whitespace is ignored.

  try {
    ifs.open(path, std::ifstream::in | std::ifstream::binary);
  }
  catch (...) {
    spdlog::error("Parse-XML: Failed to open file {}", path);
    m_strLastError = "Unable to open " + path;
    return VSCP_ERROR_NOT_OPEN;
  }

  if (!ifs.is_open()) {
    spdlog::error("Parse-XML: Failed to open file {}", path);
    m_strLastError = "Unable to open " + path;
    return VSCP_ERROR_NOT_OPEN;
  }

  try {
    // The first character that is not white space (after an optional
    // UTF-8 byte order mark) decides the format. Blank leading lines are
    // allowed and the whole file is parsed (the first line used to be
    // consumed before XML parsing started).
    int ch;
    bool bFirst = true;
    while (EOF != (ch = ifs.get())) {
      if (bFirst && (0xEF == ch) && (0xBB == ifs.peek())) {
        ifs.get();
        if (0xBF == ifs.peek()) {
          ifs.get();
        }
        bFirst = false;
        continue;
      }
      bFirst = false;
      if (!isspace(ch)) {
        break;
      }
    }

    if ('{' == ch) {
      spdlog::info("Parse-XML: MDF file format is JSON");
      ifs.close();
      rv = parseMDF_JSON_raw(path);
    }
    else if ('<' == ch) {
      spdlog::info("Parse-XML: MDF file format is XML");
      ifs.clear();
      ifs.seekg(0, std::ios::beg);
      rv = parseMDF_XML_raw(ifs);
      ifs.close();
    }
    else {
      rv = VSCP_ERROR_INVALID_SYNTAX;
      spdlog::error("Parse-XML: MDF file format not supported");
      m_strLastError = "The file is neither XML nor JSON (it must start with '<' or '{')";
    }
  }  catch (const json::exception &ex) {
    spdlog::error("Parse-XML: JSON parsing exception for {}: {}", path, ex.what());
    m_strLastError = std::string("JSON error: ") + ex.what();
    rv = VSCP_ERROR_PARSING;
  }
  catch (const std::exception &ex) {
    spdlog::error("Parse-XML: Parsing exception for {}: {}", path, ex.what());
    m_strLastError = std::string("Parse error: ") + ex.what();
    rv = VSCP_ERROR_PARSING;
  }
  catch (...) {
    spdlog::error("Parse-XML: Unknown parsing exception for {}", path);
    m_strLastError = "Unknown parse error";
    rv = VSCP_ERROR_PARSING;
  }

  if ((VSCP_ERROR_SUCCESS != rv) && m_strLastError.empty()) {
    m_strLastError = "Failed to parse " + path + " (error " + std::to_string(rv) + ")";
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  parseMDF / parseMDF_XML / parseMDF_JSON
//
// Parsing is done into a new object. The content of this object is only
// replaced when the whole file has been parsed, so a file that fails to
// parse never leaves half of a model (or none) behind.
//

int
CMDF::parseMDF(const std::string &path)
{
  m_strLastError.clear();
  CMDF tmp;
  tmp.m_strLocale = m_strLocale;
  int rv          = tmp.parseMDF_raw(path);
  if (VSCP_ERROR_SUCCESS == rv) {
    swapContent(tmp);
  }
  else {
    m_strLastError = tmp.m_strLastError;
  }
  return rv;
}

int
CMDF::parseMDF_XML(std::ifstream &ifs)
{
  m_strLastError.clear();
  CMDF tmp;
  tmp.m_strLocale = m_strLocale;
  int rv;
  try {
    rv = tmp.parseMDF_XML_raw(ifs);
  }
  catch (const std::exception &ex) {
    tmp.m_strLastError = std::string("Parse error: ") + ex.what();
    rv                 = VSCP_ERROR_PARSING;
  }
  if (VSCP_ERROR_SUCCESS == rv) {
    swapContent(tmp);
  }
  else {
    m_strLastError = tmp.m_strLastError.empty() ? "Failed to parse XML MDF" : tmp.m_strLastError;
  }
  return rv;
}

int
CMDF::parseMDF_JSON(const std::string &path)
{
  m_strLastError.clear();
  CMDF tmp;
  tmp.m_strLocale = m_strLocale;
  int rv;
  try {
    rv = tmp.parseMDF_JSON_raw(path);
  }
  catch (const std::exception &ex) {
    tmp.m_strLastError = std::string("Parse error: ") + ex.what();
    rv                 = VSCP_ERROR_PARSING;
  }
  if (VSCP_ERROR_SUCCESS == rv) {
    swapContent(tmp);
  }
  else {
    m_strLastError = tmp.m_strLastError.empty() ? "Failed to parse JSON MDF" : tmp.m_strLastError;
  }
  return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  swapContent
//

void
CMDF::swapContent(CMDF &other)
{
  std::swap(m_redirectUrl, other.m_redirectUrl);
  std::swap(m_vscpLevel, other.m_vscpLevel);
  std::swap(m_name, other.m_name);
  std::swap(m_copyright, other.m_copyright);
  std::swap(m_mapDescription, other.m_mapDescription);
  std::swap(m_mapInfoURL, other.m_mapInfoURL);
  std::swap(m_strModule_changeDate, other.m_strModule_changeDate);
  std::swap(m_strModule_Model, other.m_strModule_Model);
  std::swap(m_strModule_Version, other.m_strModule_Version);
  std::swap(m_module_bufferSize, other.m_module_bufferSize);

  // Manufacturer (member by member, the lists own their items)
  std::swap(m_manufacturer.m_strName, other.m_manufacturer.m_strName);
  std::swap(m_manufacturer.m_address.m_strStreet, other.m_manufacturer.m_address.m_strStreet);
  std::swap(m_manufacturer.m_address.m_strTown, other.m_manufacturer.m_address.m_strTown);
  std::swap(m_manufacturer.m_address.m_strCity, other.m_manufacturer.m_address.m_strCity);
  std::swap(m_manufacturer.m_address.m_strPostCode, other.m_manufacturer.m_address.m_strPostCode);
  std::swap(m_manufacturer.m_address.m_strState, other.m_manufacturer.m_address.m_strState);
  std::swap(m_manufacturer.m_address.m_strRegion, other.m_manufacturer.m_address.m_strRegion);
  std::swap(m_manufacturer.m_address.m_strCountry, other.m_manufacturer.m_address.m_strCountry);
  std::swap(m_manufacturer.m_list_Phone, other.m_manufacturer.m_list_Phone);
  std::swap(m_manufacturer.m_list_Fax, other.m_manufacturer.m_list_Fax);
  std::swap(m_manufacturer.m_list_Email, other.m_manufacturer.m_list_Email);
  std::swap(m_manufacturer.m_list_Web, other.m_manufacturer.m_list_Web);
  std::swap(m_manufacturer.m_list_Social, other.m_manufacturer.m_list_Social);

  std::swap(m_list_picture, other.m_list_picture);
  std::swap(m_list_video, other.m_list_video);
  std::swap(m_list_firmware, other.m_list_firmware);
  std::swap(m_list_driver, other.m_list_driver);
  std::swap(m_list_manual, other.m_list_manual);
  std::swap(m_list_setup, other.m_list_setup);

  // Decision matrix
  std::swap(m_dmInfo.m_level, other.m_dmInfo.m_level);
  std::swap(m_dmInfo.m_startPage, other.m_dmInfo.m_startPage);
  std::swap(m_dmInfo.m_startOffset, other.m_dmInfo.m_startOffset);
  std::swap(m_dmInfo.m_rowCount, other.m_dmInfo.m_rowCount);
  std::swap(m_dmInfo.m_rowSize, other.m_dmInfo.m_rowSize);
  std::swap(m_dmInfo.m_list_action, other.m_dmInfo.m_list_action);

  // Boot loader
  std::swap(m_bootInfo.m_nAlgorithm, other.m_bootInfo.m_nAlgorithm);
  std::swap(m_bootInfo.m_nBlockSize, other.m_bootInfo.m_nBlockSize);
  std::swap(m_bootInfo.m_nBlockCount, other.m_bootInfo.m_nBlockCount);

  std::swap(m_list_register, other.m_list_register);
  std::swap(m_list_remotevar, other.m_list_remotevar);
  std::swap(m_list_event, other.m_list_event);
  std::swap(m_list_alarm, other.m_list_alarm);
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
//  getRegister
//

bool
CMDF::isRegisterWriteable(uint32_t reg, uint16_t page)
{
  CMDF_Register *preg = getRegister(reg, page);
  if (nullptr == preg) {
    return false;
  }

  if (!preg->m_access || (preg->m_access & MDF_REG_ACCESS_READ_ONLY)) {
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getDefaultRegisterValue
//

int
CMDF::getDefaultRegisterValue(uint32_t reg, uint16_t page)
{
  CMDF_Register *preg = getRegister(reg, page);
  if (nullptr == preg) {
    return -1;
  }

  uint8_t val;
  if (preg->getDefault(val)) {
    return -1;
  }

  return val;
}

///////////////////////////////////////////////////////////////////////////////
//  getRemoteVariable
//

CMDF_RemoteVariable *
CMDF::getRemoteVariable(const std::string &name)
{
  std::string remotevar = name;
  vscp_trim(remotevar);
  vscp_makeLower(remotevar);

  std::deque<CMDF_RemoteVariable *>::iterator iter;
  for (iter = m_list_remotevar.begin(); iter != m_list_remotevar.end(); ++iter) {
    CMDF_RemoteVariable *prvar = *iter;
    std::string rname          = prvar->m_name;
    vscp_trim(rname);
    vscp_makeLower(rname);
    if (rname == remotevar) {
      return prvar;
    }
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//  getRemoteVariable
//

CMDF_RemoteVariable *
CMDF::getRemoteVariable(uint32_t offset, uint16_t page)
{
  std::deque<CMDF_RemoteVariable *>::iterator iter;
  for (iter = m_list_remotevar.begin(); iter != m_list_remotevar.end(); ++iter) {
    CMDF_RemoteVariable *pvar = *iter;
    if ((nullptr != pvar) && (page == pvar->getPage()) && (offset == pvar->getOffset())) {
      return pvar;
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
  // std::map<uint32_t, CMDF_Register> mapRegs;
  std::set<uint32_t> regset;

  // Make a sorted set of registers
  for (auto reg : m_list_register) {
    if (page == reg->m_page) {
      regset.insert(reg->m_offset);
    }
  }

  for (auto reg : regset) {
    // std::map<uint32_t, CMDF_Register *> mapRegs;
    mapRegs[reg] = getRegister(reg, page);
  }

  return;
}

///////////////////////////////////////////////////////////////////////////////
// deleteRegister
//

bool
CMDF::deleteRegister(CMDF_Register *preg)
{
  // Check pointer
  if (nullptr == preg) {
    return false;
  }

  for (auto it = m_list_register.cbegin(); it != m_list_register.cend(); ++it) {
    if (preg == *it) {
      m_list_register.erase(it);
      delete preg;
      return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// deleteRemoteVariable
//

bool
CMDF::deleteRemoteVariable(CMDF_RemoteVariable *prvar)
{
  // Check pointer
  if (nullptr == prvar) {
    return false;
  }

  for (auto it = m_list_remotevar.cbegin(); it != m_list_remotevar.cend(); ++it) {
    if (prvar == *it) {
      m_list_remotevar.erase(it);
      delete prvar;
      return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// addEvent
//

bool
CMDF::addEvent(CMDF_Event *pEvent)
{
  if (nullptr != pEvent) {
    m_list_event.push_back(pEvent);
    return true;
  }
  else {
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////////
// deleteEvent
//

bool
CMDF::deleteEvent(CMDF_Event *pEvent)
{
  // Check pointer
  if (nullptr == pEvent) {
    return false;
  }

  for (auto it = m_list_event.cbegin(); it != m_list_event.cend(); ++it) {
    if (pEvent == *it) {
      m_list_event.erase(it);
      delete pEvent;
      return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// expandRegisterSpan
//

// Max number of registers one DMATRIX1/BLOCK register is expanded to. A
// larger span (from a damaged or hostile file) is not expanded: the register
// is kept as one register with its span, so nothing is changed or lost.
#define MDF_MAX_REGISTER_SPAN 4096

void
CMDF::expandRegisterSpan(CMDF_Register *preg)
{
  if (nullptr == preg) {
    return;
  }

  const char *label;
  if (MDF_REG_TYPE_DMATRIX1 == preg->m_type) {
    label = " - DM ";
  }
  else if (MDF_REG_TYPE_BLOCK == preg->m_type) {
    label = " - BLOCK ";
  }
  else {
    return; // Standard register - nothing to expand
  }

  // A span of one is a single register (kept as it is, also its name)
  if (preg->m_span <= 1) {
    return;
  }

  // The span comes from the (possibly remote) MDF file
  if (preg->m_span > MDF_MAX_REGISTER_SPAN) {
    spdlog::warn("MDF: Register '{}' span {} is larger than {}. It is not expanded.",
                 preg->m_name,
                 preg->m_span,
                 MDF_MAX_REGISTER_SPAN);
    return;
  }

  // Apply pyjamas coloring if at least one of the pyjamas colors are set.
  const bool bPyjamas = preg->m_fgeven || preg->m_fgodd || preg->m_bgeven || preg->m_bgodd;

  // Registers are named "<name> - DM 0", "<name> - DM 1", ... for both XML
  // and JSON. (JSON copies used to be named "<name> - DM 0 - DM 1", XML
  // copies had no base name and the XML parser left the first register
  // unnamed by index.)
  const std::string baseName = preg->m_name;
  preg->m_name               = baseName + label + "0";
  // Keep the colors given in the MDF; the pyjamas colors below replace them
  // for display, and save_xml()/save_json() write the MDF ones back.
  preg->m_fgcolorMdf = preg->m_fgcolor;
  preg->m_bgcolorMdf = preg->m_bgcolor;
  preg->m_spanIndex  = 0;
  if (bPyjamas) {
    preg->m_fgcolor = preg->m_fgeven;
    preg->m_bgcolor = preg->m_bgeven;
  }

  for (int pos = 1; pos < preg->m_span; pos++) {
    CMDF_Register *pregNew = new CMDF_Register;
    *pregNew               = *preg; // Deep copy, includes bits and values
    pregNew->m_name        = baseName + label + std::to_string(pos);
    pregNew->m_offset      = preg->m_offset + pos;
    pregNew->m_spanIndex   = pos;
    if (bPyjamas) {
      if (pos % 2) {
        pregNew->m_fgcolor = preg->m_fgodd;
        pregNew->m_bgcolor = preg->m_bgodd;
      }
      else {
        pregNew->m_fgcolor = preg->m_fgeven;
        pregNew->m_bgcolor = preg->m_bgeven;
      }
    }
    m_list_register.push_back(pregNew);
  }
}

///////////////////////////////////////////////////////////////////////////////
// createRegisterSortedSet
//

void
CMDF::createRegisterSortedSet(std::set<uint32_t> &set, uint16_t page)
{
  std::deque<CMDF_Register *> *regset = getRegisterObjList();
  for (auto it = regset->cbegin(); it != regset->cend(); ++it) {
    if (page == (*it)->getPage()) {
      set.insert((*it)->getOffset());
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// format
//

std::string &
CMDF::format(std::string &docs)
{
  size_t idx = 0;

  // Return if empty
  if (!docs.length()) {
    return docs;
  }

  // If first character is a # the string is coded as a
  // markddown document. If not it is coded as a HTML document.

  vscp_trim(docs);

  if (docs.empty()) {
    return docs; // Only white space
  }

  if ('#' == docs.at(0)) {
    // Markdown

    // Replace escapes
    // "\n" -> \n
    do {
      idx = docs.find("\\n", idx);
      if (idx == std::string::npos)
        break;
      docs.replace(idx, 2, "\n");
    } while (true);

    idx = 0; // Start over for the next pass

#ifdef VSCP_HAVE_MADDY
    std::stringstream markdownInput(docs);
    std::shared_ptr<maddy::Parser> parser = std::make_shared<maddy::Parser>();
    docs                                  = parser->Parse(markdownInput);
#else
    do {
      idx = docs.find("\n", idx);
      if (idx == std::string::npos)
        break;
      docs.replace(idx, 1, "<br>");
    } while (true);
#endif
  }
  else {
    // HTML

    // Replace escapes
    // "/n" -> <br>
    do {
      idx = docs.find("\\n", idx);
      if (idx == std::string::npos)
        break;
      docs.replace(idx, 2, "<br>");
    } while (true);
  }
  return docs;
}
