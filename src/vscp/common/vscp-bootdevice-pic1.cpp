// bootdevice_pic1.cpp: PIC algorithm 1 Bootloader code.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:  (C) 2007-2025
// Ake Hedman, the VSCP project, <info@vscp.org>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "devicebootloaderwizard.h"
#endif

#ifdef WIN32
#include <winsock2.h>
#endif

#include <register.h>

#include "vscp-bootdevice-pic1.h"

#include "spdlog/fmt/bin_to_hex.h"
// #include <spdlog/async.h>
// #include <spdlog/sinks/rotating_file_sink.h>
// #include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include <iomanip> 
#include <sstream> 

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

/*
  Level I
*/
CBootDevice_PIC1::CBootDevice_PIC1(CVscpClient *pclient,
                                   uint8_t nodeid,
                                   std::function<void(int, const char *)> statusCallback,
                                   uint32_t timeout)
  : CBootDevice(pclient, nodeid, statusCallback, timeout)
{
  init();
}

/*
  Level I over interface
*/
CBootDevice_PIC1::CBootDevice_PIC1(CVscpClient *pclient,
                                   uint8_t nodeid,
                                   cguid &guidif,
                                   std::function<void(int, const char *)> statusCallback,
                                   uint32_t timeout)
  : CBootDevice(pclient, nodeid, guidif, statusCallback, timeout)
{
  init();
}

/*
  Full Level II
*/
CBootDevice_PIC1::CBootDevice_PIC1(CVscpClient *pclient,
                                   cguid &guid,
                                   std::function<void(int, const char *)> statusCallback,
                                   uint32_t timeout)
  : CBootDevice(pclient, guid, statusCallback, timeout)
{
  init();
}

CBootDevice_PIC1::~CBootDevice_PIC1(void)
{
  // Clean up buffer allocations is done in base class
}

///////////////////////////////////////////////////////////////////////////////
// init
//

void
CBootDevice_PIC1::init(void)
{
  m_bHandshake = true;  // Handshake as default
  m_startAddr  = 0x800; // Default start position
  // m_pAddr      = 0;
  // m_memtype    = MEM_TYPE_PROGRAM;

  // No memory blocks
  // m_bCodeMemory   = false;
  // m_bUserIdMemory = false;
  // m_bCfgMemory    = false;
  // m_bEepromMemory = false;

  // Reset block storage
  // memset(m_pbufCode, 0xff, BUFFER_SIZE_CODE);
  // memset(m_pbufUserID, 0xff, BUFFER_SIZE_USERID);
  // memset(m_pbufCfg, 0xff, BUFFER_SIZE_CONFIG);
  // memset(m_pbufEprom, 0xff, BUFFER_SIZE_EEPROM);

  crcInit();
}

///////////////////////////////////////////////////////////////////////////////
// deviceInfo
//

