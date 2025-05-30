// FILE: mdf.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2025 Ake Hedman, the VSCP project
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
#include <vscphelper.h>
#include <vscp-class.h>
#include <vscp-type.h>

#include <expat.h>

#include <deque>
#include <map>
#include <set>
#include <string>

#include <nlohmann/json.hpp> // Needs C++11  -std=c++11

#include <curl/curl.h>

// https://github.com/nlohmann/json
using json = nlohmann::json;

// This is the access rights
typedef enum mdf_format {
  MDF_FORMAT_XML = 0,
  MDF_FORMAT_JSON
} mdf_format;

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

#define MAX_MDF_FILE_TYPES 6 // Maximum number of file types

// MDF file types
typedef enum mdf_file_type {
  mdf_file_type_none = 0, // None defined
  mdf_file_type_picture,  // Picture files
  mdf_file_type_video,    // Video files
  mdf_file_type_firmware, // Firmware files
  mdf_file_type_driver,   // Driver files
  mdf_file_type_manual,   // Manual files
  mdf_file_type_setup     // Setup files
} mdf_file_type;

// MDF record types
typedef enum mdf_record_type {
  mdf_type_unknown = 0,
  mdf_type_mdf,
  mdf_type_mdf_item,
  mdf_type_value,
  mdf_type_value_item,
  mdf_type_value_sub_item,
  mdf_type_bit,
  mdf_type_bit_item,
  mdf_type_bit_sub_item,
  mdf_type_register,
  mdf_type_register_item,
  mdf_type_register_sub_item,
  mdf_type_register_page,
  mdf_type_remotevar,
  mdf_type_remotevar_item,
  mdf_type_remotevar_sub_item,
  mdf_type_decision_matrix,
  mdf_type_decision_matrix_item,
  mdf_type_action,
  mdf_type_action_item,
  mdf_type_action_sub_item,
  mdf_type_action_param,
  mdf_type_action_param_item,
  mdf_type_action_param_sub_item,
  mdf_type_event,
  mdf_type_event_item,
  mdf_type_event_sub_item,
  mdf_type_event_data,
  mdf_type_event_data_item,
  mdf_type_event_data_sub_item,
  mdf_type_bootloader,
  mdf_type_bootloader_item,
  mdf_type_address,
  mdf_type_manufacturer,
  mdf_type_manufacturer_item,
  mdf_type_manufacturer_contact_phone,
  mdf_type_manufacturer_contact_fax,
  mdf_type_manufacturer_contact_email,
  mdf_type_manufacturer_contact_web,
  mdf_type_manufacturer_contact_social,
  mdf_type_email,
  mdf_type_email_item,
  mdf_type_phone,
  mdf_type_phone_item,
  mdf_type_fax,
  mdf_type_fax_item,
  mdf_type_web,
  mdf_type_web_item,
  mdf_type_social,
  mdf_type_social_item,
  mdf_type_file,              // File header - "File"
  mdf_type_picture,           // Picture head - "Pictures"
  mdf_type_picture_item,      // Picture object "Picture: some name"
  mdf_type_picture_sub_item,  // Picture content
  mdf_type_video,             // Video head - "Video"
  mdf_type_video_item,        // Video object - "Video : some name"
  mdf_type_video_sub_item,    // Video content
  mdf_type_firmware,          // Firmware head - "Firmware"
  mdf_type_firmware_item,     // Firmware object "Firmware: some name"
  mdf_type_firmware_sub_item, // Firmware content
  mdf_type_driver,            // Driver head - "Driver"
  mdf_type_driver_item,       // Driver object - "Driver: some name"
  mdf_type_driver_sub_item,   // Diver content
  mdf_type_setup,             // Setup head - "Setup Script"
  mdf_type_setup_item,        // Setup object - "Setup: some name"
  mdf_type_setup_sub_item,    // Setup content
  mdf_type_manual,            // Manual head - "Manual"
  mdf_type_manual_item,       // Manula object - "Manual: some name"
  mdf_type_manual_sub_item,   // Manual content"
  mdf_type_redirection,
  mdf_type_alarm,
  mdf_type_alarm_item,
  mdf_type_generic_string,           // Used for direct item editing
  mdf_type_generic_number,           // Used for direct item editing
  mdf_type_generic_url,              // Used for direct item editing
  mdf_type_generic_date,             // Used for direct item editing
  mdf_type_generic_access,           // Used for direct item editing
  mdf_type_generic_description,      // Used for direct item editing
  mdf_type_generic_description_item, // Used for direct item editing
  mdf_type_generic_help_url,         // Used for direct item editing
  mdf_type_generic_help_url_item     // Used for direct item editing
} mdf_record_type;

// * * * Settings * * *

// Forward declarations
class CMDF;
class CMDF_Bit;

/*!
  CMDF_Value
 */

class CMDF_Object {

public:
  CMDF_Object(mdf_record_type type = mdf_type_unknown);
  virtual ~CMDF_Object();

  /*!
    Get the object type
    @return Object type for record
  */
  mdf_record_type getMdfObjectType(void);

  /*!
    Get string representation for object type
    @return Standard string with description of object type
  */
  std::string getObjectTypeString(void);

  virtual std::map<std::string, std::string> *getMapDescription(void) { return nullptr; };
  virtual std::map<std::string, std::string> *getMapInfoUrl(void) { return nullptr; };

private:
  mdf_record_type m_type;
};

// ----------------------------------------------------------------------------

/*!
  CMDF_Value
 */

class CMDF_Value : public CMDF_Object {

public:
  CMDF_Value();
  virtual ~CMDF_Value();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
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
  void setName(const std::string &name) { m_name = name; };

  /*!
    Get value
    @return value .
  */
  std::string getValue(void) { return m_strValue; };

  /*!
    Set value
    @param value Value on string form.
  */
  void setValue(const std::string &value) { m_strValue = value; };

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
  std::string getDescription(const std::string lang = "en") { return m_mapDescription[lang]; };

