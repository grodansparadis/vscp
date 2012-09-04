/*
 * Winamp httpQ Plugin
 * Copyright (C) 1999-2003 Kosta Arvanitis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Kosta Arvanitis (karvanitis@hotmail.com)
 */

#include "stdafx.h"
#include "gen_winampvscp.h"
#include "array.h"
#include "configinfo.h"


//--------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------
ConfigInfo::ConfigInfo()
{
    dwVersion =          APPVERSION;

}


//--------------------------------------------------
// Assignment operator
//--------------------------------------------------
ConfigInfo& ConfigInfo::operator=(const ConfigInfo &info)
{
    if (this == &info) 
        return *this;

    this->dwVersion =           info.dwVersion;
    this->dwServerAddress =     info.dwServerAddress;
	this->nPort =               info.nPort;
    
    strcpy(this->strPassword, info.strPassword);

	return *this;
}


ConfigInfo::~ConfigInfo()
{
}

