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
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifdef _WIN32
#include <io.h>
#define access _access_s
#else
#include <unistd.h>
#endif

#include <stdexcept>

#include <vscphelper.h>

///////////////////////////////////////////////////////////////////////////////
// vscp_readStringValue

int64_t
vscp_readStringValue(const std::string &strval)
{
  int64_t val     = 0;
  std::string str = strval;
  vscp_makeLower(str);
  vscp_trim(str);

  try {
    std::size_t pos;
    if (std::string::npos != (pos = str.find("0x"))) {
      str = str.substr(2);
      val = std::stoll(str, &pos, 16);
    }
    else if (std::string::npos != (pos = str.find("0o"))) {
      str = str.substr(2);
      val = std::stoll(str, &pos, 8);
    }
    else if (std::string::npos != (pos = str.find("0b"))) {
      str = str.substr(2);
      val = std::stoll(str, &pos, 2);
    }
    else {
      val = std::stoll(str);
    }
  }
  catch (std::invalid_argument &) {
    val = 0;
  }
  catch (std::out_of_range &) {
    val = 0;
  }

  return val;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_fileExists
//

bool
vscp_fileExists(const std::string &path)
{
  return access(path.c_str(), 0) == 0;
}
