/////////////////////////////////////////////////////////////////////////////
// Name:        TestdcFrm.h
// Purpose:     wxIndustrialControls Library
// Author:      Marco Cavallini <m.cavallini AT koansoftware.com>
// Modified by: 
// Copyright:   (C)2004-2006 Copyright by Koan s.a.s. - www.koansoftware.com
// Licence:     KWIC License http://www.koansoftware.com/kwic/kwic-license.htm
/////////////////////////////////////////////////////////////////////////////


#ifndef TestdcFrame_H
#define TestdcFrame_H

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

//class MyCanvas;
class MyPanel ;


// Define a new frame type: this is going to be our main frame
class TestdcFrame : public wxFrame
{
public:
    // ctor(s)
    TestdcFrame(const wxString& title, const wxPoint& pos, const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
	virtual ~TestdcFrame();

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);


private:
    DECLARE_CLASS(TestdcFrame)
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class kwxAngularMeter ;
class kwxLinearMeter ;
class kwxBmpSwitcher ;
class kwxLCDDisplay;
class kwxLCDClock ;
class kwxLinearRegulator ;
class kwxAngularRegulator ;
class CClockThread;
class kwxBmpCheckBox ;

class MyPanel : public wxPanel
{
  public:
    MyPanel(wxWindow* parent, wxWindowID = -1,
            const wxPoint& pos   = wxDefaultPosition,
	        const wxSize&  size  = wxDefaultSize,
            const long     style = wxRAISED_BORDER);
	~MyPanel() ;

	void    OnMouse(wxMouseEvent& event);
	void	OnSlider(wxScrollEvent& event) ;
	void	OnBitButton(wxCommandEvent& event) ;
	void	OnToggle(wxCommandEvent& event) ;
	void	OnLinearRegChange(wxCommandEvent& event) ;
	void	OnAngRegChange(wxCommandEvent& event) ;

	kwxAngularMeter *angularmeter ;
	kwxLinearMeter *linearmeter ;
	kwxBmpSwitcher *bmpswitcher ;
	kwxLCDDisplay *mLCD ;
	kwxBmpCheckBox *focusbtn ;
	kwxLCDClock *mClock ;

	kwxLinearRegulator *linearreg ;
	kwxLCDDisplay *mLinearLCD ;

	kwxAngularRegulator *angularreg ;
	kwxLCDDisplay *mAngularLCD ;

	wxFont *font ;

	wxBitmap* onsel ;
	wxBitmap* offsel ;
	wxBitmap* on ;
	wxBitmap* off ;

	wxBitmap *falseImage ;
	wxBitmap *trueImage ;
	wxBitmap *testImage ;

	wxColour *border;
	wxColour *knob;


	//ANGULAR METER TEST
//	kwxAngularMeter *speedmeter ;
//	kwxAngularMeter *tempmeter ;

	int m_nVal;

  private:
    DECLARE_EVENT_TABLE()

}; // MyPanel

#endif // TestdcFrame_H
