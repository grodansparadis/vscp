 #!/usr/bin/env python

"""
 /**
 * @brief           VSCP Level I/II class definition file
 * @file            vscp_class.py
 * @author          Ake Hedman, eurosource, www.vscp.org
 *
 * @section description Description
 **********************************
 * This module contains the definitions for the
 * available VSCP class id's
 *********************************************************************/

/* ******************************************************************************
 * VSCP  Very Simple Control Protocol
 * http://www.vscp.org
 *
 * Copyright  (C)  1995-2018 Ake Hedman,
 * eurosource, <akhe@eurosource.se>
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
 *    This file is part of VSCP - Very Simple Control Protocol
 *    http://www.vscp.org
 *
 * ******************************************************************************
*/
"""

VSCP_CLASS1_PROTOCOL            =                  0
VSCP_CLASS1_ALARM               =                  1
VSCP_CLASS1_SECURITY            =                  2
VSCP_CLASS1_MEASUREMENT         =                  10
VSCP_CLASS1_DATA                =                  15
VSCP_CLASS1_INFORMATION         =                  20
VSCP_CLASS1_CONTROL             =                  30
VSCP_CLASS1_MULTIMEDIA          =                  40
VSCP_CLASS1_AOL                 =                  50
VSCP_CLASS1_MEASUREMENT64       =                  60
VSCP_CLASS1_MEASUREZONE         =                  65
VSCP_CLASS1_WEATHER             =                  90
VSCP_CLASS1_WEATHER_FORECAST    =                  95
VSCP_CLASS1_PHONE               =                  100
VSCP_CLASS1_LIN                 =                  101
VSCP_CLASS1_DISPLAY             =                  102
VSCP_CLASS1_RC5                 =                  110
VSCP_CLASS1_ONEWIRE             =                  200
VSCP_CLASS1_X10                 =                  201
VSCP_CLASS1_LON                 =                  202
VSCP_CLASS1_EIB                 =                  203
VSCP_CLASS1_SNAP                =                  204
VSCP_CLASS1_MUMIN               =                  205
VSCP_CLASS1_LOG                 =                  509
VSCP_CLASS1_LAB                 =                  510
VSCP_CLASS1_LOCAL               =                  511

# Level I classes on Level II
VSCP_CLASS2_LEVEL1_PROTOCOL     =                  512 + 0
VSCP_CLASS2_LEVEL1_ALARM        =                  512 + 1
VSCP_CLASS2_LEVEL1_SECURITY     =                  512 + 2
VSCP_CLASS2_LEVEL1_MEASUREMENT  =                  512 + 10
VSCP_CLASS2_LEVEL1_DATA         =                  512 + 15
VSCP_CLASS2_LEVEL1_INFORMATION  =                  512 + 20
VSCP_CLASS2_LEVEL1_CONTROL      =                  512 + 30
VSCP_CLASS2_LEVEL1_MULTIMEDIA   =                  512 + 40
VSCP_CLASS2_LEVEL1_AOL          =                  512 + 50
VSCP_CLASS1_LEVEL1_MEASUREMENT64       =           512 + 60
VSCP_CLASS1_LEVEL1_MEASUREZONE         =           512 + 65
VSCP_CLASS1_LEVEL1_WEATHER             =           512 + 90
VSCP_CLASS1_LEVEL1_WEATHER_FORECAST    =           512 + 95
VSCP_CLASS2_LEVEL1_PHONE        =                  512 + 100
VSCP_CLASS2_LEVEL1_LIN          =                  512 + 101
VSCP_CLASS2_LEVEL1_RC5          =                  512 + 110
VSCP_CLASS2_LEVEL1_ONEWIRE      =                  512 + 200
VSCP_CLASS2_LEVEL1_X10          =                  512 + 201
VSCP_CLASS2_LEVEL1_LON          =                  512 + 202
VSCP_CLASS2_LEVEL1_EIB          =                  512 + 203
VSCP_CLASS2_LEVEL1_SNAP         =                  512 + 204
VSCP_CLASS2_LEVEL1_MUMIN        =                  512 + 205
VSCP_CLASS2_LEVEL1_LOG          =                  512 + 509
VSCP_CLASS2_LEVEL1_LAB          =                  512 + 510
VSCP_CLASS2_LEVEL1_LOCAL        =                  512 + 511

# Level II classes
VSCP_CLASS2_PROTOCOL            =                  1024
VSCP_CLASS2_CONTROL             =                  1025
VSCP_CLASS2_INFORMATION         =                  1026
VSCP_CLASS2_TEXT2SPEECH         =                  1028
VSCP_CLASS2_CUSTOM              =                  1029
VSCP_CLASS2_DISPLAY             =                  1030
VSCP_CLASS2_MEASUREMENT_STR     =                  1040
VSCP_CLASS2_MEASUREMENT_FLOAT   =                  1060
VSCP_CLASS2_VSCPD               =                  65535

