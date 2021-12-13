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

#include <json.hpp> // Needs C++11  -std=c++11

#include <curl/curl.h>

// https://github.com/nlohmann/json
using json = nlohmann::json;

// This is the access rights
typedef enum mdf_access_mode {
  MDF_REG_ACCESS_NONE       = 0,
  MDF_REG_ACCESS_READ_ONLY  = 1,
  MDF_REG_ACCESS_WRITE_ONLY = 2,
  MDF_REG_ACCESS_READ_WRITE = 3
} mdf_access_mode;

// This is the register type codes
typedef enum mdf_register_type {
  MDF_REG_TYPE_STANDARD = 0,
  MDF_REG_TYPE_DMATRIX1 = 1,
  MDF_REG_TYPE_BLOCK    = 2
} mdf_register_type;

// This is the event direction
typedef enum mdf_event_direction { MDF_EVENT_DIR_IN = 0, MDF_EVENT_DIR_OUT } mdf_event_direction;

// This is the remote variable type
typedef enum vscp_remote_variable_type {
  remote_variable_type_unknown = 0,
  remote_variable_type_string,
  remote_variable_type_boolean,
  remote_variable_type_int8_t,
  remote_variable_type_uint8_t,
  remote_variable_type_int16_t,
  remote_variable_type_uint16_t,
  remote_variable_type_int32_t,
  remote_variable_type_uint32_t,
  remote_variable_type_int64_t,
  remote_variable_type_uint64_t,
  remote_variable_type_float,
  remote_variable_type_double,
  remote_variable_type_date,
  remote_variable_type_time
} vscp_remote_variable_type;

// * * * Settings * * *

// Forward declarations
class CMDF;

// ----------------------------------------------------------------------------

/*!
  CMDF_ValueListValue
 */

class CMDF_ValueListValue {

public:
  CMDF_ValueListValue();
  ~CMDF_ValueListValue();

  // Friend declarations
  friend CMDF;
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
    Get the value name
    @return Value name
  */
  std::string getName(void) { return m_name; };

  /*!
    Set value name
    @param name Value name to set
  */
  void setName(std::string &name) { m_name = name; };

  /*!
    Get value
    @return value .
  */
  std::string getValue(void) { return m_strValue; };

  /*!
    Set value
    @param value Value on string form.
  */
  void setValue(std::string value) { m_strValue = value; };

  /*!
    Set value
    @param value Value on numeric form.
  */
  void setValue(uint32_t value) { m_strValue = std::to_string(value); };

  /*!
    Set value
    @param value Value on numeric form.
  */
  void setValue(double value) { m_strValue = std::to_string(value); };

  /*!
    Get the value description
    @return Value description
  */
  std::string getDescription(std::string lang) { return m_mapDescription[lang]; };

  /*!
    Set value description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(std::string lang, std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the value info URL
    @return Value info URL
  */
  std::string getInfoURL(std::string lang) { return m_mapInfoURL[lang]; };

  /*!
    Set value info URL
    @param lang Language
    @param url Value info URL to set
  */
  void setInfoURL(std::string lang, std::string &url) { m_mapInfoURL[lang] = url; };

private:
  std::string m_name;
  std::string m_strValue; // String because used for remote variabels also
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapInfoURL; // Item help text or URL
};

// ----------------------------------------------------------------------------

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
  friend CMDF;
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get bit array name
    @return Bit array name
  */
  std::string getName(void) { return m_name; };

  /*!
    Get the register description
    @return Register description
  */
  std::string getDescription(std::string lang) { return m_mapDescription[lang]; };

  /*!
    Set register description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(std::string &lang, std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the register info URL
    @return Register info URL
  */
  std::string getInfoURL(std::string lang) { return m_mapInfoURL[lang]; };

  /*!
    Set register info URL
    @param lang Language
    @param url Register info URL to set
  */
  void setInfoURL(std::string &lang, std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get bit array start position (0-7)
    @return Bit array start position
  */
  uint8_t getPos(void) { return (m_pos & 7); };

  /*!
    Set bit array start position (0-7)
    @param pos Bit array start position
  */
  void setPos(uint8_t pos) { m_pos = (pos & 7); };

  /*!
    Get bit array length (1-8)
    @return Bit array length
  */
  uint8_t getWidth(void) { return m_width; };

  /*!
    Set bit array length (1-8)
    @param width Bit array length
  */
  void setWidth(uint8_t width) { m_width = width; };

  /*!
    Get bit array value
    @return Bit array value
  */
  uint8_t getDefault(void) { return m_default; };

  /*!
    Set bit array value
    @param default Bit array value
  */
  void setDefault(uint8_t default_) { m_default = default_; };

  /*!
    Get bit array value
    @return Bit array value
  */
  uint8_t getMin(void) { return m_min; };

  /*!
    Set bit array value
    @param min Bit array value
  */
  void setMin(uint8_t min) { m_min = min; };

  /*!
    Get bit array value
    @return Bit array value
  */
  uint8_t getMax(void) { return m_max; };

  /*!
    Set bit array value
    @param max Bit array value
  */
  void setMax(uint8_t max) { m_max = max; };

  /*!
    Get access for bitfield
    @return Access for bitfield.
  */
  mdf_access_mode getAccess(void) { return m_access; };

  /*!
    Set access for bitfield
    @param access Access for bitfield.
  */
  void setAccess(mdf_access_mode access) { m_access = access; };

  /*!
    Fetch the value definition list
    @return Value definition list
  */
  std::deque<CMDF_ValueListValue *> *getListValues(void) { return &m_list_value; };

private:
  std::string m_name;
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapInfoURL; // Item help text or URL

  // The following is used if the remote variable is a bit or bit field
  uint8_t m_pos;            // 'pos'      position in bit field 0-7 (from the left (high))
  uint8_t m_width;          // 'width'    1 (default) for one bit 2-8 for bit-field
  uint8_t m_default;        // 'default'  Default value for field
  uint8_t m_min;            // 'min'      Minimum value for field (if applicable)
  uint8_t m_max;            // 'max'      Maximum value for field (if applicable)
  mdf_access_mode m_access; // 'access'   Access rights for the bit(-field)

  std::deque<CMDF_ValueListValue *> m_list_value; // List with selectable values
};

