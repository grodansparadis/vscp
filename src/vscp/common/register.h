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

#ifndef _VSCP_REGISTER_H_
#define _VSCP_REGISTER_H_

#include <set>
#include <map>

#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>
#include <canal.h>
#include <guid.h>
#include <vscp_client_base.h>

#define FORMAT_REMOTEVAR_DECIMAL      0
#define FORMAT_REMOTEVAR_HEX          1

class CRegisterPage;
class CUserRegisters;
class CStandardRegisters;

/*!
  Read VSCP register
  @param client VSCP client derived from the client vase class over
                which the communication is carried out.
  @param guidNode GUID of the device to read from. Only the lsb (nickname)
                is used for level I communication. 
  @param guidInterface GUID of the interface to read from. Set to all zero
                if no interface.                
  @param page Register page to read from. 
  @param offset Register offset on page to read from.
  @param value Value read from register.
  @param timeout Timeout in milliseconds. Zero means no timeout i.e. wait forever.
  @return VSCP_ERROR_SUCCESS on success.
*/
int vscp_readLevel1Register( CVscpClient& client,
                              cguid& guidNode,
                              cguid& guidInterface,
                              uint16_t page, 
                              uint8_t offset,
                              uint8_t& value,  
                              uint32_t timeout = 2000);

/*!
  Write VSCP register
  @param client VSCP client derived from the client vase class over
                which the communication is carried out. 
  @param guid GUID of the device to read from. Only the lsb (nickname)
                is used for level I communication. 
  @param guidInterface GUID of the interface to read from. Set to all zero
                if no interface.                
  @param page Register page to read from. 
  @param offset Register offset on page to read from.
  @param value Value to write.
  @param timeout Timeout in milliseconds. Zero means no timeout i.e. wait forever.
  @return VSCP_ERROR_SUCCESS on success.           
*/
int vscp_writeLevel1Register( CVscpClient& client,
                                cguid& guidNode,
                                cguid& guidInterface,
                                uint16_t page,
                                uint8_t offset,
                                uint8_t value,
                                uint32_t timeout = 2000 );

/*!
  Read VSCP register block.
  @param client VSCP client derived from the client vase class over
                which the communication is carried out.
  @param guidNode GUID of the device to read from. Only the lsb (nickname)
                is used for level I communication. 
  @param guidInterface GUID of the interface to read from. Set to all zero
                if no interface.                
  @param page Register page to read from. 
  @param offset Register offset on page to read from.
  @param count Number of registers to read. Zero means read 256 registers (0-255).
  @param values Pointer to map with registers to read.
  @param timeout Timeout in milliseconds. Zero means no timeout i.e. wait forever.
  @return VSCP_ERROR_SUCCESS on success.
*/
int vscp_readLevel1RegisterBlock( CVscpClient& client,
                                    cguid& guidNode,
                                    cguid& guidInterface,
                                    uint16_t page, 
                                    uint8_t offset,
                                    uint8_t count,
                                    std::map<uint8_t,uint8_t>& values,
                                    uint32_t timeout = 2000);

/*!
  Write VSCP register block.
  The function tries to group register writes wherever
  @param client VSCP client derived from the client vase class over
                which the communication is carried out.
  @param guidNode GUID of the device to read from. Only the lsb (nickname)
                is used for level I communication. 
  @param guidInterface GUID of the interface to read from. Set to all zero
                if no interface.                
  @param page Register page to read from. 
  @param values Pointer to map with register values to write.
  @param timeout Timeout in milliseconds. Zero means no timeout i.e. wait forever.
  @return VSCP_ERROR_SUCCESS on success.
*/
int vscp_writeLevel1RegisterBlock( CVscpClient& client,
                                    cguid& guidNode,
                                    cguid& guidInterface,
                                    uint16_t page, 
                                    std::map<uint8_t,uint8_t>& values,
                                    uint32_t timeout = 2000);

/*!
  Read all standard registers
*/
int vscp_readStandardRegisters(CVscpClient& client,
                                cguid& guid,
                                cguid& guidInterface,
                                CStandardRegisters& stdregs,
                                uint32_t timeout = 2000 );

/*!
  Do a fast register scan using who is there protocol functionality
  @param client VSCP client derived from the client vase class over
                which the communication is carried out.
  @param guid GUID of the interface to search on. If zero no interface
                is used.
  @param found A set with nodeid's for found nodes.
  @param timeout Timeout in milliseconds. Zero means no timeout
*/
int vscp_scanForDevices(CVscpClient& client,
                                cguid& guid,
                                std::set<uint8_t> &found,
                                uint32_t timeout = 2000);