std::string
CBootDevice_PIC1::deviceInfo(void)
{
  std::string str;
  std::string strInfo;
  std::ostringstream oss;

  uint32_t min;
  uint32_t max;

  // * * * Device * * *
  oss << "<b><u>Device</u></b><br>";
  oss << "<b>nodeid :</b><font color=\"#005CB9\">";
  oss << m_nodeid;
  oss << "</font><br>";
  oss << "<b>GUID :</b><font color=\"#005CB9\">";
  oss << m_guid.toString();
  oss << "</font><br>";
  oss << "<b>Interface :</b><font color=\"#005CB9\">";
  oss << m_guidif.toString();
  oss << "</font><br>";

  // * * * Flash Memory * * *
  getMinMaxForRange(MEM_CODE_START, MEM_CODE_END, &min, &max);
  oss << "<b><u>Flash Memory</u></b><br>";
  oss << "<b>Start :</b><font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << min;
  oss << "</font><b> End :</b><font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << max;
  if (max > min) {
    oss << "<font color=\"#348017\">Will be programmed</font><br>";
  }
  else {
    oss << "<font color=\"#F6358A\">Will not be programmed</font><br>";
  }
  oss << "<br><br>";

  // * * * UserID Memory * * *
  getMinMaxForRange(MEM_USERID_START, MEM_CODE_END, &min, &max);
  oss << "<b><u>UserID Memory</u></b><br>";
  oss << "<b>Start :</b>";

  oss << "<font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << min << "</font>";

  oss << "<b> End :</b>";

  oss << "<font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << max << "</font>";

  if (max > min) {
    oss << "<font color=\"#348017\">Will be programmed</font><br>";
  }
  else {
    oss << "<font color=\"#F6358A\">Will not be programmed</font><br>";
  }
  oss << "<br><br>";

  // * * * Config Memory * * *
  getMinMaxForRange(MEM_CONFIG_START, MEM_CONFIG_END, &min, &max);
  oss << "<b><u>Config Memory</u></b><br>";
  oss << "<b>Start :</b>";

  oss << "<font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << min << "</font>";

  oss << "<b> End :</b>";

  oss << "<font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << max << "</font>";

  if (max > min) {
    oss << "<font color=\"#348017\">Will be programmed</font><br>";
  }
  else {
    oss << "<font color=\"#F6358A\">Will not be programmed</font><br>";
  }
  oss << "<br><br>";

  // * * * EEPROM * * *
  getMinMaxForRange(MEM_EEPROM_START, MEM_EEPROM_END, &min, &max);
  oss << "<b><u>EEPROM Memory</u></b><br>";
  oss << "<B>Start :</b>";

  oss << "<font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << min << "</font>";
  oss << "<b> End :</b>";

  oss << "<font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << PRIdMAX << "</font>";

  if (max > min) {
    oss << "<font color=\"#348017\">Will be programmed</font><br>";
  }
  else {
    oss << "<font color=\"#F6358A\">Will not be programmed</font><br>";
  }
  oss << "<br><br>";

  return oss.str();
}

///////////////////////////////////////////////////////////////////////////////
// deviceInit
//

