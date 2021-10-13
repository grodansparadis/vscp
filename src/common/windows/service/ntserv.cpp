/*  
Module : NTSERV.CPP
Purpose: Implementation for an MFC class framework which encapsulate the whole area of Windows services
Created: PJN / 14-07-1997
History: PJN / 17-05-1999 1. Fixed a warning when compiled with VC 6.
                          2. Fixed a bug in CNTEventLogSource::Report  
         PJN / 05-09-1999 1. Addition of more ASSERT's statements to aid in debugging.
         PJN / 02-10-1999 1. Addition of GetProfileStringArray, WriteProfileStringArray,
                             GetProfileBinary and WriteProfileBinary methods to the 
                             CNTService class.
                          2. Renamed some module names
         PJN / 10-10-1999 1. Added support for the description field which services can
                          have on Windows 2000.
                          2. Added accessor functions for the service name, friendly name
                          and the description text.
         PJN / 10-04-2000 1. Fixed a bug with the calculation of buffer sizes when compiled
                          for UNICODE in CNTEventLogSource::SetStringArrayIntoRegistry
         PJN / 21-05-2000 1. Added 2 new overridden versions of CNTEventLogSource::Report
                          2. Tweaked and set default parameters in CNTService::ReportStatusToSCM
                          following review of the book "Professional NT Services"
                          3. Disallowed accepting any controls when the service is currently 
                          processing the same control in ReportStatusToSCM
                          4. Change the ordering of parameters in ReportStatusToSCM. You should review 
                          your code to ensure it is correct if you have existing code
                          5. Class framework now by default uses a DLL of the same name for
                          the message file which the NT Event log will look to for mc string entries.
                          This means that you should review your code to ensure it is correct if you 
                          have existing code
                          6. CNTEventLogSource::Report now does not bother automatically creating
                          a SID as this should be used for services which do impersonation for clients
                          7. Changed the constructor of CNTEventLog which now takes the values it
                          needs for calling Register
                          8. CNTEventLog::Report methods are now no longer const because of change 7 above.
                          9. SCM database is now locked during installs and uninstalls of the service
                          10. Now supports all the new functionality provided for services under Windows 2000
                          while preserving binary compatability with NT 3.x and NT 4 services.
                          11. Framework now requires the presence of the January 2000 platform sdk or later. 
                          This is because of point 10 above.
                          12. Changed the prototype of the existing CNTScmService::QueryStatus function. 
                          13. Made the typedefs for the enumeration functions private to their respective
                          classes instead of polluting the global name space
         PJN / 23-05-2000 1. Added a "CNTService::WaitForStop" method
                          2. Uninstall of service now asks the service to stop and then waits for it to stop.
         PJN / 19-06-2000 1. Added an ASSERT to ReportStatusToSCM for status handle == 0
                          2. Fixed a number of other logic problems in ReportStatusToSCM
                          3. Removed an unnecessary ASSERT in CNTEventLogSource::Report
                          4. Sample service app now uses sound card default sound instead of PC speaker sound
         PJN / 20-06-2000 1. Minor change to a TRACE statement in RegisterCtrlHandler
                          2. RegisterCtrlHandler is called automatically for you now, no need to call it in
                          your derived version of ServiceMain
                          3. Added a BOOL CNTEventLogSource::Report(WORD wType, LPCTSTR lspszString) method
                          4. Fixed a bug where the sample service was not reporting its shutdown status to 
                          the SCM correctly.
         PJN / 14-12-2000 1. Fixed a bug in CNTService::Uninstall() which was causing it to fail sometimes
                          2. Made all the member variables of CNTService class public but put them down below
                          the implementation ala the way MFC classes are used declared.
                          3. Added a new overridden version of CNTEventLogSource::Report 
         PJN / 08-01-2001 1) Now includes copyright message in the source code and documentation.
                          2) CNTEventLogSource::Report now allows you to specify whether error codes should be
                          displayed in Hex or as decimal
                          3) Sample service sometimes did not display the stop event. This has been remedied by
                          reporting to the event log before we report to the SCM.
                          4) Framework has now been made independent of the need for the latest Platform SDK
                          header files. You can now compile the code in VC 5 without the Plaform SDK having to
                          be installed.
         PJN / 17-05-2001 1) Thread protected code to CNTEventLogSource::Report
         PJN / 27-05-2001 1) Added methods to framework to support the new command line options -start, -pause, 
                          -continue and -stop. Thanks to Stefan Niemeyer for this great addition.
                          2) Fixed a small issue in the testing of the CEventLogSource class in app.cpp
         PJN / 15-07-2001 1) Fixed a problem when calling CreateService with pathnames which contain embedded spaces
         PJN / 18-07-2001 1) Minor update to the sample code demonstrating the class framework.
                          2) Due to VC 6 shipping with a Winsvc.h (The main NT service header file) which includes
                          some but not all of the defines which the class requires you need to define a few preprocessor
                          define to get the code to compile cleanly for your particular setup. Use the following matrix:
                          
                                Platform SDK Installed?      NO                                                                        YES 

                          Compiler

                          Visual C++ 5.0                     "NTSERV_DO_NEW_WINSVC_DEFINES"                                            Nothing extra to define
                          Visual C++ 6.0 and later           "NTSERV_DO_NEW_WINSVC_DEFINES" & "NTSERV_EXCLUDE_VC6_WINSVC_DEFINES"      Nothing extra to define


                          The VC 5 sample app workspace included defines build configurations for the case where the Platform SDK is present and is not.
                          If you use this workspace in VC 6 or later, make sure you define the addional "NTSERV_EXCLUDE_VC6_WINSVC_DEFINES" define for the
                          Build configurations for "Missing Platform SDK". If you still cannot get the code to compile or you could not understand the 
                          explanation above, then just drop me an email.
         PJN / 09-08-2001 1) Updated SetStringArrayIntoRegistry and GetStringArrayIntoRegistry to be consistent with 
                          my other class CHKLM v1.11
         PJN / 27-11-2001 1) Fixed an issue where the class fails to read any key that is readonly. 
                          This is a problem since Win2000 defaults access to HKLM for "normal" 
                          users to readonly. Thanks to Hans-Georg Ulrich for spotting this problem
         PJN / 26-08-2002 1) Made more functions virtual to allow greater end user customisation.
                          2) All useful attributes in call to CreateService in CNTService::Install can now be modifed without having
                          to derive a custom version of CNTService::Install in your derived class.
                          3) A boolean value is now provided to enable / disable event log integration
         PJN / 28-08-2002 1) Reviewed all TRACE statements in the code for correctness
                          2) Added a parameter to CNTService::Install to allow the time to wait for the service to stop to be specified
                          3) Added a CString parameter to the CNTService::Install and Uninstall methods to return a descriptive
                          errors to callers.
                          4) Addition of a new boolean variable to CNTService called m_bConsole to allow service to decide whether it 
                          should use GUI or Console calls to display Install / Uninstall status messages
                          5) CNTService::ProcessShellCommand now displays error messages to the console or via a Message box when errors
                          occur in install, uninstalling, starting, stoping, resuming and pausing the service.
         PJN / 20-09-2002 1) Fixed an "issue" in calls to ReportStatusToSCM which your service should be calling in your ServiceMain. Specifically
                          calling ReportStatusToSCM(SERVICE_STOPPED) looks like it elicits the behaviour that the SCM forcefully terminates your
                          worker thread. This can cause issues when other code is executed after this call which uses something which is allocated
                          on the worker threads stack. To fix this "issue" (I could not find any documentation on this behaviour) you should remove
                          the single call to ReportStatusToSCM which uses the values "SERVICE_STOPPED" from the bottom of your service's ServiceMain
                          function. To force you to do this, I have changed the name of the function "ReportStatusToSCM" to just "ReportStatus". This
                          will mean that the compiler will force you to examine each call to this function and remove the one call as described above.
                          The class framework now looks after reporting that your service has stopped to the SCM. Thanks to "Jim" for spotting this.
         PJN / 20-10-2002 1. Fixed 3 level 4 warnings when the code is compiled using VC .NET. Thanks to Ed Eichman for spotting this problem.
         PJN / 12-03-2003 1. Fixed a problem since user code can't call ReportStatus(SERVICE_STOPPED) there is no way to report startup or shutdown 
                          errors via  dwWin32ExitCode or dwServiceSpecificExitCode. This has been resolved by adding a new protected function 
                          TerminateService to shut down the service in the event of a critical failure. This throws a private exception containing the 
                          exit codes which is caught by the framework in _ServiceMain (or the DebugService case of ProcessShellCommand). _ServiceMain 
                          then passes the codes on via _ReportStatus. Also since the dwWin32ExitCode and dwServiceSpecificExitCode arguments of 
                          ReportStatus can never be used these parameters have now been removed. Thanks to Jon Baggott for these 2 additions.
                          2. Performed since minor tidy ups in the inclusion of header files by the framework
         PJN / 08-05-2003 1) Update to the Install and Uninstall methods to return the value from GetLastError(). This is useful for client code
                          as it can be reset before the functions return.
                          2) Command line now supports a /T: param which allows you to specify a timeout to use when doing an uninstall. This timeout
                          is used to specify how long the code will wait if the service needs to be stopped.
                          3) Broke down all the classes into their own modules. This allows you to pull in only the classes you require, if you are 
                          not using the main class CNTService.
                          4) Removed all unnecessary TRACE statements throughout the framework as they sometimes reset the value as returned
                          from GetLastError.
         PJN / 17-05-2003 1) Further tweaks to _ReportStatus to avoid intermittent crashes. See item 1 on 20-09-2002 for detailed info about the issue.
                          2) Removed some unnecessary calls to locking the critical section
         PJN / 22-05-2003 1) Updated the way the sample service's message dll projects integrates the creation of the mc file. For detailed information
                          on how to integrate mc files into Visual Studio, please see the CodeProject article 
                          "http://www.codeproject.com/useritems/MCTutorial.asp". Please note that I did not follow the suggested settings as exactly 
                          outlined in this article as the message file's header file needs to be included into the nt service exe at compile time.
         PJN / 03-08-2003 1) Improved the error handling and reporting in CNTService::SetServiceToStatus. Thanks to Jon Pawley for reporting this issue.
         PJN / 15-08-2003 1) Optimized the string copying in CEventLogRecord::CEventLogRecord(const EVENTLOGRECORD* pRecord). A bug was reported in this area 
                          by Jeroen Walter but it looks like this is a documentation bug in current versions of the MSDN. This structure was defined 
                          correctly in some early versions of the MSDN.
         PJN / 08-09-2003 1) Moved all the defines which the framework requires into a separate header file. Thanks to Dima Polyakov for pointing
                          out a compilation problem which occurred when you did not have the platform SDK installed which necessitated this change.
                          2) Changed the ordering of the command line used to compile the MC file. This avoids a problem as reported by 
                          Rodrigo Oliveira Fernandez when using the framework in VC 7
                          3) Fixed a compilation error related to the use of the preprocessor define NTSERV_EXCLUDE_VC6_WINSVC_DEFINES
         PJN / 03-10-2003 1) Updated the custom build steps to compile the mc file. Now uses the VC macro "$(InputPath)" instead of 
                          "$(InputDir)\$(InputName).mc".
                          2) Removed a number of unreferenced variables level 4 warnings when compiled with VC .NET 2003.
                          3) Provision of a specific workspace (Testsrv for Visual Studio.Net.dsw) which allows the sample service to be compiled
                          cleanly out of the box with Visual Studio .NET 2000 or Visual Studio .NET 2003. The issue is that the format of various build
                          macros such as $(InputDir) has changed from VC 6. Just select File -> Open Project in VS .NET and select "Testsrv for Visual Studio.Net.dsw".
         PJN / 14-11-2003 1) The location where the message dll for the service is located can now be tweaked at runtime by filling in the
                          CNTService::m_sMessageDll variable. Thanks to Brodie Thiesfield for this update.
                          2) Reviewed all TRACE statements which use a CString parameter. Now explicitly casts the CString to a LPCTSTR. Thanks to Brodie Thiesfield
                          for this update.
                          3) Added a new CNTService::m_LoggingDetail variable. This in combination with a new method called CNTService::Win32ErrorToString
                          allows more flexible control over how errors are logging to the event log. Thanks to Brodie Thiesfield for suggesting this update.
                          4)  command line arguments other than the "-debug" argument are passed through to ServiceMain() when running in debug mode. Thanks to
                          Brodie Thiesfield for suggesting this update.
         PJN / 19-11-2003 1) Reworked the function Win32ErrorToString to include the string resource ID. Thanks to Brodie Thiesfield for suggesting this.
                          2) Fixed CNTS_MSG_SERVICE_FAIL_PAUSE, CNTS_MSG_SERVICE_FAIL_CONTINUE and CNTS_MSG_SERVICE_FAIL_STOP event log entries which were not
                          using the new Win32ErrorToString method. Thanks to Brodie Thiesfield for reporting this.
                          3) Win32ErrorToString now includes a "bEventLog" parameter. When this is TRUE the function is being called to obtain the error 
                          which gets displayed in the event log, when FALSE, it is being used to display to the end user or the TRACE window
         PJN / 23-11-2003 1) Fixed a memory leak in CNTService::Debug when the service uses TerminateService to shutdown the service prematurely.
         PJN / 26-11-2003 1) Now supports a /silent command line option. Using this setting in combination with for example /install will ensure that
                          the framework does not display any message boxes or console output when an error occurs. Thanks to Metrich Frederic for this addition.
                          2) Changed the return value from ProcessShellCommand from a BOOL to a DWORD. This value is now use to return an error code from the 
                          service exe when it exits. In combination with the /silent command line option, this allows third party programs such as installers 
                          control over the service instance via the exit code of the service. Again thanks to Metrich Frederic for this addition.
                          3) Updated the sample app provided with the framework to show how client services can return the Win32 error code as now returned
                          by ProcessShellCommand.
         PJN / 22-01-2004 1) Fixed a bug in CNTScmService::EnumDependents and CNTServiceControlManager::EnumServices where enumeration would continue even
                          when the callback functions return FALSE. Thanks to Edward Livingston for spotting this.
         PJN / 18-03-2004 1. Fixed a bug in the CNTScmService::WaitForServiceStatus function which caused it to return TRUE even if the service failed to 
                          return the desired status (if pending status isn't the expected pending staus. 
                          2. Also ensured that CNTScmService::WaitForServiceStatus does not wait past the requested timeout period. Thanks to Brodie Thiesfield 
                          for these bug reports and the fixes.
         PJN / 09-04-2004 1. Further work to CNTScmService::WaitForServiceStatus. It now ignores pending states. In addition it ignores wait hints since we are
                          calling QueryServiceStatus as opposed to calling the ControlService function with the SERVICE_CONTROL_INTERROGATE control code. It also
                          allows the polling interval to be specified by a 3rd optional parameter. Thanks to Edward Livingston and "yong" for reporting this 
                          problem. Please note that since the third parameter to this function means something different that previously, you should review all
                          of your calls to WaitForServiceStatus to ensure it continues to operate correctly.
         PJN / 12-04-2004 1. Removed an unused variable in CNTScmService::WaitForServiceStatus. Thanks to Edward Livingston for reporting this issue.
         PJN / 05-05-2004 1. Fixed some warnings in CNTService::GetDependencies and CNTEventLogSource::SetStringArrayIntoRegistry when the code is compiled using 
                          the "Force conformance in For loop scope" VC compiler option (/Zc:forScope). Thanks to Alexey Kuznetsov for reporting this problem.
         PJN / 20-06-2004 1. Fixed a bug in CEventLogRecord::CEventLogRecord(const EVENTLOGRECORD*..) to do
                          with the handling of inter string NULLs at the end of the record. Thanks to "yong" for reporting this problem.
                          2. Optimized the code in CEventLogRecord::CEventLogRecord(const EVENTLOGRECORD*..).
         PJN / 21-06-2004 1. Provided a new virtual implementation of CNTService::_ServiceMain. To achieve this the previous static _ServiceMain is now called 
                          "_SDKServiceMain". This allows your service code to completely customize the startup of your service. For example if you service does 
                          not require an active worker thread to do any work (e.g. it only ever responds to client requests thro the SCM using 
                          OnUserDefinedRequest), then you could implement your own version of _ServiceMain which does not call _ReportStatus(SERVICE_STOPPED... 
                          This would allow you to use RegisterWaitForSingleObject to implement your service clean up code. For more information on this type 
                          of service please consult the MSDN documentation on "RegisterWaitForSingleObject" and "ServiceMain". Thanks to Haug Leuschner for 
                          reporting this.
         PJN / 24-06-2004 1. Reissue of the last update as I forgot to declare _ServiceMain virtual, Doh!
                          2. Also made the class CNTServiceTerminateException visible in the global namespace. This is required if you implement your own
                          _ServiceMain.
         PJN / 15-10-2004 1. Framework now automatically installs a console ctrl handler when run in debug mode. This means that the framework will automatically
                          call your service's OnStop method when running in debug mode if your service is built as a console app and you hit Ctrl+C or Ctrl+Break or
                          hit the close button in the console window. This helps you to verify the shutdown code of your service without having to run the code as 
                          a service. Because the console ctrl handler is executed in a separate thread to ServiceMain, this simulation mode calls your OnStop method 
                          in a separate thread just like the SCM does. This proves very helpful when you need to debug shutdown problems in your service which are 
                          thread related.
                          2. Added a few checks to various header files to warn if various header files have not already been included e.g. Afxmt.h. This helps avoid
                          simple compilation problems when the framework is added to a new project initially.
                          3. Sample app now uses the InterlockedExchange functions instead of a critical section to serialize access to variables used across multiple
                          threads. In addition the "m_bPaused" variable of the sample app is now also marked as volatile
                          4. Fixed a double declaration of "dwTypesSupported" in CNTEventLogSource::Install. In the process this fixes a level 4 warning which the 
                          code generated.
         PJN / 17-10-2004 1. Fixed a number of level 4 warnings in the framework when "Detect 64 bit portability issues" is enabled in Visual Studio .NET
         PJN / 11-11-2004 1. Fixed a futher 64 bit issue in CNTEventLogSource::SetStringArrayIntoRegistry when compiled in Visual Studio .NET. 
                          2. Addition of a CNTSERVICE_EXT_CLASS preprocessor macro which allows the classes to be easily added to an extension dll
         PJN / 11-02-2005 1. Addition of a virtual GetEventLogSourceName method which allows the name of the event log source to be customized at runtime. Thanks to 
                          Bara Cek for requesting this addition.
                          2. Updated the copyright details in all modules.
                          3. Addition of true Visual Studio .NET 2003 files for the sample service (Testsrv.sln and Testsrv.vcproj)
         PJN / 26-03-2005 1. Addition of a m_bProfileWriteFlush variable to the CNTService class which dictates if the registry settings should be committed 
                          immediately as opposed to relying on the lazy writer.
         PJN / 21-04-2005 1. CNTService::GetProfileInt and GetProfileString functions now includes an optional DWORD output parameter which when provided allows the value from 
                          GetLastError to be returned to client code. This allows code to distinguish between a default value which is returned because it is actually 
                          stored as opposed to returning a default value because some of the registry API's failed. Thanks to Tony Ronan for reporting this issue.
         PJN / 31-07-2005 1. Provided an overridden version of CNTEventLogSource::Report which has 3 string parameters
                          2. CNTEventLogSource::GetStringArrayFromRegistry and SetStringArrayIntoRegistry methods are now public as they are helpful to have available 
                          externally
         PJN / 14-08-2005 1. Fixed an issue where the SID and binary values was not being set correctly in the overloaded CEventLogRecord constructor which takes 
                          a EVENTLOGRECORD pointer. Thanks to Brian Lee for reporting this issue. Also the arrays which this information is copied into is now preallocated 
                          to improve performance.
         PJN / 11-01-2006 1. The command line options which the service uses to start with can now be specified via a new /SCL command line option. This is achieved by 
                          supporting a new command line option e.g. TestSrv.exe /install "/SCL:/Param1 /SomeOtherParam:XYZ". This additional flexibility is controlled by 
                          BOOL CNTService::m_bAllowCommandLine which by default is FALSE.
                          2. Updated copyright details for all modules
                          3. The service display name string which the service uses for installing can now be specified via a new /SDN command line option. This is achieved 
                          by supporting a new command line option e.g. TestSrv.exe /install "/SDN:PJ's Second Test Service". This additional flexibility is controlled by 
                          BOOL CNTService::m_bAllowDisplayNameChange which by default is FALSE.
                          4. The description string which the service uses for installing can now be specified via a new /SD command line option. This is achieved by 
                          supporting a new command line option e.g. TestSrv.exe /install "/SD:Some description". This additional flexibility is controlled by BOOL 
                          CNTService::m_bAllowDescriptionChange which by default is FALSE.
                          5. The service name string which the service uses for installing can now be specified via a new /SN command line option. This is achieved by 
                          supporting a new command line option e.g. TestSrv.exe /install "/SN:PJSERVICE2". This additional flexibility is controlled by 
                          CNTService::m_bAllowNameChange.
                          6. Removed code which writes an event log entry when the service is being uninstalling. There is not much sense in logging an entry that you have
                          uninstalled the service, when you are also uninstalling the event log source.
                          7. Removed the ANSI mc file. Instead now the mc file included in the zip file is the Unicode mc file as provided by Brodie Thiesfield (see the 
                          history list for v1.35).
                          8. Removal of CNTService::GetEventLogSourceName method. Instead now you can call m_EventLogSource.SetSourceName() to customize this value.
                          9. Provision of CNTService::SetUseConsole and CNTService::GetUseConsole methods.
                          10. The default setting for CNTService::m_bUseConsole is now set depending on whether the standard _CONSOLE preprocessor define is declared
                          11. Removed the unnecessary critical section lock in the CNTService constructor
                          12. Provision of CNTService::SetControlsAccepted and CNTService::GetControlsAccepted methods.
                          13. Provision of a default CNTService constructor
                          14. Provision of CNTService::SetServiceName, CNTService::SetInstanceServiceName, CNTService::SetMasterServiceName, CNTService::GetInstanceServiceName, 
                          CNTService::GetMasterServiceName, CNTService::SetDisplayName, CNTService::SetMasterDisplayName, CNTService::SetInstanceDisplayName, 
                          CNTService::GetMasterDisplayName, CNTService::GetInstanceDisplayName, CNTService::SetDescription, CNTService::SetMasterDescription, 
                          CNTService::SetInstanceDescription methods, CNTService::GetMasterDescription and CNTService::GetInstanceDescription. Most of the work for this release 
                          of the framework is to support a new concept of "instances" in CNTService.  Prior to this version of the CNTService framework, you could only ever 
                          install a single copy of any one service on the one machine. Now with the introduction of the /SDN, /SD and /SN command line options which are used 
                          during install and uninstall you can register the service multiple times, using different service names, display names and descriptions. The original 
                          values you construct your CNTService instance are called the master values and are unmodified during the lifetime of all instances of your services.
                          The "instance" values are modifiable (if you allow them to be so via CNTService::m_bAllowNameChange, CNTService::m_bAllowDisplayNameChange and 
                          CNTService::m_bAllowDescriptionChange) at runtime. When these values are provided on the command line during registration, the command line which 
                          the service is setup to use upon launch by the SCM is customized so that these same command line options are passed to it. This allows the same 
                          instance values to be available to the service when it is running as a service. Without this neat trick, there would be no easy way for the service
                          to detect under which instance details it should run.
                          15. Addition of generic versions of CNTService::WriteProfileString, CNTService::WriteProfileInt, CNTService::WriteProfileStringArray, 
                          CNTService::WriteProfileBinary, CNTService::GetProfileString, CNTService::GetProfileInt, CNTService::GetProfileStringArray and 
                          CNTService::GetProfileBinary. These implementations allow the service name to be specified as well as the Flushing setting which is hidden by the other
                          implementations. The existing implementations of these functions now simply call these new implementations. Providing these implementations now allows
                          client code to read and write the settings for any service (including other instances of the same service) without the need to create a new
                          CNTService instance.
                          16. Addition of a CNTService::EnumerateInstances method which allows you to enumerate all the instances of a service which have been installed
                          using the new master / instance additions to the framework in this version. This could prove useful where you are writing configuration type applets
                          which configure the settings for all the instances of your installed service.
                          17. As a security precaution once the username and password strings have been used for installing, they are overwritten in memory.
         PJN / 27-04-2006 1. Code now uses SecureZeroMemory when wiping out the username and password strings. This should further improve the security of these values
                          when they are manipulated by the class framework.
         PJN / 31-05-2006.1. Updated the code to work correctly in VC 6 without the Platform SDK being installed. I personally have the Feb 2003 Platform SDK installed with 
                          VC 6 and this is how most of my code is primarily tested. It seems some of my recent updates to CNTService have broken compatibility with VC 6 on its 
                          own. Also renamed the define "NTSERV_DO_NEW_WINSVC_DEFINES" which is required when you are compiling the class framework on Visual C++ 6 without the Platform 
                          SDK to be "CNTSERVICE_MISSING_PLATFORMSDK". Please note that I would advise all developers to install the Platform SDK (February 2003 was the latest
                          version which was compatible with VC 6), as I will probably drop support for VC 6 without the Platform SDK in the future. It is becoming too much work
                          to support the older Header files which were shipped with VC 6 and most of my other open source code is moving towards requiring the Platform SDK.
                          Thanks to Matthias Miller for reporting this issue. The "NTSERV_EXCLUDE_VC6_WINSVC_DEFINES" define is now defunct and no longer used by the CNTService code.
         PJN / 03-06-2006 1. Fixed a minor typo in CNTService::SetServiceToStatus. Thanks to Matthias Miller for reporting this issue.
         PJN / 07-06-2006 1. Removed three unreferrenced variables from CNTService::ProcessShellCommand. Thanks to Frederic Metrich for reporting this issue.
                          2. Minor update to the declaration of some functions in CNTService to using "WINAPI" in their function implementations. Thanks to Frederic Metrich for 
                          reporting this issue.
                          3. Updated the code in CNTService::Debug to exclude the "/debug", "-app", "/app", "-application" and "/application" command line options in addition to the 
                          "-debug" value. Again thanks to Frederic Metrich for reporting this issue.
                          4. Updated the sample service provided with the CNTService download to build as a console app instead of a Windows app. This allows you to check out the
                          console handler support which CNTService includes.
         PJN / 09-06-2006 1. Made CNTService::Initialize non virtual. This change was made because it is called by the CNTService constructor and you cannot call virtual functions
                          from a C++ class constructor (or destructor). For more info on this issue please see the online article at http://www.artima.com/cppsource/nevercall.html 
                          from the book "Effective C++" by Scott Meyers. Thanks to Frederic Metrich for spotting this issue. Because of this change you should examine your own code
                          which uses CNTService::Initialize to make sure this change has no side effects on your code. 
         PJN / 13-06-2006 1. To allow customization in client projects, the resource id and message id values can now be customized through two new virtual functions, namely 
                          CNTService::MapResourceID and CNTService::MapMessageID. Thanks to Jan Bartels for this update.
                          2. CNTService::SetServiceToStatus now exposes the interval to wait as a parameter as well as the polling interval as a parameter. In addition the command line
                          timeout interval is now passed to all calls to SetServiceToStatus in CNTService::ProcessShellCommand. The additional command line operations which are passed
                          the timeout value include starting, stopping, pausing and continuing the service, in addition to uninstalling the service which when it was introduced
                          in v1.27 was the only operation which got the timeout value.
         PJN / 24-06-2006 1. Added a missing MapResourceID call in CNTService::Win32ErrorToString. 
                          2. Updated the sample dsp and vcproj files to produce Unicode mc resources instead of ASCII.
                          3. Framework now accepts the command line /console to mean the same as /debug i.e. run the program as a normal (console) application without interacting 
                          with the Service Control Manager.
                          4. Updated code to compile cleanly using VC 2005.
                          5. Included info in ntservEventLogSource.cpp on how to find further info on the various registry values which are required to create user defined event logs.
         PJN / 25-06-2006 1. Combined the functionality of the _NTSERV_DATA class into the main CNTService class.
                          2. Code now uses newer C++ style casts instead of C style casts.
                          3. The class framework now requires the Platform SDK if compiled using VC 6.
                          4. Fixed code which was not securely resetting the command line username and password strings after they were used
         PJN / 02-07-2006 1. All calls to m_EventLogSource.Report are now protected with "if (m_bRegisterEventLogSource)" check
         PJN / 27-07-2006 1. The static versions of the profile functions in CNTService have now been renamed to include the word "Service" in their method names. This fixes a
                          compiler error when the code is used in VC 2005.
         PJN / 01-08-2006 1. Fix for two buffer size calculations using the new _tcscpy_s functions, one in CNTEventLogSource::SetStringArrayIntoRegistry and the other in
                          CNTService::GetDependencies. Thanks to DmityShm on the codeproject forums for reporting this issue.
         PJN / 21-09-2006 1. Minor update to use %X instead of %x in various TRACE statements.
         PJN / 02-02-2007 1. Updated copyright details.
                          2. Reworked CNTService::SetServiceToStatus to allow the last error value to be reported back to client code. Thanks to Matthias Miller for reporting this
                          issue.
                          3. Optimized calls to Win32ErrorToString.
         PJN / 22-03-2007 1. CNTService::SetServiceToStatus and CNTService::Uninstall no longer specifies that it requires the SC_MANAGER_LOCK access right to connect to the SCM. 
                          This change helps to allow services to be started and stopped from Power User accounts. Thanks to Matthias Miller for reporting this issue.
                          2. CNTService::SetServiceToStatus no longer specifies the STANDARD_RIGHTS_REQUIRED access right when opening a service to control. This change helps to 
                          allow services to be started and stopped from Power User accounts. Thanks to Matthias Miller for reporting this issue.
         PJN / 13-11-2007 1. Minor update to display HRESULT's correctly.
         PJN / 12-01-2008 1. Updated copyright details.
                          2. Provided a link to my blog in the html documentation with details on compiling with VC 6.
                          3. CNTService::Win32ErrorToString method has been reworked to now uses the FORMAT_MESSAGE_IGNORE_INSERTS flag. For more information please see Raymond 
                          Chen's blog at http://blogs.msdn.com/oldnewthing/archive/2007/11/28/6564257.aspx. Thanks to Alexey Kuznetsov for reporting this issue.
                          4. Updated the sample app to clean compile on VC 2008.
                          5. SecureEmptyString method is now defined as "FORCEINLINE".
         PJN / 01-06-2008 1. Code now compiles cleanly using Code Analysis (/analyze)
                          2. The code has now been updated to support VC 2005 or later only.  
         PJN / 01-08-2010 1. Updated copyright details.
                          2. Updated the project settings to more modern default values.
                          3. Fixed a trailing "," in the definition of the "ELErrorLoggingDetail" enum
                          4. Updated the sample app to compile cleanly on VC 2010
                          5. Updated the test code in App.cpp to only use the necessary rights when connecting to the SCM or opening a specific service. This avoids Access denied
                          errors on Windows Vista or later when UAC is enabled. Thanks to Jan Bartels for reporting this issue.
                          6. Added support for the following SERVICE_CONTROL_* defines: SERVICE_CONTROL_SESSIONCHANGE, SERVICE_CONTROL_PRESHUTDOWN, SERVICE_CONTROL_TIMECHANGE & 
                          SERVICE_CONTROL_TRIGGEREVENT. This control values represent new functionality for services on Windows Vista and Windows 7
                          7. Added a comment to a #include to tell users to recompile TestSrvMsg if they get a compile error for "ntserv_msg.h". Thanks to Paul Pignon for reporting
                          this issue.
                          8. Updated code to compile correctly using _ATL_CSTRING_EXPLICIT_CONSTRUCTORS define
                          9. Fixed a minor issue in CNTService::Win32ErrorToString with the use of printf format identifiers
                          10. Reworked all the internal heap memory management code to use ATL::CHeapPtr
                          11. Reworked all the internal registry management code to use ATL::CRegKey
                          12. The CNTService::m_bRegisterEventLogSource value has been renamed to m_bEventLogSource. This is a breaking change and may require your service code
                          to be updated if you reference this value from your code.
                          13. CNTService::Install has been refactored. Internally now this function calls InstallEventLogSource and InstallServiceConfiguration
                          14. Carried out a thorough code review to ensure all functionality is correct and functional on the Windows 7 and later 
         PJN / 03-09-2010 1. Fixed a bug in CNTService::GetServiceProfileString where it would incorrectly return FALSE.
                          2. Fixed a bug in CNTService::GetServiceProfileInt where it would incorrectly return FALSE.
         PJN / 10-11-2012 1. Updated copyright details.
                          2. Updated code to clean compile on VC 2012
         PJN / 01-01-2014 1. Updated copyright details.
                          2. Updated code to clean compile on VC 2013
                          3. Reworked the classes to optionally compile without MFC. By default the classes now use STL for strings and arrays but if you define 
                          CNTSERVICE_MFC_EXTENSIONS, the classes will revert back to using the MFC collection classes.
                          4. Updated the codebase to support all Service features upto Windows 8.1. New features supported includes SERVICE_CONFIG_LAUCH_PROTECTED support available 
                          via CNTScmService::QueryLaunchProtected and CNTScmService::ChangeLaunchProtected and QueryServiceDynamicInformation support available via 
                          CNTService::QueryServiceDynamicInformation.

Copyright (c) 1996 - 2014 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


///////////////////////////////// Includes ////////////////////////////////////

#include <pch.h>
#include "resource.h"

#ifndef _WINSVC_
#pragma message("To avoid this message, please put winsvc.h in your pre compiled header (normally stdafx.h)")
#include <WinSvc.h>
#endif

#include "ntserv.h"
#include "ntserv_msg.h" //If you get an error about this missing header file, then please make sure you build the TestSrvMsg project first


///////////////////////////////// Macros /Defines /////////////////////////////

#ifdef CNTSERVICE_MFC_EXTENSIONS
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#endif

CNTService* CNTService::sm_lpService = NULL;


//////////////////////////////// Implementation ///////////////////////////////

CNTServiceTerminateException::CNTServiceTerminateException(DWORD dwWin32ExitCode, DWORD dwServiceSpecificExitCode) : m_dwWin32ExitCode(dwWin32ExitCode), 
                                                                                                                     m_dwServiceSpecificExitCode(dwServiceSpecificExitCode) 
{
}

CNTService::CNTService()
{
  InitializeFunctionPointers();
  Initialize(NULL, NULL, SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_STOP, NULL); //The dwControlsAccepted value we pass to Initialize doesn't really matter
                                                                                     //since client code can cusomize this by calling SetControlsAccepted or
                                                                                     //implementing a derived version of Initialize
}

CNTService::CNTService(LPCTSTR lpszServiceName, LPCTSTR lpszDisplayName, DWORD dwControlsAccepted, LPCTSTR lpszDescription) : m_lpfnRegisterServiceCtrlHandlerEx(NULL)
{
  //Validate our parameters
  ATLASSERT(lpszServiceName != NULL);
  ATLASSERT(lpszDisplayName != NULL);

  InitializeFunctionPointers();
  Initialize(lpszServiceName, lpszDisplayName, dwControlsAccepted, lpszDescription);
}

void CNTService::InitializeFunctionPointers()
{
  m_hAdvapi32 = GetModuleHandle(_T("ADVAPI32.DLL"));
  if (m_hAdvapi32 != NULL)
  {
  #ifdef _UNICODE
    m_lpfnRegisterServiceCtrlHandlerEx = reinterpret_cast<LPREGISTERSERVICECTRLHANDLEREX>(GetProcAddress(m_hAdvapi32, "RegisterServiceCtrlHandlerExW"));
  #else
    m_lpfnRegisterServiceCtrlHandlerEx = reinterpret_cast<LPREGISTERSERVICECTRLHANDLEREX>(GetProcAddress(m_hAdvapi32, "RegisterServiceCtrlHandlerExA"));
  #endif
    m_lpfnQueryServiceDynamicInformation = reinterpret_cast<LPQUERYSERVICEDYNAMICINFORMATION>(GetProcAddress(m_hAdvapi32, "QueryServiceDynamicInformation"));
  }
}

void CNTService::SetServiceName(LPCTSTR sServiceName) 
{
  //Change both the master service name and the active/current instance service name
  m_sServiceName = sServiceName; 
  m_sMasterServiceName = sServiceName;
}

void CNTService::SetDisplayName(LPCTSTR sDisplayName) 
{ 
  //Change both the master service name and the active/current instance display name
  m_sDisplayName = sDisplayName; 
  m_sMasterDisplayName = sDisplayName;
}

void CNTService::SetDescription(LPCTSTR sDescription) 
{ 
  //Change both the master description and the active/current instance description
  m_sDescription = sDescription; 
  m_sMasterDescription = sDescription;
}

void CNTService::Initialize(LPCTSTR lpszServiceName, LPCTSTR lpszDisplayName, DWORD dwControlsAccepted, LPCTSTR lpszDescription)
{
  if (lpszServiceName)
  {
    m_sServiceName = lpszServiceName;
    m_sMasterServiceName = m_sServiceName;
  }
  if (lpszDisplayName)
  {
    m_sDisplayName = lpszDisplayName;
    m_sMasterDisplayName = m_sDisplayName;
  }
  if (lpszDescription)
  {
    m_sDescription = lpszDescription;
    m_sMasterDescription = m_sDescription;
  }

  m_hStatus                 = 0;
  m_dwCheckPoint            = 0;
  m_dwWaitHint              = 0;
  m_dwRequestedControl      = 0;
  m_dwCurrentState          = SERVICE_STOPPED;
  m_dwControlsAccepted      = dwControlsAccepted;
  m_dwServiceType           = SERVICE_WIN32_OWN_PROCESS;
  m_dwStartType             = SERVICE_DEMAND_START;
  m_dwErrorControl          = SERVICE_ERROR_IGNORE;
  m_bEventLogSource         = TRUE;
#ifdef _CONSOLE
  m_bUseConsole             = TRUE;
#else
  m_bUseConsole             = FALSE;
#endif
  m_UILoggingDetail         = UI_StringAndErrorCodeAndErrorDescription;
  m_ELLoggingDetail         = EL_ErrorCodeAndErrorDescription;
  m_bProfileWriteFlush      = TRUE; //Default to the slower but safer form of registry writing
  m_bAllowCommandLine       = FALSE;
  m_bAllowNameChange        = FALSE; 
  m_bAllowDescriptionChange = FALSE;
  m_bAllowDisplayNameChange = FALSE;
  m_bDelayedAutoStart       = FALSE;
  m_dwServiceSidType        = SERVICE_SID_TYPE_NONE; //Use the old style sid type by default
  m_dwPreshutdownTimeout    = 0;
  m_dwEventCategoryCount    = 0;
  m_dwEventTypesSupported   = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
  m_dwLaunchProtected       = SERVICE_LAUNCH_PROTECTED_NONE;
  
  //By default use a event log source name the same as the display name. You can of course
  //customize this by calling m_EventLogSource.SetSourceName in your derived class at the 
  //appropriate time
#ifdef CNTSERVICE_MFC_EXTENSIONS
  m_EventLogSource.SetSourceName(m_sDisplayName);
#else
  m_EventLogSource.SetSourceName(m_sDisplayName.c_str());
#endif

  //Copy the address of the current object so we can access it from
  //the static member callback functions.
  //WARNING: This limits the application to only one CNTService object. 
  //i.e. the CNTService class framework only supports 1 service per 
  //process i.e. you cannot use the flag SERVICE_WIN32_SHARE_PROCESS
  sm_lpService = this;	//hive away the this pointer;
}

BOOL CNTService::_ReportStatus(DWORD dwCurrentState, DWORD dwCheckPoint, DWORD dwWaitHint, DWORD dwControlsAccepted,
                              DWORD dwWin32ExitCode, DWORD dwServiceSpecificExitCode)
{
  //synchronise access to the variables
  ATL::CComCritSecLock<ATL::CComAutoCriticalSection> sl(m_CritSect, true); 

  m_dwCurrentState = dwCurrentState;
  SERVICE_STATUS ServiceStatus;
  ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

  //The only stat that can set exit codes is STOPPED
  //Fix if necessary, just in case not properly set
  if (dwCurrentState != SERVICE_STOPPED)
  {
    dwWin32ExitCode = S_OK;
    dwServiceSpecificExitCode = 0;
  }

  //Only pending states can set checkpoints and wait hints
  //and pending states *must* set wait hints
  if (dwCurrentState == SERVICE_STOPPED || dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_PAUSED)
  {
    //Requires hint and checkpoint == 0
    dwWaitHint = 0;
    dwCheckPoint = 0;
  }
  else
  {
    //You need to set wait hints and checkpoints
    if (dwWaitHint == STATE_NO_CHANGE)
      ATLASSERT(m_dwWaitHint);
    else
      ATLASSERT(dwWaitHint != 0);
    if (dwCheckPoint == STATE_NO_CHANGE)
      ATLASSERT(m_dwCheckPoint);
    else
      ATLASSERT(dwCheckPoint != 0);
  }

  if (dwCheckPoint != STATE_NO_CHANGE)
    m_dwCheckPoint = dwCheckPoint;
  if (dwWaitHint != STATE_NO_CHANGE)
    m_dwWaitHint = dwWaitHint;
  if (dwControlsAccepted != STATE_NO_CHANGE)
    m_dwControlsAccepted = dwControlsAccepted;

  //Disable control requests while the service is in a pending state
  if (dwCurrentState == SERVICE_START_PENDING || dwCurrentState == SERVICE_STOP_PENDING || 
      dwCurrentState == SERVICE_PAUSE_PENDING || dwCurrentState == SERVICE_CONTINUE_PENDING)
    ServiceStatus.dwControlsAccepted = 0;    
  else
    ServiceStatus.dwControlsAccepted = m_dwControlsAccepted;

  ServiceStatus.dwCurrentState = dwCurrentState;
  ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
  ServiceStatus.dwServiceSpecificExitCode = dwServiceSpecificExitCode;
  ServiceStatus.dwCheckPoint = m_dwCheckPoint;
  ServiceStatus.dwWaitHint = m_dwWaitHint;

  //Release the critical section lock now that we are finished tweaking the member variables
  sl.Unlock();

  //Only report to the SCM if we have a SCM status handle
  BOOL bSuccess = FALSE;
  if (m_hStatus)
    bSuccess = SetServiceStatus(m_hStatus, &ServiceStatus);

  return bSuccess;
}

BOOL CNTService::ReportStatus(DWORD dwCurrentState, DWORD dwCheckPoint, DWORD dwWaitHint, DWORD dwControlsAccepted)
{
  ATLASSERT(dwCurrentState != SERVICE_STOPPED); //You should never call ReportStatus (previously ReportStatusToSCM) with the parameter
                                                //SERVICE_STOPPED
 
  return _ReportStatus(dwCurrentState, dwCheckPoint, dwWaitHint, dwControlsAccepted);
}

BOOL CNTService::ReportStatus()
{
  return ReportStatus(m_dwCurrentState);
}

void CNTService::OnStop()
{
  //Derived classes are required to implement
  //their own code to stop a service, all we do is
  //report that we were succesfully stopped

  //Add an Event log entry to say the service was stopped
  if (m_bEventLogSource)
  {
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, MapMessageID(CNTS_MSG_SERVICE_STOPPED), m_sDisplayName);
  #else
    m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, MapMessageID(CNTS_MSG_SERVICE_STOPPED), m_sDisplayName.c_str());
  #endif
  }

  ATLASSERT(FALSE);
}

void CNTService::OnPause()
{
  //Add an Event log entry to say the service was stopped
  if (m_bEventLogSource)
  {
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, MapMessageID(CNTS_MSG_SERVICE_PAUSED), m_sDisplayName);
  #else
    m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, MapMessageID(CNTS_MSG_SERVICE_PAUSED), m_sDisplayName.c_str());
  #endif
  }

  //Derived classes are required to implement
  //their own code to pause a service
  ATLASSERT(FALSE);
}

void CNTService::OnContinue()
{
  //Add an Event log entry to say the service was stopped
  if (m_bEventLogSource)
  {
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, MapMessageID(CNTS_MSG_SERVICE_CONTINUED), m_sDisplayName);
  #else
    m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, MapMessageID(CNTS_MSG_SERVICE_CONTINUED), m_sDisplayName.c_str());
  #endif
  }

  //Derived classes are required to implement
  //their own code to continue a service
  ATLASSERT(FALSE);
}

void CNTService::OnInterrogate()
{
  //Default implementation returns the current status
  //as stored in m_ServiceStatus
  ReportStatus();
}

void CNTService::OnShutdown()
{
  //Add an Event log entry to say the service was stopped
  if (m_bEventLogSource)
  {
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, MapMessageID(CNTS_MSG_SERVICE_SHUTDOWN), m_sDisplayName);
  #else
    m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, MapMessageID(CNTS_MSG_SERVICE_SHUTDOWN), m_sDisplayName.c_str());
  #endif
  }

  //Derived classes are required to implement
  //their own code to shutdown a service
  ATLASSERT(FALSE);
}

void CNTService::OnUserDefinedRequest(DWORD /*dwControl*/)
{
  ATLTRACE(_T("CNTService::OnUserDefinedRequest was called\n"));

  //Default implementation is do nothing
}

