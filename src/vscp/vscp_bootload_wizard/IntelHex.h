// IntelHex.h: interface for the CIntelHex class.
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
// $RCSfile: IntelHex.h,v $                                       
// $Date: 2005/01/05 12:50:58 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTELHEX_H__77CA0749_2142_4A19_B35F_109D79EA1743__INCLUDED_)
#define AFX_INTELHEX_H__77CA0749_2142_4A19_B35F_109D79EA1743__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEMREG_PRG_START		0x000000
#define MEMREG_PRG_END			0x2fffff

#define MEMREG_CONFIG_START		0x300000
#define MEMREG_CONFIG_END		0x3fffff

#define MEMREG_EEPROM_START		0xf00000
#define MEMREG_EEPROM_END		0xffffff

#define BUFFER_SIZE_PROGRAM		0x10000
#define BUFFER_SIZE_CONFIG		0x2000
#define BUFFER_SIZE_EEPROM		0x400


#define INTEL_LINETYPE_DATA				0	// Data record.
#define INTEL_LINETYPE_EOF				1	// End Of File record.
#define INTEL_LINETYPE_EXTENDED_SEGMENT	2	// Extended segment address	record.
#define INTEL_LINETYPE_RESERVED			3	// Not used
#define INTEL_LINETYPE_EXTENDED_LINEAR	4	// Extended linear address record.	

#ifndef MAX_PATH
#define MAX_PATH	128
#endif

class CIntelHex  
{

public:

	CIntelHex();

	virtual ~CIntelHex();

	/*!
		Load Intel HEX data from file to buffer
	*/
	bool load( const char *fname );

	/*!
		Check validity for Intel HEX file
	*/
	bool checkFile( const char *fname );

public:

	/*!
		Flag for flash memory handling	
	*/
	bool m_bFlashMemory;

	/*!
		Flag for config memory handling	
	*/
	bool m_bConfigMemory;

	/*!
		Flag for EEPROM memory handling	
	*/
	bool m_bEEPROMMemory;


public:

	/// Program memory buffer
	unsigned char m_bufPrg[ BUFFER_SIZE_PROGRAM ];

	/// Config memory buffer
	unsigned char m_bufCfg[ BUFFER_SIZE_CONFIG ];

	/// EEPROM memory buffer
	unsigned char m_bufEEPROM[ BUFFER_SIZE_EEPROM ];

	/// True if there is at least one program data byte
	bool m_bPrgData;

	/// True if there is at least one config data byte
	bool m_bConfigData;

	/// True if there is at least one EEPROM data byte
	bool m_bEEPROMData;

	/// Lowest flash address
	unsigned long m_minFlashAddr;

	/// Highest flash address
	unsigned long m_maxFlashAddr;

	/// Lowest config address
	unsigned long m_minConfigAddr;

	/// Highest config address
	unsigned long m_maxConfigAddr;

	/// Lowest EEPROM address
	unsigned long m_minEEPROMAddr;

	/// Highest EEPROM address
	unsigned long m_maxEEPROMAddr;

private:
	
	// Pointer into buffer
	unsigned long m_ptr;
		

	/// # data bytes in file
	unsigned long m_totalCntData;

};

#endif // !defined(AFX_INTELHEX_H__77CA0749_2142_4A19_B35F_109D79EA1743__INCLUDED_)
