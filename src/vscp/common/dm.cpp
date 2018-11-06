// dm.cpp
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

#ifdef WIN32
#include <winsock2.h>
#endif

//#include "wx/wxprec.h"
#include <wx/wx.h>
#include <wx/defs.h>
#include <wx/app.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/tokenzr.h>
#include <wx/stdpaths.h>
#include <wx/thread.h>
#include <wx/socket.h>
#include <wx/url.h>
#include <wx/datetime.h>
#include <wx/filename.h>
#include <wx/cmdline.h>

#include <stdlib.h>
#include <string.h>
#include <float.h>

#include <vscp.h>
#include <vscpdb.h>
#include <version.h>
#include <vscp_debug.h>
#include <vscphelper.h>
#include <actioncodes.h>
#include <remotevariable.h>
#include <userlist.h>
#include <controlobject.h>
#include <vscpremotetcpif.h>
#include <duktape_vscp_wrkthread.h>
#include <lua_vscp_wrkthread.h>
#include <duktape.h>
#include <duktape_vscp_func.h>
#include <duk_module_node.h>
#include <dm.h>


WX_DEFINE_LIST( PLUGINLIST );
WX_DEFINE_LIST( DMLIST );
WX_DEFINE_LIST( ACTIONTIME );

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;



///////////////////////////////////////////////////////////////////////////////
// Constructor dmTimer
//

dmTimer::dmTimer()
{
    m_pThread = NULL;       // No working thread

    m_id = 0;
    m_bActive = false;
    m_delayStart = 0;
    m_delay = 0;
    m_bReload = false;
    m_nameVariable = _("");
    m_reloadlimit = -1;
    m_reloadCounter = -1;
}

dmTimer::dmTimer( wxString& nameVar, 
                    uint32_t id, 
                    uint32_t delay, 
                    bool bStart, 
                    bool bReload,
                    int reloadLimit )
{
    init( nameVar, 
            id, 
            delay, 
            bStart, 
            bReload,
            reloadLimit ); 
}

///////////////////////////////////////////////////////////////////////////////
// Destructor dmTimer
//

dmTimer::~dmTimer()
{

}

///////////////////////////////////////////////////////////////////////////////
// init
//

void dmTimer::init( wxString& nameVar, 
                        uint32_t id, 
                        uint32_t delay, 
                        bool bStart, 
                        bool bReload,
                        int reloadLimit )
{
    m_bPaused = false;
    m_pThread = NULL;       // No working thread

    m_id = id;
    m_bActive = bStart;
    m_delayStart = m_delay = delay;
    m_bReload = bReload;
    m_nameVariable = nameVar;
    m_reloadCounter = m_reloadlimit = reloadLimit;
}

///////////////////////////////////////////////////////////////////////////////
// startTimer
//

void dmTimer::startTimer( void ) 
{ 
    reload(); 
    resumeTimer(); 
};

///////////////////////////////////////////////////////////////////////////////
// stopTimer
//

void dmTimer::stopTimer( void ) 
{ 
    m_bActive = false; 
    m_bPaused = false; 
};


///////////////////////////////////////////////////////////////////////////////
// pauseTimer
//

void dmTimer::pauseTimer( void ) 
{ 
    m_bActive = false; 
    m_bPaused = true; 
};


///////////////////////////////////////////////////////////////////////////////
// resumeTimer
//

void dmTimer::resumeTimer( void ) { 
    if ( m_delay ) {
        m_bActive = true; 
        m_bPaused = false; 
        m_reloadCounter = m_reloadlimit;
    }; 
};

///////////////////////////////////////////////////////////////////////////////
// isActive
//

bool dmTimer::isActive( void ) 
{ 
    return m_bActive; 
};

///////////////////////////////////////////////////////////////////////////////
// setActive
//

void dmTimer::setActive( bool bState ) 
{ 
    m_bActive = bState; 
};


///////////////////////////////////////////////////////////////////////////////
// decTimer
//

uint32_t dmTimer::decTimer( void ) 
{ 
    if ( m_delay ) m_delay--;      
    return m_delay; 
};





//-----------------------------------------------------------------------------





///////////////////////////////////////////////////////////////////////////////
// Constructor  actionTime
//