int
CBootDevice_PIC1::deviceInit(cguid& ourguid, uint16_t /*devicecode*/, bool bAbortOnFirmwareCodeFail)
{
  int rv;

  // Save our local GUID
  m_ourguid = ourguid;

  /*
    First do a test to see if the device is already in boot mode
    if it is 0x14nn/0x15nn should be returned (nn == nodeid).
  */

  spdlog::info("Test to see if remote device is already in boot mode.");
  if (nullptr != m_statusCallback) {
    m_statusCallback(-1, "Test to see if remote device is already in boot mode.");
  }

  // Clear checksum
  m_checksum = 0;

  canalMsg msg, rcvmsg;
  time_t tstart, tnow;
  memset(msg.data, 0x00, 8);

  // Send probe message
  msg.id    = ID_PUT_BASE_INFO;
  msg.flags = CANAL_IDFLAG_EXTENDED;
  memset(msg.data, 0, 8);
  msg.sizeData = 8;
  if (CANAL_ERROR_SUCCESS != (rv = m_pclient->send(msg))) {
    spdlog::error("Failed to send init message (ID_PUT_BASE_INFO) to remote device rv = {0}.", rv);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1,
                       vscp_str_format("Failed to send init message (ID_PUT_BASE_INFO) "
                                       "to remote device rv = %d.",
                                       rv)
                         .c_str());
    }
    return rv;
  }

  time(&tstart); // Get start time

  bool bRun = true;
  while (bRun) {

    time(&tnow);
    if ((unsigned long) (tnow - tstart) > 2) {
      bRun = false; // End loop
    }

    uint16_t cnt;
    if ((VSCP_ERROR_SUCCESS == m_pclient->getcount(&cnt)) && cnt) { // Check if data is available

      // There is data get message
      if (VSCP_ERROR_SUCCESS != (rv = m_pclient->receive(rcvmsg))) {
        spdlog::error("Failed to receive response form init command (ID_PUT_BASE_INFO). rv = {0}.", rv);
        if (nullptr != m_statusCallback) {
          m_statusCallback(-1,
                           vscp_str_format("Failed to receive response form init command (ID_PUT_BASE_INFO). "
                                           "rv = %d.",
                                           rv)
                             .c_str());
        }
        return rv;
      }

      spdlog::debug("Initial probe: Received CAN message: id={0:X} size={1}", rcvmsg.id, rcvmsg.sizeData);

      // Is this a read/write reply from the node?
      if ((rcvmsg.id & 0xffffff) == (uint32_t) (ID_RESPONSE_PUT_BASE_INFO + m_nodeid)) {
        // yes already in bootmode - return
        spdlog::info("Device is set in boot mode ID_RESPONSE_PUT_BASE_INFO");
        if (nullptr != m_statusCallback) {
          m_statusCallback(-1, "Device is set in boot mode ID_RESPONSE_PUT_BASE_INFO");
        }
        return VSCP_ERROR_SUCCESS;
      }

      // Is this a read/write reply from the node?
      if ((rcvmsg.id & 0xffffff) == (uint32_t) (ID_RESPONSE_PUT_DATA + m_nodeid)) {
        // yes already in bootmode - return
        spdlog::info("Device is set in boot mode ID_RESPONSE_PUT_DATA");
        if (nullptr != m_statusCallback) {
          m_statusCallback(-1, "Device is set in boot mode ID_RESPONSE_PUT_DATA");
        }
        return VSCP_ERROR_SUCCESS;
      }
    }
  } // while

  // Read standard registers
  if (VSCP_ERROR_SUCCESS != m_stdRegs.init(*m_pclient, m_guid, m_guidif, nullptr, REGISTER_DEFAULT_TIMEOUT)) {
    spdlog::error("deviceInit: Failed to read standard registers");
    return VSCP_ERROR_COMMUNICATION;
  }

  /*
    Check that the remote device expect the firmware load type
    we offer
  */
  if (VSCP_BOOTLOADER_PIC1 != m_stdRegs.getBootloaderAlgorithm()) {
    spdlog::error("deviceInit: Bootloader algorithm is not Microchip PIC1");
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "Bootloader algorithm is not Microchip PIC1");
    }
    return VSCP_ERROR_NOT_SUPPORTED;
  }

  /*
    The device code tell the type of hardware of the remote device
    Must be the same as the firmware we try to load is intended for
  */
  if (m_firmwaredeviceCode != m_stdRegs.getFirmwareDeviceCode()) {
    spdlog::warn("Firmware device code is not equal the one on the device local: {0} device: {1}",
                 m_firmwaredeviceCode,
                 m_stdRegs.getFirmwareDeviceCode());
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "Firmware device code is not equal the one on the device local: {0} device: {1}");
    }
    if (bAbortOnFirmwareCodeFail) {
      return VSCP_ERROR_PARAMETER;
    }
  }

  // Set device in boot mode
  cguid guid;
  m_stdRegs.getGUID(guid);
  msg.data[0] = (uint8_t)m_nodeid;             // Nickname to read register from
  msg.data[1] = VSCP_BOOTLOADER_PIC1; // VSCP PIC1 bootloader algorithm
  msg.data[2] = guid.getAt(0);
  msg.data[3] = guid.getAt(3);
  msg.data[4] = guid.getAt(5);
  msg.data[5] = guid.getAt(7);
  msg.data[6] = m_stdRegs.getRegisterPage() >> 8;
  msg.data[7] = m_stdRegs.getRegisterPage() & 0xff;

  // Send message
  msg.id       = (VSCP_PIC1_ENTER_BOOTLODER_MODE << 8);
  msg.flags    = CANAL_IDFLAG_EXTENDED;
  msg.sizeData = 8;
  if (CANAL_ERROR_SUCCESS != (rv = m_pclient->send(msg))) {
    spdlog::error("deviceInit: Failed to send enter bootloader event {0}", rv);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "deviceInit: Failed to send enter bootloader event");
    }
    return rv;
  }

  bRun = true;

  // Get start time
  time(&tstart);

  while (bRun) {

    time(&tnow);
    if ((unsigned long) (tnow - tstart) > BOOT_COMMAND_DEFAULT_RESPONSE_TIMEOUT) {
      bRun = false;
    }

    uint16_t cnt;
    if ((VSCP_ERROR_SUCCESS == m_pclient->getcount(&cnt)) && cnt) {

      m_pclient->receive(rcvmsg);

      spdlog::debug("Init boot mode: Received CAN message: id={0:X} size={1}", rcvmsg.id, rcvmsg.sizeData);

      // Is this a read/write reply from the node?
      if ((rcvmsg.id & 0xffffff) == (uint32_t) (ID_RESPONSE_PUT_BASE_INFO + m_nodeid)) {
        // OK in bootmode - return
        spdlog::debug("deviceInit: Confirmed, device is in boot mood.");
        if (nullptr != m_statusCallback) {
          m_statusCallback(-1, "deviceInit: Confirmed, device is in boot mood.");
        }
        return VSCP_ERROR_SUCCESS;
      }

      // Is this a read/write reply from the node?
      if ((rcvmsg.id & 0xffffff) == (uint32_t) (ID_RESPONSE_PUT_DATA + m_nodeid)) {
        // OK in bootmode - return
        spdlog::debug("deviceInit: Confirmed, device is in boot mood.");
        if (nullptr != m_statusCallback) {
          m_statusCallback(-1, "deviceInit: Confirmed, device is in boot mood.");
        }
        return VSCP_ERROR_SUCCESS;
      }
    }
  } // while

  return VSCP_ERROR_TIMEOUT;

  /*
    vscpEventEx event;
    time_t tstart, tnow;

    memset(event.data, 0, sizeof(event.data));
    event.head       = 0;
    event.vscp_class = 512;                     // CLASS2.PROTOCOL1
    event.vscp_type  = (ID_PUT_BASE_INFO >> 8); // Special bootloader command
    memset(event.GUID, 0, 16);                  // We use interface GUID
    event.sizeData = 16;                        // Interface GUID
    memcpy(event.data, m_guidif.m_id, 16);      // Address node i/f

    if (VSCP_ERROR_SUCCESS == m_pclient->send(event)) {

      bRun = true;

      // Get start time
      time(&tstart);

      while (bRun) {

        time(&tnow);
        if ((unsigned long) (tnow - tstart) > PIC_BOOTLOADER_RESPONSE_TIMEOUT) {
          bRun = false;
        }

        uint16_t cnt;
        if ((VSCP_ERROR_SUCCESS == m_pclient->getcount(&cnt)) && cnt) { // Message available

          m_pclient->receive(event);

          if ((event.vscp_type == (ID_RESPONSE_PUT_BASE_INFO >> 8))) {
            // yes already in bootmode - return
            return true;
          }

          if ((event.vscp_type == (ID_RESPONSE_PUT_DATA >> 8))) {
            // yes already in bootmode - return
            return true;
          }
        }

      } // while
    }

    // Read page register MSB
    if (CANAL_ERROR_SUCCESS !=
        vscp_readLevel1Register(*m_pclient, m_guid, m_guidif, 0, VSCP_STD_REGISTER_PAGE_SELECT_MSB, pageSelectMsb)) {
      return false;
    }

    // Read page register LSB
    if (CANAL_ERROR_SUCCESS !=
        vscp_readLevel1Register(*m_pclient, m_guid, m_guidif, 0, VSCP_STD_REGISTER_PAGE_SELECT_LSB, pageSelectLsb)) {
      return false;
    }

    // Read GUID0
    if (CANAL_ERROR_SUCCESS != vscp_readLevel1Register(*m_pclient, m_guid, m_guidif, 0, VSCP_STD_REGISTER_GUID, guid0))
    { return false;
    }

    // Read GUID3
    if (CANAL_ERROR_SUCCESS !=
        vscp_readLevel1Register(*m_pclient, m_guid, m_guidif, 0, VSCP_STD_REGISTER_GUID + 3, guid3)) {
      return false;
    }

    // Read GUID5
    if (CANAL_ERROR_SUCCESS !=
        vscp_readLevel1Register(*m_pclient, m_guid, m_guidif, 0, VSCP_STD_REGISTER_GUID + 5, guid5)) {
      return false;
    }

    // Read GUID7
    if (CANAL_ERROR_SUCCESS !=
        vscp_readLevel1Register(*m_pclient, m_guid, m_guidif, 0, VSCP_STD_REGISTER_GUID + 7, guid7)) {
      return false;
    }

    // Set device in boot mode

    // Send message
    event.head       = 0;
    event.vscp_class = 512;                            // CLASS2.PROTOCOL1
    event.vscp_type  = VSCP_PIC1_ENTER_BOOTLODER_MODE; // We want to enter bootloader mode
    memset(event.GUID, 0, 16);                         // We use interface GUID
    event.sizeData = 16 + 8;                           // Interface GUID
    memset(event.data, 0, sizeof(event.data));
    memcpy(event.data, m_guidif.m_id, 16); // Address node i/f
    event.data[16] = m_guid.getLSB();      // Nickname for device
    event.data[17] = VSCP_BOOTLOADER_PIC1; // VSCP PIC1 bootloader algorithm
    event.data[18] = guid0;
    event.data[19] = guid3;
    event.data[20] = guid5;
    event.data[21] = guid7;
    event.data[22] = pageSelectMsb;
    event.data[23] = pageSelectLsb;

    if (VSCP_ERROR_SUCCESS == m_pclient->send(event)) {

      bRun = true;

      // Get start time
      time(&tstart);

      while (bRun) {

        time(&tnow);
        if ((unsigned long) (tnow - tstart) > PIC_BOOTLOADER_RESPONSE_TIMEOUT) {
          bRun = false;
        }

        // vscpEventEx rcvmsg;

        if (VSCP_ERROR_SUCCESS == m_pclient->receive(event)) {

          // Is this a read/write reply from the node?
          if ((ID_RESPONSE_PUT_BASE_INFO >> 8) == event.vscp_type) {
            // OK in bootmode - return
            return true;
          }

          // Is this a read/write reply from the node?
          if ((ID_RESPONSE_PUT_DATA >> 8) == event.vscp_type) {
            // OK in bootmode - return
            return true;
          }
        }
      }
    }
    */
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// writeDeviceControlRegs
//

