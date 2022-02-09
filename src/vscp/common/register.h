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
    CStandardRegisters( void );

    /*!
        Default Destructor
    */
    ~CStandardRegisters( void );

    /*!
      Get MDF URL
      @param url - Reference to string to store URL in.
    */
    std::string getMDF(void);

    /*!
      Get pointer to GUID array
    */
    const uint8_t *getGUID( void ) { return ( m_reg + 0xD0 - 0x80 ); };

    /*!
      Get guid as GUID class
    */
    void getGUID( cguid *pguid ) { pguid->getFromArray( m_reg + 0xD0 - 0x80 ); };

    /*! 
      Get buffer size
    */
    uint8_t getBufferSize( void ) { return m_reg[ 0x98 - 0x80 ]; };

    /*!
      Get bootloader algorithm
    */
    uint8_t getBootloaderAlgorithm( void ) { return m_reg[ 0x97 - 0x80 ]; };

    /*!
      Get firmware major version
    */
    uint8_t getMajorVersion( void ) { return m_reg[ 0x94 - 0x80 ]; };

    /*!
      Get firmware minor version
    */
    uint8_t getMinorVersion( void ) { return m_reg[ 0x95 - 0x80 ]; };

    /*!
      Get formware sub minor version
    */
    uint8_t getSubMinorVersion( void ) { return m_reg[ 0x96 - 0x80 ]; };

    /*!
      Get firmware version as string
    */
    std::string getFirmwareVersionString( void );

    /*!
      Get register page
    */
    uint16_t getRegisterPage( void ) { return ( m_reg[ 0x92 - 0x80 ] * 256 +
                                    m_reg[ 0x93 - 0x80 ] ); };

    /*!
      Get nickname id
    */
    uint8_t getNickname( void ) { return m_reg[ 0x91 - 0x80 ]; };

    /*!
      Get alarm byte
    */
    uint8_t getAlarm( void ) { return m_reg[ 0x80 - 0x80 ]; };

    /*!
      Get VSCP protocol conformance major version
    */
    uint8_t getConfirmanceVersionMajor( void ) { return m_reg[ 0x81 - 0x80 ]; };

    /*!
      Get VSCP protocol conformance minor version
    */
    uint8_t getConfirmanceVersionMinor( void ) { return m_reg[ 0x82 - 0x80 ]; };

    /*!
      Get node error counter
      (This was the node control byte in specs prior to 1.6)
      @return error counter
    */
    uint8_t getErrorCounter( void ) { return m_reg[ 0x83 - 0x80 ]; };

    /*!
      Get user ID
      @param index Index of user ID to get.
      @return User ID
    */
    uint8_t getUserId( uint8_t index ) { return m_reg[ 0x84 - 0x80 + index ]; };

    /*!
      Get number of register pages
      Deprecated
    */
    uint8_t getNumberOfRegisterPages( void ) { return m_reg[ 0x99 - 0x80 ]; };

    /*!
      Get manufacturer device id
    */
    uint32_t getManufacturerDeviceID( void )
                                { return ( ( m_reg[ 0x89 - 0x80 ] << 24 ) +
                                            ( m_reg[ 0x8A - 0x80 ] << 16 ) +
                                            ( m_reg[ 0x8B - 0x80 ] << 8 ) +
                                            ( m_reg[ 0x8C - 0x80 ] ) ); };

    /*!
      Get manufacturer sub device id
    */
    uint32_t getManufacturerSubDeviceID( void )
                                { return ( ( m_reg[ 0x8D - 0x80 ] << 24 ) +
                                            ( m_reg[ 0x8E - 0x80 ] << 16 ) +
                                            ( m_reg[ 0x8F - 0x80 ] << 8 ) +
                                            ( m_reg[ 0x90 - 0x80 ] ) ); };

    

    /*!
      Get standard family code (added in spec 1.9)
      @return Standard family code as 32-bit unsigned integer.
    */
    uint32_t getStandardDeviceFamilyCode(void) 
                                { return ( ( m_reg[ 0x9A - 0x80 ] << 24 ) +
                                            ( m_reg[ 0x9B - 0x80 ] << 16 ) +
                                            ( m_reg[ 0x9C - 0x80 ] << 8 ) +
                                            ( m_reg[ 0x9D - 0x80 ] ) ); };

    /*!
      Get standard family type (added in spec 1.9)
      @return Standard family type as 32-bit unsigned integer.
    */
    uint32_t getStandardDeviceFamilyType(void) 
                                { return ( ( m_reg[ 0x9E - 0x80 ] << 24 ) +
                                            ( m_reg[ 0x9F - 0x80 ] << 16 ) +
                                            ( m_reg[ 0xA0 - 0x80 ] << 8 ) +
                                            ( m_reg[ 0xA1 - 0x80 ] ) ); };

    /*!
      Get firmare device code (added in 1.13)
      @return Firmware device code as 16-bit unsigned integer
    */
    uint16_t getFirmwareDeviceCode(void)
                                { return (  ( m_reg[ 0xA3 - 0x80 ] << 8 ) +
                                            ( m_reg[ 0xA4 - 0x80 ] ) ); };

    /*!
      Get a standard register from offset
      @param reg Offset of standard register to read
      @return Register content of standard register or -1 on failure.
    */
    int getStandardReg( uint8_t reg );

    /*!
      Return a pointer to the register storage
      @return A pointer to standard register storage
    */
    unsigned char *getRegPointer( void ) { return m_reg; };

    /*!
        Set value for register
        @param reg Register to set value for
        @param val Value to set register to
    */
    void setReg( uint8_t reg, uint8_t val ) { m_reg[ reg - 0x80 ] = val; };


    /*!
        Get value for register
        @param reg Register to get value for
        @return Value of requested register.
    */
    int getReg( uint8_t reg ) { return m_reg[ reg - 0x80 ]; };

private:

    /*!
      The level for the device this registerset belongs to.
      VSCP_LEVEL1 or VSCP_LEVEL2
    */
    uint8_t m_level;

    /// Standard register storage
    uint8_t m_reg[ 128 ];

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
