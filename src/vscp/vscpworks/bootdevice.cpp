/////////////////////////////////////////////////////////////////////////////
// Name:        bootdevice.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     16/12/2009 22:26:09
// RCS-ID:      
// Copyright:   (C) 2009-2015 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
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
//  Grodans Paradis AB at info@grodansparadis.com, http://www.grodansparadis.com
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "devicebootloaderwizard.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes


#include "bootdevice.h"

CBootDevice::CBootDevice( CCanalSuperWrapper *pcsw, cguid &guid )
{
    m_minFlashAddr = 0;
	m_maxFlashAddr = 0xffffffff;
	m_totalCntData = 0;

    m_pbufPrg = NULL;
    m_pbufCfg = NULL;
    m_pbufEEPROM = NULL;

    m_responseTimeout = BOOT_COMMAND_DEFAULT_RESPONSE_TIMEOUT;

    m_pCanalSuperWrapper = pcsw;

}

CBootDevice::~CBootDevice(void)
{
    if ( NULL != m_pbufPrg ) {
        delete [] m_pbufPrg;
        m_pbufPrg = NULL;
    }

    if ( NULL != m_pbufCfg ) {
        delete [] m_pbufCfg;
        m_pbufCfg = NULL;
    }

    if ( NULL != m_pbufEEPROM ) {
        delete [] m_pbufEEPROM;
        m_pbufEEPROM = NULL;
    }

}

///////////////////////////////////////////////////////////////////////////////
// readRegister
//

bool readRegister(  uint8_t reg, uint8_t *content )
{
    return true;
}


