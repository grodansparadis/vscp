// FILE: mdf.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, the VSCP project
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

#ifndef _MDF_H_
#define _MDF_H_

#include <canal.h>
#include <crc.h>
#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>

#include <expat.h>

#include <deque>
#include <map>
#include <set>
#include <string>

#define MDF_ACCESS_READ  1
#define MDF_ACCESS_WRITE 2

#define REGISTER_TYPE_STANDARD 0
#define REGISTER_TYPE_DMATRIX1 1
#define REGISTER_TYPE_BLOCK    2

enum vscp_level { level1 = 1, level2 };
enum vscp_access { access_read = 1, access_write };

enum vscp_remote_variable_type {
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

class CMDF_ValueListValue {

public:
  CMDF_ValueListValue();
  ~CMDF_ValueListValue();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  std::string m_strName;
  std::map<std::string, std::string> m_strDescription;
  std::map<std::string, std::string> m_strHelpType;
  std::map<std::string, std::string> m_strHelp; // Item help text or URL
  std::string m_strValue;                       // The abstraction tells the type
};

/*!
  CMDF_RemoteVariable
 */

class CMDF_RemoteVariable {

public:
  CMDF_RemoteVariable();
  ~CMDF_RemoteVariable();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear data storage
  */
  void clearStorage(void);

  /*!
      Get real text description of type
      @param type Remote variable type
      @return Real text description of type.
  */
  std::string getRemoteVariableValueType(void);

  /*!
      Get number of bytes for an remote variable type
      @return Number of bytes for remote variable type.
   */
  uint16_t getRemoteVariableTypeByteCount(void);

  std::string m_strName;
  std::map<std::string, std::string> m_strDescription;
  std::map<std::string, std::string> m_strHelpType;
  std::map<std::string, std::string> m_strHelp; // Item help text or URL

  std::string m_strID;      // Abstract variable id (unique
                            // inside of MDF
  std::string m_strDefault; // default value

  vscp_remote_variable_type m_nType; // One of the predefined types

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

  std::deque<CMDF_ValueListValue *> m_list_value; // list with selectable values
};

// * * * Register * * *

/*!
  CMDF_Bit

  This represents a bit or a bitfield. A valuelist can be
  defined to describe the bit combinations.

 */

class CMDF_Bit {

public:
  CMDF_Bit();
  ~CMDF_Bit();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  std::string m_strName;
  std::map<std::string, std::string> m_strDescription;
  std::map<std::string, std::string> m_strHelpType;
  std::map<std::string, std::string> m_strHelp; // Item help text or URL

  // The following is used if the remote variable is a bit or bit field
  uint8_t m_nPos;     // 'pos'      position in bit field 0-7 (from the left)
  uint8_t m_nWidth;   // 'width'    1 for one bit 2-8 for bit-field
  uint8_t m_nDefault; // 'default'  Default value for field

  uint8_t m_nAccess; // 'access'   Access rights for the bit(-field)

  std::deque<CMDF_ValueListValue *> m_list_value; // List with selectable values
};

/*!
  CMDF_Register

  Holds information for one register

 */

class CMDF_Register {

public:
  CMDF_Register();
  ~CMDF_Register();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

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
  std::map<std::string, std::string> m_strDescription;
  std::map<std::string, std::string> m_strHelpType;
  std::map<std::string, std::string> m_strHelp; // Item help text or url

  uint16_t m_nPage;
  uint16_t m_nOffset;
  uint16_t m_nWidth; // Defaults to 1

  uint8_t m_type; // std=0/dmatix1=1/block=2
  uint8_t m_size; // Size for special types (default = 1)

  uint32_t m_nMin;
  uint32_t m_nMax;

  std::string m_strDefault; // "UNDEF" if not set

  uint8_t m_nAccess;

  std::deque<CMDF_Bit *> m_list_bit;              // dll list with bit defines
  std::deque<CMDF_ValueListValue *> m_list_value; // dll list with selectable values

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

class CMDF_ActionParameter {

public:
  CMDF_ActionParameter();
  ~CMDF_ActionParameter();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  std::string m_strName;
  std::map<std::string, std::string> m_strDescription;
  std::map<std::string, std::string> m_strHelpType;
  std::map<std::string, std::string> m_strHelp; // Item help text or url

