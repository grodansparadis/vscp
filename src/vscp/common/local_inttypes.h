#ifndef WIN32
//#include <stdint.h>
//#include <sys/types.h>
#endif

// TODO should be moved to platform file.  /AKHE
#define SIZEOF_CHAR                         1
#define SIZEOF_SHORT                        2
#define SIZEOF_INT                          4
//#ifndef _MSC_EXTENSIONS
#define SIZEOF_LONG_LONG                    8
//#endif


#ifndef ___int8_t_defined
#define ___int8_t_defined

//#if ( __VISUALC__ == 1200 || __VISUALC__ == 600 )


////#ifdef __VISUALC__
// VC6 does not support 'long long'.
// http://support.microsoft.com/kb/65472
//typedef __int64                           int64_t;
//#else
//typedef long long                         int64_t;
//#endif



#if SIZEOF_CHAR == 1
typedef unsigned char                       uint8_t;
typedef signed char                         int8_t;
#elif SIZEOF_INT == 1
typedef unsigned int                        uint8_t;
typedef signed int                          int8_t;
#else  /* XXX */
#error "there's no appropriate type for uint8_t"
#endif


#if SIZEOF_INT == 4
typedef signed int                          int32_t;
typedef unsigned int                        uint32_t;
#elif SIZEOF_LONG == 4
typedef unsigned long                       uint32_t;
typedef unsigned long                       uint32_t;
#elif SIZEOF_SHORT == 4
typedef unsigned short                      uint32_t;
typedef unsigned short                      uint32_t;
#else  /* XXX */
#error "there's no appropriate type for uint32_t"
#endif


#if SIZEOF_SHORT == 2
typedef unsigned short                      uint16_t;
typedef signed short                        int16_t;
#elif SIZEOF_INT == 2
typedef unsigned int                        uint16_t;
typedef signed int                          int16_t;
#elif SIZEOF_CHAR == 2
typedef unsigned char                       uint16_t;
typedef signed char                         int16_t;
#else  /* XXX */
#error "there's no appropriate type for uint16_t"
#endif


//#if ( __VISUALC__ == 1200 || __VISUALC__ == 600 )
//typedef unsigned __int64                  uint64_t;
// VC6 does not support 'long long'.
//#else
//typedef unsigned long long                uint64_t;
//#endif


#ifdef __LINUX__ 


#else

#if SIZEOF_LONG_LONG == 8
typedef unsigned long long                  uint64_t;
typedef long long                           int64_t;
// AKHE 2012-04-18 Changed due to problems on Linux 64-bit
//typedef unsigned long int                  uint64_t;
//typedef long int 			   int64_t;
#elif defined(_MSC_EXTENSIONS)
typedef unsigned _int64                     uint64_t;
typedef _int64                              int64_t;
#elif SIZEOF_INT == 8
typedef unsigned int                        uint64_t;
#elif SIZEOF_LONG == 8
typedef unsigned long                       uint64_t;
#elif SIZEOF_SHORT == 8
typedef unsigned short                      uint64_t;
#else  /* XXX */
#error "there's no appropriate type for     u_int64_t"
#endif

#endif


#endif  // __int8_t_defined