int
CBootDevice_PIC1::writeDeviceControlRegs(uint32_t addr, uint8_t flags, uint8_t cmd, uint8_t cmdData0, uint8_t cmdData1)
{
  int rv;
  //vscpEventEx event;
  canalMsg msg;

  // Save the internal addresss
  m_pAddr = addr;

  msg.id       = ID_PUT_BASE_INFO;
  msg.flags    = CANAL_IDFLAG_EXTENDED;
  msg.sizeData = 8;
  msg.id += m_nodeid; // Add node id.
  msg.data[0] = (unsigned char) (addr & 0xff);
  msg.data[1] = (unsigned char) ((addr >> 8) & 0xff);
  msg.data[2] = (unsigned char) ((addr >> 16) & 0xff);
  msg.data[3] = (unsigned char) ((addr >> 24 & 0xff)); // Ignored by remote device
  msg.data[4] = flags;
  msg.data[5] = cmd;
  msg.data[6] = cmdData0;
  msg.data[7] = cmdData1;

  if (flags & MODE_ACK) {
    m_bHandshake = true;
  }
  else {
    m_bHandshake = false;
  }

  if (VSCP_ERROR_SUCCESS == (rv = m_pclient->send(msg))) {

    // Message queued - ( wait for response from client(s) ).
    if (m_bHandshake) {
      if (VSCP_ERROR_SUCCESS != (rv = checkResponseLevel1(ID_RESPONSE_PUT_BASE_INFO))) {
        spdlog::error("writeDeviceControlRegs: Timeout while waiting for response from device.");
        if (nullptr != m_statusCallback) {
          m_statusCallback(-1, "writeDeviceControlRegs: Timeout while waiting for response from device.");
        }
        return rv;
      }
    }
  }
  else {
    spdlog::error("writeDeviceControlRegs: Failed to send boot command to remote device.");
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "writeDeviceControlRegs: Failed to send boot command to remote device.");
    }
    return rv;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// checkResponseLevel1
