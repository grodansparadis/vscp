// vscpunitexpr.h
//
// Measurement unit conversion from expressions loaded at runtime.
//
// This is the expression based alternative to the compiled unit table
// (vscp_convertMeasurementUnit() in vscphelper). The conversions are read
// from vscp-units.json (made by vscp-classes, script/make_vscphelper_units.py)
// or from the full vscp-hashtypeevents.json and evaluated when a value is
// converted, so new or changed units work without rebuilding.
//
// Each unit has two expressions:
//   conversion0  value in the unit -> value in unit 0
//   conversion   value in unit 0   -> value in the unit
// where {{{val}}} (or {{val}}) is the value. Expressions may only hold
// numbers (also 1e-4 form), + - * /, unary + -, parentheses and the value.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2026 Ake Hedman, the VSCP project
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

#ifndef VSCP_UNIT_EXPR_H
#define VSCP_UNIT_EXPR_H

#include <cstdint>
#include <map>
#include <string>
#include <utility>

/*!
  Evaluate a unit conversion expression.
  @param presult Set to the result.
  @param expr Expression, for example "({{{val}}} + 459.67) * 5/9".
  @param value Value used for {{{val}}}.
  @return VSCP_ERROR_SUCCESS, VSCP_ERROR_INVALID_POINTER, VSCP_ERROR_INVALID_SYNTAX
    if the expression is not valid, or VSCP_ERROR_INVALID_FORMAT if the
    result is not a finite number (for example division by zero).
*/
int
vscp_evalUnitExpression(double *presult, const std::string &expr, double value);

/*!
  Measurement units with conversion expressions, loaded from JSON.
*/
class CVscpUnitExpressions {

public:
  /// One unit
  struct Unit {
    std::string name;
    std::string description;
    std::string symbol_ascii;
    std::string symbol_utf8;
    bool convertible = true;
    std::string conversion0; ///< Unit -> unit 0
    std::string conversion;  ///< Unit 0 -> unit
  };

  /*!
    Load units from a file: vscp-units.json, or vscp-hashtypeevents.json
    (the units of CLASS1.MEASUREMENT are used). Replaces the loaded units
    only when the whole file was read.
    @return VSCP_ERROR_SUCCESS, VSCP_ERROR_INVALID_PATH if the file can not
      be read, or VSCP_ERROR_INVALID_FORMAT / VSCP_ERROR_INVALID_SYNTAX if
      the content or an expression is not valid. getLastError() tells why.
  */
  int loadFile(const std::string &path);

  /// As loadFile() for JSON text
  int loadString(const std::string &json);

  /// Number of loaded units
  size_t size() const { return m_units.size(); }

  /// Get a unit. Returns nullptr if it is not loaded.
  const Unit *getUnit(uint16_t vscp_type, uint8_t unit) const;

  /*!
    Convert a value from one unit to another through unit 0.
    @return VSCP_ERROR_SUCCESS, VSCP_ERROR_INVALID_POINTER, VSCP_ERROR_UNKNOWN_ITEM
      if the type or a unit is not loaded, VSCP_ERROR_NOT_SUPPORTED if a
      unit can not be converted, or an error from vscp_evalUnitExpression().
  */
  int convert(double *presult, uint16_t vscp_type, uint8_t from_unit, uint8_t to_unit, double value) const;

  /// Why the last load failed
  const std::string &getLastError() const { return m_lastError; }

private:
  std::map<std::pair<uint16_t, uint8_t>, Unit> m_units;
  std::string m_lastError;
};

#endif // VSCP_UNIT_EXPR_H
