// FILE: mdf.h
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
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

#ifndef _MDF_H_
#define _MDF_H_

#include <canal.h>
#include <crc.h>
#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>

#define MDF_ACCESS_READ 1
#define MDF_ACCESS_WRITE 2

#define REGISTER_TYPE_STANDARD 0
#define REGISTER_TYPE_DMATRIX1 1
#define REGISTER_TYPE_BLOCK 2

enum vscp_level
{
    level1 = 1,
    level2
};

enum vscp_abstraction_type
{
    type_unknown = 0,
    type_string,
    type_bitfield,
    type_boolval,
    type_int8_t,
    type_uint8_t,
    type_int16_t,
    type_uint16_t,
    type_int32_t,
    type_uint32_t,
    type_int64_t,
    type_uint64_t,
    type_float,
    type_double,
    type_date,
    type_time,
    type_guid,
    type_index8_int16_t,
    type_index8_uint16_t,
    type_index8_int32_t,
    type_index8_uint32_t,
    type_index8_int64_t,
    type_index8_uint64_t,
    type_index8_float,
    type_index8_double,
    type_index8_date,
    type_index8_time,
    type_index8_guid,
    type_index8_string
};

// * * * Settings * * *

/*!
  CMDF_ValueListValue

 */

class CMDF_ValueListValue
{

  public:
    CMDF_ValueListValue();
    ~CMDF_ValueListValue();

    std::string m_strName;
    std::string m_strDescription;
    std::string m_strHelpType;
    std::string m_strHelp;  // Item help text or URL
    std::string m_strValue; // The abstraction tells the type
};

/*!
  CMDF_Abstraction

 */

class CMDF_Abstraction
{

  public:
    CMDF_Abstraction();
    ~CMDF_Abstraction();

    /*!
        Clear data storage
    */
    void clearStorage(void);

    /*!
        Get real text description of type
        @param type Abstraction type
        @return Real text description of type.
    */
    std::string getAbstractionValueType(void);

    /*!
        Get number of bytes for an abstraction type
        @return Number of bytes for abstraction type.
     */
    uint16_t getAbstractionTypeByteCount(void);

    std::string m_strName;
    std::string m_strDescription;
    std::string m_strHelpType;
    std::string m_strHelp; // Item help text or URL

    std::string m_strID;      // Abstract variable id (unique
                              // inside of MDF
    std::string m_strDefault; // default value

    vscp_abstraction_type m_nType; // One of the predefined types

    uint16_t m_nPage;      // stored on this page
    uint32_t m_nOffset;    // stored at this offset
    uint16_t m_nBitnumber; // Stored at this bit position.

    uint16_t m_nWidth; // Width for bit field and strings.

    uint32_t m_nMax; // If numeric max value can be set
    uint32_t m_nMin; // If numeric min value can be set

    uint8_t m_nAccess; // Access rights

    bool m_bIndexed; // True of indexed storage

    uint32_t m_bgcolor; // Cell background colour. Default = white.
    uint32_t m_fgcolor; // Cell foreground colour. Default = black.

    std::deque<CMDF_ValueListValue *>
      m_list_value; // list with selectable values
};

// * * * Register * * *

/*!
  CMDF_Bit

  This represents a bit or a bitfield. A valuelist can be
  defined to describe the bit combinations.

 */

class CMDF_Bit
{

  public:
    CMDF_Bit();
    ~CMDF_Bit();

    std::string m_strName;
    std::string m_strDescription;

    /*!
        Clear storage
    */
    void clearStorage(void);

    std::string m_strHelpType;
    std::string m_strHelp; // Item help text or URL

    // The following is used if the abstraction is a bit or bit field
    uint8_t m_nPos;     // 'pos'      position in bit field 0-7 (from the left)
    uint8_t m_nWidth;   // 'width'    1 for one bit 2-8 for bit-field
    uint8_t m_nDefault; // 'default'  Default value for field

    uint8_t m_nAccess; // 'access'   Access rights for the bit(-field)

    std::deque<CMDF_ValueListValue *>
      m_list_value; // List with selectable values
};

/*!
  CMDF_Register

  Holds information for one register

 */

class CMDF_Register
{

  public:
    CMDF_Register();
    ~CMDF_Register();

    /*!
        Clear storage
    */
    void clearStorage(void);

    /*!
     Set default for value if it is defined.
     */
    uint8_t setDefault(void);

    /*!
        Assignment
    */
    CMDF_Register &operator=(const CMDF_Register &other);

    std::string m_strName;
    std::string m_strDescription;
    std::string m_strHelpType;
    std::string m_strHelp; // Item help text or url

    uint16_t m_nPage;
    uint16_t m_nOffset;
    uint16_t m_nWidth; // Defaults to 1