/*!
  Do a fast register scan using who is there protocol functionality
  @param client VSCP client derived from the client vase class over
                which the communication is carried out.
  @param guid GUID of the interface to search on. If zero no interface
                is used.
  @param found_nodes A set with nodeid's for found nodes.
  @param start_nodeid Start nodeid to search from.
  @param end_nodeid End nodeid to search to.
  @param timeout Timeout in milliseconds. Zero means no timeout
*/
int vscp_scanSlowForDevices(CVscpClient& client,
                                cguid& guid,
                                std::set<uint8_t> &found_nodes,
                                uint8_t start_node = 0,
                                uint8_t end_node = 255,
                                uint32_t delay = 10000,
                                uint32_t timeout = 2000);


/*!
  Get device information on HTML format
  @param mdf Initialized MDF object
  @param stdregs INitialized standard registers
  @return HTML formatted device information in a standard string
*/
std::string vscp_getDeviceInfoHtml(CMDF& mdf, CStandardRegisters& stdregs);

///////////////////////////////////////////////////////////////////////////////



/*!
    \class CRegistersPage
    \brief Encapsulates one page of user registers of a device

    This class encapsulates one page of user registers of a device. For a 
    level II device this is the page 0x00 (the only page).
*/
class CRegisterPage
{

public:
  /*!
    Constructor
    @param level - Level of device.
    @param page - Page this set of registers is located on.
  */
  CRegisterPage(uint8_t level = 0, uint16_t page = 0);
  ~CRegisterPage();

  /*!
    Write level 1 register map to page map
    @param registers - Registers to write
  */
  void putLevel1Registers(std::map<uint8_t, uint8_t>& registers);

  /*!
    Get level 1 register map 
    @param registers - Registers to write
  */
  void getLevel1Registers(std::map<uint8_t, uint8_t>& registers);

  /*!
    Read register content on a register page.
    Register must exist.

    @param offset Register to read
    @return Register content or -1 on failure.
  */
  int getReg(uint32_t offset);

  /*!
      Write value to a register on a register page.
      Register is created if it does not exist.

      @param offset Register to write to.
      @param value to write to register.
      @return Register content or -1 on failure.
  */
  int putReg(uint32_t offset, uint8_t value);

  /*!
    Return page for this set of registers.
    @return Register page
  */
  uint16_t getPage(void) { return m_page; };

  /*!
    Set page for this set of registers.
    @param page Register page to set
  */
  void setPage(uint16_t page) { m_page = page; };

  /*!
    Manually set a single change position to true opr false
    @param offset Register offset to set.
    @param state State to set change to.
  */
  void setSingleChange(uint32_t offset, bool state = true) { m_change[offset] = state; };

  /*!
    Clear changes
  */
  void clearChanges() { m_change.clear(); };

  /*!
    Clear history
  */
  void clearHistory() { m_list_undo_value.clear(); m_list_redo_value.clear(); };

  /*!
    Get the register changes
    @return Register changes
  */
  std::map<uint32_t, bool> *getChanges(void) { return &m_change; };

  /*!
    Set changed state
    @param offset Register offset.
    @param state Changed state
  */
  void setChangedState(uint32_t offset, bool state = true) { m_change[offset] = state; };

  /*!
    Check if a register has an unwritten change
    @param offset Register offset on page to read from.
    @return true if register has an unwritten change.
  */
  bool isChanged(uint32_t offset) { return m_change[offset]; };

  /*!
    Return true if one or more changes are pending
    @return true if one or more changes are pending
  */
  bool hasChanges(void);

  /*!
    Check if register has previously been changed
    @param offset Register offset on page to read from.
    @return true if register has previously been changed.
  */
  bool hasWrittenChange(uint32_t offset) { return (m_list_undo_value[offset].size() > 0); };

  /*!
    Get the register map for this page
  */
  std::map<uint32_t, uint8_t> *getRegisterMap(void) { return &m_registers; }; 

private:

  /*!
    The level for the device this registerset belongs to.
    VSCP_LEVEL1 or VSCP_LEVEL2
  */
  uint8_t m_level;

  /*!
    The page for this register set. If a level II register set
    there is only one page.
  */
  uint16_t m_page;

