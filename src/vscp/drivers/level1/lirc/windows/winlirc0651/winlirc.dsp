# Microsoft Developer Studio Project File - Name="winlirc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=winlirc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "winlirc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "winlirc.mak" CFG="winlirc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "winlirc - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "winlirc - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "winlirc - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ws2_32.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "winlirc - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "winlirc - Win32 Release"
# Name "winlirc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\confdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\config.cpp
# End Source File
# Begin Source File

SOURCE=.\drvdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dumpcfg.cpp
# End Source File
# Begin Source File

SOURCE=.\globals.cpp
# End Source File
# Begin Source File

SOURCE=.\irconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\irdriver.cpp
# End Source File
# Begin Source File

SOURCE=.\learndlg.cpp
# End Source File
# Begin Source File

SOURCE=.\remote.cpp
# End Source File
# Begin Source File

SOURCE=.\server.cpp
# End Source File
# Begin Source File

SOURCE=.\Trayicon.cpp
# End Source File
# Begin Source File

SOURCE=.\winlirc.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\confdlg.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\drvdlg.h
# End Source File
# Begin Source File

SOURCE=.\dumpcfg.h
# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\irconfig.h
# End Source File
# Begin Source File

SOURCE=.\irdriver.h
# End Source File
# Begin Source File

SOURCE=.\learndlg.h
# End Source File
# Begin Source File

SOURCE=.\remote.h
# End Source File
# Begin Source File

SOURCE=.\server.h
# End Source File
# Begin Source File

SOURCE=.\Trayicon.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# Begin Source File

SOURCE=.\winlirc.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\l_error.ico
# End Source File
# Begin Source File

SOURCE=.\l_init.ico
# End Source File
# Begin Source File

SOURCE=.\l_ok.ico
# End Source File
# Begin Source File

SOURCE=.\l_recv.ico
# End Source File
# Begin Source File

SOURCE=.\l_send.ico
# End Source File
# Begin Source File

SOURCE=.\lirc.ico
# End Source File
# Begin Source File

SOURCE=.\winlirc.rc
# End Source File
# End Group
# End Target
# End Project
