/**
 * @brief           VSCP Level I/II class definition file
 * @file            vscp_class.h
 * @author          Ake Hedman, Grodans Paradis AB, www.vscp.org
 *
 * @section description Description
 **********************************
 * This module contains the definitions for the
 * available VSCP class id's
 *********************************************************************/

/* ******************************************************************************
 * VSCP (Very Simple Control Protocol)
 * http://www.vscp.org
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2000-2019 Ake Hedman, 
 * Grodans Paradis AB <info@grodansparadis.com>
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
 *  This file is part of VSCP - Very Simple Control Protocol
 *  http://www.vscp.org
 *
 * ******************************************************************************
 */


/*
            !!!!!!!!!!!!!!!!!!!!  W A R N I N G  !!!!!!!!!!!!!!!!!!!!
                           This file is auto-generated
                see https://github.com/grodansparadis/vscp-classes
                        Generated: 2019-09-25 15:42:26.574361
*/
 
#ifndef VSCP_CLASS_H
#define VSCP_CLASS_H
 
fname=../classes/class_0_protocol.xml
#define VSCP_CLASS1_PROTOCOL                 0         /* VSCP Protocol Functionality */
fname=../classes/class_1_alarm.xml
#define VSCP_CLASS1_ALARM                    1         /* Alarm functionality */
fname=../classes/class_2_security.xml
#define VSCP_CLASS1_SECURITY                 2         /* Security */
fname=../classes/class_10_measurement.xml
#define VSCP_CLASS1_MEASUREMENT              10        /* Measurement */
fname=../classes/class_15_data.xml
#define VSCP_CLASS1_DATA                     15        /* Data */
fname=../classes/class_20_information.xml
#define VSCP_CLASS1_INFORMATION              20        /* Information */
fname=../classes/class_30_control.xml
#define VSCP_CLASS1_CONTROL                  30        /* Control */
fname=../classes/class_40_multimedia.xml
#define VSCP_CLASS1_MULTIMEDIA               40        /* Multimedia */
fname=../classes/class_50_aol.xml
#define VSCP_CLASS1_AOL                      50        /* Alert On LAN */
fname=../classes/class_60_measurement64.xml
#define VSCP_CLASS1_MEASUREMENT64            60        /* Double precision floating point measurement */
fname=../classes/class_65_.measurezone.xml
#define VSCP_CLASS1_MEASUREZONE              65        /* Measurement with zone */
fname=../classes/class_70_.measurement32.xml
#define VSCP_CLASS1_MEASUREMENT32            70        /* Single precision floating point measurement */
fname=../classes/class_85_setvaluezone.xml
#define VSCP_CLASS1_SETVALUEZONE             85        /* Set value with zone */
fname=../classes/class_90_weather.xml
#define VSCP_CLASS1_WEATHER                  90        /* Weather */
fname=../classes/class_95_weather_forecast.xml
#define VSCP_CLASS1_WEATHER_FORECAST         95        /* Weather forecast */
fname=../classes/class_100_phone.xml
#define VSCP_CLASS1_PHONE                    100       /* Phone */
fname=../classes/class_102_display.xml
#define VSCP_CLASS1_DISPLAY                  102       /* Display */
fname=../classes/class_110_remote.xml
#define VSCP_CLASS1_IR                       110       /* IR Remote I/f */
fname=../classes/class_206_gnss.xml
#define VSCP_CLASS1_GNSS                     206       /* Position (GNSS) */
fname=../classes/class_212_wireless.xml
#define VSCP_CLASS1_WIRELESS                 212       /* Wireless */
fname=../classes/class_506_diagnostic.xml
#define VSCP_CLASS1_DIAGNOSTIC               506       /* Diagnostic */
fname=../classes/class_508_error.xml
#define VSCP_CLASS1_ERROR                    508       /* Error */
fname=../classes/class_509_log.xml
#define VSCP_CLASS1_LOG                      509       /* Logging */
fname=../classes/class_510_laboratory.xml
#define VSCP_CLASS1_LABORATORY               510       /* Laboratory use */
fname=../classes/class_511_local.xml
#define VSCP_CLASS1_LOCAL                    511       /* Local use */
fname=../classes/class_512_protocol.xml
#define VSCP_CLASS2_LEVEL1_PROTOCOL          512       /* Class2 Level I Protocol */
fname=../classes/class_513_alarm.xml
#define VSCP_CLASS2_LEVEL1_ALARM             513       /* Class2 Level I Alarm */
fname=../classes/class_514_security.xml
#define VSCP_CLASS2_LEVEL1_SECURITY          514       /* Class2 Level I Security */
fname=../classes/class_522_measurement.xml
#define VSCP_CLASS2_LEVEL1_MEASUREMENT       522       /* Class2 Level I Measurements */
fname=../classes/class_527_data.xml
#define VSCP_CLASS2_LEVEL1_DATA              527       /* Class2 Level I Data */
fname=../classes/class_532_information.xml
#define VSCP_CLASS2_LEVEL1_INFORMATION1      532       /* Class2 Level I Information */
fname=../classes/class_542_control.xml
#define VSCP_CLASS2_LEVEL1_CONTROL           542       /* Class2 Level I Control */
fname=../classes/class_552_multimedia.xml
#define VSCP_CLASS2_LEVEL1_MULTIMEDIA        552       /* Class2 Level I Multimedia */
fname=../classes/class_562_aol.xml
#define VSCP_CLASS2_LEVEL1_AOL               562       /* Class2 Level I AOL */
fname=../classes/class_572_measurement64.xml
#define VSCP_CLASS2_LEVEL1_MEASUREMENT64     572       /* Class2 Level I Measurement64 */
fname=../classes/class_577_measurezone.xml
#define VSCP_CLASS2_LEVEL1_MEASUREZONE       577       /* Class2 Level I Measurementzone */
fname=../classes/class_582_measurement32.xml
#define VSCP_CLASS2_LEVEL1_MEASUREMENT32     582       /* Class2 Level I Measuremet32 */
fname=../classes/class_597_setvaluezone.xml
#define VSCP_CLASS2_LEVEL1_SETVALUEZONE      597       /* Class2 Level I SetValueZone */
fname=../classes/class_602.weather.xml
#define VSCP_CLASS2_LEVEL1_WEATHER           602       /* Class2 Level I Weather */
fname=../classes/class_607_weather_forecast.xml
#define VSCP_CLASS2_LEVEL1_WEATHERFORECAST   607       /* Class2 Level I Weather Forecast */
fname=../classes/class_612_phone.xml
#define VSCP_CLASS2_LEVEL1_PHONE             612       /* Class2 Level I Phone */
fname=../classes/class_614_display.xml
#define VSCP_CLASS2_LEVEL1_DISPLAY           614       /* Class2 Level I Display */
fname=../classes/class_622_remote.xml
#define VSCP_CLASS2_LEVEL1_IR                622       /* Class2 Level I IR */
fname=../classes/class_718_gnss.xml
#define VSCP_CLASS2_LEVEL1_GNSS              718       /* Class2 Level I GNSS */
fname=../classes/class_724_wireless.xml
#define VSCP_CLASS2_LEVEL1_WIRELESS          724       /* Class2 Level I Wireless */
fname=../classes/class_1018_diagnostic.xml
#define VSCP_CLASS2_LEVEL1_DIAGNOSTIC        1018      /* Class2 Level I Diagnostic */
fname=../classes/class_1020_error.xml
#define VSCP_CLASS2_LEVEL1_ERROR             1020      /* Class2 Level I Error */
fname=../classes/class_1021_log.xml
#define VSCP_CLASS2_LEVEL1_LOG               1021      /* Class2 Level I Log */
fname=../classes/class_1022_laboratory.xml
#define VSCP_CLASS2_LEVEL1_LABORATORY        1022      /* Class2 Level I Laboratory */
fname=../classes/class_1023_local.xml
#define VSCP_CLASS2_LEVEL1_LOCAL             1023      /* Class2 Level I Local */
fname=../classes/class_1024_protocol.xml
#define VSCP_CLASS2_PROTOCOL                 1024      /* Level II Protocol Functionality */
fname=../classes/class_1025_control.xml
#define VSCP_CLASS2_CONTROL                  1025      /* Level II Control */
fname=../classes/class_1026_information.xml
#define VSCP_CLASS2_INFORMATION              1026      /* Level II Information */
fname=../classes/class_1027_text2speech.xml
#define VSCP_CLASS2_TEXT2SPEECH              1027      /* Text to speech */
fname=../classes/class_1029_custom.xml
#define VSCP_CLASS2_CUSTOM                   1029      /* Level II Custom */
fname=../classes/class_1030_display.xml
#define VSCP_CLASS2_DISPLAY                  1030      /* Level II Display */
fname=../classes/class_1040_measurement_str.xml
#define VSCP_CLASS2_MEASUREMENT_STR          1040      /* Measurement string */
fname=../classes/class_1060_measurement_float.xml
#define VSCP_CLASS2_MEASUREMENT_FLOAT        1060      /* Measurement float */
fname=../classes/class_65535_vscpd.xml
#define VSCP_CLASS2_VSCPD                    65535     /* VSCP Daemon internal events */
 
#endif
 
