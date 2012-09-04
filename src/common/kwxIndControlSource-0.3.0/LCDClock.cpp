/////////////////////////////////////////////////////////////////////////////
// Name:        LCDClock.cpp
// Purpose:     wxIndustrialControls Library
// Author:      Marco Cavallini <m.cavallini AT koansoftware.com>
// Modified by: 
// Copyright:   (C)2004-2006 Copyright by Koan s.a.s. - www.koansoftware.com
// Licence:     KWIC License http://www.koansoftware.com/kwic/kwic-license.htm
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "kprec.h"		//#include "wx/wxprec.h"

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include <wx/event.h>

#include "LCDClock.h"
#include "LCDWindow.h"
#include "TimeAlarm.h"

BEGIN_EVENT_TABLE(kwxLCDClock,kwxLCDDisplay)
	EVT_TIMER(TIMER_TIME, kwxLCDClock::OnTimer)
END_EVENT_TABLE()

kwxLCDClock::kwxLCDClock(wxWindow *parent, 
						const wxPoint& pos , 
						const wxSize& size ) 
:kwxLCDDisplay(parent, pos, size)
{
	time_t reset = 0 ;

	m_LastCheck.Set(reset) ;

	wxDateTime now = wxDateTime::Now();

	alarm = new CTimeAlarm();
	alarm->SetAlarmTime(now) ;
	
}


kwxLCDClock::~kwxLCDClock()
{
	if (m_timer.IsRunning())
		m_timer.Stop() ;
		
	delete alarm ;	
}


void kwxLCDClock::OnTimer(wxTimerEvent &event)
{
	wxDateTime now = wxDateTime::Now();
	double jDate ;

	if (!now.IsEqualTo(m_LastCheck))
	{
		jDate = now.GetModifiedJulianDayNumber();

		SetValue(now.FormatISOTime()) ;	//(HH:MM:SS)
//		SetValue(DateTime.FormatISODate()) ;	//(YYYY-MM-DD)
//		SetValue(DateTime.Format("%d-%m-%Y") );	//(DD-MM-YYYY) ;
		m_LastCheck = now ;
	}


}


void kwxLCDClock::StartClock()
{
	m_timer.SetOwner(this, TIMER_TIME) ; 

	m_timer.Start(500, wxTIMER_CONTINUOUS) ;
}


void kwxLCDClock::StopClock()
{
	m_timer.Stop() ;
}