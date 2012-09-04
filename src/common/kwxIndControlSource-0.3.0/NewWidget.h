/////////////////////////////////////////////////////////////////////////////
// Name:        NewWidget.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     
// RCS-ID:      
// Copyright:   (C)2003 
// Licence:     wxWindows
/////////////////////////////////////////////////////////////////////////////

#ifndef NewWidget_H
#define NewWidget_H

class kwxNewWidget : public wxWindow
{
//	DECLARE_DYNAMIC_CLASS(kwxNewWidget)

public:
    // ctor(s)
    kwxNewWidget(wxWindow *parent,
				const wxWindowID id         = -1,
				const wxString&  label      = wxEmptyString,
				wxBitmap&  defaultBitmap	= wxNullBitmap,
				wxBitmap&  MoveMouseBitmap	= wxNullBitmap,
				wxBitmap&  PressMouseBitmap	= wxNullBitmap,
				const wxPoint&   pos        = wxDefaultPosition,
				const wxSize&    size       = wxDefaultSize,
				const long int   style      = 0);
	int m_stato,m_oldstato;

//	virtual ~kwxNewWidget();

    void     SetLabel(wxString str); // Change button label.
	void	kwxNewWidget::OnEraseBackGround(wxEraseEvent& event);

private:
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE()

	void     OnPaint(wxPaintEvent& event);
	void     OnMouse(wxMouseEvent& event);
	bool	 MouseIsOnButton();


	wxString mLabelStr;
    wxBitmap* mDefaultBitmap;
	wxBitmap* mMoveMouseBitmap;
	wxBitmap* mPressMouseBitmap;

};

#endif // KwxNewWidget_H