void CNTService::OnNetBindAdd()
{
  ATLTRACE(_T("CNTService::OnNetBindAdd was called\n"));

  //Default implementation is do nothing
}

void CNTService::OnNetBindRemove()
{
  ATLTRACE(_T("CNTService::OnNetBindRemove was called\n"));

  //Default implementation is do nothing
}

void CNTService::OnNetBindEnable()
{
  ATLTRACE(_T("CNTService::OnNetBindEnable was called\n"));

  //Default implementation is do nothing
}

void CNTService::OnNetBindDisable()
{
  ATLTRACE(_T("CNTService::OnNetBindDisable was called\n"));

  //Default implementation is do nothing
}

void CNTService::OnParamChange()
{
  ATLTRACE(_T("CNTService::OnParamChange was called\n"));

  //Default implementation is do nothing
}

#ifdef _DEBUG
DWORD CNTService::OnHardwareProfileChange(DWORD dwEventType, LPVOID /*lpEventData*/)
#else
DWORD CNTService::OnHardwareProfileChange(DWORD /*dwEventType*/, LPVOID /*lpEventData*/)
#endif
{
#ifdef _DEBUG
  ATLTRACE(_T("CNTService::OnHardwareProfileChange was called, EventType:%08X\n"), dwEventType);
#endif

  //Default implementation just returns NO_ERROR which grants the request
  return NO_ERROR;
}

