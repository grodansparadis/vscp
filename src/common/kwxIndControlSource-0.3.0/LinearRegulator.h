/////////////////////////////////////////////////////////////////////////////
// Name:        LinearRegulator.h
// Purpose:     wxIndustrialControls Library
// Author:      Marco Cavallini <m.cavallini AT koansoftware.com>
// Modified by: 
// Copyright:   (C)2004-2006 Copyright by Koan s.a.s. - www.koansoftware.com
// Licence:     KWIC License http://www.koansoftware.com/kwic/kwic-license.htm
/////////////////////////////////////////////////////////////////////////////


const kwxEVT_LINEARREG_FIRST = wxEVT_FIRST + 5401;

const wxEventType kwxEVT_LINEARREG_CHANGE	= kwxEVT_LINEARREG_FIRST + 1;

#define EVT_LINEARREG_CHANGE(id, fn)	\
		DECLARE_EVENT_TABLE_ENTRY(kwxEVT_LINEARREG_CHANGE, id, -1, \
		(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)&fn, \
		(wxObject*)NULL ),

class kwxLinearRegulator : public wxControl
{
public:
	kwxLinearRegulator(void) { m_id = -1 ; };

	kwxLinearRegulator(wxWindow *parent,
				const wxWindowID id         = -1,
				const wxString&  label      = wxEmptyString,
				const wxPoint&   pos        = wxDefaultPosition,
				const wxSize&    size       = wxDefaultSize,
				const long int   style      = 0);

	virtual ~kwxLinearRegulator();

	void SetRangeVal(int min, int max) {m_nMin = min; m_nMax = max ; } ;
	void SetOrizDirection(bool dir) { m_bDirOrizFlag = dir ; } ;

	void SetActiveBarColour(wxColour *colour) { m_cActiveBar = colour ; } ;
	void SetPassiveBarColour(wxColour *colour) { m_cPassiveBar = colour ; } ;
	void SetBorderColour(wxColour *colour) { m_cBorderColour = colour ; } ;

	void SetTxtLimitColour(wxColour *colour) { m_cLimitColour = colour ; } ;
	void SetTxtValueColour(wxColour *colour) { m_cValueColour = colour ; } ;
	void SetTagsColour(wxColour *colour) { m_cTagsColour = colour ; } ;
	void SetTxtFont(wxFont &font) {m_Font = font ; } ;

	void ShowCurrent(bool show) { m_bShowCurrent = show ; } ;
	void ShowLimits(bool show) { m_bShowLimits = show ; } ;

	void AddTag(int val) ;

	int GetVal() { return m_nRealVal ; } ;
	void SetVal(int val) ;


private:
	DECLARE_DYNAMIC_CLASS(kwxLinearRegulator)
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE()

	void	OnMouse(wxMouseEvent& event) ;
	void	OnEraseBackGround(wxEraseEvent& event) {};
	void	OnPaint(wxPaintEvent& event);
	void	DrawPosition() ;
	void	Change();

	void DrawLimits(wxDC &dc);
	void DrawCurrent(wxDC &dc);
	void DrawTags(wxDC &dc);

	wxWindowID	 GetID() { return m_id; } ;

	wxWindowID m_id ;

	int m_nStato ;
	int m_nMax ;
	int m_nMin ;

	int m_nScaledVal ;
	int m_nRealVal ;
	int m_nTagsNum ;

	int m_aTagsVal[10] ;

	bool m_bDirOrizFlag ;
	bool m_bShowCurrent ;
	bool m_bShowLimits ;

	wxPoint m_mousePosition ;
	wxSize m_ControlSize ;
	wxBitmap* membitmap ;

	wxColour *m_cActiveBar ;
	wxColour *m_cPassiveBar ;
	wxColour *m_cBorderColour ;
	wxColour *m_cLimitColour ;
	wxColour *m_cValueColour ;
	wxColour *m_cTagsColour ;

	wxFont m_Font ;

} ;