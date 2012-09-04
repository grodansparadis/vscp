// SysLogListner.cpp : implementation file
//

#include "stdafx.h"
#include "PhoneLog.h"
#include "SysLogListner.h"
#include <mmsystem.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSysLogListner

CSysLogListner::CSysLogListner()
{
	MMRESULT rc;              // Return code.
    HMIXER hMixer;            // Mixer handle used in mixer API calls.
    //MIXERCONTROL mxc;         // Holds the mixer control data.
    MIXERLINE mxl;            // Holds the mixer line data.
    //MIXERLINECONTROLS mxlc;   // Obtains the mixer control.

    // Open the mixer. This opens the mixer with a deviceID of 0. If you
    // have a single sound card/mixer, then this will open it. If you have
    // multiple sound cards/mixers, the deviceIDs will be 0, 1, 2, and
    // so on.
    rc = mixerOpen( &hMixer, 0,0,0,0 );
    if ( MMSYSERR_NOERROR != rc ) {
        // Couldn't open the mixer.
    }

    // Initialize MIXERLINE structure.
    ZeroMemory( &mxl,sizeof( mxl ) );
    mxl.cbStruct = sizeof( mxl );

	m_pstrStatus = NULL;

}

CSysLogListner::~CSysLogListner()
{

}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CSysLogListner, CAsyncSocket)
	//{{AFX_MSG_MAP(CSysLogListner)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0


void CSysLogListner::Create( UINT nSocketPort, CString *str )  
{

	CString Message;
	DWORD Error;

	m_pstrStatus = str;

	if ( CAsyncSocket::Create( nSocketPort, SOCK_DGRAM, FD_READ, "0.0.0.0") ) {
		return;
	}
	
	Error = GetLastError();
	Message.Format("The socket for port %u was not created; error %u",
						nSocketPort, 
						Error );


	AfxMessageBox( Message );

}

/////////////////////////////////////////////////////////////////////////////
// CSysLogListner member functions