// ----------------------------------------------------------------------------

/*!
  CMDF_Register

  Holds information for one register
 */

class CMDF_Register {

public:
  CMDF_Register();
  ~CMDF_Register();

  // Friend declarations
  friend CMDF;
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get the register name
    @return Register name
  */
  std::string &getName(void) { return m_name; };

  /*!
    Set register name
    @param name Register name to set
  */
  void setName(std::string &name) { m_name = name; };

  /*!
    Get the register description
    @return Register description
  */
  std::string getDescription(std::string lang) { return m_mapDescription[lang]; };

  /*!
    Set register description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(std::string &lang, std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the register info URL
    @return Register info URL
  */
  std::string getInfoURL(std::string lang) { return m_mapInfoURL[lang]; };

  /*!
    Set register info URL
    @param lang Language
    @param url Register info URL to set
  */
  void setInfoURL(std::string &lang, std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get register default string
    @return Register default string. This string is set to "UNDEF"
            if no default is set.
  */
  std::string &getDefault(void) { return m_strDefault; };

  /*!
    Get register default
    @param val Pointer to byte that will get default value if defined.
    @return true if default value is defined. If so return the default
            value in default_value.
  */
  bool getDefault(uint8_t &default_value);

  /*!
    Set register default
    @param str Default string to set
  */
  void setDefault(std::string &str) { m_strDefault = str; };

  /*!
    Set register default
    @param val Default value to set
  */
  void setDefault(uint8_t val) { m_strDefault = std::to_string(val); };

  /*!
    Set default for register value if it is defined.
    @return New default value.
  */
  uint8_t setValueToDefault(void);

  /*!
    Get minimum for value.
    @return Minimum value.
  */
  uint8_t getMin(void) { return m_min; };

  /*!
    Set minimum for value.
    @param min Minimum value to set.
  */
  void setMin(uint8_t min) { m_min = min; };

  /*!
    Get maximum for value.
    @return Maximum value.
  */
  uint8_t getMax(void) { return m_max; };

  /*!
    Set maximum for value.
    @param max Maximum value to set.
  */
  void setMax(uint8_t max) { m_max = max; };

  /*!
    Get offset for register
    @return Offset for register.
  */
  uint32_t getOffset(void) { return m_offset; };

  /*!
    Set offset for register
    @param offset Offset for register.
  */
  void setOffset(uint32_t offset) { m_offset = offset; };

  /*!
    Get page for register
    @return Page for register.
  */
  uint32_t getPage(void) { return m_page; };

  /*!
    Set page for register
    @param page Page for register.
  */
  void setPage(uint32_t page) { m_page = page; };

  /*!
    Get span for register
    @return Span for register.
  */
  uint32_t getSpan(void) { return m_span; };

  /*!
    Set span for register
    @param span Span for register.
  */
  void setSpan(uint32_t span) { m_span = span; };

  /*!
    Get width for register
    @return Width for register.
  */
  uint32_t getWidth(void) { return m_width; };

  /*!
    Set width for register
    @param width Width for register.
  */
  void setWidth(uint32_t width) { m_width = width; };

  /*!
    Get size for register
    @return Size for register.
  */
  uint32_t getSize(void) { return m_size; };

  /*!
    Set size for register
    @param size Size for register.
  */
  void setSize(uint32_t size) { m_size = size; };

  /*!
    Get access for register
    @return Access for register.
  */
  mdf_access_mode getAccess(void) { return m_access; };

  /*!
    Set access for register
    @param access Access for register.
  */
  void setAccess(mdf_access_mode access) { m_access = access; };

  /*!
    Set type for register
    @param type Type for register.
  */
  void setType(mdf_register_type type) { m_type = type; };

  /*!
    Get type for register
    @return Type for register.
  */
  mdf_register_type getType(void) { return m_type; };

  /*!
    Fetch the bit definition list
    @return Bit definition list
  */
  std::deque<CMDF_Bit *> *getListBits(void) { return &m_list_bit; };

  /*!
    Fetch the value definition list
    @return Value definition list
  */
  std::deque<CMDF_ValueListValue *> *getListValues(void) { return &m_list_value; };

  // * * * VSCP Works Special methods * * *

  /*!
    Get value for register
    @return value for register.
  */
  uint8_t getValue(void) { return m_value; };

  /*!
    Set value for register
    @param value Value for register.
  */
  void setValue(uint8_t value);

  /*!
    Get VSCP Works grid position.
    @return VSCP Works grid postion. Set to -1 if not set.
  */
  long getRowPosition(void) { return m_rowInGrid; };

  /*!
    Set VSCP Works grid position.
    @param rowInGrid VSCP Works grid postion.
  */
  void setRowPosition(long rowInGrid) { m_rowInGrid = rowInGrid; };

  /*!
    Set Foreground color for VSCP Works grid.
    @param color Foreground color to set.
  */
  void setForegroundColor(uint32_t color) { m_fgcolor = color; };

  /*!
    Get Foreground color for VSCP Works grid.
    @return Fourground color.
  */
  uint32_t getForegroundColor(void) { return m_fgcolor; };

  /*!
    Set Background color for VSCP Works grid.
    @param color Background color to set.
  */
  void setBackgroundColor(uint32_t color) { m_bgcolor = color; };

  /*!
    Get Background color for VSCP Works grid.
    @return Background color.
  */
  uint32_t getBackgroundColor(void) { return m_bgcolor; };

  /*!
    Undo command on register value in VSCP grid.
    @return Previous register value. -1 if there is no previous value.
  */
  int undo(void);

  /*!
    Redo command on register value in VSCP grid.
    @return Previous register value. -1 if there is no previous value.
  */
  int redo(void);

private:
  /*!
      Assignment
  */
  CMDF_Register &operator=(const CMDF_Register &other);

  std::string m_name;
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapInfoURL; // Url that contain extra help information

  uint16_t m_page;   // Level 1 page is 8-bit, pageing is not used for Level II.
  uint32_t m_offset; // Level 1 offset is 7 bits, Level II offset is 32 bits.
  uint16_t m_span;   // Defaults to 1. Number of bytes for a group of registers
  uint16_t m_width;  // Defaults to 8. Width in bits for register (1-8 bits)

  mdf_register_type m_type; // std=0/dmatix1=1/block=2
  uint8_t m_size;           // Size for special types (default = 1)

  uint32_t m_min; // Defaults to 0
  uint32_t m_max; // Defaults to 255

  std::string m_strDefault; // "UNDEF" if not set

  mdf_access_mode m_access;

  std::deque<CMDF_Bit *> m_list_bit;              // List with bit defines
  std::deque<CMDF_ValueListValue *> m_list_value; // List with selectable values

  // Below are for VSCP Works use only
  long m_rowInGrid;   // Helper for display (row reg is displayed on)
  uint8_t m_value;    // Initial value read. This is the value
                      // that will be restored.
  uint32_t m_fgcolor; // Cell foreground colour. Default = black.
  uint32_t m_bgcolor; // Cell background colour. Default = white.

  std::deque<uint8_t> m_list_undo_value; // List with undo values
  std::deque<uint8_t> m_list_redo_value; // List with redo values
};

// ----------------------------------------------------------------------------

/*!
  CMDF_RemoteVariable
 */

class CMDF_RemoteVariable {

public:
  CMDF_RemoteVariable();
  ~CMDF_RemoteVariable();

