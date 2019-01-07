// FILE: vscpeventhelper.cpp
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
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

// HISTORY:
//    021107 - AKHE Started this file
//

#ifdef __GNUG__
//#pragma implementation
#endif

#include <deque>
#include <map>
#include <string>

#include <limits>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <sys/times.h>
#include <sys/types.h>
#endif

#include <mdf.h>
#include <vscp.h>
#include <vscpdatetime.h>
#include <vscphelper.h>

#include "vscpeventhelper.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor
//

VSCPInformation::VSCPInformation(void)
{
    m_hashClass[0]         = _("CLASS1_PROTOCOL");
    m_hashClass[1]         = _("CLASS1_ALARM");
    m_hashClass[2]         = _("CLASS1_SECURITY");
    m_hashClass[10]        = _("CLASS1_MEASUREMENT");
    m_hashClass[15]        = _("CLASS1_DATA");
    m_hashClass[20]        = _("CLASS1_INFORMATION");
    m_hashClass[30]        = _("CLASS1_CONTROL");
    m_hashClass[40]        = _("CLASS1_MULTIMEDIA");
    m_hashClass[50]        = _("CLASS1_AOL");
    m_hashClass[60]        = _("CLASS1_MEASUREMENT64");
    m_hashClass[65]        = _("CLASS1_MEASUREZONE");
    m_hashClass[85]        = _("CLASS1_SETVALUEZONE");
    m_hashClass[90]        = _("CLASS1_WEATHER");
    m_hashClass[95]        = _("CLASS1_WEATHER_FORECAST");
    m_hashClass[100]       = _("CLASS1_PHONE");
    m_hashClass[102]       = _("CLASS1_DISPLAY");
    m_hashClass[110]       = _("CLASS1_REMOTE");
    m_hashClass[206]       = _("CLASS1_GPS");
    m_hashClass[212]       = _("CLASS1_WIRELESS");
    m_hashClass[506]       = _("CLASS1_DIAGNOSTIC");
    m_hashClass[508]       = _("CLASS1_ERROR");
    m_hashClass[509]       = _("CLASS1_LOG");
    m_hashClass[510]       = _("CLASS1_LAB");
    m_hashClass[511]       = _("CLASS1_LOCAL");
    m_hashClass[512 + 0]   = _("CLASS2_LEVEL1_PROTOCOL");
    m_hashClass[512 + 1]   = _("CLASS2_LEVEL1_ALARM");
    m_hashClass[512 + 2]   = _("CLASS2_LEVEL1_SECURITY");
    m_hashClass[512 + 10]  = _("CLASS2_LEVEL1_MEASUREMENT");
    m_hashClass[512 + 15]  = _("CLASS2_LEVEL1_DATA");
    m_hashClass[512 + 20]  = _("CLASS2_LEVEL1_INFORMATION");
    m_hashClass[512 + 30]  = _("CLASS2_LEVEL1_CONTROL");
    m_hashClass[512 + 40]  = _("CLASS2_LEVEL1_MULTIMEDIA");
    m_hashClass[512 + 50]  = _("CLASS2_LEVEL1_AOL");
    m_hashClass[512 + 60]  = _("CLASS2_MEASUREMENT64");
    m_hashClass[512 + 65]  = _("CLASS2_MEASUREZONE");
    m_hashClass[512 + 85]  = _("CLASS2_SETVALUEZONE");
    m_hashClass[512 + 100] = _("CLASS2_LEVEL1_PHONE");
    m_hashClass[512 + 101] = _("CLASS2_LEVEL1_LIN");
    m_hashClass[512 + 110] = _("CLASS2_LEVEL1_REMOTE");
    m_hashClass[512 + 206] = _("CLASS2_LEVEL1_GPS");
    m_hashClass[512 + 212] = _("CLASS2_LEVEL1_WIRELESS");
    m_hashClass[512 + 506] = _("CLASS2_LEVEL1_DIAGNOSTIC");
    m_hashClass[512 + 508] = _("CLASS2_LEVEL1_ERROR");
    m_hashClass[512 + 509] = _("CLASS2_LEVEL1_LOG");
    m_hashClass[512 + 510] = _("CLASS2_LEVEL1_LAB");
    m_hashClass[512 + 511] = _("CLASS2_LEVEL1_LOCAL");
    m_hashClass[1024]      = _("CLASS2_PROTOCOL");
    m_hashClass[1025]      = _("CLASS2_CONTROL");
    m_hashClass[1026]      = _("CLASS2_INFORMATION");
    m_hashClass[1028]      = _("CLASS2_TEXT2SPEECH");
    m_hashClass[1029]      = _("CLASS2_CUSTOM");
    m_hashClass[1030]      = _("CLASS2_DISPLAY");
    m_hashClass[1040]      = _("CLASS2_MEASUREMENT_STR");
    m_hashClass[1060]      = _("CLASS2_MEASUREMENT_FLOAT");
    m_hashClass[65535]     = _("CLASS2_VSCPD");

    // PROTOCOL
    m_hashType[MAKE_CLASSTYPE_LONG(0, 0)]  = _("PROTOCOL_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 1)]  = _("SEGCTRL_HEARTBEAT");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 2)]  = _("NEW_NODE_ONLINE");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 3)]  = _("PROBE_ACK");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 4)]  = _("NO_NICKNAME");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 5)]  = _("RESERVED");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 6)]  = _("SET_NICKNAME");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 7)]  = _("NICKNAME_ACCEPTED");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 8)]  = _("DROP_NICKNAME");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 9)]  = _("READ_REGISTER");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 10)] = _("RW_RESPONSE");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 11)] = _("WRITE_REGISTER");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 12)] = _("ENTER_BOOT_LOADER");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 13)] = _("ACK_BOOT_LOADER");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 14)] = _("NACK_BOOT_LOADER");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 15)] = _("START_BLOCK");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 16)] = _("BLOCK_DATA");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 17)] = _("BLOCK_DATA_ACK");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 18)] = _("BLOCK_DATA_NACK");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 19)] = _("PROGRAM_BLOCK_DATA");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 20)] = _("PROGRAM_BLOCK_DATA_ACK");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 21)] = _("PROGRAM_BLOCK_DATA_NACK");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 22)] = _("ACTIVATE_NEW_IMAGE");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 23)] = _("RESET_DEVICE");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 24)] = _("PAGE_READ");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 25)] = _("PAGE_WRITE");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 26)] = _("RW_PAGE_RESPONSE");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 27)] = _("HIGH_END_SERVER_PROBE");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 28)] = _("HIGH_END_SERVER_RESPONSE");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 29)] = _("INCREMENT_REGISTER");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 30)] = _("DECREMENT_REGISTER");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 31)] = _("WHO_IS_THERE");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 32)] = _("WHO_IS_THERE_RESPONSE");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 33)] = _("GET_MATRIX_INFO");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 34)] = _("GET_MATRIX_INFO_RESPONSE");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 35)] = _("GET_EMBEDDED_MDF");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 36)] = _("GET_EMBEDDED_MDF_RESPONSE");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 37)] = _("EXTENDED_PAGE_READ_REGISTER");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 38)] = _("EXTENDED_PAGE_WRITE_REGISTER");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 39)] = _("EXTENDED_PAGE_RW_RESPONSE");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 40)] = _("GET_EVENT_INTEREST");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 41)] = _("GET_EVENT_INTEREST_RESPONSE");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 42)] = _("RESERVED");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 43)] = _("RESERVED");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 44)] = _("RESERVED");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 45)] = _("RESERVED");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 46)] = _("RESERVED");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 47)] = _("RESERVED");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 48)] = _("ACTIVATE_NEW_IMAGE_ACK");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 49)] = _("ACTIVATE_NEW_IMAGE_NACK");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 50)] = _("START_BLOCK_ACK");
    m_hashType[MAKE_CLASSTYPE_LONG(0, 51)] = _("START_BLOCK_NACK");

    // ALARM
    m_hashType[MAKE_CLASSTYPE_LONG(1, 0)]  = _("ALARM_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(1, 1)]  = _("WARNING");
    m_hashType[MAKE_CLASSTYPE_LONG(1, 2)]  = _("ALARM");
    m_hashType[MAKE_CLASSTYPE_LONG(1, 3)]  = _("SOUND");
    m_hashType[MAKE_CLASSTYPE_LONG(1, 4)]  = _("LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(1, 5)]  = _("POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(1, 6)]  = _("EMERGENCY_STOP");
    m_hashType[MAKE_CLASSTYPE_LONG(1, 7)]  = _("EMERGENCY_PAUSE");
    m_hashType[MAKE_CLASSTYPE_LONG(1, 8)]  = _("EMERGENCY_RESET");
    m_hashType[MAKE_CLASSTYPE_LONG(1, 9)]  = _("EMERGENCY_RESUME");
    m_hashType[MAKE_CLASSTYPE_LONG(1, 10)] = _("ARM");
    m_hashType[MAKE_CLASSTYPE_LONG(1, 11)] = _("DISARM");

    // SECURITY
    m_hashType[MAKE_CLASSTYPE_LONG(2, 0)]  = _("SECURITY_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 1)]  = _("MOTION");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 2)]  = _("GLASS BREAK");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 3)]  = _("BEAM BREAK");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 4)]  = _("SENSOR TAMPER");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 5)]  = _("SHOCK SENSOR");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 6)]  = _("SMOKE SENSOR");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 7)]  = _("HEAT_SENSOR");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 8)]  = _("PANIC SWITCH");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 9)]  = _("DOOR CONTACT");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 10)] = _("WINDOW CONTACT");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 11)] = _("CO SENSOR");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 12)] = _("FROST DETECTED");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 13)] = _("FLAME DETECTED");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 14)] = _("OXYGEN LOW DETECTED");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 15)] = _("WEIGHT DETECTED");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 16)] = _("WATER DETECTED");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 17)] = _("CONDENSATION DETECTED");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 18)] = _("SOUND DETECTED");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 19)] = _("HARMFUL_SOUND_LEVEL");
    m_hashType[MAKE_CLASSTYPE_LONG(2, 20)] = _("TAMPER DETECTED");

    // MEASUREMENT
    m_hashType[MAKE_CLASSTYPE_LONG(10, 0)]  = _("MEASUREMENT_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 1)]  = _("COUNT");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 2)]  = _("LENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 3)]  = _("MASS");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 4)]  = _("TIME");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 5)]  = _("ELECTRIC_CURRENT");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 6)]  = _("TEMPERATURE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 7)]  = _("AMOUNT_OF_SUBSTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 8)]  = _("INTENSITY_OF_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 9)]  = _("FREQUENCY");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 10)] = _("RADIOACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 11)] = _("FORCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 12)] = _("PRESSURE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 13)] = _("ENERGY");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 14)] = _("POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 15)] = _("ELECTRICAL_CHARGE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 16)] = _("ELECTRICAL_POTENTIAL");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 17)] = _("ELECTRICAL_CAPACITANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 18)] = _("ELECTRICAL_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 19)] = _("ELECTRICAL_CONDUCTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 20)] = _("MAGNETIC_FIELD_STRENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 21)] = _("MAGNETIC_FLUX");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 22)] = _("MAGNETIC_FLUX_DENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 23)] = _("INDUCTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 24)] = _("FLUX_OF_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 25)] = _("ILLUMINANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 26)] = _("RADIATION_DOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 27)] = _("CATALYTIC_ACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 28)] = _("VOLUME");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 29)] = _("SOUND_INTENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 30)] = _("ANGLE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 31)] = _("POSITION");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 32)] = _("SPEED");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 33)] = _("ACCELERATION");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 34)] = _("TENSION");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 35)] = _("HUMIDITY");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 36)] = _("FLOW");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 27)] = _("THERMAL_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 38)] = _("REFRACTIVE_POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 39)] = _("DYNAMIC_VISCOSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 40)] = _("SOUND_IMPEDANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 41)] = _("SOUND_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 42)] = _("ELECTRIC_ELASTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 43)] = _("LUMINOUS_ENERGY");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 44)] = _("LUMINANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 45)] = _("CHEMICAL_CONCENTRATION");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 46)] = _("ABSORBED_DOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 47)] = _("DOSE_EQUIVALENT");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 49)] = _("DEWPOINT");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 50)] = _("RELATIVE_LEVEL");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 51)] = _("ALTITUDE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 52)] = _("AREA");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 53)] = _("RADIANT INTENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 54)] = _("RADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 55)] = _("IRRADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 56)] = _("SPECTRAL RADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 57)] = _("SPECTRAL IRRADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 58)] = _("SOUND PRESSURE");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 59)] = _("SOUND DENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(10, 59)] = _("SOUND LEVEL");

    // Class 15 (0x0f) -- DATA
    m_hashType[MAKE_CLASSTYPE_LONG(15, 0)] = _("DATA_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(15, 1)] = _("IO-VALUE");
    m_hashType[MAKE_CLASSTYPE_LONG(15, 2)] = _("AD-VALUE");
    m_hashType[MAKE_CLASSTYPE_LONG(15, 3)] = _("DA-VALUE");
    m_hashType[MAKE_CLASSTYPE_LONG(15, 4)] = _("RELATIVE_STRENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(15, 5)] = _("SIGNAL_LEVEL");
    m_hashType[MAKE_CLASSTYPE_LONG(15, 6)] = _("SIGNAL_QUALITY");
    m_hashType[MAKE_CLASSTYPE_LONG(15, 7)] = _("COUNT");

    // class 20 (0x14) -- INFORMATION
    m_hashType[MAKE_CLASSTYPE_LONG(20, 0)]  = _("INFORMATION_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 1)]  = _("BUTTON");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 2)]  = _("MOUSE");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 3)]  = _("ON");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 4)]  = _("OFF");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 5)]  = _("ALIVE");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 6)]  = _("TERMINATING");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 7)]  = _("OPENED");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 8)]  = _("CLOSED");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 9)]  = _("NODE_HEARTBEAT");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 10)] = _("BELOW_LIMIT");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 11)] = _("ABOVE_LIMIT");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 12)] = _("PULSE");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 13)] = _("ERROR");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 14)] = _("RESUMED");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 15)] = _("PAUSED");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 17)] = _("GOOD_MORNING");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 18)] = _("GOOD_DAY");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 19)] = _("GOOD_AFTERNOON");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 20)] = _("GOOD_EVENING");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 21)] = _("GOOD_NIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 22)] = _("SEE_YOU_SOON");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 23)] = _("GOODBYE");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 24)] = _("STOP");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 25)] = _("START");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 26)] = _("RESET_COMPLETED");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 27)] = _("INTERRUPTED");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 28)] = _("PREPARING_TO_SLEEP");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 29)] = _("WOKEN_UP");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 30)] = _("DUSK");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 31)] = _("DAWN");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 32)] = _("ACTIVE");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 33)] = _("INACTIVE");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 34)] = _("BUSY");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 35)] = _("IDLE");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 36)] = _("STREAM_DATA");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 37)] = _("TOKEN_ACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 38)] = _("STREAM_DATA_WITH_ZONE");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 39)] = _("CONFIRM");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 40)] = _("LEVEL_CHANGED");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 41)] = _("WARNING");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 42)] = _("STATE");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 43)] = _("ACTION_TRIGGER");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 44)] = _("SUNRISE");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 45)] = _("SUNSET");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 46)] = _("START_OF_RECORD");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 47)] = _("END_OF_RECORD");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 48)] = _("PRESET_ACTIVE");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 49)] = _("DETECT");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 50)] = _("OVERLOW");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 51)] = _("BIG_LEVEL_CHANGE");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 52)] = _("CIVIL_SUNRISE_TWILIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 53)] = _("CIVIL_SUNSET_TWILIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 54)] =
      _("NAUTICAL_SUNRISE_TWILIGHT_START");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 55)] =
      _("NAUTICAL_SUNSET_TWILIGHT_START");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 56)] =
      _("ASTRONOMICAL_SUNRISE_TWILIGHT_START");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 57)] =
      _("ASTRONOMICAL_SUNSET_TWILIGHT_START");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 58)] = _("CALCULATED_NOON");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 59)] = _("SHUTTER_UP");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 60)] = _("SHUTTER_DOWN");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 61)] = _("SHUTTER_LEFT");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 62)] = _("SHUTTER_RIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 63)] = _("SHUTTER_END_TOP");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 64)] = _("SHUTTER_END_BOTTOM");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 65)] = _("SHUTTER_END_MIDDLE");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 66)] = _("SHUTTER_END_PRESET");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 67)] = _("SHUTTER_END_LEFT");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 68)] = _("SHUTTER_END_RIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 69)] = _("LONG_CLICK");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 70)] = _("SINGLE_CLICK");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 71)] = _("DOUBLE_CLICK");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 72)] = _("DATE");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 73)] = _("TIME");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 74)] = _("WEEKDAY");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 75)] = _("LOCK");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 76)] = _("UNLOCK");
    m_hashType[MAKE_CLASSTYPE_LONG(20, 77)] = _("DATETIME");

    // class 30 (0x1E) -- CONTROL
    m_hashType[MAKE_CLASSTYPE_LONG(30, 0)]  = _("CONTROL_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 1)]  = _("MUTE");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 2)]  = _("ALL_LAMPS");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 3)]  = _("OPEN");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 4)]  = _("CLOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 5)]  = _("TURNON");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 6)]  = _("TURNOFF");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 7)]  = _("START");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 8)]  = _("STOP");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 9)]  = _("RESET");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 10)] = _("INTERRUPT");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 11)] = _("SLEEP");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 12)] = _("WAKEUP");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 13)] = _("RESUME");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 14)] = _("PAUSE");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 15)] = _("ACTIVATE");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 16)] = _("DEACTIVATE");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 20)] = _("DIM_LAMPS");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 21)] = _("CHANGE_CHANNEL");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 22)] = _("CHANGE_LEVEL");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 23)] = _("RELATIVE_CHANGE_LEVEL");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 24)] = _("MEASUREMENT_REQUEST");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 25)] = _("STREAM_DATA");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 26)] = _("SYNC");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 27)] = _("ZONED_STREAM_DATA");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 28)] = _("SET_PRESET");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 29)] = _("TOGGLE_STATE");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 30)] = _("TIMED_PULSE_ON");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 31)] = _("TIMED_PULSE_OFF");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 32)] = _("SET_COUNTRY_LANGUAGE");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 33)] = _("BIG_CHANGE_LEVEL");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 34)] = _("SHUTTER_UP");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 35)] = _("SHUTTER_DOWN");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 36)] = _("SHUTTER_LEFT");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 37)] = _("SHUTTER_RIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 38)] = _("SHUTTER_MIDDLE");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 39)] = _("SHUTTER_PRESET");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 40)] = _("ALL_LAMPS_ON");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 41)] = _("ALL_LAMPS_OFF");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 42)] = _("LOCK");
    m_hashType[MAKE_CLASSTYPE_LONG(30, 43)] = _("UNLOCK");

    // class 40 (0x28) -- MULTIMEDIA
    m_hashType[MAKE_CLASSTYPE_LONG(40, 0)]  = _("MULTIMEDIA_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 1)]  = _("PLAYBACKL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 2)]  = _("NAVIGATOR_KEY_ENGL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 3)]  = _("ADJUST_CONTRASTL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 4)]  = _("ADJUST_FOCUSL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 5)]  = _("ADJUST_TINTL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 6)]  = _("ADJUST_COLOUR_BALANCEL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 7)]  = _("ADJUST_BRIGHTNESSL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 8)]  = _("ADJUST_HUEL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 9)]  = _("ADJUST_BASSL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 10)] = _("ADJUST_TREBLEL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 11)] = _("ADJUST_MASTER_VOLUMEL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 12)] = _("ADJUST_FRONT_VOLUMEL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 13)] = _("ADJUST_CENTRE_VOLUMEL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 14)] = _("ADJUST_REAR_VOLUMEL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 15)] = _("ADJUST_SIDE_VOLUMEL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 20)] = _("ADJUST_SELECT_DISKL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 21)] = _("ADJUST_SELECT_TRACKL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 22)] = _("ADJUST_SELECT_ALBUML");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 23)] = _("ADJUST_SELECT_CHANNELL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 24)] = _("ADJUST_SELECT_PAGEL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 25)] = _("ADJUST_SELECT_CHAPTERL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 26)] = _("ADJUST_SELECT_SCREEN_FORMATL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 27)] = _("ADJUST_SELECT_INPUT_SOURCEL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 28)] = _("ADJUST_SELECT_OUTPUTL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 29)] = _("RECORDL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 30)] = _("SET_RECORDING_VOLUMEL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 40)] = _("TIVO_FUNCTIONL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 50)] = _("GET_CURRENT_TITLEL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 51)] = _("SET_POSITIONL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 52)] = _("GET_MEDIA_INFOL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 53)] = _("REMOVE_ITEML");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 54)] = _("REMOVE_ALL_ITEMSL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 55)] = _("SAVE_ALBUML");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 60)] = _("CONTROLL");
    m_hashType[MAKE_CLASSTYPE_LONG(40, 61)] = _("CONTROL_RESPONSEL");

    // class 50 (0x32) -- AOL
    m_hashType[MAKE_CLASSTYPE_LONG(50, 0)]  = _("AOL_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 1)]  = _("UNPLUGGED_POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 2)]  = _("UNPLUGGED_LAN");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 3)]  = _("CHASSIS_INTRUSION");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 4)]  = _("PROCESSOR_REMOVAL");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 5)]  = _("ENVIRONMENT_ERROR");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 6)]  = _("HIGH_TEMPERATURE");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 7)]  = _("FAN_SPEED");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 8)]  = _("VOLTAGE_FLUCTUATIONS");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 9)]  = _("OS_ERROR");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 10)] = _("POWER_ON_ERROR");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 11)] = _("SYSTEM_HUNG");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 12)] = _("COMPONENT_FAILURE");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 13)] = _("REBOOT_UPON_FAILURE");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 14)] = _("REPAIR_OPERATING_SYSTEM");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 15)] = _("UPDATE_BIOS_IMAGE");
    m_hashType[MAKE_CLASSTYPE_LONG(50, 16)] = _("UPDATE_DIAGNOSTIC_PROCEDURE");

    // MEASUREMENT64
    m_hashType[MAKE_CLASSTYPE_LONG(60, 0)]  = _("MEASUREMENT_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 1)]  = _("COUNT");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 2)]  = _("LENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 3)]  = _("MASS");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 4)]  = _("TIME");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 5)]  = _("ELECTRIC_CURRENT");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 6)]  = _("TEMPERATURE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 7)]  = _("AMOUNT_OF_SUBSTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 8)]  = _("INTENSITY_OF_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 9)]  = _("FREQUENCY");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 10)] = _("RADIOACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 11)] = _("FORCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 12)] = _("PRESSURE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 13)] = _("ENERGY");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 14)] = _("POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 15)] = _("ELECTRICAL_CHARGE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 16)] = _("ELECTRICAL_POTENTIAL");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 17)] = _("ELECTRICAL_CAPACITANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 18)] = _("ELECTRICAL_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 19)] = _("ELECTRICAL_CONDUCTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 20)] = _("MAGNETIC_FIELD_STRENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 21)] = _("MAGNETIC_FLUX");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 22)] = _("MAGNETIC_FLUX_DENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 23)] = _("INDUCTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 24)] = _("FLUX_OF_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 25)] = _("ILLUMINANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 26)] = _("RADIATION_DOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 27)] = _("CATALYTIC_ACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 28)] = _("VOLUME");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 29)] = _("SOUND_INTENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 30)] = _("ANGLE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 31)] = _("POSITION");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 32)] = _("SPEED");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 33)] = _("ACCELERATION");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 34)] = _("TENSION");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 35)] = _("HUMIDITY");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 36)] = _("FLOW");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 27)] = _("THERMAL_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 38)] = _("REFRACTIVE_POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 39)] = _("DYNAMIC_VISCOSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 40)] = _("SOUND_IMPEDANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 41)] = _("SOUND_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 42)] = _("ELECTRIC_ELASTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 43)] = _("LUMINOUS_ENERGY");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 44)] = _("LUMINANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 45)] = _("CHEMICAL_CONCENTRATION");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 46)] = _("ABSORBED_DOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 47)] = _("DOSE_EQUIVALENT");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 49)] = _("DEWPOINT");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 50)] = _("RELATIVE_LEVEL");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 51)] = _("ALTITUDE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 52)] = _("AREA");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 53)] = _("RADIANT INTENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 54)] = _("RADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 55)] = _("IRRADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 56)] = _("SPECTRAL RADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 57)] = _("SPECTRAL IRRADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 58)] = _("SOUND PRESSURE");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 59)] = _("SOUND DENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(60, 59)] = _("SOUND LEVEL");

    // VSCP_CLASS1_MEASUREZONE
    m_hashType[MAKE_CLASSTYPE_LONG(65, 0)]  = _("MEASUREMENT_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 1)]  = _("COUNT");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 2)]  = _("LENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 3)]  = _("MASS");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 4)]  = _("TIME");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 5)]  = _("ELECTRIC_CURRENT");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 6)]  = _("TEMPERATURE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 7)]  = _("AMOUNT_OF_SUBSTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 8)]  = _("INTENSITY_OF_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 9)]  = _("FREQUENCY");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 10)] = _("RADIOACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 11)] = _("FORCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 12)] = _("PRESSURE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 13)] = _("ENERGY");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 14)] = _("POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 15)] = _("ELECTRICAL_CHARGE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 16)] = _("ELECTRICAL_POTENTIAL");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 17)] = _("ELECTRICAL_CAPACITANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 18)] = _("ELECTRICAL_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 19)] = _("ELECTRICAL_CONDUCTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 20)] = _("MAGNETIC_FIELD_STRENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 21)] = _("MAGNETIC_FLUX");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 22)] = _("MAGNETIC_FLUX_DENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 23)] = _("INDUCTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 24)] = _("FLUX_OF_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 25)] = _("ILLUMINANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 26)] = _("RADIATION_DOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 27)] = _("CATALYTIC_ACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 28)] = _("VOLUME");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 29)] = _("SOUND_INTENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 30)] = _("ANGLE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 31)] = _("POSITION");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 32)] = _("SPEED");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 33)] = _("ACCELERATION");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 34)] = _("TENSION");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 35)] = _("HUMIDITY");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 36)] = _("FLOW");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 27)] = _("THERMAL_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 38)] = _("REFRACTIVE_POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 39)] = _("DYNAMIC_VISCOSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 40)] = _("SOUND_IMPEDANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 41)] = _("SOUND_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 42)] = _("ELECTRIC_ELASTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 43)] = _("LUMINOUS_ENERGY");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 44)] = _("LUMINANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 45)] = _("CHEMICAL_CONCENTRATION");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 46)] = _("ABSORBED_DOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 47)] = _("DOSE_EQUIVALENT");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 49)] = _("DEWPOINT");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 50)] = _("RELATIVE_LEVEL");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 51)] = _("ALTITUDE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 52)] = _("AREA");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 53)] = _("RADIANT INTENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 54)] = _("RADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 55)] = _("IRRADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 56)] = _("SPECTRAL RADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 57)] = _("SPECTRAL IRRADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 58)] = _("SOUND PRESSURE");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 59)] = _("SOUND DENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(65, 59)] = _("SOUND LEVEL");

    // VSCP_CLASS1_MEASUREMENT32
    m_hashType[MAKE_CLASSTYPE_LONG(70, 0)]  = _("MEASUREMENT_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 1)]  = _("COUNT");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 2)]  = _("LENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 3)]  = _("MASS");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 4)]  = _("TIME");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 5)]  = _("ELECTRIC_CURRENT");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 6)]  = _("TEMPERATURE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 7)]  = _("AMOUNT_OF_SUBSTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 8)]  = _("INTENSITY_OF_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 9)]  = _("FREQUENCY");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 10)] = _("RADIOACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 11)] = _("FORCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 12)] = _("PRESSURE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 13)] = _("ENERGY");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 14)] = _("POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 15)] = _("ELECTRICAL_CHARGE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 16)] = _("ELECTRICAL_POTENTIAL");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 17)] = _("ELECTRICAL_CAPACITANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 18)] = _("ELECTRICAL_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 19)] = _("ELECTRICAL_CONDUCTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 20)] = _("MAGNETIC_FIELD_STRENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 21)] = _("MAGNETIC_FLUX");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 22)] = _("MAGNETIC_FLUX_DENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 23)] = _("INDUCTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 24)] = _("FLUX_OF_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 25)] = _("ILLUMINANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 26)] = _("RADIATION_DOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 27)] = _("CATALYTIC_ACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 28)] = _("VOLUME");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 29)] = _("SOUND_INTENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 30)] = _("ANGLE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 31)] = _("POSITION");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 32)] = _("SPEED");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 33)] = _("ACCELERATION");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 34)] = _("TENSION");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 35)] = _("HUMIDITY");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 36)] = _("FLOW");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 27)] = _("THERMAL_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 38)] = _("REFRACTIVE_POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 39)] = _("DYNAMIC_VISCOSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 40)] = _("SOUND_IMPEDANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 41)] = _("SOUND_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 42)] = _("ELECTRIC_ELASTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 43)] = _("LUMINOUS_ENERGY");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 44)] = _("LUMINANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 45)] = _("CHEMICAL_CONCENTRATION");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 46)] = _("ABSORBED_DOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 47)] = _("DOSE_EQUIVALENT");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 49)] = _("DEWPOINT");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 50)] = _("RELATIVE_LEVEL");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 51)] = _("ALTITUDE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 52)] = _("AREA");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 53)] = _("RADIANT INTENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 54)] = _("RADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 55)] = _("IRRADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 56)] = _("SPECTRAL RADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 57)] = _("SPECTRAL IRRADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 58)] = _("SOUND PRESSURE");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 59)] = _("SOUND DENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(70, 59)] = _("SOUND LEVEL");

    // VSCP_CLASS1_SETVALUEZONE
    m_hashType[MAKE_CLASSTYPE_LONG(85, 0)]  = _("MEASUREMENT_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 1)]  = _("COUNT");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 2)]  = _("LENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 3)]  = _("MASS");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 4)]  = _("TIME");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 5)]  = _("ELECTRIC_CURRENT");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 6)]  = _("TEMPERATURE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 7)]  = _("AMOUNT_OF_SUBSTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 8)]  = _("INTENSITY_OF_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 9)]  = _("FREQUENCY");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 10)] = _("RADIOACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 11)] = _("FORCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 12)] = _("PRESSURE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 13)] = _("ENERGY");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 14)] = _("POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 15)] = _("ELECTRICAL_CHARGE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 16)] = _("ELECTRICAL_POTENTIAL");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 17)] = _("ELECTRICAL_CAPACITANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 18)] = _("ELECTRICAL_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 19)] = _("ELECTRICAL_CONDUCTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 20)] = _("MAGNETIC_FIELD_STRENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 21)] = _("MAGNETIC_FLUX");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 22)] = _("MAGNETIC_FLUX_DENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 23)] = _("INDUCTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 24)] = _("FLUX_OF_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 25)] = _("ILLUMINANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 26)] = _("RADIATION_DOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 27)] = _("CATALYTIC_ACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 28)] = _("VOLUME");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 29)] = _("SOUND_INTENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 30)] = _("ANGLE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 31)] = _("POSITION");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 32)] = _("SPEED");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 33)] = _("ACCELERATION");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 34)] = _("TENSION");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 35)] = _("HUMIDITY");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 36)] = _("FLOW");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 27)] = _("THERMAL_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 38)] = _("REFRACTIVE_POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 39)] = _("DYNAMIC_VISCOSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 40)] = _("SOUND_IMPEDANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 41)] = _("SOUND_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 42)] = _("ELECTRIC_ELASTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 43)] = _("LUMINOUS_ENERGY");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 44)] = _("LUMINANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 45)] = _("CHEMICAL_CONCENTRATION");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 46)] = _("ABSORBED_DOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 47)] = _("DOSE_EQUIVALENT");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 49)] = _("DEWPOINT");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 50)] = _("RELATIVE_LEVEL");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 51)] = _("ALTITUDE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 52)] = _("AREA");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 53)] = _("RADIANT INTENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 54)] = _("RADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 55)] = _("IRRADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 56)] = _("SPECTRAL RADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 57)] = _("SPECTRAL IRRADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 58)] = _("SOUND PRESSURE");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 59)] = _("SOUND DENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(85, 59)] = _("SOUND LEVEL");

    // class 90 (0x5A) -- WEATHER
    m_hashType[MAKE_CLASSTYPE_LONG(90, 0)]  = _("WEATHER_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 1)]  = _("WEATHER_SEASONS_WINTER");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 2)]  = _("WEATHER_SEASONS_SPRING");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 3)]  = _("WEATHER_SEASONS_SUMMER");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 4)]  = _("WEATHER_SEASONS_AUTUMN");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 5)]  = _("WEATHER_WIND_NONE");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 6)]  = _("WEATHER_WIND_LOW");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 7)]  = _("WEATHER_WIND_MEDIUM");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 8)]  = _("WEATHER_WIND_HIGH");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 9)]  = _("WEATHER_WIND_VERY_HIGH");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 10)] = _("WEATHER_AIR_FOGGY");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 11)] = _("WEATHER_AIR_FREEZING");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 12)] = _("WEATHER_AIR_COLD");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 13)] = _("WEATHER_AIR_VERY_COLD");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 14)] = _("WEATHER_AIR_NORMAL");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 15)] = _("WEATHER_AIR_HOT");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 16)] = _("WEATHER_AIR_VERY_HOT");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 17)] = _("WEATHER_AIR_POLLUTION_LOW");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 18)] = _("WEATHER_AIR_POLLUTION_MEDIUM");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 19)] = _("WEATHER_AIR_POLLUTION_HIGH");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 20)] = _("WEATHER_AIR_HUMID");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 21)] = _("WEATHER_AIR_DRY");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 22)] = _("WEATHER_SOIL_HUMID");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 23)] = _("WEATHER_SOIL_DRY");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 24)] = _("WEATHER_RAIN_NONE");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 25)] = _("WEATHER_RAIN_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 26)] = _("WEATHER_RAIN_HEAVY");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 27)] = _("WEATHER_RAIN_VERY_HEAVY");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 28)] = _("WEATHER_SUN_NONE");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 29)] = _("WEATHER_SUN_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 30)] = _("WEATHER_SUN_HEAVY");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 31)] = _("WEATHER_SNOW_NONE");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 32)] = _("WEATHER_SNOW_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 33)] = _("WEATHER_SNOW_HEAVY");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 34)] = _("WEATHER_DEW_POINT");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 35)] = _("WEATHER_STORM");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 36)] = _("WEATHER_FLOOD");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 37)] = _("WEATHER_EARTHQUAKE");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 38)] = _("WEATHER_NUCLEAR_DISASTER");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 39)] = _("WEATHER_FIRE");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 40)] = _("WEATHER_LIGHTNING");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 41)] = _("WEATHER_UV_RADIATION_LOW");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 42)] = _("WEATHER_UV_RADIATION_MEDIUM");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 43)] = _("WEATHER_UV_RADIATION_NORMAL");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 44)] = _("WEATHER_UV_RADIATION_HIGH");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 45)] =
      _("WEATHER_UV_RADIATION_VERY_HIGH");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 46)] = _("WEATHER_WARNING_LEVEL1");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 47)] = _("WEATHER_WARNING_LEVEL2");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 48)] = _("WEATHER_WARNING_LEVEL3");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 49)] = _("WEATHER_WARNING_LEVEL4");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 50)] = _("WEATHER_WARNING_LEVEL5");
    m_hashType[MAKE_CLASSTYPE_LONG(90, 51)] = _("WEATHER_ARMAGEDON");

    // class 95 (0x5F) -- WEATHER_FORECAST
    m_hashType[MAKE_CLASSTYPE_LONG(95, 0)]  = _("WEATHER_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 1)]  = _("WEATHER_SEASONS_WINTER");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 2)]  = _("WEATHER_SEASONS_SPRING");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 3)]  = _("WEATHER_SEASONS_SUMMER");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 4)]  = _("WEATHER_SEASONS_AUTUMN");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 5)]  = _("WEATHER_WIND_NONE");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 6)]  = _("WEATHER_WIND_LOW");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 7)]  = _("WEATHER_WIND_MEDIUM");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 8)]  = _("WEATHER_WIND_HIGH");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 9)]  = _("WEATHER_WIND_VERY_HIGH");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 10)] = _("WEATHER_AIR_FOGGY");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 11)] = _("WEATHER_AIR_FREEZING");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 12)] = _("WEATHER_AIR_COLD");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 13)] = _("WEATHER_AIR_VERY_COLD");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 14)] = _("WEATHER_AIR_NORMAL");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 15)] = _("WEATHER_AIR_HOT");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 16)] = _("WEATHER_AIR_VERY_HOT");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 17)] = _("WEATHER_AIR_POLLUTION_LOW");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 18)] = _("WEATHER_AIR_POLLUTION_MEDIUM");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 19)] = _("WEATHER_AIR_POLLUTION_HIGH");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 20)] = _("WEATHER_AIR_HUMID");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 21)] = _("WEATHER_AIR_DRY");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 22)] = _("WEATHER_SOIL_HUMID");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 23)] = _("WEATHER_SOIL_DRY");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 24)] = _("WEATHER_RAIN_NONE");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 25)] = _("WEATHER_RAIN_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 26)] = _("WEATHER_RAIN_HEAVY");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 27)] = _("WEATHER_RAIN_VERY_HEAVY");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 28)] = _("WEATHER_SUN_NONE");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 29)] = _("WEATHER_SUN_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 30)] = _("WEATHER_SUN_HEAVY");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 31)] = _("WEATHER_SNOW_NONE");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 32)] = _("WEATHER_SNOW_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 33)] = _("WEATHER_SNOW_HEAVY");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 34)] = _("WEATHER_DEW_POINT");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 35)] = _("WEATHER_STORM");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 36)] = _("WEATHER_FLOOD");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 37)] = _("WEATHER_EARTHQUAKE");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 38)] = _("WEATHER_NUCLEAR_DISASTER");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 39)] = _("WEATHER_FIRE");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 40)] = _("WEATHER_LIGHTNING");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 41)] = _("WEATHER_UV_RADIATION_LOW");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 42)] = _("WEATHER_UV_RADIATION_MEDIUM");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 43)] = _("WEATHER_UV_RADIATION_NORMAL");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 44)] = _("WEATHER_UV_RADIATION_HIGH");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 45)] =
      _("WEATHER_UV_RADIATION_VERY_HIGH");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 46)] = _("WEATHER_WARNING_LEVEL1");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 47)] = _("WEATHER_WARNING_LEVEL2");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 48)] = _("WEATHER_WARNING_LEVEL3");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 49)] = _("WEATHER_WARNING_LEVEL4");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 50)] = _("WEATHER_WARNING_LEVEL5");
    m_hashType[MAKE_CLASSTYPE_LONG(95, 51)] = _("WEATHER_ARMAGEDON");

    // class 100 (0x64) -- PHONE
    m_hashType[MAKE_CLASSTYPE_LONG(100, 0)] = _("PHONE_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(100, 1)] = _("INCOMING_CALL");
    m_hashType[MAKE_CLASSTYPE_LONG(100, 2)] = _("OUTGOING_CALL");
    m_hashType[MAKE_CLASSTYPE_LONG(100, 3)] = _("RING");
    m_hashType[MAKE_CLASSTYPE_LONG(100, 4)] = _("ANSWER");
    m_hashType[MAKE_CLASSTYPE_LONG(100, 5)] = _("HANGUP");
    m_hashType[MAKE_CLASSTYPE_LONG(100, 6)] = _("GIVEUP");
    m_hashType[MAKE_CLASSTYPE_LONG(100, 7)] = _("TRANSFER");
    m_hashType[MAKE_CLASSTYPE_LONG(100, 8)] = _("DATABASE_INFO");

    // class 102 (0x66) -- DISPLAY
    m_hashType[MAKE_CLASSTYPE_LONG(102, 0)]  = _("DISPLAY_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(102, 1)]  = _("CLEAR_DISPLAY");
    m_hashType[MAKE_CLASSTYPE_LONG(102, 2)]  = _("POSITION_CURSOR");
    m_hashType[MAKE_CLASSTYPE_LONG(102, 3)]  = _("WRITE_DISPLAY");
    m_hashType[MAKE_CLASSTYPE_LONG(102, 4)]  = _("WRITE_DISPLAY_BUFFER");
    m_hashType[MAKE_CLASSTYPE_LONG(102, 5)]  = _("SHOW_DISPLAY_BUFFER");
    m_hashType[MAKE_CLASSTYPE_LONG(102, 6)]  = _("SET_DISPLAY_BUFFER_PARAM");
    m_hashType[MAKE_CLASSTYPE_LONG(102, 32)] = _("SHOW_TEXT");
    m_hashType[MAKE_CLASSTYPE_LONG(102, 48)] = _("SHOW_LED");
    m_hashType[MAKE_CLASSTYPE_LONG(102, 49)] = _("SHOW_LED_COLOR");

    // class 110 (0x6E) -- REMOTE
    m_hashType[MAKE_CLASSTYPE_LONG(110, 0)]  = _("REMOTE_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(110, 1)]  = _("RC5");
    m_hashType[MAKE_CLASSTYPE_LONG(110, 2)]  = _("SONY12");
    m_hashType[MAKE_CLASSTYPE_LONG(110, 32)] = _("LIRC");
    m_hashType[MAKE_CLASSTYPE_LONG(110, 48)] = _("VSCP");

    // class 206 (0xCE) -- GPS
    m_hashType[MAKE_CLASSTYPE_LONG(206, 0)] = _("GPS_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(206, 1)] = _("POSITION");
    m_hashType[MAKE_CLASSTYPE_LONG(206, 2)] = _("SATELLITES");

    // class 212 (0xD4) -- WIRELESS
    m_hashType[MAKE_CLASSTYPE_LONG(212, 0)] = _("WIRELESS_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(212, 1)] = _("GSM_CELL");

    // class 507 (0x1FB) -- DIAGNOSTIC
    m_hashType[MAKE_CLASSTYPE_LONG(506, 0)]  = _("GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 1)]  = _("OVERVOLTAGE");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 2)]  = _("UNDERVOLTAGE");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 3)]  = _("VBUS LOW");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 4)]  = _("BATTERY LOW");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 5)]  = _("BATTERY FULL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 6)]  = _("BATTERY ERROR");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 7)]  = _("BATTERY OK");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 8)]  = _("OVERCURRENT");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 9)]  = _("CIRCUIT ERROR");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 10)] = _("SHORT CIRCUIT");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 11)] = _("OPEN CIRCUIT");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 12)] = _("MOIST");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 13)] = _("WIRE FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 14)] = _("WIRELESS FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 15)] = _("IR FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 16)] = _("1WIRE FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 17)] = _("RS222 FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 18)] = _("RS232 FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 19)] = _("RS423 FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 20)] = _("RS485 FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 21)] = _("CAN FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 22)] = _("LAN FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 23)] = _("USB FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 24)] = _("WIFI FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 25)] = _("NFC RFID FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 26)] = _("LOW SIGNAL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 27)] = _("HIGH SIGNAL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 28)] = _("ADC FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 29)] = _("ALU FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 30)] = _("ASSERT");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 31)] = _("DAC FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 32)] = _("DMA FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 33)] = _("ETH FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 34)] = _("EXCEPTION");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 35)] = _("FPU FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 36)] = _("GPIO FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 37)] = _("I2C FAIL ");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 38)] = _("I2S FAIL ");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 39)] = _("INVALID CONFIG");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 40)] = _("MMU FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 41)] = _("NMI");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 42)] = _("OVERHEAT");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 43)] = _("PLL FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 44)] = _("POR FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 45)] = _("PWM FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 46)] = _("RAM FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 47)] = _("ROM FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 48)] = _("SPI FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 49)] = _("STACK FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 50)] = _("LIN FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 51)] = _("UART FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 52)] = _("UNHANDLED INT");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 53)] = _("MEMORY FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 54)] = _("VARIABLE RANGE");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 55)] = _("WDT");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 56)] = _("EEPROM FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 57)] = _("ENCRYPTION FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 58)] = _("BAD USER INPUT");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 59)] = _("DECRYPTION FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 60)] = _("NOISE");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 61)] = _("BOOTLOADER FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 62)] = _("PROGRAMFLOW FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 63)] = _("RTC FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 64)] = _("SYSTEM TEST FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 65)] = _("SENSOR FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 66)] = _("SAFESTATE");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 67)] = _("SIGNAL IMPLAUSIBLE");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 68)] = _("STORAGE FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 69)] = _("SELFTEST FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 70)] = _("ESD EMC EMI");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 71)] = _("TIMEOUT");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 72)] = _("LCD FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 73)] = _("TOUCHPANEL FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 74)] = _("NOLOAD");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 75)] = _("COOLING FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 76)] = _("HEATING FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 77)] = _("TX FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 78)] = _("RX FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(506, 78)] = _("EXTERNAL IC FAIL");

    // class 508 (0x1FC) -- ERROR
    m_hashType[MAKE_CLASSTYPE_LONG(508, 0)]  = _("SUCCESS");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 1)]  = _("ERROR");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 2)]  = _("-");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 3)]  = _("-");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 4)]  = _("-");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 5)]  = _("-");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 6)]  = _("-");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 7)]  = _("CHANNEL");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 8)]  = _("FIFO_EMPTY");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 9)]  = _("FIFO_FULL");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 10)] = _("FIFO_SIZE");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 11)] = _("FIFO_WAIT");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 12)] = _("GENERIC");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 13)] = _("HARDWARE");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 14)] = _("INIT_FAIL");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 15)] = _("INIT_MISSING");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 16)] = _("INIT_READY");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 17)] = _("NOT_SUPPORTED");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 18)] = _("OVERRUN");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 19)] = _("RECEIVER_EMPTY");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 20)] = _("REGISTER");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 21)] = _("TRANSMITTER_FULL");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 22)] = _("-");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 23)] = _("-");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 24)] = _("-");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 25)] = _("-");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 26)] = _("-");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 27)] = _("-");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 28)] = _("LIBRARY");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 29)] = _("PROCADDRESS");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 30)] = _("ONLY_ONE_INSTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 31)] = _("SUB_DRIVER");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 32)] = _("TIMEOUT");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 33)] = _("NOT_OPEN");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 34)] = _("PARAMETER");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 35)] = _("MEMORY");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 36)] = _("INTERNAL");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 37)] = _("COMMUNICATION");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 38)] = _("USER");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 39)] = _("PASSWORD");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 40)] = _("CONNECTION");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 41)] = _("INVALID_HANDLE");
    m_hashType[MAKE_CLASSTYPE_LONG(508, 42)] = _("OPERATION_FAILED");

    // class 509 (0x1FD) -- LOG
    m_hashType[MAKE_CLASSTYPE_LONG(509, 0)] = _("LOG_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(509, 1)] = _("MESSAGE");
    m_hashType[MAKE_CLASSTYPE_LONG(509, 2)] = _("START");
    m_hashType[MAKE_CLASSTYPE_LONG(509, 3)] = _("STOP");
    m_hashType[MAKE_CLASSTYPE_LONG(509, 4)] = _("LEVEL");

    // class 510 (0x1FE) -- LABORATORY
    m_hashType[MAKE_CLASSTYPE_LONG(510, 0)] = _("LABORATORY_GENERAL");

    // class 511 (0x1FF) -- LOCAL
    m_hashType[MAKE_CLASSTYPE_LONG(511, 0)] = _("LOCAL_GENERAL");

    // Level II Protocol functionality Class=1024 (0x400)
    m_hashType[MAKE_CLASSTYPE_LONG(1024, 0)]  = _("PROTOCOL2_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(1024, 1)]  = _("READ_REGISTER");
    m_hashType[MAKE_CLASSTYPE_LONG(1024, 2)]  = _("WRITE_REGISTER");
    m_hashType[MAKE_CLASSTYPE_LONG(1024, 3)]  = _("READ_WRITE_RESPONSE");
    m_hashType[MAKE_CLASSTYPE_LONG(1024, 20)] = _("HIGH_END_SERVER_CAPS");
    m_hashType[MAKE_CLASSTYPE_LONG(1024, 32)] =
      _("LEVEL2_WHO_IS_THERE_RESPONSE");

    // Level II Control functionality Class=1025 (0x401)
    m_hashType[MAKE_CLASSTYPE_LONG(1025, 0)] = _("CONTROL_GENERAL");

    // Level II Information functionality Class=1026 (0x402)
    m_hashType[MAKE_CLASSTYPE_LONG(1026, 0)] = _("INFORMATION_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(1026, 1)] = _("TOKEN_ACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1026, 2)] = _("HEART_BEAT");
    m_hashType[MAKE_CLASSTYPE_LONG(1026, 3)] = _("PROXY_HEART_BEAT");

    // Level II Text to speech functionality Class=1028 (0x404)
    m_hashType[MAKE_CLASSTYPE_LONG(1028, 0)] = _("SPEECH_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(1028, 1)] = _("TALK");

    // Level II Custom functionality Class=1029 (0x405)
    m_hashType[MAKE_CLASSTYPE_LONG(1029, 0)] = _("CUSTOM_GENERAL");

    // Level II Display functionality Class=1030 (0x406)
    m_hashType[MAKE_CLASSTYPE_LONG(1030, 0)] = _("DISPLAY_GENERAL");

    // Level II Measurement class string Class=1040 (0x410)
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 0)]  = _("MEASUREMENT_STR_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 1)]  = _("COUNT");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 2)]  = _("LENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 3)]  = _("MASS");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 4)]  = _("TIME");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 5)]  = _("ELECTRIC_CURRENT");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 6)]  = _("TEMPERATURE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 7)]  = _("AMOUNT_OF_SUBSTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 8)]  = _("INTENSITY_OF_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 9)]  = _("FREQUENCY");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 10)] = _("RADIOACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 11)] = _("FORCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 12)] = _("PRESSURE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 13)] = _("ENERGY");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 14)] = _("POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 15)] = _("ELECTRICAL_CHARGE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 16)] = _("ELECTRICAL_POTENTIAL");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 17)] = _("ELECTRICAL_CAPACITANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 18)] = _("ELECTRICAL_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 19)] = _("ELECTRICAL_CONDUCTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 20)] = _("MAGNETIC_FIELD_STRENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 21)] = _("MAGNETIC_FLUX");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 22)] = _("MAGNETIC_FLUX_DENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 23)] = _("INDUCTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 24)] = _("FLUX_OF_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 25)] = _("ILLUMINANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 26)] = _("RADIATION_DOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 27)] = _("CATALYTIC_ACITIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 28)] = _("VOLUME");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 29)] = _("SOUND_INTENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 30)] = _("ANGLE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 31)] = _("POSITION");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 32)] = _("SPEED");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 33)] = _("ACCELERATION");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 34)] = _("TENSION");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 35)] = _("HUMIDITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 36)] = _("FLOW");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 27)] = _("THERMAL_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 38)] = _("REFRACTIVE_POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 39)] = _("DYNAMIC_VISCOSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 40)] = _("SOUND_IMPEDANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 41)] = _("SOUND_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 42)] = _("ELECTRIC_ELASTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 43)] = _("LUMINOUS_ENERGY");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 44)] = _("LUMINANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 45)] = _("CHEMICAL_CONCENTRATION");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 46)] = _("ABSORBED_DOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 47)] = _("DOSE_EQUIVALENT");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 49)] = _("DEWPOINT");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 50)] = _("RELATIVE_LEVEL");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 51)] = _("ALTITUDE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 52)] = _("AREA");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 53)] = _("RADIANT INTENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 54)] = _("RADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 55)] = _("IRRADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 56)] = _("SPECTRAL RADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 57)] = _("SPECTRAL IRRADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 58)] = _("SOUND PRESSURE");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 59)] = _("SOUND DENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1040, 59)] = _("SOUND LEVEL");

    // Level II Measurement class float Class=1060 (0x424)
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 0)]  = _("MEASUREMENT_STR_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 1)]  = _("COUNT");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 2)]  = _("LENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 3)]  = _("MASS");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 4)]  = _("TIME");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 5)]  = _("ELECTRIC_CURRENT");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 6)]  = _("TEMPERATURE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 7)]  = _("AMOUNT_OF_SUBSTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 8)]  = _("INTENSITY_OF_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 9)]  = _("FREQUENCY");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 10)] = _("RADIOACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 11)] = _("FORCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 12)] = _("PRESSURE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 13)] = _("ENERGY");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 14)] = _("POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 15)] = _("ELECTRICAL_CHARGE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 16)] = _("ELECTRICAL_POTENTIAL");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 17)] = _("ELECTRICAL_CAPACITANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 18)] = _("ELECTRICAL_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 19)] = _("ELECTRICAL_CONDUCTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 20)] = _("MAGNETIC_FIELD_STRENGTH");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 21)] = _("MAGNETIC_FLUX");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 22)] = _("MAGNETIC_FLUX_DENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 23)] = _("INDUCTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 24)] = _("FLUX_OF_LIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 25)] = _("ILLUMINANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 26)] = _("RADIATION_DOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 27)] = _("CATALYTIC_ACTIVITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 28)] = _("VOLUME");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 29)] = _("SOUND_INTENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 30)] = _("ANGLE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 31)] = _("POSITION");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 32)] = _("SPEED");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 33)] = _("ACCELERATION");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 34)] = _("TENSION");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 35)] = _("HUMIDITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 36)] = _("FLOW");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 27)] = _("THERMAL_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 38)] = _("REFRACTIVE_POWER");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 39)] = _("DYNAMIC_VISCOSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 40)] = _("SOUND_IMPEDANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 41)] = _("SOUND_RESISTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 42)] = _("ELECTRIC_ELASTANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 43)] = _("LUMINOUS_ENERGY");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 44)] = _("LUMINANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 45)] = _("CHEMICAL_CONCENTRATION");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 46)] = _("ABSORBED_DOSE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 47)] = _("DOSE_EQUIVALENT");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 49)] = _("DEWPOINT");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 50)] = _("RELATIVE_LEVEL");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 51)] = _("ALTITUDE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 52)] = _("AREA");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 53)] = _("RADIANT INTENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 54)] = _("RADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 55)] = _("IRRADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 56)] = _("SPECTRAL RADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 57)] = _("SPECTRAL IRRADIANCE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 58)] = _("SOUND PRESSURE");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 59)] = _("SOUND DENSITY");
    m_hashType[MAKE_CLASSTYPE_LONG(1060, 59)] = _("SOUND LEVEL");

    // Level II VSCPD daemon functionality Class=65535 (0xFFFF)
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 0)]  = _("VSCPD_GENERAL");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 1)]  = _("LOOP");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 3)]  = _("PAUSE");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 5)]  = _("ACTIVATE");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 5)]  = _("SECOND");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 6)]  = _("MINUTE");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 7)]  = _("HOUR");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 8)]  = _("NOON");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 9)]  = _("MIDNIGHT");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 11)] = _("WEEK");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 12)] = _("MONTH");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 13)] = _("QUARTER");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 14)] = _("YEAR");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 15)] = _("RANDOM_MINUTE");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 16)] = _("RANDOM_HOUR");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 17)] = _("RANDOM_DAY");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 18)] = _("RANDOM_WEEK");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 19)] = _("RANDOM_MONTH");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 20)] = _("RANDOM_YEAR");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 21)] = _("DUSK");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 22)] = _("DAWN");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 23)] = _("STARTING_UP");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 24)] = _("SHUTTING_DOWN");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 25)] = _("TIMER_STARTED");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 26)] = _("TIMER_PAUSED");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 27)] = _("TIMER_RESUMED");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 28)] = _("TIMER_STOPPED");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 29)] = _("TIMER_ELLPSED");
    m_hashType[MAKE_CLASSTYPE_LONG(65535, 30)] = _("NEW_CALCULATIONS");
}

