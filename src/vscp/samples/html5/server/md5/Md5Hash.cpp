#include <Windows.h>
#include <stdio.h>
#include <wincrypt.h>      // CryptoAPI definitions
/*
For non C/C++ users the constants used here are:
#define ALG_CLASS_HASH                  (4 << 13)
#define ALG_TYPE_ANY                    (0)
#define ALG_SID_MD5                     3
#define CALG_MD5        (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_MD5)
#define HP_HASHVAL              0x0002  // Hash value
#define HP_HASHSIZE             0x0004  // Hash value size
*/
/*
For non-C/C++ users the constants used here are:
#define MS_DEF_PROV       "Microsoft Base Cryptographic Provider v1.0"
#define PROV_RSA_FULL           1
*/
BOOL bResult;
HCRYPTHASH hHash;
DWORD dwBufferSize;
DWORD dwValue;
PBYTE pBuffer;
HCRYPTPROC hProv;

int main()
{
	bResult = CryptAcquireContext(
            &hProv,            // Variable to hold returned handle.
            NULL,              // Use default key container.
            MS_DEF_PROV,       // Use default CSP.
            PROV_RSA_FULL,     // Type of provider to acquire.
            0);                // No special action.

	// Obtain handle to hash object.
			bResult = CryptCreateHash(
            hProv,               // Handle to CSP obtained earlier
            CALG_MD5,            // Hashing algorithm
            0,                   // Non-keyed hash
            0,                   // Should be zero
            &hHash);             // Variable to hold hash object handle 
	return 0;
}