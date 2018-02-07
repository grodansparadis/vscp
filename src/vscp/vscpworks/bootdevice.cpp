/////////////////////////////////////////////////////////////////////////////
// Name:        bootdevice.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     16/12/2009 22:26:09
// RCS-ID:      
// Copyright:   (C) 2009-2018 
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
// 

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "devicebootloaderwizard.h"
#endif

#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "bootdevice.h"


///////////////////////////////////////////////////////////////////////////////
// Ctor
//

CBootDevice::CBootDevice( CDllWrapper *pdll, uint8_t nodeid, bool bDeviceFound )
{
    init();

    m_type = USE_DLL_INTERFACE;
    m_pdll = pdll;
    m_nodeid = nodeid;
    m_bDeviceFound = bDeviceFound;
}

CBootDevice::CBootDevice( VscpRemoteTcpIf *ptcpip, cguid &guid, cguid &ifguid, bool bDeviceFound )
{
    init();

    m_type = USE_TCPIP_INTERFACE;
    m_ptcpip = ptcpip;
    m_guid = guid;
    m_ifguid = ifguid;
    m_bDeviceFound = bDeviceFound;
}


///////////////////////////////////////////////////////////////////////////////
// Dtor
//

CBootDevice::~CBootDevice( void )
{
    if ( NULL != m_pbufPrg ) {
        delete [] m_pbufPrg;
        m_pbufPrg = NULL;
    }

    if ( NULL != m_pbufUserID ) {
        delete [] m_pbufUserID;
        m_pbufUserID = NULL;
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
// init
//

void CBootDevice::init( void )
{
    m_minFlashAddr = 0;
    m_maxFlashAddr = 0xffffffff;
    m_totalCntData = 0;

    m_pbufPrg = NULL;
    m_pbufUserID = NULL;
    m_pbufCfg = NULL;
    m_pbufEEPROM = NULL;

    m_pdll = NULL;
    m_ptcpip = NULL;

    m_bDeviceFound = false;
}

///////////////////////////////////////////////////////////////////////////////
// readRegister
//

bool readRegister(  uint8_t reg, uint8_t *content )
{
    return true;
}