//

int
CBootDevice_PIC1::checkResponseLevel1(uint32_t response_id)
{
  int rv;
  canalMsg msg;
  time_t tstart, tnow;

  // Get system time
  time(&tstart);

  bool bRun = true;
  while (bRun) {

    if (CANAL_ERROR_SUCCESS == (rv = m_pclient->receive(msg))) {
      spdlog::debug("checkResponseLevel1: RECEIVE id={:X} {:n}", msg.id, spdlog::to_hex(msg.data, msg.data + 7));
      if (((msg.id & 0xffffff00) == response_id)) { // correct id
        if ((int) (msg.id & 0xff) == m_nodeid) {    // from correct node
          // Response received from all - return success
          spdlog::debug("checkResponseLevel1: RECEIVE OK");
          if (nullptr != m_statusCallback) {
            //m_statusCallback(-1, "checkResponseLevel1: Response received OK.");
          }
          return VSCP_ERROR_SUCCESS;
        }
      }
    }

    // check for timeout
    time(&tnow);
    if ((unsigned long) (tnow - tstart) > BOOT_COMMAND_DEFAULT_RESPONSE_TIMEOUT) {
      bRun = false; // End
    }
  }

  spdlog::error("checkResponseLevel1: Timeout {}", rv);
  if (nullptr != m_statusCallback) {
    m_statusCallback(-1, "checkResponseLevel1: Timeout waiting while for response");
  }

  return VSCP_ERROR_TIMEOUT;
}

