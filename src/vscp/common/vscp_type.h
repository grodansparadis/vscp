/**
 * @brief           VSCP Level I/II type definition file
 * @file            vscp_type.h
 * @author          Ake Hedman, Grodans Paradis AB, www.vscp.org
 *
 *
 * @section description Description
 **********************************
 * This module contains the code that implements that 
 *
 *********************************************************************/

/* ******************************************************************************
 * 	VSCP (Very Simple Control Protocol) 
 * 	http://www.vscp.org
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2000-2015 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 *	This file is part of VSCP - Very Simple Control Protocol 	
 *	http://www.vscp.org
 *
 * ******************************************************************************
 */

#ifndef VSCP_TYPE_H
#define VSCP_TYPE_H

#define VSCP_TYPE_UNDEFINED                                 0

// VSCP Protocol Functionality
#define VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT                1
#define VSCP_TYPE_PROTOCOL_NEW_NODE_ONLINE                  2
#define VSCP_TYPE_PROTOCOL_PROBE_ACK                        3
#define VSCP_TYPE_PROTOCOL_SET_NICKNAME                     6
#define VSCP_TYPE_PROTOCOL_NICKNAME_ACCEPTED                7
#define VSCP_TYPE_PROTOCOL_DROP_NICKNAME                    8
#define VSCP_TYPE_PROTOCOL_READ_REGISTER                    9
#define VSCP_TYPE_PROTOCOL_RW_RESPONSE                      10
#define VSCP_TYPE_PROTOCOL_WRITE_REGISTER                   11
#define VSCP_TYPE_PROTOCOL_ENTER_BOOT_LOADER                12
#define VSCP_TYPE_PROTOCOL_ACK_BOOT_LOADER                  13
#define VSCP_TYPE_PROTOCOL_NACK_BOOT_LOADER                 14
#define VSCP_TYPE_PROTOCOL_START_BLOCK                      15
#define VSCP_TYPE_PROTOCOL_BLOCK_DATA                       16
#define VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK                   17
#define VSCP_TYPE_PROTOCOL_BLOCK_DATA_NACK                  18
#define VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA               19
#define VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK           20
#define VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_NACK          21
#define VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE               22
#define VSCP_TYPE_PROTOCOL_RESET_DEVICE                     23
#define VSCP_TYPE_PROTOCOL_PAGE_READ                        24
#define VSCP_TYPE_PROTOCOL_PAGE_WRITE                       25
#define VSCP_TYPE_PROTOCOL_RW_PAGE_RESPONSE                 26
#define VSCP_TYPE_PROTOCOL_HIGH_END_SERVER_PROBE            27
#define VSCP_TYPE_PROTOCOL_HIGH_END_SERVER_RESPONSE         28
#define VSCP_TYPE_PROTOCOL_INCREMENT_REGISTER               29
#define VSCP_TYPE_PROTOCOL_DECREMENT_REGISTER               30
#define VSCP_TYPE_PROTOCOL_WHO_IS_THERE                     31
#define VSCP_TYPE_PROTOCOL_WHO_IS_THERE_RESPONSE            32
#define VSCP_TYPE_PROTOCOL_GET_MATRIX_INFO                  33
#define VSCP_TYPE_PROTOCOL_GET_MATRIX_INFO_RESPONSE         34
#define VSCP_TYPE_PROTOCOL_GET_EMBEDDED_MDF                 35
#define VSCP_TYPE_PROTOCOL_GET_EMBEDDED_MDF_RESPONSE        36
#define VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ               37
#define VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE              38
#define VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE           39
#define VSCP_TYPE_PROTOCOL_GET_EVENT_INTEREST               40
#define VSCP_TYPE_PROTOCOL_GET_EVENT_INTEREST_RESPONSE      41
#define VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE_ACK           48
#define VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE_NACK          49
#define VSCP_TYPE_PROTOCOL_START_BLOCK_ACK                  50
#define VSCP_TYPE_PROTOCOL_START_BLOCK_NACK                 51

// Class 1 (0x01) -- ALARM
#define VSCP_TYPE_ALARM_GENERAL                             0
#define VSCP_TYPE_ALARM_WARNING                             1
#define VSCP_TYPE_ALARM_ALARM                               2
#define VSCP_TYPE_ALARM_SOUND                               3
#define VSCP_TYPE_ALARM_LIGHT                               4
#define VSCP_TYPE_ALARM_POWER                               5
#define VSCP_TYPE_ALARM_EMERGENCY_STOP                      6
#define VSCP_TYPE_ALARM_EMERGENCY_PAUSE                     7
#define VSCP_TYPE_ALARM_EMERGENCY_RESET                     8
#define VSCP_TYPE_ALARM_EMERGENCY_RESUME                    9

// Class 2 (0x02) -- SECURITY
#define VSCP_TYPE_SECURITY_GENERAL                          0
#define VSCP_TYPE_SECURITY_MOTION                           1
#define VSCP_TYPE_SECURITY_GLASS_BREAK                      2
#define VSCP_TYPE_SECURITY_BEAM_BREAK                       3
#define VSCP_TYPE_SECURITY_SENSOR_TAMPER                    4
#define VSCP_TYPE_SECURITY_SHOCK_SENSOR                     5
#define VSCP_TYPE_SECURITY_SMOKE_SENSOR                     6
#define VSCP_TYPE_SECURITY_HEAT_SENSOR                      7
#define VSCP_TYPE_SECURITY_PANIC_SWITCH                     8
#define VSCP_TYPE_SECURITY_DOOR_OPEN                        9
#define VSCP_TYPE_SECURITY_WINDOW_OPEN                      10
#define VSCP_TYPE_SECURITY_CO_SENSOR                        11
#define VSCP_TYPE_SECURITY_FROST_DETECTED                   12
#define VSCP_TYPE_SECURITY_FLAME_DETECTED                   13
#define VSCP_TYPE_SECURITY_OXYGEN_LOW                       14
#define VSCP_TYPE_SECURITY_WEIGHT_DETECTED                  15
#define VSCP_TYPE_SECURITY_WATER_DETECTED                   16
#define VSCP_TYPE_SECURITY_CONDENSATION_DETECTED            17
#define VSCP_TYPE_SECURITY_SOUND_DETECTED                   18
#define VSCP_TYPE_SECURITY_HARMFUL_SOUND_LEVEL              19
#define VSCP_TYPE_SECURITY_TAMPER                           20

// Class 10 (0x0a) -- MEASUREMENT
#define VSCP_TYPE_MEASUREMENT_GENERAL                       0
#define VSCP_TYPE_MEASUREMENT_COUNT                         1
#define VSCP_TYPE_MEASUREMENT_LENGTH                        2
#define VSCP_TYPE_MEASUREMENT_MASS                          3
#define VSCP_TYPE_MEASUREMENT_TIME                          4
#define VSCP_TYPE_MEASUREMENT_ELECTRIC_CURRENT              5
#define VSCP_TYPE_MEASUREMENT_TEMPERATURE                   6
#define VSCP_TYPE_MEASUREMENT_AMOUNT_OF_SUBSTANCE           7
#define VSCP_TYPE_MEASUREMENT_INTENSITY_OF_LIGHT            8
#define VSCP_TYPE_MEASUREMENT_FREQUENCY                     9
#define VSCP_TYPE_MEASUREMENT_RADIOACTIVITY                 10	// or other random event
#define VSCP_TYPE_MEASUREMENT_FORCE                         11
#define VSCP_TYPE_MEASUREMENT_PRESSURE                      12
#define VSCP_TYPE_MEASUREMENT_ENERGY                        13
#define VSCP_TYPE_MEASUREMENT_POWER                         14
#define VSCP_TYPE_MEASUREMENT_ELECTRICAL_CHARGE             15
#define VSCP_TYPE_MEASUREMENT_ELECTRICAL_POTENTIAL          16	// Voltage
#define VSCP_TYPE_MEASUREMENT_ELECTRICAL_CAPACITANCE        17
#define VSCP_TYPE_MEASUREMENT_ELECTRICAL_RECISTANCE         18
#define VSCP_TYPE_MEASUREMENT_ELECTRICAL_CONDUCTANCE        19
#define VSCP_TYPE_MEASUREMENT_MAGNETIC_FIELD_STRENGTH       20
#define VSCP_TYPE_MEASUREMENT_MAGNETIC_FLUX                 21
#define VSCP_TYPE_MEASUREMENT_MAGNETIC_FLUX_DENSITY         22
#define VSCP_TYPE_MEASUREMENT_INDUCTANCE                    23
#define VSCP_TYPE_MEASUREMENT_FLUX_OF_LIGHT                 24
#define VSCP_TYPE_MEASUREMENT_ILLUMINANCE                   25
#define VSCP_TYPE_MEASUREMENT_RADIATION_DOSE                26
#define VSCP_TYPE_MEASUREMENT_CATALYTIC_ACITIVITY           27
#define VSCP_TYPE_MEASUREMENT_VOLUME                        28
#define VSCP_TYPE_MEASUREMENT_SOUND_INTENSITY               29
#define VSCP_TYPE_MEASUREMENT_ANGLE                         30
#define VSCP_TYPE_MEASUREMENT_POSITION                      31
#define VSCP_TYPE_MEASUREMENT_SPEED                         32
#define VSCP_TYPE_MEASUREMENT_ACCELERATION                  33
#define VSCP_TYPE_MEASUREMENT_TENSION                       34
#define VSCP_TYPE_MEASUREMENT_HUMIDITY                      35	// Damp/moist (Hygrometer reading)
#define VSCP_TYPE_MEASUREMENT_FLOW                          36
#define VSCP_TYPE_MEASUREMENT_THERMAL_RESISTANCE            37
#define VSCP_TYPE_MEASUREMENT_REFRACTIVE_POWER              38
#define VSCP_TYPE_MEASUREMENT_DYNAMIC_VISCOSITY             39
#define VSCP_TYPE_MEASUREMENT_SOUND_IMPEDANCE               40
#define VSCP_TYPE_MEASUREMENT_SOUND_RESISTANCE              41
#define VSCP_TYPE_MEASUREMENT_ELECTRIC_ELASTANCE            42
#define VSCP_TYPE_MEASUREMENT_LUMINOUS_ENERGY               43
#define VSCP_TYPE_MEASUREMENT_LUMINANCE                     44
#define VSCP_TYPE_MEASUREMENT_CHEMICAL_CONCENTRATION        45
#define VSCP_TYPE_MEASUREMENT_RESERVED	                    46
#define VSCP_TYPE_MEASUREMENT_DOSE_EQVIVALENT               47
#define VSCP_TYPE_MEASUREMENT_DEWPOINT                      49
#define VSCP_TYPE_MEASUREMENT_RELATIVE_LEVEL                50
#define VSCP_TYPE_MEASUREMENT_ALTITUDE                      51
#define VSCP_TYPE_MEASUREMENT_AREA                          52
#define VSCP_TYPE_MEASUREMENT_RADIANT_INTENSITY             53
#define VSCP_TYPE_MEASUREMENT_RADIANCE                      54
#define VSCP_TYPE_MEASUREMENT_IRRADIANCE                    55
#define VSCP_TYPE_MEASUREMENT_SPECTRAL_RADIANCE             56
#define VSCP_TYPE_MEASUREMENT_SPECTRAL_IRRADIANCE           57


