// IntelHex.cpp: implementation of the CIntelHex class.
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
// $RCSfile: IntelHex.cpp,v $                                       
// $Date: 2005/01/05 12:50:58 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 
//////////////////////////////////////////////////////////////////////

#include "IntelHex.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIntelHex::CIntelHex()
{
	unsigned long i;
	m_ptr = 0;
	m_minFlashAddr = 0;
	m_maxFlashAddr = 0xffffffff;
	m_totalCntData = 0;

	// Init Program memory buffer
	for ( i = 0; i < BUFFER_SIZE_PROGRAM; i++ ) {
		m_bufPrg[ i ] = 0xff;
	}

	// Init Config memory buffer
	for ( i = 0; i < BUFFER_SIZE_CONFIG; i++ ) {
		m_bufCfg[ i ] = 0xff;
	}

	// Init EEPROM memory buffer
	for ( i = 0; i < BUFFER_SIZE_EEPROM; i++ ) {
		m_bufEEPROM[ i ] = 0xff;
	}

	m_bPrgData = false;
	m_bConfigData = false;
	m_bEEPROMData = false;

}

CIntelHex::~CIntelHex()
{

}


///////////////////////////////////////////////////////////////////////////////
// load
//

bool CIntelHex::load( const char *fname )
{
	unsigned int i;
	bool rv = false;
	FILE *fs;
	unsigned long fullAddr = 0;
	unsigned long highAddr = 0;
	unsigned long lowAddr = 0;
	unsigned long cntData = 0;
	unsigned long recType = 0;

	// Init. flash memory pointers
	m_minFlashAddr = 0xffffffff;
	m_maxFlashAddr = 0;
	m_totalCntData = 0;

	// Init. config memory pointers
	m_minConfigAddr = 0xffffffff;
	m_maxConfigAddr = 0;

	// Init. EEPROM memory pointers
	m_minEEPROMAddr = 0xffffffff;
	m_maxEEPROMAddr = 0;
	
	if ( NULL == ( fs  = fopen( fname, "r" ) ) ) {
		return false;
	}	

	char szLine[ MAX_PATH ];
	char szData[ 16 ];
	char *endptr;

	bool bRun = true;
	while ( bRun && ( NULL != fgets( szLine, MAX_PATH, fs ) ) ) {
		
		if ( ':' == szLine [ 0 ] ) {

			// Get data count
			memset( szData, 0, strlen( szData ) );
			strncpy( szData, ( szLine + 1 ), 2 );
			cntData = strtoul( szData, &endptr, 16 );
			m_totalCntData += cntData;

			// Get address
			memset( szData, 0, strlen( szData ) );
			strncpy( szData, ( szLine + 3 ), 4 );
			lowAddr = strtoul( szData, &endptr, 16 );

			// Get record type
			memset( szData, 0, strlen( szData ) );
			strncpy( szData, ( szLine + 7 ), 2 );
			recType = strtoul( szData, &endptr, 16 );

			fullAddr = ( highAddr * 0xffff ) + lowAddr;			

			// Decode the record type
			switch ( recType ) {
				
				case INTEL_LINETYPE_DATA:

					for ( i=0; i < cntData; i++ ) {
											
						memset( szData, 0, strlen( szData ) );
						strncpy( szData, ( szLine + ( ( i * 2 ) + 9 ) ), 2 );
						unsigned char val = (unsigned char)( strtoul( szData, &endptr, 16 ) & 0xff);

						if ( ( fullAddr < MEMREG_PRG_END ) && ( fullAddr < BUFFER_SIZE_PROGRAM ) ) {
							
							// Write into program memory buffer
							m_bufPrg[ fullAddr ] = val;
							m_bPrgData = true;

							// Set min flash address
							if ( fullAddr < m_minFlashAddr ) m_minFlashAddr = fullAddr;

							// Set max flash address
							if ( fullAddr > m_maxFlashAddr ) m_maxFlashAddr = fullAddr;
								
						}
						else if ( ( fullAddr >= MEMREG_CONFIG_START ) && 
									( ( fullAddr < MEMREG_CONFIG_START + BUFFER_SIZE_CONFIG ) ) ) {
							
							// Write into config memory buffer
							m_bufCfg[ fullAddr - MEMREG_CONFIG_START ] = val;
							m_bConfigData = true;

							// Set min config address
							if ( fullAddr < m_minConfigAddr ) m_minConfigAddr = fullAddr;

							// Set max config address
							if ( fullAddr > m_maxConfigAddr ) m_maxConfigAddr = fullAddr;

						}
						else if ( ( fullAddr >= MEMREG_EEPROM_START ) && 
									( ( fullAddr <= MEMREG_EEPROM_START + BUFFER_SIZE_EEPROM ) ) ) {

							// Write into EEPROM memory buffer
							m_bufEEPROM[ fullAddr - MEMREG_EEPROM_START ] = val;
							m_bEEPROMData = true;

							// Set min EEEPROM address
							if ( fullAddr < m_minEEPROMAddr ) m_minEEPROMAddr = fullAddr;

							// Set max config address
							if ( fullAddr > m_maxEEPROMAddr ) m_maxEEPROMAddr = fullAddr;

						}

						fullAddr = fullAddr + 1;

					}
					break;

				case INTEL_LINETYPE_EOF:
					bRun = false;	// We are done
					rv = true;
					break;

				case INTEL_LINETYPE_EXTENDED_SEGMENT:
					// We don't handle this
					break;

				case INTEL_LINETYPE_EXTENDED_LINEAR :
					memset( szData, 0, strlen( szData ) );
					strncpy( szData, ( szLine + 9 ), 4 );
					highAddr = strtoul( szData, &endptr, 16 );
					break;

			}
			
		}
	}

	fclose( fs );

	return rv;
}