void CSysLogListner::OnReceive(int nErrorCode) 
{
	CUDPRecord UDPRecord;
	CString Message;
	char Buffer[512];	// Increase in size as needed
	DWORD Error;


	CAsyncSocket::OnReceive( nErrorCode );

	if ( nErrorCode ) {
		Message.Format( "OnReceive nErrorCode: %i", nErrorCode );
		AfxMessageBox( Message );
		return;
	}
	
	UDPRecord.m_Size = ReceiveFrom( Buffer, 
										sizeof( Buffer ), 
										UDPRecord.m_IPAddress,
										UDPRecord.m_Port );
	
	if ( !UDPRecord.m_Size || UDPRecord.m_Size == SOCKET_ERROR ) {
		Error = GetLastError();
		Message.Format( "ReceiveFrom error code: %u", Error );
		AfxMessageBox( Message );
		return;
	}

	Buffer[ UDPRecord.m_Size ] = 0x00;

	char *p;
	//char *pIncoming1;
	//char *pIncoming2;

	char  OriginatingCode[ MAX_PATH ];
	char  OriginatingNumber[ MAX_PATH ];
	char  ReceivingNumber[ MAX_PATH ];


	if ( ( NULL != strstr( Buffer, "isdn_net" ) ) &&
		( NULL != ( p = strstr( Buffer, "call from" ) ) ) ) {

		if ( NULL != ( p = strtok( p + 10, " " ) ) ) {
			
			strncpy( OriginatingCode, p, MAX_PATH );

		}

		p = strtok( NULL, " " ); // Remove middle stuff
		

		if ( NULL != ( p = strtok( NULL, " " ) ) ) {
		
			strncpy( ReceivingNumber, p, MAX_PATH );

		}

		if ( NULL != ( p = strtok( OriginatingCode, ", " ) ) ) {
			
			strncpy( OriginatingNumber, p, MAX_PATH );

		}

		CString str;
		str.Format( "Call from %s to %s", OriginatingNumber, ReceivingNumber );


		// http://www.hitta.se/ViewDetailsPink.aspx?SearchType=4&UserControlSearchBlock%3aWflWhite=1a1b&UserControlSearchBlock%3aWflPink=1a1b&UserControlSearchBlock%3aTextBoxWho=0657-413430&UserControlSearchBlock%3aTextBoxWhere=&Vkiid=MFlB6MutAO6lJv%2fAXJYmKQ%3d%3d&Vkid=2921010
		//http://www.hitta.se/ViewDetailsWhite.aspx?SearchType=4&UserControlSearchBlock%3aWflWhite=1a1b&UserControlSearchBlock%3aWflPink=1a1b&UserControlSearchBlock%3aTextBoxWho=0657-10530&UserControlSearchBlock%3aTextBoxWhere=&Vkiid=TFpfjb69ObpSp%2fkD9EwI7A%3d%3d&Vkid=10628101
		//http://www.hitta.se/ExternalSeach/Default.aspx?id="hittaForm"&SearchType=4&TextBoxWho="0657-413430"
		char sbuf[ 256 ];
		strcpy( sbuf, "http://www.hitta.se/ExternalSeach/Default.aspx?id=\"hittaForm\"&SearchType=4&TextBoxWho=" );
		strcat( sbuf, "\"0" );
		strcat( sbuf, OriginatingNumber );
		strcat( sbuf, "\"" );
		Execute( 1, 
			sbuf, 
			SW_SHOWNORMAL, 
			"", 
			"" );
/*
		Sleep( 1000 );

		INPUT input;
		input.type = INPUT_KEYBOARD;
			input.ki.wVk = VK_TAB;  
			input.ki.wScan = 1;
			input.ki.dwFlags = 0; 
			input.ki.dwExtraInfo = GetMessageExtraInfo();
			::SendInput( 1, &input, sizeof( input ) );

		input.type = INPUT_KEYBOARD;
			input.ki.wVk = VK_TAB;  
			input.ki.wScan = 1;
			input.ki.dwFlags = KEYEVENTF_KEYUP; 
			input.ki.dwExtraInfo = GetMessageExtraInfo();
			::SendInput( 1, &input, sizeof( input ) );
*/
/*
		input.type = INPUT_KEYBOARD;
			input.ki.wVk = VK_ENTER;  
			input.ki.wScan = 1;
			input.ki.dwFlags = 0; 
			input.ki.dwExtraInfo = GetMessageExtraInfo();
			::SendInput( 1, &input, sizeof( input ) );

		input.type = INPUT_KEYBOARD;
			input.ki.wVk = VK_ENTER;  
			input.ki.wScan = 1;
			input.ki.dwFlags = KEYEVENTF_KEYUP; 
			input.ki.dwExtraInfo = GetMessageExtraInfo();
			::SendInput( 1, &input, sizeof( input ) );
*/

		//VK_VOLUME_MUTE (0xAD) 
		// 0xAE = Down, ;
		// B3 Media Stop

		/*
		for ( int i= 0; i< 255; i++ ) {
			INPUT input;
			input.type = INPUT_KEYBOARD;
			input.ki.wVk = 0xAE; //VK_VOLUME_MUTE;
			input.ki.wScan = 1;
			input.ki.dwFlags = 0; 
			input.ki.dwExtraInfo = GetMessageExtraInfo();
			::SendInput( 1, &input, sizeof( input ) );

			input.type = INPUT_KEYBOARD;
			input.ki.wVk = 0xAE; //VK_VOLUME_MUTE (0xAD) 0xAE = Down, ;
			input.ki.wScan = 1;
			input.ki.dwFlags = KEYEVENTF_KEYUP; 
			input.ki.dwExtraInfo = GetMessageExtraInfo();
			::SendInput( 1, &input, sizeof( input ) );
		}
		*/

		INPUT input;
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = 0xAD; //VK_VOLUME_MUTE;
		input.ki.wScan = 1;
		input.ki.dwFlags = 0; 
		input.ki.dwExtraInfo = GetMessageExtraInfo();
		::SendInput( 1, &input, sizeof( input ) );

		input.type = INPUT_KEYBOARD;
		input.ki.wVk = 0xAD; //VK_VOLUME_MUTE (0xAD) 0xAE = Down, ;
		input.ki.wScan = 1;
		input.ki.dwFlags = KEYEVENTF_KEYUP; 
		input.ki.dwExtraInfo = GetMessageExtraInfo();
		::SendInput( 1, &input, sizeof( input ) );
	
		if ( NULL != m_pstrStatus ) {
			*m_pstrStatus = Buffer;
			//UpdateData( true );
		}

		// Try a global action with a "Post Message" command with message 
		// ID=0x319, WParam=0, LParam=0x80000. 
		// (That's WM_APPCOMMAND and APPCOMMAND_VOLUME_MUTE.) Should work 
		// anywhere in Win2000 and WinXP.
		//PostMessage( 0x319, 0, 0x80000 );

		//AfxMessageBox( str );
 
	}
	else {
		Beep( 1000, 100 );
	}
}


