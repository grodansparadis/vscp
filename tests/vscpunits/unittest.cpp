// Tests for measurement unit conversion:
//   - the compiled unit table (vscp_convertMeasurementUnit() and friends in vscphelper)
//   - the expression based alternative (vscpunitexpr)
//
// The MIT License (MIT)
// Copyright © 2000-2026 Ake Hedman, the VSCP project <info@vscp.org>

#include <vscp.h>
#include <vscp-class.h>
#include <vscp-type.h>
#include <vscphelper.h>
#include <vscpunitexpr.h>

#include "vscp-units.h"

#include <gtest/gtest.h>

#include <clocale>
#include <cmath>
#include <cstring>
#include <set>
#include <string>

#ifndef VSCP_UNITS_JSON
#error "VSCP_UNITS_JSON must be the path to vscp-units.json"
#endif

namespace {

// Relative compare
::testing::AssertionResult
near(double expected, double actual, double rel = 1e-12)
{
  double diff = std::fabs(expected - actual);
  if (diff <= rel * std::max(1.0, std::max(std::fabs(expected), std::fabs(actual)))) {
    return ::testing::AssertionSuccess();
  }
  return ::testing::AssertionFailure() << "expected " << expected << " got " << actual;
}

double
conv(uint16_t type, uint8_t from, uint8_t to, double v)
{
  double r = NAN;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, vscp_convertMeasurementUnit(&r, type, from, to, v));
  return r;
}

// An event owned by the test
struct Event {
  vscpEvent *pev = nullptr;
  Event()
  {
    vscp_newEvent(&pev);
    pev->pdata    = nullptr;
    pev->sizeData = 0;
  }
  ~Event() { vscp_deleteEvent_v2(&pev); }
};

} // namespace

// ---------------------------------------------------------------------------
// Compiled table
// ---------------------------------------------------------------------------

TEST(UnitTable, SortedAndUnique)
{
  ASSERT_GT(VSCP_UNIT_TABLE_SIZE, 100u);
  for (size_t i = 1; i < VSCP_UNIT_TABLE_SIZE; i++) {
    uint32_t a = (uint32_t(vscp_unit_table[i - 1].vscp_type) << 8) | vscp_unit_table[i - 1].unit;
    uint32_t b = (uint32_t(vscp_unit_table[i].vscp_type) << 8) | vscp_unit_table[i].unit;
    EXPECT_LT(a, b) << "entry " << i;
  }
  // Every unit 0 is the identity
  for (size_t i = 0; i < VSCP_UNIT_TABLE_SIZE; i++) {
    if (0 == vscp_unit_table[i].unit) {
      EXPECT_EQ(1.0, vscp_unit_table[i].scale) << "type " << vscp_unit_table[i].vscp_type;
      EXPECT_EQ(0.0, vscp_unit_table[i].offset) << "type " << vscp_unit_table[i].vscp_type;
    }
  }
}

