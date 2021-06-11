// md5.h: interface for the Cmd5 class.
//
// Copyright © 2000-2021 Ake Hedman, the VSCP project, <info@vscp.org>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//////////////////////////////////////////////////////////////////////

#if !defined(MD5_H__438D2BEF_6F1B_4C5C_830F_0E7B6D1FD7E2__INCLUDED_)
#define MD5_H__438D2BEF_6F1B_4C5C_830F_0E7B6D1FD7E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef WIN32
#else
#include <openssl/md5.h>
#endif


class Cmd5
{

public:

    /*!
        Default constructor
    */
    Cmd5( void );

    /*!
        Constructor
        @param pstr string to encrypt.
    */
    Cmd5( unsigned char *pstr );
    
    /*!
        Destructor
    */
    virtual ~Cmd5();

    /*!
        Perform MD5
        @param pstr string to encrypt.
        @return Encrypted data.
    */
    char *digest( unsigned char *pstr );

    /*!
        Perform MD5
        @return Encrypted data.
    */
    char *getDigest( void ) { return m_szDigest; };


private:
    
    /*!
        MD5 data
    */
    char m_szDigest[128];
};

#ifdef  __cplusplus
}
#endif


#endif // !defined(MD5_H__438D2BEF_6F1B_4C5C_830F_0E7B6D1FD7E2__INCLUDED_)
