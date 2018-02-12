// vscpautomation.h
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
    void enableSunRiseEvent( bool bEnable = true ) { m_bSunRiseEvent = bEnable; };

    /// setter for bSunRiseEvent
    void disableSunRiseEvent( void ) { m_bSunRiseEvent = false; };

    
    /// setter for bSunSetEvent
    void enableSunSetEvent( bool bEnable = true ) { m_bSunSetEvent = bEnable; };

    /// setter for bSunSetEvent
    void disableSunSetEvent( void ) { m_bSunSetEvent = false; };


    /// setter for bSunRiseTwilightEven
    void enableSunRiseTwilightEvent( bool bEnable = true ) { m_bSunRiseTwilightEvent = bEnable; };

    /// setter for bSunRiseTwilightEvent
    void disableSunRiseTwilightEvent( void ) { m_bSunRiseTwilightEvent = false; };


    /// setter for bSunSetTwilightEvent
    void enableSunSetTwilightEvent( bool bEnable = true ) { m_bSunSetTwilightEvent = bEnable; };

    /// setter for bSunSetTwilightEvent
    void disableSunSetTwilightEvent( void ) { m_bSunSetTwilightEvent = false; };
    
    /// setter for longitude
    void setLongitude( double l ) { m_longitude = l; };

    /// setter for latitude
    void setLatitude( double l ) { m_latitude = l; };

    /// setter for timezone
    //void setTimezone( double tz ) { m_timezone = tz; };
    //double getTimeZone( void ) { return m_timezone; };

    /// setter for m_bSegmentControllerHeartbeat
    void enableSegmentControllerHeartbeat( void ) { m_bSegmentControllerHeartbeat = true; };

    /// setter for m_bSegmentControllerHeartbeat
    void disableSegmentControllerHeartbeat( void ) { m_bSegmentControllerHeartbeat = false; };
    
    bool isSegmentControllerHeartbeat( void ) { return m_bSegmentControllerHeartbeat; };

    /// setter/getter for m_intervalSegmentControllerHeartbeat
    void setSegmentControllerHeartbeatInterval( long interval ) 
                                        { m_intervalSegmentControllerHeartbeat = interval; };
                                        
    long getSegmentControllerHeartbeatInterval( void ) 
                                        { return m_intervalSegmentControllerHeartbeat; };

    /// setter for m_bHeartBeatEvent
    void enableHeartbeatEvent( void ) { m_bHeartBeatEvent = true; };

    /// setter for m_bHeartBeatEvent
    void disableHeartbeatEvent( void ) { m_bHeartBeatEvent = false; };
    
    bool isHeartbeatEvent( void ) { return m_bHeartBeatEvent; };

    /// setter for setIntervalHeartbeatEvent
    void setHeartbeatEventInterval( long interval ) 
                                        { m_intervalHeartBeat = interval; };
    long getHeartbeatEventInterval( void ) 
                                        { return m_intervalHeartBeat; };                                        

                                         
    /// setter for m_bCapabilitiesEvent
    void enableCapabilitiesEvent( void ) { m_bCapabilitiesEvent = true; };

    /// setter for m_bCapabilitiesEvent
    void disableCapabilitiesEvent( void ) { m_bCapabilitiesEvent = false; };
    
    bool isCapabilitiesEvent( void ) { return m_bCapabilitiesEvent; };

    /// setter for setIntervalCapabilitiesEvent
    void setCapabilitiesEventInterval( long interval ) 
                                        { m_intervalCapabilities = interval; };
    long getcapabilitiesEventInterval( void ) 
                                        { return m_intervalCapabilities; };                                       
                                        
                                        
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
    wxDateTime &getSunriseEventSent( void ) { return m_SunriseTime_sent; };
    
    bool isSendSunriseTwilightEvent( void ) { return m_bSunRiseTwilightEvent; };
    wxDateTime &getSunriseTwilightEventSent( void ) { return m_civilTwilightSunriseTime_sent; };
    
    bool isSendSunsetEvent( void ) { return m_bSunSetEvent; };
    wxDateTime &getSunsetEventSent( void ) { return m_SunsetTime_sent; };
    
    bool isSendSunsetTwilightEvent( void ) { return m_bSunSetTwilightEvent; }; 
    wxDateTime &getSunsetTwilightEventSent( void ) { return m_civilTwilightSunsetTime_sent; };
    
    bool isSendCalculatedNoonEvent( void ) { return m_bCalculatedNoonEvent; };
    wxDateTime &getCalculatedNoonEventSent( void ) { return m_noonTime_sent; };

    bool isSendSegmentControllerHeartbeat( void ) { return m_bSegmentControllerHeartbeat; };
    long getIntervalSegmentControllerHeartbeat( void ) { return m_intervalSegmentControllerHeartbeat; };
    wxDateTime &getSegmentControllerHeartbeatSent( void ) { return m_SegmentHeartbeat_sent; };

    bool  isSendHeartbeat( void ) { return m_bHeartBeatEvent; };
    long getIntervalHeartbeat( void ) { return m_intervalHeartBeat; };
    wxDateTime &getHeartbeatSent( void ) { return m_Heartbeat_Level1_sent; };
    
    bool  isSendCapabilities( void ) { return m_bHeartBeatEvent; };
    long getIntervalCapabilities( void ) { return m_intervalHeartBeat; };
    wxDateTime &getCapabilitiesSent( void ) { return m_Heartbeat_Level1_sent; };

private:

    bool m_bAutomation;

    /// Zone that automation server belongs to
    uint8_t m_zone;

    /// Sub zone that automation server belongs to
    uint8_t m_subzone;

    
    
    //////////////////////////////////////////////
    //       Segment controller heartbeat
    //////////////////////////////////////////////
    
    
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
    
    
    //////////////////////////////////////////////
    //               Heartbeat
    //////////////////////////////////////////////

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
    
    
    //////////////////////////////////////////////
    //             Capabilities
    //////////////////////////////////////////////
    
    /*!
        Enable/disable capabilities event.
    */
    bool m_bCapabilitiesEvent;

    /*!
        Interval in seconds for capabilities event
    */
    long m_intervalCapabilities;                   // long because of .toLong

    // Time when capabilities event was last sent
    wxDateTime m_Capabilities_sent;

    
    /*!
        Enable/disable the CLASS1.INFORMATION, Type=52 (Civil sunrise twilight time) to be sent. 
        Longitude, latitude and time zone must be set for this to work correctly.
    */
    bool m_bSunRiseEvent;

    /*!
        Enable/disable the CLASS1.INFORMATION, Type=52 (Civil sunrise twilight time) to be sent. 
        Longitude, latitude and time zone must be set for this to work correctly.
    */
    bool m_bSunRiseTwilightEvent;

    /*!
        Enable/disable the CLASS1.INFORMATION, Type=45 (Sunset) to be sent. 
        Longitude, latitude and time zone must be set for this to work correctly.
    */
    bool m_bSunSetEvent;

    /*!
        Enable/disable the CLASS1.INFORMATION, Type=53 (Civil sunset twilight time) 
        to be sent. Longitude, latitude and time zone must be set for this to work correctly.
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

    /// Longitude for this server
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