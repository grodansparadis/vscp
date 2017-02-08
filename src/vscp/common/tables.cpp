// tables.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// Copyright (C) 2000-2017 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
    //#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <sys/stat.h>

#include "wx/defs.h"
#include "wx/app.h"
#include <wx/datetime.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

#include <wx/listimpl.cpp>

#ifndef WIN32
#include <errno.h>
#endif

#include <vscp.h>
#include <vscphelper.h>
#include <variablecodes.h>
#include <tables.h>


// This list holds the tables for vscpd
WX_DEFINE_LIST( listVSCPTables );


///////////////////////////////////////////////////////////////////////////////
// Constructor
//

// Constructor for nmormal table
CVSCPTable::CVSCPTable( const char *path, int type, uint32_t size )
{   
    
}


///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVSCPTable::~CVSCPTable( void )
{
    
}


///////////////////////////////////////////////////////////////////////////////
// init
//

bool CVSCPTable::init() 
{
    
    return true; 
}

///////////////////////////////////////////////////////////////////////////////
// setTableInfo
//

int CVSCPTable::setTableInfo( const char *path,
                                    uint8_t type,
                                    const char *tableName, 
                                    const char *tableDescription,
                                    const char *xAxisLabel, 
                                    const char *yAxisLabel,
                                    uint32_t size,
                                    uint16_t vscp_class, 
                                    uint16_t vscp_type,
                                    uint8_t vscp_unit )
{
  
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// fileExists
//

int CVSCPTable::fileExists( const char *path )
{
    struct stat buffer;
    return ( stat(path, &buffer) == 0);
}

///////////////////////////////////////////////////////////////////////////////
// getRangeOfData
//

long CVSCPTable::getRangeOfData( uint64_t start, uint64_t end, void *buf, uint16_t size )
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// getRangeOfData
//

long CVSCPTable::getRangeOfData( wxDateTime& wxStart, 
                                    wxDateTime& wxEnd, 
                                    void *buf, 
                                    uint16_t size )
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// getRangeOfData
//

long CVSCPTable::getRangeOfData( uint32_t startpos, 
                                    uint16_t count, 
                                    struct _vscpFileRecord *buf )
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// getStaticData
//

long CVSCPTable::getStaticData( void *buf, uint16_t size )
{

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// getStaticRequiredBuffSize
//

long CVSCPTable::getStaticRequiredBuffSize( void )
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// logData
//

long logData( unsigned long time, double value )
{
    return 0;
}


