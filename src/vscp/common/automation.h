// vscpautomation.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, the VSCP project
// <akhe@vscp.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
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

#include <vscpdatetime.h>

class CControlObject;

///////////////////////////////////////////////////////////////////////////////
// Class that holds VSCP automation
//

class CAutomation
{

  public:
    /// Constructor
    CAutomation(void);

    // Destructor
    virtual ~CAutomation(void);

    /*!
     * Initialize control object
     */
    void setControlObject(CControlObject *pCtrlObj) { m_pCtrlObj = pCtrlObj; };

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
    static void convert2HourMinute(double floatTime,
                                   int *pHours,
                                   int *pMinutes);

    /*!
        Do automation work
    */
    bool doWork(vscpEventEx *pEventEx);

    /// Setter for zone
    void setZone(uint8_t zone) { m_zone = zone; }

    /// getter for zone
    uint8_t getZone(void) { return m_zone; }

    /// Setter for subzone
    void setSubzone(uint8_t subzone) { m_subzone = subzone; };

    /// getter for subzone
    uint8_t getSubzone(void) { return m_subzone; };

    /// setter for timezone
    // void setTimezone( double tz ) { m_timezone = tz; };
    // double getTimeZone( void ) { return m_timezone; };

    /// setter for m_bSegmentControllerHeartbeat
    void enableSegmentControllerHeartbeat(bool bVal=true)
    {
        m_bSegmentControllerHeartbeat = bVal;
    };

    /// setter for m_bSegmentControllerHeartbeat
    void disableSegmentControllerHeartbeat(void)
    {
        m_bSegmentControllerHeartbeat = false;
    };

    bool isSegmentControllerHeartbeat(void)
    {
        return m_bSegmentControllerHeartbeat;
    };

    /// setter/getter for m_intervalSegmentControllerHeartbeat
    void setSegmentControllerHeartbeatInterval(long interval)
    {
        m_intervalSegmentControllerHeartbeat = interval;
    };

    long getSegmentControllerHeartbeatInterval(void)
    {
        return m_intervalSegmentControllerHeartbeat;
    };

    /// setter for m_bHeartBeatEvent
    void enableHeartbeatEvent(bool bVal=true) { m_bHeartBeatEvent = bVal; };

    /// setter for m_bHeartBeatEvent
    void disableHeartbeatEvent(void) { m_bHeartBeatEvent = false; };

    bool isHeartbeatEvent(void) { return m_bHeartBeatEvent; };

    /// setter for setIntervalHeartbeatEvent
    void setHeartbeatEventInterval(long interval)
    {
        m_intervalHeartBeat = interval;
    };
    long getHeartbeatEventInterval(void) { return m_intervalHeartBeat; };

    /// setter for m_bCapabilitiesEvent
    void enableCapabilitiesEvent(bool bVal=true) { m_bCapabilitiesEvent = bVal; };

    /// setter for m_bCapabilitiesEvent
    void disableCapabilitiesEvent(void) { m_bCapabilitiesEvent = false; };

    bool isCapabilitiesEvent(void) { return m_bCapabilitiesEvent; };

    /// setter for setIntervalCapabilitiesEvent
    void setCapabilitiesEventInterval(long interval)
    {
        m_intervalCapabilities = interval;
    };
    long getcapabilitiesEventInterval(void) { return m_intervalCapabilities; };

    // Setter/getter for automation enable/disable
    void enableAutomation(bool bVal = true) { m_bEnableAutomation = bVal; };
    void disableAutomation(void) { m_bEnableAutomation = false; };
    bool isAutomationEnabled(void) { return m_bEnableAutomation; };

    bool isSendSegmentControllerHeartbeat(void)
    {
        return m_bSegmentControllerHeartbeat;
    };
    long getIntervalSegmentControllerHeartbeat(void)
    {
        return m_intervalSegmentControllerHeartbeat;
    };
    vscpdatetime &getSegmentControllerHeartbeatSent(void)
    {
        return m_SegmentHeartbeat_sent;
    };

    bool isSendHeartbeat(void) { return m_bHeartBeatEvent; };
    long getIntervalHeartbeat(void) { return m_intervalHeartBeat; };
    vscpdatetime &getHeartbeatSent(void) { return m_Heartbeat_Level1_sent; };

    bool isSendCapabilities(void) { return m_bHeartBeatEvent; };
    long getIntervalCapabilities(void) { return m_intervalHeartBeat; };
    vscpdatetime &getCapabilitiesSent(void) { return m_Heartbeat_Level1_sent; };

  private:

    bool m_bEnableAutomation;

    /// Zone that automation server belongs to
    uint8_t m_zone;

    /// Sub zone that automation server belongs to
    uint8_t m_subzone;

    //////////////////////////////////////////////
    //       Segment controller heartbeat
    //////////////////////////////////////////////

    /*!
        Set to true for  periodic CLASS1.PROTOCOL, Type=1 (Segment Status
       Heartbeat) event to be sent from the daemon. The interval between events
       is set in seconds.
    */
    bool m_bSegmentControllerHeartbeat;

    /*!
        Seconds between Segment Status Heartbeat
    */
    long m_intervalSegmentControllerHeartbeat; // long because of .toLong

    // Time when segment controller heartbeat send
    vscpdatetime m_SegmentHeartbeat_sent;

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
    long m_intervalHeartBeat; // long because of .toLong

    // Time when heartbeat Level I sent
    vscpdatetime m_Heartbeat_Level1_sent;

    // Time when heartbeat Level II sent
    vscpdatetime m_Heartbeat_Level2_sent;

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
    long m_intervalCapabilities; // long because of .toLong

    // Time when capabilities event was last sent
    vscpdatetime m_Capabilities_sent;

    /*!
     * Pointer to control object
     */
    CControlObject *m_pCtrlObj;
};

#endif