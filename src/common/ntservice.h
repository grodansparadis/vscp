// ntservice.h
//
// Definitions for CNTService
//

#ifndef _NTSERVICE_H_
#define _NTSERVICE_H_

#include "ntservmsg.h" // Event message ids

/// Service code for user codes
#define SERVICE_CONTROL_USER 128

/// Mamimum number of concurrent threads
#define MAX_THREADS 256

/**
    Windows NT Service Base class

    Derive from this base class to implement a Windows NT
    userspace service. The model can use many (MAX_THREADS) separate threads
    of excecution. An active thread must have its handle in the hThreads
    list. A dead/killed thread should have NULL for its handle in the same
    list.

    Put initialization code in OnInit.
    The application loop is placed in Run.

*/

class CNTService
{

public:

    /// Constructor
    CNTService(LPCTSTR szServiceName);
    
    /// Destructor
    virtual ~CNTService();
    
    /**
        Parse command line argument.

        @param argc number of arguments.
        @param argv vector with string arguments.

        @return Returns TRUE if it found an arg it recognised, FALSE if not
                Note: processing some arguments causes output to stdout to 
                be generated.
    */
    BOOL ParseStandardArgs(int argc, char* argv[]);

    /** 
        Show help message. 
    
        Gives information about valid command line switches.
    */
    void showHelp( void );

    /**
        Check if service is installed.

        @return TRUE if successfull false otherwise.
    */
    BOOL IsInstalled();

    /**
        Install service.

        @return TRUE if successfull false otherwise..
    */
    BOOL Install();

    /**
        Uninstall service

        @return TRUE if successfull false otherwise..
    */
    BOOL Uninstall();

    /**
        Log event to NT application message log.

        @param wType
        @param dwID
        @param pszS1
        @param pszS2
        @param pszS3
    */
    void LogEvent(WORD wType, DWORD dwID,
                        LPCWSTR pszS1 = NULL,
                        LPCWSTR pszS2 = NULL,
                        LPCWSTR pszS3 = NULL);

    /**
        Start the service.

        @return TRUE if successfull false otherwise..
    */
    BOOL StartService();

    /**
        Set status for service.
    */
    void SetStatus(DWORD dwState);

    /**
        Initialize service

        @return TRUE if successfull false otherwise..
    */
    BOOL Initialize();

    /**
        Execution loop for service in its active state.
    */
    virtual void Run( void );

    /**
        Called when the service is first initialized.

        @return TRUE if successfull false otherwise.
    */
    virtual BOOL OnInit( void );

    /**
        Called when the service control manager wants to stop the service.

    */
    virtual void OnStop(void );

    /**
        Called when the service is interrogated.

    */
    virtual void OnInterrogate( void );

    /**
        Called when the service is paused.

    */
    virtual void OnPause( void );

    /**
        Called when the service is continued.

    */
    virtual void OnContinue( void );

    /**
        Called when the service is shut down.

    */
    virtual void OnShutdown( void );

    /**
        Called when the service gets a user control message.

        @return TRUE if successfull false otherwise.

    */
    virtual BOOL OnUserControl( DWORD dwOpcode );

    /**
        Send debug message.

    */
    void DebugMsg(LPCTSTR pszFormat, ...);
    
    
    /**@name static member functions */
    //@{
    /// ServiceMain entry.
    static void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);

    /// Handler entry.
    static void WINAPI Handler(DWORD dwOpcode);
    //@}

public:
    // data members

    /// Name of service.
    WCHAR m_szServiceName[64];

    /// Major version number.
    int m_iMajorVersion;

    /// Minor version number.
    int m_iMinorVersion;

    /// Sub Minor version number
    int m_iSubMinorVersion;

    /// Handle to service status.
    SERVICE_STATUS_HANDLE m_hServiceStatus;

    /// The current service status value.
    SERVICE_STATUS m_Status;

    /// State variable - True if service is running.
    bool m_bIsRunning;

    /**@name static member functions */
    //@{ static data

    /// nasty hack to get object ptr
    static CNTService* m_pThis; 
    //@}

private:

    /// Handle to event source
    HANDLE m_hEventSource;

};

#endif // _NTSERVICE_H_
