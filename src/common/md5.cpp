//////////////////////////////////////////////////////////////////////
// md5.cpp: implementation of the Cmd5 class.
// Calculate MD5 Digest using the WIN Crypto API.
//
// Copyright (C) 2000-2018 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
// Original code from Victor M. Valenzuela
// http://www.codeproject.com/cpp/cryptest.asp
//
// Needs defined  _WIN32_WINNT >= 0x0400 to compile on windows
//
//////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"

#ifdef WIN32
#include <wincrypt.h>	// Cryptographic API Prototypes and Definitions
#else
//#include <openssl/md5.h>
#include <stdio.h>
#include <openssl/evp.h>
#endif


#include "md5.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Cmd5::Cmd5(void)
{
    *m_szDigest = 0;
}

Cmd5::Cmd5(unsigned char *pstr)
{
    digest(pstr);
}

Cmd5::~Cmd5()
{

}



///////////////////////////////////////////////////////////////////////////////
// digest
//

char *Cmd5::digest(unsigned char *pstr)
{
#ifdef WIN32

    HCRYPTPROV hCryptProv;
    HCRYPTHASH hHash;
    unsigned char bHash[0x7f];
    DWORD dwHashLen = 16; // The MD5 algorithm always returns 16 bytes.  


    if (CryptAcquireContext(&hCryptProv,
        NULL,
        NULL,
        PROV_RSA_FULL,
        CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET)) {

        if (CryptCreateHash(hCryptProv,
            CALG_MD5, // algorithm identifier definitions see: wincrypt.h
            0, 0, &hHash)) {

            if (CryptHashData(hHash,
                pstr,
                strlen((const char *) pstr), 0)) {

                if (CryptGetHashParam(hHash, HP_HASHVAL, bHash, &dwHashLen, 0)) {

                    // Make a string version of the numeric digest value
                    *m_szDigest = 0;
                    char tmp[ 10 ];

                    for (int i = 0; i < 16; i++) {
                        sprintf(tmp, "%02x", bHash[i]);
                        strcat(m_szDigest, tmp);
                    }

                } else {
                    return NULL;
                }

            } else {
                return NULL;
            }
        } else {
            return NULL;
        }

    } else {
        return NULL;
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hCryptProv, 0);

#else

    EVP_MD_CTX mdctx;
    const EVP_MD *md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;

    OpenSSL_add_all_digests();

    md = EVP_get_digestbyname("md5");
    EVP_MD_CTX_init(&mdctx);
    EVP_DigestInit_ex(&mdctx, md, NULL);
    EVP_DigestUpdate(&mdctx, pstr, strlen((const char *) pstr));
    EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
    EVP_MD_CTX_cleanup(&mdctx);

    memset(m_szDigest, 0, sizeof(m_szDigest));
    sprintf(m_szDigest,
        "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
        md_value[0], md_value[1], md_value[2], md_value[3], md_value[4], md_value[5], md_value[6], md_value[7],
        md_value[8], md_value[9], md_value[10], md_value[11], md_value[12], md_value[13], md_value[14], md_value[15]);
    /*
        MD5_CTX *pctx;
        if ( 0 == MD5_Init( pctx ) ) return NULL;
        if ( 0 == MD5_Update( pctx, 
                                                            pstr,
                                            strlen( (const char *)pstr ) ) ) return NULL;
        unsigned char buf[ 17 ];
        if ( 0 == MD5_Final( buf, pctx ) ) return NULL;	
        sprintf( m_szDigest, 
            "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\0",
            buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],
            buf[8],buf[9],buf[10],buf[11],buf[12],buf[13],buf[14],buf[15] ); 
     */

#endif

    return m_szDigest;

}