// Class 15 (0x0f) -- DATA
#define VSCP_TYPE_DATA_GENERAL                              0
#define VSCP_TYPE_DATA_IO                                   1
#define VSCP_TYPE_DATA_AD                                   2
#define VSCP_TYPE_DATA_DA                                   3
#define VSCP_TYPE_DATA_RELATIVE_STRENGTH                    4
#define VSCP_TYPE_DATA_SIGNAL_LEVEL                         5
#define VSCP_TYPE_DATA_SIGNAL_QUALITY                       6

// class 20 (0x14) -- INFORMATION
#define VSCP_TYPE_INFORMATION_GENERAL                       0
#define VSCP_TYPE_INFORMATION_BUTTON                        1
#define VSCP_TYPE_INFORMATION_MOUSE                         2
#define VSCP_TYPE_INFORMATION_ON                            3
#define VSCP_TYPE_INFORMATION_OFF                           4
#define VSCP_TYPE_INFORMATION_ALIVE                         5
#define VSCP_TYPE_INFORMATION_TERMINATING                   6
#define VSCP_TYPE_INFORMATION_OPENED                        7
#define VSCP_TYPE_INFORMATION_CLOSED                        8
#define VSCP_TYPE_INFORMATION_NODE_HEARTBEAT                9
#define VSCP_TYPE_INFORMATION_BELOW_LIMIT                   10
#define VSCP_TYPE_INFORMATION_ABOVE_LIMIT                   11
#define VSCP_TYPE_INFORMATION_PULSE                         12
#define VSCP_TYPE_INFORMATION_ERROR                         13
#define VSCP_TYPE_INFORMATION_RESUMED                       14
#define VSCP_TYPE_INFORMATION_PAUSED                        15
#define VSCP_TYPE_INFORMATION_SLEEP                         16
#define VSCP_TYPE_INFORMATION_GOOD_MORNING                  17
#define VSCP_TYPE_INFORMATION_GOOD_DAY                      18
#define VSCP_TYPE_INFORMATION_GOOD_AFTERNOON                19
#define VSCP_TYPE_INFORMATION_GOOD_EVENING                  20
#define VSCP_TYPE_INFORMATION_GOOD_NIGHT                    21
#define VSCP_TYPE_INFORMATION_SEE_YOU_SOON                  22
#define VSCP_TYPE_INFORMATION_GOODBYE                       23
#define VSCP_TYPE_INFORMATION_STOP                          24
#define VSCP_TYPE_INFORMATION_START                         25
#define VSCP_TYPE_INFORMATION_RESET_COMPLETED               26
#define VSCP_TYPE_INFORMATION_INTERRUPTED                   27
#define VSCP_TYPE_INFORMATION_PREPARING_TO_SLEEP            28
#define VSCP_TYPE_INFORMATION_WOKEN_UP                      29
#define VSCP_TYPE_INFORMATION_DUSK                          30
#define VSCP_TYPE_INFORMATION_DAWN                          31
#define VSCP_TYPE_INFORMATION_ACTIVE                        32
#define VSCP_TYPE_INFORMATION_INACTIVE                      33
#define VSCP_TYPE_INFORMATION_BUSY                          34
#define VSCP_TYPE_INFORMATION_IDLE                          35
#define VSCP_TYPE_INFORMATION_STREAM_DATA                   36
#define VSCP_TYPE_INFORMATION_TOKEN_ACTIVITY                37
#define VSCP_TYPE_INFORMATION_STREAM_DATA_WITH_ZONE         38
#define VSCP_TYPE_INFORMATION_CONFIRM                       39
#define VSCP_TYPE_INFORMATION_LEVEL_CHANGED                 40
#define VSCP_TYPE_INFORMATION_WARNING                       41
#define VSCP_TYPE_INFORMATION_STATE                         42
#define VSCP_TYPE_INFORMATION_ACTION_TRIGGER                43
#define VSCP_TYPE_INFORMATION_SUNRISE                       44
#define VSCP_TYPE_INFORMATION_SUNSET                        45
#define VSCP_TYPE_INFORMATION_START_OF_RECORD               46
#define VSCP_TYPE_INFORMATION_END_OF_RECORD                 47
#define VSCP_TYPE_INFORMATION_PRESET_ACTIVE                 48
#define VSCP_TYPE_INFORMATION_DETECT                        49
#define VSCP_TYPE_INFORMATION_OVERFLOW                      50
#define VSCP_TYPE_INFORMATION_BIG_LEVEL_CHANGED             51
#define VSCP_TYPE_INFORMATION_SUNRISE_TWILIGHT_START        52
#define VSCP_TYPE_INFORMATION_SUNSET_TWILIGHT_START         53
#define VSCP_TYPE_INFORMATION_NAUTICAL_SUNRISE_TWILIGHT_START       54
#define VSCP_TYPE_INFORMATION_NAUTICAL_SUNSET_TWILIGHT_START        55
#define VSCP_TYPE_INFORMATION_ASTRONOMICAL_SUNRISE_TWILIGHT_START   56
#define VSCP_TYPE_INFORMATION_ASTRONOMICAL_SUNSET_TWILIGHT_START    57
#define VSCP_TYPE_INFORMATION_CALCULATED_NOON                       58
#define VSCP_TYPE_INFORMATION_SHUTTER_UP                    59
#define VSCP_TYPE_INFORMATION_SHUTTER_DOWN                  60
#define VSCP_TYPE_INFORMATION_SHUTTER_LEFT                  61
#define VSCP_TYPE_INFORMATION_SHUTTER_RIGHT                 62
#define VSCP_TYPE_INFORMATION_SHUTTER_END_TOP               63
#define VSCP_TYPE_INFORMATION_SHUTTER_END_BOTTOM            64
#define VSCP_TYPE_INFORMATION_SHUTTER_END_MIDDLE            65
#define VSCP_TYPE_INFORMATION_SHUTTER_END_PRESET            66
#define VSCP_TYPE_INFORMATION_SHUTTER_END_LEFT              67
#define VSCP_TYPE_INFORMATION_SHUTTER_END_RIGHT             68

// class 30 (0x1E) -- CONTROL
#define VSCP_TYPE_CONTROL_GENERAL                           0
#define VSCP_TYPE_CONTROL_MUTE                              1
#define VSCP_TYPE_CONTROL_ALL_LAMPS                         2
#define VSCP_TYPE_CONTROL_OPEN                              3
#define VSCP_TYPE_CONTROL_CLOSE                             4
#define VSCP_TYPE_CONTROL_TURNON                            5
#define VSCP_TYPE_CONTROL_TURNOFF                           6
#define VSCP_TYPE_CONTROL_START                             7
#define VSCP_TYPE_CONTROL_STOP                              8
#define VSCP_TYPE_CONTROL_RESET                             9
#define VSCP_TYPE_CONTROL_INTERRUPT                         10
#define VSCP_TYPE_CONTROL_SLEEP                             11
#define VSCP_TYPE_CONTROL_WAKEUP                            12
#define VSCP_TYPE_CONTROL_RESUME                            13
#define VSCP_TYPE_CONTROL_PAUSE                             14
#define VSCP_TYPE_CONTROL_ACTIVATE                          15
#define VSCP_TYPE_CONTROL_DEACTIVATE                        16
#define VSCP_TYPE_CONTROL_DIM_LAMPS                         20
#define VSCP_TYPE_CONTROL_CHANGE_CHANNEL                    21
#define VSCP_TYPE_CONTROL_CHANGE_LEVEL                      22
#define VSCP_TYPE_CONTROL_RELATIVE_CHANGE_LEVEL             23
#define VSCP_TYPE_CONTROL_MEASUREMENT_REQUEST               24
#define VSCP_TYPE_CONTROL_STREAM_DATA                       25
#define VSCP_TYPE_CONTROL_SYNC                              26
#define VSCP_TYPE_CONTROL_ZONED_STREAM_DATA                 27
#define VSCP_TYPE_CONTROL_SET_PRESET                        28
#define VSCP_TYPE_CONTROL_TOGGLE_STATE                      29
#define VSCP_TYPE_CONTROL_TIMED_PULE_ON                     30
#define VSCP_TYPE_CONTROL_TIMED_PULSE_OFF                   31
#define VSCP_TYPE_CONTROL_SET_COUNTRY_LANGUAGE	            32
#define VSCP_TYPE_CONTROL_BIG_CHANGE_LEVEL                  33
#define VSCP_TYPE_CONTROL_SHUTTER_UP                        34
#define VSCP_TYPE_CONTROL_SHUTTER_DOWN                      35
#define VSCP_TYPE_CONTROL_SHUTTER_LEFT                      36
#define VSCP_TYPE_CONTROL_SHUTTER_RIGHT                     37
#define VSCP_TYPE_CONTROL_SHUTTER_MIDDLE                    38
#define VSCP_TYPE_CONTROL_SHUTTER_PRESET                    39

