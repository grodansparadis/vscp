///////////////////////////////////////////////////////////////////////////////
//
// Define a new application type
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
		Timer event handler
	*/
	void OnTimerRefresh( wxTimerEvent& event );




private:




	/*!
		Layout panel for primary dialog
	*/
    wxPanel *m_panel;


	/*!
		Status Text area
	*/
	wxStaticText *m_pStatusArea;

	/*!
		Log Text area
	*/
	wxStaticText *m_pLogArea;

	/*!
		Exit Button
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
		LIRC Interface
	*/
	CSysLogObj m_syslogobj;


    DECLARE_EVENT_TABLE()
};

enum 
{
    MenuAbout = 100,
    MenuQuit,
    MenuTest,
	MenuOptions,

	RefreshTimer = 200,


    Control_First = 1000,
    Control_Listbox,
    BtnRefresh,
    BtnExit,
	BtnPrint,
	Control_PictureFrame,
	GaugeProgress,
	BtnStatusText,
	BtnLogText,

};