// vscpautomation.cpp
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
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

#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/defs.h"
#include "wx/app.h"
#include <wx/datetime.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/stdpaths.h>

#include <wx/listimpl.cpp>

#include <vscp.h>
#include <vscphelper.h>
#include <vscp_class.h>
#include <vscp_type.h>

#include "controlobject.h"
#include "automation.h"

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

//-----------------------------------------------------------------------------
//                   Helpers for sunrise/sunset calculations
//-----------------------------------------------------------------------------

// C program calculating the sunrise and sunset for
// the current date and a fixed location(latitude,longitude)
// Note, twilight calculation gives insufficient accuracy of results
// Jarmo Lammi 1999 - 2001
// Last update July 21st, 2001

static double pi = 3.14159;
static double degs;
static double rads;

static double L, g, daylen;
static double SunDia = 0.53; // Sun radius degrees

static double AirRefr = 34.0 / 60.0; // atmospheric refraction degrees //


 

//-----------------------------------------------------------------------------
//                       End of sunset/sunrise functions
//-----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CVSCPAutomation::CVSCPAutomation( void )
{
    m_pCtrlObj = NULL;
    
    m_bAutomation = true;

    m_zone = 0;
    m_subzone = 0;

    // Take me the freedom to use my own place as reference
    m_longitude =  15.1604167;  // Home sweet home
    m_latitude = 61.7441833;

    m_bSegmentControllerHeartbeat = true;
    m_intervalSegmentControllerHeartbeat = 60;
    
    m_bCapabilitiesEvent = true;
    m_intervalCapabilities = 60;

    m_bHeartBeatEvent = true;
    m_intervalHeartBeat = 60;

    m_bSunRiseEvent = true;
    m_bSunRiseTwilightEvent = true;
    m_bSunSetEvent = true;
    m_bSunSetTwilightEvent = true;
    m_bCalculatedNoonEvent = true;

    m_declination = 0.0f;
    m_daylength = 0.0f;
    m_SunMaxAltitude = 0.0f;

    m_bCalulationHasBeenDone = false;   // No calculations has been done yet

    // Set to some early date to indicate that they have not been sent
    wxTimeSpan in_the_past(-8760);
    m_civilTwilightSunriseTime_sent  = wxDateTime::Now() + in_the_past;
    m_SunriseTime_sent = wxDateTime::Now() + in_the_past;
    m_SunsetTime_sent = wxDateTime::Now() + in_the_past;
    m_civilTwilightSunsetTime_sent  = wxDateTime::Now() + in_the_past;
    m_noonTime_sent = wxDateTime::Now() + in_the_past;

    m_lastCalculation = wxDateTime::Now();

    m_Heartbeat_Level1_sent =  wxDateTime::Now() + in_the_past;
    m_Heartbeat_Level2_sent = wxDateTime::Now() + in_the_past;
    
    m_SegmentHeartbeat_sent =  wxDateTime::Now() + in_the_past;
        
    m_Capabilities_sent = wxDateTime::Now() + in_the_past;
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVSCPAutomation::~CVSCPAutomation( void )
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// isDaylightSavingTime
//

int CVSCPAutomation::isDaylightSavingTime( void ) 
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

int CVSCPAutomation::getTimeZoneDiffHours( void )
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
// FNday
//
// Get the days to J2000
// h is UT in decimal hours
// FNday only works between 1901 to 2099 - see Meeus chapter 7
//

double CVSCPAutomation::FNday(int y, int m, int d, float h)
{
    long int luku = -7 * (y + (m + 9) / 12) / 4 + 275 * m / 9 + d;
    // type casting necessary on PC DOS and TClite to avoid overflow
    luku += (long int) y * 367;
    return(double) luku - 730531.5 + h / 24.0;
};


///////////////////////////////////////////////////////////////////////////////
// FNrange
//
// the function below returns an angle in the range
// 0 to 2*pi
//

double CVSCPAutomation::FNrange(double x)
{
    double b = 0.5 * x / pi;
    double a = 2.0 * pi * (b - (long) (b));
    if (a < 0) a = 2.0 * pi + a;
    return a;
};

///////////////////////////////////////////////////////////////////////////////
// f0
//
// Calculating the hourangle
//

double CVSCPAutomation::f0(double lat, double declin)
{
    double fo, dfo;
    // Correction: different sign at S HS
    dfo = rads * (0.5 * SunDia + AirRefr);
    if (lat < 0.0) dfo = -dfo;
    fo = tan(declin + dfo) * tan(lat * rads);
    if (fo > 0.99999) fo = 1.0; // to avoid overflow //
    fo = asin(fo) + pi / 2.0;
    return fo;
};

///////////////////////////////////////////////////////////////////////////////
// f1
//
// Calculating the hourangle for twilight times
//

double CVSCPAutomation::f1(double lat, double declin)
{
    double fi, df1;
    // Correction: different sign at S HS
    df1 = rads * 6.0;
    if (lat < 0.0) df1 = -df1;
    fi = tan(declin + df1) * tan(lat * rads);
    if (fi > 0.99999) fi = 1.0; // to avoid overflow //
    fi = asin(fi) + pi / 2.0;
    return fi;
};


///////////////////////////////////////////////////////////////////////////////
// FNsun
//
//   Find the ecliptic longitude of the Sun

double CVSCPAutomation::FNsun(double d)
{

    //   mean longitude of the Sun
    L = FNrange(280.461 * rads + .9856474 * rads * d);

    //   mean anomaly of the Sun
    g = FNrange(357.528 * rads + .9856003 * rads * d);

    //   Ecliptic longitude of the Sun
    return FNrange(L + 1.915 * rads * sin(g) + .02 * rads * sin(2 * g));
};


///////////////////////////////////////////////////////////////////////////////
// convert2HourMinute
//
// Display decimal hours in hours and minutes
//

void CVSCPAutomation::convert2HourMinute( double floatTime, int *pHours, int *pMinutes )
{
    *pHours = ((int)floatTime) % 24;
    *pMinutes = ((int)((floatTime - (double)*pHours)*60 )) % 60;
};


///////////////////////////////////////////////////////////////////////////////
// calcSun
//

void CVSCPAutomation::calcSun( void )
{
    double year, month, day, hour; 
    double d, lambda;
    double obliq, alpha, delta, LL, equation, ha, hb, twx;
    double twilightSunrise, maxAltitude, noonTime, sunsetTime, sunriseTime, twilightSunset;
    double tzone = 0;  
    
    degs = 180.0 / pi;
    rads = pi / 180.0;

    // get the date and time from the user
    // read system date and extract the year

    // First get time
    wxDateTime nowLocal = wxDateTime::Now();
    year = nowLocal.GetYear();
    month = nowLocal.GetMonth() + 1;
    day = nowLocal.GetDay();
    hour = nowLocal.GetHour();

    // Set offset from UTC - daytimesaving is taken care of
    wxDateTime::TimeZone ttt( wxDateTime::Local );
    tzone = ttt.GetOffset()/3600;

    // Adjust for daylight saving time
    /*if ( isDaylightSavingTime() ) {
        tzone = getTimeZoneDiffHours();
    }*/
    
    d = FNday(year, month, day, hour);

    // Use FNsun to find the ecliptic longitude of the Sun
    lambda = FNsun(d);

    //   Obliquity of the ecliptic
    obliq = 23.439 * rads - 0.0000004 * rads * d;

    //   Find the RA and DEC of the Sun
    alpha = atan2(cos(obliq) * sin(lambda), cos(lambda));
    delta = asin(sin(obliq) * sin(lambda));

    // Find the Equation of Time
    // in minutes
    // Correction suggested by David Smith
    LL = L - alpha;
    if (L < pi) LL += 2.0 * pi;
    equation = 1440.0 * (1.0 - LL / pi / 2.0);
    ha = f0(m_latitude, delta);
    hb = f1(m_latitude, delta);
    twx = hb - ha;          // length of twilight in radians
    twx = 12.0 * twx / pi;  // length of twilight in hours

    // Conversion of angle to hours and minutes
    daylen = degs * ha / 7.5;
    if (daylen < 0.0001) {
        daylen = 0.0;
    }

    // arctic winter 
    sunriseTime = 12.0 - 12.0 * ha / pi + tzone - m_longitude / 15.0 + equation / 60.0;
    sunsetTime = 12.0 + 12.0 * ha / pi + tzone - m_longitude / 15.0 + equation / 60.0;
    noonTime = sunriseTime + 12.0 * ha / pi;
    maxAltitude = 90.0 + delta * degs - m_latitude;
    // Correction for S HS suggested by David Smith
    // to express altitude as degrees from the N horizon
    if ( m_latitude < delta * degs ) maxAltitude = 180.0 - maxAltitude;

    twilightSunrise = sunriseTime - twx;    // morning twilight begin
    twilightSunset = sunsetTime + twx;      // evening twilight end

    if (sunriseTime > 24.0) sunriseTime -= 24.0;
    if (sunsetTime > 24.0) sunsetTime -= 24.0;
    if (twilightSunrise > 24.0) twilightSunrise -= 24.0;    // 160921
    if (twilightSunset > 24.0) twilightSunset -= 24.0;

    m_declination = delta * degs;
    m_daylength = daylen;
    m_SunMaxAltitude = maxAltitude;

    // Set last calculated time
    m_lastCalculation = wxDateTime::Now();

    int intHour, intMinute;

    // Civil Twilight Sunrise
    convert2HourMinute( twilightSunrise, &intHour, &intMinute );
    m_civilTwilightSunriseTime = wxDateTime::Now();
    m_civilTwilightSunriseTime.ResetTime();     // Set to midnight
    m_civilTwilightSunriseTime.SetHour( intHour );
    m_civilTwilightSunriseTime.SetMinute( intMinute ); 

    // Sunrise
    convert2HourMinute( sunriseTime, &intHour, &intMinute );
    m_SunriseTime = wxDateTime::Now();
    m_SunriseTime.ResetTime();     // Set to midnight
    m_SunriseTime.SetHour( intHour );
    m_SunriseTime.SetMinute( intMinute ); 

    // Sunset
    convert2HourMinute( sunsetTime, &intHour, &intMinute );
    m_SunsetTime = wxDateTime::Now();
    m_SunsetTime.ResetTime();     // Set to midnight
    m_SunsetTime.SetHour( intHour );
    m_SunsetTime.SetMinute( intMinute ); 

    // Civil Twilight Sunset
    convert2HourMinute( twilightSunset, &intHour, &intMinute );
    m_civilTwilightSunsetTime = wxDateTime::Now();
    m_civilTwilightSunsetTime.ResetTime();     // Set to midnight
    m_civilTwilightSunsetTime.SetHour( intHour );
    m_civilTwilightSunsetTime.SetMinute( intMinute );

    // NoonTime
    convert2HourMinute( noonTime, &intHour, &intMinute );
    m_noonTime = wxDateTime::Now();
    m_noonTime.ResetTime();     // Set to midnight
    m_noonTime.SetHour( intHour );
    m_noonTime.SetMinute( intMinute );
}


///////////////////////////////////////////////////////////////////////////////
// doWork
//

bool CVSCPAutomation::doWork( vscpEventEx *pEventEx )
{
    wxString wxstr;
    wxDateTime now = wxDateTime::Now();
    wxTimeSpan span24( 24 );  // Twentyfour hour span

    // Calculate Sunrise/sunset parameters once a day
    if ( !m_bCalulationHasBeenDone && 
            ( 0 == wxDateTime::Now().GetHour() ) ) {
                
        calcSun();     
        m_bCalulationHasBeenDone = true;
        
        int hours, minutes;        
        m_pCtrlObj->m_automation.convert2HourMinute( m_pCtrlObj->m_automation.getDayLength(),
                                                        &hours, 
                                                        &minutes );

        // Send VSCP_CLASS2_VSCPD, Type=30/VSCP2_TYPE_VSCPD_NEW_CALCULATION
        pEventEx->obid = 0;     // IMPORTANT Must be set by caller before event is sent
        pEventEx->head = 0;
        pEventEx->timestamp = vscp_makeTimeStamp();
        vscp_setEventExToNow( pEventEx ); // Set time to current time
        pEventEx->vscp_class = VSCP_CLASS2_VSCPD;
        pEventEx->vscp_type = VSCP2_TYPE_VSCPD_NEW_CALCULATION;
        pEventEx->sizeData = 0;
        
        // IMPORTANT - GUID must be set by caller before event is sent
        
        return true;
                
    }
    
    // Trigger for next noon calculation
    if ( 0 != wxDateTime::Now().GetHour() ) {
        m_bCalulationHasBeenDone = false;
    }

    // Sunrise Time
    if ( ( now.GetYear() == m_SunriseTime.GetYear() ) && 
         ( now.GetMonth() == m_SunriseTime.GetMonth() ) &&
         ( now.GetDay() == m_SunriseTime.GetDay() ) &&
         ( now.GetHour() == m_SunriseTime.GetHour() ) && 
         ( now.GetMinute() == m_SunriseTime.GetMinute() ) ) {

        m_SunriseTime += span24;   // Add 24h's
        m_SunriseTime_sent = wxDateTime::Now();                              

        // Send VSCP_CLASS1_INFORMATION, Type=44/VSCP_TYPE_INFORMATION_SUNRISE
        pEventEx->obid = 0;     // IMPORTANT Must be set by caller before event is sent
        pEventEx->head = 0;
        pEventEx->timestamp = vscp_makeTimeStamp();
        vscp_setEventExToNow( pEventEx ); // Set time to current time
        pEventEx->vscp_class = VSCP_CLASS1_INFORMATION;
        pEventEx->vscp_type = VSCP_TYPE_INFORMATION_SUNRISE;
        pEventEx->sizeData = 3;
        // IMPORTANT - GUID must be set by caller before event is sent
        pEventEx->data[ 0 ] = 0;            // index
        pEventEx->data[ 1 ] = m_zone;       // zone
        pEventEx->data[ 2 ] = m_subzone;    // subzone

        return true;
    }

    // Civil Twilight Sunrise Time
    if ( ( now.GetYear() == m_civilTwilightSunriseTime.GetYear() ) && 
         ( now.GetMonth() == m_civilTwilightSunriseTime.GetMonth() ) &&
         ( now.GetDay() == m_civilTwilightSunriseTime.GetDay() ) && 
         ( now.GetHour() == m_civilTwilightSunriseTime.GetHour() ) && 
         ( now.GetMinute() == m_civilTwilightSunriseTime.GetMinute() ) ) {

        m_civilTwilightSunriseTime += span24;   // Add 24h's
        m_civilTwilightSunriseTime_sent = wxDateTime::Now();

        // Send VSCP_CLASS1_INFORMATION, Type=52/VSCP_TYPE_INFORMATION_SUNRISE_TWILIGHT_START
        pEventEx->obid = 0;     // IMPORTANT Must be set by caller before event is sent
        pEventEx->head = 0;
        pEventEx->timestamp = vscp_makeTimeStamp();
        vscp_setEventExToNow( pEventEx ); // Set time to current time
        pEventEx->vscp_class = VSCP_CLASS1_INFORMATION;
        pEventEx->vscp_type = VSCP_TYPE_INFORMATION_SUNRISE_TWILIGHT_START;
        pEventEx->sizeData = 3;
        // IMPORTANT - GUID must be set by caller before event is sent
        pEventEx->data[ 0 ] = 0;            // index
        pEventEx->data[ 1 ] = m_zone;       // zone
        pEventEx->data[ 2 ] = m_subzone;    // subzone

        return true;
    }

    // Sunset Time
    if ( ( now.GetYear() == m_SunsetTime.GetYear() ) &&
         ( now.GetMonth() == m_SunsetTime.GetMonth() ) &&
         ( now.GetDay() == m_SunsetTime.GetDay() ) &&
         ( now.GetHour() == m_SunsetTime.GetHour() ) &&
         ( now.GetMinute() == m_SunsetTime.GetMinute() ) ) {

        m_SunsetTime += span24;     // Add 24h's
        m_SunsetTime_sent = wxDateTime::Now(); 

        // Send VSCP_CLASS1_INFORMATION, Type=45/VSCP_TYPE_INFORMATION_SUNSET
        pEventEx->obid = 0;         // IMPORTANT Must be set by caller before event is sent
        pEventEx->head = 0;
        pEventEx->timestamp = vscp_makeTimeStamp();
        vscp_setEventExToNow( pEventEx ); // Set time to current time
        pEventEx->vscp_class = VSCP_CLASS1_INFORMATION;
        pEventEx->vscp_type = VSCP_TYPE_INFORMATION_SUNSET;
        pEventEx->sizeData = 3;
        // IMPORTANT - GUID must be set by caller before event is sent
        pEventEx->data[ 0 ] = 0;            // index
        pEventEx->data[ 1 ] = m_zone;       // zone
        pEventEx->data[ 2 ] = m_subzone;    // subzone

        return true;
    }

    // Civil Twilight Sunset Time
    if ( ( now.GetYear() == m_civilTwilightSunsetTime.GetYear() ) && 
         ( now.GetMonth() == m_civilTwilightSunsetTime.GetMonth() ) &&
         ( now.GetDay() == m_civilTwilightSunsetTime.GetDay() ) &&
         ( now.GetHour() == m_civilTwilightSunsetTime.GetHour() ) && 
         ( now.GetMinute() == m_civilTwilightSunsetTime.GetMinute() ) ) {

        m_civilTwilightSunsetTime += span24;   // Add 24h's
        m_civilTwilightSunsetTime_sent = wxDateTime::Now();
                                            
        // Send VSCP_CLASS1_INFORMATION, Type=53/VSCP_TYPE_INFORMATION_SUNSET_TWILIGHT_START
        pEventEx->obid = 0;     // IMPORTANT Must be set by caller before event is sent
        pEventEx->head = 0;
        pEventEx->timestamp = vscp_makeTimeStamp();
        vscp_setEventExToNow( pEventEx ); // Set time to current time
        pEventEx->vscp_class = VSCP_CLASS1_INFORMATION;
        pEventEx->vscp_type = VSCP_TYPE_INFORMATION_SUNSET_TWILIGHT_START;
        pEventEx->sizeData = 3;
        // IMPORTANT - GUID must be set by caller before event is sent
        pEventEx->data[ 0 ] = 0;            // index
        pEventEx->data[ 1 ] = m_zone;       // zone
        pEventEx->data[ 2 ] = m_subzone;    // subzone

        return true;
    }

    // Noon Time
    if ( ( now.GetYear() == m_noonTime.GetYear() ) && 
         ( now.GetMonth() == m_noonTime.GetMonth() ) &&
         ( now.GetDay() == m_noonTime.GetDay() ) &&
         ( now.GetHour() == m_noonTime.GetHour() ) && 
         ( now.GetMinute() == m_noonTime.GetMinute() ) ) {

        m_noonTime += span24;   // Add 24h's
        m_noonTime_sent = wxDateTime::Now();

        // Send VSCP_CLASS1_INFORMATION, Type=58/VSCP_TYPE_INFORMATION_CALCULATED_NOON
        pEventEx->obid = 0;         // IMPORTANT Must be set by caller before event is sent
        pEventEx->head = 0;
        pEventEx->timestamp = vscp_makeTimeStamp();
        vscp_setEventExToNow( pEventEx ); // Set time to current time
        pEventEx->vscp_class = VSCP_CLASS1_INFORMATION;
        pEventEx->vscp_type = VSCP_TYPE_INFORMATION_CALCULATED_NOON;
        pEventEx->sizeData = 3;
        // IMPORTANT - GUID must be set by caller before event is sent
        pEventEx->data[ 0 ] = 0;            // index
        pEventEx->data[ 1 ] = m_zone;       // zone
        pEventEx->data[ 2 ] = m_subzone;    // subzone

        return true;
    }

    // Heartbeat Level I
    wxTimeSpan HeartBeatLevel1Period( 0, 0, m_intervalHeartBeat );
    if ( m_bHeartBeatEvent && 
         ( ( wxDateTime::Now() - m_Heartbeat_Level1_sent ) >= HeartBeatLevel1Period ) ) {
        
        m_Heartbeat_Level1_sent = wxDateTime::Now();

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
    wxTimeSpan HeartBeatLevel2Period( 0, 0, m_intervalHeartBeat );
    if ( m_bHeartBeatEvent &&
         ( ( wxDateTime::Now() - m_Heartbeat_Level2_sent ) >= HeartBeatLevel2Period ) ) {

        m_Heartbeat_Level2_sent = wxDateTime::Now();

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
                    gpobj->m_strServerName.mbc_str(), 
                    strlen( gpobj->m_strServerName.mbc_str() ) );         
        return true;
    }

    // Segment Controller Heartbeat
    wxTimeSpan SegmentControllerHeartBeatPeriod( 0, 0, m_intervalSegmentControllerHeartbeat );
    if ( m_bSegmentControllerHeartbeat && 
         ( ( wxDateTime::Now() - m_SegmentHeartbeat_sent ) >= SegmentControllerHeartBeatPeriod ) ) {

        m_SegmentHeartbeat_sent = wxDateTime::Now();

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
    wxTimeSpan CapabilitiesPeriod( 0, 0, m_intervalCapabilities );
    if ( m_bCapabilitiesEvent && 
         ( ( wxDateTime::Now() - m_Capabilities_sent ) >= CapabilitiesPeriod ) ) {

        m_Capabilities_sent = wxDateTime::Now();
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
                    (const char *)m_pCtrlObj->m_strServerName.mbc_str(),
                    MIN( 64, strlen((const char *)m_pCtrlObj->m_strServerName.mbc_str()) ) );
        
        // Capabilities array
        m_pCtrlObj->getVscpCapabilities( pEventEx->data );
        
        // non-standard ports
        // TODO
        
        pEventEx->sizeData = 104;

        return true;
    }
    
    return false;
}
