// unittest.cpp
//
// Unit tests for vscphelper.cpp
//
// Copyright © 2000-2025 Ake Hedman, the VSCP project
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

#include <gtest/gtest.h>
#include <string>
#include <deque>
#include <cstring>
#include <climits>
#include <cmath>
#include <cstdint>

#include "vscphelper.h"
#include "vscp.h"
#include "guid.h"
#include "canal.h"

// =============================================================================
//                           String Value Parsing
// =============================================================================

TEST(VscpHelper, readStringValue_Decimal)
{
    EXPECT_EQ(54321, vscp_readStringValue("54321"));
    EXPECT_EQ(102, vscp_readStringValue("102"));
    EXPECT_EQ(12345, vscp_readStringValue("12345"));
    EXPECT_EQ(0, vscp_readStringValue("0"));
    EXPECT_EQ(1, vscp_readStringValue("1"));
}

TEST(VscpHelper, readStringValue_Hexadecimal)
{
    EXPECT_EQ(65535, vscp_readStringValue("0xffff"));
    EXPECT_EQ(255, vscp_readStringValue("0xFF"));
    EXPECT_EQ(16, vscp_readStringValue("0x10"));
    EXPECT_EQ(0, vscp_readStringValue("0x0"));
    EXPECT_EQ(0xABCD, vscp_readStringValue("0xABCD"));
}

TEST(VscpHelper, readStringValue_Octal)
{
    EXPECT_EQ(63, vscp_readStringValue("0o77"));
    EXPECT_EQ(8, vscp_readStringValue("0o10"));
    EXPECT_EQ(0, vscp_readStringValue("0o0"));
    EXPECT_EQ(7, vscp_readStringValue("0o7"));
}

TEST(VscpHelper, readStringValue_Binary)
{
    EXPECT_EQ(10, vscp_readStringValue("0b1010"));
    EXPECT_EQ(255, vscp_readStringValue("0b11111111"));
    EXPECT_EQ(0, vscp_readStringValue("0b0"));
    EXPECT_EQ(1, vscp_readStringValue("0b1"));
    // Invalid binary digits should stop parsing
    EXPECT_EQ(1, vscp_readStringValue("0b189"));
}

TEST(VscpHelper, readStringValue_LargeNumbers)
{
    EXPECT_EQ(UINT32_MAX, vscp_readStringValue("4294967295"));
}

// =============================================================================
//                           String Case Conversion
// =============================================================================

TEST(VscpHelper, lowercase)
{
    // vscp_lowercase returns lowercase of first character, not modifies string
    EXPECT_EQ('h', vscp_lowercase("HELLO"));
    EXPECT_EQ('a', vscp_lowercase("ABC"));
    EXPECT_EQ('z', vscp_lowercase("Z"));
    EXPECT_EQ('0', vscp_lowercase("0"));  // non-alpha unchanged
}

TEST(VscpHelper, makeUpper_InPlace)
{
    std::string s1 = "hello world";
    vscp_makeUpper(s1);
    EXPECT_EQ("HELLO WORLD", s1);
    
    std::string s2 = "HeLLo WoRLd";
    vscp_makeUpper(s2);
    EXPECT_EQ("HELLO WORLD", s2);
}

TEST(VscpHelper, makeUpper_Copy)
{
    std::string s = "hello world";
    std::string result = vscp_makeUpper_copy(s);
    EXPECT_EQ("HELLO WORLD", result);
    EXPECT_EQ("hello world", s);  // Original unchanged
}

TEST(VscpHelper, upper)
{
    std::string result = vscp_upper("hello world");
    EXPECT_EQ("HELLO WORLD", result);
}

TEST(VscpHelper, makeLower_InPlace)
{
    std::string s1 = "HELLO WORLD";
    vscp_makeLower(s1);
    EXPECT_EQ("hello world", s1);
    
    std::string s2 = "HeLLo WoRLd";
    vscp_makeLower(s2);
    EXPECT_EQ("hello world", s2);
}

TEST(VscpHelper, makeLower_Copy)
{
    std::string s = "HELLO WORLD";
    std::string result = vscp_makeLower_copy(s);
    EXPECT_EQ("hello world", result);
    EXPECT_EQ("HELLO WORLD", s);  // Original unchanged
}

TEST(VscpHelper, lower)
{
    std::string result = vscp_lower("HELLO WORLD");
    EXPECT_EQ("hello world", result);
}

// =============================================================================
//                           String Comparison
// =============================================================================

TEST(VscpHelper, strcasecmp)
{
    EXPECT_EQ(0, vscp_strcasecmp("hello", "HELLO"));
    EXPECT_EQ(0, vscp_strcasecmp("HeLLo", "hElLo"));
    EXPECT_NE(0, vscp_strcasecmp("hello", "world"));
    EXPECT_NE(0, vscp_strcasecmp("hello", "hello1"));
}

TEST(VscpHelper, strncasecmp)
{
    EXPECT_EQ(0, vscp_strncasecmp("hello", "HELLO", 5));
    EXPECT_EQ(0, vscp_strncasecmp("hello_world", "HELLO_PLANET", 5));
    EXPECT_NE(0, vscp_strncasecmp("hello_world", "HELLO_PLANET", 10));
}

// =============================================================================
//                           String Trimming
// =============================================================================

TEST(VscpHelper, ltrim_InPlace)
{
    std::string s1 = "   hello";
    vscp_ltrim(s1);
    EXPECT_EQ("hello", s1);
    
    std::string s2 = "\t\n  hello";
    vscp_ltrim(s2);
    EXPECT_EQ("hello", s2);
    
    std::string s3 = "hello   ";
    vscp_ltrim(s3);
    EXPECT_EQ("hello   ", s3);  // Right side unchanged
}

TEST(VscpHelper, rtrim_InPlace)
{
    std::string s1 = "hello   ";
    vscp_rtrim(s1);
    EXPECT_EQ("hello", s1);
    
    std::string s2 = "hello\t\n  ";
    vscp_rtrim(s2);
    EXPECT_EQ("hello", s2);
    
    std::string s3 = "   hello";
    vscp_rtrim(s3);
    EXPECT_EQ("   hello", s3);  // Left side unchanged
}

TEST(VscpHelper, trim_InPlace)
{
    std::string s1 = "   hello   ";
    vscp_trim(s1);
    EXPECT_EQ("hello", s1);
    
    std::string s2 = "\t\n  hello  \t\n";
    vscp_trim(s2);
    EXPECT_EQ("hello", s2);
}

TEST(VscpHelper, ltrim_Copy)
{
    std::string result = vscp_ltrim_copy("   hello");
    EXPECT_EQ("hello", result);
}

TEST(VscpHelper, rtrim_Copy)
{
    std::string result = vscp_rtrim_copy("hello   ");
    EXPECT_EQ("hello", result);
}

TEST(VscpHelper, trim_Copy)
{
    std::string result = vscp_trim_copy("   hello   ");
    EXPECT_EQ("hello", result);
}

TEST(VscpHelper, trimWhiteSpace)
{
    char buf[] = "   hello world   ";
    char *result = vscp_trimWhiteSpace(buf);
    EXPECT_STREQ("hello world", result);
}

// =============================================================================
//                           String Split
// =============================================================================

TEST(VscpHelper, split_Comma)
{
    std::deque<std::string> parts;
    vscp_split(parts, "one,two,three", ",");
    ASSERT_EQ(3u, parts.size());
    EXPECT_EQ("one", parts[0]);
    EXPECT_EQ("two", parts[1]);
    EXPECT_EQ("three", parts[2]);
}

TEST(VscpHelper, split_Space)
{
    std::deque<std::string> parts;
    vscp_split(parts, "one two three", " ");
    ASSERT_EQ(3u, parts.size());
    EXPECT_EQ("one", parts[0]);
    EXPECT_EQ("two", parts[1]);
    EXPECT_EQ("three", parts[2]);
}

TEST(VscpHelper, split_MultiCharDelimiter)
{
    std::deque<std::string> parts;
    vscp_split(parts, "one::two::three", "::");
    ASSERT_EQ(3u, parts.size());
    EXPECT_EQ("one", parts[0]);
    EXPECT_EQ("two", parts[1]);
    EXPECT_EQ("three", parts[2]);
}

TEST(VscpHelper, split_EmptyParts)
{
    std::deque<std::string> parts;
    vscp_split(parts, "one,,three", ",");
    ASSERT_EQ(3u, parts.size());
    EXPECT_EQ("one", parts[0]);
    EXPECT_EQ("", parts[1]);
    EXPECT_EQ("three", parts[2]);
}

// =============================================================================
//                           String Left/Right
// =============================================================================

TEST(VscpHelper, str_left)
{
    EXPECT_EQ("hel", vscp_str_left("hello", 3));
    EXPECT_EQ("hello", vscp_str_left("hello", 5));
    EXPECT_EQ("hello", vscp_str_left("hello", 10));
    EXPECT_EQ("", vscp_str_left("hello", 0));
}

TEST(VscpHelper, str_right)
{
    EXPECT_EQ("llo", vscp_str_right("hello", 3));
    EXPECT_EQ("hello", vscp_str_right("hello", 5));
    EXPECT_EQ("hello", vscp_str_right("hello", 10));
    EXPECT_EQ("", vscp_str_right("hello", 0));
}

TEST(VscpHelper, str_before)
{
    EXPECT_EQ("hello", vscp_str_before("hello,world", ','));
    EXPECT_EQ("", vscp_str_before("hello", ','));
}

TEST(VscpHelper, str_after)
{
    EXPECT_EQ("world", vscp_str_after("hello,world", ','));
    EXPECT_EQ("", vscp_str_after("hello", ','));
}

// =============================================================================
//                           String Starts With
// =============================================================================

TEST(VscpHelper, startsWith)
{
    EXPECT_TRUE(vscp_startsWith("hello world", "hello"));
    EXPECT_TRUE(vscp_startsWith("hello world", "h"));
    EXPECT_TRUE(vscp_startsWith("hello world", "hello world"));
    EXPECT_FALSE(vscp_startsWith("hello world", "world"));
    EXPECT_FALSE(vscp_startsWith("hello", "hello world"));
    EXPECT_TRUE(vscp_startsWith("hello", ""));
}

// =============================================================================
//                           String Reverse
// =============================================================================

TEST(VscpHelper, reverse)
{
    // vscp_reverse returns a newly allocated reversed string
    char *r1 = vscp_reverse("hello");
    ASSERT_NE(nullptr, r1);
    EXPECT_STREQ("olleh", r1);
    free(r1);
    
    char *r2 = vscp_reverse("ab");
    ASSERT_NE(nullptr, r2);
    EXPECT_STREQ("ba", r2);
    free(r2);
    
    char *r3 = vscp_reverse("a");
    ASSERT_NE(nullptr, r3);
    EXPECT_STREQ("a", r3);
    free(r3);
}

// =============================================================================
//                           String Format
// =============================================================================

TEST(VscpHelper, str_format)
{
    EXPECT_EQ("Hello World", vscp_str_format("Hello %s", "World"));
    EXPECT_EQ("Number: 42", vscp_str_format("Number: %d", 42));
    EXPECT_EQ("Hex: ff", vscp_str_format("Hex: %x", 255));
}

// =============================================================================
//                           String Search
// =============================================================================

TEST(VscpHelper, strcasestr)
{
    const char *str = "Hello World";
    EXPECT_NE(nullptr, vscp_strcasestr(str, "world"));
    EXPECT_NE(nullptr, vscp_strcasestr(str, "HELLO"));
    EXPECT_EQ(nullptr, vscp_strcasestr(str, "xyz"));
}

TEST(VscpHelper, stristr)
{
    char str[] = "Hello World";
    char *result = vscp_stristr(str, "world");
    EXPECT_NE(nullptr, result);
    if (result) {
        EXPECT_EQ(str + 6, result);
    }
}

// =============================================================================
//                           is_Number
// =============================================================================

TEST(VscpHelper, isNumber)
{
    EXPECT_TRUE(vscp_isNumber("123"));
    EXPECT_TRUE(vscp_isNumber("0x1F"));
    EXPECT_TRUE(vscp_isNumber("0o77"));
    EXPECT_TRUE(vscp_isNumber("0b1010"));
    EXPECT_TRUE(vscp_isNumber("  123  "));  // With whitespace
    EXPECT_FALSE(vscp_isNumber("abc"));
    EXPECT_FALSE(vscp_isNumber("hello123"));
}

// =============================================================================
//                           Float Comparison
// =============================================================================

TEST(VscpHelper, almostEqualRelativeFloat)
{
    EXPECT_TRUE(vscp_almostEqualRelativeFloat(1.0f, 1.0f, 0.001f));
    EXPECT_TRUE(vscp_almostEqualRelativeFloat(1.0f, 1.0001f, 0.001f));
    EXPECT_FALSE(vscp_almostEqualRelativeFloat(1.0f, 1.01f, 0.001f));
    EXPECT_TRUE(vscp_almostEqualRelativeFloat(0.0f, 0.0f, 0.001f));
}

TEST(VscpHelper, almostEqualRelativeDouble)
{
    EXPECT_TRUE(vscp_almostEqualRelativeDouble(1.0, 1.0, 0.001));
    EXPECT_TRUE(vscp_almostEqualRelativeDouble(1.0, 1.0001, 0.001));
    EXPECT_FALSE(vscp_almostEqualRelativeDouble(1.0, 1.01, 0.001));
    EXPECT_TRUE(vscp_almostEqualRelativeDouble(0.0, 0.0, 0.001));
}

// =============================================================================
//                           Platform Detection
// =============================================================================

TEST(VscpHelper, platformBitness)
{
    // One of these must be true
    EXPECT_TRUE(vscp_is64Bit() || vscp_is32Bit());
    // They should be mutually exclusive
    EXPECT_NE(vscp_is64Bit(), vscp_is32Bit());
}

TEST(VscpHelper, platformEndianness)
{
    // One of these must be true
    EXPECT_TRUE(vscp_isLittleEndian() || vscp_isBigEndian());
    // They should be mutually exclusive
    EXPECT_NE(vscp_isLittleEndian(), vscp_isBigEndian());
}

// =============================================================================
//                           strlcpy / strdup
// =============================================================================

TEST(VscpHelper, strlcpy)
{
    char dest[10];
    vscp_strlcpy(dest, "hello", sizeof(dest));
    EXPECT_STREQ("hello", dest);
    
    // Test truncation
    vscp_strlcpy(dest, "hello world!", sizeof(dest));
    EXPECT_STREQ("hello wor", dest);  // Truncated to fit buffer
}

TEST(VscpHelper, strdup)
{
    char *dup = vscp_strdup("hello");
    ASSERT_NE(nullptr, dup);
    EXPECT_STREQ("hello", dup);
    free(dup);
}

TEST(VscpHelper, strndup)
{
    char *dup = vscp_strndup("hello world", 5);
    ASSERT_NE(nullptr, dup);
    EXPECT_STREQ("hello", dup);
    free(dup);
}

// =============================================================================
//                           Base64 Encoding/Decoding
// =============================================================================

TEST(VscpHelper, base64_encode_decode)
{
    std::string original = "Hello, World!";
    std::string encoded = original;
    
    EXPECT_TRUE(vscp_base64_std_encode(encoded));
    EXPECT_NE(original, encoded);
    
    EXPECT_TRUE(vscp_base64_std_decode(encoded));
    EXPECT_EQ(original, encoded);
}

TEST(VscpHelper, base64_convertToBase64)
{
    std::string result = vscp_convertToBase64("Hello");
    EXPECT_FALSE(result.empty());
    
    // Decode it back
    EXPECT_TRUE(vscp_base64_std_decode(result));
    EXPECT_EQ("Hello", result);
}

TEST(VscpHelper, decodeBase64IfNeeded)
{
    // With BASE64: prefix
    std::string encoded;
    vscp_base64_std_encode(encoded = "Hello");
    std::string withPrefix = "BASE64:" + encoded;
    std::string result;
    EXPECT_TRUE(vscp_std_decodeBase64IfNeeded(withPrefix, result));
    EXPECT_EQ("Hello", result);
    
    // Without prefix - should pass through unchanged
    std::string plain = "Hello";
    EXPECT_TRUE(vscp_std_decodeBase64IfNeeded(plain, result));
    EXPECT_EQ("Hello", result);
}

// =============================================================================
//                           Time Functions
// =============================================================================

TEST(VscpHelper, to_unix_ns)
{
    // Test Jan 1, 2000, 00:00:00
    int64_t ns = vscp_to_unix_ns(2000, 1, 1, 0, 0, 0, 0);
    EXPECT_GT(ns, 0);  // Should be positive
    
    // Unix epoch (1970-01-01)
    ns = vscp_to_unix_ns(1970, 1, 1, 0, 0, 0, 0);
    EXPECT_EQ(0, ns);
}

TEST(VscpHelper, from_unix_ns)
{
    int year, month, day, hour, minute, second;
    uint32_t timestamp;
    
    // Unix epoch
    vscp_from_unix_ns(&year, &month, &day, &hour, &minute, &second, &timestamp, 0);
    EXPECT_EQ(1970, year);
    EXPECT_EQ(1, month);
    EXPECT_EQ(1, day);
    EXPECT_EQ(0, hour);
    EXPECT_EQ(0, minute);
    EXPECT_EQ(0, second);
}

TEST(VscpHelper, to_from_unix_ns_roundtrip)
{
    // Create a timestamp
    int64_t original_ns = vscp_to_unix_ns(2024, 6, 15, 12, 30, 45, 123456);
    
    // Convert back
    int year, month, day, hour, minute, second;
    uint32_t timestamp;
    vscp_from_unix_ns(&year, &month, &day, &hour, &minute, &second, &timestamp, original_ns);
    
    EXPECT_EQ(2024, year);
    EXPECT_EQ(6, month);
    EXPECT_EQ(15, day);
    EXPECT_EQ(12, hour);
    EXPECT_EQ(30, minute);
    EXPECT_EQ(45, second);
}

// =============================================================================
//                           GUID Functions
// =============================================================================

TEST(VscpHelper, getGuidFromStringToArray)
{
    uint8_t guid[16];
    std::string strGUID = "00:01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F";
    
    EXPECT_TRUE(vscp_getGuidFromStringToArray(guid, strGUID));
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(i, guid[i]);
    }
}

TEST(VscpHelper, parseGuid)
{
    uint8_t guid[16] = {};
    std::string strGUID = "00:01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F";

    EXPECT_TRUE(vscp_parseGuid(guid, strGUID));

    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(i, guid[i]);
    }
}

TEST(VscpHelper, parseGuid_InvalidInput)
{
    uint8_t guid[16] = {};

    EXPECT_FALSE(vscp_parseGuid(guid, "not-a-guid"));
    EXPECT_FALSE(vscp_parseGuid(nullptr, "00:01:02:03"));
}

