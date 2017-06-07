#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>


// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES encryption in CBC-mode of operation.
// ECB enables the basic ECB 16-byte block algorithm. Both can be enabled simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at compile time.
#ifndef CBC
  #define CBC 1
#endif

#ifndef ECB
  #define ECB 1
#endif

#define AES128  0
#define AES192  1
#define AES256  2

#if defined(ECB) && ECB

void AES_ECB_encrypt(uint8_t type, const uint8_t* input, const uint8_t* key, uint8_t *output, const uint32_t length);
void AES_ECB_decrypt(uint8_t type, const uint8_t* input, const uint8_t* key, uint8_t *output, const uint32_t length);

#endif // #if defined(ECB) && ECB


#if defined(CBC) && CBC

void AES_CBC_encrypt_buffer(uint8_t type,uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
void AES_CBC_decrypt_buffer(uint8_t type,uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);

#endif // #if defined(CBC) && CBC

///////////////////////////////////////////////////////////////////////////////
// getIV
//
// Get initialization vector
//
// @param buf Pointer to buffer that will get random data
// @param len Number of random values to fill buffer with.
// @return Zero on failure, Number of read values on success.
//

int getIV( uint8_t *buf, size_t len );

#endif //_AES_H_
