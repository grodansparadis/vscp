/////////////////////////////////////////////////////////////////////////////
// Name:        vscpbootloader.cpp
// Purpose:
// Author:      Ake Hedman
// Modified by:
// Created:     Thu 25 Oct 2007 22:32:58 CEST
// RCS-ID:
// Copyright:  (C) 2007-2024
// Ake Hedman, the VSCP project, <info@vscp.org>
//				(C) 2012 Dinesh Guleria
// Licence:
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
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

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "vscpboot_loader.h"
#endif

#include "vscp_bootloader.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// VSCPBootloader constructors
//

VSCPBootloader::VSCPBootloader()
{
    init();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// VSCPBootloader destructor
//

VSCPBootloader::~VSCPBootloader() {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// init
//

void
VSCPBootloader::init()
{}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Runs the wizard.
//

bool
VSCPBootloader::run()
{
    return false;
}