// class 40 (0x28) -- MULTIMEDIA
#define VSCP_TYPE_MULTIMEDIA_GENERAL                        0
#define VSCP_TYPE_MULTIMEDIA_PLAYBACK                       1
#define VSCP_TYPE_MULTIMEDIA_NAVIGATOR_KEY_ENG              2
#define VSCP_TYPE_MULTIMEDIA_ADJUST_CONTRAST                3
#define VSCP_TYPE_MULTIMEDIA_ADJUST_FOCUS                   4
#define VSCP_TYPE_MULTIMEDIA_ADJUST_TINT                    5
#define VSCP_TYPE_MULTIMEDIA_ADJUST_COLOUR_BALANCE          6
#define VSCP_TYPE_MULTIMEDIA_ADJUST_BRIGHTNESS              7
#define VSCP_TYPE_MULTIMEDIA_ADJUST_HUE                     8
#define VSCP_TYPE_MULTIMEDIA_ADJUST_BASS                    9
#define VSCP_TYPE_MULTIMEDIA_ADJUST_TREBLE                  10
#define VSCP_TYPE_MULTIMEDIA_ADJUST_MASTER_VOLUME           11
#define VSCP_TYPE_MULTIMEDIA_ADJUST_FRONT_VOLUME            12
#define VSCP_TYPE_MULTIMEDIA_ADJUST_CENTRE_VOLUME           13
#define VSCP_TYPE_MULTIMEDIA_ADJUST_REAR_VOLUME             14
#define VSCP_TYPE_MULTIMEDIA_ADJUST_SIDE_VOLUME             15
#define VSCP_TYPE_MULTIMEDIA_ADJUST_SELECT_DISK             20
#define VSCP_TYPE_MULTIMEDIA_ADJUST_SELECT_TRACK            21
#define VSCP_TYPE_MULTIMEDIA_ADJUST_SELECT_ALBUM            22
#define VSCP_TYPE_MULTIMEDIA_ADJUST_SELECT_CHANNEL          23
#define VSCP_TYPE_MULTIMEDIA_ADJUST_SELECT_PAGE             24
#define VSCP_TYPE_MULTIMEDIA_ADJUST_SELECT_CHAPTER          25
#define VSCP_TYPE_MULTIMEDIA_ADJUST_SELECT_SCREEN_FORMAT    26
#define VSCP_TYPE_MULTIMEDIA_ADJUST_SELECT_INPUT_SOURCE     27
#define VSCP_TYPE_MULTIMEDIA_ADJUST_SELECT_OUTPUT           28
#define VSCP_TYPE_MULTIMEDIA_RECORD                         29
#define VSCP_TYPE_MULTIMEDIA_SET_RECORDING_VOLUME           30
#define VSCP_TYPE_MULTIMEDIA_TIVO_FUNCTION                  40
#define VSCP_TYPE_MULTIMEDIA_GET_CURRENT_TITLE              50
#define VSCP_TYPE_MULTIMEDIA_SET_POSITION                   51
#define VSCP_TYPE_MULTIMEDIA_GET_MEDIA_INFO                 52
#define VSCP_TYPE_MULTIMEDIA_REMOVE_ITEM                    53
#define VSCP_TYPE_MULTIMEDIA_REMOVE_ALL_ITEMS               54
#define VSCP_TYPE_MULTIMEDIA_SAVE_ALBUM                     55
#define VSCP_TYPE_MULTIMEDIA_CONTROL                        60
#define VSCP_TYPE_MULTIMEDIA_CONTROL_RESPONSE               61

// class 50 (0x32) -- AOL
#define VSCP_TYPE_AOL_GENERAL                               0
#define VSCP_TYPE_AOL_UNPLUGGED_POWER                       1 
#define VSCP_TYPE_AOL_UNPLUGGED_LAN                         2
#define VSCP_TYPE_AOL_CHASSIS_INTRUSION                     3
#define VSCP_TYPE_AOL_PROCESSOR_REMOVAL                     4
#define VSCP_TYPE_AOL_ENVIRONMENT_ERROR                     5
#define VSCP_TYPE_AOL_HIGH_TEMPERATURE                      6
#define VSCP_TYPE_AOL_FAN_SPEED                             7
#define VSCP_TYPE_AOL_VOLTAGE_FLUCTUATIONS                  8
#define VSCP_TYPE_AOL_OS_ERROR                              9
#define VSCP_TYPE_AOL_POWER_ON_ERROR                        10
#define VSCP_TYPE_AOL_SYSTEM_HUNG                           11
#define VSCP_TYPE_AOL_COMPONENT_FAILURE                     12
#define VSCP_TYPE_AOL_REBOOT_UPON_FAILURE                   13
#define VSCP_TYPE_AOL_REPAIR_OPERATING_SYSTEM               14
#define VSCP_TYPE_AOL_UPDATE_BIOS_IMAGE                     15
#define VSCP_TYPE_AOL_UPDATE_DIAGNOSTIC_PROCEDURE           16

// class 60 (0x3C) -- CLASS1.MEASUREMENT64
// Types are the same as CLASS1.MEASUREMENT=10
#define VSCP_TYPE_MEASUREMENT64_GENERAL                     0
#define VSCP_TYPE_MEASUREMENT64_COUNT                       1
#define VSCP_TYPE_MEASUREMENT64_LENGTH                      2
#define VSCP_TYPE_MEASUREMENT64_MASS                        3
#define VSCP_TYPE_MEASUREMENT64_TIME                        4
#define VSCP_TYPE_MEASUREMENT64_ELECTRIC_CURRENT            5
#define VSCP_TYPE_MEASUREMENT64_TEMPERATURE                 6
#define VSCP_TYPE_MEASUREMENT64_AMOUNT_OF_SUBSTANCE         7
#define VSCP_TYPE_MEASUREMENT64_INTENSITY_OF_LIGHT          8
#define VSCP_TYPE_MEASUREMENT64_FREQUENCY                   9
#define VSCP_TYPE_MEASUREMENT64_RADIOACTIVITY               10	// or other random event
#define VSCP_TYPE_MEASUREMENT64_FORCE                       11
#define VSCP_TYPE_MEASUREMENT64_PRESSURE                    12
#define VSCP_TYPE_MEASUREMENT64_ENERGY                      13
#define VSCP_TYPE_MEASUREMENT64_POWER                       14
#define VSCP_TYPE_MEASUREMENT64_ELECTRICAL_CHARGE           15
#define VSCP_TYPE_MEASUREMENT64_ELECTRICAL_POTENTIAL        16	// Voltage
#define VSCP_TYPE_MEASUREMENT64_ELECTRICAL_CAPACITANCE      17
#define VSCP_TYPE_MEASUREMENT64_ELECTRICAL_RECISTANCE       18
#define VSCP_TYPE_MEASUREMENT64_ELECTRICAL_CONDUCTANCE      19
#define VSCP_TYPE_MEASUREMENT64_MAGNETIC_FIELD_STRENGTH     20
#define VSCP_TYPE_MEASUREMENT64_MAGNETIC_FLUX               21
#define VSCP_TYPE_MEASUREMENT64_MAGNETIC_FLUX_DENSITY       22
#define VSCP_TYPE_MEASUREMENT64_INDUCTANCE                  23
#define VSCP_TYPE_MEASUREMENT64_FLUX_OF_LIGHT               24
#define VSCP_TYPE_MEASUREMENT64_ILLUMINANCE                 25
#define VSCP_TYPE_MEASUREMENT64_RADIATION_DOSE              26
#define VSCP_TYPE_MEASUREMENT64_CATALYTIC_ACITIVITY         27
#define VSCP_TYPE_MEASUREMENT64_VOLUME                      28
#define VSCP_TYPE_MEASUREMENT64_SOUND_INTENSITY             29
#define VSCP_TYPE_MEASUREMENT64_ANGLE                       30
#define VSCP_TYPE_MEASUREMENT64_POSITION                    31
#define VSCP_TYPE_MEASUREMENT64_SPEED                       32
#define VSCP_TYPE_MEASUREMENT64_ACCELERATION                33
#define VSCP_TYPE_MEASUREMENT64_TENSION                     34
#define VSCP_TYPE_MEASUREMENT64_HUMIDITY                    35	// Damp/moist (Hygrometer reading)
#define VSCP_TYPE_MEASUREMENT64_FLOW                        36
#define VSCP_TYPE_MEASUREMENT64_THERMAL_RESISTANCE          37
#define VSCP_TYPE_MEASUREMENT64_REFRACTIVE_POWER            38
#define VSCP_TYPE_MEASUREMENT64_DYNAMIC_VISCOSITY           39
#define VSCP_TYPE_MEASUREMENT64_SOUND_IMPEDANCE             40
#define VSCP_TYPE_MEASUREMENT64_SOUND_RESISTANCE            41
#define VSCP_TYPE_MEASUREMENT64_ELECTRIC_ELASTANCE          42
#define VSCP_TYPE_MEASUREMENT64_LUMINOUS_ENERGY             43
#define VSCP_TYPE_MEASUREMENT64_LUMINANCE                   44
#define VSCP_TYPE_MEASUREMENT64_CHEMICAL_CONCENTRATION      45
#define VSCP_TYPE_MEASUREMENT64_RESERVED	            	46
#define VSCP_TYPE_MEASUREMENT64_DOSE_EQVIVALENT             47
#define VSCP_TYPE_MEASUREMENT64_DEWPOINT                    49
#define VSCP_TYPE_MEASUREMENT64_RELATIVE_LEVEL              50
#define VSCP_TYPE_MEASUREMENT64_ALTITUDE                    51
#define VSCP_TYPE_MEASUREMENT64_AREA                        52
#define VSCP_TYPE_MEASUREMENT64_RADIANT_INTENSITY           53
#define VSCP_TYPE_MEASUREMENT64_RADIANCE                    54
#define VSCP_TYPE_MEASUREMENT64_IRRADIANCE                  55
#define VSCP_TYPE_MEASUREMENT64_SPECTRAL_RADIANCE           56
#define VSCP_TYPE_MEASUREMENT64_SPECTRAL_IRRADIANCE         57

