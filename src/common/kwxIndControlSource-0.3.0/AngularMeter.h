/////////////////////////////////////////////////////////////////////////////
// Name:        AngularMeter.h
// Purpose:     wxIndustrialControls Library
// Author:      Marco Cavallini <m.cavallini AT koansoftware.com>
// Modified by: 
// Copyright:   (C)2004-2006 Copyright by Koan s.a.s. - www.koansoftware.com
// Licence:     KWIC License http://www.koansoftware.com/kwic/kwic-license.htm
/////////////////////////////////////////////////////////////////////////////


#define MAXSECTORCOLOR 10

class kwxAngularMeter : public wxWindow
{
public:
    kwxAngularMeter(wxWindow *parent,
				const wxWindowID id         = -1,
				const wxString&  label      = wxEmptyString,
				const wxPoint&   pos        = wxDefaultPosition,
				const wxSize&    size       = wxDefaultSize,
				const long int   style      = 0);

	virtual ~kwxAngularMeter();
	void SetSectorColor(int nSector, wxColour colour) ;
	void SetNumSectors(int nSector) { m_nSec = nSector ; };
	void SetNumTick(int nTick) { m_nTick = nTick ; };
	void SetRange(int min, int max) { m_nRangeStart = min ; m_nRangeEnd = max ; } ;
	void SetAngle(int min, int max) { m_nAngleStart = min ; m_nAngleEnd = max ; } ;
	void SetValue(int val);
	void SetNeedleColour(wxColour colour) { m_cNeedleColour = colour ; } ;
	void SetBackColour(wxColour colour) { m_cBackColour = colour ; } ;
	void SetBorderColour(wxColour colour) { m_cBorderColour = colour ; } ;
	void SetTxtFont(wxFont &font) { m_Font = font ; } ;
	void DrawCurrent(bool state) { m_bDrawCurrent = state ; } ;


private:
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE()

	void    OnPaint(wxPaintEvent& event);
	void	OnEraseBackGround(wxEraseEvent& event) {};
	void	DrawTicks(wxDC &dc) ;
	void	DrawNeedle(wxDC &dc) ;
	void	DrawSectors(wxDC &dc) ;
	wxWindowID	 GetID() { return m_id ; } ;

private:
	wxWindowID m_id;
	int		m_nRangeStart ;
	int		m_nRangeEnd ;
	int		m_nAngleStart ;
	int		m_nAngleEnd ;
	double	m_nScaledVal ;
	int		m_nTick ;
	int		m_nSec ;
	double	m_dPI ;
	int		m_nRealVal ;
	bool	m_bDrawCurrent ;
	wxColour m_aSectorColor[MAXSECTORCOLOR] ;
	wxBitmap *membitmap ;
	wxFont m_Font ;
	wxColour m_cNeedleColour ;
	wxColour m_cBackColour ;
	wxColour m_cBorderColour ;

    wxBitmap  *m_pPreviewBmp ;


};