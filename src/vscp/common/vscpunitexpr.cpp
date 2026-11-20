// vscpunitexpr.cpp
//
// Measurement unit conversion from expressions loaded at runtime.
// See vscpunitexpr.h.
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

#include "vscpunitexpr.h"

#include <vscp.h>
#include <vscp-class.h>

#include <nlohmann/json.hpp>

#include <cctype>
#include <cmath>
#include <fstream>
#include <locale>
#include <sstream>

using json = nlohmann::json;

namespace {

// Recursive descent parser for
//   expr   := term (('+' | '-') term)*
//   term   := factor (('*' | '/') factor)*
//   factor := ('+' | '-') factor | number | value | '(' expr ')'
//   value  := '{{{val}}}' | '{{val}}'
class Parser {
public:
  Parser(const std::string &text, double value)
    : m_text(text)
    , m_value(value)
  {
  }

  bool parse(double &result)
  {
    if (!expr(result, 0)) {
      return false;
    }
    skipSpace();
    return m_pos == m_text.size();
  }

private:
  static const int MAX_DEPTH = 64; // Parentheses / unary operators

  void skipSpace()
  {
    while ((m_pos < m_text.size()) && std::isspace(static_cast<unsigned char>(m_text[m_pos]))) {
      m_pos++;
    }
  }

  bool accept(const char *token)
  {
    skipSpace();
    size_t len = std::char_traits<char>::length(token);
    if (0 == m_text.compare(m_pos, len, token)) {
      m_pos += len;
      return true;
    }
    return false;
  }

  bool expr(double &result, int depth)
  {
    if (!term(result, depth)) {
      return false;
    }
    while (true) {
      skipSpace();
      if (m_pos >= m_text.size()) {
        return true;
      }
      char op = m_text[m_pos];
      if ((op != '+') && (op != '-')) {
        return true;
      }
      m_pos++;
      double rhs;
      if (!term(rhs, depth)) {
        return false;
      }
      result = ('+' == op) ? result + rhs : result - rhs;
    }
  }

  bool term(double &result, int depth)
  {
    if (!factor(result, depth)) {
      return false;
    }
    while (true) {
      skipSpace();
      if (m_pos >= m_text.size()) {
        return true;
      }
      char op = m_text[m_pos];
      if ((op != '*') && (op != '/')) {
        return true;
      }
      m_pos++;
      double rhs;
      if (!factor(rhs, depth)) {
        return false;
      }
      result = ('*' == op) ? result * rhs : result / rhs;
    }
  }

  bool factor(double &result, int depth)
  {
    if (depth > MAX_DEPTH) {
      return false;
    }
    skipSpace();
    if (m_pos >= m_text.size()) {
      return false;
    }
    char c = m_text[m_pos];
    if (('+' == c) || ('-' == c)) {
      m_pos++;
      if (!factor(result, depth + 1)) {
        return false;
      }
      if ('-' == c) {
        result = -result;
      }
      return true;
    }
    if ('(' == c) {
      m_pos++;
      if (!expr(result, depth + 1)) {
        return false;
      }
      return accept(")");
    }
    if (accept("{{{val}}}") || accept("{{val}}")) {
      result = m_value;
      return true;
    }
    return number(result);
  }

  bool number(double &result)
  {
    // digits [. digits] [e [+-] digits]  or  . digits ...
    size_t start = m_pos;
    size_t p     = m_pos;
    size_t digits = 0;
    while ((p < m_text.size()) && std::isdigit(static_cast<unsigned char>(m_text[p]))) {
      p++;
      digits++;
    }
    if ((p < m_text.size()) && ('.' == m_text[p])) {
      p++;
      while ((p < m_text.size()) && std::isdigit(static_cast<unsigned char>(m_text[p]))) {
        p++;
        digits++;
      }
    }
    if (0 == digits) {
      return false;
    }
    if ((p < m_text.size()) && (('e' == m_text[p]) || ('E' == m_text[p]))) {
      size_t q = p + 1;
      if ((q < m_text.size()) && (('+' == m_text[q]) || ('-' == m_text[q]))) {
        q++;
      }
      size_t expDigits = 0;
      while ((q < m_text.size()) && std::isdigit(static_cast<unsigned char>(m_text[q]))) {
        q++;
        expDigits++;
      }
      if (0 == expDigits) {
        return false;
      }
      p = q;
    }
    // Locale independent (a decimal comma locale must not change the result)
    std::istringstream iss(m_text.substr(start, p - start));
    iss.imbue(std::locale::classic());
    if (!(iss >> result)) {
      return false;
    }
    m_pos = p;
    return true;
  }

  const std::string &m_text;
  double m_value;
  size_t m_pos = 0;
};

bool
isFalse(const json &j)
{
  if (j.is_boolean()) {
    return !j.get<bool>();
  }
  if (j.is_number_integer()) {
    return 0 == j.get<int>();
  }
  if (j.is_string()) {
    std::string s = j.get<std::string>();
    return ("false" == s) || ("0" == s) || ("no" == s);
  }
  return false;
}

std::string
stringOf(const json &obj, const char *key, const char *def = "")
{
  auto it = obj.find(key);
  return ((it != obj.end()) && it->is_string()) ? it->get<std::string>() : std::string(def);
}

} // namespace

///////////////////////////////////////////////////////////////////////////////
// vscp_evalUnitExpression
//