TEST(VscpHelper, getGuidFromStringToArray_UUID)
{
    uint8_t guid[16];
    // UUID format: 8-4-4-4-12 hex digits
    std::string strGUID = "00010203-0405-0607-0809-0A0B0C0D0E0F";
    
    EXPECT_TRUE(vscp_getGuidFromStringToArray(guid, strGUID));
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(i, guid[i]);
    }
}

TEST(VscpHelper, getGuidFromStringToArray_Empty)
{
    uint8_t guid[16];
    memset(guid, 0xFF, 16);  // Pre-fill with non-zero
    
    // Empty string should result in all zeros
    EXPECT_TRUE(vscp_getGuidFromStringToArray(guid, ""));
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(0, guid[i]);
    }
}

TEST(VscpHelper, getGuidFromStringToArray_Dash)
{
    uint8_t guid[16];
    memset(guid, 0xFF, 16);  // Pre-fill with non-zero
    
    // "-" means all zeros
    EXPECT_TRUE(vscp_getGuidFromStringToArray(guid, "-"));
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(0, guid[i]);
    }
}

TEST(VscpHelper, getGuidFromStringToArray_LeadingZeros)
{
    uint8_t guid[16];
    
    // "-:" prefix means leading zeros with trailing values
    EXPECT_TRUE(vscp_getGuidFromStringToArray(guid, "-:01:02:03"));
    
    // First 13 bytes should be zero
    for (int i = 0; i < 13; i++) {
        EXPECT_EQ(0, guid[i]);
    }
    // Last 3 bytes should be 1, 2, 3
    EXPECT_EQ(0x01, guid[13]);
    EXPECT_EQ(0x02, guid[14]);
    EXPECT_EQ(0x03, guid[15]);
}

TEST(VscpHelper, getGuidFromStringToArray_Compact)
{
    uint8_t guid[16];
    
    // "::" prefix means leading 0xFF bytes with trailing values
    EXPECT_TRUE(vscp_getGuidFromStringToArray(guid, "::01:02:03"));
    
    // First 13 bytes should be 0xFF
    for (int i = 0; i < 13; i++) {
        EXPECT_EQ(0xFF, guid[i]);
    }
    // Last 3 bytes should be 1, 2, 3
    EXPECT_EQ(0x01, guid[13]);
    EXPECT_EQ(0x02, guid[14]);
    EXPECT_EQ(0x03, guid[15]);
}

TEST(VscpHelper, getGuidFromStringToArray_CompactAllFF)
{
    uint8_t guid[16];
    
    // "::" alone means all 0xFF
    EXPECT_TRUE(vscp_getGuidFromStringToArray(guid, "::"));
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(0xFF, guid[i]);
    }
}

TEST(VscpHelper, getGuidFromStringToArray_WithBraces)
{
    uint8_t guid[16];
    std::string strGUID = "{00:01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F}";
    
    EXPECT_TRUE(vscp_getGuidFromStringToArray(guid, strGUID));
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(i, guid[i]);
    }
}

TEST(VscpHelper, getGuidFromStringToArray_NullPointer)
{
    // Should return false for null pointer
    EXPECT_FALSE(vscp_getGuidFromStringToArray(nullptr, "00:01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F"));
}

TEST(VscpHelper, getGuidFromStringToArray_Lowercase)
{
    uint8_t guid[16];
    // Lowercase hex should work
    std::string strGUID = "aa:bb:cc:dd:ee:ff:00:11:22:33:44:55:66:77:88:99";
    
    EXPECT_TRUE(vscp_getGuidFromStringToArray(guid, strGUID));
    
    EXPECT_EQ(0xAA, guid[0]);
    EXPECT_EQ(0xBB, guid[1]);
    EXPECT_EQ(0xCC, guid[2]);
    EXPECT_EQ(0xDD, guid[3]);
    EXPECT_EQ(0xEE, guid[4]);
    EXPECT_EQ(0xFF, guid[5]);
}

