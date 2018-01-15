///////////////////////////////////////////////////////////////////////////////
// randPassword.h
//
// This file is part is part of VSCP (Very Simple Control Protocol)
// http://www.vscp.org)
//
// Copyright (c) 2000-2018 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef RANDOMPASSWORD_H
#define RANDOMPASSWORD_H

class randPassword  
{

public:

    // Constructor
    randPassword( unsigned char type = 5 );

    // Destructor
    ~randPassword();

    /*!
        generatePassword
        Generate a new password of the give length.
        \param length Length of generated password.
        \param pPassword Buffer that will hold the generated password
        The buffer should be at least ob byte gretaer then the length
        for the password to have toom for the end of string marker.
    */
    void generatePassword( unsigned char length, char *pPassword );


private:

    /*!
        Generate the pool
        \param type 1 = a - z 
            2 = A - Z 
            3 = a - z and A - Z 
            4 = a - z, A - Z and 0 - 9 
            5 = a - z, A - Z, 0 - 9 and chars !#$%&() 
        \param pstr Character buffer that will receive the password.
    */
    char *generatePool( unsigned char type, char *pstr );

    /*!
        Pool to use for password generation
    */
    char m_pool[ 256 ];

};


#endif
