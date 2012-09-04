/////////////////////////////////////////////////////////////////////////////
// Name:        LCDClock.h
// Purpose:     wxIndustrialControls Library
// Author:      Marco Cavallini <m.cavallini AT koansoftware.com>
// Modified by: 
// Copyright:   (C)2004-2006 Copyright by Koan s.a.s. - www.koansoftware.com
// Licence:     KWIC License http://www.koansoftware.com/kwic/kwic-license.htm
/////////////////////////////////////////////////////////////////////////////


#include "LCDWindow.h"
#include <wx/datetime.h>

#define TIMER_TIME 3001

class CTimeAlarm;

class kwxLCDClock : public kwxLCDDisplay
{
public:

	kwxLCDClock(wxWindow *parent, 
				const wxPoint& pos = wxDefaultPosition, 
				const wxSize& size = wxDefaultSize) ;

	~kwxLCDClock() ;

	void StartClock() ;
	void StopClock() ;
	CTimeAlarm *alarm ;

private:
	DECLARE_EVENT_TABLE()

	wxTimer m_timer ;

	wxDateTime m_LastCheck ;
	void OnTimer(wxTimerEvent& event);
};