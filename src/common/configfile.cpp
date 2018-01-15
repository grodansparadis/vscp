// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "configfile.h"

///////////////////////////////////////////////////////////////////////////////
//

Config::Config( char *name )
{
    m_fIn = NULL;
    m_pBuffer = NULL;
    
    if ( NULL != name ) {
        if ( NULL == ( m_fIn = fopen( name, "rb" ) ) ) {
            m_fIn = NULL;
            return;
        }
    }
    
    m_pBuffer = new char[255];
    
}

///////////////////////////////////////////////////////////////////////////////
//

Config::~Config()
{
    if ( NULL != m_fIn ) fclose( m_fIn );
    if ( NULL != m_pBuffer ) delete m_pBuffer;
}

///////////////////////////////////////////////////////////////////////////////
//

FILE * Config::open( char *name )
{
    // If a configuration file is opened, close it first.
    if ( NULL != m_fIn ) {
        fclose( m_fIn );
    }

    return ( m_fIn = fopen( name, "rb" ));	
}

///////////////////////////////////////////////////////////////////////////////
//

char *Config::ScanKey( char *key )
{
    char *eq;
    
    // Configuratin file must be open
    if ( NULL == m_fIn ) return NULL;
    
    rewind( m_fIn );
    while( !feof( m_fIn ) ) {
        char *p = fgets( m_pBuffer, 255, m_fIn );
        Clean();
        
        // Scan for the =
        eq = strchr( m_pBuffer, '=' );
        if ( eq != NULL ) {
            *eq = 0;
            if ( 0 == strncmp( m_pBuffer, key, strlen( key ) ) ) {
                return eq + 1;
            }
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//

void Config::Clean(void)
{
    char *p;
    p = strchr( m_pBuffer, ';' );
    if (p)
    {
        *p=0;
    }
    p = strchr( m_pBuffer, '\n' );
    if ( p )
    {
        *p=0;
    }
}

///////////////////////////////////////////////////////////////////////////////
//

bool Config::LoadKey( char *key, char *value, int max )
{
    char *f,*a,*b;

    // Configuration file must be open
    if ( NULL == m_fIn ) return false;
            
    f = ScanKey( key );
    
    if ( f == NULL )
    {
        //throw("Key Not Found.");
        //printf("Key not found\n");
        return false;
    }
    a = strchr(f,'"');
    
    if ( a == NULL )
    {
        //throw("Key Not Text.");
        //printf("Key Not Text.\n");
        return false;
    }
    a++;
    b = strchr(a,'"');
    
    if ( b == NULL )
    {
        //throw("No closing quote");
        //printf("No closing quote.\n");
        return false;
    }
    f = value;
    
    while ( ( a != b ) && ( ( f - value ) < max ) )
    {
        *f++ = *a++;
    }
    *f=0;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//

bool Config::LoadKey( char *key, long int &num )
{
    // Configuration file must be open
    if ( NULL == m_fIn ) return false;
    
    char *a = ScanKey(key);
    
    if ( a == NULL )
    {
        return false;
    }
    
    num = strtol( a, NULL, 0 );
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//

bool Config::LoadKey( char *key, int &num )
{
    long v;
    
    // Configuration file must be open
    if ( NULL == m_fIn ) return false;
    
    LoadKey( key, v );
    
    num = v;
    return true;
}
