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
 * Copyright (C) 1995-2014 Ake Hedman, 
 * Grodans Paradis AB, <akhe@grodansparadis.com>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 
 *	This file is part of VSCP - Very Simple Control Protocol 	
 *	http://www.vscp.org
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
#define VSCP_CLASS1_MEASUREMENT64						60
#define VSCP_CLASS1_MEASUREZONE							65
#define VSCP_CLASS1_SETVALUEZONE						85
#define VSCP_CLASS1_PHONE                               100
#define VSCP_CLASS1_LIN                                 101
#define VSCP_CLASS1_DISPLAY                             102
#define VSCP_CLASS1_RC5                                 110
#define VSCP_CLASS1_ONEWIRE                             200
#define VSCP_CLASS1_X10                                 201
#define VSCP_CLASS1_LON                                 202
#define VSCP_CLASS1_EIB                                 203
#define VSCP_CLASS1_SNAP                                204
#define VSCP_CLASS1_MUMIN                               205
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
#define VSCP_CLASS2_MEASUREMENT64						( 512 + 60 )
#define VSCP_CLASS2_MEASUREZONE							( 512 + 65 )
#define VSCP_CLASS2_SETVALUEZONE						( 512 + 85 )
#define VSCP_CLASS2_LEVEL1_PHONE                        ( 512 + 100 )
#define VSCP_CLASS2_LEVEL1_LIN                          ( 512 + 101 )
#define VSCP_CLASS2_LEVEL1_RC5                          ( 512 + 110 )
#define VSCP_CLASS2_LEVEL1_ONEWIRE                      ( 512 + 200 )
#define VSCP_CLASS2_LEVEL1_X10                          ( 512 + 201 )
#define VSCP_CLASS2_LEVEL1_LON                          ( 512 + 202 )
#define VSCP_CLASS2_LEVEL1_EIB                          ( 512 + 203 )
#define VSCP_CLASS2_LEVEL1_SNAP                         ( 512 + 204 )
#define VSCP_CLASS2_LEVEL1_MUMIN                        ( 512 + 205 )
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
#define VSCP_CLASS2_VSCPD                               ( 65535 )

#endif
