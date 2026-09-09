// FILE: vscphelper.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project
// <info@vscp.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/*!
    \file vscphelper.h
    \brief	The vscphelper file contains often used functionality when
   working with VSCP. \details vscphelper have common used functionality to do
   things in the VSCP world. It can be seen as the main toolbox for the VSCP
   programmer.
 */

#if !defined(VSCPHELPER_H__INCLUDED_)
#define VSCPHELPER_H__INCLUDED_

#ifndef WIN32
#ifdef __linux__
#include <byteswap.h>
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define bswap_16(x) OSSwapInt16(x)
#define bswap_32(x) OSSwapInt32(x)
#define bswap_64(x) OSSwapInt64(x)
#endif
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <sys/types.h>
#endif

#include <algorithm>
#include <cctype>
#include <deque>
#include <functional>
#include <locale>
#include <string>
#include <vector>

#include <float.h>
#include <semaphore.h>

#include <canal.h>
#include <sockettcp.h>
#include <vscp.h>
#include <vscp-class.h>
#include <vscp-type.h>

// For windows
#ifndef CAN_MTU
#define CAN_MTU 8 // TODO
#endif

/*  byte swapping */

#if !defined(VSCP_UINT16_SWAP_ALWAYS)
#define VSCP_UINT16_SWAP_ALWAYS(val)                                                                                   \
  ((uint16_t) ((((uint16_t) (val) & (uint16_t) 0x00ffU) << 8) | (((uint16_t) (val) & (uint16_t) 0xff00U) >> 8)))
#endif

#if !defined(VSCP_INT16_SWAP_ALWAYS)
#define VSCP_INT16_SWAP_ALWAYS(val)                                                                                    \
  ((int16_t) ((((uint16_t) (val) & (uint16_t) 0x00ffU) << 8) | (((uint16_t) (val) & (uint16_t) 0xff00U) >> 8)))
#endif

#if !defined(VSCP_UINT32_SWAP_ALWAYS)
#define VSCP_UINT32_SWAP_ALWAYS(val)                                                                                   \
  ((uint32_t) ((((uint32_t) (val) & (uint32_t) 0x000000ffU) << 24) |                                                   \
               (((uint32_t) (val) & (uint32_t) 0x0000ff00U) << 8) |                                                    \
               (((uint32_t) (val) & (uint32_t) 0x00ff0000U) >> 8) |                                                    \
               (((uint32_t) (val) & (uint32_t) 0xff000000U) >> 24)))
#endif

#if !defined(VSCP_INT32_SWAP_ALWAYS)
#define VSCP_INT32_SWAP_ALWAYS(val)                                                                                    \
  ((int32_t) ((((uint32_t) (val) & (uint32_t) 0x000000ffU) << 24) |                                                    \
              (((uint32_t) (val) & (uint32_t) 0x0000ff00U) << 8) |                                                     \
              (((uint32_t) (val) & (uint32_t) 0x00ff0000U) >> 8) |                                                     \
              (((uint32_t) (val) & (uint32_t) 0xff000000U) >> 24)))
#endif

/*  machine specific byte swapping */

#if !defined(VSCP_UINT64_SWAP_ALWAYS)
#define VSCP_UINT64_SWAP_ALWAYS(val)                                                                                   \
  ((uint64_t) ((((uint64_t) (val) & (uint64_t) (0x00000000000000ff)) << 56) |                                          \
               (((uint64_t) (val) & (uint64_t) (0x000000000000ff00)) << 40) |                                          \
               (((uint64_t) (val) & (uint64_t) (0x0000000000ff0000)) << 24) |                                          \
               (((uint64_t) (val) & (uint64_t) (0x00000000ff000000)) << 8) |                                           \
               (((uint64_t) (val) & (uint64_t) (0x000000ff00000000)) >> 8) |                                           \
               (((uint64_t) (val) & (uint64_t) (0x0000ff0000000000)) >> 24) |                                          \
               (((uint64_t) (val) & (uint64_t) (0x00ff000000000000)) >> 40) |                                          \
               (((uint64_t) (val) & (uint64_t) (0xff00000000000000)) >> 56)))
#endif

#if !defined(VSCP_INT64_SWAP_ALWAYS)
#define VSCP_INT64_SWAP_ALWAYS(val)                                                                                    \
  ((int64_t) ((((uint64_t) (val) & (uint64_t) (0x00000000000000ff)) << 56) |                                           \
              (((uint64_t) (val) & (uint64_t) (0x000000000000ff00)) << 40) |                                           \
              (((uint64_t) (val) & (uint64_t) (0x0000000000ff0000)) << 24) |                                           \
              (((uint64_t) (val) & (uint64_t) (0x00000000ff000000)) << 8) |                                            \
              (((uint64_t) (val) & (uint64_t) (0x000000ff00000000)) >> 8) |                                            \
              (((uint64_t) (val) & (uint64_t) (0x0000ff0000000000)) >> 24) |                                           \
              (((uint64_t) (val) & (uint64_t) (0x00ff000000000000)) >> 40) |                                           \
              (((uint64_t) (val) & (uint64_t) (0xff00000000000000)) >> 56)))
#endif
#ifdef __BIG_ENDIAN__
#if !defined(VSCP_UINT16_SWAP_ON_BE)
#define VSCP_UINT16_SWAP_ON_BE(val) VSCP_UINT16_SWAP_ALWAYS(val)
#define VSCP_INT16_SWAP_ON_BE(val)  VSCP_INT16_SWAP_ALWAYS(val)
#define VSCP_UINT16_SWAP_ON_LE(val) (val)
#define VSCP_INT16_SWAP_ON_LE(val)  (val)
#define VSCP_UINT32_SWAP_ON_BE(val) VSCP_UINT32_SWAP_ALWAYS(val)
#define VSCP_INT32_SWAP_ON_BE(val)  VSCP_INT32_SWAP_ALWAYS(val)
#define VSCP_UINT32_SWAP_ON_LE(val) (val)
#define VSCP_INT32_SWAP_ON_LE(val)  (val)
#define VSCP_UINT64_SWAP_ON_BE(val) VSCP_UINT64_SWAP_ALWAYS(val)
#define VSCP_UINT64_SWAP_ON_LE(val) (val)
#define VSCP_INT64_SWAP_ON_BE(val)  VSCP_INT64_SWAP_ALWAYS(val)
#define VSCP_INT64_SWAP_ON_LE(val)  (val)
#endif
#else
#if !defined(VSCP_UINT16_SWAP_ON_LE)
#define VSCP_UINT16_SWAP_ON_LE(val) VSCP_UINT16_SWAP_ALWAYS(val)
#define VSCP_INT16_SWAP_ON_LE(val)  VSCP_INT16_SWAP_ALWAYS(val)
#define VSCP_UINT16_SWAP_ON_BE(val) (val)
#define VSCP_INT16_SWAP_ON_BE(val)  (val)
#define VSCP_UINT32_SWAP_ON_LE(val) VSCP_UINT32_SWAP_ALWAYS(val)
#define VSCP_INT32_SWAP_ON_LE(val)  VSCP_INT32_SWAP_ALWAYS(val)
#define VSCP_UINT32_SWAP_ON_BE(val) (val)
#define VSCP_INT32_SWAP_ON_BE(val)  (val)
#define VSCP_UINT64_SWAP_ON_LE(val) VSCP_UINT64_SWAP_ALWAYS(val)
#define VSCP_UINT64_SWAP_ON_BE(val) (val)
#define VSCP_INT64_SWAP_ON_LE(val)  VSCP_INT64_SWAP_ALWAYS(val)
#define VSCP_INT64_SWAP_ON_BE(val)  (val)
#endif
#endif

#if !defined(Swap8Bytes)
#define Swap8Bytes(val)                                                                                                \
  ((((val) >> 56) & 0x00000000000000FF) | (((val) >> 40) & 0x000000000000FF00) |                                       \
   (((val) >> 24) & 0x0000000000FF0000) | (((val) >> 8) & 0x00000000FF000000) | (((val) << 8) & 0x000000FF00000000) |  \
   (((val) << 24) & 0x0000FF0000000000) | (((val) << 40) & 0x00FF000000000000) | (((val) << 56) & 0xFF00000000000000))
#endif

/*!
 * @name Min/max macros
 * @{
 */
#if !defined(MIN)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#if !defined(MAX)
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
/* @} */

// For platform independet non-case dependent string compare
#ifdef _MSC_VER
// not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp  _stricmp
#endif

// Forward declaration
class CMDF;

/*!
  @enum vscp_guid_str_format
  @brief GUID string output format options
*/
enum vscp_guid_str_format {
  VSCP_GUID_STR_FORMAT_NORMAL = 0, //!< Standard colon-separated: FF:FF:FF:...
  VSCP_GUID_STR_FORMAT_COMPACT,    //!< Compact with :: for leading 0xFF: ::01:02:03
  VSCP_GUID_STR_FORMAT_UUID        //!< UUID format (8-4-4-4-12): FFFFFFFF-FFFF-FFFF-0102-030405060708
};

/*
#ifdef __cplusplus
extern "C"
{
#endif
*/

/*!
    \union vscp_value
    \brief VSCP Data coding declaration
*/
union vscp_value {

  /// float value
  double float_value;

  /// Integer and long value and
  long long_value;

  /// Integer value
  int int_value;

  /// Byte value
  unsigned char byte_value;
};

// ***************************************************************************
//                                General Helpers
// ***************************************************************************

/*!
  @fn vscp_is64Bit
  Return non-zero if platform is 64-bit
  @return Non-zero if platform is 64-bit, zero otherwise
*/
int
vscp_is64Bit();

/*!
  @fn vscp_is32Bit
  Return non-zero if platform is 32-bit
  @return Non-zero if platform is 32-bit, zero otherwise
*/
int
vscp_is32Bit();

/*!
  @fn vscp_isLittleEndian
  Return non zero if endiness is little endian
  @return Non-zero if endiness is little endian, zero otherwise
*/
int
vscp_isLittleEndian(void);

/*!
  @fn vscp_isBigEndian
  Return non zero if endiness is big endian
  @return Non-zero if endiness is big endian, zero otherwise
*/
int
vscp_isBigEndian(void);

#ifndef WIN32
/*!
  @fn vscp_mem_usage
  Get program memory usage
  @param vm_usage Reference to variable to store virtual memory usage
  @param resident_set Reference to variable to store resident set size
*/
void
vscp_mem_usage(double &vm_usage, double &resident_set);
#endif

#ifdef WIN32
/*!
  Wait for semaphore

  @fn vscp_sem_wait
  @param hHandle Pointer to handle to semaphore. The pointer
    is needed to be compatible with Linux semaphore handles.
  @param waitms Time in milliseconds to wait.
    if waitms is 0 - return immidiatly
    if waitms is INFINITE - wait forever
    else wait for waitms milliseconds
  @return Return 0 if semaphore is signaled, -1 if error. errno
  is set accordingly. ETIMEDOUT is returned for timeout.
  https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject
*/
int
vscp_sem_wait(HANDLE *phHandle, uint32_t waitms);
#else
/*!
  Wait a number of milliseconds for semaphore

  @fn vscp_sem_wait
  @param sem Pointer to semaphore to wait for
  @param waitms Time in milliseconds to wait. Max 3999 ms.
  @return Return 0 if semaphore is signaled, -1 if error. errno
  is set accordingly. ETIMEDOUT is returned for timeout.
*/
int
vscp_sem_wait(sem_t *sem, uint32_t waitms);
#endif

#ifdef WIN32
/*!
  @param phHandle Pointer to handle of semaphore to wait for
  @return 0 on success, -1 on error
*/
int
vscp_sem_post(HANDLE *phHandle);
#else
/*!
  @fn vscp_sem_post
  Post (signal) a semaphore
  @param sem Pointer to semaphore to wait for
  @return 0 on success, -1 on error
*/
int
vscp_sem_post(sem_t *sem);
#endif

#ifdef WIN32
/*!
  @fn vscp_usleep
  Sleep for a number of microseconds
  @param usec Number of microseconds to sleep
*/
static void
vscp_usleep(__int64 usec);
#endif

/*
 * @fn vscp_almostEqualRelativeFloat
 * Check two floats for equality
 * @param A Float to compare
 * @param B Float to compare
 * @param maxRelDiff Maxdiff (see article)
 * @return true if same, false if different (sort of)
 *
 * https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
 */

bool
vscp_almostEqualRelativeFloat(float A, float B, float maxRelDiff = FLT_EPSILON);

/*!
 * @fn vscp_almostEqualUlpsAndAbsFloat
 * https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
 * Check two floats for equality
 * @param A Float to compare
 * @param B Float to compare
 * @param maxDiff Maxdiff (see article)
 * @param maxUlpsDiff Max ULPs difference (see article)
 */
