// randPassword.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2011 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "randpassword.h"

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

randPassword::randPassword( unsigned char type )
{
    memset( m_pool, 0, sizeof( m_pool ) );
    generatePool( type, m_pool );
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

randPassword::~randPassword()
{

}

///////////////////////////////////////////////////////////////////////////////
// generatePassword
//

void randPassword::generatePassword( unsigned char length, char *pPassword )
{
    memset( pPassword, 0, length + 1 );
    srand( (unsigned)time( NULL ) );
    while( strlen( pPassword ) < length ) {  
        pPassword[ strlen( pPassword ) ] = m_pool[ rand() % strlen( m_pool ) ];  
        pPassword[ strlen( pPassword ) ] = 0;
    } 
}


///////////////////////////////////////////////////////////////////////////////
// randPassword
//

char *randPassword::generatePool( unsigned char type, char *pstr )  
{  
    int i;
    char buf[ 256 ];

    memset( buf, 0, sizeof( buf ) );

    switch ( type ) { 

        case 1:     // a - z
            for ( i = 0x61; i <= 0x7A; i++ )  {  
                pstr[ strlen( pstr ) ] = (char)i;  
            }  
            break;

        case 2:     // A - Z   
            for (i = 0x41; i <= 0x5A; i++ ) {  
                pstr[ strlen( pstr ) ] = (char)i;  
            }  
            break;

        case 3:     // a - z and A - Z      
            strcat( pstr, generatePool( 1, buf ) );  
            strcat( pstr, generatePool( 2, buf ) );  
            break;

        case 4:     // 0 - 9, A - Z and a - z 
            strcat( pstr, generatePool ( 3, buf ) );    // get chars a - z and A - Z first  
            for (i = 0x30; i <= 0x39; i++ ) {  
                pstr[ strlen( pstr ) ] = (char)i; // add chars 0 - 9;  
            }  
            break;  
                        
        case 5:  
            // This will add these chars into the string !#$%&() 
            strcat( pstr, generatePool( 4, buf ) );  
            for ( i = 0x21; i < 0x29; i++ ) {  
                if ( i == 0x22 || i == 0x27) continue;  // Exclude characters " and '  
                pstr[ strlen( pstr ) ] = (char)i;  
            }  
            break;  

    } 

    return pstr;

}