// class 65 (0x41) -- CLASS1.MEASUREZONE 
// Types are the same as CLASS1.MEASUREMENT=10
#define VSCP_TYPE_MEASUREZONE_GENERAL                     	0
#define VSCP_TYPE_MEASUREZONE_COUNT                       	1
#define VSCP_TYPE_MEASUREZONE_LENGTH                      	2
#define VSCP_TYPE_MEASUREZONE_MASS                        	3
#define VSCP_TYPE_MEASUREZONE_TIME                        	4
#define VSCP_TYPE_MEASUREZONE_ELECTRIC_CURRENT            	5
#define VSCP_TYPE_MEASUREZONE_TEMPERATURE                 	6
#define VSCP_TYPE_MEASUREZONE_AMOUNT_OF_SUBSTANCE         	7
#define VSCP_TYPE_MEASUREZONE_INTENSITY_OF_LIGHT          	8
#define VSCP_TYPE_MEASUREZONE_FREQUENCY                   	9
#define VSCP_TYPE_MEASUREZONE_RADIOACTIVITY               	10	// or other random event
#define VSCP_TYPE_MEASUREZONE_FORCE                       	11
#define VSCP_TYPE_MEASUREZONE_PRESSURE                    	12
#define VSCP_TYPE_MEASUREZONE_ENERGY                      	13
#define VSCP_TYPE_MEASUREZONE_POWER                       	14
#define VSCP_TYPE_MEASUREZONE_ELECTRICAL_CHARGE           	15
#define VSCP_TYPE_MEASUREZONE_ELECTRICAL_POTENTIAL        	16	// Voltage
#define VSCP_TYPE_MEASUREZONE_ELECTRICAL_CAPACITANCE      	17
#define VSCP_TYPE_MEASUREZONE_ELECTRICAL_RECISTANCE       	18
#define VSCP_TYPE_MEASUREZONE_ELECTRICAL_CONDUCTANCE      	19
#define VSCP_TYPE_MEASUREZONE_MAGNETIC_FIELD_STRENGTH     	20
#define VSCP_TYPE_MEASUREZONE_MAGNETIC_FLUX               	21
#define VSCP_TYPE_MEASUREZONE_MAGNETIC_FLUX_DENSITY       	22
#define VSCP_TYPE_MEASUREZONE_INDUCTANCE                  	23
#define VSCP_TYPE_MEASUREZONE_FLUX_OF_LIGHT               	24
#define VSCP_TYPE_MEASUREZONE_ILLUMINANCE                 	25
#define VSCP_TYPE_MEASUREZONE_RADIATION_DOSE              	26
#define VSCP_TYPE_MEASUREZONE_CATALYTIC_ACITIVITY         	27
#define VSCP_TYPE_MEASUREZONE_VOLUME                      	28
#define VSCP_TYPE_MEASUREZONE_SOUND_INTENSITY             	29
#define VSCP_TYPE_MEASUREZONE_ANGLE                       	30
#define VSCP_TYPE_MEASUREZONE_POSITION                    	31
#define VSCP_TYPE_MEASUREZONE_SPEED                       	32
#define VSCP_TYPE_MEASUREZONE_ACCELERATION                	33
#define VSCP_TYPE_MEASUREZONE_TENSION                     	34
#define VSCP_TYPE_MEASUREZONE_HUMIDITY                    	35	// Damp/moist (Hygrometer reading)
#define VSCP_TYPE_MEASUREZONE_FLOW                        	36
#define VSCP_TYPE_MEASUREZONE_THERMAL_RESISTANCE          	37
#define VSCP_TYPE_MEASUREZONE_REFRACTIVE_POWER            	38
#define VSCP_TYPE_MEASUREZONE_DYNAMIC_VISCOSITY           	39
#define VSCP_TYPE_MEASUREZONE_SOUND_IMPEDANCE             	40
#define VSCP_TYPE_MEASUREZONE_SOUND_RESISTANCE            	41
#define VSCP_TYPE_MEASUREZONE_ELECTRIC_ELASTANCE          	42
#define VSCP_TYPE_MEASUREZONE_LUMINOUS_ENERGY             	43
#define VSCP_TYPE_MEASUREZONE_LUMINANCE                   	44
#define VSCP_TYPE_MEASUREZONE_CHEMICAL_CONCENTRATION      	45
#define VSCP_TYPE_MEASUREZONE_RESERVED	               	  	46
#define VSCP_TYPE_MEASUREZONE_DOSE_EQVIVALENT             	47
#define VSCP_TYPE_MEASUREZONE_DEWPOINT                    	49
#define VSCP_TYPE_MEASUREZONE_RELATIVE_LEVEL              	50
#define VSCP_TYPE_MEASUREZONE_ALTITUDE                    	51
#define VSCP_TYPE_MEASUREZONE_AREA                        	52
#define VSCP_TYPE_MEASUREZONE_RADIANT_INTENSITY           	53
#define VSCP_TYPE_MEASUREZONE_RADIANCE                    	54
#define VSCP_TYPE_MEASUREZONE_IRRADIANCE                  	55
#define VSCP_TYPE_MEASUREZONE_SPECTRAL_RADIANCE           	56
#define VSCP_TYPE_MEASUREZONE_SPECTRAL_IRRADIANCE         	57

// class 70 (0x46) -- CLASS1.MEASUREMENT32
// Types are the same as CLASS1.MEASUREMENT=10
#define VSCP_TYPE_MEASUREMENT32_GENERAL                     0
#define VSCP_TYPE_MEASUREMENT32_COUNT                       1
#define VSCP_TYPE_MEASUREMENT32_LENGTH                      2
#define VSCP_TYPE_MEASUREMENT32_MASS                        3
#define VSCP_TYPE_MEASUREMENT32_TIME                        4
#define VSCP_TYPE_MEASUREMENT32_ELECTRIC_CURRENT            5
#define VSCP_TYPE_MEASUREMENT32_TEMPERATURE                 6
#define VSCP_TYPE_MEASUREMENT32_AMOUNT_OF_SUBSTANCE         7
#define VSCP_TYPE_MEASUREMENT32_INTENSITY_OF_LIGHT          8
#define VSCP_TYPE_MEASUREMENT32_FREQUENCY                   9
#define VSCP_TYPE_MEASUREMENT32_RADIOACTIVITY               10	// or other random event
#define VSCP_TYPE_MEASUREMENT32_FORCE                       11
#define VSCP_TYPE_MEASUREMENT32_PRESSURE                    12
#define VSCP_TYPE_MEASUREMENT32_ENERGY                      13
#define VSCP_TYPE_MEASUREMENT32_POWER                       14
#define VSCP_TYPE_MEASUREMENT32_ELECTRICAL_CHARGE           15
#define VSCP_TYPE_MEASUREMENT32_ELECTRICAL_POTENTIAL        16	// Voltage
#define VSCP_TYPE_MEASUREMENT32_ELECTRICAL_CAPACITANCE      17
#define VSCP_TYPE_MEASUREMENT32_ELECTRICAL_RECISTANCE       18
#define VSCP_TYPE_MEASUREMENT32_ELECTRICAL_CONDUCTANCE      19
#define VSCP_TYPE_MEASUREMENT32_MAGNETIC_FIELD_STRENGTH     20
#define VSCP_TYPE_MEASUREMENT32_MAGNETIC_FLUX               21
#define VSCP_TYPE_MEASUREMENT32_MAGNETIC_FLUX_DENSITY       22
#define VSCP_TYPE_MEASUREMENT32_INDUCTANCE                  23
#define VSCP_TYPE_MEASUREMENT32_FLUX_OF_LIGHT               24
#define VSCP_TYPE_MEASUREMENT32_ILLUMINANCE                 25
#define VSCP_TYPE_MEASUREMENT32_RADIATION_DOSE              26
#define VSCP_TYPE_MEASUREMENT32_CATALYTIC_ACITIVITY         27
#define VSCP_TYPE_MEASUREMENT32_VOLUME                      28
#define VSCP_TYPE_MEASUREMENT32_SOUND_INTENSITY             29
#define VSCP_TYPE_MEASUREMENT32_ANGLE                       30
#define VSCP_TYPE_MEASUREMENT32_POSITION                    31
#define VSCP_TYPE_MEASUREMENT32_SPEED                       32
#define VSCP_TYPE_MEASUREMENT32_ACCELERATION                33
#define VSCP_TYPE_MEASUREMENT32_TENSION                     34
#define VSCP_TYPE_MEASUREMENT32_HUMIDITY                    35	// Damp/moist (Hygrometer reading)
#define VSCP_TYPE_MEASUREMENT32_FLOW                        36
#define VSCP_TYPE_MEASUREMENT32_THERMAL_RESISTANCE          37
#define VSCP_TYPE_MEASUREMENT32_REFRACTIVE_POWER            38
#define VSCP_TYPE_MEASUREMENT32_DYNAMIC_VISCOSITY           39
#define VSCP_TYPE_MEASUREMENT32_SOUND_IMPEDANCE             40
#define VSCP_TYPE_MEASUREMENT32_SOUND_RESISTANCE            41
#define VSCP_TYPE_MEASUREMENT32_ELECTRIC_ELASTANCE          42
#define VSCP_TYPE_MEASUREMENT32_LUMINOUS_ENERGY             43
#define VSCP_TYPE_MEASUREMENT32_LUMINANCE                   44
#define VSCP_TYPE_MEASUREMENT32_CHEMICAL_CONCENTRATION      45
#define VSCP_TYPE_MEASUREMENT32_RESERVED	            46
#define VSCP_TYPE_MEASUREMENT32_DOSE_EQVIVALENT             47
#define VSCP_TYPE_MEASUREMENT32_DEWPOINT                    49
#define VSCP_TYPE_MEASUREMENT32_RELATIVE_LEVEL              50
#define VSCP_TYPE_MEASUREMENT32_ALTITUDE                    51
#define VSCP_TYPE_MEASUREMENT32_AREA                        52
#define VSCP_TYPE_MEASUREMENT32_RADIANT_INTENSITY           53
#define VSCP_TYPE_MEASUREMENT32_RADIANCE                    54
#define VSCP_TYPE_MEASUREMENT32_IRRADIANCE                  55
#define VSCP_TYPE_MEASUREMENT32_SPECTRAL_RADIANCE           56
#define VSCP_TYPE_MEASUREMENT32_SPECTRAL_IRRADIANCE         57