TEST(UnitTable, KnownValues)
{
  // Temperature: 0 K, 1 °C, 2 °F
  EXPECT_TRUE(near(273.15, conv(VSCP_TYPE_MEASUREMENT_TEMPERATURE, 1, 0, 0)));
  EXPECT_TRUE(near(100, conv(VSCP_TYPE_MEASUREMENT_TEMPERATURE, 2, 1, 212)));
  EXPECT_TRUE(near(-40, conv(VSCP_TYPE_MEASUREMENT_TEMPERATURE, 1, 2, -40)));
  EXPECT_TRUE(near(32, conv(VSCP_TYPE_MEASUREMENT_TEMPERATURE, 0, 2, 273.15)));
  // Pressure: 1 atm = 1.01325 bar, 1 psi = 6894.757293168 Pa, 760 torr = 1 atm
  EXPECT_TRUE(near(1.01325, conv(VSCP_TYPE_MEASUREMENT_PRESSURE, 3, 1, 1)));
  EXPECT_TRUE(near(6894.757293168, conv(VSCP_TYPE_MEASUREMENT_PRESSURE, 2, 0, 1)));
  EXPECT_TRUE(near(1, conv(VSCP_TYPE_MEASUREMENT_PRESSURE, 4, 3, 760), 1e-9));
  // Energy: 1 kWh = 3.6 MJ = 1000 Wh
  EXPECT_TRUE(near(3.6e6, conv(VSCP_TYPE_MEASUREMENT_ENERGY, 1, 0, 1)));
  EXPECT_TRUE(near(1000, conv(VSCP_TYPE_MEASUREMENT_ENERGY, 1, 2, 1)));
  // Angle: 180° = pi rad = 10800 arcmin
  EXPECT_TRUE(near(M_PI, conv(VSCP_TYPE_MEASUREMENT_ANGLE, 1, 0, 180)));
  EXPECT_TRUE(near(10800, conv(VSCP_TYPE_MEASUREMENT_ANGLE, 1, 2, 180)));
  // Speed: 36 km/h = 10 m/s, 1 knot = 1.852 km/h
  EXPECT_TRUE(near(10, conv(VSCP_TYPE_MEASUREMENT_SPEED, 1, 0, 36)));
  EXPECT_TRUE(near(1.852, conv(VSCP_TYPE_MEASUREMENT_SPEED, 3, 1, 1)));
  // Volume: 1 l = 1000 ml = 1e-3 m³
  EXPECT_TRUE(near(1000, conv(VSCP_TYPE_MEASUREMENT_VOLUME, 1, 2, 1)));
  EXPECT_TRUE(near(1e-3, conv(VSCP_TYPE_MEASUREMENT_VOLUME, 1, 0, 1)));
  // Time: 1500 ms = 1.5 s
  EXPECT_TRUE(near(1.5, conv(VSCP_TYPE_MEASUREMENT_TIME, 1, 0, 1500)));
  // Same unit
  EXPECT_EQ(12.5, conv(VSCP_TYPE_MEASUREMENT_TEMPERATURE, 2, 2, 12.5));
}

TEST(UnitTable, RoundTripAllPairs)
{
  for (size_t i = 0; i < VSCP_UNIT_TABLE_SIZE; i++) {
    for (size_t j = 0; j < VSCP_UNIT_TABLE_SIZE; j++) {
      const vscp_unit_def_t &a = vscp_unit_table[i];
      const vscp_unit_def_t &b = vscp_unit_table[j];
      if ((a.vscp_type != b.vscp_type) || !a.convertible || !b.convertible) {
        continue;
      }
      for (double v : { -123.25, 0.0, 1.0, 98765.4321 }) {
        double r1, r2;
        ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_convertMeasurementUnit(&r1, a.vscp_type, a.unit, b.unit, v));
        ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_convertMeasurementUnit(&r2, a.vscp_type, b.unit, a.unit, r1));
        EXPECT_TRUE(near(v, r2, 1e-9)) << "type " << a.vscp_type << " " << int(a.unit) << "->" << int(b.unit);
      }
    }
  }
}

TEST(UnitTable, Errors)
{
  double r = 0;
  EXPECT_EQ(VSCP_ERROR_INVALID_POINTER, vscp_convertMeasurementUnit(nullptr, VSCP_TYPE_MEASUREMENT_TEMPERATURE, 0, 1, 1));
  EXPECT_EQ(VSCP_ERROR_UNKNOWN_ITEM, vscp_convertMeasurementUnit(&r, VSCP_TYPE_MEASUREMENT_TEMPERATURE, 0, 9, 1));
  EXPECT_EQ(VSCP_ERROR_UNKNOWN_ITEM, vscp_convertMeasurementUnit(&r, 47, 0, 0, 1)); // No units defined
  EXPECT_EQ(VSCP_ERROR_UNKNOWN_ITEM, vscp_convertMeasurementUnit(&r, 9999, 0, 0, 1));
  // Longitude <-> latitude can not be converted, but the same unit can
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, vscp_convertMeasurementUnit(&r, VSCP_TYPE_MEASUREMENT_POSITION, 0, 1, 1));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, vscp_convertMeasurementUnit(&r, VSCP_TYPE_MEASUREMENT_POSITION, 1, 1, 59.5));
  EXPECT_EQ(59.5, r);
  // ppm to mol/m³
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, vscp_convertMeasurementUnit(&r, 45, 2, 0, 1));
}

