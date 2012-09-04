# Microsoft Developer Studio Project File - Name="vscpservice" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=vscpservice - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vscpservice.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vscpservice.mak" CFG="vscpservice - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vscpservice - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "vscpservice - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vscpservice - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "release"
# PROP Intermediate_Dir "release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "_CONSOLE" /D "_WIN32" /D _WIN32_WINNT=0x0500 /D "WIN32" /D "_MBCS" /D "_AFXDLL" /D "BUILD_VSCPD_SERVICE" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib /nologo /subsystem:console /machine:I386 /libpath:"$(wxwin)\lib\vc_lib"
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy release\*.exe ..\delivery	copy release\*.exe $(WINDIR)\system32
# End Special Build Tool

!ELSEIF  "$(CFG)" == "vscpservice - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "debug"
# PROP Intermediate_Dir "debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MDd /W3 /Gm /GX /ZI /Od /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "_WIN32" /D _WIN32_WINNT=0x0500 /D "_MBCS" /D "WIN32" /D "_DEBUG" /D "VSCP" /D "WINDOWS" /D "_AFXDLL" /D "BUILD_VSCPD_SERVICE" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"$(wxwin)\lib\vc_lib"
# SUBTRACT LINK32 /incremental:no

!ENDIF 

# Begin Target

# Name "vscpservice - Win32 Release"
# Name "vscpservice - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\common\clientlist.cpp
# End Source File
# Begin Source File

SOURCE=..\common\controlobject.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\crc.c
# End Source File
# Begin Source File

SOURCE=..\common\daemonvscp.cpp
# End Source File
# Begin Source File

SOURCE=..\common\devicelist.cpp
# End Source File
# Begin Source File

SOURCE=..\common\devicethread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\dllist.c
# End Source File
# Begin Source File

SOURCE=..\common\dm.cpp
# End Source File
# Begin Source File

SOURCE=..\common\interfacelist.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\md5.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\md5.h
# End Source File
# Begin Source File

SOURCE=..\..\common\NTService.cpp
# End Source File
# Begin Source File

SOURCE=..\common\tcpipclientthread.cpp
# End Source File
# Begin Source File

SOURCE=..\common\udpthread.cpp
# End Source File
# Begin Source File

SOURCE=..\common\userlist.cpp
# End Source File
# Begin Source File

SOURCE=..\common\vscphelper.cpp
# End Source File
# Begin Source File

SOURCE=..\common\vscphelper.h
# End Source File
# Begin Source File

SOURCE=.\VSCPServApp.cpp
# End Source File
# Begin Source File

SOURCE=.\vscpservice.cpp
# End Source File
# Begin Source File

SOURCE=..\common\vscpvariable.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\common\canal.h
# End Source File
# Begin Source File

SOURCE=..\common\canal_macro.h
# End Source File
# Begin Source File

SOURCE=..\common\canal_win32_ipc.h
# End Source File
# Begin Source File

SOURCE=..\common\canaldlldef.h
# End Source File
# Begin Source File

SOURCE=..\common\canalshmem_level1_win32.h
# End Source File
# Begin Source File

SOURCE=..\common\canalshmem_level2_win32.h
# End Source File
# Begin Source File

SOURCE=..\common\clientlist.h
# End Source File
# Begin Source File

SOURCE=..\common\controlobject.h
# End Source File
# Begin Source File

SOURCE=..\common\devicelist.h
# End Source File
# Begin Source File

SOURCE=..\..\common\NTService.h
# End Source File
# Begin Source File

SOURCE=..\..\common\ntservmsg.h
# End Source File
# Begin Source File

SOURCE=..\common\options.h
# End Source File
# Begin Source File

SOURCE=..\common\pipecmd.h
# End Source File
# Begin Source File

SOURCE=..\VSCPDiagnostic\Resource.h
# End Source File
# Begin Source File

SOURCE=..\..\vscp\common\vscp.h
# End Source File
# Begin Source File

SOURCE=..\..\vscp\common\vscp_classes.h
# End Source File
# Begin Source File

SOURCE=..\..\vscp\common\vscp_types.h
# End Source File
# Begin Source File

SOURCE=.\VSCPServApp.h
# End Source File
# Begin Source File

SOURCE=.\vscpservice.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\common\ntservmsg.rc
# End Source File
# Begin Source File

SOURCE=.\VSCPServApp.rc
# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Target
# End Project