#ifdef _DEBUG
DWORD CNTService::OnPowerEvent(DWORD dwEventType, LPVOID /*lpEventData*/)
#else
DWORD CNTService::OnPowerEvent(DWORD /*dwEventType*/, LPVOID /*lpEventData*/)
#endif
{
#ifdef _DEBUG
  ATLTRACE(_T("CNTService::OnPowerEvent was called, EventType:%08X\n"), dwEventType);
#endif

  //Default implementation just returns NO_ERROR which grants the request
  return NO_ERROR;
}

#ifdef _DEBUG
DWORD CNTService::OnDeviceEvent(DWORD dwEventType, LPVOID /*lpEventData*/)
#else
DWORD CNTService::OnDeviceEvent(DWORD /*dwEventType*/, LPVOID /*lpEventData*/)
#endif
{
#ifdef _DEBUG
  ATLTRACE(_T("CNTService::OnDeviceEvent was called, EventType:%08X\n"), dwEventType);
#endif

  //Default implementation just returns NO_ERROR which grants the request
  return NO_ERROR;
}

#ifdef _DEBUG
void CNTService::OnSessionChange(DWORD dwEventType, WTSSESSION_NOTIFICATION* /*lpEventData*/)
#else
void CNTService::OnSessionChange(DWORD /*dwEventType*/, WTSSESSION_NOTIFICATION* /*lpEventData*/)
#endif
{
#ifdef _DEBUG
  ATLTRACE(_T("CNTService::OnSessionChange was called, EventType:%08X\n"), dwEventType);
#endif
}

