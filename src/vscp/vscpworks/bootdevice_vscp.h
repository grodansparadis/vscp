/////////////////////////////////////////////////////////////////////////////
// Name:        bootdevice_vscp.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     16/12/2009 22:26:09
// RCS-ID:      
// Copyright:   (C) 2007-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// Copyright (C) 2000-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// 
// This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.
// 
//  As a special exception, if other files instantiate templates or use macros
//  or inline functions from this file, or you compile this file and link it
//  with other works to produce a work based on this file, this file does not
//  by itself cause the resulting work to be covered by the GNU General Public
//  License. However the source code for this file must still be made available
//  in accordance with section (3) of the GNU General Public License.
// 
//  This exception does not invalidate any other reasons why a work based on
//  this file might be covered by the GNU General Public License.
// 
//  Alternative licenses for VSCP & Friends may be arranged by contacting 
//  Grodans Paradis AB at info@Grodans Paradis AB.se, http://www.grodansparadis.com
/////////////////////////////////////////////////////////////////////////////


#pragma once
#include "bootdevice.h"

class CBootDevice_VSCP :
    public CBootDevice
{
public:
    CBootDevice_VSCP( CCanalSuperWrapper *pcsw, cguid &guid );
    ~CBootDevice_VSCP(void);

    /*!
		Load a binary file to the image

        This is typically an Intel HEX file that contains the memory
        image of the device.

		@param path Path to file
		@return true on success 
	*/
    bool loadBinaryFile( const wxString& path, uint16_t type );

    /*!
        Show info for hex file
        @param Pointer to HTML window that will receive information.
    */
    void showInfo( wxHtmlWindow *phtmlWnd );


    /*!
		Set a device in bootmode
        @return true on success.
	*/
	bool setDeviceInBootMode( void );

    /*!
        Perform the actual boot process
        @return true on success.
    */
    bool doFirmwareLoad( void );

    /*!
		Check for response from nodes.

		This routine is used as a check for response from nodes under boot. 
		Node id's must have been added using addClient.
		The supplied id is valid from bit 8 and upwards. The lower eight bits
		are the id.
		Only extended messages are accepted as a valid response.

		@param id Response code to look for.
		@return true on success.
	*/
	bool checkResponseLevel1( unsigned long id );
};