TEST(VscpHelper, writeGuidArrayToString)
{
    uint8_t guid[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    std::string strGUID;
    
    EXPECT_TRUE(vscp_writeGuidArrayToString(strGUID, guid));
    EXPECT_EQ("00:01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F", strGUID);
}

TEST(VscpHelper, writeGuidArrayToString_UUID)
{
    uint8_t guid[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    std::string strGUID;
    
    EXPECT_TRUE(vscp_writeGuidArrayToString(strGUID, guid, false, VSCP_GUID_STR_FORMAT_UUID));
    EXPECT_EQ("00010203-0405-0607-0809-0A0B0C0D0E0F", strGUID);
}

TEST(VscpHelper, writeGuidArrayToString_Compact)
{
    std::string strGUID;
    
    // GUID with leading 0xFF bytes
    uint8_t guid1[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x02, 0x03};
    EXPECT_TRUE(vscp_writeGuidArrayToString(strGUID, guid1, false, VSCP_GUID_STR_FORMAT_COMPACT));
    EXPECT_EQ("::01:02:03", strGUID);
    
    // All 0xFF
    uint8_t guid2[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    EXPECT_TRUE(vscp_writeGuidArrayToString(strGUID, guid2, false, VSCP_GUID_STR_FORMAT_COMPACT));
    EXPECT_EQ("::", strGUID);
    
    // No leading 0xFF - should use normal format
    uint8_t guid3[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                         0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    EXPECT_TRUE(vscp_writeGuidArrayToString(strGUID, guid3, false, VSCP_GUID_STR_FORMAT_COMPACT));
    EXPECT_EQ("00:01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F", strGUID);
}

TEST(VscpHelper, writeGuidArrayToString_Comma)
{
    uint8_t guid[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    std::string strGUID;
    
    // bUseComma = true gives decimal comma-separated
    EXPECT_TRUE(vscp_writeGuidArrayToString(strGUID, guid, true));
    EXPECT_EQ("0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15", strGUID);
}

TEST(VscpHelper, writeGuidToString_Normal)
{
    vscpEvent ev;
    memset(&ev, 0, sizeof(ev));
    uint8_t guid[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    memcpy(ev.GUID, guid, 16);
    std::string strGUID;
    
    // Default format (normal)
    EXPECT_TRUE(vscp_writeGuidToString(strGUID, &ev));
    EXPECT_EQ("00:01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F", strGUID);
    
    // Explicit normal format
    EXPECT_TRUE(vscp_writeGuidToString(strGUID, &ev, VSCP_GUID_STR_FORMAT_NORMAL));
    EXPECT_EQ("00:01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F", strGUID);
}

TEST(VscpHelper, writeGuidToString_UUID)
{
    vscpEvent ev;
    memset(&ev, 0, sizeof(ev));
    uint8_t guid[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    memcpy(ev.GUID, guid, 16);
    std::string strGUID;
    
    EXPECT_TRUE(vscp_writeGuidToString(strGUID, &ev, VSCP_GUID_STR_FORMAT_UUID));
    EXPECT_EQ("00010203-0405-0607-0809-0A0B0C0D0E0F", strGUID);
}

TEST(VscpHelper, writeGuidToString_Compact)
{
    vscpEvent ev;
    memset(&ev, 0, sizeof(ev));
    std::string strGUID;
    
    // GUID with leading 0xFF bytes
    uint8_t guid1[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x02, 0x03};
    memcpy(ev.GUID, guid1, 16);
    EXPECT_TRUE(vscp_writeGuidToString(strGUID, &ev, VSCP_GUID_STR_FORMAT_COMPACT));
    EXPECT_EQ("::01:02:03", strGUID);
    
    // All 0xFF
    uint8_t guid2[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    memcpy(ev.GUID, guid2, 16);
    EXPECT_TRUE(vscp_writeGuidToString(strGUID, &ev, VSCP_GUID_STR_FORMAT_COMPACT));
    EXPECT_EQ("::", strGUID);
    
    // No leading 0xFF - should use normal format
    uint8_t guid3[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                         0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    memcpy(ev.GUID, guid3, 16);
    EXPECT_TRUE(vscp_writeGuidToString(strGUID, &ev, VSCP_GUID_STR_FORMAT_COMPACT));
    EXPECT_EQ("00:01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F", strGUID);
}

TEST(VscpHelper, writeGuidToStringEx_Normal)
{
    vscpEventEx ev;
    memset(&ev, 0, sizeof(ev));
    uint8_t guid[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    memcpy(ev.GUID, guid, 16);
    std::string strGUID;
    
    // Default format
    EXPECT_TRUE(vscp_writeGuidToStringEx(strGUID, &ev));
    EXPECT_EQ("00:01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F", strGUID);
}

TEST(VscpHelper, writeGuidToStringEx_UUID)
{
    vscpEventEx ev;
    memset(&ev, 0, sizeof(ev));
    uint8_t guid[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                        0x01, 0x02, 0xAA, 0xBB, 0x44, 0x01, 0x30, 0x00};
    memcpy(ev.GUID, guid, 16);
    std::string strGUID;
    
    EXPECT_TRUE(vscp_writeGuidToStringEx(strGUID, &ev, VSCP_GUID_STR_FORMAT_UUID));
    EXPECT_EQ("FFFFFFFF-FFFF-FFFF-0102-AABB44013000", strGUID);
}

TEST(VscpHelper, writeGuidToStringEx_Compact)
{
    vscpEventEx ev;
    memset(&ev, 0, sizeof(ev));
    std::string strGUID;
    
    // GUID with leading 0xFF bytes
    uint8_t guid1[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0xBB, 0xCC};
    memcpy(ev.GUID, guid1, 16);
    EXPECT_TRUE(vscp_writeGuidToStringEx(strGUID, &ev, VSCP_GUID_STR_FORMAT_COMPACT));
    EXPECT_EQ("::AA:BB:CC", strGUID);
}

TEST(VscpHelper, isGUIDEmpty)
{
    uint8_t emptyGuid[16] = {0};
    uint8_t nonEmptyGuid[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    
    EXPECT_TRUE(vscp_isGUIDEmpty(emptyGuid));
    EXPECT_FALSE(vscp_isGUIDEmpty(nonEmptyGuid));
}

TEST(VscpHelper, isSameGUID)
{
    uint8_t guid1[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    uint8_t guid2[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    uint8_t guid3[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0};
    
    EXPECT_TRUE(vscp_isSameGUID(guid1, guid2));
    EXPECT_FALSE(vscp_isSameGUID(guid1, guid3));
}

TEST(VscpHelper, reverseGUID)
{
    uint8_t guid[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    
    EXPECT_TRUE(vscp_reverseGUID(guid));
    
    EXPECT_EQ(15, guid[0]);
    EXPECT_EQ(14, guid[1]);
    EXPECT_EQ(0, guid[15]);
}

TEST(VscpHelper, calcCRC4GUIDArray)
{
    uint8_t guid[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    
    uint8_t crc = vscp_calcCRC4GUIDArray(guid);
    // CRC should be consistent for same input
    EXPECT_EQ(crc, vscp_calcCRC4GUIDArray(guid));
}

TEST(VscpHelper, calcCRC4GUIDString)
{
    std::string strGUID = "FF:FF:FF:FF:FF:FF:FF:FE:00:00:00:00:00:00:00:01";
    
    uint8_t crc = vscp_calcCRC4GUIDString(strGUID);
    // CRC should be consistent for same input
    EXPECT_EQ(crc, vscp_calcCRC4GUIDString(strGUID));
}

// =============================================================================
//                           Event Basic Operations
// =============================================================================

TEST(VscpHelper, newEvent)
{
    vscpEvent *pEvent = nullptr;
    EXPECT_TRUE(vscp_newEvent(&pEvent));
    ASSERT_NE(nullptr, pEvent);
    
    vscp_deleteEvent(pEvent);
}

TEST(VscpHelper, makeTimeStamp)
{
    unsigned long ts1 = vscp_makeTimeStamp();
    unsigned long ts2 = vscp_makeTimeStamp();
    
    // Timestamps should be non-zero and close in value
    EXPECT_GT(ts1, 0u);
    EXPECT_GE(ts2, ts1);
}

TEST(VscpHelper, getMsTimeStamp)
{
    uint32_t ts1 = vscp_getMsTimeStamp();
    uint32_t ts2 = vscp_getMsTimeStamp();
    
    EXPECT_GE(ts2, ts1);
}

TEST(VscpHelper, makeTimeStampNs)
{
    uint64_t ts1 = vscp_makeTimeStampNs();
    uint64_t ts2 = vscp_makeTimeStampNs();
    
    // Timestamps should be non-zero and close in value
    EXPECT_GT(ts1, 0ULL);
    EXPECT_GE(ts2, ts1);
    
    // Verify timestamp is in reasonable range (after year 2020, before year 2100)
    // 2020-01-01 00:00:00 UTC = 1577836800 seconds = 1577836800000000000 ns
    // 2100-01-01 00:00:00 UTC = 4102444800 seconds = 4102444800000000000 ns
    EXPECT_GT(ts1, 1577836800000000000ULL);
    EXPECT_LT(ts1, 4102444800000000000ULL);
}

// =============================================================================
//                           Event Priority
// =============================================================================

TEST(VscpHelper, setGetEventPriority)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    
    for (unsigned char p = 0; p < 8; p++) {
        vscp_setEventPriority(&event, p);
        EXPECT_EQ(p, vscp_getEventPriority(&event));
    }
}

TEST(VscpHelper, setGetEventExPriority)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    for (unsigned char p = 0; p < 8; p++) {
        vscp_setEventExPriority(&eventEx, p);
        EXPECT_EQ(p, vscp_getEventExPriority(&eventEx));
    }
}

// =============================================================================
//                         Event Conversion
// =============================================================================

TEST(VscpHelper, eventToEventEx_Empty)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    event.sizeData = 0;
    event.vscp_class = 10;
    event.vscp_type = 6;
    
    vscpEventEx eventEx;
    EXPECT_TRUE(vscp_convertEventToEventEx(&eventEx, &event));
    EXPECT_EQ(10, eventEx.vscp_class);
    EXPECT_EQ(6, eventEx.vscp_type);
    EXPECT_EQ(0, eventEx.sizeData);
}

TEST(VscpHelper, eventToEventEx_OriginalFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    event.sizeData = 0;
    
    // Set header with ORIGINAL frame version (0x0000)
    event.head = VSCP_HEADER16_FRAME_VERSION_ORIGINAL | 0x0003;  // priority bits
    event.vscp_class = 10;
    event.vscp_type = 6;
    event.year = 2026;
    event.month = 3;
    // Note: day, hour, minute, second, timestamp share a union with timestamp_ns
    event.day = 12;
    event.hour = 14;
    event.minute = 30;
    event.second = 45;
    event.timestamp = 123456789;
    event.obid = 42;
    memset(event.GUID, 0xAA, 16);
    
    vscpEventEx eventEx;
    EXPECT_TRUE(vscp_convertEventToEventEx(&eventEx, &event));
    
    EXPECT_EQ(event.head, eventEx.head);
    EXPECT_EQ(10, eventEx.vscp_class);
    EXPECT_EQ(6, eventEx.vscp_type);
    EXPECT_EQ(2026, eventEx.year);
    EXPECT_EQ(3, eventEx.month);
    EXPECT_EQ(12, eventEx.day);
    EXPECT_EQ(14, eventEx.hour);
    EXPECT_EQ(30, eventEx.minute);
    EXPECT_EQ(45, eventEx.second);
    EXPECT_EQ(123456789, eventEx.timestamp);
    EXPECT_EQ(42, eventEx.obid);
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(0xAA, eventEx.GUID[i]);
    }
}

TEST(VscpHelper, eventToEventEx_UnixNsFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = new uint8_t[3]{0x11, 0x22, 0x33};
    event.sizeData = 3;
    
    // Set header with UNIX_NS frame version (0x0100)
    event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS | 0x0007;  // priority bits
    event.vscp_class = 20;
    event.vscp_type = 10;
    // Note: timestamp_ns shares a union with day/hour/minute/second/timestamp
    event.timestamp_ns = 1234567890123456789ULL;  // Unix nanoseconds
    event.obid = 99;
    memset(event.GUID, 0xBB, 16);
    
    vscpEventEx eventEx;
    EXPECT_TRUE(vscp_convertEventToEventEx(&eventEx, &event));
    
    EXPECT_EQ(event.head, eventEx.head);
    EXPECT_EQ(20, eventEx.vscp_class);
    EXPECT_EQ(10, eventEx.vscp_type);
    EXPECT_EQ(1234567890123456789ULL, eventEx.timestamp_ns);
    // For UNIX_NS frame, year should be 0xffff and month 0xff
    EXPECT_EQ(0xffff, eventEx.year);
    EXPECT_EQ(0xff, eventEx.month);
    EXPECT_EQ(99, eventEx.obid);
    EXPECT_EQ(3, eventEx.sizeData);
    EXPECT_EQ(0x11, eventEx.data[0]);
    EXPECT_EQ(0x22, eventEx.data[1]);
    EXPECT_EQ(0x33, eventEx.data[2]);
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(0xBB, eventEx.GUID[i]);
    }
    
    delete[] event.pdata;
}

TEST(VscpHelper, eventExToEvent)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    eventEx.vscp_class = 20;
    eventEx.vscp_type = 3;
    eventEx.sizeData = 3;
    eventEx.data[0] = 1;
    eventEx.data[1] = 2;
    eventEx.data[2] = 3;
    
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    
    EXPECT_TRUE(vscp_convertEventExToEvent(&event, &eventEx));
    EXPECT_EQ(20, event.vscp_class);
    EXPECT_EQ(3, event.vscp_type);
    EXPECT_EQ(3, event.sizeData);
    ASSERT_NE(nullptr, event.pdata);
    EXPECT_EQ(1, event.pdata[0]);
    EXPECT_EQ(2, event.pdata[1]);
    EXPECT_EQ(3, event.pdata[2]);
    
    vscp_deleteEvent(&event);
}

TEST(VscpHelper, eventExToEvent_OriginalFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    // Set header with ORIGINAL frame version (0x0000)
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_ORIGINAL | 0x0003;  // priority bits
    eventEx.vscp_class = 10;
    eventEx.vscp_type = 6;
    eventEx.year = 2026;
    eventEx.month = 3;
    // Note: In vscpEventEx, day/hour/minute/second/timestamp share union with timestamp_ns AND crc
    eventEx.day = 12;
    eventEx.hour = 14;
    eventEx.minute = 30;
    eventEx.second = 45;
    eventEx.timestamp = 123456789;
    eventEx.obid = 42;
    eventEx.sizeData = 2;
    eventEx.data[0] = 0xAA;
    eventEx.data[1] = 0xBB;
    memset(eventEx.GUID, 0xCC, 16);
    
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    
    EXPECT_TRUE(vscp_convertEventExToEvent(&event, &eventEx));
    
    EXPECT_EQ(eventEx.head, event.head);
    EXPECT_EQ(10, event.vscp_class);
    EXPECT_EQ(6, event.vscp_type);
    EXPECT_EQ(2026, event.year);
    EXPECT_EQ(3, event.month);
    EXPECT_EQ(12, event.day);
    EXPECT_EQ(14, event.hour);
    EXPECT_EQ(30, event.minute);
    EXPECT_EQ(45, event.second);
    EXPECT_EQ(123456789, event.timestamp);
    EXPECT_EQ(42, event.obid);
    EXPECT_EQ(2, event.sizeData);
    ASSERT_NE(nullptr, event.pdata);
    EXPECT_EQ(0xAA, event.pdata[0]);
    EXPECT_EQ(0xBB, event.pdata[1]);
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(0xCC, event.GUID[i]);
    }
    
    vscp_deleteEvent(&event);
}

TEST(VscpHelper, eventExToEvent_UnixNsFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    // Set header with UNIX_NS frame version (0x0100)
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS | 0x0007;  // priority bits
    eventEx.vscp_class = 20;
    eventEx.vscp_type = 10;
    // Note: In vscpEventEx, timestamp_ns shares union with day/hour/minute/second/timestamp AND crc
    eventEx.timestamp_ns = 1234567890123456789ULL;  // Unix nanoseconds
    eventEx.obid = 99;
    eventEx.sizeData = 4;
    eventEx.data[0] = 0x11;
    eventEx.data[1] = 0x22;
    eventEx.data[2] = 0x33;
    eventEx.data[3] = 0x44;
    memset(eventEx.GUID, 0xDD, 16);
    
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    
    EXPECT_TRUE(vscp_convertEventExToEvent(&event, &eventEx));
    
    EXPECT_EQ(eventEx.head, event.head);
    EXPECT_EQ(20, event.vscp_class);
    EXPECT_EQ(10, event.vscp_type);
    EXPECT_EQ(1234567890123456789ULL, event.timestamp_ns);
    // For UNIX_NS frame, year should be 0xffff and month 0xff
    EXPECT_EQ(0xffff, event.year);
    EXPECT_EQ(0xff, event.month);
    EXPECT_EQ(99, event.obid);
    EXPECT_EQ(4, event.sizeData);
    ASSERT_NE(nullptr, event.pdata);
    EXPECT_EQ(0x11, event.pdata[0]);
    EXPECT_EQ(0x22, event.pdata[1]);
    EXPECT_EQ(0x33, event.pdata[2]);
    EXPECT_EQ(0x44, event.pdata[3]);
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(0xDD, event.GUID[i]);
    }
    
    vscp_deleteEvent(&event);
}

TEST(VscpHelper, copyEvent)
{
    vscpEvent eventFrom;
    memset(&eventFrom, 0, sizeof(eventFrom));
    eventFrom.vscp_class = 10;
    eventFrom.vscp_type = 6;
    eventFrom.sizeData = 3;
    eventFrom.pdata = new uint8_t[3]{1, 2, 3};
    
    vscpEvent eventTo;
    memset(&eventTo, 0, sizeof(eventTo));
    eventTo.pdata = nullptr;
    
    EXPECT_TRUE(vscp_copyEvent(&eventTo, &eventFrom));
    EXPECT_EQ(10, eventTo.vscp_class);
    EXPECT_EQ(6, eventTo.vscp_type);
    EXPECT_EQ(3, eventTo.sizeData);
    ASSERT_NE(nullptr, eventTo.pdata);
    EXPECT_EQ(1, eventTo.pdata[0]);
    EXPECT_EQ(2, eventTo.pdata[1]);
    EXPECT_EQ(3, eventTo.pdata[2]);
    
    delete[] eventFrom.pdata;
    delete[] eventTo.pdata;
}

TEST(VscpHelper, copyEventEx)
{
    vscpEventEx eventFrom;
    memset(&eventFrom, 0, sizeof(eventFrom));
    eventFrom.vscp_class = 10;
    eventFrom.vscp_type = 6;
    eventFrom.sizeData = 3;
    eventFrom.data[0] = 1;
    eventFrom.data[1] = 2;
    eventFrom.data[2] = 3;
    
    vscpEventEx eventTo;
    EXPECT_TRUE(vscp_copyEventEx(&eventTo, &eventFrom));
    EXPECT_EQ(10, eventTo.vscp_class);
    EXPECT_EQ(6, eventTo.vscp_type);
    EXPECT_EQ(3, eventTo.sizeData);
    EXPECT_EQ(1, eventTo.data[0]);
    EXPECT_EQ(2, eventTo.data[1]);
    EXPECT_EQ(3, eventTo.data[2]);
}

TEST(VscpHelper, copyEvent_OriginalFrame)
{
    vscpEvent eventFrom;
    memset(&eventFrom, 0, sizeof(eventFrom));
    
    // Set header with ORIGINAL frame version (0x0000)
    eventFrom.head = VSCP_HEADER16_FRAME_VERSION_ORIGINAL | 0x0005;  // priority bits
    eventFrom.vscp_class = 10;
    eventFrom.vscp_type = 6;
    eventFrom.year = 2026;
    eventFrom.month = 3;
    eventFrom.day = 12;
    eventFrom.hour = 14;
    eventFrom.minute = 30;
    eventFrom.second = 45;
    eventFrom.timestamp = 123456789;
    eventFrom.obid = 42;
    eventFrom.sizeData = 3;
    eventFrom.pdata = new uint8_t[3]{0xAA, 0xBB, 0xCC};
    memset(eventFrom.GUID, 0xDD, 16);
    
    vscpEvent eventTo;
    memset(&eventTo, 0, sizeof(eventTo));
    eventTo.pdata = nullptr;
    
    EXPECT_TRUE(vscp_copyEvent(&eventTo, &eventFrom));
    
    EXPECT_EQ(eventFrom.head, eventTo.head);
    EXPECT_EQ(10, eventTo.vscp_class);
    EXPECT_EQ(6, eventTo.vscp_type);
    EXPECT_EQ(2026, eventTo.year);
    EXPECT_EQ(3, eventTo.month);
    EXPECT_EQ(12, eventTo.day);
    EXPECT_EQ(14, eventTo.hour);
    EXPECT_EQ(30, eventTo.minute);
    EXPECT_EQ(45, eventTo.second);
    EXPECT_EQ(123456789, eventTo.timestamp);
    EXPECT_EQ(42, eventTo.obid);
    EXPECT_EQ(3, eventTo.sizeData);
    ASSERT_NE(nullptr, eventTo.pdata);
    EXPECT_EQ(0xAA, eventTo.pdata[0]);
    EXPECT_EQ(0xBB, eventTo.pdata[1]);
    EXPECT_EQ(0xCC, eventTo.pdata[2]);
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(0xDD, eventTo.GUID[i]);
    }
    
    delete[] eventFrom.pdata;
    delete[] eventTo.pdata;
}

TEST(VscpHelper, copyEvent_UnixNsFrame)
{
    vscpEvent eventFrom;
    memset(&eventFrom, 0, sizeof(eventFrom));
    
    // Set header with UNIX_NS frame version (0x0100)
    eventFrom.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS | 0x0007;  // priority bits
    eventFrom.vscp_class = 20;
    eventFrom.vscp_type = 10;
    eventFrom.year = 0xffff;  // Not used in UNIX_NS
    eventFrom.month = 0xff;   // Not used in UNIX_NS
    eventFrom.timestamp_ns = 1234567890123456789ULL;  // Unix nanoseconds
    eventFrom.obid = 99;
    eventFrom.sizeData = 4;
    eventFrom.pdata = new uint8_t[4]{0x11, 0x22, 0x33, 0x44};
    memset(eventFrom.GUID, 0xEE, 16);
    
    vscpEvent eventTo;
    memset(&eventTo, 0, sizeof(eventTo));
    eventTo.pdata = nullptr;
    
    EXPECT_TRUE(vscp_copyEvent(&eventTo, &eventFrom));
    
    EXPECT_EQ(eventFrom.head, eventTo.head);
    EXPECT_EQ(20, eventTo.vscp_class);
    EXPECT_EQ(10, eventTo.vscp_type);
    EXPECT_EQ(1234567890123456789ULL, eventTo.timestamp_ns);
    EXPECT_EQ(0xffff, eventTo.year);
    EXPECT_EQ(0xff, eventTo.month);
    EXPECT_EQ(99, eventTo.obid);
    EXPECT_EQ(4, eventTo.sizeData);
    ASSERT_NE(nullptr, eventTo.pdata);
    EXPECT_EQ(0x11, eventTo.pdata[0]);
    EXPECT_EQ(0x22, eventTo.pdata[1]);
    EXPECT_EQ(0x33, eventTo.pdata[2]);
    EXPECT_EQ(0x44, eventTo.pdata[3]);
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(0xEE, eventTo.GUID[i]);
    }
    
    delete[] eventFrom.pdata;
    delete[] eventTo.pdata;
}

TEST(VscpHelper, copyEventEx_OriginalFrame)
{
    vscpEventEx eventFrom;
    memset(&eventFrom, 0, sizeof(eventFrom));
    
    // Set header with ORIGINAL frame version (0x0000)
    eventFrom.head = VSCP_HEADER16_FRAME_VERSION_ORIGINAL | 0x0003;  // priority bits
    eventFrom.vscp_class = 10;
    eventFrom.vscp_type = 6;
    eventFrom.year = 2026;
    eventFrom.month = 3;
    // Note: In vscpEventEx, day/hour/minute/second/timestamp share union with timestamp_ns AND crc
    eventFrom.day = 12;
    eventFrom.hour = 14;
    eventFrom.minute = 30;
    eventFrom.second = 45;
    eventFrom.timestamp = 123456789;
    eventFrom.obid = 42;
    eventFrom.sizeData = 3;
    eventFrom.data[0] = 0xAA;
    eventFrom.data[1] = 0xBB;
    eventFrom.data[2] = 0xCC;
    memset(eventFrom.GUID, 0xDD, 16);
    
    vscpEventEx eventTo;
    memset(&eventTo, 0, sizeof(eventTo));
    
    EXPECT_TRUE(vscp_copyEventEx(&eventTo, &eventFrom));
    
    EXPECT_EQ(eventFrom.head, eventTo.head);
    EXPECT_EQ(10, eventTo.vscp_class);
    EXPECT_EQ(6, eventTo.vscp_type);
    EXPECT_EQ(2026, eventTo.year);
    EXPECT_EQ(3, eventTo.month);
    EXPECT_EQ(12, eventTo.day);
    EXPECT_EQ(14, eventTo.hour);
    EXPECT_EQ(30, eventTo.minute);
    EXPECT_EQ(45, eventTo.second);
    EXPECT_EQ(123456789, eventTo.timestamp);
    EXPECT_EQ(42, eventTo.obid);
    EXPECT_EQ(3, eventTo.sizeData);
    EXPECT_EQ(0xAA, eventTo.data[0]);
    EXPECT_EQ(0xBB, eventTo.data[1]);
    EXPECT_EQ(0xCC, eventTo.data[2]);
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(0xDD, eventTo.GUID[i]);
    }
}

TEST(VscpHelper, copyEventEx_UnixNsFrame)
{
    vscpEventEx eventFrom;
    memset(&eventFrom, 0, sizeof(eventFrom));
    
    // Set header with UNIX_NS frame version (0x0100)
    eventFrom.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS | 0x0007;  // priority bits
    eventFrom.vscp_class = 20;
    eventFrom.vscp_type = 10;
    // Note: In vscpEventEx, timestamp_ns shares union with day/hour/minute/second/timestamp AND crc
    eventFrom.timestamp_ns = 1234567890123456789ULL;  // Unix nanoseconds
    eventFrom.obid = 99;
    eventFrom.sizeData = 4;
    eventFrom.data[0] = 0x11;
    eventFrom.data[1] = 0x22;
    eventFrom.data[2] = 0x33;
    eventFrom.data[3] = 0x44;
    memset(eventFrom.GUID, 0xEE, 16);
    
    vscpEventEx eventTo;
    memset(&eventTo, 0, sizeof(eventTo));
    
    EXPECT_TRUE(vscp_copyEventEx(&eventTo, &eventFrom));
    
    EXPECT_EQ(eventFrom.head, eventTo.head);
    EXPECT_EQ(20, eventTo.vscp_class);
    EXPECT_EQ(10, eventTo.vscp_type);
    EXPECT_EQ(1234567890123456789ULL, eventTo.timestamp_ns);
    EXPECT_EQ(99, eventTo.obid);
    EXPECT_EQ(4, eventTo.sizeData);
    EXPECT_EQ(0x11, eventTo.data[0]);
    EXPECT_EQ(0x22, eventTo.data[1]);
    EXPECT_EQ(0x33, eventTo.data[2]);
    EXPECT_EQ(0x44, eventTo.data[3]);
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(0xEE, eventTo.GUID[i]);
    }
}

// =============================================================================
//                         Event String Conversion
// =============================================================================

TEST(VscpHelper, getDateStringFromEvent_OriginalFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    
    // Set header with ORIGINAL frame version (0x0000)
    event.head = VSCP_HEADER16_FRAME_VERSION_ORIGINAL;
    event.year = 2026;
    event.month = 3;
    event.day = 12;
    event.hour = 14;
    event.minute = 30;
    event.second = 45;
    
    std::string dt;
    EXPECT_TRUE(vscp_getDateStringFromEvent(dt, &event));
    EXPECT_EQ("2026-03-12T14:30:45Z", dt);
}

TEST(VscpHelper, getDateStringFromEvent_UnixNsFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    
    // Set header with UNIX_NS frame version (0x0100)
    event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
    // 2026-03-12T14:30:45.123456789Z in nanoseconds since epoch
    // Using a known timestamp: 1773509445123456789 (approx 2026-03-12)
    event.timestamp_ns = 1773509445123456789ULL;
    
    std::string dt;
    EXPECT_TRUE(vscp_getDateStringFromEvent(dt, &event));
    // Should contain nanoseconds
    EXPECT_FALSE(dt.empty());
    EXPECT_NE(std::string::npos, dt.find("."));  // Should have decimal point for nanos
    EXPECT_NE(std::string::npos, dt.find("2026"));  // Should be year 2026
}

TEST(VscpHelper, getDateStringFromEvent_OriginalFrame_Empty)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    
    // Set header with ORIGINAL frame version (0x0000) but all zeros
    event.head = VSCP_HEADER16_FRAME_VERSION_ORIGINAL;
    event.year = 0;
    event.month = 0;
    event.day = 0;
    event.hour = 0;
    event.minute = 0;
    event.second = 0;
    
    std::string dt;
    EXPECT_TRUE(vscp_getDateStringFromEvent(dt, &event));
    EXPECT_TRUE(dt.empty());  // Should return empty string when all zeros
}

TEST(VscpHelper, getDateStringFromEventEx_OriginalFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    // Set header with ORIGINAL frame version (0x0000)
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_ORIGINAL;
    eventEx.year = 2026;
    eventEx.month = 3;
    eventEx.day = 12;
    eventEx.hour = 14;
    eventEx.minute = 30;
    eventEx.second = 45;
    
    std::string dt;
    EXPECT_TRUE(vscp_getDateStringFromEventEx(dt, &eventEx));
    EXPECT_EQ("2026-03-12T14:30:45Z", dt);
}

TEST(VscpHelper, getDateStringFromEventEx_UnixNsFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    // Set header with UNIX_NS frame version (0x0100)
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
    // Using a known timestamp (approx 2026-03-12)
    eventEx.timestamp_ns = 1773509445123456789ULL;
    
    std::string dt;
    EXPECT_TRUE(vscp_getDateStringFromEventEx(dt, &eventEx));
    // Should contain nanoseconds
    EXPECT_FALSE(dt.empty());
    EXPECT_NE(std::string::npos, dt.find("."));  // Should have decimal point for nanos
    EXPECT_NE(std::string::npos, dt.find("2026"));  // Should be year 2026
}

TEST(VscpHelper, getDateStringFromEventEx_OriginalFrame_Empty)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    // Set header with ORIGINAL frame version (0x0000) but all zeros
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_ORIGINAL;
    eventEx.year = 0;
    eventEx.month = 0;
    // Note: day/hour/minute/second share union with timestamp_ns
    eventEx.day = 0;
    eventEx.hour = 0;
    eventEx.minute = 0;
    eventEx.second = 0;
    
    std::string dt;
    EXPECT_TRUE(vscp_getDateStringFromEventEx(dt, &eventEx));
    EXPECT_TRUE(dt.empty());  // Should return empty string when all zeros
}

TEST(VscpHelper, convertEventToString)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = 0;
    event.vscp_class = 10;
    event.vscp_type = 6;
    event.obid = 0;
    event.timestamp = 0;
    memset(event.GUID, 0, 16);
    event.sizeData = 3;
    event.pdata = new uint8_t[3]{0x01, 0x02, 0x03};
    
    std::string str;
    EXPECT_TRUE(vscp_convertEventToString(str, &event));
    EXPECT_FALSE(str.empty());
    
    delete[] event.pdata;
}

TEST(VscpHelper, convertStringToEvent)
{
    // Format: head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....
    std::string str = "0,10,6,0,,0,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00,0x01,0x02,0x03";
    
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    
    EXPECT_TRUE(vscp_convertStringToEvent(&event, str));
    EXPECT_EQ(10, event.vscp_class);
    EXPECT_EQ(6, event.vscp_type);
    EXPECT_EQ(3, event.sizeData);
    ASSERT_NE(nullptr, event.pdata);
    EXPECT_EQ(1, event.pdata[0]);
    EXPECT_EQ(2, event.pdata[1]);
    EXPECT_EQ(3, event.pdata[2]);
    
    vscp_deleteEvent(&event);
}

TEST(VscpHelper, convertEventExToString)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    eventEx.vscp_class = 10;
    eventEx.vscp_type = 6;
    eventEx.sizeData = 3;
    eventEx.data[0] = 1;
    eventEx.data[1] = 2;
    eventEx.data[2] = 3;
    
    std::string str;
    EXPECT_TRUE(vscp_convertEventExToString(str, &eventEx));
    EXPECT_FALSE(str.empty());
}

TEST(VscpHelper, convertStringToEventEx)
{
    // Format: head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....
    std::string str = "0,10,6,0,,0,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00,0x01,0x02,0x03";
    
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    EXPECT_TRUE(vscp_convertStringToEventEx(&eventEx, str));
    EXPECT_EQ(10, eventEx.vscp_class);
    EXPECT_EQ(6, eventEx.vscp_type);
    EXPECT_EQ(3, eventEx.sizeData);
    EXPECT_EQ(1, eventEx.data[0]);
    EXPECT_EQ(2, eventEx.data[1]);
    EXPECT_EQ(3, eventEx.data[2]);
}