TEST(UnitTable, UnitInfo)
{
  std::string name, ascii, utf8;
  bool convertible = false;
  ASSERT_EQ(VSCP_ERROR_SUCCESS,
            vscp_getMeasurementUnitInfo(VSCP_TYPE_MEASUREMENT_TEMPERATURE, 1, name, ascii, utf8, &convertible));
  EXPECT_EQ("Celsius", name);
  EXPECT_EQ("C", ascii);
  EXPECT_EQ("\xC2\xB0" "C", utf8);
  EXPECT_TRUE(convertible);
  ASSERT_EQ(VSCP_ERROR_SUCCESS,
            vscp_getMeasurementUnitInfo(VSCP_TYPE_MEASUREMENT_POSITION, 1, name, ascii, utf8, &convertible));
  EXPECT_EQ("Latitude", name);
  EXPECT_FALSE(convertible);
  EXPECT_EQ(VSCP_ERROR_UNKNOWN_ITEM,
            vscp_getMeasurementUnitInfo(VSCP_TYPE_MEASUREMENT_TEMPERATURE, 3, name, ascii, utf8, nullptr));
}

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

TEST(UnitEvent, Class1MeasurementAndX1To4)
{
  for (uint16_t cls : { VSCP_CLASS1_MEASUREMENT, VSCP_CLASS1_MEASUREMENTX1, VSCP_CLASS1_MEASUREMENTX4 }) {
    Event e;
    e.pev->vscp_class = VSCP_CLASS1_MEASUREMENT; // The maker only knows the base class
    e.pev->vscp_type  = VSCP_TYPE_MEASUREMENT_TEMPERATURE;
    ASSERT_TRUE(vscp_makeFloatMeasurementEvent(e.pev, 25.0f, 1 /* °C */, 0));
    e.pev->vscp_class = cls;
    double r          = 0;
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_convertMeasurement(&r, e.pev, 0)) << "class " << cls;
    EXPECT_TRUE(near(298.15, r, 1e-6)) << "class " << cls;
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_convertMeasurement(&r, e.pev, 2)) << "class " << cls;
    EXPECT_TRUE(near(77, r, 1e-6)) << "class " << cls;
  }
}

TEST(UnitEvent, Measurement32DefaultUnit)
{
  // CLASS1.MEASUREMENT32: a float MSB first, always in unit 0
  for (uint16_t cls : { VSCP_CLASS1_MEASUREMENT32, VSCP_CLASS1_MEASUREMENT32X2 }) {
    Event e;
    e.pev->vscp_class = cls;
    e.pev->vscp_type  = VSCP_TYPE_MEASUREMENT_TEMPERATURE;
    float f           = 300.0f;
    uint32_t bits;
    memcpy(&bits, &f, 4);
    e.pev->sizeData = 4;
    e.pev->pdata    = new uint8_t[4] { uint8_t(bits >> 24), uint8_t(bits >> 16), uint8_t(bits >> 8), uint8_t(bits) };
    double r        = 0;
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_convertMeasurement(&r, e.pev, 1)) << "class " << cls;
    EXPECT_TRUE(near(26.85, r, 1e-6));
  }
}