void CNTService::OnPreShutdown()
{
#ifdef _DEBUG
  ATLTRACE(_T("CNTService::OnPreShutdown was called\n"));
#endif
}

void CNTService::OnTimeChange(SERVICE_TIMECHANGE_INFO* /*lpEventData*/)
{
#ifdef _DEBUG
  ATLTRACE(_T("CNTService::OnTimeChange was called\n"));
#endif
}

void CNTService::OnTriggerEvent()
{
#ifdef _DEBUG
  ATLTRACE(_T("CNTService::OnTriggerEvent was called\n"));
#endif
}

DWORD WINAPI CNTService::ServiceCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData)
{
  //Need to keep an additional control request of the same type 
  //from coming in when your're already handling it.
  if (dwControl == m_dwRequestedControl)
    return ERROR_CALL_NOT_IMPLEMENTED;

  //Just switch on the control code sent to 
  //us and call the relavent virtual function
  DWORD dwRet = NO_ERROR;
  switch (dwControl)
  {
    case SERVICE_CONTROL_STOP: 
    {
      OnStop();
      break;
    }
    case SERVICE_CONTROL_PAUSE:
    {
      OnPause();
      break;
    }
    case SERVICE_CONTROL_CONTINUE:
    {
      OnContinue();
      break;
    }
    case SERVICE_CONTROL_INTERROGATE:
    {
      OnInterrogate();
      break;
    }
    case SERVICE_CONTROL_SHUTDOWN:
    {
      OnShutdown();
      break;
    }
    case SERVICE_CONTROL_PARAMCHANGE: 
    {
      OnParamChange();
      break;
    }
    case SERVICE_CONTROL_NETBINDADD: 
    {
      OnNetBindAdd();
      break;
    }
    case SERVICE_CONTROL_NETBINDREMOVE:
    {
      OnNetBindRemove();
      break;
    }
    case SERVICE_CONTROL_NETBINDENABLE:
    {
      OnNetBindEnable();
      break;
    }
    case SERVICE_CONTROL_NETBINDDISABLE:
    {
      OnNetBindDisable();
      break;
    }
    case SERVICE_CONTROL_DEVICEEVENT:
    {
      dwRet = OnDeviceEvent(dwEventType, lpEventData);
      break;
    }
    case SERVICE_CONTROL_HARDWAREPROFILECHANGE:
    {
      dwRet = OnHardwareProfileChange(dwEventType, lpEventData);
      break;
    }
    case SERVICE_CONTROL_POWEREVENT:
    {
      dwRet = OnPowerEvent(dwEventType, lpEventData);
      break;
    }
    case SERVICE_CONTROL_SESSIONCHANGE:
    {
      OnSessionChange(dwEventType, static_cast<WTSSESSION_NOTIFICATION*>(lpEventData));
      break;
    }
    case SERVICE_CONTROL_PRESHUTDOWN:
    {
      OnPreShutdown();
      break;
    }
    case SERVICE_CONTROL_TIMECHANGE:
    {
      OnTimeChange(static_cast<SERVICE_TIMECHANGE_INFO*>(lpEventData));
      break;
    }
    case SERVICE_CONTROL_TRIGGEREVENT:
    {
      OnTriggerEvent();
      break;
    }
    default:
    {
      OnUserDefinedRequest(dwControl);
      break;
    }
  }

  //Any request from the SCM will be acked by this service
  ReportStatus();

  return dwRet;
}

BOOL CNTService::RegisterCtrlHandler()
{
  ATL::CComCritSecLock<ATL::CComAutoCriticalSection> sl(m_CritSect, true); //Synchronize access to the variables

  ATLASSERT(m_hStatus == 0); //If this ASSERTs, most likely you are calling RegisterCtrlHandler
                             //yourself. Just remove this call from your code as this is now
                             //done for you automatically by the CNTService framework

  //if we are running on Windows 2000 or later then use the RegisterCtrlHandlerEx call
  //and if not fall back to using RegisterCtrlHandler
  if (m_lpfnRegisterServiceCtrlHandlerEx)
  {
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    m_hStatus = m_lpfnRegisterServiceCtrlHandlerEx(m_sServiceName, _ServiceCtrlHandlerEx, NULL);
  #else
    m_hStatus = m_lpfnRegisterServiceCtrlHandlerEx(m_sServiceName.c_str(), _ServiceCtrlHandlerEx, NULL);
  #endif
  }
  else
  {
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    m_hStatus = RegisterServiceCtrlHandler(m_sServiceName, _ServiceCtrlHandler);
  #else
    m_hStatus = RegisterServiceCtrlHandler(m_sServiceName.c_str(), _ServiceCtrlHandler);
  #endif
  }

  return (m_hStatus != 0);
}

void WINAPI CNTService::ServiceMain(DWORD /*dwArgc*/, LPTSTR* /*lpszArgv*/)
{
  //Default implementation does nothing but asserts, your version should
  //implement its own service specific code
  ATLASSERT(FALSE); 
}

void WINAPI CNTService::_ServiceCtrlHandler(DWORD dwControl)
{
  //Validate our parameters
  ATLASSUME(sm_lpService != NULL);

  //Convert from the SDK world to the C++ world. In this
  //implementation we just use a single static, In the 
  //future we could use a map just like MFC does for HWND
  //to CWnd conversions
  sm_lpService->ServiceCtrlHandler(dwControl, 0, NULL);
}

DWORD WINAPI CNTService::_ServiceCtrlHandlerEx(DWORD dwControl, DWORD dwEventType,  LPVOID lpEventData, LPVOID /*lpContext*/)
{
  //Validate our parameters
  ATLASSUME(sm_lpService != NULL);

  //Convert from the SDK world to the C++ world. In this
  //implementation we just use a single static, In the 
  //future we could use a map just like MFC does for HWND
  //to CWnd conversions. Note we throw away the lpServiceName as it's available
  //as a the C++ member variable of CNTService
  return sm_lpService->ServiceCtrlHandler(dwControl, dwEventType, lpEventData);
}

void WINAPI CNTService::_SDKServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
  //Validate our parameters
  ATLASSUME(sm_lpService != NULL);

  //Convert from the SDK world to the C++ world. In this
  //implementation we just use a single static, In the 
  //future we could use a map just like MFC does for HWND
  //to CWnds conversions
  sm_lpService->_ServiceMain(dwArgc, lpszArgv);
}

void CNTService::_ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
  //Register the Control Handler
  RegisterCtrlHandler();
  
  DWORD dwWin32ExitCode = NO_ERROR;
  DWORD dwServiceSpecificExitCode = 0;
  
  //Call the main C++ function
  try
  {
    ServiceMain(dwArgc, lpszArgv);
  }
  catch(CNTServiceTerminateException& e)
  {
    dwWin32ExitCode = e.m_dwWin32ExitCode;
    dwServiceSpecificExitCode = e.m_dwServiceSpecificExitCode;
  }
  
  //Report to the SCM that the service has stopped.
  //Note that it is important that we do not access anything on the stack 
  //after this call as the SCM could have terminated this worker thread by then
  _ReportStatus(SERVICE_STOPPED, 0, 0, 0, dwWin32ExitCode, dwServiceSpecificExitCode);
}

BOOL CNTService::WriteServiceProfileString(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue, BOOL bFlush, DWORD* pLastError)
{
  //Validate our parameters
  ATLASSERT(lpszService != NULL);
  ATLASSUME(lpszSection != NULL);

  if (pLastError)
    *pLastError = ERROR_SUCCESS;

  LONG lResult;
  if (lpszEntry == NULL) //delete whole section
  {
    ATL::CRegKey serviceKey;
    if (!GetServiceParametersKey(serviceKey, lpszService, FALSE, pLastError))
      return FALSE;

    lResult = serviceKey.DeleteSubKey(lpszSection);
    if ((lResult != ERROR_SUCCESS) && pLastError)
      *pLastError = lResult;
    if (bFlush)
      serviceKey.Flush();
  }
  else if (lpszValue == NULL)
  {
    ATL::CRegKey secKey;
    if (!GetSectionKey(secKey, lpszService, lpszSection, FALSE, pLastError))
      return FALSE;

    lResult = secKey.DeleteValue(lpszEntry);
    if ((lResult != ERROR_SUCCESS) && pLastError)
      *pLastError = lResult;
    if (bFlush)
      secKey.Flush();
  }
  else
  {
    ATL::CRegKey secKey;
    if (!GetSectionKey(secKey, lpszService, lpszSection, FALSE, pLastError))
      return FALSE;

    lResult = secKey.SetStringValue(lpszEntry, lpszValue);
    if ((lResult != ERROR_SUCCESS) && pLastError)
      *pLastError = lResult;
    if (bFlush)
      secKey.Flush();
  }
  return lResult == ERROR_SUCCESS;
}

BOOL CNTService::WriteServiceProfileInt(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue, BOOL bFlush, DWORD* pLastError)
{
  //Validate our parameters
  ATLASSERT(lpszService != NULL);
  ATLASSERT(lpszSection != NULL);

  if (pLastError)
    *pLastError = ERROR_SUCCESS;

  ATL::CRegKey secKey;
  if (!GetSectionKey(secKey, lpszService, lpszSection, FALSE, pLastError))
    return FALSE;

  LONG lResult = secKey.SetDWORDValue(lpszEntry, nValue);
  if ((lResult != ERROR_SUCCESS) && pLastError)
    *pLastError = lResult;
  if (bFlush)
    secKey.Flush();
  return lResult == ERROR_SUCCESS;
}

BOOL CNTService::WriteServiceProfileStringArray(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, const CNTServiceStringArray& array, BOOL bFlush, DWORD* pLastError)
{
  //Validate our parameters
  ATLASSERT(lpszSection != NULL);
  ATLASSERT(lpszService != NULL);

  if (pLastError)
    *pLastError = ERROR_SUCCESS;

  ATL::CRegKey secKey;
  if (!GetSectionKey(secKey, lpszService, lpszSection, FALSE, pLastError))
    return FALSE;

  BOOL bSuccess = CNTEventLogSource::SetStringArrayIntoRegistry(secKey, lpszEntry, array, pLastError);
  if (bFlush)
    secKey.Flush();
  return bSuccess;
}

BOOL CNTService::WriteServiceProfileBinary(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, ULONG nBytes, BOOL bFlush, DWORD* pLastError)
{ 
  //Validate our parameters
  ATLASSERT(lpszSection != NULL);
  ATLASSERT(lpszService != NULL);

  if (pLastError)
    *pLastError = ERROR_SUCCESS;

  ATL::CRegKey secKey;
  if (!GetSectionKey(secKey, lpszService, lpszSection, FALSE, pLastError))
    return FALSE;

  LONG lResult = secKey.SetBinaryValue(lpszEntry, pData, nBytes);
  if ((lResult != ERROR_SUCCESS) && pLastError)
    *pLastError = lResult;
  if (bFlush)
    secKey.Flush();
  return lResult == ERROR_SUCCESS;
}

