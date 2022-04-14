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
#include <iostream>

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

  // Clear input queue
  if (VSCP_ERROR_SUCCESS != (rv = client.clear())) {
    return rv;
  }

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

  
  // Clear the input queue
  if (VSCP_ERROR_SUCCESS != (rv = client.clear())) {
    return rv;
  }

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

    //std::cout << "." << std::flush;
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
//  vscp_readLevel1RegisterBlock
//

int vscp_readLevel1RegisterBlock( CVscpClient& client,
                                    cguid& guidNode,
                                    cguid& guidInterface,
                                    uint16_t page, 
                                    uint8_t offset,
                                    uint8_t count,
                                    std::map<uint8_t,uint8_t>& values,
                                    uint32_t timeout)
{
  int rv = VSCP_ERROR_ERROR;
  uint8_t rcvcnt = 0;   // Number of registers read
  vscpEventEx ex;
  CVscpClient::connType conntype = client.getType();
  uint8_t nickname               = guidNode.getNickname();
  uint8_t ifoffset               = guidInterface.isNULL() ? 0 : 16;
  
  /*
    Frames may not come in order on all interfaces
  */
  std::set<int> frameset;
  for (int i=0; i<count/4; i++) {
    frameset.insert(i);    
  }
  if (count%4) {
    frameset.insert(count/4);
  }

  ex.head = VSCP_PRIORITY_NORMAL;
  ex.vscp_class = VSCP_CLASS1_PROTOCOL + (guidInterface.isNULL() ? 0 : 512);
  ex.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ;
  ex.sizeData   = ifoffset + 5;
  if (ifoffset) memcpy(ex.data, guidInterface.getGUID(), 16);
  ex.data[0 + ifoffset]    = nickname;
  ex.data[1 + ifoffset]    = (page >> 8) & 0x0ff;
  ex.data[2 + ifoffset]    = page & 0x0ff;
  ex.data[3 + ifoffset]    = offset;
  ex.data[4 + ifoffset]    = count;

  // Clear input queue
  if (VSCP_ERROR_SUCCESS != (rv = client.clear())) {
    return rv;
  }

  // Send event
  if (VSCP_ERROR_SUCCESS != (rv = client.send(ex))) {
    return rv;
  }

  uint32_t startTime = vscp_getMsTimeStamp();

  // Wait for reponse
  
  do {
    // Get response
    uint16_t cntRead;
    rv = client.getcount(&cntRead);
    if (cntRead && VSCP_ERROR_SUCCESS == rv) {

      if (VSCP_ERROR_SUCCESS != (rv = client.receive(ex))) {
        return rv;
      }

      //std::cout << "class=" << (int)ex.vscp_class << " type = " << (int)ex.vscp_type << std::endl << std::flush;

      if (VSCP_CLASS1_PROTOCOL == ex.vscp_class) {
        if (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == ex.vscp_type) {
          if ((ex.GUID[15] = nickname) && 
              (ex.sizeData >= 5) && 
              /* ex.data[0] is frame index */
              (ex.data[1] == (page >> 8) & 0x0ff) && 
              (ex.data[2] == page & 0x0ff)  
              /*(ex.data[3] == offset + rcvcnt)*/ ) {

            // Another frame received    
            frameset.erase(ex.data[0]); 
            //std::cout << "idx=" << (int)ex.data[0] << " left = " << (int)frameset.size() << std::endl << std::flush;
            
            // Get read data 
            for (int i = 0; i < ex.sizeData-4; i++) {
              //values[offset + rcvcnt] = ex.data[4 + i];
              values[ex.data[3]+i] = ex.data[4 + i];
              rcvcnt++;
            }

            // Check if we are ready
            if (rcvcnt == count && frameset.empty()) {
              return VSCP_ERROR_SUCCESS;
            }
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

  } while (true);

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  vscp_writeLevel1RegisterBlock
//

int vscp_writeLevel1RegisterBlock( CVscpClient& client,
                                    cguid& guidNode,
                                    cguid& guidInterface,
                                    uint16_t page, 
                                    std::map<uint8_t,uint8_t>& regvalues,
                                    uint32_t timeout)
{
  int rv = VSCP_ERROR_SUCCESS;
  CVscpClient::connType conntype = client.getType();
  uint8_t nickname               = guidNode.getNickname();
  uint8_t ifoffset               = guidInterface.isNULL() ? 0 : 16;
  int reg = -99;
  int count = 0;
  vscpEventEx ex;
  uint8_t offset_data = 0;
  std::map<uint8_t,uint8_t> startmap; 
  
  // The startregs map contains reg, count for each sequence start
  for (auto const& item : regvalues) {
    if (item.first != (reg + count)) {
      reg = item.first;
      startmap[item.first] = count = 1;
    }
    else {
      startmap[reg]++;
      count = startmap[reg];
    }
  }

  // Write out
  for (auto const& start : startmap) {

    //std::cout << "Writing reg: " << (int)start.first << " count: " << (int)start.second << std::endl;
    
    // Find out how many frames to send
    uint8_t nwrites = start.second/4;
    if (start.second%4) nwrites++;

    for (int i=0; i<nwrites; i++) {

      uint8_t bytes2write = start.second - (i*4);
      if (bytes2write > 4) bytes2write = 4;

      ex.head = VSCP_PRIORITY_NORMAL;
      ex.vscp_class = VSCP_CLASS1_PROTOCOL+ (guidInterface.isNULL() ? 0 : 512);;
      ex.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE;
      ex.sizeData   = ifoffset + 4 + bytes2write;
      if (ifoffset) memcpy(ex.data, guidInterface.getGUID(), 16);
      ex.data[0 + ifoffset]    = nickname;
      ex.data[1 + ifoffset]    = (page >> 8) & 0x0ff;
      ex.data[2 + ifoffset]    = page & 0x0ff;
      ex.data[3 + ifoffset]    = start.first + i*4;

      // Fill data 
      for (int j=0; j<bytes2write; j++) {
        ex.data[4 + ifoffset + j] = regvalues[start.first + i*4 + j];
      }

      // Clear input queue
      if (VSCP_ERROR_SUCCESS != (rv = client.clear())) {
        return rv;
      }

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
        if (count && VSCP_ERROR_SUCCESS == rv) {

          if (VSCP_ERROR_SUCCESS != (rv = client.receive(ex))) {
            return rv;
          }

          if (VSCP_CLASS1_PROTOCOL == ex.vscp_class) {
            if (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == ex.vscp_type) {
              if ((ex.GUID[15] = nickname) && 
                  (ex.sizeData >= 5) && 
                  (ex.data[0] == 0) &&
                  (ex.data[1] == (page >> 8) & 0x0ff) && 
                  (ex.data[2] == page & 0x0ff)) {
       
                for (int k=4; k<ex.sizeData; k++) {
                  if (ex.data[k] == regvalues[ex.data[3]+k-4]) {
                    regvalues.erase(ex.data[3]+k-4);
                  }
                  //std::cout << "   Erased reg: " << (int)ex.data[3]+k-4 << std::endl;
                }
                
                if (regvalues.empty()) {
                  return VSCP_ERROR_SUCCESS;
                }

                break; // Always do next set
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

    } // writes

  } // startmap

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

  // Clear input queue
  if (VSCP_ERROR_SUCCESS != (rv = client.clear())) {
    return rv;
  }

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
  CVscpClient::connType conntype = client.getType();
  
  memset(&ex, 0, sizeof(vscpEventEx));
  ex.vscp_class = VSCP_CLASS1_PROTOCOL + 512;
  ex.vscp_type  = VSCP_TYPE_PROTOCOL_READ_REGISTER;
  memcpy(ex.data, guid.getGUID(), 16); // Use GUID of interface
  memset(ex.GUID, 0, 16); // Use GUID of interface
  ex.sizeData   = 18;

  for (int idx = start_node; idx<end_node; idx++) {    
    
    ex.data[16] = idx;  // nodeid
    ex.data[17] = 0xd0; // register: First byte of GUID

    // Clear input queue
    if (VSCP_ERROR_SUCCESS != (rv = client.clear())) {
      return rv;
    }

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
// putLevel1Registers
//

void 
CRegisterPage::putLevel1Registers(std::map<uint8_t,uint8_t>& registers)
{
  for (auto const& reg : registers) {
    putReg(reg.first, reg.second);
  }
}

///////////////////////////////////////////////////////////////////////////////
// getLevel1Registers
//

void 
CRegisterPage::getLevel1Registers(std::map<uint8_t,uint8_t>& registers)
{
  for (auto const& reg : m_registers) {
    if (reg.first < 128) {
      registers[reg.first] = reg.second;
    }
  }
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

  //std::cout << "Put reg " << (int)reg << " new value = " << (int)value << " old value =  " << (int)m_registers[reg] << std::endl;

  // Mark as changed only if different
  if (m_registers[reg] != value) {
    m_change[reg] = true;       // Mark as changed
    m_list_undo_value[reg].push_back(m_registers[reg]); // Save old value
  }

  return m_registers[reg] = value; // Assign value
}

///////////////////////////////////////////////////////////////////////////////
// hasChanges
//

bool 
CRegisterPage::hasChanges(void)
{
  for (auto const& reg : m_change) {
    if (reg.second) {
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
//  Constructor
//

CStandardRegisters::CStandardRegisters(uint8_t level)
{
  m_level = level;
}

///////////////////////////////////////////////////////////////////////////////
//  Destructor
//

CStandardRegisters::~CStandardRegisters() 
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
//  getFirmwareVersionString.
//

int CStandardRegisters::init(CVscpClient& client,
                              cguid& guidNode,
                              cguid& guidInterface,
                              uint32_t timeout) 
{
  int rv;
  m_regs.clear();
  m_change.clear();
  rv = vscp_readLevel1RegisterBlock(client, guidNode, guidInterface, 0, 0x80, 128, m_regs, timeout);
  return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getFirmwareVersionString.
//

int CStandardRegisters::init(CRegisterPage& regPage)
{
  if (regPage.getPage() != 0) {
    return VSCP_ERROR_INIT_MISSING;
  }

  for (int i=0x80; i<0x100; i++) {
    if ((i < 0xa5) || (i > 0xcf)) {
      m_regs[i] = regPage.getReg(i);
    }
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  restoreStandardConfig.
//

int restoreStandardConfig(CVscpClient& client,
                            cguid& guidNode,
                            cguid& guidInterface,
                            uint32_t timeout)
{
  int rv;
  rv = vscp_writeLevel1Register(client, guidNode, guidInterface, 0, 0xA2, 0x55, timeout);
  if ( VSCP_ERROR_SUCCESS != rv ) {
    return rv;
  }
  rv = vscp_writeLevel1Register(client, guidNode, guidInterface, 0, 0xA2, 0xAA, timeout);
  return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  getFirmwareVersionString.
//

std::string
CStandardRegisters::getFirmwareVersionString(void)
{
  std::string str;

  str = str = vscp_str_format("%d.%d.%d", m_regs[0x94 - 0x80], m_regs[0x95 - 0x80], m_regs[0x96 - 0x80]);
  return str;
}


///////////////////////////////////////////////////////////////////////////////
//  getGUID
//

void CStandardRegisters::getGUID(cguid& guid) 
{ 
  for (int i=0; i<16; i++) {
    //std::cout << std::hex << (int)m_regs[0xd0 + i] << std::endl;
    guid.setAt(i, m_regs[0xd0 + i]);
  }
}

///////////////////////////////////////////////////////////////////////////////
//  getMDF
//

std::string
CStandardRegisters::getMDF(void)
{
  char url[33];
  std::string remoteFile;

  for (int i = 0; i < 32; i++) {
    url[i] = m_regs[0xe0 + i];
  }

  remoteFile = std::string(url);
  return(remoteFile);
}

///////////////////////////////////////////////////////////////////////////////
// hasChanges
//

bool 
CStandardRegisters::hasChanges(void)
{
  for (auto const& reg : m_change) {
    if (reg.second) {
      return true;
    }
  }
  return false;
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
//  init
//

int CUserRegisters::init(CVscpClient& client,
                          cguid& guidNode,
                          cguid& guidInterface,
                          std::set<uint16_t>& pages, 
                          uint32_t timeout)
{
  int rv = VSCP_ERROR_SUCCESS;
  m_pages.clear();

  for (auto const& page : pages) {
    
    std::map<uint8_t, uint8_t> registers;
    m_registerPageMap[page] = new CRegisterPage(m_level, page);
    rv = vscp_readLevel1RegisterBlock(client, 
                                        guidNode, 
                                        guidInterface,
                                        page,
                                        0,
                                        127,
                                        registers,
                                        timeout);
    if ( VSCP_ERROR_SUCCESS != rv ) {
      return rv;
    }

    // Transfer data to register page
    m_registerPageMap[page]->putLevel1Registers(registers);
    m_registerPageMap[page]->clearChanges();
    m_pages.insert(page);
    //std::cout << "Page " << std::hex << page << " size " << registers.size() << std::endl;
  }
  return rv;
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

///////////////////////////////////////////////////////////////////////////////
// getReg
//

int
CUserRegisters::getReg(uint32_t offset, uint16_t page)
{
  CRegisterPage *ppage = m_registerPageMap[page];
  if (nullptr == ppage) {
    return -1;
  }

  auto it = ppage->getRegisterMap()->find(offset);
  if (it == ppage->getRegisterMap()->end()) {
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

  // A new page will automatically be created
  // if thew page does not exist
  CRegisterPage *pPage = m_registerPageMap[page];
  pPage->putReg(reg, value); // Assign value

  m_pages.insert(page); // Add page to list of pages

  // // Look up page or create a new one if not found
  // std::map<uint16_t, CRegisterPage *>::iterator it;
  // if (m_registerPageMap.end() != (it = m_registerPageMap.find(page))) {

  //   // Page already exist
  //   CRegisterPage *pPage = it->second;
  //   if (nullptr == pPage) {
  //     return false; // Invalid page
  //   }

  //   pPage->putReg(reg, value); // Assign value
  // }
  // else {

  //   // Page does not exist

  //   // Create page
  //   CRegisterPage *pPage = new CRegisterPage(m_level);
  //   if (nullptr == pPage) {
  //     return false; // Unable to create page
  //   }

  //   m_registerPageMap[page] = pPage;

  //   pPage->putReg(reg, value); // Assign value
  // }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
//  setChangedState
//

bool 
CUserRegisters::setChangedState(uint32_t offset, uint16_t page, bool state)
{
  CRegisterPage *pPage = m_registerPageMap[page];
  if (nullptr == pPage) {
    return false;
  }

  pPage->setChangedState(offset, state);  
  return true;
}

///////////////////////////////////////////////////////////////////////////////
//  isChanged
//

bool 
CUserRegisters::isChanged(uint32_t offset, uint16_t page)
{
  CRegisterPage *pPage = m_registerPageMap[page];
  if (nullptr == pPage) {
    return false;
  }

  return pPage->isChanged(offset);  
}

///////////////////////////////////////////////////////////////////////////////
//  hasWrittenChange
//

bool 
CUserRegisters::hasWrittenChange(uint32_t offset, uint16_t page)
{
  CRegisterPage *pPage = m_registerPageMap[page];
  if (nullptr == pPage) {
    return false;
  }

  // If there are undo values there has been changes
  return pPage->hasWrittenChange(offset);
}

///////////////////////////////////////////////////////////////////////////////
//  clearChanges
//

void
CUserRegisters::clearChanges(void)
{
  for (auto const& page : m_registerPageMap) {
    page.second->clearChanges();
  }  
}

///////////////////////////////////////////////////////////////////////////////
//  clearHistory
//

void
CUserRegisters::clearHistory(void)
{
  for (auto const& page : m_registerPageMap) {
    page.second->clearHistory();
  }  
}

///////////////////////////////////////////////////////////////////////////////
//  remoteVarFromRegToString
//

int
CUserRegisters::remoteVarFromRegToString(CMDF_RemoteVariable& remoteVar, 
                                          std::string& strValue, 
                                          uint8_t format)
{

  int rv = VSCP_ERROR_SUCCESS;
  CRegisterPage *ppage;

  // Get register page (will always return a page even if it does not exists)
  ppage = getRegisterPage(remoteVar.getPage());
  if (nullptr == ppage) {
    return VSCP_ERROR_ERROR;
  }

  // vscp_remote_variable_type
  switch (remoteVar.getType()) {

    case remote_variable_type_string:
      {
        uint8_t *pstr;
        pstr = new uint8_t[remoteVar.getTypeByteCount() + 1 ];
        if ( nullptr == pstr ) return false;
        memset(pstr, 0, sizeof(pstr));
        for (int i = remoteVar.getOffset(); 
              i < remoteVar.getOffset() + remoteVar.getTypeByteCount(); 
              i++) {
          pstr[i] = ppage->getReg(i);
        }
        strValue = (const char *)pstr;
        delete [] pstr;
      }
      break;

    case remote_variable_type_boolean:
      {
        strValue = ppage->getReg(remoteVar.getOffset() & (1<<remoteVar.getBitPos())) ? "true" : "false";
      }
      break;

    case remote_variable_type_int8_t:
      {
        if ( FORMAT_REMOTEVAR_DECIMAL == format ) {
          strValue = vscp_str_format("%had", ppage->getReg(remoteVar.getOffset()));
        }
        else {
          strValue = vscp_str_format( "0x%02hx", ppage->getReg(remoteVar.getOffset()));
        }
      }
      break;

    case remote_variable_type_uint8_t:
      {
        if ( FORMAT_REMOTEVAR_DECIMAL == format ) {
          strValue = vscp_str_format( "%hu", ppage->getReg(remoteVar.getOffset()));
        }
        else {
          strValue = vscp_str_format( "0x%02hx", ppage->getReg(remoteVar.getOffset()));
        }
      }
      break;

    case remote_variable_type_int16_t:
      {
        uint8_t buf[2];
        buf[0] = ppage->getReg(remoteVar.getOffset());
        buf[1] = ppage->getReg(remoteVar.getOffset()+1);
        int16_t val = (buf[0] << 8 ) + buf[1];

        if ( FORMAT_REMOTEVAR_DECIMAL == format ) {
          strValue = vscp_str_format( "%hd", val );
        }
        else {
          strValue = vscp_str_format( "0x%04hx", val );          
        }
      }
      break;

    case remote_variable_type_uint16_t:
      {
        uint8_t buf[2];
        buf[0] = ppage->getReg(remoteVar.getOffset());
        buf[1] = ppage->getReg(remoteVar.getOffset()+1);
        uint16_t val = (buf[0] << 8 ) + buf[1];

        if ( FORMAT_REMOTEVAR_DECIMAL == format ) {
          strValue = vscp_str_format( "hd", val );
        }
        else {
          strValue = vscp_str_format( "0x%04hx", val );
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
        if ( FORMAT_REMOTEVAR_DECIMAL == format ) {
          strValue = vscp_str_format( "%ld", val);
        }
        else {
          strValue = vscp_str_format( "0x%04lx", val);
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
        if ( FORMAT_REMOTEVAR_DECIMAL == format ) {
          strValue = vscp_str_format( "%lu", val);
        }
        else {
          strValue = vscp_str_format( "0x%04lx", val);
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

        if ( FORMAT_REMOTEVAR_DECIMAL == format ) {
            strValue = vscp_str_format( "%lld", val );
        }
        else {
            strValue = vscp_str_format( "0x%llx", val );
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
        if ( FORMAT_REMOTEVAR_DECIMAL == format ) {
            strValue = vscp_str_format("%ullu", val);
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

int
CUserRegisters::remoteVarFromStringToReg(CMDF_RemoteVariable& remoteVar, std::string &strValue)
{
  bool rv = VSCP_ERROR_SUCCESS;
  CRegisterPage *ppage = getRegisterPage( remoteVar.getPage() );
  if (nullptr == ppage) return VSCP_ERROR_ERROR;

  switch (remoteVar.getType()) {

    case remote_variable_type_string:
      {
        /*!
          It is possible to write registers here that is out of bonds
          for a level I device but that is not a problem as they never
          will be written to the device.
        */
        for (int i=0; i<strValue.length(); i++) {
          ppage->putReg(remoteVar.getOffset() + i, strValue[i]);
        }
      }
      break;

    case remote_variable_type_boolean:
      {
        uint8_t val = ppage->getReg(remoteVar.getOffset());
        if ( 0 == strValue.compare("true") ) {
          val |= (1 << remoteVar.getBitPos());
        }
        else if ( 0 == strValue.compare("false") ) {
          val &=  ~(1 << remoteVar.getBitPos());
        }
        else {
          val = (uint8_t)vscp_readStringValue(strValue);
        }    
        
        ppage->putReg(remoteVar.getOffset(), val);  
      }
      break;

    case remote_variable_type_int8_t:
      {
        int8_t val = vscp_readStringValue(strValue);
        ppage->putReg(remoteVar.getOffset(), val);
      }
      break;

    case remote_variable_type_uint8_t:
      {
        uint8_t val = vscp_readStringValue(strValue);
        ppage->putReg(remoteVar.getOffset(), val);
      }
      break;

    case remote_variable_type_int16_t:
      {
        int16_t val = vscp_readStringValue(strValue);
        ppage->putReg(remoteVar.getOffset(), (val >> 8)  & 0xff);
        ppage->putReg(remoteVar.getOffset() + 1, val & 0xff);
      }
      break;

    case remote_variable_type_uint16_t:
      {
        uint16_t val = vscp_readStringValue(strValue);
        ppage->putReg(remoteVar.getOffset(), (val >> 8)  & 0xff);
        ppage->putReg(remoteVar.getOffset() + 1, val & 0xff);
      }
      break;

    case remote_variable_type_int32_t:
      {
        int32_t val = vscp_readStringValue(strValue);
        ppage->putReg(remoteVar.getOffset(), (val >> 24) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 1, (val >> 16) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 2, (val >> 8)  & 0xff);
        ppage->putReg(remoteVar.getOffset() + 3, val & 0xff);
      }
      break;

    case remote_variable_type_uint32_t:
      {
        uint32_t val = vscp_readStringValue(strValue);
        ppage->putReg(remoteVar.getOffset(), (val >> 24) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 1, (val >> 16) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 2, (val >> 8)  & 0xff);
        ppage->putReg(remoteVar.getOffset() + 3, val & 0xff);
      }
      break;

    case remote_variable_type_int64_t:
      {
        int64_t val = vscp_readStringValue(strValue);
        ppage->putReg(remoteVar.getOffset(), (val >> 56) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 1, (val >> 48) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 2, (val >> 40) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 3, (val >> 32) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 4, (val >> 24) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 5, (val >> 16) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 6, (val >> 8)  & 0xff);
        ppage->putReg(remoteVar.getOffset() + 7, val & 0xff);  
      }
      break;

    case remote_variable_type_uint64_t:
      {
        uint64_t val = vscp_readStringValue(strValue);
        ppage->putReg(remoteVar.getOffset(), (val >> 56) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 1, (val >> 48) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 2, (val >> 40) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 3, (val >> 32) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 4, (val >> 24) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 5, (val >> 16) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 6, (val >> 8)  & 0xff);
        ppage->putReg(remoteVar.getOffset() + 7, val & 0xff);  
      }
      break;

    case remote_variable_type_float:
      {
        float val = vscp_readStringValue(strValue);
        uint8_t *p = (uint8_t *)&val;
        val = VSCP_INT32_SWAP_ON_LE(*((int64_t *)p));
        ppage->putReg(remoteVar.getOffset(), (*p >> 24) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 1, (*p >> 16) & 0xff);
        ppage->putReg(remoteVar.getOffset() + 2, (*p >> 8)  & 0xff);
        ppage->putReg(remoteVar.getOffset() + 3, *p & 0xff);
      }
      break;

    case remote_variable_type_double:
      {
        double val = vscp_readStringValue(strValue);
        uint8_t *p = (uint8_t *)&val;
#ifndef __BIG_ENDIAN__
        val = Swap8Bytes(*((int64_t *)p));
#endif        
        ppage->putReg(remoteVar.getOffset(), *p);
        ppage->putReg(remoteVar.getOffset() + 1, *(p+1));
        ppage->putReg(remoteVar.getOffset() + 2, *(p+2));
        ppage->putReg(remoteVar.getOffset() + 3, *(p+3));
        ppage->putReg(remoteVar.getOffset() + 4, *(p+4));
        ppage->putReg(remoteVar.getOffset() + 5, *(p+5));
        ppage->putReg(remoteVar.getOffset() + 6, *(p+6));
        ppage->putReg(remoteVar.getOffset() + 7, *(p+7));
      }
      break;

    // YY:MM:DD
    case remote_variable_type_date:
      {
        std::deque<std::string> vec;
        vscp_split(vec, strValue, ":");
        if (vec.size() == 3) {
          uint8_t val = vscp_readStringValue(vec[0]);
          ppage->putReg(remoteVar.getOffset(), val);
          val = vscp_readStringValue(vec[1]);
          ppage->putReg(remoteVar.getOffset() + 1, val);
          val = vscp_readStringValue(vec[2]);
          ppage->putReg(remoteVar.getOffset() + 2, val);
        }
      }
      break;

    // HH:MM:SS
    case remote_variable_type_time:
      {
        std::deque<std::string> vec;
        vscp_split(vec, strValue, ":");
        if (vec.size() == 3) {
          uint8_t val = vscp_readStringValue(vec[0]);
          ppage->putReg(remoteVar.getOffset(), val);
          val = vscp_readStringValue(vec[1]);
          ppage->putReg(remoteVar.getOffset() + 1, val);
          val = vscp_readStringValue(vec[2]);
          ppage->putReg(remoteVar.getOffset() + 2, val);
        }
      }
      break;

    case remote_variable_type_unknown:
    default:
        break;
    }
  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getGetDmRegisterContent
//

int 
CUserRegisters::vscp_getGetDmRegisterContent(CMDF_DecisionMatrix& dm, uint16_t row, uint8_t pos)
{
  CRegisterPage *ppage = getRegisterPage(dm.getStartPage());
  if (nullptr == ppage) {
    return VSCP_ERROR_ERROR;
  }

  // Validate row
  if (row >= dm.getRowCount()) {
    return VSCP_ERROR_ERROR;
  }

  // Validate position
  if (pos >= dm.getRowSize()) {
    return VSCP_ERROR_ERROR;
  }

  // Get register
  return(ppage->getReg(dm.getRowSize() * row)); 
}

///////////////////////////////////////////////////////////////////////////////
// vscp_readDmRow
//

int 
CUserRegisters::vscp_readDmRow(CMDF_DecisionMatrix& dm, uint16_t row, uint8_t *buf)
{
  CRegisterPage *ppage = getRegisterPage(dm.getStartPage());
  if (nullptr == ppage) {
    return VSCP_ERROR_ERROR;
  }

  // Validate row
  if (row >= dm.getRowCount()) {
    return VSCP_ERROR_ERROR;
  }

  for (uint8_t i = 0; i < dm.getRowSize(); i++) {
    buf[i] = ppage->getReg(dm.getRowSize() * row + i);
  }

  return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_writeDmRow
//

int 
CUserRegisters::vscp_writeDmRow(CMDF_DecisionMatrix& dm, uint16_t row, uint8_t *buf)
{
  CRegisterPage *ppage = getRegisterPage(dm.getStartPage());
  if (nullptr == ppage) {
    return VSCP_ERROR_ERROR;
  }

  // Validate row
  if (row >= dm.getRowCount()) {
    return VSCP_ERROR_ERROR;
  }

  for (uint8_t i = 0; i < dm.getRowSize(); i++) {
    ppage->putReg(dm.getRowSize() * row + i, buf[i]);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// hasChanges
//

bool 
CUserRegisters::hasChanges(void)
{
  for (auto it = m_registerPageMap.begin(); it != m_registerPageMap.end(); ++it) {
    if (it->second->hasChanges()) {
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////


CVscpNode::CVscpNode()
{

}


CVscpNode::~CVscpNode()
{
  
}