  uint16_t m_nOffset;
  uint8_t m_width;

  std::deque<CMDF_Bit *> m_list_bit;              // List with bit defines
  std::deque<CMDF_ValueListValue *> m_list_value; // List with selectable values
};

/*!
  CMDF_Action

  Holds information for one available action

 */

class CMDF_Action {

public:
  CMDF_Action();
  ~CMDF_Action();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  std::string m_strName;
  std::map<std::string, std::string> m_strDescription;
  std::map<std::string, std::string> m_strHelpType;
  std::map<std::string, std::string> m_strHelp; // Item help text or url

  uint16_t m_nCode;

  std::deque<CMDF_ActionParameter *> m_list_ActionParameter; // List with action parameters
};

/*!
  CMDF_DecisionMatrix

  Holds information about the capabilities of a system
  decision matrix.
 */

class CMDF_DecisionMatrix {

public:
  CMDF_DecisionMatrix();
  ~CMDF_DecisionMatrix();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

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

class CMDF_EventData {

public:
  CMDF_EventData();
  ~CMDF_EventData();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  std::string m_strName;
  std::map<std::string, std::string> m_strDescription;
  std::map<std::string, std::string> m_strHelpType;
  std::map<std::string, std::string> m_strHelp; // Item help text or url

  uint16_t m_nOffset;

  std::deque<CMDF_Bit *> m_list_bit;              // List with bit defines
  std::deque<CMDF_ValueListValue *> m_list_value; // List with selectable values
};

/*!
  CMDF_Event

  Holds one available event on the module

 */

class CMDF_Event {

public:
  CMDF_Event();
  ~CMDF_Event();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  std::string m_strName;
  std::map<std::string, std::string> m_strDescription;
  std::map<std::string, std::string> m_strHelpType;
  std::map<std::string, std::string> m_strHelp; // Item help text or url

  uint16_t m_nClass;
  uint16_t m_nType;
  uint8_t m_nPriority;

  std::deque<CMDF_EventData *> m_list_eventdata; // List with event data descriptions
};

/*!
  CMDF_Item

  Holds one item like a phone number, fax,
  email-address, web-address etc

 */

class CMDF_Item {

public:
  CMDF_Item();
  ~CMDF_Item();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  std::string m_strItem;
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapHelpType; // Type for item (if any)
  std::map<std::string, std::string> m_mapHelp;     // Item help text or url (if any)
};

/*!
  CMDF_BootLoaderInfo

  Holds information about the capabilities of a system
  decission matrix.
 */

class CMDF_BootLoaderInfo {

public:
  CMDF_BootLoaderInfo();
  ~CMDF_BootLoaderInfo();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

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

class CMDF_Address {

public:
  CMDF_Address();
  ~CMDF_Address();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

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

class CMDF_Manufacturer {

public:
  CMDF_Manufacturer();
  ~CMDF_Manufacturer();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  std::string m_strName;
  CMDF_Address m_address;

  std::deque<CMDF_Item *> m_list_Phone;
  std::deque<CMDF_Item *> m_list_Fax;
  std::deque<CMDF_Item *> m_list_Email;
  std::deque<CMDF_Item *> m_list_Web;
};

class CMDF_Picture {

public:
  CMDF_Picture();
  ~CMDF_Picture();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get URL for picture
    @return URL for picture
  */
  std::string getUrl(void) { return m_strURL; };

  /*!
    Set URL for picture
    @param strURL URL for picture
  */
  void setUrl(std::string &strURL) { m_strURL = strURL; };

  /*!
    Get format for picture
    @return format for picture
  */
  std::string getFormat(void) { return m_strFormat; };

  /*!
    Set format for picture
    @param strFormat Format for picture
  */
  void setFormat(std::string &strFormat) { m_strFormat = strFormat; };

  /*!
    Set description for picture
  */
  void setDescription(std::string &strDescription, std::string &strLanguage)
  {
    m_mapDescription[strLanguage] = strDescription;
  };

