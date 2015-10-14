// FILE: canalobj.h 
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2015 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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

#pragma once

#ifndef _CANALOBJ_H_
#define _CANALOBJ_H_

#include "wx/wx.h"
#include <wx/window.h>

#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>
#include <canal.h>
#include <canaldlldef.h>
#include <dllwrapper.h>
#include <crc.h>

enum canalobj_type {
    type_unknown,
    type_string,
    type_boolean,
    type_int32,
    type_uint32,
    type_float,
    type_choice,
    type_isotime,
    type_isodate,
    type_isodatetime,
    type_guid,
    type_event,
    type_filter,
    type_mask
};

enum typeDescription {
    type_text,
    type_html
};

enum flagtype {
    flagtype_bool,
    flagtype_value,
    flagtype_choice
};


/*!
    CCanalObj_Choice
*/

class CCanalObj_Choice
{

public:

    CCanalObj_Choice();
    ~CCanalObj_Choice();

    wxString m_value;             // The value for this choice
    wxString m_description;       // User description of item

};

WX_DECLARE_LIST( CCanalObj_Choice, CANALOBJ_CHOICE_LIST );

/*!
    CCanalObj_OneItem

    Encapsulates one item of the configuration string
*/

class CCanalObj_OneItem
{

public:

    CCanalObj_OneItem();
    ~CCanalObj_OneItem();

    uint16_t m_pos;             // Position in configuration string (base = 0)
    bool m_bOptional;           // True if optional
    canalobj_type m_type;       // Type for this
    wxString m_description;     // User description of item
    wxString m_infourl;         // Pointer to web-based help page

    CANALOBJ_CHOICE_LIST m_listChoice;  // List with choices if type = choice
    wxString m_strValue;                // Value (always in string from)

};

WX_DECLARE_LIST( CCanalObj_OneItem, CANALOBJ_ITEM_LIST );

/*!
CCanalObj_OneItem

Encapsulates one item of the configuration string
*/

class CCanalObj_FlagBit
{

public:

    CCanalObj_FlagBit();
    ~CCanalObj_FlagBit();

    uint16_t m_pos;             // Position in configuration string (base = 0)
    uint8_t m_width;            // Bitwidth
    wxString m_description;     // User description of item
    wxString m_infourl;         // Pointer to web-based help page
    flagtype m_type;            // Type for flag bool, value or choice
    uint32_t m_defaultVal;      // Default value for 'value' type

    CANALOBJ_CHOICE_LIST m_listChoice;  // List with choices if type = choice

};

WX_DECLARE_LIST( CCanalObj_FlagBit, CANALOBJ_FLAGBIT_LIST );

/*!
    CCanalObj_Item
*/

class CCanalObj_Item
{

public:

    CCanalObj_Item();
    ~CCanalObj_Item();

    /*!
        Parse drver info
        @param  info Driver info fetched from file
        @return true if the parsing went well.
    */
    bool parseDriverInfo( wxString& path );

    /*!
        Run the graphical wizard that assist the user in setting the parameters.
        @return true if the parsing went well.
    */
    bool runWizard( wxWindow *pwnd, wxString& resultConfigString, wxString& resultConfigFlags );

    wxString m_strDecription;           // Description of DLL
    typeDescription m_TypeDescription;  // Format for description (text or html)
    wxString m_strInfoUrl;              // Pointer to site with info about driver

    int m_level;                        // Level for driver
    bool m_bBlocking;                   // True if driver is blocking

    CDllWrapper m_dll;

    CANALOBJ_ITEM_LIST m_listItem;
    CANALOBJ_FLAGBIT_LIST m_listFlagBits;
};



#endif