  // Friend declarations
  friend CMDF;
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear data storage
  */
  void clearStorage(void);

  /*!
      Fetch the name of the remote variable
      @return Name of the remote variable
  */
  std::string getName(void) { return m_name; };

  /*!
    Set name of the remote variable
    @param name Name of the remote variable
  */
  void setName(std::string name) { m_name = name; };

  /*!
    Get the register description
    @return Register description
  */
  std::string getDescription(std::string lang) { return m_mapDescription[lang]; };

  /*!
    Set register description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(std::string &lang, std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the register info URL
    @return Register info URL
  */
  std::string getInfoURL(std::string lang) { return m_mapInfoURL[lang]; };

  /*!
    Set register info URL
    @param lang Language
    @param url Register info URL to set
  */
  void setInfoURL(std::string &lang, std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get the type for the remote variable
    @return Type for the remote variable.
  */
  vscp_remote_variable_type getType(void) { return m_type; };

  /*!
    Set type for remote variable
    @param type Type for remote variable.
  */
  void setType(vscp_remote_variable_type type) { m_type = type; };

  /*!
      Get real text description of type
      @param type Remote variable type
      @return Real text description of type.
  */
  std::string getTypeString(void);

  /*!
      Get number of bytes for an remote variable type
      @return Number of bytes for remote variable type.
   */
  uint16_t getTypeByteCount(void);

  /*!
      Get default value for remote variable
      @return Default value.
  */
  std::string getDefault(void) { return m_strDefault; };

  /*!
    Set default value for remote variable
    @param strDefault Default value.
  */
  void setDefault(std::string strDefault) { m_strDefault = strDefault; };

  /*!
    Get Page for remote variable
    @return Page for remote variable.
  */
  uint16_t getPage(void) { return m_page; };

  /*!
    Set Page for remote variable
    @param page Page for remote variable.
  */
  void setPage(uint16_t page) { m_page = page; };

  /*!
    Get Offset for remote variable
    @return Offset for remote variable.
  */
  uint32_t getOffset(void) { return m_offset; };

  /*!
    Set Offset for remote variable
    @param offset Offset for remote variable.
  */
  void setOffset(uint32_t offset) { m_offset = offset; };

  /*!
      Get access mode for remote variable
      @return Access mode for remote variable.
  */
  mdf_access_mode getAccess(void) { return m_access; };

  /*!
    Set access mode for remote variable
    @param access Access mode for remote variable.
  */
  void setAccess(mdf_access_mode access) { m_access = access; };

  /*!
    Get value list for remote variable
    @return Value list for remote variable.
  */
  std::deque<CMDF_ValueListValue *> *getListValues(void) { return &m_list_value; };

  /*!
    Get bit list for remote variable
    @return Bit list for remote variable.
  */
  std::deque<CMDF_Bit *> *getListBits(void) { return &m_list_bit; };

  /*!
    Get VSCP Works grid position.
    @return VSCP Works grid postion. Set to -1 if not set.
  */
  long getRowPosition(void) { return m_rowInGrid; };

  /*!
    Set VSCP Works grid position.
    @param rowInGrid VSCP Works grid postion.
  */
  void setRowPosition(long rowInGrid) { m_rowInGrid = rowInGrid; };

  /*!
    Set Foreground color for VSCP Works grid.
    @param color Foreground color to set.
  */
  void setForegroundColor(uint32_t color) { m_fgcolor = color; };

  /*!
    Get Foreground color for VSCP Works grid.
    @return Fourground color.
  */
  uint32_t getForegroundColor(void) { return m_fgcolor; };

  /*!
    Set Background color for VSCP Works grid.
    @param color Background color to set.
  */
  void setBackgroundColor(uint32_t color) { m_bgcolor = color; };

  /*!
    Get Background color for VSCP Works grid.
    @return Background color.
  */
  uint32_t getBackgroundColor(void) { return m_bgcolor; };

private:
  std::string m_name; // Abstract variable name (unique
                      // inside of MDF
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapInfoURL; // Item help text or URL

  std::string m_strDefault;         // default value
  vscp_remote_variable_type m_type; // One of the predefined types

  uint16_t m_page;          // stored on this page
  uint32_t m_offset;        // stored at this offset
  int8_t m_bitpos;          // For booleans (can be a single bit in a byte).
  uint16_t m_size;          // Size of string.
  mdf_access_mode m_access; // Access rights

  // For VSCP Works usage

  long m_rowInGrid;   // Helper for display (row reg is displayed on)
  uint32_t m_bgcolor; // Cell background colour. Default = white.
  uint32_t m_fgcolor; // Cell foreground colour. Default = black.

  std::deque<CMDF_Bit *> m_list_bit;              // List with bit defines
  std::deque<CMDF_ValueListValue *> m_list_value; // List with selectable values
};

// ----------------------------------------------------------------------------

/*!
  CMDF_ActionParameter

  Holds information for one action parameter

 */

class CMDF_ActionParameter {

public:
  CMDF_ActionParameter();
  ~CMDF_ActionParameter();