/////////////////////////////////////////////////////////////////////////////
// Execute
//
// "D:\Program Files\Adobe\Acrobat 6.0\Reader\AcroRd32.exe" /p /h "%1"
//

bool CSysLogListner::Execute( int Operation, 
							char *wxCommand, 
							int nShow, 
							char *wxParams, 
							char *wxDir )
{
	bool rv = false;
	SHELLEXECUTEINFO execinfo;
	
	execinfo.cbSize = sizeof(execinfo);
	execinfo.fMask = ( SEE_MASK_DOENVSUBST | SEE_MASK_FLAG_NO_UI );
	
	execinfo.hwnd = NULL; 

	// Set verb
	if ( Operation == 1 ) {
		// open
		execinfo.lpVerb = _T("open");
	}
	else if ( Operation == 2 ) {
		// explore
		execinfo.lpVerb = _T("explore");
	}
	else if ( Operation == 3 ) {
		// print
		execinfo.lpVerb = _T("print");
	}

	// Set File
	execinfo.lpFile = wxCommand;

	// Set Parameters
	execinfo.lpParameters = wxParams;

	// Set Parameters
	execinfo.lpDirectory = wxDir;

	// ShowWindow
	execinfo.nShow = nShow;

	if ( ShellExecuteEx( &execinfo ) ) {
		// Success
		rv = true;
		//writeLog( _T("Execute: Program körning utfördes utan fel."), 2 );
	}
	else {
		
		// Fail
		rv = false;
		 
		switch( GetLastError() ) {
			case ERROR_FILE_NOT_FOUND:
				//writeLog( _T("Execute: Den angivna filen kunde inte hittas.") );
				break;

			case ERROR_PATH_NOT_FOUND:
				//writeLog( _T("Execute: Den angivna vägen kunde inte hittas.") );
				break;

			case ERROR_DDE_FAIL:
				//writeLog( _T("Execute: DDE transaktionen kunde inte utföras.") );
				break;

			case ERROR_NO_ASSOCIATION:
				//writeLog( _T("Execute: Ingen applikation är kopplad till den angivna fil ändelsen.") );
				break;

			case ERROR_ACCESS_DENIED:
				//writeLog( _T("Execute: Har ej rättigheter att hantera filen.") );
				break;

			case ERROR_DLL_NOT_FOUND:
				//writeLog( _T("Execute: En av filerna som behövs för att köra denna applikation kunde inte hittas.") );
				break;

			case ERROR_CANCELLED:
				//writeLog( _T("Execute: Användaren avbröt operationen.") );
				break;
	
			case ERROR_NOT_ENOUGH_MEMORY:
				//writeLog( _T("Execute: Det finns inte tillräckligt med minne för att utföra operationen.") );
				break;

			case ERROR_SHARING_VIOLATION:
				//writeLog( _T("Execute: Ett delningsfel uppstod.") );
				break;

			default:
				 //writeLog( "Execute: Error: Ingen felkod.");		
				break;
		}
	}

	return rv;

}