actionTime::actionTime()
{
    allowAlways();
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

actionTime::~actionTime()
{
    clearTables();
}

///////////////////////////////////////////////////////////////////////////////
// clearTables
//

void actionTime::clearTables()
{
    ACTIONTIME::iterator iter;
    for (iter = m_actionYear.begin(); iter != m_actionYear.end(); ++iter) {
        if ( NULL != *iter ) {
            delete *iter;
        }
    }

    m_actionYear.Clear();

    for (iter = m_actionMonth.begin(); iter != m_actionMonth.end(); ++iter) {
        if ( NULL != *iter ) {
            delete *iter;
        }
    }

    m_actionMonth.Clear();

    for (iter = m_actionDay.begin(); iter != m_actionDay.end(); ++iter) {
        if ( NULL != *iter ) {
            delete *iter;
        }
    }

    m_actionDay.Clear();

    for (iter = m_actionHour.begin(); iter != m_actionHour.end(); ++iter) {
        if ( NULL != *iter ) {
            delete *iter;
        }
    }

    m_actionHour.Clear();

    for (iter = m_actionMinute.begin(); iter != m_actionMinute.end(); ++iter) {
        if ( NULL != *iter ) {
            delete *iter;
        }
    }

    m_actionMinute.Clear();

    for (iter = m_actionSecond.begin(); iter != m_actionSecond.end(); ++iter) {
        if ( NULL != *iter ) {
            delete *iter;
        }
    }

    m_actionSecond.Clear();
}

///////////////////////////////////////////////////////////////////////////////
// allowAlways
//

void actionTime::allowAlways()
{
    clearTables();
    
    for ( int i=0; i<7; i++ ) {
        m_weekDay[ i ] = true;  // Allow for all weekdays
    }

    // Allow from the beginning of time
    setFromTime( _("*") );

    // to the end of time
    setEndTime( _("*") );    
    
    
    parseActionTime( _("*-*-* *:*:*") );
}

///////////////////////////////////////////////////////////////////////////////
// setWeekDays
//

bool actionTime::setWeekDays( const wxString& strWeek )
{
    wxString str = strWeek;

    // Trim it
    str.Trim();
    str.Trim( false );

    if ( 7 != str.Length() ) return false;

    for ( int i=0; i<7; i++ ) {
        if ( '-' == str[ i ] ) {
            m_weekDay[ i ] = false;
        }
        else {
            m_weekDay[ i ] = true;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getWeekDays
//

wxString actionTime::getWeekDays( void )
{
    wxString strWeekDays;

    for ( int i=0; i<7; i++ ) {

        if ( m_weekDay[ i ] ) {

            switch ( i ) {
            case 0:
                strWeekDays += 'm';
                break;
            case 1:
                strWeekDays += 't';
                break;
            case 2:
                strWeekDays += 'w';
                break;
            case 3:
                strWeekDays += 't';
                break;
            case 4:
                strWeekDays += 'f';
                break;
            case 5:
                strWeekDays += 's';
                break;
            case 6:
                strWeekDays += 's';
                break;
            }

        }
        else {
            strWeekDays += '-';
        }
    }

    return strWeekDays;
}

///////////////////////////////////////////////////////////////////////////////
// setWeekDay
//

bool actionTime::setWeekDay( const wxString& strWeekday )
{
    wxString str = strWeekday;

    // Trim it
    str.Trim();
    str.Trim( false );

    str = str.Upper();

    if ( str.Cmp(_("MONDAY")) ) {
        m_weekDay[ 0 ] = true;
    }
    else if ( str.Cmp(_("TUESDAY")) ) {
        m_weekDay[ 1 ] = true;
    }
    else if ( str.Cmp(_("WEDNESDAY")) ) {
        m_weekDay[ 2 ] = true;
    }
    else if ( str.Cmp(_("THURSDAY")) ) {
        m_weekDay[ 3 ] = true;
    }
    else if ( str.Cmp(_("FRIDAY")) ) {
        m_weekDay[ 4 ] = true;
    }
    else if ( str.Cmp(_("SATURDAY")) ) {
        m_weekDay[ 5 ] = true;
    }
    else if ( str.Cmp(_("SUNDAY")) ) {
        m_weekDay[ 6 ] = true;
    }
    else {
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// getMultiItem
//

bool actionTime::getMultiItem( const wxString& items, ACTIONTIME *pList )
{
    // Check pointer
    if ( NULL == pList ) return false;

    // Clear list
    pList->Clear();

    wxStringTokenizer tkz( items, _("/" ) );
    
    if ( tkz.CountTokens() > 1 ) {

        wxString token;
        while ( tkz.CountTokens() ) {
            
            token = tkz.GetNextToken();
            token.Trim();
            token.Trim(false);
           
            // 'n' or 'n-m'  TODO add @2 == every second minute
            wxStringTokenizer tkzRange( items, _("-" ) );
            if ( tkzRange.CountTokens() > 1 ) {
                int from = vscp_readStringValue( tkzRange.GetNextToken() );
                int to = vscp_readStringValue( tkzRange.GetNextToken() );
                if ( from < to ) {
                    for ( int i=from; i<to; i++ ) {
                        int *pInt = new int;
                        if ( NULL != pInt ) {
                            *pInt = i;
                            pList->Append( pInt );
                        }
                    }
                }
            }
            else {
                int *pInt = new int;
                if ( NULL != pInt ) {
                    *pInt = (int)vscp_readStringValue( token );
                    pList->Append( pInt );
                }
            }
            
        }
    }
    else {
        // One Token ( or none )
        // If '*' we do nothing (same for none) and
        // list will be have count == 0 => No care.
        unsigned long val;
        wxString token = tkz.GetNextToken();

        if ( token.ToULong( &val ) ) {
            int *pInt = new int;
            if ( NULL != pInt ) {
                *pInt = (int)val;
                pList->Append( pInt );
            }
        }
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// parseActionTime
//

bool actionTime::parseActionTime( const wxString& actionTime )
{
    wxString buf = actionTime;
    wxString strDate;
    wxString strYear;
    wxString strMonth;
    wxString strDay;

    wxString strTime;
    wxString strHour;
    wxString strMinute;
    wxString strSecond;

    buf.Trim();
    buf.Trim(false);
    
    // '*' is 'always
    if ( '*' == buf ) {
        buf = _("*-*-* *:*:*");
    }

    // Formats:
    // YYYY-MM-SS HH:MM:SS
    // * *
    // *-*-* *:*:*
    // YYYY-0/1/4/5-DD HH:MM:SS or variants of it
    // n-m  . n, n+1, n+2,...
    wxStringTokenizer tkzFull( buf, _(" ") );
    if ( tkzFull.CountTokens() < 2 ) return false;	// Wrong format

    // Get date
    strDate = tkzFull.GetNextToken();
    strDate.Trim();
    strDate.Trim( false );

    // Get Time
    strTime = tkzFull.GetNextToken();
    strTime.Trim();
    strTime.Trim( false );

    // Date
    if ( !strDate.IsSameAs( _("*") ) ) {

        // Work on date separator is '-'
        wxStringTokenizer tkzDate( strDate, _("-" ) );
        if ( tkzDate.CountTokens() < 3 ) return false;	// Wrong format

        // Get year
        strYear = tkzDate.GetNextToken();
        strYear.Trim();
        strYear.Trim( false );
        getMultiItem( strYear, &m_actionYear );

        // Get month
        strMonth = tkzDate.GetNextToken();
        strMonth.Trim();
        strMonth.Trim( false );
        getMultiItem( strMonth, &m_actionMonth );

        // Get day
        strDay = tkzDate.GetNextToken();
        strDay.Trim();
        strDay.Trim( false );
        getMultiItem( strDay, &m_actionDay );

    }
    else {
        ACTIONTIME::iterator iter;
        for (iter = m_actionYear.begin(); iter != m_actionYear.end(); ++iter) {
            if ( NULL != *iter ) {
                delete *iter;
            }
        }

        m_actionYear.Clear();

        for (iter = m_actionMonth.begin(); iter != m_actionMonth.end(); ++iter) {
            if ( NULL != *iter ) {
                delete *iter;
            }
        }

        m_actionMonth.Clear();

        for (iter = m_actionDay.begin(); iter != m_actionDay.end(); ++iter) {
            if ( NULL != *iter ) {
                delete *iter;
            }
        }

        m_actionDay.Clear();

    }

    // Time
    if ( !strTime.IsSameAs( _("*") ) ) {

        // Work on date separator is ':'
        wxStringTokenizer tkzTime( strTime, _(":" ) );
        //if ( tkzTime.CountTokens() < 3 ) return false;	// Wrong format

        // Get hour
        strHour = tkzTime.GetNextToken();
        strHour.Trim();
        strHour.Trim( false );
        getMultiItem( strHour, &m_actionHour );

        // Get minute
        strMinute = tkzTime.GetNextToken();
        strMinute.Trim();
        strMinute.Trim( false );
        getMultiItem( strMinute, &m_actionMinute );

        // Get second
        strSecond = tkzTime.GetNextToken();
        strSecond.Trim();
        strSecond.Trim( false );
        getMultiItem( strSecond, &m_actionSecond );

    }
    else {
        ACTIONTIME::iterator iter;
        for (iter = m_actionHour.begin(); iter != m_actionHour.end(); ++iter) {
            if ( NULL != *iter ) {
                delete *iter;
            }
        }

        m_actionHour.Clear();

        for (iter = m_actionMinute.begin(); iter != m_actionMinute.end(); ++iter) {
            if ( NULL != *iter ) {
                delete *iter;
            }
        }

        m_actionMinute.Clear();

        for (iter = m_actionSecond.begin(); iter != m_actionSecond.end(); ++iter) {
            if ( NULL != *iter ) {
                delete *iter;
            }
        }

        m_actionSecond.Clear();
    }

    return true;

}





///////////////////////////////////////////////////////////////////////////////
// ShouldWeDoAction
//

bool actionTime::ShouldWeDoAction( void )
{
    bool bMatch;
    wxDateTime now( wxDateTime::Now() );	// Get current date/time
    
    // Check that times are valid
    if ( !m_fromTime.IsValid() ) return false;
    if ( !m_endTime.IsValid() ) return false;

    // for debug
    //wxString s1 = m_fromTime.FormatISODate() + _(" ") + m_fromTime.FormatISOTime();
    //wxString s2 = m_endTime.FormatISODate() + _(" ") + m_endTime.FormatISOTime();
    //wxString s3 = now.FormatISODate() + _(" ") + now.FormatISOTime();

    // If we are before starttime
    if ( now.IsEarlierThan( m_fromTime ) ) return false;

    // If we are after endtime
    if ( now.IsLaterThan( m_endTime ) ) return false;

    wxDateTime::WeekDay day = now.GetWeekDay();
    if ( ( day == wxDateTime::Mon ) && ( 0 == m_weekDay[0] ) ) return false;
    if ( ( day == wxDateTime::Tue ) && ( 0 == m_weekDay[1] ) ) return false;
    if ( ( day == wxDateTime::Wed ) && ( 0 == m_weekDay[2] ) ) return false;
    if ( ( day == wxDateTime::Thu ) && ( 0 == m_weekDay[3] ) ) return false;
    if ( ( day == wxDateTime::Fri ) && ( 0 == m_weekDay[4] ) ) return false;
    if ( ( day == wxDateTime::Sat ) && ( 0 == m_weekDay[5] ) ) return false;
    if ( ( day == wxDateTime::Sun ) && ( 0 == m_weekDay[6] ) ) return false;

    // Check Year
    if ( m_actionYear.GetCount() ) {

        bMatch = false;
        ACTIONTIME::iterator iter;
        for ( iter = m_actionYear.begin(); iter != m_actionYear.end(); ++iter ) {
            if ( NULL != *iter ) {
                if ( **iter == wxDateTime::Now().GetYear() ) {
                    bMatch = true;
                    break;
                }
            }
        }

        // Fail if no match found
        if ( !bMatch ) return false;

    }

    // Check Month
    if ( m_actionMonth.GetCount() ) {

        bMatch = false;
        ACTIONTIME::iterator iter;
        for ( iter = m_actionMonth.begin(); iter != m_actionMonth.end(); ++iter ) {
            if ( NULL != *iter ) {
                if ( **iter == wxDateTime::Now().GetMonth() ) {
                    bMatch = true;
                    break;
                }
            }
        }

        // Fail if no match found
        if ( !bMatch ) return false;

    }

    // Check Day
    if ( m_actionDay.GetCount() ) {

        bMatch = false;
        ACTIONTIME::iterator iter;
        for ( iter = m_actionDay.begin(); iter != m_actionDay.end(); ++iter ) {
            if ( NULL != *iter ) {
                if ( **iter == wxDateTime::Now().GetDay() ) {
                    bMatch = true;
                    break;
                }
            }
        }

        // Fail if no match found
        if ( !bMatch ) return false;
    }

    // Check Hour
    if ( m_actionHour.GetCount() ) {

        bMatch = false;
        ACTIONTIME::iterator iter;
        for ( iter = m_actionHour.begin(); iter != m_actionHour.end(); ++iter ) {
            if ( NULL != *iter ) {
                if ( **iter == wxDateTime::Now().GetHour() ) {
                    bMatch = true;
                    break;
                }
            }
        }

        // Fail if no match found
        if ( !bMatch ) return false;

    }

    // Check Minute
    if ( m_actionMinute.GetCount() ) {

        bMatch = false;
        ACTIONTIME::iterator iter;
        for ( iter = m_actionMinute.begin(); iter != m_actionMinute.end(); ++iter ) {
            if ( NULL != *iter ) {
                if ( **iter == wxDateTime::Now().GetMinute() ) {
                    bMatch = true;
                    break;
                }
            }
        }

        // Fail if no match found
        if ( !bMatch ) return false;

    }

    // Check Second
    if ( m_actionSecond.GetCount() ) {

        bMatch = false;
        ACTIONTIME::iterator iter;
        for ( iter = m_actionSecond.begin(); iter != m_actionSecond.end(); ++iter ) {
            if ( NULL != *iter ) {
                if ( **iter == wxDateTime::Now().GetSecond() ) {
                    bMatch = true;
                    break;
                }
            }
        }

        // Fail if no match found
        if ( !bMatch ) return false;

    }

    return true;

}

///////////////////////////////////////////////////////////////////////////////
// getActionTimeAsString
//

wxString actionTime::getActionTimeAsString( void )
{
    wxString str;
    int cnt;

    // ActionYear
    if ( ( cnt = m_actionYear.GetCount() ) ) {

        ACTIONTIME::iterator iter;
        for ( iter = m_actionYear.begin(); iter != m_actionYear.end(); ++iter ) {

            if ( NULL != *iter ) {
                str += wxString::Format( _("%4d"), **iter );
                // Add backslash for all but last
                cnt--;
                if ( cnt ) {
                    str += _("/");
                }
                else {
                    // Add date separator
                    str += _("-");
                }

            }

        }

    }
    else {
        str += _("*-");     // All years trigger.
    }

    // ActionMonth
    if ( ( cnt = m_actionMonth.GetCount() ) ) {

        ACTIONTIME::iterator iter;
        for ( iter = m_actionMonth.begin(); iter != m_actionMonth.end(); ++iter ) {

            if ( NULL != *iter ) {
                str += wxString::Format( _("%02d"), **iter );
                // Add backslash for all but last
                cnt--;
                if ( cnt ) {
                    str += _("/");
                }
                else {
                    // Add date separator
                    str += _("-");
                }

            }

        }

    }
    else {
        str += _("*-");     // All months trigger.
    }


    // ActionDay
    if ( ( cnt = m_actionDay.GetCount() ) ) {

        ACTIONTIME::iterator iter;
        for ( iter = m_actionDay.begin(); iter != m_actionDay.end(); ++iter ) {

            if ( NULL != *iter ) {
                str += wxString::Format( _("%02d"), **iter );
                // Add backslash for all but last
                cnt--;
                if ( cnt ) {
                    str += _("/");
                }

            }

        }

    }
    else {
        str += _("*");     // All days trigger.
    }


    // Add seperator to time part
    str += _(" ");


    // ActionHour
    if ( ( cnt = m_actionHour.GetCount() ) ) {

        ACTIONTIME::iterator iter;
        for ( iter = m_actionHour.begin(); iter != m_actionHour.end(); ++iter ) {

            if ( NULL != *iter ) {
                str += wxString::Format( _("%02d"), **iter );
                // Add backslash for all but last
                cnt--;
                if ( cnt ) {
                    str += _("/");
                }
                else {
                    // Add date separator
                    str += _(":");
                }
            }

        }

    }
    else {
        str += _("*:");     // All hours trigger.
    }


    // ActionMinute
    if ( ( cnt = m_actionMinute.GetCount() ) ) {

        ACTIONTIME::iterator iter;
        for ( iter = m_actionMinute.begin(); iter != m_actionMinute.end(); ++iter ) {

            if ( NULL != *iter ) {
                str += wxString::Format( _("%02d"), **iter );
                // Add backslash for all but last
                cnt--;
                if ( cnt ) {
                    str += _("/");
                }
                else {
                    // Add date separator
                    str += _(":");
                }
            }

        }


    }
    else {
        str += _("*:");     // All minutes trigger.
    }


    // ActionSeconds
    if ( ( cnt = m_actionSecond.GetCount() ) ) {

        ACTIONTIME::iterator iter;
        for ( iter = m_actionSecond.begin(); iter != m_actionSecond.end(); ++iter ) {

            if ( NULL != *iter ) {
                str += wxString::Format( _("%02d"), **iter );
                // Add backslash for all but last
                cnt--;
                if ( cnt ) {
                    str += _("/");
                }

            }

        }

    }
    else {
        str += _("*");     // All seconds trigger.
    }

    return str;
}



///////////////////////////////////////////////////////////////////////////////



// Constructor

dmElement::dmElement()
{
    m_bEnable = false;
    m_bStatic = false;
    m_id = 0;
    m_strGroupID.Empty();
    vscp_clearVSCPFilter( &m_vscpfilter );
    m_actionCode = 0;
    m_triggCounter = 0;
    m_errorCounter = 0;
    m_actionparam.Empty();
    m_comment.Empty();

    m_bCheckIndex = false;
    m_index = 0;
    
    m_bCheckMeasurementIndex = false;
    
    m_bCheckZone = false;
    m_zone = 0;
    
    m_bCheckSubZone = false;
    m_subzone = 0;
    
    m_timeAllow.allowAlways();
    
    m_bCompareMeasurement = false;  // No measurement comparison
    m_measurementValue = 0;
    m_measurementUnit = 0;          // Default unit
    m_measurementCompareCode = DM_MEASUREMENT_COMPARE_NOOP;   

    m_pDM = NULL;	// Initially no owner
}

// Destructor
dmElement::~dmElement()
{
     ;
}

///////////////////////////////////////////////////////////////////////////////
// Assignment =
//

dmElement& dmElement::operator=( const dmElement& dm)
{
    // Check for self-assignment!
    if ( this == &dm ) {    // Same object?
        return *this;       // Yes, so skip assignment, and just return *this.
    }
    
    m_id = dm.m_id;
    m_bStatic = dm.m_bStatic;
    m_bEnable = dm.m_bEnable;
    m_strGroupID = dm.m_strGroupID;
    memcpy( &m_vscpfilter, &dm.m_vscpfilter, sizeof(vscpEventFilter) );
    m_actionCode = dm.m_actionCode;
    m_actionparam = dm.m_actionparam;
    m_triggCounter = dm.m_triggCounter;
    m_errorCounter = dm.m_errorCounter;
    m_bCheckIndex = dm.m_bCheckIndex;
    m_index = dm.m_index;
    m_bCheckMeasurementIndex = dm.m_bCheckMeasurementIndex;
    m_bCheckZone = dm.m_bCheckZone;
    m_zone = dm.m_zone;
    m_bCheckSubZone = dm.m_bCheckSubZone;
    m_subzone = dm.m_subzone;
    m_strLastError = dm.m_strLastError;
    m_comment = dm.m_comment;
    m_timeAllow = dm.m_timeAllow;
    m_bCompareMeasurement = dm.m_bCompareMeasurement;
    m_measurementValue = dm.m_measurementValue;
    m_measurementUnit = dm.m_measurementUnit;
    m_measurementCompareCode = dm.m_measurementCompareCode;
    m_pDM = dm.m_pDM;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
// getAsString
//
 
wxString dmElement::getAsString( bool bCRLF )
{
    wxString strRow;

    // id
    strRow = wxString::Format( _("%d,"), m_id );
    
    // bEnable
    if ( isEnabled() ) {
        strRow += _("true,");
    }
    else {
        strRow += _("false,");
    }
    
    // GroupID
    strRow += m_strGroupID;
    strRow += _(",");

    // Priority mask
    strRow += wxString::Format(_("0x%02X,"), m_vscpfilter.mask_priority );

    // Class mask
    strRow += wxString::Format(_("0x%04X,"), m_vscpfilter.mask_class );

    // Type mask
    strRow += wxString::Format(_("0x%04X,"), m_vscpfilter.mask_type );

    // GUID mask
    wxString strGUID;
    vscp_writeGuidArrayToString( m_vscpfilter.mask_GUID, strGUID );
    strRow += strGUID;
    strRow += _(",");

    // Priority filter
    strRow += wxString::Format(_("0x%02X,"), m_vscpfilter.filter_priority );

    // Class filter
    strRow += wxString::Format(_("0x%04X,"), m_vscpfilter.filter_class );

    // Type filter
    strRow += wxString::Format(_("0x%04X,"), m_vscpfilter.filter_type );

    // GUID filter
    vscp_writeGuidArrayToString( m_vscpfilter.filter_GUID, strGUID );
    strRow += strGUID;
    strRow += _(",");
    
    // From time
    strRow += m_timeAllow.getFromTime().FormatISODate() + _(" ") +
                                m_timeAllow.getFromTime().FormatISOTime() + _(",");

    // End time
    strRow += m_timeAllow.getEndTime().FormatISODate() + _(" ") +
                                m_timeAllow.getEndTime().FormatISOTime() + _(",");

    // Allowed weekdays
    strRow += m_timeAllow.getWeekDays() + _(",");

    // Action time
    strRow += m_timeAllow.getActionTimeAsString() + _(",");

    // bIndex
    strRow += m_bCheckIndex ? _("true,") : _("false,");
    
    // index
    strRow += wxString::Format(_("%d,"), m_index );
    
    // bMeasurementIndex
    strRow += m_bCheckMeasurementIndex ? _("true,") : _("false,");

    // bCheckZone
    strRow += m_bCheckZone ? _("true,") : _("false,");
    
    // zone
    strRow += wxString::Format(_("%d,"), m_zone );
    
    // bCheckSubZone
    strRow += m_bCheckSubZone ? _("true,") : _("false,");
    
    // subzone
    strRow += wxString::Format(_("%d,"), m_subzone );
    
    // m_bCompareMeasurement
    strRow += m_bCompareMeasurement ? _("true,") : _("false,");
    
    // measurement value
    strRow += wxString::Format(_("%lf,"), m_measurementValue );
    
    // measurement unit
    strRow += wxString::Format(_("%d,"), m_measurementUnit );
    
    // measurement compare code
    strRow += getSymbolicMeasurementFromCompareCode( m_measurementCompareCode );
    strRow += _(",");
    
    // Action Code
    strRow += wxString::Format(_("0x%08X,"), m_actionCode );

    // Action Parameters
    strRow += m_actionparam;
    strRow += _(",");

    // trig-counter:
    strRow += wxString::Format(_("%d,"), m_triggCounter );

    // error-counter:
    strRow += wxString::Format(_("%d,"), m_errorCounter );
    
    // Last error
    strRow += m_strLastError;
    strRow += _(",");        

    strRow += m_comment;

    // New line
    if ( bCRLF ) {
        strRow += _("\r\n");
    }

    return strRow;
}

///////////////////////////////////////////////////////////////////////////////
// getCompareCodeFromSymbolicMeasurement
//

uint8_t dmElement::getCompareCodeFromSymbolicMeasurement( wxString& strCompare )
{
    uint8_t cc = DM_MEASUREMENT_COMPARE_NOOP;
    
    strCompare.MakeUpper();
    strCompare.Trim();
    strCompare.Trim( false );
    
    if ( _("EQ") == strCompare ) {
        cc = DM_MEASUREMENT_COMPARE_EQ;
    }
    else if ( _("==") == strCompare ) {
        cc = DM_MEASUREMENT_COMPARE_EQ;
    }
    else if ( _("NEQ") == strCompare ) {
        cc = DM_MEASUREMENT_COMPARE_NEQ;
    }
    else if ( _("!=") == strCompare ) {
        cc = DM_MEASUREMENT_COMPARE_NEQ;
    }
    else if ( _("LT") == strCompare ) {
        cc = DM_MEASUREMENT_COMPARE_LT;
    }
    else if ( _("<") == strCompare ) {
        cc = DM_MEASUREMENT_COMPARE_LT;
    }
    else if ( _("LTEQ") == strCompare ) {
        cc = DM_MEASUREMENT_COMPARE_LTEQ;
    }
    else if ( _("<=") == strCompare ) {
        cc = DM_MEASUREMENT_COMPARE_LTEQ;
    }
    else if ( _("GT") == strCompare ) {
        cc = DM_MEASUREMENT_COMPARE_GT;
    }
    else if ( _(">") == strCompare ) {
        cc = DM_MEASUREMENT_COMPARE_GT;
    }
    else if ( _("GTEQ") == strCompare ) {
        cc = DM_MEASUREMENT_COMPARE_GTEQ;
    }
    else if ( _(">=") == strCompare ) {
        cc = DM_MEASUREMENT_COMPARE_GTEQ;
    }
    else if ( _("NOOP") == strCompare ) {
        cc = DM_MEASUREMENT_COMPARE_NOOP;
    }
    
    return cc;
}

///////////////////////////////////////////////////////////////////////////////
// getSymbolicMeasurementCompareCode
//

wxString dmElement::getSymbolicMeasurementFromCompareCode( uint8_t cc, uint8_t type )
{
    wxString scc = _("NOOP");
    
    if ( DM_MEASUREMENT_COMPARE_EQ == cc ) {
        if ( 0 == type ) {
            scc = _("==");
        }
        else {
            scc = _("EQ");
        }
    }
    else if ( DM_MEASUREMENT_COMPARE_NEQ == cc ) {
        if ( 0 == type ) {
            scc = _("!=");
        }
        else {
            scc = _("NEQ");
        }
    }
    else if ( DM_MEASUREMENT_COMPARE_LT == cc ) {
        if ( 0 == type ) {
            scc = _("<");
        }
        else {
            scc = _("LT");
        }
    }
    else if ( DM_MEASUREMENT_COMPARE_LTEQ == cc ) {
        if ( 0 == type ) {
            scc = _("<=");
        }
        else {
            scc = _("LTEQ");
        }
    }
    else if ( DM_MEASUREMENT_COMPARE_GT == cc ) {
        if ( 0 == type ) {
            scc = _(">");
        }
        else {
            scc = _("GT");
        }
    }
    else if ( DM_MEASUREMENT_COMPARE_GTEQ == cc ) {
        if ( 0 == type ) {
            scc = _(">=");
        }
        else {
            scc = _("GTEQ");
        }
    }
    else {
        scc = _("NOOP");
    }
    
    return scc;
}

///////////////////////////////////////////////////////////////////////////////
// setFromString
//


bool dmElement::setFromString( wxString& strDM )
{
    wxString wxstr;
    long unsigned int lval;
    
    wxStringTokenizer tkz( strDM, _(",") );

    // bEnable 
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken().MakeUpper();
        if ( wxNOT_FOUND == wxstr.Find( _("TRUE") ) ) {
            m_bEnable = true;
        }
        else {
            m_bEnable = false;
        }
    }
    
    // GroupID
    if ( tkz.HasMoreTokens() ) {
        m_strGroupID = tkz.GetNextToken();
    }
    
    // Mask priority
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( wxstr.ToULong( &lval ) ) {
            m_vscpfilter.mask_priority = lval;
        }
        else {
            m_vscpfilter.mask_priority = 0;
        }
    }
    
    // Mask class
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( wxstr.ToULong( &lval ) ) {
            m_vscpfilter.mask_class = lval;
        }
        else {
            m_vscpfilter.mask_class = 0;
        }
    }
    
    // Mask type
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( wxstr.ToULong( &lval ) ) {
            m_vscpfilter.mask_type = lval;
        }
        else {
            m_vscpfilter.mask_type = 0;
        }
    }
    
    // mask GUID
    if ( tkz.HasMoreTokens() ) {
        cguid guid;
        wxstr = tkz.GetNextToken();
        guid.getFromString( wxstr );
        memcpy( m_vscpfilter.mask_GUID, guid.getGUID(), 16 );
    }
    
    // Filter priority
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( wxstr.ToULong( &lval ) ) {
            m_vscpfilter.filter_priority = lval;
        }
        else {
            m_vscpfilter.filter_priority = 0;
        }
    }
    
    // Filter class
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( wxstr.ToULong( &lval ) ) {
            m_vscpfilter.filter_class = lval;
        }
        else {
            m_vscpfilter.filter_class = 0;
        }
    }
    
    // Filter type
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( wxstr.ToULong( &lval ) ) {
            m_vscpfilter.filter_type = lval;
        }
        else {
            m_vscpfilter.filter_type = 0;
        }
    }
    
    // Filter GUID
    if ( tkz.HasMoreTokens() ) {
        cguid guid;
        wxstr = tkz.GetNextToken();
        guid.getFromString( wxstr );
        memcpy( m_vscpfilter.filter_GUID, guid.getGUID(), 16 );
    }
    
    // Allowed start
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();  
    }
    
    // Allowed end
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
    }
    
    // Allowed weekdays
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        m_timeAllow.setWeekDay( wxstr );
    }
    
    // Allowed time
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        m_timeAllow.parseActionTime( wxstr );
    }
    
    // bCheckIndex
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken().MakeUpper();
        if ( wxNOT_FOUND == wxstr.Find( _("TRUE") ) ) {
            m_bCheckIndex = true;
        }
        else {
            m_bCheckIndex = false;
        }
    }
    
    // index
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( wxstr.ToULong( &lval ) ) {
            m_index = lval;
        }
        else {
            m_index = 0;
        }
    }
    
    // bCheckZone
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken().MakeUpper();
        if ( wxNOT_FOUND == wxstr.Find( _("TRUE") ) ) {
            m_bCheckZone = true;
        }
        else {
            m_bCheckZone = false;
        }
    }
    
    // zone
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( wxstr.ToULong( &lval ) ) {
            m_zone = lval;
        }
        else {
            m_zone = 0;
        }
    }
    
    // bCheckSubZone
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken().MakeUpper();
        if ( wxNOT_FOUND == wxstr.Find( _("TRUE") ) ) {
            m_bCheckSubZone = true;
        }
        else {
            m_bCheckSubZone = false;
        }
    }
    
    // subzone
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( wxstr.ToULong( &lval ) ) {
            m_subzone = lval;
        }
        else {
            m_subzone = 0;
        }
    }
    
    // bCheckMeasurementIndex
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken().MakeUpper();
        if ( wxNOT_FOUND == wxstr.Find( _("TRUE") ) ) {
            m_bCheckMeasurementIndex = true;
        }
        else {
            m_bCheckMeasurementIndex = false;
        }
    }
    
    // actioncode
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( wxstr.ToULong( &lval ) ) {
            m_actionCode = lval;
        }
        else {
            m_actionCode = 0;
        }
    }
    
    // Action parameter
    if ( tkz.HasMoreTokens() ) {
        m_actionparam = tkz.GetNextToken();
    }
    
    
    // Measurement compare flag
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken().MakeUpper();
        if ( wxNOT_FOUND == wxstr.Find( _("TRUE") ) ) {
            m_bCompareMeasurement = true;
        }
        else {
            m_bCompareMeasurement = false;
        }
    }
    
    // Measurement value
    if ( tkz.HasMoreTokens() ) {
        double dval;
        wxstr = tkz.GetNextToken();
        if ( wxstr.ToDouble( &dval ) ) {
            m_measurementValue = dval;
        }
        else {
            m_measurementValue = dval;
        }
    }
    
    // measurement unit
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( wxstr.ToULong( &lval ) ) {
            m_measurementUnit = lval;
        }
        else {
            m_measurementUnit = 0;
        }
    }
    
    // measurement compare code
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( wxstr.ToULong( &lval ) ) {
            m_measurementCompareCode = lval;
        }
        else {
            m_measurementCompareCode = 0;
        }
    }
    
    // comment
    if ( tkz.HasMoreTokens() ) {
        m_comment = tkz.GetNextToken();
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// handleEscapes
//

bool dmElement::handleEscapes( vscpEvent *pEvent, wxString& str )
{
    int pos;
    wxString strResult;

    while ( wxNOT_FOUND != ( pos = str.Find( _("%") ) ) ) {

        strResult += str.Left( pos );
        str = str.Right( str.Length() - pos );

        if ( str.Length() > 1 ) {

            // Check for percent i.e. %%
            if ( str.StartsWith( _("%%"), &str ) ) {
                strResult += _("%");                  // add percent
            }
            // Check for ';' i.e. %;
            else if ( str.StartsWith( _("%;"), &str ) ) {
                strResult += _(";");                  // add semicolon
            }
            // Check for %cr
            else if ( str.StartsWith( _("%cr"), &str ) ) {
                strResult += _("\r");                 // add carrige return
            }
            // Check for %lf
            else if ( str.StartsWith( _("%lf"), &str ) ) {
                strResult += _("\n");                 // add new line
            }
            // Check for %crlf
            else if ( str.StartsWith( _("%crlf"), &str ) ) {
                strResult += _("\r\n");               // add carrige return line feed
            }
            // Check for %tab
            else if ( str.StartsWith( _("%tab"), &str ) ) {
                strResult += _("\t");                 // add tab
            }
            // Check for %bell
            else if ( str.StartsWith( _("%bell"), &str ) ) {
                strResult += _("\a");                 // add bell
            }
            // Check for %amp.html
            else if ( str.StartsWith( _("%amp.html"), &str ) ) {
                strResult += _("&amp;");              // add bell
            }
            // Check for %amp
            else if ( str.StartsWith( _("%amp"), &str ) ) {
                strResult += _("&");                  // add bell
            }
            // Check for %lt.html
            else if ( str.StartsWith( _("%lt.html"), &str ) ) {
                strResult += _T("&lt;");               // add bell
            }
            // Check for %lt
            else if ( str.StartsWith( _T("%lt"), &str ) ) {
                strResult += _T("<");                  // add bell
            }
            // Check for %gt.html
            else if ( str.StartsWith( _T("%gt.html"), &str ) ) {
                strResult += _T("&gt;");               // add bell
            }
            // Check for %gt
            else if ( str.StartsWith( _T("%gt"), &str ) ) {
                strResult += _T(">");                  // add bell
            }
            // Check for head escape
            else if ( str.StartsWith( _T("%event.head"), &str ) ) {
                strResult +=  wxString::Format( _("%d"), pEvent->head );
            }
            // Check for priority escape
            else if ( str.StartsWith( _T("%event.priority"), &str ) ) {
                strResult +=  wxString::Format( _T("%d"),
                    ( ( pEvent->head & VSCP_HEADER_PRIORITY_MASK ) >> 5 ) );
            }
            // Check for hardcoded escape
            else if ( str.StartsWith( _T("%event.hardcoded"), &str ) ) {
                if ( pEvent->head & VSCP_HEADER_HARD_CODED ) {
                    strResult += _T("1");
                }
                else {
                    strResult += _T("0");
                }
            }
            // Check for class escape
            else if ( str.StartsWith( _("%event.class"), &str ) ) {
                strResult +=  wxString::Format( _("%d"), pEvent->vscp_class );
            }
            // Check for class string  escape
            else if ( str.StartsWith( _("%event.class.str"), &str ) ) {
                //VSCPInformation info;  TODO
                //strResult +=  info.getClassDescription( pEvent->vscp_class );
            }
            // Check for type escape
            else if ( str.StartsWith( _("%event.type"), &str ) ) {
                strResult +=  wxString::Format( _("%d"), pEvent->vscp_type );
            }
            // Check for type string escape
            else if ( str.StartsWith( _("%event.type.str"), &str ) ) {
                //VSCPInformation info;  TODO
                //strResult +=  info.getTypeDescription( pEvent->vscp_class,
                //                                    pEvent->vscp_type );
            }
            // Check for data[n] escape
            else if ( str.StartsWith( _("%event.data["), &str ) ) {

                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {

                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }

                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( idx < pEvent->sizeData ) {
                        strResult +=  wxString::Format( _("%d"), pEvent->pdata[ idx ] );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }


                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
            }
            // Check for data[n] escape
            else if ( str.StartsWith( _("%event.hexdata["), &str ) ) {

                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {

                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }

                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( idx < pEvent->sizeData ) {
                        strResult +=  wxString::Format( _("%02X"), pEvent->pdata[ idx ] );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }


                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
            }
            // Check for data.bit[n] escape
            else if ( str.StartsWith( _("%event.data.bit["), &str ) ) {
                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    
                    long idx=0;
                    wxstr.ToLong( &idx );
                    uint8_t byte = idx/8;
                    uint8_t bit = idx % 8;
                    if ( byte < pEvent->sizeData ) {
                        strResult +=  wxString::Format( _("%d"), 
                                ( pEvent->pdata[ byte ] & (1<<(7-bit) ) ) ? 1 : 0 );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }
                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
                 
            }
            // Check for data.bool[n] escape
            else if ( str.StartsWith( _("%event.data.bool["), &str ) ) {
                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    
                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( idx < pEvent->sizeData ) {
                        strResult +=  wxString::Format( _("%s"), 
                                    pEvent->pdata[ idx ] ? "true" : "false"  );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }
                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
            }            
            // Check for data.int8[n] escape
            else if ( str.StartsWith( _("%event.data.int8["), &str ) ) {
                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    
                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( idx < pEvent->sizeData ) {
                        strResult +=  wxString::Format( _("%d"), 
                                    (int8_t)pEvent->pdata[ idx ]  );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }
                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
            }
            // Check for data.uint8[n] escape
            else if ( str.StartsWith( _("%event.data.uint8["), &str ) ) {
                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    
                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( idx < pEvent->sizeData ) {
                        strResult +=  wxString::Format( _("%d"), 
                                    (uint8_t)pEvent->pdata[ idx ]  );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }
                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
            }
            // Check for data.uint8[n] escape
            else if ( str.StartsWith( _("%event.hexdata.uint8["), &str ) ) {
                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    
                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( idx < pEvent->sizeData ) {
                        strResult +=  wxString::Format( _("%02X"), 
                                    (uint8_t)pEvent->pdata[ idx ]  );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }
                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
            }
            // Check for data.int16[n] escape
            else if ( str.StartsWith( _("%event.data.int16["), &str ) ) {
                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    
                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( (idx+1) < pEvent->sizeData ) {
                        int16_t val = ((int16_t)pEvent->pdata[ idx ] << 8 ) +
                                      ((int16_t)pEvent->pdata[ idx + 1 ] );
                        strResult +=  wxString::Format( _("%d"), (int)val  );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }
                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
            }
            // Check for data.uint16[n] escape
            else if ( str.StartsWith( _("%event.data.uint16["), &str ) ) {
                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    
                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( (idx+1) < pEvent->sizeData ) {
                        uint16_t val = ((uint16_t)pEvent->pdata[ idx ] << 8 ) +
                                       ((uint16_t)pEvent->pdata[ idx + 1 ] );
                        strResult +=  wxString::Format( _("%d"), (int)val  );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }
                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
            }// Check for data.uint16[n] escape
            else if ( str.StartsWith( _("%event.hexdata.uint16["), &str ) ) {
                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    
                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( (idx+1) < pEvent->sizeData ) {
                        uint16_t val = ((uint16_t)pEvent->pdata[ idx ] << 8) +
                                       ((uint16_t)pEvent->pdata[ idx + 1 ]);
                        strResult +=  wxString::Format( _("%04X"), val  );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }
                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
            }            
            // Check for data.int32[n] escape
            else if ( str.StartsWith( _("%event.data.int32["), &str ) ) {
                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    
                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( (idx+3) < pEvent->sizeData ) {
                        long val = ((long)pEvent->pdata[ idx ] << 24) +
                                        ((long)pEvent->pdata[ idx + 1 ] << 16) +
                                        ((long)pEvent->pdata[ idx + 2 ] << 8) +
                                        ((long)pEvent->pdata[ idx + 3 ]);
                        strResult +=  wxString::Format( _("%ld"), val  );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }
                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
            }
            // Check for data.uint32[n] escape
            else if ( str.StartsWith( _("%event.data.uint32["), &str ) ) {
                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    
                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( (idx+3) < pEvent->sizeData ) {
                        uint32_t val = ((uint32_t)pEvent->pdata[ idx ] << 24) +
                                       ((uint32_t)pEvent->pdata[ idx + 1 ] << 16) +
                                       ((uint32_t)pEvent->pdata[ idx + 2 ] << 8) +
                                       ((uint32_t)pEvent->pdata[ idx + 3 ]);
                        strResult +=  wxString::Format( _("%lu"), (unsigned long)val  );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }
                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
            }
            // Check for hexdata.uint32[n] escape
            else if ( str.StartsWith( _("%event.hexdata.uint32["), &str ) ) {
                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    
                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( (idx+3) < pEvent->sizeData ) {
                        uint32_t val = ((uint32_t)pEvent->pdata[ idx ] << 24) +
                                       ((uint32_t)pEvent->pdata[ idx + 1 ] << 16) +
                                       ((uint32_t)pEvent->pdata[ idx + 2 ] << 8) +
                                       ((uint32_t)pEvent->pdata[ idx + 3 ]);
                        strResult +=  wxString::Format( _("%08X"), val  );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }
                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
            }
            // Check for data.float[n] escape
            else if ( str.StartsWith( _("%event.data.float["), &str ) ) {
                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    
                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( (idx+3) < pEvent->sizeData ) {
                        float val =  *( (float *)( pEvent->pdata + idx ) );
                        strResult +=  wxString::Format( _("%f"), val  );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }
                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
            }
            // Check for data.double[n] escape
            else if ( str.StartsWith( _("%event.data.double["), &str ) ) {
                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    
                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( (idx+7) < pEvent->sizeData ) {
                        double val =  *( (double *)( pEvent->pdata + idx ) );
                        strResult +=  wxString::Format( _("%lf"), val  );
                    }
                    else {
                        strResult +=  _("?");     // invalid index
                    }
                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( _("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  _("");      // No data
                }
            }
            // Check for sizedata escape
            else if ( str.StartsWith( _("%event.sizedata"), &str ) ) {
                strResult +=  wxString::Format( _("%d"), pEvent->sizeData );
            }
            // Check for data escape
            else if ( str.StartsWith( _("%event.data"), &str ) ) {
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr;
                    vscp_writeVscpDataToString( pEvent, wxstr, false );
                    strResult +=  wxstr;
                }
                else {
                    strResult +=  _("empty");     // No data
                }
            }
            // Check for guid escape
            else if ( str.StartsWith( _("%event.guid"), &str ) ) {
                wxString strGUID;
                vscp_writeGuidToString ( pEvent, strGUID );
                strResult +=  strGUID;
            }
            // Check for nickname escape
            else if ( str.StartsWith( _( "%event.nickname" ), &str ) ) {
                strResult += wxString::Format( _( "%d" ),
                                                pEvent->GUID[ VSCP_GUID_LSB ] );
            }
            // Check for obid escape
            else if ( str.StartsWith( _("%event.obid"), &str ) ) {
                strResult +=  wxString::Format( _("%d"), pEvent->obid );
            }
            // Check for timestamp escape
            else if ( str.StartsWith( _("%event.timestamp"), &str ) ) {
                strResult +=  wxString::Format( _("%d"), pEvent->timestamp );
            }
            // Check for event index escape
            else if ( str.StartsWith( _( "%event.index" ), &str ) ) {
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    strResult += wxString::Format( _( "%d" ), pEvent->pdata[ 0 ] );
                }
                else {
                    strResult += _( "empty" );        // No data
                }
            }
            // Check for event zone escape
            else if ( str.StartsWith( _( "%event.zone" ), &str ) ) {
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    strResult += wxString::Format( _( "%d" ), pEvent->pdata[ 1 ] );
                }
                else {
                    strResult += _( "empty" );        // No data
                }
            }
            // Check for event subzone escape
            else if ( str.StartsWith( _( "%event.subzone" ), &str ) ) {
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    strResult += wxString::Format( _( "%d" ), pEvent->pdata[ 2 ] );
                }
                else {
                    strResult += _( "empty" );        // No data
                }
            }
            // Check for event escape
            else if ( str.StartsWith( _("%event"), &str ) ) {
                wxString strEvent;
                vscp_writeVscpEventToString( pEvent, strEvent );
                strResult += strEvent;
            }
            // Check for isodate escape
            else if ( str.StartsWith( _("%isodate"), &str ) ) {
                strResult += wxDateTime::Now().FormatISODate();
            }
            // Check for isotime escape
            else if ( str.StartsWith( _("%isotime"), &str ) ) {
                strResult += wxDateTime::Now().FormatISOTime();
            }
            // Check for isobothms escape
            else if ( str.StartsWith( _("%isobothms"), &str ) ) {
                // Milliseconds  
                long            ms; // Milliseconds
#ifdef WIN32  
                SYSTEMTIME st;
                GetSystemTime( &st );
                ms = st.wMilliseconds;
#else                
                time_t          s;  // Seconds
                struct timespec spec;
                clock_gettime(CLOCK_REALTIME, &spec);
                s  = spec.tv_sec;
                ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
#endif
                //int ms = wxDateTime::Now().GetMillisecond();   !!!!! Does not work  !!!!!
                wxString msstr = wxString::Format(_(".%ld"), ms );
                strResult += wxDateTime::Now().FormatISODate();
                strResult += _("T");
                strResult += wxDateTime::Now().FormatISOTime();
                strResult += msstr;
            }
            // Check for isoboth escape
            else if ( str.StartsWith( _("%isoboth"), &str ) ) {
                strResult += wxDateTime::Now().FormatISODate();
                strResult += _("T");
                strResult += wxDateTime::Now().FormatISOTime();
            }            
            // Check for mstime escape
            else if ( str.StartsWith( _("%mstime"), &str ) ) {
                strResult += wxString::Format( _("%d"),
                                                wxDateTime::Now().GetTicks() );
            }
            // Check for unixtime escape
            else if ( str.StartsWith( _("%unixtime"), &str ) ) {
                time_t now = time(NULL);    // Get current time
                uint64_t tnow = now;
                strResult += wxString::Format( _("%llu"), tnow );
            }
            // Check for hour escape
            else if ( str.StartsWith( _("%hour"), &str ) ) {
                strResult += wxString::Format( _("%d"), wxDateTime::Now().GetHour() );
            }
            // Check for minute escape
            else if ( str.StartsWith( _("%minute"), &str ) ) {
                strResult += wxString::Format( _("%d"), wxDateTime::Now().GetMinute() );
            }
            // Check for second escape
            else if ( str.StartsWith( _("%second"), &str ) ) {
                strResult += wxString::Format( _("%d"), wxDateTime::Now().GetSecond() );
            }
            // Check for week0 escape
            else if ( str.StartsWith( _("%week0"), &str ) ) {
                strResult += wxString::Format( _("%d"),
                                                wxDateTime::Now().GetWeekOfYear(),
                                                wxDateTime::Sunday_First );
            }
            // Check for week1 escape
            else if ( str.StartsWith( _("%week1"), &str ) ) {
                strResult += wxString::Format( _("%d"),
                                                wxDateTime::Now().GetWeekOfYear(),
                                                wxDateTime::Monday_First );
            }
            // Check for weekdaytxtfull escape
            else if ( str.StartsWith( _("%weekdaytxtfull"), &str ) ) {
                strResult += wxDateTime::GetWeekDayName(
                        wxDateTime::Now().GetWeekDay(), wxDateTime::Name_Full );
            }
            // Check for weekdaytxt escape
            else if ( str.StartsWith( _("%weekdaytxt"), &str ) ) {
                strResult += wxDateTime::GetWeekDayName(
                        wxDateTime::Now().GetWeekDay(), wxDateTime::Name_Abbr );
            }
            // Check for monthtxtfull escape
            else if ( str.StartsWith( _("%monthtxtfull"), &str ) ) {
                strResult += wxDateTime::GetMonthName(
                        wxDateTime::Now().GetMonth(), wxDateTime::Name_Full );
            }
            // Check for monthtxt escape
            else if ( str.StartsWith( _("%monthtxt"), &str ) ) {
                strResult += wxDateTime::GetMonthName(
                        wxDateTime::Now().GetMonth(), wxDateTime::Name_Abbr );
            }
            // Check for month escape
            else if ( str.StartsWith( _("%month"), &str ) ) {
                strResult += wxString::Format( _("%d"),
                        wxDateTime::Now().GetMonth() + 1 );
            }
            // Check for year escape
            else if ( str.StartsWith( _("%year"), &str ) ) {
                strResult += wxString::Format( _("%d"),
                        wxDateTime::Now().GetYear() );
            }
            // Check for quarter escape
            else if ( str.StartsWith( _("%quarter"), &str ) ) {
                if ( wxDateTime::Now().GetMonth() < 4 ) {
                    strResult += _("1");
                }
                else if ( wxDateTime::Now().GetMonth() < 7 ) {
                    strResult += _("2");
                }
                else if ( wxDateTime::Now().GetMonth() < 10 ) {
                    strResult += _("3");
                }
                else {
                    strResult += _("1");
                }
            }
            // Check for path_config escape
            else if ( str.StartsWith( _("%path.config"), &str ) ) {
                strResult += wxStandardPaths::Get().GetConfigDir();
            }
            // Check for path_datadir escape
            else if ( str.StartsWith( _("%path.datadir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetDataDir();
            }
            // Check for path_documentsdir escape
            else if ( str.StartsWith( _("%path.documentsdir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetDocumentsDir();
            }
            // Check for path_executable escape
            else if ( str.StartsWith( _("%path.executable"), &str ) ) {
                strResult += wxStandardPaths::Get().GetExecutablePath();
            }
            // Check for path_localdatadir escape
            else if ( str.StartsWith( _("%path.localdatadir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetLocalDataDir();
            }
            // Check for path_pluginsdir escape
            else if ( str.StartsWith( _("%path.pluginsdir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetPluginsDir();
            }
            // Check for path_resourcedir escape
            else if ( str.StartsWith( _("%path.resourcedir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetResourcesDir();
            }
            // Check for path_tempdir escape
            else if ( str.StartsWith( _("%path.tempdir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetTempDir();
            }
            // Check for path_userconfigdir escape
            else if ( str.StartsWith( _("%path.userconfigdir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetUserConfigDir();
            }
            // Check for path_userdatadir escape
            else if ( str.StartsWith( _("%path.userdatadir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetUserDataDir();
            }
            // Check for path_localdatadir escape
            else if ( str.StartsWith( _("%path.localdatadir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetUserLocalDataDir();
            }
            // Check for toliveafter
            else if ( str.StartsWith( _("%toliveafter1"), &str ) ) {
                strResult += _("Carpe diem quam minimum credula postero.");
            }
            // Check for toliveafter
            else if ( str.StartsWith( _("%toliveafter2"), &str ) ) {
                strResult += _("Be Hungry - Stay Foolish.");
            }
            // Check for toliveafter
            else if ( str.StartsWith( _("%toliveafter3"), &str ) ) {
                strResult += _("Be Foolish - Stay Hungry.");
            }
            // Check for measurement.index escape
            else if ( str.StartsWith( _("%measurement.index"), &str ) ) {
                uint8_t data_coding_byte = vscp_getMeasurementDataCoding( pEvent );
                if ( -1 != data_coding_byte ) {
                    strResult += wxString::Format( _("%d"),
                                    VSCP_DATACODING_INDEX( data_coding_byte ) );
                }
            }
            // Check for measurement.unit escape
            else if ( str.StartsWith( _("%measurement.unit"), &str ) ) {
                uint8_t data_coding_byte = vscp_getMeasurementDataCoding( pEvent );
                if ( -1 != data_coding_byte ) {
                    strResult += wxString::Format( _("%d"),
                                    VSCP_DATACODING_UNIT( data_coding_byte ) );
                }
            }
            // Check for measurement.coding escape
            else if ( str.StartsWith( _("%measurement.coding"), &str ) ) {
                uint8_t data_coding_byte = vscp_getMeasurementDataCoding( pEvent );
                if ( -1 != data_coding_byte ) {
                    strResult += wxString::Format( _("%d"),
                                    VSCP_DATACODING_TYPE( data_coding_byte ) );
                }
            }
            // Check for measurement.float escape
            else if ( str.StartsWith( _("%measurement.float"), &str ) ) {
                wxString str;
                vscp_getVSCPMeasurementAsString( pEvent, str );
                strResult += str;
            }
            // Check for measurement.string escape
            else if ( str.StartsWith( _("%measurement.string"), &str ) ) {
                wxString str;
                vscp_getVSCPMeasurementAsString( pEvent, str );
                strResult += str;
            }
            // Check for measurement.convert.data escape
            else if ( str.StartsWith( _("%measurement.convert.data"), &str ) ) {
                wxString str;
                if ( vscp_getVSCPMeasurementAsString(pEvent, str ) ) {
                    for ( unsigned int i=0; i<str.Length(); i++ ) {
                        if (0!=i) strResult += ','; // Not at first location
                        strResult += str.GetChar(i);
                    }
                }
            }

            // Check for eventdata.realtext escape
            else if ( str.StartsWith( _("%eventdata.realtext"), &str ) ) {
                strResult += vscp_getRealTextData( pEvent );
            }

            // Check for %variable:[name] (name is name of variable)
            else if ( str.StartsWith( _("%variable:["), &str ) ) {

                str.Trim(); // Trim of leading white space
                if ( wxNOT_FOUND != ( pos = str.First(']') ) ) {
                    
                    CVSCPVariable variable;
                    
                    wxString variableName = str.Left( pos );
                    str = str.Right( str.Length() - pos - 1 );
                    
                    // Assign value if variable exist
                    if ( gpobj->m_variables.find( variableName, variable ) ) {

                        wxString wxwrk;
                        variable.writeValueToString( wxwrk, true );                       
                       
                        strResult += wxwrk;

                    }
                    
                }

            }
            
            // Check for %variable:[name] (name is name of variable)
            else if ( str.StartsWith( _("%vardecode:["), &str ) ) {
                
                str.Trim(); // Trim of leading white space
                if ( wxNOT_FOUND != ( pos = str.First(']') ) ) {
                    
                    CVSCPVariable variable;
                    
                    wxString variableName = str.Left( pos );
                    str = str.Right( str.Length() - pos - 1 );
                    
                    // Assign value if variable exist
                    if ( gpobj->m_variables.find( variableName, variable ) ) {

                        wxString wxwrk;
                        variable.writeValueToString( wxwrk );
                        strResult += wxwrk;

                    }
                    
                }
                
            }
            
            // Check for %file:[path] (path is path to file to include)
            else if ( str.StartsWith( _("%file:["), &str ) ) {
                
                str.Trim(); // Trim of leading white space
                if ( wxNOT_FOUND != ( pos = str.First(']') ) ) {
                    
                    wxString path = str.Left( pos );
                    str = str.Right( str.Length() - pos - 1 );
                   
                    wxFile f;
                    if ( f.Open( path ) ) {
                        
                        char buf[ 0x10000 ];                        
                        memset( buf, 0, sizeof( buf ) );
                        
                        int size= 0;
                        if ( wxInvalidOffset != 
                                ( size = f.Read( buf, sizeof( buf ) - 1  ) ) ) {
                            strResult += wxString::FromUTF8( buf, size );
                        }
                                
                        f.Close();
                        
                    }
                    
                }           
                
            }

            ////////////////////////////////////////////////////////////////////
            //                  VSCP Stock variables
            ////////////////////////////////////////////////////////////////////

            else if (  str.StartsWith( _("%vscp.version.major"), &str ) ) {
                strResult += wxString::Format( _("%d"), VSCPD_MAJOR_VERSION );
            }
            else if (  str.StartsWith( _("%vscp.version.minor"), &str ) ) {
                strResult += wxString::Format( _("%d"), VSCPD_MINOR_VERSION );
            }
            else if (  str.StartsWith( _("%vscp.version.sub"), &str ) ) {
                strResult += wxString::Format( _("%d"), VSCPD_RELEASE_VERSION );
            }
            else if (  str.StartsWith( _("%vscp.version.build"), &str ) ) {
                strResult += wxString::Format( _("%d"), VSCPD_BUILD_VERSION );
            }
            else if (  str.StartsWith( _("%vscp.version.str"), &str ) ) {
                strResult += wxString::Format( _("%s"), VSCPD_DISPLAY_VERSION );
            }
            else if (  str.StartsWith( _("%vscp.version.wxwidgets"), &str ) ) {
                strResult += wxString::Format( _("%s"), wxVERSION_STRING );
            }
            else if (  str.StartsWith( _("%vscp.version.wxwidgets.str"), &str ) ) {
                strResult += wxString::Format( _("%s"), wxVERSION_STRING );
            }
            else if (  str.StartsWith( _("%vscp.copyright"), &str ) ) {
                strResult += wxString::Format( _("%s"), VSCPD_COPYRIGHT );
            }
            else if (  str.StartsWith( _("%vscp.copyright.vscp"), &str ) ) {
                strResult += wxString::Format( _("%s"), VSCPD_COPYRIGHT );
            }
            else if (  str.StartsWith( _("%vscp.copyright.wxwidgets"), &str ) ) {
                strResult += wxString::Format( _("%s"), " Copyright (c) 1998-2005 Julian Smart, Robert Roebling et al" );
            }
            else if (  str.StartsWith( _("%vscp.copyright.mongoose"), &str ) ) {
                strResult += wxString::Format( _("%s"), "Copyright (c) 2013-2015 Cesanta Software Limited" );
            }
            else if (  str.StartsWith( _("%vscp.os.str"), &str ) ) {
                strResult += wxGetOsDescription();
            }
            else if (  str.StartsWith( _("%vscp.os.width"), &str ) ) {
                if ( wxIsPlatform64Bit() ) {
                    strResult += _("64-bit ");
                }
                else {
                    strResult += _("32-bit ");
                }
            }
            else if (  str.StartsWith( _("%vscp.os.endian"), &str ) ) {
                if ( wxIsPlatformLittleEndian() ) {
                    strResult += _("Little endian ");
                }
                else {
                    strResult += _("Big endian ");
                }
            }
            else if (  str.StartsWith( _("%vscp.memory.free"), &str ) ) {
                wxMemorySize memsize;
                strResult += memsize.ToString();
            }
            else if (  str.StartsWith( _("%vscp.host.fullname"), &str ) ) {
                strResult += wxGetFullHostName();
            }
            // ****************  ESCAPE WAS NOT FOUND ****************
            else {
                // Move beyond the '%'
                strResult += _("%");
                str = str.Right( str.Length() - 1 );
            }

        }        

    }

    // Add last part of working string if any.
    strResult += str;
    str = strResult;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// DoAction
//

bool dmElement::doAction( vscpEvent *pEvent )
{
    wxString logStr;

    // Leave here for test of escapes
    //handleEscapes( pEvent, wxString( _("This is a test %class %type 
    // [%sizedata] %data  %data[0] %data[90]") ) );

    m_triggCounter++;

    // Must be a valid event
    if ( NULL == pEvent ) return false;

    switch ( m_actionCode ) {

        case  VSCP_DAEMON_ACTION_CODE_EXECUTE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_EXECUTE.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }

            doActionExecute( pEvent );
            break;

        case  VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }

            doActionExecuteConditional( pEvent );
            break;    

        case  VSCP_DAEMON_ACTION_CODE_SEND_EVENT:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_SEND_EVENT.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }

            doActionSendEvent( pEvent );
            break;
            
        case VSCP_DAEMON_ACTION_CODE_SEND_TO_REMOTE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_SEND_EVENT.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }

            doActionSendEventRemote( pEvent, false );
            break;

        case VSCP_DAEMON_ACTION_CODE_SEND_EVENT_CONDITIONAL:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_SEND_EVENT_CONDITIONAL.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            
            doActionSendEventConditional( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_SEND_EVENTS_FROM_FILE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_SEND_EVENTS_FROM_FILE.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            
            doActionSendEventsFromFile( pEvent );
            break;

        case  VSCP_DAEMON_ACTION_CODE_STORE_VARIABLE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_STORE_VARIABLE.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            
            doActionStoreVariable( pEvent );
            break;

        case  VSCP_DAEMON_ACTION_CODE_ADD_VARIABLE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_ADD_VARIABLE.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg( _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }

            doActionAddVariable( pEvent );
            break;

        case  VSCP_DAEMON_ACTION_CODE_SUBTRACT_VARIABLE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_SUBTRACT_VARIABLE.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            
            doActionSubtractVariable( pEvent );
            break;

        case  VSCP_DAEMON_ACTION_CODE_MULTIPLY_VARIABLE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_MULTIPLY_VARIABLE.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            
            doActionMultiplyVariable( pEvent );
            break;

        case  VSCP_DAEMON_ACTION_CODE_DIVIDE_VARIABLE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_DIVIDE_VARIABLE.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            
            doActionDivideVariable( pEvent );
            break;
            
        case VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE_TRUE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE_TRUE.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }            
            doActionCheckVariable( pEvent, VARIABLE_CHECK_SET_TRUE );
            break;
            
        case VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE_FALSE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE_FALSE.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg( _("[DM] ") +  _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            doActionCheckVariable( pEvent, VARIABLE_CHECK_SET_FALSE );
            break;
            
        case VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            doActionCheckVariable( pEvent, VARIABLE_CHECK_SET_OUTCOME );
            break;
            
        case VSCP_DAEMON_ACTION_CODE_CHECK_MEASUREMENT:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_CHECK_MEASUREMENT.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            doActionCheckMeasurement( pEvent );
            break;
            
        case VSCP_DAEMON_ACTION_CODE_STORE_MIN:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_STOR_MIN.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") +  _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            doActionStoreMin( pEvent );
            break;
            
        case VSCP_DAEMON_ACTION_CODE_STORE_MAX:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_STOR_MAX.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            doActionStoreMax( pEvent );
            break;    

        case VSCP_DAEMON_ACTION_CODE_START_TIMER:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_START_TIMER.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            doActionStartTimer( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_PAUSE_TIMER:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_PAUSE_TIMER.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            doActionPauseTimer( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_STOP_TIMER:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_STOP_TIMER.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM);
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            doActionStopTimer( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_RESUME_TIMER:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_RESUME_TIMER.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + 
                                _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            doActionResumeTimer( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_WRITE_FILE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_WRITE_FILE.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + 
                                getAsString( false ) + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + 
                                logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            doActionWriteFile( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_GET_PUT_POST_URL:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_GET_PUT_POST_URL.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg( _("[DM] ") +  _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            doActionGetURL( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_WRITE_TABLE: 
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_WRITE_TABLE.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + 
                                getAsString( false ) + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + 
                                logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            {                
                // Write in possible escapes
                wxString strParam = m_actionparam;
                handleEscapes( pEvent, strParam );
                
                actionThread_Table *pThread = 
                        new actionThread_Table( strParam, pEvent );
                if ( NULL == pThread ) return false;
                
                wxThreadError err;
                if (wxTHREAD_NO_ERROR == (err = pThread->Create())) {
                    pThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                    if (wxTHREAD_NO_ERROR != (err = pThread->Run())) {
                        gpobj->logMsg(_("[DM] ") + 
                             _("Unable to run actionThread_Table client thread.\n"),
                            DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                    }
                }
                else {
                    gpobj->logMsg(_("[DM] ") + 
                            _("Unable to create actionThread_Table client thread.\n"),
                            DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                }

            }
            break;
            
        case VSCP_DAEMON_ACTION_CODE_CLEAR_TABLE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = 
                  wxString::Format(_("VSCP_DAEMON_ACTION_CODE_CLEAR_TABLE.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + 
                                getAsString( false ) + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            {
                CVSCPTable *pTable;     // Table object
                wxString wxstrErr;
                
                // Write in possible escapes
                wxString strParam = m_actionparam;
                handleEscapes( pEvent, strParam );
                
                wxStringTokenizer tkz( strParam, _(";") );

                if ( !tkz.HasMoreTokens() ) {
                    // Strange action parameter
                    wxstrErr = _( "[Action] Delete Table: Action parameter "
                                    "is not correct. Parameter= ");
                    wxstrErr += m_actionparam;
                    wxstrErr += _("\n");
                    gpobj->logMsg( _("[DM] ") + wxstrErr, 
                                    DAEMON_LOGMSG_NORMAL, 
                                    DAEMON_LOGTYPE_DM );
                    break;
                }
    
                // Get table name
                wxString name = tkz.GetNextToken();
    
                // Get table object
                if ( NULL == ( pTable = 
                                gpobj->m_userTableObjects.getTable( name ) ) ) {        
                    wxstrErr = _( "[Action] Delete Table: A table with that "
                                  "name was not found. Parameter= ");
                    wxstrErr += m_actionparam;
                    wxstrErr += _("\n");
                    gpobj->logMsg( _("[DM] ") + wxstrErr, 
                                    DAEMON_LOGMSG_NORMAL, 
                                    DAEMON_LOGTYPE_DM );
                    return NULL;        
                }

                if ( !tkz.HasMoreTokens() ) {
            
                    wxString sql = pTable->getSQLDelete();
            
                    // Escapes
                    dmElement::handleEscapes( pEvent, sql );
                    
                    // Do the delete
                    if ( !pTable->executeSQL( sql ) ) {
                        wxstrErr = _( "[Action] Delete Table (internal SQL): "
                                      "Failed SQL= ");
                        wxstrErr += sql;
                        wxstrErr += _("\n");
                        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                                        DAEMON_LOGMSG_NORMAL, 
                                        DAEMON_LOGTYPE_DM );
                    }
                    
                }
                else {
    
                    wxString sql;
                    wxString str = tkz.GetNextToken();
                    
                    if ( !vscp_decodeBase64IfNeeded( sql, str ) ) {
                        gpobj->logMsg( _("[DM] ") + _( "[Action] Write Table: "
                                         "Failed to decode sql string. "
                                         "Will continue anyway."), 
                                        DAEMON_LOGMSG_NORMAL, 
                                        DAEMON_LOGTYPE_DM );
                        sql = str;
                    }
                        
                    // Escapes
                    dmElement::handleEscapes( pEvent, sql );
                    
                    // Do the delete
                    if ( !pTable->executeSQL( sql ) ) {
                        wxstrErr = _( "[Action] Delete Table (Action parameter "
                                      "SQL): Failed SQL= ");
                        wxstrErr += sql;
                        wxstrErr += _("\n");
                        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                                        DAEMON_LOGMSG_NORMAL, 
                                        DAEMON_LOGTYPE_DM );
                    }

                }                
                
            }
            
            break;

        case VSCP_DAEMON_ACTION_CODE_RUN_JAVASCRIPT:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_RUN_JAVASCRIPT.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            { 
                // If BASE64 encoded then decode 
                wxString strParam = m_actionparam;
                strParam.Trim( false );
                if ( strParam.StartsWith( _("BASE64:"), &strParam ) ) {
                    // Yes should be decoded
                    vscp_base64_wxdecode( strParam );
                    if ( 0 == strParam.Length() ) {
                        gpobj->logMsg( _("[DM] ") + "Failed to decode BASE64 "
                                         "parameter (len=0)\n", 
                                            DAEMON_LOGMSG_NORMAL, 
                                            DAEMON_LOGTYPE_DM );
                        break;
                    }
                    
                }
                
                // Write in possible escapes
                handleEscapes( pEvent, strParam );

                actionThread_JavaScript *pThread = 
                        new actionThread_JavaScript( strParam );
                if ( NULL == pThread ) return false;
                
                vscp_convertVSCPtoEx( &pThread->m_feedEvent, 
                                        pEvent );   // Save feed event  

                wxThreadError err;
                if ( wxTHREAD_NO_ERROR == (err = pThread->Create() ) ) {
                    pThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                    if ( wxTHREAD_NO_ERROR != (err = pThread->Run() ) ) {
                        gpobj->logMsg( _("[DM] ") + _("Unable to run "
                                         "actionThread_JavaScript client "
                                         "thread.\n"),
                                            DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                    }
                }
                else {
                    gpobj->logMsg( _("[DM] ") + _("Unable to create "
                                     "actionThread_JavaScript client thread.\n"),
                                       DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                }

            }
            break;
            
        case VSCP_DAEMON_ACTION_CODE_RUN_LUASCRIPT:
            
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_RUN_LUASCRIPT.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("[DM] ") + _("DM = ") + getAsString( false ) + 
                                _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            { 
                // If BASE64 encoded then decode 
                wxString strParam = m_actionparam;
                strParam.Trim( false );
                if ( strParam.StartsWith( _("BASE64:"), &strParam ) ) {
                    // Yes should be decoded
                    vscp_base64_wxdecode( strParam );
                    if ( 0 == strParam.Length() ) {
                        gpobj->logMsg( _("[DM] ") + "Failed to decode BASE64 "
                                         "parameter (len=0)\n", 
                                            DAEMON_LOGMSG_NORMAL, 
                                            DAEMON_LOGTYPE_DM );
                        break;
                    }

                }
                
                // Write in possible escapes
                handleEscapes( pEvent, strParam );
                
                strParam.Trim( false );

                actionThread_Lua *pThread = 
                        new actionThread_Lua( strParam );
                if ( NULL == pThread ) return false;
                
                vscp_convertVSCPtoEx( &pThread->m_feedEvent, 
                                        pEvent );   // Save feed event  

                wxThreadError err;
                if ( wxTHREAD_NO_ERROR == (err = pThread->Create() ) ) {
                    pThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                    if ( wxTHREAD_NO_ERROR != (err = pThread->Run() ) ) {
                        gpobj->logMsg( _("[DM] ") + _("Unable to run "
                                         "actionThread_Lua client "
                                         "thread.\n"),
                                            DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                    }
                }
                else {
                    gpobj->logMsg( _("[DM] ") + _("Unable to create "
                                     "actionThread_Lua client thread.\n"),
                                       DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                }

            }
            break;    

        default:
        case VSCP_DAEMON_ACTION_CODE_NOOP:
            // We do nothing
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_NOOP.\n") ); // Log
                gpobj->logMsg( _("[DM] ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                gpobj->logMsg( _("[DM] ") +  _("DM = ") + 
                                    getAsString( false ) + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("[DM] ") + _("Event = ") + logStr + _("\n"), 
                                DAEMON_LOGMSG_DEBUG, 
                                DAEMON_LOGTYPE_DM );
            }
            break;

    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionExecute
//

bool dmElement::doActionExecute(vscpEvent *pDMEvent, bool bCheckExecutable )
{
    
    // Write in possible escapes
    wxString wxstr = m_actionparam;
    wxstr.Trim(true);
    wxstr.Trim(false);
    handleEscapes(pDMEvent, wxstr);
    
    // Check for bCheckExecutable flag
    if ( wxstr[0] == '!' ) {
        wxstr = wxstr.Right( wxstr.Length() - 1 );
        bCheckExecutable = false;
    }
    
    wxString strfn = m_actionparam;
    bool bOK = true;
    
    if ( bCheckExecutable ) {
        int pos = m_actionparam.First(' ');
        if (wxNOT_FOUND != pos) {
            strfn = m_actionparam.Left(pos);
        }
    }

    // wxExecute breaks if the path does not exist so we have to
    // check that it does.
    if ( bCheckExecutable && 
            ( !wxFileName::FileExists( strfn ) ||
              !wxFileName::IsFileExecutable( strfn ) ) ) {
        gpobj->logMsg( _("[DM] Target does not exist or is not executable") + 
                        _("\n"), 
                        DAEMON_LOGMSG_DEBUG, 
                        DAEMON_LOGTYPE_DM);
        bOK = false;
    }

    //wxString cdir = wxGetCwd();
    //bool rv = wxSetWorkingDirectory(_("c:\\programdata\\vscp\\actions"));
#ifdef WIN32
    if ( bOK && ( ::wxExecute(wxstr, wxEXEC_ASYNC | wxEXEC_HIDE_CONSOLE ) ) ) {
#else
    //if ( bOK && ( ::wxExecute(wxstr, wxEXEC_ASYNC  ) ) ) {
    if ( unixVSCPExecute( wxstr ) ) {
#endif
        if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
            wxString wxstr = _("[Action] Executed: ");
            wxstr += m_actionparam;
            wxstr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), 
                            DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM);
        }
    }
    else {
        // Failed to execute
        m_errorCounter++;
        if ( bOK ) {
            m_strLastError = _("[Action] Failed to execute :");
            gpobj->logMsg( _("[DM] ") + 
                           _("[Action] Failed to execute \n"), 
                            DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM);
        } 
        else {
            m_strLastError = _("File does not exists or is not an executable :");
            gpobj->logMsg( _("[DM] ") + 
                           _("File does not exists or is not an executable \n"), 
                            DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM);
        }
        
        m_strLastError += m_actionparam;
        m_strLastError += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstr, 
                            DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// doActionExecuteConditional
//

bool dmElement::doActionExecuteConditional( vscpEvent *pDMEvent, 
                                            bool bCheckExecutable )
{
    bool bTrigger;
    wxString varName;

    // Write in possible escapes
    wxString escaped_actionparam = m_actionparam;
    handleEscapes( pDMEvent, escaped_actionparam );

    wxStringTokenizer tkz( escaped_actionparam, _(";") );

    // Handle variable
    if ( tkz.HasMoreTokens() ) {

        wxString varname = tkz.GetNextToken();

        CVSCPVariable variable; 
        if ( !gpobj->m_variables.find( varname, variable ) ) {
            
            // Variable was not found - create it
            if ( !gpobj->m_variables.add( varname, 
                                        _("false"), 
                                        VSCP_DAEMON_VARIABLE_CODE_BOOLEAN ) ) {
                wxString logStr = 
                            wxString::Format(_("[Action] Conditional send event: Variable [%s] "
                                        "not defined. Failed to create it.\n"),
                                                (const char *)varName.mbc_str() );
                gpobj->logMsg( _("[DM] ") + logStr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                return false;
            }
            
            // Get the variable
            if ( !gpobj->m_variables.find( varName, variable ) ) {
                
                // Well should not happen - but in case...
                wxString logStr = 
                            wxString::Format(_("[Action] Conditional execute: Variable [%s] "
                                               "was not found (but was created alright).\n"),
                                                (const char *)varName.mbc_str() );
                gpobj->logMsg( _("[DM] ") + logStr, 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                return false;
                
            }
            
        }
        
        if (  VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != variable.getType() ) {
            
            // must be a variable
            wxString wxstrErr = 
                    _("[Action] Conditional execute: "
                      "Condition variable must be boolean ");
            wxstrErr += escaped_actionparam;
            wxstrErr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstrErr, 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
            return false;
            
        }
        
        // Get the value
        variable.getValue( &bTrigger );
        
        // if the variable is false we should do nothing
        if ( !bTrigger ) return false;

    }
    else {
        // must be a condition variable
        wxString wxstrErr = 
                _("[Action] Conditional execute: No variable defined ");
        wxstrErr += escaped_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }

    wxString wxstr;

    // Handle variable
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
    }
    
    // Check for bCheckExecutable flag
    if ( wxstr[0] == '!' ) {
        wxstr = wxstr.Right( wxstr.Length() - 1 );
        bCheckExecutable = false;
    }
    
    wxString strfn = wxstr;
    bool bOK = true;
    
    if ( bCheckExecutable ) {
        int pos = wxstr.First(' ');
        if (wxNOT_FOUND != pos) {
            strfn = wxstr.Left(pos);
        }
    }

    // wxExecute breaks if the path does not exist so we have to
    // check that it does.
    if ( bCheckExecutable && 
            ( !wxFileName::FileExists( strfn ) ||
              !wxFileName::IsFileExecutable( strfn ) ) ) {
        gpobj->logMsg( _("[DM] Conditional execute: Target does not exist or is not executable") + 
                        _("\n"), 
                        DAEMON_LOGMSG_DEBUG, 
                        DAEMON_LOGTYPE_DM);
        bOK = false;
    }

    //wxString cdir = wxGetCwd();
    //bool rv = wxSetWorkingDirectory(_("c:\\programdata\\vscp\\actions"));
#ifdef WIN32
    if ( bOK && ( ::wxExecute(wxstr, wxEXEC_ASYNC | wxEXEC_HIDE_CONSOLE ) ) ) {
#else
    //if ( bOK && ( ::wxExecute(wxstr, wxEXEC_ASYNC  ) ) ) {
    if ( unixVSCPExecute( wxstr ) ) {
#endif
        if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
            wxString wxstr = _("[Action] Conditional Executed: ");
            wxstr += m_actionparam;
            wxstr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), 
                            DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM);
        }
    }
    else {
        // Failed to execute
        m_errorCounter++;
        if ( bOK ) {
            m_strLastError = _("[Action] Failed to conditional execute :");
            gpobj->logMsg( _("[DM] ") + 
                           _("[Action] Failed to execute \n"), 
                            DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM);
        } 
        else {
            m_strLastError = _("File does not exists or is not an executable :");
            gpobj->logMsg( _("[DM] Conditional execute: ") + 
                           _("File does not exists or is not an executable \n"), 
                            DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM);
        }
        
        m_strLastError += m_actionparam;
        m_strLastError += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstr, 
                            DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// unixVSCPExecute
//
// http://stackoverflow.com/questions/21558937/i-do-not-understand-how-execlp-work-in-linux
// http://stackoverflow.com/questions/6718272/c-exec-fork-defunct-processes
// http://beej.us/guide/bgipc/output/html/multipage/fork.html
// https://www.gidforums.com/t-3369.html
//

#ifndef WIN32
bool dmElement::unixVSCPExecute( wxString& argExec )
{
    int pos;
    int cntArgs;
    wxString strPath;
    wxString strExe;
    wxArrayString wxargs;
    char** args;

    wxCmdLineParser cmdParser;

#if wxMAJOR_VERSION >= 3
#ifdef WIN32
    wxargs = cmdParser.ConvertStringToArgs( argExec, wxCMD_LINE_SPLIT_DOS );
#else
    wxargs = cmdParser.ConvertStringToArgs( argExec, wxCMD_LINE_SPLIT_UNIX );
#endif
#else
    wxargs = cmdParser.ConvertStringToArgs( argExec );
#endif
    cntArgs = wxargs.Count();

    if ( !cntArgs ) return false;   // Must at least have an exec file

    // First parameter is executable
    strPath = wxargs[ 0 ];

    // If it contains a path we need to pick out the executable name
    if ( wxNOT_FOUND != ( pos = strPath.Find( '/', true ) ) ) {
        strExe = strPath.Right( strPath.Length()-pos-1 );
        wxargs[ 0 ] = strExe;
    }
    else {
        strExe = strPath;
    }

    pid_t pid = fork();

    if( pid == 0 ) {

        fclose(stdin);
        fclose(stdout);
        fclose(stderr);
        open("/dev/null", O_RDONLY); // stdin
        open("/dev/null", O_WRONLY); // stdout
        open("/dev/null", O_WRONLY); // stderr

        // Create the args
        args = (char **)malloc( (cntArgs + 1) * sizeof(char *) );
        for ( int i=0; i<cntArgs; i++ ) {
            args[ i ] = ( char *)malloc( wxargs[i].Length() + 1 );
            strcpy( args[ i ], (const char*)wxargs[i].mbc_str() );
        }

        // Last is NULL pointer
        args[ cntArgs ] = NULL;

        int rc = execvp( (const char*)strPath.mbc_str(), args );

        // Here only if execvp fails

        // free allocated resources before terminating
        for ( int i=0; i<cntArgs; i++ ) {
            free( args[ i ] );
            args[ i ] = NULL;
        }

        free( args );

        // Terminate
        exit( rc );
    }
    else if ( -1 == pid ) {

        wxargs.Clear();

        // fork error
        return false;
    }
    else {

        wxargs.Clear();

        // OK
        return true;
    }
}
#endif


///////////////////////////////////////////////////////////////////////////////
// doActionSendEvent
//

bool dmElement::doActionSendEvent( vscpEvent *pDMEvent )
{
    //int idx;
    wxString wxstr;
    wxString strEvent;
    wxString varName;

    // Write in possible escapes
    wxString escaped_actionparam = m_actionparam;
    handleEscapes( pDMEvent, escaped_actionparam );
    
    wxStringTokenizer tkz( escaped_actionparam, _(";") );

    // Get event
    if ( tkz.HasMoreTokens() ) {
        strEvent = tkz.GetNextToken();
    }
    else {
        // must be an event
        wxString wxstrErr = 
                _("[Action] Send event: No event defined. Param = ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Get confirmation variable (if any)
    if ( tkz.HasMoreTokens() ) {
        varName = tkz.GetNextToken();
    }

    // There must be room in the send queue
    if ( gpobj->m_maxItemsInClientReceiveQueue >
            gpobj->m_clientOutputQueue.GetCount() ) {

        vscpEvent *pEvent = new vscpEvent;
        
        if ( NULL != pEvent ) {

            pEvent->pdata = NULL;

            if ( !vscp_setVscpEventFromString( pEvent, strEvent ) ) {
                    
                vscp_deleteVSCPevent_v2( &pEvent );
                    
                // Event has wrong format
                wxString wxstrErr = 
                        _("[Action] Send event: Invalid format for "
                          "event. Param = ");
                wxstrErr += wxstr;
                wxstrErr += _("\n");
                gpobj->logMsg( _("[DM] ") + wxstrErr, 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                return false;
                    
            }

            gpobj->m_mutexClientOutputQueue.Lock();
            gpobj->m_clientOutputQueue.Append ( pEvent );
            gpobj->m_semClientOutputQueue.Post();
            gpobj->m_mutexClientOutputQueue.Unlock();

            // TX Statistics
            m_pDM->m_pClientItem->m_statistics.cntTransmitData +=
                                                            pEvent->sizeData;
            m_pDM->m_pClientItem->m_statistics.cntTransmitFrames++;

            // Set the condition variable to false if it is defined
            if ( varName.Length() ) {

                CVSCPVariable variable;
                if ( !gpobj->m_variables.find( varName, variable ) ) {
                    
                    // Variable was not found - create it
                    if ( !gpobj->m_variables.add( varName, 
                                        _("false"), 
                                        VSCP_DAEMON_VARIABLE_CODE_BOOLEAN ) ) {
                        wxString logStr = 
                                wxString::Format(_("[Action] Send event: Variable [%s] "
                                        "not defined. Failed to create it.\n"),
                                                (const char *)varName.mbc_str() );
                        gpobj->logMsg( _("[DM] ") + logStr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                        return false;
                    }
                    
                    // Get the variable
                    if ( !gpobj->m_variables.find( varName, variable ) ) {
                
                        // Well should not happen - but in case...
                        wxString logStr = 
                            wxString::Format(_("[Action] Check measurement: Variable [%s] "
                                               "was not found (but was created alright).\n"),
                                                (const char *)varName.mbc_str() );
                        gpobj->logMsg( _("[DM] ") + logStr, 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                        return false;
                
                    }
                    
                    
                }
                        
                // Set it to true
                variable.setValue( true );
                        
                // Update the variable
                if ( !gpobj->m_variables.update( variable ) ) {
                    wxString wxstrErr = 
                                _("[Action] Send event: Failed to update "
                                  "variable.");
                    wxstrErr += wxstr;
                    wxstrErr += _("\n");
                    gpobj->logMsg( _("[DM] ") + wxstrErr, 
                                        DAEMON_LOGMSG_NORMAL, 
                                        DAEMON_LOGTYPE_DM );
                    return false;
                }                        
          
            }
        }

    }
    else {        
        m_pDM->m_pClientItem->m_statistics.cntOverruns++;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionSendEventConditional
//

bool dmElement::doActionSendEventConditional( vscpEvent *pDMEvent )
{
    bool bTrigger = false;
    wxString varName;    

    // Write in possible escapes
    wxString escaped_actionparam = m_actionparam;
    handleEscapes( pDMEvent, escaped_actionparam );

    wxStringTokenizer tkz( escaped_actionparam, _(";") );
    
    // Handle variable
    if ( tkz.HasMoreTokens() ) {

        wxString varname = tkz.GetNextToken();

        CVSCPVariable variable; 
        if ( !gpobj->m_variables.find( varname, variable ) ) {
            
            // Variable was not found - create it
            if ( !gpobj->m_variables.add( varname, 
                                        _("false"), 
                                        VSCP_DAEMON_VARIABLE_CODE_BOOLEAN ) ) {
                wxString logStr = 
                            wxString::Format(_("[Action] Conditional send event: Variable [%s] "
                                        "not defined. Failed to create it.\n"),
                                                (const char *)varName.mbc_str() );
                gpobj->logMsg( _("[DM] ") + logStr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                return false;
            }
            
            // Get the variable
            if ( !gpobj->m_variables.find( varName, variable ) ) {
                
                // Well should not happen - but in case...
                wxString logStr = 
                            wxString::Format(_("[Action] Check measurement: Variable [%s] "
                                               "was not found (but was created alright).\n"),
                                                (const char *)varName.mbc_str() );
                gpobj->logMsg( _("[DM] ") + logStr, 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                return false;
                
            }
            
        }
        
        if (  VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != variable.getType() ) {
            
            // must be a variable
            wxString wxstrErr = 
                    _("[Action] Conditional send event: "
                      "Condition variable must be boolean ");
            wxstrErr += escaped_actionparam;
            wxstrErr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstrErr, 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
            return false;
            
        }
        
        // Get the value
        variable.getValue( &bTrigger );
        
        // if the variable is false we should do nothing
        if ( !bTrigger ) return false;

    }
    else {
        // must be a condition variable
        wxString wxstrErr = 
                _("[Action] Conditional send event: No variable defined ");
        wxstrErr += escaped_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }

    vscpEvent *pEvent = new vscpEvent;
    pEvent->pdata = NULL;

    // We must have an event to send
    if ( tkz.HasMoreTokens() ) {

        wxString strEvent = tkz.GetNextToken();
     
        if ( !vscp_setVscpEventFromString( pEvent, strEvent ) ) {
            
            vscp_deleteVSCPevent_v2( &pEvent );
            
            // Could not parse event string
            wxString wxstrErr = 
                    _("[Action] Conditional send event: Unable to parse event ");
            wxstrErr += escaped_actionparam;
            wxstrErr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstrErr, 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
            return false;
        }
    }
    else {
        // must be an event
        wxString wxstrErr = _("[Action] Conditional send event: No event defined ");
        wxstrErr += escaped_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }

    // Get confirmation variable (if any)
    if ( tkz.HasMoreTokens() ) {
        varName = tkz.GetNextToken();
    }

    // There must be room in the send queue
    if ( gpobj->m_maxItemsInClientReceiveQueue >
        gpobj->m_clientOutputQueue.GetCount() ) {

            gpobj->m_mutexClientOutputQueue.Lock();
            gpobj->m_clientOutputQueue.Append ( pEvent );
            gpobj->m_semClientOutputQueue.Post();
            gpobj->m_mutexClientOutputQueue.Unlock();

            // TX Statistics
            m_pDM->m_pClientItem->m_statistics.cntTransmitData += 
                                                              pEvent->sizeData;
            m_pDM->m_pClientItem->m_statistics.cntTransmitFrames++;

            // Set the condition variable to false if it is defined
            if ( varName.Length() ) {

                CVSCPVariable variable;
                if ( gpobj->m_variables.find( varName, variable ) ) {
                        
                    // Set it to true
                    variable.setValue( true );
                        
                    // Update the variable
                    if ( !gpobj->m_variables.update( variable ) ) {
                        wxString wxstrErr = 
                                _("[Action] Send event: Failed to update "
                                  "variable.");
                        wxstrErr += escaped_actionparam;
                        wxstrErr += _("\n");
                        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                                        DAEMON_LOGMSG_NORMAL, 
                                        DAEMON_LOGTYPE_DM );
                        return false;
                    }
                        
                }
                else {
                    wxString wxstrErr = 
                            _("[Action] Send event: Confirmation variable "
                              "was not found.");
                    wxstrErr += escaped_actionparam;
                    wxstrErr += _("\n");
                    gpobj->logMsg( _("[DM] ") + wxstrErr, 
                                    DAEMON_LOGMSG_NORMAL, 
                                    DAEMON_LOGTYPE_DM );
                    return false;
                }
            }
    }
    else {
        vscp_deleteVSCPevent_v2( &pEvent );
        m_pDM->m_pClientItem->m_statistics.cntOverruns++;
    }

    return true;

}

///////////////////////////////////////////////////////////////////////////////
// doActionSendEventsFromFile
//

bool dmElement::doActionSendEventsFromFile( vscpEvent *pDMEvent )
{
    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    // File must exist
    if ( !wxFile::Exists( wxstr ) ) {
        wxString wxstrErr = 
                _("[Action] Send event from file: Non existent file  ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }

    wxXmlDocument doc;
    if ( !doc.Load ( wxstr ) ) {
        wxString wxstrErr = 
                _("[Action] Send event from file: Failed to load "
                  "event XML file  ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }

    // start processing the XML file
    if ( doc.GetRoot()->GetName() != _( "events" ) ) {
        wxString wxstrErr = _("[Action] Send event from file: "
                              "<events> tag is missing.");
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while ( child ) {

        if ( child->GetName() == _( "event" ) ) {

            // Set defaults for event
            vscpEvent *pEvent = new vscpEvent;

            if ( NULL != pEvent ) {

                pEvent->head = 0;
                pEvent->obid = 0;
                pEvent->timestamp = 0;
                vscp_setEventDateTimeBlockToNow( pEvent );
                pEvent->sizeData = 0;
                pEvent->pdata = NULL;
                pEvent->vscp_class = 0;
                pEvent->vscp_type = 0;
                memset( pEvent->GUID, 0, 16 );

                wxXmlNode *subchild = child->GetChildren();
                while ( subchild ) {

                    if ( subchild->GetName() == _( "head" ) ) {
                        pEvent->head = 
                                vscp_readStringValue( subchild->GetNodeContent() );
                    }
                    else if ( subchild->GetName() == _( "class" ) ) {
                        pEvent->vscp_class = 
                                vscp_readStringValue( subchild->GetNodeContent() );
                    }
                    else if ( subchild->GetName() == _( "type" ) ) {
                        pEvent->vscp_type = 
                                vscp_readStringValue( subchild->GetNodeContent() );
                    }
                    else if ( subchild->GetName() == _( "guid" ) ) {
                        vscp_getGuidFromString( pEvent, 
                                                 subchild->GetNodeContent() );
                    }
                    if ( subchild->GetName() == _( "data" ) ) {
                        vscp_setVscpEventDataFromString( pEvent, 
                                                     subchild->GetNodeContent() );
                    }
                    
                    subchild = subchild->GetNext();

                }

                // ==============
                // Send the event
                // ==============

                // There must be room in the send queue
                if ( gpobj->m_maxItemsInClientReceiveQueue >
                    gpobj->m_clientOutputQueue.GetCount() ) {

                        gpobj->m_mutexClientOutputQueue.Lock();
                        gpobj->m_clientOutputQueue.Append ( pEvent );
                        gpobj->m_semClientOutputQueue.Post();
                        gpobj->m_mutexClientOutputQueue.Unlock();

                        // TX Statistics
                        m_pDM->m_pClientItem->m_statistics.cntTransmitData += 
                                                                pEvent->sizeData;
                        m_pDM->m_pClientItem->m_statistics.cntTransmitFrames++;

                }
                else {
                    
                    // Remove the event
                    vscp_deleteVSCPevent_v2( &pEvent );
                    
                    m_pDM->m_pClientItem->m_statistics.cntOverruns++;
                }
                

            }                        

        }
        
        child = child->GetNext();

    }

    return true;

}


///////////////////////////////////////////////////////////////////////////////
// doActionSendEventRemote
//

bool dmElement::doActionSendEventRemote( vscpEvent *pDMEvent, bool bSecure )
{
    wxString strHostname;
    short port;
    wxString strUsername;
    wxString strPassword;
    wxString strEvent;                                        
    
    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    wxStringTokenizer tkz( wxstr, _(";") );

    // Get servername
    if ( tkz.HasMoreTokens() ) {
        strHostname = tkz.GetNextToken();
    }
    else {
        return false;
    }
    
    // Get port
    if ( tkz.HasMoreTokens() ) {
        port = vscp_readStringValue( tkz.GetNextToken() );
    }
    else {
        return false;
    }
    
    // Get username
    if ( tkz.HasMoreTokens() ) {
        strUsername = tkz.GetNextToken();
    }
    else {
        return false;
    }
    
    // Get password
    if ( tkz.HasMoreTokens() ) {
        strPassword = tkz.GetNextToken();
    }
    else {
        return false;
    }
    
    // Get event
    if ( tkz.HasMoreTokens() ) {
        strEvent = tkz.GetNextToken();
    }
    else {
        return false;
    }
    
    // Go do your work mate
    actionThread_VSCPSrv *thread =
            new actionThread_VSCPSrv( gpobj,
                                        strHostname,
                                        port,
                                        strUsername,
                                        strPassword,
                                        strEvent );
    if ( NULL == thread ) return false;

    // Go Go Go
    thread->Run();
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionWriteFile
//

bool dmElement::doActionWriteFile( vscpEvent *pDMEvent )
{
    wxFile f;
    wxString path;
    wxString strOut;
    bool bAppend = true;

    // Write in possible escapes
    wxString escaped_param = m_actionparam;
    handleEscapes( pDMEvent, escaped_param );

    wxStringTokenizer tkz( escaped_param, _(";") );

    // Handle path
    if ( tkz.HasMoreTokens() ) {
        path = tkz.GetNextToken();
    }
    else {
        // Must have a path
        wxString wxstrErr = _("[Action] Write to file: No path to file given  ");
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // Handle append/overwrite flag
    if ( tkz.HasMoreTokens() ) {
        if ( !vscp_readStringValue( tkz.GetNextToken() ) ) {
            bAppend = false;
        }
    }
    
    // Get string to write
    if ( tkz.HasMoreTokens() ) {
        strOut = tkz.GetNextToken();
    }

    if ( f.Open( path, ( bAppend ? wxFile::write_append : wxFile::write ) ) ) {

        f.Write( strOut );
        f.Flush();
        f.Close();

    }
    else {
        // Failed to open file
        wxString wxstrErr = _("[Action] Write to file: Failed to open file ");
        wxstrErr += path;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionGetURL
//

bool dmElement::doActionGetURL( vscpEvent *pDMEvent )
{
    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    wxStringTokenizer tkz( wxstr, _(";") );
    if ( !tkz.HasMoreTokens() ) {
        // Action parameter is wrong
        wxString wxstrErr = _("[Action] Get URL: Wrong action parameter (method;URL required");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // Access method
    actionThread_URL::accessmethod_t nAccessMethod = actionThread_URL::GET;
    wxString access = tkz.GetNextToken();
    access.MakeUpper();
    if ( wxNOT_FOUND != access.Find(_("PUT") ) ) {
        nAccessMethod = actionThread_URL::PUT;
    }
    else if ( wxNOT_FOUND != access.Find(_("POST") ) ) {
        nAccessMethod = actionThread_URL::POST;
    }

    // Get URL
    wxString url;
    if ( tkz.HasMoreTokens() ) {
        url = tkz.GetNextToken();
    }
    else {
        // URL is required
        wxString wxstrErr = _("[Action] Get URL: Wrong action parameter (URL required)");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // Get POST/PUT data
    wxString putdata;
    if ( tkz.HasMoreTokens() ) {
        putdata = tkz.GetNextToken();
    }

    // Get extra headers
    wxString extraheaders;
    if ( tkz.HasMoreTokens() ) {
        extraheaders = tkz.GetNextToken();
    }

    // Get proxy
    wxString proxy;
    if ( tkz.HasMoreTokens() ) {
        proxy = tkz.GetNextToken();
    }

    // Go do your work mate
    actionThread_URL *thread =
            new actionThread_URL( gpobj,
                                    url,
                                    nAccessMethod,
                                    putdata,
                                    extraheaders,
                                    proxy );
    if ( NULL == thread ) return false;

    // Go Go Go
    thread->Run();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionStoreVariable
//

bool dmElement::doActionStoreVariable( vscpEvent *pDMEvent )
{
    // Write in possible escapes
    wxString params = m_actionparam;
    handleEscapes( pDMEvent, params );
    
    wxStringTokenizer tkz( params, _(";") );
    
    CVSCPVariable var;
    
    if ( tkz.CountTokens() >= 6 ) {

        // The form is variable-name; variable-type; persistence; value
        
        if ( !( var.setVariableFromString( params ) ) ) {
            // must be a variable
            wxString wxstrErr = _("[Action] Store Variable: Could not set new variable ");
            wxstrErr += params;
            wxstrErr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
            
        }

        if ( !gpobj->m_variables.add( var ) ) {
            // must be a variable
            wxString wxstrErr = _("[Action] Store Variable: Could not add variable ");
            wxstrErr += params;
            wxstrErr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
        }
    
    }
    else {
        
        // variablename; type; persistence
        uint16_t varType = VSCP_DAEMON_VARIABLE_CODE_STRING;
        bool bPersistent = false;
        
        // We must have the name 
        if ( !tkz.HasMoreTokens() ) return false;
        wxString varName = tkz.GetString();
        
        // Variable type
        if ( tkz.HasMoreTokens() ) {
            varType = vscp_readStringValue( tkz.GetString() );
        }
        
        // Persistence
        if ( tkz.HasMoreTokens() ) {
            wxString wxstr = tkz.GetString();
            wxstr.MakeUpper();
            if ( wxNOT_FOUND != wxstr.Find(_("TRUE") ) ) {
                bPersistent = true;
            }
        }
        
        if ( !gpobj->m_variables.find( varName, var ) ) {
            
            // The variable was not found - it should be added
            var.setName( varName );
            var.setType( varType );
            var.setPersistent( bPersistent );
            
        }
        
        
        
        // Add/create the variable
        if ( !gpobj->m_variables.add( var ) ) {
            // must be a variable
            wxString wxstrErr = _("[Action] Store Variable: Could not add variable ");
            wxstrErr += params;
            wxstrErr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
        }
        
        switch ( var.getType() ) {
            
            case VSCP_DAEMON_VARIABLE_CODE_STRING:
                if ( vscp_isVSCPMeasurement( pDMEvent ) ) {
                    wxString strValue;
                    if ( vscp_getVSCPMeasurementAsString( pDMEvent, strValue ) ) {
                        var.setValue( strValue );
                    }
                    else {
                        wxString wxstrErr = _("[Action] Store Variable: Failed to convert value to string ");
                        wxstrErr += params;
                        wxstrErr += _("\n");
                        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                        return false;
                    }
                }
                break;
                                
            case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
                if ( vscp_isVSCPMeasurement( pDMEvent ) ) {
                    double value;
                    if ( vscp_getVSCPMeasurementAsDouble( pDMEvent, &value ) ) {
                        var.setValue( (int)value );
                    }
                    else {
                        wxString wxstrErr = _("[Action] Store Variable: Failed to convert value to double ");
                        wxstrErr += params;
                        wxstrErr += _("\n");
                        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                        return false;
                    }
                }
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_LONG:
                if ( vscp_isVSCPMeasurement( pDMEvent ) ) {
                    double value;
                    if ( vscp_getVSCPMeasurementAsDouble( pDMEvent, &value ) ) {
                        var.setValue( (long)value );
                    }
                    else {
                        wxString wxstrErr = _("[Action] Store Variable: Failed to convert value to double ");
                        wxstrErr += params;
                        wxstrErr += _("\n");
                        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                        return false;
                    }
                }
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
                if ( vscp_isVSCPMeasurement( pDMEvent ) ) {
                    double value;
                    if ( vscp_getVSCPMeasurementAsDouble( pDMEvent, &value ) ) {
                        var.setValue( value );
                    }
                    else {
                        wxString wxstrErr = _("[Action] Store Variable: Failed to convert value to double ");
                        wxstrErr += params;
                        wxstrErr += _("\n");
                        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                        return false;
                    }
                }
                break;    
            
            case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
                if ( vscp_isVSCPMeasurement( pDMEvent ) ) {
                    
                    double value;
                    uint8_t unit = 0;
                    uint8_t sensor_index = 0;
                    uint8_t zone = 0;
                    uint8_t subzone = 0;
                    
                    if ( -1 == ( unit = vscp_getVSCPMeasurementUnit( pDMEvent ) ) ) {
                        unit = 0;
                    }
                    
                    if ( -1 == ( sensor_index = vscp_getVSCPMeasurementSensorIndex( pDMEvent ) ) ) {
                        sensor_index = 0;
                    }
                    
                    if ( -1 == ( zone = vscp_getVSCPMeasurementZone( pDMEvent ) ) ) {
                        zone = 0;
                    }
                    
                    if ( -1 == ( subzone = vscp_getVSCPMeasurementSubZone( pDMEvent ) ) ) {
                        subzone = 0;
                    }
                    
                    if ( vscp_getVSCPMeasurementAsDouble( pDMEvent, &value ) ) {
                        
                        // (MEASUREMENT|6;true|false;)value;unit;sensor-index;zone;subzone
                        wxString strValue = wxString::Format( _("%lf;%d;%d;%d;%d"), 
                                                                    value,
                                                                    unit,
                                                                    sensor_index,
                                                                    zone,
                                                                    subzone );
                        var.setValue( strValue );
                    }
                    else {
                        wxString wxstrErr = _("[Action] Store Variable: Failed to convert value to double ");
                        wxstrErr += params;
                        wxstrErr += _("\n");
                        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                        return false;
                    }                    
                    
                }
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
                {
                    wxString strEvent;
                    if ( vscp_writeVscpEventToString( pDMEvent, strEvent ) ) {
                        var.setValue( strEvent );
                    }
                }
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
                {
                    wxString wxstr;
                    cguid guid;
                    guid.getFromArray( pDMEvent->GUID );
                    guid.toString( wxstr );
                    var.setValue( wxstr );
                }
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
                {
                    wxString strData;
                    if ( vscp_writeVscpDataToString( pDMEvent, strData ) ) {
                        var.setValue( strData );
                    }
                }
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
                var.setValue( pDMEvent->vscp_class );    
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
                var.setValue( pDMEvent->vscp_type );
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
                var.setValue( (double)pDMEvent->timestamp );
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
                {
                    wxDateTime dt = wxDateTime::Now();
                    var.setValue( dt.FormatISOCombined() );
                }
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_DATE:
                {
                    wxDateTime dt = wxDateTime::Now();
                    var.setValue( dt.FormatISODate() );
                }                
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_TIME:
                {
                    wxDateTime dt = wxDateTime::Now();
                    var.setValue( dt.FormatISOTime() );
                }                
                break;
                           
            case VSCP_DAEMON_VARIABLE_CODE_HTML:
                {
                    wxString strData;
                    vscp_writeVscpDataToString( pDMEvent, strData );
                    
                    wxString strGUID;
                    vscp_writeGuidToString( pDMEvent, strGUID );
                    
                    wxString wxstr = wxString::Format( VSCP_HTML_EVENT_TEMPLATE,
                                                        wxDateTime::Now().FormatISOCombined(),
                                                        pDMEvent->vscp_class,
                                                        pDMEvent->vscp_type,
                                                        pDMEvent->sizeData,
                                                        (const char *)strData.mbc_str(),
                                                        (const char *)strGUID.mbc_str(),
                                                        pDMEvent->head,
                                                        pDMEvent->timestamp,
                                                        pDMEvent->obid,
                                                        "" );
                    var.setValue( wxstr );
                }
                break;
                             
            case VSCP_DAEMON_VARIABLE_CODE_JSON:
                {
                    wxString strData;
                    vscp_writeVscpDataToString( pDMEvent, strData );
                    
                    wxString strGUID;
                    vscp_writeGuidToString( pDMEvent, strGUID );
                    
                    wxString wxstr = wxString::Format( VSCP_JSON_EVENT_TEMPLATE,
                                                        wxDateTime::Now().FormatISOCombined(),
                                                        pDMEvent->head,
                                                        pDMEvent->timestamp,
                                                        pDMEvent->obid,
                                                        pDMEvent->vscp_class,
                                                        pDMEvent->vscp_type,
                                                        (const char *)strGUID.mbc_str(),
                                                        (const char *)strData.mbc_str(),
                                                        "" );
                    var.setValue( wxstr );
                }
                break;
                
            case VSCP_DAEMON_VARIABLE_CODE_XML:
                {
                    wxString strData;
                    vscp_writeVscpDataToString( pDMEvent, strData );
                    
                    wxString strGUID;
                    vscp_writeGuidToString( pDMEvent, strGUID );
                    
                    wxString wxstr = wxString::Format( VSCP_XML_EVENT_TEMPLATE,
                                                        wxDateTime::Now().FormatISOCombined(),
                                                        pDMEvent->head,
                                                        pDMEvent->timestamp,
                                                        pDMEvent->obid,
                                                        pDMEvent->vscp_class,
                                                        pDMEvent->vscp_type,
                                                        (const char *)strGUID.mbc_str(),
                                                        pDMEvent->sizeData,
                                                        (const char *)strData.mbc_str() );
                    var.setValue( wxstr );
                }
                break;
                
        }
    
    
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionAddVariable
//

bool dmElement::doActionAddVariable( vscpEvent *pDMEvent )
{
    CVSCPVariable variable;
    wxString strName;
    long val = 0;
    double floatval = 0.0;

    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    // Get the variable name
    strName = wxstr.BeforeFirst( wxChar(';') );

    // Get the value
    wxString strval = wxstr.AfterFirst( wxChar(';') );

    if ( 0 == gpobj->m_variables.find( strName, variable ) ) {
        wxString wxstrErr = _("[Action] Add to Variable: Variable was not found ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Must be a numerical variable
    if ( !variable.isNumerical() ) {
        wxString wxstrErr = _("[Action] Add to Variable: Variable is not numerical ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG == variable.getType() ) ) {
        long lval;
        val = atol( strval );
        variable.getValue( &lval );        
        lval += val;
        variable.setValue( lval );
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == variable.getType() ) ) {
        long lval;
        val = atol( strval );
        variable.getValue( &lval );
        lval += val;
        variable.setValue( lval );
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == variable.getType() ) ) {
        double dval;
        strval.ToDouble( &floatval );
        variable.getValue( &dval );
        dval += floatval;
        variable.setValue( dval );
    }
    
    if ( !gpobj->m_variables.update( variable ) ) {
        wxString wxstrErr = _("[Action] Add to Variable: Failed to update variable ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionSubtractVariable
//

bool dmElement::doActionSubtractVariable( vscpEvent *pDMEvent )
{
    CVSCPVariable variable;
    wxString strName;
    long val = 0;
    double floatval = 0.0;

    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    // Get the variable name
    strName = wxstr.BeforeFirst( wxChar(';') );

    // Get the value
    wxString strval = wxstr.AfterFirst( wxChar(';') );

    if ( 0 == gpobj->m_variables.find( strName, variable ) ) {
        wxString wxstrErr = _("[Action] Subtract from Variable: Variable was not found ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Must be a numerical variable
    if ( !variable.isNumerical() ) {
        wxString wxstrErr = _("[Action] Subtract from Variable: Variable is not numerical ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG == variable.getType() ) ) {
        long lval;
        val = atol( strval );
        variable.getValue( &lval );        
        lval -= val;
        variable.setValue( lval );
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == variable.getType() ) ) {
        long lval;
        val = atol( strval );
        variable.getValue( &lval );
        lval -= val;
        variable.setValue( lval );
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == variable.getType() ) ) {
        double dval;
        strval.ToDouble( &floatval );
        variable.getValue( &dval );
        dval -= floatval;
        variable.setValue( dval );
    }
    
    if ( !gpobj->m_variables.update( variable ) ) {
        wxString wxstrErr = _("[Action] Subtract from Variable: Failed to update variable ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionMultiplyVariable
//

bool dmElement::doActionMultiplyVariable( vscpEvent *pDMEvent )
{
    CVSCPVariable variable;
    wxString strName;
    long val = 0;
    double floatval = 0.0;

    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    // Get the variable name
    strName = wxstr.BeforeFirst( wxChar(';') );

    // Get the value
    wxString strval = wxstr.AfterFirst( wxChar(';') );

    if ( 0 == gpobj->m_variables.find( strName, variable ) ) {
        wxString wxstrErr = _("[Action] Multiply Variable: Variable was not found ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Must be a numerical variable
    if ( !variable.isNumerical() ) {
        wxString wxstrErr = _("[Action] Multiply Variable: Variable is not numerical ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG == variable.getType() ) ) {
        long lval;
        val = atol( strval );
        variable.getValue( &lval );        
        lval *= val;
        variable.setValue( lval );
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == variable.getType() ) ) {
        long lval;
        val = atol( strval );
        variable.getValue( &lval );
        lval *= val;
        variable.setValue( lval );
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == variable.getType() ) ) {
        double dval;
        strval.ToDouble( &floatval );
        variable.getValue( &dval );
        dval *= floatval;
        variable.setValue( dval );
    }
    
    if ( !gpobj->m_variables.update( variable ) ) {
        wxString wxstrErr = _("[Action] Multiply Variable: Failed to update variable ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionDivideVariable
//

bool dmElement::doActionDivideVariable( vscpEvent *pDMEvent )
{
    CVSCPVariable variable;
    wxString strName;
    long val = 0;
    double floatval = 0.0;

    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    // Get the variable name
    strName = wxstr.BeforeFirst( wxChar(';') );

    // Get the value
    wxString strval = wxstr.AfterFirst( wxChar(';') );

    if ( 0 == gpobj->m_variables.find( strName, variable ) ) {
        wxString wxstrErr = _("[Action] Divide Variable: Variable was not found ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Must be a numerical variable
    if ( !variable.isNumerical() ) {
        wxString wxstrErr = _("[Action] Divide Variable: Variable is not numerical ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG == variable.getType() ) ) {
        long lval;
        val = atol( strval );
        if ( 0 == val ) {
            wxString wxstrErr = _("[Action] Can't divide with zero ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            val = 1;
        }
        variable.getValue( &lval );        
        lval /= val;
        variable.setValue( lval );
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == variable.getType() ) ) {
        long lval;
        val = atol( strval );
        if ( 0 == val ) {
            wxString wxstrErr = _("[Action] Can't divide with zero ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            val = 1;
        }
        variable.getValue( &lval );
        lval /= val;
        variable.setValue( lval );
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == variable.getType() ) ) {
        double dval;
        strval.ToDouble( &floatval );
        if ( 0 == floatval ) {
            wxString wxstrErr = _("[Action] Can't divide with zero ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            floatval = 1;
        }
        variable.getValue( &dval );
        dval /= floatval;
        variable.setValue( dval );
    }
    
    if ( !gpobj->m_variables.update( variable ) ) {
        wxString wxstrErr = _("[Action] Divide Variable: Failed to update variable ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionCheckVariable
//

bool dmElement::doActionCheckVariable( vscpEvent *pDMEvent, VariableCheckType type )
{
    wxString wxstr;
    double value = 0;
    uint8_t operation = DM_MEASUREMENT_COMPARE_NOOP;
    CVSCPVariable varCompare;   // Variable to compare
    CVSCPVariable varFlag;      // Variable with flag
    
    // Write in possible escapes
    wxString params = m_actionparam;
    handleEscapes( pDMEvent, params );

    // value;unit;operation;variable;flag-variable
    wxStringTokenizer tkz( params, _(";") );
    
    // Value
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( !wxstr.ToCDouble( &value ) ) {
            value = 0;
            wxstr = _("[Action] Failed to convert to double (set to zero). param = ") + params;
            gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        }
    }
    else {
        wxstr = _("[Action] Missing needed parameter (value). param = ") + params;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // Operation
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        operation = CDM::getCompareCodeFromToken( wxstr );
        if ( -1 == operation ) {
            wxstr = _("[Action] Invalid compare operation. param = ") + params;
            gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            operation = DM_MEASUREMENT_COMPARE_NOOP;
        }
    }
    else {
        wxstr = _("[Action] Missing needed parameter (operation). param = ") + params;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Variable to compare
    if ( tkz.HasMoreTokens() ) {
        
        wxstr = tkz.GetNextToken();
        wxstr.Trim();
        
        if ( !gpobj->m_variables.find( wxstr, varCompare ) ) {
            // Variable not found
            wxstr = _("[Action] Compare variable was not found. param = ") + params;
            gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
        }
        
        // The variable must be numerical
        if ( !varCompare.isNumerical() ) {
            wxstr = _("[Action] Compare variable need to be numerical. param = ") + params;
            gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
        }
        
    }
    else {
        wxstr = _("[Action] Missing needed parameter (variable). param = ") + params;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Variable with flag
    if ( tkz.HasMoreTokens() ) {
        
        wxString varName = tkz.GetNextToken();
        varName.Trim();
        if ( !gpobj->m_variables.find( varName, varFlag ) ) {
            
            // Get the variable
            if ( !gpobj->m_variables.find( varName, varFlag ) ) {
                
                // Well should not happen - but in case...
                wxString logStr = 
                            wxString::Format(_("[Action] Check measurement: Variable [%s] "
                                               "was not found (but was created alright).\n"),
                                                (const char *)varName.mbc_str() );
                gpobj->logMsg( _("[DM] ") + logStr, 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                return false;
                
            }
            
        }
        
        // The variable must be boolean
        if ( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != varFlag.getType() ) {
            wxstr = _("[Action] Flag variable must be boolean. param = ") + params;
            gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
        }
        
    }
    else {
        wxstr = _("[Action] Missing needed parameter (flag variable). param =") + params;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    double valCompare;
    varCompare.getValue( &valCompare );
    
    switch ( operation ) {
        
        case DM_MEASUREMENT_COMPARE_NOOP:
            if ( VARIABLE_CHECK_SET_TRUE == type ) {
                varFlag.setValue( true );
            }
            else if ( VARIABLE_CHECK_SET_FALSE == type ) {
                varFlag.setValue( false );
            }
            break;
        
        case DM_MEASUREMENT_COMPARE_EQ:
            if ( vscp_almostEqualRelativeDouble( value, valCompare ) ) {
                if ( VARIABLE_CHECK_SET_TRUE ==  type ) {
                    varFlag.setValue( true );
                }
                else if ( VARIABLE_CHECK_SET_FALSE ==  type ) {
                    varFlag.setValue( false );
                }
                else if ( VARIABLE_CHECK_SET_OUTCOME ==  type ) {
                    varFlag.setValue( true );
                }
            }
            else {
                if ( VARIABLE_CHECK_SET_TRUE ==  type ) {
                    varFlag.setValue( false );
                }
                else if ( VARIABLE_CHECK_SET_FALSE ==  type ) {
                    varFlag.setValue( true );
                }
                else if ( VARIABLE_CHECK_SET_OUTCOME ==  type ) {
                    varFlag.setValue( false );
                }
            }
            break;
            
        case DM_MEASUREMENT_COMPARE_NEQ:
            if ( !vscp_almostEqualRelativeDouble( value, valCompare ) ) {
                if ( VARIABLE_CHECK_SET_TRUE ==  type ) {
                    varFlag.setValue( true );
                }
                else if ( VARIABLE_CHECK_SET_FALSE ==  type ) {
                    varFlag.setValue( false );
                }
                else if ( VARIABLE_CHECK_SET_OUTCOME ==  type ) {
                    varFlag.setValue( true );
                }
            }
            else {
                if ( VARIABLE_CHECK_SET_TRUE ==  type ) {
                    varFlag.setValue( false );
                }
                else if ( VARIABLE_CHECK_SET_FALSE ==  type ) {
                    varFlag.setValue( true );
                }
                else if ( VARIABLE_CHECK_SET_OUTCOME ==  type ) {
                    varFlag.setValue( false );
                }
            }
            break;     
            
        case DM_MEASUREMENT_COMPARE_LT:
            if ( value < valCompare ) {
                if ( VARIABLE_CHECK_SET_TRUE ==  type ) {
                    varFlag.setValue( true );
                }
                else if ( VARIABLE_CHECK_SET_FALSE ==  type ) {
                    varFlag.setValue( false );
                }
                else if ( VARIABLE_CHECK_SET_OUTCOME ==  type ) {
                    varFlag.setValue( true );
                }
            }
            else {
                if ( VARIABLE_CHECK_SET_TRUE ==  type ) {
                    varFlag.setValue( false );
                }
                else if ( VARIABLE_CHECK_SET_FALSE ==  type ) {
                    varFlag.setValue( true );
                }
                else if ( VARIABLE_CHECK_SET_OUTCOME ==  type ) {
                    varFlag.setValue( false );
                }
            }
            break;
            
        case DM_MEASUREMENT_COMPARE_LTEQ:
            if ( ( value < valCompare ) ||
                    vscp_almostEqualRelativeDouble( value, valCompare ) ) {
                if ( VARIABLE_CHECK_SET_TRUE ==  type ) {
                    varFlag.setValue( true );
                }
                else if ( VARIABLE_CHECK_SET_FALSE ==  type ) {
                    varFlag.setValue( false );
                }
                else if ( VARIABLE_CHECK_SET_OUTCOME ==  type ) {
                    varFlag.setValue( true );
                }
            }
            else {
                if ( VARIABLE_CHECK_SET_TRUE ==  type ) {
                    varFlag.setValue( false );
                }
                else if ( VARIABLE_CHECK_SET_FALSE ==  type ) {
                    varFlag.setValue( true );
                }
                else if ( VARIABLE_CHECK_SET_OUTCOME ==  type ) {
                    varFlag.setValue( false );
                }
            }
            break;
            
        case DM_MEASUREMENT_COMPARE_GT:
            if ( value > valCompare ) {
                if ( VARIABLE_CHECK_SET_TRUE ==  type ) {
                    varFlag.setValue( true );
                }
                else if ( VARIABLE_CHECK_SET_FALSE ==  type ) {
                    varFlag.setValue( false );
                }
                else if ( VARIABLE_CHECK_SET_OUTCOME ==  type ) {
                    varFlag.setValue( true );
                }
            }
            else {
                if ( VARIABLE_CHECK_SET_TRUE ==  type ) {
                    varFlag.setValue( false );
                }
                else if ( VARIABLE_CHECK_SET_FALSE ==  type ) {
                    varFlag.setValue( true );
                }
                else if ( VARIABLE_CHECK_SET_OUTCOME ==  type ) {
                    varFlag.setValue( false );
                }
            }
            break;      
            
        case DM_MEASUREMENT_COMPARE_GTEQ:
            if ( ( value >= valCompare ) || 
                    vscp_almostEqualRelativeDouble( value, valCompare ) ) {
                if ( VARIABLE_CHECK_SET_TRUE ==  type ) {
                    varFlag.setValue( true );
                }
                else if ( VARIABLE_CHECK_SET_FALSE ==  type ) {
                    varFlag.setValue( false );
                }
                else if ( VARIABLE_CHECK_SET_OUTCOME ==  type ) {
                    varFlag.setValue( true );
                }
            }
            else {
               if ( VARIABLE_CHECK_SET_TRUE ==  type ) {
                    varFlag.setValue( false );
                }
                else if ( VARIABLE_CHECK_SET_FALSE ==  type ) {
                    varFlag.setValue( true );
                }
                else if ( VARIABLE_CHECK_SET_OUTCOME ==  type ) {
                    varFlag.setValue( false );
                }
            }
            break;      
            
    }
    
    if ( !gpobj->m_variables.update( varFlag ) ) {
        wxString wxstrErr = _("[Action] Compare Variable: Failed to update variable ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// doActionCheckMeasurement
//

bool dmElement::doActionCheckMeasurement( vscpEvent *pDMEvent )
{
    wxString wxstr;
    int unit = 0;
    int sensorIndex = 0;
    double value = 0;
    double valueMeasurement = 0;
    uint8_t operation = DM_MEASUREMENT_COMPARE_NOOP;
    CVSCPVariable varFlag;      // Variable for result flag
    
    // Write in possible escapes
    wxString escaped_param = m_actionparam;
    handleEscapes( pDMEvent, escaped_param );
    
    // Make sure it is a measurement event
    if ( !vscp_isVSCPMeasurement( pDMEvent ) ) {
        wxstr = _("[Action] Must be measurement event. param = ") + escaped_param;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Get value
    if ( !vscp_getVSCPMeasurementAsDouble( pDMEvent, &valueMeasurement ) ) {
        wxstr = _("[Action] Failed to get measurement value. param = ") + 
                    escaped_param;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // unit;sendorindex;value;operation;flag-variable
    wxStringTokenizer tkz( escaped_param, _(";") );
    
    // unit
    if ( tkz.HasMoreTokens() ) {
        unit = vscp_readStringValue( tkz.GetNextToken() );
        
        if ( unit != vscp_getVSCPMeasurementUnit( pDMEvent )  ) {
            // It's another unit
            wxstr = _("[Action] Different unit. param = ") + escaped_param;
            gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
            return false;
        }
                
    }
    else {
        wxstr = _("[Action] Missing needed parameter (unit). param = ") + 
                    escaped_param;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // sensor index
    if ( tkz.HasMoreTokens() ) {
        sensorIndex = vscp_readStringValue( tkz.GetNextToken() );
        
        if ( sensorIndex != vscp_getVSCPMeasurementSensorIndex( pDMEvent )  ) {
            // It's another unit
            wxstr = _("[Action] Different sensor index. param = ") + escaped_param;
            gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
            return false;
        }
    }
    else {
        wxstr = _("[Action] Missing needed parameter "
                  "(sensor index). param = ") + escaped_param;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // valueCompare
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( !wxstr.ToCDouble( &value ) ) {
            value = 0;
            wxstr = _("[Action] Failed to convert to double "
                      "(set to zero). param = ") + escaped_param;
            gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
        }
    }
    else {
        wxstr = _("[Action] Missing needed parameter (value). param = ") + 
                    escaped_param;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }

    // Operation
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        operation = CDM::getCompareCodeFromToken( wxstr );
        if ( -1 == operation ) {
            wxstr = _("[Action] Invalid compare operation. param = ") + 
                        escaped_param;
            gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
            operation = DM_MEASUREMENT_COMPARE_NOOP;
        }
    }
    else {
        wxstr = _("[Action] Missing needed parameter (operation). param = ") + 
                    escaped_param;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Variable (flag) for result (Boolean)
    if ( tkz.HasMoreTokens() ) {
        
        wxString varName = tkz.GetNextToken();
        varName.Trim();
        if ( !gpobj->m_variables.find( varName, varFlag ) ) {

            // Variable was not found - create it
            if ( !gpobj->m_variables.add( varName,
                                            _("false"),
                                            VSCP_DAEMON_VARIABLE_CODE_BOOLEAN ) ) {
                wxString logStr = 
                    wxString::Format(_("[Action] Check measurement: Variable [%s] "
                                   "not defined. Failed to create it.\n"),
                                   (const char *)varName.mbc_str() );
                gpobj->logMsg( _("[DM] ") + logStr, 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                return false;
            }
            
            // Get the variable
            if ( !gpobj->m_variables.find( varName, varFlag ) ) {
                
                wxString logStr = 
                    wxString::Format(_("[Action] Check measurement: Variable [%s] "
                                   "was not found.\n"),
                                   (const char *)varName.mbc_str() );
                gpobj->logMsg( _("[DM] ") + logStr, 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
                return false;
                
            }

        }
        
        // The variable must be boolean
        if ( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != varFlag.getType() ) {
            wxstr = _("[Action] Result (flag) variable must be boolean. param = ") + 
                        escaped_param;
            gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
            return false;
        }
        
    }
    else {
        wxstr = _("[Action] Missing needed parameter (flag variable). param =") + 
                    escaped_param;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }
       
    switch ( operation ) {
        
        case DM_MEASUREMENT_COMPARE_NOOP:
            break;
        
        case DM_MEASUREMENT_COMPARE_EQ:
            if ( vscp_almostEqualRelativeDouble( value, valueMeasurement ) ) {
                varFlag.setValue( true );
            }
            else {
                varFlag.setValue( false );
            }
            break;
            
        case DM_MEASUREMENT_COMPARE_NEQ:
            if ( !vscp_almostEqualRelativeDouble( value, valueMeasurement ) ) {
                varFlag.setValue( true );
            }
            else {
                varFlag.setValue( false );
            }
            break;     
            
        case DM_MEASUREMENT_COMPARE_LT:
            if ( value < valueMeasurement ) {
                varFlag.setValue( true );
            }
            else {
                varFlag.setValue( false );
            }
            break;
            
        case DM_MEASUREMENT_COMPARE_LTEQ:
            if ( ( value < valueMeasurement ) ||
                    vscp_almostEqualRelativeDouble( value, valueMeasurement ) ) {
                varFlag.setValue( true );
            }
            else {
                varFlag.setValue( false );
            }
            break;
            
        case DM_MEASUREMENT_COMPARE_GT:
            if ( value > valueMeasurement ) {
                varFlag.setValue( true );
            }
            else {
                varFlag.setValue( false );
            }
            break;      
            
        case DM_MEASUREMENT_COMPARE_GTEQ:
            if ( ( value >= valueMeasurement ) || 
                    vscp_almostEqualRelativeDouble( value, valueMeasurement ) ) {
                varFlag.setValue( true );
            }
            else {
               varFlag.setValue( false );
            }
            break;      
            
    }
    
    if ( !gpobj->m_variables.update( varFlag ) ) {
        wxString wxstrErr = _("[Action] Compare Variable: "
                              "Failed to update variable ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// doActionStoreMin
//

bool dmElement::doActionStoreMin( vscpEvent *pDMEvent )
{
    wxString varname;
    uint8_t unit;
    uint8_t sensorindex;
    uint8_t zone;
    uint8_t subzone;
    CVSCPVariable variable;     // Variable to store possible new min in
    double currentValue;        // Current value of variable
    double value;               // Value of event
    
    // Write in possible escapes
    wxString params = m_actionparam;
    handleEscapes( pDMEvent, params );
    
    // Event must be measurement
    if ( !vscp_isVSCPMeasurement( pDMEvent ) ) {
        wxString wxstr = _("[Action] Event must be a measurement event. param =") + params;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Get value from measurement event
    vscp_getVSCPMeasurementAsDouble( pDMEvent, &value );
    
    // variable;unit;index;zone;subzone
    wxStringTokenizer tkz( params, _(";") );
    
    if ( !tkz.HasMoreTokens() ) {
        wxString wxstr = _("[Action] Need variable name. param =") + params;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Get variable name
    varname = tkz.GetNextToken();    
    varname.Trim();
    
    // Find the variable
    if ( !gpobj->m_variables.find( varname, variable ) ) {        
        // Variable not found
        wxString wxstr = _("[Action] Variable was not found. param = ") + params;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Variable must be numerical to be comparable
    if ( !variable.isNumerical() ) {
        wxString wxstr = _("[Action] Can't compare with non numeric variable. param = ") + params;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Get variable value
    variable.getValue( &currentValue );
    
    // Unit
    if ( tkz.HasMoreTokens() ) {
        unit = vscp_readStringValue( tkz.GetNextToken() );
    }
    else {
        unit = 0;   // Default unit
    }
    
    // Sensor index
    if ( tkz.HasMoreTokens() ) {
        sensorindex = vscp_readStringValue( tkz.GetNextToken() );
    }
    else {
        sensorindex = 0;   // Default unit
    }
    
    // zone
    if ( tkz.HasMoreTokens() ) {
        zone = vscp_readStringValue( tkz.GetNextToken() );
    }
    else {
        zone = 0;   // Default unit
    }
    
    // subzone
    if ( tkz.HasMoreTokens() ) {
        subzone = vscp_readStringValue( tkz.GetNextToken() );
    }
    else {
        subzone = 0;   // Default unit
    }
    
    if ( ( currentValue > value ) &&
            ( unit == vscp_getVSCPMeasurementUnit( pDMEvent ) ) &&
            ( sensorindex == vscp_getVSCPMeasurementSensorIndex( pDMEvent ) ) &&
            ( zone == vscp_getVSCPMeasurementZone( pDMEvent ) ) &&
            ( subzone == vscp_getVSCPMeasurementSubZone( pDMEvent ) ) ) {
        // Store new lowest value
        variable.setValue( value );
    }
    
    if ( !gpobj->m_variables.update( variable ) ) {
        wxString wxstrErr = _("[Action] Minimum: Failed to update variable ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionStoreMax
//

bool dmElement::doActionStoreMax( vscpEvent *pDMEvent )
{
    wxString varname;
    uint8_t unit;
    uint8_t sensorindex;
    uint8_t zone;
    uint8_t subzone;
    CVSCPVariable variable;     // Variable to store possible new max in
    double currentValue;        // Current value of variable
    double value;               // Value of event
    
    // Write in possible escapes
    wxString params = m_actionparam;
    handleEscapes( pDMEvent, params );
    
    // Event must be measurement
    if ( !vscp_isVSCPMeasurement( pDMEvent ) ) {
        wxString wxstr = _("[Action] Event must be a measurement event. param =") + params;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Get value from measurement event
    vscp_getVSCPMeasurementAsDouble( pDMEvent, &value );
    
    // variable;unit;index;zone;subzone
    wxStringTokenizer tkz( params, _(";") );
    
    if ( !tkz.HasMoreTokens() ) {
        wxString wxstr = _("[Action] Need variable name. param =") + params;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Get variable name
    varname = tkz.GetNextToken();    
    varname.Trim();
    
    // Find the variable
    if ( !gpobj->m_variables.find( varname, variable ) ) {
        // Variable not found
        wxString wxstr = _("[Action] Variable was not found. param = ") + params;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Variable must be numerical to be comparable
    if ( !variable.isNumerical() ) {
        wxString wxstr = _("[Action] Can't compare with non numeric variable. param =") + params;
        gpobj->logMsg( _("[DM] ") + wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Get variable value
    variable.getValue( &currentValue );
    
    // Unit
    if ( tkz.HasMoreTokens() ) {
        unit = vscp_readStringValue( tkz.GetNextToken() );
    }
    else {
        unit = 0;   // Default unit
    }
    
    // Sensor index
    if ( tkz.HasMoreTokens() ) {
        sensorindex = vscp_readStringValue( tkz.GetNextToken() );
    }
    else {
        sensorindex = 0;   // Default unit
    }
    
    // zone
    if ( tkz.HasMoreTokens() ) {
        zone = vscp_readStringValue( tkz.GetNextToken() );
    }
    else {
        zone = 0;   // Default unit
    }
    
    // subzone
    if ( tkz.HasMoreTokens() ) {
        subzone = vscp_readStringValue( tkz.GetNextToken() );
    }
    else {
        subzone = 0;   // Default unit
    }
    
    if ( ( currentValue < value ) &&
            ( unit == vscp_getVSCPMeasurementUnit( pDMEvent ) ) &&
            ( sensorindex == vscp_getVSCPMeasurementSensorIndex( pDMEvent ) ) &&
            ( zone == vscp_getVSCPMeasurementZone( pDMEvent ) ) &&
            ( subzone == vscp_getVSCPMeasurementSubZone( pDMEvent ) ) ) {
        // Store new highest value
        variable.setValue( value );
    }
    
    if ( !gpobj->m_variables.update( variable ) ) {
        wxString wxstrErr = _("[Action] Maximum: Failed to update variable ");
        wxstrErr += m_actionparam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionStartTimer
//

bool dmElement::doActionStartTimer( vscpEvent *pDMEvent )
{
    bool bAutoReload = false;
    wxString strVarName;
    int reloadLimit = -1;
    
    // Write in possible escapes
    wxString escaped_action = m_actionparam;
    handleEscapes( pDMEvent, escaped_action );

    wxStringTokenizer tkz( escaped_action, _(";") );
    
    // Timer id
    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter
        wxString wxstrErr = _("[Action] Start timer: Wrong action parameter ");
        wxstrErr += escaped_action;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Get timer id
    uint32_t id = vscp_readStringValue( tkz.GetNextToken() );

    
    // Initial count
    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter
        wxString wxstrErr = _("[Action] Start timer: Delay is missing ");
        wxstrErr += escaped_action;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;        
    }    
    
    // Get initial time
    uint32_t initialTime = vscp_readStringValue( tkz.GetNextToken() );    
    
    
    
    // Flag variable   // Can be blank
    if ( tkz.HasMoreTokens() ) {
        strVarName = tkz.GetNextToken();
        strVarName.Trim();
    }
    
    // Auto reload
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        str.MakeUpper();
        if ( wxNOT_FOUND != str.Find( _("TRUE") ) ) {
            bAutoReload = true;
        }
    }
    
    //  Reload Limit
    if ( tkz.HasMoreTokens() ) {
        reloadLimit = vscp_readStringValue( tkz.GetNextToken() );

    }

    if ( 0 == m_pDM->startTimer( id, 
                                    strVarName, 
                                    initialTime, 
                                    bAutoReload, 
                                    reloadLimit ) ) {
        // Strange action parameter
        wxString wxstrErr = _("[Action] Start timer: Failed to start timer ");
        wxstrErr += escaped_action;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionPauseTimer
//

bool dmElement::doActionPauseTimer( vscpEvent *pDMEvent )
{
    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    wxStringTokenizer tkz( wxstr, _(";") );
    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter
        wxString wxstrErr = _("[Action] Stop timer: Wrong action parameter ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // Get timer id
    uint32_t id = vscp_readStringValue( tkz.GetNextToken() );

    m_pDM->pauseTimer( id );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionResumeTimer
//

bool dmElement::doActionResumeTimer( vscpEvent *pDMEvent )
{
    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    wxStringTokenizer tkz( wxstr, _(";") );
    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter
        wxString wxstrErr = _("[Action] Stop timer: Wrong action parameter ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // Get timer id
    uint32_t id = vscp_readStringValue( tkz.GetNextToken() );

    m_pDM->startTimer( id );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionStopTimer
//

bool dmElement::doActionStopTimer( vscpEvent *pDMEvent )
{
    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    wxStringTokenizer tkz( wxstr, _(";") );
    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter
        wxString wxstrErr = _("[Action] Stop timer: Wrong action parameter ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // Get timer id
    uint32_t id = vscp_readStringValue( tkz.GetNextToken() );

    m_pDM->stopTimer( id );

    return true;
}







//*****************************************************************************
//*****************************************************************************
//*****************************************************************************





///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CDM::CDM( CControlObject *ctrlObj )
{
#ifndef BUILD_VSCPD_SERVICE

    // Set the default dm configuration path
#ifdef WIN32
    m_staticXMLPath = wxStandardPaths::Get().GetConfigDir();
    m_staticXMLPath += _("/vscp/dm.xml");
#else
    m_staticXMLPath = _("/srv/vscp/dm.xml");
#endif
#endif

    // Not allowed to edit XML content
    bAllowXMLsave = false;

    // Default is to feed all events through the matrix
    vscp_clearVSCPFilter( &m_DM_Table_filter );

    srand( wxDateTime::Now().GetYear()+ wxDateTime::Now().GetSecond() );
    m_rndMinute = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 60;
    m_rndHour = (uint8_t)(( (double)rand()/ ((double)(RAND_MAX) + (double)(1)) ) * 60);
    m_rndDay = (uint8_t)( (double)rand()/ ((double)(RAND_MAX) + (double)(1)) ) * 24;
    m_rndWeek = (uint8_t)( (double)rand()/ ((double)(RAND_MAX) + (double)(1)) ) * 7;
    m_rndMonth = (uint8_t)( (double)rand()/ ((double)(RAND_MAX) + (double)(1)) ) * 30;
    m_rndYear = (uint8_t)( (double)rand()/ ((double)(RAND_MAX) + (double)(1)) ) * 365;

    m_lastTime = wxDateTime::Now();

#ifdef WIN32
#ifdef BUILD_VSCPD_SERVICE
    //wxStandardPaths stdPath;

    // Set the default dm configuration path
    m_configPath = wxStandardPaths::Get().GetConfigDir();
    m_configPath += _("/vscp/dm.xml");
#else
    //wxStandardPaths stdPath;

    // Set the default dm configuration path
    m_staticXMLPath = wxStandardPaths::Get().GetConfigDir();
    m_staticXMLPath += _("/vscp/dm.xml");
    
    //m_path_db_vscp_dm = wxStandardPaths::Get().GetConfigDir();
    m_path_db_vscp_dm.Assign( wxStandardPaths::Get().GetConfigDir() +
                                "/vscp/dm.sqlite3");
#endif
#else
	m_staticXMLPath = _("/srv/vscp/dm.xml");
    m_path_db_vscp_dm.Assign( _("/srv/vscp/dm.sqlite3") ); 
#endif
        
   m_db_vscp_dm = NULL; 
   //m_db_vscp_dm_memory = NULL;

}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CDM::~CDM()
{
    // Must remove any timers
    DMTIMERS::iterator it;
    for ( it = m_timerHash.begin(); it != m_timerHash.end(); it++ ) {

        dmTimer *pTimer = it->second;

        // Check if variable name is already there
        if ( NULL != pTimer ) {
            delete pTimer;
            pTimer = NULL;
        }
    }

    // Remove all elements
    m_timerHash.clear();
}

///////////////////////////////////////////////////////////////////////////////
// init
//

void CDM::init( void )
{
    wxString wxstr;
    
    // * * * VSCP Daemon DM database * * * 
    
    // Check filename
    if ( m_path_db_vscp_dm.IsOk() && 
            m_path_db_vscp_dm.FileExists() ) {

        if ( SQLITE_OK != sqlite3_open( m_path_db_vscp_dm.GetFullPath().mbc_str(),
                                            &m_db_vscp_dm  ) ) {

            // Failed to open/create the database file
            fprintf( stderr, "VSCP Daemon DM database could not be opened. - Will not be used.\n" );
            wxstr.Printf( _("Path=%s error=%s\n"),
                            (const char *)m_path_db_vscp_dm.GetFullPath().mbc_str(),
                            sqlite3_errmsg( m_db_vscp_dm ) );
            fprintf( stderr, "%s", (const char *)wxstr.mbc_str() );
            if ( NULL != m_db_vscp_dm ) sqlite3_close( m_db_vscp_dm  );
            m_db_vscp_dm = NULL;

        }

    }
    else {
        if ( m_path_db_vscp_dm.IsOk() ) {
            
            // We need to create the database from scratch. This may not work if
            // the database is in a read only location.
            fprintf( stderr, "VSCP Daemon DM database does not exist - will be created.\n" );
            wxstr.Printf(_("Path=%s\n"), (const char *)m_path_db_vscp_dm.GetFullPath().mbc_str() );
            fprintf( stderr, "%s", (const char *)wxstr.mbc_str() );
            
            if ( SQLITE_OK == sqlite3_open( (const char *)m_path_db_vscp_dm.GetFullPath().mbc_str(),
                                            &m_db_vscp_dm ) ) {            
                // create the table.
                doCreateDMTable();
            }
            else {
                fprintf( stderr, "Failed to create VSCP DM database - will not be used.\n" );
                wxstr.Printf( _("Path=%s error=%s\n"),
                            (const char *)m_path_db_vscp_dm.GetFullPath().mbc_str(),
                            sqlite3_errmsg( m_db_vscp_dm ) );
                fprintf( stderr, "%s", (const char *)wxstr.mbc_str() );
            }
        }
        else {
            fprintf( stderr, "VSCP DM database path invalid - will not be used.\n" );
            wxstr.Printf(_("Path=%s\n"), (const char *)m_path_db_vscp_dm.GetFullPath().mbc_str() );
            fprintf( stderr, "%s", (const char *)wxstr.mbc_str() );
        }

    }
    
    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
        wxString wxlogmsg = 
        wxString::Format(_("DM engine started. DM from [%s]\n"),
                            (const char *)m_staticXMLPath.mbc_str() );
        gpobj->logMsg( _("[DM] ") + wxlogmsg, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
    }
}


///////////////////////////////////////////////////////////////////////////////
// cleanup
//

void CDM::cleanup( void )
{
    if ( NULL != m_db_vscp_dm ) sqlite3_close( m_db_vscp_dm  );
    m_db_vscp_dm = NULL; 
}


///////////////////////////////////////////////////////////////////////////////
// getCompareCodeFromToken
//

int CDM::getCompareCodeFromToken( wxString& token ) 
{
    
    token.MakeLower();
    token.Trim();
    
    if ( token.StartsWith(_("noop")) ) {
        return DM_MEASUREMENT_COMPARE_NOOP;
    } 
    else if ( token.StartsWith(_("eqgt")) ) {
        return DM_MEASUREMENT_COMPARE_GTEQ;
    }
    else if ( token.StartsWith(_("eqlt")) ) {
        return DM_MEASUREMENT_COMPARE_LTEQ;
    }
    else if ( token.StartsWith(_("eq")) ) {
        return DM_MEASUREMENT_COMPARE_EQ;
    } 
    else if ( token.StartsWith(_("==")) ) {
        return DM_MEASUREMENT_COMPARE_EQ;
    }
    else if ( token.StartsWith(_("neq")) ) {
        return DM_MEASUREMENT_COMPARE_NEQ;
    } 
    else if ( token.StartsWith(_("!=")) ) {
        return DM_MEASUREMENT_COMPARE_NEQ;
    }     
    else if ( token.StartsWith(_("<")) ) {
        return DM_MEASUREMENT_COMPARE_LT;
    }   
    else if ( token.StartsWith(_(">")) ) {
        return DM_MEASUREMENT_COMPARE_GT;
    }
    else if ( token.StartsWith(_("gteq")) ) {
        return DM_MEASUREMENT_COMPARE_GTEQ;
    }
    else if ( token.StartsWith(_("gt")) ) {
        return DM_MEASUREMENT_COMPARE_GT;
    }    
    else if ( token.StartsWith(_(">=")) ) {
        return DM_MEASUREMENT_COMPARE_GTEQ;
    }
    else if ( token.StartsWith(_("lteq")) ) {
        return DM_MEASUREMENT_COMPARE_LTEQ;
    }
    else if ( token.StartsWith(_("lt")) ) {
        return DM_MEASUREMENT_COMPARE_LT;
    }
    else if ( token.StartsWith(_("<=")) ) {
        return DM_MEASUREMENT_COMPARE_LTEQ;
    }
    else {
        return -1;
    }
}

///////////////////////////////////////////////////////////////////////////////
// getTokenFromCompareCode
//

wxString CDM::getTokenFromCompareCode( uint8_t code ) 
{
    switch ( code ) {
        
        case DM_MEASUREMENT_COMPARE_NOOP:
            return _("noop");
            break;
            
        case DM_MEASUREMENT_COMPARE_EQ:
            return _("eq");
            break;
            
        case  DM_MEASUREMENT_COMPARE_NEQ:
            return _("neq");
            break;
            
        case  DM_MEASUREMENT_COMPARE_LT:
            return _("lt");
            break;
            
        case  DM_MEASUREMENT_COMPARE_LTEQ:
            return _("lteq");
            break;
            
        case  DM_MEASUREMENT_COMPARE_GT:
            return _("eq");
            break;
            
        case  DM_MEASUREMENT_COMPARE_GTEQ:
            return _("lteq");
            break;
            
        default:
            break;
            
    }

    return _("error");
}

///////////////////////////////////////////////////////////////////////////////
// addMemoryElement
//
// Read decision matrix from file
//

bool CDM::addMemoryElement( dmElement *pItem )
{
    // Check pointer
    if ( NULL == pItem ) return false;

    // Let it know we own it
    pItem->m_pDM = this;

    m_mutexDM.Lock();
    m_DMList.Append( pItem );
    m_mutexDM.Unlock();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeMemoryElement
//

bool CDM::removeMemoryElement( unsigned short row )
{
    if ( row >= m_DMList.GetCount() ) return false;

    wxDMLISTNode *node = m_DMList.Item( row );

    m_mutexDM.Lock();
    m_DMList.DeleteNode(node);
    m_mutexDM.Unlock();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getMemoryElementFromRow
//

dmElement *CDM::getMemoryElementFromRow( const short row )
{
    if (row < 0) return NULL;
    if ( (unsigned short)row >= m_DMList.GetCount() ) return NULL;

    return m_DMList.Item( row )->GetData();
}

///////////////////////////////////////////////////////////////////////////////
// getDbIndexFromRow
//

uint32_t CDM::getDbIndexFromRow( const short row )
{
    uint32_t idx = 0;

    dmElement *pElement = m_DMList.Item( row )->GetData();

    if ( idx ) {
        idx = pElement->m_id;
    }

    return idx;
}

///////////////////////////////////////////////////////////////////////////////
// getMemoryElementFromId
//

dmElement *CDM::getMemoryElementFromId( const uint32_t idx, short *prow )
{
    short row = 0;
    DMLIST::iterator iter;
    for (iter = m_DMList.begin(); iter != m_DMList.end(); ++iter) {
        dmElement *pe = *iter;
        if ( idx == pe->m_id ) {
            if ( NULL != prow ) *prow = row;
            return pe;
        }
        row++;
    }
    
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// getAllRows
//

bool CDM::getAllRows( wxString& strAllRows )
{
    short row = 0;
    
    strAllRows.Empty();
    
    DMLIST::iterator iter;
    for (iter = m_DMList.begin(); iter != m_DMList.end(); ++iter) {
        dmElement *pe = *iter;      
        if ( NULL != pe ) strAllRows += pe->getAsString();
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// feedPeriodicEvent
//

bool CDM::feedPeriodicEvent( void )
{
    if ( m_lastTime.GetSecond() != wxDateTime::Now().GetSecond() ) {

        vscpEvent EventSecond;
        EventSecond.vscp_class = VSCP_CLASS2_VSCPD;
        EventSecond.vscp_type = VSCP2_TYPE_VSCPD_SECOND;    // Internal Second Event
        EventSecond.head = VSCP_PRIORITY_NORMAL;            // Set priority
        EventSecond.timestamp = vscp_makeTimeStamp();       // Set timestamp        
        vscp_setEventDateTimeBlockToNow( &EventSecond );    // Set time
        EventSecond.sizeData = 0;                           // No data
        EventSecond.pdata = NULL;
        //memcpy( EventSecond.GUID, m_pCtrlObject->m_GUID, 16 ); // Server GUID
        gpobj->m_guid.writeGUID( EventSecond.GUID );
        wxLogTrace( _("_RACE_vscpd_dm"), _("Internal second event\n") );
        feed( &EventSecond );

        // Update timers
        serviceTimers();

        if ( m_rndMinute == wxDateTime::Now().GetSecond() ) {

            vscpEvent EventRandomMinute;
            EventRandomMinute.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomMinute.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_MINUTE;   // Internal Random-Minute Event
            EventRandomMinute.head = VSCP_PRIORITY_NORMAL;          // Set priority
            EventRandomMinute.sizeData = 0;                         // No data
            EventRandomMinute.timestamp = vscp_makeTimeStamp();     // Set timestamp
            vscp_setEventDateTimeBlockToNow( &EventRandomMinute );  // Set time
            EventRandomMinute.pdata = NULL;
            //memcpy( EventRandomMinute.GUID, m_pCtrlObject->m_GUID, 16 ); // Server GUID
            gpobj->m_guid.writeGUID( EventRandomMinute.GUID );
            wxLogTrace( _("_RACE_vscpd_dm"), _("Internal random minute event\n")  );
            feed( &EventRandomMinute );

        }

    }

    if ( m_lastTime.GetMinute() != wxDateTime::Now().GetMinute() ) {

        m_rndMinute = (rand()/ RAND_MAX) * 60;
        vscpEvent EventMinute;
        EventMinute.vscp_class = VSCP_CLASS2_VSCPD;
        EventMinute.vscp_type = VSCP2_TYPE_VSCPD_MINUTE;    // Internal Minute Event
        EventMinute.head = VSCP_PRIORITY_NORMAL;            // Set priority
        EventMinute.timestamp = vscp_makeTimeStamp();       // Set timestamp
        vscp_setEventDateTimeBlockToNow( &EventMinute );     // Set time
        EventMinute.sizeData = 0;                           // No data
        EventMinute.pdata = NULL;
        //memcpy( EventMinute.GUID, m_pCtrlObject->m_GUID, 16 ); // Server GUID
        gpobj->m_guid.writeGUID( EventMinute.GUID );
        wxLogTrace( _("_RACE_vscpd_dm"), _("Internal minute event\n") );
        m_rndMinute = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 60;
        feed( &EventMinute );

        if ( m_rndHour == wxDateTime::Now().GetMinute() ) {

            vscpEvent EventRandomHour;
            EventRandomHour.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomHour.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_HOUR;       // Internal Random-Hour Event
            EventRandomHour.head = VSCP_PRIORITY_NORMAL;                    // Set priority
            EventRandomHour.timestamp = vscp_makeTimeStamp();               // Set timestamp
            vscp_setEventDateTimeBlockToNow( &EventRandomHour );            // Set time
            EventRandomHour.sizeData = 0;                                   // No data
            EventRandomHour.pdata = NULL;
            //memcpy( EventRandomHour.GUID, m_pCtrlObject->m_GUID, 16 );    // Server GUID
            gpobj->m_guid.writeGUID( EventRandomHour.GUID );
            wxLogTrace( _("_RACE_vscpd_dm"), _("Internal random hour event\n") );
            feed( &EventRandomHour );

        }
    }

    if ( m_lastTime.GetHour() != wxDateTime::Now().GetHour() ) {

        m_rndHour = (rand()/ RAND_MAX) * 60;
        vscpEvent EventHour;
        EventHour.vscp_class = VSCP_CLASS2_VSCPD;
        EventHour.vscp_type = VSCP2_TYPE_VSCPD_HOUR;                        // Internal Hour Event
        EventHour.head = VSCP_PRIORITY_NORMAL;                              // Set priority
        EventHour.timestamp = vscp_makeTimeStamp();                         // Set timestamp
        vscp_setEventDateTimeBlockToNow( &EventHour );                       // Set time
        EventHour.sizeData = 0;                                             // No data
        EventHour.pdata = NULL;
        //memcpy( EventtHour.GUID, m_pCtrlObject->m_GUID, 16 );             // Server GUID
        gpobj->m_guid.writeGUID( EventHour.GUID );
        wxLogTrace( _("_RACE_vscpd_dm"), _("Internal hour event\n") );
        m_rndHour = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 24;
        feed( &EventHour );

        // Check if it's time to send radom day event
        if ( m_rndDay == wxDateTime::Now().GetHour() ) {

            vscpEvent EventRandomDay;
            EventRandomDay.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomDay.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_DAY;         // Internal Random-Day Event
            EventRandomDay.head = VSCP_PRIORITY_NORMAL;                     // Set priority
            EventRandomDay.timestamp = vscp_makeTimeStamp();                // Set timestamp
            vscp_setEventDateTimeBlockToNow( &EventRandomDay );             // Set time
            EventRandomDay.sizeData = 0;                                    // No data
            EventRandomDay.pdata = NULL;
            //memcpy( EventRandomDay.GUID, m_pCtrlObject->m_GUID, 16 );     // Server GUID
            gpobj->m_guid.writeGUID( EventRandomDay.GUID );
            wxLogTrace( _("_RACE_vscpd_dm"), _("Internal random day event\n") );
            feed( &EventRandomDay );

        }
    }

    if ( m_lastTime.GetDay() != wxDateTime::Now().GetDay() ) {

        m_rndDay = (rand()/ RAND_MAX) * 24;
        vscpEvent EventDay;
        EventDay.vscp_class = VSCP_CLASS2_VSCPD;
        EventDay.vscp_type = VSCP2_TYPE_VSCPD_MIDNIGHT;     // Internal Midnight Event
        EventDay.head = VSCP_PRIORITY_NORMAL;               // Set priority
        EventDay.timestamp = vscp_makeTimeStamp();          // Set timestamp
        vscp_setEventDateTimeBlockToNow( &EventDay );       // Set time
        EventDay.sizeData = 0;                              // No data
        EventDay.pdata = NULL;
        //memcpy( EventDay.GUID, m_pCtrlObject->m_GUID, 16 );   // Server GUID
        gpobj->m_guid.writeGUID( EventDay.GUID );
        wxLogTrace( _("_RACE_vscpd_dm"), _("Internal day event\n") );
        m_rndDay = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 7;
        feed( &EventDay );

        // Check if it's time to send random week event
        if ( m_rndWeek == wxDateTime::Now().GetDay() ) {

            vscpEvent EventRandomWeek;
            EventRandomWeek.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomWeek.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_WEEK;   // Internal Random-Week Event
            EventRandomWeek.head = VSCP_PRIORITY_NORMAL;                // Set priority
            EventRandomWeek.timestamp = vscp_makeTimeStamp();           // Set timestamp
            vscp_setEventDateTimeBlockToNow( &EventRandomWeek );        // Set time
            EventRandomWeek.sizeData = 0;                               // No data
            EventRandomWeek.pdata = NULL;
            //memcpy( EventRandomWeek.GUID, m_pCtrlObject->m_GUID, 16 );// Server GUID
            gpobj->m_guid.writeGUID( EventRandomWeek.GUID );
            wxLogTrace( _("_RACE_vscpd_dm"), _("Internal random week event\n") );
            feed( &EventRandomWeek );

        }
    }

    if ( ( 1 == wxDateTime::Now().GetWeekDay() ) &&
        ( m_lastTime.GetWeekDay() != wxDateTime::Now().GetWeekDay() ) ) {

            m_rndWeek = (rand()/ RAND_MAX) * 7;
            vscpEvent EventWeek;
            EventWeek.vscp_class = VSCP_CLASS2_VSCPD;
            EventWeek.vscp_type = VSCP2_TYPE_VSCPD_WEEK;    // Internal Week Event
            EventWeek.head = VSCP_PRIORITY_NORMAL;          // Set priority
            EventWeek.timestamp = vscp_makeTimeStamp();     // Set timestamp
            vscp_setEventDateTimeBlockToNow( &EventWeek );   // Set time
            EventWeek.sizeData = 0;                         // No data
            EventWeek.pdata = NULL;
            //memcpy( EventWeek.GUID, m_pCtrlObject->m_GUID, 16 ); // Server GUID
            gpobj->m_guid.writeGUID( EventWeek.GUID );
            wxLogTrace( _("_RACE_vscpd_dm"), _("Internal week event\n") );
            m_rndWeek = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 52;
            feed( &EventWeek );

            if ( m_rndMonth == wxDateTime::Now().GetWeekDay() ) {

                vscpEvent EventRandomMonth;
                EventRandomMonth.vscp_class = VSCP_CLASS2_VSCPD;
                EventRandomMonth.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_MONTH;     // Internal Random-Month Event
                EventRandomMonth.head = VSCP_PRIORITY_NORMAL;                   // Set priority
                EventRandomMonth.timestamp = vscp_makeTimeStamp();              // Set timestamp
                vscp_setEventDateTimeBlockToNow( &EventRandomMonth );           // Set time
                EventRandomMonth.sizeData = 0;                                  // No data
                EventRandomMonth.pdata = NULL;
                //memcpy( EventRandomMonth.GUID, m_pCtrlObject->m_GUID, 16 );   // Server GUID
                gpobj->m_guid.writeGUID( EventRandomMonth.GUID );
                wxLogTrace( _("_RACE_vscpd_dm"), _("Internal random month event\n") );
                feed( &EventRandomMonth );

            }
    }

    if ( m_lastTime.GetMonth() != wxDateTime::Now().GetMonth() ) {

        m_rndMonth = (rand()/ RAND_MAX) * 30;
        vscpEvent EventMonth;
        EventMonth.vscp_class = VSCP_CLASS2_VSCPD;
        EventMonth.vscp_type = VSCP2_TYPE_VSCPD_MONTH;      // Internal Month Event
        EventMonth.head = VSCP_PRIORITY_NORMAL;             // Set priority
        EventMonth.timestamp = vscp_makeTimeStamp();        // Set timestamp
        vscp_setEventDateTimeBlockToNow( &EventMonth );      // Set time
        EventMonth.sizeData = 0;                            // No data
        EventMonth.pdata = NULL;
        //memcpy( EventMonth.GUID, m_pCtrlObject->m_GUID, 16 );  // Server GUID
        gpobj->m_guid.writeGUID( EventMonth.GUID );
        wxLogTrace( _("_RACE_vscpd_dm"), _("Internal month event\n") );
        m_rndMonth = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 12;
        feed( &EventMonth );

        if ( m_rndYear == wxDateTime::Now().GetMonth() ) {

            vscpEvent EventRandomYear;
            EventRandomYear.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomYear.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_YEAR;   // Internal Random-Minute Event
            EventRandomYear.head = VSCP_PRIORITY_NORMAL;                // Set priority
            EventRandomYear.timestamp = vscp_makeTimeStamp();           // Set timestamp
            vscp_setEventDateTimeBlockToNow( &EventRandomYear );        // Set time
            EventRandomYear.sizeData = 0;                               // No data
            EventRandomYear.pdata = NULL;
            //memcpy( EventRandomYear.GUID, m_pCtrlObject->m_GUID, 16 ); // Server GUID
            gpobj->m_guid.writeGUID( EventRandomYear.GUID );
            wxLogTrace( _("_RACE_vscpd_dm"), _("Internal random year event\n") );
            feed( &EventRandomYear );

        }
    }

    if ( m_lastTime.GetYear() != wxDateTime::Now().GetYear() ) {

        m_rndYear = (rand()/ RAND_MAX) * 365;
        vscpEvent EventYear;
        EventYear.vscp_class = VSCP_CLASS2_VSCPD;
        EventYear.vscp_type = VSCP2_TYPE_VSCPD_YEAR;            // Internal Year Event
        EventYear.head = VSCP_PRIORITY_NORMAL;                  // Set priority
        EventYear.timestamp = vscp_makeTimeStamp();             // Set timestamp
        vscp_setEventDateTimeBlockToNow( &EventYear );          // Set time
        EventYear.sizeData = 0;                                 // No data
        EventYear.pdata = NULL;
        //memcpy( EventYear.GUID, m_pCtrlObject->m_GUID, 16 );        // Server GUID
        gpobj->m_guid.writeGUID( EventYear.GUID );
        wxLogTrace( _("_RACE_vscpd_dm"), _("Internal year event\n") );
        m_rndYear = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 365;
        feed( &EventYear );

    }

    if ( ( 1 == wxDateTime::Now().GetMonth() ) &&
        ( m_lastTime.GetMonth() != wxDateTime::Now().GetMonth() ) ) {

            vscpEvent EventQuarter;
            EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
            EventQuarter.vscp_type = VSCP2_TYPE_VSCPD_QUARTER;  // Internal Quarter Event
            EventQuarter.head = VSCP_PRIORITY_NORMAL;           // Set priority
            EventQuarter.timestamp = vscp_makeTimeStamp();      // Set timestamp
            vscp_setEventDateTimeBlockToNow( &EventQuarter );   // Set time
            EventQuarter.sizeData = 0;                          // No data
            EventQuarter.pdata = NULL;
            //memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 );     // Server GUID
            gpobj->m_guid.writeGUID( EventQuarter.GUID );
            wxLogTrace( _("_RACE_vscpd_dm"), _("Internal quarter event\n") );
            feed( &EventQuarter );

    }

    if ( ( 4 == wxDateTime::Now().GetMonth() ) &&
        ( m_lastTime.GetMonth() != wxDateTime::Now().GetMonth() ) ) {

            vscpEvent EventQuarter;
            EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
            EventQuarter.vscp_type = VSCP2_TYPE_VSCPD_QUARTER;  // Internal Quarter Event
            EventQuarter.head = VSCP_PRIORITY_NORMAL;           // Set priority
            EventQuarter.timestamp = vscp_makeTimeStamp();      // Set timestamp
            vscp_setEventDateTimeBlockToNow( &EventQuarter );   // Set time
            EventQuarter.sizeData = 0;                          // No data
            EventQuarter.pdata = NULL;
            //memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 );   // Server GUID
            gpobj->m_guid.writeGUID( EventQuarter.GUID );
            wxLogTrace( _("_RACE_vscpd_dm"), _("Internal quarter event\n") );
            feed( &EventQuarter );

    }

    if ( ( 7 == wxDateTime::Now().GetMonth() ) &&
        ( m_lastTime.GetMonth() != wxDateTime::Now().GetMonth() ) ) {

            vscpEvent EventQuarter;
            EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
            EventQuarter.vscp_type = VSCP2_TYPE_VSCPD_QUARTER;  // Internal Quarter Event
            EventQuarter.head = VSCP_PRIORITY_NORMAL;           // Set priority
            EventQuarter.timestamp = vscp_makeTimeStamp();      // Set timestamp
            vscp_setEventDateTimeBlockToNow( &EventQuarter );   // Set time
            EventQuarter.sizeData = 0;                          // No data
            EventQuarter.pdata = NULL;
            //memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 );     // Server GUID
            gpobj->m_guid.writeGUID( EventQuarter.GUID );
            wxLogTrace( _("_RACE_vscpd_dm"), _("Internal quarter event\n") );
            feed( &EventQuarter );

    }

    if ( ( 10 == wxDateTime::Now().GetMonth() ) &&
        ( m_lastTime.GetMonth() != wxDateTime::Now().GetMonth() ) ) {

            vscpEvent EventQuarter;
            EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
            EventQuarter.vscp_type = VSCP2_TYPE_VSCPD_QUARTER;  // Internal Quarter Event
            EventQuarter.head = VSCP_PRIORITY_NORMAL;           // Set priority
            EventQuarter.timestamp = vscp_makeTimeStamp();      // Set timestamp
            vscp_setEventDateTimeBlockToNow( &EventQuarter );   // Set time
            EventQuarter.sizeData = 0;                          // No data
            EventQuarter.pdata = NULL;
            //memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 ); // Server GUID
            gpobj->m_guid.writeGUID( EventQuarter.GUID );
            wxLogTrace( _("_RACE_vscpd_dm"), _("Internal quarter event\n") );
            feed( &EventQuarter );

    }

    m_lastTime = wxDateTime::Now();

    return true;

}

///////////////////////////////////////////////////////////////////////////////
// feedTimerStarted
//

bool CDM::feedTimerStarted( uint32_t id, uint32_t time )
{
    vscpEvent EventTimerStart;
    EventTimerStart.vscp_class = VSCP_CLASS2_VSCPD;
    EventTimerStart.vscp_type = VSCP2_TYPE_VSCPD_TIMER_STARTED;   // Internal Random-Minute Event
    EventTimerStart.head = VSCP_PRIORITY_NORMAL;                // Set priority
    EventTimerStart.timestamp = vscp_makeTimeStamp();           // Set timestamp
    vscp_setEventDateTimeBlockToNow( &EventTimerStart );        // Set time
    EventTimerStart.sizeData = 8;                               // No data
    uint8_t data[8];
    data[0] = ( id >> 24 ) & 0xff;
    data[1] = ( id >> 16 ) & 0xff;
    data[2] = ( id >> 8 ) & 0xff;
    data[3] = id & 0xff;
    data[4] = ( time >> 24 ) & 0xff;
    data[5] = ( time >> 16 ) & 0xff;
    data[6] = ( time >> 8 ) & 0xff;
    data[7] = time & 0xff;
    EventTimerStart.pdata = data;
    gpobj->m_guid.writeGUID( EventTimerStart.GUID );
    
    return feed( &EventTimerStart );
}

///////////////////////////////////////////////////////////////////////////////
// feedTimerPaused
//

bool CDM::feedTimerPaused( uint32_t id, uint32_t time )
{
    vscpEvent EventTimerStart;
    EventTimerStart.vscp_class = VSCP_CLASS2_VSCPD;
    EventTimerStart.vscp_type = VSCP2_TYPE_VSCPD_TIMER_PAUSED;  // Internal Random-Minute Event
    EventTimerStart.head = VSCP_PRIORITY_NORMAL;                // Set priority
    EventTimerStart.timestamp = vscp_makeTimeStamp();           // Set timestamp
    vscp_setEventDateTimeBlockToNow( &EventTimerStart );        // Set time
    EventTimerStart.sizeData = 8;                               // No data
    uint8_t data[8];
    data[0] = ( id >> 24 ) & 0xff;
    data[1] = ( id >> 16 ) & 0xff;
    data[2] = ( id >> 8 ) & 0xff;
    data[3] = id & 0xff;
    data[4] = ( time >> 24 ) & 0xff;
    data[5] = ( time >> 16 ) & 0xff;
    data[6] = ( time >> 8 ) & 0xff;
    data[7] = time & 0xff;
    EventTimerStart.pdata = data;
    gpobj->m_guid.writeGUID( EventTimerStart.GUID );
    
    return feed( &EventTimerStart );
}

///////////////////////////////////////////////////////////////////////////////
// feedTimerResumed
//

bool CDM::feedTimerResumed( uint32_t id, uint32_t time )
{
    vscpEvent EventTimerStart;
    EventTimerStart.vscp_class = VSCP_CLASS2_VSCPD;
    EventTimerStart.vscp_type = VSCP2_TYPE_VSCPD_TIMER_RESUMED; // Internal Random-Minute Event
    EventTimerStart.head = VSCP_PRIORITY_NORMAL;                // Set priority
    EventTimerStart.timestamp = vscp_makeTimeStamp();           // Set timestamp
    vscp_setEventDateTimeBlockToNow( &EventTimerStart );        // Set time
    EventTimerStart.sizeData = 8;                               // No data
    uint8_t data[8];
    data[0] = ( id >> 24 ) & 0xff;
    data[1] = ( id >> 16 ) & 0xff;
    data[2] = ( id >> 8 ) & 0xff;
    data[3] = id & 0xff;
    data[4] = ( time >> 24 ) & 0xff;
    data[5] = ( time >> 16 ) & 0xff;
    data[6] = ( time >> 8 ) & 0xff;
    data[7] = time & 0xff;
    EventTimerStart.pdata = data;
    gpobj->m_guid.writeGUID( EventTimerStart.GUID );
    
    return feed( &EventTimerStart );
}

///////////////////////////////////////////////////////////////////////////////
// feedTimerStopped
//

bool CDM::feedTimerStopped( uint32_t id, uint32_t time )
{
    vscpEvent EventTimerStart;
    EventTimerStart.vscp_class = VSCP_CLASS2_VSCPD;
    EventTimerStart.vscp_type = VSCP2_TYPE_VSCPD_TIMER_STOPPED; // Internal Random-Minute Event
    EventTimerStart.head = VSCP_PRIORITY_NORMAL;                // Set priority
    EventTimerStart.timestamp = vscp_makeTimeStamp();           // Set timestamp
    vscp_setEventDateTimeBlockToNow( &EventTimerStart );        // Set time
    EventTimerStart.sizeData = 8;                               // No data
    uint8_t data[8];
    data[0] = ( id >> 24 ) & 0xff;
    data[1] = ( id >> 16 ) & 0xff;
    data[2] = ( id >> 8 ) & 0xff;
    data[3] = id & 0xff;
    data[4] = ( time >> 24 ) & 0xff;
    data[5] = ( time >> 16 ) & 0xff;
    data[6] = ( time >> 8 ) & 0xff;
    data[7] = time & 0xff;
    EventTimerStart.pdata = data;
    gpobj->m_guid.writeGUID( EventTimerStart.GUID );
    
    return feed( &EventTimerStart );
}

///////////////////////////////////////////////////////////////////////////////
// feedTimerElapsed
//

bool CDM::feedTimerElapsed( uint32_t id )
{
    vscpEvent EventTimerElapsed;
    EventTimerElapsed.vscp_class = VSCP_CLASS2_VSCPD;
    EventTimerElapsed.vscp_type = VSCP2_TYPE_VSCPD_TIMER_ELLAPSED;// Internal Random-Minute Event
    EventTimerElapsed.head = VSCP_PRIORITY_NORMAL;                // Set priority
    EventTimerElapsed.timestamp = vscp_makeTimeStamp();           // Set timestamp
    vscp_setEventDateTimeBlockToNow( &EventTimerElapsed );        // Set time
    EventTimerElapsed.sizeData = 4;                               // No data
    uint8_t data[4];
    data[0] = ( id >> 24 ) & 0xff;
    data[1] = ( id >> 16 ) & 0xff;
    data[2] = ( id >> 8 ) & 0xff;
    data[3] = id & 0xff;
    EventTimerElapsed.pdata = data;
    gpobj->m_guid.writeGUID( EventTimerElapsed.GUID );
    
    return feed( &EventTimerElapsed );
}
            
///////////////////////////////////////////////////////////////////////////////
// serviceTimers
//

void CDM::serviceTimers( void )
{
    CVSCPVariable variable;

    DMTIMERS::iterator it;
    for ( it = m_timerHash.begin(); it != m_timerHash.end(); it++ ) {

        dmTimer *pTimer = it->second;

        if ( pTimer->isActive() ) {
            
            if ( !pTimer->decTimer() ) {

                // We have reached zero
                
                bool bVariableFound = false;
                if ( gpobj->m_variables.find( pTimer->getVariableName(), variable ) ) {
                    bVariableFound = true;
                }
                
                // Set variable to true if one was defined
                if ( bVariableFound ) {
                    variable.setTrue();
                    if ( !gpobj->m_variables.update( variable ) ) {
                        wxString wxstrErr = 
                                _("[serviceTimers] Maximum: "
                                  "Failed to update variable ");
                        wxstrErr += _("\n");
                        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                    }
                }
                    
                // Feed timer elapsed event
                feedTimerElapsed( pTimer->getId() );

                if ( pTimer->isReloading() ) {
                    if ( !pTimer->isReloadLimit() ) {
                        pTimer->reload();
                        feedTimerStarted( pTimer->getId(), pTimer->getDelay() * 1000 );
                    }
                    else {
                        pTimer->reload(); // Reload even if we should stop
                        if ( pTimer->getReloadCounter() > 0 ) {                            
                            feedTimerStarted( pTimer->getId(), pTimer->getDelay() * 1000 );
                        }
                        else {
                            pTimer->setActive( false );  // We go inactive
                            // Feed timer stopped event
                            feedTimerStopped( pTimer->getId(), pTimer->getDelay() + 1000 );
                        }
                    }
                }
                else {
                    pTimer->setActive( false );  // We go inactive
                    // Feed timer stopped event
                    feedTimerStopped( pTimer->getId(), pTimer->getDelay() + 1000 );
                }

            }

        } // Active and variable exists

    }

}

///////////////////////////////////////////////////////////////////////////////
// addTimer
//

int CDM::addTimer( uint32_t id,
                    wxString& nameVar,
                    uint32_t delay,
                    bool bStart,
                    bool bReload,
                    int reloadLimit )
{

    int rv = 0; // Default return value is failure
    dmTimer *pTimer;

    // Log
    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM_TIMERS ) {
        wxString logStr = wxString::Format(_("[Add timer] Add Timer %s.\n"),
                                            (const char *)nameVar.mbc_str() );
        gpobj->logMsg( _("[DM] ") + logStr, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
    }

    // Check if the timer already exist - set new values
    if ( NULL != ( pTimer = m_timerHash[ (int)id ] ) ) {
        
        pTimer->setVariableName( nameVar );
        pTimer->setId( id );
        pTimer->setReload( bReload );
        pTimer->setDelay( delay );
        pTimer->setActive( bStart );
        pTimer->setReloadLimit( reloadLimit );

        if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM_TIMERS ) {
            wxString logStr = 
                    wxString::Format(_("[Add timer] Timer %d "
                                       "already exist.\n"), id );
            gpobj->logMsg( _("[DM] ") + logStr, 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
        }
       
    }
    else {
        // Create the timer
        dmTimer *pTimer = new dmTimer( nameVar,
                                        id,
                                        delay,
                                        bStart,
                                        bReload,
                                        reloadLimit );

        if ( NULL == pTimer ) {
            wxString logStr = 
                    wxString::Format(_("[Add timer] Timer %d "
                                       "Could not create.\n"), id );
            gpobj->logMsg( _("[DM] ") + 
                            logStr, 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
            return 0;
        }
        
        // add the timer to the hash table
        rv = id;
        m_timerHash[ id ] = pTimer;
        
    }

    nameVar.Trim(true);
    nameVar.Trim(false);

    // Create variable if it does not exist
    if ( !gpobj->m_variables.exist( nameVar ) ) {

        if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM_TIMERS ) {
            wxString logStr = wxString::Format(_("[Add timer] Variable [%s] not defined. "
                                                 "Creating it.\n"),
                                                (const char *)nameVar.mbc_str() );
            gpobj->logMsg( _("[DM] ") + logStr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        }

        if ( !gpobj->m_variables.add( nameVar, 
                                        _("false"), 
                                        VSCP_DAEMON_VARIABLE_CODE_BOOLEAN ) ) {
            wxString logStr = wxString::Format(_("[Add timer] Variable [%s] not defined. "
                                                 "Failed to create it.\n"),
                                                (const char *)nameVar.mbc_str() );
            gpobj->logMsg( _("[DM] ") + logStr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
        }

    }
    
    // Start timer if requested to do so
    if ( bStart ) startTimer( id );

    return rv;

}

///////////////////////////////////////////////////////////////////////////////
// startTimer
//

bool CDM::startTimer( uint32_t idTimer )
{
    dmTimer *pTimer = m_timerHash[ idTimer ];
    if ( NULL == pTimer ) return false;

    pTimer->startTimer();
    
    feedTimerStarted( idTimer, pTimer->getDelay()*1000 );

    // Log
    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM_TIMERS ) {
        wxString logStr = wxString::Format(_("Timer %d started.\n"), idTimer );
        gpobj->logMsg( _("[DM] ") + logStr, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// startTimer
//

int CDM::startTimer( uint32_t idTimer, 
                        wxString& nameVariable, 
                        uint32_t delay,
                        bool bSetValue,
                        int reloadLimit )
{
    uint16_t rv = 0;

    dmTimer *pTimer = m_timerHash[ idTimer ];
    if ( NULL == pTimer ) {

        // Log
        if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM_TIMERS ) {
            wxString logStr = wxString::Format(_("Timer %d created.\n"), idTimer );
            gpobj->logMsg( _("[DM] ") + logStr, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
        }

        // Nonexistent timer - create it
        rv = addTimer( idTimer,
                        nameVariable,
                        delay,
                        true,
                        bSetValue,
                        reloadLimit );
    }
    else {
        pTimer->startTimer();
        rv = 1;
    }

    // Log
    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM_TIMERS ) {
        wxString logStr = wxString::Format(_("Timer %d started.\n"), idTimer );
        gpobj->logMsg( _("[DM] ") + logStr, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// stopTimer
//

bool CDM::stopTimer( uint32_t idTimer )
{
    dmTimer *pTimer = m_timerHash[ idTimer ];
    if ( NULL == pTimer ) return false;

    // Stop it
    pTimer->stopTimer();
    
    // Tell the world that it is stopped
    feedTimerStopped( idTimer, pTimer->getDelay() );
    
    // Log
    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM_TIMERS ) {
        wxString logStr = wxString::Format(_("Timer %d stopped.\n"), idTimer );
        gpobj->logMsg( _("[DM] ") + logStr, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// pauseTimer
//

bool CDM::pauseTimer( uint32_t idTimer )
{
    dmTimer *pTimer = m_timerHash[ idTimer ];
    if ( NULL == pTimer ) return false;
    
    // Pause the timer
    pTimer->pauseTimer();
    
    // Tell the world it is paused
    feedTimerPaused( idTimer, pTimer->getDelay() );

    // Log
    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM_TIMERS ) {
        wxString logStr = wxString::Format(_("Timer %d pused.\n"), idTimer );
        gpobj->logMsg( _("[DM] ") + logStr, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
    }

    

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// resumeTimer
//

bool CDM::resumeTimer( uint32_t idTimer )
{
    dmTimer *pTimer = m_timerHash[ idTimer ];
    if ( NULL == pTimer ) return false;

    // Resume timer
    pTimer->resumeTimer();
    
    // Tell the world
    feedTimerResumed( idTimer, pTimer->getDelay() );
    
    // Log
    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM_TIMERS ) {
        wxString logStr = wxString::Format(_("Timer %d resumed.\n"), idTimer );
        gpobj->logMsg( _("[DM] ") + logStr, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
    }

    

    return true;
}


//*****************************************************************************
//                                Database
//*****************************************************************************


///////////////////////////////////////////////////////////////////////////////
// doCreateDMTable
//

bool CDM::doCreateDMTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_DM_CREATE;
    
    // Check if database is open
    if ( NULL == m_db_vscp_dm ) return false;
    
    m_mutexDM.Lock();
    
    if ( SQLITE_OK != sqlite3_exec(m_db_vscp_dm, psql, NULL, NULL, &pErrMsg ) ) {
        m_mutexDM.Unlock();
        return false;
    }
    
    m_mutexDM.Unlock();
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// getDatabaseRecordCount
//
//

uint32_t CDM::getDatabaseRecordCount( void )
{
    wxString wxstr;
    sqlite3_stmt *ppStmt;
    uint32_t count = 0;
    
    // Database file must be open
    if ( NULL == m_db_vscp_dm ) {
        gpobj->logMsg( _("[DM] Get record count. Database file is not open.\n"),
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    if ( SQLITE_OK != sqlite3_prepare_v2( m_db_vscp_dm,
                                            "SELECT count(*) FROM dm WHERE bEnable!=0;",
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        return false;
    }
    
    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        count = sqlite3_column_int( ppStmt, 0 );
    }
    
    sqlite3_finalize( ppStmt );
       
    
    return count;
}

///////////////////////////////////////////////////////////////////////////////
// deleteDatabaseRecord
//
//

bool CDM::deleteDatabaseRecord( uint32_t idx )
{
    char *pErrMsg;
    
    // Database file must be open
    if ( NULL == m_db_vscp_dm ) {
        gpobj->logMsg( _("[DM] Delete record. Database file is not open.\n"),
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    char *sql = sqlite3_mprintf( "DELETE FROM \"dm\" "
                                 "WHERE id='%d';",
                                    idx );
        if ( SQLITE_OK != sqlite3_exec( m_db_vscp_dm, sql, NULL, NULL, &pErrMsg ) ) {
            sqlite3_free( sql );
            return false;
        }
        
        sqlite3_free( sql );
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addDatabaseRecord
//
//

bool CDM::addDatabaseRecord( dmElement *pdm )
{
    char *pErrMsg;
    cguid guid_mask;
    cguid guid_filter;
            
    // Database file must be open
    if ( NULL == m_db_vscp_dm ) {
        gpobj->logMsg( _("[DM] Add record. Database file is not open.\n"),
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    guid_mask.getFromArray( pdm->m_vscpfilter.mask_GUID );
    guid_filter.getFromArray( pdm->m_vscpfilter.filter_GUID );
    
    char *sql = sqlite3_mprintf( VSCPDB_DM_INSERT,
                (const char *)pdm->m_strGroupID.mbc_str(),
                pdm->m_bEnable ? 1 : 0,
                pdm->m_vscpfilter.mask_priority,
                pdm->m_vscpfilter.mask_class,
                pdm->m_vscpfilter.mask_type,
                (const char *)guid_mask.getAsString().mbc_str(),
                pdm->m_vscpfilter.filter_priority,
                pdm->m_vscpfilter.filter_class,
                pdm->m_vscpfilter.filter_type,
                (const char *)guid_filter.getAsString().mbc_str(),
                //--------------------------------------------------------------          
                (const char *)pdm->m_timeAllow.getFromTime().FormatISOCombined().mbc_str(),
                (const char *)pdm->m_timeAllow.getEndTime().FormatISOCombined().mbc_str(),
           
                pdm->m_timeAllow.getWeekday(0) ? 1 : 0, 
                pdm->m_timeAllow.getWeekday(1) ? 1 : 0,
                pdm->m_timeAllow.getWeekday(2) ? 1 : 0,
                pdm->m_timeAllow.getWeekday(3) ? 1 : 0,
                pdm->m_timeAllow.getWeekday(4) ? 1 : 0,
                pdm->m_timeAllow.getWeekday(5) ? 1 : 0,  
                //--------------------------------------------------------------
                pdm->m_timeAllow.getWeekday(6) ? 1 : 0,
                (const char *)pdm->m_timeAllow.getActionTimeAsString().mbc_str(),
                pdm->m_bCheckIndex ? 1 : 0,
                (int)pdm->m_index,
                pdm->m_bCheckZone ? 1 : 0,
                (int)pdm->m_zone,
                pdm->m_bCheckSubZone ? 1 : 0,
                (int)pdm->m_subzone,
                pdm->m_bCheckMeasurementIndex ? 1 : 0, 
                //--------------------------------------------------------------
                (int)pdm->m_actionCode,
                (const char *)pdm->m_actionparam.mbc_str(),
                pdm->m_bCompareMeasurement ? 1 : 0,
                pdm->m_measurementValue,
                (int)pdm->m_measurementUnit,
                (int)pdm->m_measurementCompareCode,
                (const char *)pdm->m_comment.mbc_str()
            );

        if ( SQLITE_OK != sqlite3_exec( m_db_vscp_dm, 
                                            sql, NULL, NULL, &pErrMsg ) ) {
            wxString err = 
                    wxString::Format( _("[DM] addDatabaseRecord failed. SQL = %s, err = %s\n"), 
                    sql, 
                    pErrMsg ); 
            //wxPrintf( "%s", sql );                            
            gpobj->logMsg( err,
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
            sqlite3_free( sql );            
            return false;
        }

        sqlite3_free( sql );
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// updateDatabaseRecord
//

bool CDM::updateDatabaseRecord( dmElement *pdm )
{
    char *pErrMsg;
    cguid guid_mask;
    cguid guid_filter;
    
    // Can't save to db if loaded from XML
    if ( pdm->m_bStatic ) return false;
    
    // Database file must be open
    if ( NULL == m_db_vscp_dm ) {
        gpobj->logMsg( _("[DM] ") + _("Add record. Database file is not open.\n"),
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    guid_mask.getFromArray( pdm->m_vscpfilter.mask_GUID );
    guid_filter.getFromArray( pdm->m_vscpfilter.filter_GUID );
    
    wxString dtFrom = pdm->m_timeAllow.getFromTime().FormatISOCombined();
    wxString dtEnd = pdm->m_timeAllow.getEndTime().FormatISOCombined();
    
    char *sql = sqlite3_mprintf( VSCPDB_DM_UPDATE,
                (const char *)pdm->m_strGroupID.mbc_str(),
                pdm->m_bEnable ? 1 : 0,
                pdm->m_vscpfilter.mask_priority,
                pdm->m_vscpfilter.mask_class,
                pdm->m_vscpfilter.mask_type,
                (const char *)guid_mask.getAsString().mbc_str(),
                pdm->m_vscpfilter.filter_priority,
                pdm->m_vscpfilter.filter_class,
                pdm->m_vscpfilter.filter_type,
                (const char *)guid_filter.getAsString().mbc_str(),
                //--------------------------------------------------------------            
                (const char *)dtFrom.mbc_str(),
                (const char *)dtFrom.mbc_str(),
           
                pdm->m_timeAllow.getWeekday(0) ? 1 : 0,
                pdm->m_timeAllow.getWeekday(1) ? 1 : 0,
                pdm->m_timeAllow.getWeekday(2) ? 1 : 0,
                pdm->m_timeAllow.getWeekday(3) ? 1 : 0,
                pdm->m_timeAllow.getWeekday(4) ? 1 : 0,
                pdm->m_timeAllow.getWeekday(5) ? 1 : 0,
                //--------------------------------------------------------------
                pdm->m_timeAllow.getWeekday(6) ? 1 : 0,
                (const char *)pdm->m_timeAllow.getActionTimeAsString().mbc_str(),
                pdm->m_bCheckIndex ? 1 : 0,
                pdm->m_index,
                pdm->m_bCheckZone,
                pdm->m_zone,
                pdm->m_bCheckSubZone,
                pdm->m_subzone,
                pdm->m_bCheckMeasurementIndex,
                //--------------------------------------------------------------
                pdm->m_actionCode,
                (const char *)pdm->m_actionparam.mbc_str(),
                pdm->m_bCompareMeasurement,
                pdm->m_measurementValue,
                pdm->m_measurementUnit,
                pdm->m_measurementCompareCode,
                (const char *)pdm->m_comment.mbc_str(),
                pdm->m_id     // Where clause
            );

    if ( SQLITE_OK != sqlite3_exec( m_db_vscp_dm, 
                                            sql, NULL, NULL, &pErrMsg)) {
        wxString err = _("[DM] updateDatabaseRecord failed. SQL = %s error=%s\n"); 
        gpobj->logMsg( wxString::Format( err, sql, pErrMsg ),
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
        sqlite3_free( sql );        
        return false;
    }

    sqlite3_free( sql );
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// updateDatabaseRecordItem
//

bool CDM::updateDatabaseRecordItem( unsigned long id, 
                                        const wxString& strUpdateField, 
                                        const wxString& strUpdateValue )
{
    char *pErrMsg;
            
    // Database file must be open
    if ( NULL == m_db_vscp_dm ) {
        gpobj->logMsg( _("[DM] ") + _("Update record item. Database file is not open.\n"),
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    char *sql = sqlite3_mprintf( VSCPDB_DM_UPDATE_ITEM, 
                                    (const char *)strUpdateField.mbc_str(),
                                    (const char *)strUpdateValue.mbc_str(),
                                    id );
    if ( SQLITE_OK != sqlite3_exec( m_db_vscp_dm, 
                                            sql, NULL, NULL, &pErrMsg)) {
        wxString err = _("[DM] updateDatabaseRecordItem failed. SQL = %s error=%s\n"); 
        gpobj->logMsg( wxString::Format( err, sql, pErrMsg ),
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );                                                    
        sqlite3_free( sql );
        return false;
    }

    sqlite3_free( sql );
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// getDatabaseRecord
//
//

bool CDM::getDatabaseRecord( uint32_t idx, dmElement *pDMitem )
{
    wxString wxstr;
    sqlite3_stmt *ppStmt;
    
    // Check pointer
    if ( NULL == pDMitem ) return false;
    
    // Database file must be open
    if ( NULL == m_db_vscp_dm ) {
        gpobj->logMsg( _("[DM] ") + _("Get record. Database file is not open.\n"), 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_DM );
        return false;
    }
    
    char *sql = sqlite3_mprintf( "SELECT * FROM dm WHERE id='%d';", idx );
    
    if ( SQLITE_OK != sqlite3_prepare_v2( m_db_vscp_dm,
                                            sql,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        wxString err = _("[DM] getDatabaseRecord failed. SQL = %s\n"); 
        gpobj->logMsg( wxString::Format( err, sql ),
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );                                            
        sqlite3_free( sql );
        return false;
    }
    
    if ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
                  
        const char *p;
            
        pDMitem->m_pDM = this;  // Record owner
            
        // id in database
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ID ) ) ) {
            pDMitem->m_id = atol( p );
        }
            
        // bEnable
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ENABLE ) ) ) {
            pDMitem->m_bEnable = atoi( p ) ? true : false;
        }
            
        // groupid
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_GROUPID ) ) ) {
            pDMitem->m_strGroupID = wxString::FromUTF8( p );
        }
            
        // Mask priority
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_MASK_PRIORITY ) ) ) {
            pDMitem->m_vscpfilter.mask_priority = atoi( p );
        }
            
        // Filter priority
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_FILTER_PRIORITY ) ) ) {
            pDMitem->m_vscpfilter.filter_priority = atoi( p );
        }
            
        // Mask class
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_MASK_CLASS ) ) ) {
            pDMitem->m_vscpfilter.mask_class = atoi( p );
        }
            
        // Filter class
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_FILTER_CLASS ) ) ) {
            pDMitem->m_vscpfilter.filter_class = atoi( p );
        }
            
        // Mask type
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_MASK_TYPE ) ) ) {
            pDMitem->m_vscpfilter.mask_type = atoi( p );
        }
            
        // Filter type
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_FILTER_TYPE ) ) ) {
            pDMitem->m_vscpfilter.filter_type = atoi( p );
        }
            
        // Mask GUID
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_MASK_GUID ) ) ) {
            cguid guid;
            guid.getFromString( p );
            memcpy( pDMitem->m_vscpfilter.mask_GUID, guid.getGUID(), 16 );
        }
            
        // Filter GUID
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_FILTER_GUID ) ) ) {
            cguid guid;
            guid.getFromString( p );
            memcpy( pDMitem->m_vscpfilter.filter_GUID, guid.getGUID(), 16 );
        }
            
        // Allowed start
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_START ) ) ) {
            pDMitem->m_timeAllow.setFromTime( wxString::FromUTF8( p ) );
        }
            
        // Allowed end
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_END ) ) ) {
            pDMitem->m_timeAllow.getEndTime().ParseDate( wxString::FromUTF8( p ) );
        }
            
        // Allowed time
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_TIME ) ) ) {
            pDMitem->m_timeAllow.parseActionTime( wxString::FromUTF8( p ) );
        }
            
        // Allow Monday
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_MONDAY ) ) ) {
            atoi( p ) ? pDMitem->m_timeAllow.allowMonday() : pDMitem->m_timeAllow.allowMonday( false );
        }
            
        // Allow Tuesday
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_TUESDAY ) ) ) {
            atoi( p ) ? pDMitem->m_timeAllow.allowTuesday() : pDMitem->m_timeAllow.allowTuesday( false );
        }
            
        // Allow Wednesday
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_WEDNESDAY ) ) ) {
            atoi( p ) ? pDMitem->m_timeAllow.allowWednesday() : pDMitem->m_timeAllow.allowWednesday( false );
        }
            
        // Allow Thursday
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_THURSDAY ) ) ) {
            atoi( p ) ? pDMitem->m_timeAllow.allowThursday() : pDMitem->m_timeAllow.allowThursday( false );
        }
            
        // Allow Friday
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_FRIDAY ) ) ) {
            atoi( p ) ? pDMitem->m_timeAllow.allowFriday() : pDMitem->m_timeAllow.allowFriday( false );
        }
            
        // Allow Saturday
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_SATURDAY ) ) ) {
            atoi( p ) ? pDMitem->m_timeAllow.allowSaturday() : pDMitem->m_timeAllow.allowSaturday( false );
        }
            
        // Allow Sunday
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_SUNDAY ) ) ) {
            atoi( p ) ? pDMitem->m_timeAllow.allowSunday() : pDMitem->m_timeAllow.allowSunday( false );
        }

        // bCheckIndex
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_CHECK_INDEX ) ) ) {
            pDMitem->m_bCheckIndex = atoi( p ) ? true : false;
        }
            
        // Index
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_INDEX ) ) ) {
            pDMitem->m_index = atoi( p );
        }
            
        // bCheckZone
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_CHECK_ZONE ) ) ) {
            pDMitem->m_bCheckZone = atoi( p ) ? true : false;
        }
            
        // Zone
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ZONE ) ) ) {
            pDMitem->m_zone = atoi( p );
        }
            
        // bCheckSubZone
        pDMitem->m_bCheckZone = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_DM_CHECK_SUBZONE ) ? true : false;
            
        // SubZone
        pDMitem->m_zone = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_DM_SUBZONE );
            
        // bMeasurementIndex
        pDMitem->m_bCheckMeasurementIndex = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_DM_CHECK_MEASUREMENT_INDEX ) ? true : false;
             
        // ActionCode
        pDMitem->m_actionCode = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_DM_ACTIONCODE );
            
        // ActionParamter
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ACTIONPARAMETER ) ) ) {
            pDMitem->m_actionparam = wxString::FromUTF8( p );
        }
            
        // Measurement check value flag
        pDMitem->m_bCompareMeasurement = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_CHECK_VALUE ) ? true : false;        
                
        // Measurement value
        pDMitem->m_measurementValue = sqlite3_column_double( ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_VALUE );
            
        // Measurement unit
        pDMitem->m_measurementUnit = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_UNIT );
            
        // Measurement compare code
        pDMitem->m_measurementCompareCode = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_COMPARE );
        
    }
    
    sqlite3_finalize( ppStmt );
    sqlite3_free( sql );
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// loadFromDatabase
//
// Read decision matrix from the database
//

bool CDM::loadFromDatabase( void )
{
    wxString wxstr;
    sqlite3_stmt *ppStmt;
        
    // Database file must be open
    if ( NULL == m_db_vscp_dm ) {
        gpobj->logMsg( _("[DM] ") + _("Loading. Database file is not open.\n"), 
                        DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    gpobj->logMsg( wxString::Format( _("[DM] ") + _
                        ("Loading decision matrix from database : %s\n"), 
                        (const char *)m_path_db_vscp_dm.GetFullPath().mbc_str() ),
                        DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
    
    m_mutexDM.Lock();
    
    if ( SQLITE_OK != sqlite3_prepare_v2( m_db_vscp_dm,
                                            "SELECT * FROM dm;",
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        m_mutexDM.Unlock();
        return false;
    }
    
    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        
        // Set row default values for row
        dmElement *pDMitem = new dmElement;
        if ( NULL != pDMitem ) {
            
            const char *p;
            
            pDMitem->m_pDM = this;  // Record owner
            
            // id in database
            if ( NULL != 
                 ( p = (const char *)sqlite3_column_text( ppStmt, 
                                                    VSCPDB_ORDINAL_DM_ID ) ) ) {
                pDMitem->m_id = atol( p );
            }
            
            // bEnable
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                                VSCPDB_ORDINAL_DM_ENABLE ) ) ) {
                pDMitem->m_bEnable = atoi( p ) ? true : false; 
            }
            
            // groupid
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                               VSCPDB_ORDINAL_DM_GROUPID ) ) ) {
                pDMitem->m_strGroupID = wxString::FromUTF8( p );
            }
            
            // Mask priority
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                         VSCPDB_ORDINAL_DM_MASK_PRIORITY ) ) ) {
                pDMitem->m_vscpfilter.mask_priority = atoi( p );
            }
            
            // Filter priority
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                       VSCPDB_ORDINAL_DM_FILTER_PRIORITY ) ) ) {
                pDMitem->m_vscpfilter.filter_priority = atoi( p );
            }
            
            // Mask class
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                            VSCPDB_ORDINAL_DM_MASK_CLASS ) ) ) {
                pDMitem->m_vscpfilter.mask_class = atoi( p );
            }
            
            // Filter class
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                          VSCPDB_ORDINAL_DM_FILTER_CLASS ) ) ) {
                pDMitem->m_vscpfilter.filter_class = atoi( p );
            }
            
            // Mask type
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                             VSCPDB_ORDINAL_DM_MASK_TYPE ) ) ) {
                pDMitem->m_vscpfilter.mask_type = atoi( p );
            }
            
            // Filter type
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                           VSCPDB_ORDINAL_DM_FILTER_TYPE ) ) ) {
                pDMitem->m_vscpfilter.filter_type = atoi( p );
            }
            
            // Mask GUID
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                             VSCPDB_ORDINAL_DM_MASK_GUID ) ) ) {
                if ( ( NULL != p ) && ( '*' != *p ) ) {
                    cguid guid;
                    guid.getFromString( p );
                    memcpy( pDMitem->m_vscpfilter.mask_GUID, guid.getGUID(), 16 );
                }
            }
            
            // Filter GUID
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                           VSCPDB_ORDINAL_DM_FILTER_GUID ) ) ) {
                if ( ( NULL != p ) && ( '*' != *p ) ) {
                    cguid guid;
                    guid.getFromString( p );
                    memcpy( pDMitem->m_vscpfilter.filter_GUID, guid.getGUID(), 16 );
                }
            }
            
            // Allowed from
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                         VSCPDB_ORDINAL_DM_ALLOWED_START ) ) ) {
                if ( ( NULL != p ) && ( '*' != *p ) ) {
                    pDMitem->m_timeAllow.setFromTime( wxString::FromUTF8( p ) );
                }
            }
            
            // Allowed to
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                           VSCPDB_ORDINAL_DM_ALLOWED_END ) ) ) {
                if ( ( NULL != p ) && ( '*' != *p ) ) {
                    pDMitem->m_timeAllow.setEndTime( wxString::FromUTF8( p ) );
                }
            }
            
            // Allowed time
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                          VSCPDB_ORDINAL_DM_ALLOWED_TIME ) ) ) {
                if ( ( NULL != p ) && ( '*' != *p ) ) {
                    pDMitem->m_timeAllow.parseActionTime( wxString::FromUTF8( p ) );
                }
            }
            
            // Allow Monday
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_DM_ALLOWED_MONDAY ) ) ) {
                atoi( p ) ? 
                    pDMitem->m_timeAllow.allowMonday() : 
                    pDMitem->m_timeAllow.allowMonday( false );
            }
            
            // Allow Tuesday
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                       VSCPDB_ORDINAL_DM_ALLOWED_TUESDAY ) ) ) {
                atoi( p ) ? 
                    pDMitem->m_timeAllow.allowTuesday() : 
                    pDMitem->m_timeAllow.allowTuesday( false );
            }
            
            // Allow Wednesday
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                     VSCPDB_ORDINAL_DM_ALLOWED_WEDNESDAY ) ) ) {
                atoi( p ) ? 
                    pDMitem->m_timeAllow.allowWednesday() : 
                    pDMitem->m_timeAllow.allowWednesday( false );
            }
            
            // Allow Thursday
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                      VSCPDB_ORDINAL_DM_ALLOWED_THURSDAY ) ) ) {
                atoi( p ) ? 
                    pDMitem->m_timeAllow.allowThursday() : 
                    pDMitem->m_timeAllow.allowThursday( false );
            }
            
            // Allow Friday
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_DM_ALLOWED_FRIDAY ) ) ) {
                atoi( p ) ? 
                    pDMitem->m_timeAllow.allowFriday() : 
                    pDMitem->m_timeAllow.allowFriday( false );
            }
            
            // Allow Saturday
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                     VSCPDB_ORDINAL_DM_ALLOWED_SATURDAY ) ) ) {
                atoi( p ) ? 
                    pDMitem->m_timeAllow.allowSaturday() : 
                    pDMitem->m_timeAllow.allowSaturday( false );
            }
            
            // Allow Sunday
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_DM_ALLOWED_SUNDAY ) ) ) {
                atoi( p ) ? 
                    pDMitem->m_timeAllow.allowSunday() : 
                    pDMitem->m_timeAllow.allowSunday( false );
            }

            // bCheckIndex
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                           VSCPDB_ORDINAL_DM_CHECK_INDEX ) ) ) {
                pDMitem->m_bCheckIndex = atoi( p ) ? true : false;
            }
            
            // Index
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                                 VSCPDB_ORDINAL_DM_INDEX ) ) ) {
                pDMitem->m_index = atoi( p );
            }
            
            // bCheckZone
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                            VSCPDB_ORDINAL_DM_CHECK_ZONE ) ) ) {
                pDMitem->m_bCheckZone = atoi( p ) ? true : false;
            }
            
            // Zone
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                                  VSCPDB_ORDINAL_DM_ZONE ) ) ) {
                pDMitem->m_zone = atoi( p );
            }
            
            // bCheckSubZone
            pDMitem->m_bCheckZone = sqlite3_column_int( ppStmt, 
                               VSCPDB_ORDINAL_DM_CHECK_SUBZONE ) ? true : false;
            
            // SubZone
            pDMitem->m_zone = sqlite3_column_int( ppStmt, 
                                                    VSCPDB_ORDINAL_DM_SUBZONE );
            
            // bMeasurementIndex
            pDMitem->m_bCheckMeasurementIndex = sqlite3_column_int( ppStmt, 
                     VSCPDB_ORDINAL_DM_CHECK_MEASUREMENT_INDEX ) ? true : false;
            
            // ActionCode
            pDMitem->m_actionCode = sqlite3_column_int( ppStmt, 
                                                 VSCPDB_ORDINAL_DM_ACTIONCODE );
            
            // ActionParamter
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                       VSCPDB_ORDINAL_DM_ACTIONPARAMETER ) ) ) {
                pDMitem->m_actionparam = wxString::FromUTF8( p );
            }
            
            // Measurement check value flag
            pDMitem->m_bCompareMeasurement = sqlite3_column_int( ppStmt, 
                     VSCPDB_ORDINAL_DM_MEASUREMENT_CHECK_VALUE ) ? true : false;        
            
            // Measurement value
            pDMitem->m_measurementValue = sqlite3_column_double( ppStmt, 
                                           VSCPDB_ORDINAL_DM_MEASUREMENT_VALUE );
            
            // Measurement unit
            pDMitem->m_measurementUnit = sqlite3_column_int( ppStmt, 
                                            VSCPDB_ORDINAL_DM_MEASUREMENT_UNIT );
            
            // Measurement compare code
            pDMitem->m_measurementCompareCode = sqlite3_column_int( ppStmt, 
                                         VSCPDB_ORDINAL_DM_MEASUREMENT_COMPARE );

            // Comment
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, 
                                         VSCPDB_ORDINAL_DM_COMMENT ) ) ) {
                pDMitem->m_comment = wxString::FromUTF8( p );                             
            }                             

            // Add the DM row to the matrix 
           addMemoryElement ( pDMitem );
            
        }
        
    }
    
    sqlite3_finalize( ppStmt );
    
    m_mutexDM.Unlock();
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// loadFromXML
//
// Read decision matrix from file
//

