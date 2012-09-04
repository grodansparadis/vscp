///////////////////////////////////////////////////////////////////////////////
//
// Define a new application type
//
// Copyright (C) 2002-2011 Ake Hedman akhe@eurosource.se 
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

#include "../../../common/vscptcpif.h"

#define WEB_COMMAND					"dopdworklist.asp?operation=2"
#define WEB_COMMAND_MARK_HANDLED	"dopdworklist.asp?operation=3&oid="


class SimpleButtonApp: public wxApp
{
public:
    bool OnInit();

	/*!
		Helper to get data values in decimal and
		hexadecimal
	*/
	unsigned long getDataValue( const char *szData );
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
    void OnButton( wxCommandEvent& event );


	/*!
		Timer event handler
	*/
	void OnTimerRefresh( wxTimerEvent& event );

	/*!
		Dialog settings
	*/
	void OnDialogSettings( wxCommandEvent& event );

	/*!
		Set address from MAC address
	*/
	bool getMacAddress( void );

public:

	/*!
		Node address
	*/
	unsigned char m_nodeAddr[ 16 ];

	/*!
		Class Code
	*/
	unsigned char m_class;

	/*!
		type Code
	*/
	unsigned char m_type;

	/*!
		Message data
	*/
	unsigned char m_msgdata[ 8 ];

	/*!
		Size of data
	*/
	unsigned char m_datasize;

	/*!
		Button type
	*/
	boolean m_bToggle;


private:


	/*!
		Button state
	*/
	boolean m_state;

	/*!
		Layout panel for primary dialog
	*/
    wxPanel *m_panel;


	/*!
		Command button
	*/
	//wxToggleButton *m_button;
	wxButton *m_button;


	/*!
		Timer event 
	*/
	wxTimer m_timer;

	/*!
		One second counter
	*/
	unsigned long m_secondCounter;


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
    BtnCmd,
};