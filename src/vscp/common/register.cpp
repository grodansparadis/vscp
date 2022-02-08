// FILE: register.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2022 Ake Hedman, the VSCP project <info@vscp.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifdef WIN32
#include <pch.h>
#else
#include <unistd.h>
#endif

#include <map>
#include <set>
#include <string>

#include <mdf.h>
#include <vscp.h>
#include <vscp_client_base.h>
#include <register.h>
#include <vscphelper.h>

///////////////////////////////////////////////////////////////////////////////
//  vscp_readRegister
//

/*!
  value (0-255) or error code if negative
*/

int
vscp_readLevel1Register(CVscpClient& client,
                          cguid& guidNode,
                          cguid& guidInterface,
                          uint16_t page,
                          uint8_t offset,
                          uint8_t& value,
                          uint32_t timeout)
{
  int rv = VSCP_ERROR_ERROR;
  vscpEventEx ex;

  CVscpClient::connType conntype = client.getType();
  uint8_t nickname               = guidNode.getNickname();
  uint8_t ifoffset               = guidInterface.isNULL() ? 0 : 16;

  ex.head = VSCP_PRIORITY_NORMAL;
  ex.vscp_class = VSCP_CLASS1_PROTOCOL + (guidInterface.isNULL() ? 0 : 512);
  ex.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ;
  ex.sizeData   = ifoffset + 5;
  if (ifoffset) memcpy(ex.data, guidInterface.getGUID(), 16);
  ex.data[0 + ifoffset]    = nickname;
  ex.data[1 + ifoffset]    = (page >> 8) & 0x0ff;
  ex.data[2 + ifoffset]    = page & 0x0ff;
  ex.data[3 + ifoffset]    = offset;
  ex.data[4 + ifoffset]    = 1;

  // Send event
  if (VSCP_ERROR_SUCCESS != (rv = client.send(ex))) {
    return rv;
  }

  // uint32_t resendTime = m_registerOpResendTimeout;
  uint32_t startTime = vscp_getMsTimeStamp();

  // Wait for reponse

  do {
    // Get response
    uint16_t count;
    rv = client.getcount(&count);
    if (count &&
        VSCP_ERROR_SUCCESS == rv) {

      if (VSCP_ERROR_SUCCESS != (rv = client.receive(ex))) {
        return rv;
      }

      if (VSCP_CLASS1_PROTOCOL == ex.vscp_class &&
          VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == ex.vscp_type) {
        if ((ex.GUID[15] = nickname) && 
              (ex.sizeData >= 5) && 
              (ex.data[0] == 0) &&
              (ex.data[1] == (page >> 8) & 0x0ff) && 
              (ex.data[2] == page & 0x0ff) && 
              (ex.data[3] == offset)) {
          value = ex.data[4];
          return VSCP_ERROR_SUCCESS;
        }
      }
    }

    if (timeout && ((vscp_getMsTimeStamp() - startTime) > timeout)) {
      rv = VSCP_ERROR_TIMEOUT;
      break;
    }
    

#ifdef WIN32
    win_usleep(2000);    
#else
    usleep(2000);
#endif

  } while (timeout);

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  vscp_writeRegister
//

int
vscp_writeLevel1Register(CVscpClient& client,
                          cguid& guidNode,
                          cguid& guidInterface,
                          uint16_t page,
                          uint8_t offset,
                          uint8_t value,
                          uint32_t timeout)
{
  int rv = VSCP_ERROR_ERROR;
  vscpEventEx ex;
  CVscpClient::connType conntype = client.getType();
  uint8_t nickname               = guidNode.getNickname();
  uint8_t ifoffset               = guidInterface.isNULL() ? 0 : 16;

  ex.head = VSCP_PRIORITY_NORMAL;
  ex.vscp_class = VSCP_CLASS1_PROTOCOL+ (guidInterface.isNULL() ? 0 : 512);;
  ex.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE;
  ex.sizeData   = ifoffset + 5;
  if (ifoffset) memcpy(ex.data, guidInterface.getGUID(), 16);
  ex.data[0 + ifoffset]    = nickname;
  ex.data[1 + ifoffset]    = (page >> 8) & 0x0ff;
  ex.data[2 + ifoffset]    = page & 0x0ff;
  ex.data[3 + ifoffset]    = offset;
  ex.data[4 + ifoffset]    = value;

  // Send event
  if (VSCP_ERROR_SUCCESS != (rv = client.send(ex))) {
    return rv;
  }

  uint32_t startTime = vscp_getMsTimeStamp();

  // Wait for reponse
  
  do {
    // Get response
    uint16_t count;
    rv = client.getcount(&count);
    if (count &&
        VSCP_ERROR_SUCCESS == rv) {

      if (VSCP_ERROR_SUCCESS != (rv = client.receive(ex))) {
        return rv;
      }

      if (VSCP_CLASS1_PROTOCOL == ex.vscp_class) {
        if (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == ex.vscp_type) {
          if ((ex.GUID[15] = nickname) && 
              (ex.sizeData >= 5) && 
              (ex.data[0] == 0) &&
              (ex.data[1] == (page >> 8) & 0x0ff) && 
              (ex.data[2] == page & 0x0ff) && 
              (ex.data[3] == offset) && 
              (ex.data[4] == value)) {
            return VSCP_ERROR_SUCCESS;
          }
        }
      }
    }

    if (timeout && ((vscp_getMsTimeStamp() - startTime) > timeout)) {
      rv = VSCP_ERROR_TIMEOUT;
      break;
    }

#ifdef WIN32
    win_usleep(2000);    
#else
    usleep(2000);
#endif

  } while (timeout);

  return rv;
}


///////////////////////////////////////////////////////////////////////////////
//  vscp_scanForDevices
//

int vscp_scanForDevices(CVscpClient& client,
                          cguid& guid,
                          std::set<uint8_t> &found,
                          uint32_t timeout)
{
  int rv = VSCP_ERROR_SUCCESS;
  uint8_t offset = guid.isNULL() ? 0 : 16;
  vscpEventEx ex;

  memset(&ex, 0, sizeof(vscpEventEx));
  ex.vscp_class = VSCP_CLASS1_PROTOCOL + (guid.isNULL() ? 0 : 512);
  ex.vscp_type  = VSCP_TYPE_PROTOCOL_WHO_IS_THERE;
  memcpy(ex.data + offset, guid.getGUID(), 16); // Use GUID of interface
  memset(ex.GUID, 0, 16); // Use GUID of interface
  ex.sizeData   = 17;  
  ex.data[16] = 0xff;  // all devices

  if (VSCP_ERROR_SUCCESS != (rv = client.send(ex))) {
    return rv;
  }

  uint32_t startTime = vscp_getMsTimeStamp();

  while (true) {
    uint16_t cnt;
    client.getcount(&cnt);
    if (cnt) {
      rv = client.receive(ex);
    }
    if (VSCP_ERROR_SUCCESS == rv) {
      if ( ex.vscp_class == VSCP_CLASS1_PROTOCOL &&
          ex.vscp_type == VSCP_TYPE_PROTOCOL_WHO_IS_THERE_RESPONSE ) {
        found.insert(ex.GUID[15]);
      }
    }

    // Check timeout
    if ((vscp_getMsTimeStamp() - startTime) > 500) {
      rv = VSCP_ERROR_TIMEOUT;
      break;
    }
  }
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  vscp_scanSlowForDevices
//

int vscp_scanSlowForDevices(CVscpClient& client,
                              cguid& guid,
                              std::set<uint8_t> &found_nodes,
                              uint8_t start_node,
                              uint8_t end_node,
                              uint32_t delay,
                              uint32_t timeout)
{
  int rv = VSCP_ERROR_SUCCESS;
  vscpEventEx ex;
  for (int idx = start_node; idx<end_node; idx++) {    
    memset(&ex, 0, sizeof(vscpEventEx));
    ex.vscp_class = VSCP_CLASS1_PROTOCOL + 512;
    ex.vscp_type  = VSCP_TYPE_PROTOCOL_READ_REGISTER;
    memcpy(ex.data, guid.getGUID(), 16); // Use GUID of interface
    memset(ex.GUID, 0, 16); // Use GUID of interface
    ex.sizeData   = 18;
    
    ex.data[16] = idx;  // nodeid
    ex.data[17] = 0xd0; // register: First byte of GUID

    rv = client.send(ex);
    if (VSCP_ERROR_SUCCESS != rv) {
      return rv;
    }

#ifdef WIN32
    win_usleep(delay);
#else
    usleep(delay);
#endif    
  } 

  uint32_t startTime = vscp_getMsTimeStamp();

  while (true) {
    uint16_t cnt;

    rv = client.getcount(&cnt);
    if (cnt) {      
      rv = client.receive(ex);
      if (VSCP_ERROR_SUCCESS != rv) {
        return rv;
      }
      if ( VSCP_ERROR_SUCCESS == rv && 
          ex.vscp_class == VSCP_CLASS1_PROTOCOL &&
          ex.vscp_type == VSCP_TYPE_PROTOCOL_RW_RESPONSE ) {
        found_nodes.insert(ex.GUID[15]);
      }     
    }

    if ((vscp_getMsTimeStamp() - startTime) > timeout) {
      break;
    }
  }

  return rv;
}


// --------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
//  Constructor
//

CRegisterPage::CRegisterPage(uint8_t level, uint16_t page)
{
  m_level = level;
  m_page  = page;
}

CRegisterPage::~CRegisterPage()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// getReg
//

int
CRegisterPage::getReg(uint32_t offset)
{
  // Check validity of register pointer
  if (VSCP_LEVEL1 == m_level) {
    if (offset > 127) {
      return -1; // Invalid reg offset for Level I device
    }
  }

  return m_registers[offset];
}

///////////////////////////////////////////////////////////////////////////////
// putReg
//

int
CRegisterPage::putReg(uint32_t reg, uint8_t value)
{
  // Check validity of register pointer
  if (VSCP_LEVEL1 == m_level) {
    if (reg > 128)
      return -1; // Invalid reg offset for Level I device
  }

  return m_registers[reg] = value; // Assign value
}

//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//  Constructor
//

CStandardRegisters::CStandardRegisters()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
//  Destructor
//

CStandardRegisters::~CStandardRegisters() {}

///////////////////////////////////////////////////////////////////////////////
//  getFirmwareVersionString.
//

std::string
CStandardRegisters::getFirmwareVersionString(void)
{
  std::string str;

  str = str = vscp_str_format("%d.%d.%d", m_reg[0x94 - 0x80], m_reg[0x95 - 0x80], m_reg[0x96 - 0x80]);
  return str;
}

///////////////////////////////////////////////////////////////////////////////
//  getMDF
//

std::string
CStandardRegisters::getMDF(void)
{
  char url[33];
  std::string remoteFile;
  
  memset(url, 0, sizeof(url));
  memcpy(url, (m_reg + 0xe0 - 0x80), 32);
  remoteFile = "http://";
  remoteFile += std::string(url);
  return(remoteFile);
}



//-----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
//  Constructor
//

CUserRegisters::CUserRegisters(uint8_t level)
{
  m_level = level;
}

///////////////////////////////////////////////////////////////////////////////
//  Destructor
//

CUserRegisters::~CUserRegisters() 
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
//  getRegPointer
//

std::map<uint32_t, uint8_t> *CUserRegisters::getRegisterMap( uint16_t page ) 
{ 
  CRegisterPage *pPage = NULL;

  pPage = m_registerPageMap[page]; 
  if (nullptr == pPage) {
    return nullptr;
  }

  return pPage->getRegisterMap();
}

///////////////////////////////////////////////////////////////////////////////
// getRegisterPage
//

CRegisterPage *
CUserRegisters::getRegisterPage(uint16_t page)
{
  return m_registerPageMap[page];
}

int
CUserRegisters::getRegister(uint16_t page, uint32_t offset)
{
  CRegisterPage *ppage = m_registerPageMap[page];
  if (nullptr == ppage) {
    return -1;
  }

  return ppage->getRegisterMap()->at(offset);
}

///////////////////////////////////////////////////////////////////////////////
// putReg
//

bool
CUserRegisters::putReg(uint32_t reg, uint32_t page, uint8_t value)
{
  // Check validity of register pointer
  if (VSCP_LEVEL1 == m_level) {
    if (reg > 127) {
      return false; // Invalid reg offset for Level I device
    }
  }
  else if (VSCP_LEVEL2 == m_level) {
    if (page > 0xffffff80) {
      return false; // Invalid page for level II device
    }
  }
  else {    
    return false;  // Level is wrong
  }

  std::map<uint16_t, CRegisterPage *>::iterator it;
  if (m_registerPageMap.end() != (it = m_registerPageMap.find(page))) {

    // Page already exist
    CRegisterPage *pPage = it->second;
    if (NULL == pPage) {
      return false; // Invalid page
    }

    pPage->putReg(reg, value); // Assign value
  }
  else {

    // Page does not exist

    // Create page
    CRegisterPage *pPage = new CRegisterPage(m_level);
    if (NULL == pPage) {
      return false; // Unable to create page
    }
    m_registerPageMap[page] = pPage;

    pPage->putReg(reg, value); // Assign value
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
//  remoteVarFromRegToString
//

bool
CUserRegisters::remoteVarFromRegToString(CMDF_RemoteVariable& remoteVar, 
                                          std::string& strValue, 
                                          uint8_t format)
{

  bool rv = false;
  CRegisterPage *ppage;

  // Get register page
  if (NULL == (ppage = getRegisterPage(remoteVar.getPage()))) {
    return false;
  }

  // vscp_remote_variable_type
  switch (remoteVar.getType()) {

    case remote_variable_type_string:
      {
        uint8_t *pstr;
        pstr = new uint8_t[remoteVar.getTypeByteCount() + 1 ];
        if ( NULL == pstr ) return false;
        memset(pstr, 0, sizeof(pstr));
        for (int i = remoteVar.getOffset(); 
              i < remoteVar.getOffset() + remoteVar.getTypeByteCount(); 
              i++) {
          pstr[i] = ppage->getReg(i);
        }
        strValue = (const char *)pstr;
        delete [] pstr;
        return true;
      }
      break;

    case remote_variable_type_boolean:
      {
        strValue = ppage->getReg(remoteVar.getOffset()) ? "true" : "false";
      }
      break;

    case remote_variable_type_int8_t:
      {
        if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
          strValue = vscp_str_format("%d", ppage->getReg(remoteVar.getOffset()));
        }
        else {
          strValue = vscp_str_format( "0x%02x", ppage->getReg(remoteVar.getOffset()));
        }
      }
      break;

    case remote_variable_type_uint8_t:
      {
        if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
          strValue = vscp_str_format( "%ud", ppage->getReg(remoteVar.getOffset()));
        }
        else {
          strValue = vscp_str_format( "0x%02x", ppage->getReg(remoteVar.getOffset()));
        }
      }
      break;

    case remote_variable_type_int16_t:
      {
        uint8_t buf[2];
        buf[0] = ppage->getReg(remoteVar.getOffset());
        buf[1] = ppage->getReg(remoteVar.getOffset()+1);
        int16_t val = (buf[0] << 8 ) + buf[1];

        if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
          strValue = vscp_str_format( "%d", val );
        }
        else {
          strValue = vscp_str_format( "%04x", val );
          strValue = "0x";
          strValue += strValue.substr( strValue.length() - 4 ); // Handles negative numbers correct
        }
      }
      break;

    case remote_variable_type_uint16_t:
      {
        uint8_t buf[2];
        buf[0] = ppage->getReg(remoteVar.getOffset());
        buf[1] = ppage->getReg(remoteVar.getOffset()+1);
        uint16_t val = (buf[0] << 8 ) + buf[1];

        if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
          strValue = vscp_str_format( "%ud", val );
        }
        else {
          strValue = vscp_str_format( "0x%04x", val );
        }
      }
      break;

    case remote_variable_type_int32_t:
      {
        uint8_t buf[remoteVar.getTypeByteCount()];
        for ( int i = 0; i < remoteVar.getTypeByteCount(); i++ ) {
          buf[i] = ppage->getReg(remoteVar.getOffset() + i);
        }
        int32_t val = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
        if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
          strValue = vscp_str_format( "%ld", val);
        }
        else {
          strValue = vscp_str_format( "%04lx", val);
          strValue = "0x";
          strValue += strValue.substr( strValue.length() - 8 ); // Handles negative numbers correct
        }
      }
      break;

    case remote_variable_type_uint32_t:
      {
        uint8_t buf[remoteVar.getTypeByteCount()];
        for ( int i = 0; i < remoteVar.getTypeByteCount(); i++ ) {
          buf[i] = ppage->getReg(remoteVar.getOffset() + i);
        }
        uint32_t val = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
        if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
          strValue = vscp_str_format( "%ld", val);
        }
        else {
          strValue = vscp_str_format( "%04lx", val);
          strValue = "0x";
          strValue += strValue.substr( strValue.length() - 8 ); // Handles negative numbers correct
        }
      }
      break;

    case remote_variable_type_int64_t:
      {
        uint8_t buf[remoteVar.getTypeByteCount()];
        for ( int i = 0; i < remoteVar.getTypeByteCount(); i++ ) {
          buf[i] = ppage->getReg(remoteVar.getOffset() + i);
        }

        int64_t val =  (int64_t)(((uint64_t)buf[0] << 56) +
                        ((uint64_t)buf[1] << 48) +
                        ((uint64_t)buf[2] << 40) +
                        ((uint64_t)buf[3] << 32) +
                        ((uint64_t)buf[4] << 24) + 
                        ((uint64_t)buf[5] << 16) + 
                        ((uint64_t)buf[6] << 8) + 
                         (uint64_t)buf[7]);

        if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
            strValue = vscp_str_format( "%lld", val );
        }
        else {
            strValue = vscp_str_format( "%llx", val );
            strValue = "0x";
            strValue += strValue.substr( strValue.length() - 8 ); // Handles negative numbers correct
        }
      }
      break;

    case remote_variable_type_uint64_t:
      {
        uint8_t buf[remoteVar.getTypeByteCount()];
        for ( int i = 0; i < remoteVar.getTypeByteCount(); i++ ) {
          buf[i] = ppage->getReg(remoteVar.getOffset() + i);
        }

        uint64_t val =  ((uint64_t)buf[0] << 56) +
                        ((uint64_t)buf[1] << 48) +
                        ((uint64_t)buf[2] << 40) +
                        ((uint64_t)buf[3] << 32) +
                        ((uint64_t)buf[4] << 24) + 
                        ((uint64_t)buf[5] << 16) + 
                        ((uint64_t)buf[6] << 8) + 
                         (uint64_t)buf[7];
        if ( FORMAT_ABSTRACTION_DECIMAL == format ) {
            strValue = vscp_str_format("%ulld", val);
        }
        else {
            strValue = vscp_str_format("0x%ullx", val);
        }
      }
      break;

    case remote_variable_type_float:
      {
        uint8_t buf[remoteVar.getTypeByteCount()];
        for ( int i = 0; i < remoteVar.getTypeByteCount(); i++ ) {
          buf[i] = ppage->getReg(remoteVar.getOffset() + i);
        }
        uint32_t n = VSCP_UINT32_SWAP_ON_LE( *((uint32_t *)buf));
        float f = *( (float *)((uint8_t *)&n ) );
        strValue = vscp_str_format( "%f", *((float *)buf));
      }
      break;

    case remote_variable_type_double:
      {
        uint8_t buf[remoteVar.getTypeByteCount()];
        for ( int i = 0; i < remoteVar.getTypeByteCount(); i++ ) {
          buf[i] = ppage->getReg(remoteVar.getOffset() + i);
        }
        uint64_t n = VSCP_UINT32_SWAP_ON_LE( *( (uint32_t *)buf));
        double f = *( (double *)((uint8_t *)&n ) );
        strValue = vscp_str_format( "%g", *((double *)buf));
      }
      break;

    case remote_variable_type_date:
      {
        uint8_t buf[remoteVar.getTypeByteCount()];
        for ( int i = 0; i < remoteVar.getTypeByteCount(); i++ ) {
          buf[i] = ppage->getReg(remoteVar.getOffset() + i);
        }
        strValue = vscp_str_format("%02d-%02d-%02d", 
                                    *((uint8_t *)buf),
                                    *((uint8_t *)(buf+2)),
                                    *((uint8_t *)(buf+4)));
      }
      break;

    case remote_variable_type_time:
      {
        uint8_t buf[remoteVar.getTypeByteCount()];
        for ( int i = 0; i < remoteVar.getTypeByteCount(); i++ ) {
          buf[i] = ppage->getReg(remoteVar.getOffset() + i);
        }
        strValue = vscp_str_format("%02d:%02d:%02d", 
                                    *((uint8_t *)buf),
                                    *((uint8_t *)(buf+2)),
                                    *((uint8_t *)(buf+4)));
      }
      break;

    case remote_variable_type_unknown:
    default:
        strValue = "";
        break;
    }
  return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  remoteVarFromStringToReg