bool
vscp_almostEqualUlpsAndAbsFloat(float A, float B, float maxDiff, int maxUlpsDiff);

/*!
 * @fn vscp_almostEqualRelativeAndAbsFloat
 * https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
 * Check two floats for equality
 * @param A Float to compare
 * @param B Float to compare
 * @param maxDiff Maxdiff (see article)
 * @param maxRelDiff Max relative difference (see article)
 * @return true if same, false if different (sort of)
 */
bool
vscp_almostEqualRelativeAndAbsFloat(float A, float B, float maxDiff, float maxRelDiff = FLT_EPSILON);

/*
 * @fn vscp_almostEqualRelativeDouble
 * Check two floats for equality
 * @param A Double to compare
 * @param B Double to compare
 * @param maxRelDiff Maxdiff (see article)
 * @return true if same, false if different (sort of)
 *
 * https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
 */

bool
vscp_almostEqualRelativeDouble(double A, double B, double maxRelDiff = DBL_EPSILON);

/*!
  @fn vscp_readStringValue
  Read a numerical value from a string
  The string value can be set as a hex or a decimal value.
  @param strval String containing value to be converted
  @return 64-bit signed integer containing value
*/

int64_t
vscp_readStringValue(const std::string &strval);

/*!
  @fn vscp_lowercase
  Convert string to lowercase
  @param s String to convert
  @return Lowercase character
*/
int
vscp_lowercase(const char *s);

/*!
  @fn vscp_strcasecmp
  String non case compare
  @param s1 String1 to compare
  @param s2 String2 to compare
  @return 0 if strings are the same
*/

int
vscp_strcasecmp(const char *s1, const char *s2);

/*!
  @fn vscp_strncasecmp
  String non case compare with length
  @param s1 String1 to compare
  @param s2 String2 to compare
  @param len Number of byte to compare
  @return 0 if strings are the same
*/
int
vscp_strncasecmp(const char *s1, const char *s2, size_t len);

/*!
  @fn vscp_strlcpy
  Copy string with size limit
  @param dst Destination buffer
  @param src Source string
  @param n Size of destination buffer
*/
void
vscp_strlcpy(char *dst, const char *src, size_t n);

/*!
  @fn vscp_strlcat
  Concatenate string with size limit
  @param dst Destination buffer
  @param src Source string
  @param n Size of destination buffer
*/
char *
vscp_strdup(const char *str);

/*!
  @fn vscp_strlcat
  Concatenate string with size limit
  @param dst Destination buffer
  @param src Source string
  @param n Size of destination buffer
*/
char *
vscp_strndup(const char *ptr, size_t len);

/*!
  @fn vscp_strcasestr
  Case insensitive string search
  @param big_str String to search in
  @param small_str String to search for
  @return Pointer to first occurrence of small_str in big_str or NULL if not found
*/
const char *
vscp_strcasestr(const char *big_str, const char *small_str);

/*!
  @fn vscp_stristr
  Case insensitive string search and return pointer to first occurrence
  @param str1 String to search in
  @param str2 String to search for
  @return Pointer to first occurrence of str2 in str1 or NULL if not found
*/
char *
vscp_stristr(char *str1, const char *str2);

/*!
  @fn vscp_trimWhiteSpace
  Trim leading and trailing white space from string
  @param str String to trim
  @return Pointer to trimmed string (same as input pointer)
*/
char *
vscp_trimWhiteSpace(char *str);

/*!
  @fn vscp_strrev
  Reverse a string in place
  @param str String to reverse
  @return Pointer to reversed string (same as input pointer)
*/
char *
vscp_reverse(const char *const s);

/*!
  @fn vscp_rstrstr
  Reverse string search
  @param s1 String to search in
  @param s2 String to search for
  @return Pointer to last occurrence of s2 in s1 or NULL if not found
*/
char *
vscp_rstrstr(const char *s1, const char *s2);

/*!
  @fn vscp_str_format
  Format a string using printf-style formatting

  Note! Putting the format string as a reference does dot work on window.

  https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf/49812018
  @param format string
  @param Variables part of resulting string
  @return formated string
*/
std::string
vscp_str_format(std::string fstr, ...);

/*!
  @fn vscp_startsWith

  Check if a string have a start token and optionally return
  the rest of the string if it has. If the token is not available and
  rest is non-null the original string will be returned in rest.
  @param origstr String to check.
  @param searchstr Start token to look for.
  @param rest If not set to null the rest string is returned here.
  Original string is returned if token string is not found.
  @return True if start token is found.
*/
bool
vscp_startsWith(const std::string &origstr, const std::string &searchstr, std::string *rest = NULL);

/*!
  @fn vscp_makeUpper
  String to upper case (in place)

  @param s String to convert
 */
static inline void
vscp_makeUpper(std::string &s)
{
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) -> unsigned char { return std::toupper(c); });
}

/*!
  @fn vscp_makeUpper_copy
  String to upper case (copying)
  @param s String to convert
  @return Converted string
*/
static inline std::string
vscp_makeUpper_copy(std::string s)
{
  vscp_makeUpper(s);
  return s;
}

/*!
  @fn vscp_upper
  String to upper case
  @param s String to convert
  @return Converted string
*/
static inline std::string
vscp_upper(const std::string &s)
{
  std::string newstr = s;
  vscp_makeUpper(newstr);
  return newstr;
}

/*!
  @fn vscp_makeLower
  String to lower case (in place)
  @param s String to convert
 */
static inline void
vscp_makeLower(std::string &s)
{
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}

/*!
  @fn vscp_makeLower_copy
  String to lower case (copying)
  @param s String to convert
  @return Converted string
*/
static inline std::string
vscp_makeLower_copy(std::string s)
{
  vscp_makeLower(s);
  return s;
}

/*!
  @fn vscp_lower
  String to lower case
  @param s String to convert
  @return Converted string
*/
static inline std::string
vscp_lower(const std::string &s)
{
  std::string newstr = s;
  vscp_makeLower(newstr);
  return newstr;
}