CNTServiceString CNTService::GetServiceProfileString(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault, DWORD* pLastError)
{
  //Validate our parameters
  ATLASSERT(lpszService != NULL);
  ATLASSERT(lpszSection != NULL);

  if (pLastError)
    *pLastError = ERROR_SUCCESS;

  ATL::CRegKey secKey;
  if (!GetSectionKey(secKey, lpszService, lpszSection, TRUE, pLastError))
    return CNTServiceString((lpszDefault == NULL) ? _T("") : lpszDefault);

  CNTServiceString sValue;
  DWORD dwType = 0;
  ULONG nBytes = 0;
  LONG lResult = secKey.QueryValue(lpszEntry, &dwType, NULL, &nBytes);
  if (lResult == ERROR_SUCCESS)
  {
    ULONG nChars = nBytes/sizeof(TCHAR);
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    lResult = secKey.QueryStringValue(lpszEntry, sValue.GetBuffer(nChars), &nChars);
    sValue.ReleaseBuffer();
  #else
    sValue.resize(nChars + 1);
    lResult = secKey.QueryStringValue(lpszEntry, &(sValue[0]), &nChars);
  #endif
  }

  if ((lResult != ERROR_SUCCESS) && pLastError)
    *pLastError = lResult;

  if (lResult == ERROR_SUCCESS)
    return sValue;

  return CNTServiceString((lpszDefault == NULL) ? _T("") : lpszDefault);
}

UINT CNTService::GetServiceProfileInt(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault, DWORD* pLastError)
{
  //Validate our parameters
  ATLASSERT(lpszService != NULL);
  ATLASSERT(lpszSection != NULL);

  if (pLastError)
    *pLastError = ERROR_SUCCESS;

  ATL::CRegKey secKey;
  if (!GetSectionKey(secKey, lpszService, lpszSection, TRUE, pLastError))
    return nDefault;

  DWORD dwValue;
  LONG lResult = secKey.QueryDWORDValue(lpszEntry, dwValue);
  if ((lResult != ERROR_SUCCESS) && pLastError)
    *pLastError = lResult;

  if (lResult == ERROR_SUCCESS)
    return static_cast<UINT>(dwValue);

  return nDefault;
}

BOOL CNTService::GetServiceProfileStringArray(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, CNTServiceStringArray& array, DWORD* pLastError)
{
  //Validate our parameters
  ATLASSERT(lpszService != NULL);
  ATLASSERT(lpszSection != NULL);

  if (pLastError)
    *pLastError = ERROR_SUCCESS;

  //Remove all the array entries before we go any further
#ifdef CNTSERVICE_MFC_EXTENSIONS
  array.RemoveAll();
#else
  array.clear();
#endif

  ATL::CRegKey secKey;
  if (!GetSectionKey(secKey, lpszService, lpszSection, TRUE, pLastError))
    return FALSE;

  return CNTEventLogSource::GetStringArrayFromRegistry(secKey, lpszEntry, array, pLastError);
}

BOOL CNTService::GetServiceProfileBinary(LPCTSTR lpszService, LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE* ppData, ULONG* pnBytes, DWORD* pLastError)
{
  //Validate our parameters
  ATLASSERT(lpszService != NULL);
  ATLASSERT(lpszSection != NULL);
  ATLASSERT(lpszEntry != NULL);
  ATLASSUME(ppData != NULL);
  ATLASSUME(pnBytes != NULL);

  if (pLastError)
    *pLastError = ERROR_SUCCESS;

  *ppData = NULL;
  *pnBytes = 0;

  ATL::CRegKey secKey;
  if (!GetSectionKey(secKey, lpszService, lpszSection, TRUE, pLastError))
    return FALSE;

  DWORD dwType = 0;
  LONG lResult = secKey.QueryValue(lpszEntry, &dwType, NULL, pnBytes);
  if (lResult == ERROR_SUCCESS)
  {
    *ppData = new BYTE[*pnBytes];
    lResult = secKey.QueryBinaryValue(lpszEntry, *ppData, pnBytes);
    if ((lResult != ERROR_SUCCESS) && pLastError)
      *pLastError = lResult;
  }
  else
  {
    if (pLastError)
      *pLastError = lResult;
  }

  if (lResult == ERROR_SUCCESS)
    return TRUE;
  else
  {
    delete [] *ppData;
    *ppData = NULL;
  }

  return FALSE;
}

// returns key for:
//      HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\ServiceName\Parameters\lpszSection
// creating it if it doesn't exist.
BOOL CNTService::GetSectionKey(ATL::CRegKey& sectionKey, LPCTSTR lpszServiceName, LPCTSTR lpszSection, BOOL bReadOnly, DWORD* pLastError)
{
  //Validate our parameters
  ATLASSERT(lpszServiceName != NULL);
  ATLASSUME(lpszSection != NULL);

  if (pLastError)
    *pLastError = ERROR_SUCCESS;

  ATL::CRegKey serviceParametersKey;
  if (!GetServiceParametersKey(serviceParametersKey, lpszServiceName, bReadOnly, pLastError))
    return FALSE;

  LONG lResult = sectionKey.Create(serviceParametersKey, lpszSection, REG_NONE, REG_OPTION_NON_VOLATILE, bReadOnly ? KEY_READ : KEY_WRITE | KEY_READ, NULL);
  if ((lResult != ERROR_SUCCESS) && pLastError)
    *pLastError = lResult;

  return (lResult == ERROR_SUCCESS);
}

// returns key for:
//      HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\ServiceName\Parameters
// creating it if it doesn't exist
BOOL CNTService::GetServiceParametersKey(ATL::CRegKey& serviceParametersKey, LPCTSTR lpszServiceName, BOOL bReadOnly, DWORD* pLastError)
{
  //Validate our parameters
  ATLASSUME(lpszServiceName != NULL);

  if (pLastError)
    *pLastError = ERROR_SUCCESS;

  ATL::CRegKey servicesKey;
  LONG lResult = servicesKey.Open(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services"), bReadOnly ? KEY_READ : KEY_WRITE | KEY_READ);
  if (lResult == ERROR_SUCCESS)
  {
    //Create the service key
    ATL::CRegKey serviceKey;
    lResult = serviceKey.Create(servicesKey, lpszServiceName, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL);
    if (lResult == ERROR_SUCCESS)
    {
      //Create the parameters key
      lResult = serviceParametersKey.Create(serviceKey, _T("Parameters"), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL);
      if ((lResult != ERROR_SUCCESS) && pLastError)
        *pLastError = lResult;
    }
    else
    {
      if (pLastError)
        *pLastError = lResult;
    }
  }
  else
  {
    if (pLastError)
      *pLastError = lResult;
  }

  return (lResult == ERROR_SUCCESS);
}

BOOL CNTService::WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
#ifdef CNTSERVICE_MFC_EXTENSIONS
  return WriteServiceProfileString(m_sServiceName, lpszSection, lpszEntry, lpszValue, m_bProfileWriteFlush);
#else
  return WriteServiceProfileString(m_sServiceName.c_str(), lpszSection, lpszEntry, lpszValue, m_bProfileWriteFlush);
#endif
}

BOOL CNTService::WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
#ifdef CNTSERVICE_MFC_EXTENSIONS
  return WriteServiceProfileInt(m_sServiceName, lpszSection, lpszEntry, nValue, m_bProfileWriteFlush);
#else
  return WriteServiceProfileInt(m_sServiceName.c_str(), lpszSection, lpszEntry, nValue, m_bProfileWriteFlush);
#endif
}

BOOL CNTService::WriteProfileStringArray(LPCTSTR lpszSection, LPCTSTR lpszEntry, const CNTServiceStringArray& array)
{
#ifdef CNTSERVICE_MFC_EXTENSIONS
  return WriteServiceProfileStringArray(m_sServiceName, lpszSection, lpszEntry, array, m_bProfileWriteFlush);
#else
  return WriteServiceProfileStringArray(m_sServiceName.c_str(), lpszSection, lpszEntry, array, m_bProfileWriteFlush);
#endif
}

BOOL CNTService::WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes)
{
#ifdef CNTSERVICE_MFC_EXTENSIONS
  return WriteServiceProfileBinary(m_sServiceName, lpszSection, lpszEntry, pData, nBytes, m_bProfileWriteFlush);
#else
  return WriteServiceProfileBinary(m_sServiceName.c_str(), lpszSection, lpszEntry, pData, nBytes, m_bProfileWriteFlush);
#endif
}

CNTServiceString CNTService::GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault, DWORD* pLastError)
{
#ifdef CNTSERVICE_MFC_EXTENSIONS
  return GetServiceProfileString(m_sServiceName, lpszSection, lpszEntry, lpszDefault, pLastError);
#else
  return GetServiceProfileString(m_sServiceName.c_str(), lpszSection, lpszEntry, lpszDefault, pLastError);
#endif
}

UINT CNTService::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault, DWORD* pLastError)
{
#ifdef CNTSERVICE_MFC_EXTENSIONS
  return GetServiceProfileInt(m_sServiceName, lpszSection, lpszEntry, nDefault, pLastError);
#else
  return GetServiceProfileInt(m_sServiceName.c_str(), lpszSection, lpszEntry, nDefault, pLastError);
#endif
}

BOOL CNTService::GetProfileStringArray(LPCTSTR lpszSection, LPCTSTR lpszEntry, CNTServiceStringArray& array)
{
#ifdef CNTSERVICE_MFC_EXTENSIONS
  return GetServiceProfileStringArray(m_sServiceName, lpszSection, lpszEntry, array);
#else
  return GetServiceProfileStringArray(m_sServiceName.c_str(), lpszSection, lpszEntry, array);
#endif
}

BOOL CNTService::GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE* ppData, ULONG* pnBytes)
{
#ifdef CNTSERVICE_MFC_EXTENSIONS
  return GetServiceProfileBinary(m_sServiceName, lpszSection, lpszEntry, ppData, pnBytes);
#else
  return GetServiceProfileBinary(m_sServiceName.c_str(), lpszSection, lpszEntry, ppData, pnBytes);
#endif
}

BOOL CNTService::Run()
{
  //Set up the SERVICE table array
  SERVICE_TABLE_ENTRY ServiceTable[2];
  TCHAR szServiceName[256];
#ifdef CNTSERVICE_MFC_EXTENSIONS
  _tcscpy_s(szServiceName, sizeof(szServiceName)/sizeof(TCHAR), m_sServiceName);
#else
  _tcscpy_s(szServiceName, sizeof(szServiceName)/sizeof(TCHAR), m_sServiceName.c_str());
#endif
  ServiceTable[0].lpServiceName = szServiceName;
  ServiceTable[0].lpServiceProc = _SDKServiceMain;
  ServiceTable[1].lpServiceName = 0;
  ServiceTable[1].lpServiceProc = 0;

  //Notify the SCM of our service
  return StartServiceCtrlDispatcher(ServiceTable);
}

BOOL CNTService::GetDependencies(ATL::CHeapPtr<TCHAR>& mszDependencies)
{
  //Work out the size of the string we need
#ifdef CNTSERVICE_MFC_EXTENSIONS
  int nSize = 0;
  INT_PTR nDependencies = m_sDependencies.GetSize();
  for (INT_PTR i=0; i<nDependencies; i++)
    nSize += (m_sDependencies.GetAt(i).GetLength() + 1);
#else
  CNTServiceStringArray::size_type nSize = 0;
  CNTServiceStringArray::size_type nDependencies = m_sDependencies.size();
  for (CNTServiceStringArray::size_type i=0; i<nDependencies; i++)
    nSize += (m_sDependencies[i].length() + 1);
#endif
    
  //Need one second NULL for the double NULL at the end
  nSize++;

  //Now form the string
  if (!mszDependencies.Allocate(nSize))
  {
    SetLastError(ERROR_OUTOFMEMORY);
    return FALSE;
  }

  //Now copy thr strings into the buffer
  LPTSTR lpszString = mszDependencies.m_pData;
#ifdef CNTSERVICE_MFC_EXTENSIONS
  int nCurOffset = 0;
  for (INT_PTR i=0; i<nDependencies; i++)
#else
  CNTServiceStringArray::size_type nCurOffset = 0;
  for (CNTServiceStringArray::size_type i=0; i<nDependencies; i++)
#endif
  {
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    CNTServiceString sDependency(m_sDependencies.GetAt(i));
    int nDependencyLength = sDependency.GetLength();
    _tcscpy_s(&lpszString[nCurOffset], nDependencyLength+1, sDependency);
  #else
    const CNTServiceString& sDependency = m_sDependencies[i];
    CNTServiceString::size_type nDependencyLength = sDependency.length();
    _tcscpy_s(&lpszString[nCurOffset], nDependencyLength+1, sDependency.c_str());
  #endif
    nCurOffset += (nDependencyLength + 1);
  }
  //Don't forgot to doubly NULL terminate
  lpszString[nCurOffset] = _T('\0');

  return TRUE;
}

void CNTService::TerminateService(DWORD dwWin32ExitCode, DWORD dwServiceSpecificExitCode)
{
  throw CNTServiceTerminateException(dwWin32ExitCode, dwServiceSpecificExitCode);
}

void CNTService::SecureEmptyString(CNTServiceString& sVal)
{
#ifdef CNTSERVICE_MFC_EXTENSIONS
  int nLength = sVal.GetLength();
  LPTSTR pszVal = sVal.GetBuffer(nLength);
  SecureZeroMemory(pszVal, nLength*sizeof(TCHAR));
  sVal.ReleaseBuffer();
#else
  SecureZeroMemory(&(sVal[0]), sVal.length());
#endif
}

BOOL CNTService::InstallEventLogSource(CNTServiceString& sErrorMsg, DWORD& dwError)
{
  //Validate our parameters
  ATLASSERT(m_bEventLogSource);

  //Setup this service as an event log source (using the friendly name)
  //If a message dll has not been specified, then we use one with the same name 
  //as this application and in the same path
#ifdef CNTSERVICE_MFC_EXTENSIONS
  if (m_sEventMessageFile.IsEmpty())
#else
  if (m_sEventMessageFile.length() == 0)
#endif
  {
    TCHAR szAppPath[_MAX_PATH];
    szAppPath[0] = _T('\0');
    if (!GetModuleFileName(NULL, szAppPath, _MAX_PATH))
    {
      dwError = GetLastError();
      sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_INSTALL_FAIL_INSTALL_AS_EVENT_LOG_SOURCE), dwError, FALSE);
      TraceMessage(sErrorMsg);
      return FALSE;
    }
    TCHAR szPath[_MAX_PATH];
    szPath[0] = _T('\0');
    TCHAR szDrive[_MAX_DRIVE];
    szDrive[0] = _T('\0');
    TCHAR szDir[_MAX_DIR];
    szDir[0] = _T('\0');
    TCHAR szFname[_MAX_FNAME];
    szFname[0] = _T('\0');
    _tsplitpath_s(szAppPath, szDrive, sizeof(szDrive)/sizeof(TCHAR), szDir, sizeof(szDir)/sizeof(TCHAR), szFname, sizeof(szFname)/sizeof(TCHAR), NULL, 0);
    _tmakepath_s(szPath, sizeof(szPath)/sizeof(TCHAR), szDrive, szDir, szFname, _T("DLL"));
    m_sEventMessageFile = szPath;
  }