    uint8_t m_type; // std=0/dmatix1=1/block=2
    uint8_t m_size; // Size for special types (default = 1)

    uint32_t m_nMin;
    uint32_t m_nMax;

    std::string m_strDefault; // "UNDEF" if not set

    uint8_t m_nAccess;

    std::deque<CMDF_Bit *> m_list_bit; // dll list with bit defines
    std::deque<CMDF_ValueListValue *>
      m_list_value; // dll list with selectable values

    // For VSCP Works
    long m_rowInGrid;   // Helper for display (row reg is displayed on)
    uint8_t m_value;    // Initial value read. This is the value
                        // that will be restored.
    uint32_t m_bgcolor; // Cell background colour. Default = white.
    uint32_t m_fgcolor; // Cell foreground colour. Default = black.
};

/*!
  CMDF_ActionParameter

  Holds information for one action parameter

 */

class CMDF_ActionParameter
{

  public:
    CMDF_ActionParameter();
    ~CMDF_ActionParameter();

    /*!
        Clear storage
    */
    void clearStorage(void);

    std::string m_strName;
    std::string m_strDescription;
    std::string m_strHelpType;
    std::string m_strHelp; // Item help text or url

    uint16_t m_nOffset;
    uint8_t m_width;

    std::deque<CMDF_Bit *> m_list_bit; // List with bit defines
    std::deque<CMDF_ValueListValue *>
      m_list_value; // List with selectable values
};

/*!
  CMDF_Action

  Holds information for one available action

 */

class CMDF_Action
{

  public:
    CMDF_Action();
    ~CMDF_Action();

    /*!
        Clear storage
    */
    void clearStorage(void);

    std::string m_strName;
    std::string m_strDescription;
    std::string m_strHelpType;
    std::string m_strHelp; // Item help text or url

    uint16_t m_nCode;

    std::deque<CMDF_ActionParameter *>
      m_list_ActionParameter; // List with action parameters
};

/*!
  CMDF_DecisionMatrix

  Holds information about the capabilities of a system
  decision matrix.
 */

class CMDF_DecisionMatrix
{

  public:
    CMDF_DecisionMatrix();
    ~CMDF_DecisionMatrix();

    /*!
        Clear storage
    */
    void clearStorage(void);

    uint8_t m_nLevel;        // 1 or 2 (defaults to 1)
    uint16_t m_nStartPage;   // Page where DM starts
    uint16_t m_nStartOffset; // Offset on start page for DM
    uint16_t m_nRowCount;    // Number of rows in DM
    uint16_t m_nRowSize;     // Size of a DM row (Normally 8)
    bool m_bIndexed;         // True of storage is indexed

    std::deque<CMDF_Action *> m_list_action; // Action description
};

/*!
  CMDF_EventData

  Holds info about one event data item

 */

class CMDF_EventData
{

  public:
    CMDF_EventData();
    ~CMDF_EventData();

    /*!
        Clear storage
    */
    void clearStorage(void);

    std::string m_strName;
    std::string m_strDescription;
    std::string m_strHelpType;
    std::string m_strHelp; // Item help text or url

    uint16_t m_nOffset;

    std::deque<CMDF_Bit *> m_list_bit; // List with bit defines
    std::deque<CMDF_ValueListValue *>
      m_list_value; // List with selectable values
};

/*!
  CMDF_Event

  Holds one available event on the module

 */

class CMDF_Event
{

  public:
    CMDF_Event();
    ~CMDF_Event();

    /*!
        Clear storage
    */
    void clearStorage(void);

    std::string m_strName;
    std::string m_strDescription;
    std::string m_strHelpType;
    std::string m_strHelp; // Item help text or url

    uint16_t m_nClass;
    uint16_t m_nType;
    uint8_t m_nPriority;

    std::deque<CMDF_EventData *>
      m_list_eventdata; // List with event data descriptions
};

/*!
  CMDF_Item

  Holds one item like a phone number, fax,
  email-address, web-address etc

 */

class CMDF_Item
{

  public:
    CMDF_Item();
    ~CMDF_Item();

    std::string m_strItem;
    std::string m_strDescription;
    std::string m_strHelpType;
    std::string m_strHelp; // Item help text or url
};

/*!
  CMDF_BootLoaderInfo

  Holds information about the capabilities of a system
  decission matrix.
 */

class CMDF_BootLoaderInfo
{

  public:
    CMDF_BootLoaderInfo();
    ~CMDF_BootLoaderInfo();

    /*!
        Clear storage
    */
    void clearStorage(void);

    uint8_t m_nAlgorithm;   // Bootloader algorithm used by device
    uint32_t m_nBlockSize;  // Size for one boot block
    uint32_t m_nBlockCount; // Number of boot blocks
};