  /*!
    Get description for picture
    @param strLanguage Language for description (default="en")
    @return Description for picture as string
  */
  std::string getDescription(std::string strLanguage = "en")
  {
    if (m_mapDescription.find(strLanguage) != m_mapDescription.end()) {
      return m_mapDescription[strLanguage];
    }
    else {
      return "";
    }
  };

private:
  /*!
    Path to picture file
  */
  std::string m_strURL;

  /*!
    Picture type
  */
  std::string m_strFormat;

  /*!
      Description of file
  */
  std::map<std::string, std::string> m_mapDescription;
};

/*!
  CMDF_Firmware

  Holds information about one firmware of the module

 */

class CMDF_Firmware {

public:
  CMDF_Firmware();
  ~CMDF_Firmware();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get URL for firmware
    @return URL for firmware
  */
  std::string getUrl(void) { return m_strURL; };

  /*!
    Get target string for firmware
    @return Target string for firmware
  */
  std::string getTarget(void) { return m_strTarget; };

  /*!
    Get hex file string format for firmware
    @return Hex string format
  */
  std::string getFormat(void) { return m_strFormat; };

  /*!
    Get ISO date string for firmware
    @return ISO date string for firmware
  */
  std::string getDate(void) { return m_strDate; };

  /*!
    Get target code for firmware
    @return Target code for firmware
  */
  uint16_t getTargetCode(void) { return m_targetCode; };

  /*!
    Get version major for firmware
    @return Version major for firmware
  */
  uint16_t getVersionMajor(void) { return m_version_major; };

  /*!
    Get version minor for firmware
    @return Version minor for firmware
  */
  uint16_t getVersionMinor(void) { return m_version_minor; };

  /*!
    Get version patch for firmware
    @return Version patch for firmware
  */
  uint16_t getVersionPatch(void) { return m_version_patch; };

  /*!
    Get size for firmware
    @return Version patch for firmware
  */
  size_t getSize(void) { return m_size; };

  /*!
    Get MD5 for firmware file on hex string format
    @return md5 hash for firmware file on string format.
  */
  std::string getMd5(void) { return m_strMd5; };

  /*!
    Set description for picture
  */
  void setDescription(std::string &strDescription, std::string &strLanguage)
  {
    m_mapDescription[strLanguage] = strDescription;
  };

  /*!
    Get description for firmware
    @param strLanguage Language for description (default="en")
    @return Description for firmware as string
  */
  std::string getDescription(std::string strLanguage = "en")
  {
    if (m_mapDescription.find(strLanguage) != m_mapDescription.end()) {
      return m_mapDescription[strLanguage];
    }
    else {
      return "";
    }
  };

private:

  /*!
      Path/url to firmware hex file
  */
  std::string m_strURL;

  /*!
    Target system or processor for this firmware
  */
  std::string m_strTarget;

  /*!
    The target code is the same as the string
    target but coded to save space in device. This code
    can be used to make sure the firmware is compatible
    with the device.
  */
  uint16_t m_targetCode;

  /*!
    Format for firmware (intelhex8|intelhex16 etc)
  */
  std::string m_strFormat;

  /*!
    Date of firmware
  */
  std::string m_strDate;

  /*!
      Size for firmware file (not the binary image)
  */
  size_t m_size;

  /// Major version number
  uint16_t m_version_major;

  /// Minor version number
  uint16_t m_version_minor;

  /// Subminor version number
  uint16_t m_version_patch;

  /*!
    MD5 hash on hex string form for firmware file
  */
  std::string m_strMd5;

  /*!
      Description of file
  */
  std::map<std::string, std::string> m_mapDescription;

  /*!
      Help type
  */
  std::map<std::string, std::string> m_mapHelpType;

  /*!
      Pointer to help
  */
  std::map<std::string, std::string> m_mapHelp;
};


// ---------------------------------------------------------------------------


class CMDF_Manual {

public:
  CMDF_Manual();
  ~CMDF_Manual();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get URL for manual
    @return URL for manual
  */
  std::string getUrl(void) { return m_strURL; };

