// NTService.cpp
//
// Implementation of CNTService

#ifdef __GNUG__
    //#pragma implementation
#endif


#include <NTService.h>
#include <version.h>  // A little ugly hack

//
// Global variables.
//

HANDLE  ghStopEvent;
HANDLE  ghThreads[ MAX_THREADS ];

// static variables
CNTService* CNTService::m_pThis = NULL;


///////////////////////////////////////////////////////////////////////////////
// CNTService
//

CNTService::CNTService( LPCTSTR szServiceName )
{
    // copy the address of the current object so we can access it from
    // the static member callback functions.
    // WARNING: This limits the application to only one CNTService object.
    m_pThis = this;

    // Set the default service name and version
    wcscpy( m_szServiceName, szServiceName );
    m_iMajorVersion = VSCPD_MAJOR_VERSION;
    m_iMinorVersion = VSCPD_MINOR_VERSION;
    m_iSubMinorVersion = VSCPD_SUB_VERSION;
    m_hEventSource = NULL;

    // No threads running yet
    for (int i=0; i<MAX_THREADS; i++ ) {
        ghThreads[ i ] = NULL;
    }

    // set up the initial service status
    m_hServiceStatus = NULL;
    m_Status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_Status.dwCurrentState = SERVICE_STOPPED;
    m_Status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    m_Status.dwWin32ExitCode = 0;
    m_Status.dwServiceSpecificExitCode = 0;
    m_Status.dwCheckPoint = 0;
    m_Status.dwWaitHint = 0;
    m_bIsRunning = false;

    // Create the event to signal the service to stop.
    ghStopEvent = CreateEvent( NULL, true, false, NULL );
}

///////////////////////////////////////////////////////////////////////////////
// ~CNTService
//


CNTService::~CNTService()
{
    DebugMsg(_("CNTService::~CNTService()"));
    if ( m_hEventSource ) {
        ::DeregisterEventSource( m_hEventSource );
    }
}

///////////////////////////////////////////////////////////////////////////////
// ParseStandardArgs
//