int
vscp_evalUnitExpression(double *presult, const std::string &expr, double value)
{
  if (nullptr == presult) {
    return VSCP_ERROR_INVALID_POINTER;
  }
  double result = 0;
  Parser parser(expr, value);
  if (!parser.parse(result)) {
    return VSCP_ERROR_INVALID_SYNTAX;
  }
  if (!std::isfinite(result)) {
    return VSCP_ERROR_INVALID_FORMAT;
  }
  *presult = result;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// loadFile
//

int
CVscpUnitExpressions::loadFile(const std::string &path)
{
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    m_lastError = "Can not open " + path;
    return VSCP_ERROR_INVALID_PATH;
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  return loadString(ss.str());
}

///////////////////////////////////////////////////////////////////////////////
// loadString
//

int
CVscpUnitExpressions::loadString(const std::string &text)
{
  json j = json::parse(text, nullptr, false);
  if (j.is_discarded() || !j.is_object()) {
    m_lastError = "Not valid JSON";
    return VSCP_ERROR_INVALID_FORMAT;
  }

  // vscp-units.json: { "units": [ { "type", "unit", ... } ] }
  // vscp-hashtypeevents.json: { "events": [ { "class": 10, "types": [ { "type", "units": [ { "id", ... } ] } ] } ] }
  std::map<std::pair<uint16_t, uint8_t>, Unit> units;
  auto add = [&](int type, int id, const json &u) -> int {
    if ((type < 0) || (type > 0xffff) || (id < 0) || (id > 0xff)) {
      m_lastError = "Type or unit out of range";
      return VSCP_ERROR_INVALID_FORMAT;
    }
    Unit unit;
    unit.name         = stringOf(u, "name");
    unit.description  = stringOf(u, "description");
    unit.symbol_ascii = stringOf(u, "symbol-ascii");
    unit.symbol_utf8  = stringOf(u, "symbol-utf8");
    unit.conversion0  = stringOf(u, "conversion0", "{{{val}}}");
    unit.conversion   = stringOf(u, "conversion", "{{{val}}}");
    auto it           = u.find("convertible");
    unit.convertible  = (it == u.end()) || !isFalse(*it);
    // Check the expressions now, not when a value is converted
    double dummy;
    if ((VSCP_ERROR_INVALID_SYNTAX == vscp_evalUnitExpression(&dummy, unit.conversion0, 1.0)) ||
        (VSCP_ERROR_INVALID_SYNTAX == vscp_evalUnitExpression(&dummy, unit.conversion, 1.0))) {
      m_lastError = "Invalid expression for type " + std::to_string(type) + " unit " + std::to_string(id);
      return VSCP_ERROR_INVALID_SYNTAX;
    }
    units[std::make_pair(static_cast<uint16_t>(type), static_cast<uint8_t>(id))] = unit;
    return VSCP_ERROR_SUCCESS;
  };

  try {
    if (j.contains("units") && j["units"].is_array()) {
      for (const auto &u : j["units"]) {
        int rv = add(u.at("type").get<int>(), u.at("unit").get<int>(), u);
        if (VSCP_ERROR_SUCCESS != rv) {
          return rv;
        }
      }
    }
    else if (j.contains("events") && j["events"].is_array()) {
      for (const auto &cls : j["events"]) {
        if (!cls.contains("class") || (VSCP_CLASS1_MEASUREMENT != cls["class"].get<int>()) ||
            !cls.contains("types")) {
          continue;
        }
        for (const auto &t : cls["types"]) {
          if (!t.contains("units") || !t["units"].is_array()) {
            continue;
          }
          for (const auto &u : t["units"]) {
            int rv = add(t.at("type").get<int>(), u.at("id").get<int>(), u);
            if (VSCP_ERROR_SUCCESS != rv) {
              return rv;
            }
          }
        }
      }
    }
    else {
      m_lastError = "No \"units\" or \"events\" in the JSON";
      return VSCP_ERROR_INVALID_FORMAT;
    }
  }
  catch (const json::exception &ex) {
    m_lastError = std::string("Invalid unit: ") + ex.what();
    return VSCP_ERROR_INVALID_FORMAT;
  }

  if (units.empty()) {
    m_lastError = "No units in the JSON";
    return VSCP_ERROR_INVALID_FORMAT;
  }

  m_units.swap(units);
  m_lastError.clear();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getUnit
//

const CVscpUnitExpressions::Unit *
CVscpUnitExpressions::getUnit(uint16_t vscp_type, uint8_t unit) const
{
  auto it = m_units.find(std::make_pair(vscp_type, unit));
  return (it == m_units.end()) ? nullptr : &it->second;
}

///////////////////////////////////////////////////////////////////////////////
// convert
//

int
CVscpUnitExpressions::convert(double *presult, uint16_t vscp_type, uint8_t from_unit, uint8_t to_unit, double value)
  const
{
  if (nullptr == presult) {
    return VSCP_ERROR_INVALID_POINTER;
  }
  const Unit *pfrom = getUnit(vscp_type, from_unit);
  const Unit *pto   = getUnit(vscp_type, to_unit);
  if ((nullptr == pfrom) || (nullptr == pto)) {
    return VSCP_ERROR_UNKNOWN_ITEM;
  }
  if (from_unit == to_unit) {
    *presult = value;
    return VSCP_ERROR_SUCCESS;
  }
  if (!pfrom->convertible || !pto->convertible) {
    return VSCP_ERROR_NOT_SUPPORTED;
  }
  double value0;
  int rv = vscp_evalUnitExpression(&value0, pfrom->conversion0, value);
  if (VSCP_ERROR_SUCCESS != rv) {
    return rv;
  }
  return vscp_evalUnitExpression(presult, pto->conversion, value0);
}
