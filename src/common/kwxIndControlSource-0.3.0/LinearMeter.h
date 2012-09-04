/////////////////////////////////////////////////////////////////////////////
// Name:        LinearMeter.h
// Purpose:     wxIndustrialControls Library
// Author:      Marco Cavallini <m.cavallini AT koansoftware.com>
// Modified by: 
// Copyright:   (C)2004-2006 Copyright by Koan s.a.s. - www.koansoftware.com
// Licence:     KWIC License http://www.koansoftware.com/kwic/kwic-license.htm
/////////////////////////////////////////////////////////////////////////////


#ifndef __LINEARMETER_H__
#define __LINEARMETER_H__

#define MAX_TAGS 10

class kwxLinearMeter : public wxWindow
{
public:
    kwxLinearMeter(wxWindow *parent,
				const wxWindowID id         = -1,
				const wxString&  label      = wxEmptyString,
				const wxPoint&   pos        = wxDefaultPosition,
				const wxSize&    size       = wxDefaultSize,
				const long int   style      = 0);

	virtual ~kwxLinearMeter();

	void SetRangeVal(int min, int max) {m_nMin = min; m_nMax = max ; } ;
	void SetActiveBarColour(wxColour *colour) { m_cActiveBar = colour ; } ;
	void SetPassiveBarColour(wxColour *colour) { m_cPassiveBar = colour ; } ;
	void SetBorderColour(wxColour *colour) { m_cBorderColour = colour ; } ;
	void SetValue(int val) ; 
	void SetTxtFont(wxFont &font) {m_Font = font ; } ;
	void SetTxtLimitColour(wxColour *colour) { m_cLimitColour = colour ; } ;
	void SetTxtValueColour(wxColour *colour) { m_cValueColour = colour ; } ;
	void SetTagsColour(wxColour *colour) { m_cTagsColour = colour ; } ;

	void SetOrizDirection(bool dir) { m_bDirOrizFlag = dir ; } ;
	void ShowCurrent(bool show) { m_bShowCurrent = show ; } ;
	void ShowLimits(bool show) { m_bShowLimits = show ; } ;

	void AddTag(int val) ;

	int GetValue() { return m_nRealVal ; } ;

protected:
	int m_nScaledVal ;
	int m_nRealVal ;

	bool m_bDirOrizFlag ;

	int m_nMax ;
	int m_nMin ;

private:
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE()

	void    OnPaint(wxPaintEvent& event);
	void	OnEraseBackGround(wxEraseEvent& event) {};
	wxWindowID	 GetID() { return m_id; } ;

private:

	void DrawLimits(wxDC &dc);
	void DrawCurrent(wxDC &dc);
	void DrawTags(wxDC &dc);

	wxWindowID m_id ;
	wxBitmap *membitmap ;

	int m_aTagsVal[MAX_TAGS] ;

	int m_nTagsNum ;

	
	bool m_bShowCurrent ;
	bool m_bShowLimits ;

	wxColour *m_cActiveBar ;
	wxColour *m_cPassiveBar ;
	wxColour *m_cLimitColour ;
	wxColour *m_cValueColour ;
	wxColour *m_cBorderColour ;
	wxColour *m_cTagsColour ;
	wxFont m_Font ;
};

#endif