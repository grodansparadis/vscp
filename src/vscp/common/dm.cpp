// dm.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2015 
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
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/tokenzr.h>
#include <wx/stdpaths.h>
#include "wx/thread.h"
#include "wx/socket.h"
#include <wx/url.h>
#include "wx/datetime.h"
#include <wx/filename.h>

#include <stdlib.h>
#include <string.h>

#include <vscp.h>
#include <vscphelper.h>
#include <vscpeventhelper.h>
#include <actioncodes.h>
#include <vscpvariable.h>
#include <controlobject.h>
#include <vscpremotetcpif.h>
#include <dm.h>


WX_DEFINE_LIST( PLUGINLIST );
WX_DEFINE_LIST( DMLIST );
WX_DEFINE_LIST( ACTIONTIME );


///////////////////////////////////////////////////////////////////////////////
// actionThreadURL
//
// This thread connects to a HTTP server on a specified port
//

actionThreadURL::actionThreadURL( CControlObject *pCtrlObject, 
                                    wxString& url,
                                    uint8_t nAccessMethod,
                                    wxString& putdata,
                                    wxString& proxy,
                                    wxThreadKind kind )
                                    : wxThread( kind )
{
    //OutputDebugString( "actionThreadURL: Create");
    m_pCtrlObject = pCtrlObject;

    // Set URL
    m_url.SetURL( url );

    m_acessMethod = nAccessMethod;
    m_putdata = putdata;

    m_url.SetProxy( proxy );

}

actionThreadURL::~actionThreadURL()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//
// http://www.jmarshall.com/easy/http/