/*!
  @fn vscp_ltrim
  Trim from start (in place)
  @param s String to trim
*/
static inline void
vscp_ltrim(std::string &s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

/*!
  @fn vscp_rtrim
  Trim from end (in place)
  @param s String to trim
*/
static inline void
vscp_rtrim(std::string &s)
{
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

/*!
  @fn vscp_trim
  Trim from both ends (in place)
  @param s String to trim
*/
static inline void
vscp_trim(std::string &s)
{
  vscp_ltrim(s);
  vscp_rtrim(s);
}

/*!
  @fn vscp_ltrim_copy
  Trim from start (copying)
  @param s String to trim
  @return Trimmed string
*/
static inline std::string
vscp_ltrim_copy(std::string s)
{
  vscp_ltrim(s);
  return s;
}

/*!
  @fn vscp_rtrim_copy
  Trim from end (copying)
  @param s String to trim
  @return Trimmed string
*/
static inline std::string
vscp_rtrim_copy(std::string s)
{
  vscp_rtrim(s);
  return s;
}

/*!
  @fn vscp_trim_copy
  Trim from both ends (copying)
  @param s String to trim
  @return Trimmed string
*/
static inline std::string
vscp_trim_copy(std::string s)
{
  vscp_trim(s);
  return s;
}

/*!
  @fn vscp_split
  Split a string into a deque of strings based on a delimiter
  @param theStringVector Deque to store the split strings
  @param theString String to split
  @param theDelimiter Delimiter to use for splitting
*/
static inline void
vscp_split(std::deque<std::string> &theStringVector, const std::string &theString, const std::string &theDelimiter)
{
  size_t start = 0, end = 0;

  std::string str = theString;
  vscp_trim(str);
  if (!theString.length())
    return;

  while (end != std::string::npos) {
    end = theString.find(theDelimiter, start);

    // If at end, use length=maxLength.  Else use length=end-start.
    theStringVector.push_back(theString.substr(start, (end == std::string::npos) ? std::string::npos : end - start));

    // If at end, use start=maxSize.  Else use start=end+delimiter.
    start = ((end > (std::string::npos - theDelimiter.size())) ? std::string::npos : end + theDelimiter.size());
  }
}

/*!
  @fn vscp_str_left
  Return left part of std string

  @param str String to work on
  @param length Number of characters to return
  @return Left part of string
*/
static inline std::string
vscp_str_left(const std::string &str, size_t length)
{
  if (0 == length)
    return std::string("");
  return str.substr(0, length);
}

/*!
  @fn vscp_str_right
  Return right part of std string
  @param str String to work on
  @param length Number of characters to return
  @return Right part of string
*/
static inline std::string
vscp_str_right(const std::string &str, size_t length)
{
  if (0 == length)
    return std::string("");
  if (length > str.length())
    length = str.length();
  return str.substr(str.length() - length);
}

/*!
  @fn vscp_str_after
  Return string part after char
  @param str Str to work on
  @param c Character to look for
  @return Part of sting after character or empty string if character si
   not found.
*/
static inline std::string
vscp_str_before(const std::string &str, char c)
{
  size_t pos = str.find(c);
  if (str.npos == pos)
    return std::string();

  return vscp_str_left(str, pos);
}

/*!
  @fn vscp_str_after
  Return string part after char
  @param str Str to work on
  @param c Character to look for
  @return Part of sting after character or empty string if character si
   not found.
*/
static inline std::string
vscp_str_after(const std::string &str, char c)
{
  size_t pos = str.find(c);
  if (str.npos == pos)
    return std::string();

  return vscp_str_right(str, pos);
}

/*!
  @fn vscp_safe_encode_str
  Safely encode a standard string to be encoded in
  HTML, XML or JSON
  " (double quote)
  \ (backslash)
  all control characters like \n, \t

  @param str Standard C++ string to encode
  @return Encoded string.
*/
std::string
vscp_safe_encode_str(const std::string &str);

/*!
  @fn vscp_isNumber
  Check if a string is a number
  @param strNumber String to check
  @return True if string is a number, false otherwise
*/
static inline bool
vscp_isNumber(const std::string &strNumber)
{
  std::string str = strNumber;
  vscp_trim(str);
  vscp_makeLower(str);
  if (isdigit(str[0]) || vscp_startsWith(str, "0x") || vscp_startsWith(str, "0o") || vscp_startsWith(str, "0b")) {
    return true;
  }

  return false;
}

/*!
  @fn vscp_fileExists
  Check if a file exists

  @param path Path to file to check
  @return True if file exists, false otherwise.
*/
bool
vscp_fileExists(const std::string &path);

/*!
  @fn vscp_dirExists
  Checks to see if a directory exists. Note: This method only checks the
  existence of the full path AND if path leaf is a dir.

  @param path path to directory.
  @return  >0 if dir exists AND is a dir,
            0 if dir does not exist OR exists but not a dir,
            <0 if an error occurred (errno is also set)
*/
int
vscp_dirExists(const char *const path);

/*!
  @fn vscp_base64_std_decode
  BASE64 decode std string
  @param str str to encode.
  @return true on success, false on failure.
 */

bool
vscp_base64_std_decode(std::string &str);

/*!
  @fn vscp_base64_std_encode
  BASE64 encode std string
  @param str str to encode.
  @return true on success, false on failure.
 */

bool
vscp_base64_std_encode(std::string &str);

/*!
  @fn vscp_std_decodeBase64IfNeeded
  Decode string from BASE64 is it have "BASE64:" in the front if not just
  leave the string as it is.

  @param str String to decode
  @param strResult String result
  @return True if all is OK (decoded or not) and false if decode error.
 */

bool
vscp_std_decodeBase64IfNeeded(const std::string &str, std::string &strResult);

/*!
  @fn vscp_convertToBase64
  Convert string to BASE64

  @param str String to be encoded.
  @return BASE64 coded string
*/
std::string
vscp_convertToBase64(std::string str);

/*!
  @fn vscp_getTimeString
  Get GMT time
  http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3
  @param buf Buffer that will get result
  @param buf_len Size of buffer
  @param t Unix time
  @return True if all is OK; false otherwise.
*/
bool
vscp_getTimeString(char *buf, size_t buf_len, time_t *t);

/*!
  @fn vscp_getISOTimeString
  Get ISO GMT datetime
  @param buf Buffer that will get result
  @param buf_len Size of buffer
  @param t Unix time
  @return True if all is OK; false otherwise.
*/
bool
vscp_getISOTimeString(char *buf, size_t buf_len, time_t *t);

/*!
  @fn vscp_parseTimeString
  Parse ISO combined string (YYYY-MM-DDTHH:MM:SS)
  @param dt Datestring to parse.
  @param ptm Pointer to tm structure that will receive result.
  @return True on success, false on failure.
*/
bool
vscp_parseISOCombined(struct tm *ptm, std::string &dt);

/*!
  @fn vscp_to_unix_ns
  Convert date and time to unix 64-bit time in nanoseconds
  @param year Year
  @param month Month
  @param day Day
  @param hour Hour
  @param minute Minute
  @param second Second
  @param timestamp Timestamp in microseconds
  @return Unix time in nanoseconds or -1 on error.
*/
int64_t
vscp_to_unix_ns(int year, int month, int day, int hour, int minute, int second, uint32_t timestamp);

/*!
  @fn vscp_from_unix_ns
  Convert unix time in nanoseconds to date and time
  @param year Year
  @param month Month
  @param day Day
  @param hour Hour
  @param minute Minute
  @param second Second
  @param timestamp Timestamp in microseconds
  @param unix_ns Unix time in nanoseconds
*/

void
vscp_from_unix_ns(int *year,
                  int *month,
                  int *day,
                  int *hour,
                  int *minute,
                  int *second,
                  uint32_t *timestamp,
                  int64_t unix_ns);

/*!
  @fn vscp_XML_Escape
  Escape XML string
  @param dst Resulting string. Buffer size must be large enough to
          hold expanded result.
  @param fst_len Size of dst buffer.
  @param src Pointer to string that should be converted.
  @return True on success, false on failure.
  */
bool
vscp_XML_Escape(char *dst, size_t dst_len, const char *src);

/*!
  @fn vscp_hostname_to_ip
  Get ip from domain name

  @param ip IP address for resolved hostname
  @param hostname Hostname to resolve
  @return VSCP_ERROR_SUCCESS if it manage to resolve the hostname and
          VSCP_ERROR_ERROR else.
*/
int
vscp_hostname_to_ip(char *ip, const char *hostname);

/*!
    @fn vscp_getPortFromInterface
    Get port from interface string on form host:port
    @param interface on prefix:\\host:port form
    @return port number if any found or -1 if not found or error
*/
int
vscp_getPortFromInterface(const std::string &iface);

/*!
  @fn vscp_getHostFromInterface

  Get host part from interface on form protocol://host:port
  (tcp://, stcp:// etc)

  @param interface Interface on form [protocol://]host[:port]
  @return host as string
*/
std::string
vscp_getHostFromInterface(const std::string &iface);

/*!
  @fn vscp_getProtocolFromInterface
  Get protocol part from interface on form protocol://host:port
  (tcp://, stcp:// etc)

  @param interface Interface on form [protocol://]host[:port]
  @return protocol as string
*/
std::string
vscp_getProtocolFromInterface(const std::string &iface);

/*!
  @fn vscp_parse_ipv4_addr
  Parse IPv4 address and return

  @param addr ipv4 address to parse (a.b.c.d/m)
  @param net Network part of address
  @param mask Mask part of address
  @return 0 on error,
 */
int
vscp_parse_ipv4_addr(const char *addr, uint32_t *net, uint32_t *mask);

/*!
  @fn vscp_parse_ipv6_addr
  Parse IPv4/IPv6 address
  @param addr IPv4 (+a.b.c.d/x) or IPv6 (+fe80::/64) address plus net. Firts char should
      be plus or minus to indicate allow or diallow.
  @param sa Structure holding address to check
  @param no_strict Set to zero to disable strict control for IPv6
  @return TRUE if addr is OK
*/
int
vscp_parse_match_net(const char *addr, const union usa *sa, int no_strict);

// ***************************************************************************
//                             Measurement Helpers
// ***************************************************************************

/*!
  @fn vscp_getMeasurementDataCoding
  Fetch data coding byte from measurement events
  @param pev Pointer to VSCP event
  @return Measurement data coding byte or zero if its not an
  event with a data coding.
*/
uint8_t
vscp_getMeasurementDataCoding(const vscpEvent *pev);

/*!
  @fn vscp_getDataCodingBitArray
  Get bit array from coded event data
  @param pCode Pointer to data coding byte.
  @param length Number of bytes it consist of including
  the first normalise byte.
  @return Bit array as a unsigned 64-bit integer.
*/
uint64_t
vscp_getDataCodingBitArray(const uint8_t *pCode, uint8_t length);

/*!
  @fn vscp_getDataCodingInteger
  Get integer from coded event data
  @param pCode Pointer to normalised integer.
  @param length Number of bytes it consist of including
  the first normalise byte.
  @return returns value as a 64-bit integer.
*/
int64_t
vscp_getDataCodingInteger(const uint8_t *pCode, uint8_t length);

/*!
  @fn vscp_getDataCodingNormalizedInteger
  Get normalised integer from coded event data
  @param pCode Pointer to normalised integer.
  @param length Number of bytes it consist of including
  the first normalise byte.
  @return returns value as a double.
*/
double
vscp_getDataCodingNormalizedInteger(const unsigned char *pCode, unsigned char length);

/*!
  @fn vscp_getDataCodingString
  Get the string from coded event data
  @param strResult Returned string result
  @param pCode Pointer to normalized integer.
  @param dataSize Number of bytes it consist of including
  the first normalise byte.
  @return Returns unicode UTF-8 string of event data
*/

bool
vscp_getDataCodingString(std::string &strResult, const unsigned char *pCode, unsigned char dataSize);

/*!
  @fn vscp_getMeasurementAsString
  Write data from event in the VSCP data coding format to a string.

  Works for

  CLASS1.MEASUREMENT
  CLASS2_LEVEL1.MEASUREMENT
  VSCP_CLASS1_MEASUREZONE
  VSCP_CLASS1_SETVALUEZONE
  CLASS2_MEASUREMENT_FLOAT
  CLASS2_MEASUREMENT_STR

  @param str String that holds the result
  @param pev Pointer to VSCP event.
  @return true on success, false on failure.
*/

bool
vscp_getMeasurementAsString(std::string &str, const vscpEvent *pev);

/*!
  @fn vscp_getMeasurementAsStringEx
  Write data from event ex in the VSCP data coding format to a string.

  Works for

  CLASS1.MEASUREMENT
  CLASS2_LEVEL1.MEASUREMENT
  VSCP_CLASS1_MEASUREZONE
  VSCP_CLASS1_SETVALUEZONE
  CLASS2_MEASUREMENT_FLOAT
  CLASS2_MEASUREMENT_STR

  @param pev Pointer to VSCP event.
  @param pvalue Pointer to double that holds the result
  @return true on success, false on failure.
*/
bool
vscp_getMeasurementAsDouble(double *pvalue, const vscpEvent *pev);

/*!
  @fn vscp_getMeasurementAsDoubleEx
  Write data from event ex in the VSCP data coding format as a double.

  Works for

  CLASS1.MEASUREMENT
  CLASS2_LEVEL1.MEASUREMENT
  VSCP_CLASS1_MEASUREZONE
  VSCP_CLASS1_SETVALUEZONE
  CLASS2_MEASUREMENT_FLOAT
  CLASS2_MEASUREMENT_STR

  @param pex Pointer to VSCP event ex.
  @param pvalue Pointer to double that holds the result
  @return true on success, false on failure.
*/
bool
vscp_getMeasurementAsDoubleEx(double *pvalue, const vscpEventEx *pex);

/*!
  @fn vscp_getMeasurementUnit
  Get measurement unit for any of the valid measurement events.
  @param pev Pointer to VSCP event.
  @return Measurement unit or -1 for error (event that is not a
  measurement).
 */
int
vscp_getMeasurementUnit(const vscpEvent *pev);

/*!
  @fn vscp_getMeasurementUnitEx
  Get measurement unit for any of the valid measurement events.
  @param pex Pointer to VSCP event ex.
  @return Measurement unit or -1 for error (event that is not a
  measurement).
 */
int
vscp_getMeasurementUnitEx(const vscpEventEx *pex);

/*!
  @fn vscp_getMeasurementSensorIndex
  Get measurement sensor index for any of the valid measurement events.
  @param pev Pointer to VSCP event.
  @return Measurement sensor index or -1 for error or for event that is not
  a measurement or measurement event that does not have a sensor index.
 */
int
vscp_getMeasurementSensorIndex(const vscpEvent *pev);

/*!
  @fn vscp_getMeasurementSensorIndexEx
  Get measurement sensor index for any of the valid measurement events.
  @param pex Pointer to VSCP event ex.
  @return Measurement sensor index or -1 for error or for event that is not
  a measurement or measurement event that does not have a sensor index.
 */
int
vscp_getMeasurementSensorIndexEx(const vscpEventEx *pex);

/*!
  @fn vscp_getMeasurementUnit
  Get measurement zone for any of the valid measurement events.
  @param pev Pointer to VSCP event.
  @return Measurement zone or 0 for error or event that is not a
  measurement or measurement event that does not have a zone.
 */
int
vscp_getMeasurementZone(const vscpEvent *pev);

/*!
  @fn vscp_getMeasurementZoneEx
  Get measurement zone for any of the valid measurement events.
  @param pex Pointer to VSCP event ex.
  @return Measurement zone or 0 for error or event that is not a
  measurement or measurement event that does not have a zone.
 */
int
vscp_getMeasurementZoneEx(const vscpEventEx *pex);

/*!
  @fn vscp_getMeasurementSubZone
  Get measurement subzone for any of the valid measurement events.
  @param pev Pointer to VSCP event.
  @return Measurement subzone or -1 for error or for event that is not a
  measurement or measurement event that does not have a subzone.
 */
int
vscp_getMeasurementSubZone(const vscpEvent *pev);

/*!
  @fn vscp_getMeasurementSubZoneEx
  Get measurement subzone for any of the valid measurement events.
  @param pex Pointer to VSCP event ex.
  @return Measurement subzone or -1 for error or for event that is not a
  measurement or measurement event that does not have a subzone.
 */
int
vscp_getMeasurementSubZoneEx(const vscpEventEx *pex);

/*!
  @fn vscp_isMeasurement
 Check if event is a measurement
 * @param pev Pointer to VSCP event.
 * @return Return true if the event is a measurement.
 */
bool
vscp_isMeasurement(const vscpEvent *pev);

/*!
  @fn vscp_isMeasurementEx
  Check if event is a measurement
  @param pex Pointer to VSCP event ex.
  @return Return true if the event is a measurement.
 */
bool
vscp_isMeasurementEx(const vscpEventEx *pex);

/*!
  @fn vscp_getMeasurementFloat64AsString
  Get data in the VSCP data coding format to a string. Works for
  CLASS1.MEASUREMENT64, CLASS2_LEVEL1.MEASUREMENT64
  @param pev Pointer to VSCP event.
  @param str String that holds the result
  @return true on success, false on failure.
 */
bool
vscp_getMeasurementFloat64AsString(std::string &str, const vscpEvent *pev);

/*!
  @fn vscp_getMeasurementFloat64AsDouble
  Convert a floating point measurement value into VSCP data with the
  first byte being the normalizer byte
  CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT
  @param value Floating point value to convert.
  @param pdata Pointer to beginning of VSCP returned event data.
  @param psize Pointer to size for returned data.
  @param unit Unit for the data. Zero is default.
  @param sensoridx Sensor index 0-7. Zero is default.
  @return true on success, false on failure.
*/
bool
vscp_convertFloatToNormalizedEventData(uint8_t *pdata, uint16_t *psize, double value, uint8_t unit, uint8_t sensoridx);

/*!
  @fn vscp_convertFloatToFloatEventData
  Convert a floating point measurement value into VSCP data as a
  single precision float (32-bit) for
  *  VSCP_CLASS1_MEASUREMENT, VSCP_CLASS2_LEVEL1.MEASUREMENT
  @param value Floating point value to convert.
  @param pdata Pointer to beginning of VSCP returned event data.
  @param psize Pointer to size for returned data.
  @param unit Unit for the data. Zero is default.
  @param sensoridx Sensor index 0-7. Zero is default.
  @return true on success, false on failure.
*/
bool
vscp_convertFloatToFloatEventData(uint8_t *pdata, uint16_t *psize, float value, uint8_t unit, uint8_t sensoridx);

/*!
  @fn vscp_convertIntegerToNormalizedEventData
  Convert an integer measurement value into VSCP data with the
  first byte being the normalizer byte
  CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT
  @param value integer value to convert.
  @param pdata Pointer to beginning of VSCP returned event data.
  @param psize Pointer to size for returned data.
  @param unit Unit for the data. Zero is default.
  @param sensoridx Sensor index 0-7. Zero is default.
  @return true on success, false on failure.
*/
bool
vscp_convertIntegerToNormalizedEventData(uint8_t *pdata,
                                         uint16_t *psize,
                                         uint64_t val64,
                                         uint8_t unit      = 0,
                                         uint8_t sensoridx = 0);

/*!
  @fn vscp_makeIntegerMeasurementEvent
  Make level I integer measurement event
  @param pev Pointer to event with pdata set to NULL. VSCP_class and
    vscp_type must be set to CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT,
  @param value Integer value to set
  @param unit Unit for the data. Zero is default.
  @param sensoridx Sensor index 0-7. Zero is default.
  @return true on success, false on failure.
*/
bool
vscp_makeIntegerMeasurementEvent(vscpEvent *pev, int64_t value, uint8_t unit, uint8_t sensoridx);

/*!
  @fn vscp_makeIntegerMeasurementEventEx
  Make level I integer measurement event ex
  @param pex Pointer to event with pdata set to NULL. VSCP_class and
    vscp_type must be set to CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT,
  @param value Integer value to set
  @param unit Unit for the data. Zero is default.
  @param sensoridx Sensor index 0-7. Zero is default.
  @return true on success, false on failure.
*/
bool
vscp_makeIntegerMeasurementEventEx(vscpEventEx *pex, int64_t value, uint8_t unit, uint8_t sensoridx);

/*!
  @fn vscp_makeFloatMeasurementEvent
  Convert a floating point measurement value into VSCP data as a
  single precision float (32-bit) for
      CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT
  @param value Floating point value to convert.
  @param pev Pointer to event with pdata set to NULL. VSCP_class and
  vscp_type must be set to CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT,
  @param psize Pointer to size for returned data.
  @param unit Unit for the data. Zero is default.
  @param sensoridx Sensor index 0-7. Zero is default.
  @return true on success, false on failure.
*/
bool
vscp_makeFloatMeasurementEvent(vscpEvent *pev, float value, uint8_t unit, uint8_t sensoridx);

/*!
  @fn vscp_makeFloatMeasurementEvent
  Convert a floating point measurement value into VSCP data as a
  single precision float (32-bit) for
      CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT
  @param value Floating point value to convert.
  @param pex Pointer to event ex. VSCP_class and
  vscp_type must be set to CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT,
  @param psize Pointer to size for returned data.
  @param unit Unit for the data. Zero is default.
  @param sensoridx Sensor index 0-7. Zero is default.
  @return true on success, false on failure.
*/

bool
vscp_makeFloatMeasurementEventEx(vscpEventEx *pex, float value, uint8_t unit, uint8_t sensoridx);
/*!
  @fn vscp_makeStringMeasurementEvent
  Convert a floating point measurement value into VSCP data as a
  string for CLASS1.MEASUREMENT or CLASS2_LEVEL1.MEASUREMENT
  @param value Floating point value to convert.
  @param pev Pointer to event with pdata set to NULL. vscp_class and
  vscp_type must be set to CLASS1.MEASUREMENT or
  CLASS2_LEVEL1.MEASUREMENT,
  @param psize Pointer to size for returned data.
  @param unit Unit for the data. Zero is default.
  @param sensoridx Sensor index 0-7. Zero is default.
  @return true on success, false on failure.
*/
bool
vscp_makeStringMeasurementEvent(vscpEvent *pev, double value, uint8_t unit, uint8_t sensoridx);

/*!
  @fn vscp_makeStringMeasurementEventEx
  Convert a floating point measurement value into VSCP data as a
  string for CLASS1.MEASUREMENT or CLASS2_LEVEL1.MEASUREMENT
  @param value Floating point value to convert.
  @param pex Pointer to event ex. vscp_class and
  vscp_type must be set to CLASS1.MEASUREMENT or
  CLASS2_LEVEL1.MEASUREMENT,
  @param psize Pointer to size for returned data.
  @param unit Unit for the data. Zero is default.
  @param sensoridx Sensor index 0-7. Zero is default.
  @return true on success, false on failure.
*/
bool
vscp_makeStringMeasurementEventEx(vscpEventEx *pex, double value, uint8_t unit, uint8_t sensoridx);

/*!
  @fn vscp_makeLevel2FloatMeasurementEvent
  Construct CLASS2.MEASUREMENT_FLOAT level II measurement event

  @param pev Pointer to event that will be filled with data or NULL
            if event should be allocated.
  @param vscp_type Measurement type
  @param value Floating point value that event should be made from.
  @param unit Unit for this measurement. 0-255. Default is zero.
  @param sensorsidx Sensor index for this measurement. 0-255. Default us
  zero.
  @param zone Zone for this measurement. 0-255. Zero is default.
  @param Subzone Sub zone for this measurement. 0-255- Zero is default.
  @return True is returned on success, false if an error occurred.
 */
bool
vscp_makeLevel2FloatMeasurementEvent(vscpEvent *pev,
                                     uint16_t vscp_type,
                                     double value,
                                     uint8_t unit,
                                     uint8_t sensoridx,
                                     uint8_t zone,
                                     uint8_t subzone);

/*!
  @fn vscp_makeLevel2FloatMeasurementEventEx
  Construct CLASS2.MEASUREMENT_FLOAT level II measurement event

  @param pex Pointer to event that will be filled with data or NULL
            if event should be allocated. 
  @param vscp_type Measurement type
  @param value Floating point value that event should be made from.
  @param unit Unit for this measurement. 0-255. Default is zero.
  @param sensorsidx Sensor index for this measurement. 0-255. Default us
  zero.
  @param zone Zone for this measurement. 0-255. Zero is default.
  @param Subzone Sub zone for this measurement. 0-255- Zero is default.
  @return True is returned on success, false if an error occurred.
 */
bool
vscp_makeLevel2FloatMeasurementEventEx(vscpEventEx *pex,
                                       uint16_t vscp_type,
                                       double value,
                                       uint8_t unit,
                                       uint8_t sensoridx,
                                       uint8_t zone,
                                       uint8_t subzone);

/*!
  @fn vscp_makeLevel2StringMeasurementEvent
  Construct CLASS2.MEASUREMENT_STRING level II measurement event

  @param pev Pointer to event that will be filled with data or NULL
            if event should be allocated.
  @param vscp_type Measurement type
  @param value Floating point value that event should be made from.
  @param unit Unit for this measurement. 0-255. Default is zero.
  @param sensorsidx Sensor index for this measurement. 0-255. Default us
  zero.
  @param zone Zone for this measurement. 0-255. Zero is default.
  @param Subzone Sub zone for this measurement. 0-255- Zero is default.
  @return True is returned on success, false if an error occurred.
 */
bool
vscp_makeLevel2StringMeasurementEvent(vscpEvent *pev,
                                      uint16_t vscp_type,
                                      double value,
                                      uint8_t unit,
                                      uint8_t sensoridx,
                                      uint8_t zone,
                                      uint8_t subzone);

/*!
  @fn vscp_makeLevel2StringMeasurementEventEx
  Construct CLASS2.MEASUREMENT_STRING level II measurement event

  @param pex Pointer to event that will be filled with data or NULL
            if event should be allocated.
  @param vscp_type Measurement type
  @param value Floating point value that event should be made from.
  @param unit Unit for this measurement. 0-255. Default is zero.
  @param sensorsidx Sensor index for this measurement. 0-255. Default us
  zero.
  @param zone Zone for this measurement. 0-255. Zero is default.
  @param Subzone Sub zone for this measurement. 0-255- Zero is default.
  @return True is returned on success, false if an error occurred.
 */
bool
vscp_makeLevel2StringMeasurementEventEx(vscpEventEx *pex,
                                        uint16_t vscp_type,
                                        double value,
                                        uint8_t unit,
                                        uint8_t sensoridx,
                                        uint8_t zone,
                                        uint8_t subzone);

/*!
  @fn vscp_getMeasurementAsFloat
  Get data in the VSCP data coding set as float format to a float
  @param pNorm Pointer to VSCP event.
  @param length Number of bytes it consist of including data coding byte
  @return value as float
*/
float
vscp_getMeasurementAsFloat(const unsigned char *pNorm, const unsigned char length);

/*!
  @fn vscp_convertLevel1MeasurementToLevel2Double
  Convert a Level I measurement event to a Level II double measurement
  event VSCP_CLASS2_MEASUREMENT_FLOAT

  @param pev Pointer to level I measurement event to be converted.
  @return true on success, false otherwise.
 */
bool
vscp_convertLevel1MeasurementToLevel2Double(vscpEvent *pev);

/*!
  @fn vscp_convertLevel1MeasurementToLevel2DoubleEx
  Convert a Level I measurement event to a Level II double measurement
  event VSCP_CLASS2_MEASUREMENT_FLOAT

  @param pex Pointer to level I measurement event to be converted.
  @return true on success, false otherwise.
 */
bool
vscp_convertLevel1MeasurementToLevel2DoubleEx(vscpEventEx *pex);

/*!
  @fn vscp_convertLevel1MeasurementToLevel2Float
  Convert a Level I measurement event to a Level II float measurement
  event VSCP_CLASS2_MEASUREMENT_FLOAT

  @param pev Pointer to level I measurement event to be converted.
  @return true on success, false otherwise.
 */
bool
vscp_convertLevel1MeasurementToLevel2Float(vscpEvent *pev);

/*!
  @fn vscp_convertLevel1MeasurementToLevel2String
  Convert a Level I measurement event to a Level II string measurement
  event VSCP_CLASS2_MEASUREMENT_STRING

  @param pev Pointer to level I measurement event to be converted.
  @param pev2 Pointer to resulting level II measurement event.
  @return true on success, false otherwise.
 */
bool
vscp_convertLevel1MeasurementToLevel2String(vscpEvent *pev, vscpEvent *pev2);

/*!
  @fn vscp_convertLevel1MeasurementToLevel2StringEx
  Convert a Level I measurement event to a Level II string measurement
  event VSCP_CLASS2_MEASUREMENT_STRING

  @param pex Pointer to level I measurement event to be converted.
  @return true on success, false otherwise.
 */
bool
vscp_convertLevel1MeasurementToLevel2StringEx(vscpEventEx *pex);

// -------------------------------------------------------------------------

/*!
  @fn vscp_replaceBackslash
  Replace backslashes in a string with forward slashes
  @param strval String that should be handled.
  @return Fixed string.
*/
std::string &
vscp_replaceBackslash(std::string &strval);

/*!
  @fn vscp_getEventPriority
  Get VSCP priority
  @param pev Pointer to VSCP event to set priority for.
  @return Priority (0-7) for event.
*/
unsigned char
vscp_getEventPriority(const vscpEvent *pev);

/*!
  @fn vscp_getEventExPriority
  Get VSCP priority
  @param pex Pointer to VSCP event ex to set priority for.
  @return Priority (0-7) for event.
*/
unsigned char
vscp_getEventExPriority(const vscpEventEx *pex);

/*!
  @fn vscp_setEventPriority
  Set VSCP priority
  @param pev Pointer to VSCP event to set priority for.
  @param priority Priority (0-7) to set.
*/
void
vscp_setEventPriority(vscpEvent *pev, unsigned char priority);

/*!
  @fn vscp_setEventExPriority
  Set VSCP priority Ex
  @param pex Pointer to VSCP event ex to set priority for.
  @param priority Priority (0-7) to set.
*/
void
vscp_setEventExPriority(vscpEventEx *pex, unsigned char priority);

/*!
  @fn vscp_getHeadFromCANALid
  Get VSCP head from CANAL id
  @param id CAN id
  @return VSCP head
*/
unsigned char
vscp_getHeadFromCANALid(uint32_t id);

/*!
  @fn vscp_getVscpClassFromCANALid
  Get VSCP class from CANAL id
  @param id CAN id
    @return VSCP class
*/
uint16_t
vscp_getVscpClassFromCANALid(uint32_t id);

/*!
  @fn vscp_getVscpTypeFromCANALid
    Get VSCP type from CANAL id
    @param id CAN id
    @return VSCP type
*/
uint16_t
vscp_getVscpTypeFromCANALid(uint32_t id);

/*!
  Get VSCP nide nickname from CANAL id
  @param id CAN id
  @return VSCP node nickname
*/
uint8_t
vscp_getNicknameFromCANALid(uint32_t id);

/*!
  @fn vscp_getCANALidFromData
  Get CANAL id from VSCP info
  @param priority VSCP priority
  @param vscp_class VSCP class
  @param vscp_type VSCP type
  @return CAN id with nickname == 0
*/
uint32_t
vscp_getCANALidFromData(unsigned char priority, const uint16_t vscp_class, const uint16_t vscp_type);

/*!
  @fn vscp_getCANALidFromEvent
  Get CANAL id from VSCP event
  @param pev Pointer to VSCP event
  @return CAN id with nickname == 0
*/
uint32_t
vscp_getCANALidFromEvent(const vscpEvent *pev);

/*!
  @fn vscp_getCANALidFromEventEx
  Get CAN id from VSCP event
  @param pex Pointer to VSCP event
  @return CAN id with nickname == 0
*/
uint32_t
vscp_getCANALidFromEventEx(const vscpEventEx *pex);

/*!
  @fn vscp_calc_crc_Event
  Calculate CRC for VSCP event

  @param pev Pointer to VSCP event
  @param bSet If true, the calculated CRC will be set in the event's crc field. If false, the CRC will be calculated but
  not set in the event.
  @return Calculated CRC value for the event.
*/
unsigned short
vscp_calc_crc_Event(vscpEvent *pev, short bSet);

/*!
  @fn vscp_calc_crc_EventEx
  Calculate CRC for VSCP event Ex

  @param pex Pointer to VSCP event Ex
  @param bSet If true, the calculated CRC will be set in the event's crc field. If false, the CRC will be calculated but
  not set in the event.
  @return Calculated CRC value for the event.
*/
unsigned short
vscp_calc_crc_EventEx(vscpEventEx *pex, short bSet);

// Helpers

/*!
  @fn vscp_calcCRC4GUIDArray
  Calculate CRC for GUID array

  @param pguid Pointer to GUID array (MSB-LSB order)
  @return CRC for GUID.
*/
uint8_t
vscp_calcCRC4GUIDArray(const uint8_t *pguid);

/*!
  @fn vscp_calcCRC4GUIDString
  Calculate CRC for GUID string

  @param strguid String with GUID (xx:yy:zz....)
  @return CRC for GUID.
*/
uint8_t
vscp_calcCRC4GUIDString(const std::string &strguid);

/*!
  @fn vscp_setEventGuidFromString
  Get GUID from string

  @param pev Pointer to VSCP event
  @param strGUID String with GUID (xx:yy:zz....)
  @return True on success, false on failure.
*/
bool
vscp_setEventGuidFromString(vscpEvent *pev, const std::string &strGUID);

/*!
  @fn vscp_setEventExGuidFromString
  Get GUID from string

  @param pex Pointer to VSCP event
  @param strGUID String with GUID (xx:yy:zz....)
  @return True on success, false on failure.
*/
bool
vscp_setEventExGuidFromString(vscpEventEx *pex, const std::string &strGUID);

/*!
  @fn vscp_getGuidFromStringToArray
  Fill GUID array from a string. Accepts all common GUID string formats:

  - Standard colon-separated: "FF:AA:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD:EE"
  - UUID format: "FFAA1122-3344-5566-7788-99AABBCCDDEE"
  - Compact format with :: for leading 0xFF bytes: "::11:22:33" results in
    FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:11:22:33
  - "-" alone means all zeros: 00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
  - "-:" prefix for leading zeros: "-:11:22:33" results in
    00:00:00:00:00:00:00:00:00:00:00:00:00:11:22:33
  - With braces: "{FF:AA:11:22:...}"
  - Various separators supported: colon (:), dash (-), comma (,)
  - Empty string results in all zeros

  @param pGUID Pointer to GUID array (must be at least 16 bytes)
  @param strGUID String with GUID in any of the supported formats
  @return True on success, false on failure (nullptr pGUID).
*/
bool
vscp_getGuidFromStringToArray(unsigned char *pGUID, const std::string &strGUID);

/*!
  @fn vscp_parseGuid
  Parse a GUID string into a 16-byte array using the extended VSCP GUID parser.

  @param pGUID Pointer to GUID array (must be at least 16 bytes)
  @param strGUID String with GUID in a supported VSCP format
  @return True on success, false on failure.
*/
bool
vscp_parseGuid(uint8_t *pGUID, const std::string &strGUID);

/*!
  @fn vscp_writeGuidArrayToString
  Write out GUID array to string in specified format.

  Supported formats:
  - VSCP_GUID_STR_FORMAT_NORMAL (default): Standard colon-separated
    "FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF"
  - VSCP_GUID_STR_FORMAT_COMPACT: Uses :: for leading 0xFF bytes
    "::01:02:03" for a GUID starting with 0xFF bytes
  - VSCP_GUID_STR_FORMAT_UUID: UUID format (8-4-4-4-12 hex digits)
    "FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF"

  Note: bUseComma only applies to VSCP_GUID_STR_FORMAT_NORMAL format.

  @param strGUID Reference to string that will get GUID on string form
  @param pGUID Pointer to VSCP GUID array.
  @param bUseComma Use comma as a separator instead of colon with
                      values in decimal (only for NORMAL format).
  @param format Output format (default: VSCP_GUID_STR_FORMAT_NORMAL)
  @return True on success, false on failure.
*/
bool
vscp_writeGuidArrayToString(std::string &strGUID,
                            const unsigned char *pGUID,
                            bool bUseComma              = false,
                            vscp_guid_str_format format = VSCP_GUID_STR_FORMAT_NORMAL);

/*!
  @fn vscp_writeGuidToString
  Write out GUID to string in specified format.

  Supported formats:
  - VSCP_GUID_STR_FORMAT_NORMAL (default): Standard colon-separated
    "FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF"
  - VSCP_GUID_STR_FORMAT_COMPACT: Uses :: for leading 0xFF bytes
    "::01:02:03" for a GUID starting with 0xFF bytes
  - VSCP_GUID_STR_FORMAT_UUID: UUID format (8-4-4-4-12 hex digits)
    "FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF"

  @param strGUID Reference to string for written GUID
  @param pev Pointer to VSCP event
  @param format Output format (default: VSCP_GUID_STR_FORMAT_NORMAL)
  @return True on success, false on failure.
*/

bool
vscp_writeGuidToString(std::string &strGUID,
                       const vscpEvent *pev,
                       vscp_guid_str_format format = VSCP_GUID_STR_FORMAT_NORMAL);

/*!
  @fn vscp_writeGuidToStringEx
  Write out GUID to string in specified format.

  Supported formats:
  - VSCP_GUID_STR_FORMAT_NORMAL (default): Standard colon-separated
    "FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF"
  - VSCP_GUID_STR_FORMAT_COMPACT: Uses :: for leading 0xFF bytes
    "::01:02:03" for a GUID starting with 0xFF bytes
  - VSCP_GUID_STR_FORMAT_UUID: UUID format (8-4-4-4-12 hex digits)
    "FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF"

  @param strGUID Reference to string for written GUID
  @param pex Pointer to VSCP event ex
  @param format Output format (default: VSCP_GUID_STR_FORMAT_NORMAL)
  @return True on success, false on failure.
*/

bool
vscp_writeGuidToStringEx(std::string &strGUID,
                         const vscpEventEx *pex,
                         vscp_guid_str_format format = VSCP_GUID_STR_FORMAT_NORMAL);

/*!
  @fn vscp_writeGuidToString4Rows
  Write out GUID to string as four rows

  @param strGUID Reference to string for written GUID
  @param pev Pointer to VSCP event
  @return True on success, false on failure.
*/

bool
vscp_writeGuidToString4Rows(std::string &strGUID, const vscpEvent *pev);

/*!
  @fn vscp_writeGuidToString4RowsEx
  Write out GUID to string as four rows

  @param strGUID Reference to string for written GUID
  @param pex Pointer to VSCP event
  @return True on success, false on failure.
*/

bool
vscp_writeGuidToString4RowsEx(std::string &strGUID, const vscpEventEx *pex);

/*!
  @fn vscp_isGUIDEmpty
  Check if GUID is all null
  @param pGUID pointer to GUID to check
  @return true if empty, false if not.
    */
bool
vscp_isGUIDEmpty(const unsigned char *pGUID);

/*!
  @fn vscp_isSameGUID
  Compare two GUID's
  @param pGUID1 First GUID to compare
  @param pGUID2 Second GUID to compare
  @return True if the two GUID's are equal. False otherwise.
*/
bool
vscp_isSameGUID(const unsigned char *pGUID1, const unsigned char *pGUID2);

/*!
  @fn vscp_reverseGUID
  Reverse GUID
  @param pGUID Pointer to GUID to reverse.
  @return true if OK.
*/
bool
vscp_reverseGUID(unsigned char *pGUID);

/*!
  @fn vscp_convertEventToEventEx
  Convert a standard VSCP event to the Ex version
  @param pex Pointer to VSCP eventEx to fill in.
  @param pev Pointer to standard VSCP event to convert.
  @return True on success, false on failure.
*/
bool
vscp_convertEventToEventEx(vscpEventEx *pex, const vscpEvent *pev);

/*!
  @fn vscp_convertEventExToEvent
  Convert an Ex event to a standard VSCP event

  @param pev Pointer to standard VSCP event to fill in.
  @param pex Pointer to VSCP eventEx to convert.
  @return True on success, false on failure.
*/
bool
vscp_convertEventExToEvent(vscpEvent *pev, const vscpEventEx *pex);

/*!
  @fn vscp_newEvent
  Create a standard VSCP event
  @param ppev Pointer to a pointer toa standard VSCP event.
  @param version Frame version for the new event (default: VSCP_HEADER16_FRAME_VERSION_ORIGINAL)
  @return True if the event was created successfully,
                false otherwise.
 */
bool
vscp_newEvent(vscpEvent **ppev, uint16_t = VSCP_HEADER16_FRAME_VERSION_UNIX_NS);

/*!
  @fn vscp_newEventEx
  Create an Ex event
  @param ppex Pointer to a pointer to an Ex event.
  @return True if the event was created successfully, false otherwise.
 */
bool
vscp_newEventEx(vscpEventEx **ppex, uint16_t version = VSCP_HEADER16_FRAME_VERSION_UNIX_NS);

/*!
  @fn vscp_deleteEvent
  Delete a standard VSCP event
  @param pev Pointer to standard VSCP event to delete.
*/
void
vscp_deleteEvent(vscpEvent *pev);

/*!
  @fn vscp_deleteEventEx
  Delete an Ex VSCP event
  @param pex Pointer to Ex VSCP event to delete.
*/
void
vscp_deleteEventEx(vscpEventEx *pex);

/*!
  @fn vscp_deleteEvent_v2
  Delete standard VSCP event and NULL
  @param ppev Pointer to pointer to standard VSCP event to delete and NULL after deletion.
 */
void
vscp_deleteEvent_v2(vscpEvent **ppev);



/*!
  @fn vscp_setFrameVersion
  Set frame version for a standard VSCP event or Ex event

  @param pev Pointer to standard VSCP event or Ex event to set frame version for.
  @param version Frame version to set for the event.
  @return True on success, false on failure.
*/
bool
setFrameVersion(vscpEvent *pev, uint16_t version = VSCP_HEADER16_FRAME_VERSION_UNIX_NS);

/*!
  @fn vscp_setFrameVersionEx
  Set frame version for a standard VSCP event or Ex event

  @param pex Pointer to standard VSCP event or Ex event to set frame version for.
  @param version Frame version to set for the event.
  @return True on success, false on failure.
*/
bool
setFrameVersion(vscpEventEx *pex, uint16_t version = VSCP_HEADER16_FRAME_VERSION_UNIX_NS);


/*!
  @fn vscp_makeTimeStamp
  Make a timestamp for events etc
  @return Event timestamp as an unsigned long
*/
unsigned long
vscp_makeTimeStamp(void);

/*!
  @fn vscp_getMsTimeStamp
  Get milliseconds timestamp for events etc
  @return Timestamp as uint32_t
*/
uint32_t
vscp_getMsTimeStamp(void);

/*!
  @fn vscp_makeTimeStampNs
  Make a nanosecond timestamp for UNIX_NS frame events.
  Returns nanoseconds since Unix epoch (January 1, 1970).
  @return Event timestamp as uint64_t nanoseconds since epoch
*/
uint64_t
vscp_makeTimeStampNs(void);

/*!
  @fn vscp_setEventDateTimeBlockToNow
  Set date & time in stamp block.

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), sets the
  individual year/month/day/hour/minute/second fields to the current UTC time.

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), sets
  timestamp_ns to the current time in nanoseconds since Unix epoch.
  Also sets year=0xffff, month=0xff to indicate UNIX_NS frame.

  @param pev Event to set date/time block in.
  @return True on success.
*/
bool
vscp_setEventDateTimeBlockToNow(vscpEvent *pev);

/*!
  @fn vscp_setEventExDateTimeBlockToNow
  Set date & time in stamp block.

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), sets the
  individual year/month/day/hour/minute/second fields to the current UTC time.

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), sets
  timestamp_ns to the current time in nanoseconds since Unix epoch.
  Also sets year=0xffff, month=0xff to indicate UNIX_NS frame.

  @param pex EventEx to set date/time block in.
  @return True on success.
*/
bool
vscp_setEventExDateTimeBlockToNow(vscpEventEx *pex);

/*!
  @fn vscp_getDateStringFromEvent
  Get datestring from VSCP event.

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), uses
  year/month/day/hour/minute/second fields and returns format "YYYY-MM-DDTHH:MM:SSZ".
  Returns empty string if all fields are zero.

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), converts
  timestamp_ns (nanoseconds since Unix epoch) to ISO 8601 format with nanosecond
  precision: "YYYY-MM-DDTHH:MM:SS.nnnnnnnnnZ". Returns empty string if timestamp_ns is zero.

  @param dt Reference to String that will get ISO datetime string
  @param pev Event to get date/time info from
  @return True on success.
*/
bool
vscp_getDateStringFromEvent(std::string &dt, const vscpEvent *pev);

/*!
  @fn vscp_getDateStringFromEventEx
  Get datestring from VSCP EventEx.

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), uses
  year/month/day/hour/minute/second fields and returns format "YYYY-MM-DDTHH:MM:SSZ".
  Returns empty string if all fields are zero.

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), converts
  timestamp_ns (nanoseconds since Unix epoch) to ISO 8601 format with nanosecond
  precision: "YYYY-MM-DDTHH:MM:SS.nnnnnnnnnZ". Returns empty string if timestamp_ns is zero.

  @param dt Reference to String that will get ISO datetime string
  @param pex EventEx to get date/time info from
  @return True on success.
*/
bool
vscp_getDateStringFromEventEx(std::string &dt, const vscpEventEx *pex);

/*!
  @fn vscp_convertEventToJSON
  Convert VSCP Event to JSON formatted string.

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), outputs
  the 32-bit timestamp field in the JSON "timestamp" field.

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), outputs
  the 64-bit timestamp_ns (nanoseconds since Unix epoch) in the JSON "timestamp" field.

  @param strJSON Reference to string that will get JSON formatted event
  @param pev Event to convert to JSON
  @return True on success. False on failure.
 */
bool
vscp_convertEventToJSON(std::string &strJSON, const vscpEvent *pev);

/*!
    @fn vscp_convertEventExToJSON
    Convert VSCP EventEx to JSON formatted string.

    For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), outputs
    the 32-bit timestamp field in the JSON "timestamp" field.

    For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), outputs
    the 64-bit timestamp_ns (nanoseconds since Unix epoch) in the JSON "timestamp" field.

    @param strJSON Reference to string that will get JSON formatted event
    @param pex EventEx to convert to JSON
    @return True on success. False on failure.
 */
bool
vscp_convertEventExToJSON(std::string &strJSON, const vscpEventEx *pex);

/*!
  @fn vscp_convertJSONToEvent
  Convert JSON string to event.

  The frame version is determined by the "head" field in the JSON:
  - For ORIGINAL frame version (head & 0x0300 == 0x0000), the "timestamp" field
    is parsed as a 32-bit value and stored in the timestamp field.
  - For UNIX_NS frame version (head & 0x0300 == 0x0100), the "timestamp" field
    is parsed as a 64-bit nanoseconds-since-epoch value and stored in timestamp_ns.
    The year and month fields are set to 0xffff and 0xff respectively.

  @param pev Pointer to event that will be filled with data from JSON string.
  @param strJSON Reference to string with JSON formatted event data.
  @return True on success. False on failure.

  If a value is not found in the JSON string
  it will be set to zero. This means a value
  can be omitted in the JSON string. The only
  values that must be in the JSON string is
  class and type as long as the GUID is known
  beforehand
 */
bool
vscp_convertJSONToEvent(vscpEvent *pev, std::string &strJSON);

/*!
  @fn vscp_convertJSONToEventEx
  Convert JSON string to eventex.

  The frame version is determined by the "head" field in the JSON:
  - For ORIGINAL frame version (head & 0x0300 == 0x0000), the "timestamp" field
    is parsed as a 32-bit value and stored in the timestamp field.
  - For UNIX_NS frame version (head & 0x0300 == 0x0100), the "timestamp" field
    is parsed as a 64-bit nanoseconds-since-epoch value and stored in timestamp_ns.
    The year and month fields are set to 0xffff and 0xff respectively.

  @param pex Pointer to eventex that will be filled with data from JSON string.
  @param strJSON Reference to string with JSON formatted event data.
  @return True on success. False on failure.

  If a value is not found in the JSON string
  it will be set to zero. This means a value
  can be omitted in the JSON string. The only
  values that must be in the JSON string is
  class and type as long as the GUID is known
  beforehand
 */
bool
vscp_convertJSONToEventEx(vscpEventEx *pex, std::string &strJSONx);

/*!
  @fn vscp_convertEventToXML
  Convert VSCP Event to XML formatted string.

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), outputs
  the 32-bit timestamp field in the XML "timestamp" attribute.

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), outputs
  the 64-bit timestamp_ns (nanoseconds since Unix epoch) in the XML "timestamp" attribute.

  @param strXML Reference to string that will get XML formatted event
  @param pev Event to convert to XML
  @return True on success. False on failure.
 */
bool
vscp_convertEventToXML(std::string &strXML, vscpEvent *pev);

/*!
  @fn vscp_convertXMLToEvent
  Convert XML string to Event.

  For ORIGINAL frame version (head & VSCP_HEADER16_FRAME_VERSION_MASK == 0),
  parses the timestamp as a 32-bit value.

  For UNIX_NS frame version (head & VSCP_HEADER16_FRAME_VERSION_MASK == VSCP_HEADER16_FRAME_VERSION_UNIX_NS),
  parses the timestamp as a 64-bit nanosecond value and sets year=0xffff, month=0xff.

  @param pev Pointer to event that will be filled with data from XML string.
  @param strXML Reference to string with XML formatted event data.
  @return True on success. False on failure.
 */
bool
vscp_convertXMLToEvent(vscpEvent *pev, std::string &strXML);

/*!
  @fn vscp_convertEventExToXML
  Convert VSCP EventEx to XML formatted string.

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), outputs
  the 32-bit timestamp field in the XML "timestamp" attribute.

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), outputs
  the 64-bit timestamp_ns (nanoseconds since Unix epoch) in the XML "timestamp" attribute.

  @param strXML Reference to string that will get XML formatted event
  @param pex EventEx to convert to XML
  @return True on success. False on failure.
 */
bool
vscp_convertEventExToXML(std::string &strXML, vscpEventEx *pex);

/*!
  @fn vscp_convertXMLToEventEx
  Convert XML string to EventEx.

  For ORIGINAL frame version (head & VSCP_HEADER16_FRAME_VERSION_MASK == 0),
  parses the timestamp as a 32-bit value.

  For UNIX_NS frame version (head & VSCP_HEADER16_FRAME_VERSION_MASK == VSCP_HEADER16_FRAME_VERSION_UNIX_NS),
  parses the timestamp as a 64-bit nanosecond value and sets year=0xffff, month=0xff.

  @param pex EventEx to convert to XML
  @param strXML Reference to string with XML formatted event data.
  @return True on success. False on failure.
 */
bool
vscp_convertXMLToEventEx(vscpEventEx *pex, std::string &strXML);

/*!
  @fn vscp_convertEventToHTML
  Convert VSCP Event to HTML formatted string.

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), outputs
  the 32-bit timestamp field.

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), outputs
  the 64-bit timestamp_ns (nanoseconds since Unix epoch).

  @param strHTML Reference to string that will get HTML formatted event
  @param pev Event to convert to HTML
  @return True on success. False on failure.
 */
bool
vscp_convertEventToHTML(std::string &strHTML, vscpEvent *pev);

/*!
  @fn vscp_convertEventExToHTML
  Convert VSCP EventEx to HTML formatted string.

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), outputs
  the 32-bit timestamp field.

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), outputs
  the 64-bit timestamp_ns (nanoseconds since Unix epoch).

  @param strHTML Reference to string that will get HTML formatted event
  @param pex EventEx to convert to HTML
  @return True on success. False on failure.
 */
bool
vscp_convertEventExToHTML(std::string &strHTML, vscpEventEx *pex);

/*!
  @fn vscp_parse_topic
  @brief Parse topic string to extract GUID, VSCP class, and VSCP type

  @param topic Null terminated string with topic data.
  @param pguid Array to store extracted GUID (16 bytes).
  @param pVscpClass Pointer to store extracted VSCP class.
  @param pVscpType Pointer to store extracted VSCP type.
  @return int Returns VSCP_ERROR_SUCCESS on OK, error code else.
 */
int
vscp_parse_topic(const char *topic, uint8_t *pguid, uint16_t *pVscpClass, uint16_t *pVscpType);

/*!
  @fn vscp_set_event_info_from_topic
  @brief Parse topic string and set event GUID, VSCP class, and VSCP type

  @param pev Pointer to event to set data for.
  @param topic Null terminated string with topic data.
  @return int Returns VSCP_ERROR_SUCCESS on OK, error code else.

  topic is on form "vscp/guid/CLASS/TYPE" where guid is 16 bytes in hex format
  with ":" as separator, CLASS and TYPE are in decimal format. We can get this
  information from MQTT topic when MQTT is used as transport and the topic is
  on the above form. This allows us to set guid, class and type of the event
  from the topic without needing to include this information in the payload of
  the event. This is useful for MQTT where the topic is often used to route the
  event to the correct place and including guid, class and type in the topic is
  more efficient than including it in the payload.
 */
int
vscp_set_event_info_from_topic(vscpEvent *pev, const char *topic);

/*!
  @fn vscp_set_eventex_info_from_topic
  @brief Parse topic string and set event GUID, VSCP class, and VSCP type

  @param pex Pointer to event to set data for.
  @param topic Null terminated string with topic data.
  @return int Returns VSCP_ERROR_SUCCESS on OK, error code else.

  topic is on form "vscp/guid/CLASS/TYPE" where guid is 16 bytes in hex format
  with ":" as separator, CLASS and TYPE are in decimal format. We can get this
  information from MQTT topic when MQTT is used as transport and the topic is
  on the above form. This allows us to set guid, class and type of the event
  from the topic without needing to include this information in the payload of
  the event. This is useful for MQTT where the topic is often used to route the
  event to the correct place and including guid, class and type in the topic is
  more efficient than including it in the payload.
 */
int
vscp_set_eventex_info_from_topic(vscpEventEx *pex, const char *topic);

/*!
  @fn vscp_setEventDateTime
  Set event datetime from struct tm with optional subsecond precision.

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), sets the
  individual year/month/day/hour/minute/second fields from the struct tm.
  The ns parameter is ignored for ORIGINAL frame (no subsecond precision).

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), converts
  the struct tm to nanoseconds since Unix epoch and adds the ns parameter
  for subsecond precision. Also sets year=0xffff, month=0xff to indicate
  UNIX_NS frame.

  @param pev Pointer to event that will have date set.
  @param ptm Pointer to struct tm with date/time information to set in event.
  @param ns Optional nanoseconds within the second (0-999999999). Default is 0.
            Values > 999999999 are clamped. Only used for UNIX_NS frame.
  @return True on success, false on failure.
 */
bool
vscp_setEventDateTime(vscpEvent *pev, struct tm *ptm, uint32_t ns = 0);

/*!
  @fn vscp_setEventExDateTime
  Set eventex datetime from struct tm with optional subsecond precision.

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), sets the
  individual year/month/day/hour/minute/second fields from the struct tm.
  The ns parameter is ignored for ORIGINAL frame (no subsecond precision).

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), converts
  the struct tm to nanoseconds since Unix epoch and adds the ns parameter
  for subsecond precision. Also sets year=0xffff, month=0xff to indicate
  UNIX_NS frame.

  @param pex Pointer to eventex that will have date set.
  @param ptm Pointer to struct tm with date/time information to set in eventex.
  @param ns Optional nanoseconds within the second (0-999999999). Default is 0.
            Values > 999999999 are clamped. Only used for UNIX_NS frame.
  @return True on success, false on failure.
 */
bool
vscp_setEventExDateTime(vscpEventEx *pex, struct tm *ptm, uint32_t ns = 0);

/*!
  @fn vscp_setEventToNow
  Set the event date to now.

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), sets the
  individual year/month/day/hour/minute/second fields to the current UTC time.

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), sets
  timestamp_ns to the current time in nanoseconds since Unix epoch (second
  precision only). Also sets year=0xffff, month=0xff to indicate UNIX_NS frame.

  @param pev Pointer to event that will have date set to now
  @return True on success, false on failure.
 */
bool
vscp_setEventToNow(vscpEvent *pev);

/*!
  @fn vscp_setEventExToNow
  Set the eventex date to now.

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), sets the
  individual year/month/day/hour/minute/second fields to the current UTC time.

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), sets
  timestamp_ns to the current time in nanoseconds since Unix epoch (second
  precision only). Also sets year=0xffff, month=0xff to indicate UNIX_NS frame.

  @param pex Pointer to eventex that will have date set to now
  @return True on success, false on failure.
 */
bool
vscp_setEventExToNow(vscpEventEx *pex);

/*!
  @fn vscp_clearVSCPFilter
  Clear VSCP filter so it will allow all events to go through
  @param pfilter Pointer to VSCP filter.
*/
void
vscp_clearVSCPFilter(vscpEventFilter *pfilter);

/*!
  @fn vscp_copyVSCPFilter
  Copy filter from one filter to another
  @param ptofilter Pointer to filter to copy data to
  @param pfromfilter Pinter to filter to copy data from
 */
void
vscp_copyVSCPFilter(vscpEventFilter *ptofilter, const vscpEventFilter *pfromfilter);

/*!
  @fn vscp_doLevel2Filter
  Check filter/mask to check if filter should be delivered

  filter ^ bit    mask    out
  ============================
      0           0       1    filter == bit, mask = don't care result = true
      0           1       1    filter == bit, mask = valid, result = true
      1           0       1    filter != bit, mask = don't care, result = true
      1           1       0    filter != bit, mask = valid, result = false

  Mask tells *which* bits that are of interest means
  it always returns true if bit set to zero (0=don't care).

  Filter tells the value for valid bits. If filter bit is == 1 the bits
  must be equal to get a true filter return.

  So a nill mask will let everything through

  A filter pointer set to NULL will let every event through.

  @param pev Pointer to event to check against filter
  @param pfilter Pointer to filter to check with. If NULL, all events will
  @return true if message should be delivered false if not.
*/
bool
vscp_doLevel2Filter(const vscpEvent *pev, const vscpEventFilter *pfilter);

/*!
  @fn vscp_doLevel2FilterEx
  Check filter/mask to check if filter should be delivered for eventex

  filter ^ bit    mask    out
  ============================
      0           0       1    filter == bit, mask = don't care result = true
      0           1       1    filter == bit, mask = valid, result = true
      1           0       1    filter != bit, mask = don't care, result = true
      1           1       0    filter != bit, mask = valid, result = false

  Mask tells *which* bits that are of interest means
  it always returns true if bit set to zero (0=don't care).

  Filter tells the value for valid bits. If filter bit is == 1 the bits
  must be equal to get a true filter return.

  So a nill mask will let everything through

  A filter pointer set to NULL will let every event through.

  @param pex Pointer to eventex to check against filter
  @param pfilter Pointer to filter to check with. If NULL, all events will
                   be allowed through.
  @return true if message should be delivered false if not.
*/
bool
vscp_doLevel2FilterEx(const vscpEventEx *pex, const vscpEventFilter *pfilter);

/*!
  @fn vscp_readFilterFromString
  Read a filter from a string
  If strFilter is an empty string all elements in filter will be set to
  zero. Arguments is priority,class,type,GUID and all is optional but if
  given must be given in order.
  @param pfilter Filter structure to write filter to.
  @param strFilter Filter in string form
          filter-priority, filter-class, filter-type, filter-GUID
  @return true on success, false on failure.
*/

bool
vscp_readFilterFromString(vscpEventFilter *pfilter, const std::string &strFilter);

/*!
  @fn vscp_writeFilterToString
  @brief Write filter to string
  @param strFilter Filter in string form
          filter-priority, filter-class, filter-type, filter-GUID
  @param pfilter Filter structure to write out to string.
  @return true on success, false on failure.
*/
bool
vscp_writeFilterToString(std::string &strFilter, const vscpEventFilter *pfilter);

/*!
  @fn vscp_readMaskFromString
  Read a mask from a string
  If strMask is an empty string elements in mask will be set to zero.
  Arguments is

  "priority,class,type,GUID"

  and all is optional but if given must be given in order.
  @param pfilter Filter structure to write mask to.
  @param strMask Mask in string form
          mask-priority, mask-class, mask-type, mask-GUID
  @return true on success, false on failure.
*/

bool
vscp_readMaskFromString(vscpEventFilter *pfilter, const std::string &strMask);

/*!
  @fn vscp_writeMaskToString
  @brief Write mask to string
  @param strMask Mask in string form
          mask-priority, mask-class, mask-type, mask-GUID
  @param pfilter Filter structure to write out to string.
  @return true on success, false on failure.
*/
bool
vscp_writeMaskToString(std::string &strMask, const vscpEventFilter *pfilter);

/*!
  @fn vscp_readFilterMaskFromString
  Read both filter and mask from string

  @param pfilter Pointer to VSCP filter structure which will receive filter
                    mask data.
  @param strFilter Filter and mask in comma separated list
  @return true on success, false on failure.
 */
bool
vscp_readFilterMaskFromString(vscpEventFilter *pfilter, const std::string &strFilterMask);

/*!
    @fn vscp_writeFilterMaskToString
    Write both filter and mask to string

    @param strFilter Reference to string that will get filter and mask in comma separated list
    @param pfilter Pointer to VSCP filter structure which contains filter
                      mask data.
    @return true on success, false on failure.
 */
bool
vscp_writeFilterMaskToString(std::string &strFilter, const vscpEventFilter *pfilter);

/*!
  @fn vscp_readFilterMaskFromXML
  Read filter and mask from XML coded string

 <filter
       mask_priority="number"
       mask_class="number"
       mask_type="number"
       mask_guid="GUID string"
       filter_priority="number"
       filter_class="number"
       filter_type="number"
       filter_guid="GUID string"
  />

  @param pfilter Pointer to filter structure that will get result.
  @param strFilter Filter on XML format.
  @return True on success, false on failure.
 */
bool
vscp_readFilterMaskFromXML(vscpEventFilter *pfilter, const std::string &strFilter);

/*!
  @fn vscp_writeFilterMaskToXML
  Write filter to XML coded string

  @param strFilter String that will get XML coded filter
  @param pfilter Pointer to filter structure
  @return True on success, false on failure.
 */
bool
vscp_writeFilterMaskToXML(std::string &strFilter, vscpEventFilter *pfilter);

/*!
  @fn vscp_readFilterMaskFromJSON
  Read filter from JSON coded string

  {
    'mask_priority': number,
    'mask_class': number,
    'mask_type': number,
    'mask_guid': 'string',
    'filter_priority'; number,
    'filter_class': number,
    'filter_type': number,
    'filter_guid' 'string'
  }

  @param pfilter Pointer to filter structure that will get result.
  @param strFilter Filter on JSON format.
  @return True on success, false on failure.
 */
bool
vscp_readFilterMaskFromJSON(vscpEventFilter *pfilter, const std::string &strFilter);

/*!
  @fn vscp_writeFilterMaskToJSON
  Write filter to JSON coded string

  @param strFilter String that will get JSON coded filter
  @param pfilter Pointer to VSCP filter structure
  @return True on success, false on failure.
 */
bool
vscp_writeFilterMaskToJSON(std::string &strFilter, vscpEventFilter *pfilter);

/*!
  @fn vscp_convertCanalToEvent
  Convert a CANAL message to an event
  @param pev Pointer to VSCP event that get result
  @param pcanalMsg Pointer to CANAL message that should be converted
  @param pguid GUID to use for event
  @return true on success
*/
bool
vscp_convertCanalToEvent(vscpEvent *pev, const canalMsg *pcanalMsg, unsigned char *pguid);

/*!
  @fn vscp_convertCanalToEventEx
  Convert a CANAL message to an event ex
  @param pex Pointer to VSCP event ex that get result
  @param pcanalMsg Pointer to CANAL message that should be converted
  @param pguid GUID to use for event
  @return true on success
*/
bool
vscp_convertCanalToEventEx(vscpEventEx *pex, const canalMsg *pcanalMsg, unsigned char *pguid = nullptr);

/*!
  @fn vscp_convertEventToCanal
  Convert VSCP event to a CANAL message
  @param pev Pointer to VSCP event that get result
  @param pcanalMsg Pointer to CANAL message that should be converted
  @param mode Size of CAN frame structure. Used to detect fd-mode
  @return true on success
*/
bool
vscp_convertEventToCanal(canalMsg *pcanalMsg, const vscpEvent *pev, uint8_t mode = CAN_MTU);

/*!
  @fn vscp_convertEventExToCanal
  Covert VSCP event to CANAL message

  Convert VSCP event to a CANAL message
  @param pcanalMsg Pointer to CANAL message that get result
  @param pex Pointer to VSCP event ex that should be converted
  @param mode Size of CAN frame structure. Used to detect fd-mode
  @return true on success
*/
bool
vscp_convertEventExToCanal(canalMsg *pcanalMsg, const vscpEventEx *pex, uint8_t mode = CAN_MTU);

/*!
  @fn vscp_copyEvent
  Copy one VSCP event to another

  @param pevTo Pointer to event to copy to.
  @param pevFrom Pointer to event to copy from.
  @return true on success.
*/
bool
vscp_copyEvent(vscpEvent *pevTo, const vscpEvent *pevFrom);

/*!
  @fn vscp_copyEventEx
  Copy one VSCP event ex to another

  @param pexTo Pointer to event ex to copy to.
  @param pexFrom Pointer to event ex to copy from.
  @return True on success.
*/
bool
vscp_copyEventEx(vscpEventEx *pexTo, const vscpEventEx *pexFrom);

/*!
  @fn vscp_writeDataToString
  Write VSCP data to string
  DEPRECATED USE: vscp_writeDataWithSizeToString

  @param str String that receive result.
  @param pev Pointer to event where data is fetched from
  @param bUseHtmlBreak Set to true to use <br> instead of \\n as
          line break
  @param bBreak String gets formatted in groups of eight bytes with
          a break between the lines.
  @return True on success false on failure.
*/

bool
vscp_writeDataToString(std::string &str, const vscpEvent *pev, bool bUseHtmlBreak = false, bool bBreak = false);

/*!
  @fn vscp_writeDataWithSizeToString
  Write VSCP data to string

  @param str String that receive result.
  @param pData Pointer to data structure.
  @param sizeData Number of data bytes.
  @param bUseHtmlBreak Set to true to use <br> instead of \\n as
          line break
  @param bDecimal True if string should have decimal numbers instead of
          hexadecimal numbers.
  @return True on success false on failure.
*/

bool
vscp_writeDataWithSizeToString(std::string &str,
                               const unsigned char *pData,
                               const uint16_t sizeData,
                               bool bUseHtmlBreak = false,
                               bool bBreak        = true,
                               bool bDecimal      = false);

/*!
  @fn vscp_setEventDataFromString
  Set VSCP Event data from a string
  @param pev Pointer to a VSCP event to write parsed data to.
  @param str A string with comma or whitespace separated data in decimal
  or hexadecimal form. Data can span multiple lines.
  @return true on success, false on failure.
*/

bool
vscp_setEventDataFromString(vscpEvent *pev, const std::string &str);

/*!
  @fn vscp_setEventExDataFromString
  Set VSCP EventEx data from a string
  @param pex Pointer to a VSCP event ex to write parsed data to.
  @param str A string with comma or whitespace separated data in decimal
  or hexadecimal form. Data can span multiple lines.
  @return true on success, false on failure.
*/

bool
vscp_setEventExDataFromString(vscpEventEx *pex, const std::string &str);

/*!
  @fn vscp_setDataArrayFromString
  Set VSCP data from a string
  @param pData Pointer to a unsigned byte array to write parsed data to.
  @param psizeData Number of data bytes.
  @param str A string with comma or whitespace separated data in decimal
  or hexadecimal form. Data can span multiple lines.
  @return true on success, false on failure.
*/

bool
vscp_setDataArrayFromString(uint8_t *pData, uint16_t *psizeData, const std::string &str);

/*!
  @fn vscp_convertEventToString
  Write event to string.
  Format: head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), uses
  the 32-bit timestamp field.

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), uses
  the 64-bit timestamp_ns field.

  @param str String that receive the result
  @param pev Event that should be presented
  @return true on success, false on failure.
*/

bool
vscp_convertEventToString(std::string &str, const vscpEvent *pev);

/*!
  @fn vscp_getEventAsString
  Get Event as string
  @param pev Pointer to event
  @return String representation of event.
*/
std::string
vscp_getEventAsString(const vscpEvent *pev);

/*!
  @fn vscp_convertEventExToString
  Write eventex to string.
  Format: head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....

  For ORIGINAL frame version (VSCP_HEADER16_FRAME_VERSION_ORIGINAL), uses
  the 32-bit timestamp field.

  For UNIX_NS frame version (VSCP_HEADER16_FRAME_VERSION_UNIX_NS), uses
  the 64-bit timestamp_ns field.

  @param str String that receive the result
  @param pex Event that should be presented
  @return true on success, false on failure.
*/

bool
vscp_convertEventExToString(std::string &str, const vscpEventEx *pex);

/*!
  @fn vscp_getEventExAsString
  Get EventEx as string
  @param pex Pointer to event ex
  @return String representation of event.
*/
std::string
vscp_getEventExAsString(const vscpEventEx *pex);

/*!
  @fn vscp_convertStringToEvent
  Get event data from string format.
  Always converts to UNIX_NS frame format with nanosecond precision.

  Format: head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....

  If datetime is empty, the timestamp field is treated as nanoseconds since
  Unix epoch. If datetime is present, it is parsed and converted to nanoseconds.
  The head field will have VSCP_HEADER16_FRAME_VERSION_UNIX_NS set.

  @param pev Event that will get data
  @param str String that contains the event in string form
  @return true on success, false on failure.
*/

bool
vscp_convertStringToEvent(vscpEvent *pev, const std::string &str);

/*!
  @fn vscp_convertStringToEventEx
  Get event data from string format.
  Always converts to UNIX_NS frame format with nanosecond precision.

  Format: head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....

  If datetime is empty, the timestamp field is treated as nanoseconds since
  Unix epoch. If datetime is present, it is parsed and converted to nanoseconds.
  The head field will have VSCP_HEADER16_FRAME_VERSION_UNIX_NS set.

  @param pex Pointer to VSCP event that will get the parsed data
  @param str String that contain the event on string form
  @return true on success, false on failure.
*/

bool
vscp_convertStringToEventEx(vscpEventEx *pex, const std::string &str);

/*!
  @fn vscp_makeHtml
  This function makes a HTML string from a standard string. LF is replaced
  with a '<BR>'.
  @param str String that should be HTML coded.
*/

void
vscp_makeHtml(std::string &str);

////////////////////////////////////////////////////////////////////////////
//                           Encrypted frames
////////////////////////////////////////////////////////////////////////////

/*!
  @fn vscp_getEncryptionCodeFromToken
  Get encryption code from token.

  @param token Encryption token to set encryption from. Tokens are
               define in vscp,h
  @return Return the encryption code if the encryption code is known.
          The function does not return an error code and instead set no
          encryption if the token is invalid.
 */

uint8_t
vscp_getEncryptionCodeFromToken(std::string &token);

/*!
  @fn vscp_getEncryptionTokenFromCode
  Fetch encryption string token from code.

  @param code Should be a valid encryption code as defined in vscp.h
  @param token A encryption token is returned if the code is valid. For a
           a code == 0 which means no encryption an empty string is
           returned. This is also true for invalid codes.
 */

void
vscp_getEncryptionTokenFromCode(uint8_t code, std::string &token);

/*!
  @fn vscp_getFrameSizeFromEvent
  Get UDP frame size from event

  @param pev Pointer to event.
  @return Size of resulting UDP frame on success. Zero on failure.
 */
size_t
vscp_getFrameSizeFromEvent(vscpEvent *pev);

/*!
  @fn vscp_getFrameSizeFromEventEx
  Get UDP frame size from event ex
  @param pex Pointer to event ex.
  @return Size of resulting UDP frame on success. Zero on failure.
 */
size_t
vscp_getFrameSizeFromEventEx(vscpEventEx *pex);

/*!
  @fn vscp_writeEventToFrame
  Write event on binary frame format.

  Uses packet format 0 (35-byte header with microsecond timestamp and datetime fields)
  for original frame version, and packet format 1 (35-byte header with 8-byte nanosecond
  timestamp) for UNIX_NS frame version. The frame format is determined by the event's
  head field (VSCP_HEADER16_FRAME_VERSION_MASK).

  @param buf A pointer to a buffer that will receive the event.
  @param len Size of the buffer.
  @param pkttype Is the first byte of binary type frames that holds
           type of packet and encryption (lower nibble for encryption).
  @param pev Pointer to event that should be written to the frame.
  @return True on success, false on failure.
 */
bool
vscp_writeEventToFrame(uint8_t *frame, size_t len, uint8_t pkttype, const vscpEvent *pev);

/*!
  @fn vscp_writeEventExToFrame
  Write event ex to binary frame format.

  Uses packet format 0 (35-byte header with microsecond timestamp and datetime fields)
  for original frame version, and packet format 1 (35-byte header with 8-byte nanosecond
  timestamp) for UNIX_NS frame version. The frame format is determined by the event's
  head field (VSCP_HEADER16_FRAME_VERSION_MASK).

  @param buf A pointer to a buffer that will receive the event.
  @param len Size of the buffer.
  @param pkttype Is the first byte of binary type frames that holds
           type of packet and encryption (lower nibble for encryption).
  @param pex Pointer to event that should be written to the frame.
  @return True on success, false on failure.
 */
bool
vscp_writeEventExToFrame(uint8_t *frame, size_t len, uint8_t pkttype, const vscpEventEx *pex);

/*!
  @fn vscp_getEventFromFrame
  Get VSCP event from binary frame.

  Handles both packet format 0 (original with microsecond timestamp and datetime)
  and packet format 1 (UNIX_NS with 8-byte nanosecond timestamp). The format is
  determined from the packet type byte (upper nibble).

  @param pev Pointer to VSCP event that will get data from the frame,
  @param buf A pointer to a buffer that will receive the event.
  @param len Size of the buffer.
  @return True on success, false on failure.
 */
bool
vscp_getEventFromFrame(vscpEvent *pev, const uint8_t *buf, size_t len);

/*!
  @fn vscp_getEventExFromFrame
  Get VSCP event ex from binary frame.

  Handles both packet format 0 (original with microsecond timestamp and datetime)
  and packet format 1 (UNIX_NS with 8-byte nanosecond timestamp). The format is
  determined from the packet type byte (upper nibble).

  @param pex Pointer to VSCP event ex that will get data from the
  frame,
  @param buf A pointer to a buffer that will receive the event.
  @param len Size of the buffer.
  @return True on success, false on failure.
 */
bool
vscp_getEventExFromFrame(vscpEventEx *pex, const uint8_t *buf, size_t len);

/*!
  @fn vscp_writeCommandToFrame
  Write a command to a binary frame.

  The command is written in the same format as the command part of a reply frame,
  but without the error code and with the command code in place of the error code.
  This allows the command to be used in both command frames and reply frames.

  @param frame Buffer that will receive the command frame. Should be at least
               3 bytes long to hold the command code and at least one byte
               of argument.
  @param len Size of the buffer.
  @param command Command code to write to frame.
  @param arg Optional argument data to include in the frame. Can be NULL if no
             argument data is needed.
  @param arg_len Length of argument data. Should be zero if arg is NULL.
  @return True on success, false on failure.
 */
int
vscp_writeCommandToFrame(uint8_t *frame, size_t len, uint16_t command, const uint8_t *arg, size_t arg_len);

/*!
  @fn vscp_writeReplyToFrame
  Write a reply to a binary frame.

  The reply is written in the same format as the command part of a reply frame,
  but with the command code in place of the error code. This allows the reply
  to be used in both command frames and reply frames.

  @param frame Buffer that will receive the reply frame. Should be at least
               3 bytes long to hold the command code, error code, and at least
               one byte of argument.
  @param len Size of the buffer.
  @param command Command code to write to frame.
  @param error Error code to write to frame.
  @param arg Optional argument data to include in the frame. Can be NULL if no
             argument data is needed.
  @param arg_len Length of argument data. Should be zero if arg is NULL.
  @return True on success, false on failure.
*/
int
vscp_writeReplyToFrame(uint8_t *frame,
                             size_t len,
                             uint16_t command,
                             uint16_t error,
                             const uint8_t *arg,
                             size_t arg_len);

/*!
  @fn vscp_getBootLoaderDescription
  Get a pointer to a bootloader description string from boot loader code
  @param code Boot loader code to get description for
  @return Pointer to string with description. "Unknown" will be returned for
    a code that is not known.
*/
const char *
vscp_getBootLoaderDescription(uint8_t code);

/*!
  @fn vscp_encryptFrame
  Encrypt VSCP frame using the selected encryption algorithm. The iv
  (initialization vector) is appended to the end of the encrypted data.

  @param output Buffer that will receive the encrypted result. The buffer
           should be at least 16 bytes larger than the frame.
  @param input This is the frame that should be encrypted. The first
           byte in the frame is the packet type which is not encrypted.
  @param len This is the length of the frame to be encrypted. This
           length includes the packet tye in the first byte.
  @param key This is a pointer to the secret encryption key. This key
           should be 128 bits for AES128, 192 bits for AES192, 256 bits
           for AES256.
  @param iv Pointer to the initialization vector. Should always point to a
             128 bit content. If NULL the iv will be created from random
             system data. In both cases the end result will have the iv
             appended to the encrypted block.
  @param nAlgorithm The VSCP defined algorithm (0-15) to encrypt the frame
  with. If set to 15 (VSCP_ENCRYPTION_FROM_TYPE_BYTE) the algorithm will be
  set from the four lower bits of the buffer to decrypt.
  @return Packet length on success, zero on failure.
 */
size_t
vscp_encryptFrame(uint8_t *output,
                  uint8_t *input,
                  size_t len,
                  const uint8_t *key,
                  const uint8_t *iv,
                  uint8_t nAlgorithm);

/*!
  @fn vscp_decryptFrame
  Decrypt VSCP frame using the selected encryption algorithm. The iv
  (initialization vector) is appended to the end of the encrypted data.

  @param output Buffer that will receive the decrypted result. The buffer
           should have a size of at lest equal to the encrypted block.
  @param input This is the frame that should be decrypted.
  @param len This is the length of the frame to be decrypted.
  @param key This is a pointer to the secret encryption key. This key
             should be 128 bits for AES128, 192 bits for AES192,
             256 bits for AES256.
  @param iv Pointer to the initialization vector. Should always point to a
             128 bit content. If NULL the iv is expected to be the last
             16 bytes of the encrypted data.
  @param nAlgorithm The VSCP defined algorithm (0-15) to decrypt the frame
          with. (vscp.h) If set to 15 (VSCP_ENCRYPTION_FROM_TYPE_BYTE) the algorithm
          will be set from the four lower bits of the buffer to decrypt.
  @return True on success, false on failure.

  NOTE: Note that VSCP packet type (first byte in UDP frame) is not
  recognised here.
 */
bool
vscp_decryptFrame(uint8_t *output,
                  uint8_t *input,
                  size_t len,
                  const uint8_t *key,
                  const uint8_t *iv,
                  uint8_t nAlgorithm);

/*!
  @fn vscp_setFrameEncryptionUseOpenSSL
  Select AES backend for vscp_encryptFrame/vscp_decryptFrame.

  Default is false which uses the internal AES implementation.

  @param bEnable Set true to use OpenSSL EVP AES-CBC, false for internal AES.
*/
void
vscp_setFrameEncryptionUseOpenSSL(bool bEnable);

/*!
  @fn vscp_getFrameEncryptionUseOpenSSL
  Get current AES backend selection for vscp_encryptFrame/vscp_decryptFrame.

  @return True if OpenSSL backend is selected, false if internal AES is selected.
*/
bool
vscp_getFrameEncryptionUseOpenSSL(void);

///////////////////////////////////////////////////////////////////////////
//                         Password/key handling
///////////////////////////////////////////////////////////////////////////

/*!
  @fn vscp_md5
  Calculate md5 hex digest for buf

  @param digest Buffer (33 bytes) that will receive the digest in hex
           format.
  @param buf Data to calculate md5 on.
  @param len Len of input data.
 */
void
vscp_md5(char *digest, const unsigned char *buf, size_t len);

/*!
  @fn vscp_byteArray2HexStr
  Stringify binary data.

  @param to Pointer output buffer that holds the result.
    Output buffer must be twice as big as input,
    because each byte takes 2 bytes in string representation
  @param p Pointer to digest.
  @param len Digest len
*/
void
vscp_byteArray2HexStr(char *to, const unsigned char *p, size_t len);

/*!
  @fn vscp_hexStr2ByteArray
  Convert a hex string to a byte array.

  @param array Byte array that will receive result.
  @param size Size of byte array.
  @param hexstr Hex string that should be converted.
  @return Number of bytes written to the byte array or
          zero on error.
 */

size_t
vscp_hexStr2ByteArray(uint8_t *array, size_t size, const char *hexstr);

/*!
  @fn vscp_getHashPasswordComponents
  Get components form a hashed password

  VSCP passwords is stored as two hex strings separated with a ";"-
  The first string is the salt, the second the hashed password.

  @param pSalt Pointer to a 16 byte buffer that will receive the salt.
  @param pHash Pointer to a 32 byte buffer that will receive the hash.
  @param stored_pw Stored password on the form salt;hash
  @return True on success, false on failure.
 */
bool
vscp_getHashPasswordComponents(uint8_t *pSalt, uint8_t *pHash, const std::string &stored_pw);

/*!
  @fn vscp_getSalt
  Get salt

  @param buf Pointer to byte array to write salt to.
  @param len Number of bytes to generate.
  @return True on success.
 */
bool
vscp_getSalt(uint8_t *buf, size_t len);

/*!
  @fn vscp_getSaltHex
  Get salt as hex string

  @param strSalt String that will get the salt in hex format.
  @param len Number of bytes to generate.
  @return True on success.
 */
bool
vscp_getSaltHex(std::string &strSalt, size_t len);

/*
#ifdef __cplusplus
}
#endif
*/

#endif // #if !defined(VSCPHELPER_H__INCLUDED_)