TEST(UnitEvent, Measurement64AndLevel2Variants)
{
  // CLASS1.MEASUREMENT64 (and X3): a double MSB first, always in unit 0
  const double k = 300.123456789;
  uint64_t bits;
  memcpy(&bits, &k, 8);
  for (uint16_t cls : { VSCP_CLASS1_MEASUREMENT64, VSCP_CLASS1_MEASUREMENT64X3 }) {
    Event e;
    e.pev->vscp_class = cls;
    e.pev->vscp_type  = VSCP_TYPE_MEASUREMENT_TEMPERATURE;
    e.pev->sizeData   = 8;
    e.pev->pdata      = new uint8_t[8];
    for (int i = 0; i < 8; i++) {
      e.pev->pdata[i] = uint8_t(bits >> (56 - 8 * i));
    }
    double r = 0;
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_convertMeasurement(&r, e.pev, 1)) << "class " << cls;
    EXPECT_TRUE(near(k - 273.15, r)) << "class " << cls; // Full double precision
  }

  // CLASS2.LEVEL1.MEASUREMENT32X1: 16-byte GUID, then the float
  Event e;
  e.pev->vscp_class = VSCP_CLASS2_LEVEL1_MEASUREMENT32X1;
  e.pev->vscp_type  = VSCP_TYPE_MEASUREMENT_PRESSURE;
  float f           = 101325.0f;
  uint32_t fb;
  memcpy(&fb, &f, 4);
  e.pev->sizeData = 20;
  e.pev->pdata    = new uint8_t[20]();
  for (int i = 0; i < 4; i++) {
    e.pev->pdata[16 + i] = uint8_t(fb >> (24 - 8 * i));
  }
  double r = 0;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_convertMeasurement(&r, e.pev, 3 /* atm */));
  EXPECT_TRUE(near(1.0, r, 1e-7));
}

TEST(UnitEvent, FloatCodingIsMsbFirst)
{
  // VSCP numbers are big endian: 25.0f is 41 C8 00 00
  Event e;
  e.pev->vscp_class = VSCP_CLASS1_MEASUREMENT;
  e.pev->vscp_type  = VSCP_TYPE_MEASUREMENT_TEMPERATURE;
  ASSERT_TRUE(vscp_makeFloatMeasurementEvent(e.pev, 25.0f, 1, 0));
  ASSERT_EQ(5, e.pev->sizeData);
  EXPECT_EQ(0x41, e.pev->pdata[1]);
  EXPECT_EQ(0xC8, e.pev->pdata[2]);
  EXPECT_EQ(0x00, e.pev->pdata[3]);
  EXPECT_EQ(0x00, e.pev->pdata[4]);
  double v = 0;
  ASSERT_TRUE(vscp_getMeasurementAsDouble(&v, e.pev));
  EXPECT_EQ(25.0, v);
}

TEST(UnitEvent, Level2FloatAndString)
{
  Event f;
  ASSERT_TRUE(vscp_makeLevel2FloatMeasurementEvent(f.pev, VSCP_TYPE_MEASUREMENT_PRESSURE, 2.5, 1 /* bar */, 0, 0, 0));
  double r = 0;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_convertMeasurement(&r, f.pev, 0));
  EXPECT_TRUE(near(250000, r));

  Event s;
  ASSERT_TRUE(vscp_makeLevel2StringMeasurementEvent(s.pev, VSCP_TYPE_MEASUREMENT_SPEED, 36, 1 /* km/h */, 0, 0, 0));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_convertMeasurement(&r, s.pev, 0));
  EXPECT_TRUE(near(10, r, 1e-9));

  // Ex version
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  ASSERT_TRUE(vscp_convertEventToEventEx(&ex, f.pev));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_convertMeasurementEx(&r, &ex, 0));
  EXPECT_TRUE(near(250000, r));
}

