/*
            !!!!!!!!!!!!!!!!!!!!  W A R N I N G  !!!!!!!!!!!!!!!!!!!!
                           This file is auto-generated
                see https://github.com/grodansparadis/vscp-classes
                        Generated: 2026-11-19 23:46:28.456372
*/

/*
  Measurement units of CLASS1.MEASUREMENT (class 10). The same types and
  units are used by all measurement classes (X1-X4 included).

  A value in a unit is converted to the default unit (unit 0) with

      value_in_unit_0 = value * scale + offset

  and back with  value = (value_in_unit_0 - offset) / scale.
  Units with convertible = 0 can not be converted (for example latitude
  and longitude); they only have a name and symbol.
*/

#ifndef VSCP_UNITS_H
#define VSCP_UNITS_H

#include <stdint.h>

typedef struct vscp_unit_def {
  uint16_t vscp_type;        /* Measurement type (CLASS1.MEASUREMENT type) */
  uint8_t unit;              /* Unit 0-255 */
  uint8_t convertible;       /* 1 if the unit can be converted to/from unit 0 */
  double scale;              /* value_in_unit_0 = value * scale + offset */
  double offset;
  const char *name;
  const char *symbol_ascii;
  const char *symbol_utf8;
} vscp_unit_def_t;

static const vscp_unit_def_t vscp_unit_table[] = {
  { 1, 0, 1, 1.0, 0.0, "Count", "", "" },
  { 2, 0, 1, 1.0, 0.0, "Meter", "m", "m" },
  { 3, 0, 1, 1.0, 0.0, "Kilogram", "kg", "kg" },
  { 4, 0, 1, 1.0, 0.0, "Seconds", "s", "s" },
  { 4, 1, 1, 0.001, 0.0, "Milliseconds", "ms", "ms" },
  { 4, 2, 0, 1.0, 0.0, "yymdhms", "", "" },
  { 4, 3, 0, 1.0, 0.0, "HHMMSS", "", "" },
  { 5, 0, 1, 1.0, 0.0, "Ampere", "A", "A" },
  { 6, 0, 1, 1.0, 0.0, "Kelvin", "K", "K" },
  { 6, 1, 1, 1.0, 273.15, "Celsius", "C", "\302\260C" },
  { 6, 2, 1, 0.5555555555555556, 255.37222222222223, "Fahrenheit", "F", "\302\260F" },
  { 7, 0, 1, 1.0, 0.0, "mole", "mol", "mol" },
  { 8, 0, 1, 1.0, 0.0, "candela", "cd", "cd" },
  { 9, 0, 1, 1.0, 0.0, "hertz", "Hz", "hz" },
  { 10, 0, 1, 1.0, 0.0, "becquerel", "Bq", "Bq" },
  { 10, 1, 1, 37000000000.0, 0.0, "curie", "Ci", "Ci" },
  { 10, 2, 1, 1000000.0, 0.0, "rutherford", "Rd", "Rd" },
  { 11, 0, 1, 1.0, 0.0, "newton", "N", "N" },
  { 12, 0, 1, 1.0, 0.0, "pascal", "Pa", "Pa" },
  { 12, 1, 1, 100000.0, 0.0, "bar", "bar", "bar" },
  { 12, 2, 1, 6894.757293168, 0.0, "psi", "psi", "psi" },
  { 12, 3, 1, 101325.0, 0.0, "atm", "atm", "atm" },
  { 12, 4, 1, 133.322368421, 0.0, "torr", "torr", "torr" },
  { 12, 5, 1, 98066.5, 0.0, "at", "at", "at" },
  { 12, 6, 1, 98.0665, 0.0, "cmH2O", "cmH2O", "cmH2O" },
  { 13, 0, 1, 1.0, 0.0, "joule", "J", "J" },
  { 13, 1, 1, 3600000.0, 0.0, "kwh", "kWh", "kWh" },
  { 13, 2, 1, 3600.0, 0.0, "wh", "Wh", "Wh" },
  { 13, 3, 1, 1.602176634e-19, 0.0, "eV", "eV", "eV" },
  { 14, 0, 1, 1.0, 0.0, "watt", "W", "W" },
  { 14, 1, 1, 735.49875, 0.0, "Horsepower Metric", "cv", "cv" },
  { 14, 2, 1, 745.69987158227, 0.0, "Horsepower Imperial", "hp", "hp" },
  { 15, 0, 1, 1.0, 0.0, "coulomb", "C", "C" },
  { 16, 0, 1, 1.0, 0.0, "voltage", "V", "V" },
  { 17, 0, 1, 1.0, 0.0, "farad", "F", "F" },
  { 18, 0, 1, 1.0, 0.0, "ohm", "ohm", "\316\251" },
  { 19, 0, 1, 1.0, 0.0, "siemens", "S", "S" },
  { 20, 0, 1, 1.0, 0.0, "ampere per meter", "A/m", "A/m" },
  { 20, 1, 1, 79.57747154594767, 0.0, "Oersted", "oe", "oe" },
  { 21, 0, 1, 1.0, 0.0, "weber", "Wb", "Wb" },
  { 22, 0, 1, 1.0, 0.0, "tesla", "T", "T" },
  { 22, 1, 1, 0.0001, 0.0, "gauss", "Gs", "Gs" },
  { 23, 0, 1, 1.0, 0.0, "henry", "H", "H" },
  { 24, 0, 1, 1.0, 0.0, "lumen", "lm", "lm" },
  { 25, 0, 1, 1.0, 0.0, "lux", "lx", "lx" },
  { 26, 0, 1, 1.0, 0.0, "gray", "gy", "gy" },
  { 27, 0, 1, 1.0, 0.0, "katal", "kat", "kat" },
  { 28, 0, 1, 1.0, 0.0, "cubic meter", "m3", "m\302\263" },
  { 28, 1, 1, 0.001, 0.0, "litre", "L", "dm\302\263" },
  { 28, 2, 1, 1e-06, 0.0, "millilitre", "ml", "cm\302\263" },
  { 28, 3, 1, 0.0001, 0.0, "decilitre", "dl", "dl" },
  { 28, 4, 1, 1e-05, 0.0, "centilitre", "cl", "cl" },
  { 29, 0, 1, 1.0, 0.0, "watt per square meter", "W/m2", "W/m\302\262" },
  { 30, 0, 1, 1.0, 0.0, "radian", "rad", "rad" },
  { 30, 1, 1, 0.017453292519943295, 0.0, "degree", "deg", "\302\260" },
  { 30, 2, 1, 0.0002908882086657216, 0.0, "arcminute", "arcmin", "arcmin" },
  { 30, 3, 1, 4.84813681109536e-06, 0.0, "arcsecond", "arcsec", "arcsec" },
  { 31, 0, 1, 1.0, 0.0, "Longitude", "long", "long" },
  { 31, 1, 0, 1.0, 0.0, "Latitude", "lat", "lat" },
  { 32, 0, 1, 1.0, 0.0, "meter per second", "m/s", "m/s" },
  { 32, 1, 1, 0.2777777777777778, 0.0, "kilometers per hour", "km/h", "km/h" },
  { 32, 2, 1, 0.44704, 0.0, "miles per hour", "mph", "mph" },
  { 32, 3, 1, 0.5144444444444445, 0.0, "knot", "kt", "kt" },
  { 33, 0, 1, 1.0, 0.0, "Metre per second squared", "m/s2", "m/s\302\262" },
  { 34, 0, 1, 1.0, 0.0, "newton meter", "N/m", "N/m" },
  { 35, 0, 1, 1.0, 0.0, "relative humidity (0-100%)", "%", "%" },
  { 36, 0, 1, 1.0, 0.0, "cubic meters per second", "m3/s", "m\302\263/s" },
  { 36, 1, 1, 0.001, 0.0, "litres per second", "L/s", "dm\302\263/s" },
  { 37, 0, 1, 1.0, 0.0, "kelvins per watt", "K/W", "K/W" },
  { 38, 0, 1, 1.0, 0.0, "refractive (optical) power", "dpt", "dpt" },
  { 39, 0, 1, 1.0, 0.0, "pascal second", "Pa s", "Pa s" },
  { 39, 1, 1, 1.0, 0.0, "poiseuille", "Pl", "Pl" },
  { 39, 2, 1, 0.1, 0.0, "poise", "P", "P" },
  { 40, 0, 1, 1.0, 0.0, "pascal second per cubic metre", "Pa s/m3", "Pa\302\267s/m\302\263" },
  { 41, 0, 1, 1.0, 0.0, "acoustic ohm", "Pa s/ m3", "Pa \302\267 s/ m\302\263" },
  { 42, 0, 1, 1.0, 0.0, "reciprocal farad (daraf)", "1/F", "1/F" },
  { 43, 0, 1, 1.0, 0.0, "talbot", "tb", "tb" },
  { 44, 0, 1, 1.0, 0.0, "candela per square metre", "cd/m2", "cd/m\302\262" },
  { 45, 0, 1, 1.0, 0.0, "mol per cubic metre", "mol/m3", "mol/m\302\263" },
  { 45, 1, 1, 1000.0, 0.0, "mol per litre", "mol/L", "mol/L" },
  { 45, 2, 0, 1.0, 0.0, "ppm", "ppm", "ppm" },
  { 45, 3, 0, 1.0, 0.0, "percent", "%", "%" },
  { 46, 0, 1, 1.0, 0.0, "kilogram per cubic metre", "kg/m3", "kg/m\302\263" },
  { 49, 0, 1, 1.0, 0.0, "kelvin", "K", "K" },
  { 49, 1, 1, 1.0, 273.15, "celsius", "C", "\302\260C" },
  { 49, 2, 1, 0.5555555555555556, 255.37222222222223, "fahrenheit", "F", "\302\260F" },
  { 50, 0, 1, 1.0, 0.0, "relative value", "relative", "relative" },
  { 51, 0, 1, 1.0, 0.0, "meter", "m", "m" },
  { 51, 1, 1, 0.3048, 0.0, "feet", "ft", "ft" },
  { 51, 2, 1, 0.0254, 0.0, "inch", "in", "in" },
  { 52, 0, 1, 1.0, 0.0, "square meter", "m2", "m\302\262" },
  { 52, 1, 1, 100.0, 0.0, "are", "a", "a" },
  { 52, 2, 1, 10000.0, 0.0, "hectare", "ha", "ha" },
  { 52, 3, 1, 1000000.0, 0.0, "square kilometer", "km2", "km\302\262" },
  { 53, 0, 1, 1.0, 0.0, "watt per steradian", "W/sr", "W/sr" },
  { 54, 0, 1, 1.0, 0.0, "watt per steradian per square metre", "W\342\213\205sr\342\210\2221\342\213\205m\342\210\2222", "W\342\213\205sr^\342\210\2221\342\213\205m^\342\210\2222" },
  { 55, 0, 1, 1.0, 0.0, "watt per square metre", "W/m2", "W/m\302\262" },
  { 56, 0, 1, 1.0, 0.0, "watt per steradian per square metre, per metre", "W\342\213\205sr\342\210\2221\342\213\205m\342\210\2222", "W\342\213\205sr^\342\210\2221\342\213\205m*^\342\210\2222" },
  { 56, 1, 0, 1.0, 0.0, "watt per square metre, per metre", "W\342\213\205sr\342\210\2221\342\213\205m\342\210\2223", "W\342\213\205sr^\342\210\2221\342\213\205m*^\342\210\2223" },
  { 56, 2, 0, 1.0, 0.0, "watt per square metre per hertz", "W/m3", "W/m^3" },
  { 57, 0, 1, 1.0, 0.0, "watt per square metre per hertz", "W\342\213\205m\342\210\2222\342\213\205Hz\342\210\2221", "W\342\213\205m^\342\210\2222\342\213\205Hz^\342\210\2221" },
  { 57, 1, 0, 1.0, 0.0, "watt per square metre, per metre", "W\342\213\205sr\342\210\2221\342\213\205m\342\210\2223", "W\342\213\205sr^\342\210\2221\342\213\205m*^\342\210\2223" },
  { 57, 2, 0, 1.0, 0.0, "watt per square metre per hertz", "W/m3", "W/m^3" },
  { 58, 0, 1, 1.0, 0.0, "pascal", "Pa", "Pa" },
  { 59, 0, 1, 1.0, 0.0, "pascal", "Pa", "Pa" },
  { 60, 0, 1, 1.0, 0.0, "decibel", "dB", "dB" },
  { 61, 0, 1, 1.0, 0.0, "sievert", "sv", "sv" },
  { 61, 1, 1, 0.01, 0.0, "rem", "rem", "rem" },
  { 62, 0, 1, 1.0, 0.0, "coulomb per kilogram", "C/kg", "C/kg" },
  { 62, 1, 1, 0.000258, 0.0, "r\303\266ntgen", "R", "R" },
  { 63, 0, 1, 1.0, 0.0, "cos(a)", "PF", "PF" },
  { 64, 0, 1, 1.0, 0.0, "VAr", "VAr", "VAr" },
  { 64, 1, 1, 1000.0, 0.0, "kVAr", "kVAr", "kVAr" },
  { 64, 2, 1, 1000000.0, 0.0, "MVAr", "MVAr", "MVAr" },
  { 65, 0, 1, 1.0, 0.0, "kVArh", "kVArh", "kVArh" },
  { 65, 1, 1, 1000.0, 0.0, "MVArh", "MVArh", "MVArh" },
};

#define VSCP_UNIT_TABLE_SIZE (sizeof(vscp_unit_table) / sizeof(vscp_unit_table[0]))

#endif /* VSCP_UNITS_H */
