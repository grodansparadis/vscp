# Security

Security is very important in VSCP and so also of course in the VSCP daemon. Still much of it has been put aside during the development of VSCP & Friends. The reason is that the subject of security is difficult and hard and need  a lot of thought before it is deployed. But now from version 1.13 of the package security will be implemented in all areas of the framework.

## Encryption key

For encryption a 256-bit key is used. This key can be stored in the *vscpd.conf* file as 32 byte hex string (`<general>` section `<security>`) in which case the *vscpd.conf* file must be protected from read from other uses. The second alternative is to give it as a key when the VSCP server starts up. Use the **-k** switch for that as

    vscpd -kA4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E 
    
If *no* key is given 

    A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E 

is used.

For AES128 and AES192 the fist 16 respective 24 bytes is sed.

## Storage of passwords

* Uses [[https://en.wikipedia.org/wiki/PBKDF2|pbkdf2].
* Uses SHA256 hash.
* A 64 bit (8 byte) salt is used.
* 70000 iterations.

Passwords is calculated as SHA256 over "username;authdomain;password" 

## TCP/IP

* Uses username/password.
* Uses SSL.
* Possible to use AES128/192/256 encryption.

Password is the MD5 over *username:authdomain:password*

## Web/REST/websockets

* Uses username/password.
* Uses SSL.
* Possible to use AES128/192/256 enryption.

## VSCP UDP

* Encryption with AES128/192/256 and 128 bit random IV.

## VSCP Muticast

* Encryption with AES128/192/256 and 128 bit random IV.
* Password is the MD5 over *username:authdomain:password*



[filename](./bottom_copyright.md ':include')

