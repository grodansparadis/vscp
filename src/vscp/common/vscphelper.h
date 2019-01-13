// FILE: vscphelper.h
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
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
   working with VSCP. \details vscphealper have common used functionality to do
   things in the VSCP world. It can be seen as the main toolbox for the VSCP
   programmer.
 */

#if !defined(VSCPHELPER_H__INCLUDED_)
#define VSCPHELPER_H__INCLUDED_

#include <config.h> // autoconf

#ifndef WIN32
#include <sys/times.h>
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
#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>

/*  byte swapping */

#define VSCP_UINT16_SWAP_ALWAYS(val)                                           \
    ((uint16_t)((((uint16_t)(val) & (uint16_t)0x00ffU) << 8) |                 \
                (((uint16_t)(val) & (uint16_t)0xff00U) >> 8)))

#define VSCP_INT16_SWAP_ALWAYS(val)                                            \
    ((int16_t)((((uint16_t)(val) & (uint16_t)0x00ffU) << 8) |                  \
               (((uint16_t)(val) & (uint16_t)0xff00U) >> 8)))

#define VSCP_UINT32_SWAP_ALWAYS(val)                                           \
    ((uint32_t)((((uint32_t)(val) & (uint32_t)0x000000ffU) << 24) |            \
                (((uint32_t)(val) & (uint32_t)0x0000ff00U) << 8) |             \
                (((uint32_t)(val) & (uint32_t)0x00ff0000U) >> 8) |             \
                (((uint32_t)(val) & (uint32_t)0xff000000U) >> 24)))

#define VSCP_INT32_SWAP_ALWAYS(val)                                            \
    ((int32_t)((((uint32_t)(val) & (uint32_t)0x000000ffU) << 24) |             \
               (((uint32_t)(val) & (uint32_t)0x0000ff00U) << 8) |              \
               (((uint32_t)(val) & (uint32_t)0x00ff0000U) >> 8) |              \
               (((uint32_t)(val) & (uint32_t)0xff000000U) >> 24)))
/*  machine specific byte swapping */

#define VSCP_UINT64_SWAP_ALWAYS(val)                                           \
    ((uint64_t)((((uint64_t)(val) & (uint64_t)(0x00000000000000ff)) << 56) |   \
                (((uint64_t)(val) & (uint64_t)(0x000000000000ff00)) << 40) |   \
                (((uint64_t)(val) & (uint64_t)(0x0000000000ff0000)) << 24) |   \
                (((uint64_t)(val) & (uint64_t)(0x00000000ff000000)) << 8) |    \
                (((uint64_t)(val) & (uint64_t)(0x000000ff00000000)) >> 8) |    \
                (((uint64_t)(val) & (uint64_t)(0x0000ff0000000000)) >> 24) |   \
                (((uint64_t)(val) & (uint64_t)(0x00ff000000000000)) >> 40) |   \
                (((uint64_t)(val) & (uint64_t)(0xff00000000000000)) >> 56)))

#define VSCP_INT64_SWAP_ALWAYS(val)                                            \
    ((int64_t)((((uint64_t)(val) & (uint64_t)(0x00000000000000ff)) << 56) |    \
               (((uint64_t)(val) & (uint64_t)(0x000000000000ff00)) << 40) |    \
               (((uint64_t)(val) & (uint64_t)(0x0000000000ff0000)) << 24) |    \
               (((uint64_t)(val) & (uint64_t)(0x00000000ff000000)) << 8) |     \
               (((uint64_t)(val) & (uint64_t)(0x000000ff00000000)) >> 8) |     \
               (((uint64_t)(val) & (uint64_t)(0x0000ff0000000000)) >> 24) |    \
               (((uint64_t)(val) & (uint64_t)(0x00ff000000000000)) >> 40) |    \
               (((uint64_t)(val) & (uint64_t)(0xff00000000000000)) >> 56)))

#ifdef WORDS_BIGENDIAN
#define VSCP_UINT16_SWAP_ON_BE(val) VSCP_UINT16_SWAP_ALWAYS(val)
#define VSCP_INT16_SWAP_ON_BE(val) VSCP_INT16_SWAP_ALWAYS(val)
#define VSCP_UINT16_SWAP_ON_LE(val) (val)
#define VSCP_INT16_SWAP_ON_LE(val) (val)
#define VSCP_UINT32_SWAP_ON_BE(val) VSCP_UINT32_SWAP_ALWAYS(val)
#define VSCP_INT32_SWAP_ON_BE(val) VSCP_INT32_SWAP_ALWAYS(val)
#define VSCP_UINT32_SWAP_ON_LE(val) (val)
#define VSCP_INT32_SWAP_ON_LE(val) (val)
#define VSCP_UINT64_SWAP_ON_BE(val) VSCP_UINT64_SWAP_ALWAYS(val)
#define VSCP_UINT64_SWAP_ON_LE(val) (val)
#define VSCP_INT64_SWAP_ON_BE(val) VSCP_INT64_SWAP_ALWAYS(val)
#define VSCP_INT64_SWAP_ON_LE(val) (val)
#else
#define VSCP_UINT16_SWAP_ON_LE(val) VSCP_UINT16_SWAP_ALWAYS(val)
#define VSCP_INT16_SWAP_ON_LE(val) VSCP_INT16_SWAP_ALWAYS(val)
#define VSCP_UINT16_SWAP_ON_BE(val) (val)
#define VSCP_INT16_SWAP_ON_BE(val) (val)
#define VSCP_UINT32_SWAP_ON_LE(val) VSCP_UINT32_SWAP_ALWAYS(val)
#define VSCP_INT32_SWAP_ON_LE(val) VSCP_INT32_SWAP_ALWAYS(val)
#define VSCP_UINT32_SWAP_ON_BE(val) (val)
#define VSCP_INT32_SWAP_ON_BE(val) (val)
#define VSCP_UINT64_SWAP_ON_LE(val) VSCP_UINT64_SWAP_ALWAYS(val)
#define VSCP_UINT64_SWAP_ON_BE(val) (val)
#define VSCP_INT64_SWAP_ON_LE(val) VSCP_INT64_SWAP_ALWAYS(val)
#define VSCP_INT64_SWAP_ON_BE(val) (val)
#endif

// Forward declaration
class CMDF;