  /*! 
    Defined registers on the register page [offset, value]
    
    Level I devices: 0-127
    Level II devices have a single page of registers in the 
    range 0x00000000 - 0xffff0000.
  */
  std::map<uint32_t, uint8_t> m_registers;

  /*!
    VSCP Works rowtype for this register.
    0 = register
    1 = group
  */
  std::map<uint32_t, int>  m_rowType;

  /*!
    This map contains pointers to MDF remote variable objects
    for registers that define remote variables.
  */
  std::map<uint32_t, CMDF_RemoteVariable*> m_remoteVariableMap;

  /*!
    This map contains pointers to MDF remote variable objects
    for registers that define remote variables.
  */
  std::map<uint32_t, CMDF_DecisionMatrix*> m_DecsionMatrixMap;

  /*!
    Here a register position is true for a register that
    has received a value it did not have before.
  */
  std::map<uint32_t, bool> m_change;

  /*!
    undo/redo queues
    put:
      write old value to m_list_undo_value
    undo: 
      current value to m_list_redo_value (if there is undo values)
      m_list_undo_value to current value (if there is undo values)
    redo:
      current value to m_list_undo_value (if there is redo values)
      m_list_redo_value to current value (if there is redo values) 
  */

  // offset, queue with value
  std::map<uint32_t, std::deque<uint8_t>> m_list_undo_value;
  std::map<uint32_t, std::deque<uint8_t>> m_list_redo_value;

  //std::deque<uint8_t> m_list_undo_value;  // List with undo values
  //std::deque<uint8_t> m_list_redo_value;  // List with redo values
};



///////////////////////////////////////////////////////////////////////////////



/*!
    \class CUserRegisters
    \brief Encapsulates the user registers (all pages) for a Level I or Level 
            II device
*/
class CUserRegisters
{

public:

  CUserRegisters( uint8_t level = VSCP_LEVEL1 );
  ~CUserRegisters();

  /*! 
      Read all user register content
      @param client The communication interface to use
      @param guidNode GUID for node. Only LSB is used for level I node.
      @param guidInterface GUID for interface. If zero no interface is used.
      @param pages A set holding the valied pages
      @param timeout Timeout in milliseconds. Zero means no timeout
      @return VSCP_ERROR_SUCCESS on success.
    */
  int init(CVscpClient& client,
              cguid& guidNode,
              cguid& guidInterface,
              std::set<uint16_t>& pages, 
              uint32_t timeout = 1000);

  /*!
      Set value for register
      @param reg Register to set value for
      @param val Value to set register to
  */
  bool putReg(uint32_t reg, uint32_t page, uint8_t value);

  /*!
    Return a pointer to the register storage map for a page of registers
    @return A pointer to the register storage map
  */
  std::map<uint32_t, uint8_t> *getRegisterMap( uint16_t page );

  /*!
    Get pointer to a user register page
    @param page Page of user register to get
    @return Pointer to user register object
  */
  CRegisterPage *getRegisterPage(uint16_t page);

  /*!
    Get the register content from a register at a specific page
    @param offset Register offset on page to read from.
    @param page Page to read from.    
    @return Register content or -1 on failure.
  */
  int getReg(uint32_t offset, uint16_t page = 0);

  /*!
    Get register pages
    @return Sorted set with pages
  */
  std::set<long> *getPages(void) { return &m_pages; };

  /*!
    Set changed state
    @param offset Register offset
    @param  page Register page
    @param state State to set
    @return true on success.
  */
  bool setChangedState(uint32_t offset, uint16_t page = 0,  bool state = true);

  /*!
    Check if a register has an unwritten change
    @param page Page to read from.
    @param offset Register offset on page to read from.
    @return true if register has an unwritten change.
  */
  bool isChanged(uint32_t offset, uint16_t page = 0);

  /*!
    Check if a register has a written change (blue)
    @param page Page to read from.
    @param offset Register offset on page to read from.
    @return true if register has an written change (blue).
  */
  bool hasWrittenChange(uint32_t offset, uint16_t page = 0);

  /*!
    Clear changes
  */
  void clearChanges();

  /*!
    Return true if one or more changes are pending
    @return true if one or more changes are pending
  */
  bool hasChanges(void);

  /*!
    Clear history
  */
  void clearHistory();

  /*!
      Get abstraction value from registers into string value.
      @param abstraction Abstraction record from MDF.
      @param strValue Abstraction value in string form on return if call successful.
      @return VSCP_ERROR_SUCCES on success and error code else.
  */
  int remoteVarFromRegToString( CMDF_RemoteVariable& remoteVar,
                                  std::string& strValue,
                                  uint8_t format = FORMAT_REMOTEVAR_DECIMAL  );