TEST(VscpHelper, convertEventToJSON_OriginalFrame)
{
    // Even with original frame format, JSON output always uses timestamp_ns
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    
    event.head = VSCP_HEADER16_FRAME_VERSION_ORIGINAL | 0x0003;
    event.vscp_class = 10;
    event.vscp_type = 6;
    event.obid = 42;
    event.year = 2026;
    event.month = 3;
    event.day = 12;
    event.hour = 14;
    event.minute = 30;
    event.second = 45;
    event.timestamp = 123456789;  // microseconds
    memset(event.GUID, 0xAA, 16);
    event.sizeData = 2;
    event.pdata = new uint8_t[2]{0x11, 0x22};
    
    std::string strJSON;
    EXPECT_TRUE(vscp_convertEventToJSON(strJSON, &event));
    EXPECT_FALSE(strJSON.empty());
    
    // Should contain timestamp_ns in hex format (converted from date/time)
    // 2026-03-12 14:30:45 UTC = 1773506445 seconds since epoch
    // Plus 123456789 microseconds = 123456789000 nanoseconds
    // Total: 1773506445 * 1e9 + 123456789 * 1e3 = 1773506445123456789000 ns
    // But we just check it has timestamp_ns format
    EXPECT_NE(std::string::npos, strJSON.find("\"timestamp_ns\": \"0x"));
    EXPECT_NE(std::string::npos, strJSON.find("\"class\": 10"));
    EXPECT_NE(std::string::npos, strJSON.find("\"type\": 6"));
    // Head should have UNIX_NS frame version bit set
    uint16_t expectedHead = (0x0003 & ~VSCP_HEADER16_FRAME_VERSION_MASK) | VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
    std::string headStr = "\"head\": " + std::to_string(expectedHead);
    EXPECT_NE(std::string::npos, strJSON.find(headStr));
    
    delete[] event.pdata;
}

TEST(VscpHelper, convertEventToJSON_UnixNsFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    
    event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS | 0x0007;
    event.vscp_class = 20;
    event.vscp_type = 10;
    event.obid = 99;
    event.timestamp_ns = 1234567890123456789ULL;
    memset(event.GUID, 0xBB, 16);
    event.sizeData = 3;
    event.pdata = new uint8_t[3]{0x33, 0x44, 0x55};
    
    std::string strJSON;
    EXPECT_TRUE(vscp_convertEventToJSON(strJSON, &event));
    EXPECT_FALSE(strJSON.empty());
    
    // Should contain 64-bit nanosecond timestamp in hex format
    // 1234567890123456789 = 0x112210f47de98115
    EXPECT_NE(std::string::npos, strJSON.find("\"timestamp_ns\": \"0x112210f47de98115\""));
    EXPECT_NE(std::string::npos, strJSON.find("\"class\": 20"));
    EXPECT_NE(std::string::npos, strJSON.find("\"type\": 10"));
    
    delete[] event.pdata;
}

TEST(VscpHelper, convertEventExToJSON_OriginalFrame)
{
    // Even with original frame format, JSON output always uses timestamp_ns
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_ORIGINAL | 0x0003;
    eventEx.vscp_class = 10;
    eventEx.vscp_type = 6;
    eventEx.obid = 42;
    eventEx.year = 2026;
    eventEx.month = 3;
    eventEx.day = 12;
    eventEx.hour = 14;
    eventEx.minute = 30;
    eventEx.second = 45;
    eventEx.timestamp = 123456789;  // microseconds
    memset(eventEx.GUID, 0xAA, 16);
    eventEx.sizeData = 2;
    eventEx.data[0] = 0x11;
    eventEx.data[1] = 0x22;
    
    std::string strJSON;
    EXPECT_TRUE(vscp_convertEventExToJSON(strJSON, &eventEx));
    EXPECT_FALSE(strJSON.empty());
    
    // Should contain timestamp_ns in hex format (converted from date/time)
    EXPECT_NE(std::string::npos, strJSON.find("\"timestamp_ns\": \"0x"));
    EXPECT_NE(std::string::npos, strJSON.find("\"class\": 10"));
    EXPECT_NE(std::string::npos, strJSON.find("\"type\": 6"));
    // Head should have UNIX_NS frame version bit set
    uint16_t expectedHead = (0x0003 & ~VSCP_HEADER16_FRAME_VERSION_MASK) | VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
    std::string headStr = "\"head\": " + std::to_string(expectedHead);
    EXPECT_NE(std::string::npos, strJSON.find(headStr));
}

TEST(VscpHelper, convertEventExToJSON_UnixNsFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS | 0x0007;
    eventEx.vscp_class = 20;
    eventEx.vscp_type = 10;
    eventEx.obid = 99;
    eventEx.timestamp_ns = 1234567890123456789ULL;
    memset(eventEx.GUID, 0xBB, 16);
    eventEx.sizeData = 3;
    eventEx.data[0] = 0x33;
    eventEx.data[1] = 0x44;
    eventEx.data[2] = 0x55;
    
    std::string strJSON;
    EXPECT_TRUE(vscp_convertEventExToJSON(strJSON, &eventEx));
    EXPECT_FALSE(strJSON.empty());
    
    // Should contain 64-bit nanosecond timestamp in hex format
    // 1234567890123456789 = 0x112210f47de98115
    EXPECT_NE(std::string::npos, strJSON.find("\"timestamp_ns\": \"0x112210f47de98115\""));
    EXPECT_NE(std::string::npos, strJSON.find("\"class\": 20"));
    EXPECT_NE(std::string::npos, strJSON.find("\"type\": 10"));
}

TEST(VscpHelper, convertJSONToEvent_OriginalFrame)
{
    // Even with old-format JSON (datetime + timestamp), output is always frame type 1
    std::string strJSON = R"({
        "head": 3,
        "obid": 42,
        "datetime": "2026-03-12T14:30:45Z",
        "timestamp": 123456789,
        "class": 10,
        "type": 6,
        "guid": "AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA",
        "data": [17, 34]
    })";
    
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    
    EXPECT_TRUE(vscp_convertJSONToEvent(&event, strJSON));
    
    // Always outputs UNIX_NS frame version - head should have UNIX_NS bit set
    uint16_t expectedHead = (3 & ~VSCP_HEADER16_FRAME_VERSION_MASK) | VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
    EXPECT_EQ(expectedHead, event.head);
    EXPECT_EQ(42, event.obid);
    // Verify timestamp_ns is populated (converted from datetime + timestamp)
    // Should be a reasonable 2026 epoch value (around 1.77e18 nanoseconds)
    EXPECT_GT(event.timestamp_ns, 1700000000000000000ULL);  // After 2023
    EXPECT_LT(event.timestamp_ns, 2000000000000000000ULL);  // Before 2033
    EXPECT_EQ(0xffff, event.year);  // UNIX_NS marker
    EXPECT_EQ(0xff, event.month);   // UNIX_NS marker
    EXPECT_EQ(10, event.vscp_class);
    EXPECT_EQ(6, event.vscp_type);
    EXPECT_EQ(2, event.sizeData);
    ASSERT_NE(nullptr, event.pdata);
    EXPECT_EQ(0x11, event.pdata[0]);
    EXPECT_EQ(0x22, event.pdata[1]);
    
    vscp_deleteEvent(&event);
}

TEST(VscpHelper, convertJSONToEvent_UnixNsFrame)
{
    // Head 0x0107 = VSCP_HEADER16_FRAME_VERSION_UNIX_NS (0x0100) | priority bits (0x0007)
    std::string strJSON = R"({
        "head": 263,
        "obid": 99,
        "timestamp": 1234567890123456789,
        "class": 20,
        "type": 10,
        "guid": "BB:BB:BB:BB:BB:BB:BB:BB:BB:BB:BB:BB:BB:BB:BB:BB",
        "data": [51, 68, 85]
    })";
    
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    
    EXPECT_TRUE(vscp_convertJSONToEvent(&event, strJSON));
    
    // UNIX_NS frame version (0x0100) = 256 + priority 7 = 263
    EXPECT_EQ(263, event.head);
    EXPECT_EQ(99, event.obid);
    EXPECT_EQ(1234567890123456789ULL, event.timestamp_ns);
    EXPECT_EQ(0xffff, event.year);  // Should be set to 0xffff for UNIX_NS
    EXPECT_EQ(0xff, event.month);   // Should be set to 0xff for UNIX_NS
    EXPECT_EQ(20, event.vscp_class);
    EXPECT_EQ(10, event.vscp_type);
    EXPECT_EQ(3, event.sizeData);
    ASSERT_NE(nullptr, event.pdata);
    EXPECT_EQ(0x33, event.pdata[0]);
    EXPECT_EQ(0x44, event.pdata[1]);
    EXPECT_EQ(0x55, event.pdata[2]);
    
    vscp_deleteEvent(&event);
}

TEST(VscpHelper, convertJSONToEvent_LegacyVscpKeys)
{
    std::string strJSON = R"({
        "vscpHead": 263,
        "obid": 77,
        "vscpDateTime": "2026-03-12T14:30:45Z",
        "vscpTimeStamp": 123456,
        "vscpClass": 1040,
        "vscpType": 6,
        "vscpGuid": "00:00:00:00:00:00:00:00:00:00:00:00:00:01:00:02",
        "vscpData": [1, 2, 3, 4]
    })";

    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;

    EXPECT_TRUE(vscp_convertJSONToEvent(&event, strJSON));

    EXPECT_EQ(263, event.head);
    EXPECT_EQ(77, event.obid);
    EXPECT_GT(event.timestamp_ns, 1700000000000000000ULL);
    EXPECT_LT(event.timestamp_ns, 2000000000000000000ULL);
    EXPECT_EQ(0xffff, event.year);
    EXPECT_EQ(0xff, event.month);
    EXPECT_EQ(1040, event.vscp_class);
    EXPECT_EQ(6, event.vscp_type);
    EXPECT_EQ(4, event.sizeData);
    ASSERT_NE(nullptr, event.pdata);
    EXPECT_EQ(0x01, event.pdata[0]);
    EXPECT_EQ(0x02, event.pdata[1]);
    EXPECT_EQ(0x03, event.pdata[2]);
    EXPECT_EQ(0x04, event.pdata[3]);

    vscp_deleteEvent(&event);
}

TEST(VscpHelper, convertJSONToEventEx_OriginalFrame)
{
    // Even with old-format JSON (datetime + timestamp), output is always frame type 1
    std::string strJSON = R"({
        "head": 3,
        "obid": 42,
        "datetime": "2026-03-12T14:30:45Z",
        "timestamp": 123456789,
        "class": 10,
        "type": 6,
        "guid": "AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA:AA",
        "data": [17, 34]
    })";
    
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    EXPECT_TRUE(vscp_convertJSONToEventEx(&eventEx, strJSON));
    
    // Always outputs UNIX_NS frame version - head should have UNIX_NS bit set
    uint16_t expectedHead = (3 & ~VSCP_HEADER16_FRAME_VERSION_MASK) | VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
    EXPECT_EQ(expectedHead, eventEx.head);
    EXPECT_EQ(42, eventEx.obid);
    // Verify timestamp_ns is populated (converted from datetime + timestamp)
    // Should be a reasonable 2026 epoch value (around 1.77e18 nanoseconds)
    EXPECT_GT(eventEx.timestamp_ns, 1700000000000000000ULL);  // After 2023
    EXPECT_LT(eventEx.timestamp_ns, 2000000000000000000ULL);  // Before 2033
    EXPECT_EQ(0xffff, eventEx.year);  // UNIX_NS marker
    EXPECT_EQ(0xff, eventEx.month);   // UNIX_NS marker
    EXPECT_EQ(10, eventEx.vscp_class);
    EXPECT_EQ(6, eventEx.vscp_type);
    EXPECT_EQ(2, eventEx.sizeData);
    EXPECT_EQ(0x11, eventEx.data[0]);
    EXPECT_EQ(0x22, eventEx.data[1]);
}

TEST(VscpHelper, convertJSONToEventEx_UnixNsFrame)
{
    // Head 0x0107 = VSCP_HEADER16_FRAME_VERSION_UNIX_NS (0x0100) | priority bits (0x0007)
    std::string strJSON = R"({
        "head": 263,
        "obid": 99,
        "timestamp": 1234567890123456789,
        "class": 20,
        "type": 10,
        "guid": "BB:BB:BB:BB:BB:BB:BB:BB:BB:BB:BB:BB:BB:BB:BB:BB",
        "data": [51, 68, 85]
    })";
    
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    EXPECT_TRUE(vscp_convertJSONToEventEx(&eventEx, strJSON));
    
    // UNIX_NS frame version (0x0100) = 256 + priority 7 = 263
    EXPECT_EQ(263, eventEx.head);
    EXPECT_EQ(99, eventEx.obid);
    EXPECT_EQ(1234567890123456789ULL, eventEx.timestamp_ns);
    EXPECT_EQ(0xffff, eventEx.year);  // Should be set to 0xffff for UNIX_NS
    EXPECT_EQ(0xff, eventEx.month);   // Should be set to 0xff for UNIX_NS
    EXPECT_EQ(20, eventEx.vscp_class);
    EXPECT_EQ(10, eventEx.vscp_type);
    EXPECT_EQ(3, eventEx.sizeData);
    EXPECT_EQ(0x33, eventEx.data[0]);
    EXPECT_EQ(0x44, eventEx.data[1]);
    EXPECT_EQ(0x55, eventEx.data[2]);
}

TEST(VscpHelper, convertJSONToEventEx_LegacyVscpKeys)
{
    std::string strJSON = R"({
        "vscpHead": 263,
        "obid": 88,
        "vscpDateTime": "2026-03-12T14:30:45Z",
        "vscpTimeStamp": 654321,
        "vscpClass": 20,
        "vscpType": 10,
        "vscpGuid": "00:00:00:00:00:00:00:00:00:00:00:00:00:01:00:02",
        "vscpData": [51, 68, 85]
    })";

    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));

    EXPECT_TRUE(vscp_convertJSONToEventEx(&eventEx, strJSON));

    EXPECT_EQ(263, eventEx.head);
    EXPECT_EQ(88, eventEx.obid);
    EXPECT_GT(eventEx.timestamp_ns, 1700000000000000000ULL);
    EXPECT_LT(eventEx.timestamp_ns, 2000000000000000000ULL);
    EXPECT_EQ(0xffff, eventEx.year);
    EXPECT_EQ(0xff, eventEx.month);
    EXPECT_EQ(20, eventEx.vscp_class);
    EXPECT_EQ(10, eventEx.vscp_type);
    EXPECT_EQ(3, eventEx.sizeData);
    EXPECT_EQ(0x33, eventEx.data[0]);
    EXPECT_EQ(0x44, eventEx.data[1]);
    EXPECT_EQ(0x55, eventEx.data[2]);
}

TEST(VscpHelper, convertEventToXML_OriginalFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    
    event.head = VSCP_HEADER16_FRAME_VERSION_ORIGINAL | 0x0003;
    event.vscp_class = 10;
    event.vscp_type = 6;
    event.obid = 42;
    event.year = 2026;
    event.month = 3;
    event.day = 12;
    event.hour = 14;
    event.minute = 30;
    event.second = 45;
    event.timestamp = 123456789;  // microseconds
    memset(event.GUID, 0xAA, 16);
    event.sizeData = 2;
    event.pdata = new uint8_t[2]{0x11, 0x22};
    
    std::string strXML;
    EXPECT_TRUE(vscp_convertEventToXML(strXML, &event));
    EXPECT_FALSE(strXML.empty());
    
    // Should always output frame version 1 (UNIX_NS) with nanosecond timestamp
    // head should have UNIX_NS bit set: (0x0003 & ~0x0F00) | 0x0100 = 0x0103 = 259
    EXPECT_NE(std::string::npos, strXML.find("head=\"259\""));
    EXPECT_NE(std::string::npos, strXML.find("class=\"10\""));
    EXPECT_NE(std::string::npos, strXML.find("type=\"6\""));
    // Timestamp should be nanoseconds (datetime converted + microseconds*1000)
    // The timestamp value should be large (nanosecond scale)
    EXPECT_NE(std::string::npos, strXML.find("timestamp="));
    // Verify it's not the old 32-bit value
    EXPECT_EQ(std::string::npos, strXML.find("timestamp=\"123456789\""));
    
    delete[] event.pdata;
}

TEST(VscpHelper, convertEventToXML_UnixNsFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    
    event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS | 0x0007;
    event.vscp_class = 20;
    event.vscp_type = 10;
    event.obid = 99;
    event.timestamp_ns = 1234567890123456789ULL;
    memset(event.GUID, 0xBB, 16);
    event.sizeData = 3;
    event.pdata = new uint8_t[3]{0x33, 0x44, 0x55};
    
    std::string strXML;
    EXPECT_TRUE(vscp_convertEventToXML(strXML, &event));
    EXPECT_FALSE(strXML.empty());
    
    // Should contain 64-bit nanosecond timestamp value
    EXPECT_NE(std::string::npos, strXML.find("timestamp=\"1234567890123456789\""));
    EXPECT_NE(std::string::npos, strXML.find("class=\"20\""));
    EXPECT_NE(std::string::npos, strXML.find("type=\"10\""));
    
    delete[] event.pdata;
}

TEST(VscpHelper, convertEventExToXML_OriginalFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_ORIGINAL | 0x0003;
    eventEx.vscp_class = 10;
    eventEx.vscp_type = 6;
    eventEx.obid = 42;
    eventEx.year = 2026;
    eventEx.month = 3;
    eventEx.day = 12;
    eventEx.hour = 14;
    eventEx.minute = 30;
    eventEx.second = 45;
    eventEx.timestamp = 123456789;  // microseconds
    memset(eventEx.GUID, 0xAA, 16);
    eventEx.sizeData = 2;
    eventEx.data[0] = 0x11;
    eventEx.data[1] = 0x22;
    
    std::string strXML;
    EXPECT_TRUE(vscp_convertEventExToXML(strXML, &eventEx));
    EXPECT_FALSE(strXML.empty());
    
    // Should always output frame version 1 (UNIX_NS) with nanosecond timestamp
    // head should have UNIX_NS bit set: (0x0003 & ~0x0F00) | 0x0100 = 0x0103 = 259
    EXPECT_NE(std::string::npos, strXML.find("head=\"259\""));
    EXPECT_NE(std::string::npos, strXML.find("class=\"10\""));
    EXPECT_NE(std::string::npos, strXML.find("type=\"6\""));
    // Timestamp should be nanoseconds (datetime converted + microseconds*1000)
    EXPECT_NE(std::string::npos, strXML.find("timestamp="));
    // Verify it's not the old 32-bit value
    EXPECT_EQ(std::string::npos, strXML.find("timestamp=\"123456789\""));
}

