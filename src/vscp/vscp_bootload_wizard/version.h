/////////////////////////////////////////////////////////////////////////////
// Name:        version.h
// Purpose:     VSCP Bootloader wizard
// Author:      Ake Hedman, akhe@eurosource.se
// Modified by:
// Created:     2005-07-24
// RCS-ID:      $Id: version.h,v 1.1 2005/07/24 21:20:29 akhe Exp $
// Copyright:   Copyright (c)  2004-2010 Ake Hedman, akhe@eurosource.se
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
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
// $RCSfile: version.h,v $                                       
// $Date: 2005/07/24 21:20:29 $                                  
// $Author: akhe $                                              
// $Revision: 1.1 $ 


#define VERSION		"0.2.0"


// History
// =======
//
// 2005-07-24 Version 0.2.0 AKHE
// Full support for the VSCP bootload protocol type PIC is now added. The program can bring
// a node with VSCP into bootmode and update its firmware.
// The bootloader also can load firmware into a device that is already in bootloader mode.
// The state for the node is detected by the program itself.