// class 85 (0x55) -- CLASS1.SETVALUEZONE 
// Types are the same as CLASS1.MEASUREMENT=10
#define VSCP_TYPE_SETVALUEZONE_GENERAL                     	0
#define VSCP_TYPE_SETVALUEZONE_COUNT                       	1
#define VSCP_TYPE_SETVALUEZONE_LENGTH                      	2
#define VSCP_TYPE_SETVALUEZONE_MASS                        	3
#define VSCP_TYPE_SETVALUEZONE_TIME                        	4
#define VSCP_TYPE_SETVALUEZONE_ELECTRIC_CURRENT            	5
#define VSCP_TYPE_SETVALUEZONE_TEMPERATURE                 	6
#define VSCP_TYPE_SETVALUEZONE_AMOUNT_OF_SUBSTANCE         	7
#define VSCP_TYPE_SETVALUEZONE_INTENSITY_OF_LIGHT          	8
#define VSCP_TYPE_SETVALUEZONE_FREQUENCY                   	9
#define VSCP_TYPE_SETVALUEZONE_RADIOACTIVITY               	10	// or other random event
#define VSCP_TYPE_SETVALUEZONE_FORCE                       	11
#define VSCP_TYPE_SETVALUEZONE_PRESSURE                    	12
#define VSCP_TYPE_SETVALUEZONE_ENERGY                      	13
#define VSCP_TYPE_SETVALUEZONE_POWER                       	14
#define VSCP_TYPE_SETVALUEZONE_ELECTRICAL_CHARGE           	15
#define VSCP_TYPE_SETVALUEZONE_ELECTRICAL_POTENTIAL        	16	// Voltage
#define VSCP_TYPE_SETVALUEZONE_ELECTRICAL_CAPACITANCE      	17
#define VSCP_TYPE_SETVALUEZONE_ELECTRICAL_RECISTANCE       	18
#define VSCP_TYPE_SETVALUEZONE_ELECTRICAL_CONDUCTANCE      	19
#define VSCP_TYPE_SETVALUEZONE_MAGNETIC_FIELD_STRENGTH     	20
#define VSCP_TYPE_SETVALUEZONE_MAGNETIC_FLUX               	21
#define VSCP_TYPE_SETVALUEZONE_MAGNETIC_FLUX_DENSITY       	22
#define VSCP_TYPE_SETVALUEZONE_INDUCTANCE                  	23
#define VSCP_TYPE_SETVALUEZONE_FLUX_OF_LIGHT               	24
#define VSCP_TYPE_SETVALUEZONE_ILLUMINANCE                 	25
#define VSCP_TYPE_SETVALUEZONE_RADIATION_DOSE              	26
#define VSCP_TYPE_SETVALUEZONE_CATALYTIC_ACITIVITY         	27
#define VSCP_TYPE_SETVALUEZONE_VOLUME                      	28
#define VSCP_TYPE_SETVALUEZONE_SOUND_INTENSITY             	29
#define VSCP_TYPE_SETVALUEZONE_ANGLE                       	30
#define VSCP_TYPE_SETVALUEZONE_POSITION                    	31
#define VSCP_TYPE_SETVALUEZONE_SPEED                       	32
#define VSCP_TYPE_SETVALUEZONE_ACCELERATION                	33
#define VSCP_TYPE_SETVALUEZONE_TENSION                     	34
#define VSCP_TYPE_SETVALUEZONE_HUMIDITY                    	35	// Damp/moist (Hygrometer reading)
#define VSCP_TYPE_SETVALUEZONE_FLOW                        	36
#define VSCP_TYPE_SETVALUEZONE_THERMAL_RESISTANCE          	37
#define VSCP_TYPE_SETVALUEZONE_REFRACTIVE_POWER            	38
#define VSCP_TYPE_SETVALUEZONE_DYNAMIC_VISCOSITY           	39
#define VSCP_TYPE_SETVALUEZONE_SOUND_IMPEDANCE             	40
#define VSCP_TYPE_SETVALUEZONE_SOUND_RESISTANCE            	41
#define VSCP_TYPE_SETVALUEZONE_ELECTRIC_ELASTANCE          	42
#define VSCP_TYPE_SETVALUEZONE_LUMINOUS_ENERGY             	43
#define VSCP_TYPE_SETVALUEZONE_LUMINANCE                   	44
#define VSCP_TYPE_SETVALUEZONE_CHEMICAL_CONCENTRATION      	45
#define VSCP_TYPE_SETVALUEZONE_RESERVED	               	  	46
#define VSCP_TYPE_SETVALUEZONE_DOSE_EQVIVALENT             	47
#define VSCP_TYPE_SETVALUEZONE_DEWPOINT                    	49
#define VSCP_TYPE_SETVALUEZONE_RELATIVE_LEVEL              	50
#define VSCP_TYPE_SETVALUEZONE_ALTITUDE                    	51
#define VSCP_TYPE_SETVALUEZONE_AREA                        	52
#define VSCP_TYPE_SETVALUEZONE_RADIANT_INTENSITY           	53
#define VSCP_TYPE_SETVALUEZONE_RADIANCE                    	54
#define VSCP_TYPE_SETVALUEZONE_IRRADIANCE                  	55
#define VSCP_TYPE_SETVALUEZONE_SPECTRAL_RADIANCE           	56
#define VSCP_TYPE_SETVALUEZONE_SPECTRAL_IRRADIANCE         	57


