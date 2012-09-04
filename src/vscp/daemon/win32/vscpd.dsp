# Microsoft Developer Studio Project File - Name="vscpd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=vscpd - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vscpd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vscpd.mak" CFG="vscpd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vscpd - Win32 Release DLL" (based on "Win32 (x86) Application")
!MESSAGE "vscpd - Win32 Debug DLL" (based on "Win32 (x86) Application")
!MESSAGE "vscpd - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "vscpd - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vscpd - Win32 Release DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseDll"
# PROP BASE Intermediate_Dir "ReleaseDll"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDll"
# PROP Intermediate_Dir "ReleaseDll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "NDEBUG_CONSOLE" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "../../include" /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /incremental:yes /machine:I386 /libpath:"$(wxwin)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "vscpd - Win32 Debug DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugDll"
# PROP BASE Intermediate_Dir "DebugDll"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugDll"
# PROP Intermediate_Dir "DebugDll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "_DEBUG_CONSOLE" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "../../include" /d "_DEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386 /libpath:"$(wxwin)\lib\vc_lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "vscpd - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x0400 /D _WIN32_WINNT=0x0400 /D "_MT" /D wxUSE_GUI=1 /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "../../include" /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib wsock32.lib netapi32.lib msvcrt.lib libc.lib /nologo /subsystem:console /machine:I386 /libpath:"$(wxwin)\lib\vc_lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy release\*.exe ..\..\delivery
# End Special Build Tool

!ELSEIF  "$(CFG)" == "vscpd - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x0400 /D _WIN32_WINNT=0x0400 /D "_MT" /D wxUSE_GUI=0 /D "__WXDEBUG__" /D WXDEBUG=1 /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "../../include" /d "_DEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libexpat.lib comctl32.lib wsock32.lib netapi32.lib rpcrt4.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib MSVCRTD.lib libcd.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /nodefaultlib:"libcd.lib MSVCRTD.lib" /pdbtype:sept /libpath:"$(wxwin)\lib\vc_lib"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "vscpd - Win32 Release DLL"
# Name "vscpd - Win32 Debug DLL"
# Name "vscpd - Win32 Release"
# Name "vscpd - Win32 Debug"
# Begin Group "resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wx\msw\blank.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\bullseye.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\cdrom.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\colours.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\msw\computer.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\csquery.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\msw\disable.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\msw\drive.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\file1.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\floppy.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\folder1.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\folder2.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\hand.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\magnif1.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\noentry.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pbrush.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pencil.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pntleft.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pntright.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\query.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\removble.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\rightarr.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\roller.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\size.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\watch1.cur
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\common\canal.h
# End Source File
# Begin Source File

SOURCE=..\..\common\canal_macro.h
# End Source File
# Begin Source File

SOURCE=..\..\common\canal_win32_ipc.h
# End Source File
# Begin Source File

SOURCE=..\..\common\canaldlldef.h
# End Source File
# Begin Source File

SOURCE=..\..\common\clientlist.h
# End Source File
# Begin Source File

SOURCE=..\..\common\controlobject.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\crc.h
# End Source File
# Begin Source File

SOURCE=..\..\common\daemonvscp.h
# End Source File
# Begin Source File

SOURCE=..\..\common\devicelist.h
# End Source File
# Begin Source File

SOURCE=..\..\common\devicethread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\dllist.h
# End Source File
# Begin Source File

SOURCE=..\..\common\dm.h
# End Source File
# Begin Source File

SOURCE=..\..\common\interfacelist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\md5.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\common\tcpipclientthread.h
# End Source File
# Begin Source File

SOURCE=..\..\common\udpthread.h
# End Source File
# Begin Source File

SOURCE=..\..\common\userlist.h
# End Source File
# Begin Source File

SOURCE=..\..\common\version.h
# End Source File
# Begin Source File

SOURCE=..\..\..\vscp\common\vscp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\vscp\common\vscp_classes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\vscp\common\vscp_types.h
# End Source File
# Begin Source File

SOURCE=.\vscpd.h
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\common\clientlist.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\controlobject.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\crc.c
# End Source File
# Begin Source File

SOURCE=..\..\common\daemonvscp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\devicelist.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\devicethread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\dllist.c
# End Source File
# Begin Source File

SOURCE=..\..\common\dm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\interfacelist.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\md5.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\tcpipclientthread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\udpthread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\userlist.cpp
# End Source File
# Begin Source File

SOURCE=.\vscpd.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\vscphelper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\vscpvariable.cpp
# End Source File
# End Group
# End Target
# End Project
