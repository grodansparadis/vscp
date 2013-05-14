///////////////////////////////////////////////////////////////////////////////
//
// Define a new application type
//
// Copyright (C) 2002 Ake Hedman akhe@eurosource.se 
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//



#define WEB_COMMAND					"dopdworklist.asp?operation=2"
#define WEB_COMMAND_MARK_HANDLED	"dopdworklist.asp?operation=3&oid="


class CheckListBoxApp: public wxApp
{
public:
    bool OnInit();
};

///////////////////////////////////////////////////////////////////////////////
// CheckListBoxFrame
//
// Define a new frame type
//

class appFrame : public wxFrame
{
public:
    // ctor & dtor
    appFrame( wxFrame *frame, 
						const wxChar *title,
						int x, int y, int w, int h);
    virtual ~appFrame();

    // notifications
	 
	/*!
		OnQuit
		
		Quit the application
	*/
    void OnQuit( wxCommandEvent& event );
    

	/*!
		OnAbout

		Show about box
	*/
	void OnAbout( wxCommandEvent& event );


	/*!
		Perform one update cycle
	*/
	void doUpdate( void );

	/*!
		Raise dimmer value
	*/
    void OnButtonUp( wxCommandEvent& event );

	/*!
		Send a dimmer response
	*/
	void sendDimResponse( void );

	/*!
		Lower dimmer value button
	*/
    void OnButtonDown( wxCommandEvent& event );

	/*!
		Lights on button
	*/
	void OnButtonOn( wxCommandEvent& WXUNUSED( event ) );

	/*!
		Lights off button
	*/
	void OnButtonOff( wxCommandEvent& WXUNUSED( event ) );

	/*!
		Send respons for On/Off event
	*/
	void sendOnOffResponse( bool bOn );

	/*!
		Timer event handler
	*/
	void OnTimerRefresh( wxTimerEvent& event );

	/*!
		The settings dilaog handler.
	*/
	void OnDialogSettings( wxCommandEvent& event );

	/*!
		Get MAC address for machines ethernet adapter.
	*/
	bool getMacAddress( void );

public:

	/*!
		Node address
	*/
	unsigned char m_nodeAddr[ 16 ];

	/*!
		Zone lamp belongs to
	*/
	unsigned char m_zone;

private:


	/*!
		Layout panel for primary dialog
	*/
    wxPanel *m_panel;


	/*!
		Up button
	*/
	wxButton *m_buttonUp;

	/*!
		Down button
	*/
	wxButton *m_buttonDown;

	/*!
		On button
	*/
	wxButton *m_buttonOn;

	/*!
		Off button
	*/
	wxButton *m_buttonOff;

	/*!
		Exit button
	*/
	wxButton *m_buttonExit;


	/*!
		Timer event 
	*/
	wxTimer m_timer;

	/*!
		One second counter
	*/
	unsigned long m_secondCounter;

	/*!
		Holder for the lamp picture
	*/
	wxStaticBitmap *m_plampFrame;

	/*!
		Picture of lamp
	*/
	wxBitmap *m_pPicLamp;


	/*!
		Gauge for dimmer procentage
	*/
	wxGauge *m_pGauge;


	/*!
		VSCP interface
	*/
	VscpTcpIf m_vscpif;


    DECLARE_EVENT_TABLE()
};

enum 
{
    MenuAbout = 100,
    MenuQuit,
	MenuOptions,

	RefreshTimer = 200,


    Control_First = 1000,
    Control_LampFrame,
	Control_Gauge,
    BtnUp,
	BtnDown,
	BtnOn,
	BtnOff,
    BtnExit,

};