/*!
  CMDF_Address

  Holds one address for the manufacturer

 */

class CMDF_Address
{

  public:
    CMDF_Address();
    ~CMDF_Address();

    /*!
        Clear storage
    */
    void clearStorage(void);

    std::string m_strStreet;
    std::string m_strTown;
    std::string m_strCity;
    std::string m_strPostCode;
    std::string m_strState;
    std::string m_strRegion;
    std::string m_strCountry;
};

/*!
  CMDF_Manufacturer

  Holds information about one manufacturer of the module

 */

class CMDF_Manufacturer
{

  public:
    CMDF_Manufacturer();
    ~CMDF_Manufacturer();

    /*!
        Clear storage
    */
    void clearStorage(void);

    std::string m_strName;

    std::deque<CMDF_Address *> m_list_Address;

    std::deque<CMDF_Item *> m_list_Phone;
    std::deque<CMDF_Item *> m_list_Fax;
    std::deque<CMDF_Item *> m_list_Email;
    std::deque<CMDF_Item *> m_list_Web;
};

/*!
  CMDF_Manufacturer

  Holds information about one manufacturer of the module

 */

class CMDF_Firmware
{

  public:
    CMDF_Firmware();
    ~CMDF_Firmware();

    /*!
        Clear storage
    */
    void clearStorage(void);

    /*!
        Path to firmware hex file
    */
    std::string m_strPath;

    /*!
        Size for firmware file (not the image)
    */
    uint32_t m_size;

    /// Major version number
    uint8_t m_version_major;

    /// Minor version number
    uint8_t m_version_minor;

    /// Subminor version number
    uint8_t m_version_subminor;

    /*!
        Description of file
    */
    std::string m_description;
};

/*!
  CMDF

  Top MDF object.

 */

class CMDF
{

  public:
    CMDF();
    ~CMDF();

    /*!
        Clear storage
    */
    void clearStorage(void);

    /*!
        Download MDF file from a server
        @param remoteFile remote file URL
        @param variable that will receive temporary filename for downloaded
       file.
        @return Return true if a valid file is downloaded.
    */
    bool downLoadMDF(std::string &remoteFile, std::string &tempFile);

    /*!
        Load MDF from local or remote storage and parse it into
        a MDF structure.
        @param Filename or URL to MDF file.
        @param blocalFile Asks for a local file if set to true.
        @return returns true on success, false on failure.
    */
    bool load(std::string &remoteFile, bool bLocalFile = false);

    /*!
        Format an MDF description so it can be shown
        @param str String to format.
    */
    void mdfDescriptionFormat(std::string &str);

    /*!
        Parse a MDF
        @param path Path to downloaded MDF
        @return true if the parsing went well.
    */
    bool parseMDF(std::string &path);

    // Helpers

    /*!
        Get number of defined registers
        @param page Register page to check
        @return Number of registers used.
    */
    uint32_t getNumberOfRegisters(uint32_t page);

    /*!
        Get number of register pages used
        @param array Reference to array with pages
        @return Number of register pages used.
    */
    uint32_t getPages(std::set<long> &pages);

    /*!
        Return register class from register + page
        @param register Register to search for.
        @param page Page top search for.
        @return Pointer to CMDF_Register class if found else NULL.
    */
    CMDF_Register *getMDFRegister(uint8_t reg, uint16_t page);

    /*
        Return temporary file path
        @return path
    */
    std::string getTempFilePath(void) { return m_tempFileName; };

    std::string m_strLocale; // ISO code for requested language
                             // defaults to "en"

    std::string m_tempFileName; // Local downloaded file path

    std::string m_strModule_Name;        // Module name
    std::string m_strModule_Model;       // Module Model
    std::string m_strModule_Version;     // Module version
    std::string m_strModule_Description; // Module description
    std::string m_strModule_InfoURL;     // URL for full module information
    std::string m_changeDate;            // Last date changed

    std::string m_strURL; // Location for MDF file

    uint16_t m_Module_buffersize; // Buffersize for module

    CMDF_Firmware m_firmware; // Firmware file

    std::deque<CMDF_Manufacturer *>
      m_list_manufacturer; // Manufacturer information

    CMDF_DecisionMatrix m_dmInfo;   // Info about decision matrix
    CMDF_BootLoaderInfo m_bootInfo; // Boot loader info

    std::deque<CMDF_Event *> m_list_event; // Events this node can generate
    std::deque<CMDF_Register *> m_list_register; // List with defined registers
    std::deque<CMDF_Abstraction *>
      m_list_abstraction;                    // List with defined abstractions
    std::deque<CMDF_Bit *> m_list_alarmbits; // List with alarm bit defines
};

#endif