TEST(VscpHelper, convertEventExToXML_UnixNsFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS | 0x0007;
    eventEx.vscp_class = 20;
    eventEx.vscp_type = 10;
    eventEx.obid = 99;
    eventEx.timestamp_ns = 1234567890123456789ULL;
    memset(eventEx.GUID, 0xBB, 16);
    eventEx.sizeData = 3;
    eventEx.data[0] = 0x33;
    eventEx.data[1] = 0x44;
    eventEx.data[2] = 0x55;
    
    std::string strXML;
    EXPECT_TRUE(vscp_convertEventExToXML(strXML, &eventEx));
    EXPECT_FALSE(strXML.empty());
    
    // Should contain 64-bit nanosecond timestamp value
    EXPECT_NE(std::string::npos, strXML.find("timestamp=\"1234567890123456789\""));
    EXPECT_NE(std::string::npos, strXML.find("class=\"20\""));
    EXPECT_NE(std::string::npos, strXML.find("type=\"10\""));
}

// =============================================================================
//                         Event Data String Conversion
// =============================================================================

TEST(VscpHelper, setEventDataFromString)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    
    EXPECT_TRUE(vscp_setEventDataFromString(&event, "0x01,0x02,0x03,0x04"));
    EXPECT_EQ(4, event.sizeData);
    ASSERT_NE(nullptr, event.pdata);
    EXPECT_EQ(1, event.pdata[0]);
    EXPECT_EQ(2, event.pdata[1]);
    EXPECT_EQ(3, event.pdata[2]);
    EXPECT_EQ(4, event.pdata[3]);
    
    vscp_deleteEvent(&event);
}

TEST(VscpHelper, setEventExDataFromString)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    EXPECT_TRUE(vscp_setEventExDataFromString(&eventEx, "0x01,0x02,0x03,0x04"));
    EXPECT_EQ(4, eventEx.sizeData);
    EXPECT_EQ(1, eventEx.data[0]);
    EXPECT_EQ(2, eventEx.data[1]);
    EXPECT_EQ(3, eventEx.data[2]);
    EXPECT_EQ(4, eventEx.data[3]);
}

TEST(VscpHelper, writeDataWithSizeToString)
{
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    std::string str;
    
    EXPECT_TRUE(vscp_writeDataWithSizeToString(str, data, sizeof(data)));
    EXPECT_FALSE(str.empty());
}

// =============================================================================
//                         Filter Operations
// =============================================================================

TEST(VscpHelper, clearVSCPFilter)
{
    vscpEventFilter filter;
    filter.filter_priority = 7;
    filter.filter_class = 100;
    filter.filter_type = 50;
    filter.mask_priority = 7;
    filter.mask_class = 0xFFFF;
    filter.mask_type = 0xFFFF;
    
    vscp_clearVSCPFilter(&filter);
    
    EXPECT_EQ(0, filter.filter_priority);
    EXPECT_EQ(0, filter.filter_class);
    EXPECT_EQ(0, filter.filter_type);
    EXPECT_EQ(0, filter.mask_priority);
    EXPECT_EQ(0, filter.mask_class);
    EXPECT_EQ(0, filter.mask_type);
}

TEST(VscpHelper, copyVSCPFilter)
{
    vscpEventFilter filterFrom;
    filterFrom.filter_priority = 7;
    filterFrom.filter_class = 100;
    filterFrom.filter_type = 50;
    filterFrom.mask_priority = 7;
    filterFrom.mask_class = 0xFFFF;
    filterFrom.mask_type = 0xFFFF;
    memset(filterFrom.filter_GUID, 0xFF, 16);
    memset(filterFrom.mask_GUID, 0xFF, 16);
    
    vscpEventFilter filterTo;
    vscp_copyVSCPFilter(&filterTo, &filterFrom);
    
    EXPECT_EQ(filterFrom.filter_priority, filterTo.filter_priority);
    EXPECT_EQ(filterFrom.filter_class, filterTo.filter_class);
    EXPECT_EQ(filterFrom.filter_type, filterTo.filter_type);
    EXPECT_EQ(filterFrom.mask_priority, filterTo.mask_priority);
    EXPECT_EQ(filterFrom.mask_class, filterTo.mask_class);
    EXPECT_EQ(filterFrom.mask_type, filterTo.mask_type);
}

TEST(VscpHelper, doLevel2Filter_NullFilter)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    
    // Null filter should pass everything
    EXPECT_TRUE(vscp_doLevel2Filter(&event, nullptr));
}

TEST(VscpHelper, doLevel2Filter_EmptyMask)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    event.vscp_class = 100;
    event.vscp_type = 50;
    
    vscpEventFilter filter;
    vscp_clearVSCPFilter(&filter);
    
    // Empty mask (all zeros) should pass everything
    EXPECT_TRUE(vscp_doLevel2Filter(&event, &filter));
}

TEST(VscpHelper, doLevel2Filter_ClassFilter)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    event.vscp_class = 10;
    event.vscp_type = 6;
    
    vscpEventFilter filter;
    vscp_clearVSCPFilter(&filter);
    filter.filter_class = 10;
    filter.mask_class = 0xFFFF;
    
    EXPECT_TRUE(vscp_doLevel2Filter(&event, &filter));
    
    // Change event class, should fail
    event.vscp_class = 20;
    EXPECT_FALSE(vscp_doLevel2Filter(&event, &filter));
}

TEST(VscpHelper, readFilterFromString)
{
    vscpEventFilter filter;
    
    EXPECT_TRUE(vscp_readFilterFromString(&filter, "3,10,6,FF:FF:FF:FF:FF:FF:FF:FE:00:00:00:00:00:00:00:01"));
    EXPECT_EQ(3, filter.filter_priority);
    EXPECT_EQ(10, filter.filter_class);
    EXPECT_EQ(6, filter.filter_type);
}

TEST(VscpHelper, readMaskFromString)
{
    vscpEventFilter filter;
    
    EXPECT_TRUE(vscp_readMaskFromString(&filter, "7,0xFFFF,0xFFFF,FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF"));
    EXPECT_EQ(7, filter.mask_priority);
    EXPECT_EQ(0xFFFF, filter.mask_class);
    EXPECT_EQ(0xFFFF, filter.mask_type);
}

// =============================================================================
//                         CANAL ID Functions
// =============================================================================

TEST(VscpHelper, getHeadFromCANALid)
{
    uint32_t id = 0x1C000000;  // Priority 7 (high 3 bits)
    EXPECT_EQ(7, (vscp_getHeadFromCANALid(id) >> 5) & 0x07);
}

TEST(VscpHelper, getVscpClassFromCANALid)
{
    // Class is in bits 16-24 with 9-bit mask (0x1ff)
    // Example: class 0x0A (10) at position 16 = 0x000A0000
    uint32_t id = 0x000A0000;  // Class 10
    uint16_t vscp_class = vscp_getVscpClassFromCANALid(id);
    EXPECT_EQ(10, vscp_class);
    
    // Max class value (511)
    id = 0x01FF0000;
    vscp_class = vscp_getVscpClassFromCANALid(id);
    EXPECT_EQ(0x1FF, vscp_class);
}

TEST(VscpHelper, getVscpTypeFromCANALid)
{
    uint32_t id = 0x0000FF00;  // Type in bits 8-15
    uint16_t vscp_type = vscp_getVscpTypeFromCANALid(id);
    EXPECT_EQ(0xFF, vscp_type);
}

TEST(VscpHelper, getNicknameFromCANALid)
{
    uint32_t id = 0x000000FF;  // Nickname in bits 0-7
    uint8_t nickname = vscp_getNicknameFromCANALid(id);
    EXPECT_EQ(0xFF, nickname);
}

TEST(VscpHelper, getCANALidFromData)
{
    uint32_t id = vscp_getCANALidFromData(7, 10, 6);
    // Should have priority, class, type encoded
    EXPECT_NE(0u, id);
    
    // Verify we can extract them back
    EXPECT_EQ(10, vscp_getVscpClassFromCANALid(id));
    EXPECT_EQ(6, vscp_getVscpTypeFromCANALid(id));
}

// =============================================================================
//                         Interface Parsing
// =============================================================================

TEST(VscpHelper, getPortFromInterface)
{
    EXPECT_EQ(9598, vscp_getPortFromInterface("tcp://localhost:9598"));
    EXPECT_EQ(80, vscp_getPortFromInterface("http://example.com:80"));
    // Returns 0 instead of -1 when port is not specified
    EXPECT_EQ(0, vscp_getPortFromInterface("tcp://localhost"));
}

TEST(VscpHelper, getHostFromInterface)
{
    EXPECT_EQ("localhost", vscp_getHostFromInterface("tcp://localhost:9598"));
    EXPECT_EQ("example.com", vscp_getHostFromInterface("http://example.com:80"));
    EXPECT_EQ("localhost", vscp_getHostFromInterface("tcp://localhost"));
}

// Note: vscp_getProtocolFromInterface is declared but not implemented

// =============================================================================
//                         Safe String Encoding
// =============================================================================

TEST(VscpHelper, safe_encode_str)
{
    std::string str = "Hello\nWorld\t\"Test\"";
    std::string encoded = vscp_safe_encode_str(str);
    
    // Should encode control characters as \x followed by hex
    EXPECT_NE(str, encoded);
    // \n (0x0A) becomes \xa
    EXPECT_NE(std::string::npos, encoded.find("\\xa"));
    // \t (0x09) becomes \x9
    EXPECT_NE(std::string::npos, encoded.find("\\x9"));
}

// =============================================================================
//                         Replace Backslash
// =============================================================================

TEST(VscpHelper, replaceBackslash)
{
    std::string path = "C:\\Users\\Test\\file.txt";
    vscp_replaceBackslash(path);
    EXPECT_EQ("C:/Users/Test/file.txt", path);
    
    std::string noBackslash = "C:/Users/Test";
    vscp_replaceBackslash(noBackslash);
    EXPECT_EQ("C:/Users/Test", noBackslash);
}

// =============================================================================
//                         CRC Functions
// =============================================================================

TEST(VscpHelper, calc_crc_Event)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.vscp_class = 10;
    event.vscp_type = 6;
    event.sizeData = 3;
    event.pdata = new uint8_t[3]{1, 2, 3};
    
    unsigned short crc1 = vscp_calc_crc_Event(&event, 0);  // Don't set
    unsigned short crc2 = vscp_calc_crc_Event(&event, 0);
    
    // CRC should be consistent
    EXPECT_EQ(crc1, crc2);
    
    // Set CRC in event
    vscp_calc_crc_Event(&event, 1);
    EXPECT_EQ(crc1, event.crc);
    
    delete[] event.pdata;
}

TEST(VscpHelper, calc_crc_EventEx)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    eventEx.vscp_class = 10;
    eventEx.vscp_type = 6;
    eventEx.sizeData = 3;
    eventEx.data[0] = 1;
    eventEx.data[1] = 2;
    eventEx.data[2] = 3;
    
    unsigned short crc1 = vscp_calc_crc_EventEx(&eventEx, 0);
    unsigned short crc2 = vscp_calc_crc_EventEx(&eventEx, 0);
    
    // CRC should be consistent
    EXPECT_EQ(crc1, crc2);
}

// =============================================================================
//                         XML Parsing Tests for Frame Types
// =============================================================================

TEST(VscpHelper, convertXMLToEvent_OriginalFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    
    // XML with ORIGINAL frame (head=0) and 32-bit timestamp
    std::string xml = "<event head=\"0\" obid=\"123\" datetime=\"2024-01-15T10:30:00Z\" "
                      "timestamp=\"50817\" class=\"10\" type=\"8\" "
                      "guid=\"00:00:00:00:00:00:00:00:00:00:00:00:00:01:00:02\" "
                      "data=\"1,2,3,4,5\" />";
    
    EXPECT_TRUE(vscp_convertXMLToEvent(&event, xml));
    // Always produces frame type 1 (UNIX_NS)
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, event.head & VSCP_HEADER16_FRAME_VERSION_MASK);
    EXPECT_EQ(123u, event.obid);
    // Timestamp should be converted to nanoseconds (> year 2020 in nanoseconds)
    EXPECT_GT(event.timestamp_ns, 1577836800000000000ULL);  // > 2020-01-01
    EXPECT_LT(event.timestamp_ns, 2524608000000000000ULL);  // < 2050-01-01
    EXPECT_EQ(10, event.vscp_class);
    EXPECT_EQ(8, event.vscp_type);
    EXPECT_EQ(5, event.sizeData);
    EXPECT_EQ(0x02, event.GUID[15]);
    EXPECT_EQ(0xffff, event.year);
    EXPECT_EQ(0xff, event.month);
    
    if (event.pdata) {
        delete[] event.pdata;
    }
}

TEST(VscpHelper, convertXMLToEvent_UnixNsFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    
    // XML with UNIX_NS frame (head=0x100) and 64-bit timestamp
    std::string xml = "<event head=\"256\" obid=\"123\" datetime=\"2024-01-15T10:30:00.123456789Z\" "
                      "timestamp=\"1705315800123456789\" class=\"10\" type=\"8\" "
                      "guid=\"00:00:00:00:00:00:00:00:00:00:00:00:00:01:00:02\" "
                      "data=\"1,2,3,4,5\" />";
    
    EXPECT_TRUE(vscp_convertXMLToEvent(&event, xml));
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, event.head & VSCP_HEADER16_FRAME_VERSION_MASK);
    EXPECT_EQ(123u, event.obid);
    EXPECT_EQ(1705315800123456789ULL, event.timestamp_ns);
    EXPECT_EQ(10, event.vscp_class);
    EXPECT_EQ(8, event.vscp_type);
    EXPECT_EQ(0xffff, event.year);
    EXPECT_EQ(0xff, event.month);
    
    if (event.pdata) {
        delete[] event.pdata;
    }
}

TEST(VscpHelper, convertXMLToEventEx_OriginalFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    // XML with ORIGINAL frame (head=0) and 32-bit timestamp
    std::string xml = "<event head=\"0\" obid=\"123\" datetime=\"2024-01-15T10:30:00Z\" "
                      "timestamp=\"50817\" class=\"10\" type=\"8\" "
                      "guid=\"00:00:00:00:00:00:00:00:00:00:00:00:00:01:00:02\" "
                      "data=\"1,2,3,4,5\" />";
    
    EXPECT_TRUE(vscp_convertXMLToEventEx(&eventEx, xml));
    // Always produces frame type 1 (UNIX_NS)
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, eventEx.head & VSCP_HEADER16_FRAME_VERSION_MASK);
    EXPECT_EQ(123u, eventEx.obid);
    // Timestamp should be converted to nanoseconds (> year 2020 in nanoseconds)
    EXPECT_GT(eventEx.timestamp_ns, 1577836800000000000ULL);  // > 2020-01-01
    EXPECT_LT(eventEx.timestamp_ns, 2524608000000000000ULL);  // < 2050-01-01
    EXPECT_EQ(10, eventEx.vscp_class);
    EXPECT_EQ(8, eventEx.vscp_type);
    EXPECT_EQ(5, eventEx.sizeData);
    EXPECT_EQ(0xffff, eventEx.year);
    EXPECT_EQ(0xff, eventEx.month);
}

TEST(VscpHelper, convertXMLToEventEx_UnixNsFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    // XML with UNIX_NS frame (head=0x100) and 64-bit timestamp
    std::string xml = "<event head=\"256\" obid=\"123\" datetime=\"2024-01-15T10:30:00.123456789Z\" "
                      "timestamp=\"1705315800123456789\" class=\"10\" type=\"8\" "
                      "guid=\"00:00:00:00:00:00:00:00:00:00:00:00:00:01:00:02\" "
                      "data=\"1,2,3,4,5\" />";
    
    EXPECT_TRUE(vscp_convertXMLToEventEx(&eventEx, xml));
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, eventEx.head & VSCP_HEADER16_FRAME_VERSION_MASK);
    EXPECT_EQ(123u, eventEx.obid);
    EXPECT_EQ(1705315800123456789ULL, eventEx.timestamp_ns);
    EXPECT_EQ(10, eventEx.vscp_class);
    EXPECT_EQ(8, eventEx.vscp_type);
    EXPECT_EQ(0xffff, eventEx.year);
    EXPECT_EQ(0xff, eventEx.month);
}

// =============================================================================
//                         HTML Conversion Tests for Frame Types
// =============================================================================

TEST(VscpHelper, convertEventToHTML_OriginalFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = 0;  // ORIGINAL frame
    event.vscp_class = 10;
    event.vscp_type = 8;
    event.timestamp = 50817;
    event.obid = 123;
    event.sizeData = 3;
    event.pdata = new uint8_t[3]{1, 2, 3};
    memset(event.GUID, 0, 16);
    event.GUID[13] = 0x01;
    event.GUID[15] = 0x02;
    
    std::string html;
    EXPECT_TRUE(vscp_convertEventToHTML(html, &event));
    
    // Verify 32-bit timestamp is in output
    EXPECT_NE(std::string::npos, html.find("timestamp: 50817"));
    EXPECT_NE(std::string::npos, html.find("class: 10"));
    EXPECT_NE(std::string::npos, html.find("type: 8"));
    
    delete[] event.pdata;
}

TEST(VscpHelper, convertEventToHTML_UnixNsFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;  // UNIX_NS frame
    event.vscp_class = 10;
    event.vscp_type = 8;
    event.timestamp_ns = 1705315800123456789ULL;
    event.year = 0xffff;
    event.month = 0xff;
    event.obid = 123;
    event.sizeData = 3;
    event.pdata = new uint8_t[3]{1, 2, 3};
    memset(event.GUID, 0, 16);
    event.GUID[13] = 0x01;
    event.GUID[15] = 0x02;
    
    std::string html;
    EXPECT_TRUE(vscp_convertEventToHTML(html, &event));
    
    // Verify 64-bit timestamp is in output
    EXPECT_NE(std::string::npos, html.find("timestamp: 1705315800123456789"));
    EXPECT_NE(std::string::npos, html.find("class: 10"));
    EXPECT_NE(std::string::npos, html.find("type: 8"));
    
    delete[] event.pdata;
}

TEST(VscpHelper, convertEventExToHTML_OriginalFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    eventEx.head = 0;  // ORIGINAL frame
    eventEx.vscp_class = 10;
    eventEx.vscp_type = 8;
    eventEx.timestamp = 50817;
    eventEx.obid = 123;
    eventEx.sizeData = 3;
    eventEx.data[0] = 1;
    eventEx.data[1] = 2;
    eventEx.data[2] = 3;
    memset(eventEx.GUID, 0, 16);
    eventEx.GUID[13] = 0x01;
    eventEx.GUID[15] = 0x02;
    
    std::string html;
    EXPECT_TRUE(vscp_convertEventExToHTML(html, &eventEx));
    
    // Verify 32-bit timestamp is in output
    EXPECT_NE(std::string::npos, html.find("timestamp: 50817"));
    EXPECT_NE(std::string::npos, html.find("class: 10"));
    EXPECT_NE(std::string::npos, html.find("type: 8"));
}