  // Friend declarations
  friend CMDF;
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get bit array name
    @return Bit array name
  */
  std::string getName(void) { return m_name; };

  /*!
    set Name for action
  */
  void setName(std::string name) { m_name = name; };

  /*!
    Get the register description
    @return Register description
  */
  std::string getDescription(std::string lang) { return m_mapDescription[lang]; };

  /*!
    Set register description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(std::string &lang, std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the register info URL
    @return Register info URL
  */
  std::string getInfoURL(std::string lang) { return m_mapInfoURL[lang]; };

  /*!
    Set register info URL
    @param lang Language
    @param url Register info URL to set
  */
  void setInfoURL(std::string &lang, std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get register offset for action parameter
    @return Register offset
  */
  uint32_t getOffset(void) { return m_offset; };

  /*!
    Set register offset for action parameter
    @param offset Register offset
  */
  void setOffset(uint32_t offset) { m_offset = offset; };

  /*!
    Set max value for action parameter
    @param max Max value
  */
  void setMax(uint8_t max) { m_max = max; };

  /*!
    Get max value for action parameter
    @return Max value
  */
  uint8_t getMax(void) { return m_max; };

  /*!
   Set min value for action parameter
   @param min Max value
 */
  void setMin(uint8_t min) { m_min = min; };

  /*!
    Get min value for action parameter
    @return Min value
  */
  uint8_t getMin(void) { return m_min; };

  /*!
   Fetch the bit definition list
   @return Bit definition list
 */
  std::deque<CMDF_Bit *> *getListBits(void) { return &m_list_bit; };

  /*!
    Fetch the value definition list
    @return Value definition list
  */
  std::deque<CMDF_ValueListValue *> *getListValues(void) { return &m_list_value; };

private:
  std::string m_name;                                  // Name for action parameter
  std::map<std::string, std::string> m_mapDescription; // Description of action parameter
  std::map<std::string, std::string> m_mapInfoURL;     // Url that contain extra help information

  uint16_t m_offset; // Offset for parameter (Always zero for level I)
  uint8_t m_min;     // Min value for parameter
  uint8_t m_max;     // Max value for parameter
  // uint8_t m_width;

  std::deque<CMDF_Bit *> m_list_bit;              // List with bit defines
  std::deque<CMDF_ValueListValue *> m_list_value; // List with selectable values
};

// ----------------------------------------------------------------------------

/*!
  CMDF_Action

  Holds information for one available action

 */

class CMDF_Action {

public:
  CMDF_Action();
  ~CMDF_Action();