bool CDM::loadFromXML( void )
{
    gpobj->logMsg( wxString::Format( _("[DM] ") + _("Loading decision matrix from: %s\n"), 
                                        (const char *)m_staticXMLPath.mbc_str() ),
                        DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );

    // File must exist
    if ( !wxFile::Exists( m_staticXMLPath ) ) {
        gpobj->logMsg( _("[DM] ") + _("File does not exist.\n"), 
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    wxXmlDocument doc;
    if ( !doc.Load ( m_staticXMLPath ) ) {
        gpobj->logMsg( _("[DM] ") + _("Failed to load DM. Check format!\n"), 
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // start processing the XML file
    if ( doc.GetRoot()->GetName() != _( "dm" ) ) {
        gpobj->logMsg( _("[DM] ") + _("Failed to load DM. <dm> not found, format is wrong!\n"), 
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while ( child ) {

        if ( child->GetName() == _( "row" ) ) {

            // Set row default values for row
            dmElement *pDMitem = new dmElement;
            pDMitem->m_bStatic = true;   // Can not be edited/saved            

            // Check if row is enabled
            wxString strEnabled = child->GetAttribute( _( "enable" ), _("false") );

            strEnabled.MakeUpper();
            if ( wxNOT_FOUND != strEnabled.Find( _("TRUE") ) ) {
                pDMitem->enableRow();
            }
            else {
                pDMitem->disableRow();
            }

            // Get group id
            pDMitem->m_strGroupID = child->GetAttribute( _( "groupid" ), _("") );

            // add the DM row to the matrix
            addMemoryElement( pDMitem );

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                if ( subchild->GetName() == _( "mask" ) ) {
                    wxString str;
                    str = subchild->GetAttribute( _( "priority" ), _("0") );
                    pDMitem->m_vscpfilter.mask_priority = vscp_readStringValue( str );

                    str = subchild->GetAttribute( _( "class" ), _("0") );
                    pDMitem->m_vscpfilter.mask_class = vscp_readStringValue( str );

                    str = subchild->GetAttribute( _( "type" ), _("0") );
                    pDMitem->m_vscpfilter.mask_type = vscp_readStringValue( str );

                    wxString strGUID = subchild->GetAttribute( _( "GUID" ),
                        _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00") );
                    vscp_getGuidFromStringToArray( pDMitem->m_vscpfilter.mask_GUID, 
                                                    strGUID );
                }
                else if ( subchild->GetName() == _( "filter" ) ) {
                    wxString str;
                    str = subchild->GetAttribute( _( "priority" ), _("0") );
                    pDMitem->m_vscpfilter.filter_priority = vscp_readStringValue( str );

                    str = subchild->GetAttribute( _( "class" ), _("0") );
                    pDMitem->m_vscpfilter.filter_class = vscp_readStringValue( str );

                    str = subchild->GetAttribute( _( "type" ), _("0") );
                    pDMitem->m_vscpfilter.filter_type = vscp_readStringValue( str );

                    wxString strGUID = subchild->GetAttribute( _( "GUID" ),
                            _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00") );
                    vscp_getGuidFromStringToArray( pDMitem->m_vscpfilter.filter_GUID, strGUID );
                }
                else if ( subchild->GetName() == _( "action" ) ) {
                    pDMitem->m_actionCode = vscp_readStringValue( subchild->GetNodeContent() );
                }
                else if ( subchild->GetName() == _( "param" ) ){
                    pDMitem->m_actionparam = subchild->GetNodeContent();
                    pDMitem->m_actionparam = pDMitem->m_actionparam.Trim();
                    pDMitem->m_actionparam = pDMitem->m_actionparam.Trim(false);
                }
                else if ( subchild->GetName() == _( "comment" ) ) {
                    pDMitem->m_comment = subchild->GetNodeContent();
                    pDMitem->m_comment = pDMitem->m_comment.Trim();
                    pDMitem->m_comment = pDMitem->m_comment.Trim(false);
                }
                else if ( subchild->GetName() == _( "allowed_from" ) ) {
                    wxString str = subchild->GetNodeContent();
                    str.Trim();
                    str.Trim(false);
                    
                    if ( 0 != str.Length() ) {
                        pDMitem->m_timeAllow.setFromTime( str );
                    }
                    else {
                        pDMitem->m_timeAllow.setFromTime(_("*"));
                    }
                }
                else if ( subchild->GetName() == _( "allowed_to" ) ) {
                    wxString str = subchild->GetNodeContent();
                    str.Trim();
                    str.Trim(false);
                    if ( 0 != str.Length() ) {
                        pDMitem->m_timeAllow.setEndTime(str);
                    }
                    else {
                        pDMitem->m_timeAllow.setEndTime( _("*") );
                    }
                }
                else if ( subchild->GetName() == _( "allowed_weekdays" ) ) {
                    wxString str = subchild->GetNodeContent();
                    str.Trim();
                    str.Trim(false);
                    if ( '*' == str ) str = _("mtwtfss");
                    if ( 0 != str.Length() ) {
                        pDMitem->m_timeAllow.setWeekDays(str);
                    }
                    else {
                        pDMitem->m_timeAllow.setWeekDays( _("mtwtfss") );
                    }
                }
                else if ( subchild->GetName() == _( "allowed_time" ) ) {
                    wxString str = subchild->GetNodeContent();
                    str.Trim();
                    str.Trim(false);
                    if ('*' == str ) str = _("*-*-* *:*:*");
                    if ( 0 != str.Length() ) {
                        pDMitem->m_timeAllow.parseActionTime( str );
                    }
                    else {
                        pDMitem->m_timeAllow.parseActionTime( _("*-*-* *:*:*") );
                    }
                }
                else if ( subchild->GetName() == _( "index" ) ) {
                    
                    wxString str;
                    
                    pDMitem->m_bCheckIndex = true;
                                           
                    // Attribute measurementindex
                    str = subchild->GetAttribute( _( "measurementindex" ), _("false") );
                    str.MakeUpper();
                    if ( wxNOT_FOUND != str.Find(_("TRUE"))) {
                        pDMitem->m_bCheckMeasurementIndex = true;
                    }
                    
                    pDMitem->m_index = 
                                vscp_readStringValue( subchild->GetNodeContent() );                                            
                    
                }
                else if ( subchild->GetName() == _( "zone" ) ) {
                    pDMitem->m_bCheckZone = true;
                    pDMitem->m_zone = vscp_readStringValue( subchild->GetNodeContent() );
                }
                else if ( subchild->GetName() == _( "subzone" ) ) {
                    pDMitem->m_bCheckSubZone = true;
                    pDMitem->m_subzone = vscp_readStringValue( subchild->GetNodeContent() );
                }
                else if ( subchild->GetName() == _( "measurement" ) ) {
                    wxString str;
                                        
                    pDMitem->m_bCompareMeasurement = true;
                    
                    str = subchild->GetAttribute( _( "compare" ), _("noop") );
                    pDMitem->m_measurementCompareCode = getCompareCodeFromToken( str );
                    
                    str = subchild->GetAttribute( _( "unit" ), _("0") );
                    pDMitem->m_measurementUnit = vscp_readStringValue( str );
                    
                    str = subchild->GetAttribute( _( "value" ), _("0") );
                    str.ToCDouble( &pDMitem->m_measurementValue );
                            
                }

                subchild = subchild->GetNext();

            }

        }

        child = child->GetNext();

    }

    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
        gpobj->logMsg( _("[DM] ") + _("Read success.\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// saveToXML
//
// Write decision matrix to file
//

bool CDM::saveToXML( bool bAll )
{
    wxString strLog;
    wxString buf;

    strLog = _("Saving decision matrix to: ") + m_staticXMLPath + _("\n");
    gpobj->logMsg( _("[DM] ") + strLog, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );

    if ( !wxFileName::IsFileWritable( m_staticXMLPath ) ) {
        strLog = _("File is not writable.\n");
        gpobj->logMsg( _("[DM] ") + strLog, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    wxFFileOutputStream *pFileStream = new wxFFileOutputStream ( m_staticXMLPath );
    if ( NULL == pFileStream ) {
        strLog = _("Failed to save: ") + m_staticXMLPath + _(" (memory allocation)\n");
        gpobj->logMsg( _("[DM] ") + strLog, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // Make a copy before we save
    wxCopyFile( m_staticXMLPath, m_staticXMLPath + _("~") );

    pFileStream->Write ( "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n",
            strlen ( "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n" ) );

    m_mutexDM.Lock();

    // DM matrix information start
    pFileStream->Write ( "<dm>\n",strlen ( "<dm>\n" ) );

    DMLIST::iterator it;
    for ( it = m_DMList.begin(); it != m_DMList.end(); ++it ) {

        dmElement *pDMitem = *it;

        if ( NULL != pDMitem ) {  // Must be an dmElement to work with  m_strGroupID

            // Static records is loaded from XML file and if bAll is false only
            // static records should be saved.
            if ( !bAll && !pDMitem->m_bStatic ) continue;

            pFileStream->Write( "  <row enable=\"",strlen ( "  <row enable=\"" ) );
            if ( pDMitem->isEnabled() ) {
                pFileStream->Write("true\" ",strlen("true\" "));
            }
            else {
                pFileStream->Write("false\" ",strlen("false\" "));
            }

            pFileStream->Write("groupid=\"",strlen("groupid=\""));
            pFileStream->Write( pDMitem->m_strGroupID.mb_str(), strlen(pDMitem->m_strGroupID.mb_str()) );
            pFileStream->Write("\" >\n", strlen("\" >\n"));

            pFileStream->Write( "    <mask ",strlen ( "    <mask " ) );
            buf.Printf( _( " priority=\"%d\" " ), pDMitem->m_vscpfilter.mask_priority );
            pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );

            buf.Printf( _( " class=\"%d\" " ), pDMitem->m_vscpfilter.mask_class );
            pFileStream->Write ( buf.mb_str(), strlen( buf.mb_str() ) );

            buf.Printf ( _( " type=\"%d\" " ), pDMitem->m_vscpfilter.mask_type );
            pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );

            buf.Printf( _( " GUID=\" " ) );
            pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );

            wxString strGUID;
            vscp_writeGuidArrayToString( pDMitem->m_vscpfilter. mask_GUID, strGUID );
            pFileStream->Write( strGUID.mb_str(), strlen( strGUID.mb_str() ) );
            pFileStream->Write( "\" > ", strlen( "\" > " ) );
            pFileStream->Write( "</mask>\n", strlen( "</mask>\n" ) );

            pFileStream->Write( "    <filter ", strlen( "    <filter " ) );
            buf.Printf( _( " priority=\"%d\" " ), pDMitem->m_vscpfilter.filter_priority );
            pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
            buf.Printf( _( " class=\"%d\" " ), pDMitem->m_vscpfilter.filter_class );
            pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
            buf.Printf( _( " type=\"%d\" " ), pDMitem->m_vscpfilter.filter_type );
            pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
            buf.Printf( _( " GUID=\" " ) );
            pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
            vscp_writeGuidArrayToString( pDMitem->m_vscpfilter.filter_GUID, strGUID );
            pFileStream->Write( strGUID.mb_str(), strlen( strGUID.mb_str() ) );
            pFileStream->Write( "\" > ", strlen( "\" > " ) );
            pFileStream->Write( "</filter>\n", strlen( "</filter>\n" ) );

            pFileStream->Write( "    <action>", strlen( "    <action>" ) );
            buf.Printf( _( "0x%x" ), pDMitem->m_actionCode );
            pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ));
            pFileStream->Write( "</action>\n", strlen ( "</action>\n" ) );

            pFileStream->Write( "    <param>", strlen ( "    <param>" ) );
            pFileStream->Write( pDMitem->m_actionparam.mb_str(),
                                    strlen( pDMitem->m_actionparam.mb_str() ) );
            pFileStream->Write( "</param>\n", strlen ( "</param>\n" ) );

            pFileStream->Write( "    <comment>", strlen ( "    <comment>" ) );
            pFileStream->Write( pDMitem->m_comment.mb_str(),
                                    strlen(pDMitem->m_comment.mb_str()) );
            pFileStream->Write( "</comment>\n", strlen ( "</comment>\n" ) );

            pFileStream->Write( "    <allowed_from>", strlen ( "    <allowed_from>" ) );
            {
                wxString str = pDMitem->m_timeAllow.getFromTime().FormatISODate() + _(" ") +
                                    pDMitem->m_timeAllow.getFromTime().FormatISOTime();
                pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
            }
            pFileStream->Write( "</allowed_from>\n", strlen( "</allowed_from>\n" ) );

            pFileStream->Write ( "    <allowed_to>", strlen( "    <allowed_to>" ) );
            {
                wxString str = pDMitem->m_timeAllow.getEndTime().FormatISODate() + _(" ") +
                                    pDMitem->m_timeAllow.getEndTime().FormatISOTime();
                pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
            }
            pFileStream->Write("</allowed_to>\n", strlen ( "</allowed_to>\n" ) );

            pFileStream->Write("    <allowed_weekdays>", strlen ( "    <allowed_weekdays>" ) );
            {
                wxString str = pDMitem->m_timeAllow.getWeekDays();
                pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
            }
            pFileStream->Write( "</allowed_weekdays>\n", strlen ( "</allowed_weekdays>\n" ) );

            pFileStream->Write( "    <allowed_time>", strlen ( "    <allowed_time>" ) );
            {
                wxString str = pDMitem->m_timeAllow.getActionTimeAsString();
                pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
            }
            pFileStream->Write( "</allowed_time>\n",strlen ( "</allowed_time>\n" ) );

            // Index
            pFileStream->Write( "    <index ", strlen( "    <index " ) );
            buf.Printf( _( " bCheckMeasurementIndex=\"%s\" " ),
                    (pDMitem->m_bCheckMeasurementIndex) ? _("true") : _("false") );
            pFileStream->Write( buf.mb_str(), strlen(buf.mb_str()) );
            pFileStream->Write( " > ", strlen( " > " ) );
            buf.Printf( _( "%d" ), pDMitem->m_index );
            pFileStream->Write( buf.mb_str(), strlen(buf.mb_str()) );
            pFileStream->Write( "</index>\n", strlen ( "</index>\n" ) );

            // Zone
            pFileStream->Write( "    <zone>", strlen ( "    <zone>" ) );
            buf.Printf( _( "%d" ), pDMitem->m_zone );
            pFileStream->Write( buf.mb_str(), strlen(buf.mb_str()) );
            pFileStream->Write( "</zone>\n", strlen ( "</zone>\n" ) );

            // Subzone
            pFileStream->Write( "    <subzone>", strlen ( "    <subzone>" ) );
            buf.Printf( _( "%d" ), pDMitem->m_subzone );
            pFileStream->Write( buf.mb_str(), strlen(buf.mb_str()) );
            pFileStream->Write( "</subzone>\n", strlen ( "</subzone>\n" ) );
            
            // measurement
            pFileStream->Write( "    <measurement ", strlen ( "    <measurement " ) );
            if ( pDMitem->m_bCompareMeasurement ) {
                pFileStream->Write( "enable=\"true\" ", strlen ( "enable=\"true\" " ) );
            }
            else {
                pFileStream->Write( "enable=\"false\" ", strlen ( "enable=\"false\" " ) );
            }
            
            wxString strCode = getTokenFromCompareCode( pDMitem->m_measurementCompareCode );
            buf = _("compare=\"") + strCode + "\" ";
            pFileStream->Write( buf.mbc_str(), strlen( buf.mbc_str() ) );
            
            buf.Printf( _( "unit=\"%d\" " ), pDMitem->m_measurementUnit );
            pFileStream->Write( buf.mb_str(), strlen(buf.mb_str()) );
            
            buf.Printf( _( "value=\"%lf\" " ), pDMitem->m_measurementValue );
            pFileStream->Write( buf.mb_str(), strlen(buf.mb_str()) );
            
            pFileStream->Write( " />\n", strlen ( " />\n" ) );


            pFileStream->Write( "  </row>\n\n",strlen( "  </row>\n\n" ) );

        }

    }

    // DM matrix information end
    pFileStream->Write ( "</dm>",strlen ( "</dm>" ) );

    m_mutexDM.Unlock();

    // Close the file
    pFileStream->Close();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// feed
//

bool CDM::feed( vscpEvent *pEvent )
{
    m_mutexDM.Lock();

    DMLIST::iterator it;
    for ( it = m_DMList.begin(); it != m_DMList.end(); ++it ) {

        dmElement *pDMitem = *it;

        // Skip if disabled
        if ( !pDMitem->isEnabled() ) continue;

        if ( vscp_doLevel2Filter( pEvent, &pDMitem->m_vscpfilter ) &&
                                            pDMitem->m_timeAllow.ShouldWeDoAction() ) {

            if ( pDMitem->isCheckIndexSet() ) {
                if ( pDMitem->m_bCheckMeasurementIndex ) {
                    if ( ( 0 == pEvent->sizeData ) ||
                        ( ( pEvent->pdata[0] & 7 ) != pDMitem->m_index ) ) continue;
                }
                else {
                    if ( ( 0 == pEvent->sizeData ) ||
                        ( pEvent->pdata[0] != pDMitem->m_index ) ) continue;
                }
            }

            if ( pDMitem->isCheckZoneSet() ) {
                if ( ( 2 > pEvent->sizeData ) ||
                    ( pEvent->pdata[1] != pDMitem->m_zone ) ) continue;
            }
                
            if ( pDMitem->isCheckSubZoneSet() ) {
                if ( ( 3 > pEvent->sizeData ) ||
                    ( pEvent->pdata[2] != pDMitem->m_subzone ) ) continue;
            }
                
            // Check if measurement value should be compared. 
            if ( pDMitem->isCompareMeasurementSet() ) {
                    
                // Must be a measurement event
                if ( !vscp_isVSCPMeasurement( pEvent ) ) continue;

                // Unit must be same
                if ( pDMitem->m_measurementUnit != vscp_getVSCPMeasurementUnit( pEvent ) ) continue;
                
                double value;
                if ( !vscp_getVSCPMeasurementAsDouble( pEvent, &value ) ) {
                    wxString strEvent;
                    vscp_writeVscpEventToString( pEvent, strEvent );
                    wxString wxstr = _("Conversion to double failed for measurement. Event=") +
                                        strEvent + _("\n");         
                    gpobj->logMsg ( _("[DM] ") + wxstr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                    continue;
                }
                

                switch ( pDMitem->m_measurementCompareCode ) {
                    
                    case DM_MEASUREMENT_COMPARE_NOOP:
                        // We always accept
                        break;
                        
                    case DM_MEASUREMENT_COMPARE_EQ:
                        if ( !vscp_almostEqualRelativeDouble( value, pDMitem->m_measurementValue ) ) continue;
                        break;   
                        
                    case DM_MEASUREMENT_COMPARE_NEQ:
                        if ( vscp_almostEqualRelativeDouble( value, pDMitem->m_measurementValue ) ) continue;
                        break;
                        
                    case DM_MEASUREMENT_COMPARE_LT:
                        if ( vscp_almostEqualRelativeDouble( value, pDMitem->m_measurementValue ) ||
                                ( value > pDMitem->m_measurementValue ) ) continue;
                        break;
                        
                    case DM_MEASUREMENT_COMPARE_LTEQ:
                        if ( value > pDMitem->m_measurementValue ) continue;
                        break;
                        
                    case DM_MEASUREMENT_COMPARE_GT:                        
                        if ( vscp_almostEqualRelativeDouble( value, pDMitem->m_measurementValue ) || 
                                ( value < pDMitem->m_measurementValue ) ) continue;
                        break;
                        
                    case DM_MEASUREMENT_COMPARE_GTEQ:
                        if ( value < pDMitem->m_measurementValue ) continue;
                        break;    
                }
                    
            }

            // Match do action for this row
            pDMitem->doAction( pEvent );

        }

    }

    m_mutexDM.Unlock();

    return true;
}







//------------------------------------------------------------------------------







////////////////////////////////////////////////////////////////////////////////
// actionThreadURL
//
// This thread connects to a HTTP server on a specified port
//

actionThread_URL::actionThread_URL( CControlObject *pCtrlObject,
                                    wxString& url,
                                    accessmethod_t nAccessMethod,
                                    wxString& putdata,
                                    wxString& extraheaders,
                                    wxString& proxy,
                                    wxThreadKind kind )
                                    : wxThread( kind )
{
    m_bOK = true;

    //OutputDebugString( "actionThreadURL: Create");

    // Set URL
    if (  wxURL_NOERR != m_url.SetURL( url ) ) {
        m_bOK = false;
    }

    m_extraheaders.Empty();

    int pos;
    wxString wxwrk = extraheaders;
    while ( wxNOT_FOUND != ( pos = wxwrk.Find( _("\\n") ) ) ) {
        m_extraheaders += wxwrk.Left( pos );
        m_extraheaders += _("\r\n");
        wxwrk = wxwrk.Right( wxwrk.Length()-pos-2 );
    }

    m_acessMethod = nAccessMethod;
    m_putdata = putdata;

    m_url.SetProxy( proxy );
}

actionThread_URL::~actionThread_URL()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//
// http://www.jmarshall.com/easy/http/
//

void *actionThread_URL::Entry()
{
    //m_pCtrlObject->logMsg ( _("[DM] ") + _( "TCP actionThreadURL: Quit.\n" ), 
    //                            DAEMON_LOGMSG_INFO );

    wxIPV4address addr;
    wxSocketClient sock;

    addr.Hostname( m_url.GetServer() );
    addr.Service( m_url.GetPort() );

    if ( sock.Connect( addr ) ) {

        wxString wxstr;
        wxString wxwork;

        // Check if access method is GET
        if ( actionThread_URL::GET == m_acessMethod ) {

            wxstr = _("GET ");
            wxstr += m_url.GetPath();
            wxstr += _("?");
            wxstr += m_url.GetQuery();
            wxstr += _(" ");
            wxstr += _("HTTP/1.1\r\n");
            wxstr += _("Host: ");
            wxstr += m_url.GetServer();
            wxstr += _(":");
            wxstr += m_url.GetPort();
            wxstr += _("\r\n");
            wxstr += wxString::Format( _("User-Agent: VSCPD/%s\r\n"),
                                        VSCPD_DISPLAY_VERSION );

            // Add extra headers if there are any
            if ( m_extraheaders.Length() ) {
                wxstr += m_extraheaders;
            }
            wxstr += _("\r\n\r\n");

        }
        // OK the access method is POST
        else if ( actionThread_URL::POST == m_acessMethod ) {

            wxstr = _("POST ");
            wxstr += m_url.GetPath();
            wxstr += _(" ");
            wxstr += _("HTTP/1.1\r\n");
            wxstr += _("Host: ");
            wxstr += m_url.GetServer();
            wxstr += _(":");
            wxstr += m_url.GetPort();
            wxstr += _("\r\n");
            wxstr += wxString::Format( _("User-Agent: VSCPD/%s\r\n"),
                                        VSCPD_DISPLAY_VERSION );
            // Add extra headers if there are any
            if ( m_extraheaders.Length() ) {
                wxstr += m_extraheaders;
            }

            wxstr += _("Accept: */*\r\n");
            wxstr += _("Content-Type: application/x-www-form-urlencoded\r\n");
            wxstr += _("Content-Length: ");
            wxstr += wxString::Format(_("%ld"),m_putdata.Length());
            wxstr += _("\r\n\r\n");
            wxstr += m_putdata;
            wxstr += _("\r\n");

        }
        // OK the access method is PUT
        else if ( actionThread_URL::PUT == m_acessMethod ) {

            wxstr = _("PUT ");
            wxstr += m_url.GetPath();
            wxstr += _(" ");
            wxstr += _("HTTP/1.1\r\n");
            wxstr += _("Host: ");
            wxstr += m_url.GetServer();
            wxstr += _(":");
            wxstr += m_url.GetPort();
            wxstr += _("\r\n");
            wxstr += wxString::Format( _("User-Agent: VSCPD/%s\r\n"),
                                        VSCPD_DISPLAY_VERSION );

            // Add extra headers if there are any
            if ( m_extraheaders.Length() ) {
                wxstr += m_extraheaders;
            }

            wxstr += _("Content-Type: application/x-www-form-urlencoded\r\n");
            wxstr += _("Content-Length: ");
            wxstr += wxString::Format(_("%ld"),m_putdata.Length());
            wxstr += _("\r\n\r\n");
            wxstr += m_putdata;
            wxstr += _("\r\n");

        }
        else {

            // Invalid method
            gpobj->logMsg( _("[DM] ") + _( "actionThreadURL: Invalid http "
                                           "access method: " ) +
                                m_url.GetServer() +
                                _(",") +
                                m_url.GetPort() +
                                _(",") +
                                m_url.GetPath() +
                                _(",") +
                                wxString::Format( _("acessMethod = %d" ), 
                                                  m_acessMethod ) +
                                _(" \n"), 
                                DAEMON_LOGMSG_NORMAL, 
                               DAEMON_LOGTYPE_DM  );

        }

        if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
            gpobj->logMsg( _("[DM] ") + _( "actionThreadURL: Request: \n" ) +
                            wxstr,
                            DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
        }

        // Send the request
        sock.Write( wxstr, wxstr.Length() );
#if  wxMAJOR_VERSION >=3
        if ( sock.Error() || (  wxstr.Length() != sock.LastWriteCount() ) ) {
#else
	if ( sock.Error() ) {
#endif
            // There was an error
            gpobj->logMsg(_("[DM] ") + _( "actionThreadURL: Error writing request: " ) +
                                m_url.GetServer() +
                                _(",") +
                                m_url.GetPort() +
                                _(",") +
                                m_url.GetPath() +
                                _(",") +
                                wxString::Format( _("acessMethod = %d" ), m_acessMethod ) +
                                _(" \n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM  );
        }

        // Get the response
        char buffer[ 8192 ];
        wxString strReponse;

        //while ( !sock.Read( buffer, sizeof( buffer ) ).LastReadCount() );
        sock.Read( buffer, sizeof( buffer ) );
        if ( !sock.Error() ) {

            // OK, Check the response
            strReponse = wxString::FromUTF8( buffer );

            // Log response
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                gpobj->logMsg( _("[DM] ") + _( "actionThreadURL: OK Response: " ) +
                                strReponse +
                                _(" \n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            }

            wxStringTokenizer tkz( strReponse );
            if ( tkz.HasMoreTokens() ) {

                wxString str = tkz.GetNextToken();
                if ( wxNOT_FOUND != str.Find( _("OK") ) ) {

                    // Something is wrong
                    gpobj->logMsg( _("[DM] ") + _( "actionThreadURL: Error reading response: " ) +
                                        m_url.GetServer() +
                                        _(",") +
                                        m_url.GetPort() +
                                        _(",") +
                                        m_url.GetPath() +
                                        _(",") +
                                        wxString::Format( _("acessMethod = %d" ), m_acessMethod ) +
                                        _(", Response = ") +
                                        strReponse +
                                        _(" \n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM  );

                }
            }

        }
        else {
            // There was an error
            gpobj->logMsg ( _("[DM] ") + _( "actionThreadURL: Error reading response: " ) +
                                m_url.GetServer() +
                                _(",") +
                                m_url.GetPort() +
                                _(",") +
                                m_url.GetPath() +
                                _(",") +
                                ( m_acessMethod ? _("PUT") : _("GET") ) +
                                _(" \n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM  );
        }

        // Close the socket
        sock.Close();

    }
    else {
        // There was an error connecting
        gpobj->logMsg( _("[DM] ") + _( "actionThreadURL: Unable to connect: " ) +
                                m_url.GetServer() +
                                _(",") +
                                m_url.GetPort() +
                                _(",") +
                                m_url.GetPath() +
                                _(",") +
                                wxString::Format( _("acessMethod = %d" ), m_acessMethod ) +
                                _(" \n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM  );
    }

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void actionThread_URL::OnExit()
{

}







//------------------------------------------------------------------------------
//                                 VSCP server
//------------------------------------------------------------------------------







///////////////////////////////////////////////////////////////////////////////
// actionThread_VSCPSrv
//
// This thread connects to remote server and send event.
//

actionThread_VSCPSrv::actionThread_VSCPSrv( CControlObject *pCtrlObject,
                                                wxString& strHostname,
                                                short port,
                                                wxString& strUsername,
                                                wxString& strPassword,
                                                wxString& strEvent,
                                                wxThreadKind kind )
                                                : wxThread( kind )
{
    //OutputDebugString( "actionThreadURL: Create");
    m_strHostname = strHostname;
    m_port = port;
    m_strUsername = strUsername;
    m_strPassword = strPassword;
    vscp_setVscpEventExFromString( &m_eventEx, strEvent );
}

actionThread_VSCPSrv::~actionThread_VSCPSrv()
{

}


///////////////////////////////////////////////////////////////////////////////
// Entry
//
//

void *actionThread_VSCPSrv::Entry()
{
    int rv;
    int tries;
    VscpRemoteTcpIf client;
    wxString interface = wxString::Format( _("tcp://%s:%d"), 
                                            (const char *)m_strHostname.mbc_str(),
                                            m_port );
    // Allow for **VERY** slower clients
    client.setResponseTimeout( 5000 );

    tries = 3;
    while ( true ) {
        if ( CANAL_ERROR_SUCCESS == 
                ( rv = client.doCmdOpen( interface,
                                            m_strUsername,
                                            m_strPassword ) ) ) {
            break;
        }
        else {                                                
                        
            if ( CANAL_ERROR_TIMEOUT != rv ) {
                goto connect_error;
            }
            
            tries--;
            if ( tries > 0 ) {
                wxSleep( 1 );
                continue;
            }

connect_error:
            // Failed to connect
            gpobj->logMsg( _("[DM] ") + 
                    _( "actionThreadVSCPSrv: Unable to connect to remote server : " ) +
                    m_strHostname + 
                    wxString::Format( _(" Return code = %d (%d)"), rv, tries ) +
                    _(" \n"), 
                    DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM  );
        
            return NULL;
        }
    } // while

    // Connected - Send the event
    
    tries = 3;
    while ( true ) {

        if ( CANAL_ERROR_SUCCESS == client.doCmdSendEx( &m_eventEx ) ) {
            break;
        }
        else {
            tries--;
            if ( tries > 0 ) {
                wxSleep( 1 );
                continue;
            }
            
            // Failed to send event
            gpobj->logMsg( _("[DM] ") + 
            _( "actionThreadVSCPSrv: Unable to send event to remote server : " ) +
            m_strHostname +
            _(" \n"), 
                DAEMON_LOGMSG_NORMAL, 
                DAEMON_LOGTYPE_DM  );
            break;    
        }
        
    } // while
    
    if ( CANAL_ERROR_SUCCESS != client.doCmdClose() ) {
        // Failed to send event
        gpobj->logMsg( _("[DM] ") + 
          _( "actionThreadVSCPSrv: Unable to close connection to remote server : " ) +
          m_strHostname +
          _(" \n"), 
                DAEMON_LOGMSG_NORMAL, 
                DAEMON_LOGTYPE_DM  );
    }

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void actionThread_VSCPSrv::OnExit()
{
    ;
}








//------------------------------------------------------------------------------
//                                     TABLE
//------------------------------------------------------------------------------







///////////////////////////////////////////////////////////////////////////////
// actionThread_Table
//
// This thread handles a table action
//

actionThread_Table::actionThread_Table( wxString &strParam,
                                            vscpEvent *pEvent,
                                            wxThreadKind kind )
                                                : wxThread( kind )
{
    m_pFeedEvent = NULL;
    m_strParam = strParam;
    
    
    if ( NULL != pEvent ) {
        m_pFeedEvent = new vscpEvent;
        if ( NULL != m_pFeedEvent ) {
            vscp_copyVSCPEvent( m_pFeedEvent, pEvent );
        }
    }
    
}

actionThread_Table::~actionThread_Table()
{
    if ( NULL != m_pFeedEvent ) {
        vscp_deleteVSCPevent( m_pFeedEvent );
        m_pFeedEvent = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
// Actionparameter: 
//      0;tablename;datetime;value[;[BASE64:]sql-expression]
//      1;tablename;[BASE64:]sql-expression


void *actionThread_Table::Entry()
{    
    CVSCPTable *pTable;     // Table object
    wxString wxstrErr;
    wxDateTime dt = wxDateTime::Now();
    double value = 0;
    int type;               // Action parameter type
    wxString name;          // Table name
    wxString sql;           // sql expression
    
    wxStringTokenizer tkz( m_strParam, _(";") );

    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter
        wxstrErr = _( "[Action] Write Table: Action parameter is not correct. "
                      "Parameter= ");
        wxstrErr += m_strParam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
        return NULL;
    }
    
    // Get action parameter format
    type = vscp_readStringValue( tkz.GetNextToken() );
    
    if ( !tkz.HasMoreTokens() ) {
        // Missing table name
        wxstrErr = _( "[Action] Write Table: Action parameter is not "
                      "correct. No tablename. Parameter= ");
        wxstrErr += m_strParam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
        return NULL;
    }
    
    // Get table name
    name = tkz.GetNextToken();
    
    // Get table object
    if ( NULL == ( pTable = gpobj->m_userTableObjects.getTable( name ) ) ) {        
        wxstrErr = _( "[Action] Write Table: A table with that name was not "
                      "found. Parameter= ");
        wxstrErr += m_strParam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
        return NULL;        
    }
    
#if 0  // Test of getNumberOfRecordsForRange
    wxDateTime from,to;
    from.ParseDateTime("0000-01-01 00:00:00");
    to.ParseDateTime("9999-06-01 00:00:00");
    double count;
    pTable->getNumberOfRecordsForRange( from, to, &count );
#endif  
    
#if 0  // Test of getxxxxxValue
    wxDateTime from,to;
    from.ParseDateTime("0000-01-01 00:00:00");
    to.ParseDateTime("9999-06-01 00:00:00");
    double val;
    //pTable->getSumValue( from, to, &val );
    //pTable->getMinValue( from, to, &val );
    //pTable->getMaxValue( from, to, &val );
    //pTable->getAverageValue( from, to, &val );
    //pTable->getMedianValue( from, to, &val );
    //pTable->getStdevValue( from, to, &val );
    //pTable->getVarianceValue( from, to, &val );
    //pTable->getModeValue( from, to, &val );
    pTable->getUppeQuartileValue( from, to, &val );
#endif

    
    
#if 0  // TEST Get range of data
    wxPrintf( _(">>>>>") + wxDateTime::Now().FormatISOTime() + _("\n") );
    
    sqlite3_stmt *ppStmt;
    wxDateTime from,to;
    from.ParseDateTime("0000-01-01 00:00:00");
    to.ParseDateTime("9999-06-01 00:00:00");
   
    if ( pTable->prepareRangeOfData( from, to, &ppStmt, true ) ) {
        
        wxString rowData;
        while ( pTable->getRowRangeOfData( ppStmt, rowData ) ) {
            gpobj->logMsg( _("[DM] ") + rowData, 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
        }
        
        pTable->finalizeRangeOfData( ppStmt );
    }
    
    wxPrintf( _(">>>>>") + wxDateTime::Now().FormatISOTime() + _("\n") );
#endif    
    
    
    
    if ( 0 == type ) {
        
        if ( !tkz.HasMoreTokens() ) {
            wxstrErr = _( "[Action] Write Table: Action parameter is "
                          "not correct. Datetime is missing. Parameter= ");
            wxstrErr += m_strParam;
            wxstrErr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstrErr, 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
            return NULL;
        }
        
        // Get date/time
        dt.ParseDateTime( tkz.GetNextToken() );
        
        if ( !tkz.HasMoreTokens() ) {
            wxstrErr = _( "[Action] Write Table: Action parameter is not "
                          "correct. value is missing. Parameter= ");
            wxstrErr += m_strParam;
            wxstrErr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstrErr, 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
            return NULL;
        }
        
        // Get value
        if ( !tkz.GetNextToken().ToCDouble( &value ) ) {
            wxstrErr = _( "[Action] Write Table: Could not get value "
                          "(invalid?). Parameter= ");
            wxstrErr += m_strParam;
            wxstrErr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstrErr, 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_DM );
            return NULL;
        }
        
        // format: dattime;value
        if ( !tkz.HasMoreTokens() ) {
            
            wxString sql = pTable->getSQLInsert();
            
            // Escapes
            dmElement::handleEscapes( m_pFeedEvent, sql );
            
            // Log the data
            dt.SetMillisecond( m_pFeedEvent->timestamp / 1000 );
            if ( !pTable->logData( dt, value, sql ) ) {
                gpobj->logMsg( _("[DM] ") + _( "[Action] Write Table: "
                                         "Failed to log data (datetime,value)"), 
                                    DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            }
        
        }
        // format: datetime;value;sql
        else {
            
            // Get custom sql expression
            sql = tkz.GetNextToken();
                        
            wxString strResult;
            if ( !vscp_decodeBase64IfNeeded( sql, strResult ) ) {
                gpobj->logMsg( _("[DM] ") + _( "[Action] Write Table: "
                         "Failed to decode SQL string. Will continue anyway."), 
                                    DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                strResult = sql;
            }
            
            // Escapes
            dmElement::handleEscapes( m_pFeedEvent, strResult );
                    
            // Log the data
            dt.SetMillisecond( m_pFeedEvent->timestamp / 1000 );
            if ( !pTable->logData( dt, value, strResult ) ) {
                gpobj->logMsg( _("[DM] ") + _( "[Action] Write Table: Failed "
                                               "to log data (datetime,value,sql)"), 
                                    DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            }
            
        }
        
    }
    // Format: [BASE64:]sql-expression
    else if ( 1 == type ) {
        
        if ( !tkz.HasMoreTokens() ) {
            wxstrErr = _( "[Action] Write Table: Action parameter is not "
                          "correct. SQL expression is missing. Parameter= ");
            wxstrErr += m_strParam;
            wxstrErr += _("\n");
            gpobj->logMsg( _("[DM] ") + wxstrErr, 
                                DAEMON_LOGMSG_NORMAL, 
                                DAEMON_LOGTYPE_DM );
            return NULL;
        }
        
        // Get custom sql expression
        sql = tkz.GetNextToken();
            
        wxString strResult;
        if ( !vscp_decodeBase64IfNeeded( sql, strResult ) ) {
            gpobj->logMsg( _("[DM] ") + _( "[Action] Write Table: Failed to "
                                           "decode sql string. Will continue "
                                           "anyway."), 
                                    DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            strResult = sql;
        }
        
        // Log the data
        if ( !pTable->logData( strResult ) ) {
            gpobj->logMsg( _("[DM] ") + _( "[Action] Write Table: "
                                           "Failed to log data (sql)"), 
                                    DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        }
        
    }
    else {
        wxstrErr = _( "[Action] Write Table: Invalid type. Parameter= ");
        wxstrErr += m_strParam;
        wxstrErr += _("\n");
        gpobj->logMsg( _("[DM] ") + wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );        
    }
    
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void actionThread_Table::OnExit()
{

}