TEST(VscpHelper, convertEventExToHTML_UnixNsFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;  // UNIX_NS frame
    eventEx.vscp_class = 10;
    eventEx.vscp_type = 8;
    eventEx.timestamp_ns = 1705315800123456789ULL;
    eventEx.year = 0xffff;
    eventEx.month = 0xff;
    eventEx.obid = 123;
    eventEx.sizeData = 3;
    eventEx.data[0] = 1;
    eventEx.data[1] = 2;
    eventEx.data[2] = 3;
    memset(eventEx.GUID, 0, 16);
    eventEx.GUID[13] = 0x01;
    eventEx.GUID[15] = 0x02;
    
    std::string html;
    EXPECT_TRUE(vscp_convertEventExToHTML(html, &eventEx));
    
    // Verify 64-bit timestamp is in output
    EXPECT_NE(std::string::npos, html.find("timestamp: 1705315800123456789"));
    EXPECT_NE(std::string::npos, html.find("class: 10"));
    EXPECT_NE(std::string::npos, html.find("type: 8"));
}

// =============================================================================
//                         setEventDateTime Tests for Frame Types
// =============================================================================

TEST(VscpHelper, setEventDateTime_OriginalFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = 0;  // ORIGINAL frame
    
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 2024 - 1900;  // Year since 1900
    tm.tm_mon = 0;              // January (0-based)
    tm.tm_mday = 15;
    tm.tm_hour = 10;
    tm.tm_min = 30;
    tm.tm_sec = 45;
    
    EXPECT_TRUE(vscp_setEventDateTime(&event, &tm));
    
    // Verify individual fields are set
    EXPECT_EQ(2024, event.year);
    EXPECT_EQ(1, event.month);
    EXPECT_EQ(15, event.day);
    EXPECT_EQ(10, event.hour);
    EXPECT_EQ(30, event.minute);
    EXPECT_EQ(45, event.second);
}

TEST(VscpHelper, setEventDateTime_UnixNsFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;  // UNIX_NS frame
    
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 2024 - 1900;  // Year since 1900
    tm.tm_mon = 0;              // January (0-based)
    tm.tm_mday = 15;
    tm.tm_hour = 10;
    tm.tm_min = 30;
    tm.tm_sec = 0;
    
    // Compute expected value the same way the function does
    time_t expected_time = timegm(&tm);
    uint64_t expected_ns = (uint64_t)expected_time * 1000000000ULL;
    
    EXPECT_TRUE(vscp_setEventDateTime(&event, &tm));
    
    // Verify timestamp_ns is set correctly
    EXPECT_EQ(expected_ns, event.timestamp_ns);
    EXPECT_EQ(0xffff, event.year);
    EXPECT_EQ(0xff, event.month);
}

TEST(VscpHelper, setEventExDateTime_OriginalFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    eventEx.head = 0;  // ORIGINAL frame
    
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 2024 - 1900;  // Year since 1900
    tm.tm_mon = 0;              // January (0-based)
    tm.tm_mday = 15;
    tm.tm_hour = 10;
    tm.tm_min = 30;
    tm.tm_sec = 45;
    
    EXPECT_TRUE(vscp_setEventExDateTime(&eventEx, &tm));
    
    // Verify individual fields are set
    EXPECT_EQ(2024, eventEx.year);
    EXPECT_EQ(1, eventEx.month);
    EXPECT_EQ(15, eventEx.day);
    EXPECT_EQ(10, eventEx.hour);
    EXPECT_EQ(30, eventEx.minute);
    EXPECT_EQ(45, eventEx.second);
}

TEST(VscpHelper, setEventExDateTime_UnixNsFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;  // UNIX_NS frame
    
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 2024 - 1900;  // Year since 1900
    tm.tm_mon = 0;              // January (0-based)
    tm.tm_mday = 15;
    tm.tm_hour = 10;
    tm.tm_min = 30;
    tm.tm_sec = 0;
    
    // Compute expected value the same way the function does
    time_t expected_time = timegm(&tm);
    uint64_t expected_ns = (uint64_t)expected_time * 1000000000ULL;
    
    EXPECT_TRUE(vscp_setEventExDateTime(&eventEx, &tm));
    
    // Verify timestamp_ns is set correctly
    EXPECT_EQ(expected_ns, eventEx.timestamp_ns);
    EXPECT_EQ(0xffff, eventEx.year);
    EXPECT_EQ(0xff, eventEx.month);
}

// =============================================================================
//                    setEventDateTime Subsecond Precision Tests
// =============================================================================

TEST(VscpHelper, setEventDateTime_UnixNsFrame_WithSubsecond)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;  // UNIX_NS frame
    
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 2024 - 1900;  // Year since 1900
    tm.tm_mon = 0;              // January (0-based)
    tm.tm_mday = 15;
    tm.tm_hour = 10;
    tm.tm_min = 30;
    tm.tm_sec = 0;
    
    uint32_t subsec_ns = 123456789;  // 123.456789 milliseconds
    
    // Compute expected value
    time_t expected_time = timegm(&tm);
    uint64_t expected_ns = (uint64_t)expected_time * 1000000000ULL + subsec_ns;
    
    EXPECT_TRUE(vscp_setEventDateTime(&event, &tm, subsec_ns));
    
    // Verify timestamp_ns includes subsecond precision
    EXPECT_EQ(expected_ns, event.timestamp_ns);
    EXPECT_EQ(0xffff, event.year);
    EXPECT_EQ(0xff, event.month);
}

TEST(VscpHelper, setEventDateTime_OriginalFrame_SubsecondIgnored)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = 0;  // ORIGINAL frame
    
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 2024 - 1900;
    tm.tm_mon = 5;    // June
    tm.tm_mday = 20;
    tm.tm_hour = 14;
    tm.tm_min = 45;
    tm.tm_sec = 30;
    
    // Pass subsecond ns - should be ignored for ORIGINAL frame
    EXPECT_TRUE(vscp_setEventDateTime(&event, &tm, 500000000));
    
    // Verify individual fields are set (subsecond ignored)
    EXPECT_EQ(2024, event.year);
    EXPECT_EQ(6, event.month);
    EXPECT_EQ(20, event.day);
    EXPECT_EQ(14, event.hour);
    EXPECT_EQ(45, event.minute);
    EXPECT_EQ(30, event.second);
}

TEST(VscpHelper, setEventDateTime_UnixNsFrame_NsClamped)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
    
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 2024 - 1900;
    tm.tm_mon = 0;
    tm.tm_mday = 1;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    
    // Pass value > 999999999 - should be clamped
    uint32_t too_large_ns = 2000000000;  // 2 billion - invalid
    
    time_t expected_time = timegm(&tm);
    uint64_t expected_ns = (uint64_t)expected_time * 1000000000ULL + 999999999;  // Clamped value
    
    EXPECT_TRUE(vscp_setEventDateTime(&event, &tm, too_large_ns));
    
    // Verify ns was clamped to max valid value
    EXPECT_EQ(expected_ns, event.timestamp_ns);
}

TEST(VscpHelper, setEventExDateTime_UnixNsFrame_WithSubsecond)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
    
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 2024 - 1900;
    tm.tm_mon = 0;
    tm.tm_mday = 15;
    tm.tm_hour = 10;
    tm.tm_min = 30;
    tm.tm_sec = 0;
    
    uint32_t subsec_ns = 987654321;  // 987.654321 milliseconds
    
    time_t expected_time = timegm(&tm);
    uint64_t expected_ns = (uint64_t)expected_time * 1000000000ULL + subsec_ns;
    
    EXPECT_TRUE(vscp_setEventExDateTime(&eventEx, &tm, subsec_ns));
    
    EXPECT_EQ(expected_ns, eventEx.timestamp_ns);
    EXPECT_EQ(0xffff, eventEx.year);
    EXPECT_EQ(0xff, eventEx.month);
}

TEST(VscpHelper, setEventExDateTime_OriginalFrame_SubsecondIgnored)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    eventEx.head = 0;  // ORIGINAL frame
    
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 2024 - 1900;
    tm.tm_mon = 11;   // December
    tm.tm_mday = 31;
    tm.tm_hour = 23;
    tm.tm_min = 59;
    tm.tm_sec = 59;
    
    // Pass subsecond ns - should be ignored for ORIGINAL frame
    EXPECT_TRUE(vscp_setEventExDateTime(&eventEx, &tm, 999999999));
    
    // Verify individual fields are set (subsecond ignored)
    EXPECT_EQ(2024, eventEx.year);
    EXPECT_EQ(12, eventEx.month);
    EXPECT_EQ(31, eventEx.day);
    EXPECT_EQ(23, eventEx.hour);
    EXPECT_EQ(59, eventEx.minute);
    EXPECT_EQ(59, eventEx.second);
}

// =============================================================================
//                    setEventToNow Tests for Frame Types
// =============================================================================

TEST(VscpHelper, setEventToNow_OriginalFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = 0;  // ORIGINAL frame
    
    time_t before = time(nullptr);
    EXPECT_TRUE(vscp_setEventToNow(&event));
    time_t after = time(nullptr);
    
    // Verify year is reasonable (current year)
    struct tm *now = gmtime(&before);
    EXPECT_EQ(now->tm_year + 1900, event.year);
    EXPECT_GE(event.month, 1);
    EXPECT_LE(event.month, 12);
    EXPECT_GE(event.day, 1);
    EXPECT_LE(event.day, 31);
}

TEST(VscpHelper, setEventToNow_UnixNsFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;  // UNIX_NS frame
    
    time_t before = time(nullptr);
    EXPECT_TRUE(vscp_setEventToNow(&event));
    time_t after = time(nullptr);
    
    // Verify timestamp_ns is in reasonable range
    uint64_t before_ns = (uint64_t)before * 1000000000ULL;
    uint64_t after_ns = (uint64_t)after * 1000000000ULL;
    
    EXPECT_GE(event.timestamp_ns, before_ns);
    EXPECT_LE(event.timestamp_ns, after_ns + 1000000000ULL);  // Allow 1 second margin
    EXPECT_EQ(0xffff, event.year);
    EXPECT_EQ(0xff, event.month);
}

TEST(VscpHelper, setEventExToNow_OriginalFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    eventEx.head = 0;  // ORIGINAL frame
    
    time_t before = time(nullptr);
    EXPECT_TRUE(vscp_setEventExToNow(&eventEx));
    time_t after = time(nullptr);
    
    // Verify year is reasonable (current year)
    struct tm *now = gmtime(&before);
    EXPECT_EQ(now->tm_year + 1900, eventEx.year);
    EXPECT_GE(eventEx.month, 1);
    EXPECT_LE(eventEx.month, 12);
    EXPECT_GE(eventEx.day, 1);
    EXPECT_LE(eventEx.day, 31);
}

TEST(VscpHelper, setEventExToNow_UnixNsFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;  // UNIX_NS frame
    
    time_t before = time(nullptr);
    EXPECT_TRUE(vscp_setEventExToNow(&eventEx));
    time_t after = time(nullptr);
    
    // Verify timestamp_ns is in reasonable range
    uint64_t before_ns = (uint64_t)before * 1000000000ULL;
    uint64_t after_ns = (uint64_t)after * 1000000000ULL;
    
    EXPECT_GE(eventEx.timestamp_ns, before_ns);
    EXPECT_LE(eventEx.timestamp_ns, after_ns + 1000000000ULL);  // Allow 1 second margin
    EXPECT_EQ(0xffff, eventEx.year);
    EXPECT_EQ(0xff, eventEx.month);
}

// =============================================================================
//                    convertCanalToEvent Tests
// =============================================================================

TEST(VscpHelper, convertCanalToEvent_SetsUnixNsFrameVersion)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    
    canalMsg canalMsg;
    memset(&canalMsg, 0, sizeof(canalMsg));
    canalMsg.id = 0x0c0a0601;  // Class 10, Type 6, priority 3
    canalMsg.sizeData = 3;
    canalMsg.data[0] = 138;
    canalMsg.data[1] = 0;
    canalMsg.data[2] = 30;
    canalMsg.timestamp = 12345;
    
    unsigned char guid[16] = {0};
    guid[0] = 0xFF;
    guid[1] = 0xFF;
    
    EXPECT_TRUE(vscp_convertCanalToEvent(&event, &canalMsg, guid));
    
    // Verify frame version is UNIX_NS
    uint16_t frameVersion = event.head & VSCP_HEADER16_FRAME_VERSION_MASK;
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, frameVersion);
    
    // Verify class and type extracted correctly
    EXPECT_EQ(10, event.vscp_class);
    EXPECT_EQ(6, event.vscp_type);
    
    // Verify data copied
    EXPECT_EQ(3, event.sizeData);
    EXPECT_EQ(138, event.pdata[0]);
    EXPECT_EQ(0, event.pdata[1]);
    EXPECT_EQ(30, event.pdata[2]);
    
    // Verify UNIX_NS frame fields are set
    EXPECT_EQ(0xffff, event.year);
    EXPECT_EQ(0xff, event.month);
    
    // Cleanup
    if (event.pdata != nullptr) {
        delete[] event.pdata;
    }
}

// =============================================================================
//                    setEventDateTimeBlockToNow Tests
// =============================================================================

TEST(VscpHelper, setEventDateTimeBlockToNow_OriginalFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = 0;  // ORIGINAL frame
    
    time_t before = time(nullptr);
    EXPECT_TRUE(vscp_setEventDateTimeBlockToNow(&event));
    time_t after = time(nullptr);
    
    // Verify year is reasonable (current year)
    struct tm *now = gmtime(&before);
    EXPECT_EQ(now->tm_year + 1900, event.year);
    EXPECT_GE(event.month, 1);
    EXPECT_LE(event.month, 12);
    EXPECT_GE(event.day, 1);
    EXPECT_LE(event.day, 31);
}

TEST(VscpHelper, setEventDateTimeBlockToNow_UnixNsFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;  // UNIX_NS frame
    
    time_t before = time(nullptr);
    EXPECT_TRUE(vscp_setEventDateTimeBlockToNow(&event));
    time_t after = time(nullptr);
    
    // Verify timestamp_ns is in reasonable range
    uint64_t before_ns = (uint64_t)before * 1000000000ULL;
    uint64_t after_ns = (uint64_t)after * 1000000000ULL;
    
    EXPECT_GE(event.timestamp_ns, before_ns);
    EXPECT_LE(event.timestamp_ns, after_ns + 1000000000ULL);  // Allow 1 second margin
    EXPECT_EQ(0xffff, event.year);
    EXPECT_EQ(0xff, event.month);
}

TEST(VscpHelper, setEventExDateTimeBlockToNow_OriginalFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    eventEx.head = 0;  // ORIGINAL frame
    
    time_t before = time(nullptr);
    EXPECT_TRUE(vscp_setEventExDateTimeBlockToNow(&eventEx));
    time_t after = time(nullptr);
    
    // Verify year is reasonable (current year)
    struct tm *now = gmtime(&before);
    EXPECT_EQ(now->tm_year + 1900, eventEx.year);
    EXPECT_GE(eventEx.month, 1);
    EXPECT_LE(eventEx.month, 12);
    EXPECT_GE(eventEx.day, 1);
    EXPECT_LE(eventEx.day, 31);
}

TEST(VscpHelper, setEventExDateTimeBlockToNow_UnixNsFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;  // UNIX_NS frame
    
    time_t before = time(nullptr);
    EXPECT_TRUE(vscp_setEventExDateTimeBlockToNow(&eventEx));
    time_t after = time(nullptr);
    
    // Verify timestamp_ns is in reasonable range
    uint64_t before_ns = (uint64_t)before * 1000000000ULL;
    uint64_t after_ns = (uint64_t)after * 1000000000ULL;
    
    EXPECT_GE(eventEx.timestamp_ns, before_ns);
    EXPECT_LE(eventEx.timestamp_ns, after_ns + 1000000000ULL);  // Allow 1 second margin
    EXPECT_EQ(0xffff, eventEx.year);
    EXPECT_EQ(0xff, eventEx.month);
}

// =============================================================================
//                    convertEventToString Tests for Frame Types
// =============================================================================

TEST(VscpHelper, convertEventToString_OriginalFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = 0;  // ORIGINAL frame
    event.vscp_class = 10;
    event.vscp_type = 6;
    event.obid = 123;
    event.timestamp = 456789;  // microseconds
    event.year = 2024;
    event.month = 1;
    event.day = 15;
    event.hour = 10;
    event.minute = 30;
    event.second = 45;
    memset(event.GUID, 0, 16);
    event.GUID[0] = 0xFF;
    event.GUID[15] = 0x01;
    event.sizeData = 0;
    event.pdata = nullptr;
    
    std::string str;
    EXPECT_TRUE(vscp_convertEventToString(str, &event));
    
    // Always outputs frame version 1 (UNIX_NS) with nanosecond timestamp
    // Should NOT contain the old 32-bit timestamp format
    EXPECT_EQ(std::string::npos, str.find(",456789,"));
    // Head should have UNIX_NS bit set (256)
    EXPECT_NE(std::string::npos, str.find("256,"));
    // Verify class and type
    EXPECT_NE(std::string::npos, str.find(",10,6,"));
}

TEST(VscpHelper, convertEventToString_UnixNsFrame)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;  // UNIX_NS frame
    event.vscp_class = 10;
    event.vscp_type = 6;
    event.obid = 123;
    event.timestamp_ns = 1705315845123456789ULL;  // Large 64-bit timestamp
    event.year = 0xffff;
    event.month = 0xff;
    memset(event.GUID, 0, 16);
    event.GUID[0] = 0xFF;
    event.GUID[15] = 0x01;
    event.sizeData = 0;
    event.pdata = nullptr;
    
    std::string str;
    EXPECT_TRUE(vscp_convertEventToString(str, &event));
    
    // Verify string contains the 64-bit timestamp
    EXPECT_NE(std::string::npos, str.find(",1705315845123456789,"));
    // Verify class and type
    EXPECT_NE(std::string::npos, str.find(",10,6,"));
}