  // Friend declarations
  friend CMDF;
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get bit array name
    @return Bit array name
  */
  std::string getName(void) { return m_name; };

  /*!
    set Name for action
  */
  void setName(std::string name) { m_name = name; };

  /*!
    Get the register description
    @return Register description
  */
  std::string getDescription(std::string lang) { return m_mapDescription[lang]; };

  /*!
    Set register description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(std::string &lang, std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the register info URL
    @return Register info URL
  */
  std::string getInfoURL(std::string lang) { return m_mapInfoURL[lang]; };

  /*!
    Set register info URL
    @param lang Language
    @param url Register info URL to set
  */
  void setInfoURL(std::string &lang, std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get action parameter code
    @return Action parameter code
  */
  uint16_t getCode(void) { return m_code; };

  /*!
    Set action parameter code
    @param code Action parameter code to set
  */
  void setCode(uint16_t code) { m_code = code; };

  /*!
    Get action parameter list
    @return Action parameter list
  */
  std::deque<CMDF_ActionParameter *> *getListActionParameter(void) { return &m_list_ActionParameter; };

private:
  std::string m_name;
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapInfoURL; // Url that contain extra hel information

  uint16_t m_code;

  std::deque<CMDF_ActionParameter *> m_list_ActionParameter; // List with action parameters
};

// ----------------------------------------------------------------------------

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
  friend CMDF;
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get decision matrix level
    @return Decision matrix level.
  */
  uint8_t getLevel(void) { return m_level; };

  /*!
    Set decision matrix level
    @param level Decision matrix level.
  */
  void setLevel(uint8_t level) { m_level = level; };

  /*!
    Get decision matrix start page
    @return Decision matrix start page.
  */
  uint16_t getStartPage(void) { return m_startPage; };

  /*!
    Set decision matrix start page
    @param startPage Decision matrix start page.
  */
  void setStartPage(uint16_t startPage) { m_startPage = startPage; };

  /*!
    Get decision matrix start offset
    @return Decision matrix start offset.
  */
  uint32_t getStartOffset(void) { return m_startOffset; };

  /*!
    Set decision matrix start offset
    @param startOffset Decision matrix start offset.
  */
  void setStartOffset(uint32_t startOffset) { m_startOffset = startOffset; };

  /*!
    Get decision matrix row count
    @return Decision matrix row count.
  */
  uint16_t getRowCount(void) { return m_rowCount; };

  /*!
    Set decision matrix row count
    @param rowCount Decision matrix row count.
  */
  void setRowCount(uint16_t rowCount) { m_rowCount = rowCount; };

  /*!
    Get decision matrix row size
    @return Decision matrix row size.
  */
  uint16_t getRowSize(void) { return m_rowSize; };

