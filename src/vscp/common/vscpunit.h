// vscpunit.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2023 Ake Hedman, the VSCP project
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
//

#if !defined(VSCP_UNIT_H__INCLUDED_)
#define VSCP_UNIT_H__INCLUDED_

#include <inttypes.h>

#include <string>

class CVscpUnit {

 public:
 	CVscpUnit(int unit = -1);
 	~CVscpUnit();
 	
    /// Unit code 0-3 for Level I, 0-255 for Level II
 	int m_unit;

    /// VSCP class
 	uint16_t m_vscp_class;

    /// VSCP type 
 	uint16_t m_vscp_type;
 	
 	/// Unit name
 	std::string m_name;
 	
 	/// Description of unit
 	std::string m_description;
 	
 	/// Conversion of this value to unit=0 value
 	std::string m_conversion0;
 	
 	/// Conversion from value in unit=0 to this unit
 	std::string m_conversion;
 	
 	/// ASCII representation of symbol for this unit
 	std::string m_symbol_ascii;
 	
 	/// Unicode representation of symbol for this unit
 	std::string m_symbol_utf8; 	
};


#endif  // VSCP_UNIT_H__INCLUDED_