  /*
    * Store abstraction value in string format in corresponding registers.
    * @param abstraction Abstraction record from MDF.
    * @param strValue Abstraction value in string form.
    * @return VSCP_ERROR_SUCCES on success and error code else.
    */
  int remoteVarFromStringToReg(CMDF_RemoteVariable& remoteVar,
                                  std::string &strValue);

  /*!
    Get register content for pos in DM row
    @param dm Decsion matrix
    @param row DM row to read.
    @param pos Position in DM row
    @return Register content for DM pos or -1 on failure.
  */
  int vscp_getGetDmRegisterContent(CMDF_DecisionMatrix& dm, uint16_t row, uint8_t pos);

  /*!
    Read DM row
    @param dm Decsion matrix
    @param row DM row to read.
    @param buf Buffer holding DM row data to write. The size of the
              data is fiven by the DM row size.
    @return VSCP_ERROR_SUCCESS on success or error code on failure.
  */
  int vscp_readDmRow(CMDF_DecisionMatrix& dm, uint16_t row, uint8_t *buf);

  /*! 
    Write DM row
    @param dm Decsion matrix
    @param row DM row to read.
    @param buf Buffer that will get DM row data on a successfull read. The
            size of the data is fiven by the DM row size.
    @return VSCP_ERROR_SUCCESS on success or error code on failure.
  */
  int vscp_writeDmRow(CMDF_DecisionMatrix& dm, uint16_t row, uint8_t *buf);

private:

    /*!
        Tells if this is Level I or Level II registers
    */
    uint8_t m_level;

    // set with valid register pages
    std::set<long> m_pages;

    // pages {page number, defined registers}
    std::map<uint16_t, CRegisterPage *> m_registerPageMap;   
 
};



///////////////////////////////////////////////////////////////////////////////



struct __struct_standard_register_defs {
  uint16_t reg;
  uint8_t access;  // 0 = read only, 1 = read/write, 2 = write only
  uint32_t bgcolor;
  std::string name;
  std::string description;
};

#define __GUID_STDREG_DESCRIPTION__ __MDF_STDREG_DESCRIPTION__
#define __MDF_STDREG_DESCRIPTION__ "Module Description File URL. A zero terminates the ASCII string if not exactly 32 bytes long. The URL points to a file that gives further information about where drivers for different environments are located. Can be returned as a zero string for devices with low memory. For a node with an embedded MDF return a zero string. The CLASS1.PROTOCOL, Type=34/35 can then be used to get the information if available."  


/*!
    \class CStandardRegisters
    \brief Encapsulates the standard registers of a device
*/
class CStandardRegisters
{

public:

  /*!
      Default Constructor
  */
  CStandardRegisters(uint8_t level = VSCP_LEVEL1);

  /*!
      Default Destructor
  */
  ~CStandardRegisters(void);

  /*!
    Standard register definitions
  */

