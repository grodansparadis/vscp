// FILE: mdf.h 
//
// Copyright (C) 2002-2012 Ake Hedman akhe@grodansparadis.com 
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//


#ifndef _MDF_H_
#define _MDF_H_

#include "wx/wx.h"
#include "vscp.h"
#include "vscp_class.h"
#include "vscp_type.h"
#include "canal.h"
#include "../../common/crc.h"

#define MDF_ACCESS_READ		1
#define MDF_ACCESS_WRITE	2

enum vscp_level {
  level1=1,
  level2
};


enum vscp_abstraction_type {
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
	type_guid
};


WX_DEFINE_ARRAY_LONG( uint32_t, SortedArrayLong );


// * * * Settings * * *


/*!
  CMDF_ValueListValue

 */

class CMDF_ValueListValue
{  

public:

    CMDF_ValueListValue();
    ~CMDF_ValueListValue();

    wxString m_strName;
    wxString m_strDescription;
    wxString m_strHelpType;
    wxString m_strHelp;             // Item help text or url
    wxString m_strValue;            // The abstraction tells the type
  
};

WX_DECLARE_LIST( CMDF_ValueListValue, MDF_VALUE_LIST );

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
    void clearStorage( void );

	/*!
		Get real text escription of type
		@param type Abstraction type
		@return Real text description of type.
	*/
	wxString getAbstractionValueType( void );

    wxString m_strName;
    wxString m_strDescription;
    wxString m_strHelpType;
    wxString m_strHelp;                 // Item help text or url

    wxString m_strID;                   // Abstract variable id (unique
                                        // inside of MDF
    vscp_abstraction_type  m_nType;     // One of the predefined types
    wxString m_strDefault;              // default value
  
    uint16_t m_nPage;                   // stored on this page
    uint32_t m_nOffset;                 // stored at this offset
    uint16_t m_nBitnumber;              // Stored at this bit position.
  
    uint16_t m_nWidth;                  // Width for bitfield and strings.

    uint32_t m_nMax;                    // If numeric max value can be set
    uint32_t m_nMin;                    // If numeric min value can be set
  
    uint8_t m_nAccess;                  // Access rights

	bool m_bIndexed;					// True of indexed storage

    MDF_VALUE_LIST  m_list_value;       // list with selectable values

};

WX_DECLARE_LIST( CMDF_Abstraction, MDF_ABSTRACTION_LIST );



// * * * Register * * *


/*!
  CMDF_Bit

  This represents a bit or a bitfield. A valuelist can be 
  defined to descrive the bit combinations.

 */

class CMDF_Bit
{  

public:

    CMDF_Bit();
    ~CMDF_Bit();


    wxString m_strName;
    wxString m_strDescription;

    /*!
        Clear storage
    */
    void clearStorage( void );

    wxString m_strHelpType;
    wxString m_strHelp;                 // Item help text or url

    // The following is used if the abstraction is a bit or bitfield
    uint8_t m_nPos;                     // 'pos'		position in bit field 0-7 (from the left)
    uint8_t m_nWidth;                   // 'width'		1 for one bit 2-8 for bit-field
    uint8_t m_nDefault;                 // 'default'	Default value for field

    uint8_t m_nAccess;                  // 'access'		Access rights for the bit(-field)

    MDF_VALUE_LIST  m_list_value;       // List with selectable values
  
};

WX_DECLARE_LIST( CMDF_Bit, MDF_BIT_LIST );

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
    void clearStorage( void );

    wxString m_strName;
    wxString m_strDescription;
    wxString m_strHelpType;
    wxString m_strHelp;                 // Item help text or url
  
    uint16_t m_nPage;
    uint16_t m_nOffset;
    uint16_t m_nWidth;					// Defaults to 1

    uint32_t m_nMin;
    uint32_t m_nMax;

    wxString m_strDefault;

    uint8_t m_nAccess;

    MDF_BIT_LIST  m_list_bit;			// dll list with bit defines
    MDF_VALUE_LIST  m_list_value;		// dll list with selectable values

	// For VSCP Works
	uint8_t m_value;					// Initial value read. This is the value
										// that will be restored.

};

WX_DECLARE_LIST( CMDF_Register, MDF_REGISTER_LIST );

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
    void clearStorage( void );

    wxString m_strName;
    wxString m_strDescription;
    wxString m_strHelpType;
    wxString m_strHelp;                 // Item help text or url

    uint16_t m_nOffset;
    uint8_t  m_width;

    MDF_BIT_LIST  m_list_bit;        // List with bit defines
    MDF_VALUE_LIST  m_list_value;    // List with selectable values

};

WX_DECLARE_LIST( CMDF_ActionParameter, MDF_ACTIONPARAMETER_LIST );

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
    void clearStorage( void );

    wxString m_strName;
    wxString m_strDescription;
    wxString m_strHelpType;
    wxString m_strHelp;                 // Item help text or url

    uint16_t m_nCode;

    MDF_ACTIONPARAMETER_LIST  m_list_ActionParameter;   // List with action parameters

};

WX_DECLARE_LIST( CMDF_Action, MDF_ACTION_LIST );

/*!
  CMDF_DecisionMatrix

  Holds information about the capabilities of a system
  decission matrix.
 */

class CMDF_DecisionMatrix
{  

public:

    CMDF_DecisionMatrix();
    ~CMDF_DecisionMatrix();

    /*!
        Clear storage
    */
    void clearStorage( void );

