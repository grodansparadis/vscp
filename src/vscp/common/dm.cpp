// dm.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2017
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
#include <wx/base64.h>

#include <stdlib.h>
#include <string.h>
#include <float.h>

#include <vscp.h>
#include <vscpdb.h>
#include <version.h>
#include <vscp_debug.h>
#include <vscphelper.h>
#include <vscpeventhelper.h>
#include <actioncodes.h>
#include <vscpvariable.h>
#include <userlist.h>
#include <controlobject.h>
#include <vscpremotetcpif.h>
#include <vscp_javascript.h>
#include <duktape.h>
#include <duktape_callbacks.h>
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

dmTimer::dmTimer( wxString& nameVar, uint32_t delay, bool bStart, bool setValue )
{
    m_pThread = NULL;       // No working thread

    m_bActive = bStart;
    m_delay = delay;
    m_bSetValue = setValue;
    m_nameVariable = nameVar;
}

///////////////////////////////////////////////////////////////////////////////
// Destructor dmTimer
//

dmTimer::~dmTimer()
{

}




//-----------------------------------------------------------------------------





///////////////////////////////////////////////////////////////////////////////
// Constructor  actionTime
//

actionTime::actionTime()
{
    for ( int i=0; i<7; i++ ) {
        m_weekDay[ i ] = true;  // Allow for all weekdays
    }

    // Allow from the beginning of time
    m_fromTime.ParseDateTime( _("0000-01-01 00:00:00") );

    // to the end of time
    m_endTime.ParseDateTime( _("9999-12-31 23:59:59") );

    // Just leave the ACTIONTIME lists empty as
    // that is no care.

}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