///////////////////////////////////////////////////////////////////////////////
// checkResponseLevel2
//

int
CBootDevice_PIC1::checkResponseLevel2(uint32_t /*id*/)
{
  int rv;
  vscpEventEx ex;
  time_t tstart, tnow;

  // Get system time
  time(&tstart);

  bool bRun = true;
  while (bRun) {

    if (VSCP_ERROR_SUCCESS == (rv = m_pclient->receive(ex))) {
      spdlog::error("checkResponseLevel2: failed to receive data {}", rv);
      return rv;
    }

    if ((VSCP_CLASS1_PROTOCOL == ex.vscp_class) && (m_guid.getLSB() == ex.GUID[15])) { // correct id
      // Response received - return success
      spdlog::debug("checkResponseLevel2: RECEIVE OK");
      if (nullptr != m_statusCallback) {
        //m_statusCallback(-1, "checkResponseLevel2: Response received OK.");
      }
      return VSCP_ERROR_SUCCESS;
    }

    // check for timeout
    time(&tnow);
    if ((tnow - tstart) > BOOT_COMMAND_DEFAULT_RESPONSE_TIMEOUT) {
      bRun = false; // abort timeout
    }
  }

  spdlog::error("checkResponseLevel2: Timeout {}", rv);
  if (nullptr != m_statusCallback) {
    m_statusCallback(-1, "checkResponseLevel2: Timeout waiting while for response");
  }

  return VSCP_ERROR_TIMEOUT;
}

///////////////////////////////////////////////////////////////////////////////
// writeFirmwareSector
//