//

bool
CUserRegisters::remoteVarFromStringToReg(CMDF_RemoteVariable& remoteVar, std::string &strValue)
{
  bool rv = false;
  switch (remoteVar.getType()) {

    case remote_variable_type_string:
      {

      }
      break;

    case remote_variable_type_boolean:
      {

      }
      break;

    case remote_variable_type_int8_t:
      {

      }
      break;

    case remote_variable_type_uint8_t:
      {

      }
      break;

    case remote_variable_type_int16_t:
      {

      }
      break;

    case remote_variable_type_uint16_t:
      {

      }
      break;

    case remote_variable_type_int32_t:
      {

      }
      break;

    case remote_variable_type_uint32_t:
      {

      }
      break;

    case remote_variable_type_int64_t:
      {
        
      }
      break;

    case remote_variable_type_uint64_t:
      {
        
      }
      break;

    case remote_variable_type_float:
      {
        
      }
      break;

    case remote_variable_type_double:
      {

      }
      break;

    case remote_variable_type_date:
      {
 
      }
      break;

    case remote_variable_type_time:
      {

      }
      break;

    case remote_variable_type_unknown:
    default:
        break;
    }
  return rv;
}


///////////////////////////////////////////////////////////////////////////////


CVscpNode::CVscpNode()
{

}


CVscpNode::~CVscpNode()
{
  
}