TEST(VscpHelper, convertEventExToString_UnixNsFrame)
{
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    eventEx.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;  // UNIX_NS frame
    eventEx.vscp_class = 20;
    eventEx.vscp_type = 3;
    eventEx.obid = 456;
    eventEx.timestamp_ns = 1705315845987654321ULL;
    eventEx.year = 0xffff;
    eventEx.month = 0xff;
    memset(eventEx.GUID, 0, 16);
    eventEx.sizeData = 0;
    
    std::string str;
    EXPECT_TRUE(vscp_convertEventExToString(str, &eventEx));
    
    // Verify string contains the 64-bit timestamp
    EXPECT_NE(std::string::npos, str.find(",1705315845987654321,"));
    // Verify class and type
    EXPECT_NE(std::string::npos, str.find(",20,3,"));
}

// Tests for vscp_convertStringToEvent producing UNIX_NS frame format

TEST(VscpHelper, convertStringToEvent_SetsUnixNsFrameVersion)
{
    // Even if head is 0, the result should have UNIX_NS frame version
    std::string str = "0,10,6,0,,0,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00,0x01,0x02";
    
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    
    EXPECT_TRUE(vscp_convertStringToEvent(&event, str));
    
    // Verify UNIX_NS frame version is set
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, event.head & VSCP_HEADER16_FRAME_VERSION_MASK);
    // Verify year and month markers for UNIX_NS
    EXPECT_EQ(0xffff, event.year);
    EXPECT_EQ(0xff, event.month);
    
    vscp_deleteEvent(&event);
}

TEST(VscpHelper, convertStringToEvent_NsTimestampWhenDatetimeEmpty)
{
    // When datetime is empty, the timestamp field should be treated as nanoseconds
    uint64_t expected_ns = 1705315845123456789ULL;
    std::string str = "0,10,6,0,," + std::to_string(expected_ns) + ",00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00,0x01";
    
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    
    EXPECT_TRUE(vscp_convertStringToEvent(&event, str));
    
    // Verify timestamp_ns contains the nanosecond value
    EXPECT_EQ(expected_ns, event.timestamp_ns);
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, event.head & VSCP_HEADER16_FRAME_VERSION_MASK);
    
    vscp_deleteEvent(&event);
}

TEST(VscpHelper, convertStringToEvent_GeneratesTimestampWhenZero)
{
    // When datetime is empty and timestamp is 0, should generate current timestamp
    std::string str = "0,10,6,0,,0,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00,0x01";
    
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    
    uint64_t before = vscp_makeTimeStampNs();
    EXPECT_TRUE(vscp_convertStringToEvent(&event, str));
    uint64_t after = vscp_makeTimeStampNs();
    
    // Verify timestamp_ns is within reasonable range (now)
    EXPECT_GE(event.timestamp_ns, before);
    EXPECT_LE(event.timestamp_ns, after);
    
    vscp_deleteEvent(&event);
}

TEST(VscpHelper, convertStringToEvent_DatetimeParsedToNs)
{
    // When datetime is present, it should be converted to nanoseconds
    // 2024-01-15T12:30:45
    std::string str = "0,10,6,0,2024-01-15T12:30:45,0,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00,0x01";
    
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.pdata = nullptr;
    
    EXPECT_TRUE(vscp_convertStringToEvent(&event, str));
    
    // Verify it's a UNIX_NS frame
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, event.head & VSCP_HEADER16_FRAME_VERSION_MASK);
    // Verify timestamp_ns is reasonable (after year 2024)
    EXPECT_GT(event.timestamp_ns, 1704067200000000000ULL);  // Jan 1, 2024 in ns
    
    vscp_deleteEvent(&event);
}

TEST(VscpHelper, convertStringToEventEx_SetsUnixNsFrameVersion)
{
    // Even if head is 0, the result should have UNIX_NS frame version
    std::string str = "0,10,6,0,,0,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00,0x01,0x02";
    
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    EXPECT_TRUE(vscp_convertStringToEventEx(&eventEx, str));
    
    // Verify UNIX_NS frame version is set
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, eventEx.head & VSCP_HEADER16_FRAME_VERSION_MASK);
    // Verify year and month markers for UNIX_NS
    EXPECT_EQ(0xffff, eventEx.year);
    EXPECT_EQ(0xff, eventEx.month);
}

TEST(VscpHelper, convertStringToEventEx_NsTimestampWhenDatetimeEmpty)
{
    // When datetime is empty, the timestamp field should be treated as nanoseconds
    uint64_t expected_ns = 1705315845123456789ULL;
    std::string str = "0,10,6,0,," + std::to_string(expected_ns) + ",00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00,0x01";
    
    vscpEventEx eventEx;
    memset(&eventEx, 0, sizeof(eventEx));
    
    EXPECT_TRUE(vscp_convertStringToEventEx(&eventEx, str));
    
    // Verify timestamp_ns contains the nanosecond value
    EXPECT_EQ(expected_ns, eventEx.timestamp_ns);
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, eventEx.head & VSCP_HEADER16_FRAME_VERSION_MASK);
}

// Tests for frame write/read with UNIX_NS format

TEST(VscpHelper, writeEventToFrame_OriginalFormat)
{
    // Even with original frame format event, writeEventToFrame always writes 
    // packet format 1 (UNIX_NS) with the date/time converted to nanoseconds
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = VSCP_HEADER16_FRAME_VERSION_ORIGINAL;  // Original frame
    event.vscp_class = 10;
    event.vscp_type = 6;
    event.timestamp = 123456789;  // microseconds
    event.year = 2024;
    event.month = 1;
    event.day = 15;
    event.hour = 12;
    event.minute = 30;
    event.second = 45;
    memset(event.GUID, 0xAA, 16);
    event.sizeData = 2;
    event.pdata = new uint8_t[2]{0x11, 0x22};
    
    uint8_t frame[128];
    memset(frame, 0, sizeof(frame));
    
    EXPECT_TRUE(vscp_writeEventToFrame(frame, sizeof(frame), 0, &event));
    
    // Verify packet type byte has type 1 in upper nibble (always writes format 1)
    EXPECT_EQ(VSCP_MULTICAST_TYPE_EVENT1, GET_VSCP_BINARY_PACKET_TYPE(frame[0]));
    
    // Verify class/type positions for packet format 1
    EXPECT_EQ(0, frame[VSCP_BINARY_PACKET_FRAME0_POS_CLASS_MSB]);
    EXPECT_EQ(10, frame[VSCP_BINARY_PACKET_FRAME0_POS_CLASS_LSB]);
    EXPECT_EQ(0, frame[VSCP_BINARY_PACKET_FRAME0_POS_TYPE_MSB]);
    EXPECT_EQ(6, frame[VSCP_BINARY_PACKET_FRAME0_POS_TYPE_LSB]);
    
    // Verify the timestamp was converted from date/time to nanoseconds
    // 2024-01-15 12:30:45 UTC = 1705321845 seconds since epoch
    // Plus 123456789 microseconds = 123456789000 nanoseconds
    uint64_t expectedNs = 1705321845ULL * 1000000000ULL + 123456789ULL * 1000ULL;
    uint64_t readTs = ((uint64_t)frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP] << 56) |
                      ((uint64_t)frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP + 1] << 48) |
                      ((uint64_t)frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP + 2] << 40) |
                      ((uint64_t)frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP + 3] << 32) |
                      ((uint64_t)frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP + 4] << 24) |
                      ((uint64_t)frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP + 5] << 16) |
                      ((uint64_t)frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP + 6] << 8) |
                      frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP + 7];
    EXPECT_EQ(expectedNs, readTs);
    
    delete[] event.pdata;
}

TEST(VscpHelper, writeEventToFrame_UnixNsFormat)
{
    vscpEvent event;
    memset(&event, 0, sizeof(event));
    event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;  // UNIX_NS frame
    event.vscp_class = 20;
    event.vscp_type = 3;
    event.timestamp_ns = 1705315845123456789ULL;
    event.year = 0xffff;
    event.month = 0xff;
    memset(event.GUID, 0xBB, 16);
    event.sizeData = 2;
    event.pdata = new uint8_t[2]{0x33, 0x44};
    
    uint8_t frame[128];
    memset(frame, 0, sizeof(frame));
    
    EXPECT_TRUE(vscp_writeEventToFrame(frame, sizeof(frame), 0, &event));
    
    // Verify packet type byte has type 1 in upper nibble
    EXPECT_EQ(VSCP_MULTICAST_TYPE_EVENT1, GET_VSCP_BINARY_PACKET_TYPE(frame[0]));
    
    // Verify class/type positions for packet format 1
    EXPECT_EQ(0, frame[VSCP_BINARY_PACKET_FRAME0_POS_CLASS_MSB]);
    EXPECT_EQ(20, frame[VSCP_BINARY_PACKET_FRAME0_POS_CLASS_LSB]);
    EXPECT_EQ(0, frame[VSCP_BINARY_PACKET_FRAME0_POS_TYPE_MSB]);
    EXPECT_EQ(3, frame[VSCP_BINARY_PACKET_FRAME0_POS_TYPE_LSB]);
    
    // Verify 8-byte nanosecond timestamp is written correctly
    uint64_t readTs = ((uint64_t)frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP] << 56) |
                      ((uint64_t)frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP + 1] << 48) |
                      ((uint64_t)frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP + 2] << 40) |
                      ((uint64_t)frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP + 3] << 32) |
                      ((uint64_t)frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP + 4] << 24) |
                      ((uint64_t)frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP + 5] << 16) |
                      ((uint64_t)frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP + 6] << 8) |
                      frame[VSCP_BINARY_PACKET_FRAME0_POS_TIMESTAMP + 7];
    EXPECT_EQ(1705315845123456789ULL, readTs);
    
    delete[] event.pdata;
}

TEST(VscpHelper, getEventFromFrame_UnixNsFormat_RoundTrip)
{
    // Create an event with UNIX_NS format
    vscpEvent eventOut;
    memset(&eventOut, 0, sizeof(eventOut));
    eventOut.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS | 0x0007;
    eventOut.vscp_class = 100;
    eventOut.vscp_type = 50;
    eventOut.timestamp_ns = 1705315845987654321ULL;
    eventOut.year = 0xffff;
    eventOut.month = 0xff;
    for (int i = 0; i < 16; i++) eventOut.GUID[i] = i;
    eventOut.sizeData = 3;
    eventOut.pdata = new uint8_t[3]{0xAA, 0xBB, 0xCC};
    
    // Write to frame
    uint8_t frame[128];
    memset(frame, 0, sizeof(frame));
    EXPECT_TRUE(vscp_writeEventToFrame(frame, sizeof(frame), 0, &eventOut));
    
    // Read back from frame
    vscpEvent eventIn;
    memset(&eventIn, 0, sizeof(eventIn));
    eventIn.pdata = nullptr;
    EXPECT_TRUE(vscp_getEventFromFrame(&eventIn, frame, sizeof(frame)));
    
    // Verify round-trip
    EXPECT_EQ(eventOut.head, eventIn.head);
    EXPECT_EQ(eventOut.vscp_class, eventIn.vscp_class);
    EXPECT_EQ(eventOut.vscp_type, eventIn.vscp_type);
    EXPECT_EQ(eventOut.timestamp_ns, eventIn.timestamp_ns);
    EXPECT_EQ(eventOut.sizeData, eventIn.sizeData);
    EXPECT_EQ(0xffff, eventIn.year);
    EXPECT_EQ(0xff, eventIn.month);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(eventOut.GUID[i], eventIn.GUID[i]);
    }
    for (uint16_t i = 0; i < eventOut.sizeData; i++) {
        EXPECT_EQ(eventOut.pdata[i], eventIn.pdata[i]);
    }
    
    delete[] eventOut.pdata;
    vscp_deleteEvent(&eventIn);
}

// =============================================================================
//                      Platform Compatibility - Byteswap
// =============================================================================

TEST(PlatformCompat, bswap_16_basic)
{
    // Test basic 16-bit byte swap
    EXPECT_EQ(0x3412, bswap_16(0x1234));
    EXPECT_EQ(0x0000, bswap_16(0x0000));
    EXPECT_EQ(0xFFFF, bswap_16(0xFFFF));
    EXPECT_EQ(0x00FF, bswap_16(0xFF00));
    EXPECT_EQ(0xFF00, bswap_16(0x00FF));
}

TEST(PlatformCompat, bswap_16_roundtrip)
{
    // Double swap should return original value
    uint16_t original = 0xABCD;
    EXPECT_EQ(original, bswap_16(bswap_16(original)));
    
    original = 0x1234;
    EXPECT_EQ(original, bswap_16(bswap_16(original)));
}

TEST(PlatformCompat, bswap_32_basic)
{
    // Test basic 32-bit byte swap
    EXPECT_EQ(0x78563412u, bswap_32(0x12345678u));
    EXPECT_EQ(0x00000000u, bswap_32(0x00000000u));
    EXPECT_EQ(0xFFFFFFFFu, bswap_32(0xFFFFFFFFu));
    EXPECT_EQ(0x000000FFu, bswap_32(0xFF000000u));
    EXPECT_EQ(0xFF000000u, bswap_32(0x000000FFu));
}

TEST(PlatformCompat, bswap_32_roundtrip)
{
    // Double swap should return original value
    uint32_t original = 0xDEADBEEF;
    EXPECT_EQ(original, bswap_32(bswap_32(original)));
    
    original = 0x12345678;
    EXPECT_EQ(original, bswap_32(bswap_32(original)));
}

TEST(PlatformCompat, bswap_64_basic)
{
    // Test basic 64-bit byte swap
    EXPECT_EQ(0xEFCDAB9078563412ULL, bswap_64(0x1234567890ABCDEFULL));
    EXPECT_EQ(0x0000000000000000ULL, bswap_64(0x0000000000000000ULL));
    EXPECT_EQ(0xFFFFFFFFFFFFFFFFULL, bswap_64(0xFFFFFFFFFFFFFFFFULL));
    EXPECT_EQ(0x00000000000000FFULL, bswap_64(0xFF00000000000000ULL));
    EXPECT_EQ(0xFF00000000000000ULL, bswap_64(0x00000000000000FFULL));
}

TEST(PlatformCompat, bswap_64_roundtrip)
{
    // Double swap should return original value
    uint64_t original = 0xDEADBEEFCAFEBABEULL;
    EXPECT_EQ(original, bswap_64(bswap_64(original)));
    
    original = 0x123456789ABCDEF0ULL;
    EXPECT_EQ(original, bswap_64(bswap_64(original)));
}

TEST(PlatformCompat, bswap_network_order_conversion)
{
    // Test conversion to/from network byte order (big-endian)
    // This ensures the byteswap works correctly for network protocol use
    uint16_t host16 = 0x1234;
    uint16_t net16 = bswap_16(host16);
    
    // bswap_16(0x1234) = 0x3412
    EXPECT_EQ(0x3412, net16);
    
    // After swap, verify the individual bytes
    uint8_t *bytes = (uint8_t *)&net16;
    // On little-endian: 0x3412 is stored as [0x12, 0x34] in memory
    // On big-endian: 0x3412 is stored as [0x34, 0x12] in memory
    // The important thing is that double-swap returns original
    EXPECT_EQ(host16, bswap_16(net16));
}

// =============================================================================
//                      Event Creation and Deletion
// =============================================================================

TEST(EventManagement, vscp_newEvent_basic)
{
    vscpEvent *pEvent = nullptr;
    
    // Create new event with default frame version
    EXPECT_TRUE(vscp_newEvent(&pEvent, 0));
    ASSERT_NE(nullptr, pEvent);
    EXPECT_EQ(0, pEvent->head);
    EXPECT_EQ(0, pEvent->sizeData);
    EXPECT_EQ(nullptr, pEvent->pdata);
    
    // Cleanup
    vscp_deleteEvent_v2(&pEvent);
    EXPECT_EQ(nullptr, pEvent);
}

TEST(EventManagement, vscp_newEvent_with_frame_version)
{
    vscpEvent *pEvent = nullptr;
    
    // Create new event with UNIX_NS frame version
    EXPECT_TRUE(vscp_newEvent(&pEvent, VSCP_HEADER16_FRAME_VERSION_UNIX_NS));
    ASSERT_NE(nullptr, pEvent);
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, pEvent->head & VSCP_HEADER16_FRAME_VERSION_MASK);
    EXPECT_EQ(0, pEvent->sizeData);
    EXPECT_EQ(nullptr, pEvent->pdata);
    
    // Cleanup
    vscp_deleteEvent_v2(&pEvent);
    EXPECT_EQ(nullptr, pEvent);
}

TEST(EventManagement, vscp_newEventEx_basic)
{
    vscpEventEx *pEventEx = nullptr;
    
    // Create new eventEx with default frame version
    EXPECT_TRUE(vscp_newEventEx(&pEventEx));
    ASSERT_NE(nullptr, pEventEx);
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, pEventEx->head);
    EXPECT_EQ(0, pEventEx->sizeData);
    
    // Cleanup
    vscp_deleteEventEx(&pEventEx);
    EXPECT_EQ(nullptr, pEventEx);
}

TEST(EventManagement, vscp_newEventEx_with_frame_version)
{
    vscpEventEx *pEventEx = nullptr;
    
    // Create new eventEx with UNIX_NS frame version
    EXPECT_TRUE(vscp_newEventEx(&pEventEx, VSCP_HEADER16_FRAME_VERSION_UNIX_NS));
    ASSERT_NE(nullptr, pEventEx);
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, pEventEx->head & VSCP_HEADER16_FRAME_VERSION_MASK);
    EXPECT_EQ(0, pEventEx->sizeData);
    
    // Cleanup
    vscp_deleteEventEx(&pEventEx);
    EXPECT_EQ(nullptr, pEventEx);
}

TEST(EventManagement, vscp_deleteEvent_with_data)
{
    vscpEvent *pEvent = nullptr;
    
    // Create event and add data
    EXPECT_TRUE(vscp_newEvent(&pEvent, 0));
    ASSERT_NE(nullptr, pEvent);
    
    // Allocate and set data
    pEvent->sizeData = 5;
    pEvent->pdata = new uint8_t[5];
    pEvent->pdata[0] = 0x11;
    pEvent->pdata[1] = 0x22;
    pEvent->pdata[2] = 0x33;
    pEvent->pdata[3] = 0x44;
    pEvent->pdata[4] = 0x55;
    
    // Delete should free data
    vscp_deleteEvent(pEvent);
    EXPECT_EQ(nullptr, pEvent->pdata);
    
    // Full cleanup
    delete pEvent;
}

TEST(EventManagement, vscp_deleteEvent_v2_sets_nullptr)
{
    vscpEvent *pEvent = nullptr;
    
    // Create event
    EXPECT_TRUE(vscp_newEvent(&pEvent, 0));
    ASSERT_NE(nullptr, pEvent);
    
    // Delete should set pointer to nullptr
    vscp_deleteEvent_v2(&pEvent);
    EXPECT_EQ(nullptr, pEvent);
}