#ifdef CNTSERVICE_MFC_EXTENSIONS
  if (!m_EventLogSource.Install(m_EventLogSource.GetLogName(), m_EventLogSource.GetSourceName(), m_sEventMessageFile, m_sEventCategoryMessageFile, m_sEventParameterMessageFile, m_dwEventTypesSupported, m_dwEventCategoryCount))
#else
  if (!m_EventLogSource.Install(m_EventLogSource.GetLogName().c_str(), m_EventLogSource.GetSourceName().c_str(), m_sEventMessageFile.c_str(), m_sEventCategoryMessageFile.c_str(), m_sEventParameterMessageFile.c_str(), m_dwEventTypesSupported, m_dwEventCategoryCount))
#endif
  {
    dwError = GetLastError();
    sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_INSTALL_FAIL_INSTALL_AS_EVENT_LOG_SOURCE), dwError, FALSE);
    TraceMessage(sErrorMsg);
    return FALSE;
  }

  return TRUE;
}

BOOL CNTService::InstallServiceConfiguration(CNTScmService& service, CNTServiceString& sErrorMsg, DWORD& dwError)
{
  //Change the service description if necessary
#ifdef CNTSERVICE_MFC_EXTENSIONS
  if (m_sDescription.GetLength())
#else
  if (m_sDescription.length())
#endif
  {
    if (!service.ChangeDescription(m_sDescription))
    {
      dwError = GetLastError();  
      sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_INSTALL_FAIL_CHANGEDESCRIPTION), dwError, FALSE);
      TraceMessage(sErrorMsg);
      return FALSE;
    }
  }

  //Change the service Sid if necessary
  if (m_dwServiceSidType != SERVICE_SID_TYPE_NONE)
  {
    if (!service.ChangeSidInfo(m_dwServiceSidType))
    {
      dwError = GetLastError();  
      sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_INSTALL_FAIL_CHANGESIDINFO), dwError, FALSE);
      TraceMessage(sErrorMsg);
      return FALSE;
    }
  }

  //Change the service DelayedAutoStart setting if necessary
  if (m_bDelayedAutoStart)
  {
    if (!service.ChangeDelayAutoStart(m_bDelayedAutoStart))
    {
      dwError = GetLastError();  
      sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_INSTALL_FAIL_CHANGEDELAYAUTOSTART), dwError, FALSE);
      TraceMessage(sErrorMsg);
      return FALSE;
    }
  }

  //Change the service privileges if necessary
#ifdef CNTSERVICE_MFC_EXTENSIONS
  if (m_sPrivileges.GetSize())
#else
  if (m_sPrivileges.size())
#endif
  {
    if (!service.ChangeRequiredPrivileges(m_sPrivileges))
    {
      dwError = GetLastError();  
      sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_INSTALL_FAIL_CHANGEPRIVILEGES), dwError, FALSE);
      TraceMessage(sErrorMsg);
      return FALSE;
    }
  }

  //Change the preshutdown timeout if necessary
  if (m_dwPreshutdownTimeout)
  {
    if (!service.ChangePreShutdown(m_dwPreshutdownTimeout))
    {
      dwError = GetLastError();  
      sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_INSTALL_FAIL_CHANGEPRESHUTDOWNTIMEOUT), dwError, FALSE);
      TraceMessage(sErrorMsg);
      return FALSE;
    }
  }

  //Change the Launch protected setting if necessary
  if (m_dwLaunchProtected != SERVICE_LAUNCH_PROTECTED_NONE)
  {
    if (!service.ChangeLaunchProtected(m_dwLaunchProtected))
    {
      dwError = GetLastError();  
      sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_INSTALL_FAIL_CHANGELAUNCHPROTECTED), dwError, FALSE);
      TraceMessage(sErrorMsg);
      return FALSE;
    }
  }

  return TRUE;
}

BOOL CNTService::Install(CNTServiceString& sErrorMsg, DWORD& dwError)
{
  //By default empty the error message 
#ifdef CNTSERVICE_MFC_EXTENSIONS
  sErrorMsg.Empty();
#else
  sErrorMsg.clear();
#endif
  dwError = ERROR_SUCCESS;

  //Open up the SCM requesting Creation rights
  CNTServiceControlManager manager;
  if (!manager.Open(NULL, SC_MANAGER_CREATE_SERVICE | SC_MANAGER_LOCK))
  {
    dwError = GetLastError();
    sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_INSTALL_FAIL_OPEN_SCM), dwError, FALSE);
    TraceMessage(sErrorMsg);
    return FALSE;
  }

  //Lock the SCM since we are going to install a service
  if (!manager.Lock())
  {
    dwError = GetLastError();
    sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_INSTALL_FAIL_LOCK_SCM), dwError, FALSE);
    TraceMessage(sErrorMsg);
    return FALSE;
  }

  //Get the dependencies for this service
  ATL::CHeapPtr<TCHAR> mszDependencies;
  if (!GetDependencies(mszDependencies))
  {
    dwError = GetLastError();
    sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_INSTALL_GET_DEPENDENCIES), dwError, FALSE);
    TraceMessage(sErrorMsg);
    return FALSE;
  }

  //Create the new service entry in the SCM database
  CNTScmService service;
#ifdef CNTSERVICE_MFC_EXTENSIONS
  if (!service.Create(manager, m_sServiceName, m_sDisplayName, SERVICE_CHANGE_CONFIG, m_dwServiceType, m_dwStartType, 
                      m_dwErrorControl, m_sBinaryPathName, m_sLoadOrderGroup.GetLength() ? m_sLoadOrderGroup.operator LPCTSTR() : NULL, 
                      NULL, mszDependencies, m_sUserName.GetLength() ? m_sUserName.operator LPCTSTR() : NULL, 
                      m_sPassword.GetLength() ? m_sPassword.operator LPCTSTR() : NULL))
#else
  if (!service.Create(manager, m_sServiceName.c_str(), m_sDisplayName.c_str(), SERVICE_CHANGE_CONFIG, m_dwServiceType, m_dwStartType, 
                      m_dwErrorControl, m_sBinaryPathName.c_str(), m_sLoadOrderGroup.length() ? m_sLoadOrderGroup.c_str() : NULL, 
                      NULL, mszDependencies, m_sUserName.length() ? m_sUserName.c_str() : NULL, 
                      m_sPassword.length() ? m_sPassword.c_str() : NULL))
#endif
  {
    dwError = GetLastError();  

    //As a security precaution we nuke the username and password details once we have used them
    SecureEmptyString(m_sUserName);
    SecureEmptyString(m_sPassword);

    sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_INSTALL_FAIL_CREATESERVICE), dwError, FALSE);
    TraceMessage(sErrorMsg);
    return FALSE;
  }

  //As a security precaution we nuke the username and password details once we have used them
  SecureEmptyString(m_sUserName);
  SecureEmptyString(m_sPassword);

  //Install the event log source
  if (m_bEventLogSource)
    if (!InstallEventLogSource(sErrorMsg, dwError))
      return FALSE;

  //Setup the service configuration settings
  if (!InstallServiceConfiguration(service, sErrorMsg, dwError))
    return FALSE;

  //Add an Event log entry to say the service was successfully installed
  if (m_bEventLogSource)
  {
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, MapMessageID(CNTS_MSG_SERVICE_INSTALLED), m_sDisplayName);
  #else
    m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, MapMessageID(CNTS_MSG_SERVICE_INSTALLED), m_sDisplayName.c_str());
  #endif
  }

#ifdef CNTSERVICE_MFC_EXTENSIONS
  ATLTRACE(_T("CNTService::Install, Service: %s was succesfully installed\n"), m_sServiceName.operator LPCTSTR());
#else
  ATLTRACE(_T("CNTService::Install, Service: %s was succesfully installed\n"), m_sServiceName.c_str());
#endif

  return TRUE;
}

//Class used by EnumerateInstances
struct _NTSERV_EnumerateInstancesInfo
{
//Constructors / Destructors
  _NTSERV_EnumerateInstancesInfo() : pServiceNames(NULL), 
                                     psAppPath(NULL),
                                     pManager(NULL) 
  {
  };

//Member variables
  CNTServiceStringArray*    pServiceNames; //The array to populate
  CNTServiceString*         psAppPath;     //The full path to this exe
  CNTServiceControlManager* pManager;      //Pointer to the controlling manager
};

BOOL CNTService::EnumerateInstances(CNTServiceStringArray& ServiceNames, DWORD& dwError)
{
  //By default empty the error message 
  dwError = ERROR_SUCCESS;

  //Empty the array before we go any further
#ifdef CNTSERVICE_MFC_EXTENSIONS
  ServiceNames.RemoveAll();
#else
  ServiceNames.clear();
#endif

  //Open up the SCM requesting enumeration rights
  CNTServiceControlManager manager;
  if (!manager.Open(NULL, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE))
  {
    dwError = GetLastError();
    return FALSE;
  }

  //Get the full path to this exe
  TCHAR szAppPath[_MAX_PATH];
  szAppPath[0] = _T('\0');
  if (!GetModuleFileName(NULL, szAppPath, _MAX_PATH))
  {
    dwError = GetLastError();
    return FALSE;
  }
  CNTServiceString sAppPath(szAppPath);
#ifdef CNTSERVICE_MFC_EXTENSIONS
  sAppPath.MakeUpper(); //Comparisons will be case insensitive
#else
  std::transform(sAppPath.begin(), sAppPath.end(), sAppPath.begin(), _totupper);
#endif

  //Create the info struct we will pass to the callback function
  _NTSERV_EnumerateInstancesInfo info;
  info.pServiceNames = &ServiceNames;
  info.psAppPath = &sAppPath;
  info.pManager = &manager;

  //Finally call the enumeration function
  return manager.EnumServices(SERVICE_WIN32, SERVICE_STATE_ALL, &info, _EnumerateInstances);
}

BOOL CALLBACK CNTService::_EnumerateInstances(void* pData, ENUM_SERVICE_STATUS& ss)
{
  //Validate our parameters
  ATLASSERT(pData != NULL);

  _NTSERV_EnumerateInstancesInfo* pInfo = static_cast<_NTSERV_EnumerateInstancesInfo*>(pData);
  ATLASSUME(pInfo != NULL);
  ATLASSUME(pInfo->pManager != NULL);
  ATLASSUME(pInfo->psAppPath != NULL);
  ATLASSUME(pInfo->pServiceNames != NULL);

  //Open up the service to get its configuration
  CNTScmService service;
  if (pInfo->pManager->OpenService(ss.lpServiceName, SERVICE_QUERY_CONFIG, service))
  {
    //Now that we have the service opened, query it configuration to get its binary path name
    LPQUERY_SERVICE_CONFIG lpServiceConfig = NULL;
    if (service.QueryConfig(lpServiceConfig))
    {
      ATLASSUME(lpServiceConfig != NULL);

      //If the binary path in the SCM contains our path, then add this found service to the instance array
      CNTServiceString sBinaryPath(lpServiceConfig->lpBinaryPathName);
    #ifdef CNTSERVICE_MFC_EXTENSIONS
      sBinaryPath.MakeUpper();
      if (sBinaryPath.Find(*pInfo->psAppPath) != -1)
    #else
      std::transform(sBinaryPath.begin(), sBinaryPath.end(), sBinaryPath.begin(), _totupper);
      if (sBinaryPath.find(*pInfo->psAppPath) != CNTServiceString::npos)
    #endif
      {
      #ifdef CNTSERVICE_MFC_EXTENSIONS
        pInfo->pServiceNames->Add(ss.lpServiceName);
      #else
        pInfo->pServiceNames->push_back(ss.lpServiceName);
      #endif
      }
    }
    else
      ATLTRACE(_T("CNTService::_EnumerateInstances, Failed to query service configuration, Service:%s\n"), ss.lpServiceName);

    //Tidy up any used heap memory
    if (lpServiceConfig)
      delete [] lpServiceConfig;
  }
  else
    ATLTRACE(_T("CNTService::_EnumerateInstances, Failed to open service, Service:%s\n"), ss.lpServiceName);

  //Always continue enumeration
  return TRUE;
}

BOOL CNTService::Uninstall(CNTServiceString& sErrorMsg, DWORD& dwError, DWORD dwTimeToWaitForStop)
{
  //By default empty the error message 
#ifdef CNTSERVICE_MFC_EXTENSIONS
  sErrorMsg.Empty();
#else
  sErrorMsg.clear();
#endif
  dwError = ERROR_SUCCESS;

  //Open up the SCM requesting connect rights
  CNTServiceControlManager manager;
  if (!manager.Open(NULL, SC_MANAGER_CONNECT))
  {
    dwError = GetLastError();
    sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_UNINSTALL_FAIL_OPEN_SCM), dwError, FALSE);
    TraceMessage(sErrorMsg);
    if (m_bEventLogSource)
    {
      CNTServiceString sError(Win32ErrorToString(MapResourceID(IDS_NTSERV_UNINSTALL_FAIL_OPEN_SCM), dwError, TRUE));
    #ifdef CNTSERVICE_MFC_EXTENSIONS
      m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_CONNECT_SCM), sError);
    #else
      m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_CONNECT_SCM), sError.c_str());
    #endif
    }
    return FALSE;
  }

  //Open up the existing service requesting deletion rights
  CNTScmService service;
#ifdef CNTSERVICE_MFC_EXTENSIONS
  if (!manager.OpenService(m_sServiceName, DELETE | SERVICE_STOP | SERVICE_QUERY_STATUS, service))
#else
  if (!manager.OpenService(m_sServiceName.c_str(), DELETE | SERVICE_STOP | SERVICE_QUERY_STATUS, service))
#endif
  {
    dwError = GetLastError();
    sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_UNINSTALL_FAIL_OPEN_SERVICE), dwError, FALSE);
    TraceMessage(sErrorMsg);
    if (m_bEventLogSource)
    {
      CNTServiceString sError(Win32ErrorToString(MapResourceID(IDS_NTSERV_UNINSTALL_FAIL_OPEN_SERVICE), dwError, TRUE));
    #ifdef CNTSERVICE_MFC_EXTENSIONS
      m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_OPEN_SERVICE), sError);
    #else
      m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_OPEN_SERVICE), sError.c_str());
    #endif
    }
    return FALSE;
  }

  //Ask the service to stop if it is running
  SERVICE_STATUS status;
  if (service.QueryStatus(status) && status.dwCurrentState != SERVICE_STOPPED)
  {
    if (!service.Stop())
    {
      dwError = GetLastError();
      sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_UNINSTALL_FAIL_STOP), dwError, FALSE);
      TraceMessage(sErrorMsg);
    }

    //Wait for the service to stop
    if (!service.WaitForStop(dwTimeToWaitForStop))
    {
      dwError = GetLastError();
      sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_UNINSTALL_FAIL_WAIT_FOR_STOP), dwError, FALSE);
      TraceMessage(sErrorMsg);
      return FALSE;
    }
  }

  //Delete the service from the SCM database
  if (!service.Delete())
  {
    dwError = GetLastError();
    sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_UNINSTALL_FAIL_DELETE_SERVICE), dwError, FALSE);
    TraceMessage(sErrorMsg);
    if (m_bEventLogSource)
    {
      CNTServiceString sError(Win32ErrorToString(MapResourceID(IDS_NTSERV_UNINSTALL_FAIL_DELETE_SERVICE), dwError, TRUE));
    #ifdef CNTSERVICE_MFC_EXTENSIONS
      m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_DELETE_SERVICE), sError);
    #else
      m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_DELETE_SERVICE), sError.c_str());
    #endif
    }
    return FALSE;
  }

  if (m_bEventLogSource)
  {
    //Remove this service as an event log source
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    if (!m_EventLogSource.Uninstall(m_EventLogSource.GetLogName(), m_EventLogSource.GetSourceName()))
  #else
    if (!m_EventLogSource.Uninstall(m_EventLogSource.GetLogName().c_str(), m_EventLogSource.GetSourceName().c_str()))
  #endif
    {
      dwError = GetLastError();
      sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_UNINSTALL_FAIL_UNINSTALL_EVENTLOG_SOURCE), dwError, FALSE);
      TraceMessage(sErrorMsg);
      return FALSE;
    }
  }

