// randPassword.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

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
    static int seed = 8;
    int len = strlen( m_pool );
    int cnt = 0;
    memset( pPassword, 0, length );
    srand( (unsigned)time( NULL ) + seed++ );
    while( cnt < length ) {  
        pPassword[ cnt++ ] = m_pool[ rand() % strlen( m_pool ) ];  
    } 
}


///////////////////////////////////////////////////////////////////////////////
// randPassword
//

char *randPassword::generatePool( unsigned char type, char *pstr )  
{  
    int i;
    int cnt = 0;
    
    switch ( type ) { 
        
        case 5:  
            // This will add these chars into the string !#$%&() 
            for ( i = 0x21; i < 0x29; i++ ) {  
                if ( i == 0x22 || i == 0x27) continue;  // Exclude characters " and '  
                pstr[ cnt++ ] = (char)i;  
            }  
            
            // Fall trough 
            
        case 4:     // 0 - 9, A - Z and a - z 
    
            for (i = 0x30; i <= 0x39; i++ ) {  
                pstr[ cnt++  ] = (char)i;       // add chars 0 - 9;  
            }  
            
            // Fall trough 
                
        case 3:     // a - z and A - Z      
            
            // Fall trough 
            
        case 2:     // A - Z   
            for (i = 0x41; i <= 0x5A; i++ ) {  
                pstr[ cnt++  ] = (char)i;  
            }  
                
            // Fall trough 
            
        default:    
        case 1:     // a - z
            for ( i = 0x61; i <= 0x7A; i++ )  {  
                pstr[ cnt++  ] = (char)i;  
            }  
            break;    

    } 

    return pstr;

}

