//
// config.cpp	
//			class Config - Read parameters from config file.
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
// $RCSfile: configfile.cpp,v $                                       
// $Date: 2005/08/30 11:00:13 $                                  
// $Author: akhe $                                              
// $Revision: 1.18 $ 
//
//  AH  000829  started this project.
//
//
//		  $RCSfile: configfile.cpp,v $                                       
//        $Date: 2005/08/30 11:00:13 $                                  
//        $Author: akhe $                                              
//        $Revision: 1.18 $  
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