BOOL CNTService::ParseStandardArgs(int argc, char* argv[])
{
    // See if we have any command line args we recognise
    if ( argc <= 1 ) {
        return false;
    }

    // * * * -v Check version * * *
    if ( _stricmp(argv[1], "-v") == 0 ) {
        // Spit out version info
        printf("%s Version %d.%d.%d\n",
               m_szServiceName, m_iMajorVersion, m_iMinorVersion, m_iSubMinorVersion);
        printf("The service is %s installed\n",
               IsInstalled() ? "currently" : "not");
        return true; // say we processed the argument

    }
    // * * * -i Install * * *
    else if (_stricmp(argv[1], "-i") == 0) {
        // Request to install.
        if ( IsInstalled() ) {
            printf("%s is already installed\n", m_szServiceName);
        }
        else  {
            // Try and install the copy that's running
            if ( Install() ) {
                printf("%s installed\n", m_szServiceName);
            }
            else {
                printf("%s failed to install. Error %d\n", m_szServiceName, GetLastError());
            }
        }

        return true; // say we processed the argument

    }
    // * * * -u Uninstall * * *
    else if ( _stricmp(argv[1], "-u" ) == 0) {

        // Request to uninstall.
        if ( !IsInstalled() ) {
            printf("%s is not installed\n", m_szServiceName);
        }
        else {
            // Try and remove the copy that's installed
            if ( Uninstall() ) {
                // Get the executable file path
                WCHAR szFilePath[_MAX_PATH];
                ::GetModuleFileName( NULL, szFilePath, sizeof(szFilePath));
                printf("%s removed. (You must delete the file (%s) yourself.)\n",
                       m_szServiceName, szFilePath);
            }
            else {
                printf("Could not remove %s. Error %d\n", m_szServiceName, GetLastError());
            }
        }

        return true; // say we processed the argument
    }
    // * * * -? Show available commands * * *
    else if ( _stricmp(argv[1], "-?") == 0 ) {
        showHelp();
        return true;
    }
    // * * * -? Show available commands * * *
    else if ( _stricmp(argv[1], "-h") == 0 ) {
        showHelp();
        return true;
    }

    // Don't recognise the args
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// showHelp
//
//

void CNTService::showHelp( void )
{
    printf("\nVSCP Service - (C) 2000-2015 the VSCP project\n");
    printf("===============================================\n\n");
    printf("Command line switches.\n");
    printf("----------------------\n");
    printf("\"no switch\"\tStart the service.\n");
    printf(" -i\t\tInstall the service.\n");
    printf(" -u\t\tUninstall the service.\n");
    printf(" -v\t\tDisplay service information.\n");
    printf(" -?\t\tDisplay this help information.\n");
    printf(" -h\t\tDisplay this help information.\n\n\n");
}


///////////////////////////////////////////////////////////////////////////////
// Install/uninstall routines
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// IsInstalled
//

BOOL CNTService::IsInstalled()
{
    BOOL bResult = FALSE;

    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager( NULL, // local machine
                                      NULL, // ServicesActive database
                                      SC_MANAGER_ALL_ACCESS); // full access
    if ( hSCM ) {
        // Try to open the service
        SC_HANDLE hService = ::OpenService( hSCM,
                                           m_szServiceName,
                                           SERVICE_QUERY_CONFIG);
        if ( hService ) {
            bResult = TRUE;
            ::CloseServiceHandle( hService );
        }

        ::CloseServiceHandle( hSCM );
    }

    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Install
//
//

BOOL CNTService::Install()
{
    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager( NULL,						// local machine
                                      NULL,						// ServicesActive database
                                      SC_MANAGER_ALL_ACCESS);	// full access
    if ( !hSCM ) {
        return FALSE;
    }

    // Get the executable file path
    WCHAR szFilePath[_MAX_PATH];
    ::GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));

    // Create the service
    SC_HANDLE hService = ::CreateService( hSCM,
                                          m_szServiceName,
                                          m_szServiceName,
                                          SERVICE_ALL_ACCESS,
                                          SERVICE_WIN32_OWN_PROCESS,
                                          SERVICE_DEMAND_START,	// start condition
                                          SERVICE_ERROR_NORMAL,
                                          szFilePath,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL );
    if ( !hService ) {
        ::CloseServiceHandle(hSCM);
        return FALSE;
    }

    // make registry entries to support logging messages
    // Add the source name as a subkey under the Application
    // key in the EventLog service portion of the registry.
    WCHAR szKey[256];
    HKEY hKey = NULL;
    wcscpy( szKey, _("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\") );
    wcscpy( szKey, m_szServiceName );
    if ( ::RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &hKey) != ERROR_SUCCESS ) {
        ::CloseServiceHandle( hService );
        ::CloseServiceHandle( hSCM );
        return FALSE;
    }

    // Add the Event ID message-file name to the 'EventMessageFile' subkey.
    ::RegSetValueEx( hKey,
                        _("EventMessageFile"),
                        0,
                        REG_EXPAND_SZ,
                        (CONST BYTE*)szFilePath,
                        wcslen(szFilePath) + 1 );

    // Set the supported types flags.
    DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    ::RegSetValueEx( hKey,
                        _("TypesSupported"),
                        0,
                        REG_DWORD,
                        (CONST BYTE*)&dwData,
                        sizeof(DWORD) );
    ::RegCloseKey(hKey);

    LogEvent( EVENTLOG_INFORMATION_TYPE, EVMSG_INSTALLED, m_szServiceName );

    // tidy up
    ::CloseServiceHandle( hService );
    ::CloseServiceHandle( hSCM );

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Uninstall
//
//

BOOL CNTService::Uninstall()
{
    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager( NULL, // local machine
                                        NULL, // ServicesActive database
                                        SC_MANAGER_ALL_ACCESS ); // full access
    if (!hSCM) return FALSE;

    BOOL bResult = FALSE;
    SC_HANDLE hService = ::OpenService( hSCM,
                                        m_szServiceName,
                                        DELETE );
    if ( hService ) {
        if ( ::DeleteService( hService ) ) {
            LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_REMOVED, m_szServiceName);
            bResult = TRUE;
        }
        else {
            LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_NOTREMOVED, m_szServiceName);
        }
        ::CloseServiceHandle(hService);
    }

    ::CloseServiceHandle(hSCM);
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Logging functions
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// LogEvent
//
// This function makes an entry into the application event log
//

void CNTService::LogEvent(WORD wType, DWORD dwID,
                          LPCWSTR pszS1,
                          LPCWSTR pszS2,
                          LPCWSTR pszS3)
{
    LPCWSTR ps[3];
    ps[0] = pszS1;
    ps[1] = pszS2;
    ps[2] = pszS3;

    int iStr = 0;
    for ( int i = 0; i < 3; i++ ) {
        if ( ps[i] != NULL ) {
            iStr++;
        }
    }

    // Check the event source has been registered and if
    // not then register it now
    if ( !m_hEventSource ) {
        m_hEventSource = ::RegisterEventSource( NULL,				// local machine
                                                m_szServiceName);	// source name
    }

    if ( m_hEventSource ) {
        ::ReportEvent(m_hEventSource,
                      wType,
                      0,
                      dwID,
                      NULL, // sid
                      iStr,
                      0,
                      ps,
                      NULL);
    }
}

///////////////////////////////////////////////////////////////////////////////
// StartService
//
// Service startup and registration
//

BOOL CNTService::StartService()
{
    SERVICE_TABLE_ENTRY st[] =
    {
        { m_szServiceName, ServiceMain },
        { NULL, NULL }
    };

    DebugMsg(_("Calling StartServiceCtrlDispatcher()"));
    BOOL b = ::StartServiceCtrlDispatcher( st );
    DebugMsg(_("Returned from StartServiceCtrlDispatcher()"));

    return b;
}

///////////////////////////////////////////////////////////////////////////////
// ServiceMain
//
// static member function (callback)
//

void CNTService::ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // Get a pointer to the C++ object
    CNTService* pService = m_pThis;

    pService->DebugMsg(_("Entering CNTService::ServiceMain()"));

    // Register the control request handler
    pService->m_Status.dwCurrentState = SERVICE_START_PENDING;
    pService->m_hServiceStatus = RegisterServiceCtrlHandler(pService->m_szServiceName,
                                                           Handler);
    if ( pService->m_hServiceStatus == NULL ) {
        pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_CTRLHANDLERNOTINSTALLED);
        return;
    }

    // Start the initialisation
    if ( pService->Initialize() )  {
        // Do the real work.
        // When the Run function returns, the service has stopped.
        pService->m_bIsRunning = true;
        pService->m_Status.dwWin32ExitCode = 0;
        pService->m_Status.dwCheckPoint = 0;
        pService->m_Status.dwWaitHint = 0;
        pService->Run();
    }


    // Now wait for threads to exit.
    DWORD dwWaitRes;
    if ( ( dwWaitRes = WaitForMultipleObjects( MAX_THREADS, ghThreads, TRUE, 3000 ) )
                    == WAIT_OBJECT_0 ) {
        // This is OK! Nothing more to do.....
        ;
    }
    else if ( ( dwWaitRes == WAIT_FAILED ) || ( dwWaitRes == WAIT_ABANDONED ) ) {
        // We failed to kill the threads we have to
        // abort anyway but we tell the world that we do so....
        pService->DebugMsg(_("Failed to terminate all threads in CNTService::ServiceMain()"));
    }

    // close the event handle and the thread handle
    CloseHandle( ghStopEvent );
    for (int i=0; i<MAX_THREADS; i++ ) {
        if ( ghThreads[i] ) {
            CloseHandle( ghThreads[i] );
        }
    }

    // Tell the service manager we are stopped
    pService->SetStatus( SERVICE_STOPPED );

    pService->DebugMsg(_("Leaving CNTService::ServiceMain()"));
}