// class 90 (0x5A) -- WATHER
#define VSCP_TYPE_WEATHER_GENERAL                           0
#define VSCP_TYPE_WEATHER_SEASONS_WINTER                    1
#define VSCP_TYPE_WEATHER_SEASONS_SPRING                    2
#define VSCP_TYPE_WEATHER_SEASONS_SUMMER                    3
#define VSCP_TYPE_WEATHER_SEASONS_AUTUMN                    4
#define VSCP_TYPE_WEATHER_WIND_NONE                         5
#define VSCP_TYPE_WEATHER_WIND_LOW                          6
#define VSCP_TYPE_WEATHER_WIND_MEDIUM                       7
#define VSCP_TYPE_WEATHER_WIND_HIGH                         8
#define VSCP_TYPE_WEATHER_WIND_VERY_HIGH                    9
#define VSCP_TYPE_WEATHER_AIR_FOGGY                         10
#define VSCP_TYPE_WEATHER_AIR_FREEZING                      11
#define VSCP_TYPE_WEATHER_AIR_COLD                          12
#define VSCP_TYPE_WEATHER_AIR_VERY_COLD                     13
#define VSCP_TYPE_WEATHER_AIR_NORMAL                        14
#define VSCP_TYPE_WEATHER_AIR_HOT                           15
#define VSCP_TYPE_WEATHER_AIR_VERY_HOT                      16    
#define VSCP_TYPE_WEATHER_AIR_POLLUTION_LOW                 17
#define VSCP_TYPE_WEATHER_AIR_POLLUTION_MEDIUM              18
#define VSCP_TYPE_WEATHER_AIR_POLLUTION_HIGH                19
#define VSCP_TYPE_WEATHER_AIR_HUMID                         20
#define VSCP_TYPE_WEATHER_AIR_DRY                           21
#define VSCP_TYPE_WEATHER_SOIL_HUMID                        22
#define VSCP_TYPE_WEATHER_SOIL_DRY                          23
#define VSCP_TYPE_WEATHER_RAIN_NONE                         24
#define VSCP_TYPE_WEATHER_RAIN_LIGHT                        25
#define VSCP_TYPE_WEATHER_RAIN_HEAVY                        26
#define VSCP_TYPE_WEATHER_RAIN_VERY_HEAVY                   27
#define VSCP_TYPE_WEATHER_SUN_NONE                          28
#define VSCP_TYPE_WEATHER_SUN_LIGHT                         29
#define VSCP_TYPE_WEATHER_SUN_HEAVY                         30
#define VSCP_TYPE_WEATHER_SNOW_NONE                         31
#define VSCP_TYPE_WEATHER_SNOW_LIGHT                        32
#define VSCP_TYPE_WEATHER_SNOW_HEAVY                        33
#define VSCP_TYPE_WEATHER_DEW_POINT                         34
#define VSCP_TYPE_WEATHER_STORM                             35
#define VSCP_TYPE_WEATHER_FLOOD                             36
#define VSCP_TYPE_WEATHER_EARTHQUAKE                        37
#define VSCP_TYPE_WEATHER_NUCLEAR_DISASTER                  38
#define VSCP_TYPE_WEATHER_FIRE                              39
#define VSCP_TYPE_WEATHER_LIGHTNING                         40
#define VSCP_TYPE_WEATHER_UV_RADIATION_LOW                  41
#define VSCP_TYPE_WEATHER_UV_RADIATION_MEDIUM               42
#define VSCP_TYPE_WEATHER_UV_RADIATION_NORMAL               43
#define VSCP_TYPE_WEATHER_UV_RADIATION_HIGH                 44
#define VSCP_TYPE_WEATHER_UV_RADIATION_VERY_HIGH            45
#define VSCP_TYPE_WEATHER_WARNING_LEVEL1                    46
#define VSCP_TYPE_WEATHER_WARNING_LEVEL2                    47
#define VSCP_TYPE_WEATHER_WARNING_LEVEL3                    48
#define VSCP_TYPE_WEATHER_WARNING_LEVEL4                    49
#define VSCP_TYPE_WEATHER_WARNING_LEVEL5                    50
#define VSCP_TYPE_WEATHER_ARMAGEDON                         51
    
// class 95 (0x5F) -- WATHER FORECAST
#define VSCP_TYPE_WEATHER_FORECAST_GENERAL                  0
#define VSCP_TYPE_WEATHER_FORECAST_SEASONS_WINTER           1
#define VSCP_TYPE_WEATHER_FORECAST_SEASONS_SPRING           2
#define VSCP_TYPE_WEATHER_FORECAST_SEASONS_SUMMER           3
#define VSCP_TYPE_WEATHER_FORECAST_SEASONS_AUTUMN           4
#define VSCP_TYPE_WEATHER_FORECAST_WIND_NONE                5
#define VSCP_TYPE_WEATHER_FORECAST_WIND_LOW                 6
#define VSCP_TYPE_WEATHER_FORECAST_WIND_MEDIUM              7
#define VSCP_TYPE_WEATHER_FORECAST_WIND_HIGH                8
#define VSCP_TYPE_WEATHER_FORECAST_WIND_VERY_HIGH           9
#define VSCP_TYPE_WEATHER_FORECAST_AIR_FOGGY                10
#define VSCP_TYPE_WEATHER_FORECAST_AIR_FREEZING             11
#define VSCP_TYPE_WEATHER_FORECAST_AIR_COLD                 12
#define VSCP_TYPE_WEATHER_FORECAST_AIR_VERY_COLD            13
#define VSCP_TYPE_WEATHER_FORECAST_AIR_NORMAL               14
#define VSCP_TYPE_WEATHER_FORECAST_AIR_HOT                  15
#define VSCP_TYPE_WEATHER_FORECAST_AIR_VERY_HOT             16    
#define VSCP_TYPE_WEATHER_FORECAST_AIR_POLLUTION_LOW        17
#define VSCP_TYPE_WEATHER_FORECAST_AIR_POLLUTION_MEDIUM     18
#define VSCP_TYPE_WEATHER_FORECAST_AIR_POLLUTION_HIGH       19
#define VSCP_TYPE_WEATHER_FORECAST_AIR_HUMID                20
#define VSCP_TYPE_WEATHER_FORECAST_AIR_DRY                  21
#define VSCP_TYPE_WEATHER_FORECAST_SOIL_HUMID               22
#define VSCP_TYPE_WEATHER_FORECAST_SOIL_DRY                 23
#define VSCP_TYPE_WEATHER_FORECAST_RAIN_NONE                24
#define VSCP_TYPE_WEATHER_FORECAST_RAIN_LIGHT               25
#define VSCP_TYPE_WEATHER_FORECAST_RAIN_HEAVY               26
#define VSCP_TYPE_WEATHER_FORECAST_RAIN_VERY_HEAVY          27
#define VSCP_TYPE_WEATHER_FORECAST_SUN_NONE                 28
#define VSCP_TYPE_WEATHER_FORECAST_SUN_LIGHT                29
#define VSCP_TYPE_WEATHER_FORECAST_SUN_HEAVY                30
#define VSCP_TYPE_WEATHER_FORECAST_SNOW_NONE                31
#define VSCP_TYPE_WEATHER_FORECAST_SNOW_LIGHT               32
#define VSCP_TYPE_WEATHER_FORECAST_SNOW_HEAVY               33
#define VSCP_TYPE_WEATHER_FORECAST_DEW_POINT                34
#define VSCP_TYPE_WEATHER_FORECAST_STORM                    35
#define VSCP_TYPE_WEATHER_FORECAST_FLOOD                    36
#define VSCP_TYPE_WEATHER_FORECAST_EARTHQUAKE               37
#define VSCP_TYPE_WEATHER_FORECAST_NUCLEAR_DISASTER         38
#define VSCP_TYPE_WEATHER_FORECAST_FIRE                     39
#define VSCP_TYPE_WEATHER_FORECAST_LIGHTNING                40
#define VSCP_TYPE_WEATHER_FORECAST_UV_RADIATION_LOW         41
#define VSCP_TYPE_WEATHER_FORECAST_UV_RADIATION_MEDIUM      42
#define VSCP_TYPE_WEATHER_FORECAST_UV_RADIATION_NORMAL      43
#define VSCP_TYPE_WEATHER_FORECAST_UV_RADIATION_HIGH        44
#define VSCP_TYPE_WEATHER_FORECAST_UV_RADIATION_VERY_HIGH   45
#define VSCP_TYPE_WEATHER_FORECAST_WARNING_LEVEL1           46
#define VSCP_TYPE_WEATHER_FORECAST_WARNING_LEVEL2           47
#define VSCP_TYPE_WEATHER_FORECAST_WARNING_LEVEL3           48
#define VSCP_TYPE_WEATHER_FORECAST_WARNING_LEVEL4           49
#define VSCP_TYPE_WEATHER_FORECAST_WARNING_LEVEL5           50
#define VSCP_TYPE_WEATHER_FORECAST_ARMAGEDDON               51

	

// class 100 (0x64) -- PHONE
#define VSCP_TYPE_PHONE_GENERAL                             0
#define VSCP_TYPE_PHONE_INCOMING_CALL                       1
#define VSCP_TYPE_PHONE_OUTGOING_CALL                       2
#define VSCP_TYPE_PHONE_RING                                3
#define VSCP_TYPE_PHONE_ANSWERE                             4
#define VSCP_TYPE_PHONE_HANGUP                              5
#define VSCP_TYPE_PHONE_GIVEUP                              6
#define VSCP_TYPE_PHONE_TRANSFER                            7
#define VSCP_TYPE_PHONE_DATABASE_INFO                       8

// class 102 (0x66) -- DISPLAY
#define VSCP_TYPE_DISPLAY_GENERAL                           0
#define VSCP_TYPE_DISPLAY_CLEAR_DISPLAY                     1
#define VSCP_TYPE_DISPLAY_POSITION_CURSOR                   2
#define VSCP_TYPE_DISPLAY_WRITE_DISPLAY                     3
#define VSCP_TYPE_DISPLAY_WRITE_DISPLAY_BUFFER              4
#define VSCP_TYPE_DISPLAY_SHOW_DISPLAY_BUFFER               5
#define VSCP_TYPE_DISPLAY_SET_DISPLAY_BUFFER_PARAM          6
#define VSCP_TYPE_DISPLAY_SHOW_TEXT                         32
#define VSCP_TYPE_DISPLAY_SHOW_LED                          48
#define VSCP_TYPE_DISPLAY_SHOW_LED_COLOR                    49

// class 110 (0x6E) -- REMOTE
#define VSCP_TYPE_REMOTE_GENERAL                            0
#define VSCP_TYPE_REMOTE_RC5                                1
#define VSCP_TYPE_REMOTE_SONY12                             2
#define VSCP_TYPE_REMOTE_LIRC                               32
#define VSCP_TYPE_REMOTE_VSCP                               48

// class 206 (0xCE) -- GPS
#define VSCP_TYPE_GPS_GENERAL                               0
#define VSCP_TYPE_GPS_POSITION                              1
#define VSCP_TYPE_GPS_SATELLITES                            2

// class 212 (0xD4) -- WIRELESS
#define VSCP_TYPE_WIRELESS_GENERAL                          0
#define VSCP_TYPE_WIRELESS_GSM_CELL                         1