  const __struct_standard_register_defs m_vscp_standard_registers_defs[85] =
  {
    {0x80,0,0xccffdd,"Alarm status register","Alarm status register content (!= 0 indicates alarm). Condition is reset by a read operation. The bits represent different alarm conditions."},
    {0x81,0,0xffff12,"VSCP specification major version number conformance","VSCP Major version number this device is constructed for."},
    {0x82,0,0xffff12,"VSCP specification minor version number conformance","VSCP Minor version number this device is constructed for."},
    {0x83,0,0xffffd2,"Error counter (was node control flag prior to version 1.6)","VSCP error counter is increased when an error occurs on the device. Reset error counter by reading it."},
    {0x84,1,0xffff12,"User id 0","Client user node-ID byte 0. Use for location info or similar."},
    {0x85,1,0xffff12,"User id 1","Client user node-ID byte 1. Use for location info or similar."},
    {0x86,1,0xffff12,"User id 2","Client user node-ID byte 2. Use for location info or similar."},
    {0x87,1,0xffff12,"User id 3","Client user node-ID byte 3. Use for location info or similar."},
    {0x88,1,0xffff12,"User id 4","Client user node-ID byte 4. Use for location info or similar."},
    {0x89,0,0xffffd2,"Manufacturer device id 0","Manufacturer device ID byte 0. For hardware/firmware/manufacturing info."},
    {0x8A,0,0xffffd2,"Manufacturer device id 1","Manufacturer device ID byte 1. For hardware/firmware/manufacturing info."},
    {0x8B,0,0xffffd2,"Manufacturer device id 2","Manufacturer device ID byte 2. For hardware/firmware/manufacturing info."},
    {0x8C,0,0xffffd2,"Manufacturer device id 3","Manufacturer device ID byte 3. For hardware/firmware/manufacturing info."},
    {0x8D,0,0xffff12,"Manufacturer sub device id 0","Manufacturer sub device ID byte 0. For hardware/firmware/manufacturing info."},
    {0x8E,0,0xffff12,"Manufacturer sub device id 1","Manufacturer sub device ID byte 1. For hardware/firmware/manufacturing info."},  
    {0x8F,0,0xffff12,"Manufacturer sub device id 2","Manufacturer sub device ID byte 2. For hardware/firmware/manufacturing info."},
    {0x90,0,0xffff12,"Manufacturer sub device id 3","Manufacturer sub device ID byte 3. For hardware/firmware/manufacturing info."},
    {0x91,0,0xffffd2,"Nickname id","Nickname for the device"},
    {0x92,1,0xffff12,"Page select MSB","MSB byte of current selected register page."},
    {0x93,1,0xffff12,"Page select LSB","LSB byte of current selected register page."},
    {0x94,0,0xffffd2,"Firmware major version number","Major version number for device firmware."},
    {0x95,0,0xffffd2,"Firmware minor version number","Minor version number for device firmware."},
    {0x96,0,0xffffd2,"Firmware build version number","Build version of device firmware."},
    {0x97,0,0xb3d9ff,"Boot loader algorithm","Boot loader algorithm used to bootload this device. Code 0xFF is used for no boot loader support. All defined codes for algorithms are specified <a href=\"https://grodansparadis.github.io/vscp-doc-spec/#/./class1.protocol?id=id\">here</a>"},
    {0x98,0,0xffffd2,"Buffer size","Buffer size. The value here gives an indication for clients that want to talk to this node if it can support the larger mid level Level I control events which has the full GUID. If set to 0 the default size should used. That is 8 bytes for Level I and 512-25 for Level II."},
    {0x99,0,0xb3d9ff,"Deprecated: Number of register pages used","Number of register pages used. If not implemented one page is assumed. Set to zero if your device have more then 255 pages. <b>Deprecated</b>: Use the MDF instead as the central place for information about actual number of pages."},
    {0x9A,0,0xffff12,"Standard device family code MSB","Standard device family code (MSB) Devices can belong to a common register structure standard. For such devices this describes the family coded as a 32-bit integer. Set all bytes to zero if not used. Also 0xff is reserved and should be interpreted as zero was read. <i>Added in version 1.9.0 of the specification</i>."},
    {0x9B,0,0xffff12,"Standard device family code","Standard device family code <i>Added in version 1.9.0 of the specification</i>."},
    {0x9C,0,0xffff12,"Standard device family code","Standard device family code <i>Added in version 1.9.0 of the specification</i>."},
    {0x9D,0,0xffff12,"Standard device family code LSB","Standard device family code <i>Added in version 1.9.0 of the specification</i>."},
    {0x9E,0,0xffffd2,"Standard device type MSB","Standard device type (MSB) This is part of the code that specifies a device that adopts to a common register standard. This is the type code represented by a 32-bit integer and defines the type belonging to a specific standard. <i>Added in version 1.9.0 of the specification</i>."},
    {0x9F,0,0xffffd2,"Standard device type","Standard device family code. <i>Added in version 1.9.0 of the specification.</i>."},
    {0xA0,0,0xffffd2,"Standard device type","Standard device family code. <i>Added in version 1.9.0 of the specification.</i>."},
    {0xA1,0,0xffffd2,"Standard device type LSB","Standard device family code (LSB). <i>Added in version 1.9.0 of the specification.</i>."},
    {0xA2,2,0xff9999,"Restore factory defaults (Added in version 1.10)}","Standard configuration should be restored for a unit if first 0x55 and then 0xAA is written to this location and is done so withing one second. <i>Added in version 1.10.0 of the specification</i>."},
    {0xA3,0,0xffffd2,"Firmware device code MSB (Added in version 1.13)","Firmware device code MSB. <i>Added in version 1.13.0 of the specification</i>."},
    {0xA4,0,0xffffd2,"Firmware device code LSB (Added in version 1.13)","Firmware device code LSB. <i>Added in version 1.13.0 of the specification</i>."},
    {0xD0,0,0xb3d9ff,"GUID byte 0 MSB",__MDF_STDREG_DESCRIPTION__},
    {0xD1,0,0xb3d9ff,"GUID byte 1",__MDF_STDREG_DESCRIPTION__},
    {0xD2,0,0xb3d9ff,"GUID byte 2",__MDF_STDREG_DESCRIPTION__},
    {0xD3,0,0xb3d9ff,"GUID byte 3",__MDF_STDREG_DESCRIPTION__},
    {0xD4,0,0xb3d9ff,"GUID byte 4",__MDF_STDREG_DESCRIPTION__},
    {0xD5,0,0xb3d9ff,"GUID byte 5",__MDF_STDREG_DESCRIPTION__},
    {0xD6,0,0xb3d9ff,"GUID byte 6",__MDF_STDREG_DESCRIPTION__},
    {0xD7,0,0xb3d9ff,"GUID byte 7",__MDF_STDREG_DESCRIPTION__},
    {0xD8,0,0xb3d9ff,"GUID byte 8",__MDF_STDREG_DESCRIPTION__},
    {0xD9,0,0xb3d9ff,"GUID byte 9",__MDF_STDREG_DESCRIPTION__},
    {0xDA,0,0xb3d9ff,"GUID byte 10",__MDF_STDREG_DESCRIPTION__},
    {0xDB,0,0xb3d9ff,"GUID byte 11",__MDF_STDREG_DESCRIPTION__},
    {0xDC,0,0xb3d9ff,"GUID byte 12",__MDF_STDREG_DESCRIPTION__},
    {0xDD,0,0xb3d9ff,"GUID byte 13",__MDF_STDREG_DESCRIPTION__},
    {0xDE,0,0xb3d9ff,"GUID byte 14",__MDF_STDREG_DESCRIPTION__},
    {0xDF,0,0xb3d9ff,"GUID byte 15 LSB",__MDF_STDREG_DESCRIPTION__},  
    {0xE0,0,0xccffdd,"MDF byte 0 MSB",__MDF_STDREG_DESCRIPTION__},
    {0xE1,0,0xccffdd,"MDF byte 1",__MDF_STDREG_DESCRIPTION__},
    {0xE2,0,0xccffdd,"MDF byte 2",__MDF_STDREG_DESCRIPTION__},
    {0xE3,0,0xccffdd,"MDF byte 3",__MDF_STDREG_DESCRIPTION__},
    {0xE4,0,0xccffdd,"MDF byte 4",__MDF_STDREG_DESCRIPTION__},
    {0xE5,0,0xccffdd,"MDF byte 5",__MDF_STDREG_DESCRIPTION__},
    {0xE6,0,0xccffdd,"MDF byte 6",__MDF_STDREG_DESCRIPTION__},
    {0xE7,0,0xccffdd,"MDF byte 7",__MDF_STDREG_DESCRIPTION__},
    {0xE8,0,0xccffdd,"MDF byte 8",__MDF_STDREG_DESCRIPTION__},
    {0xE9,0,0xccffdd,"MDF byte 9",__MDF_STDREG_DESCRIPTION__},
    {0xEA,0,0xccffdd,"MDF byte 10",__MDF_STDREG_DESCRIPTION__},
    {0xEB,0,0xccffdd,"MDF byte 11",__MDF_STDREG_DESCRIPTION__},
    {0xEC,0,0xccffdd,"MDF byte 12",__MDF_STDREG_DESCRIPTION__},
    {0xED,0,0xccffdd,"MDF byte 13",__MDF_STDREG_DESCRIPTION__},
    {0xEE,0,0xccffdd,"MDF byte 14",__MDF_STDREG_DESCRIPTION__},
    {0xEF,0,0xccffdd,"MDF byte 15",__MDF_STDREG_DESCRIPTION__}, 
    {0xF0,0,0xccffdd,"MDF byte 16",__MDF_STDREG_DESCRIPTION__},
    {0xF1,0,0xccffdd,"MDF byte 17",__MDF_STDREG_DESCRIPTION__},
    {0xF2,0,0xccffdd,"MDF byte 18",__MDF_STDREG_DESCRIPTION__},
    {0xF3,0,0xccffdd,"MDF byte 19",__MDF_STDREG_DESCRIPTION__},
    {0xF4,0,0xccffdd,"MDF byte 20",__MDF_STDREG_DESCRIPTION__},
    {0xF5,0,0xccffdd,"MDF byte 21",__MDF_STDREG_DESCRIPTION__},
    {0xF6,0,0xccffdd,"MDF byte 22",__MDF_STDREG_DESCRIPTION__},
    {0xF7,0,0xccffdd,"MDF byte 23",__MDF_STDREG_DESCRIPTION__},
    {0xF8,0,0xccffdd,"MDF byte 24",__MDF_STDREG_DESCRIPTION__},
    {0xF9,0,0xccffdd,"MDF byte 25",__MDF_STDREG_DESCRIPTION__},
    {0xFA,0,0xccffdd,"MDF byte 26",__MDF_STDREG_DESCRIPTION__},
    {0xFB,0,0xccffdd,"MDF byte 27",__MDF_STDREG_DESCRIPTION__},
    {0xFC,0,0xccffdd,"MDF byte 28",__MDF_STDREG_DESCRIPTION__},
    {0xFD,0,0xccffdd,"MDF byte 29",__MDF_STDREG_DESCRIPTION__},
    {0xFE,0,0xccffdd,"MDF byte 30",__MDF_STDREG_DESCRIPTION__},
    {0xFF,0,0xccffdd,"MDF byte 31 LSB",__MDF_STDREG_DESCRIPTION__}
  };