#ifdef CNTSERVICE_MFC_EXTENSIONS
  ATLTRACE(_T("CNTService::Uninstall, Service: %s was succesfully uninstalled\n"), m_sServiceName.operator LPCTSTR());
#else
  ATLTRACE(_T("CNTService::Uninstall, Service: %s was succesfully uninstalled\n"), m_sServiceName.c_str());
#endif

  return TRUE;
}

BOOL CNTService::SetServiceToStatus(ServiceAction esaStatusWanted, DWORD& dwError, DWORD dwTimeout, DWORD dwPollingInterval)
{
  DWORD dwStatusWanted = 0;
  DWORD dwRightNeeded = 0;

  switch (esaStatusWanted) 
  {
    case ActionStartService:
    {
      dwStatusWanted = SERVICE_RUNNING;
      dwRightNeeded = SERVICE_START;
      break;
    }
    case ActionPauseService:
    {
      dwStatusWanted = SERVICE_PAUSED;
      dwRightNeeded = SERVICE_PAUSE_CONTINUE;
      break;
    }
    case ActionContinueService:
    {
      dwStatusWanted = SERVICE_RUNNING;
      dwRightNeeded = SERVICE_PAUSE_CONTINUE;
      break;
    }
    case ActionStopService:
    {
      dwStatusWanted = SERVICE_STOPPED;
      dwRightNeeded = SERVICE_STOP;
      break;
    }
    default:
    {
      ATLASSERT(FALSE);  //unknown status wanted
      break;
    }
  }

  //Open up the SCM requesting connect rights
  CNTServiceControlManager manager;
  if (!manager.Open(NULL, SC_MANAGER_CONNECT))
  {
    dwError = GetLastError();
    CNTServiceString sErrorMsg(Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_OPEN_SCM), dwError, FALSE));
    TraceMessage(sErrorMsg);
    if (m_bEventLogSource)
    {
      sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_OPEN_SCM), dwError, TRUE);
    #ifdef CNTSERVICE_MFC_EXTENSIONS
      m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_CONNECT_SCM), sErrorMsg);
    #else
      m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_CONNECT_SCM), sErrorMsg.c_str());
    #endif
    }
    return FALSE;
  }

  //Open up the existing service requesting the necessary rights
  CNTScmService service;
#ifdef CNTSERVICE_MFC_EXTENSIONS
  if (!manager.OpenService(m_sServiceName, dwRightNeeded | SERVICE_QUERY_STATUS, service))
#else
  if (!manager.OpenService(m_sServiceName.c_str(), dwRightNeeded | SERVICE_QUERY_STATUS, service))
#endif
  {
    dwError = GetLastError();
    CNTServiceString sErrorMsg(Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_OPEN_SERVICE), dwError, FALSE));
    TraceMessage(sErrorMsg);
    if (m_bEventLogSource)
    {
      sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_OPEN_SERVICE), dwError, TRUE);
    #ifdef CNTSERVICE_MFC_EXTENSIONS
      m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_OPEN_SERVICE), sErrorMsg);
    #else
      m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_OPEN_SERVICE), sErrorMsg.c_str());
    #endif
    }
    return FALSE;
  }

  //Ask the service to change to the requested status if it is not already in that status
  SERVICE_STATUS status;
  if (service.QueryStatus(status))
  {
    if (status.dwCurrentState != dwStatusWanted)
    {
      BOOL bSuccess = FALSE;
      switch (esaStatusWanted) 
      {
        case ActionStartService:
        {
          bSuccess = service.Start(0, NULL);
          if (!bSuccess)
          {
            dwError = GetLastError();
            CNTServiceString sErrorMsg(Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_START), dwError, FALSE));
            TraceMessage(sErrorMsg);
            if (m_bEventLogSource)
            {
              sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_START), dwError, TRUE);
            #ifdef CNTSERVICE_MFC_EXTENSIONS
              m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_START), sErrorMsg);
            #else
              m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_START), sErrorMsg.c_str());
            #endif
            }
          }
          break;
        }
        case ActionPauseService:
        {
          bSuccess = service.Pause();
          if (!bSuccess)
          {
            dwError = GetLastError();
            CNTServiceString sErrorMsg(Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_PAUSE), dwError, FALSE));
            TraceMessage(sErrorMsg);
            if (m_bEventLogSource)
            {
              sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_PAUSE), dwError, TRUE);
            #ifdef CNTSERVICE_MFC_EXTENSIONS
              m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_PAUSE), sErrorMsg);
            #else
              m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_PAUSE), sErrorMsg.c_str());
            #endif
            }
          }
          break;
        }
        case ActionContinueService:
        {
          bSuccess = service.Continue();
          if (!bSuccess)
          {
            dwError = GetLastError();
            CNTServiceString sErrorMsg(Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_CONTINUE), dwError, FALSE));
            TraceMessage(sErrorMsg);
            if (m_bEventLogSource)
            {
              sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_CONTINUE), dwError, TRUE);
            #ifdef CNTSERVICE_MFC_EXTENSIONS
              m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_CONTINUE), sErrorMsg);
            #else
              m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_CONTINUE), sErrorMsg.c_str());
            #endif
            }
          }
          break;
        }
        case ActionStopService:
        {
          bSuccess = service.Stop();
          if (!bSuccess)
          {
            dwError = GetLastError();
            CNTServiceString sErrorMsg(Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_STOP), dwError, FALSE));
            TraceMessage(sErrorMsg);
            if (m_bEventLogSource)
            {
              sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_STOP), dwError, TRUE);
            #ifdef CNTSERVICE_MFC_EXTENSIONS
              m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_STOP), sErrorMsg);
            #else
              m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_STOP), sErrorMsg.c_str());
            #endif
            }
          }
          break;
        }
        default:
        {
          ATLASSERT(FALSE);
          break;
        }
      }

      if (!bSuccess)
        return FALSE;
    }

    //Wait for the service to have the status wanted
    if (!service.WaitForServiceStatus(dwTimeout, dwStatusWanted, dwPollingInterval))
    {
      dwError = GetLastError();
      CNTServiceString sErrorMsg(Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_WAITFORCHANGE), dwError, FALSE));
      TraceMessage(sErrorMsg);
      if (m_bEventLogSource)
      {
        sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_WAITFORCHANGE), dwError, TRUE);
      #ifdef CNTSERVICE_MFC_EXTENSIONS
        m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_CHANGE_STATE), sErrorMsg, dwStatusWanted, FALSE);
      #else
        m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_CHANGE_STATE), sErrorMsg.c_str(), dwStatusWanted, FALSE);
      #endif
      }

      return FALSE;
    }
  }
  else
  {
    dwError = GetLastError();
    CNTServiceString sErrorMsg(Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_QUERYSTATUS), dwError, FALSE));
    TraceMessage(sErrorMsg);
    if (m_bEventLogSource)
    {
      sErrorMsg = Win32ErrorToString(MapResourceID(IDS_NTSERV_SETSERVICETOSTATUS_FAIL_QUERYSTATUS), dwError, TRUE);
    #ifdef CNTSERVICE_MFC_EXTENSIONS
      m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_QUERY_STATUS), sErrorMsg);
    #else
      m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, MapMessageID(CNTS_MSG_SERVICE_FAIL_QUERY_STATUS), sErrorMsg.c_str());
    #endif
    }
    return FALSE;
  }

#ifdef CNTSERVICE_MFC_EXTENSIONS
  ATLTRACE(_T("CNTService::SetServiceToStatus, Service: %s was succesfully put to the new status\n"), m_sServiceName.operator LPCTSTR());
#else
  ATLTRACE(_T("CNTService::SetServiceToStatus, Service: %s was succesfully put to the new status\n"), m_sServiceName.c_str());
#endif

  return TRUE;
}

void CNTService::Debug()
{
  //Runing as EXE not as service, just execute the services
  //SeviceMain function. 
  
  //Note we pass in all parameters except for the "-debug" 
  //string which brought us here.
  DWORD argc = 0;
  ATL::CHeapPtr<LPTSTR> pArgv;
  if (!pArgv.Allocate(__argc))
    TerminateService(ERROR_OUTOFMEMORY, 0);
  
  for (int n=0; n<__argc; ++n)
  {
    if (_tcsicmp(__targv[n], _T("-debug")) && _tcsicmp(__targv[n], _T("/debug")) && 
        _tcsicmp(__targv[n], _T("-app")) && _tcsicmp(__targv[n], _T("/app")) &&
        _tcsicmp(__targv[n], _T("-application")) && _tcsicmp(__targv[n], _T("/application")) &&
        _tcsicmp(__targv[n], _T("-console")) && _tcsicmp(__targv[n], _T("/console")))
      pArgv.m_pData[argc++] = __targv[n];
  }

  //Register the console handler to allow the console to generate requests to simulate stopping the service
  SetConsoleCtrlHandler(_ConsoleCtrlHandler, TRUE);

  //Now simply call ServiceMain with out modified set of parameters
  ServiceMain(argc, pArgv);
}

void CNTService::ShowHelp()
{
  //Default behaviour is to do nothing. In your service application, you should override this function 
  //to either display something	helpful to the console or if the service is running in the GUI subsystem, 
  //to display a messagebox or dialog to provide info about your service.
}

//Based upon the function of the same name in CWinApp
void CNTService::ParseCommandLine(CNTServiceCommandLineInfo& rCmdInfo)
{
  for (int i = 1; i < __argc; i++)
  {
#ifdef _UNICODE
    LPCTSTR pszParam = __wargv[i];
#else
    LPCTSTR pszParam = __argv[i];
#endif
    BOOL bFlag = FALSE;
    BOOL bLast = ((i + 1) == __argc);
    if (pszParam[0] == _T('-') || pszParam[0] == _T('/'))
    {
      // remove flag specifier
      bFlag = TRUE;
      ++pszParam;
    }
    rCmdInfo.ParseParam(pszParam, bFlag, bLast);
  }
}

//Callback function to handle console control signals
BOOL WINAPI CNTService::_ConsoleCtrlHandler(DWORD dwCtrlType)
{
  //Validate our parameters
  ATLASSUME(sm_lpService != NULL);

  //Convert from the SDK world to the C++ world. In this
  //implementation we just use a single static, In the 
  //future we could use a map just like MFC does for HWND
  //to CWnd conversions
  return sm_lpService->ConsoleCtrlHandler(dwCtrlType);
}

BOOL CNTService::ConsoleCtrlHandler(DWORD dwCtrlType)
{
  //If the event is a shutdown event then call the OnStop virtual method
  if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT || dwCtrlType == CTRL_CLOSE_EVENT)
  {
    sm_lpService->OnStop();
    return TRUE;
  }
  else
    return FALSE;
}

void CNTService::DisplayMessage(const CNTServiceString& sMessage)
{
  if (m_bUseConsole)
  {
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    _tprintf(_T("%s\n"), sMessage.operator LPCTSTR());
  #else
    _tprintf(_T("%s\n"), sMessage.c_str());
  #endif
  }
  else
  {
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    AfxMessageBox(sMessage);
  #else
    MessageBox(NULL, sMessage.c_str(), m_sDisplayName.c_str(), MB_OK | MB_ICONSTOP);
  #endif
  }
}

void CNTService::TraceMessage(LPCTSTR pszMessage)
{
#ifdef _DEBUG
  ATLTRACE(_T("%s\n"), pszMessage);
#else
  UNREFERENCED_PARAMETER(pszMessage);
#endif
}

void CNTService::TraceMessage(const CNTServiceString& sMessage)
{
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    TraceMessage(sMessage.operator LPCTSTR());
  #else
    TraceMessage(sMessage.c_str());
  #endif
}

void CNTService::FormatString(CNTServiceString& sMsg, UINT nIDS, LPCTSTR psz1)
{
#ifdef CNTSERVICE_MFC_EXTENSIONS
  AfxFormatString1(sMsg, nIDS, psz1);
#else
  TCHAR szResource[4096];
  szResource[0] = _T('\0');
  LoadString(GetModuleHandle(NULL), nIDS, szResource, sizeof(szResource)/sizeof(TCHAR));
  sMsg = szResource;
  CNTServiceString::size_type nFind = sMsg.find(_T("%1"));
  if (nFind != CNTServiceString::npos)
    sMsg.replace(nFind, 2, psz1);
#endif
}

void CNTService::FormatString(CNTServiceString& sMsg, UINT nIDS, LPCTSTR psz1, LPCTSTR psz2)
{
#ifdef CNTSERVICE_MFC_EXTENSIONS
  AfxFormatString2(sMsg, nIDS, psz1, psz2);
#else
  TCHAR szResource[4096];
  szResource[0] = _T('\0');
  LoadString(GetModuleHandle(NULL), nIDS, szResource, sizeof(szResource)/sizeof(TCHAR));
  sMsg = szResource;
  CNTServiceString::size_type nFind = sMsg.find(_T("%1"));
  if (nFind != CNTServiceString::npos)
    sMsg.replace(nFind, 2, psz1);
  nFind = sMsg.find(_T("%2"));
  if (nFind != CNTServiceString::npos)
    sMsg.replace(nFind, 2, psz2);
#endif
}