int
CBootDevice_PIC1::writeFirmwareSector(uint8_t *paddr)
{
  canalMsg msg;
  vscpEventEx ex;
  int rv;
  int offset = 0;

  // Send event
  if ((CVscpClient::connType::CANAL == m_pclient->getType()) ||
      (CVscpClient::connType::SOCKETCAN == m_pclient->getType())) {

    msg.id       = ID_PUT_DATA;
    msg.flags    = CANAL_IDFLAG_EXTENDED;
    msg.sizeData = 8;

    // Put data into frame
    for (int i = 0; i < 8; i++) {
      msg.data[i] = paddr[i];
      m_checksum += paddr[i];
    };

    // send
    if (VSCP_ERROR_SUCCESS != (rv = m_pclient->send(msg))) {
      spdlog::error("PIC1 Firmware load: writeFirmwareSector: Failed to send CANAL message rv={} ", rv);
      if (nullptr != m_statusCallback) {
        m_statusCallback(-1, vscp_str_format("writeFirmwareSector: Failed to send CANAL message rv=%d", rv).c_str());
      }
      return rv;
    }

    spdlog::debug("writeFirmwareSector: SEND {0:n}", spdlog::to_hex(msg.data, msg.data + 7));

    if (m_bHandshake) {
      return checkResponseLevel1(ID_RESPONSE_PUT_DATA);
    }
  }
  else {
    ex.head       = 0;
    ex.vscp_class = 512;                     // CLASS2.PROTOCOL1
    ex.vscp_type  = (ID_PUT_DATA >> 8);      // 0x1100 >> 0x11
    memset(ex.GUID, 0, 16);                  // We use interface GUID
    ex.sizeData = 16 + 8;                    // Interface GUID
    memcpy(ex.data, m_guidif.getGUID(), 16); // node i/f
    offset = 16;

    // Put data into frame
    for (int i = 0; i < 8; i++) {
      msg.data[i] = paddr[i];
      m_checksum += paddr[i];
    };

    spdlog::debug("writeFirmwareSector: > {:n}", spdlog::to_hex(ex.data, ex.data + 16 + 7));

    // Send
    if (VSCP_ERROR_SUCCESS != (rv = m_pclient->send(ex))) {
      spdlog::error("PIC1 Firmware load: writeFirmwareSector: Failed to send CANAL message rv={} ", rv);
      if (nullptr != m_statusCallback) {
        m_statusCallback(-1, vscp_str_format("writeFirmwareSector: Failed to send CANAL message rv=%d", rv).c_str());
      }
      return rv;
    }

    if (m_bHandshake) {
      return checkResponseLevel2(ID_RESPONSE_PUT_DATA);
    }
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// writeFirmwareBlock
//

int
CBootDevice_PIC1::writeFirmwareBlock(uint32_t start, uint32_t end)
{
  int rv;
  uint32_t size = end - start + 1;
  uint32_t minAddr;
  uint32_t maxAddr;
  uint32_t nPackets;
  uint8_t *pbuf, *paddr;

  // If size is zero we are done
  if (!size) {
    return VSCP_ERROR_SUCCESS;
  }

  pbuf  = new uint8_t[size];
  paddr = pbuf;
  if (nullptr == pbuf) {
    return VSCP_ERROR_MEMORY;
  }

  if (VSCP_ERROR_SUCCESS != (rv = getMinMaxForRange(start, end, &minAddr, &maxAddr))) {
    delete[] pbuf;
    spdlog::error("writeFirmwareBlock: Failed to get min max range for block {0:X}-{1:X}", start, end);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, vscp_str_format("writeFirmwareBlock: Failed to get min max range for block", rv).c_str());
    }
    return rv;
  }

  // Nothing to do?
  if ((0 == minAddr) && (0 == maxAddr)) {
    delete[] pbuf;
    return VSCP_ERROR_SUCCESS;
  }

  if (maxAddr > minAddr) {
    nPackets = (maxAddr - minAddr) / 8;
    // A not completely full packet also count
    if (0 != ((maxAddr - minAddr) % 8)) {
      nPackets++;
    }

    // Init the block
    if (VSCP_ERROR_SUCCESS != (rv = fillMemoryBuffer(pbuf, size, start, end))) {
      spdlog::error("writeFirmwareBlock: Failed to fill code block with data.");
      if (nullptr != m_statusCallback) {
        m_statusCallback(-1,
                         vscp_str_format("writeFirmwareBlock: Failed to fill code block with data rv=%d", rv).c_str());
      }
      delete[] pbuf;
      return rv;
    }

    // Send the start block
    if (VSCP_ERROR_SUCCESS != (rv = writeDeviceControlRegs(minAddr))) {
      spdlog::error("Failed to send start block for flash data to remote device.");
      if (nullptr != m_statusCallback) {
        m_statusCallback(
          -1,
          vscp_str_format("writeFirmwareBlock: Failed to send start block for flash data to remote device. rv=%d", rv)
            .c_str());
      }
      delete[] pbuf;
      return rv;
    }

    // Start at beginning
    paddr += minAddr-start;

    for (uint32_t blk = 0; blk < nPackets; blk++) {
      spdlog::debug("Loading flash on remote device... block={0} {1:X}", blk, blk * 8);
      if (VSCP_ERROR_SUCCESS != writeFirmwareSector(paddr)) {
        spdlog::error("Failed to write flash data to node(s).");
        break;
      }
      paddr += 8;
      if (nullptr != m_statusCallback) {
        m_statusCallback((100 * blk) / nPackets, ""/*vscp_str_format("blk %d.", blk).c_str()*/);
      }

    } // for
  }   // code

  delete[] pbuf;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// deviceLoad
//

int
CBootDevice_PIC1::deviceLoad(std::function<void(int, const char *)> /*statusCallback*/, bool /*bAbortOnFirmwareCodeFail*/)
{
  //bool bRun = true;
  int rv;

  m_checksum        = 0;
  //uint32_t progress = 0;
  uint32_t addr = 0;
  std::string strStatus;

  //uint8_t pbuf[BUFFER_SIZE_CODE];
  //uint32_t nPackets;
  // uint32_t minAddr;
  // uint32_t maxAddr;

  if (nullptr != m_statusCallback) {
    m_statusCallback(0, "Starting firmware download");
  }

  if (VSCP_ERROR_SUCCESS !=
      writeDeviceControlRegs(addr, MODE_WRT_UNLCK | MODE_AUTO_ERASE | MODE_AUTO_INC | MODE_ACK, CMD_RST_CHKSM, 0, 0)) {
    spdlog::error("Failed to initialize checksum at node(s).");
    if (nullptr != m_statusCallback) {
      m_statusCallback(0, "Failed to initialize checksum at node(s).");
    }
    return VSCP_ERROR_COMMUNICATION;
  }

  // * * * * Flash memory * * * *
  if (VSCP_ERROR_SUCCESS != (rv = writeFirmwareBlock(MEM_CODE_START, MEM_CODE_END))) {
    spdlog::error("Failed to write flash block.");
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "Failed to write flash block.");
    }
    return rv;
  }

  // * * * * User id * * * *
  if (VSCP_ERROR_SUCCESS != (rv = writeFirmwareBlock(MEM_USERID_START, MEM_USERID_END))) {
    spdlog::error("Failed to write user id block.");
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "Failed to write user id block.");
    }
    return rv;
  }

  // * * * * Config * * * *
  if (VSCP_ERROR_SUCCESS != (rv = writeFirmwareBlock(MEM_CONFIG_START, MEM_CONFIG_END))) {
    spdlog::error("Failed to write user id block.");
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "Failed to write user id block.");
    }
    return rv;
  }

  // * * * * EEPROM * * * *
  if (VSCP_ERROR_SUCCESS != (rv = writeFirmwareBlock(MEM_EEPROM_START, MEM_EEPROM_END))) {
    spdlog::error("Failed to write user id block.");
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "Failed to write user id block.");
    }
    return rv;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// deviceRestart
