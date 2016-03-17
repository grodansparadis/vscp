// vscpautomation.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP project (http://www.vscp.org) 
//
// Copyright (C) 2000-2016 
// Ake Hedman, Paradise of the Frog, <akhe@paradiseofthefrog.com>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//



#if !defined(VSCPAUTOMATION__INCLUDED_)
#define VSCPAUTOMATION__INCLUDED_

class ControlObject;

///////////////////////////////////////////////////////////////////////////////
// Class that holds one VSCP vautomation
//

class CVSCPAutomation {

public:

    /// Constructor
    CVSCPAutomation(void);

    // Destructor
    virtual ~CVSCPAutomation(void);
    
    /*!
     * Initialize control object
     */
    void setControlObject( CControlObject *pCtrlObj ) { m_pCtrlObj = pCtrlObj; };

    /*!        
        \return Greater than zero if Daylight Saving Time is in effect, 
        zero if Daylight Saving Time is not in effect, and less than 
        zero if the information is not available.
    */
    int isDaylightSavingTime();

    /*!
        Get the hours our locale differs from GMT
    */
    int getTimeZoneDiffHours();

    /*!

    */
    static double FNday(int y, int m, int d, float h);


    /*!

    */
    static double FNrange(double x);

    /*!

    */
    static double f0(double lat, double declin);


    /*!

    */
    static double f1(double lat, double declin);

    /*!

    */
    static double FNsun(double d);


    /*!

    */
    static void convert2HourMinute( double floatTime, int *pHours, int *pMinutes );

    /*!
        Calculate Sunset/Sunrice etc   
    */
    void calcSun( void );

    /*!
        Do automation work
    */
    bool doWork( vscpEventEx *pEventEx  );

    /// Setter for zone
    void setZone( uint8_t zone ) { m_zone = zone; }

    /// getter for zone
    uint8_t getZone( void ) { return m_zone; }

    /// Setter for subzone
    void setSubzone( uint8_t subzone ) { m_subzone = subzone; };

    /// getter for subzone
    uint8_t getSubzone( void ) { return m_subzone; };

    /// setter for bSunRiseEvent
    void enableSunRiseEvent( void ) { m_bSunRiseEvent = true; };

    /// setter for bSunRiseEvent
    void disableSunRiseEvent( void ) { m_bSunRiseEvent = false; };

    
    /// setter for bSunSetEvent
    void enableSunSetEvent( void ) { m_bSunSetEvent = true; };

    /// setter for bSunSetEvent
    void disableSunSetEvent( void ) { m_bSunSetEvent = false; };


    /// setter for bSunRiseTwilightEven
    void enableSunRiseTwilightEvent( void ) { m_bSunRiseTwilightEvent = true; };

    /// setter for bSunRiseTwilightEvent
    void disableSunRiseTwilightEvent( void ) { m_bSunRiseTwilightEvent = false; };


    /// setter for bSunSetTwilightEvent
    void enableSunSetTwilightEvent( void ) { m_bSunSetTwilightEvent = true; };

    /// setter for bSunSetTwilightEvent
    void disableSunSetTwillightEvent( void ) { m_bSunSetTwilightEvent = false; };

    /// setter for daylightsavingtimeStart
    void setDaylightSavingStart( wxDateTime &dt ) { m_daylightsavingtimeStart = dt; };

    /// setter for daylightsavingtimeEnd
    void setDaylightSavingEnd( wxDateTime &dt ) { m_daylightsavingtimeEnd = dt; };

    /// setter for longitude
    void setLongitude( double l ) { m_longitude = l; };

    /// setter for latitude
    void setLatitude( double l ) { m_latitude = l; };

    /// setter for timezone
    void setTimezone( double tz ) { m_timezone = tz; };


    /// setter for m_bSegmentControllerHeartbeat
    void enableSegmentControllerHeartbeat( void ) { m_bSegmentControllerHeartbeat = true; };

    /// setter for m_bSegmentControllerHeartbeat
    void disableSegmentControllerHeartbeat( void ) { m_bSegmentControllerHeartbeat = false; };

    /// setter for m_intervalSegmentControllerHeartbeat
    void setIntervalSegmentControllerHeartbeat( long interval ) 
                                        { m_intervalSegmentControllerHeartbeat = interval; };

    /// setter for m_bHeartBeatEvent
    void enableHeartbeatEvent( void ) { m_bHeartBeatEvent = true; };

    /// setter for m_bHeartBeatEvent
    void disableHeartbeatEvent( void ) { m_bHeartBeatEvent = false; };

    /// setter for setIntervalHeartbeatEvent
    void setIntervalHeartbeatEvent( long interval ) 
                                        { m_intervalHeartBeat = interval; };

    // Setter/getter for automation enable/diable
    void enableAutomation( void ) { m_bAutomation = true; };
    void disableAutomation( void ) { m_bAutomation = false; };
    bool isAutomationEnabled( void ) { return m_bAutomation; }; 

    wxDateTime &getLastCalculation( void ) { return m_lastCalculation; };

    wxDateTime &getCivilTwilightSunriseTime( void ) { return m_civilTwilightSunriseTime; };
    wxDateTime &getCivilTwilightSunriseTimeSent( void ) { return m_civilTwilightSunriseTime_sent; };

    wxDateTime &getSunriseTime( void ) { return m_SunriseTime; };
    wxDateTime &getSunriseTimeSent( void ) { return m_SunriseTime_sent; };

    wxDateTime &getSunsetTime( void ) { return m_SunsetTime; };
    wxDateTime &getSunsetTimeSent( void ) { return m_SunsetTime_sent; };

    wxDateTime &getCivilTwilightSunsetTime( void ) { return m_civilTwilightSunsetTime; };
    wxDateTime &getCivilTwilightSunsetTimeSent( void ) { return m_civilTwilightSunsetTime_sent; };

