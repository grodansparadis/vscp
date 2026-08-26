/*
  File: vscp-guid-parser.h

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

#ifndef VSCP_GUID_PARSER_H
#define VSCP_GUID_PARSER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parse VSCP GUID from string with extended format support
 *
 * Supported formats:
 *
 * 1. Standard colon-separated (16 single-byte values):
 *    FF:FF:FF:FF:FF:FF:FF:FF:01:02:03:AA:BB:44:01:30
 *    (colons, dashes, and commas can be used as separators)
 *
 * 2. Double-colon prefix for leading 0xFF bytes:
 *    ::01:02:03:AA:BB:44:01:30
 *    (equivalent to FF:FF:FF:FF:FF:FF:FF:FF:01:02:03:AA:BB:44:01:30)
 *
 * 3. Grouped hex values (2-4 hex digits per group):
 *    ::0102:03aa:44:01:30
 *    ::0102:03aa:440130
 *
 * 4. Freely grouped hex values using colon, dash, or comma separators:
 *    FFFFFFFF-FFFF-FFFF-0102-03AABB440130
 *    FFFFFFFF-FFFF-FFFF-0102-03AABB44-0130
 *    FFFFFFFF:FFFF:FFFF:0102:03AABB440130
 *
 * 5. Special values:
 *    "-"  = All zeros (00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00)
 *    "::" = All 0xFF  (FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF)
 *
 * 6. Hyphen-colon prefix for leading zero bytes:
 *    -:01:02:03:AA:BB:44:01:30
 *    (equivalent to 00:00:00:00:00:00:00:00:01:02:03:AA:BB:44:01:30)
 *    -:1,2,3  (comma separators also supported)
 *
 * 7. Brace-enclosed formats (any of the above can be enclosed in {}):
 *    {FF:FF:FF:FF:FF:FF:FF:FF:01:02:03:AA:BB:44:01:30}
 *    {FFFFFFFF-FFFF-FFFF-0102-03AABB440130}
 *    {::01:02:03:AA:BB:44:01:30}
 *
 * @param guid    Pointer to 16-byte buffer to receive parsed GUID
 * @param strguid Input string containing GUID in any supported format
 * @param endptr  Optional pointer to receive position after parsed GUID (can be NULL)
 *
 * @return VSCP_ERROR_SUCCESS on success
 * @return VSCP_ERROR_INVALID_POINTER if guid or strguid is NULL
 * @return VSCP_ERROR_ERROR on parse error
 */
int vscp_guid_parse(uint8_t *guid, const char *strguid, char **endptr);

/**
 * @brief Write GUID to string in standard colon-separated format
 *
 * Output format: FF:FF:FF:FF:FF:FF:FF:FF:01:02:03:AA:BB:44:01:30
 *
 * @param strguid Buffer to receive string (must be at least 48 bytes)
 * @param guid    Pointer to 16-byte GUID
 *
 * @return VSCP_ERROR_SUCCESS on success
 * @return VSCP_ERROR_INVALID_POINTER if strguid or guid is NULL
 */
int vscp_guid_to_string(char *strguid, const uint8_t *guid);

/**
 * @brief Write GUID to string in compact format using :: notation
 *
 * If the GUID starts with consecutive 0xFF bytes, they are replaced with ::
 * Output examples:
 *   ::01:02:03:AA:BB:44:01:30  (if first 8 bytes are 0xFF)
 *   FF:FF:FF:FF:FF:FF:FF:FF:01:02:03:AA:BB:44:01:30 (if no leading 0xFF)
 *
 * @param strguid Buffer to receive string (must be at least 48 bytes)
 * @param guid    Pointer to 16-byte GUID
 *
 * @return VSCP_ERROR_SUCCESS on success
 * @return VSCP_ERROR_INVALID_POINTER if strguid or guid is NULL
 */
int vscp_guid_to_string_compact(char *strguid, const uint8_t *guid);

/**
 * @brief Write GUID to string in standard UUID format (8-4-4-4-12)
 *
 * Output format: FFFFFFFF-FFFF-FFFF-0102-03AABB440130
 *
 * @param strguid Buffer to receive string (must be at least 37 bytes)
 * @param guid    Pointer to 16-byte GUID
 *
 * @return VSCP_ERROR_SUCCESS on success
 * @return VSCP_ERROR_INVALID_POINTER if strguid or guid is NULL
 */
int vscp_guid_to_string_uuid(char *strguid, const uint8_t *guid);

#ifdef __cplusplus
}
#endif

#endif /* VSCP_GUID_PARSER_H */