void *actionThreadURL::Entry()
{
    //m_pCtrlObject->logMsg ( _T ( "TCP actionThreadURL: Quit.\n" ), DAEMON_LOGMSG_INFO );

    wxIPV4address addr;
    wxSocketClient sock;

    addr.Hostname( m_url.GetServer() );
    addr.Service( m_url.GetPort() );

    if ( sock.Connect( addr ) ) {

        wxString wxstr;

        // Check if access method is GET
        if ( 0 == m_acessMethod ) {
            wxstr = wxT("GET ");
            wxstr += m_url.GetPath();
            wxstr += wxT("");
            wxstr += wxT("HTTP/1.1\n");
            wxstr += wxT("Host: ");
            wxstr += m_url.GetServer();
            wxstr += wxT(":");
            wxstr += m_url.GetPort();
            wxstr += wxT("\n");
            wxstr += wxT("User-Agent: VSCPD/1.0\n\n");
        }
        // OK the access method is PUT
        else {
            wxstr = wxT("POST ");
            wxstr += m_url.GetPath();
            wxstr += wxT("");
            wxstr += wxT("HTTP/1.1\n");
            wxstr += wxT("Host: ");
            wxstr += m_url.GetServer();
            wxstr += wxT(":");
            wxstr += m_url.GetPort();
            wxstr += wxT("\n");
            wxstr += wxT("User-Agent: VSCPD/1.0\n");
            wxstr += wxT("Content-Type: application/x-www-form-urlencoded\n");
            wxstr += wxT("Content-Length: ");
            wxstr += wxString::Format(_("%d"),m_putdata.Length());
            wxstr += wxT("\n\n");
            wxstr += m_putdata;
            wxstr += wxT("\n");
        }

        // Send the request 
        sock.Write( wxstr, wxstr.Length() );
        if ( sock.Error() ) {
            // There was an error
            m_pCtrlObject->logMsg( _T ( "actionThreadURL: Error writing request: " ) +
                m_url.GetServer() + 
                wxT(",") + 
                m_url.GetPort() +
                wxT(",") + 
                m_url.GetPath() +
                wxT(",") +
                ( m_acessMethod ? wxT("GET") : wxT("PUT") ) +
                wxT(" \n"), 
                DAEMON_LOGMSG_ERROR );
        }

        // Get the response
        char buffer[ 2048 ];
        wxString strReponse;
        sock.Read( buffer, sizeof( buffer ) );
        if ( !sock.Error() ) {

            // Check the response
            strReponse.FromAscii( buffer );
            wxStringTokenizer tkz( strReponse );
            if ( tkz.HasMoreTokens() ) {
                wxString str = tkz.GetNextToken();
                if ( wxNOT_FOUND != str.Find( wxT("OK") ) ) {
                    // Something is wrong
                    m_pCtrlObject->logMsg ( _T ( "actionThreadURL: Error reading respons: " ) +
                        m_url.GetServer() + 
                        wxT(",") + 
                        m_url.GetPort() +
                        wxT(",") + 
                        m_url.GetPath() +
                        wxT(",") +
                        ( m_acessMethod ? wxT("GET") : wxT("PUT") ) +
                        wxT(", Response = ") +
                        strReponse +
                        wxT(" \n"), 
                        DAEMON_LOGMSG_ERROR );
                }
            }

        }
        else {
            // There was an error
            m_pCtrlObject->logMsg ( _T ( "actionThreadURL: Error reading respons: " ) +
                m_url.GetServer() + 
                wxT(",") + 
                m_url.GetPort() +
                wxT(",") + 
                m_url.GetPath() +
                wxT(",") +
                ( m_acessMethod ? wxT("GET") : wxT("PUT") ) +
                wxT(" \n"), 
                DAEMON_LOGMSG_ERROR );
        }

        // Close the socket
        sock.Close();

    }
    else {
        // There was an error connecting
        m_pCtrlObject->logMsg( wxT( "actionThreadURL: Unable to connect: " ) +
                                m_url.GetServer() + 
                                wxT(",") + 
                                m_url.GetPort() +
                                wxT(",") + 
                                m_url.GetPath() +
                                wxT(",") +
                                ( m_acessMethod ? wxT("GET") : wxT("PUT") ) +
                                wxT(" \n"), 
                                DAEMON_LOGMSG_ERROR );
    }

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void actionThreadURL::OnExit()
{

}




///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// actionThreadVSCPSrv
//
// This thread connects to a ODBC database and execute a SQL expression.
//

actionThreadVSCPSrv::actionThreadVSCPSrv( CControlObject *pCtrlObject, 
    wxString& strHostname, 
    short port, 
    wxString& strUsername, 
    wxString& strPassword,
    wxString& strEvent,
    wxThreadKind kind )
    : wxThread( kind )
{
    //OutputDebugString( "actionThreadURL: Create");
    m_pCtrlObject = pCtrlObject;
    m_strHostname = strHostname;
    m_port = port;
    m_strUsername = strUsername;
    m_strPassword = strPassword;
    vscp_setVscpEventExFromString( &m_eventThe, strEvent );
}

actionThreadVSCPSrv::~actionThreadVSCPSrv()
{

}


///////////////////////////////////////////////////////////////////////////////
// Entry
//
// 

void *actionThreadVSCPSrv::Entry()
{
    //m_pCtrlObject->logMsg ( _T ( "TCP actionThreadURL: Quit.\n" ), DAEMON_LOGMSG_INFO );
    VscpRemoteTcpIf client;

    if ( CANAL_ERROR_SUCCESS != client.doCmdOpen( m_strHostname,  
        m_strUsername, 
        m_strPassword ) ) {
            // Failed to connect
            m_pCtrlObject->logMsg( wxT( "actionThreadVSCPSrv: Unable to connect to remote server : " ) +
                m_strHostname +
                wxT(" \n"), 
                DAEMON_LOGMSG_ERROR );
    }

    // Connected
    if ( CANAL_ERROR_SUCCESS != client.doCmdSendEx( &m_eventThe ) ) {
        // Failed to send event
        m_pCtrlObject->logMsg( wxT( "actionThreadVSCPSrv: Unable to send event to remote server : " ) +
            m_strHostname +
            wxT(" \n"), 
            DAEMON_LOGMSG_ERROR );
    }

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void actionThreadVSCPSrv::OnExit()
{

}




///////////////////////////////////////////////////////////////////////////////







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
        m_weekDay[ i ] = true;	// Allow for all weekdays	
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
    //wxString s1 = m_fromTime.FormatISODate() + wxT(" ") + m_fromTime.FormatISOTime();
    //wxString s2 = m_endTime.FormatISODate() + wxT(" ") + m_endTime.FormatISOTime();
    //wxString s3 = now.FormatISODate() + wxT(" ") + now.FormatISOTime();

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
        for (iter = m_actionYear.begin(); iter != m_actionYear.end(); ++iter) {
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
        for (iter = m_actionMonth.begin(); iter != m_actionMonth.end(); ++iter) {
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
        for (iter = m_actionDay.begin(); iter != m_actionDay.end(); ++iter) {
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
        for (iter = m_actionHour.begin(); iter != m_actionHour.end(); ++iter) {
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
        for (iter = m_actionMinute.begin(); iter != m_actionMinute.end(); ++iter) {
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
        for (iter = m_actionSecond.begin(); iter != m_actionSecond.end(); ++iter) {
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
    vscp_clearVSCPFilter( &m_vscpfilter );
    m_control = 0;
    m_action = 0;
    m_triggCounter = 0;
    m_errorCounter = 0;
    m_actionparam.Empty();
    m_comment.Empty();
    
    m_index = 0;
    m_bMeasurement = false;
    m_zone = 0;
    m_subzone = 0;

    m_pDM = NULL;	// Initially no owner

}

// Destructor
dmElement::~dmElement()
{

}

///////////////////////////////////////////////////////////////////////////////
// getAsRealText
//


wxString dmElement::getAsRealText( bool bCRLF )
{
    wxString strRow;

    if ( isEnabled() ) {
        strRow = _("enabled,");
    }
    else {
        strRow = _("disabled,");
    }

    // From time
    strRow += m_timeAllow.m_fromTime.FormatISODate() + _(" ") + m_timeAllow.m_fromTime.FormatISOTime() + _(",");

    // End time
    strRow += m_timeAllow.m_endTime.FormatISODate() + _(" ") + m_timeAllow.m_endTime.FormatISOTime() + _(",");

    // Allowed weekdays
    strRow += m_timeAllow.getWeekDays() + _(",");

    // Action time
    strRow += m_timeAllow.getActionTimeAsString() + _(",");

    // Priority mask
    strRow += wxString::Format(_("0x%X,"), m_vscpfilter.mask_priority );

    // Class mask
    strRow += wxString::Format(_("0x%X,"), m_vscpfilter.mask_class );

    // Type mask
    strRow += wxString::Format(_("0x%X,"), m_vscpfilter.mask_type );

    // GUID mask
    wxString strGUID;
    vscp_writeGuidArrayToString( m_vscpfilter.mask_GUID, strGUID );
    strRow += strGUID;
    strRow += _(",");


    // Priority filter
    strRow += wxString::Format(_("0x%X,"), m_vscpfilter.filter_priority );

    // Class filter
    strRow += wxString::Format(_("0x%X,"), m_vscpfilter.filter_class );

    // Type filter
    strRow += wxString::Format(_("0x%X,"), m_vscpfilter.filter_type );

    // GUID filter
    vscp_writeGuidArrayToString( m_vscpfilter.filter_GUID, strGUID );
    strRow += strGUID;
    strRow += _(",");

    // Control Code
    strRow += wxString::Format(_("0x%X,"), m_control );

    // Action Code
    strRow += wxString::Format(_("0x%X,"), m_action );

    // Action Parameters
    strRow += m_actionparam;
    strRow += _(",");

    // trig-counter:
    strRow += wxString::Format(_("0x%X,"), m_triggCounter );

    // error-counter:
    strRow += wxString::Format(_("0x%X,"), m_errorCounter );

    strRow += m_comment;

    // New line
    if ( bCRLF ) {
        strRow += _("\r\n");
    }

    return strRow;
}

///////////////////////////////////////////////////////////////////////////////
// handleEscapes
//

bool dmElement::handleEscapes( vscpEvent *pEvent, wxString& str )
{
    int pos;
    wxString strResult;
    //wxStandardPaths stdPath;

    while ( wxNOT_FOUND != ( pos = str.Find( wxT("%") ) ) ) {

        strResult += str.Left( pos );
        str = str.Right( str.Length() - pos );

        if ( str.Length() > 1 ) {

            // Check for percent i.e. %%
            if ( str.StartsWith( wxT("%%"), &str ) ) {
                strResult += wxT("%");					// add percent
            }
            // Check for ';' i.e. %;
            else if ( str.StartsWith( wxT("%;"), &str ) ) {
                strResult += wxT(";");					// add semicolon
            }
            // Check for %cr
            else if ( str.StartsWith( wxT("%cr"), &str ) ) {
                strResult += wxT("\r");					// add carrige return
            }
            // Check for %lf
            else if ( str.StartsWith( wxT("%cr"), &str ) ) {
                strResult += wxT("\n");					// add new line
            }
            // Check for %crlf
            else if ( str.StartsWith( wxT("%crlf"), &str ) ) {
                strResult += wxT("\r\n");				// add carrige return line feed
            }
            // Check for %tab
            else if ( str.StartsWith( wxT("%tab"), &str ) ) {
                strResult += wxT("\t");					// add tab
            }
            // Check for %bell
            else if ( str.StartsWith( wxT("%bell"), &str ) ) {
                strResult += wxT("\a");					// add bell
            }
            // Check for head escape
            else if ( str.StartsWith( wxT("%event.head"), &str ) ) {
                strResult +=  wxString::Format( _("%d"), pEvent->head );
            }
            // Check for priority escape
            else if ( str.StartsWith( wxT("%event.priority"), &str ) ) {
                strResult +=  wxString::Format( wxT("%d"), ( ( pEvent->head & VSCP_HEADER_PRIORITY_MASK ) >> 5 ) );	
            }
            // Check for hardcoded escape
            else if ( str.StartsWith( wxT("%event.hardcoded"), &str ) ) {
                if ( pEvent->head & VSCP_HEADER_HARD_CODED ) {
                    strResult += wxT("1");	
                }
                else {
                    strResult += wxT("0");
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
                        strResult +=  wxT("?");		// invalid index	
                    }
                    

                }
                else {
                    // Just remove ending ]
                    if ( wxNOT_FOUND != ( pos = str.Find( wxT("]") ) ) ) {
                        str = str.Right( str.Length() - pos - 1 );		
                    }
                    strResult +=  wxT("");		// No data	
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
                    strResult +=  wxT("empty");		// No data
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
                strResult += wxString::Format( wxT( "%d" ), pEvent->GUID[ VSCP_GUID_LSB ] );
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
                    strResult += wxT( "empty" );		// No data
                }
            }
            // Check for event zone escape
            else if ( str.StartsWith( wxT( "%event.zone" ), &str ) ) {
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    strResult += wxString::Format( wxT( "%d" ), pEvent->pdata[ 1 ] );
                }
                else {
                    strResult += wxT( "empty" );		// No data
                }
            }
            // Check for event subzone escape
            else if ( str.StartsWith( wxT( "%event.subzone" ), &str ) ) {
                if ( pEvent->sizeData && ( NULL != pEvent->pdata ) ) {
                    strResult += wxString::Format( wxT( "%d" ), pEvent->pdata[ 2 ] );
                }
                else {
                    strResult += wxT( "empty" );		// No data
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
            // Check for mstime escape
            else if ( str.StartsWith( wxT("%mstime"), &str ) ) {
                strResult += wxString::Format( wxT("%d"), wxDateTime::Now().GetTicks() );
            }
			// Check for unixtime escape
            else if ( str.StartsWith( wxT("%unixtime"), &str ) ) {
				time_t now = time(NULL);	// Get current time
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
                strResult += wxString::Format( wxT("%d"), wxDateTime::Now().GetWeekOfYear(), wxDateTime::Sunday_First );
            }
            // Check for week1 escape
            else if ( str.StartsWith( wxT("%week1"), &str ) ) {
                strResult += wxString::Format( wxT("%d"), wxDateTime::Now().GetWeekOfYear(), wxDateTime::Monday_First );
            }
            // Check for weekdaytxtfull escape
            else if ( str.StartsWith( wxT("%weekdaytxtfull"), &str ) ) {
                strResult += wxDateTime::GetWeekDayName( wxDateTime::Now().GetWeekDay(), wxDateTime::Name_Full );
            }
            // Check for weekdaytxt escape
            else if ( str.StartsWith( wxT("%weekdaytxt"), &str ) ) {
                strResult += wxDateTime::GetWeekDayName( wxDateTime::Now().GetWeekDay(), wxDateTime::Name_Abbr );	
            }
            // Check for monthtxtfull escape
            else if ( str.StartsWith( wxT("%monthtxtfull"), &str ) ) {
                strResult += wxDateTime::GetMonthName( wxDateTime::Now().GetMonth(), wxDateTime::Name_Full );
            }
            // Check for monthtxt escape
            else if ( str.StartsWith( wxT("%monthtxt"), &str ) ) {
                strResult += wxDateTime::GetMonthName( wxDateTime::Now().GetMonth(), wxDateTime::Name_Abbr );
            }
            // Check for month escape
            else if ( str.StartsWith( wxT("%month"), &str ) ) {
                strResult += wxString::Format( wxT("%d"), wxDateTime::Now().GetMonth() + 1 );
            }
            // Check for year escape
            else if ( str.StartsWith( wxT("%year"), &str ) ) {
                strResult += wxString::Format( wxT("%d"), wxDateTime::Now().GetYear() );
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
            else if ( str.StartsWith( wxT("%path_config"), &str ) ) {
                strResult += wxStandardPaths::Get().GetConfigDir();
            }
            // Check for path_datadir escape
            else if ( str.StartsWith( wxT("%path_datadir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetDataDir();
            }
            // Check for path_documentsdir escape
            else if ( str.StartsWith( wxT("%path_documentsdir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetDocumentsDir();
            }
            // Check for path_executable escape
            else if ( str.StartsWith( wxT("%path_executable"), &str ) ) {
                strResult += wxStandardPaths::Get().GetExecutablePath();
            }
            // Check for path_localdatadir escape
            else if ( str.StartsWith( wxT("%path_localdatadir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetLocalDataDir();
            }
            // Check for path_pluginsdir escape
            else if ( str.StartsWith( wxT("%path_pluginsdir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetPluginsDir();
            }
            // Check for path_resourcedir escape
            else if ( str.StartsWith( wxT("%path_resourcedir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetResourcesDir();
            }
            // Check for path_tempdir escape
            else if ( str.StartsWith( wxT("%path_tempdir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetTempDir();
            }
            // Check for path_userconfigdir escape
            else if ( str.StartsWith( wxT("%path_userconfigdir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetUserConfigDir();
            }
            // Check for path_userdatadir escape
            else if ( str.StartsWith( wxT("%path_userdatadir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetUserDataDir();
            }
            // Check for path_localdatadir escape
            else if ( str.StartsWith( wxT("%path_localdatadir"), &str ) ) {
                strResult += wxStandardPaths::Get().GetUserLocalDataDir();
            }
            // Check for toliveafter
            else if ( str.StartsWith( wxT("%toliveafter"), &str ) ) {
                strResult += wxT("Carpe diem quam minimum credula postero.");
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

    // For test of escapes
    //handleEscapes( pEvent, wxString( wxT("This is a test %class %type [%sizedata] %data  %data[0] %data[90]") ) );

    m_triggCounter++;

    // Must be a valid event
    if ( NULL == pEvent ) return false;

    switch ( m_action ) {

    case VSCP_DAEMON_ACTION_CODE_NOOP:
        // We do nothing
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_NOOP.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );
        break;

    case  VSCP_DAEMON_ACTION_CODE_EXECUTE:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_EXECUTE.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionExecute( pEvent );
        break;

    case  VSCP_DAEMON_ACTION_CODE_SEND_EVENT:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_SEND_EVENT.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionSendEvent( pEvent );
        break;

    case VSCP_DAEMON_ACTION_CODE_SEND_EVENT_CONDITIONAL:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_SEND_EVENT_CONDITIONAL.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionSendEventConditional( pEvent );
        break;

    case VSCP_DAEMON_ACTION_CODE_SEND_EVENTS_FROM_FILE:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_SEND_EVENTS_FROM_FILE.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionSendEventsFromFile( pEvent );
        break;		

    case  VSCP_DAEMON_ACTION_CODE_STORE_VARIABLE:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_STORE_VARIABLE.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionStoreVariable( pEvent );
        break;

    case  VSCP_DAEMON_ACTION_CODE_ADD_VARIABLE:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_ADD_VARIABLE.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg( _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionAddVariable( pEvent );
        break;

    case  VSCP_DAEMON_ACTION_CODE_SUBTRACT_VARIABLE:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_SUBTRACT_VARIABLE.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionSubtractVariable( pEvent );
        break;

    case  VSCP_DAEMON_ACTION_CODE_MULTIPLY_VARIABLE:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_MULTIPLY_VARIABLE.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionMultiplyVariable( pEvent );
        break;

    case  VSCP_DAEMON_ACTION_CODE_DIVIDE_VARIABLE:	
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_DIVIDE_VARIABLE.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionDivideVariable( pEvent );
        break;

    case VSCP_DAEMON_ACTION_CODE_START_TIMER:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_START_TIMER.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionStartTimer( pEvent );
        break;

    case VSCP_DAEMON_ACTION_CODE_PAUSE_TIMER:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_PAUSE_TIMER.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionPauseTimer( pEvent );
        break;

    case VSCP_DAEMON_ACTION_CODE_STOP_TIMER:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_STOP_TIMER.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionStopTimer( pEvent );
        break;

    case VSCP_DAEMON_ACTION_CODE_RESUME_TIMER:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_RESUME_TIMER.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionResumeTimer( pEvent );
        break;

    case VSCP_DAEMON_ACTION_CODE_WRITE_FILE:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_WRITE_FILE.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionWriteFile( pEvent );
        break;

    case VSCP_DAEMON_ACTION_CODE_GET_PUT_POST_URL:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_GET_PUT_POST_URL.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionGetURL( pEvent );
        break;

	case VSCP_DAEMON_ACTION_CODE_WRITE_TABLE:
        
        logStr = wxString::Format(_("VSCP_DAEMON_ACTION_CODE_WRITE_TABLE.") ); // Log
        m_pDM->logMsg( logStr, LOG_DM_NORMAL );
        m_pDM->logMsg(  _("DM = ") + getAsRealText( false ), LOG_DM_EXTRA );
        vscp_writeVscpEventToString( pEvent, logStr );
        m_pDM->logMsg( _("Event = ") + logStr, LOG_DM_EXTRA );

        doActionWriteTable( pEvent );
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
    if ( bOK && ( ::wxExecute(wxstr, wxEXEC_ASYNC  ) ) ) {
#endif
        wxString wxstr = wxT("[Action] Executed: ");
        wxstr += m_actionparam;
        wxstr += _("\n");
        m_pDM->logMsg(wxstr, LOG_DM_NORMAL);
    } 
    else {
        // Failed to execute
        m_errorCounter++;
        if (bOK) {
            m_strLastError = _("[Action] Failed to execute :");
            m_pDM->logMsg(_("[Action] Failed to execute \n"), LOG_DM_NORMAL);
        } else {
            m_strLastError = _("File does not exists or is not an executable :");
            m_pDM->logMsg(_("File does not exists or is not an executable \n"), LOG_DM_NORMAL);
        }
        m_strLastError += m_actionparam;
        m_strLastError += _("\n");
        m_pDM->logMsg(wxstr, DAEMON_LOGMSG_ERROR);
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// doActionSendEvent
//

bool dmElement::doActionSendEvent( vscpEvent *pDMEvent )
{
    int idx;
    wxString var;

    // Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

    // There must be room in the send queue
    if ( m_pDM->m_pCtrlObject->m_maxItemsInClientReceiveQueue >
        m_pDM->m_pCtrlObject->m_clientOutputQueue.GetCount() ) {

            if (  wxNOT_FOUND != ( idx = m_actionparam.Find( wxT(";") ) ) ) {
                // There is a variable that we should set to true in 
                // this parameter line. We extract it
                var = m_actionparam.Mid( idx + 1 );
                m_actionparam = m_actionparam.Mid( 0, idx );
            }

            vscpEvent *pEvent = new vscpEvent;
            if ( NULL != pEvent ) {

                vscp_setVscpEventFromString( pEvent, m_actionparam );

                m_pDM->m_pCtrlObject->m_mutexClientOutputQueue.Lock();
                m_pDM->m_pCtrlObject->m_clientOutputQueue.Append ( pEvent );
                m_pDM->m_pCtrlObject->m_semClientOutputQueue.Post();
                m_pDM->m_pCtrlObject->m_mutexClientOutputQueue.Unlock();

                // TX Statistics
                m_pDM->m_pClientItem->m_statistics.cntTransmitData += pEvent->sizeData;
                m_pDM->m_pClientItem->m_statistics.cntTransmitFrames++;

                // Set the variable to false if it is defined
                if ( 0 != var.Length() ) {

                    CVSCPVariable *pVariable  = m_pDM->m_pCtrlObject->m_VSCP_Variables.find( var );
                    if ( NULL == pVariable ) {

                        // Non existent - add and set to false
                        m_pDM->m_pCtrlObject->m_VSCP_Variables.add( var, wxT("true") );

                    }
                    else {

                        // Existing - set value to false
                        pVariable->setValueFromString( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN, wxT("true") );

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

        CVSCPVariable *pVar = m_pDM->m_pCtrlObject->m_VSCP_Variables.find( varname );
        if ( NULL == pVar ) {
            // must be a variable	
            wxString wxstrErr = wxT("[Action] Conditional event: No variable defined ");
            wxstrErr += wxstr;
            wxstrErr += _("\n");
            m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
            return false;
        }

    }
    else {
        // must be a variable	
        wxString wxstrErr = wxT("[Action] Conditional event: No variable defined ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
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
            m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
            return false;
        }
    }
    else {
        // must be an event
        wxString wxstrErr = wxT("[Action] Conditional event: No event defined ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
        return false;
    }


    // There must be room in the send queue
    if ( m_pDM->m_pCtrlObject->m_maxItemsInClientReceiveQueue >
        m_pDM->m_pCtrlObject->m_clientOutputQueue.GetCount() ) {

            m_pDM->m_pCtrlObject->m_mutexClientOutputQueue.Lock();
            m_pDM->m_pCtrlObject->m_clientOutputQueue.Append ( pEvent );
            m_pDM->m_pCtrlObject->m_semClientOutputQueue.Post();
            m_pDM->m_pCtrlObject->m_mutexClientOutputQueue.Unlock();

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
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
        return false;
    }

    wxXmlDocument doc;
    if ( !doc.Load ( wxstr ) ) {
        wxString wxstrErr = wxT("[Action] Send event from file: Failed to load event XML file  ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
        return false;
    }

    // start processing the XML file
    if ( doc.GetRoot()->GetName() != wxT ( "vscpevents" ) ) {
        wxString wxstrErr = wxT("[Action] Send event from file: <vscpevents> tag is missing.");
        wxstrErr += _("\n");
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
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
                if ( m_pDM->m_pCtrlObject->m_maxItemsInClientReceiveQueue >
                    m_pDM->m_pCtrlObject->m_clientOutputQueue.GetCount() ) {

                        m_pDM->m_pCtrlObject->m_mutexClientOutputQueue.Lock();
                        m_pDM->m_pCtrlObject->m_clientOutputQueue.Append ( pEvent );
                        m_pDM->m_pCtrlObject->m_semClientOutputQueue.Post();
                        m_pDM->m_pCtrlObject->m_mutexClientOutputQueue.Unlock();

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
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
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
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
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
        // Strange action parameter	
        wxString wxstrErr = wxT("[Action] Get URL: Wrong action parameter ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
        return false;  
    }

    // Get URL
    wxString url = tkz.GetNextToken();

    // Use get or put method
    uint8_t nAccessMethod = 0;   //  "GET" is defualt
    if ( tkz.HasMoreTokens() ) {
        wxString access = tkz.GetNextToken();
        access.UpperCase();
        if ( wxNOT_FOUND == access.Find(wxT("PUT") ) ) {
            nAccessMethod = 1;    // Access-method is PUT
        }
    }

    // put data
    wxString putdata;  
    if ( tkz.HasMoreTokens() ) {
        putdata = tkz.GetNextToken();  
    }

    // proxy
    wxString proxy;  
    if ( tkz.HasMoreTokens() ) {
        proxy = tkz.GetNextToken();  
    }

    // Go do your work mate
    actionThreadURL *thread = new actionThreadURL( m_pDM->m_pCtrlObject,
        url,
        nAccessMethod,
        putdata,
        proxy );
    if ( NULL == thread ) thread->Run();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionStoreVariable
//

bool dmElement::doActionStoreVariable( vscpEvent *pDMEvent )
{
    // Write in possible escapes
	CVSCPVariable *pVar;
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

	//if ( !m_pDM->m_pCtrlObject->m_VSCP_Variables.find( pVar ) ) {
	//pVar = 
    pVar = new CVSCPVariable;
    if ( NULL == pVar ) {
        // must be a variable	
        wxString wxstrErr = wxT("[Action] Store Variable: Could not allocate variable ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
        return false;
    }

    if ( !( pVar->getVariableFromString( wxstr ) ) ) {
        // must be a variable	
        wxString wxstrErr = wxT("[Action] Store Variable: Could not set new variable ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
		delete pVar;
        return false;
    }

    if ( !m_pDM->m_pCtrlObject->m_VSCP_Variables.add( pVar ) ) {
        // must be a variable	
        wxString wxstrErr = wxT("[Action] Store Variable: Could not add variable ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
        return false;
    }  

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionAddVariable
//

bool dmElement::doActionAddVariable( vscpEvent *pDMEvent )
{
    CVSCPVariable *pVar;
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

    pVar = m_pDM->m_pCtrlObject->m_VSCP_Variables.find( strName );
    if ( NULL == pVar ) {

        pVar = new CVSCPVariable;
        if ( NULL == pVar ) {
            // must be a variable	
            wxString wxstrErr = wxT("[Action] Add Variable: Could not allocate variable ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
            return false;
        }

        pVar->setName( strName );
        pVar->setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        pVar->setPersistent( false );

        if ( m_pDM->m_pCtrlObject->m_VSCP_Variables.add( pVar ) ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Could not add new variable ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );

            // Remove the newly allocated var.
            delete pVar;
            return false;
        }

    }

    // Must be a numerical variable
    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG != pVar->getType() ) && 
        ( VSCP_DAEMON_VARIABLE_CODE_INTEGER != pVar->getType() ) &&
        ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE != pVar->getType() ) ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Variable is not numerical ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
            return false;      
    }

    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG != pVar->getType() ) ) {
        pVar->m_longValue += val;
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_INTEGER != pVar->getType() ) ) {
        pVar->m_longValue += val;
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE != pVar->getType() ) ) {
        pVar->m_floatValue += floatval;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionSubtractVariable
//

bool dmElement::doActionSubtractVariable( vscpEvent *pDMEvent )
{
    CVSCPVariable *pVar;
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

    pVar = m_pDM->m_pCtrlObject->m_VSCP_Variables.find( strName );
    if ( NULL == pVar ) {

        pVar = new CVSCPVariable;
        if ( NULL == pVar ) {
            // must be a variable	
            wxString wxstrErr = wxT("[Action] Add Variable: Could not allocate variable ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
            return false;
        }

        pVar->setName( strName );
        pVar->setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        pVar->setPersistent( false );

        if ( m_pDM->m_pCtrlObject->m_VSCP_Variables.add( pVar ) ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Could not add new variable ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );

            // Remove the newly allocated var.
            delete pVar;
            return false;
        }

    }

    // Must be a numerical variable
    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG != pVar->getType() ) && 
        ( VSCP_DAEMON_VARIABLE_CODE_INTEGER != pVar->getType() ) &&
        ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE != pVar->getType() ) ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Variable is not numerical ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
            return false;      
    }

    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG != pVar->getType() ) ) {
        pVar->m_longValue -= val;
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_INTEGER != pVar->getType() ) ) {
        pVar->m_longValue -= val;
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE != pVar->getType() ) ) {
        pVar->m_floatValue -= floatval;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionMultiplyVariable
//

bool dmElement::doActionMultiplyVariable( vscpEvent *pDMEvent )
{
    CVSCPVariable *pVar;
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

    pVar = m_pDM->m_pCtrlObject->m_VSCP_Variables.find( strName );
    if ( NULL == pVar ) {

        pVar = new CVSCPVariable;
        if ( NULL == pVar ) {
            // must be a variable	
            wxString wxstrErr = wxT("[Action] Add Variable: Could not allocate variable ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
            return false;
        }

        pVar->setName( strName );
        pVar->setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        pVar->setPersistent( false );

        if ( m_pDM->m_pCtrlObject->m_VSCP_Variables.add( pVar ) ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Could not add new variable ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );

            // Remove the newly allocated var.
            delete pVar;
            return false;
        }

    }

    // Must be a numerical variable
    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG != pVar->getType() ) && 
        ( VSCP_DAEMON_VARIABLE_CODE_INTEGER != pVar->getType() ) &&
        ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE != pVar->getType() ) ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Variable is not numerical ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
            return false;      
    }

    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG != pVar->getType() ) ) {
        pVar->m_longValue *= val;
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_INTEGER != pVar->getType() ) ) {
        pVar->m_longValue *= val;
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE != pVar->getType() ) ) {
        pVar->m_floatValue *= floatval;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionDivideVariable
//

bool dmElement::doActionDivideVariable( vscpEvent *pDMEvent )
{
    CVSCPVariable *pVar;
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

    pVar = m_pDM->m_pCtrlObject->m_VSCP_Variables.find( strName );
    if ( NULL == pVar ) {

        pVar = new CVSCPVariable;
        if ( NULL == pVar ) {
            // must be a variable	
            wxString wxstrErr = wxT("[Action] Add Variable: Could not allocate variable ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
            return false;
        }

        pVar->setName( strName );
        pVar->setType( VSCP_DAEMON_VARIABLE_CODE_LONG );
        pVar->setPersistent( false );

        if ( m_pDM->m_pCtrlObject->m_VSCP_Variables.add( pVar ) ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Could not add new variable ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );

            // Remove the newly allocated var.
            delete pVar;
            return false;
        }

    }

    // Must be a numerical variable
    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG != pVar->getType() ) && 
        ( VSCP_DAEMON_VARIABLE_CODE_INTEGER != pVar->getType() ) &&
        ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE != pVar->getType() ) ) {
            wxString wxstrErr = wxT("[Action] Add to Variable: Variable is not numerical ");
            wxstrErr += m_actionparam;
            wxstrErr += _("\n");
            m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
            return false;      
    }

    if ( ( VSCP_DAEMON_VARIABLE_CODE_LONG != pVar->getType() ) ) {
        if ( 0 != val ) pVar->m_longValue /= val;
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_INTEGER != pVar->getType() ) ) {
        if ( 0 != val ) pVar->m_longValue /= val;
    }
    else if ( ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE != pVar->getType() ) ) {
        if ( 0.0 != floatval) pVar->m_floatValue /= floatval;
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
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
        return false;  
    }

    // Get timer id
    uint32_t id = vscp_readStringValue( tkz.GetNextToken() );

    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter	
        wxString wxstrErr = wxT("[Action] Start timer: Variable name is missing ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
        return false;   
    }

    // Get variable
    wxString strVarName = tkz.GetNextToken();

    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter	
        wxString wxstrErr = wxT("[Action] Start timer: Delay is missing ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
        return false;   
    }

    // Get delay
    uint32_t delay = vscp_readStringValue( tkz.GetNextToken() );

    if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter	
        wxString wxstrErr = wxT("[Action] Start timer: Setvalue is missing ");
        wxstrErr += wxstr;
        wxstrErr += _("\n");
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
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
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
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
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
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
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
        return false;  
    }

    // Get timer id
    uint32_t id = vscp_readStringValue( tkz.GetNextToken() );

    m_pDM->stopTimer( id );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionWriteTable
//

bool dmElement::doActionWriteTable( vscpEvent *pDMEvent )
{
	wxString tblName;
	time_t timestamp;
	double value;
	bool bFound = false;

	// Write in possible escapes
    wxString wxstr = m_actionparam;
    handleEscapes( pDMEvent, wxstr );

	wxString wxstrErr = _( "[Action] Write Table: Wrong action parameter. Parameter= ");
    wxstrErr += wxstr;
    wxstrErr += _("\n");

	wxStringTokenizer tkz( wxstr, wxT(";") );
    
	if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter	
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
        return false;  
    }
	tblName = tkz.GetNextToken();
	tblName.MakeUpper(); // Make sure it's upercase

	if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter	
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
        return false;  
    }
	timestamp = vscp_readStringValue( tkz.GetNextToken() );

	if ( !tkz.HasMoreTokens() ) {
        // Strange action parameter	
        m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
        return false;  
    }
	value = atof( tkz.GetNextToken().mbc_str() );

	m_pDM->m_pCtrlObject->m_mutexTableList.Lock();  
	listVSCPTables::iterator iter;
	for (iter = m_pDM->m_pCtrlObject->m_listTables.begin(); iter != m_pDM->m_pCtrlObject->m_listTables.end(); ++iter)
	{
		CVSCPTable *pTable = *iter;
		if ( 0 == strcmp( pTable->m_vscpFileHead.nameTable, tblName.mbc_str() ) ) {
            pTable->m_mutexThisTable.Lock();
			pTable->logData( timestamp, value );
            pTable->m_mutexThisTable.Unlock();
			bFound = true;
			break;
		}
	}
	m_pDM->m_pCtrlObject->m_mutexTableList.Unlock();

	if ( !bFound ) {
		wxString wxstrErr = 
			wxString::Format( _("[Action] Write Table: Table [%s] not found. Parameter='%s' "),
			(const char *)tblName.c_str(), 
			(const char *)wxstr.c_str() );
		wxstrErr += wxstr;
		wxstrErr += _("\n");
		m_pDM->m_pCtrlObject->logMsg( wxstrErr, DAEMON_LOGMSG_ERROR );
		return false; 
	}

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
    m_configPath = wxStandardPaths::Get().GetConfigDir();
    m_configPath += _("/vscp/dm.xml");
#else
	m_configPath = _("/srv/vscp/dm.xml");
#endif	
#endif

    // Security logfile is enabled by default
    m_bLogEnable = true;

    m_logLevel = LOG_DM_NORMAL;


#ifdef WIN32
    m_logFileName.SetName( wxStandardPaths::Get().GetConfigDir() + _("vscp_log_dm.txt") );
#else
    m_logFileName.SetName( _("/srv/vscp/logs/vscp_log_dm") );
#endif

    m_pCtrlObject = ctrlObj;

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
    m_configPath = wxStandardPaths::Get().GetConfigDir();
    m_configPath += _("/vscp/dm.xml");
#endif
#else
	m_configPath = _("/srv/vscp/dm.xml");
#endif	

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

    // Close the logfile
    if ( m_bLogEnable ) {
        m_fileLog.Close(); 
    }

}

///////////////////////////////////////////////////////////////////////////////
// init
//

void CDM::init( void )
{
    // Open up the General logging file.
    if ( m_bLogEnable ) {
        m_fileLog.Open( m_logFileName.GetFullPath(), wxFile::write_append ); 
    }
	
	wxString wxlogmsg = 
		wxString::Format(_("DM engine started. DM from [%s]\n"),
							(const char *)m_configPath.c_str() );
	logMsg( wxlogmsg );
}

///////////////////////////////////////////////////////////////////////////////
// setControlObject
//

void CDM::setControlObject( CControlObject *ctrlObj )
{
    m_pCtrlObject = ctrlObj;
}


///////////////////////////////////////////////////////////////////////////////
// logMsg
//

void CDM::logMsg( const wxString& msg, uint8_t level )
{
    wxString logmsg;
    wxDateTime datetime( wxDateTime::GetTimeNow() );
    wxString wxdebugmsg;

    wxdebugmsg = datetime.FormatISODate() + _(" ") + datetime.FormatISOTime() + _(" - ") + msg + _("\r\n");

    // logging.
    if ( level >= m_logLevel ) {
        if ( m_bLogEnable ) {
            m_fileLog.Write( wxdebugmsg );
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// addElement
//
// Read decision matrix from file
//

bool CDM::addElement( dmElement *pItem )
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
// removeRow
//

bool CDM::removeElement( unsigned short pos )
{
    if ( pos >= m_DMList.GetCount() ) return false;
    
    wxDMLISTNode *node = m_DMList.Item( pos );
    
    m_mutexDM.Lock();
    m_DMList.DeleteNode(node);
    m_mutexDM.Unlock();
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getElement
//

dmElement *CDM::getElement( short row ) 
{ 
    if (row < 0) return NULL;
    if ( (unsigned short)row >= m_DMList.GetCount() ) return NULL;
    
    return m_DMList.Item( row )->GetData(); 
}

///////////////////////////////////////////////////////////////////////////////
// load
//
// Read decision matrix from file
//

bool CDM::load ( void )
{
    logMsg( _("DM: Loading decision matrix from :\n") );

    // debug print configuration path
    logMsg( m_configPath + _("\n") );

    // File must exist
    if ( !wxFile::Exists( m_configPath ) ) { 
        logMsg( _("DM: file does not exist.\n") );	
        return false;
    }

    wxXmlDocument doc;
    if ( !doc.Load ( m_configPath ) ) {
        logMsg( _("Faild to load DM. Check format!\n") );
        return false;
    }

    // start processing the XML file
    if ( doc.GetRoot()->GetName() != wxT ( "dm" ) ) {
        logMsg( _("Faild to load DM. <dm> tahs not found, format is wrong!\n") );
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while ( child ) {

        if ( child->GetName() == wxT ( "row" ) ) {

            // Set row default values for row
            dmElement *pDMitem = new dmElement;
            pDMitem->m_control = 0;
            pDMitem->m_action = 0;
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
            addElement ( pDMitem );

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
                else if ( subchild->GetName() == wxT ( "control" ) ) {
                    pDMitem->m_control = vscp_readStringValue( subchild->GetNodeContent() );
                }
                else if ( subchild->GetName() == wxT ( "action" ) ) {
                    pDMitem->m_action = vscp_readStringValue( subchild->GetNodeContent() );
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
						pDMitem->m_timeAllow.parseActionTime(_("*:*:*" ));
					}
                }
                else if ( subchild->GetName() == wxT ( "index" ) ) {
                    wxString str;
                   
                    str = subchild->GetAttribute( wxT( "bMeasurement" ), wxT("false") );
                   
                    str.MakeUpper();
                    if ( wxNOT_FOUND != str.Find(_("TRUE"))) {
                        pDMitem->m_bMeasurement = true;
                    }
                    pDMitem->m_index =vscp_readStringValue( subchild->GetNodeContent() );
                }
                else if ( subchild->GetName() == wxT ( "zone" ) ) {
                    pDMitem->m_zone = vscp_readStringValue( subchild->GetNodeContent() );
                }
                else if ( subchild->GetName() == wxT ( "subzone" ) ) {
                    pDMitem->m_subzone = vscp_readStringValue( subchild->GetNodeContent() );
                }

                subchild = subchild->GetNext();

            }

        }

        child = child->GetNext();

    }

    logMsg( _("DM: Read success.\n"), LOG_DM_NORMAL );

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// save
//
// Write decision matrix to file
//

bool CDM::save ( void )
{
	wxString strLog;
    wxString buf;
	
	strLog = _("DM: Saving decision matrix to: ") + m_configPath + _("\n");
	logMsg( strLog );
	
	if ( !wxFileName::IsFileWritable( m_configPath ) ) {
		strLog = _("DM: File is not writable.\n");
		logMsg( strLog );
		return false;
	}

    wxFFileOutputStream *pFileStream = new wxFFileOutputStream ( m_configPath );
    if ( NULL == pFileStream ) {
		strLog = _("DM: Failed to save: ") + m_configPath + _(" (memory allocation)\n");
		logMsg( strLog );
		return false;
	 }
		
    // Make a copy before we save
    wxCopyFile( m_configPath, m_configPath + _("~") );

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

            pFileStream->Write( "    <control>", strlen( "    <control>" ) );
            buf.Printf( _( "0x%x" ), pDMitem->m_control );
            pFileStream->Write( buf.mb_str(), strlen( buf.mb_str() ) );
            pFileStream->Write( "</control>\n", strlen( "</control>\n" ) );

            pFileStream->Write( "    <action>", strlen( "    <action>" ) );
            buf.Printf( _( "0x%x" ), pDMitem->m_action );
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
            buf.Printf( _( " bMeasurement=\"%s\" " ), 
                    (pDMitem->m_bMeasurement) ? _("true") : _("false") );
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
                    if ( pDMitem->m_bMeasurement ) {
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
                
                // Match do action for this row
                pDMitem->doAction( pEvent );
        
                // Check if DM scan should continue after this DM row
                if ( pDMitem->isScanDontContinueSet() ) break;
                
        }

    }

    m_mutexDM.Unlock();	

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
        EventSecond.vscp_type = VSCP2_TYPE_VSCPD_SECOND;		        // Internal Second Event
        EventSecond.head = VSCP_PRIORITY_NORMAL;				        // Set priority
        EventSecond.timestamp = vscp_makeTimeStamp();                   // Set timestamp
        EventSecond.sizeData = 0;								        // No data
        EventSecond.pdata = NULL;
        //memcpy( EventSecond.GUID, m_pCtrlObject->m_GUID, 16 );	        // Server GUID
        m_pCtrlObject->m_guid.writeGUID( EventSecond.GUID );
        wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal second event\n") );
        feed( &EventSecond );

        // Update timers
        serviceTimers();

        if ( m_rndMinute == wxDateTime::Now().GetSecond() ) {

            vscpEvent EventRandomMinute;
            EventRandomMinute.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomMinute.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_MINUTE;	    // Internal Random-Minute Event
            EventRandomMinute.head = VSCP_PRIORITY_NORMAL;					    // Set priority
            EventRandomMinute.sizeData = 0;									    // No data
            EventRandomMinute.timestamp = vscp_makeTimeStamp();                 // Set timestamp
            EventRandomMinute.pdata = NULL;
            //memcpy( EventRandomMinute.GUID, m_pCtrlObject->m_GUID, 16 );	    // Server GUID
            m_pCtrlObject->m_guid.writeGUID( EventRandomMinute.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal random minute event\n")  );
            feed( &EventRandomMinute );

        }

    }

    if ( m_lastTime.GetMinute() != wxDateTime::Now().GetMinute() ) {

        m_rndMinute = (rand()/ RAND_MAX) * 60;
        vscpEvent EventMinute;
        EventMinute.vscp_class = VSCP_CLASS2_VSCPD;
        EventMinute.vscp_type = VSCP2_TYPE_VSCPD_MINUTE;			        // Internal Minute Event
        EventMinute.head = VSCP_PRIORITY_NORMAL;					        // Set priority
        EventMinute.timestamp = vscp_makeTimeStamp();                       // Set timestamp
        EventMinute.sizeData = 0;									        // No data
        EventMinute.pdata = NULL;
        //memcpy( EventMinute.GUID, m_pCtrlObject->m_GUID, 16 );		        // Server GUID
        m_pCtrlObject->m_guid.writeGUID( EventMinute.GUID );
        wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal minute event\n") );
        m_rndMinute = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 60;
        feed( &EventMinute );

        if ( m_rndHour == wxDateTime::Now().GetMinute() ) {

            vscpEvent EventRandomHour;
            EventRandomHour.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomHour.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_HOUR;       // Internal Random-Hour Event
            EventRandomHour.head = VSCP_PRIORITY_NORMAL;				    // Set priority
            EventRandomHour.timestamp = vscp_makeTimeStamp();               // Set timestamp
            EventRandomHour.sizeData = 0;								    // No data
            EventRandomHour.pdata = NULL;
            //memcpy( EventRandomHour.GUID, m_pCtrlObject->m_GUID, 16 );	    // Server GUID
            m_pCtrlObject->m_guid.writeGUID( EventRandomHour.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal random hour event\n") );
            feed( &EventRandomHour );

        }
    }

    if ( m_lastTime.GetHour() != wxDateTime::Now().GetHour() ) {

        m_rndHour = (rand()/ RAND_MAX) * 60;
        vscpEvent EventHour;
        EventHour.vscp_class = VSCP_CLASS2_VSCPD;
        EventHour.vscp_type = VSCP2_TYPE_VSCPD_HOUR;				        // Internal Hour Event
        EventHour.head = VSCP_PRIORITY_NORMAL;						        // Set priority
        EventHour.timestamp = vscp_makeTimeStamp();                         // Set timestamp
        EventHour.sizeData = 0;												// No data
        EventHour.pdata = NULL;
        //memcpy( EventtHour.GUID, m_pCtrlObject->m_GUID, 16 );		        // Server GUID
        m_pCtrlObject->m_guid.writeGUID( EventHour.GUID );
        wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal hour event\n") );
        m_rndHour = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 24;
        feed( &EventHour );

        // Check if it's time to send radom day event
        if ( m_rndDay == wxDateTime::Now().GetHour() ) {

            vscpEvent EventRandomDay;
            EventRandomDay.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomDay.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_DAY;		    // Internal Random-Day Event
            EventRandomDay.head = VSCP_PRIORITY_NORMAL;					    // Set priority
            EventRandomDay.timestamp = vscp_makeTimeStamp();                // Set timestamp
            EventRandomDay.sizeData = 0;								    // No data
            EventRandomDay.pdata = NULL;
            //memcpy( EventRandomDay.GUID, m_pCtrlObject->m_GUID, 16 );	    // Server GUID
            m_pCtrlObject->m_guid.writeGUID( EventRandomDay.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal random day event\n") );
            feed( &EventRandomDay );

        }
    }

    if ( m_lastTime.GetDay() != wxDateTime::Now().GetDay() ) {

        m_rndDay = (rand()/ RAND_MAX) * 24;
        vscpEvent EventDay;
        EventDay.vscp_class = VSCP_CLASS2_VSCPD;
        EventDay.vscp_type = VSCP2_TYPE_VSCPD_MIDNIGHT;			            // Internal Midnight Event
        EventDay.head = VSCP_PRIORITY_NORMAL;					            // Set priority
        EventDay.timestamp = vscp_makeTimeStamp();                          // Set timestamp
        EventDay.sizeData = 0;									            // No data
        EventDay.pdata = NULL;
        //memcpy( EventDay.GUID, m_pCtrlObject->m_GUID, 16 );		            // Server GUID
        m_pCtrlObject->m_guid.writeGUID( EventDay.GUID );
        wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal day event\n") );
        m_rndDay = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 7;
        feed( &EventDay );

        // Check if it's time to send random week event
        if ( m_rndWeek == wxDateTime::Now().GetDay() ) {

            vscpEvent EventRandomWeek;
            EventRandomWeek.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomWeek.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_WEEK;       // Internal Random-Week Event
            EventRandomWeek.head = VSCP_PRIORITY_NORMAL;				    // Set priority
            EventRandomWeek.timestamp = vscp_makeTimeStamp();               // Set timestamp
            EventRandomWeek.sizeData = 0;								    // No data
            EventRandomWeek.pdata = NULL;
            //memcpy( EventRandomWeek.GUID, m_pCtrlObject->m_GUID, 16 );	    // Server GUID
            m_pCtrlObject->m_guid.writeGUID( EventRandomWeek.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal random week event\n") );
            feed( &EventRandomWeek );

        }
    }

    if ( ( 1 == wxDateTime::Now().GetWeekDay() ) && 
        ( m_lastTime.GetWeekDay() != wxDateTime::Now().GetWeekDay() ) ) {

            m_rndWeek = (rand()/ RAND_MAX) * 7;
            vscpEvent EventWeek;
            EventWeek.vscp_class = VSCP_CLASS2_VSCPD;
            EventWeek.vscp_type = VSCP2_TYPE_VSCPD_WEEK; 	                // Internal Week Event
            EventWeek.head = VSCP_PRIORITY_NORMAL;						    // Set priority
            EventWeek.timestamp = vscp_makeTimeStamp();                     // Set timestamp
            EventWeek.sizeData = 0;										    // No data
            EventWeek.pdata = NULL;
            //memcpy( EventWeek.GUID, m_pCtrlObject->m_GUID, 16 );		    // Server GUID
            m_pCtrlObject->m_guid.writeGUID( EventWeek.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal week event\n") );
            m_rndWeek = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 52;
            feed( &EventWeek );

            if ( m_rndMonth == wxDateTime::Now().GetWeekDay() ) {

                vscpEvent EventRandomMonth;
                EventRandomMonth.vscp_class = VSCP_CLASS2_VSCPD;
                EventRandomMonth.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_MONTH;     // Internal Random-Month Event
                EventRandomMonth.head = VSCP_PRIORITY_NORMAL;					// Set priority
                EventRandomMonth.timestamp = vscp_makeTimeStamp();              // Set timestamp
                EventRandomMonth.sizeData = 0;									// No data
                EventRandomMonth.pdata = NULL;
                //memcpy( EventRandomMonth.GUID, m_pCtrlObject->m_GUID, 16 );	    // Server GUID
                m_pCtrlObject->m_guid.writeGUID( EventRandomMonth.GUID );
                wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal random month event\n") );
                feed( &EventRandomMonth );

            }
    }

    if ( m_lastTime.GetMonth() != wxDateTime::Now().GetMonth() ) {

        m_rndMonth = (rand()/ RAND_MAX) * 30;
        vscpEvent EventMonth;
        EventMonth.vscp_class = VSCP_CLASS2_VSCPD;
        EventMonth.vscp_type = VSCP2_TYPE_VSCPD_MONTH; 	                        // Internal Month Event
        EventMonth.head = VSCP_PRIORITY_NORMAL;							        // Set priority
        EventMonth.timestamp = vscp_makeTimeStamp();                            // Set timestamp
        EventMonth.sizeData = 0;										        // No data
        EventMonth.pdata = NULL;
        //memcpy( EventMonth.GUID, m_pCtrlObject->m_GUID, 16 );			        // Server GUID
        m_pCtrlObject->m_guid.writeGUID( EventMonth.GUID );
        wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal month event\n") );
        m_rndMonth = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 12;
        feed( &EventMonth );

        if ( m_rndYear == wxDateTime::Now().GetMonth() ) {

            vscpEvent EventRandomYear;
            EventRandomYear.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomYear.vscp_type = VSCP2_TYPE_VSCPD_RANDOM_YEAR;	        // Internal Random-Minute Event 
            EventRandomYear.head = VSCP_PRIORITY_NORMAL;				        // Set priority
            EventRandomYear.timestamp = vscp_makeTimeStamp();                   // Set timestamp
            EventRandomYear.sizeData = 0;								        // No data
            EventRandomYear.pdata = NULL;
            //memcpy( EventRandomYear.GUID, m_pCtrlObject->m_GUID, 16 );	    // Server GUID
            m_pCtrlObject->m_guid.writeGUID( EventRandomYear.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal random year event\n") );
            feed( &EventRandomYear );

        }
    }

    if ( m_lastTime.GetYear() != wxDateTime::Now().GetYear() ) {

        m_rndYear = (rand()/ RAND_MAX) * 365;
        vscpEvent EventYear;
        EventYear.vscp_class = VSCP_CLASS2_VSCPD;
        EventYear.vscp_type = VSCP2_TYPE_VSCPD_YEAR; 			        // Internal Year Event
        EventYear.head = VSCP_PRIORITY_NORMAL;					        // Set priority
        EventYear.timestamp = vscp_makeTimeStamp();                     // Set timestamp
        EventYear.sizeData = 0;									        // No data
        EventYear.pdata = NULL;
        //memcpy( EventYear.GUID, m_pCtrlObject->m_GUID, 16 );	        // Server GUID
        m_pCtrlObject->m_guid.writeGUID( EventYear.GUID );
        wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal year event\n") );
        m_rndYear = (uint8_t)( (double)rand() / ((double)(RAND_MAX) + (double)(1)) ) * 365;
        feed( &EventYear );

    }

    if ( ( 1 == wxDateTime::Now().GetMonth() ) && 
        ( m_lastTime.GetMonth() != wxDateTime::Now().GetMonth() ) ) {

            vscpEvent EventQuarter;
            EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
            EventQuarter.vscp_type = VSCP2_TYPE_VSCPD_QUARTER; 	        // Internal Quarter Event
            EventQuarter.head = VSCP_PRIORITY_NORMAL;					// Set priority
            EventQuarter.timestamp = vscp_makeTimeStamp();              // Set timestamp
            EventQuarter.sizeData = 0;									// No data
            EventQuarter.pdata = NULL;
            //memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 );     // Server GUID
            m_pCtrlObject->m_guid.writeGUID( EventQuarter.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal quarter event\n") );
            feed( &EventQuarter );

    }

    if ( ( 4 == wxDateTime::Now().GetMonth() ) && 
        ( m_lastTime.GetMonth() != wxDateTime::Now().GetMonth() ) ) {

            vscpEvent EventQuarter;
            EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
            EventQuarter.vscp_type = VSCP2_TYPE_VSCPD_QUARTER; 	        // Internal Quarter Event
            EventQuarter.head = VSCP_PRIORITY_NORMAL;	                // Set priority
            EventQuarter.timestamp = vscp_makeTimeStamp();              // Set timestamp
            EventQuarter.sizeData = 0;									// No data
            EventQuarter.pdata = NULL;
            //memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 );	    // Server GUID
            m_pCtrlObject->m_guid.writeGUID( EventQuarter.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal quarter event\n") );
            feed( &EventQuarter );

    }

    if ( ( 7 == wxDateTime::Now().GetMonth() ) && 
        ( m_lastTime.GetMonth() != wxDateTime::Now().GetMonth() ) ) {

            vscpEvent EventQuarter;
            EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
            EventQuarter.vscp_type = VSCP2_TYPE_VSCPD_QUARTER; 	        // Internal Quarter Event
            EventQuarter.head = VSCP_PRIORITY_NORMAL;					// Set priority
            EventQuarter.timestamp = vscp_makeTimeStamp();              // Set timestamp
            EventQuarter.sizeData = 0;					                // No data
            EventQuarter.pdata = NULL;
            //memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 );     // Server GUID
            m_pCtrlObject->m_guid.writeGUID( EventQuarter.GUID );
            wxLogTrace( _("wxTRACE_vscpd_dm"), _("Internal quarter event\n") );
            feed( &EventQuarter );

    }

    if ( ( 10 == wxDateTime::Now().GetMonth() ) && 
        ( m_lastTime.GetMonth() != wxDateTime::Now().GetMonth() ) ) {

            vscpEvent EventQuarter;
            EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
            EventQuarter.vscp_type = VSCP2_TYPE_VSCPD_QUARTER; 	        // Internal Quarter Event
            EventQuarter.head = VSCP_PRIORITY_NORMAL;			        // Set priority
            EventQuarter.timestamp = vscp_makeTimeStamp();              // Set timestamp
            EventQuarter.sizeData = 0;									// No data
            EventQuarter.pdata = NULL;
            //memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 );     // Server GUID
            m_pCtrlObject->m_guid.writeGUID( EventQuarter.GUID );
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
    CVSCPVariable *pVariable;

    DMTIMERS::iterator it;
    for ( it = m_timerHash.begin(); it != m_timerHash.end(); it++ ) {

        dmTimer *pTimer = it->second;

        if ( pTimer->isActive() && 
            ( NULL != ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( pTimer->getVariableName() ) ) ) ) {

                if ( !pTimer->decTimer() ) {

                    // We have reached zero

                    pTimer->setActive( false );  // We go inactive 

                    // Handle the setvalue
                    if ( pTimer->getSetValue() ) {
                        pVariable->setTrue();
                    }
                    else {
                        pVariable->setFalse(); 
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
    wxString logStr = wxString::Format(_("Add Timer %s."), 
										(const char *)nameVar.c_str() );
    logMsg( logStr, LOG_DM_EXTRA );

    // Check if the timer already exist - set new values
    if ( NULL != ( pTimer = m_timerHash[ id ] ) ) {
        pTimer->setVariableName( nameVar );
        pTimer->setSetValue( setValue );
        pTimer->setDelay( delay );
        pTimer->setActive( bStart );
        
        // Log
        wxString logStr = wxString::Format(_("Timer %d already exist."), id );
        logMsg( logStr, LOG_DM_DEBUG );
        return id;
    }

    nameVar.Trim( true );
    nameVar.Trim( false );

    // Check if variable is defined
    if ( NULL != m_pCtrlObject->m_VSCP_Variables.find( nameVar ) ) {

        // Log
        wxString logStr = wxString::Format(_("Variable is not defined %s"),  
											(const char *)nameVar.c_str() );
        logMsg( logStr, LOG_DM_DEBUG );

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
        wxString logStr = wxString::Format(_("Variable already defined %s."), 
											(const char *)nameVar.c_str() );
        logMsg( logStr, LOG_DM_DEBUG );

        if ( m_pCtrlObject->m_VSCP_Variables.add( nameVar, wxT("false"), VSCP_DAEMON_VARIABLE_CODE_BOOLEAN ) ) {

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
    wxString logStr = wxString::Format(_("Timer %d started."), idTimer );
    logMsg( logStr, LOG_DM_EXTRA );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// startTimer
//

int CDM::startTimer( uint16_t idTimer, wxString& nameVariable, uint32_t delay, bool bSetValue )
{
    uint16_t rv = 0;

    dmTimer *pTimer = m_timerHash[ idTimer ];
    if ( NULL == pTimer ) {

        // Log
        wxString logStr = wxString::Format(_("Timer %d created."), idTimer );
        logMsg( logStr, LOG_DM_EXTRA );

        // Nonexistent timer - create it
        addTimer( idTimer, 
                nameVariable, 
                delay, 
                true, 
                bSetValue );
    }

    // Log
    wxString logStr = wxString::Format(_("Timer %d started."), idTimer );
    logMsg( logStr, LOG_DM_EXTRA );

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
    wxString logStr = wxString::Format(_("Timer %d stopped."), idTimer );
    logMsg( logStr, LOG_DM_EXTRA );

    pTimer->stopTimer();

    return true;
}