////////////////////////////////////////////////////////////////////////////////
// Destructor
//

VSCPInformation::~VSCPInformation(void)
{
    m_hashClass.begin();
    m_hashType.clear();
}

////////////////////////////////////////////////////////////////////////////////
// getClassHashPointer
//

std::map<unsigned long, std::string> *
VSCPInformation::getClassHashPointer(void)
{
    return &m_hashClass;
}

////////////////////////////////////////////////////////////////////////////////
// getClassHashPointer
//

std::map<unsigned long, std::string> *
VSCPInformation::getTypeHashPointer(void)
{
    return &m_hashType;
}

////////////////////////////////////////////////////////////////////////////////
// getClassDescription
//

std::string &
VSCPInformation::getClassDescription(int vscp_class)
{
    return m_hashClass[vscp_class];
}

////////////////////////////////////////////////////////////////////////////////
// getClassDescription
//

std::string &
VSCPInformation::getTypeDescription(int vscp_class, int vscp_type)
{
    // Adjust for Level II mirror class of Level I events.
    if ((vscp_class >= 0x200) && (vscp_class < 0x400)) vscp_class -= 512;

    return m_hashType[MAKE_CLASSTYPE_LONG(vscp_class, vscp_type)];
}

////////////////////////////////////////////////////////////////////////////////
// fillTypeDescriptions
//