    uint8_t  m_nLevel;        // 1 or 2 (defults to 1)
    uint16_t m_nStartPage;
    uint16_t m_nStartOffset;
    uint16_t m_nRowCount;
    uint16_t m_nRowSize;
	bool m_bIndexed;

    MDF_ACTION_LIST  m_list_action; // Action description

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
    void clearStorage( void );

    wxString m_strName;
    wxString m_strDescription;
    wxString m_strHelpType;
    wxString m_strHelp;                 // Item help text or url

    uint16_t m_nOffset;

    MDF_BIT_LIST  m_list_bit;         // List with bit defines
    MDF_VALUE_LIST  m_list_value;     // List with selectable values

};

WX_DECLARE_LIST( CMDF_EventData, MDF_EVENTDATA_LIST );

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
    void clearStorage( void );

    wxString m_strName;
    wxString m_strDescription;
    wxString m_strHelpType;
    wxString m_strHelp;                 // Item help text or url

    uint16_t m_nClass;
    uint16_t m_nType;
    uint8_t  m_nPriority;

    MDF_EVENTDATA_LIST  m_list_eventdata; // List with ecent data descriptions

};

WX_DECLARE_LIST( CMDF_Event, MDF_EVENT_LIST );


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

    wxString m_strItem;    
    wxString m_strDescription;
    wxString m_strHelpType;
    wxString m_strHelp;                 // Item help text or url
};

WX_DECLARE_LIST( CMDF_Item, MDF_ITEM_LIST );

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
    void clearStorage( void );

    uint8_t  m_nAlgorithm;				// Bootloader algorithm used by device
    uint32_t m_nBlockSize;				// Size for one boot block
    uint32_t m_nBlockCount;				// Number of boot blocks

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
    void clearStorage( void );

    wxString m_strStreet;    
    wxString m_strTown;
    wxString m_strCity;
    wxString m_strPostCode;
    wxString m_strState;
    wxString m_strRegion;
    wxString m_strCountry;
  
};

WX_DECLARE_LIST( CMDF_Address, MDF_ADDRESS_LIST );

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
    void clearStorage( void );

    wxString m_strName;
  
    MDF_ADDRESS_LIST m_list_Address;
  
    MDF_ITEM_LIST m_list_Phone;
    MDF_ITEM_LIST m_list_Fax;
    MDF_ITEM_LIST m_list_Email;
    MDF_ITEM_LIST m_list_Web;
};

WX_DECLARE_LIST( CMDF_Manufacturer, MDF_MANUFACTURER_LIST );

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
    void clearStorage( void );

    /*!
        Path to firmware hex file
    */
    wxString m_strPath;

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
    wxString m_description;
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
    void clearStorage( void );

    /*!
        Download MDF file from a server
        @param remoteFile remote file URL
        @param variable that will receive temporary filename for downloaded file.
        @return Return true if a valid file is downloaded.			
    */
    bool downLoadMDF( wxString& remoteFile, wxString& tempFile );

	/*!
		Load MDF from local or remote storage and parse it into
		a MDF stucture.
		@param Filename or URL to MDF file. If emtpty and bSilent is false
				the method will ask for this parameter.
		@param bSilent No dialogs are shown if set to true.
		@param blocalFile Asks for a local file if set to true.
		@return returns true on success, false on falure.
	*/
	bool load( wxString& remoteFile, bool bLocalFile = false, bool bSilent = false );

    /*!
        Format an MDF description so it can be shown
        @param str String to format.
    */
    void mdfDescriptionFormat( wxString& str );

         
    /*!
        Parse a MDF
        @param path Path to downloaded MDF
        @return true if the parsing went well.
    */
    bool parseMDF( wxString& path );


	// Helpers

	/*!
		Get number of defined registers
		@param page Register page to check
		@return Number of registers used.
	*/
	uint32_t getNumberOfRegisters( uint32_t page );

	/*!
		Get number of register pages used
		@return Number of regsiter pages used.
	*/
	uint32_t getPages( SortedArrayLong& arraylong );

	/*!
		Return register class from register + page
		@param register Register to search for.
		@param page Page top search for.
		@return Pointer to CMDF_Register class if found else NULL.
	*/
	CMDF_Register *getMDFRegs( uint8_t reg, uint16_t page );


    wxString m_strLocale;                       // ISO code for requested language
                                                // defaults to "en"

    wxString m_strModule_Name;                  // Module name
    wxString m_strModule_Model;                 // Module Model
    wxString m_strModule_Version;               // Module version
    wxString m_strModule_Description;           // Module description
    wxString m_strModule_InfoURL;               // URL for full module information
    wxString m_changeDate;                      // Last date changed
    uint16_t m_Module_buffersize;               // Buffersize for module

    wxString m_strURL;                          // Location for MDF file

    CMDF_Firmware m_firmware;                   // Firmware file
  
    MDF_MANUFACTURER_LIST m_list_manufacturer;  // Manufacturer information

    CMDF_DecisionMatrix m_dmInfo;               // Info about decision matrix
    CMDF_BootLoaderInfo m_bootInfo;             // Boot loader info
    
    MDF_EVENT_LIST m_list_event;                // Events this node can generate
    MDF_REGISTER_LIST m_list_register;          // List with defined registers
    MDF_ABSTRACTION_LIST m_list_abstraction;    // List with defined abstractions
    MDF_BIT_LIST m_list_alarmbits;              // List with alarm bit defines
};

#endif