#ifdef __cplusplus
extern "C"
{
#endif

    /*!
        \union vscp_value
        \brief VSCP Data coding declaration
    */
    union vscp_value
    {

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
        Return non zero if endiness is little endian
    */
    int vscp_isLittleEndian(void);

    /*!
        Return non zero if endiness is big endian
    */
    int vscp_isBigEndian(void);

    /*!
        Wait a number of milliseconds for semaphore

        @param sem Pointer to semaphore to wait for
        @param waitms Time in milliseconds to wait. Max 3999 ms.
        return 0 is semaphore is signaled, -1 if error. errno
        is set accordingly. ETIMEDOUT is returned for timeout.
    */
    int vscp_sem_wait(sem_t *sem, uint32_t waitms);

    /*
     * Check two floats for equality
     * @param A Float to compare
     * @param B Float to compare
     * @param maxRelDiff Maxdiff (see article)
     * @return true if same, false if different (sort of)
     *
     * https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
     */

    bool vscp_almostEqualRelativeFloat(float A,
                                       float B,
                                       float maxRelDiff = FLT_EPSILON);

    /*!
     * https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
     */
    bool vscp_almostEqualUlpsAndAbsFloat(float A,
                                         float B,
                                         float maxDiff,
                                         int maxUlpsDiff);
    /*!
     * https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
     */
    bool vscp_almostEqualRelativeAndAbsFloat(float A,
                                             float B,
                                             float maxDiff,
                                             float maxRelDiff = FLT_EPSILON);

    /*
     * Check two floats for equality
     * @param A Double to compare
     * @param B Double to compare
     * @param maxRelDiff Maxdiff (see article)
     * @return true if same, false if different (sort of)
     *
     * https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
     */

    bool vscp_almostEqualRelativeDouble(double A,
                                        double B,
                                        double maxRelDiff = DBL_EPSILON);

    /*!
        Read a numerical value from a string
        The string value can be set as a hex or a decimal value.
        @param strval String containing value to be converted
        @return Unsigned long containing value
        */

    int32_t vscp_readStringValue(const std::string &strval);

    /*!
        Convert string to lowercase
    */
    int vscp_lowercase(const char *s);

    /*!
        String non case compare
        @param s1 String1 to compare
        @param s2 String2 to compare
        @return 0 if strings are the same
    */
    int vscp_strcasecmp(const char *s1, const char *s2);

    /*!
        String non case compare with length
        @param s1 String1 to compare
        @param s2 String2 to compare
        @param len Number of byte to compare
        @return 0 if strings are the same
    */
    int vscp_strncasecmp(const char *s1, const char *s2, size_t len);

    void vscp_strlcpy(char *dst, const char *src, size_t n);

    char *vscp_strdup(const char *str);

    char *vscp_strndup(const char *ptr, size_t len);

    const char *vscp_strcasestr(const char *big_str, const char *small_str);

    char *vscp_stristr(char *str1, const char *str2);

    char *vscp_trimWhiteSpace(char *str);

    char *vscp_reverse(const char *const s);

    char *vscp_rstrstr(const char *s1, const char *s2);

    /*!
        vscp_str_format

        https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf/49812018
        @param format string
        @param Variables part of resulting string
        @return formated string
    */
    std::string vscp_str_format(const std::string &fmt_str, ...);

    /*!
        vscp_startsWith

        Check if a string have a start token and optionally return
        the rest of the string if it has. If the token is not available and
        rest is non-null the original string will be returned in rest.
        @param origstr String to check.
        @param searchstr Start token to look for.
        @param rest If not set to null the rest string is returned here.
       Original string is returned if token string is not found.
        @return True if start token is found.
    */
    bool vscp_startsWith(const std::string &origstr,
                         const std::string &searchstr,
                         std::string *rest = NULL);

    // String to upper case (in place)
    static inline void vscp_makeUpper(std::string &s)
    {
        std::transform(
          s.begin(), s.end(), s.begin(), [](unsigned char c) -> unsigned char {
              return std::toupper(c);
          });
    }

    // String to upper case (copying)
    static inline std::string vscp_makeUpper_copy(std::string s)
    {
        vscp_makeUpper(s);
        return s;
    }

    // String to upper case
    static inline std::string vscp_upper(const std::string &s)
    {
        std::string newstr = s;
        vscp_makeUpper(newstr);
        return newstr;
    }

    // String to lower case (in place)
    static inline void vscp_makeLower(std::string &s)
    {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    }

    // String to upper case (copying)
    static inline std::string vscp_makeLower_copy(std::string s)
    {
        vscp_makeLower(s);
        return s;
    }

    // String to lower case
    static inline std::string vscp_lower(const std::string &s)
    {
        std::string newstr = s;
        vscp_makeLower(newstr);
        return newstr;
    }

    // trim from start (in place)
    static inline void vscp_ltrim(std::string &s)
    {
        s.erase(s.begin(),
                std::find_if(s.begin(),
                             s.end(),
                             std::not1(std::ptr_fun<int, int>(std::isspace))));
    }

    // trim from end (in place)
    static inline void vscp_rtrim(std::string &s)
    {
        s.erase(std::find_if(s.rbegin(),
                             s.rend(),
                             std::not1(std::ptr_fun<int, int>(std::isspace)))
                  .base(),
                s.end());
    }

    // trim from both ends (in place)
    static inline void vscp_trim(std::string &s)
    {
        vscp_ltrim(s);
        vscp_rtrim(s);
    }

    // trim from start (copying)
    static inline std::string vscp_ltrim_copy(std::string s)
    {
        vscp_ltrim(s);
        return s;
    }

    // trim from end (copying)
    static inline std::string vscp_rtrim_copy(std::string s)
    {
        vscp_rtrim(s);
        return s;
    }

    // trim from both ends (copying)
    static inline std::string vscp_trim_copy(std::string s)
    {
        vscp_trim(s);
        return s;
    }

    // split for tokenizer
    // https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c
    static inline void vscp_split(std::deque<std::string> &theStringVector,
                                  const std::string &theString,
                                  const std::string &theDelimiter)
    {
        size_t start = 0, end = 0;

        while (end != std::string::npos) {
            end = theString.find(theDelimiter, start);

            // If at end, use length=maxLength.  Else use length=end-start.
            theStringVector.push_back(theString.substr(
              start,
              (end == std::string::npos) ? std::string::npos : end - start));

            // If at end, use start=maxSize.  Else use start=end+delimiter.
            start = ((end > (std::string::npos - theDelimiter.size()))
                       ? std::string::npos
                       : end + theDelimiter.size());
        }
    }

    /*!
        Return left part of std string
    */
    static inline std::string vscp_str_left(const std::string &str,
                                            size_t length)
    {
        if ( 0 == length ) return std::string("");
        return str.substr(0, length);
    }

    /*!
        Return right part of std string
    */
    static inline std::string vscp_str_right(const std::string &str,
                                             size_t length)
    {
        if ( 0 == length ) return std::string("");
        if (length > str.length()) length = str.length();
        return str.substr(str.length() - length);
    }

    /*!

    */
    static inline std::string vscp_str_before(const std::string &str, char c)
    {
        size_t pos = str.find(c);
        if (str.npos == pos) return std::string();

        return vscp_str_left(str, pos);
    }

    /*!

    */
    static inline std::string vscp_str_after(const std::string &str, char c)
    {
        size_t pos = str.find(c);
        if (str.npos == pos) return std::string();

        return vscp_str_right(str, pos);
    }

    /*!

    */
    static inline bool vscp_isNumber(const std::string &strNumber)
    {
        std::string str = strNumber;
        vscp_trim(str);
        vscp_makeLower(str);
        if (isdigit(str[0]) || vscp_startsWith(str, "0x")) {
            return true;
        }

        return false;
    }

    /*!
        Check if a file exists

        @param path Path to file to check
        @return True if file exists, false otherwise.
    */
    bool vscp_fileExists(const std::string &path);

    /*!
        Checks to see if a directory exists. Note: This method only checks the
        existence of the full path AND if path leaf is a dir.

        @param path path to directory.
        @return  >0 if dir exists AND is a dir,
                  0 if dir does not exist OR exists but not a dir,
                 <0 if an error occurred (errno is also set)
    */
    int vscp_dirExists(const char *const path);

    /*!
     * BASE64 decode std string
     *
     * @param str str to encode.
     * @return true on success, false on failure.
     *
     */

    bool vscp_base64_std_decode(std::string &str);

    /*!
     * BASE64 encode std string
     *
     * @param str str to encode.
     * @return true on success, false on failure.
     *
     */

    bool vscp_base64_std_encode(std::string &str);

    /*!
     * Decode string from BASE64 is it have "BASE64:" in the front if not just
     * leave the string as it is.
     *
     * @param str String to decode
     * @param strResult String result
     * @return True if all is OK (decoded or not) and false if decode error.
     */

    bool vscp_std_decodeBase64IfNeeded(const std::string &str,
                                       std::string &strResult);

    /*!
        Get GMT time
        http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3
    */
    bool vscp_getTimeString(char *buf, size_t buf_len, time_t *t);

    /*!
        Get ISO GMT datetime
    */
    bool vscp_getISOTimeString(char *buf, size_t buf_len, time_t *t);

    /*!
        Parse ISO combined string (YYYY-MM-DDTHH:MM:SS)

        @param dt Datestring to parse.
        @param ptm Pointer to tm structure that will receive result.
        @return True on success.
    */
    bool vscp_parseISOCombined(std::string &dt, struct tm *ptm);

    /*!
     *  Escape XML string
     *
     *  @param Buffer holding input string. Buffer size must be large enough to
     *          hold expanded result.
     */
    bool vscp_XML_Escape(const char *src, char *dst, size_t dst_len);

    /*!
     * Parse IPv4 address and return net part and mask part
     *
     * @param addr ipv4 address to parse (a.b.c.d/m)
     * @param net Network part of address
     * @param mask Mask part of address
     * return 0 on error,
     */
    int vscp_parse_ipv4_addr(const char *addr, uint32_t *net, uint32_t *mask);

    // ***************************************************************************
    //                             Measurement Helpers
    // ***************************************************************************

    /*!
        Fetch data coding byte from measurement events
        @param pEvent Pointer to VSCP event
        @return Measurement data coding byte or zero if its not an
        event with a data coding.
    */
    uint8_t vscp_getMeasurementDataCoding(const vscpEvent *pEvent);

    /*!
        Get bit array from coded event data
        @param pCode Pointer to data coding byte.
        @param length Number of bytes it consist of including
        the first normalise byte.
        @return Bit array as a unsigned 64-bit integer.
    */
    uint64_t vscp_getDataCodingBitArray(const uint8_t *pCode, uint8_t length);

    /*!
        Get integer from coded event data
        @param pCode Pointer to normalised integer.
        @param length Number of bytes it consist of including
        the first normalise byte.
        @return returns value as a 64-bit integer.
        */
    int64_t vscp_getDataCodingInteger(const uint8_t *pCode, uint8_t length);

    /*!
        Get normalised integer from coded event data
        @param pCode Pointer to normalised integer.
        @param length Number of bytes it consist of including
        the first normalise byte.
        @return returns value as a double.
    */
    double vscp_getDataCodingNormalizedInteger(const unsigned char *pCode,
                                               unsigned char length);

    /*!
        Get the string from coded event data
        @param pString Pointer to normalised integer.
        @param length Number of bytes it consist of including
        the first normalise byte.
        @return Returns unicode UTF-8 string of event data
    */

    bool vscp_getDataCodingString(const unsigned char *pCode,
                                  unsigned char dataSize,
                                  std::string &strResult);

    /*!
        Write data from event in the VSCP data coding format to a string.

        Works for

        CLASS1.MEASUREMENT
        CLASS2_LEVEL1.MEASUREMENT
        VSCP_CLASS1_MEASUREZONE
        VSCP_CLASS1_SETVALUEZONE
        CLASS2_MEASUREMENT_FLOAT
        CLASS2_MEASUREMENT_STR

        @param pEvent Pointer to VSCP event.
        @param str String that holds the result
        @return true on success, false on failure.
    */

    bool vscp_getVSCPMeasurementAsString(const vscpEvent *pEvent,
                                         std::string &str);

    /*!
        Write data from event in the VSCP data coding format as a double.

        Works for

        CLASS1.MEASUREMENT
        CLASS2_LEVEL1.MEASUREMENT
        VSCP_CLASS1_MEASUREZONE
        VSCP_CLASS1_SETVALUEZONE
        CLASS2_MEASUREMENT_FLOAT
        CLASS2_MEASUREMENT_STR

        @param pEvent Pointer to VSCP event.
        @param pvalue Pointer to double that holds the result
        @return true on success, false on failure.
    */
    bool vscp_getVSCPMeasurementAsDouble(const vscpEvent *pEvent,
                                         double *pvalue);

    /*!
     * Get measurement unit for any of the valid measurement events.
     * @param pEvent Pointer to VSCP event.
     * @return Measurement unit or -1 for error (event that is not a
     * measurement).
     */
    int vscp_getVSCPMeasurementUnit(const vscpEvent *pEvent);

    /*!
     * Get measurement sensor index for any of the valid measurement events.
     * @param pEvent Pointer to VSCP event.
     * @return Measurement sensor index or -1 for error or for event that is not
     * a measurement or measurement event that does not have a sensor index.
     */
    int vscp_getVSCPMeasurementSensorIndex(const vscpEvent *pEvent);

    /*!
     * Get measurement zone for any of the valid measurement events.
     * @param pEvent Pointer to VSCP event.
     * @return Measurement zone or 0 for error or event that is not a
     * measurement or measurement event that does not have a zone).
     */
    int vscp_getVSCPMeasurementZone(const vscpEvent *pEvent);

    /*!
     * Get measurement subzone for any of the valid measurement events.
     * @param pEvent Pointer to VSCP event.
     * @return Measurement subzone or -1 for error or for event that is not a
     * measurement or measurement event that does not have a subzone.
     */
    int vscp_getVSCPMeasurementSubZone(const vscpEvent *pEvent);

    /*!
     * Check if event is a measurement
     * @param pEvent Pointer to VSCP event.
     * @return Return true if the event is a measurement.
     */
    bool vscp_isVSCPMeasurement(const vscpEvent *pEvent);

    /*!
        Get data in the VSCP data coding format to a string. Works for
        CLASS1.MEASUREMENT64, CLASS2_LEVEL1.MEASUREMENT64
        @param pEvent Pointer to VSCP event.
        @param str String that holds the result
        @return true on success, false on failure.
        */
    bool vscp_getVSCPMeasurementFloat64AsString(const vscpEvent *pEvent,
                                                std::string &str);

    /*!
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
    bool vscp_convertFloatToNormalizedEventData(uint8_t *pdata,
                                                uint16_t *psize,
                                                double value,
                                                uint8_t unit,
                                                uint8_t sensoridx);

    /*!
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
    bool vscp_convertFloatToFloatEventData(uint8_t *pdata,
                                           uint16_t *psize,
                                           float value,
                                           uint8_t unit,
                                           uint8_t sensoridx);

    /*!
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
    bool vscp_convertIntegerToNormalizedEventData(uint8_t *pdata,
                                                  uint16_t *psize,
                                                  uint64_t val64,
                                                  uint8_t unit      = 0,
                                                  uint8_t sensoridx = 0);

    /*!
        Convert a floating point measurement value into VSCP data as a
        single precision float (32-bit) for
            CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT
        @param value Floating point value to convert.
        @param pEvent Pointer to event with pdata set to NULL. VSCP_class and
        vscp_type must be set to CLASS1.MEASUREMENT, CLASS2_LEVEL1.MEASUREMENT,
        @param psize Pointer to size for returned data.
        @param unit Unit for the data. Zero is default.
        @param sensoridx Sensor index 0-7. Zero is default.
        @return true on success, false on failure.
    */
    bool vscp_makeFloatMeasurementEvent(vscpEvent *pEvent,
                                        float value,
                                        uint8_t unit,
                                        uint8_t sensoridx);

    /*!
        Convert a floating point measurement value into VSCP data as a
        string for CLASS1.MEASUREMENT or CLASS2_LEVEL1.MEASUREMENT
        @param value Floating point value to convert.
        @param pEvent Pointer to event with pdata set to NULL. vscp_class and
        vscp_type must be set to CLASS1.MEASUREMENT or
       CLASS2_LEVEL1.MEASUREMENT,
        @param psize Pointer to size for returned data.
        @param unit Unit for the data. Zero is default.
        @param sensoridx Sensor index 0-7. Zero is default.
        @return true on success, false on failure.
    */
    bool vscp_makeStringMeasurementEvent(vscpEvent *pEvent,
                                         double value,
                                         uint8_t unit,
                                         uint8_t sensoridx);

    /*!
     * Construct CLASS2.MEASUREMENT_FLOAT level II measurement event
     *
     * @param pEvent Pointer to event that will be filled with data or NULL
     *          if event should be allocated.
     * @param value Floating point value that event should be made from.
     * @param unit Unit for this measurement. 0-255. Default is zero.
     * @param sensorsidx Sensor index for this measurement. 0-255. Default us
     * zero.
     * @param zone Zone for this measurement. 0-255. Zero is default.
     * @param Subzone Sub zone for this measurement. 0-255- Zero is default.
     * @return True is returned on success, false if an error occurred.
     */
    bool vscp_makeLevel2FloatMeasurementEvent(vscpEvent *pEvent,
                                              uint16_t type,
                                              double value,
                                              uint8_t unit,
                                              uint8_t sensoridx,
                                              uint8_t zone,
                                              uint8_t subzone);

    /*!
     * Construct CLASS2.MEASUREMENT_STRING level II measurement event
     *
     * @param pEvent Pointer to event that will be filled with data or NULL
     *          if event should be allocated.
     * @param value Floating point value that event should be made from.
     * @param unit Unit for this measurement. 0-255. Default is zero.
     * @param sensorsidx Sensor index for this measurement. 0-255. Default us
     * zero.
     * @param zone Zone for this measurement. 0-255. Zero is default.
     * @param Subzone Sub zone for this measurement. 0-255- Zero is default.
     * @return True is returned on success, false if an error occurred.
     */
    bool vscp_makeLevel2StringMeasurementEvent(vscpEvent *pEvent,
                                               uint16_t type,
                                               double value,
                                               uint8_t unit,
                                               uint8_t sensoridx,
                                               uint8_t zone,
                                               uint8_t subzone);

    /*!
        Get data in the VSCP data coding set as float format to a float
        @param pNorm Pointer to VSCP event.
        @param length Number of bytes it consist of including data coding byte
        @return value as float
        */
    float vscp_getMeasurementAsFloat(const unsigned char *pNorm,
                                     const unsigned char length);
    /*!
     * Convert a Level I measurement event to a Level II double measurement
     * event VSCP_CLASS2_MEASUREMENT_FLOAT
     *
     * @param pEventLevel1 Pointer to level I measurement event to be converted.
     * @param pEventÖevel2 Pointer to resulting level II measurement event.
     * @return true on success, false otherwise.
     */
    bool vscp_convertLevel1MeasuremenToLevel2Double(vscpEvent *pEventLevel1);

    /*!
     * Convert a Level I measurement event to a Level II string measurement
     * event VSCP_CLASS2_MEASUREMENT_STRING
     *
     * @param pEventLevel1 Pointer to level I measurement event to be converted.
     * @param pEventÖevel2 Pointer to resulting level II measurement event.
     * @return true on success, false otherwise.
     */
    bool vscp_convertLevel1MeasuremenToLevel2String(vscpEvent *pEventLevel1);

    // -------------------------------------------------------------------------

    /*!
        Replace backslashes in a string with forward slashes
        @param strval String that should be handled.
        @return Fixed string.
        */
    std::string &vscp_replaceBackslash(std::string &strval);

    /*!
        Get VSCP priority
        @param pEvent Pointer to VSCP event to set priority for.
        @return Priority (0-7) for event.
        */
    unsigned char vscp_getVscpPriority(const vscpEvent *pEvent);

    /*!
        Get VSCP priority
        @param pEvent Pointer to VSCP event to set priority for.
        @return Priority (0-7) for event.
        */
    unsigned char vscp_getVscpPriorityEx(const vscpEventEx *pEvent);

    /*!
        Set VSCP priority
        @param pEvent Pointer to VSCP event to set priority for.
        @param priority Priority (0-7) to set.
        */
    void vscp_setVscpPriority(vscpEvent *pEvent, unsigned char priority);

    /*!
    Set VSCP priority Ex
    @param pEvent Pointer to VSCP event to set priority for.
    @param priority Priority (0-7) to set.
    */
    void vscp_setVscpPriorityEx(vscpEventEx *pEvent, unsigned char priority);

    /*!
        Get VSCP head from CANAL id
        @param id CAN id
        @return VSCP head
        */
    unsigned char vscp_getVSCPheadFromCANALid(uint32_t id);

    /*!
        Get VSCP class from CANAL id
        @param id CAN id
        @return VSCP class
        */
    uint16_t vscp_getVSCPclassFromCANALid(uint32_t id);

    /*!
        Get VSCP type from CANAL id
        @param id CAN id
        @return VSCP type
        */
    uint16_t vscp_getVSCPtypeFromCANALid(uint32_t id);

    /*!
        Get VSCP nide nickname from CANAL id
        @param id CAN id
        @return VSCP node nickname
        */
    uint8_t vscp_getVSCPnicknameFromCANALid(uint32_t id);

    /*!
        Get CANAL id from VSCP info
        @param priority VSCP priority
        @param vscp_class VSCP class
        @param vscp_type VSCP type
        @return CAN id with nickname == 0
        */
    uint32_t vscp_getCANALidFromVSCPdata(unsigned char priority,
                                         const uint16_t vscp_class,
                                         const uint16_t vscp_type);

    /*!
        Get CANAL id from VSCP event
        @param pEvent Pointer to VSCP event
        @return CAN id with nickname == 0
        */
    uint32_t vscp_getCANALidFromVSCPevent(const vscpEvent *pEvent);

    /*!
        Get CAN id from VSCP event
        @param pEvent Pointer to VSCP event
        @return CAN id with nickname == 0
        */
    uint32_t vscp_getCANALidFromVSCPeventEx(const vscpEventEx *pEvent);

    /*!
        Calculate CRC for VSCP event
        */
    unsigned short vscp_calc_crc_Event(vscpEvent *pEvent, short bSet);

    /*!
        Calculate CRC for VSCP event
        */
    unsigned short vscp_calc_crc_EventEx(vscpEventEx *pEvent, short bSet);

    // Helpers

    /*!
        calcCRC4GUIDArray

        @param Pointer to GUID array (MSB-LSB order)
        \return crc for GUID.
    */
    uint8_t vscp_calcCRC4GUIDArray(const uint8_t *pguid);

    /*!
        calcCRC4GUIDString

        @param Pointer to GUID array (MSB-LSB order)
        \return crc for GUID.
    */
    uint8_t vscp_calcCRC4GUIDString(const std::string &strguid);

    /*!
        Get GUID from string

        @param pEvent Pointer to VSCP event
        @param strGUID String with GUID (xx:yy:zz....)
        @return True on success, false on failure.
    */

    bool vscp_setEventGuidFromString(vscpEvent *pEvent,
                                     const std::string &strGUID);

    /*!
        Get GUID from string

        @param pEvent Pointer to VSCP event
        @param strGUID String with GUID (xx:yy:zz....)
        @return True on success, false on failure.
    */

    bool vscp_setEventExGuidFromString(vscpEventEx *pEventEx,
                                       const std::string &strGUID);

    /*!
        Fill event GUID from a string
    */

    bool vscp_getGuidFromStringToArray(unsigned char *pGUID,
                                       const std::string &strGUID);

    /*!
        Write out GUID to string

        @param pGUID Pointer to VSCP GUID array.
        @param strGUID Reference to string for written GUID
        @return True on success, false on failure.
    */

    bool vscp_writeGuidArrayToString(const unsigned char *pGUID,
                                     std::string &strGUID);

    /*!
        Write out GUID to string

        @param pEvent Pointer to VSCP event
        @param strGUID Reference to string for written GUID
        @return True on success, false on failure.
        */

    bool vscp_writeGuidToString(const vscpEvent *pEvent, std::string &strGUID);

    /*!
    Write out GUID to string

    @param pEvent Pointer to VSCP event
    @param strGUID Reference to string for written GUID
    @return True on success, false on failure.
    */

    bool vscp_writeGuidToStringEx(const vscpEventEx *pEvent,
                                  std::string &strGUID);

    /*!
        Write out GUID to string as four rows

        @param pEvent Pointer to VSCP event
        @param strGUID Reference to string for written GUID
        @return True on success, false on failure.
        */

    bool vscp_writeGuidToString4Rows(const vscpEvent *pEvent,
                                     std::string &strGUID);

    /*!
        Write out GUID to string as four rows

        @param pEvent Pointer to VSCP event
        @param strGUID Reference to string for written GUID
        @return True on success, false on failure.
    */

    bool vscp_writeGuidToString4RowsEx(const vscpEventEx *pEvent,
                                       std::string &strGUID);

    /*!
        Check if GUID is all null
        @param pGUID pointer to GUID to check
        @return true if empty, false if not.
        */
    bool vscp_isGUIDEmpty(const unsigned char *pGUID);

    /*!
        Compare two GUID's
        @param pGUID1 First GUID to compare
        @param pGUID2 Second GUID to compare
        @return True if the two GUID's are equal. False otherwise.
        */
    bool vscp_isSameGUID(const unsigned char *pGUID1,
                         const unsigned char *pGUID2);

    /*!
        Reverse GUID
        @param pGUID Pointer to GUID to reverse.
        @return true if OK.
        */
    bool vscp_reverseGUID(unsigned char *pGUID);

    /*!
        Convert a standard VSCP event to the Ex version
        */
    bool vscp_convertVSCPtoEx(vscpEventEx *pEventEx, const vscpEvent *pEvent);

    /*!
        Convert an Ex event to a standard VSCP event
        */
    bool vscp_convertVSCPfromEx(vscpEvent *pEvent, const vscpEventEx *pEventEx);

    /*!
     * Create a standard VSCP event
     * @param ppEvent Pointer to a pointer toa standard VSCP event.
     * @return True if the event was created successfully,
     *              false otherwise.
     */
    bool vscp_newVSCPevent(vscpEvent **ppEvent);

    /*!
        Delete a standard VSCP event
        */
    void vscp_deleteVSCPevent(vscpEvent *pEvent);

    /*!
     * Delete standard VSCP event and NULL
     * @param Pointer to pointer to standard VSCP event.
     */
    void vscp_deleteVSCPevent_v2(vscpEvent **pEvent);

    /*!
        Delete an Ex event
        */
    void vscp_deleteVSCPeventEx(vscpEventEx *pEventEx);

    /*!
        Make a timestamp for events etc
        @return Event timestamp as an unsigned long
        */
    unsigned long vscp_makeTimeStamp(void);

    /*!
        Get milliseconds timestamp
        @return Timestamp as uint32_t
        */
    uint32_t vscp_getMsTimeStamp(void);

    /*!
        Set date & time in stamp block
        *
        * @param pEvent Event to set date/time block in.
        * @return True on success.
        */
    bool vscp_setEventDateTimeBlockToNow(vscpEvent *pEvent);

    /*!
        Set date & time in stamp block

        @param pEventEx EventEx to set date/time block in.
        @return True on success.
        */
    bool vscp_setEventExDateTimeBlockToNow(vscpEventEx *pEventEx);

    /*!
        Get datestring from VSCP event
        @param pEvent Event to get date/time info from
        @param dt Reference to String that will get ISO datetime string
        @return True on success.
        */
    bool vscp_getDateStringFromEvent(const vscpEvent *pEvent, std::string &dt);

    /*!
        Get datestring from VSCP Event ex
        @param pEventEx EventEx to get date/time info from
        @param dt Reference to String that will get ISO datetime string
        @return True on success.
        */
    bool vscp_getDateStringFromEventEx(const vscpEventEx *pEventEx,
                                       std::string &dt);

    /*!
     * Convert VSCP Event to JSON formated string
     */
    bool vscp_convertEventToJSON(vscpEvent *pEvent, std::string &strJSON);

    /*!
     * Convert VSCP EventEx to JSON formated string
     */
    bool vscp_convertEventExToJSON(vscpEventEx *pEventEx, std::string &strJSON);

    /*!
     * Convert JSON string to event
     */
    bool vscp_convertJSONToEvent(std::string &strJSON, vscpEvent *pEvent);

    /*!
     * Convert JSON string to eventex
     */
    bool vscp_convertJSONToEventEx(std::string &strJSON, vscpEventEx *pEventEx);

    /*!
     * Convert VSCP Event to XML formated string
     */
    bool vscp_convertEventToXML(vscpEvent *pEvent, std::string &strXML);

    /*!
     * Convert XML string to event
     */
    bool vscp_convertXMLToEvent(std::string &strXML, vscpEvent *pEvent);

    /*!
     * Convert VSCP EventEx to XML formated string
     */
    bool vscp_convertEventExToXML(vscpEventEx *pEventEx, std::string &strXML);

    /*!
     * Convert XML string to EventEx
     */
    bool vscp_convertXMLToEventEx(std::string &strXML, vscpEventEx *pEventEx);

    /*!
     * Convert VSCP Event to HTML formated string
     */
    bool vscp_convertEventToHTML(vscpEvent *pEvent, std::string &strHTML);

    /*!
     * Convert VSCP EventEx to HTML formated string
     */
    bool vscp_convertEventExToHTML(vscpEventEx *pEventEx, std::string &strHTML);

    /*!
     * Set event datetime from DateTime
     */
    bool vscp_setEventDateTime(vscpEvent *pEvent, struct tm *ptm);

    /*!
     * Set eventex datetime from DateTime
     */
    bool vscp_setEventExDateTime(vscpEventEx *pEventEx, struct tm *ptm);

    /*!
     * Set the event date to now
     *
     * @param pEvent Pointer to event that will have date set to now
     * @return True on success, false on failure.
     */
    bool vscp_setEventToNow(vscpEvent *pEvent);

    /*!
     * Set the event date to now
     *
     * @param pEventEx Pointer to event that will have date set to now
     * @return True on success, false on failue.
     */
    bool vscp_setEventExToNow(vscpEventEx *pEventEx);

    /*!
        Clear VSCP filter so it will allow all events to go through
        @param pFilter Pointer to VSCP filter.
        */
    void vscp_clearVSCPFilter(vscpEventFilter *pFilter);

    /*!
     * Copy filter from one filter to another
     * @param pToFilter Pointer to filter to copy data to
     * @param pFromFilter Pinter to filter to copy data from
     */
    void vscp_copyVSCPFilter(vscpEventFilter *pToFilter,
                             const vscpEventFilter *pFromFilter);

    /*!
        Check filter/mask to check if filter should be delivered

        filter ^ bit    mask    out
        ============================
            0           0       1    filter == bit, mask = don't care result =
       true 0           1       1    filter == bit, mask = valid, result = true
            1           0       1    filter != bit, mask = don't care, result =
       true 1           1       0    filter != bit, mask = valid, result = false

        Mask tells *which* bits that are of interest means
        it always returns true if bit set to zero (0=don't care).

        Filter tells the value for valid bits. If filter bit is == 1 the bits
        must be equal to get a true filter return.

        So a nill mask will let everything through

        A filter pointer set to NULL will let every event through.

        @return true if message should be delivered false if not.
        */
    bool vscp_doLevel2Filter(const vscpEvent *pEvent,
                             const vscpEventFilter *pFilter);

    bool vscp_doLevel2FilterEx(const vscpEventEx *pEventEx,
                               const vscpEventFilter *pFilter);

    /*!
        Read a filter from a string
        If strFilter is an empty string all elements in filter will be set to
       zero. Arguments is priority,class,type,GUID and all is optional but if
       given must be given in order.
        @param pFilter Filter structure to write filter to.
        @param strFilter Filter in string form
                filter-priority, filter-class, filter-type, filter-GUID
        @return true on success, false on failure.
        */

    bool vscp_readFilterFromString(vscpEventFilter *pFilter,
                                   const std::string &strFilter);

    /*!
        Write filter to string
        @param pFilter Filter structure to write out to string.
        @param strFilter Filter in string form
                filter-priority, filter-class, filter-type, filter-GUID
        @return true on success, false on failure.
    */
    bool vscp_writeFilterToString(const vscpEventFilter *pFilter,
                                  std::string &strFilter);

    /*!
        Read a mask from a string
        If strMask is an empty string elements in mask will be set to zero.
       Arguments is

        "priority,class,type,GUID"

        and all is optional but if given must be given in order.
        @param pFilter Filter structure to write mask to.
        @param strMask Mask in string form
                mask-priority, mask-class, mask-type, mask-GUID
        @return true on success, false on failure.
        */

    bool vscp_readMaskFromString(vscpEventFilter *pFilter,
                                 const std::string &strMask);

    /*!
        Write mask to string
        @param pFilter Filter structure to write out to string.
        @param strFilter Mask in string form
                mask-priority, mask-class, mask-type, mask-GUID
        @return true on success, false on failure.
    */
    bool vscp_writeMaskToString(const vscpEventFilter *pFilter,
                                std::string &strFilter);

    /*!
     * Read both filter and mask from string
     *
     * @param pFilter Pointer to VSCP filter structure which will receive filter
     *                  mask data.
     * @param strFilter Filter and mask in comma separated list
     * @return true on success, false on failure.
     */
    bool vscp_readFilterMaskFromString(vscpEventFilter *pFilter,
                                       const std::string &strFilterMask);

    /*!
     * Read filter from XML coded string
     *
     * <filter
     *      mask_priority="number"
     *      mask_class="number"
     *      mask_type="number"
     *      mask_guid="GUID string"
     *     filter_priority="number"
     *      filter_class="number"
     *      filter_type="number"
     *      filter_guid="GUID string"
     * />
     */
    bool vscp_readFilterMaskFromXML(vscpEventFilter *pFilter,
                                    const std::string &strFilter);

    /*!
     * Write filter to XML coed string
     */
    bool vscp_writeFilterMaskToXML(vscpEventFilter *pFilter,
                                   std::string &strFilter);

    /*!
     * Read filter from JSNOM coded string
     *
     * {
     *      'mask_priority': number,
     *      'mask_class': number,
     *      'mask_type': number,
     *      'mask_guid': 'string',
     *      'filter_priority'; number,
     *      'filter_class': number,
     *      'filter_type': number,
     *      'filter_guid' 'string'
     * }
     *
     */
    bool vscp_readFilterMaskFromJSON(vscpEventFilter *pFilter,
                                     const std::string &strFilter);

    /*
     * Write filter to JSON coded string
     */
    bool vscp_writeFilterMaskToJSON(vscpEventFilter *pFilter,
                                    std::string &strFilter);

    /*!
        Convert an Event from a CANAL message
        */
    bool vscp_convertCanalToEvent(vscpEvent *pvscpEvent,
                                  const canalMsg *pcanalMsg,
                                  unsigned char *pGUID);

    /*!
        Convert an Event from a CANAL message
        */
    bool vscp_convertCanalToEventEx(vscpEventEx *pvscpEvent,
                                    const canalMsg *pcanalMsg,
                                    unsigned char *pGUID);

    /*!
        Covert VSCP event to CANAL message
        */
    bool vscp_convertEventToCanal(canalMsg *pcanalMsg,
                                  const vscpEvent *pvscpEvent);

    /*!
        Covert VSCP event to CANAL message
    */
    bool vscp_convertEventExToCanal(canalMsg *pcanalMsg,
                                    const vscpEventEx *pvscpEvent);

    /*!
        Copy one VSCP event to another

        @param pEventTo Pointer to event to copy to.
        @param pEventFrom Pointer to event to copy from.
        @return True on success.
    */
    bool vscp_copyVSCPEvent(vscpEvent *pEventTo, const vscpEvent *pEventFrom);

    /*!
    Copy one VSCP event ex to another

    @param pEventTo Pointer to event to copy to.
    @param pEventFrom Pointer to event to copy from.
    @return True on success.
*/
    bool vscp_copyVSCPEventEx(vscpEventEx *pEventTo,
                              const vscpEventEx *pEventFrom);

    /*!
        Write VSCP data to string    DEPRECATED: USE:
       vscp_writeVscpDataWithSizeToString
        @param pEvent Pointer to event where data is fetched from
        @param str String that receive result.
        @param bUseHtmlBreak Set to true to use <br> instead of \\n as
        line break
        @return True on success false on failure.
        */

    bool vscp_writeVscpDataToString(const vscpEvent *pEvent,
                                    std::string &str,
                                    bool bUseHtmlBreak = false,
                                    bool bBreak        = false);

    /*!
        Write VSCP data to string
        @param sizeData Number of data bytes.
        @param pData Pointer to data structure.
        @param str String that receive result.
        @param bUseHtmlBreak Set to true to use <br> instead of \\n as
        line break
        @return True on success false on failure.
    */

    bool vscp_writeVscpDataWithSizeToString(const uint16_t sizeData,
                                            const unsigned char *pData,
                                            std::string &str,
                                            bool bUseHtmlBreak = false,
                                            bool bBreak        = true);

    /*!
        Set VSCP Event data from a string
        @param pEvent Pointer to a VSCP event to write parsed data to.
        @param str A string with comma or whitespace separated data in decimal
        or hexadecimal form. Data can span multiple lines.
        @return true on success, false on failure.
    */

    bool vscp_setVscpEventDataFromString(vscpEvent *pEvent,
                                         const std::string &str);

    /*!
        Set VSCP EventEx data from a string
        @param pEventEx Pointer to a VSCP event to write parsed data to.
        @param str A string with comma or whitespace separated data in decimal
        or hexadecimal form. Data can span multiple lines.
        @return true on success, false on failure.
    */

    bool vscp_setVscpEventExDataFromString(vscpEventEx *pEventEx,
                                           const std::string &str);

    /*!
        Set VSCP data from a string
        @param pData Pointer to a unsigned byte array to write parsed data to.
        @param psizeData Number of data bytes.
        @param str A string with comma or whitespace separated data in decimal
        or hexadecimal form. Data can span multiple lines.
        @return true on success, false on failure.
    */

    bool vscp_setVscpDataArrayFromString(uint8_t *pData,
                                         uint16_t *psizeData,
                                         const std::string &str);

    /*!
        Write event to string.
        priority,class,type,guid,data
        @param pEvent Event that should be presented
        @param str String that receive the result
        @return true on success, false on failure.
    */

    bool vscp_writeVscpEventToString(const vscpEvent *pEvent, std::string &str);

    /*!
        Write event to string.
        priority,class,type,guid,data
        @param pEvent Event that should be presented
        @param str String that receive the result
        @return true on success, false on failure.
        */

    bool vscp_writeVscpEventExToString(const vscpEventEx *pEvent,
                                       std::string &str);

    /*!
        Get event data from string format
        Format: head,class,type,obid,timestamp,GUID,data1,data2,data3....
        @param pEvent Event that will get data
        @param str String that contain the event on string form
        @return true on success, false on failure.
        */

    bool vscp_setVscpEventFromString(vscpEvent *pEvent, const std::string &str);

    /*!
        Get event data from string format
        Format: head,class,type,obid,timestamp,GUID,data1,data2,data3....
        @param pEventEx Pointer to VSCP event that will get the parsed data
        @param str String that contain the event on string form
        @return true on success, false on failure.
        */

    bool vscp_setVscpEventExFromString(vscpEventEx *pEventEx,
                                       const std::string &str);

    /*!
        Write in measurement data into real text string             TODO REMOVE
       !!!!!!!!!
        @param vscptype VSCP type
        @param unit Measurement unit, 0-3 for Level I, 0-255 for Level II.
        @param sensoridx Index for sensor, 0-7 for Level I, 0-255 for Level II.
    */
    std::string &vscp_writeMeasurementValue(uint16_t vscptype,
                                            uint8_t unit,
                                            uint8_t sensoridx,
                                            std::string &strValue);

    /*!
        Get Data in real text.
        This for measurement class events that can be describes in real text
        in a deterministic way. Temperature event is typical which can be
       returned in clear text as "Temperature = 22.5 C".
        @param pEvent Event that should be presented
        @return Text data representation of the event data or an empty string
        if the class/type pair is not supported..
        */

    std::string vscp_getRealTextData(vscpEvent *pEvent);

    /*!
        This function makes a HTML string from a standard string. LF is replaced
        with a '<BR>'.
        @param str String that should be HTML coded.
        */

    void vscp_makeHtml(std::string &str);

    /*
        Get device HTML status from device
        @param registers Array with all 256 registers for the device
        @param pmdf Optional pointer to CMDF class which gives more info
        about the device if it is supplied.
    */

    std::string &vscp_getDeviceHtmlStatusInfo(const uint8_t *registers,
                                              CMDF *pmdf);

    ////////////////////////////////////////////////////////////////////////////
    // Encryption
    ////////////////////////////////////////////////////////////////////////////

    /*!
     * Get encryption code from token.
     *
     * @param token Encryption token to set encryption from. Tokens are
     *              define in vscp,h
     * @return Return the encryption code if the encryption code is known.
     *         The function does not return an error code and instead set no
     *         encryption if the token is invalid.
     *
     */

    uint8_t vscp_getEncryptionCodeFromToken(std::string &token);

    /*!
     * Fetch encryption token from code.
     *
     * @param code Should be a valid encryption code as defined in vscp.h
     * @param token A encryption token is returned if the code is valid. For a
     *         a code == 0 which means no encryption an empty string is
     *         returned. This is also true for invalid codes.
     */

    void vscp_getEncryptionTokenFromCode(uint8_t code, std::string &token);

    /*!
     * Get UDP frame size from event
     *
     * @param pEvent Pointer to event.
     * @return Size of resulting UDP frame on success. Zero on failure.
     */
    size_t vscp_getUDpFrameSizeFromEvent(vscpEvent *pEvent);

    /*!
     * Get UDP frame size from event
     *
     * @param pEventEx Pointer to event ex.
     * @return Size of resulting UDP frame on success. Zero on failure.
     */
    size_t vscp_getUDpFrameSizeFromEventEx(vscpEventEx *pEventEx);

    /*!
     * Write event on UDP frame format
     *
     * @param buf A pointer to a buffer that will receive the event.
     * @param len Size of the buffer.
     * @param pkttype Is the first byte of UDP type frames that holds
     *          type of packet and encryption.
     * @param pEvent Pointer to event that should be handled.
     * @return True on success, false on failure.
     */
    bool vscp_writeEventToUdpFrame(uint8_t *frame,
                                   size_t len,
                                   uint8_t pkttype,
                                   const vscpEvent *pEvent);

    /*!
     * Write event ex on UDP frame format
     *
     * @param buf A pointer to a buffer that will receive the event.
     * @param len Size of the buffer.
     * @param pkttype Is the first byte of UDP type frames that holds
     *          type of packet and encryption.
     * @param pEventEx Pointer to event that should be handled.
     * @return True on success, false on failure.
     */
    bool vscp_writeEventExToUdpFrame(uint8_t *frame,
                                     size_t len,
                                     uint8_t pkttype,
                                     const vscpEventEx *pEventEx);

    /*!
     * Get VSCP event from UDP frame
     *
     * @param pEvent Pointer to VSCP event that will get data from the frame,
     * @param buf A pointer to a buffer that will receive the event.
     * @param len Size of the buffer.
     * @return True on success, false on failure.
     */
    bool vscp_getEventFromUdpFrame(vscpEvent *pEvent,
                                   const uint8_t *buf,
                                   size_t len);

    /*!
     * Get VSCP event ex from UDP frame
     *
     * @param pEventEx Pointer to VSCP event ex that will get data from the
     * frame,
     * @param buf A pointer to a buffer that will receive the event.
     * @param len Size of the buffer.
     * @return True on success, false on failure.
     */
    bool vscp_getEventExFromUdpFrame(vscpEventEx *pEventEx,
                                     const uint8_t *buf,
                                     size_t len);

    /*!
     * Encrypt VSCP UDP frame using the selected encryption algorithm. The iv
     * initialization vector) is appended to the end of the encrypted data.
     *
     * @param output Buffer that will receive the encrypted result. The buffer
     *          should be at least 16 bytes larger than the frame.
     * @param input This is the UDP frame that should be encrypted. The first
     *          byte in the frame is the packet type which is not encrypted.
     * @param len This is the length of the UDP frame to be encrypted. This
     *          length includes the packet tye in the first byte.
     * @param key This is a pointer to the secret encryption key. This key
     * should be 128 bits for AES128, 192 bits for AES192, 256 bits for AES256.
     * @param iv Pointer to the initialization vector. Should always point to a
     * 128 bit content. If NULL the iv will be created from random system data.
     * In both cases the end result will have the iv appended to the encrypted
     * block.
     * @param nAlgorithm The VSCP defined algorithm to encrypt the frame with.
     * @return Packet length on success, zero on failure.
     *
     */
    size_t vscp_encryptVscpUdpFrame(uint8_t *output,
                                    uint8_t *input,
                                    size_t len,
                                    const uint8_t *key,
                                    const uint8_t *iv,
                                    uint8_t nAlgorithm);

    /*!
     * Decrypt VSCP UDP frame using the selected encryption algorithm. The iv
     * initialization vector) is appended to the end of the encrypted data.
     *
     * @param output Buffer that will receive the decrypted result. The buffer
     *          should have a size at lest equal to the encrypted block.
     * @param input This is the UDP frame that should be decrypted.
     * @param len This is the length of the UDP frame to be decrypted.
     * @param key This is a pointer to the secret encryption key. This key
     * should be 128 bytes for AES128, 192 bytes for AES192, 256 bytes for
     * AES256.
     * @param iv Pointer to the initialization vector. Should always point to a
     * 128 bit content. If NULL the iv is expected to be the last 16 bytes of
     * the encrypted data.
     * @param nAlgorithm The VSCP defined algorithm to decrypt the frame with.
     * @return True on success, false on failure.
     *
     * NOTE: Note that VSCP packet type (first byte in UDP frame) is not
     * recognised here.
     *
     */
    bool vscp_decryptVscpUdpFrame(uint8_t *output,
                                  uint8_t *input,
                                  size_t len,
                                  const uint8_t *key,
                                  const uint8_t *iv,
                                  uint8_t nAlgorithm);

    ///////////////////////////////////////////////////////////////////////////
    //                         Password/key handling
    ///////////////////////////////////////////////////////////////////////////

    /*!
     * Calculate md5 hex digest for buf
     *
     * @param digest Buffer (33 bytes) that will receive the digest in hex
     * format.
     * @param buf Data to calculate md5 on.
     * @param len Len of input data.
     */
    void vscp_md5(char *digest, const unsigned char *buf, size_t len);

    /*!
    Stringify binary data.
    @param to Pointer output buffer that holds the result.
    Output buffer must be twice as big as input,
    because each byte takes 2 bytes in string representation
    @param p Pointer to digest.
    @param len Digest len
*/
    void vscp_byteArray2HexStr(char *to, const unsigned char *p, size_t len);

    /*!
     * Convert a hext string to a byte array
     *
     * @param array Byte array that will receive result.
     * @param size Size of byte array.
     * @param hextstr Hex string that should be converted.
     * @return Number of bytes written to the byte array.
     *
     */

    size_t vscp_hexStr2ByteArray(uint8_t *array,
                                 size_t size,
                                 const char *hexstr);

    /*!
     * Get components form a hashed password
     *
     * VSCP passwords is stored as two hex strings separated with a ";"-
     * The first string is the salt, the second the hashed password.
     *
     * @param pSalt Pointer to a 16 byte buffer that will receive the salt.
     * @param pHash Pointer to a 32 byte buffer that will receive the salt.
     * @param stored_pw Stored password on the form salt;hash
     * @return True on success, false on failure.
     *
     */
    bool vscp_getHashPasswordComponents(uint8_t *pSalt,
                                        uint8_t *pHash,
                                        const std::string &stored_pw);

    /*!
     * Make password hash with prepended salt from clear text password.
     *
     * @param result Will get hex hash string with random salt prepended
     * separated with ";".
     * @param password Clear text password to be hashed.
     * @return true on success, false otherwise.
     */
    bool vscp_makePasswordHash(std::string &result,
                               const std::string &password,
                               uint8_t *pSalt = NULL);

    /*!
     * Validate password
     *
     * @param stored_pw Stored password on the form "salt;hash"
     * @param password Password to test.
     * @return true on success, false otherwise.
     */

    bool vscp_isPasswordValid(const std::string &stored_pw,
                              const std::string &password);

    /*!
     * Get salt
     *
     * @param Pointer to byte array to write salt to.
     * @param len Number of bytes to generate.
     * @return True on success.
     *
     */
    bool vscp_getSalt(uint8_t *buf, size_t len);

    /*!
     * Get salt as hex string
     *
     * @param strSalt String that will get the salt in hex format.
     * @param len Number of bytes to generate.
     * @return True on success.
     */
    bool vscp_getSaltHex(std::string &strSalt, size_t len);

#ifdef __cplusplus
}
#endif

#endif // #if !defined(VSCPHELPER_H__INCLUDED_)