void
VSCPInformation::fillTypeDescriptions(std::deque<std::string> &strArray,
                                      unsigned int vscp_class,
                                      VSCPInformationFormat format)
{
    std::string str;

    VSCPHashType::iterator it;
    for (it = m_hashType.begin(); it != m_hashType.end(); ++it) {
        unsigned long key = it->first;
        std::string value = it->second;

        if (vscp_class == (key >> 16)) {

            switch (format) {

                case WITH_DECIMAL_PREFIX:
                    str = _("(") + str.Format(_("%lu"), key) + _(") ") + value;
                    (void)strArray.Add(str);
                    break;

                case WITH_HEX_PREFIX:
                    str =
                      _("(") + str.Format(_("%04lX"), key) + _(") ") + value;
                    (void)strArray.Add(str);
                    break;

                case WITH_DECIMAL_SUFFIX:
                    str = value + _(" (") +
                          str.Format(_("%lu"), (key & 0xffff)) + _(")");
                    (void)strArray.Add(str);
                    break;

                case WITH_HEX_SUFFIX:
                    str = value + _(" (") +
                          str.Format(_("%04lX"), (key & 0xffff)) + _(")");
                    (void)strArray.Add(str);
                    break;

                default:
                case DEFAULT:
                    (void)strArray.Add(value);
                    break;
            }
        }
    }
}
