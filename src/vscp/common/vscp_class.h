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
 * Copyright (c) 2000-2018 Ake Hedman, 
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
('                        Generated:', datetime.datetime(2018, 7, 4, 15, 41, 28, 274503))
*/
 
#ifndef VSCP_CLASS_H
#define VSCP_CLASS_H
 
#define CLASS1_PROTOCOL                 0         /* VSCP Protocol Functionality */
#define CLASS1_ALARM                    1         /* Alarm functionality */
#define CLASS1_SECURITY                 2         /* Security */
#define CLASS1_MEASUREMENT              10        /* Measurement */
#define CLASS1_DATA                     15        /* Data */
#define CLASS1_INFORMATION              20        /* Information */
#define CLASS1_CONTROL                  30        /* Control */
#define CLASS1_MULTIMEDIA               40        /* Multimedia */
#define CLASS1_AOL                      50        /* Alert On LAN */
#define CLASS1_MEASUREMENT64            60        /* Double precision floating point measurement */
#define CLASS1_MEASUREZONE              65        /* Measurement with zone */
#define CLASS1_MEASUREMENT32            70        /* Single precision floating point measurement */
#define CLASS1_SETVALUEZONE             85        /* Set value with zone */
#define CLASS1_WEATHER                  90        /* Weather */
#define CLASS1_WEATHER_FORECAST         95        /* Weather forecast */
#define CLASS1_PHONE                    100       /* Phone */
#define CLASS1_DISPLAY                  102       /* Display */
#define CLASS1_IR                       110       /* IR Remote I/f */
#define CLASS1_GNSS                     206       /* Position (GNSS) */
#define CLASS1_WIRELESS                 212       /* Wireless */
#define CLASS1_DIAGNOSTIC               506       /* Diagnostic */
#define CLASS1_ERROR                    508       /* Error */
#define CLASS1_LOG                      509       /* Logging */
#define CLASS1_LABORATORY               510       /* Laboratory use */
#define CLASS1_LOCAL                    511       /* Local use */
#define CLASS2_LEVEL1_PROTOCOL          512       /* Class2 Level I Protocol */
#define CLASS2_LEVEL1_ALARM             513       /* Class2 Level I Alarm */
#define CLASS2_LEVEL1_SECURITY          514       /* Class2 Level I Security */
#define CLASS2_LEVEL1_MEASURMENT        522       /* Class2 Level I Measurements */
#define CLASS2_LEVEL1_DATA              527       /* Class2 Level I Data */
#define CLASS2_LEVEL1_INFORMATION1      532       /* Class2 Level I Information */
#define CLASS2_LEVEL1_CONTROL           542       /* Class2 Level I Control */
#define CLASS2_LEVEL1_MULTIMEDIA        552       /* Class2 Level I Multimedia */
#define CLASS2_LEVEL1_AOL               562       /* Class2 Level I AOL */
#define CLASS2_LEVEL1_MEASURMENT64      572       /* Class2 Level I Measurement64 */
#define CLASS2_LEVEL1_MEASUREMENTZONE   577       /* Class2 Level I Measurementzone */
#define CLASS2_LEVEL1_MEASUREMENT32     582       /* Class2 Level I Measuremet32 */
#define CLASS2_LEVEL1_SETVALUEZONE      597       /* Class2 Level I SetValueZone */
#define CLASS2_LEVEL1_WEATHER           602       /* Class2 Level I Weather */
#define CLASS2_LEVEL1_WEATHERFORECAST   607       /* Class2 Level I Weather Forecast */
#define CLASS2_LEVEL1_PHONE             612       /* Class2 Level I Phone */
#define CLASS2_LEVEL1_DISPLAY           614       /* Class2 Level I Display */
#define CLASS2_LEVEL1_IR                622       /* Class2 Level I IR */
#define CLASS2_LEVEL1_GNSS              718       /* Class2 Level I GNSS */
#define CLASS2_LEVEL1_WIRELESS          724       /* Class2 Level I Wireless */
#define CLASS2_LEVEL1_DIAGNOSTIC        1018      /* Class2 Level I Diagnostic */
#define CLASS2_LEVEL1_ERROR             1020      /* Class2 Level I Error */
#define CLASS2_LEVEL1_LOG               1021      /* Class2 Level I Log */
#define CLASS2_LEVEL1_LABORATORY        1022      /* Class2 Level I Laboratory */
#define CLASS2_LEVEL1_LOCAL             1023      /* Class2 Level I Local */
#define CLASS2_PROTOCOL                 1024      /* Level II Protocol Functionality */
#define CLASS2_CONTROL                  1025      /* Level II Control */
#define CLASS2_INFORMATION              1026      /* Level II Information */
#define CLASS2_TEXT2SPEECH              1027      /* Text to speech */
#define CLASS2_CUSTOM                   1029      /* Level II Custom */
#define CLASS2_DISPLAY                  1030      /* Level II Display */
#define CLASS2_MEASUREMENT_STR          1040      /* Measurement string */
#define CLASS2_MEASUREMENT_FLOAT        1060      /* Measurement float */
#define CLASS2_VSCPD                    65535     /* VSCP Daemon internal events */
 
#endif
 
