/////////////////////////////////////////////////////////////////////////////
// Name:        LCDWindow.h
// Purpose:     wxIndustrialControls Library
// Author:      Marco Cavallini <m.cavallini AT koansoftware.com>
// Modified by: 
// Copyright:   (C)2004-2006 Copyright by Koan s.a.s. - www.koansoftware.com
// Licence:     KWIC License http://www.koansoftware.com/kwic/kwic-license.htm
/////////////////////////////////////////////////////////////////////////////
//
// Portions are from Class to display numbers like a Liquid Crystal Display, 
// by Tsolakos Stavros
/////////////////////////////////////////////////////////////////////////////
// Print a value on the display. The accepted characters are :
//	- All the hexadecimal digits ( 0-F )
//	- The characters :,.-roC and space
//	- The character ^ is displayed as an o, but displayed at the top. 
//	( The symbol of degrees )
//////////////////////////////////////////////////////////////////////////////

#ifndef __LCDWINDOW_H__
#define __LCDWINDOW_H__

#include "kprec.h"		//#include "wx/wxprec.h"

// This structure is used internally by the window.
struct wxDigitData
{
	char value;
	bool comma;
};


class kwxLCDDisplay : public wxWindow
{

protected:
	int mSegmentLen;
	int mSegmentWidth;
	int mSpace;

	int mNumberDigits;
	wxString mValue;
	wxColour mLightColour;
	wxColour mGrayColour;

protected:
// Internal functions used by the control.
// No time for documentation yet. Sorry.
	int GetDigitWidth( void );
	int GetDigitHeight( void );
	
	int GetBitmapWidth( void );
	int GetBitmapHeight( void );

	int DigitX( int digit );
	int DigitY( int digit );

	void DoDrawing( wxDC *dc );
	void DrawSegment( wxDC *dc, int digit, int segment, bool state );
	void DrawDigit( wxDC *dc, int digit, wxDigitData *data );
	void DrawTwoDots( wxDC *dc, int digit );

	unsigned char Decode( char c );
	
	void OnPaint( wxPaintEvent &event );
	void OnSize( wxSizeEvent &event );

public:
	kwxLCDDisplay( wxWindow *parent,
				 const wxPoint& pos = wxDefaultPosition ,
				 const wxSize& size = wxDefaultSize);

	virtual ~kwxLCDDisplay();

// Sets the desired number of digits our 7seg display.
// The default is 4.
	void SetNumberDigits( int ndigits );

// Gets the current number of digits.
	int GetNumberDigits( void );

/* Print a value on the display. The accepted characters are :
   - All the hexadecimal digits ( 0-F )
   - The characters :,.-roC and space
   - The character ^ is displayed as an o, but displayed at the top. ( The symbol of degrees )
*/
	void SetValue( wxString value );

// Gets the value currently displayed.
	wxString GetValue( void );

// Sets the colour which the lighted parts of the display should have.
	void SetLightColour( wxColour c );

// Sets the colour that the grayed parts of the display will have.
	void SetGrayColour( wxColour c );

	wxColour GetLightColour( void );
	wxColour GetGrayColour( void );

// Returns the amount of digits required to display the current value.
// The amount of digits needed is not necessarily the length of the string.
// For example, a dot does not require an extra space in order to be displayed.
	int GetDigitsNeeded( wxString value );

DECLARE_EVENT_TABLE()
};

#endif // __LCDWINDOW_H__
