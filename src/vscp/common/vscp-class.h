/**
 * @brief           VSCP Level I/II class definition file
 * @file            vscp_class.h
 * @author          Ake Hedman, The VSCP Project, www.vscp.org
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
 * Copyright (C) 2000-2022 Ake Hedman, 
 * The VSCP Project <info@grodansparadis.com>
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
                        Generated: 2024-12-12 17:07:55.637062
*/
 
#ifndef VSCP_CLASS_H
#define VSCP_CLASS_H
 
#define VSCP_CLASS1_PROTOCOL                 0         /* VSCP Protocol Functionality */
#define VSCP_CLASS1_ALARM                    1         /* Alarm functionality */
#define VSCP_CLASS1_SECURITY                 2         /* Security */
#define VSCP_CLASS1_MEASUREMENT              10        /* Measurement */
#define VSCP_CLASS1_MEASUREMENTX1            11        /* Measurement */
#define VSCP_CLASS1_MEASUREMENTX2            12        /* Measurement */
#define VSCP_CLASS1_MEASUREMENTX3            13        /* Measurement */
#define VSCP_CLASS1_MEASUREMENTX4            14        /* Measurement */
#define VSCP_CLASS1_DATA                     15        /* Data */
#define VSCP_CLASS1_INFORMATION              20        /* Information */
#define VSCP_CLASS1_CONTROL                  30        /* Control */
#define VSCP_CLASS1_MULTIMEDIA               40        /* Multimedia */
#define VSCP_CLASS1_AOL                      50        /* Alert On LAN */
#define VSCP_CLASS1_MEASUREMENT64            60        /* Double precision floating point measurement */
#define VSCP_CLASS1_MEASUREMENT64X1          61        /* Double precision floating point measurement */
#define VSCP_CLASS1_MEASUREMENT64X2          62        /* Double precision floating point measurement */
#define VSCP_CLASS1_MEASUREMENT64X3          63        /* Double precision floating point measurement */
#define VSCP_CLASS1_MEASUREMENT64X4          64        /* Double precision floating point measurement */
#define VSCP_CLASS1_MEASUREZONE              65        /* Measurement with zone */
#define VSCP_CLASS1_MEASUREZONEX1            66        /* Measurement with zone */
#define VSCP_CLASS1_MEASUREZONEX2            67        /* Measurement with zone */
#define VSCP_CLASS1_MEASUREZONEX3            68        /* Measurement with zone */
#define VSCP_CLASS1_MEASUREZONEX4            69        /* Measurement with zone */
#define VSCP_CLASS1_MEASUREMENT32            70        /* Single precision floating point measurement */
#define VSCP_CLASS1_MEASUREMENT32X1          71        /* Single precision floating point measurement */
#define VSCP_CLASS1_MEASUREMENT32X2          72        /* Single precision floating point measurement */
#define VSCP_CLASS1_MEASUREMENT32X3          73        /* Single precision floating point measurement */
#define VSCP_CLASS1_MEASUREMENT32X4          74        /* Single precision floating point measurement */
#define VSCP_CLASS1_SETVALUEZONE             85        /* Set value with zone */
#define VSCP_CLASS1_SETVALUEZONEX1           86        /* Set value with zone */
#define VSCP_CLASS1_SETVALUEZONEX2           87        /* Set value with zone */
#define VSCP_CLASS1_SETVALUEZONEX3           88        /* Set value with zone */
#define VSCP_CLASS1_SETVALUEZONEX4           89        /* Set value with zone */
#define VSCP_CLASS1_WEATHER                  90        /* Weather */
#define VSCP_CLASS1_WEATHER_FORECAST         95        /* Weather forecast */
#define VSCP_CLASS1_PHONE                    100       /* Phone */
#define VSCP_CLASS1_DISPLAY                  102       /* Display */
#define VSCP_CLASS1_IR                       110       /* IR Remote I/f */
#define VSCP_CLASS1_CONFIGURATION            120       /* Configuration */
#define VSCP_CLASS1_GNSS                     206       /* Position (GNSS) */
#define VSCP_CLASS1_WIRELESS                 212       /* Wireless */
#define VSCP_CLASS1_DIAGNOSTIC               506       /* Diagnostic */
#define VSCP_CLASS1_ERROR                    508       /* Error */
#define VSCP_CLASS1_LOG                      509       /* Logging */
#define VSCP_CLASS1_LABORATORY               510       /* Laboratory use */
#define VSCP_CLASS1_LOCAL                    511       /* Local use */
#define VSCP_CLASS2_LEVEL1_PROTOCOL          512       /* Class2 Level I Protocol */
#define VSCP_CLASS2_LEVEL1_ALARM             513       /* Class2 Level I Alarm */
#define VSCP_CLASS2_LEVEL1_SECURITY          514       /* Class2 Level I Security */
#define VSCP_CLASS2_LEVEL1_MEASUREMENT       522       /* Class2 Level I Measurements */
#define VSCP_CLASS2_LEVEL1_MEASUREMENTX1     523       /* Class2 Level I Measurements */
#define VSCP_CLASS2_LEVEL1_MEASUREMENTX2     524       /* Class2 Level I Measurements */
#define VSCP_CLASS2_LEVEL1_MEASUREMENTX3     525       /* Class2 Level I Measurements */
#define VSCP_CLASS2_LEVEL1_MEASUREMENTX4     526       /* Class2 Level I Measurements */
#define VSCP_CLASS2_LEVEL1_DATA              527       /* Class2 Level I Data */
#define VSCP_CLASS2_LEVEL1_INFORMATION1      532       /* Class2 Level I Information */
#define VSCP_CLASS2_LEVEL1_CONTROL           542       /* Class2 Level I Control */
#define VSCP_CLASS2_LEVEL1_MULTIMEDIA        552       /* Class2 Level I Multimedia */
#define VSCP_CLASS2_LEVEL1_AOL               562       /* Class2 Level I AOL */
#define VSCP_CLASS2_LEVEL1_MEASUREMENT64     572       /* Class2 Level I Measurement64 */
#define VSCP_CLASS2_LEVEL1_MEASUREMENT64X1   573       /* Class2 Level I Measurement64 */
#define VSCP_CLASS2_LEVEL1_MEASUREMENT64X2   574       /* Class2 Level I Measurement64 */
#define VSCP_CLASS2_LEVEL1_MEASUREMENT64X3   575       /* Class2 Level I Measurement64 */
#define VSCP_CLASS2_LEVEL1_MEASUREMENT64X4   576       /* Class2 Level I Measurement64 */
#define VSCP_CLASS2_LEVEL1_MEASUREZONE       577       /* Class2 Level I Measurementzone */
#define VSCP_CLASS2_LEVEL1_MEASUREZONEX1     578       /* Class2 Level I Measurementzone */
#define VSCP_CLASS2_LEVEL1_MEASUREZONEX2     579       /* Class2 Level I Measurementzone */
#define VSCP_CLASS2_LEVEL1_MEASUREZONEX3     580       /* Class2 Level I Measurementzone */
#define VSCP_CLASS2_LEVEL1_MEASUREZONEX4     581       /* Class2 Level I Measurementzone */
#define VSCP_CLASS2_LEVEL1_MEASUREMENT32     582       /* Class2 Level I Measuremet32 */
#define VSCP_CLASS2_LEVEL1_MEASUREMENT32X1   583       /* Class2 Level I Measuremet32 */
#define VSCP_CLASS2_LEVEL1_MEASUREMENT32X2   584       /* Class2 Level I Measuremet32 */
#define VSCP_CLASS2_LEVEL1_MEASUREMENT32X3   585       /* Class2 Level I Measuremet32 */
#define VSCP_CLASS2_LEVEL1_MEASUREMENT32X4   586       /* Class2 Level I Measuremet32 */
#define VSCP_CLASS2_LEVEL1_SETVALUEZONE      597       /* Class2 Level I SetValueZone */
#define VSCP_CLASS2_LEVEL1_SETVALUEZONEX1    598       /* Class2 Level I SetValueZone */
#define VSCP_CLASS2_LEVEL1_SETVALUEZONEX2    599       /* Class2 Level I SetValueZone */
#define VSCP_CLASS2_LEVEL1_SETVALUEZONEX3    600       /* Class2 Level I SetValueZone */
#define VSCP_CLASS2_LEVEL1_SETVALUEZONEX4    601       /* Class2 Level I SetValueZone */
#define VSCP_CLASS2_LEVEL1_WEATHER           602       /* Class2 Level I Weather */
#define VSCP_CLASS2_LEVEL1_WEATHERFORECAST   607       /* Class2 Level I Weather Forecast */
#define VSCP_CLASS2_LEVEL1_PHONE             612       /* Class2 Level I Phone */
#define VSCP_CLASS2_LEVEL1_DISPLAY           614       /* Class2 Level I Display */
#define VSCP_CLASS2_LEVEL1_IR                622       /* Class2 Level I IR */
#define VSCP_CLASS2_LEVEL1_GNSS              718       /* Class2 Level I GNSS */
#define VSCP_CLASS2_LEVEL1_WIRELESS          724       /* Class2 Level I Wireless */
#define VSCP_CLASS2_LEVEL1_DIAGNOSTIC        1018      /* Class2 Level I Diagnostic */
#define VSCP_CLASS2_LEVEL1_ERROR             1020      /* Class2 Level I Error */
#define VSCP_CLASS2_LEVEL1_LOG               1021      /* Class2 Level I Log */
#define VSCP_CLASS2_LEVEL1_LABORATORY        1022      /* Class2 Level I Laboratory */
#define VSCP_CLASS2_LEVEL1_LOCAL             1023      /* Class2 Level I Local */
#define VSCP_CLASS2_PROTOCOL                 1024      /* Level II Protocol Functionality */
#define VSCP_CLASS2_CONTROL                  1025      /* Level II Control */
#define VSCP_CLASS2_INFORMATION              1026      /* Level II Information */
#define VSCP_CLASS2_TEXT2SPEECH              1027      /* Level II Text to speech */
#define VSCP_CLASS2_HLO                      1028      /* Level II High Level Object */
#define VSCP_CLASS2_CUSTOM                   1029      /* Level II Custom */
#define VSCP_CLASS2_DISPLAY                  1030      /* Level II Display */
#define VSCP_CLASS2_SECURITY                 1034      /* Level II Security */
#define VSCP_CLASS2_MEASUREMENT_STR          1040      /* Level II Measurement string */
#define VSCP_CLASS2_MEASUREMENT_FLOAT        1060      /* Level II Measurement float */
#define VSCP_CLASS2_VSCPD                    65535     /* Level II VSCP Daemon internal events */
 
#endif
 