  /*! 
    Read all standard register content
    @param client The communication interface to use
    @param guidNode GUID for node. Only LSB is used for level I node.
    @param guidInterface GUID for interface. If zero no interface is used.
    @param timeout Timeout in milliseconds. Zero means no timeout
    @return VSCP_ERROR_SUCCESS on success.
  */
  int init(CVscpClient& client,
              cguid& guidNode,
              cguid& guidInterface,
              uint32_t timeout = 1000);

  /*!
    Initialization with already read standard registers
    @param userRegs Reference to already read registers.
    @return VSCP_ERROR_SUCCESS on success.
  */
  int init(CRegisterPage& userRegs);

  /*!
    Get alarm byte
    @return VSCP alarm byte
  */
  uint8_t getAlarm(void) { return m_regs[0x80]; };

  
  /*!
    Get VSCP protocol conformance major version
    @return VSCP protocol conformance major version
  */
  uint8_t getConformanceVersionMajor(void) { return m_regs[0x81]; };

  /*!
    Get VSCP protocol conformance minor version
    @return VSCP protocol conformance minor version
  */
  uint8_t getConformanceVersionMinor(void) { return m_regs[0x82]; };

  /*!
    Get node error counter
    (This was the node control byte in specs prior to 1.6)
    @return error counter
  */
  uint8_t getErrorCounter(void) { return m_regs[0x83]; };

