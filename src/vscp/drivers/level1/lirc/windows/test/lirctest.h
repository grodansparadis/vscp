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
		OnAbout

		Show about box
	*/
	void OnAbout( wxCommandEvent& event );


	/*!
		
	*/
    void OnListboxSelect( wxCommandEvent& event );


	/*!
		
	*/
	void OnListboxDblClick( wxCommandEvent& WXUNUSED( event ) );

	/*!
		Perform one update cycle
	*/
	void doUpdate( void );

	/*!
		Refresh the list manually
	*/
    void OnButtonRefresh( wxCommandEvent& event );


	/*!
		Timer event handler
	*/
	void OnTimerRefresh( wxTimerEvent& event );

	/*!
		saveRegistryData

	*/
	void saveRegistryData( void );

	/*!
		loadRegistryData
	*/
	void loadRegistryData( void );

	/*!
		Download a file to the cache
	*/
	bool downLoadFile( const char* path );

	/*!
		Execute external command
	*/
	bool Execute( int Operation, wxString wxCommand, int nShow, 
					wxString wxParams, wxString wxDir );


private:

	/*!
		Get folder from the server foler string
	*/
	void getFolder( wxString& wxstr );


	/*!
		Remove invalid chars from stream
	*/
	void fixInvalidChars( char * buf, long cnt );

	/*!
		Fix terminating slash
	*/
	void fixTermSlash( wxString &wststr );

	/*!
		Make filename from URL
	*/
	void makeCacheFileName( wxString &wxURL );


	/*!
		Change colors of listbox rows
	*/
    void AdjustColour( size_t index );

	/*!
		Log data to logfile (if it should be done)

		@param str String to send to log file
		@param nLogLevel 0 - Nornal Log data, 5 - Very detaild
	*/
	void writeLog( const char* str, unsigned char nLogLevel = 0 );


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
	CLircObj m_lirc;


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