//Based upon the function of the same name in CWinApp
DWORD CNTService::ProcessShellCommand(CNTServiceCommandLineInfo& rCmdInfo)
{
  //Update the service name if allowed and provided on the command line
#ifdef CNTSERVICE_MFC_EXTENSIONS
  if (m_bAllowNameChange && rCmdInfo.m_sServiceName.GetLength())
#else
  if (m_bAllowNameChange && rCmdInfo.m_sServiceName.length())
#endif
    m_sServiceName = rCmdInfo.m_sServiceName;

  //Update the display name if allowed and provided on the command line
#ifdef CNTSERVICE_MFC_EXTENSIONS
  if (m_bAllowDisplayNameChange && rCmdInfo.m_sServiceDisplayName.GetLength())
#else
  if (m_bAllowDisplayNameChange && rCmdInfo.m_sServiceDisplayName.length())
#endif
  {
    m_sDisplayName = rCmdInfo.m_sServiceDisplayName;

    //Also by default update the event log source name 
  #ifdef CNTSERVICE_MFC_EXTENSIONS
    m_EventLogSource.SetSourceName(m_sDisplayName);
  #else
    m_EventLogSource.SetSourceName(m_sDisplayName.c_str());
  #endif
  }

  //Update the description string if allowed and provided on the command line
#ifdef CNTSERVICE_MFC_EXTENSIONS
  if (m_bAllowDescriptionChange && rCmdInfo.m_sServiceDescription.GetLength())
#else
  if (m_bAllowDescriptionChange && rCmdInfo.m_sServiceDescription.length())
#endif
    m_sDescription = rCmdInfo.m_sServiceDescription;

  //What will be the return value from this function
  DWORD dwError = ERROR_SUCCESS;

  switch (rCmdInfo.m_nShellCommand)
  {
    case CNTServiceCommandLineInfo::RunAsService:
    {
      if (!Run())
        dwError = GetLastError();
      break;
    }
    case CNTServiceCommandLineInfo::StartTheService:
    {
      //Display the error message if the install failed
      if (!SetServiceToStatus(ActionStartService, dwError, rCmdInfo.m_dwTimeout))
      {
        CNTServiceString sErrorMsg(Win32ErrorToString(MapResourceID(IDS_NTSERV_FAIL_START_SERVICE), dwError, FALSE));
        if (!rCmdInfo.m_bSilent)
          DisplayMessage(sErrorMsg);
      }
      break;
    }
    case CNTServiceCommandLineInfo::PauseService:
    {
      //Display the error message if the install failed
      if (!SetServiceToStatus(ActionPauseService, dwError, rCmdInfo.m_dwTimeout))
      {
        CNTServiceString sErrorMsg(Win32ErrorToString(MapResourceID(IDS_NTSERV_FAIL_PAUSE_SERVICE), dwError, FALSE));
        if (!rCmdInfo.m_bSilent)
          DisplayMessage(sErrorMsg);
      }
      break;
    }
    case CNTServiceCommandLineInfo::ContinueService:
    {
      //Display the error message if the install failed
      if (!SetServiceToStatus(ActionContinueService, dwError, rCmdInfo.m_dwTimeout))
      {
        CNTServiceString sErrorMsg(Win32ErrorToString(MapResourceID(IDS_NTSERV_FAIL_CONTINUE_SERVICE), dwError, FALSE));
        if (!rCmdInfo.m_bSilent)
          DisplayMessage(sErrorMsg);
      }
      break;
    }
    case CNTServiceCommandLineInfo::StopService:
    {
      //Display the error message if the install failed
      if (!SetServiceToStatus(ActionStopService, dwError, rCmdInfo.m_dwTimeout))
      {
        CNTServiceString sErrorMsg(Win32ErrorToString(MapResourceID(IDS_NTSERV_FAIL_STOP_SERVICE), dwError, FALSE));
        if (!rCmdInfo.m_bSilent)
          DisplayMessage(sErrorMsg);
      }
      break;
    }
    case CNTServiceCommandLineInfo::InstallService:
    {
      //Form the command line options which we use for the command line when 
      //registering the service
      CNTServiceString sCommandLine;

      //First get the full path of this exe
      TCHAR szAppPath[_MAX_PATH];
      szAppPath[0] = _T('\0');
      if (GetModuleFileName(NULL, szAppPath, _MAX_PATH))
      {
        m_sBinaryPathName = szAppPath;

        //Always ensure the sevice path is quoted if there are spaces in it
      #ifdef CNTSERVICE_MFC_EXTENSIONS
        if (m_sBinaryPathName.Find(_T(' ')) != -1)
      #else
        if (std::find(m_sBinaryPathName.begin(), m_sBinaryPathName.end(), _T(' ')) != m_sBinaryPathName.end())
      #endif
          m_sBinaryPathName = _T('\"') + m_sBinaryPathName + _T('\"');

        //Add in the pertinent options we have already parsed from the command line
      #ifdef CNTSERVICE_MFC_EXTENSIONS
        if (m_bAllowNameChange && rCmdInfo.m_sServiceName.GetLength())
      #else
        if (m_bAllowNameChange && rCmdInfo.m_sServiceName.length())
      #endif
        {
          TCHAR szName[2048];
        #ifdef CNTSERVICE_MFC_EXTENSIONS
          _stprintf_s(szName, sizeof(szName)/sizeof(TCHAR), _T(" \"/SN:%s\""), rCmdInfo.m_sServiceName.operator LPCTSTR());
        #else
          _stprintf_s(szName, sizeof(szName)/sizeof(TCHAR), _T(" \"/SN:%s\""), rCmdInfo.m_sServiceName.c_str());
        #endif
          m_sBinaryPathName += szName;
        }
      #ifdef CNTSERVICE_MFC_EXTENSIONS
        if (m_bAllowDisplayNameChange && rCmdInfo.m_sServiceDisplayName.GetLength())
      #else
        if (m_bAllowDisplayNameChange && rCmdInfo.m_sServiceDisplayName.length())
      #endif
        {
          TCHAR szDisplayName[2048];
        #ifdef CNTSERVICE_MFC_EXTENSIONS
          _stprintf_s(szDisplayName, sizeof(szDisplayName)/sizeof(TCHAR), _T(" \"/SDN:%s\""), rCmdInfo.m_sServiceDisplayName.operator LPCTSTR());
        #else
          _stprintf_s(szDisplayName, sizeof(szDisplayName)/sizeof(TCHAR), _T(" \"/SDN:%s\""), rCmdInfo.m_sServiceDisplayName.c_str());
        #endif
          m_sBinaryPathName += szDisplayName;
        }
      #ifdef CNTSERVICE_MFC_EXTENSIONS
        if (m_bAllowDescriptionChange && rCmdInfo.m_sServiceDescription.GetLength())
      #else
        if (m_bAllowDescriptionChange && rCmdInfo.m_sServiceDescription.length())
      #endif
        {
          TCHAR szDescription[2048];
        #ifdef CNTSERVICE_MFC_EXTENSIONS
          _stprintf_s(szDescription, sizeof(szDescription)/sizeof(TCHAR), _T(" \"/SD:%s\""), rCmdInfo.m_sServiceDescription.operator LPCTSTR());
        #else
          _stprintf_s(szDescription, sizeof(szDescription)/sizeof(TCHAR), _T(" \"/SD:%s\""), rCmdInfo.m_sServiceDescription.c_str());
        #endif
          m_sBinaryPathName += szDescription;
        }

        //If a service command line is used, then use it for registration
      #ifdef CNTSERVICE_MFC_EXTENSIONS
        if (m_bAllowCommandLine && rCmdInfo.m_sServiceCommandLine.GetLength())
      #else
        if (m_bAllowCommandLine && rCmdInfo.m_sServiceCommandLine.length())
      #endif
        {
          m_sBinaryPathName += _T(" ");
          m_sBinaryPathName += rCmdInfo.m_sServiceCommandLine;
        }

        //If a username was specified, then use it for registration
      #ifdef CNTSERVICE_MFC_EXTENSIONS
        if (rCmdInfo.m_sUserName.GetLength())
      #else
        if (rCmdInfo.m_sUserName.length())
      #endif
        {
          m_sUserName = rCmdInfo.m_sUserName;

          //As a security precaution nuke the command line copy of the username now that we have used it
          SecureEmptyString(rCmdInfo.m_sUserName);
        }

        //If a password was specified, then use it for registration
      #ifdef CNTSERVICE_MFC_EXTENSIONS
        if (rCmdInfo.m_sPassword.GetLength())
      #else
        if (rCmdInfo.m_sPassword.length())
      #endif
        {
          m_sPassword = rCmdInfo.m_sPassword;

          //As a security precaution nuke the command line copy of the username now that we have used it
          SecureEmptyString(rCmdInfo.m_sPassword);
        }

        //Should the service be started automatically
        if (rCmdInfo.m_bAutoStart)
          m_dwStartType = SERVICE_AUTO_START;

        //Display the error message if the install failed
        CNTServiceString sErrorMsg;
        if (!Install(sErrorMsg, dwError) && !rCmdInfo.m_bSilent)
          DisplayMessage(sErrorMsg);
      }
      else
      {
        CNTServiceString sErrorMsg(Win32ErrorToString(MapResourceID(IDS_NTSERV_FAIL_GET_MODULE_FILENAME), dwError, FALSE));
        if (!rCmdInfo.m_bSilent)
          DisplayMessage(sErrorMsg);
      }
      break;
    }
    case CNTServiceCommandLineInfo::UninstallService:
    {
      //Display the error message if the uninstall failed
      CNTServiceString sErrorMsg;
      if (!Uninstall(sErrorMsg, dwError, rCmdInfo.m_dwTimeout) && !rCmdInfo.m_bSilent)
        DisplayMessage(sErrorMsg);
      break;
    }
    case CNTServiceCommandLineInfo::DebugService:
    {
      try
      {
        Debug();
      }
      catch(CNTServiceTerminateException& e)
      {
        dwError = e.m_dwWin32ExitCode;
      }

      break;
    }
    case CNTServiceCommandLineInfo::ShowServiceHelp:
    {
      ShowHelp();
      break;
    }
    default:
    {
      ATLASSERT(FALSE);
      break;
    }
  }

  return dwError;
}

CNTServiceString CNTService::GetErrorMessage(DWORD dwError)
{
  //What will be the return value from this function
  CNTServiceString sError;

  //Look up the error description using FormatMessage
  LPTSTR lpBuffer;
  if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT), reinterpret_cast<LPTSTR>(&lpBuffer), 0, NULL) != 0)
  {
    sError = lpBuffer;
    LocalFree(lpBuffer);
  }

  return sError;
}

CNTServiceString CNTService::Win32ErrorToString(UINT nStringID, DWORD dwError, BOOL bEventLog)
{
  //What will be the return value from this function
  CNTServiceString sMsg;

  if (bEventLog)
  {
    switch (m_ELLoggingDetail)
    {
      case EL_ErrorCode:
      {
        TCHAR szTemp[32];
        _stprintf_s(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%u"), dwError);
        sMsg = szTemp;
        break;
      }
      case EL_ErrorDescription:
      {
        sMsg = GetErrorMessage(dwError);
        break;
      }
      case EL_ErrorCodeAndErrorDescription:
      {
        CNTServiceString sDescription(GetErrorMessage(dwError));

        TCHAR szErrorCode[32];
        _stprintf_s(szErrorCode, sizeof(szErrorCode)/sizeof(TCHAR), _T("%u"), dwError);

      #ifdef CNTSERVICE_MFC_EXTENSIONS      
        FormatString(sMsg, MapResourceID(IDS_NTSERV_VERBOSE_ERROR_STRING), szErrorCode, sDescription);
      #else
        FormatString(sMsg, MapResourceID(IDS_NTSERV_VERBOSE_ERROR_STRING), szErrorCode, sDescription.c_str());
      #endif
        break;
      }
      default:
      {
        ATLASSERT(FALSE);
        break;
      }
    }
  }
  else
  {
    switch (m_UILoggingDetail)
    {
      case UI_ErrorCode:
      {
        TCHAR szTemp[32];
        szTemp[0] = _T('\0');
        _stprintf_s(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%u"), dwError);
        sMsg = szTemp;
        break;
      }
      case UI_ErrorDescription:
      {
        sMsg = GetErrorMessage(dwError);
        break;
      }
      case UI_ErrorCodeAndErrorDescription:
      {
        CNTServiceString sDescription(GetErrorMessage(dwError));

        TCHAR szErrorCode[32];
        szErrorCode[0] = _T('\0');
        _stprintf_s(szErrorCode, sizeof(szErrorCode)/sizeof(TCHAR), _T("%u"), dwError);
      #ifdef CNTSERVICE_MFC_EXTENSIONS
        FormatString(sMsg, MapResourceID(IDS_NTSERV_VERBOSE_ERROR_STRING), szErrorCode, sDescription);
      #else
        FormatString(sMsg, MapResourceID(IDS_NTSERV_VERBOSE_ERROR_STRING), szErrorCode, sDescription.c_str());
      #endif
        break;
      }
      case UI_StringAndErrorCode:
      {
        TCHAR szError[32];
        szError[0] = _T('\0');
        _stprintf_s(szError, sizeof(szError)/sizeof(TCHAR), _T("%u"), dwError);

        FormatString(sMsg, nStringID, szError);
        break;
      }
      case UI_StringAndErrorDescription:
      {
        CNTServiceString sDescription(GetErrorMessage(dwError));

      #ifdef CNTSERVICE_MFC_EXTENSIONS
        FormatString(sMsg, nStringID, sDescription);
      #else
        FormatString(sMsg, nStringID, sDescription.c_str());
      #endif
        break;
      }
      case UI_StringAndErrorCodeAndErrorDescription:
      {
        CNTServiceString sDescription(GetErrorMessage(dwError));

        TCHAR szError[32];
        szError[0] = _T('\0');
        _stprintf_s(szError, sizeof(szError)/sizeof(TCHAR), _T("%u"), dwError);

        CNTServiceString sErrorAndDescription;
      #ifdef CNTSERVICE_MFC_EXTENSIONS    
        FormatString(sErrorAndDescription, MapResourceID(IDS_NTSERV_VERBOSE_ERROR_STRING), szError, sDescription);
        FormatString(sMsg, nStringID, sErrorAndDescription);
      #else
        FormatString(sErrorAndDescription, MapResourceID(IDS_NTSERV_VERBOSE_ERROR_STRING), szError, sDescription.c_str());
        FormatString(sMsg, nStringID, sErrorAndDescription.c_str());
      #endif
        break;
      }
      default:
      {
        ATLASSERT(FALSE);
        break;
      }
    }
  }

  return sMsg;
}

UINT CNTService::MapResourceID(UINT nID)
{
  //Just return the same value as what was asked for. Derived classes can customize the values 
  //for their own use
  return nID;
}

UINT CNTService::MapMessageID(UINT nID)
{
  //Just return the same value as what was asked for. Derived classes can customize the values 
  //for their own use
  return nID;
}

BOOL CNTService::QueryServiceDynamicInformation(DWORD dwInfoLevel, PVOID* ppDynamicInfo)
{
  //Check to see if the function pointer is available
  if (m_lpfnQueryServiceDynamicInformation == NULL)
  {
    ATLTRACE(_T("CNTService::EnumServices, QueryServiceDynamicInformation function is not supported on this OS. You need to be running at least Windows 8 / Windows Server 2012 to use this function\n"));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
  }

  //Call through the function pointer
  return m_lpfnQueryServiceDynamicInformation(m_hStatus, dwInfoLevel, ppDynamicInfo);
}