    wxDateTime &getNoonTime( void ) { return m_noonTime; };
    wxDateTime &getNoonTimeSent( void ) { return m_noonTime_sent; };

    double getLongitude( void ) { return m_longitude; };
    double getLatitude( void ) { return m_latitude; };
    double getDayLength( void ) { return m_daylength; }; // use convert2HourMinute to hh:mm
    double getDeclination( void ) { return m_declination; };
    double getSunMaxAltitude( void ) { return m_SunMaxAltitude; };

    bool isSendSunriseEvent( void ) { return m_bSunRiseEvent; };
    bool isSendSunriseTwilightEvent( void ) { return m_bSunRiseTwilightEvent; };
    bool isSendSunsetEvent( void ) { return m_bSunSetEvent; };
    bool isSendSunsetTwilightEvent( void ) { return m_bSunSetTwilightEvent; }; 
    bool isSendCalculatedNoonEvent( void ) { return m_bCalculatedNoonEvent; };

    bool  isSendSegmentControllerHeartbeat( void ) { return m_bSegmentControllerHeartbeat; };
    long getIntervalSegmentControllerHeartbeat( void ) { return m_intervalSegmentControllerHeartbeat; };
    wxDateTime &getSegmentControllerHeartbeatSent( void ) { return m_SegmentHeartbeat_sent; };

    bool  isSendHeartbeat( void ) { return m_bHeartBeatEvent; };
    long getIntervalHeartbeat( void ) { return m_intervalHeartBeat; };
    wxDateTime &getHeartbeatSent( void ) { return m_Heartbeat_Level1_sent; };

private:

    bool m_bAutomation;

    /// Zone that automation server belongs to
    uint8_t m_zone;

    /// Subzone that automation server belongs to
    uint8_t m_subzone;

    /*!
        Timezone for this computer
        GMT = 0;
    */
    double m_timezone;

    /*!
        Set to true for  periodic CLASS1.PROTOCOL, Type=1 (Segment Status Heartbeat) 
        event to be sent from the daemon. The interval between events is set in seconds.
    */
    bool m_bSegmentControllerHeartbeat; 

    /*!
        Seconds between Segment Status Heartbeat
    */
    long m_intervalSegmentControllerHeartbeat;   // long because of .toLong

    // Time when segment controller heartbeat send
    wxDateTime m_SegmentHeartbeat_sent;

    /*!
        Enable/disable heartbeat.
    */
    bool m_bHeartBeatEvent;

    /*!
        Interval in seconds for heartbeats
    */
    long m_intervalHeartBeat;                   // long because of .toLong

    // Time when heartbeat Level I sent
    wxDateTime m_Heartbeat_Level1_sent;

    // Time when heartbeat Level II sent
    wxDateTime m_Heartbeat_Level2_sent;

    // Time when capabilities Level II sent
    wxDateTime m_Capabilities_Level2_sent;

    /*!
        Start date time when daylight saving time starts. When daylight saving time is in 
        effect the zone value will be increase with one for all calculations. Date/Time should 
        be on the form YY-MM-DD HH:MM:SS
    */
    wxDateTime m_daylightsavingtimeStart;

    /*!
        Start date time when daylight saving time starts. When daylight saving time is in 
        effect the zone value will be increase with one for all calculations. Date/Time should 
        be on the form YY-MM-DD HH:MM:SS
    */
    wxDateTime m_daylightsavingtimeEnd;

    /*!
        Enable/disable the CLASS1.INFORMATION, Type=52 (Civil sunrise twilight time) to be sent. 
        Longitude, latitude and timezone must be set for this to work correctly.
    */
    bool m_bSunRiseEvent;

    /*!
        Enable/disable the CLASS1.INFORMATION, Type=52 (Civil sunrise twilight time) to be sent. 
        Longitude, latitude and timezone must be set for this to work correctly.
    */
    bool m_bSunRiseTwilightEvent;

    /*!
        Enable/disable the CLASS1.INFORMATION, Type=45 (Sunset) to be sent. 
        Longitude, latitude and timezone must be set for this to work correctly.
    */
    bool m_bSunSetEvent;

    /*!
        Enable/disable the CLASS1.INFORMATION, Type=53 (Civil sunset twilight time) 
        to be sent. Longitude, latitude and timezone must be set for this to work correctly.
    */
    bool m_bSunSetTwilightEvent;

    /*!
        Enable/Disable calculated noon event
    */
    bool m_bCalculatedNoonEvent;


    /*!
        calculations holders.
        ---------------------
        Calculated values
        use convert2HourMinute to convert from
        double to hour/minutes
    */
    double m_declination;
    double m_daylength;               // hours/minutes
    double m_SunMaxAltitude;

    /// Logitude for this server
    double m_longitude;

    /// Latitude for this server
    double m_latitude;

    /*!
        Calculations done every 24 hours and at startup
    */
    wxDateTime m_lastCalculation;

    wxDateTime m_civilTwilightSunriseTime;
    wxDateTime m_civilTwilightSunriseTime_sent;

    wxDateTime m_SunriseTime;
    wxDateTime m_SunriseTime_sent;
    
    wxDateTime m_SunsetTime;
    wxDateTime m_SunsetTime_sent;
    
    wxDateTime m_civilTwilightSunsetTime;
    wxDateTime m_civilTwilightSunsetTime_sent;
    
    wxDateTime m_noonTime;
    wxDateTime m_noonTime_sent;

    /*!
        Set to true when calculations has been done and
        time is 12:00
    */
    bool m_bCalulationHasBeenDone;
    
    /*!
     * Pointer to control object
     */
    CControlObject *m_pCtrlObj;

};


#endif