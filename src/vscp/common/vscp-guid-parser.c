/*
  File: vscp-guid-parser.c

  VSCP GUID string formatters

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
#include <string.h>

#include <vscp.h>

#include "vscp-guid-parser.h"

int
vscp_guid_to_string(char *strguid, const uint8_t *guid)
{
  if (NULL == strguid || NULL == guid) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  sprintf(strguid,
          "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:"
          "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
          guid[0], guid[1], guid[2], guid[3], guid[4], guid[5], guid[6], guid[7],
          guid[8], guid[9], guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]);

  return VSCP_ERROR_SUCCESS;
}

int
vscp_guid_to_string_compact(char *strguid, const uint8_t *guid)
{
  int ff_count = 0;

  if (NULL == strguid || NULL == guid) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  while (ff_count < 16 && guid[ff_count] == 0xFF) {
    ff_count++;
  }

  if (ff_count == 16) {
    strcpy(strguid, "::");
    return VSCP_ERROR_SUCCESS;
  }

  if (ff_count == 0) {
    return vscp_guid_to_string(strguid, guid);
  }

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

int
vscp_guid_to_string_uuid(char *strguid, const uint8_t *guid)
{
  if (NULL == strguid || NULL == guid) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  sprintf(strguid,
          "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
          guid[0], guid[1], guid[2], guid[3],
          guid[4], guid[5],
          guid[6], guid[7],
          guid[8], guid[9],
          guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]);

  return VSCP_ERROR_SUCCESS;
}
