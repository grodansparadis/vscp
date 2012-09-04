/////////////////////////////////////////////////////////////////////////////
// Name:        BmpSwitcher.h
// Purpose:     wxIndustrialControls Library
// Author:      Marco Cavallini <m.cavallini AT koansoftware.com>
// Modified by: 
// Copyright:   (C)2004-2006 Copyright by Koan s.a.s. - www.koansoftware.com
// Licence:     KWIC License http://www.koansoftware.com/kwic/kwic-license.htm
/////////////////////////////////////////////////////////////////////////////



WX_DECLARE_LIST(wxBitmap, CBmpList);

class kwxBmpSwitcher : public wxWindow
{
public:
	kwxBmpSwitcher(wxWindow *parent,
				const wxWindowID id         = -1,
				const wxString&  label      = wxEmptyString,
				const wxPoint&   pos        = wxDefaultPosition,
				const wxSize&    size       = wxDefaultSize,
				const long int   style      = 0);

	virtual ~kwxBmpSwitcher();
	
	//Incrementa stato di avanzamento
	void IncState() ;
	//Imposta stato di avanzamento
	void SetState(int state) ;

	int GetState() { return m_nState ; } ;
	//Aggiunge una bitmap alla lista
	void AddBitmap(wxBitmap *bitmap) ;

	CBmpList m_bmplist;

	wxBitmap *m_pCurrent ;

private:
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE()

	void    OnPaint(wxPaintEvent& event);
	void	OnEraseBackGround(wxEraseEvent& event) {};

	wxBitmap *membitmap ;

	int m_nCount ;
	int m_nState ;

} ;