  /*!
    Set value description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(const std::string &lang, std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the value info URL
    @return Value info URL
  */
  std::string getInfoURL(std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set value info URL
    @param lang Language
    @param url Value info URL to set
  */
  void setInfoURL(std::string lang, std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get description map
    @return Pointer to description map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get info URL map
    @return Pointer to info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

private:
  std::string m_name;
  std::string m_strValue;   // String because used for remote variabels also
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

class CMDF_Bit : public CMDF_Object {

public:
  CMDF_Bit();
  ~CMDF_Bit();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
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
    Set bit array name
    @return Set array name
  */
  void setName(const std::string &name) { m_name = name; };

  /*!
    Get the bit description
    @return bit description
  */
  std::string getDescription(std::string lang = "en") { return m_mapDescription[lang]; };

  /*!
    Set bit description
    @param lang Language
    @param desc bit description to set
  */
  void setDescription(std::string &lang, std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the bit info URL
    @return bit info URL
  */
  std::string getInfoURL(std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set bit info URL
    @param lang Language
    @param url bit info URL to set
  */
  void setInfoURL(const std::string &lang, const std::string &url) { m_mapInfoURL[lang] = url; };

  

  /*!
    Get description map
    @return Pointer to description map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get info URL map
    @return Pointer to info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

  /*!
    Get bit array start position (0-7)
    @return Bit array start position
  */
  uint8_t getPos(void) { return (m_pos & 7); };

  /*!
    Set bit array start position (0-7)
    @param pos Bit array start position
  */
  void setPos(uint8_t pos);

  /*!
    Get bit array length (1-8)
    @return Bit array length
  */
  uint8_t getWidth(void) { return m_width; };

  /*!
    Set bit array length (1-8)
    @param width Bit array length
  */
  void setWidth(uint8_t width);

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
  void setMin(uint8_t min); //{ m_min = min; };

  /*!
    Get bit array value
    @return Bit array value
  */
  uint8_t getMax(void) { return m_max; };

  /*!
    Set bit array value
    @param max Bit array value
  */
  void setMax(uint8_t max); // { m_max = max; };

  /*!
    Get access for bitfield
    @return Access for bitfield.
  */
  mdf_access_mode getAccess(void) { return m_access; };

  /*!
    Get acces for register on string form
    @return String with access form
  */
  std::string getAccessStr(void);

  /*!
    Set access for bitfield
    @param access Access for bitfield.
  */
  void setAccess(mdf_access_mode access) { m_access = access; };

  /*!
    Get the calculated mask for the bitfiled
    @return Mask
  */
  uint8_t getMask(void) { return m_mask; };

  /*!
    Fetch the value definition list
    @return Value definition list
  */
  std::deque<CMDF_Value *> *getListValues(void) { return &m_list_value; };

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

  uint8_t m_mask; // Calculated mask for bit field

  std::deque<CMDF_Value *> m_list_value; // List with selectable values
};

// ----------------------------------------------------------------------------

/*!
  CMDF_Register

  Holds information for one register
 */

class CMDF_Register : public CMDF_Object {

public:
  CMDF_Register();
  ~CMDF_Register();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Assignment
  */
  CMDF_Register &operator=(const CMDF_Register &other);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get the register name
    @return Register name
  */
  std::string getName(void) { return m_name; };

  /*!
    Set register name
    @param name Register name to set
  */
  void setName(const std::string &name) { m_name = name; };

  /*!
    Get the register description
    @return Register description
  */
  std::string getDescription(std::string lang = "en") { return m_mapDescription[lang]; };

  /*!
    Set register description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(const std::string &lang, const std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the register info URL
    @return Register info URL
  */
  std::string getInfoURL(const std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set register info URL
    @param lang Language
    @param url Register info URL to set
  */
  void setInfoURL(const std::string &lang, const std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get register default string
    @return Register default string. This string is set to "UNDEF"
            if no default is set.
  */
  std::string getDefault(void) { return m_strDefault; };

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
  void setDefault(const std::string &str) { m_strDefault = str; };

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
  void setPage(uint16_t page) { m_page = page; };

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
    *** Deprectated ***
    Get size for register
    @return Size for register.
  */
  uint32_t getSize(void) { return m_span; };

  /*!
    *** Deprcated ***
    Set size for register
    @param size Size for register.
  */
  void setSize(uint32_t size) { m_span = size; };

  /*!
    Get access for register
    @return Access for register.
  */
  mdf_access_mode getAccess(void) { return m_access; };

  /*!
    Get acces for register on string form
    @return String with access form
  */
  std::string getAccessStr(void);

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
    Get the type for a register on string form
    @return Type for register on string form
  */
  std::string getTypeStr(void);

  /*!
    Fetch the bit definition list
    @return Bit definition list
  */
  std::deque<CMDF_Bit *> *getListBits(void) { return &m_list_bit; };

  /*!
    Get the bit mask for all defined bits
    @return A combined bitmask for all bits
  */
  uint8_t getAllBitMask(void)
  {
    uint8_t mask                  = 0;
    std::deque<CMDF_Bit *> *pbits = getListBits();
    for (auto it = pbits->cbegin(); it != pbits->cend(); ++it) {
      mask |= (*it)->getMask();
    }
    return mask;
  };

  /*!
    Fetch the value definition list
    @return Value definition list
  */
  std::deque<CMDF_Value *> *getListValues(void) { return &m_list_value; };

  /*!
    Set Foreground color for VSCP Works grid.
    @param color Foreground color to set.
  */
  void setForegroundColor(uint32_t color) { m_fgcolor = color; };

  /*!
    Get Foreground color for VSCP Works grid.
    @return Foreground color.
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
    Get description map
    @return Pointer to description map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get info URL map
    @return Pointer to info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

private:
  std::string m_name;
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapInfoURL; // Url that contain extra help information

  uint16_t m_page;   // Level 1 page is 8-bit, pageing is not used for Level II.
  uint32_t m_offset; // Level 1 offset is 7 bits, Level II offset is 32 bits.
  uint16_t m_span;   // Defaults to 1. Number of bytes for a group of registers
  uint16_t m_width;  // Defaults to 8. Width in bits for register (1-8 bits)

  mdf_register_type m_type; // std=0/dmatix1=1/block=2

  // !!! Removed: Use 'span' instead
  // uint8_t m_size;           // Size for special types (default = 1)

  uint32_t m_min; // Defaults to 0
  uint32_t m_max; // Defaults to 255

  std::string m_strDefault; // "UNDEF" if not set

  mdf_access_mode m_access;

  std::deque<CMDF_Bit *> m_list_bit;     // List with bit defines
  std::deque<CMDF_Value *> m_list_value; // List with selectable values

  uint32_t m_fgcolor; // Cell foreground colour. Default = black.
  uint32_t m_bgcolor; // Cell background colour. Default = white.

  /*!
    Colors for odd and even rows. This is used to create register
    records when size is larger than one which is typical for
    registers defined as block or dmatix1.

    The parser set fgcolor and bgcolor when records are created for each item
    so they are not used directly
  */
  uint32_t m_fgeven; // Foreground for even row
  uint32_t m_fgodd;  // Foreground for odd row
  uint32_t m_bgeven; // Background for even row
  uint32_t m_bgodd;  // Background for odd row
};

// ----------------------------------------------------------------------------

/*!
  CMDF_RemoteVariable
 */

class CMDF_RemoteVariable : public CMDF_Object {

public:
  CMDF_RemoteVariable();
  ~CMDF_RemoteVariable();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
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
  void setName(const std::string &name) { m_name = name; };

  /*!
    Get the register description
    @return Register description
  */
  std::string getDescription(const std::string &lang = "en") { return m_mapDescription[lang]; };

  /*!
    Set register description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(const std::string &lang, const std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the register info URL
    @return Register info URL
  */
  std::string getInfoURL(std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set register info URL
    @param lang Language
    @param url Register info URL to set
  */
  void setInfoURL(const std::string &lang, const std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get description map
    @return Pointer to description map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get info URL map
    @return Pointer to info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

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
  std::string getTypeStr(void) { return getTypeString(); };

  /*!
      Get number of bytes for a remote variable type
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
  void setDefault(const std::string &strDefault) { m_strDefault = strDefault; };

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
    Get acces for register on string form
    @return String with access form
  */
  std::string getAccessStr(void);

  /*!
    Set access mode for remote variable
    @param access Access mode for remote variable.
  */
  void setAccess(mdf_access_mode access) { m_access = access; };

  /*!
    Get bit position for boolean
    @return Page for remote variable.
  */
  uint16_t getBitPos(void) { return m_bitpos & 0x07; };

  /*!
    Set bit position for boolean
    @param bitpos Bit postion 0-7.
  */
  void setBitPos(uint8_t bitpos) { m_bitpos = bitpos & 0x07; };

  /*!
    Get value list for remote variable
    @return Value list for remote variable.
  */
  std::deque<CMDF_Value *> *getListValues(void) { return &m_list_value; };

  /*!
    Get bit list for remote variable
    @return Bit list for remote variable.
  */
  std::deque<CMDF_Bit *> *getListBits(void) { return &m_list_bit; };

  /*!
    Set Foreground color for VSCP Works grid.
    @param color Foreground color to set.
  */
  void setForegroundColor(uint32_t color) { m_fgcolor = color; };

  /*!
    Get Foreground color for VSCP Works grid.
    @return Forgrund color.
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

  uint32_t m_bgcolor; // Cell background colour. Default = white.
  uint32_t m_fgcolor; // Cell foreground colour. Default = black.

  std::deque<CMDF_Bit *> m_list_bit;     // List with bit defines
  std::deque<CMDF_Value *> m_list_value; // List with selectable values
};

// ----------------------------------------------------------------------------

/*!
  CMDF_ActionParameter

  Holds information for one action parameter

 */

class CMDF_ActionParameter : public CMDF_Object {

public:
  CMDF_ActionParameter();
  ~CMDF_ActionParameter();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
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
  void setName(const std::string &name) { m_name = name; };

  /*!
    Get the register description
    @return Register description
  */
  std::string getDescription(std::string lang = "en") { return m_mapDescription[lang]; };

  /*!
    Set register description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(const std::string &lang, const std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the register info URL
    @return Register info URL
  */
  std::string getInfoURL(std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set register info URL
    @param lang Language
    @param url Register info URL to set
  */
  void setInfoURL(const std::string &lang, const std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get description map
    @return Pointer to description map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get info URL map
    @return Pointer to info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

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
  std::deque<CMDF_Value *> *getListValues(void) { return &m_list_value; };

private:
  std::string m_name;                                  // Name for action parameter
  std::map<std::string, std::string> m_mapDescription; // Description of action parameter
  std::map<std::string, std::string> m_mapInfoURL;     // Url that contain extra help information

  uint16_t m_offset; // Offset for parameter (Always zero for level I)
  uint8_t m_min;     // Min value for parameter
  uint8_t m_max;     // Max value for parameter

  std::deque<CMDF_Bit *> m_list_bit;     // List with bit defines
  std::deque<CMDF_Value *> m_list_value; // List with selectable values
};

// ----------------------------------------------------------------------------

/*!
  CMDF_Action

  Holds information for one available action

 */

class CMDF_Action : public CMDF_Object {

public:
  CMDF_Action();
  ~CMDF_Action();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
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
  void setName(const std::string &name) { m_name = name; };

  /*!
    Get the register description
    @return Register description
  */
  std::string getDescription(const std::string &lang = "en") { return m_mapDescription[lang]; };

  /*!
    Set register description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(const std::string &lang, const std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the register info URL
    @return Register info URL
  */
  std::string getInfoURL(std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set register info URL
    @param lang Language
    @param url Register info URL to set
  */
  void setInfoURL(const std::string &lang, const std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get description map
    @return Pointer to description map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get info URL map
    @return Pointer to info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

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
    Get pointer to action parameter object from its offset
    @param code Offset for action parameter to fetch
    @return Pointer to action parameter object or nullptr if no action parameter with that offset is found.
  */
  CMDF_ActionParameter *getActionParam(uint16_t offset);

  /*!
    Add action parameter
    @param pactionparam Pointer to action parameter to add. The offset
    of the action parameter must be unique.
    @return true on success, false otherwise.
  */
  bool addActionParam(CMDF_ActionParameter *pactionparam);

  /*!
    Delete action parameter
    @param pactionparam Pointer to action parameter to delete
    @return true on success, false on failure.
  */
  bool deleteActionParam(CMDF_ActionParameter *pactionparam);

  /*!
    Delete action parameter
    @param offset Offset of action parameter to delete.
    @return true on success, false on failure.
  */
  bool deleteActionParam(uint8_t offset);

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

class CMDF_DecisionMatrix : public CMDF_Object {

public:
  CMDF_DecisionMatrix();
  ~CMDF_DecisionMatrix();

  // Ordinals for level I DM items on row
  static const int IDX_ADDRESS_ORIGIN           = VSCP_LEVEL1_DM_OFFSET_OADDR;
  static const int IDX_ADDRESS_FLAGS            = VSCP_LEVEL1_DM_OFFSET_FLAGS;
  static const int IDX_ADDRESS_CLASS_MASK       = VSCP_LEVEL1_DM_OFFSET_CLASS_MASK;
  static const int IDX_ADDRESS_CLASS_FILTER     = VSCP_LEVEL1_DM_OFFSET_CLASS_FILTER;
  static const int IDX_ADDRESS_TYPE_MASK        = VSCP_LEVEL1_DM_OFFSET_TYPE_MASK;
  static const int IDX_ADDRESS_TYPE_FILTER      = VSCP_LEVEL1_DM_OFFSET_TYPE_FILTER;
  static const int IDX_ADDRESS_ACTION           = VSCP_LEVEL1_DM_OFFSET_ACTION;
  static const int IDX_ADDRESS_ACTION_PARAMETER = VSCP_LEVEL1_DM_OFFSET_ACTION_PARAM;

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
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
    Get pointer to action object from 6its code
    @param code Code for action to fetch
    @return Pointer to action or nullptr if no action with that code is found.
  */
  CMDF_Action *getAction(uint16_t code);

  /*!
   Add action
   @param paction Pointer to action to add. The code
   of the action must be unique.
   @return true on success, false otherwise.
 */
  bool addAction(CMDF_Action *paction);

  /*!
    Delete action
    @param paction Pointer to action to delete
    @return true on success, false on failure.
  */
  bool deleteAction(CMDF_Action *paction);

  /*!
    Delete action
    @param offset Offset of action to delete.
    @return true on success, false on failure.
  */
  bool deleteAction(uint8_t code);

  /*!
   Get decision matrix row count
   @return Decision matrix action list.
  */
  std::deque<CMDF_Action *> *getActionList(void) { return &m_list_action; };

  // int getRegister(uint8_t row, CMDF_DecisionMatrix__dmindex idx);

private:
  uint8_t m_level;        // 1 or 2 (defaults to 1)
  uint16_t m_startPage;   // Page where DM starts
  uint16_t m_startOffset; // Offset on start page for DM
  uint16_t m_rowCount;    // Number of rows in DM
  uint16_t m_rowSize;     // Size of a DM row (Normally 8)

  std::deque<CMDF_Action *> m_list_action; // Action description
};

// ----------------------------------------------------------------------------

/*!
  CMDF_EventData

  Holds info about one event data item

 */

class CMDF_EventData : public CMDF_Object {

public:
  CMDF_EventData();
  ~CMDF_EventData();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get event data
    @return Event data name
  */
  std::string getName(void) { return m_name; };

  /*!
    Set event data name
    @param name Event data name to set
  */
  void setName(const std::string &name) { m_name = name; };

  /*!
    Get the register description
    @return Register description
  */
  std::string getDescription(std::string lang = "en") { return m_mapDescription[lang]; };

  /*!
    Set register description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(const std::string &lang, const std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the register info URL
    @return Register info URL
  */
  std::string getInfoURL(std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set register info URL
    @param lang Language
    @param url Register info URL to set
  */
  void setInfoURL(const std::string &lang, const std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get description map
    @return Pointer to description map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get info URL map
    @return Pointer to info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

  /*!
    Get event data offset
    @return Event data offset
  */
  uint16_t getOffset(void) { return m_offset; };

  /*!
    Set event data offset
    @param offset Event data offset
  */
  void setOffset(uint16_t offset) { m_offset = offset; };

  /*!
    Fetch the bit definition list
    @return Bit definition list
  */
  std::deque<CMDF_Bit *> *getListBits(void) { return &m_list_bit; };

  /*!
    Fetch the value definition list
    @return Value definition list
  */
  std::deque<CMDF_Value *> *getListValues(void) { return &m_list_value; };

private:
  std::string m_name;
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapInfoURL; // Url that contain extra hel information

  uint16_t m_offset;

  std::deque<CMDF_Bit *> m_list_bit;     // List with bit defines
  std::deque<CMDF_Value *> m_list_value; // List with selectable values
};

// ----------------------------------------------------------------------------

/*!
  CMDF_Event

  Holds one available event on the module

 */

class CMDF_Event : public CMDF_Object {

public:
  CMDF_Event();
  ~CMDF_Event();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
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
  void setName(const std::string &name) { m_name = name; };

  /*!
    Get event class
    @return Event class
  */
  uint16_t getClass(void) { return m_class; };

  /*!
    Set event class
    @param vscpclass Event class
  */
  void setClass(int vscpclass) { m_class = vscpclass; };

  /*!
    Get event type
    @return Event type
  */
  int getType(void) { return m_type; };

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
  std::string getDescription(std::string lang = "en") { return m_mapDescription[lang]; };

  /*!
    Set event description
    @param lang Language
    @param desc Event description to set
  */
  void setDescription(const std::string &lang, const std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the event info URL
    @return Event info URL
  */
  std::string getInfoURL(std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set event info URL
    @param lang Language
    @param url Event info URL to set
  */
  void setInfoURL(const std::string &lang, const std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get description map
    @return Pointer to description map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get info URL map
    @return Pointer to info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

  /*!
    Get the event data list
    @return Pointer to event data list
  */
  std::deque<CMDF_EventData *> *getListEventData(void) { return &m_list_eventdata; };

  /*!
    Get pointer to event data object from offset
    @param offset Offset of eventdata
    @return Pointer to eventdata object or nullpointer if no object is found.
  */
  CMDF_EventData *getEventData(uint8_t offset);

  /*!
    Add event data object
    @param pEventData Pointer top event data object to add.
    @return true on success, false on failure.
  */
  bool addEventData(CMDF_EventData *pEventData);

  /*!
    Delete event data object
    @param pEventData Pointer top event data object to add.
    @return true on success, false on failure.
  */
  bool deleteEventData(CMDF_EventData *pEventData);

  /*!
    Check if offset is unique for an item
    @param offset Offset to check
    @return True of unique
  */
  bool isEventDataOffsetUnique(uint8_t offset);

  /*!
    Check if offset is unique for a event data item. 
    The item itself is not checked.
    @param pEventData Event data item to check
    @return True of unique
  */
  bool isEventDataOffsetUnique(CMDF_EventData *pEventData);

private:
  std::string m_name;
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapInfoURL; // Url that contain extra hel information

  int m_class; // Event class (-1 is all)
  int m_type;  // Event type (-1 is all)
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

class CMDF_Item : public CMDF_Object {

public:
  CMDF_Item();
  ~CMDF_Item();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
    Get item value (***Deprecated use getValue()***)
    @return Value
  */
  std::string getName(void) { return m_value; };

  /*!
    Get item value
    @return Value
  */
  std::string getValue(void) { return m_value; };

  /*!
    Set item value
    @param Value to set
  */
  void setValue(const std::string &str) { m_value = str; };

  /*!
    Get the register description
    @return Register description
  */
  std::string getDescription(std::string lang = "en") { return m_mapDescription[lang]; };

  /*!
    Set register description
    @param lang Language
    @param desc Register description to set
  */
  void setDescription(const std::string &lang, const std::string &desc) { m_mapDescription[lang] = desc; };

  /*!
    Get the register info URL
    @return Register info URL
  */
  std::string getInfoURL(std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set register info URL
    @param lang Language
    @param url Register info URL to set
  */
  void setInfoURL(const std::string &lang, const std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get description map
    @return map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get Info/URL map
    @return map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

private:
  std::string m_value; // Item value
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapInfoURL; // Url that contain extra hel information
};

// ----------------------------------------------------------------------------

/*!
  CMDF_BootLoaderInfo

  Holds information about the capabilities of a system
  decision matrix.
 */

class CMDF_BootLoaderInfo : public CMDF_Object {

public:
  CMDF_BootLoaderInfo();
  ~CMDF_BootLoaderInfo();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
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
    Set bootloader algorithm
    @param nAlgorithm Code for Bootloader algorithm
  */
  void setAlgorithm(uint8_t nAlgorithm) { m_nAlgorithm = nAlgorithm; }

  /*!
    get bootloader block size
    @return Bootloader block size
  */
  uint32_t getBlockSize(void) { return m_nBlockSize; };

  /*!
    Set bootloader blocksize
    @param size Blocksize to set
  */
  void setBlocksize(uint32_t size) { m_nBlockSize = size; };

  /*!
    Get bootloader block count
    @return Bootloader block count
  */
  uint32_t getBlockCount(void) { return m_nBlockCount; };

  /*!
    Set block count
    @param cnt Number of bootloader blocks
  */
  void setBlockCount(uint32_t cnt) { m_nBlockCount = cnt; };

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

class CMDF_Address : public CMDF_Object {

public:
  CMDF_Address();
  ~CMDF_Address();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(const std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
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
    Set street address
    @param Street address to set
  */
  void setStreet(const std::string &str) { m_strStreet = str; };

  /*!
    Get town address
    @return Town address
  */
  std::string getTown(void) { return m_strTown; };

  /*!
    Set town address
    @param str Town address to set
  */
  void setTown(const std::string &str) { m_strTown = str; };

  /*!
    Get city address
    @return City address
  */
  std::string getCity(void) { return m_strCity; };

  /*!
    Set city address
    @param str City address to set
  */
  void setCity(const std::string &str) { m_strCity = str; };

  /*!
    Get post code address
    @return Post code address
  */
  std::string getPostCode(void) { return m_strPostCode; };

  /*!
    Set post code address
    @param str Post code address to set
  */
  void setPostCode(const std::string &str) { m_strPostCode = str; };

  /*!
    Get state address
    @return State address
  */
  std::string getState(void) { return m_strState; };

  /*!
    Set state address
    @param str State address to set
  */
  void setState(const std::string &str) { m_strState = str; };

  /*!
    Get region address
    @return Region address
  */
  std::string getRegion(void) { return m_strRegion; };

  /*!
    Set region address
    @param str Region address to set
  */
  void setRegion(const std::string &str) { m_strRegion = str; };

  /*!
    Get country address
    @return Country address
  */
  std::string getCountry(void) { return m_strCountry; };

  /*!
    Set country address
    @param str Country address to set
  */
  void setCountry(const std::string &str) { m_strCountry = str; };

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

class CMDF_Manufacturer : public CMDF_Object {

public:
  CMDF_Manufacturer();
  ~CMDF_Manufacturer();

  enum contact_type { contact_type_phone, contact_type_fax, contact_type_email, contact_type_web, contact_type_social };

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /// Get company name
  std::string getName(void) { return m_strName; };

  /// Set company name
  void setName(const std::string &name) { m_strName = name; };

  /// Get address object
  CMDF_Address *getAddressObj(void) { return &m_address; };

  /*!
    Get a phone object from it's index
    @param index Index of phone object to get.
    @return Pointer to phone object or NULL if index is out of range.
  */
  CMDF_Item *getPhoneObj(size_t index = 0) { return ((m_list_Phone.size() <= index) ? nullptr : m_list_Phone[index]); };

  /*!
    Get number of phone elements
    @return Number of phone elements
  */
  size_t getPhoneObjCount(void) { return m_list_Phone.size(); };

  /*!
    Get a fax object from it's index
    @param index Index of fax objext to get.
    @return Pointer to fax object or NULL if index is out of range.
  */
  CMDF_Item *getFaxObj(size_t index = 0) { return ((m_list_Fax.size() <= index) ? nullptr : m_list_Fax[index]); };

  /*!
    Get number of fax elements
    @return Number of fax elements
  */
  size_t getFaxObjCount(void) { return m_list_Fax.size(); };

  /*!
    Get a email object from it's index
    @param index Index of email object to get.
    @return Pointer to email object or NULL if index is out of range.
  */
  CMDF_Item *getEmailObj(size_t index = 0) { return ((m_list_Email.size() <= index) ? nullptr : m_list_Email[index]); };

  /*!
    Get number of email elements
    @return Number of email elements
  */
  size_t getEmailObjCount(void) { return m_list_Email.size(); };

  /*!
    Get a web object from it's index
    @param index Index of web object to get.
    @return Pointer to web object or NULL if index is out of range.
  */
  CMDF_Item *getWebObj(size_t index = 0) { return ((m_list_Web.size() <= index) ? nullptr : m_list_Web[index]); };

  /*!
    Get number of web elements
    @return Number of web elements
  */
  size_t getWebObjCount(void) { return m_list_Web.size(); };

  /*!
    Get a social object from it's index
    @param index Index of web object to get.
    @return Pointer to web object or NULL if index is out of range.
  */
  CMDF_Item *getSocialObj(size_t index = 0)
  {
    return ((m_list_Social.size() <= index) ? nullptr : m_list_Social[index]);
  };

  /*!
    Get number of social elements
    @return Number of social elements
  */
  size_t getSocialObjCount(void) { return m_list_Social.size(); };

  /*!
    Get pointer to list for phone contact item
    @ſeturn Pointer to phone contact list
  */
  std::deque<CMDF_Item *> *getPhoneContactList(void) { return &m_list_Phone; };

  /*!
    Get pointer to list for fax contact item
    @ſeturn Pointer to fax contact list
  */
  std::deque<CMDF_Item *> *getFaxContactList(void) { return &m_list_Fax; };

  /*!
    Get pointer to list for email contact item
    @ſeturn Pointer to email contact list
  */
  std::deque<CMDF_Item *> *getEmailContactList(void) { return &m_list_Email; };

  /*!
    Get pointer to list for web contact item
    @ſeturn Pointer to web contact list
  */
  std::deque<CMDF_Item *> *getWebContactList(void) { return &m_list_Web; };

  /*!
    Get pointer to list for social contact item
    @ſeturn Pointer to social contact list
  */
  std::deque<CMDF_Item *> *getSocialContactList(void) { return &m_list_Social; };

private:
  std::string m_strName;  // Manufacturer name
  CMDF_Address m_address; // Address of manufacturer

  std::deque<CMDF_Item *> m_list_Phone;
  std::deque<CMDF_Item *> m_list_Fax;
  std::deque<CMDF_Item *> m_list_Email;
  std::deque<CMDF_Item *> m_list_Web;
  std::deque<CMDF_Item *> m_list_Social;
};

// ----------------------------------------------------------------------------

class CMDF_Picture : public CMDF_Object {

public:
  CMDF_Picture();
  ~CMDF_Picture();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get name of picture
    @return name for picture
  */
  std::string getName(void) { return m_strName; };

  /*!
    Set name of picture
    @param strURL name
  */
  void setName(const std::string &name) { m_strName = name; };

  /*!
    Get URL for picture
    @return URL for picture
  */
  std::string getUrl(void) { return m_strURL; };

  /*!
    Set URL for picture
    @param strURL URL for picture
  */
  void setUrl(const std::string &strURL) { m_strURL = strURL; };

  /*!
    Get format for picture
    @return format for picture
  */
  std::string getFormat(void) { return m_strFormat; };

  /*!
    Set format for picture
    @param strFormat Format for picture
  */
  void setFormat(const std::string &strFormat) { m_strFormat = strFormat; };

  /*
    Get date for object
    @return date on ISO string format
  */
  std::string getDate(void) { return m_strDate; };

  /*!
    Set date for object
    @param isodate Date for object oin ISO string form.
  */
  void setDate(const std::string &isodate) { m_strDate = isodate; };

  /*!
    Assignment
  */
  CMDF_Picture &operator=(const CMDF_Picture &other)
  {
    // Guard self assignment
    if (this == &other) {
      return *this;
    }
    m_strName   = other.m_strName;
    m_strURL    = other.m_strURL;
    m_strFormat = other.m_strFormat;
    m_strDate   = other.m_strDate;
    return *this;
  };

  /*!
    Set description for picture
  */
  void setDescription(const std::string &strDescription, const std::string &strLanguage)
  {
    m_mapDescription[strLanguage] = strDescription;
  };

  /*!
    Get description for picture
    @param strLanguage Language for description (default="en")
    @return Description for picture as string
  */
  std::string getDescription(std::string strLanguage = "en") { return m_mapDescription[strLanguage]; };

  /*!
    Get the value info URL
    @return Value info URL
  */
  std::string getInfoURL(std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set value info URL
    @param lang Language
    @param url Value info URL to set
  */
  void setInfoURL(const std::string lang, const std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get description map
    @return Pointer to description map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get info URL map
    @return Pointer to info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

private:
  /*!
    Name of picture
  */
  std::string m_strName;

  /*!
    Path to picture file
  */
  std::string m_strURL;

  /*!
    Picture type
  */
  std::string m_strFormat;

  /*
    Date
  */
  std::string m_strDate;

  /*!
      Description of file
  */
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapInfoURL; // URL for full module information
};

// ----------------------------------------------------------------------------

class CMDF_Video : public CMDF_Object {

public:
  CMDF_Video();
  ~CMDF_Video();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get name of video
    @return name for video
  */
  std::string getName(void) { return m_strName; };

  /*!
    Set name of video
    @param strURL name
  */
  void setName(const std::string &name) { m_strName = name; };

  /*!
    Get URL for video
    @return URL for video
  */
  std::string getUrl(void) { return m_strURL; };

  /*!
    Set URL for video
    @param strURL URL for video
  */
  void setUrl(const std::string &strURL) { m_strURL = strURL; };

  /*!
    Get format for video
    @return format for video
  */
  std::string getFormat(void) { return m_strFormat; };

  /*!
    Set format for video
    @param strFormat Format for video
  */
  void setFormat(const std::string &strFormat) { m_strFormat = strFormat; };

  /*
    Get date for object
    @return date on ISO string format
  */
  std::string getDate(void) { return m_strDate; };

  /*!
    Set date for object
    @param isodate Date for object oin ISO string form.
  */
  void setDate(const std::string &isodate) { m_strDate = isodate; };

  /*!
    Assignment
  */
  CMDF_Video &operator=(const CMDF_Video &other)
  {
    // Guard self assignment
    if (this == &other) {
      return *this;
    }
    m_strName   = other.m_strName;
    m_strURL    = other.m_strURL;
    m_strFormat = other.m_strFormat;
    m_strDate   = other.m_strDate;
    return *this;
  };

  /*!
    Set description for video
  */
  void setDescription(const std::string &strDescription, const std::string &strLanguage)
  {
    m_mapDescription[strLanguage] = strDescription;
  };

  /*!
    Get description for video
    @param strLanguage Language for description (default="en")
    @return Description for video as string
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

  /*!
    Get the value info URL
    @return Value info URL
  */
  std::string getInfoURL(std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set value info URL
    @param lang Language
    @param url Value info URL to set
  */
  void setInfoURL(const std::string lang, const std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get description map
    @return Pointer to description map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get info URL map
    @return Pointer to info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

private:
  /*!
    Name of video
  */
  std::string m_strName;

  /*!
    Path to video file
  */
  std::string m_strURL;

  /*!
    video type
  */
  std::string m_strFormat;

  /*
    Date
  */
  std::string m_strDate;

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

class CMDF_Firmware : public CMDF_Object {

public:
  CMDF_Firmware();
  ~CMDF_Firmware();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get name of firmware
    @return name for firmware
  */
  std::string getName(void) { return m_strName; };

  /*!
    Set name of firmware
    @param strURL name
  */
  void setName(const std::string &name) { m_strName = name; };

  /*!
    Get URL for firmware
    @return URL for firmware
  */
  std::string getUrl(void) { return m_strURL; };

  /*!
    Set URL
    @param url URL to set
  */
  void setUrl(const std::string &url) { m_strURL = url; };

  /*!
    Get target string for firmware
    @return Target string for firmware
  */
  std::string getTarget(void) { return m_strTarget; };

  /*!
    Get target code for firmware
    @return Target code for firmware
  */
  uint16_t getTargetCode(void) { return m_targetCode; };

  /*!
    Set target code
    @param code
  */
  void setTargetCode(uint16_t code) { m_targetCode = code; };

  /*!
    Get hex file string format for firmware (INTELHEX is default)
    @return Hex string format
  */
  std::string getFormat(void) { return vscp_makeUpper_copy(m_strFormat); };

  /*!
    Set format
    @param format Format to set
  */
  void setFormat(const std::string &format) { m_strFormat = vscp_makeUpper_copy(format); };

  /*
    Get date for object
    @return date on ISO string format
  */
  std::string getDate(void) { return m_strDate; };

  /*!
    Set date for object
    @param isodate Date for object oin ISO string form.
  */
  void setDate(const std::string &isodate) { m_strDate = isodate; };

  /*!
    Get version major for firmware
    @return Version major for firmware
  */
  uint16_t getVersionMajor(void) { return m_version_major; };

  /*!
    Set major version
    @param ver Major version to set
  */
  void setVersionMajor(uint16_t ver) { m_version_major = ver; };

  /*!
    Get version minor for firmware
    @return Version minor for firmware
  */
  uint16_t getVersionMinor(void) { return m_version_minor; };

  /*!
    Set minor version
    @param ver Minor version to set
  */
  void setVersionMinor(uint16_t ver) { m_version_minor = ver; };

  /*!
    Get version patch for firmware
    @return Version patch for firmware
  */
  uint16_t getVersionPatch(void) { return m_version_patch; };

  /*!
    Set patch version
    @param ver Patch version to set
  */
  void setVersionPatch(uint16_t ver) { m_version_patch = ver; };

  /*!
    Get version on string form (x.y.z)
    @return Version on string form
  */
  std::string getVersion(void)
  {
    return (std::to_string(getVersionMajor()) + "." + std::to_string(getVersionMinor()) + "." +
            std::to_string(getVersionPatch()));
  };

  /*!
    Set version from string on form x.y.z
    @param ver Version string
  */
  void setVersion(const std::string &ver);

  /*!
    Get size for firmware
    @return Version patch for firmware
  */
  size_t getSize(void) { return m_size; };

  /*!
    Set flash size
    @param size Flash size to set
  */
  void setSize(size_t size) { m_size = size; };

  /*!
    Get MD5 for firmware file on hex string format
    @return md5 hash for firmware file on string format.
  */
  std::string getMd5(void) { return m_strMd5; };

  /*!
    Set MD5 checksum
    @param md5 MD5 checksum string to set
  */

  void setMd5(const std::string &md5) { m_strMd5 = md5; };

  /*!
    Assignment
  */
  CMDF_Firmware &operator=(const CMDF_Firmware &other)
  {
    // Guard self assignment
    if (this == &other) {
      return *this;
    }
    m_strName       = other.m_strName;
    m_strURL        = other.m_strURL;
    m_strTarget     = other.m_strTarget;
    m_targetCode    = other.m_targetCode;
    m_strFormat     = other.m_strFormat;
    m_strDate       = other.m_strDate;
    m_size          = other.m_size;
    m_version_major = other.m_version_major;
    m_version_minor = other.m_version_minor;
    m_version_patch = other.m_version_patch;
    m_strMd5        = other.m_strMd5;
    return *this;
  };

  /*!
    Set description for firmware
  */
  void setDescription(const std::string &strDescription, const std::string &strLanguage)
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

  /*!
    Get the value info URL
    @return Value info URL
  */
  std::string getInfoURL(std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set value info URL
    @param lang Language
    @param url Value info URL to set
  */
  void setInfoURL(const std::string lang, const std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get description map
    @return Pointer to description map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get info URL map
    @return Pointer to info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

private:
  // Nam of firmware
  std::string m_strName;

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

/*!
  CMDF_Driver

  Holds information about one driver for the module

 */

class CMDF_Driver : public CMDF_Object {

public:
  CMDF_Driver();
  ~CMDF_Driver();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get name of setup
    @return name for setup
  */
  std::string getName(void) { return m_strName; };

  /*!
    Set name of driver
    @param name name
  */
  void setName(const std::string &name) { m_strName = name; };

  /*!
    Get URL for CMDF_Driver
    @return URL for CMDF_Driver
  */
  std::string getUrl(void) { return m_strURL; };

  /*!
    Set URL of driver
    @param url name
  */
  void setUrl(const std::string &url) { m_strURL = url; };

  /*!
    Get driver type for CMDF_Driver
    @return Target string for CMDF_Driver
  */
  std::string getType(void) { return m_strType; };

  /*!
    Set type of driver
    @param type Type of driver
  */
  void setType(const std::string &type) { m_strType = type; };

  /*!
    Get driver type for CMDF_Driver
    @return Target string for CMDF_Driver
  */
  std::string getArchitecture(void) { return m_strArchitecture; };

  /*!
    Set architecture of driver
    @param arch Architecture of driver
  */
  void setArchitecture(const std::string &arch) { m_strArchitecture = arch; };

  // Deprecated
  std::string getFormat(void) { return m_strArchitecture; };

  /*!
    Get OS (Operation System) for driver
    @return OS for driver
  */
  std::string getOS(void) { return m_strOS; };

  /*!
    Set OS of driver
    @param os Type of driver
  */
  void setOS(const std::string &os) { m_strOS = os; };

  /*!
    Get OS version for the driver
    @return OS version for driver
  */
  std::string getOSVer(void) { return m_strOSVer; };

  /*!
    Set OS version of driver
    @param osver Type of driver
  */
  void setOSVer(const std::string &osver) { m_strOSVer = osver; };

  /*!
    Get version major for driver
    @return Version major for driver
  */
  uint16_t getVersionMajor(void) { return m_version_major; };

  /*!
    Set major version of driver
    @param ver Major version to set
  */
  void setVersionMajor(uint16_t ver) { m_version_major = ver; };

  /*!
    Get version minor for driver
    @return Version minor for driver
  */
  uint16_t getVersionMinor(void) { return m_version_minor; };

  /*!
    Set minor version of driver
    @param ver Major version to set
  */
  void setVersionMinor(uint16_t ver) { m_version_minor = ver; };

  /*!
    Get version patch for driver
    @return Version patch for driver
  */
  uint16_t getVersionPatch(void) { return m_version_patch; };

  /*!
    Set patch version of Driver
    @param ver Major version to set
  */
  void setVersionPatch(uint16_t ver) { m_version_patch = ver; };

  /*!
    Get version on string format x.y.x
    @return Version on string format
  */
  std::string getVersion(void)
  {
    return (std::to_string(getVersionMajor()) + "." + std::to_string(getVersionMinor()) + "." +
            std::to_string(getVersionPatch()));
  };

  /*!
    Set version from string on form x.y.z
    @param ver Version string
  */
  void setVersion(const std::string &ver);

  /*!
    Get MD5 for driver file on hex string format
    @return md5 hash for driver file on string hex format.
  */
  std::string getMd5(void) { return m_strMd5; };

  /*!
    Set MD5 checksum
    @param md5 MD5 checksum to set as hex string.
  */
  void setMd5(const std::string &md5) { m_strMd5 = md5; };

  /*
    Get date for object
    @return date on ISO string format
  */
  std::string getDate(void) { return m_strDate; };

  /*!
    Set date for object
    @param isodate Date for object oin ISO string form.
  */
  void setDate(const std::string &isodate) { m_strDate = isodate; };

  /*!
    Assignment
  */
  CMDF_Driver &operator=(const CMDF_Driver &other)
  {
    // Guard self assignment
    if (this == &other) {
      return *this;
    }
    m_strName         = other.m_strName;
    m_strURL          = other.m_strURL;
    m_strType         = other.m_strType;
    m_strOS           = other.m_strOS;
    m_strOSVer        = other.m_strOSVer;
    m_strDate         = other.m_strDate;
    m_strArchitecture = other.m_strArchitecture;
    m_version_major   = other.m_version_major;
    m_version_minor   = other.m_version_minor;
    m_version_patch   = other.m_version_patch;
    m_strMd5          = other.m_strMd5;
    return *this;
  };

  /*!
    Set description for firmware
  */
  void setDescription(const std::string &strDescription, const std::string &strLanguage)
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

  /*!
    Get the value info URL
    @return Value info URL
  */
  std::string getInfoURL(std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set value info URL
    @param lang Language
    @param url Value info URL to set
  */
  void setInfoURL(const std::string lang, const std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get description map
    @return Pointer to description map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get info URL map
    @return Pointer to info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

private:
  /*!
      Unique name for driver
  */
  std::string m_strName;

  /*!
      Path/url to driver file
  */
  std::string m_strURL;

  /*!
    Target system or processor for this driver
  */
  std::string m_strType;

  /*!
    Operation system for driver
  */
  std::string m_strOS;

  /*!
    Operation system version for driver
  */
  std::string m_strOSVer;

  /*!
    Date for object
  */
  std::string m_strDate;

  /*!
    Processor architecture
  */
  std::string m_strArchitecture;

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

// ----------------------------------------------------------------------------

class CMDF_Setup : public CMDF_Object {

public:
  CMDF_Setup();
  ~CMDF_Setup();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get name of setup
    @return name for setup
  */
  std::string getName(void) { return m_strName; };

  /*!
    Set name of setup
    @param strURL name
  */
  void setName(const std::string &name) { m_strName = name; };

  /*!
    Get URL for setup
    @return URL for setup
  */
  std::string getUrl(void) { return m_strURL; };

  /*!
    Set URL for setup
    @param strURL URL for setup
  */
  void setUrl(const std::string &strURL) { m_strURL = strURL; };

  /*!
    Get format for setup
    @return format for setup
  */
  std::string getFormat(void) { return m_strFormat; };

  /*!
    Set format for setup
    @param strFormat Format for setup
  */
  void setFormat(const std::string &strFormat) { m_strFormat = strFormat; };

  /*
    Get date for object
    @return date on ISO string format
  */
  std::string getDate(void) { return m_strDate; };

  /*!
    Set date for object
    @param isodate Date for object oin ISO string form.
  */
  void setDate(const std::string &isodate) { m_strDate = isodate; };

  /*!
    Get version for setup
    @return version for setup
  */
  std::string getVersion(void) { return m_strVersion; };

  /*!
    Set version for setup
    @param strVersion Version for setup
  */
  void setVersion(const std::string &strVersion) { m_strVersion = strVersion; };

  /*!
    Assignment
  */
  CMDF_Setup &operator=(const CMDF_Setup &other)
  {
    // Guard self assignment
    if (this == &other) {
      return *this;
    }
    m_strName    = other.m_strName;
    m_strURL     = other.m_strURL;
    m_strFormat  = other.m_strFormat;
    m_strDate    = other.m_strDate;
    m_strVersion = other.m_strVersion;
    return *this;
  };

  /*!
    Set description for setup
  */
  void setDescription(const std::string &strDescription, const std::string &strLanguage)
  {
    m_mapDescription[strLanguage] = strDescription;
  };

  /*!
    Get description for setup
    @param strLanguage Language for description (default="en")
    @return Description for setup as string
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

  

  /*!
    Get the value info URL
    @return Value info URL
  */
  std::string getInfoURL(std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set value info URL
    @param lang Language
    @param url Value info URL to set
  */
  void setInfoURL(const std::string lang, const std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get description map
    @return Pointer to description map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get info URL map
    @return Pointer to info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

private:
  /*!
    Name of setup file
  */
  std::string m_strName;

  /*!
    Path to setup file
  */
  std::string m_strURL;

  /*!
    setup type
  */
  std::string m_strFormat;

  /*
    Date
  */
  std::string m_strDate;

  /*
     Version
   */
  std::string m_strVersion;

  /*!
      Description of file
  */
  std::map<std::string, std::string> m_mapDescription;
  std::map<std::string, std::string> m_mapInfoURL; // URL for full module information
};

// ---------------------------------------------------------------------------

class CMDF_Manual : public CMDF_Object {

public:
  CMDF_Manual();
  ~CMDF_Manual();

  // Friend declarations
  friend CMDF;
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  /*!
      Clear storage
  */
  void clearStorage(void);

  /*!
    Get name of manual
    @return name for  manual
  */
  std::string getName(void) { return m_strName; };

  /*!
    Set name of manual
    @param strURL manual
  */
  void setName(const std::string &name) { m_strName = name; }

  /*!
    Get URL for manual
    @return URL for manual
  */
  std::string getUrl(void) { return m_strURL; };

  /*!
    Set url
    @param url URL to set.
  */
  void setUrl(const std::string &url) { m_strURL = url; };

  /*!
    Get format for manual
    @return Manual format string.
  */
  std::string getFormat(void) { return m_strFormat; };

  /*!
    Set format
    @param fmt Format to set.
  */
  void setFormat(const std::string &fmt) { m_strFormat = fmt; };

  /*!
    Get language
    @return Manual language string
  */
  std::string getLanguage(void) { return m_strLanguage; };

  /*!
    Set language
    @param lang Language on ISO string form to set.
  */
  void setLanguage(const std::string &lang) { m_strLanguage = lang; };

  /*
    Get date for object
    @return date on ISO string format
  */
  std::string getDate(void) { return m_strDate; };

  /*!
    Set date for object
    @param isodate Date for object oin ISO string form.
  */
  void setDate(const std::string &isodate) { m_strDate = isodate; };

  /*!
    Assignment
  */
  CMDF_Manual &operator=(const CMDF_Manual &other)
  {
    // Guard self assignment
    if (this == &other) {
      return *this;
    }
    m_strName     = other.m_strName;
    m_strURL      = other.m_strURL;
    m_strFormat   = other.m_strFormat;
    m_strDate     = other.m_strDate;
    m_strLanguage = other.m_strLanguage;
    return *this;
  };

  /*!
    Set description for manual item
  */
  void setDescription(const std::string &strDescription, const std::string &strLanguage)
  {
    m_mapDescription[strLanguage] = strDescription;
  };

  /*!
    Get description for manual item
    @param strLanguage Language for description (default="en")
    @return Description for manual as string
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

  /*!
    Get the value info URL
    @return Value info URL
  */
  std::string getInfoURL(std::string lang = "en") { return m_mapInfoURL[lang]; };

  /*!
    Set value info URL
    @param lang Language
    @param url Value info URL to set
  */
  void setInfoURL(const std::string lang, const std::string &url) { m_mapInfoURL[lang] = url; };

  /*!
    Get description map
    @return Pointer to description map
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get info URL map
    @return Pointer to info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

private:
  /*!
    name manual file
  */
  std::string m_strName;

  /*!
    URL for manual file
  */
  std::string m_strURL;

  /*!
    Format for manual / txt, html, pdf etc
  */
  std::string m_strFormat;

  /*!
    Language [format for manual
  */
  std::string m_strLanguage;

  /*!
    Object date
  */
  std::string m_strDate;

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

class CMDF : public CMDF_Object {

public:
  CMDF();
  ~CMDF();

  const uint32_t type = 0;

  // Friend declarations
  friend bool __getBitAttributes(std::deque<CMDF_Bit *> *pbitlist, const char **attr);
  friend bool __getValueAttributes(std::deque<CMDF_Value *> *pvaluelist, const char **attr);
  friend void __startSetupMDFParser(void *data, const char *name, const char **attr);
  friend void __handleMDFParserData(void *data, const XML_Char *content, int length);
  friend void __endSetupMDFParser(void *data, const char *name);

  

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
    Get Module name.
    @return Return string with module name
  */
  std::string getModuleName(void) { return m_name; };

  /*!
    Set Module name.
    @parm Module name to set
  */
  void setModuleName(const std::string &str) { m_name = str; };

  /*!
    Get module model
    @return Return string with module version.
  */
  std::string getModuleModel(void) { return m_strModule_Model; };

  /*!
    Set module model
    @Param Module model as string
  */
  void setModuleModel(const std::string &str) { m_strModule_Model = str; };

  /*!
    Get module change date
    @return Return string with module change date.
  */
  std::string getModuleChangeDate(void) { return m_strModule_changeDate; };

  /*!
    Set module change date
    @pparam str Module change date as ISO date formatted string
  */
  void setModuleChangeDate(const std::string &str) { m_strModule_changeDate = str; };

  /*!
    Get module version
    @return Return string with module version.
  */
  std::string getModuleVersion(void) { return m_strModule_Version; };

  /*!
    Set module version
    @param str Module version on stringt form
  */
  void setModuleVersion(const std::string &str) { m_strModule_Version = str; };

  /*!
    Get module buffer size
    @return Buffer size in bytes.
  */
  uint16_t getModuleBufferSize(void) { return m_module_bufferSize; };

  /*!
    Set module buffer size
    @param size Module buffer size to set
  */
  void setModuleBufferSize(uint16_t size) { m_module_bufferSize = size; };

  /*!
    Get Module copyright.
    @return Return string with module copyright
  */
  std::string getModuleCopyright(void) { return m_copyright; };

  /*!
    Set Module copyright.
    @parm Module copyright to set
  */
  void setModuleCopyright(const std::string &str) { m_copyright = str; };

  /*!
    Return VSCP level for device. NOTE! that
    0 == Level I, 1 == Level II
    @return Return VSCP level for module
  */
  int getModuleLevel(void) { return m_vscpLevel; };

  /*!
    Set VSCP level for device
    @param level VSCP level to set (0=Level I, 1=Level II).
  */
  void setModuleLevel(int level = 0) { m_vscpLevel = level & 1; };

  /*!
    Return VSCP level for device. NOTE! that
    0 == Level I, 1 == Level II (Deprecated)
    @return Return VSCP level for module
  */
  int getLevel(void) { return m_vscpLevel; };

  /*!
    Set VSCP level for device (Deprecated)
    @param level VSCP level to set (0=Level I, 1=Level II).
  */
  void setLevel(int level = 0) { m_vscpLevel = level & 1; };

  /*!
    Get Module description in selected language.
    @param language Language to get description in.
    @return Return string with module description in selected language or
      in english if the language is not available.
  */
  std::string getModuleDescription(const std::string language = "en");

  /*!
    Get the number of module descriptions available in different languages
    @return Number of descriptions available.
  */
  size_t getModuleDescriptionSize(void) { return m_mapDescription.size(); };

  /*!
    Get the module description map
    @return Pointer to module description map.
  */
  std::map<std::string, std::string> *getMapDescription(void) { return &m_mapDescription; };

  /*!
    Get the module info URL map
    @return Pointer to module info URL map
  */
  std::map<std::string, std::string> *getMapInfoUrl(void) { return &m_mapInfoURL; };

  /*!
    Get Module info url in selected language (Deprecated).
    @param language Language for the help resource.
    @return Return string with module info url in selected language or
      in english if the language is not available.
  */
  std::string getModuleHelpUrl(const std::string language = "en") { return getModuleInfoUrl(language); };

  /*!
    Get Module info url in selected language.
    @param language Language for the help resource.
    @return Return string with module info url in selected language or
      in english if the language is not available.
  */
  std::string getModuleInfoUrl(const std::string language = "en");

  /*!
    Get the number of module help URL's available in different mime types (deprecated)
    @return Number of info url's available.
  */
  size_t getModuleHelpUrlCount(void) { return m_mapInfoURL.size(); };

  /*!
    Get the number of module help URL's available in different mime types 
    @return Number of info url's available.
  */
  size_t getModuleInfoUrlCount(void) { return m_mapInfoURL.size(); };

  /*!
    Get the module description map
    Deprecated
  */
  std::map<std::string, std::string> *getHelpUrlMap(void) { return &m_mapInfoURL; };

  /*!
    Get manufacturer object
    @return Pointer to manufacturer object
  */
  CMDF_Manufacturer *getManufacturer(void) { return &m_manufacturer; };

  /*!
    Get module manufacturer
    @return Return string with module manufacturer.
  */
  std::string getManufacturerName(void) { return m_manufacturer.m_strName; };

  /*!
    Get manufacturer street address
    @return Return string with module manufacturer street address.
  */
  std::string getManufacturerStreetAddress(void) { return m_manufacturer.m_address.m_strStreet; };

  /*!
    Get manufacturer city address
    @return Return string with module manufacturer city address.
  */
  std::string getManufacturerCityAddress(void) { return m_manufacturer.m_address.m_strCity; };

  /*!
    Get manufacturer town address
    @return Return string with module manufacturer town address.
  */
  std::string getManufacturerTownAddress(void) { return m_manufacturer.m_address.m_strTown; };

  /*!
    Get manufacturer post code address
    @return Return string with module manufacturer post code address.
  */
  std::string getManufacturerPostCodeAddress(void) { return m_manufacturer.m_address.m_strPostCode; };

  /*!
    Get manufacturer region address
    @return Return string with module manufacturer region address.
  */
  std::string getManufacturerRegionAddress(void) { return m_manufacturer.m_address.m_strRegion; };

  /*!
    Get manufacturer state address
    @return Return string with module manufacturer state address.
  */
  std::string getManufacturerStateAddress(void) { return m_manufacturer.m_address.m_strState; };

  /*!
    Get manufacturer city address
    @return Return string with module manufacturer city address.
  */
  std::string getManufacturerCountryAddress(void) { return m_manufacturer.m_address.m_strCountry; };

  // ---------------------------------------------------------------------------------------------

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
    Get pointer to picture object list
    @return Picture object
  */
  std::deque<CMDF_Picture *> *getPictureObjList(void) { return &m_list_picture; };

  /*!
    Get number of module videos
    @return Number of videos available.
  */
  size_t getVideoCount(void) { return m_list_video.size(); };

  /*!
    Get a video object from it's index
    @param index Index of video to get.
    @return Pointer to video object or NULL if index is out of range.
  */
  CMDF_Video *getVideoObj(uint16_t index = 0)
  {
    return ((m_list_video.size() <= index) ? nullptr : m_list_video[index]);
  };

  /*!
    Get pointer to video object list
    @return Video object
  */
  std::deque<CMDF_Video *> *getVideoObjList(void) { return &m_list_video; };

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
    Get pointer to firmware object
    @return Firmware object
  */
  std::deque<CMDF_Firmware *> *getFirmwareObjList(void) { return &m_list_firmware; };

  /*!
    Get number of driver files
    @return Number of driver available.
  */
  size_t getDriverCount(void) { return m_list_driver.size(); };

  /*!
    Get a driver object from it's index
    @param index Index of driver to get.
    @return Pointer to driver object or NULL if index is out of range.
  */
  CMDF_Driver *getDriverObj(uint16_t index = 0)
  {
    return ((m_list_driver.size() <= index) ? nullptr : m_list_driver[index]);
  };

  /*!
    Get pointer to driver object
    @return Driver object
  */
  std::deque<CMDF_Driver *> *getDriverObjList(void) { return &m_list_driver; };

  /*!
    Get number of setup files
    @return Number of setup available.
  */
  size_t getSetupCount(void) { return m_list_setup.size(); };

  /*!
    Get a setup object from it's index
    @param index Index of setup to get.
    @return Pointer to setup object or NULL if index is out of range.
  */
  CMDF_Setup *getSetupObj(uint16_t index = 0)
  {
    return ((m_list_setup.size() <= index) ? nullptr : m_list_setup[index]);
  };

  /*!
    Get pointer to setup object
    @return Setup object
  */
  std::deque<CMDF_Setup *> *getSetupObjList(void) { return &m_list_setup; };

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
    Get pointer to manual object list
    @return Manual object
  */
  std::deque<CMDF_Manual *> *getManualObjList(void) { return &m_list_manual; };

  // ---------------------------------------------------------------------------------------------

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
    Get the complete register list
    @return Pointer to register list.
  */
  std::deque<CMDF_Register *> *getRegisterObjList(void) { return &m_list_register; };

  /*!
    Delete a defined register
    @param preg Pointer to registere
    @return True on success, false otherwise
  */
  bool deleteRegister(CMDF_Register *preg);

  /*!
    Create a set with sorted register offsets for a page
    @param set a std_set with offset uint32_t items
  */
  void createRegisterSortedSet(std::set<uint32_t> &set, uint16_t page);

  /*!
    Get all registers for a specific page
    @param page Page to get registers for.
    @return Pointer to register definition for page ot a null pointer if page
              does not exist.
  */
  void getRegisterMap(uint16_t page, std::map<uint32_t, CMDF_Register *> &mapRegs);

  /*!
      Get number of defined registers
      @param page Register page to check
      @return Number of registers used.
  */
  size_t getRegisterCount(uint32_t page);

  /*!
    Return register definition from register + page
    @param reg Register to search for.
    @param page Page top search for.
    @return Pointer to CMDF_Register class if found else NULL.
  */
  CMDF_Register *getRegister(uint32_t reg, uint16_t page = 0);

  /*!
    Check if a register position is writable
    @param reg Register to search for.
    @param page Page top search for.
    @return True if register is writable else false.
  */
  bool isRegisterWriteable(uint32_t reg, uint16_t page = 0);

  /*!
    Get default value for register
    @param reg Register to search for.
    @param page Page top search for.
    @return Default value for register or -1 if error.
  */
  int getDefaultRegisterValue(uint32_t reg, uint16_t page = 0);

  /*!
      Get number of register pages used
      @param array Reference to array with pages
      @return Number of register pages used.
  */
  uint32_t getPages(std::set<uint16_t> &pages);

  //-----------------------------------------------------------------------------

  /*!
    Get number of defined remote variables
    @return Number of remote variables.
  */
  size_t getRemoteVariableCount(void) { return m_list_remotevar.size(); };

  /*!
      Return remote variable from its name
      @param name Name of remote variable to search for.
      @return Pointer to CMDF_RemoteVariable class if found, else NULL.
  */
  CMDF_RemoteVariable *getRemoteVariable(const std::string &name);

  /*!
    Return remote variable from its offset/page
      @param offset Offset for start register for remote variable
      @param page Page where remote variable is located
      @return Pointer to CMDF_RemoteVariable class if found, else NULL.
  */
  CMDF_RemoteVariable *getRemoteVariable(uint32_t offset, uint16_t page = 0);

  /*!
    Return remote variable list from its name
    @return Pointer to CMDF_RemoteVariable class list
  */
  std::deque<CMDF_RemoteVariable *> *getRemoteVariableList(void) { return &m_list_remotevar; }; 
  //std::deque<CMDF_RemoteVariable *> *getRemoteVariableObjList(void) { return &m_list_remotevar; };

  /*!
    Delete a defined remote variable
    @param pvar Pointer to remote variable
    @return True on success, false otherwise
  */
  bool deleteRemoteVariable(CMDF_RemoteVariable *pvar);

  //-----------------------------------------------------------------------------

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
    Add event to MDF event list
    @param pEvent Pointer to event to add.
    @return True on success, flase on failure
  */
  bool addEvent(CMDF_Event *pEvent);

  /*!
    Delete event
    @param pEvent Pointer to event to add.
    @return True on success, flase on failure
  */
  bool deleteEvent(CMDF_Event *pEvent);

  /*!
    Get the alarm list
    @return Pointer to the alarm list.
  */
  std::deque<CMDF_Bit *> *getAlarmList() { return &m_list_alarm; };

  /*!
    Get the alarm list bits
    (Alternative consistent with other bit list getters)
    @return Pointer to the alarm list.
  */
  std::deque<CMDF_Bit *> *getAlarmListBits() { return &m_list_alarm; };

  /*!
    Get items from bit list
    @param j JSON object with bit list
    @param list List to store bit items to
    @return true on VSCP_ERROR_SUCCESS on success, error code on failure.
  */
  int getBitList(json &j, std::deque<CMDF_Bit *> &list);

  /*!
    Get items from valulist
    @param j JSON object with valuelist
    @param list List to store valuelist items to
    @return true on VSCP_ERROR_SUCCESS on success, error code on failure.
  */
  int getValueList(json &j, std::deque<CMDF_Value *> &list);

  /*!
    Get items from description list/item
    @param j JSON object with valuelist
    @param list List to store description items to
    @return true on VSCP_ERROR_SUCCESS on success, error code on failure.
  */
  int getDescriptionList(json &j, std::map<std::string, std::string> &map);

  /*!
    Get items from infoURL list/item
    @param j JSON object with valuelist
    @param list List to store infoURL items to
    @return true on VSCP_ERROR_SUCCESS on success, error code on failure.
  */
  int getInfoUrlList(json &j, std::map<std::string, std::string> &map);

  /*
      Return temporary file path
      @return path
  */
  std::string getTempFilePath(void) { return m_tempFileName; };

  /*
    Format a description string.
    The description can be html or markdown coded and will
    always be returned formatted as HTML.
  */
  std::string &format(std::string &docs);

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
  int parseMDF_JSON(const std::string &path);

  /*!
    Parse a MDF. The format can be XML or JSON. If the format is
    unknown, the function will try to determine it.
    @param path Path to downloaded MDF
    @return returns VSCP_ERROR_SUCCESS on success, error code on failure.
  */
  int parseMDF(const std::string &path);

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
  static CURLcode downLoadMDF(const std::string &remoteFile, const std::string &tempFile);

  /*!
    Load MDF from local or remote storage and parse it into
    a MDF structure.
    @param file or URL to MDF file.
    @param blocalFile Asks for a local file if set to true.
    @return returns true on success, false on failure.
  */
  bool load(const std::string &file, bool bLocalFile = false);

  /*!
    Write document/infourl map file in XML format
    @param fout Output stream
    @param pmap Pointer to map to output
    @param tag Tag to use for XML
  */
  void
  writeMap_xml(std::ofstream& fout, std::map<std::string, std::string> *pmap, const std::string& tag);

  /*!
    Write document/infourl map file in JSON format
    @param fout Output stream
    @param pmap Pointer to map to output
    @param tag Tag to use for JSON
    @param bCommaAtEnd Sett to true to poutput comma at the end
  */
  void
  writeMap_json(std::ofstream& fout, std::map<std::string, std::string> *pmap, const std::string& tag, bool bCommaAtEnd = true);

  /*!
    Write MDF to a file in XML format.
    @param path  Path to external file to write to. The file will be 
      created if it does not exist.
    @return returns VSCP_ERROR_SUCCESS on success, error code on failure.  
  */
  int save_xml(const std::string &path);

  /*!
    Write MDF to a file in JSON format.
    @param path  Path to external file to write to. The file will be 
      created if it does not exist.
    @return returns VSCP_ERROR_SUCCESS on success, error code on failure.  
  */
  int save_json(const std::string &path);

  /*!
    Write MDF to a file
    @param path  Path to external file to write to. The file will be 
      created if it does not exist.
    @param format Can be either MDF_FORMAT_XML or MDF_FORMAT_JSON
    @return returns VSCP_ERROR_SUCCESS on success, error code on failure.
  */
 int save(const std::string &path, mdf_format format);


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

  uint8_t m_vscpLevel;                                 // Module level. Default to level I
  std::string m_name;                                  // Module name
  std::string m_copyright;                             // Copyright
  std::map<std::string, std::string> m_mapDescription; // Module description
  std::map<std::string, std::string> m_mapInfoURL;     // URL for full module information

  std::string m_strModule_changeDate; // Last date changed
  std::string m_strModule_Model;      // Module Model
  std::string m_strModule_Version;    // Module version

  uint16_t m_module_bufferSize; // Buffersize for module

  CMDF_Manufacturer m_manufacturer; // Manufacturer information

  // File lists
  std::deque<CMDF_Picture *> m_list_picture;   // Picture file(s)
  std::deque<CMDF_Video *> m_list_video;       // Video file(s)
  std::deque<CMDF_Firmware *> m_list_firmware; // Firmware file(s)
  std::deque<CMDF_Driver *> m_list_driver;     // Picture file(s)
  std::deque<CMDF_Manual *> m_list_manual;     // Manual file(s)
  std::deque<CMDF_Setup *> m_list_setup;       // Setup file(s)

  CMDF_DecisionMatrix m_dmInfo;   // Info about decision matrix
  CMDF_BootLoaderInfo m_bootInfo; // Boot loader info

  std::deque<CMDF_Register *> m_list_register;        // List with defined registers
  std::deque<CMDF_RemoteVariable *> m_list_remotevar; // List with defined remote variables
  std::deque<CMDF_Event *> m_list_event;              // Events this node can generate
  std::deque<CMDF_Bit *> m_list_alarm;                // List with alarm bit defines
};

#endif