TEST(UnitEvent, NotAMeasurement)
{
  Event e;
  e.pev->vscp_class = VSCP_CLASS1_INFORMATION;
  e.pev->vscp_type  = 1;
  double r          = 0;
  EXPECT_EQ(VSCP_ERROR_PARAMETER, vscp_convertMeasurement(&r, e.pev, 0));
  e.pev->vscp_class = VSCP_CLASS1_DATA; // Has its own units
  EXPECT_EQ(VSCP_ERROR_PARAMETER, vscp_convertMeasurement(&r, e.pev, 0));
  EXPECT_EQ(VSCP_ERROR_INVALID_POINTER, vscp_convertMeasurement(nullptr, e.pev, 0));
  EXPECT_EQ(VSCP_ERROR_INVALID_POINTER, vscp_convertMeasurement(&r, nullptr, 0));
}

// ---------------------------------------------------------------------------
// Expression based alternative
// ---------------------------------------------------------------------------

TEST(UnitExpression, Evaluate)
{
  double r = 0;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, vscp_evalUnitExpression(&r, "({{{val}}} + 459.67) * 5/9", 212));
  EXPECT_TRUE(near(373.15, r));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, vscp_evalUnitExpression(&r, "{{val}} * 9/5 - 459.67", 373.15));
  EXPECT_TRUE(near(212, r));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, vscp_evalUnitExpression(&r, "2 + 3 * 4 - 6 / 2", 0));
  EXPECT_EQ(11, r);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, vscp_evalUnitExpression(&r, "-(2 - 5) * -2", 0));
  EXPECT_EQ(-6, r);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, vscp_evalUnitExpression(&r, " {{{val}}}*1e-4 ", 3));
  EXPECT_TRUE(near(3e-4, r));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, vscp_evalUnitExpression(&r, "{{{val}}}/2.58E+4", 2.58e4));
  EXPECT_EQ(1, r);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, vscp_evalUnitExpression(&r, ".5 + 1.", 0));
  EXPECT_EQ(1.5, r);
}

TEST(UnitExpression, Invalid)
{
  double r = 42;
  for (const char *bad : { "", "1 +", "(1", "1)", "2^3", "pi", "{{{val}}} x", "1..2", "1e", "--", "{{{value}}}",
                           "{{{val}}}×2", "1,5" }) {
    EXPECT_EQ(VSCP_ERROR_INVALID_SYNTAX, vscp_evalUnitExpression(&r, bad, 1)) << bad;
  }
  EXPECT_EQ(42, r); // Untouched on error
  EXPECT_EQ(VSCP_ERROR_INVALID_FORMAT, vscp_evalUnitExpression(&r, "{{{val}}}/0", 1));
  EXPECT_EQ(VSCP_ERROR_INVALID_POINTER, vscp_evalUnitExpression(nullptr, "1", 1));
  std::string deep(200, '(');
  deep += "1" + std::string(200, ')');
  EXPECT_EQ(VSCP_ERROR_INVALID_SYNTAX, vscp_evalUnitExpression(&r, deep, 1));
}

TEST(UnitExpression, LocaleIndependent)
{
  const char *old = setlocale(LC_NUMERIC, nullptr);
  std::string saved = old ? old : "C";
  bool changed = (nullptr != setlocale(LC_NUMERIC, "de_DE.UTF-8")) || (nullptr != setlocale(LC_NUMERIC, "sv_SE.UTF-8"));
  double r = 0;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, vscp_evalUnitExpression(&r, "{{{val}}} + 273.15", 1.5));
  EXPECT_EQ(274.65, r);
  setlocale(LC_NUMERIC, saved.c_str());
  if (!changed) {
    GTEST_SKIP() << "No decimal comma locale installed";
  }
}

