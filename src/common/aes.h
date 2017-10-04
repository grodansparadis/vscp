#ifndef _AES_H_
#define _AES_H_

#ifdef __cplusplus
extern "C" {
#endif

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

void AES_ECB_encrypt( uint8_t type, 
                        const uint8_t* input, 
                        const uint8_t* key, 
                        uint8_t *output, 
                        const uint32_t length );
void AES_ECB_decrypt( uint8_t type, 
                        const uint8_t* input, 
                        const uint8_t* key, 
                        uint8_t *output, 
                        const uint32_t length );

#endif // #if defined(ECB) && ECB


#if defined(CBC) && CBC

/*!
 * Encrypt buffer with AES128/AES192/AES256
 * 
 * @param type The algorithm to use AES127/AES192/AES256
 * @param output Buffer that holds the result.
 * @param input Buffer with data that should be encrypted.
 * @param length Size of the data that should be encrypted.
 * @param key Pointer to encryption key. Should be of same length as
 *            the algorithm used (128/192/256)
 * @param if Pointer to initialization vector. Should always be 128 bits.
 */
void AES_CBC_encrypt_buffer( uint8_t type,
                                uint8_t* output, 
                                uint8_t* input, 
                                uint32_t length, 
                                const uint8_t* key, 
                                const uint8_t* iv);

/*!
 * Encrypt buffer using AES128/AES192/AES256
 * 
 * @param type The algorithm to use AES128/AES192/AES256
 * @param output Buffer that holds the result.
 * @param input Buffer with data that should be decrypted.
 * @param length Size of the data that should be decrypted.
 * @param key Pointer to encryption key. Should be of same length as
 *            the algorithm used (128/192/256)
 * @param if Pointer to initialization vector. Should always be 128 bits.
 */
void AES_CBC_decrypt_buffer( uint8_t type,
                                uint8_t* output,
                                uint8_t* input,
                                uint32_t length,
                                const uint8_t* key,
                                const uint8_t* iv );

#endif // #if defined(CBC) && CBC

/*!
 *  Get initialization vector.
 *  Always 128 bit for AES128/192/256
 *     https://security.stackexchange.com/questions/90848/encrypting-using-aes-256-can-i-use-256-bits-iv
 *
 * @param buf Pointer to buffer that will get random data
 * @param len Number of random values to fill buffer with.
 * @return Zero on failure, Number of read values on success.
 *
 */ 

size_t getRandomIV( uint8_t *buf, size_t len );

#ifdef __cplusplus
}
#endif

#endif //_AES_H_