  /*!
    Get user ID
    @param index Index of user ID to get.
    @return User ID for pos
  */
  uint8_t getUserId(uint8_t index) {return m_regs[ 0x84 + index ];};

  /*!
    Get user ID
    @param uid Buffer of five bytes for user id.
  */
  void getUserId(uint8_t *puid) 
                        { 
                          puid[0] = m_regs[0x84];
                          puid[1] = m_regs[0x85];
                          puid[2] = m_regs[0x86];
                          puid[3] = m_regs[0x87];
                          puid[4] = m_regs[0x88];
                        };

  /*!
    Get manufacturer device id
    @return Manufacturer device id
  */
  uint32_t getManufacturerDeviceID(void)
                              { return ( ( m_regs[0x89] << 24 ) +
                                          ( m_regs[0x8A] << 16 ) +
                                          ( m_regs[0x8B] << 8 ) +
                                          ( m_regs[0x8C] ) ); };
  /*!
    Get manufacturer sub device id
    @return Manufacturer sub device id
  */
  uint32_t getManufacturerSubDeviceID(void)
                              { return ( ( m_regs[0x8D] << 24 ) +
                                          ( m_regs[0x8E] << 16 ) +
                                          ( m_regs[0x8F] << 8 ) +
                                          ( m_regs[0x90] ) ); };

  /*!
    Get nickname id
    @return nickname id
  */
  uint8_t getNickname(void) {return m_regs[0x91];};

  /*!
    Get register page
    @return register page
  */
  uint16_t getRegisterPage(void) { return ( m_regs[0x92] * 256 +
                                            m_regs[0x93] ); };

  /*!
    Get firmware major version
  */
  uint8_t getFirmwareMajorVersion(void) 
                                  {return m_regs[0x94];};

  /*!
    Get firmware minor version
  */
  uint8_t getFirmwareMinorVersion(void) 
                                  {return m_regs[0x95];};