TEST(UnitExpression, LoadAndMatchTable)
{
  CVscpUnitExpressions units;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, units.loadFile(VSCP_UNITS_JSON)) << units.getLastError();
  EXPECT_EQ(VSCP_UNIT_TABLE_SIZE, units.size());

  // Both solutions give the same result for every pair of units
  size_t pairs = 0;
  for (size_t i = 0; i < VSCP_UNIT_TABLE_SIZE; i++) {
    for (size_t j = 0; j < VSCP_UNIT_TABLE_SIZE; j++) {
      const vscp_unit_def_t &a = vscp_unit_table[i];
      const vscp_unit_def_t &b = vscp_unit_table[j];
      if (a.vscp_type != b.vscp_type) {
        continue;
      }
      const CVscpUnitExpressions::Unit *pu = units.getUnit(a.vscp_type, a.unit);
      ASSERT_NE(nullptr, pu);
      EXPECT_EQ(bool(a.convertible), pu->convertible);
      EXPECT_EQ(a.name, pu->name);
      for (double v : { -40.0, 0.0, 21.5, 1234.5 }) {
        double rt = 0, re = 0;
        int rvt = vscp_convertMeasurementUnit(&rt, a.vscp_type, a.unit, b.unit, v);
        int rve = units.convert(&re, a.vscp_type, a.unit, b.unit, v);
        ASSERT_EQ(rvt, rve) << "type " << a.vscp_type << " " << int(a.unit) << "->" << int(b.unit);
        if (VSCP_ERROR_SUCCESS == rvt) {
          EXPECT_TRUE(near(rt, re, 1e-12)) << "type " << a.vscp_type << " " << int(a.unit) << "->" << int(b.unit);
          pairs++;
        }
      }
    }
  }
  EXPECT_GT(pairs, 500u);
  double r;
  EXPECT_EQ(VSCP_ERROR_UNKNOWN_ITEM, units.convert(&r, 9999, 0, 0, 1));
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, units.convert(&r, VSCP_TYPE_MEASUREMENT_POSITION, 0, 1, 1));
}

TEST(UnitExpression, LoadErrorsKeepUnits)
{
  CVscpUnitExpressions units;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, units.loadFile(VSCP_UNITS_JSON));
  size_t n = units.size();
  EXPECT_EQ(VSCP_ERROR_INVALID_PATH, units.loadFile("/no/such/file.json"));
  EXPECT_EQ(VSCP_ERROR_INVALID_FORMAT, units.loadString("{ not json"));
  EXPECT_EQ(VSCP_ERROR_INVALID_FORMAT, units.loadString("{\"something\": 1}"));
  EXPECT_EQ(VSCP_ERROR_INVALID_SYNTAX,
            units.loadString(R"({"units":[{"type":6,"unit":1,"conversion0":"{{{val}}}^2","conversion":"{{{val}}}"}]})"));
  EXPECT_FALSE(units.getLastError().empty());
  EXPECT_EQ(n, units.size());
}

TEST(UnitExpression, LoadHashTypeEventsFormat)
{
  // The format of vscp-hashtypeevents.json (make_json.py): units of class 10 only
  const char *text = R"({
    "events": [
      { "class": 0, "types": [ { "type": 1 } ] },
      { "class": 10, "types": [
        { "type": 6, "units": [
          { "id": 0, "name": "Kelvin", "convertible": true, "conversion0": "{{{val}}}", "conversion": "{{{val}}}" },
          { "id": 1, "name": "Celsius", "conversion0": "{{{val}}} + 273.15", "conversion": "{{{val}}} - 273.15" } ] },
        { "type": 31, "units": [
          { "id": 0, "name": "Longitude" },
          { "id": 1, "name": "Latitude", "convertible": false } ] } ] },
      { "class": 60, "types": [ { "type": 6, "units": [ { "id": 7, "name": "ignored" } ] } ] }
    ] })";
  CVscpUnitExpressions units;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, units.loadString(text)) << units.getLastError();
  EXPECT_EQ(4u, units.size());
  double r = 0;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, units.convert(&r, 6, 0, 1, 300));
  EXPECT_TRUE(near(26.85, r));
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, units.convert(&r, 31, 1, 0, 1));
  EXPECT_EQ(nullptr, units.getUnit(6, 7));
}