TEST(EventManagement, vscp_deleteEventEx_sets_nullptr)
{
    vscpEventEx *pEventEx = nullptr;
    
    // Create eventEx
    EXPECT_TRUE(vscp_newEventEx(&pEventEx, 0));
    ASSERT_NE(nullptr, pEventEx);
    
    // Delete should set pointer to nullptr
    vscp_deleteEventEx(&pEventEx);
    EXPECT_EQ(nullptr, pEventEx);
}

TEST(EventManagement, vscp_deleteEvent_null_safe)
{
    vscpEvent *pEvent = nullptr;
    
    // Should not crash when called with nullptr
    vscp_deleteEvent(nullptr);
    vscp_deleteEvent_v2(&pEvent);
    EXPECT_EQ(nullptr, pEvent);
}

TEST(EventManagement, vscp_deleteEventEx_null_safe)
{
    vscpEventEx *pEventEx = nullptr;
    
    // Should not crash when called with nullptr
    vscp_deleteEventEx(&pEventEx);
    EXPECT_EQ(nullptr, pEventEx);
}

// =============================================================================
//                      Frame Version Functions
// =============================================================================

TEST(FrameVersion, setFrameVersion_event_original)
{
    vscpEvent *pEvent = nullptr;
    EXPECT_TRUE(vscp_newEvent(&pEvent, 0));
    ASSERT_NE(nullptr, pEvent);
    
    // Set to original frame version (0)
    EXPECT_TRUE(setFrameVersion(pEvent, VSCP_HEADER16_FRAME_VERSION_ORIGINAL));
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_ORIGINAL, pEvent->head & VSCP_HEADER16_FRAME_VERSION_MASK);
    
    vscp_deleteEvent_v2(&pEvent);
}

TEST(FrameVersion, setFrameVersion_event_unix_ns)
{
    vscpEvent *pEvent = nullptr;
    EXPECT_TRUE(vscp_newEvent(&pEvent, 0));
    ASSERT_NE(nullptr, pEvent);
    
    // Set to UNIX_NS frame version
    EXPECT_TRUE(setFrameVersion(pEvent, VSCP_HEADER16_FRAME_VERSION_UNIX_NS));
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, pEvent->head & VSCP_HEADER16_FRAME_VERSION_MASK);
    
    vscp_deleteEvent_v2(&pEvent);
}

TEST(FrameVersion, setFrameVersion_event_preserves_other_bits)
{
    vscpEvent *pEvent = nullptr;
    EXPECT_TRUE(vscp_newEvent(&pEvent, 0));
    ASSERT_NE(nullptr, pEvent);
    
    // Set priority and other header bits
    pEvent->head = 0xE0;  // Priority 7 (bits 5-7)
    
    // Change frame version - should preserve priority bits
    EXPECT_TRUE(setFrameVersion(pEvent, VSCP_HEADER16_FRAME_VERSION_UNIX_NS));
    
    // Priority should still be 7
    EXPECT_EQ(7, (pEvent->head >> 5) & 0x07);
    // Frame version should be UNIX_NS
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, pEvent->head & VSCP_HEADER16_FRAME_VERSION_MASK);
    
    vscp_deleteEvent_v2(&pEvent);
}

TEST(FrameVersion, setFrameVersion_eventEx_original)
{
    vscpEventEx *pEventEx = nullptr;
    EXPECT_TRUE(vscp_newEventEx(&pEventEx, 0));
    ASSERT_NE(nullptr, pEventEx);
    
    // Set to original frame version (0)
    EXPECT_TRUE(setFrameVersion(pEventEx, VSCP_HEADER16_FRAME_VERSION_ORIGINAL));
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_ORIGINAL, pEventEx->head & VSCP_HEADER16_FRAME_VERSION_MASK);
    
    vscp_deleteEventEx(&pEventEx);
}

TEST(FrameVersion, setFrameVersion_eventEx_unix_ns)
{
    vscpEventEx *pEventEx = nullptr;
    EXPECT_TRUE(vscp_newEventEx(&pEventEx, 0));
    ASSERT_NE(nullptr, pEventEx);
    
    // Set to UNIX_NS frame version
    EXPECT_TRUE(setFrameVersion(pEventEx, VSCP_HEADER16_FRAME_VERSION_UNIX_NS));
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, pEventEx->head & VSCP_HEADER16_FRAME_VERSION_MASK);
    
    vscp_deleteEventEx(&pEventEx);
}

TEST(FrameVersion, setFrameVersion_eventEx_preserves_other_bits)
{
    vscpEventEx *pEventEx = nullptr;
    EXPECT_TRUE(vscp_newEventEx(&pEventEx, 0));
    ASSERT_NE(nullptr, pEventEx);
    
    // Set priority and other header bits
    pEventEx->head = 0xE0;  // Priority 7 (bits 5-7)
    
    // Change frame version - should preserve priority bits
    EXPECT_TRUE(setFrameVersion(pEventEx, VSCP_HEADER16_FRAME_VERSION_UNIX_NS));
    
    // Priority should still be 7
    EXPECT_EQ(7, (pEventEx->head >> 5) & 0x07);
    // Frame version should be UNIX_NS
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, pEventEx->head & VSCP_HEADER16_FRAME_VERSION_MASK);
    
    vscp_deleteEventEx(&pEventEx);
}

TEST(FrameVersion, setFrameVersion_null_event)
{
    vscpEvent *pEvent = nullptr;
    EXPECT_FALSE(setFrameVersion(pEvent, VSCP_HEADER16_FRAME_VERSION_UNIX_NS));
}

TEST(FrameVersion, setFrameVersion_null_eventEx)
{
    vscpEventEx *pEventEx = nullptr;
    EXPECT_FALSE(setFrameVersion(pEventEx, VSCP_HEADER16_FRAME_VERSION_UNIX_NS));
}

TEST(FrameVersion, setFrameVersion_switch_versions)
{
    vscpEvent *pEvent = nullptr;
    EXPECT_TRUE(vscp_newEvent(&pEvent, VSCP_HEADER16_FRAME_VERSION_ORIGINAL));
    ASSERT_NE(nullptr, pEvent);
    
    // Start with original
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_ORIGINAL, pEvent->head & VSCP_HEADER16_FRAME_VERSION_MASK);
    
    // Switch to UNIX_NS
    EXPECT_TRUE(setFrameVersion(pEvent, VSCP_HEADER16_FRAME_VERSION_UNIX_NS));
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, pEvent->head & VSCP_HEADER16_FRAME_VERSION_MASK);
    
    // Switch back to original
    EXPECT_TRUE(setFrameVersion(pEvent, VSCP_HEADER16_FRAME_VERSION_ORIGINAL));
    EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_ORIGINAL, pEvent->head & VSCP_HEADER16_FRAME_VERSION_MASK);
    
    vscp_deleteEvent_v2(&pEvent);
}

// =============================================================================
//                  vscp_writeCommandToFrame / vscp_writeReplyToFrame
// =============================================================================

TEST(VscpHelper, writeCommandToFrame_basic)
{
    uint8_t frame[64];
    memset(frame, 0, sizeof(frame));
    uint8_t arg[] = {0xAA, 0xBB, 0xCC};
    uint16_t command = 0x1234;

    int rv = vscp_writeCommandToFrame(frame, sizeof(frame), command, arg, sizeof(arg));
    EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);

    // Packet type: command | no encryption
    EXPECT_EQ(VSCP_BINARY_PACKET_TYPE_COMMAND | VSCP_ENCRYPTION_NONE, frame[0]);

    // Command MSB/LSB
    EXPECT_EQ(0x12, frame[1]);
    EXPECT_EQ(0x34, frame[2]);

    // Argument data
    EXPECT_EQ(0xAA, frame[3]);
    EXPECT_EQ(0xBB, frame[4]);
    EXPECT_EQ(0xCC, frame[5]);
}

TEST(VscpHelper, writeCommandToFrame_no_args)
{
    uint8_t frame[64];
    memset(frame, 0, sizeof(frame));
    uint16_t command = 0x0001;

    int rv = vscp_writeCommandToFrame(frame, sizeof(frame), command, NULL, 0);
    EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);

    // Packet type
    EXPECT_EQ(VSCP_BINARY_PACKET_TYPE_COMMAND | VSCP_ENCRYPTION_NONE, frame[0]);

    // Command
    EXPECT_EQ(0x00, frame[1]);
    EXPECT_EQ(0x01, frame[2]);
}

TEST(VscpHelper, writeCommandToFrame_null_frame)
{
    uint8_t arg[] = {0x01};
    int rv = vscp_writeCommandToFrame(NULL, 64, 0x0001, arg, sizeof(arg));
    EXPECT_EQ(VSCP_ERROR_PARAMETER, rv);
}

TEST(VscpHelper, writeCommandToFrame_null_arg_with_nonzero_len)
{
    uint8_t frame[64];
    int rv = vscp_writeCommandToFrame(frame, sizeof(frame), 0x0001, NULL, 5);
    EXPECT_EQ(VSCP_ERROR_PARAMETER, rv);
}

TEST(VscpHelper, writeCommandToFrame_buffer_too_small)
{
    uint8_t frame[4];  // Too small: need 1 + 2 + 0 + 2 = 5
    int rv = vscp_writeCommandToFrame(frame, sizeof(frame), 0x0001, NULL, 0);
    EXPECT_EQ(VSCP_ERROR_BUFFER_TO_SMALL, rv);
}

TEST(VscpHelper, writeCommandToFrame_buffer_exact_size)
{
    uint8_t frame[5];  // Exact: 1 pkttype + 2 cmd + 0 arg + 2 crc
    memset(frame, 0, sizeof(frame));
    int rv = vscp_writeCommandToFrame(frame, sizeof(frame), 0x0042, NULL, 0);
    EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
    EXPECT_EQ(VSCP_BINARY_PACKET_TYPE_COMMAND | VSCP_ENCRYPTION_NONE, frame[0]);
    EXPECT_EQ(0x00, frame[1]);
    EXPECT_EQ(0x42, frame[2]);
}

TEST(VscpHelper, writeCommandToFrame_command_zero)
{
    uint8_t frame[64];
    memset(frame, 0, sizeof(frame));
    int rv = vscp_writeCommandToFrame(frame, sizeof(frame), 0x0000, NULL, 0);
    EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
    EXPECT_EQ(0x00, frame[1]);
    EXPECT_EQ(0x00, frame[2]);
}

// --- vscp_writeReplyToFrame tests ---

TEST(VscpHelper, writeReplyToFrame_basic)
{
    uint8_t frame[64];
    memset(frame, 0, sizeof(frame));
    uint8_t arg[] = {0xDE, 0xAD};
    uint16_t command = 0x5678;
    uint16_t error   = 0x0001;

    int rv = vscp_writeReplyToFrame(frame, sizeof(frame), command, error, arg, sizeof(arg));
    EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);

    // Packet type: response | no encryption
    EXPECT_EQ(VSCP_BINARY_PACKET_TYPE_RESPONSE | VSCP_ENCRYPTION_NONE, frame[0]);

    // Command MSB/LSB
    EXPECT_EQ(0x56, frame[1]);
    EXPECT_EQ(0x78, frame[2]);

    // Error MSB/LSB
    EXPECT_EQ(0x00, frame[3]);
    EXPECT_EQ(0x01, frame[4]);

    // Argument data
    EXPECT_EQ(0xDE, frame[5]);
    EXPECT_EQ(0xAD, frame[6]);
}

TEST(VscpHelper, writeReplyToFrame_no_args)
{
    uint8_t frame[64];
    memset(frame, 0, sizeof(frame));
    uint16_t command = 0x0010;
    uint16_t error   = 0x0000;

    int rv = vscp_writeReplyToFrame(frame, sizeof(frame), command, error, NULL, 0);
    EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);

    // Packet type
    EXPECT_EQ(VSCP_BINARY_PACKET_TYPE_RESPONSE | VSCP_ENCRYPTION_NONE, frame[0]);

    // Command
    EXPECT_EQ(0x00, frame[1]);
    EXPECT_EQ(0x10, frame[2]);

    // Error
    EXPECT_EQ(0x00, frame[3]);
    EXPECT_EQ(0x00, frame[4]);
}

TEST(VscpHelper, writeReplyToFrame_null_frame)
{
    uint8_t arg[] = {0x01};
    int rv = vscp_writeReplyToFrame(NULL, 64, 0x0001, 0x0000, arg, sizeof(arg));
    EXPECT_EQ(VSCP_ERROR_PARAMETER, rv);
}

TEST(VscpHelper, writeReplyToFrame_null_arg_with_nonzero_len)
{
    uint8_t frame[64];
    int rv = vscp_writeReplyToFrame(frame, sizeof(frame), 0x0001, 0x0000, NULL, 5);
    EXPECT_EQ(VSCP_ERROR_PARAMETER, rv);
}

TEST(VscpHelper, writeReplyToFrame_buffer_too_small)
{
    uint8_t frame[6];  // Too small: need 1 + 2 + 2 + 0 + 2 = 7
    int rv = vscp_writeReplyToFrame(frame, sizeof(frame), 0x0001, 0x0000, NULL, 0);
    EXPECT_EQ(VSCP_ERROR_BUFFER_TO_SMALL, rv);
}

TEST(VscpHelper, writeReplyToFrame_buffer_exact_size)
{
    uint8_t frame[7];  // Exact: 1 pkttype + 2 cmd + 2 err + 0 arg + 2 crc
    memset(frame, 0, sizeof(frame));
    int rv = vscp_writeReplyToFrame(frame, sizeof(frame), 0x00FF, 0x0002, NULL, 0);
    EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
    EXPECT_EQ(VSCP_BINARY_PACKET_TYPE_RESPONSE | VSCP_ENCRYPTION_NONE, frame[0]);
    EXPECT_EQ(0x00, frame[1]);
    EXPECT_EQ(0xFF, frame[2]);
    EXPECT_EQ(0x00, frame[3]);
    EXPECT_EQ(0x02, frame[4]);
}

TEST(VscpHelper, writeReplyToFrame_large_args)
{
    uint8_t frame[512];
    memset(frame, 0, sizeof(frame));
    uint8_t arg[256];
    for (int i = 0; i < 256; i++) arg[i] = (uint8_t) i;

    int rv = vscp_writeReplyToFrame(frame, sizeof(frame), 0xABCD, 0x0000, arg, sizeof(arg));
    EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);

    // Verify packet type
    EXPECT_EQ(VSCP_BINARY_PACKET_TYPE_RESPONSE | VSCP_ENCRYPTION_NONE, frame[0]);

    // Verify command
    EXPECT_EQ(0xAB, frame[1]);
    EXPECT_EQ(0xCD, frame[2]);

    // Verify error
    EXPECT_EQ(0x00, frame[3]);
    EXPECT_EQ(0x00, frame[4]);

    // Verify argument data integrity
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ((uint8_t) i, frame[5 + i]);
    }
}

TEST(VscpHelper, writeCommandToFrame_large_args)
{
    uint8_t frame[512];
    memset(frame, 0, sizeof(frame));
    uint8_t arg[256];
    for (int i = 0; i < 256; i++) arg[i] = (uint8_t)(255 - i);

    int rv = vscp_writeCommandToFrame(frame, sizeof(frame), 0xBEEF, arg, sizeof(arg));
    EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);

    // Verify packet type
    EXPECT_EQ(VSCP_BINARY_PACKET_TYPE_COMMAND | VSCP_ENCRYPTION_NONE, frame[0]);

    // Verify command
    EXPECT_EQ(0xBE, frame[1]);
    EXPECT_EQ(0xEF, frame[2]);

    // Verify argument data integrity
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ((uint8_t)(255 - i), frame[3 + i]);
    }
}

static size_t runFrameEncryptDecrypt(uint8_t *encrypted,
                                     uint8_t *decrypted,
                                     uint8_t *frame,
                                     size_t plain_len,
                                     uint8_t *key,
                                     uint8_t *iv,
                                     bool useOpenSSL)
{
    vscp_setFrameEncryptionUseOpenSSL(useOpenSSL);

    size_t enc_len = vscp_encryptFrame(encrypted,
                                       frame,
                                       plain_len,
                                       key,
                                       iv,
                                       VSCP_ENCRYPTION_FROM_TYPE_BYTE);
    EXPECT_GT(enc_len, 0U);
    if (!enc_len) {
        return 0;
    }

    EXPECT_TRUE(vscp_decryptFrame(decrypted,
                                  encrypted,
                                  enc_len,
                                  key,
                                  nullptr,
                                  VSCP_ENCRYPTION_FROM_TYPE_BYTE));

    // Basic frame invariants expected from both backends.
    EXPECT_EQ(frame[0], encrypted[0]);
    EXPECT_EQ(frame[0], decrypted[0]);
    EXPECT_EQ(0, memcmp(encrypted + enc_len - 16, iv, 16));

    return enc_len;
}

TEST(VscpHelper, frameCryptoBackendSwitch_Aes128RoundtripAndParity)
{
    uint8_t key[16] = {
        0x10, 0x21, 0x32, 0x43, 0x54, 0x65, 0x76, 0x87,
        0x98, 0xA9, 0xBA, 0xCB, 0xDC, 0xED, 0xFE, 0x0F
    };

    uint8_t iv[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE
    };

    // Keep packet buffer larger than plaintext len to match legacy padding read behavior.
    uint8_t frame[80];
    memset(frame, 0, sizeof(frame));
    frame[0] = VSCP_BINARY_PACKET_TYPE_EVENT | VSCP_ENCRYPTION_AES128;
    for (int i = 1; i < 33; i++) {
        frame[i] = (uint8_t) i;
    }

    const size_t plain_len = 33;

    uint8_t enc_internal[128];
    uint8_t enc_openssl[128];
    uint8_t dec_internal[128];
    uint8_t dec_openssl[128];
    memset(enc_internal, 0, sizeof(enc_internal));
    memset(enc_openssl, 0, sizeof(enc_openssl));
    memset(dec_internal, 0, sizeof(dec_internal));
    memset(dec_openssl, 0, sizeof(dec_openssl));

    size_t enc_len_internal = runFrameEncryptDecrypt(enc_internal,
                                                     dec_internal,
                                                     frame,
                                                     plain_len,
                                                     key,
                                                     iv,
                                                     false);

    size_t enc_len_openssl = runFrameEncryptDecrypt(enc_openssl,
                                                    dec_openssl,
                                                    frame,
                                                    plain_len,
                                                    key,
                                                    iv,
                                                    true);

    // Both backends should produce a valid encrypted frame size.
    EXPECT_EQ(enc_len_internal, enc_len_openssl);
    EXPECT_GT(enc_len_internal, plain_len);

    // Restore default behavior for subsequent tests.
    vscp_setFrameEncryptionUseOpenSSL(false);
}

// Entry point for Google Test
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