actionTime::~actionTime()
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
        unsigned long val;

        while ( tkz.CountTokens() ) {
            token = tkz.GetNextToken();
            if ( token.ToULong( &val ) ) {
                int *pInt = new int;
                if ( NULL != pInt ) {
                    *pInt = (int)val;
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

    // Formats:
    // YYYY-MM-SS HH:MM:SS
    // * *
    // *-*-* *:*:*
    // YYYY-0/1/4/5-DD HH:MM:SS or variants of it
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
    m_measurementIndex = 0;
    
    m_bCheckZone = false;
    m_zone = 0;
    
    m_bCheckSubZone = false;
    m_subzone = 0;
    
    m_timeAllow.m_fromTime.ParseDateTime( _("1970-01-01 00:00:00") );
    m_timeAllow.m_endTime.ParseDateTime( _("2199-12-31 23:59:59") );
    m_timeAllow.parseActionTime( _("*:*:*" ) );
    m_timeAllow.setWeekDays(_("mtwtfss"));
    
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
    m_measurementIndex = dm.m_measurementIndex;
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
    strRow += m_timeAllow.m_fromTime.FormatISODate() + _(" ") +
                                m_timeAllow.m_fromTime.FormatISOTime() + _(",");

    // End time
    strRow += m_timeAllow.m_endTime.FormatISODate() + _(" ") +
                                m_timeAllow.m_endTime.FormatISOTime() + _(",");

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
    
    // measurementindex
    strRow += wxString::Format(_("%d,"), m_measurementIndex );
    
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
    strRow += wxString::Format(_("%f,"), m_measurementValue );
    
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
    
    // measurementindex
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( wxstr.ToULong( &lval ) ) {
            m_measurementIndex = lval;
        }
        else {
            m_measurementIndex = 0;
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
            if ( str.StartsWith( wxT("%%"), &str ) ) {
                strResult += wxT("%");                  // add percent
            }
            // Check for ';' i.e. %;
            else if ( str.StartsWith( wxT("%;"), &str ) ) {
                strResult += wxT(";");                  // add semicolon
            }
            // Check for %cr
            else if ( str.StartsWith( wxT("%cr"), &str ) ) {
                strResult += wxT("\r");                 // add carrige return
            }
            // Check for %lf
            else if ( str.StartsWith( wxT("%cr"), &str ) ) {
                strResult += wxT("\n");                 // add new line
            }
            // Check for %crlf
            else if ( str.StartsWith( wxT("%crlf"), &str ) ) {
                strResult += wxT("\r\n");               // add carrige return line feed
            }
            // Check for %tab
            else if ( str.StartsWith( wxT("%tab"), &str ) ) {
                strResult += wxT("\t");                 // add tab
            }
            // Check for %bell
            else if ( str.StartsWith( wxT("%bell"), &str ) ) {
                strResult += wxT("\a");                 // add bell
            }
            // Check for %amp.html
            else if ( str.StartsWith( wxT("%amp.html"), &str ) ) {
                strResult += wxT("&amp;");              // add bell
            }
            // Check for %amp
            else if ( str.StartsWith( wxT("%amp"), &str ) ) {
                strResult += wxT("&");                  // add bell
            }
            // Check for %lt.html
            else if ( str.StartsWith( wxT("%lt.html"), &str ) ) {
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
            else if ( str.StartsWith( wxT("%event.class"), &str ) ) {
                strResult +=  wxString::Format( wxT("%d"), pEvent->vscp_class );
            }
            // Check for class string  escape
            else if ( str.StartsWith( wxT("%event.class.str"), &str ) ) {
                VSCPInformation info;
                strResult +=  info.getClassDescription( pEvent->vscp_class );
            }
            // Check for type escape
            else if ( str.StartsWith( wxT("%event.type"), &str ) ) {
                strResult +=  wxString::Format( wxT("%d"), pEvent->vscp_type );
            }
            // Check for type string escape
            else if ( str.StartsWith( wxT("%event.type.str"), &str ) ) {
                VSCPInformation info;
                strResult +=  info.getTypeDescription( pEvent->vscp_class,
                                                    pEvent->vscp_type );
            }
            // Check for data[n] escape
            else if ( str.StartsWith( wxT("%event.data["), &str ) ) {

                // Must be data
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {

                    wxString wxstr = str;
                    if ( wxNOT_FOUND != ( pos = str.Find( wxT("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }

                    long idx=0;
                    wxstr.ToLong( &idx );
                    if ( idx < pEvent->sizeData ) {
                        strResult +=  wxString::Format( wxT("%d"), pEvent->pdata[ idx ] );
                    }
                    else {
                        strResult +=  wxT("?");     // invalid index
                    }


                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( wxT("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );
                    }
                    strResult +=  wxT("");      // No data
                }
            }
            // Check for sizedata escape
            else if ( str.StartsWith( wxT("%event.sizedata"), &str ) ) {
                strResult +=  wxString::Format( wxT("%d"), pEvent->sizeData );
            }
            // Check for data escape
            else if ( str.StartsWith( wxT("%event.data"), &str ) ) {
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    wxString wxstr;
                    vscp_writeVscpDataToString( pEvent, wxstr, false );
                    strResult +=  wxstr;
                }
                else {
                    strResult +=  wxT("empty");     // No data
                }
            }
            // Check for guid escape
            else if ( str.StartsWith( wxT("%event.guid"), &str ) ) {
                wxString strGUID;
                vscp_writeGuidToString ( pEvent, strGUID );
                strResult +=  strGUID;
            }
            // Check for nickname escape
            else if ( str.StartsWith( wxT( "%event.nickname" ), &str ) ) {
                strResult += wxString::Format( wxT( "%d" ),
                                                pEvent->GUID[ VSCP_GUID_LSB ] );
            }
            // Check for obid escape
            else if ( str.StartsWith( wxT("%event.obid"), &str ) ) {
                strResult +=  wxString::Format( wxT("%d"), pEvent->obid );
            }
            // Check for timestamp escape
            else if ( str.StartsWith( wxT("%event.timestamp"), &str ) ) {
                strResult +=  wxString::Format( wxT("%d"), pEvent->timestamp );
            }
            // Check for event index escape
            else if ( str.StartsWith( wxT( "%event.index" ), &str ) ) {
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    strResult += wxString::Format( wxT( "%d" ), pEvent->pdata[ 0 ] );
                }
                else {
                    strResult += wxT( "empty" );        // No data
                }
            }
            // Check for event zone escape
            else if ( str.StartsWith( wxT( "%event.zone" ), &str ) ) {
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    strResult += wxString::Format( wxT( "%d" ), pEvent->pdata[ 1 ] );
                }
                else {
                    strResult += wxT( "empty" );        // No data
                }
            }
            // Check for event subzone escape
            else if ( str.StartsWith( wxT( "%event.subzone" ), &str ) ) {
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    strResult += wxString::Format( wxT( "%d" ), pEvent->pdata[ 2 ] );
                }
                else {
                    strResult += wxT( "empty" );        // No data
                }
            }
            // Check for event escape
            else if ( str.StartsWith( wxT("%event"), &str ) ) {
                wxString strEvent;
                vscp_writeVscpEventToString( pEvent, strEvent );
                strResult += strEvent;
            }
            // Check for isodate escape
            else if ( str.StartsWith( wxT("%isodate"), &str ) ) {
                strResult += wxDateTime::Now().FormatISODate();
            }
            // Check for isotime escape
            else if ( str.StartsWith( wxT("%isotime"), &str ) ) {
                strResult += wxDateTime::Now().FormatISOTime();
            }
            // Check for isobothms escape
            else if ( str.StartsWith( wxT("%isobothms"), &str ) ) {
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
            else if ( str.StartsWith( wxT("%isoboth"), &str ) ) {
                strResult += wxDateTime::Now().FormatISODate();
                strResult += _("T");
                strResult += wxDateTime::Now().FormatISOTime();
            }            
            // Check for mstime escape
            else if ( str.StartsWith( wxT("%mstime"), &str ) ) {
                strResult += wxString::Format( wxT("%d"),
                                                wxDateTime::Now().GetTicks() );
            }
            // Check for unixtime escape
            else if ( str.StartsWith( wxT("%unixtime"), &str ) ) {
                time_t now = time(NULL);    // Get current time
                uint64_t tnow = now;
                strResult += wxString::Format( wxT("%llu"), tnow );
            }
            // Check for hour escape
            else if ( str.StartsWith( wxT("%hour"), &str ) ) {
                strResult += wxString::Format( wxT("%d"), wxDateTime::Now().GetHour() );
            }
            // Check for minute escape
            else if ( str.StartsWith( wxT("%minute"), &str ) ) {
                strResult += wxString::Format( wxT("%d"), wxDateTime::Now().GetMinute() );
            }
            // Check for second escape
            else if ( str.StartsWith( wxT("%second"), &str ) ) {
                strResult += wxString::Format( wxT("%d"), wxDateTime::Now().GetSecond() );
            }
            // Check for week0 escape
            else if ( str.StartsWith( wxT("%week0"), &str ) ) {
                strResult += wxString::Format( wxT("%d"),
                                                wxDateTime::Now().GetWeekOfYear(),
                                                wxDateTime::Sunday_First );
            }
            // Check for week1 escape
            else if ( str.StartsWith( wxT("%week1"), &str ) ) {
                strResult += wxString::Format( wxT("%d"),
                                                wxDateTime::Now().GetWeekOfYear(),
                                                wxDateTime::Monday_First );
            }
            // Check for weekdaytxtfull escape
            else if ( str.StartsWith( wxT("%weekdaytxtfull"), &str ) ) {
                strResult += wxDateTime::GetWeekDayName(
                        wxDateTime::Now().GetWeekDay(), wxDateTime::Name_Full );
            }
            // Check for weekdaytxt escape
            else if ( str.StartsWith( wxT("%weekdaytxt"), &str ) ) {
                strResult += wxDateTime::GetWeekDayName(
                        wxDateTime::Now().GetWeekDay(), wxDateTime::Name_Abbr );
            }
            // Check for monthtxtfull escape
            else if ( str.StartsWith( wxT("%monthtxtfull"), &str ) ) {
                strResult += wxDateTime::GetMonthName(
                        wxDateTime::Now().GetMonth(), wxDateTime::Name_Full );
            }
            // Check for monthtxt escape
            else if ( str.StartsWith( wxT("%monthtxt"), &str ) ) {
                strResult += wxDateTime::GetMonthName(
                        wxDateTime::Now().GetMonth(), wxDateTime::Name_Abbr );
            }
            // Check for month escape
            else if ( str.StartsWith( wxT("%month"), &str ) ) {
                strResult += wxString::Format( wxT("%d"),
                        wxDateTime::Now().GetMonth() + 1 );
            }
            // Check for year escape
            else if ( str.StartsWith( wxT("%year"), &str ) ) {
                strResult += wxString::Format( wxT("%d"),
                        wxDateTime::Now().GetYear() );
            }
            // Check for quarter escape
            else if ( str.StartsWith( wxT("%quarter"), &str ) ) {
                if ( wxDateTime::Now().GetMonth() < 4 ) {
                    strResult += wxT("1");
                }
                else if ( wxDateTime::Now().GetMonth() < 7 ) {
                    strResult += wxT("2");
                }
                else if ( wxDateTime::Now().GetMonth() < 10 ) {
                    strResult += wxT("3");
                }
                else {
                    strResult += wxT("1");
                }
            }
            // Check for path_config escape
            else if ( str.StartsWith( wxT("%path.config"), &str ) ) {
                strResult += wxStandardPaths::Get().GetConfigDir();
            }
            // Check for path_datadir escape
            else if ( str.StartsWith( wxT("%path.datadir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetDataDir();
            }
            // Check for path_documentsdir escape
            else if ( str.StartsWith( wxT("%path.documentsdir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetDocumentsDir();
            }
            // Check for path_executable escape
            else if ( str.StartsWith( wxT("%path.executable"), &str ) ) {
                strResult += wxStandardPaths::Get().GetExecutablePath();
            }
            // Check for path_localdatadir escape
            else if ( str.StartsWith( wxT("%path.localdatadir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetLocalDataDir();
            }
            // Check for path_pluginsdir escape
            else if ( str.StartsWith( wxT("%path.pluginsdir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetPluginsDir();
            }
            // Check for path_resourcedir escape
            else if ( str.StartsWith( wxT("%path.resourcedir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetResourcesDir();
            }
            // Check for path_tempdir escape
            else if ( str.StartsWith( wxT("%path.tempdir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetTempDir();
            }
            // Check for path_userconfigdir escape
            else if ( str.StartsWith( wxT("%path.userconfigdir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetUserConfigDir();
            }
            // Check for path_userdatadir escape
            else if ( str.StartsWith( wxT("%path.userdatadir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetUserDataDir();
            }
            // Check for path_localdatadir escape
            else if ( str.StartsWith( wxT("%path.localdatadir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetUserLocalDataDir();
            }
            // Check for toliveafter
            else if ( str.StartsWith( wxT("%toliveafter1"), &str ) ) {
                strResult += wxT("Carpe diem quam minimum credula postero.");
            }
            // Check for toliveafter
            else if ( str.StartsWith( wxT("%toliveafter2"), &str ) ) {
                strResult += wxT("Be Hungry - Stay Foolish.");
            }
            // Check for toliveafter
            else if ( str.StartsWith( wxT("%toliveafter3"), &str ) ) {
                strResult += wxT("Be Foolish - Stay Hungry.");
            }
            // Check for measurement.index escape
            else if ( str.StartsWith( wxT("%measurement.index"), &str ) ) {
                uint8_t data_coding_byte = vscp_getMeasurementDataCoding( pEvent );
                if ( -1 != data_coding_byte ) {
                    strResult += wxString::Format( wxT("%d"),
                                    VSCP_DATACODING_INDEX( data_coding_byte ) );
                }
            }
            // Check for measurement.unit escape
            else if ( str.StartsWith( wxT("%measurement.unit"), &str ) ) {
                uint8_t data_coding_byte = vscp_getMeasurementDataCoding( pEvent );
                if ( -1 != data_coding_byte ) {
                    strResult += wxString::Format( wxT("%d"),
                                    VSCP_DATACODING_UNIT( data_coding_byte ) );
                }
            }
            // Check for measurement.coding escape
            else if ( str.StartsWith( wxT("%measurement.coding"), &str ) ) {
                uint8_t data_coding_byte = vscp_getMeasurementDataCoding( pEvent );
                if ( -1 != data_coding_byte ) {
                    strResult += wxString::Format( wxT("%d"),
                                    VSCP_DATACODING_TYPE( data_coding_byte ) );
                }
            }
            // Check for measurement.float escape
            else if ( str.StartsWith( wxT("%measurement.float"), &str ) ) {
                wxString str;
                vscp_getVSCPMeasurementAsString( pEvent, str );
                strResult += str;
            }
            // Check for measurement.string escape
            else if ( str.StartsWith( wxT("%measurement.string"), &str ) ) {
                wxString str;
                vscp_getVSCPMeasurementAsString( pEvent, str );
                strResult += str;
            }
            // Check for measurement.convert.data escape
            else if ( str.StartsWith( wxT("%measurement.convert.data"), &str ) ) {
                wxString str;
                if ( vscp_getVSCPMeasurementAsString(pEvent, str ) ) {
                    for ( unsigned int i=0; i<str.Length(); i++ ) {
                        if (0!=i) strResult += ','; // Not at first location
                        strResult += str.GetChar(i);
                    }
                }
            }

            // Check for eventdata.realtext escape
            else if ( str.StartsWith( wxT("%eventdata.realtext"), &str ) ) {
                strResult += vscp_getRealTextData( pEvent );
            }

            // Check for %variable:[name] (name is name of variable)
            else if ( str.StartsWith( wxT("%variable:["), &str ) ) {

                str.Trim(); // Trim of leading white space
                if ( wxNOT_FOUND != ( pos = str.First(']') ) ) {
                    
                    CVSCPVariable variable;
                    
                    wxString variableName = str.Left( pos );
                    str = str.Right( str.Length() - pos - 1 );
                    
                    // Assign value if variable exist
                    if ( gpobj->m_VSCP_Variables.find( variableName, variable ) ) {

                        wxString wxwrk;
                        variable.writeValueToString( wxwrk );
                        strResult += wxwrk;

                    }
                    
                }

            }
            
            // Check for %variable:[name] (name is name of variable)
            else if ( str.StartsWith( wxT("%vardecode:["), &str ) ) {
                
                str.Trim(); // Trim of leading white space
                if ( wxNOT_FOUND != ( pos = str.First(']') ) ) {
                    
                    CVSCPVariable variable;
                    
                    wxString variableName = str.Left( pos );
                    str = str.Right( str.Length() - pos - 1 );
                    
                    // Assign value if variable exist
                    if ( gpobj->m_VSCP_Variables.find( variableName, variable ) ) {

                        wxString wxwrk;
                        variable.writeValueToString( wxwrk );
                        if ( CVSCPVariable::isValueBase64Encoded( variable.getType() ) ) {
                            size_t len = wxBase64Decode( NULL, 0, wxwrk );
                            if ( 0 == len ) continue;
                            uint8_t *pbuf = new uint8_t[ len ];
                            if ( NULL == pbuf ) continue;
                            len = wxBase64Decode( pbuf, len, wxwrk );
                            wxwrk = wxString::FromUTF8( (const char *)pbuf, len );
                            delete [] pbuf;
                        }
                        strResult += wxwrk;

                    }
                    
                }
                
            }
            
            // Check for %file:[path] (path is path to file to include)
            else if ( str.StartsWith( wxT("%file:["), &str ) ) {
                
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

            else if (  str.StartsWith( wxT("%vscp.version.major"), &str ) ) {
                strResult += wxString::Format( _("%d"), VSCPD_MAJOR_VERSION );
            }
            else if (  str.StartsWith( wxT("%vscp.version.minor"), &str ) ) {
                strResult += wxString::Format( _("%d"), VSCPD_MINOR_VERSION );
            }
            else if (  str.StartsWith( wxT("%vscp.version.sub"), &str ) ) {
                strResult += wxString::Format( _("%d"), VSCPD_RELEASE_VERSION );
            }
            else if (  str.StartsWith( wxT("%vscp.version.build"), &str ) ) {
                strResult += wxString::Format( _("%d"), VSCPD_BUILD_VERSION );
            }
            else if (  str.StartsWith( wxT("%vscp.version.str"), &str ) ) {
                strResult += wxString::Format( _("%s"), VSCPD_DISPLAY_VERSION );
            }
            else if (  str.StartsWith( wxT("%vscp.version.wxwidgets"), &str ) ) {
                strResult += wxString::Format( _("%s"), wxVERSION_STRING );
            }
            else if (  str.StartsWith( wxT("%vscp.version.wxwidgets.str"), &str ) ) {
                strResult += wxString::Format( _("%s"), wxVERSION_STRING );
            }
            else if (  str.StartsWith( wxT("%vscp.copyright"), &str ) ) {
                strResult += wxString::Format( _("%s"), VSCPD_COPYRIGHT );
            }
            else if (  str.StartsWith( wxT("%vscp.copyright.vscp"), &str ) ) {
                strResult += wxString::Format( _("%s"), VSCPD_COPYRIGHT );
            }
            else if (  str.StartsWith( wxT("%vscp.copyright.wxwidgets"), &str ) ) {
                strResult += wxString::Format( _("%s"), " Copyright (c) 1998-2005 Julian Smart, Robert Roebling et al" );
            }
            else if (  str.StartsWith( wxT("%vscp.copyright.mongoose"), &str ) ) {
                strResult += wxString::Format( _("%s"), "Copyright (c) 2013-2015 Cesanta Software Limited" );
            }
            else if (  str.StartsWith( wxT("%vscp.os.str"), &str ) ) {
                strResult += wxGetOsDescription();
            }
            else if (  str.StartsWith( wxT("%vscp.os.width"), &str ) ) {
                if ( wxIsPlatform64Bit() ) {
                    strResult += _("64-bit ");
                }
                else {
                    strResult += _("32-bit ");
                }
            }
            else if (  str.StartsWith( wxT("%vscp.os.endian"), &str ) ) {
                if ( wxIsPlatformLittleEndian() ) {
                    strResult += _("Little endian ");
                }
                else {
                    strResult += _("Big endian ");
                }
            }
            else if (  str.StartsWith( wxT("%vscp.memory.free"), &str ) ) {
                wxMemorySize memsize;
                strResult += memsize.ToString();
            }
            else if (  str.StartsWith( wxT("%vscp.host.fullname"), &str ) ) {
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
    //handleEscapes( pEvent, wxString( wxT("This is a test %class %type [%sizedata] %data  %data[0] %data[90]") ) );

    m_triggCounter++;

    // Must be a valid event
    if ( NULL == pEvent ) return false;

    switch ( m_actionCode ) {

        case  VSCP_DAEMON_ACTION_CODE_EXECUTE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_EXECUTE.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }

            doActionExecute( pEvent );
            break;

        case  VSCP_DAEMON_ACTION_CODE_SEND_EVENT:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_SEND_EVENT.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }

            doActionSendEvent( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_SEND_EVENT_CONDITIONAL:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_SEND_EVENT_CONDITIONAL.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            
            doActionSendEventConditional( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_SEND_EVENTS_FROM_FILE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_SEND_EVENTS_FROM_FILE.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            
            doActionSendEventsFromFile( pEvent );
            break;

        case  VSCP_DAEMON_ACTION_CODE_STORE_VARIABLE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_STORE_VARIABLE.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            
            doActionStoreVariable( pEvent );
            break;

        case  VSCP_DAEMON_ACTION_CODE_ADD_VARIABLE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_ADD_VARIABLE.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg( _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }

            doActionAddVariable( pEvent );
            break;

        case  VSCP_DAEMON_ACTION_CODE_SUBTRACT_VARIABLE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_SUBTRACT_VARIABLE.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            
            doActionSubtractVariable( pEvent );
            break;

        case  VSCP_DAEMON_ACTION_CODE_MULTIPLY_VARIABLE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_MULTIPLY_VARIABLE.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            
            doActionMultiplyVariable( pEvent );
            break;

        case  VSCP_DAEMON_ACTION_CODE_DIVIDE_VARIABLE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_DIVIDE_VARIABLE.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            
            doActionDivideVariable( pEvent );
            break;
            
        case VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE_TRUE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE_TRUE.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }            
            doActionCheckVariable( pEvent, VARIABLE_CHECK_SET_TRUE );
            break;
            
        case VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE_FALSE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE_FALSE.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            doActionCheckVariable( pEvent, VARIABLE_CHECK_SET_FALSE );
            break;
            
        case VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            doActionCheckVariable( pEvent, VARIABLE_CHECK_SET_OUTCOME );
            break;
            
        case VSCP_DAEMON_ACTION_CODE_CHECK_MEASUREMENT:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_CHECK_MEASUREMENT.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            // TODO !!!!
            break;
            
        case VSCP_DAEMON_ACTION_CODE_STORE_MIN:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_STOR_MIN.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            doActionStoreMin( pEvent );
            break;
            
        case VSCP_DAEMON_ACTION_CODE_STORE_MAX:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_STOR_MAX.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            doActionStoreMax( pEvent );
            break;    

        case VSCP_DAEMON_ACTION_CODE_START_TIMER:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_START_TIMER.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            doActionStartTimer( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_PAUSE_TIMER:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_PAUSE_TIMER.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            doActionPauseTimer( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_STOP_TIMER:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_STOP_TIMER.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM);
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            doActionStopTimer( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_RESUME_TIMER:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_RESUME_TIMER.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            doActionResumeTimer( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_WRITE_FILE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_WRITE_FILE.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            doActionWriteFile( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_GET_PUT_POST_URL:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_GET_PUT_POST_URL.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            doActionGetURL( pEvent );
            break;

        case VSCP_DAEMON_ACTION_CODE_WRITE_TABLE: 
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_WRITE_TABLE.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            {                
                // Write in possible escapes
                wxString strParam = m_actionparam;
                handleEscapes( pEvent, strParam );
                
                actionThread_Table *pThread = new actionThread_Table( strParam, pEvent );
                if ( NULL == pThread ) return false;
                
                wxThreadError err;
                if (wxTHREAD_NO_ERROR == (err = pThread->Create())) {
                    pThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                    if (wxTHREAD_NO_ERROR != (err = pThread->Run())) {
                        gpobj->logMsg(_("Unable to run actionThread_Table client thread.\n"),
                                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                    }
                }
                else {
                    gpobj->logMsg(_("Unable to create actionThread_Table client thread.\n"),
                                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                }

            }
            break;
            
        case VSCP_DAEMON_ACTION_CODE_CLEAR_TABLE:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_CLEAR_TABLE.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            {
                CVSCPTable *pTable;     // Table object
                wxString wxstrErr;
                
                // Write in possible escapes
                wxString wxstr = m_actionparam;
                handleEscapes( pEvent, wxstr );
                
                wxStringTokenizer tkz( m_actionparam, wxT(";") );

                if ( !tkz.HasMoreTokens() ) {
                    // Strange action parameter
                    wxstrErr = _( "[Action] Delete Table: Action parameter is not correct. Parameter= ");
                    wxstrErr += m_actionparam;
                    wxstrErr += _("\n");
                    gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                    break;
                }
    
                // Get table name
                wxString name = tkz.GetNextToken();
    
                // Get table object
                if ( NULL == ( pTable = gpobj->m_userTableObjects.getTable( name ) ) ) {        
                    wxstrErr = _( "[Action] Delete Table: A table with that name was not found. Parameter= ");
                    wxstrErr += m_actionparam;
                    wxstrErr += _("\n");
                    gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                    return NULL;        
                }

                if ( !tkz.HasMoreTokens() ) {
            
                    wxString sql = pTable->getSQLDelete();
            
                    // Escapes
                    dmElement::handleEscapes( pEvent, sql );
                    
                    // Do the delete
                    if ( !pTable->executeSQL( sql ) ) {
                        wxstrErr = _( "[Action] Delete Table (internal SQL): Failed SQL= ");
                        wxstrErr += sql;
                        wxstrErr += _("\n");
                        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                    }
                    
                }
                else {
    
                    wxString sql;
                    wxString str = tkz.GetNextToken();
                    
                    if ( !vscp_decodeBase64IfNeeded( sql, str ) ) {
                        gpobj->logMsg( _( "[Action] Write Table: Failed to decode sql string. Will continue anyway."), 
                                    DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                        sql = str;
                    }
                        
                    // Escapes
                    dmElement::handleEscapes( pEvent, sql );
                    
                    // Do the delete
                    if ( !pTable->executeSQL( sql ) ) {
                        wxstrErr = _( "[Action] Delete Table (Action parameter SQL): Failed SQL= ");
                        wxstrErr += sql;
                        wxstrErr += _("\n");
                        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                    }

                }                
                
            }
            
            break;

        case VSCP_DAEMON_ACTION_CODE_RUN_JAVASCRIPT:
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_RUN_JAVASCRIPT.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            { 
                // If BASE64 encoded then decode 
                wxString wxstr = m_actionparam;
                wxstr.Trim( false );
                if ( wxstr.StartsWith( _("BASE64:"), &wxstr ) ) {
                    // Yes should be decoded
                    size_t len = wxBase64Decode( NULL, 0, wxstr );
                    if ( 0 == len ) {
                        gpobj->logMsg( "Failed to decode BASE64 parameter (len=0)\n", 
                                            DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                        break;
                    }
                    uint8_t *pbuf = new uint8_t[len];
                    if ( NULL == pbuf ) {
                        gpobj->logMsg( "Failed to decode BASE64 parameter  pbuf=NULL)\n", 
                                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                        break;
                    }
                    len = wxBase64Decode( pbuf, len, wxstr );
                    wxstr = wxString::FromUTF8( (const char *)pbuf, len );
                    delete [] pbuf;
                }
                
                // Write in possible escapes
                handleEscapes( pEvent, wxstr );

                actionThread_JavaScript *pThread = new actionThread_JavaScript( wxstr );
                if ( NULL == pThread ) return false;
                
                vscp_convertVSCPtoEx( &pThread->m_feedEvent, pEvent );   // Save feed event  

                wxThreadError err;
                if ( wxTHREAD_NO_ERROR == (err = pThread->Create() ) ) {
                    pThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                    if ( wxTHREAD_NO_ERROR != (err = pThread->Run() ) ) {
                        gpobj->logMsg( _("Unable to run actionThread_JavaScript client thread.\n"),
                                            DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                    }
                }
                else {
                    gpobj->logMsg( _("Unable to create actionThread_JavaScript client thread.\n"),
                                       DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                }

            }
            break;

        default:
        case VSCP_DAEMON_ACTION_CODE_NOOP:
            // We do nothing
            if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
                logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_NOOP.\n") ); // Log
                gpobj->logMsg( logStr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                gpobj->logMsg(  _("DM = ") + getAsString( false ) + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
                vscp_writeVscpEventToString( pEvent, logStr );
                gpobj->logMsg( _("Event = ") + logStr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
            }
            break;

    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionExecute
//

bool dmElement::doActionExecute(vscpEvent *pDMEvent)
{
    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes(pDMEvent, wxstr);

    // wxExecute breaks if the path does not exist so we have to
    // check that it does.
    wxString strfn = m_actionparam;
    bool bOK = true;
    int pos = m_actionparam.First(' ');
    if (wxNOT_FOUND != pos) {
        strfn = m_actionparam.Left(pos);
    }

    if (!wxFileName::FileExists(strfn) ||
            !wxFileName::IsFileExecutable(strfn)) {
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
            wxString wxstr = wxT("[Action] Executed: ");
            wxstr += m_actionparam;
            wxstr += _("\n");
            gpobj->logMsg(wxstr + _("\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM);
        }
    }
    else {
        // Failed to execute
        m_errorCounter++;
        if ( bOK ) {
            m_strLastError = _("[Action] Failed to execute :");
            gpobj->logMsg(_("[Action] Failed to execute \n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM);
        } 
        else {
            m_strLastError = _("File does not exists or is not an executable :");
            gpobj->logMsg(_("File does not exists or is not an executable \n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM);
        }
        
        m_strLastError += m_actionparam;
        m_strLastError += _("\n");
        gpobj->logMsg( wxstr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
    int idx;
    wxString varName;

    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    // There must be room in the send queue
    if ( gpobj->m_maxItemsInClientReceiveQueue >
            gpobj->m_clientOutputQueue.GetCount() ) {

            if (  wxNOT_FOUND != ( idx = m_actionparam.Find( wxT(";") ) ) ) {
                // There is a variable that we should set to true in
                // this parameter line. We extract it
                varName = m_actionparam.Mid( idx + 1 );
                m_actionparam = m_actionparam.Mid( 0, idx );
            }

            vscpEvent *pEvent = new vscpEvent;
            if ( NULL != pEvent ) {

                vscp_setVscpEventFromString( pEvent, m_actionparam );

                gpobj->m_mutexClientOutputQueue.Lock();
                gpobj->m_clientOutputQueue.Append ( pEvent );
                gpobj->m_semClientOutputQueue.Post();
                gpobj->m_mutexClientOutputQueue.Unlock();

                // TX Statistics
                m_pDM->m_pClientItem->m_statistics.cntTransmitData +=
                                                            pEvent->sizeData;
                m_pDM->m_pClientItem->m_statistics.cntTransmitFrames++;

                // Set the variable to false if it is defined
                if ( 0 != varName.Length() ) {

                    CVSCPVariable variable;
                    if ( gpobj->m_VSCP_Variables.find( varName, variable ) ) {

                        // Non existent - add and set to false
                        gpobj->m_VSCP_Variables.add( varName, wxT("true") );

                    }
                    else {

                        // Existing - set value to false
                        variable.setValueFromString( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN, wxT("true") );

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
    vscpEvent *pEvent = NULL;
    CVSCPVariable *pVar = NULL;

    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    wxStringTokenizer tkz( wxstr, wxT(";") );

    // Handle variable
    if ( tkz.HasMoreTokens() ) {

        wxString varname = tkz.GetNextToken();

        CVSCPVariable variable; 
        if ( gpobj->m_VSCP_Variables.find( varname, variable ) ) {
            // must be a variable
            wxString wxstrErr = wxT("[Action] Conditional event: No variable defined ");
            wxstrErr += wxstr;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
        }

    }
    else {
        // must be a variable
        wxString wxstrErr = wxT("[Action] Conditional event: No variable defined ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // if the variable is false we should do nothing
    if ( !pVar->isTrue() ) return false;

    // We must have an event to send
    if ( tkz.HasMoreTokens() ) {

        wxString strEvent = tkz.GetNextToken();

        if ( !vscp_setVscpEventFromString( pEvent, strEvent ) ) {
            // Could not parse evenet data
            wxString wxstrErr = wxT("[Action] Conditional event: Unable to parse event ");
            wxstrErr += wxstr;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
        }
    }
    else {
        // must be an event
        wxString wxstrErr = wxT("[Action] Conditional event: No event defined ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }


    // There must be room in the send queue
    if ( gpobj->m_maxItemsInClientReceiveQueue >
        gpobj->m_clientOutputQueue.GetCount() ) {

            gpobj->m_mutexClientOutputQueue.Lock();
            gpobj->m_clientOutputQueue.Append ( pEvent );
            gpobj->m_semClientOutputQueue.Post();
            gpobj->m_mutexClientOutputQueue.Unlock();

            // TX Statistics
            m_pDM->m_pClientItem->m_statistics.cntTransmitData += pEvent->sizeData;
            m_pDM->m_pClientItem->m_statistics.cntTransmitFrames++;

    }
    else {
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
        wxString wxstrErr = wxT("[Action] Send event from file: Non existent file  ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    wxXmlDocument doc;
    if ( !doc.Load ( wxstr ) ) {
        wxString wxstrErr = wxT("[Action] Send event from file: Failed to load event XML file  ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // start processing the XML file
    if ( doc.GetRoot()->GetName() != wxT ( "vscpevents" ) ) {
        wxString wxstrErr = wxT("[Action] Send event from file: <vscpevents> tag is missing.");
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while ( child ) {

        if ( child->GetName() == wxT ( "event" ) ) {

            // Set defaults for event
            vscpEvent *pEvent = new vscpEvent;

            if ( NULL != pEvent ) {

                pEvent->head = 0;
                pEvent->obid = 0;
                pEvent->sizeData = 0;
                pEvent->pdata = NULL;
                pEvent->vscp_class = 0;
                pEvent->vscp_type = 0;
                memset( pEvent->GUID, 0, 16 );

                wxXmlNode *subchild = child->GetChildren();
                while ( subchild ) {

                    if ( subchild->GetName() == wxT ( "head" ) ) {
                        pEvent->head = vscp_readStringValue( subchild->GetNodeContent() );
                    }
                    else if ( subchild->GetName() == wxT ( "class" ) ) {
                        pEvent->vscp_class = vscp_readStringValue( subchild->GetNodeContent() );
                    }
                    else if ( subchild->GetName() == wxT ( "type" ) ) {
                        pEvent->vscp_type = vscp_readStringValue( subchild->GetNodeContent() );
                    }
                    else if ( subchild->GetName() == wxT ( "guid" ) ) {
                        vscp_getGuidFromString( pEvent, subchild->GetNodeContent() );
                    }
                    if ( subchild->GetName() == wxT ( "data" ) ) {
                        vscp_setVscpDataFromString( pEvent, subchild->GetNodeContent() );
                    }

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
                        m_pDM->m_pClientItem->m_statistics.cntTransmitData += pEvent->sizeData;
                        m_pDM->m_pClientItem->m_statistics.cntTransmitFrames++;

                }
                else {
                    m_pDM->m_pClientItem->m_statistics.cntOverruns++;
                }

                // Remove the event
                vscp_deleteVSCPevent( pEvent );

            }

        }

    }

    return true;

}

///////////////////////////////////////////////////////////////////////////////
// doActionWriteFile
//

bool dmElement::doActionWriteFile( vscpEvent *pDMEvent )
{
    wxFile f;
    wxString path;
    bool bAppend = true;

    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    wxStringTokenizer tkz( wxstr, wxT(";") );

    // Handle path
    if ( tkz.HasMoreTokens() ) {
        wxString path = tkz.GetNextToken();
    }
    else {
        // Must have a path
        wxString wxstrErr = wxT("[Action] Write to file: No path to file given  ");
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // Handle append/overwrite flag
    if ( tkz.HasMoreTokens() ) {
        if ( !vscp_readStringValue( tkz.GetNextToken() ) ) {
            bAppend = false;
        }
    }

    if ( f.Open( path, ( bAppend ? wxFile::write_append : wxFile::write ) ) ) {

        f.Write( wxstr );
        f.Flush();
        f.Close();

    }
    else {
        // Faild to open file
        wxString wxstrErr = wxT("[Action] Write to file: Failed to open file ");
        wxstrErr += path;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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

    wxStringTokenizer tkz( wxstr, wxT(";") );
    if ( !tkz.HasMoreTokens() ) {
        // Action parameter is wrong
        wxString wxstrErr = wxT("[Action] Get URL: Wrong action parameter (method;URL required");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // Access method
    actionThread_URL::accessmethod_t nAccessMethod = actionThread_URL::GET;
    wxString access = tkz.GetNextToken();
    access.MakeUpper();
    if ( wxNOT_FOUND != access.Find(wxT("PUT") ) ) {
        nAccessMethod = actionThread_URL::PUT;
    }
    else if ( wxNOT_FOUND != access.Find(wxT("POST") ) ) {
        nAccessMethod = actionThread_URL::POST;
    }

    // Get URL
    wxString url;
    if ( tkz.HasMoreTokens() ) {
        url = tkz.GetNextToken();
    }
    else {
        // URL is required
        wxString wxstrErr = wxT("[Action] Get URL: Wrong action parameter (URL required)");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
    CVSCPVariable *pVar;
    wxString params = m_actionparam;
    handleEscapes( pDMEvent, params );
    
    wxStringTokenizer tkz( params, _(";") );

    
    CVSCPVariable var;
    
    if ( tkz.CountTokens() >= 4 ) {

        // The form is variable-name; variable-type; persistence; value
        
        if ( !( var.getVariableFromString( params ) ) ) {
            // must be a variable
            wxString wxstrErr = wxT("[Action] Store Variable: Could not set new variable ");
            wxstrErr += params;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
            
        }

        if ( !gpobj->m_VSCP_Variables.add( var ) ) {
            // must be a variable
            wxString wxstrErr = wxT("[Action] Store Variable: Could not add variable ");
            wxstrErr += params;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
        
        // Persistens
        if ( tkz.HasMoreTokens() ) {
            wxString wxstr = tkz.GetString();
            wxstr.MakeUpper();
            if ( wxNOT_FOUND != wxstr.Find(_("TRUE") ) ) {
                bPersistent = true;
            }
        }
        
        if ( !gpobj->m_VSCP_Variables.find( varName, var ) ) {
            
            // The variable was not found - it should be added
            var.setName( varName );
            var.setType( varType );
            var.setPersistent( bPersistent );
            
        }
        
        
        
        // Add/create the variable
        if ( !gpobj->m_VSCP_Variables.add( var ) ) {
            // must be a variable
            wxString wxstrErr = wxT("[Action] Store Variable: Could not add variable ");
            wxstrErr += params;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
                        wxString wxstrErr = wxT("[Action] Store Variable: Failed to convert value to string ");
                        wxstrErr += params;
                        wxstrErr += _("\n");
                        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
                        wxString wxstrErr = wxT("[Action] Store Variable: Failed to convert value to double ");
                        wxstrErr += params;
                        wxstrErr += _("\n");
                        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
                        wxString wxstrErr = wxT("[Action] Store Variable: Failed to convert value to double ");
                        wxstrErr += params;
                        wxstrErr += _("\n");
                        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
                        wxString wxstrErr = wxT("[Action] Store Variable: Failed to convert value to double ");
                        wxstrErr += params;
                        wxstrErr += _("\n");
                        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
                        wxString strValue = wxString::Format( _("%f;%d;%d;%d;%d"), 
                                                                    value,
                                                                    unit,
                                                                    sensor_index,
                                                                    zone,
                                                                    subzone );
                        var.setValue( strValue );
                    }
                    else {
                        wxString wxstrErr = wxT("[Action] Store Variable: Failed to convert value to double ");
                        wxstrErr += params;
                        wxstrErr += _("\n");
                        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
                                                        (const char *)strData.mbc_str(),
                                                        "" );
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
    long val;
    double floatval;

    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    // Get the variable name
    strName = wxstr.BeforeFirst( wxChar(';') );

    // Get the value
    wxString strval = wxstr.AfterFirst( wxChar(';') );

    if ( wxNOT_FOUND == strval.Find( wxChar('.') ) ) {
        val = vscp_readStringValue( strval );
    }
    else {
        strval.ToDouble( &floatval );
    }

    if ( gpobj->m_VSCP_Variables.find( strName, variable ) ) {

        CVSCPVariable var;
        var.setName( strName );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        var.setPersistent( false );

        if ( gpobj->m_VSCP_Variables.add( var ) ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Could not add new variable ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );

            return false;
        }

    }

    // Must be a numerical variable
    if ( !variable.isNumerical() ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Variable is not numerical ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
    }

    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG != variable.getType() ) ) {
        long lval;
        variable.getValue( &lval );
        lval += val;
        variable.setValue( lval );
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_INTEGER != variable.getType() ) ) {
        long lval;
        variable.getValue( &lval );
        lval += val;
        variable.setValue( lval );
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE != variable.getType() ) ) {
        double dval;
        variable.getValue( &dval );
        dval += floatval;
        variable.setValue( dval );
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
    long val;
    double floatval;

    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    // Get the variable name
    strName = wxstr.BeforeFirst( wxChar(';') );

    // Get the value
    wxString strval = wxstr.AfterFirst( wxChar(';') );

    if ( wxNOT_FOUND == strval.Find( wxChar('.') ) ) {
        val = vscp_readStringValue( strval );
    }
    else {
        strval.ToDouble( &floatval );
    }

    if ( gpobj->m_VSCP_Variables.find( strName, variable ) ) {

        CVSCPVariable var;

        var.setName( strName );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        var.setPersistent( false );

        if ( gpobj->m_VSCP_Variables.add( var ) ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Could not add new variable ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );

            return false;
        }

    }

    // Must be a numerical variable
    if ( !variable.isNumerical() ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Variable is not numerical ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM  );
            return false;
    }

    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG != variable.getType() ) ) {
        long lval;
        variable.getValue( &lval );
        lval -= val;
        variable.setValue( lval );
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_INTEGER != variable.getType() ) ) {
        long lval;
        variable.getValue( &lval );
        lval -= val;
        variable.setValue( lval );
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE != variable.getType() ) ) {
        double dval;
        variable.getValue( &dval );
        dval -= floatval;
        variable.setValue( dval );
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
    long val;
    double floatval;

    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    // Get the variable name
    strName = wxstr.BeforeFirst( wxChar(';') );

    // Get the value
    wxString strval = wxstr.AfterFirst( wxChar(';') );

    if ( wxNOT_FOUND == strval.Find( wxChar('.') ) ) {
        val = vscp_readStringValue( strval );
    }
    else {
        strval.ToDouble( &floatval );
    }
   
    if ( gpobj->m_VSCP_Variables.find( strName, variable ) ) {

        CVSCPVariable var;
        var.setName( strName );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        var.setPersistent( false );

        if ( gpobj->m_VSCP_Variables.add( var ) ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Could not add new variable ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );

            return false;
        }

    }

    // Must be a numerical variable
    if ( !variable.isNumerical() ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Variable is not numerical ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
    }

    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG != variable.getType() ) ) {
        long lval;
        variable.getValue( &lval );
        lval *= val;
        variable.setValue( lval );
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_INTEGER != variable.getType() ) ) {
        long lval;
        variable.getValue( &lval );
        lval *= val;
        variable.setValue( lval );
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE != variable.getType() ) ) {
        double dval;
        variable.getValue( &dval );
        dval *= floatval;
        variable.setValue( dval );
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
    long val;
    double floatval;

    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    // Get the variable name
    strName = wxstr.BeforeFirst( wxChar(';') );

    // Get the value
    wxString strval = wxstr.AfterFirst( wxChar(';') );

    if ( wxNOT_FOUND == strval.Find( wxChar('.') ) ) {
        val = vscp_readStringValue( strval );
    }
    else {
        strval.ToDouble( &floatval );
    }

    if ( gpobj->m_VSCP_Variables.find( strName, variable )) {

        CVSCPVariable var;
        var.setName( strName );
        var.setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        var.setPersistent( false );

        if ( gpobj->m_VSCP_Variables.add( var ) ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Could not add new variable ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );

            return false;
        }

    }

    // Must be a numerical variable
    if ( !variable.isNumerical() ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Variable is not numerical ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
    }

    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG != variable.getType() ) ) {
        if ( 0 != val ) {
            long lval;
            variable.getValue( &lval );
            lval /= val;
            variable.setValue( lval );
        }
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_INTEGER != variable.getType() ) ) {
        if ( 0 != val ) {
            long lval;
            variable.getValue( &lval );
            lval /= val;
            variable.setValue( lval );
        }
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE != variable.getType() ) ) {
        if ( 0.0 != floatval) {
            double dval;
            variable.getValue( &dval );
            dval *= floatval;
            variable.setValue( dval );
        }
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
    uint8_t unit = 0;
    uint8_t operation = DM_MEASUREMENT_COMPARE_NOOP;
    CVSCPVariable varCompare;   // Variable to compare
    CVSCPVariable varFlag;      // Variable with flag
    
    // Write in possible escapes
    wxString params = m_actionparam;
    handleEscapes( pDMEvent, params );

    // value;unit;operation;variable;flag-variable
    wxStringTokenizer tkz( params, wxT(";") );
    
    // Value
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( !wxstr.ToCDouble( &value ) ) {
            wxstr = _("Failed to convert to double (set to zero). param =") + params;
            gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        }
    }
    else {
        wxstr = _("Missing needed parameter (value). param =") + params;
        gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Unit
    if ( tkz.HasMoreTokens() ) {
        unit = vscp_readStringValue( tkz.GetNextToken() );
    }
    else {
        wxstr = _("Missing needed parameter (unit). param =") + params;
        gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Operation
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        operation = CDM::getCompareCodeFromToken( wxstr );
        if ( -1 == operation ) {
            wxstr = _("Invalid compare operation. param =") + params;
            gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            operation = DM_MEASUREMENT_COMPARE_NOOP;
        }
    }
    else {
        wxstr = _("Missing needed parameter (operation). param =") + params;
        gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Variable to compare
    if ( tkz.HasMoreTokens() ) {
        
        wxstr.Trim();
        if ( !gpobj->m_VSCP_Variables.find( wxstr, varCompare ) ) {
            // Variable not found
            wxstr = _("Compare variable was not found. param =") + params;
            gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
        }
        
        // The variable must be numerical
        if ( varCompare.isNumerical() ) {
            wxstr = _("Compare variable need to be numerical. param =") + params;
            gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
        }
        
    }
    else {
        wxstr = _("Missing needed parameter (variable). param =") + params;
        gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Variable with flag
    if ( tkz.HasMoreTokens() ) {
        
        wxstr.Trim();
        if ( !gpobj->m_VSCP_Variables.find( wxstr, varFlag ) ) {
            // Variable not found
            wxstr = _("Flag variable was not found. param =") + params;
            gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
        }
        
        // The variable must be boolean
        if ( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != varFlag.getType() ) {
            wxstr = _("Flag variable must be boolean. param =") + params;
            gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
        }
        
    }
    else {
        wxstr = _("Missing needed parameter (flag variable). param =") + params;
        gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    double valCompare;
    varCompare.getValue( &valCompare );
    
    switch ( operation ) {
        
        case DM_MEASUREMENT_COMPARE_NOOP:
            if ( VARIABLE_CHECK_SET_TRUE ==  type ) {
                varFlag.setValue( true );
            }
            else if ( VARIABLE_CHECK_SET_FALSE ==  type ) {
                varFlag.setValue( false );
            }
            break;
        
        case DM_MEASUREMENT_COMPARE_EQ:
            if ( value == valCompare ) {
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
                    varFlag.setValue( true );
                }
            }
            break;
            
        case DM_MEASUREMENT_COMPARE_NEQ:
            if ( value != valCompare ) {
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
                    varFlag.setValue( true );
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
                    varFlag.setValue( true );
                }
            }
            break;
            
        case DM_MEASUREMENT_COMPARE_LTEQ:
            if ( value <= valCompare ) {
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
                    varFlag.setValue( true );
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
                    varFlag.setValue( true );
                }
            }
            break;      
            
        case DM_MEASUREMENT_COMPARE_GTEQ:
            if ( value >= valCompare ) {
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
                    varFlag.setValue( true );
                }
            }
            break;      
            
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
        wxString wxstr = _("Event must be a measurement event. param =") + params;
        gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Get value from measurement event
    vscp_getVSCPMeasurementAsDouble( pDMEvent, &value );
    
    // variable;unit;index;zone;subzone
    wxStringTokenizer tkz( params, wxT(";") );
    
    if ( !tkz.HasMoreTokens() ) {
        wxString wxstr = _("Need variable name. param =") + params;
        gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Get variable name
    varname = tkz.GetNextToken();    
    varname.Trim();
    
    // Find the variable
    if ( !gpobj->m_VSCP_Variables.find( varname, variable ) ) {
        
        variable.setName( varname );
        variable.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
        variable.setOwnerID( USER_ID_ADMIN );    // Admin user owns variable
        variable.setPersistent( false );
        variable.setAccessRights( PERMISSON_ALL_RIGHTS );
        variable.setValue( DBL_MAX );
        
        if ( !gpobj->m_VSCP_Variables.add( variable ) ) {
            wxString wxstr = _("Unable to add min variable. param =") + params;
            gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
        }
    }
    
    // Variable must be numerical to be comparable
    if ( !variable.isNumerical() ) {
        wxString wxstr = _("Can't compare with non numeric variable. param =") + params;
        gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
        wxString wxstr = _("Event must be a measurement event. param =") + params;
        gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Get value from measurement event
    vscp_getVSCPMeasurementAsDouble( pDMEvent, &value );
    
    // variable;unit;index;zone;subzone
    wxStringTokenizer tkz( params, wxT(";") );
    
    if ( !tkz.HasMoreTokens() ) {
        wxString wxstr = _("Need variable name. param =") + params;
        gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    // Get variable name
    varname = tkz.GetNextToken();    
    varname.Trim();
    
    // Find the variable
    if ( !gpobj->m_VSCP_Variables.find( varname, variable ) ) {
        
        variable.setName( varname );
        variable.setType( VSCP_DAEMON_VARIABLE_CODE_DOUBLE );
        variable.setOwnerID( USER_ID_ADMIN );    // Admin user owns variable
        variable.setPersistent( false );
        variable.setAccessRights( PERMISSON_ALL_RIGHTS );
        variable.setValue( DBL_MIN );
        
        if ( !gpobj->m_VSCP_Variables.add( variable ) ) {
            wxString wxstr = _("Unable to add max variable. param =") + params;
            gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return false;
        }
    }
    
    // Variable must be numerical to be comparable
    if ( !variable.isNumerical() ) {
        wxString wxstr = _("Can't compare with non numeric variable. param =") + params;
        gpobj->logMsg( wxstr + _("\n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
        // Store new hightes value
        variable.setValue( value );
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionStartTimer
//

bool dmElement::doActionStartTimer( vscpEvent *pDMEvent )
{
    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    wxStringTokenizer tkz( wxstr, wxT(";") );
    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter
        wxString wxstrErr = wxT("[Action] Start timer: Wrong action parameter ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // Get timer id
    uint32_t id = vscp_readStringValue( tkz.GetNextToken() );

    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter
        wxString wxstrErr = wxT("[Action] Start timer: Variable name is missing ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // Get variable
    wxString strVarName = tkz.GetNextToken();

    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter
        wxString wxstrErr = wxT("[Action] Start timer: Delay is missing ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // Get delay
    uint32_t delay = vscp_readStringValue( tkz.GetNextToken() );

    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter
        wxString wxstrErr = wxT("[Action] Start timer: Setvalue is missing ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    bool bSetValue = true;
    wxString str = tkz.GetNextToken();
    str.Upper();
    if ( str.Find( wxT("FALSE") ) ) {
        bSetValue = false;
    }

    m_pDM->startTimer( id, strVarName, delay, bSetValue );

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

    wxStringTokenizer tkz( wxstr, wxT(";") );
    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter
        wxString wxstrErr = wxT("[Action] Stop timer: Wrong action parameter ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // Get timer id
    uint32_t id = vscp_readStringValue( tkz.GetNextToken() );

    m_pDM->stopTimer( id );

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

    wxStringTokenizer tkz( wxstr, wxT(";") );
    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter
        wxString wxstrErr = wxT("[Action] Stop timer: Wrong action parameter ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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

    wxStringTokenizer tkz( wxstr, wxT(";") );
    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter
        wxString wxstrErr = wxT("[Action] Stop timer: Wrong action parameter ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
            fprintf( stderr, wxstr.mbc_str() );
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
            fprintf( stderr, wxstr.mbc_str() );
            
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
                fprintf( stderr, wxstr.mbc_str() );
            }
        }
        else {
            fprintf( stderr, "VSCP DM database path invalid - will not be used.\n" );
            wxstr.Printf(_("Path=%s\n"), (const char *)m_path_db_vscp_dm.GetFullPath().mbc_str() );
            fprintf( stderr, wxstr.mbc_str() );
        }

    }
    
    // * * * VSCP Daemon internal DM database - Always created in-memory * * *
    // Internal table still used so this part will probably be removed 
/*    
    if ( SQLITE_OK == sqlite3_open( NULL, &m_dm.m_db_vscp_dm_memory ) ) {
        
        // Should always be created
        m_dm.doCreateInMemoryDMTable();
        
        // Fill internal DM with data from external
        m_dm.doFillMemoryDMTable();
        
    }
    else {
        // Failed to open/create the database file
        fprintf( stderr, "VSCP Daemon internal DM database could not be opened - Will not be used.\n" );
        str.Printf( _("Error=%s\n"),
                            sqlite3_errmsg( m_dm.m_db_vscp_dm_memory ) );
        fprintf( stderr, str.mbc_str() );
        if ( NULL != m_dm.m_db_vscp_dm_memory  ) sqlite3_close( m_dm.m_db_vscp_dm_memory  );
        m_dm.m_db_vscp_dm_memory  = NULL;
    }
*/

    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
        wxString wxlogmsg = 
        wxString::Format(_("DM engine started. DM from [%s]\n"),
                            (const char *)m_staticXMLPath.c_str() );
        gpobj->logMsg( wxlogmsg, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
    }
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
    else if ( token.StartsWith(_("lt")) ) {
        return DM_MEASUREMENT_COMPARE_LT;
    } 
    else if ( token.StartsWith(_("<")) ) {
        return DM_MEASUREMENT_COMPARE_LT;
    }
    else if ( token.StartsWith(_("gt")) ) {
        return DM_MEASUREMENT_COMPARE_GT;
    } 
    else if ( token.StartsWith(_(">")) ) {
        return DM_MEASUREMENT_COMPARE_GT;
    }
    else if ( token.StartsWith(_("gteq")) ) {
        return DM_MEASUREMENT_COMPARE_GTEQ;
    } 
    else if ( token.StartsWith(_(">=")) ) {
        return DM_MEASUREMENT_COMPARE_GTEQ;
    }
    else if ( token.StartsWith(_("lteq")) ) {
        return DM_MEASUREMENT_COMPARE_LTEQ;
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

bool CDM::removeMemoryElement( unsigned short pos )
{
    if ( pos >= m_DMList.GetCount() ) return false;

    wxDMLISTNode *node = m_DMList.Item( pos );

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
        EventSecond.sizeData = 0;                           // No data
        EventSecond.pdata = NULL;
        //memcpy( EventSecond.GUID, m_pCtrlObject->m_GUID, 16 ); // Server GUID
        gpobj->m_guid.writeGUID( EventSecond.GUID );
        wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal second event\n") );
        feed( &EventSecond );

        // Update timers
        serviceTimers();

        if ( m_rndMinute == wxDateTime::Now().GetSecond() ) {

            vscpEvent EventRandomMinute;
            EventRandomMinute.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomMinute.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_MINUTE;   // Internal Random-Minute Event
            EventRandomMinute.head = VSCP_PRIORITY_NORMAL;      // Set priority
            EventRandomMinute.sizeData = 0;                     // No data
            EventRandomMinute.timestamp = vscp_makeTimeStamp(); // Set timestamp
            EventRandomMinute.pdata = NULL;
            //memcpy( EventRandomMinute.GUID, m_pCtrlObject->m_GUID, 16 ); // Server GUID
            gpobj->m_guid.writeGUID( EventRandomMinute.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal random minute event\n")  );
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
        EventMinute.sizeData = 0;                           // No data
        EventMinute.pdata = NULL;
        //memcpy( EventMinute.GUID, m_pCtrlObject->m_GUID, 16 ); // Server GUID
        gpobj->m_guid.writeGUID( EventMinute.GUID );
        wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal minute event\n") );
        m_rndMinute = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 60;
        feed( &EventMinute );

        if ( m_rndHour == wxDateTime::Now().GetMinute() ) {

            vscpEvent EventRandomHour;
            EventRandomHour.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomHour.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_HOUR;       // Internal Random-Hour Event
            EventRandomHour.head = VSCP_PRIORITY_NORMAL;                    // Set priority
            EventRandomHour.timestamp = vscp_makeTimeStamp();               // Set timestamp
            EventRandomHour.sizeData = 0;                                   // No data
            EventRandomHour.pdata = NULL;
            //memcpy( EventRandomHour.GUID, m_pCtrlObject->m_GUID, 16 );    // Server GUID
            gpobj->m_guid.writeGUID( EventRandomHour.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal random hour event\n") );
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
        EventHour.sizeData = 0;                                             // No data
        EventHour.pdata = NULL;
        //memcpy( EventtHour.GUID, m_pCtrlObject->m_GUID, 16 );             // Server GUID
        gpobj->m_guid.writeGUID( EventHour.GUID );
        wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal hour event\n") );
        m_rndHour = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 24;
        feed( &EventHour );

        // Check if it's time to send radom day event
        if ( m_rndDay == wxDateTime::Now().GetHour() ) {

            vscpEvent EventRandomDay;
            EventRandomDay.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomDay.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_DAY;         // Internal Random-Day Event
            EventRandomDay.head = VSCP_PRIORITY_NORMAL;                     // Set priority
            EventRandomDay.timestamp = vscp_makeTimeStamp();                // Set timestamp
            EventRandomDay.sizeData = 0;                                    // No data
            EventRandomDay.pdata = NULL;
            //memcpy( EventRandomDay.GUID, m_pCtrlObject->m_GUID, 16 );     // Server GUID
            gpobj->m_guid.writeGUID( EventRandomDay.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal random day event\n") );
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
        EventDay.sizeData = 0;                              // No data
        EventDay.pdata = NULL;
        //memcpy( EventDay.GUID, m_pCtrlObject->m_GUID, 16 );   // Server GUID
        gpobj->m_guid.writeGUID( EventDay.GUID );
        wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal day event\n") );
        m_rndDay = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 7;
        feed( &EventDay );

        // Check if it's time to send random week event
        if ( m_rndWeek == wxDateTime::Now().GetDay() ) {

            vscpEvent EventRandomWeek;
            EventRandomWeek.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomWeek.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_WEEK;   // Internal Random-Week Event
            EventRandomWeek.head = VSCP_PRIORITY_NORMAL;                // Set priority
            EventRandomWeek.timestamp = vscp_makeTimeStamp();           // Set timestamp
            EventRandomWeek.sizeData = 0;                               // No data
            EventRandomWeek.pdata = NULL;
            //memcpy( EventRandomWeek.GUID, m_pCtrlObject->m_GUID, 16 );// Server GUID
            gpobj->m_guid.writeGUID( EventRandomWeek.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal random week event\n") );
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
            EventWeek.sizeData = 0;                         // No data
            EventWeek.pdata = NULL;
            //memcpy( EventWeek.GUID, m_pCtrlObject->m_GUID, 16 ); // Server GUID
            gpobj->m_guid.writeGUID( EventWeek.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal week event\n") );
            m_rndWeek = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 52;
            feed( &EventWeek );

            if ( m_rndMonth == wxDateTime::Now().GetWeekDay() ) {

                vscpEvent EventRandomMonth;
                EventRandomMonth.vscp_class = VSCP_CLASS2_VSCPD;
                EventRandomMonth.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_MONTH;     // Internal Random-Month Event
                EventRandomMonth.head = VSCP_PRIORITY_NORMAL;                   // Set priority
                EventRandomMonth.timestamp = vscp_makeTimeStamp();              // Set timestamp
                EventRandomMonth.sizeData = 0;                                  // No data
                EventRandomMonth.pdata = NULL;
                //memcpy( EventRandomMonth.GUID, m_pCtrlObject->m_GUID, 16 );   // Server GUID
                gpobj->m_guid.writeGUID( EventRandomMonth.GUID );
                wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal random month event\n") );
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
        EventMonth.sizeData = 0;                            // No data
        EventMonth.pdata = NULL;
        //memcpy( EventMonth.GUID, m_pCtrlObject->m_GUID, 16 );  // Server GUID
        gpobj->m_guid.writeGUID( EventMonth.GUID );
        wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal month event\n") );
        m_rndMonth = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 12;
        feed( &EventMonth );

        if ( m_rndYear == wxDateTime::Now().GetMonth() ) {

            vscpEvent EventRandomYear;
            EventRandomYear.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomYear.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_YEAR;   // Internal Random-Minute Event
            EventRandomYear.head = VSCP_PRIORITY_NORMAL;                // Set priority
            EventRandomYear.timestamp = vscp_makeTimeStamp();           // Set timestamp
            EventRandomYear.sizeData = 0;                               // No data
            EventRandomYear.pdata = NULL;
            //memcpy( EventRandomYear.GUID, m_pCtrlObject->m_GUID, 16 ); // Server GUID
            gpobj->m_guid.writeGUID( EventRandomYear.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal random year event\n") );
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
        EventYear.sizeData = 0;                                 // No data
        EventYear.pdata = NULL;
        //memcpy( EventYear.GUID, m_pCtrlObject->m_GUID, 16 );        // Server GUID
        gpobj->m_guid.writeGUID( EventYear.GUID );
        wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal year event\n") );
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
            EventQuarter.sizeData = 0;                          // No data
            EventQuarter.pdata = NULL;
            //memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 );     // Server GUID
            gpobj->m_guid.writeGUID( EventQuarter.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal quarter event\n") );
            feed( &EventQuarter );

    }

    if ( ( 4 == wxDateTime::Now().GetMonth() ) &&
        ( m_lastTime.GetMonth() != wxDateTime::Now().GetMonth() ) ) {

            vscpEvent EventQuarter;
            EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
            EventQuarter.vscp_type = VSCP2_TYPE_VSCPD_QUARTER;  // Internal Quarter Event
            EventQuarter.head = VSCP_PRIORITY_NORMAL;           // Set priority
            EventQuarter.timestamp = vscp_makeTimeStamp();      // Set timestamp
            EventQuarter.sizeData = 0;                          // No data
            EventQuarter.pdata = NULL;
            //memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 );   // Server GUID
            gpobj->m_guid.writeGUID( EventQuarter.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal quarter event\n") );
            feed( &EventQuarter );

    }

    if ( ( 7 == wxDateTime::Now().GetMonth() ) &&
        ( m_lastTime.GetMonth() != wxDateTime::Now().GetMonth() ) ) {

            vscpEvent EventQuarter;
            EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
            EventQuarter.vscp_type = VSCP2_TYPE_VSCPD_QUARTER;  // Internal Quarter Event
            EventQuarter.head = VSCP_PRIORITY_NORMAL;           // Set priority
            EventQuarter.timestamp = vscp_makeTimeStamp();      // Set timestamp
            EventQuarter.sizeData = 0;                          // No data
            EventQuarter.pdata = NULL;
            //memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 );     // Server GUID
            gpobj->m_guid.writeGUID( EventQuarter.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal quarter event\n") );
            feed( &EventQuarter );

    }

    if ( ( 10 == wxDateTime::Now().GetMonth() ) &&
        ( m_lastTime.GetMonth() != wxDateTime::Now().GetMonth() ) ) {

            vscpEvent EventQuarter;
            EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
            EventQuarter.vscp_type = VSCP2_TYPE_VSCPD_QUARTER;  // Internal Quarter Event
            EventQuarter.head = VSCP_PRIORITY_NORMAL;           // Set priority
            EventQuarter.timestamp = vscp_makeTimeStamp();      // Set timestamp
            EventQuarter.sizeData = 0;                          // No data
            EventQuarter.pdata = NULL;
            //memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 ); // Server GUID
            gpobj->m_guid.writeGUID( EventQuarter.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal quarter event\n") );
            feed( &EventQuarter );

    }

    m_lastTime = wxDateTime::Now();

    return true;

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

        if ( pTimer->isActive() &&
            ( gpobj->m_VSCP_Variables.find( pTimer->getVariableName(), variable ) ) ) {

                if ( !pTimer->decTimer() ) {

                    // We have reached zero

                    pTimer->setActive( false );  // We go inactive

                    // Handle the setvalue
                    if ( pTimer->getSetValue() ) {
                        variable.setTrue();
                    }
                    else {
                        variable.setFalse();
                    }

                }

        } // Active and variable exists

    }

}

///////////////////////////////////////////////////////////////////////////////
// addTimer
//

int CDM::addTimer( uint16_t id,
                    wxString& nameVar,
                    uint32_t delay,
                    bool bStart,
                    bool setValue )
{

    int rv = 0; // Default return value is failure
    dmTimer *pTimer;

    // Log
    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
        wxString logStr = wxString::Format(_("Add Timer %s.\n"),
                                            (const char *)nameVar.c_str() );
        gpobj->logMsg( logStr, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
    }

    // Check if the timer already exist - set new values
    if ( NULL != ( pTimer = m_timerHash[ id ] ) ) {
        pTimer->setVariableName( nameVar );
        pTimer->setSetValue( setValue );
        pTimer->setDelay( delay );
        pTimer->setActive( bStart );

        // Log
        wxString logStr = wxString::Format(_("Timer %d already exist.\n"), id );
        gpobj->logMsg( logStr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return id;
    }

    nameVar.Trim( true );
    nameVar.Trim( false );

    // Check if variable is defined
    if ( gpobj->m_VSCP_Variables.exist( nameVar ) ) {

        // Log
        wxString logStr = wxString::Format(_("Variable is not defined %s.\n"),
                                            (const char *)nameVar.c_str() );
        gpobj->logMsg( logStr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );

        // Create a new timer
        dmTimer *pTimer = new dmTimer( nameVar,
            delay,
            bStart,
            setValue );

        if ( NULL != pTimer ) {

            // add the timer to the hash table
            rv = id;
            m_timerHash[ id ] = pTimer;

        }

    }
    // variable is not defined - we do that
    else {

        // Log
        wxString logStr = wxString::Format(_("Variable already defined %s.\n"),
                                            (const char *)nameVar.c_str() );
        gpobj->logMsg( logStr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );

        if ( gpobj->m_VSCP_Variables.add( nameVar, wxT("false"), 
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN ) ) {

            dmTimer *pTimer = new dmTimer( nameVar,
                delay,
                bStart,
                setValue );

            if ( NULL != pTimer ) {

                // add the timer to the hash table
                rv = id;
                m_timerHash[ id ] = pTimer;

            }

        }

    }

    return rv;

}

///////////////////////////////////////////////////////////////////////////////
// startTimer
//

bool CDM::startTimer( int idTimer )
{
    dmTimer *pTimer = m_timerHash[ idTimer ];
    if ( NULL == pTimer ) return false;

    pTimer->startTimer();

    // Log
    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
        wxString logStr = wxString::Format(_("Timer %d started.\n"), idTimer );
        gpobj->logMsg( logStr, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// startTimer
//

int CDM::startTimer( uint16_t idTimer, 
                        wxString& nameVariable, 
                        uint32_t delay,
                        bool bSetValue )
{
    uint16_t rv = 0;

    dmTimer *pTimer = m_timerHash[ idTimer ];
    if ( NULL == pTimer ) {

        // Log
        if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
            wxString logStr = wxString::Format(_("Timer %d created.\n"), idTimer );
            gpobj->logMsg( logStr, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
        }

        // Nonexistent timer - create it
        addTimer( idTimer,
                nameVariable,
                delay,
                true,
                bSetValue );
    }

    // Log
    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
        wxString logStr = wxString::Format(_("Timer %d started.\n"), idTimer );
        gpobj->logMsg( logStr, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// stopTimer
//

bool CDM::stopTimer( int idTimer )
{
    dmTimer *pTimer = m_timerHash[ idTimer ];
    if ( NULL == pTimer ) return false;

    // Log
    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
        wxString logStr = wxString::Format(_("Timer %d stopped.\n"), idTimer );
        gpobj->logMsg( logStr, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
    }

    pTimer->stopTimer();

    return true;
}

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

/*
 
 // Internal table still used so this part will probably be removed
 
///////////////////////////////////////////////////////////////////////////////
// doCreateInMemoryDMTable
//
// Not used
//

bool CDM::doCreateInMemoryDMTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_DM_CREATE;
    
    // Check if database is open
    if ( NULL == m_db_vscp_dm_memory ) return false;
    
    m_mutexDM.Lock();
    
    if ( SQLITE_OK != sqlite3_exec(m_db_vscp_dm_memory, psql, NULL, NULL, &pErrMsg ) ) {
        m_mutexDM.Unlock();
        return false;
    }
    
    m_mutexDM.Unlock();
    
    return true;
}
 
// Internal table still used so this part will probably be removed

///////////////////////////////////////////////////////////////////////////////
// doFillMemoryDMTable
//
// Not used
//

bool CDM::doFillMemoryDMTable()
{  
    wxString str;
    sqlite3_stmt *ppStmt;
    char *pErrMsg;
    
    // Check if databases are open
    if ( NULL == m_db_vscp_dm ) return false;
    if ( NULL == m_db_vscp_dm_memory ) return false;
    
    m_mutexDM.Lock();
    
    // Delete all elements in memory database
    pErrMsg = 0;
    if ( SQLITE_OK != sqlite3_exec( m_db_vscp_dm_memory, "DELETE * FROM dm;", NULL, NULL, &pErrMsg ) ) {
        //sqlite3_free( insert_sql );
    }
           
    if ( SQLITE_OK != sqlite3_prepare_v2( m_db_vscp_dm,
                                            "SELECT * FROM dm;",
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        return false;
    }
    
    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        
        char *insert_sql = sqlite3_mprintf("INSERT INTO 'dm' ("
                "id,GroupID,bEnable,maskPriority,maskClass,maskType,maskGUID,"
                "filterPriority,filterClass,filterType,filterGUID,allowedFrom,allowedTo,"
                "allowedMonday,allowedTuesday,allowsWednesday,allowedThursday,allowedFriday,allowedSaturday,"
                "allowedSunday,allowedTime,bCheckIndex,index,bCheckZone,zone,"
                "bCheckSubZone,subzone,bCheckMeasurementIndex,meaurementIndex,"
                "actionCode,actionParameter,measurementValue,measurementUnit,measurementCompare"
                ") VALUES ( "
                "'%s','%s', '%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',"
                "'%s','%s', '%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',"
                "'%s','%s', '%s','%s','%s','%s','%s','%s','%s','%s'"
                ")",
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ID ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_GROUPID ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ENABLE ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_MASK_PRIORITY ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_MASK_CLASS ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_MASK_FILTER ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_MASK_GUID ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_FILTER_PRIORITY ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_FILTER_CLASS ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_FILTER_FILTER ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_FILTER_GUID ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_FROM ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_TO ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_MONDAY ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_TUESDAY ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_WEDNESDAY ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_THURSDAY ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_FRIDAY ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_SATURDAY ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_SUNDAY ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_TIME ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_CHECK_INDEX ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_INDEX ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_CHECK_ZONE ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ZONE ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_CHECK_SUBZONE ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_SUBZONE ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_CHECK_MEASUREMENT_INDEX ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_INDEX ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ACTIONCODE ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ACTIONPARAMETER ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_VALUE ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_UNIT ),
                (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_COMPARE )            
            ); 
        
        // Insert into memory database
        pErrMsg = 0;
        if ( SQLITE_OK != sqlite3_exec( m_db_vscp_dm_memory, insert_sql, NULL, NULL, &pErrMsg ) ) {
            //sqlite3_free( insert_sql );
        }

        sqlite3_free( insert_sql );

    }
    
    sqlite3_finalize( ppStmt );
    
    m_mutexDM.Unlock();
    
    return true;
}
*/

///////////////////////////////////////////////////////////////////////////////
// getDatabaseRecordCount
//
//

uint32_t CDM::getDatabaseRecordCount( void )
{
    wxString wxstr;
    char *pErrMsg;
    sqlite3_stmt *ppStmt;
    uint32_t count = 0;
    
    // Database file must be open
    if ( NULL == m_db_vscp_dm ) {
        gpobj->logMsg( _("DM: Get record count. Database file is not open.\n"),
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
        gpobj->logMsg( _("DM: Delete record. Database file is not open.\n"),
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

bool CDM::addDatabaseRecord( dmElement& dm )
{
    char *pErrMsg;
    cguid guid_mask;
    cguid guid_filter;
            
    // Database file must be open
    if ( NULL == m_db_vscp_dm ) {
        gpobj->logMsg( _("DM: Add record. Database file is not open.\n"),
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    guid_mask.getFromArray( dm.m_vscpfilter.mask_GUID );
    guid_filter.getFromArray( dm.m_vscpfilter.filter_GUID );
    
    char *sql = sqlite3_mprintf( VSCPDB_DM_INSERT,
                (const char *)dm.m_strGroupID.mbc_str(),
                dm.m_bEnable ? 1 : 0,
                dm.m_vscpfilter.mask_priority,
                dm.m_vscpfilter.mask_class,
                dm.m_vscpfilter.mask_type,
                (const char *)guid_mask.getAsString().mbc_str(),
                dm.m_vscpfilter.filter_priority,
                dm.m_vscpfilter.filter_class,
                dm.m_vscpfilter.filter_type,
                (const char *)guid_filter.getAsString().mbc_str(),
                //--------------------------------------------------------------
#if wxCHECK_VERSION( 3,0,0 )            
                (const char *)dm.m_timeAllow.m_fromTime.FormatISOCombined().mbc_str(),
                (const char *)dm.m_timeAllow.m_endTime.FormatISOCombined().mbc_str(),
#else
                (const char *)( dm.m_timeAllow.m_fromTime.FormatISODate() + _("T") + dm.m_timeAllow.m_fromTime.FormatISOTime() ).mbc_str(),
                (const char *)( dm.m_timeAllow.m_endTime.FormatISODate() + _("T") + dm.m_timeAllow.m_endTime.FormatISOTime() ).mbc_str(),
#endif            
                dm.m_timeAllow.m_weekDay[ 0 ] ? 1 : 0,
                dm.m_timeAllow.m_weekDay[ 1 ] ? 1 : 0,
                dm.m_timeAllow.m_weekDay[ 2 ] ? 1 : 0,
                dm.m_timeAllow.m_weekDay[ 3 ] ? 1 : 0,
                dm.m_timeAllow.m_weekDay[ 4 ] ? 1 : 0,
                dm.m_timeAllow.m_weekDay[ 5 ] ? 1 : 0,
                //--------------------------------------------------------------
                dm.m_timeAllow.m_weekDay[ 6 ] ? 1 : 0,
                (const char *)dm.m_timeAllow.getActionTimeAsString().mbc_str(),
                dm.m_bCheckIndex ? 1 : 0,
                dm.m_index,
                dm.m_bCheckZone,
                dm.m_zone,
                dm.m_bCheckSubZone,
                dm.m_subzone,
                dm.m_bCheckMeasurementIndex,
                //--------------------------------------------------------------
                dm.m_measurementIndex,
                dm.m_actionCode,
                (const char *)dm.m_actionparam.mbc_str(),
                dm.m_bCompareMeasurement,
                dm.m_measurementValue,
                dm.m_measurementUnit,
                dm.m_measurementCompareCode
            );

        if ( SQLITE_OK != sqlite3_exec( m_db_vscp_dm, 
                                            sql, NULL, NULL, &pErrMsg)) {
            sqlite3_free( sql );
            return false;
        }

        sqlite3_free( sql );
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// updateDatabaseRecord
//

bool CDM::updateDatabaseRecord( dmElement& dm )
{
    char *pErrMsg;
    cguid guid_mask;
    cguid guid_filter;
            
    // Database file must be open
    if ( NULL == m_db_vscp_dm ) {
        gpobj->logMsg( _("DM: Add record. Database file is not open.\n"),
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    guid_mask.getFromArray( dm.m_vscpfilter.mask_GUID );
    guid_filter.getFromArray( dm.m_vscpfilter.filter_GUID );
    
    char *sql = sqlite3_mprintf( VSCPDB_DM_UPDATE,
                (const char *)dm.m_strGroupID.mbc_str(),
                dm.m_bEnable ? 1 : 0,
                dm.m_vscpfilter.mask_priority,
                dm.m_vscpfilter.mask_class,
                dm.m_vscpfilter.mask_type,
                (const char *)guid_mask.getAsString().mbc_str(),
                dm.m_vscpfilter.filter_priority,
                dm.m_vscpfilter.filter_class,
                dm.m_vscpfilter.filter_type,
                (const char *)guid_filter.getAsString().mbc_str(),
                //--------------------------------------------------------------
#if wxCHECK_VERSION( 3,0,0 )             
                (const char *)dm.m_timeAllow.m_fromTime.FormatISOCombined().mbc_str(),
                (const char *)dm.m_timeAllow.m_endTime.FormatISOCombined().mbc_str(),
#else
                (const char *)( dm.m_timeAllow.m_fromTime.FormatISODate() + _("T") + dm.m_timeAllow.m_fromTime.FormatISOTime() ).mbc_str(),
                (const char *)( dm.m_timeAllow.m_endTime.FormatISODate() + _("T") + dm.m_timeAllow.m_endTime.FormatISOTime() ).mbc_str(),
#endif            
                dm.m_timeAllow.m_weekDay[ 0 ] ? 1 : 0,
                dm.m_timeAllow.m_weekDay[ 1 ] ? 1 : 0,
                dm.m_timeAllow.m_weekDay[ 2 ] ? 1 : 0,
                dm.m_timeAllow.m_weekDay[ 3 ] ? 1 : 0,
                dm.m_timeAllow.m_weekDay[ 4 ] ? 1 : 0,
                dm.m_timeAllow.m_weekDay[ 5 ] ? 1 : 0,
                //--------------------------------------------------------------
                dm.m_timeAllow.m_weekDay[ 6 ] ? 1 : 0,
                (const char *)dm.m_timeAllow.getActionTimeAsString().mbc_str(),
                dm.m_bCheckIndex ? 1 : 0,
                dm.m_index,
                dm.m_bCheckZone,
                dm.m_zone,
                dm.m_bCheckSubZone,
                dm.m_subzone,
                dm.m_bCheckMeasurementIndex,
                //--------------------------------------------------------------
                dm.m_measurementIndex,
                dm.m_actionCode,
                (const char *)dm.m_actionparam.mbc_str(),
                dm.m_bCompareMeasurement,
                dm.m_measurementValue,
                dm.m_measurementUnit,
                dm.m_measurementCompareCode,
                dm.m_id     // Where clause
            );

    if ( SQLITE_OK != sqlite3_exec( m_db_vscp_dm, 
                                            sql, NULL, NULL, &pErrMsg)) {
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
        gpobj->logMsg( _("DM: Update record. Database file is not open.\n"),
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    char *sql = sqlite3_mprintf( VSCPDB_DM_UPDATE_ITEM, 
                                    (const char *)strUpdateField.mbc_str(),
                                    (const char *)strUpdateValue.mbc_str(),
                                    id );
    if ( SQLITE_OK != sqlite3_exec( m_db_vscp_dm, 
                                            sql, NULL, NULL, &pErrMsg)) {
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
    char *pErrMsg;
    sqlite3_stmt *ppStmt;
    
    // Check pointer
    if ( NULL == pDMitem ) return false;
    
    // Database file must be open
    if ( NULL == m_db_vscp_dm ) {
        gpobj->logMsg( _("DM: Get record. Database file is not open.\n"), 
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    char *sql = sqlite3_mprintf( "SELECT * FROM dm WHERE id='%d';", idx );
    
    if ( SQLITE_OK != sqlite3_prepare_v2( m_db_vscp_dm,
                                            sql,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
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
            pDMitem->m_timeAllow.m_fromTime.ParseDate( wxString::FromUTF8( p ) );
        }
            
        // Allowed end
        if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_END ) ) ) {
            pDMitem->m_timeAllow.m_endTime.ParseDate( wxString::FromUTF8( p ) );
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
            
        // MeasurementIndex
        pDMitem->m_measurementIndex = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_INDEX );
            
        // ActionCode
        pDMitem->m_actionCode = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_INDEX );
            
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
    char *pErrMsg;
    sqlite3_stmt *ppStmt;
        
    // Database file must be open
    if ( NULL == m_db_vscp_dm ) {
        gpobj->logMsg( _("DM: Loading. Database file is not open.\n"), 
                        DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
        return false;
    }
    
    gpobj->logMsg( _("DM: Loading decision matrix from database :\n"), 
                        DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
    
    // debug print configuration path
    gpobj->logMsg( m_path_db_vscp_dm.GetFullPath() + _("\n"), 
                        DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );

    m_mutexDM.Lock();
    
    if ( SQLITE_OK != sqlite3_prepare_v2( m_db_vscp_dm,
                                            "SELECT * FROM dm WHERE bEnable='true';",
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        return false;
    }
    
    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        
        // Set row default values for row
        dmElement *pDMitem = new dmElement;
        if ( NULL == pDMitem ) {
            
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
            
            // Allowed from
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_START ) ) ) {
                pDMitem->m_timeAllow.m_fromTime.ParseDate( wxString::FromUTF8( p ) );
            }
            
            // Allowed to
            if ( NULL != ( p = (const char *)sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_END ) ) ) {
                pDMitem->m_timeAllow.m_endTime.ParseDate( wxString::FromUTF8( p ) );
            }
            
            // Allowed to
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
            
            // MeasurementIndex
            pDMitem->m_measurementIndex = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_INDEX );
            
            // ActionCode
            pDMitem->m_actionCode = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_INDEX );
            
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
    gpobj->logMsg( _("DM: Loading decision matrix from :\n"), 
                        DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );

    // debug print configuration path
    gpobj->logMsg( m_staticXMLPath + _("\n"), 
                        DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );

    // File must exist
    if ( !wxFile::Exists( m_staticXMLPath ) ) {
        gpobj->logMsg( _("DM: file does not exist.\n"), 
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    wxXmlDocument doc;
    if ( !doc.Load ( m_staticXMLPath ) ) {
        gpobj->logMsg( _("Faild to load DM. Check format!\n"), 
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    // start processing the XML file
    if ( doc.GetRoot()->GetName() != wxT ( "dm" ) ) {
        gpobj->logMsg( _("Failed to load DM. <dm> not found, format is wrong!\n"), 
                        DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while ( child ) {

        if ( child->GetName() == wxT ( "row" ) ) {

            // Set row default values for row
            dmElement *pDMitem = new dmElement;
            pDMitem->m_actionCode = 0;
            pDMitem->m_triggCounter = 0;
            pDMitem->m_errorCounter = 0;
            pDMitem->m_actionparam.Empty();
            pDMitem->m_comment.Empty();
            pDMitem->m_timeAllow.m_fromTime = wxDateTime::Now();
            pDMitem->m_timeAllow.m_endTime = wxDateTime::Now();
            pDMitem->m_timeAllow.setWeekDays(wxT("mtwtfss"));
            pDMitem->m_index = 0;
            pDMitem->m_zone = 0;
            pDMitem->m_subzone = 0;
            vscp_clearVSCPFilter( &pDMitem->m_vscpfilter );

            // Check if row is enabled
            wxString strEnabled = child->GetAttribute( wxT( "enable" ), wxT("false") );

            strEnabled.MakeUpper();
            if ( wxNOT_FOUND != strEnabled.Find( _("TRUE") ) ) {
                pDMitem->enableRow();
            }
            else {
                pDMitem->disableRow();
            }

            // Get group id
            pDMitem->m_strGroupID = child->GetAttribute( wxT( "groupid" ), wxT("") );

            // add the DM row to the matrix
            addMemoryElement ( pDMitem );

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                if ( subchild->GetName() == wxT ( "mask" ) ) {
                    wxString str;
                    str = subchild->GetAttribute( wxT( "priority" ), wxT("0") );
                    pDMitem->m_vscpfilter.mask_priority = vscp_readStringValue( str );

                    str = subchild->GetAttribute( wxT( "class" ), wxT("0") );
                    pDMitem->m_vscpfilter.mask_class = vscp_readStringValue( str );

                    str = subchild->GetAttribute( wxT( "type" ), wxT("0") );
                    pDMitem->m_vscpfilter.mask_type = vscp_readStringValue( str );

                    wxString strGUID = subchild->GetAttribute( wxT( "GUID" ),
                                                                wxT("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00") );
                    vscp_getGuidFromStringToArray( pDMitem->m_vscpfilter.mask_GUID, strGUID );
                }
                else if ( subchild->GetName() == wxT ( "filter" ) ) {
                    wxString str;
                    str = subchild->GetAttribute( wxT( "priority" ), wxT("0") );
                    pDMitem->m_vscpfilter.filter_priority = vscp_readStringValue( str );

                    str = subchild->GetAttribute( wxT( "class" ), wxT("0") );
                    pDMitem->m_vscpfilter.filter_class = vscp_readStringValue( str );

                    str = subchild->GetAttribute( wxT( "type" ), wxT("0") );
                    pDMitem->m_vscpfilter.filter_type = vscp_readStringValue( str );

                    wxString strGUID = subchild->GetAttribute( wxT( "GUID" ),
                            wxT("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00") );
                    vscp_getGuidFromStringToArray( pDMitem->m_vscpfilter.filter_GUID, strGUID );
                }
                else if ( subchild->GetName() == wxT ( "action" ) ) {
                    pDMitem->m_actionCode = vscp_readStringValue( subchild->GetNodeContent() );
                }
                else if ( subchild->GetName() == wxT ( "param" ) ){
                    pDMitem->m_actionparam = subchild->GetNodeContent();
                    pDMitem->m_actionparam = pDMitem->m_actionparam.Trim();
                    pDMitem->m_actionparam = pDMitem->m_actionparam.Trim(false);
                }
                else if ( subchild->GetName() == wxT ( "comment" ) ) {
                    pDMitem->m_comment = subchild->GetNodeContent();
                    pDMitem->m_comment = pDMitem->m_comment.Trim();
                    pDMitem->m_comment = pDMitem->m_comment.Trim(false);
                }
                else if ( subchild->GetName() == wxT ( "allowed_from" ) ) {
                    wxString str = subchild->GetNodeContent();
                    str.Trim();
                    if ( 0 != str.Length() ) {
                        pDMitem->m_timeAllow.m_fromTime.ParseDateTime(str);
                    }
                    else {
                        pDMitem->m_timeAllow.m_fromTime.ParseDateTime( _("1970-01-01 00:00:00") );
                    }
                }
                else if ( subchild->GetName() == wxT ( "allowed_to" ) ) {
                    wxString str = subchild->GetNodeContent();
                    str.Trim();
                    if ( 0 != str.Length() ) {
                        pDMitem->m_timeAllow.m_endTime.ParseDateTime(str);
                    }
                    else {
                        pDMitem->m_timeAllow.m_endTime.ParseDateTime( _("2199-12-31 23:59:59") );
                    }
                }
                else if ( subchild->GetName() == wxT ( "allowed_weekdays" ) ) {
                    wxString str = subchild->GetNodeContent();
                    str.Trim();
                    if ( 0 != str.Length() ) {
                        pDMitem->m_timeAllow.setWeekDays(str);
                    }
                    else {
                        pDMitem->m_timeAllow.setWeekDays( _("mtwtfss") );
                    }
                }
                else if ( subchild->GetName() == wxT ( "allowed_time" ) ) {
                    wxString str = subchild->GetNodeContent();
                    str.Trim();
                    if ( 0 != str.Length() ) {
                        pDMitem->m_timeAllow.parseActionTime(str);
                    }
                    else {
                        pDMitem->m_timeAllow.parseActionTime( _("*:*:*" ) );
                    }
                }
                else if ( subchild->GetName() == wxT ( "index" ) ) {
                    wxString str;

                    str = subchild->GetAttribute( wxT( "bCheckMeasurementIndex" ), wxT("---") );
                    if ( str == _("---") ) {
                        str = subchild->GetAttribute( wxT( "bMeasurement" ), wxT("false") );
                    }

                    str.MakeUpper();
                    if ( wxNOT_FOUND != str.Find(_("TRUE"))) {
                        pDMitem->m_bCheckMeasurementIndex = true;
                    }
                    pDMitem->m_index = vscp_readStringValue( subchild->GetNodeContent() );
                }
                else if ( subchild->GetName() == wxT ( "zone" ) ) {
                    pDMitem->m_zone = vscp_readStringValue( subchild->GetNodeContent() );
                }
                else if ( subchild->GetName() == wxT ( "subzone" ) ) {
                    pDMitem->m_subzone = vscp_readStringValue( subchild->GetNodeContent() );
                }
                else if ( subchild->GetName() == wxT ( "measurement" ) ) {
                    wxString str;
                    
                    pDMitem->m_bCompareMeasurement = false;
                    str = subchild->GetAttribute( wxT( "enable" ), wxT("false") );
                    str.MakeUpper();
                    if ( wxNOT_FOUND != str.Find(_("TRUE"))) {
                        pDMitem->m_bCompareMeasurement = true;
                    }
                    
                    str = subchild->GetAttribute( wxT( "compare" ), wxT("noop") );
                    pDMitem->m_measurementCompareCode = getCompareCodeFromToken( str );
                    
                    str = subchild->GetAttribute( wxT( "unit" ), wxT("0") );
                    pDMitem->m_measurementUnit = vscp_readStringValue( str );
                    
                    str = subchild->GetAttribute( wxT( "value" ), wxT("0") );
                    str.ToCDouble( &pDMitem->m_measurementValue );
                            
                }

                subchild = subchild->GetNext();

            }

        }

        child = child->GetNext();

    }

    if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
        gpobj->logMsg( _("DM: Read success.\n"), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// saveToXML
//
// Write decision matrix to file
//

bool CDM::saveToXML( void )
{
    wxString strLog;
    wxString buf;

    strLog = _("DM: Saving decision matrix to: ") + m_staticXMLPath + _("\n");
    gpobj->logMsg( strLog, DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_DM );

    if ( !wxFileName::IsFileWritable( m_staticXMLPath ) ) {
        strLog = _("DM: File is not writable.\n");
        gpobj->logMsg( strLog, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return false;
    }

    wxFFileOutputStream *pFileStream = new wxFFileOutputStream ( m_staticXMLPath );
    if ( NULL == pFileStream ) {
        strLog = _("DM: Failed to save: ") + m_staticXMLPath + _(" (memory allocation)\n");
        gpobj->logMsg( strLog, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
                wxString str = pDMitem->m_timeAllow.m_fromTime.FormatISODate() + _(" ") +
                                    pDMitem->m_timeAllow.m_fromTime.FormatISOTime();
                pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
            }
            pFileStream->Write( "</allowed_from>\n", strlen( "</allowed_from>\n" ) );

            pFileStream->Write ( "    <allowed_to>", strlen( "    <allowed_to>" ) );
            {
                wxString str = pDMitem->m_timeAllow.m_endTime.FormatISODate() + _(" ") +
                                    pDMitem->m_timeAllow.m_endTime.FormatISOTime();
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
            
            buf.Printf( _( "value=\"%f\" " ), pDMitem->m_measurementValue );
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
                    wxString wxstr = _("DM: Conversion to double failed for measurement. Event=") +
                                        strEvent + _("\n");         
                    gpobj->logMsg ( wxstr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                    continue;
                }
                

                switch ( pDMitem->m_measurementCompareCode ) {
                    
                    case DM_MEASUREMENT_COMPARE_NOOP:
                        // We always accept
                        break;
                        
                    case DM_MEASUREMENT_COMPARE_EQ:
                        if ( value != pDMitem->m_measurementValue ) continue;
                        break;   
                        
                    case DM_MEASUREMENT_COMPARE_NEQ:
                        if ( value == pDMitem->m_measurementValue ) continue;
                        break;
                        
                    case DM_MEASUREMENT_COMPARE_LT:
                        if ( value >= pDMitem->m_measurementValue ) continue;
                        break;
                        
                    case DM_MEASUREMENT_COMPARE_LTEQ:
                        if ( value > pDMitem->m_measurementValue ) continue;
                        break;
                        
                    case DM_MEASUREMENT_COMPARE_GT:                        
                        if ( value <= pDMitem->m_measurementValue ) continue;
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
    //m_pCtrlObject->logMsg ( _( "TCP actionThreadURL: Quit.\n" ), 
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

            wxstr = wxT("GET ");
            wxstr += m_url.GetPath();
            wxstr += wxT("?");
            wxstr += m_url.GetQuery();
            wxstr += wxT(" ");
            wxstr += wxT("HTTP/1.1\r\n");
            wxstr += wxT("Host: ");
            wxstr += m_url.GetServer();
            wxstr += wxT(":");
            wxstr += m_url.GetPort();
            wxstr += wxT("\r\n");
            wxstr += wxString::Format( wxT("User-Agent: VSCPD/%s\r\n"),
                                        VSCPD_DISPLAY_VERSION );

            // Add extra headers if there are any
            if ( m_extraheaders.Length() ) {
                wxstr += m_extraheaders;
            }
            wxstr += wxT("\r\n\r\n");

        }
        // OK the access method is POST
        else if ( actionThread_URL::POST == m_acessMethod ) {

            wxstr = wxT("POST ");
            wxstr += m_url.GetPath();
            wxstr += wxT(" ");
            wxstr += wxT("HTTP/1.1\r\n");
            wxstr += wxT("Host: ");
            wxstr += m_url.GetServer();
            wxstr += wxT(":");
            wxstr += m_url.GetPort();
            wxstr += wxT("\r\n");
            wxstr += wxString::Format( wxT("User-Agent: VSCPD/%s\r\n"),
                                        VSCPD_DISPLAY_VERSION );
            // Add extra headers if there are any
            if ( m_extraheaders.Length() ) {
                wxstr += m_extraheaders;
            }

            wxstr += wxT("Accept: */*\r\n");
            wxstr += wxT("Content-Type: application/x-www-form-urlencoded\r\n");
            wxstr += wxT("Content-Length: ");
            wxstr += wxString::Format(_("%ld"),m_putdata.Length());
            wxstr += wxT("\r\n\r\n");
            wxstr += m_putdata;
            wxstr += wxT("\r\n");

        }
        // OK the access method is PUT
        else if ( actionThread_URL::PUT == m_acessMethod ) {

            wxstr = wxT("PUT ");
            wxstr += m_url.GetPath();
            wxstr += wxT(" ");
            wxstr += wxT("HTTP/1.1\r\n");
            wxstr += wxT("Host: ");
            wxstr += m_url.GetServer();
            wxstr += wxT(":");
            wxstr += m_url.GetPort();
            wxstr += wxT("\r\n");
            wxstr += wxString::Format( wxT("User-Agent: VSCPD/%s\r\n"),
                                        VSCPD_DISPLAY_VERSION );

            // Add extra headers if there are any
            if ( m_extraheaders.Length() ) {
                wxstr += m_extraheaders;
            }

            wxstr += wxT("Content-Type: application/x-www-form-urlencoded\r\n");
            wxstr += wxT("Content-Length: ");
            wxstr += wxString::Format(_("%ld"),m_putdata.Length());
            wxstr += wxT("\r\n\r\n");
            wxstr += m_putdata;
            wxstr += wxT("\r\n");

        }
        else {

            // Invalid method
            gpobj->logMsg( _T( "actionThreadURL: Invalid http access method: " ) +
                                m_url.GetServer() +
                                wxT(",") +
                                m_url.GetPort() +
                                wxT(",") +
                                m_url.GetPath() +
                                wxT(",") +
                                wxString::Format( _("acessMethod = %d" ), m_acessMethod ) +
                                wxT(" \n"), 
                                DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM  );

        }

        if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_DM ) {
            gpobj->logMsg( _( "actionThreadURL: Request: \n" ) +
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
            gpobj->logMsg( _T( "actionThreadURL: Error writing request: " ) +
                                m_url.GetServer() +
                                wxT(",") +
                                m_url.GetPort() +
                                wxT(",") +
                                m_url.GetPath() +
                                wxT(",") +
                                wxString::Format( _("acessMethod = %d" ), m_acessMethod ) +
                                wxT(" \n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM  );
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
                gpobj->logMsg( _( "actionThreadURL: OK Response: " ) +
                                strReponse +
                                _(" \n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            }

            wxStringTokenizer tkz( strReponse );
            if ( tkz.HasMoreTokens() ) {

                wxString str = tkz.GetNextToken();
                if ( wxNOT_FOUND != str.Find( wxT("OK") ) ) {

                    // Something is wrong
                    gpobj->logMsg ( _T( "actionThreadURL: Error reading response: " ) +
                                        m_url.GetServer() +
                                        wxT(",") +
                                        m_url.GetPort() +
                                        wxT(",") +
                                        m_url.GetPath() +
                                        wxT(",") +
                                        wxString::Format( _("acessMethod = %d" ), m_acessMethod ) +
                                        wxT(", Response = ") +
                                        strReponse +
                                        wxT(" \n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM  );

                }
            }

        }
        else {
            // There was an error
            gpobj->logMsg ( _T( "actionThreadURL: Error reading response: " ) +
                                m_url.GetServer() +
                                wxT(",") +
                                m_url.GetPort() +
                                wxT(",") +
                                m_url.GetPath() +
                                wxT(",") +
                                ( m_acessMethod ? wxT("PUT") : wxT("GET") ) +
                                wxT(" \n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM  );
        }

        // Close the socket
        sock.Close();

    }
    else {
        // There was an error connecting
        gpobj->logMsg( wxT( "actionThreadURL: Unable to connect: " ) +
                                m_url.GetServer() +
                                wxT(",") +
                                m_url.GetPort() +
                                wxT(",") +
                                m_url.GetPath() +
                                wxT(",") +
                                wxString::Format( _("acessMethod = %d" ), m_acessMethod ) +
                                wxT(" \n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM  );
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
    vscp_setVscpEventExFromString( &m_eventThe, strEvent );
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
    //m_pCtrlObject->logMsg ( _T ( "TCP actionThreadURL: Quit.\n" ), DAEMON_LOGMSG_INFO );
    VscpRemoteTcpIf client;

    if ( CANAL_ERROR_SUCCESS != client.doCmdOpen( m_strHostname,
                                                    m_strUsername,
                                                    m_strPassword ) ) {
        // Failed to connect
        gpobj->logMsg( wxT( "actionThreadVSCPSrv: Unable to connect to remote server : " ) +
                                    m_strHostname +
                                    wxT(" \n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM  );
    }

    // Connected
    if ( CANAL_ERROR_SUCCESS != client.doCmdSendEx( &m_eventThe ) ) {
        // Failed to send event
        gpobj->logMsg( wxT( "actionThreadVSCPSrv: Unable to send event to remote server : " ) +
                                m_strHostname +
                                wxT(" \n"), DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM  );
    }

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void actionThread_VSCPSrv::OnExit()
{

}




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
    
    wxStringTokenizer tkz( m_strParam, wxT(";") );

    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter
        wxstrErr = _( "[Action] Write Table: Action parameter is not correct. Parameter= ");
        wxstrErr += m_strParam;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return NULL;
    }
    
    // Get action parameter format
    type = vscp_readStringValue( tkz.GetNextToken() );
    
    if ( !tkz.HasMoreTokens() ) {
        // Missing table name
        wxstrErr = _( "[Action] Write Table: Action parameter is not correct. No tablename. Parameter= ");
        wxstrErr += m_strParam;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return NULL;
    }
    
    // Get table name
    name = tkz.GetNextToken();
    
    // Get table object
    if ( NULL == ( pTable = gpobj->m_userTableObjects.getTable( name ) ) ) {        
        wxstrErr = _( "[Action] Write Table: A table with that name was not found. Parameter= ");
        wxstrErr += m_strParam;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
            gpobj->logMsg( rowData, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        }
        
        pTable->finalizeRangeOfData( ppStmt );
    }
    
    wxPrintf( _(">>>>>") + wxDateTime::Now().FormatISOTime() + _("\n") );
#endif    
    
    
    
    if ( 0 == type ) {
        
        if ( !tkz.HasMoreTokens() ) {
            wxstrErr = _( "[Action] Write Table: Action parameter is not correct. Datetime is missing. Parameter= ");
            wxstrErr += m_strParam;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return NULL;
        }
        
        // Get date/time
        dt.ParseDateTime( tkz.GetNextToken() );
        
        if ( !tkz.HasMoreTokens() ) {
            wxstrErr = _( "[Action] Write Table: Action parameter is not correct. value is missing. Parameter= ");
            wxstrErr += m_strParam;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return NULL;
        }
        
        // Get value
        if ( !tkz.GetNextToken().ToCDouble( &value ) ) {
            wxstrErr = _( "[Action] Write Table: Could not get value (invalid?). Parameter= ");
            wxstrErr += m_strParam;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
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
                gpobj->logMsg( _( "[Action] Write Table: Failed to log data (datetime,value)"), 
                                    DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            }
        
        }
        // format: datetime;value;sql
        else {
            
            // Get custom sql expression
            sql = tkz.GetNextToken();
                        
            wxString strResult;
            if ( !vscp_decodeBase64IfNeeded( sql, strResult ) ) {
                gpobj->logMsg( _( "[Action] Write Table: Failed to decode SQL string. Will continue anyway."), 
                                    DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
                strResult = sql;
            }
            
            // Escapes
            dmElement::handleEscapes( m_pFeedEvent, strResult );
                    
            // Log the data
            dt.SetMillisecond( m_pFeedEvent->timestamp / 1000 );
            if ( !pTable->logData( dt, value, strResult ) ) {
                gpobj->logMsg( _( "[Action] Write Table: Failed to log data (datetime,value,sql)"), 
                                    DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            }
            
        }
        
    }
    // Format: [BASE64:]sql-expression
    else if ( 1 == type ) {
        
        if ( !tkz.HasMoreTokens() ) {
            wxstrErr = _( "[Action] Write Table: Action parameter is not correct. SQL expression is missing. Parameter= ");
            wxstrErr += m_strParam;
            wxstrErr += _("\n");
            gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            return NULL;
        }
        
        // Get custom sql expression
        sql = tkz.GetNextToken();
            
        wxString strResult;
        if ( !vscp_decodeBase64IfNeeded( sql, strResult ) ) {
            gpobj->logMsg( _( "[Action] Write Table: Failed to decode sql string. Will continue anyway."), 
                                    DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
            strResult = sql;
        }
        
        // Log the data
        if ( !pTable->logData( strResult ) ) {
            gpobj->logMsg( _( "[Action] Write Table: Failed to log data (sql)"), 
                                    DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        }
        
    }
    else {
        wxstrErr = _( "[Action] Write Table: Invalid type. Parameter= ");
        wxstrErr += m_strParam;
        wxstrErr += _("\n");
        gpobj->logMsg( wxstrErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_DM );
        return NULL;
    }
    
}

///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void actionThread_Table::OnExit()
{

}


