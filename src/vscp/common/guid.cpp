///////////////////////////////////////////////////////////////////////////////
// guid.cpp:
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

#ifdef __GNUG__
//#pragma implementation
#endif

#ifdef WIN32
#include <pch.h>
#endif

#include <algorithm>
#include <cctype>
#include <deque>
#include <functional>
#include <locale>
#include <memory>
#include <string>
#include <vector>

#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include "guid.h"
#include "vscphelper.h"

// Helper functions for GUID parsing (inline in implementation)
namespace {

inline bool isHexDigit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

inline uint8_t hexToVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

inline int countHexDigits(const char *p) {
    int count = 0;
    while (isHexDigit(*p)) {
        count++;
        p++;
    }
    return count;
}

inline uint32_t parseHexValue(const char **pp, int maxDigits) {
    uint32_t value = 0;
    int count = 0;
    const char *p = *pp;
    while (isHexDigit(*p) && count < maxDigits) {
        value = (value << 4) | hexToVal(*p);
        p++;
        count++;
    }
    *pp = p;
    return value;
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cguid::cguid()
{
    clear();
}

cguid::cguid(const uint8_t *pguid)
{
    memcpy(m_guid, pguid, sizeof(m_guid));
}

cguid::cguid(const cguid &guid)
{
    memcpy(m_guid, guid.m_guid, sizeof(m_guid));
}

cguid::cguid(const std::string &strGUID) {
    getFromString(strGUID);
}

cguid::~cguid()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// operator=
//

cguid &
cguid::operator=(const cguid &guid)
{
    // Check for self-assignment!
    if (this == &guid) { // Same object?
        return *this;    // Yes, so skip assignment, and just return *this.
    }

    memcpy(m_guid, guid.m_guid, sizeof(m_guid));

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
// operator==
//

bool
cguid::operator==(const cguid &guid)
{
    if (0 != memcmp(m_guid, guid.m_guid, 16)) return false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// operator!=
//

bool
cguid::operator!=(const cguid &guid)
{
    return !(*this == guid);
}

///////////////////////////////////////////////////////////////////////////////
// operator[] - a[i] 
//

uint8_t
cguid::operator[](uint8_t pos) const
{
    return m_guid[pos & 0x0f];
}

///////////////////////////////////////////////////////////////////////////////
// operator[] - a[i] = x
//

uint8_t& 
cguid::operator[](uint8_t pos)
{
    return m_guid[pos & 0x0f];
}

///////////////////////////////////////////////////////////////////////////////
// getFromString
//

void
cguid::getFromString(const std::string &strGUID)
{
    (void)vscp_guid_parse(m_guid, strGUID.c_str(), nullptr);
}

///////////////////////////////////////////////////////////////////////////////
// vscp_guid_parse
//

int
cguid::vscp_guid_parse(uint8_t *guid, const char *strguid, char **endptr)
{
    if (nullptr == guid || nullptr == strguid) {
        return VSCP_ERROR_INVALID_POINTER;
    }

    memset(guid, 0, 16);
    const char *p = strguid;
    bool hasBraces = false;
    int guidIdx = 0;

    auto finish = [&]() {
        if (hasBraces) {
            while (*p == ' ' || *p == '\t') p++;
            if (*p == '}') p++;
        }
        if (endptr) *endptr = const_cast<char *>(p);
        return VSCP_ERROR_SUCCESS;
    };

    while (*p == ' ' || *p == '\t') p++;
    if (*p == '{') {
        hasBraces = true;
        p++;
        while (*p == ' ' || *p == '\t') p++;
    }
    if (!*p) return finish();

    auto parseGroups = [](const char *&input, uint8_t *output, int capacity) {
        int count = 0;
        while (*input && count < capacity && isHexDigit(*input)) {
            int hexLen = countHexDigits(input);
            if (hexLen <= 2) {
                output[count++] = (uint8_t)parseHexValue(&input, 2);
            }
            else if (hexLen <= 4) {
                uint16_t value = (uint16_t)parseHexValue(&input, 4);
                output[count++] = (value >> 8) & 0xFF;
                if (count < capacity) output[count++] = value & 0xFF;
            }
            else {
                int bytesToParse = (hexLen + 1) / 2;
                if (bytesToParse > capacity - count) bytesToParse = capacity - count;
                for (int i = 0; i < bytesToParse; i++) {
                    uint8_t high = hexToVal(*input++);
                    uint8_t low = isHexDigit(*input) ? hexToVal(*input++) : 0;
                    output[count++] = (high << 4) | low;
                }
            }
            if (count < capacity && (*input == ':' || *input == '-' || *input == ',')) input++;
        }
        return count;
    };

    if (*p == '-') {
        if (p[1] == ':') {
            p += 2;
            uint8_t trailing[16] = {};
            int trailingCount = parseGroups(p, trailing, 16);
            memcpy(guid + 16 - trailingCount, trailing, trailingCount);
            return finish();
        }
        if (!p[1] || !isHexDigit(p[1])) {
            p++;
            return finish();
        }
    }

    if ((p[0] == ':' && p[1] == ':') || (p[0] == '*' && p[1] == ':')) {
        bool allFF = !p[2] || !isHexDigit(p[2]);
        if (allFF) {
            memset(guid, 0xFF, 16);
            p += 2;
            return finish();
        }
        p += 2;
        uint8_t trailing[16] = {};
        int trailingCount = parseGroups(p, trailing, 16);
        int ffCount = 16 - trailingCount;
        memset(guid, 0xFF, ffCount);
        memcpy(guid + ffCount, trailing, trailingCount);
        return finish();
    }

    const char *middle = strstr(p, "::");
    if (middle && middle != p) {
        auto countParsedBytes = [](const char *value) {
            int count = 0;
            while (*value) {
                if (!isHexDigit(*value)) {
                    value++;
                    continue;
                }
                int hexLen = countHexDigits(value);
                count += hexLen <= 2 ? 1 : (hexLen <= 4 ? 2 : (hexLen + 1) / 2);
                value += hexLen;
            }
            return count;
        };
        std::string prefix(p, middle - p);
        std::string suffix(middle + 2);
        uint8_t prefixGuid[16] = {};
        uint8_t suffixGuid[16] = {};
        (void)vscp_guid_parse(prefixGuid, prefix.c_str(), nullptr);
        (void)vscp_guid_parse(suffixGuid, suffix.c_str(), nullptr);
        int prefixCount = (std::min)(countParsedBytes(prefix.c_str()), 16);
        int suffixCount = (std::min)(countParsedBytes(suffix.c_str()), 16 - prefixCount);
        memcpy(guid, prefixGuid, prefixCount);
        memcpy(guid + 16 - suffixCount, suffixGuid, suffixCount);
        return finish();
    }

    while (*p && guidIdx < 16 && isHexDigit(*p)) {
        int hexLen = countHexDigits(p);
        if (hexLen <= 2) {
            guid[guidIdx++] = (uint8_t)parseHexValue(&p, 2);
        }
        else if (hexLen <= 4) {
            uint16_t value = (uint16_t)parseHexValue(&p, 4);
            if (guidIdx + 1 < 16) {
                guid[guidIdx++] = (value >> 8) & 0xFF;
                guid[guidIdx++] = value & 0xFF;
            }
            else {
                guid[guidIdx++] = value & 0xFF;
            }
        }
        else {
            int bytesToParse = (hexLen + 1) / 2;
            if (bytesToParse > 16 - guidIdx) bytesToParse = 16 - guidIdx;
            for (int i = 0; i < bytesToParse; i++) {
                uint8_t high = hexToVal(*p++);
                uint8_t low = isHexDigit(*p) ? hexToVal(*p++) : 0;
                guid[guidIdx++] = (high << 4) | low;
            }
        }
        if (guidIdx < 16 && (*p == ':' || *p == '-' || *p == ',')) p++;
    }

    if (guidIdx != 16) return VSCP_ERROR_INVALID_SYNTAX;
    return finish();
}

///////////////////////////////////////////////////////////////////////////////
// getFromString
//

void
cguid::getFromString(const char *pszGUID)
{
    std::string str;
    str = std::string(pszGUID);
    getFromString(str);
}

///////////////////////////////////////////////////////////////////////////////
// getFromArray
//

void
cguid::getFromArray(const uint8_t *pguid)
{
    memcpy(m_guid, pguid, 16);
}

///////////////////////////////////////////////////////////////////////////////
// toString
//

void
cguid::toString(std::string &strGUID)
{
    char buf[48];
    snprintf(buf, sizeof(buf),
             "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
             m_guid[0], m_guid[1], m_guid[2], m_guid[3],
             m_guid[4], m_guid[5], m_guid[6], m_guid[7],
             m_guid[8], m_guid[9], m_guid[10], m_guid[11],
             m_guid[12], m_guid[13], m_guid[14], m_guid[15]);
    strGUID = buf;
}

///////////////////////////////////////////////////////////////////////////////
// toString
//

std::string
cguid::toString(void)
{
    std::string str;
    toString(str);
    return str;
}

///////////////////////////////////////////////////////////////////////////////
// toStringCompact
//

void
cguid::toStringCompact(std::string &strGUID)
{
    // Count leading 0xFF bytes
    int ffCount = 0;
    while (ffCount < 16 && m_guid[ffCount] == 0xFF) {
        ffCount++;
    }

    // All 0xFF
    if (ffCount == 16) {
        strGUID = "::";
        return;
    }

    // No leading 0xFF bytes - use standard format
    if (ffCount == 0) {
        toString(strGUID);
        return;
    }

    // Some leading 0xFF bytes - use :: notation
    strGUID = "::";
    for (int i = ffCount; i < 16; i++) {
        if (i > ffCount) {
            strGUID += ":";
        }
        char buf[4];
        snprintf(buf, sizeof(buf), "%02X", m_guid[i]);
        strGUID += buf;
    }
}

///////////////////////////////////////////////////////////////////////////////
// toStringCompact
//

std::string
cguid::toStringCompact(void)
{
    std::string str;
    toStringCompact(str);
    return str;
}

///////////////////////////////////////////////////////////////////////////////
// toStringUUID
//

void
cguid::toStringUUID(std::string &strGUID)
{
    // Format: 8-4-4-4-12 hex digits with dashes
    // FFFFFFFF-FFFF-FFFF-0102-03AABB440130
    char buf[37];
    snprintf(buf, sizeof(buf),
             "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
             m_guid[0], m_guid[1], m_guid[2], m_guid[3],
             m_guid[4], m_guid[5],
             m_guid[6], m_guid[7],
             m_guid[8], m_guid[9],
             m_guid[10], m_guid[11], m_guid[12], m_guid[13], m_guid[14], m_guid[15]);
    strGUID = buf;
}

///////////////////////////////////////////////////////////////////////////////
// toStringUUID
//

std::string
cguid::toStringUUID(void)
{
    std::string str;
    toStringUUID(str);
    return str;
}

///////////////////////////////////////////////////////////////////////////////
// isSameGUID
//

bool
cguid::isSameGUID(const unsigned char *pguid)
{
    if (NULL == pguid) return false;
    if (0 != memcmp(m_guid, pguid, 16)) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isNULL
//

bool
cguid::isNULL(void)
{
    for (int i = 0; i < 16; i++) {
        if (m_guid[i]) return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// writeGUID
//

void
cguid::writeGUID(uint8_t *pArray)
{
    // Check pointer
    if (NULL == pArray) return;

    memcpy(pArray, m_guid, 16);
}

///////////////////////////////////////////////////////////////////////////////
// reverse
//

void
cguid::reverse(void)
{
    uint8_t tmp[16];
    for (int i = 0; i < 16; i++) {
        tmp[15 - i] = m_guid[i];
    }
    memcpy(m_guid, tmp, 16);
}

///////////////////////////////////////////////////////////////////////////////
// writeGUID_reverse
//

void
cguid::writeGUID_reverse(uint8_t *pArray)
{
    // Check pointer
    if (NULL == pArray) return;

    for (int i = 0; i < 16; i++) {
        pArray[15 - i] = m_guid[i];
    }
}

///////////////////////////////////////////////////////////////////////////////
// setClientID
//

void
cguid::setClientID(uint16_t clientid)
{
    m_guid[12] = (clientid >> 8) & 0xff;
    m_guid[13] = clientid & 0xff;
}

///////////////////////////////////////////////////////////////////////////////
// getClientID
//

uint16_t
cguid::getClientID(void)
{
    return (uint16_t)(m_guid[12] << 8) + m_guid[13];
}

///////////////////////////////////////////////////////////////////////////////
// setNicknameID
//

void
cguid::setNicknameID(uint16_t nicknameid)
{
    m_guid[14] = (nicknameid >> 8) & 0xff;
    m_guid[15] = nicknameid & 0xff;
}