///////////////////////////////////////////////////////////////////////////////
// status functions
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// SetStatus
//
//

void CNTService::SetStatus(DWORD dwState)
{
    DebugMsg(_("CNTService::SetStatus(%lu, %lu)"), m_hServiceStatus, dwState);
    m_Status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_Status);
}

///////////////////////////////////////////////////////////////////////////////
// Initialize
//
// Service initialization
//

BOOL CNTService::Initialize()
{
    DebugMsg(_("Entering CNTService::Initialize()"));

    // Start the initialization
    SetStatus(SERVICE_START_PENDING);

    // Perform the actual initialization
    BOOL bResult = OnInit();

    // Set final state
    m_Status.dwWin32ExitCode = GetLastError();
    m_Status.dwCheckPoint = 0;
    m_Status.dwWaitHint = 0;
    if ( !bResult ) {
        LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_FAILEDINIT);
        SetStatus(SERVICE_STOPPED);
        return FALSE;
    }

    LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_STARTED);
    SetStatus(SERVICE_RUNNING);

    DebugMsg(_("Leaving CNTService::Initialize()"));
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Run
//
// main function to do the real work of the service
//
// This function performs the main work of the service.
// When this function returns the service has stopped.
//

void CNTService::Run()
{
    DebugMsg(_("Entering CNTService::Run()"));

    while ( m_bIsRunning ) {
        DebugMsg(_("Sleeping..."));
        Sleep(5000);
    }

    // nothing more to do
    DebugMsg(_("Leaving CNTService::Run()"));
}