// class 506 (0x1FA) -- DIAGNOSTIC
#define VSCP_TYPE_DIAGNOSTIC_GENERAL                        0
#define VSCP_TYPE_DIAGNOSTIC_OVERVOLTAGE                    1
#define VSCP_TYPE_DIAGNOSTIC_UNDERVOLTAGE                   2
#define VSCP_TYPE_DIAGNOSTIC_VBUS_LOW                       3
#define VSCP_TYPE_DIAGNOSTIC_BATTERY_LOW                    4
#define VSCP_TYPE_DIAGNOSTIC_BATTERY_FULL                   5
#define VSCP_TYPE_DIAGNOSTIC_BATTERY_ERROR                  6
#define VSCP_TYPE_DIAGNOSTIC_BATTERY_OK                     7
#define VSCP_TYPE_DIAGNOSTIC_OVERCURRENT                    8
#define VSCP_TYPE_DIAGNOSTIC_CIRCUIT_ERROR                  9
#define VSCP_TYPE_DIAGNOSTIC_SHORT_CIRCUIT                  10
#define VSCP_TYPE_DIAGNOSTIC_OPEN_CIRCUIT                   11
#define VSCP_TYPE_DIAGNOSTIC_MOIST                          12
#define VSCP_TYPE_DIAGNOSTIC_WIRE_FAIL                      13
#define VSCP_TYPE_DIAGNOSTIC_WIRELESS_FAIL                  14
#define VSCP_TYPE_DIAGNOSTIC_IR_FAIL                        15
#define VSCP_TYPE_DIAGNOSTIC_1WIRE_FAIL                     16
#define VSCP_TYPE_DIAGNOSTIC_RS222_FAIL                     17
#define VSCP_TYPE_DIAGNOSTIC_RS232_FAIL                     18
#define VSCP_TYPE_DIAGNOSTIC_RS423_FAIL                     19
#define VSCP_TYPE_DIAGNOSTIC_RS485_FAIL                     20
#define VSCP_TYPE_DIAGNOSTIC_CAN_FAIL                       21
#define VSCP_TYPE_DIAGNOSTIC_LAN_FAIL                       22
#define VSCP_TYPE_DIAGNOSTIC_USB_FAIL                       23
#define VSCP_TYPE_DIAGNOSTIC_WIFI_FAIL                      24
#define VSCP_TYPE_DIAGNOSTIC_NFC_RFID_FAIL                  25
#define VSCP_TYPE_DIAGNOSTIC_LOW_SIGNAL                     26
#define VSCP_TYPE_DIAGNOSTIC_HIGH_SIGNAL                    27
#define VSCP_TYPE_DIAGNOSTIC_ADC_FAIL                       28
#define VSCP_TYPE_DIAGNOSTIC_ALU_FAIL                       29
#define VSCP_TYPE_DIAGNOSTIC_ASSERT                         30
#define VSCP_TYPE_DIAGNOSTIC_DAC_FAIL                       31
#define VSCP_TYPE_DIAGNOSTIC_DMA_FAIL                       32
#define VSCP_TYPE_DIAGNOSTIC_ETH_FAIL                       33
#define VSCP_TYPE_DIAGNOSTIC_EXCEPTION                      34
#define VSCP_TYPE_DIAGNOSTIC_FPU_FAIL                       35
#define VSCP_TYPE_DIAGNOSTIC_GPIO_FAIL                      36
#define VSCP_TYPE_DIAGNOSTIC_I2C_FAIL                       37
#define VSCP_TYPE_DIAGNOSTIC_I2S_FAIL                       38
#define VSCP_TYPE_DIAGNOSTIC_INVALID_CONFIG                 39
#define VSCP_TYPE_DIAGNOSTIC_MMU_FAIL                       40
#define VSCP_TYPE_DIAGNOSTIC_NMI                            41
#define VSCP_TYPE_DIAGNOSTIC_OVERHEAT                       42
#define VSCP_TYPE_DIAGNOSTIC_PLL_FAIL                       43
#define VSCP_TYPE_DIAGNOSTIC_POR_FAIL                       44
#define VSCP_TYPE_DIAGNOSTIC_PWM_FAIL                       45
#define VSCP_TYPE_DIAGNOSTIC_RAM_FAIL                       46
#define VSCP_TYPE_DIAGNOSTIC_ROM_FAIL                       47
#define VSCP_TYPE_DIAGNOSTIC_SPI_FAIL                       48
#define VSCP_TYPE_DIAGNOSTIC_STACK_FAIL                     49
#define VSCP_TYPE_DIAGNOSTIC_LIN_FAIL                       50
#define VSCP_TYPE_DIAGNOSTIC_UART_FAIL                      51
#define VSCP_TYPE_DIAGNOSTIC_UNHANDLED_INT                  52
#define VSCP_TYPE_DIAGNOSTIC_USB_FAIL                       53
#define VSCP_TYPE_DIAGNOSTIC_VARIABLE_RANGE                 54
#define VSCP_TYPE_DIAGNOSTIC_WDT                            55
#define VSCP_TYPE_DIAGNOSTIC_EEPROM_FAIL                    56
#define VSCP_TYPE_DIAGNOSTIC_ENCRYPTION_FAIL                57
#define VSCP_TYPE_DIAGNOSTIC_BAD_USER_INPUT                 58
#define VSCP_TYPE_DIAGNOSTIC_DECRYPTION_FAIL                59
#define VSCP_TYPE_DIAGNOSTIC_NOISE                          60
#define VSCP_TYPE_DIAGNOSTIC_BOOTLOADER_FAIL                61
#define VSCP_TYPE_DIAGNOSTIC_PROGRAMFLOW_FAIL               62
#define VSCP_TYPE_DIAGNOSTIC_RTC_FAIL                       63
#define VSCP_TYPE_DIAGNOSTIC_SELFTEST_FAIL                  64
#define VSCP_TYPE_DIAGNOSTIC_SENSOR_FAIL                    65
#define VSCP_TYPE_DIAGNOSTIC_SAFESTATE                      66
#define VSCP_TYPE_DIAGNOSTIC_SIGNAL_IMPLAUSIBLE             67
#define VSCP_TYPE_DIAGNOSTIC_STORAGE_FAIL                   68
#define VSCP_TYPE_DIAGNOSTIC_SELFTEST_FAIL                  69
#define VSCP_TYPE_DIAGNOSTIC_ESD_EMC_EMI                    70
#define VSCP_TYPE_DIAGNOSTIC_TIMEOUT                        71
#define VSCP_TYPE_DIAGNOSTIC_LCD_FAIL                       72
#define VSCP_TYPE_DIAGNOSTIC_TOUCHPANEL_FAIL                73
#define VSCP_TYPE_DIAGNOSTIC_NOLOAD                         74
#define VSCP_TYPE_DIAGNOSTIC_COOLING_FAIL                   75
#define VSCP_TYPE_DIAGNOSTIC_HEATING_FAIL                   76
#define VSCP_TYPE_DIAGNOSTIC_TX_FAIL                        77
#define VSCP_TYPE_DIAGNOSTIC_RX_FAIL                        78
// 507 reserved for diagnostics

// class 508 (0x1FC) -- ERROR
#define VSCP_TYPE_ERROR_SUCCESS                             0
#define VSCP_TYPE_ERROR_ERROR                               1
// 2-6 reserved
#define VSCP_TYPE_ERROR_CHANNEL                             7
#define VSCP_TYPE_ERROR_FIFO_EMPTY                          8
#define VSCP_TYPE_ERROR_FIFO_FULL                           9
#define VSCP_TYPE_ERROR_FIFO_SIZE                           10
#define VSCP_TYPE_ERROR_FIFO_WAIT                           11      
#define VSCP_TYPE_ERROR_GENERIC                             12
#define VSCP_TYPE_ERROR_HARDWARE                            13
#define VSCP_TYPE_ERROR_INIT_FAIL                           14
#define VSCP_TYPE_ERROR_INIT_MISSING                        15		
#define VSCP_TYPE_ERROR_INIT_READY                          16
#define VSCP_TYPE_ERROR_NOT_SUPPORTED                       17
#define VSCP_TYPE_ERROR_OVERRUN                             18
#define VSCP_TYPE_ERROR_RCV_EMPTY                           19
#define VSCP_TYPE_ERROR_REGISTER                            20
#define VSCP_TYPE_ERROR_TRM_FULL                            21
// 22-27 reserved
#define VSCP_TYPE_ERROR_LIBRARY                             28
#define VSCP_TYPE_ERROR_PROCADDRESS                         29
#define VSCP_TYPE_ERROR_ONLY_ONE_INSTANCE                   30
#define VSCP_TYPE_ERROR_SUB_DRIVER                          31
#define VSCP_TYPE_ERROR_TIMEOUT                             32
#define VSCP_TYPE_ERROR_NOT_OPEN                            33
#define VSCP_TYPE_ERROR_PARAMETER                           34
#define VSCP_TYPE_ERROR_MEMORY                              35
#define VSCP_TYPE_ERROR_INTERNAL                            36
#define VSCP_TYPE_ERROR_COMMUNICATION                       37
#define VSCP_TYPE_ERROR_USER                                38
#define VSCP_TYPE_ERROR_PASSWORD                            39
#define VSCP_TYPE_ERROR_CONNECTION                          40   
#define VSCP_TYPE_ERROR_INVALID_HANDLE                      41
#define VSCP_TYPE_ERROR_OPERATION_FAILED                    42

// class 509 (0x1FD) -- LOG
#define VSCP_TYPE_LOG_GENERAL                               0
#define VSCP_TYPE_LOG_MESSAGE                               1
#define VSCP_TYPE_LOG_START                                 2
#define VSCP_TYPE_LOG_STOP                                  3
#define VSCP_TYPE_LOG_LEVEL                                 4