  /*!
    Get URL for firmware
    @return Manual format string.
  */
  std::string getFormat(void) { return m_strFormat; };

  /*!
    Get language
    @return Manual language string
  */
  std::string getLanguage(void) { return m_strLanguage; };

private:

  /*!
    URL for manual file
  */
  std::string m_strURL;

  /*!
    Format for manual / txt, html, pdf etc
  */
  std::string m_strFormat;

  /*!
    Language format for manual
  */
  std::string m_strLanguage;

  /*!
      Description of file
  */
  std::map<std::string, std::string> m_mapDescription;
};

/*!
  CMDF

  Top MDF object.

 */

class CMDF {

public:
  CMDF();
  ~CMDF();

  // Friend declarations
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

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

  /*
    Get device HTML status from device
    @param registers Array with all 256 registers for the device
    @param pmdf Optional pointer to CMDF class which gives more info
    about the device if it is supplied.
  */

  std::string vscp_getDeviceHtmlStatusInfo(const uint8_t *registers, CMDF *pmdf);

  /*!
    Get Module name in selected language.
    @param language Language to get name in.
    @return Return string with module name in selected language or
      in english if the language is not available.
  */
  std::string getModuleName(std::string language = "en");

  /*!
    Get the number of module names availabel in different languages
    @return Number of names available.
  */
  size_t getModuleNameSize(void) { return m_mapModule_Name.size(); };

  /*!
    Get Module description in selected language.
    @param language Language to get description in.
    @return Return string with module description in selected language or
      in english if the language is not available.
  */
  std::string getModuleDescription(std::string language = "en");

  /*!
    Get the number of module descriptions availabel in different languages
    @return Number of descriptions available.
  */
  size_t getModuleDescriptionSize(void) { return m_mapModule_Description.size(); };

  /*!
    Get Module info url in selected language.
    @param language Language for the help resource.
    @return Return string with module info url in selected language or
      in english if the language is not available.
  */
  std::string getModuleHelpUrl(std::string language = "en");

  /*!
    Get the number of module help URL's availabel in different mime types
    @return Number of info url's available.
  */
  size_t getModuleHelpUrlCount(void) { return m_mapModule_HelpURL.size(); };

  /*!
    Get module change date
    @return Return string with module change date.
  */
  std::string getModuleChangeDate(void) { return m_strModule_changeDate; };

  /*!
    Get module version
    @return Return string with module version.
  */
  std::string getModuleModel(void) { return m_strModule_Model; };

  /*!
    Get module version
    @return Return string with module version.
  */
  std::string getModuleVersion(void) { return m_strModule_Version; };

  /*!
    Get module buffer size
    @return Buffer size in bytes.
  */
  uint16_t getModuleBufferSize(void) { return m_module_bufferSize; };

  /*!
    Get module manufacturer
    @param index (with default 0) is the index of the manufacturer.
    @return Return string with module manufacturer.
  */
  std::string getManufacturerName(uint8_t index = 0);

  /*!
    Get manufacturer street address
    @param index (with default 0) is the index of the manufacturer.
    @return Return string with module manufacturer street address.
  */
  std::string getManufacturerStreetAddress(uint8_t index = 0);

  /*!
    Get manufacturer city address
    @param index (with default 0) is the index of the manufacturer.
    @return Return string with module manufacturer city address.
  */
  std::string getManufacturerCityAddress(uint8_t index = 0);

  /*!
    Get manufacturer town address
    @param index (with default 0) is the index of the manufacturer.
    @return Return string with module manufacturer town address.
  */
  std::string getManufacturerTownAddress(uint8_t index = 0);

  /*!
    Get manufacturer post code address
    @param index (with default 0) is the index of the manufacturer.
    @return Return string with module manufacturer post code address.
  */
  std::string getManufacturerPostCodeAddress(uint8_t index = 0);

  /*!
    Get manufacturer region address
    @param index (with default 0) is the index of the manufacturer.
    @return Return string with module manufacturer region address.
  */
  std::string getManufacturerRegionAddress(uint8_t index = 0);

