/////////////////////////////////////////////////////////////////////////////
// Name:        vscpbootloader.h
// Purpose:
// Author:      Ake Hedman
// Modified by:
// Created:     Thu 25 Oct 2007 22:32:58 CEST
// RCS-ID:
// Copyright:   (C) 2007-2019
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//              (C) 2012 Dinesh Guleria
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
//  eurosource at info@eurosource.se, http://www.eurosource.se
/////////////////////////////////////////////////////////////////////////////

#ifndef _VSCPBOOTLOADER_H_
#define _VSCPBOOTLOADER_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "vscpbootloader.h"
#endif

/*!
 * Control identifiers
 */

/*!
 * VSCPBootloader class declaration
 */

class VSCPBootloader : public wxWizard
{

  public:
    /// Constructors
    VSCPBootloader();

    /// Destructor
    ~VSCPBootloader();

    /// Initialises member variables
    void init();

    /// Runs the wizard
    bool run();
};

#endif
// _VSCPBOOTLOADER_H_
