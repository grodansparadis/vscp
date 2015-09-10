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
 *  This file is part of VSCP - Very Simple Control Protocol
 *  http://www.vscp.org
 *
 * ******************************************************************************
 */

#ifndef VSCP_CLASS_H
#define VSCP_CLASS_H

#define VSCP_CLASS1_PROTOCOL                            0
#define VSCP_CLASS1_ALARM                               1
#define VSCP_CLASS1_SECURITY                            2
#define VSCP_CLASS1_MEASUREMENT                         10
#define VSCP_CLASS1_DATA                                15
#define VSCP_CLASS1_INFORMATION                         20
#define VSCP_CLASS1_CONTROL                             30
#define VSCP_CLASS1_MULTIMEDIA                          40
#define VSCP_CLASS1_AOL                                 50
#define VSCP_CLASS1_MEASUREMENT64                       60
#define VSCP_CLASS1_MEASUREZONE                         65
#define VSCP_CLASS1_MEASUREMENT32                       70
#define VSCP_CLASS1_SETVALUEZONE                        85
#define VSCP_CLASS1_WEATHER                             90
#define VSCP_CLASS1_WEATHER_FORECAST                    95
#define VSCP_CLASS1_PHONE                               100
#define VSCP_CLASS1_LIN                                 101
#define VSCP_CLASS1_DISPLAY                             102
#define VSCP_CLASS1_REMOTE                              110

// 200-205 has been deprecated
#define VSCP_CLASS1_ONEWIRE                             200
#define VSCP_CLASS1_X10                                 201
#define VSCP_CLASS1_LON                                 202
#define VSCP_CLASS1_EIB                                 203
#define VSCP_CLASS1_SNAP                                204
#define VSCP_CLASS1_MUMIN                               205

#define VSCP_CLASS1_GPS                                 206
#define VSCP_CLASS1_WIRELESS                            212
#define VSCP_CLASS1_DIAGNOSTIC                          506
#define VSCP_CLASS1_ERROR                               508
#define VSCP_CLASS1_LOG                                 509
#define VSCP_CLASS1_LAB                                 510
#define VSCP_CLASS1_LOCAL                               511

// Level I classes on Level II

#define VSCP_CLASS2_LEVEL1_PROTOCOL                     ( 512 + 0 )
#define VSCP_CLASS2_LEVEL1_ALARM                        ( 512 + 1 )
#define VSCP_CLASS2_LEVEL1_SECURITY                     ( 512 + 2 )
#define VSCP_CLASS2_LEVEL1_MEASUREMENT                  ( 512 + 10 )
#define VSCP_CLASS2_LEVEL1_DATA                         ( 512 + 15 )
#define VSCP_CLASS2_LEVEL1_INFORMATION                  ( 512 + 20 )
#define VSCP_CLASS2_LEVEL1_CONTROL                      ( 512 + 30 )
#define VSCP_CLASS2_LEVEL1_MULTIMEDIA                   ( 512 + 40 )
#define VSCP_CLASS2_LEVEL1_AOL                          ( 512 + 50 )
#define VSCP_CLASS2_MEASUREMENT64                       ( 512 + 60 )
#define VSCP_CLASS2_MEASUREZONE                         ( 512 + 65 )
#define VSCP_CLASS2_MEASUREMENT32                       ( 512 + 70 )
#define VSCP_CLASS2_SETVALUEZONE                        ( 512 + 85 )
#define VSCP_CLASS2_LEVEL1_PHONE                        ( 512 + 100 )
#define VSCP_CLASS2_LEVEL1_LIN                          ( 512 + 101 )
#define VSCP_CLASS2_LEVEL1_REMOTE                       ( 512 + 110 )
#define VSCP_CLASS2_LEVEL1_GPS                          ( 512 + 206 )
#define VSCP_CLASS2_LEVEL1_WIRELESS                     ( 512 + 212 )
#define VSCP_CLASS2_LEVEL1_LOG                          ( 512 + 509 )
#define VSCP_CLASS2_LEVEL1_LAB                          ( 512 + 510 )
#define VSCP_CLASS2_LEVEL1_LOCAL                        ( 512 + 511 )

// Level II classes
#define VSCP_CLASS2_PROTOCOL                            ( 1024 )
#define VSCP_CLASS2_CONTROL                             ( 1025 )
#define VSCP_CLASS2_INFORMATION                         ( 1026 )
#define VSCP_CLASS2_TEXT2SPEECH                         ( 1028 )
#define VSCP_CLASS2_CUSTOM                              ( 1029 )
#define VSCP_CLASS2_DISPLAY                             ( 1030 )
#define VSCP_CLASS2_MEASUREMENT_STR                     ( 1040 )
#define VSCP_CLASS2_MEASUREMENT_FLOAT                   ( 1060 )
#define VSCP_CLASS2_VSCPD                               ( 65535 )

#endif
