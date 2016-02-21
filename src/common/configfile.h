//
// config.h
//          class Config - Read parameters from config file.
//
//
// Copyright (C) 2000-2016 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
//  AH  981224  started this project.
//
//

#ifndef H_CONFIG_H
#define H_CONFIG_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
    A class for configure file handling
*/

class Config
{

public:
    /**
        Constructor

        @param name Name (including path) of configuration file.
    */
    Config( char *name = NULL );
    /// Destructor
    virtual ~Config();
    
    /// Open Configuration filr
    FILE * open( char *name );
    
    /**
        Load a string with a maximum length from a key

        @param key pointer to string containing key name
        @param value pointer to buffer to hold string value of key
        @param max Buffer size for value.
    */
    bool LoadKey( char *key, char *value, int max );
    
    bool LoadKey( char *key, int &value );
    bool LoadKey( char *key, long &value );

private:
    char *ScanKey( char *key );
    void Clean( void );
    
private:	
    FILE *m_fIn;
    char *m_pBuffer;
};


#endif