  /*!
    Get manufacturer state address
    @param index (with default 0) is the index of the manufacturer.
    @return Return string with module manufacturer state address.
  */
  std::string getManufacturerStateAddress(uint8_t index = 0);

  /*!
    Get number of module pictures
    @return Number of pictures available.
  */
  size_t getPictureCount(void) { return m_list_picture.size(); };

  /*!
    Get a picture object from it's index
    @param index Index of picture to get.
    @return Pointer to picture object or NULL if index is out of range.
  */
  CMDF_Picture *getPictureObj(uint16_t index = 0)
  {
    return ((m_list_picture.size() <= index) ? nullptr : m_list_picture[index]);
  };

  /*!
    Get number of module firmware files
    @return Number of firmware files available.
  */
  size_t getFirmwareCount(void) { return m_list_firmware.size(); };

  /*!
    Get a firmware object from it's index
    @param index Index of picture to get.
    @return Pointer to firmware object or NULL if index is out of range.
  */
  CMDF_Firmware *getFirmwareObj(uint16_t index = 0)
  {
    return ((m_list_firmware.size() <= index) ? nullptr : m_list_firmware[index]);
  };

  /*!
    Get number of module manual files
    @return Number of manual files available.
  */
  size_t getManualCount(void) { return m_list_manual.size(); };

  /*!
    Get a manual object from it's index
    @param index Index of picture to get.
    @return Pointer to manual object or NULL if index is out of range.
  */
  CMDF_Manual *getManualObj(size_t index = 0)
  {
    return ((m_list_manual.size() <= index) ? nullptr : m_list_manual[index]);
  };

  // ------------------------------------------------------------------------

  /*!
    Parse XML formated MDF file
    @param ifs Open input file stream for the file to parse.
    @return returns VSCP_ERROR_SUCCESS on success, error code on failure.
  */
  int parseMDF_XML(std::ifstream &ifs);

  /*!
    Parse JSON formated MDF file
    @param path Path to the JSON file to parse.
    @return returns VSCP_ERROR_SUCCESS on success, error code on failure.
  */
  int parseMDF_JSON(std::string &path);

  /*!
      Parse a MDF. The format can be XML or JSON. If the format is
      unknown, the function will try to determine it.
      @param path Path to downloaded MDF
      @return returns VSCP_ERROR_SUCCESS on success, error code on failure.
  */
  int parseMDF(std::string &path);

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

  // --------------------------------------------------------------------------

private:
  std::string m_strLocale; // ISO code for requested language
                           // defaults to "en"

  std::string m_tempFileName; // Local downloaded file path

  std::string m_strURL; // Location for MDF file

  // Redirect URL if MDF with real content is located elsewhere.
  std::string m_redirectUrl;

  // Language specific information. "en" is default, but any variant
  // if ISO two letter language code can be used. Key is always lower
  // case.
  std::map<std::string, std::string> m_mapModule_Name;        // Module name
  std::map<std::string, std::string> m_mapModule_Description; // Module description
  std::map<std::string, std::string> m_mapModule_HelpURL;     // URL for full module information

  std::string m_strModule_changeDate; // Last date changed
  std::string m_strModule_Model;      // Module Model
  std::string m_strModule_Version;    // Module version

  uint16_t m_module_bufferSize; // Buffersize for module

  CMDF_Manufacturer m_manufacturer; // Manufacturer information

  // File lists
  std::deque<CMDF_Picture *> m_list_picture;   // Picture file(s)
  std::deque<CMDF_Firmware *> m_list_firmware; // Firmware file(s)
  std::deque<CMDF_Manual *> m_list_manual;     // Manual file(s)

  CMDF_DecisionMatrix m_dmInfo;   // Info about decision matrix
  CMDF_BootLoaderInfo m_bootInfo; // Boot loader info

  std::deque<CMDF_Event *> m_list_event;              // Events this node can generate
  std::deque<CMDF_Register *> m_list_register;        // List with defined registers
  std::deque<CMDF_RemoteVariable *> m_list_remotevar; // List with defined remote variables
  std::deque<CMDF_Bit *> m_list_alarmbits;            // List with alarm bit defines
};

#endif