//

int
CBootDevice_PIC1::deviceRestart(void)
{
  int rv;
  uint32_t min, max;

  if (VSCP_ERROR_SUCCESS !=
      getMinMaxForRange(CBootDevice_PIC1::MEM_CODE_START, CBootDevice_PIC1::MEM_CODE_END, &min, &max)) {
    spdlog::warn("deviceRestart: Failed to get start address. Use default.");
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "deviceRestart: Failed to get start flash address.");
    }
  }

  m_startAddr = min;

  // Check if checksum is correct and reset device(s)
  uint16_t calc_chksum = (0x10000 - (m_checksum & 0xffff));
  if (VSCP_ERROR_SUCCESS != (rv = writeDeviceControlRegs(m_startAddr,
                                                         MODE_WRT_UNLCK | MODE_AUTO_ERASE | MODE_ACK,
                                                         CMD_CHK_RUN,
                                                         (calc_chksum & 0xff),
                                                         ((calc_chksum >> 8) & 0xff)))) {
    spdlog::warn("deviceRestart: Failed to checksum restart device.rv={}.", rv);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, vscp_str_format("deviceRestart: Failed to checksum restart device.rv=%d", rv).c_str());
    }
    return rv;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// deviceReboot
//

int
CBootDevice_PIC1::deviceReboot(void)
{
  int rv;
  if (VSCP_ERROR_SUCCESS != (rv = writeDeviceControlRegs(0x0000, 0, CMD_RESET, 0, 0))) {
    spdlog::warn("deviceRestart: Failed to reboot device.rv={}.", rv);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, vscp_str_format("deviceReboot: Failed to reboot device.rv=%d", rv).c_str());
    }
    return rv;
  }
  return VSCP_ERROR_SUCCESS;
}
