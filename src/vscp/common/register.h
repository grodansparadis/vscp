// FILE: register.h
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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

#ifndef _REGISTER_H_
#define _REGISTER_H_

#include "wx/wx.h"
#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>
#include <canal.h>
#include <guid.h>

#define FORMAT_ABSTRACTION_DECIMAL      0
#define FORMAT_ABSTRACTION_HEX          1

/*!
    \class CDecisionMatrix
    \brief Encapsulates the decision matrix of a device
*/
class CDecisionMatrix  
{

public:

    /*! 
        Default Constructor
    */
    CDecisionMatrix( CMDF_DecisionMatrix *pdm, bool bIndexed = false );

    /*! 
        Default Destructor
    */
    ~CDecisionMatrix( void );

    /*!
        
    */
    bool loadMatrix();

    /*!
        Load a DM row
        @param row Row to load.
        @param pRow Pointer t array which must hold eight bytes and
                    will receive the row.
    */
    bool getRow( uint32_t row, uint8_t *pRow );
    

private:

    /*!
        True if the matrix is indexed. That is if it consist
        of one row precided by an index into the matrix.
    */
    bool m_bIndexedDM;

    /*!
        A memory array holding the full decision matrix
    */
    uint8_t *m_pdm;

    /*!
        Pointer to decsion matrix info from MDF
    */
    CMDF_DecisionMatrix *m_pmdfdm;
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

    void getMDF( wxString& remoteFile );

    const uint8_t *getGUID( void ) { return ( m_reg + 0xD0 - 0x80 ); };
    
    void getGUID( cguid *pguid ) { pguid->getFromArray( m_reg + 0xD0 - 0x80 ); };

    uint8_t getBufferSize( void ) { return m_reg[ 0x98 - 0x80 ]; };

    uint8_t getBootloaderAlgorithm( void ) { return m_reg[ 0x97 - 0x80 ]; };

    uint8_t getMajorVersion( void ) { return m_reg[ 0x94 - 0x80 ]; };

    uint8_t getMinorVersion( void ) { return m_reg[ 0x95 - 0x80 ]; };

    uint8_t getSubMinorVersion( void ) { return m_reg[ 0x96 - 0x80 ]; };

    wxString getFirmwareVersionString( void );

    uint16_t getPage( void ) { return ( m_reg[ 0x92 - 0x80 ] * 256 +
                                    m_reg[ 0x93 - 0x80 ] ); };

    uint8_t getNickname( void ) { return m_reg[ 0x91 - 0x80 ]; };

    uint8_t getAlarm( void ) { return m_reg[ 0x80 - 0x80 ]; };

    uint8_t getConfirmanceVersionMajor( void ) { return m_reg[ 0x81 - 0x80 ]; };

    uint8_t getConfirmanceVersonMinor( void ) { return m_reg[ 0x82 - 0x80 ]; };

    uint8_t getNodeControl( void ) { return m_reg[ 0x83 - 0x80 ]; };

    uint8_t getNumberOfRegisterPages( void ) { return m_reg[ 0x99 - 0x80 ]; };

    uint32_t getManufacturerSubDeviceID( void ) 
                                { return ( ( m_reg[ 0x8D - 0x80 ] << 24 ) +
                                            ( m_reg[ 0x8E - 0x80 ] << 16 ) +
                                            ( m_reg[ 0x8F - 0x80 ] << 8 ) +
                                            ( m_reg[ 0x90 - 0x80 ] ) ); };

    uint32_t getManufacturerDeviceID( void ) 
                                { return ( ( m_reg[ 0x89 - 0x80 ] << 24 ) +
                                            ( m_reg[ 0x8A - 0x80 ] << 16 ) +
                                            ( m_reg[ 0x8B - 0x80 ] << 8 ) +
                                            ( m_reg[ 0x8C - 0x80 ] ) ); };


    uint8_t getStandardReg( uint8_t reg );

    /*!
        Return a pointer to the register storage
    */
    unsigned char *getRegs( void ) { return m_reg; };

    /*!
        Set value for register
        @param reg Register to set value for
        @param val Value to set register to
    */
    void setReg( uint8_t reg, uint8_t val ) { m_reg[ reg ] = val; };


    /*!
        Get value for register
        @param reg Register to get value for
        @return Value of requested register.
    */
    uint8_t getReg( uint8_t reg ) { return m_reg[ reg ]; };

private:

    /// Standard register storage
    uint8_t m_reg[ 128 ];

};


///////////////////////////////////////////////////////////////////////////////


/*!
    \class CUserRegisters
    \brief Encapsulates the user registers of a device
*/
class CUserRegisters  
{

public:
    CUserRegisters();
    ~CUserRegisters();

    // Init the standard registers
    void init( wxArrayLong &pagesArray );

    /*!
        Return a pointer to the register storage
    */
    unsigned char *getRegs( void ) { return m_reg; };

    /*!
        Get array of pages
    */
    wxArrayLong &getArrayOfPages( void ) { return m_arrayPages; };

    /*!
        Get registers for a specific page
    */
    uint8_t *getRegs4Page( uint16_t page );

    /*!
        Get the value for a specific register on a page
    */
    uint8_t getValue( uint16_t page, uint8_t offset );


    /*!
        Set the value for a specific register on a page
    */
    uint8_t setValue( uint16_t page, uint8_t offset, uint8_t value );

    /*!
        Get abstraction value from registers into string value.
        @param abstraction Abstraction record from MDF.
        @param strValue Abstraction value in string form on return if call successful.
        @return true on success.
    */
    bool abstractionValueFromRegsToString( CMDF_Abstraction *pAbstraction, 
                                                wxString &strValue,
                                                uint8_t format = FORMAT_ABSTRACTION_DECIMAL  );
    
    /*
     * Store abstraction value in string format in corresponding registers.
     * @param abstraction Abstraction record from MDF.
     * @param strValue Abstraction value in string form.
     * @return true on success.
     */
    bool abstractionValueFromStringToRegs( CMDF_Abstraction *pAbstraction, 
                                                wxString &strValue );

private:

    /*! 
        All registers are here in chunks of 128 bytes
    */
    uint8_t *m_reg;  

    /// Size for the register arrays
    uint32_t m_size;

    // Array with valid register pages
    wxArrayLong m_arrayPages;
};


#endif
