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