  /*!
    Get firmware sub minor version
  */
  uint8_t getFirmwareSubMinorVersion(void) 
                                  {return m_regs[0x96];};

  /*!
    Get firmware version as string
  */
  std::string getFirmwareVersionString(void);

  /*!
    Get bootloader algorithm
    @return bootloader algorithm
  */
  uint8_t getBootloaderAlgorithm(void) 
                                  {return m_regs[0x97];};

  /*! 
    Get buffer size
    @return buffer size
  */
  uint8_t getBufferSize(void) {return m_regs[0x98];};                                    

  /*!
    Get number of register pages !!!DEPRECATED!!!
    @return number of register pages
  */
  uint8_t getNumberOfRegisterPages(void) {return m_regs[0x99]; };

  /*!
    Get standard family code (added in spec 1.9)
    @return Standard family code as 32-bit unsigned integer.
  */
  uint32_t getStandardDeviceFamilyCode(void) 
                              { return ( ( m_regs[0x9A] << 24 ) +
                                          ( m_regs[0x9B] << 16 ) +
                                          ( m_regs[0x9C] << 8 ) +
                                          ( m_regs[0x9D] ) ); };

  /*!
    Get standard family type (added in spec 1.9)
    @return Standard family type as 32-bit unsigned integer.
  */
  uint32_t getStandardDeviceFamilyType(void) 
                              { return ( ( m_regs[0x9E] << 24 ) +
                                          ( m_regs[0x9F] << 16 ) +
                                          ( m_regs[0xA0] << 8 ) +
                                          ( m_regs[0xA1] ) ); };

  /*!
    Restore standard configuration for node
  */
  int restoreStandardConfig(CVscpClient& client,
                              cguid& guidNode,
                              cguid& guidInterface,
                              uint32_t timeout = 1000);

  /*!
    Get firmare device code (added in 1.13)
    @return Firmware device code as 16-bit unsigned integer
  */
  uint16_t getFirmwareDeviceCode(void)
                              { return ( (m_regs[0xA3] << 8 ) +
                                          (m_regs[0xA4] ) ); };
  
  /*!
    Get guid as GUID class
  */
  void getGUID(cguid& guid);


  /*!
    Get MDF URL
    @param url - Reference to string to store URL in.
  */
  std::string getMDF(void);


  /*!
    Get a standard register value from offset
    @param reg Offset of standard register to read
    @return Register content of standard register or -1 on failure.
  */
  int getReg(uint8_t reg) { return m_regs[reg]; };

  /*!
      Set value for register
      @param reg Register to set value for
      @param val Value to set register to
  */
  void putReg(uint8_t reg, uint8_t val) { m_regs[reg] = val; };

  /*!
    Clear the register changes marks
  */
  void clearChanges(void) { m_change.clear(); };

  /*!
    Get the register changes
    @return Register changes
  */
  std::map<uint32_t, bool> *getChanges(void) { return &m_change; };

  /*!
    Set changed state
    @param offset Register offset.
    @param state Changed state
  */
  void setChangedState(uint32_t offset, bool state = true) { m_change[offset] = state; };

  /*!
    Check if a register has an unwritten change
    @param offset Register offset on page to read from.
    @return true if register has an unwritten change.
  */
  bool isChanged(uint32_t offset) { return m_change[offset]; };

  /*!
    Return true if one or more changes are pending
    @return true if one or more changes are pending
  */
  bool hasChanges(void);

private:

  /// LEVEL1 or LEVEL2    
  uint8_t m_level;

  /// Standard register storage
  std::map<uint8_t, uint8_t> m_regs;

  /*!
    Here a register position is true for a register that
    has received a value it did not have before.
  */
  std::map<uint32_t, bool> m_change;
};


///////////////////////////////////////////////////////////////////////////////


/*!
  Class that describes a full node of a VSCP device
*/

class CVscpNode
{

public:
  CVscpNode(void);
  ~CVscpNode(void);

  /*!
    Get pointer to user register object
    @return Pointer to user register object
  */
  CUserRegisters *getUserRegs(void) {return &m_reg;};

  /*!
    Get pointer to standard register object
    @return Pointer to standard register object
  */
  CStandardRegisters *getStandardRegs( void ) {return &m_stdReg;};

private:

  /*!
    Holds the user registers of the device
  */
  CUserRegisters m_reg;

  /*!
    Holds the standard registers of the device
  */
  CStandardRegisters m_stdReg;

};

#endif