  /*!
    Set decision matrix row size
    @param rowSize Decision matrix row size.
  */
  void setRowSize(uint16_t rowSize) { m_rowSize = rowSize; };

  /*!
    Check if decision matrix is indexed
    @return True if decision matrix is indexed.
  */
  bool isIndexed(void) { return m_bIndexed; };

  /*!
    Set decision matrix indexed
    @param bIndexed True if decision matrix is indexed.
  */
  void setIndexed(bool bIndexed = true) { m_bIndexed = bIndexed; };

  /*!
   Get decision matrix row count
   @return Decision matrix action list.
  */
  std::deque<CMDF_Action *> *getActionList(void) { return &m_list_action; };

private:
  uint8_t m_level;        // 1 or 2 (defaults to 1)
  uint16_t m_startPage;   // Page where DM starts
  uint16_t m_startOffset; // Offset on start page for DM
  uint16_t m_rowCount;    // Number of rows in DM
  uint16_t m_rowSize;     // Size of a DM row (Normally 8)
  bool m_bIndexed;        // True of storage is indexed

  std::deque<CMDF_Action *> m_list_action; // Action description
};

// ----------------------------------------------------------------------------

/*!
  CMDF_EventData

  Holds info about one event data item

 */

class CMDF_EventData {

public:
  CMDF_EventData();
  ~CMDF_EventData();

  // Friend declarations
  friend CMDF;
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get bit array name
    @return Bit array name
  */
  std::string getName(void) { return m_name; };

  /*!
    Get the register description
    @return Register description
  */
  std::string getDescription(std::string lang) { return m_mapDescription[lang]; };

  /*!
    Set register description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(std::string &lang, std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the register info URL
    @return Register info URL
  */
  std::string getInfoURL(std::string lang) { return m_mapInfoURL[lang]; };

  /*!
    Set register info URL
    @param lang Language
    @param url Register info URL to set
  */
  void setInfoURL(std::string &lang, std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Fetch the bit definition list
    @return Bit definition list
  */
  std::deque<CMDF_Bit *> *getListBits(void) { return &m_list_bit; };

  /*!
    Fetch the value definition list
    @return Value definition list
  */
  std::deque<CMDF_ValueListValue *> *getListValues(void) { return &m_list_value; };

private:
  std::string m_name;
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapInfoURL; // Url that contain extra hel information

  uint16_t m_offset;

  std::deque<CMDF_Bit *> m_list_bit;              // List with bit defines
  std::deque<CMDF_ValueListValue *> m_list_value; // List with selectable values
};

// ----------------------------------------------------------------------------

/*!
  CMDF_Event

  Holds one available event on the module

 */

class CMDF_Event {

public:
  CMDF_Event();
  ~CMDF_Event();

  // Friend declarations
  friend CMDF;
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get event name
    @return EWvent name
  */
  std::string getName(void) { return m_name; };

  /*!
    Set event name
  */
  void setName(std::string &name) { m_name = name; };

  /*!
    Get event class
    @return Event class
  */
  uint16_t getClass(void) { return m_class; };

  /*!
    Set event class
    @param vscpclass Event class
  */
  void setClass(uint16_t vscpclass) { m_class = vscpclass; };

  /*!
    Get event type
    @return Event type
  */
  uint16_t getType(void) { return m_type; };

  /*!
    Set event type
    @param vscptype Event type
  */
  void setType(uint16_t vscptype) { m_type = vscptype; };

  /*!
    Get event priority
    @return Event priority
  */
  uint8_t getPriority(void) { return (m_priority & 7); };

  /*!
    Set event priority
    @param priority Event priority
  */
  void setPriority(uint8_t priority) { m_priority = (priority & 7); };

  /*!
    Get event direction
    @return Event direction
  */
  mdf_event_direction getDirection(void) { return m_direction; };

  /*!
    Set event direction
    @param direction Event direction
  */
  void setDirection(mdf_event_direction direction) { m_direction = direction; };

  /*!
    Get the event description
    @return event description
  */
  std::string getDescription(std::string lang) { return m_mapDescription[lang]; };

