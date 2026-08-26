/*
  File: vscp-guid-parser.c

  VSCP GUID Parser - Extended format support

  This file is part of the VSCP (https://www.vscp.org)

  The MIT License (MIT)
  Copyright (C) 2025-2026 Ake Hedman, the VSCP project <info@vscp.org>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <vscp.h>

#include "vscp-guid-parser.h"

///////////////////////////////////////////////////////////////////////////////
// Helper: Check if character is a hex digit
//

static inline int
is_hex_digit(char c)
{
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

///////////////////////////////////////////////////////////////////////////////
// Helper: Convert hex character to value
//

static inline uint8_t
hex_to_val(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Helper: Count consecutive hex digits
//

static int
count_hex_digits(const char *p)
{
  int count = 0;
  while (is_hex_digit(*p)) {
    count++;
    p++;
  }
  return count;
}

///////////////////////////////////////////////////////////////////////////////
// Helper: Parse hex value of up to max_digits
//

static uint32_t
parse_hex_value(const char **pp, int max_digits)
{
  uint32_t value = 0;
  int count      = 0;
  const char *p  = *pp;

  while (is_hex_digit(*p) && count < max_digits) {
    value = (value << 4) | hex_to_val(*p);
    p++;
    count++;
  }
  *pp = p;
  return value;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_guid_parse
//

int
vscp_guid_parse(uint8_t *guid, const char *strguid, char **endptr)
{
  const char *p = strguid;
  int guid_idx  = 0;
  int has_braces = 0;

  // Check pointers
  if (NULL == strguid) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if (NULL == guid) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  // Initialize GUID to zeros
  memset(guid, 0, 16);

  // Skip leading whitespace
  while (*p && (*p == ' ' || *p == '\t')) {
    p++;
  }

  // Check for opening brace
  if (*p == '{') {
    has_braces = 1;
    p++;
    // Skip whitespace after opening brace
    while (*p && (*p == ' ' || *p == '\t')) {
      p++;
    }
  }

  // Empty string or just whitespace
  if (!*p) {
    if (endptr)
      *endptr = (char *) p;
    return VSCP_ERROR_SUCCESS;
  }

  // Special case: "-" means all zeros, "-:" means leading zeros with trailing values
  if (*p == '-') {
    // Check for "-:" prefix (leading zeros with trailing values)
    if (*(p + 1) == ':') {
      p += 2; // Skip "-:"

      // Parse the remaining bytes into a temporary buffer
      uint8_t temp_bytes[16];
      int temp_count = 0;

      while (*p && temp_count < 16) {
        if (!is_hex_digit(*p)) {
          break;
        }

        int hex_len = count_hex_digits(p);

        if (hex_len <= 2) {
          // Single byte value
          temp_bytes[temp_count++] = (uint8_t) parse_hex_value(&p, 2);
        }
        else if (hex_len <= 4) {
          // Two byte value (big endian)
          uint16_t val = (uint16_t) parse_hex_value(&p, 4);
          temp_bytes[temp_count++] = (val >> 8) & 0xFF;
          temp_bytes[temp_count++] = val & 0xFF;
        }
        else {
          // Up to 4 bytes (8 hex digits)
          int bytes_to_parse = (hex_len + 1) / 2;
          if (bytes_to_parse > 4)
            bytes_to_parse = 4;

          uint32_t val = (uint32_t) parse_hex_value(&p, bytes_to_parse * 2);

          // Store bytes in big-endian order
          for (int i = bytes_to_parse - 1; i >= 0 && temp_count < 16; i--) {
            temp_bytes[temp_count++] = (val >> (i * 8)) & 0xFF;
          }
        }

        // Skip separator if present (colon, dash, or comma)
        if (*p == ':' || *p == '-' || *p == ',') {
          p++;
        }
      }

      // GUID already initialized to zeros, copy temp bytes to end
      int zero_count = 16 - temp_count;
      memcpy(guid + zero_count, temp_bytes, temp_count);

      // Skip closing brace if we had an opening one
      if (has_braces) {
        while (*p && (*p == ' ' || *p == '\t')) {
          p++;
        }
        if (*p == '}') {
          p++;
        }
      }

      if (endptr)
        *endptr = (char *) p;
      return VSCP_ERROR_SUCCESS;
    }

    // "-" alone (not followed by hex) means all zeros
    if (!*(p + 1) || !is_hex_digit(*(p + 1))) {
      p++;
      // Skip closing brace if we had an opening one
      if (has_braces) {
        while (*p && (*p == ' ' || *p == '\t')) {
          p++;
        }
        if (*p == '}') {
          p++;
        }
      }
      if (endptr)
        *endptr = (char *) p;
      return VSCP_ERROR_SUCCESS;
    }
  }

  // Special case: "::" at start
  if (p[0] == ':' && p[1] == ':') {
    // "::" alone means all 0xFF
    if (!p[2] || !is_hex_digit(p[2])) {
      memset(guid, 0xFF, 16);
      p += 2;
      // Skip closing brace if we had an opening one
      if (has_braces) {
        while (*p && (*p == ' ' || *p == '\t')) {
          p++;
        }
        if (*p == '}') {
          p++;
        }
      }
      if (endptr)
        *endptr = (char *) p;
      return VSCP_ERROR_SUCCESS;
    }

    // "::" followed by hex means fill leading bytes with 0xFF
    p += 2;

    // Parse the remaining bytes into a temporary buffer
    uint8_t temp_bytes[16];
    int temp_count = 0;

    while (*p && temp_count < 16) {
      if (!is_hex_digit(*p)) {
        break;
      }

      int hex_len = count_hex_digits(p);

      if (hex_len <= 2) {
        // Single byte value
        temp_bytes[temp_count++] = (uint8_t) parse_hex_value(&p, 2);
      }
      else if (hex_len <= 4) {
        // Two byte value (big endian)
        uint16_t val = (uint16_t) parse_hex_value(&p, 4);
        temp_bytes[temp_count++] = (val >> 8) & 0xFF;
        temp_bytes[temp_count++] = val & 0xFF;
      }
      else {
        // Up to 4 bytes (8 hex digits)
        int bytes_to_parse = (hex_len + 1) / 2;
        if (bytes_to_parse > 4)
          bytes_to_parse = 4;

        uint32_t val = (uint32_t) parse_hex_value(&p, bytes_to_parse * 2);

        // Store bytes in big-endian order
        for (int i = bytes_to_parse - 1; i >= 0 && temp_count < 16; i--) {
          temp_bytes[temp_count++] = (val >> (i * 8)) & 0xFF;
        }
      }

      // Skip separator if present
      if (*p == ':' || *p == '-' || *p == ',') {
        p++;
      }
    }

    // Fill leading bytes with 0xFF, then copy temp bytes to end
    int ff_count = 16 - temp_count;
    if (ff_count > 0) {
      memset(guid, 0xFF, ff_count);
    }
    memcpy(guid + ff_count, temp_bytes, temp_count);

    // Skip closing brace if we had an opening one
    if (has_braces) {
      while (*p && (*p == ' ' || *p == '\t')) {
        p++;
      }
      if (*p == '}') {
        p++;
      }
    }

    if (endptr)
      *endptr = (char *) p;
    return VSCP_ERROR_SUCCESS;
  }

  // Parse hex groups separated by ':', '-', or ','. Group sizes are unrestricted.
  // Parse bytes one group at a time
  while (*p && guid_idx < 16) {
    if (!is_hex_digit(*p)) {
      break;
    }

    int hex_len = count_hex_digits(p);

    if (hex_len <= 2) {
      // Single byte value (1-2 hex digits)
      guid[guid_idx++] = (uint8_t) parse_hex_value(&p, 2);
    }
    else if (hex_len <= 4) {
      // Two bytes (3-4 hex digits) - big endian
      uint16_t val = (uint16_t) parse_hex_value(&p, 4);
      if (guid_idx + 1 < 16) {
        guid[guid_idx++] = (val >> 8) & 0xFF;
        guid[guid_idx++] = val & 0xFF;
      }
      else {
        guid[guid_idx++] = val & 0xFF;
      }
    }
    else {
      // More than 4 hex digits - parse as multiple bytes
      int bytes_to_parse = (hex_len + 1) / 2;
      if (bytes_to_parse > (16 - guid_idx)) {
        bytes_to_parse = 16 - guid_idx;
      }

      for (int i = 0; i < bytes_to_parse && guid_idx < 16; i++) {
        uint8_t hi       = hex_to_val(*p++);
        uint8_t lo       = is_hex_digit(*p) ? hex_to_val(*p++) : 0;
        guid[guid_idx++] = (hi << 4) | lo;
      }
    }

    // Skip separator only if we haven't completed the GUID
    // (don't consume trailing comma which may be part of outer format)
    if (guid_idx < 16 && (*p == ':' || *p == '-' || *p == ',')) {
      p++;
    }
  }

  // If no bytes were parsed in standard format, return error
  // (special cases like "-", "::", "-:", etc. return earlier)
  if (guid_idx == 0) {
    return VSCP_ERROR_INVALID_SYNTAX;
  }

  // Skip closing brace if we had an opening one
  if (has_braces) {
    while (*p && (*p == ' ' || *p == '\t')) {
      p++;
    }
    if (*p == '}') {
      p++;
    }
  }

  if (endptr)
    *endptr = (char *) p;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_guid_to_string
//

int
vscp_guid_to_string(char *strguid, const uint8_t *guid)
{
  if (NULL == strguid) {
    return VSCP_ERROR_INVALID_POINTER;
  }
  if (NULL == guid) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  sprintf(strguid,
          "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:"
          "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
          guid[0], guid[1], guid[2], guid[3], guid[4], guid[5], guid[6], guid[7],
          guid[8], guid[9], guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_guid_to_string_compact
//

int
vscp_guid_to_string_compact(char *strguid, const uint8_t *guid)
{
  int ff_count = 0;

  if (NULL == strguid) {
    return VSCP_ERROR_INVALID_POINTER;
  }
  if (NULL == guid) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  // Count leading 0xFF bytes
  while (ff_count < 16 && guid[ff_count] == 0xFF) {
    ff_count++;
  }

  // All 0xFF
  if (ff_count == 16) {
    strcpy(strguid, "::");
    return VSCP_ERROR_SUCCESS;
  }

  // No leading 0xFF bytes - use standard format
  if (ff_count == 0) {
    return vscp_guid_to_string(strguid, guid);
  }

  // Some leading 0xFF bytes - use :: notation
  char *p = strguid;
  *p++    = ':';
  *p++    = ':';

  for (int i = ff_count; i < 16; i++) {
    if (i > ff_count) {
      *p++ = ':';
    }
    sprintf(p, "%02X", guid[i]);
    p += 2;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_guid_to_string_uuid
//

int
vscp_guid_to_string_uuid(char *strguid, const uint8_t *guid)
{
  if (NULL == strguid) {
    return VSCP_ERROR_INVALID_POINTER;
  }
  if (NULL == guid) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  // Format: 8-4-4-4-12 hex digits with dashes
  // FFFFFFFF-FFFF-FFFF-0102-03AABB440130
  sprintf(strguid,
          "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
          guid[0], guid[1], guid[2], guid[3],
          guid[4], guid[5],
          guid[6], guid[7],
          guid[8], guid[9],
          guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]);

  return VSCP_ERROR_SUCCESS;
}
