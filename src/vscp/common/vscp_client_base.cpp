// vscp_base_client.cpp
//
// Virtual base class for client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:   © 2007-2021
// Ake Hedman, Grodans Paradis AB, <akhe@vscp.org>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#ifdef __GNUG__
#pragma implementation
#endif

#include <deque>
#include <list>
#include <string>

#include "vscp_client_base.h"

///////////////////////////////////////////////////////////////////////////////
// CVscpClient
//

CVscpClient::CVscpClient()
{
    m_evcallback = NULL;
    m_excallback = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// ~CVscpClient
//

CVscpClient::~CVscpClient()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int CVscpClient::setCallback(LPFNDLL_EV_CALLBACK evcallback) 
{ 
    m_evcallback = evcallback; 
    return VSCP_ERROR_SUCCESS;
};

///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int CVscpClient::setCallback(LPFNDLL_EX_CALLBACK excallback) 
{ 
    m_excallback = excallback; 
    return VSCP_ERROR_SUCCESS;
};