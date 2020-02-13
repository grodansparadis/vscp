// vscpautomation.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2020 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice cat /sys/class/net/eth0/addressand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifdef __GNUG__
    //#pragma implementation
#endif

#define _POSIX

#include <string>
#include <algorithm>

#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include <vscp.h>
#include <vscphelper.h>
#include <vscp_class.h>
#include <vscp_type.h>
#include <controlobject.h>
#include <vscpdatetime.h>

#include "automation.h"

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

// Seconds for 24h
#define SPAN24      (24*3600)


///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CAutomation::CAutomation( void )
{
    m_pCtrlObj = NULL;

    m_bEnableAutomation = true;

    m_zone = 0;
    m_subzone = 0;

    m_Heartbeat_Level1_sent =  vscpdatetime::dateTimeZero();
    m_Heartbeat_Level2_sent = vscpdatetime::dateTimeZero();

    m_SegmentHeartbeat_sent =  vscpdatetime::dateTimeZero();

    m_Capabilities_sent = vscpdatetime::dateTimeZero();
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CAutomation::~CAutomation( void )
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// isDaylightSavingTime
//

int CAutomation::isDaylightSavingTime( void )
{
    time_t rawtime;
    struct tm *timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    return timeinfo->tm_isdst;
}

///////////////////////////////////////////////////////////////////////////////
// getTimeZoneDiffHours
//

int CAutomation::getTimeZoneDiffHours( void )
{
    time_t rawtime;
    struct tm *timeinfo;
    struct tm *timeinfo_gmt;
    int h1, h2;

    time ( &rawtime );
    timeinfo = localtime( &rawtime );
    h2 = timeinfo->tm_hour;
    if ( 0 == h2 ) h2 = 24;

    timeinfo_gmt = gmtime( &rawtime );
    h1 = timeinfo_gmt->tm_hour;

    return ( h2 - h1 );
}


///////////////////////////////////////////////////////////////////////////////
// convert2HourMinute
//
// Display decimal hours in hours and minutes
//

void CAutomation::convert2HourMinute( double floatTime, int *pHours, int *pMinutes )
{
    *pHours = ((int)floatTime) % 24;
    *pMinutes = ((int)((floatTime - (double)*pHours)*60 )) % 60;
};


///////////////////////////////////////////////////////////////////////////////
// doWork
//

bool CAutomation::doWork( vscpEventEx *pEventEx )
{
    std::string str;
    vscpdatetime now = vscpdatetime::Now();

    
    // Heartbeat Level I
    //xxTimeSpan HeartBeatLevel1Period( 0, 0, m_intervalHeartBeat );

    if ( m_bHeartBeatEvent &&
        ( vscpdatetime::diffSeconds(now,m_Heartbeat_Level1_sent ) >= m_intervalHeartBeat ) )  {
         //( ( vscpdatetime::Now() - m_Heartbeat_Level1_sent ) >= HeartBeatLevel1Period ) ) {

        m_Heartbeat_Level1_sent = vscpdatetime::Now();

        // Send VSCP_CLASS1_INFORMATION, Type=9/VSCP_TYPE_INFORMATION_NODE_HEARTBEAT
        pEventEx->obid = 0;         // IMPORTANT Must be set by caller before event is sent
        pEventEx->head = 0;
        pEventEx->timestamp = vscp_makeTimeStamp();
        vscp_setEventExToNow( pEventEx ); // Set time to current time
        pEventEx->vscp_class = VSCP_CLASS1_INFORMATION;
        pEventEx->vscp_type = VSCP_TYPE_INFORMATION_NODE_HEARTBEAT;
        pEventEx->sizeData = 3;
        // IMPORTANT - GUID must be set by caller before event is sent
        pEventEx->data[ 0 ] = 0;            // index
        pEventEx->data[ 1 ] = m_zone;       // zone
        pEventEx->data[ 2 ] = m_subzone;    // subzone

        return true;
    }

    // Heartbeat Level II
    //xxTimeSpan HeartBeatLevel2Period( 0, 0, m_intervalHeartBeat );
    if ( m_bHeartBeatEvent &&
    ( vscpdatetime::diffSeconds(now,m_Heartbeat_Level2_sent ) >= m_intervalHeartBeat ) )  {
    //     ( ( vscpdatetime::Now() - m_Heartbeat_Level2_sent ) >= HeartBeatLevel2Period ) ) {

        m_Heartbeat_Level2_sent = vscpdatetime::Now();

        // Send VSCP_CLASS1_INFORMATION, Type=9/VSCP_TYPE_INFORMATION_NODE_HEARTBEAT
        pEventEx->obid = 0;         // IMPORTANT Must be set by caller before event is sent
        pEventEx->head = 0;
        pEventEx->timestamp = vscp_makeTimeStamp();
        vscp_setEventExToNow( pEventEx ); // Set time to current time
        pEventEx->vscp_class = VSCP_CLASS2_INFORMATION;
        pEventEx->vscp_type = VSCP2_TYPE_INFORMATION_HEART_BEAT;
        pEventEx->sizeData = 64;
        // IMPORTANT - GUID must be set by caller before event is sent

        memset( pEventEx->data, 0, sizeof( pEventEx->data ) );
        memcpy( pEventEx->data,
                    gpobj->m_strServerName.c_str(),
                    strlen( gpobj->m_strServerName.c_str() ) );
        return true;
    }

    // Segment Controller Heartbeat
    //xxTimeSpan SegmentControllerHeartBeatPeriod( 0, 0, m_intervalSegmentControllerHeartbeat );
    if ( m_bSegmentControllerHeartbeat &&
    ( vscpdatetime::diffSeconds(now,m_SegmentHeartbeat_sent ) >= m_intervalSegmentControllerHeartbeat ) )  {
    //     ( ( vscpdatetime::Now() - m_SegmentHeartbeat_sent ) >= SegmentControllerHeartBeatPeriod ) ) {

        m_SegmentHeartbeat_sent = vscpdatetime::Now();

        // Send VSCP_CLASS1_PROTOCOL, Type=1/VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT
        pEventEx->obid = 0;     // IMPORTANT Must be set by caller before event is sent
        pEventEx->head = 0;
        pEventEx->timestamp = vscp_makeTimeStamp();
        vscp_setEventExToNow( pEventEx ); // Set time to current time
        pEventEx->vscp_class = VSCP_CLASS1_PROTOCOL;
        pEventEx->vscp_type = VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT;
        pEventEx->sizeData = 5;

        // IMPORTANT - GUID must be set by caller before event is sent

        time_t tnow;
        time( &tnow );
        uint32_t time32 = (uint32_t)tnow;

        pEventEx->data[ 0 ] = 0;  // 8 - bit crc for VSCP daemon GUID
        pEventEx->data[ 1 ] = (uint8_t)((time32>>24) & 0xff);    // Time since epoch MSB
        pEventEx->data[ 2 ] = (uint8_t)((time32>>16) & 0xff);
        pEventEx->data[ 3 ] = (uint8_t)((time32>>8)  & 0xff);
        pEventEx->data[ 4 ] = (uint8_t)((time32) & 0xff);        // Time since epoch LSB

        return true;

    }

    // High end server capabilities
    //xxTimeSpan CapabilitiesPeriod( 0, 0, m_intervalCapabilities );
    if ( m_bCapabilitiesEvent &&
    ( vscpdatetime::diffSeconds(now,m_Capabilities_sent ) >= m_intervalCapabilities ) )  {
    //     ( ( vscpdatetime::Now() - m_Capabilities_sent ) >= CapabilitiesPeriod ) ) {

        m_Capabilities_sent = vscpdatetime::Now();
        //
        // Send VSCP_CLASS2_PROTOCOL, Type=20/VSCP2_TYPE_PROTOCOL_HIGH_END_SERVER_CAPS
        pEventEx->obid = 0;     // IMPORTANT Must be set by caller before event is sent
        pEventEx->head = 0;
        pEventEx->timestamp = vscp_makeTimeStamp();
        vscp_setEventExToNow( pEventEx ); // Set time to current time
        pEventEx->vscp_class = VSCP_CLASS2_PROTOCOL;
        pEventEx->vscp_type = VSCP2_TYPE_PROTOCOL_HIGH_END_SERVER_CAPS;


        // IMPORTANT - GUID must be set by caller before event is sent


        // Fill in data
        memset( pEventEx->data, 0, sizeof( pEventEx->data ) );

        // GUID
        memcpy( pEventEx->data + VSCP_CAPABILITY_OFFSET_GUID,
                    m_pCtrlObj->m_guid.getGUID(), 16 );

        // Server ip address
        cguid guid;
        if ( m_pCtrlObj->getIPAddress( guid ) ) {
            pEventEx->data[ VSCP_CAPABILITY_OFFSET_IP_ADDR ] = guid.getAt(8);
            pEventEx->data[ VSCP_CAPABILITY_OFFSET_IP_ADDR + 1 ] = guid.getAt(9);
            pEventEx->data[ VSCP_CAPABILITY_OFFSET_IP_ADDR + 2 ] = guid.getAt(10);
            pEventEx->data[ VSCP_CAPABILITY_OFFSET_IP_ADDR + 3 ] = guid.getAt(11);
        }

        // Server name
        memcpy( pEventEx->data + VSCP_CAPABILITY_OFFSET_SRV_NAME,
                    (const char *)m_pCtrlObj->m_strServerName.c_str(),
                    std::min( 64, (int)m_pCtrlObj->m_strServerName.length() ) );

        // Capabilities array
        m_pCtrlObj->getVscpCapabilities( pEventEx->data );

        // non-standard ports
        // TODO

        pEventEx->sizeData = 104;

        return true;
    }

    return false;
}
