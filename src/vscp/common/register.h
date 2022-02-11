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

#define FORMAT_ABSTRACTION_DECIMAL      0
#define FORMAT_ABSTRACTION_HEX          1

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
                              uint32_t timeout = 1000);

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
                                uint32_t timeout = 1000 );

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
                                    uint32_t timeout = 1000);

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
                                    uint32_t timeout = 1000);

/*!
  Read all standard registers
*/
int vscp_readStandardRegisters(CVscpClient& client,
                                cguid& guid,
                                cguid& guidInterface,
                                CStandardRegisters& stdregs,
                                uint32_t timeout = 0 );

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
                                uint32_t timeout = 1000);

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
    Get register from specific page
    @param page Page to read from.
    @param offset Register offset on page to read from.
    @return Register content or -1 on failure.
  */                                                
  int getReg(uint32_t reg, uint32_t page);

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
  //std::map<uint32_t, uint8_t> *
  CRegisterPage *getRegisterPage(uint16_t page);

  /*!
    Get the register content from a register at a specific page
    @param page Page to read from.
    @param offset Register offset on page to read from.
    @return Register content or -1 on failure.
  */
  int getRegister(uint16_t page, uint32_t offset);

  /*!
      Get abstraction value from registers into string value.
      @param abstraction Abstraction record from MDF.
      @param strValue Abstraction value in string form on return if call successful.
      @return true on success.
  */
  bool remoteVarFromRegToString( CMDF_RemoteVariable& remoteVar,
                                  std::string& strValue,
                                  uint8_t format = FORMAT_ABSTRACTION_DECIMAL  );

  /*
    * Store abstraction value in string format in corresponding registers.
    * @param abstraction Abstraction record from MDF.
    * @param strValue Abstraction value in string form.
    * @return true on success.
    */
  bool remoteVarFromStringToReg(CMDF_RemoteVariable& remoteVar,
                                  std::string &strValue);

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
      Get alarm byte
      @return VSCP alarm byte
    */
    uint8_t getAlarm(void) { return m_regs[0x80]; };

    
    /*!
      Get VSCP protocol conformance major version
      @return VSCP protocol conformance major version
    */
    uint8_t getConfirmanceVersionMajor(void) { return m_regs[0x81]; };

    /*!
      Get VSCP protocol conformance minor version
      @return VSCP protocol conformance minor version
    */
    uint8_t getConfirmanceVersionMinor(void) { return m_regs[0x82]; };

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
    int getReg(uint8_t reg) { return m_regs[reg & 0x7F]; };

    /*!
        Set value for register
        @param reg Register to set value for
        @param val Value to set register to
    */
    void setReg(uint8_t reg, uint8_t val) { m_regs[reg & 0x7F] = val; };



private:

    /// LEVEL1 or LEVELII    
    uint8_t m_level;

    /// Standard register storage
    std::map<uint8_t,uint8_t> m_regs;

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