// class 510 (0x1FE) -- LABORATORY
#define VSCP_TYPE_LABORATORY_GENERAL                        0

// class 511 (0x1FF) -- LOCAL
#define VSCP_TYPE_LOCAL_GENERAL                             0

///////////////////////////////////////////////////////////////////////////////
//				Level II
///////////////////////////////////////////////////////////////////////////////

// Level II Protocol functionality Class=1024 (0x400)
#define VSCP2_TYPE_PROTOCOL_GENERAL                         0
#define VSCP2_TYPE_PROTOCOL_READ_REGISTER                   1
#define VSCP2_TYPE_PROTOCOL_WRITE_REGISTER                  2
#define VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE             3

// Level II Control functionality Class=1025 (0x401)
#define VSCP2_TYPE_CONTROL_GENERAL                          0

// Level II Information functionality Class=1026 (0x402)
#define VSCP2_TYPE_INFORMATION_GENERAL                      0
#define VSCP2_TYPE_INFORMATION_TOKEN_ACTIVITY               1

// Level II Text to speech functionality Class=1028 (0x404)
#define VSCP2_TYPE_TEXT2SPEECH_GENERAL                      0
#define VSCP2_TYPE_TEXT2SPEECH_TALK                         1

// Level II Custom functionality Class=1029 (0x405)
#define VSCP2_TYPE_CUSTOM_GENERAL                           0

// Level II Display functionality Class=1030 (0x406)
#define VSCP2_TYPE_DISPLAY_GENERAL                          0

// Level II Measurement String Class=1040 (0x410)
#define VSCP2_TYPE_MEASUREMENT_STR_GENERAL                  0
#define VSCP_TYPE_MEASUREMENT_COUNT                         1
#define VSCP_TYPE_MEASUREMENT_COUNT_LENGTH                  2
#define VSCP_TYPE_MEASUREMENT_COUNT_MASS                        3
#define VSCP_TYPE_MEASUREMENT_COUNT_TIME                        4
#define VSCP_TYPE_MEASUREMENT_COUNT_ELECTRIC_CURRENT            5
#define VSCP_TYPE_MEASUREMENT_COUNT_TEMPERATURE                 6
#define VSCP_TYPE_MEASUREMENT_COUNT_AMOUNT_OF_SUBSTANCE         7
#define VSCP_TYPE_MEASUREMENT_COUNT_INTENSITY_OF_LIGHT          8
#define VSCP_TYPE_MEASUREMENT_COUNT_FREQUENCY                   9
#define VSCP_TYPE_MEASUREMENT_COUNT_RADIOACTIVITY               10	// or other random event
#define VSCP_TYPE_MEASUREMENT_COUNT_FORCE                       11
#define VSCP_TYPE_MEASUREMENT_COUNT_PRESSURE                    12
#define VSCP_TYPE_MEASUREMENT_COUNT_ENERGY                      13
#define VSCP_TYPE_MEASUREMENT_COUNT_POWER                       14
#define VSCP_TYPE_MEASUREMENT_COUNT_ELECTRICAL_CHARGE           15
#define VSCP_TYPE_MEASUREMENT_COUNT_ELECTRICAL_POTENTIAL        16	// Voltage
#define VSCP_TYPE_MEASUREMENT_COUNT_ELECTRICAL_CAPACITANCE      17
#define VSCP_TYPE_MEASUREMENT_COUNT_ELECTRICAL_RECISTANCE       18
#define VSCP_TYPE_MEASUREMENT_COUNT_ELECTRICAL_CONDUCTANCE      19
#define VSCP_TYPE_MEASUREMENT_COUNT_MAGNETIC_FIELD_STRENGTH     20
#define VSCP_TYPE_MEASUREMENT_COUNT_MAGNETIC_FLUX               21
#define VSCP_TYPE_MEASUREMENT_COUNT_MAGNETIC_FLUX_DENSITY       22
#define VSCP_TYPE_MEASUREMENT_COUNT_INDUCTANCE                  23
#define VSCP_TYPE_MEASUREMENT_COUNT_FLUX_OF_LIGHT               24
#define VSCP_TYPE_MEASUREMENT_COUNT_ILLUMINANCE                 25
#define VSCP_TYPE_MEASUREMENT_COUNT_RADIATION_DOSE              26
#define VSCP_TYPE_MEASUREMENT_COUNT_CATALYTIC_ACITIVITY         27
#define VSCP_TYPE_MEASUREMENT_COUNT_VOLUME                      28
#define VSCP_TYPE_MEASUREMENT_COUNT_SOUND_INTENSITY             29
#define VSCP_TYPE_MEASUREMENT_COUNT_ANGLE                       30
#define VSCP_TYPE_MEASUREMENT_COUNT_POSITION                    31
#define VSCP_TYPE_MEASUREMENT_COUNT_SPEED                       32
#define VSCP_TYPE_MEASUREMENT_COUNT_ACCELERATION                33
#define VSCP_TYPE_MEASUREMENT_COUNT_TENSION                     34
#define VSCP_TYPE_MEASUREMENT_COUNT_HUMIDITY                    35	// Damp/moist (Hygrometer reading)
#define VSCP_TYPE_MEASUREMENT_COUNT_FLOW                        36
#define VSCP_TYPE_MEASUREMENT_COUNT_THERMAL_RESISTANCE          37
#define VSCP_TYPE_MEASUREMENT_COUNT_REFRACTIVE_POWER            38
#define VSCP_TYPE_MEASUREMENT_COUNT_DYNAMIC_VISCOSITY           39
#define VSCP_TYPE_MEASUREMENT_COUNT_SOUND_IMPEDANCE             40
#define VSCP_TYPE_MEASUREMENT_COUNT_SOUND_RESISTANCE            41
#define VSCP_TYPE_MEASUREMENT_COUNT_ELECTRIC_ELASTANCE          42
#define VSCP_TYPE_MEASUREMENT_COUNT_LUMINOUS_ENERGY             43
#define VSCP_TYPE_MEASUREMENT_COUNT_LUMINANCE                   44
#define VSCP_TYPE_MEASUREMENT_COUNT_CHEMICAL_CONCENTRATION      45
#define VSCP_TYPE_MEASUREMENT_COUNT_RESERVED	                46
#define VSCP_TYPE_MEASUREMENT_COUNT_DOSE_EQVIVALENT             47
#define VSCP_TYPE_MEASUREMENT_COUNT_DEWPOINT                    49
#define VSCP_TYPE_MEASUREMENT_COUNT_RELATIVE_LEVEL              50
#define VSCP_TYPE_MEASUREMENT_COUNT_ALTITUDE                    51
#define VSCP_TYPE_MEASUREMENT_COUNT_AREA                        52
#define VSCP_TYPE_MEASUREMENT_COUNT_RADIANT_INTENSITY           53
#define VSCP_TYPE_MEASUREMENT_COUNT_RADIANCE                    54
#define VSCP_TYPE_MEASUREMENT_COUNT_IRRADIANCE                  55
#define VSCP_TYPE_MEASUREMENT_COUNT_SPECTRAL_RADIANCE           56
#define VSCP_TYPE_MEASUREMENT_COUNT_SPECTRAL_IRRADIANCE         57

#define VSCP2_TYPE_INFORMATION_GENERAL                          0
#define VSCP2_TYPE_INFORMATION_TOKEN_ACTIVITY                   1
#define VSCP2_TYPE_INFORMATION_HEART_BEAT                       2

// Level II VSCPD daemon functionality Class=65535 (0xFFFF)
#define VSCP2_TYPE_VSCPD_GENERAL                          0
#define VSCP2_TYPE_VSCPD_LOOP                             1
#define VSCP2_TYPE_VSCPD_PAUSE                            3
#define VSCP2_TYPE_VSCPD_ACTIVATE                         4
#define VSCP2_TYPE_VSCPD_SECOND                           5
#define VSCP2_TYPE_VSCPD_MINUTE                           6
#define VSCP2_TYPE_VSCPD_HOUR                             7
#define VSCP2_TYPE_VSCPD_NOON                             8
#define VSCP2_TYPE_VSCPD_MIDNIGHT                         9
#define VSCP2_TYPE_VSCPD_WEEK                             11
#define VSCP2_TYPE_VSCPD_MONTH                            12
#define VSCP2_TYPE_VSCPD_QUARTER                          13
#define VSCP2_TYPE_VSCPD_YEAR                             14
#define VSCP2_TYPE_VSCPD_RANDOM_MINUTE                    15
#define VSCP2_TYPE_VSCPD_RANDOM_HOUR                      16
#define VSCP2_TYPE_VSCPD_RANDOM_DAY                       17
#define VSCP2_TYPE_VSCPD_RANDOM_WEEK                      18
#define VSCP2_TYPE_VSCPD_RANDOM_MONTH                     19
#define VSCP2_TYPE_VSCPD_RANDOM_YEAR                      20
#define VSCP2_TYPE_VSCPD_DUSK                             21
#define VSCP2_TYPE_VSCPD_DAWN                             22
#define VSCP2_TYPE_VSCPD_STARTING_UP                      23
#define VSCP2_TYPE_VSCPD_SHUTTING_DOWN                    24
#define VSCP2_TYPE_VSCPD_TIMER_STARTED                    25
#define VSCP2_TYPE_VSCPD_TIMER_PAUSED                     26
#define VSCP2_TYPE_VSCPD_TIMER_RESUMED                    27
#define VSCP2_TYPE_VSCPD_TIMER_STOPPED                    28
#define VSCP2_TYPE_VSCPD_TIMER_ELLAPSED                   29

#endif