  /*!
    Set event description
    @param lang Language
    @param desc Event description to set
  */
  void setDescription(std::string &lang, std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the event info URL
    @return Event info URL
  */
  std::string getInfoURL(std::string lang) { return m_mapInfoURL[lang]; };

  /*!
    Set event info URL
    @param lang Language
    @param url Event info URL to set
  */
  void setInfoURL(std::string &lang, std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get the event data list
    @return Pointer to event data list
  */
  std::deque<CMDF_EventData *> *getListEventData(void) { return &m_list_eventdata; };

private:
  std::string m_name;
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapInfoURL; // Url that contain extra hel information

  uint16_t m_class;
  uint16_t m_type;
  uint8_t m_priority;
  mdf_event_direction m_direction;

  std::deque<CMDF_EventData *> m_list_eventdata; // List with event data descriptions
};

// ----------------------------------------------------------------------------

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
  friend CMDF;
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
    Get bit array name
    @return Bit array name
  */
  std::string getName(void) { return m_name; };

  /*!
    Get the register description
    @return Register description
  */
  std::string getDescription(std::string lang) { return m_mapDescription[lang]; };

  /*!
    Set register description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(std::string &lang, std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the register info URL
    @return Register info URL
  */
  std::string getInfoURL(std::string lang) { return m_mapInfoURL[lang]; };

  /*!
    Set register info URL
    @param lang Language
    @param url Register info URL to set
  */
  void setInfoURL(std::string &lang, std::string &url) { m_mapInfoURL[lang] = url; };

private:
  std::string m_name;
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapInfoURL; // Url that contain extra hel information
};

// ----------------------------------------------------------------------------

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
  friend CMDF;
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get bootloader algorithm
    @return Bootloader algorithm
  */
  uint8_t getAlgorithm(void) { return m_nAlgorithm; };

  /*!
    get bootloader block size
    @return Bootloader block size
  */
  uint32_t getBlockSize(void) { return m_nBlockSize; };

  /*!
    Get bootloader block count
    @return Bootloader block count
  */
  uint32_t getBlockCount(void) { return m_nBlockCount; };

private:
  uint8_t m_nAlgorithm;   // Bootloader algorithm used by device
  uint32_t m_nBlockSize;  // Size for one boot block
  uint32_t m_nBlockCount; // Number of boot blocks
};

// ----------------------------------------------------------------------------

/*!
  CMDF_Address

  Holds one address for the manufacturer

 */

class CMDF_Address {

public:
  CMDF_Address();
  ~CMDF_Address();

  // Friend declarations
  friend CMDF;
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get street address
    @return Street address
  */
  std::string getStreet(void) { return m_strStreet; };

  /*!
    Get town address
    @return Town address
  */
  std::string getTown(void) { return m_strTown; };

  /*!
    Get city address
    @return City address
  */
  std::string getCity(void) { return m_strCity; };

  /*!
    Get post code address
    @return Post code address
  */
  std::string getPostCode(void) { return m_strState; };

  /*!
    Get state address
    @return State address
  */
  std::string getState(void) { return m_strState; };

  /*!
    Get region address
    @return Region address
  */
  std::string getRegion(void) { return m_strRegion; };

  /*!
    Get country address
    @return Country address
  */
  std::string getCountry(void) { return m_strCountry; };

private:
  std::string m_strStreet;
  std::string m_strTown;
  std::string m_strCity;
  std::string m_strPostCode;
  std::string m_strState;
  std::string m_strRegion;
  std::string m_strCountry;
};

// ----------------------------------------------------------------------------

/*!
  CMDF_Manufacturer

  Holds information about one manufacturer of the module

 */

class CMDF_Manufacturer {

public:
  CMDF_Manufacturer();
  ~CMDF_Manufacturer();

  // Friend declarations
  friend CMDF;
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get a phone object from it's index
    @param index Index of phone object to get.
    @return Pointer to phone object or NULL if index is out of range.
  */
  CMDF_Item *getPhoneObj(size_t index = 0) { return ((m_list_Phone.size() <= index) ? nullptr : m_list_Phone[index]); };

  /*!
    Get a fax object from it's index
    @param index Index of fax objext to get.
    @return Pointer to fax object or NULL if index is out of range.
  */
  CMDF_Item *getFaxObj(size_t index = 0) { return ((m_list_Fax.size() <= index) ? nullptr : m_list_Fax[index]); };

  /*!
    Get a email object from it's index
    @param index Index of email object to get.
    @return Pointer to email object or NULL if index is out of range.
  */
  CMDF_Item *getEmailObj(size_t index = 0) { return ((m_list_Email.size() <= index) ? nullptr : m_list_Email[index]); };

  /*!
    Get a web object from it's index
    @param index Index of web object to get.
    @return Pointer to web object or NULL if index is out of range.
  */
  CMDF_Item *getWebObj(size_t index = 0) { return ((m_list_Web.size() <= index) ? nullptr : m_list_Web[index]); };

private:
  std::string m_strName;  // Manufacturer name
  CMDF_Address m_address; // Address of manufacturer

  std::deque<CMDF_Item *> m_list_Phone;
  std::deque<CMDF_Item *> m_list_Fax;
  std::deque<CMDF_Item *> m_list_Email;
  std::deque<CMDF_Item *> m_list_Web;
};

// ----------------------------------------------------------------------------

class CMDF_Picture {

public:
  CMDF_Picture();
  ~CMDF_Picture();

