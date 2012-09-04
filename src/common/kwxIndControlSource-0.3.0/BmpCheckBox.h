/////////////////////////////////////////////////////////////////////////////
// Name:        BmpCheckBox.h
// Purpose:     wxIndustrialControls Library
// Author:      Marco Cavallini <m.cavallini AT koansoftware.com>
// Modified by: 
// Copyright:   (C)2004-2006 Copyright by Koan s.a.s. - www.koansoftware.com
// Licence:     KWIC License http://www.koansoftware.com/kwic/kwic-license.htm
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


class kwxBmpCheckBox : public wxControl
{

public:
    // ctor(s)
    kwxBmpCheckBox(void) { m_id = -1 ; };

    kwxBmpCheckBox(wxWindow *parent,
				const wxWindowID id         = -1,
				const wxString&  label      = wxEmptyString,
				wxBitmap&  OnBitmap	= wxNullBitmap,
				wxBitmap&  OffBitmap	= wxNullBitmap,
				wxBitmap&  OnSelBitmap	= wxNullBitmap,
				wxBitmap&  OffSelBitmap	= wxNullBitmap,
				const wxPoint&   pos        = wxDefaultPosition,
				const wxSize&    size       = wxDefaultSize,
				const long int   style      = 0 );


	virtual ~kwxBmpCheckBox();

    void	SetLabel(wxString str); // Change button label.
	void	SetBorder(bool bord , int style) { m_bBord = bord ; m_nStyle = style ; } ;
	bool	GetState() { return m_bPress ; } ;
	void	SetState(bool newstate) ;

private:

	DECLARE_DYNAMIC_CLASS(kwxBmpCheckBox)
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
	bool m_bPress;
	bool m_bBord;
	int m_nStyle ;
	
	wxString mLabelStr ;
    wxBitmap* mOffBitmap ;
	wxBitmap* mOnBitmap ;
	wxBitmap* mOffSelBitmap ;
	wxBitmap* mOnSelBitmap ;
	wxBitmap* membitmap ;

};

#endif // KwxNewWidget_H