///////////////////////////////////////////////////////////////////////////////
// Handler
//
// Control request handlers
//
// static member function (callback) to handle commands from the
// service control manager
//

void CNTService::Handler( DWORD dwOpcode )
{
    // Get a pointer to the object
    CNTService* pService = m_pThis;

    pService->DebugMsg(_("CNTService::Handler(%lu)"), dwOpcode);

    switch ( dwOpcode ) {

    case SERVICE_CONTROL_STOP:			// 1
        pService->SetStatus(SERVICE_STOP_PENDING);
        pService->OnStop();
        pService->m_bIsRunning = false;
        pService->LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_STOPPED);
        break;

    case SERVICE_CONTROL_PAUSE:			// 2
        pService->OnPause();
        break;

    case SERVICE_CONTROL_CONTINUE:		// 3
        pService->OnContinue();
        break;

    case SERVICE_CONTROL_INTERROGATE:	// 4
        pService->OnInterrogate();
        break;

    case SERVICE_CONTROL_SHUTDOWN:		// 5
        pService->OnShutdown();
        break;

    default:
        if ( dwOpcode >= SERVICE_CONTROL_USER ) {
            if (!pService->OnUserControl( dwOpcode ) ) {
                pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_BADREQUEST);
            }
        }
        else {
            pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_BADREQUEST);
        }
        break;
    }

    // Report current status
    pService->DebugMsg(_("Updating status (%lu, %lu)"),
                       pService->m_hServiceStatus,
                       pService->m_Status.dwCurrentState);
    ::SetServiceStatus(pService->m_hServiceStatus, &pService->m_Status);
}

///////////////////////////////////////////////////////////////////////////////
// OnInit
//

BOOL CNTService::OnInit()
{
    DebugMsg(_("CNTService::OnInit()"));
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// OnStop
//

void CNTService::OnStop()
{
    DebugMsg(_("CNTService::OnStop()"));
}

///////////////////////////////////////////////////////////////////////////////
// OnInterrogate
//

void CNTService::OnInterrogate()
{
    DebugMsg(_("CNTService::OnInterrogate()"));
}

///////////////////////////////////////////////////////////////////////////////
// OnPause
//
//

void CNTService::OnPause()
{
    DebugMsg(_("CNTService::OnPause()"));
}

///////////////////////////////////////////////////////////////////////////////
// OnContinue
//
//

void CNTService::OnContinue()
{
    DebugMsg(_("CNTService::OnContinue()"));
}

///////////////////////////////////////////////////////////////////////////////
// OnShutdown
//
//

void CNTService::OnShutdown()
{
    DebugMsg(_("CNTService::OnShutdown()"));
}

///////////////////////////////////////////////////////////////////////////////
// OnUserControl
//
//

BOOL CNTService::OnUserControl( DWORD dwOpcode )
{
    DebugMsg(_("CNTService::OnUserControl(%8.8lXH)"), dwOpcode);
    return FALSE; // say not handled
}


///////////////////////////////////////////////////////////////////////////////
// Debugging support
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// DebugMsg
//
//

void CNTService::DebugMsg(LPCTSTR pszFormat, ...)
{
    WCHAR buf[1024];
    swprintf( buf, _("[%s](%lu): "), m_szServiceName, GetCurrentThreadId() );
    va_list arglist;
    va_start( arglist, pszFormat );
    _vswprintf( &buf[wcslen(buf)], pszFormat, arglist );
    va_end( arglist );
    wcscat( buf, _("\n") );
    OutputDebugString( buf );
}