  // Friend declarations
  friend CMDF;
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
  std::map<std::string, std::string> m_mapInfoURL; // URL for full module information
};

// ----------------------------------------------------------------------------

/*!
  CMDF_Firmware

  Holds information about one firmware of the module

 */

class CMDF_Firmware {

public:
  CMDF_Firmware();
  ~CMDF_Firmware();

  // Friend declarations
  friend CMDF;
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
  std::map<std::string, std::string> m_mapInfoURL; // URL for full module information
};

// ---------------------------------------------------------------------------

class CMDF_Manual {

public:
  CMDF_Manual();
  ~CMDF_Manual();

  // Friend declarations
  friend CMDF;
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
  std::map<std::string, std::string> m_mapInfoURL; // URL for full module information
};

// ----------------------------------------------------------------------------

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
      @return Return CURLE_OK if a valid file is downloaded, else a
              curl error code.
  */
  CURLcode downLoadMDF(std::string &remoteFile, std::string &tempFile);

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
    @return Return string with module name
  */
  std::string getModuleName(void) { return m_name; };

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
  size_t getModuleDescriptionSize(void) { return m_mapDescription.size(); };

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
  size_t getModuleHelpUrlCount(void) { return m_mapInfoURL.size(); };

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

  /*!
    Get bootloader object
    @return Pointer to bootloader object.
  */
  CMDF_BootLoaderInfo *getBootLoaderObj(void) { return &m_bootInfo; };

  //-----------------------------------------------------------------------------

  /*!
    Get number of registers
    @return Number of registers.
  */
  size_t getRegisterCount(void) { return m_list_register.size(); };

  /*!
      Get number of defined registers
      @param page Register page to check
      @return Number of registers used.
  */
  size_t getRegisterCount(uint32_t page);

  /*!
      Return register definition from register + page
      @param register Register to search for.
      @param page Page top search for.
      @return Pointer to CMDF_Register class if found else NULL.
  */
  CMDF_Register *getRegister(uint32_t reg, uint32_t page = 0);

  /*!
      Get number of register pages used
      @param array Reference to array with pages
      @return Number of register pages used.
  */
  uint32_t getPages(std::set<long> &pages);

  //-----------------------------------------------------------------------------

  /*!
    Get number of defined remote variables
    @return Number of remote variables.
  */
  size_t getRemoteVariableCount(void) { return m_list_remotevar.size(); };

  /*!
      Return remote variable from its name
      @param name Name iof remote variable to search for.
      @return Pointer to CMDF_RemoteVariable class if found else NULL.
  */
  CMDF_RemoteVariable *getRemoteVariable(std::string name);

  /*!
    Get the decision matrix
    @return Pointer to decision matrix.
  */
  CMDF_DecisionMatrix *getDM(void) { return &m_dmInfo; };

  /*!
    Get the event list
    @return Pointer to the event list.
  */
  std::deque<CMDF_Event *> *getEventList(void) { return &m_list_event; };

  /*!
    Get the alarm list
    @return Pointer to the alarm list.
  */
  std::deque<CMDF_Bit *> *getAlarmList() { return &m_list_alarm; };

  // ----------------------------------------------------------------------------

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

  /*!
    Get items from bit list
    @param j JSON object with bit list
    @param list List to store bit items to
    @return true on VSCP_ERROR_SUCCESS on success, error code on failure.
  */
  int getBitList(json &j, std::deque<CMDF_Bit *> &list);

  /*!
    Get items from valulist
    @param j JSON object with valulist
    @param list List to store valuelist items to
    @return true on VSCP_ERROR_SUCCESS on success, error code on failure.
  */
  int getValueList(json &j, std::deque<CMDF_ValueListValue *> &list);

  /*!
    Get items from description list/item
    @param j JSON object with valulist
    @param list List to store description items to
    @return true on VSCP_ERROR_SUCCESS on success, error code on failure.
  */
  int getDescriptionList(json &j, std::map<std::string, std::string> &map);

  /*!
    Get items from infoURL list/item
    @param j JSON object with valulist
    @param list List to store infoURL items to
    @return true on VSCP_ERROR_SUCCESS on success, error code on failure.
  */
  int getInfoUrlList(json &j, std::map<std::string, std::string> &map);

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
  std::string m_name;                                  // Module name
  std::map<std::string, std::string> m_mapDescription; // Module description
  std::map<std::string, std::string> m_mapInfoURL;     // URL for full module information

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

  std::deque<CMDF_Register *> m_list_register;        // List with defined registers
  std::deque<CMDF_RemoteVariable *> m_list_remotevar; // List with defined remote variables
  std::deque<CMDF_Event *> m_list_event;              // Events this node can generate
  std::deque<CMDF_Bit *> m_list_alarm;                // List with alarm bit defines
};

#endif
