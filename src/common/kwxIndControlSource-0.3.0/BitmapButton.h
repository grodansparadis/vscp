/////////////////////////////////////////////////////////////////////////////
// Name:        NewBitmapButton.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     
// RCS-ID:      
// Copyright:   (C)2003 
// Licence:     wxWindows
/////////////////////////////////////////////////////////////////////////////

#ifndef KwxNewWidget_H
#define KwxNewWidget_H

////////////////// Custom events /////////////////////////

const kwxEVT_BITBUTTON_FIRST = wxEVT_FIRST + 5400;

const wxEventType kwxEVT_BITBUTTON_CLICK	= kwxEVT_BITBUTTON_FIRST + 1;

#define EVT_BITBUTTON_CLICK(id, fn)	\
		DECLARE_EVENT_TABLE_ENTRY(kwxEVT_BITBUTTON_CLICK, id, -1, \
		(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)&fn, \
		(wxObject*)NULL ),


//////////////////////////////////////////////////////////


class kwxBitmapButton : public wxControl
{

public:
    // ctor(s)
    kwxBitmapButton() { };

    kwxBitmapButton(wxWindow *parent,
				const wxWindowID id         = -1,
				const wxString&  label      = wxEmptyString,
				wxBitmap&  defaultBitmap	= wxNullBitmap,
				wxBitmap&  MoveMouseBitmap	= wxNullBitmap,
				wxBitmap&  PressMouseBitmap	= wxNullBitmap,
				const wxPoint&   pos        = wxDefaultPosition,
				const wxSize&    size       = wxDefaultSize,
				const long int   style      = 0);


//	virtual ~kwxBitmapButton();

    void     SetLabel(wxString str); // Change button label.

private:

	DECLARE_DYNAMIC_CLASS(kwxBitmapButton)
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE()

	void	OnPaint(wxPaintEvent& event);
	void	OnMouse(wxMouseEvent& event);
	void	OnEraseBackGround(wxEraseEvent& event) {};
	void	Click();
	wxWindowID	 GetID() { return m_id; } ;
	
	wxWindowID m_id ;
	int m_stato ;
	int m_oldstato ;
	
	wxString mLabelStr ;
    wxBitmap* mDefaultBitmap ;
	wxBitmap* mMoveMouseBitmap ;
	wxBitmap* mPressMouseBitmap ;
	wxBitmap* membitmap ;

};

#endif // KwxNewWidget_H
