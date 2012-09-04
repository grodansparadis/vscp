# Microsoft Developer Studio Project File - Name="cw" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=cw - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cw.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cw.mak" CFG="cw - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cw - Win32 Release DLL" (based on "Win32 (x86) Application")
!MESSAGE "cw - Win32 Debug DLL" (based on "Win32 (x86) Application")
!MESSAGE "cw - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "cw - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cw - Win32 Release DLL"

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
# ADD BASE CPP /nologo /W4 /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "../../include" /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 msvcrt.lib libc.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib libexpat.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(wxwin)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "cw - Win32 Debug DLL"

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
# ADD BASE CPP /nologo /W4 /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "../../include" /d "_DEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib msvcrtd.lib libcd.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"$(wxwin)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "cw - Win32 Release"

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
# ADD BASE CPP /nologo /W4 /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "../../include" /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msvcrt.lib libc.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(wxwin)\lib\vc_lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy release\*.exe ..\delivery
# End Special Build Tool

!ELSEIF  "$(CFG)" == "cw - Win32 Debug"

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
# ADD BASE CPP /nologo /W4 /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "../../include" /d "_DEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib msvcrtd.lib libcd.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /pdbtype:sept /libpath:"$(wxwin)\lib\vc_lib"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "cw - Win32 Release DLL"
# Name "cw - Win32 Debug DLL"
# Name "cw - Win32 Release"
# Name "cw - Win32 Debug"
# Begin Group "Resources"

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

SOURCE=.\chart.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\computer.ico
# End Source File
# Begin Source File

SOURCE=.\cw.rc
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

SOURCE=.\mondrian.ico
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

# PROP Default_Filter ""
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

SOURCE=..\..\common\canalsuperwrapper.h
# End Source File
# Begin Source File

SOURCE=..\common\canaltcpif.h
# End Source File
# Begin Source File

SOURCE=..\..\common\configfile.h
# End Source File
# Begin Source File

SOURCE=.\cw.h
# End Source File
# Begin Source File

SOURCE=..\common\dllwrapper.h
# End Source File
# Begin Source File

SOURCE=.\EditIfDlg.h
# End Source File
# Begin Source File

SOURCE=.\MsgListWnd.h
# End Source File
# Begin Source File

SOURCE=.\msglistwnd_level2.h
# End Source File
# Begin Source File

SOURCE=.\SelectIfDlg.h
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

SOURCE=..\..\common\vscpclientthreads.h
# End Source File
# Begin Source File

SOURCE=.\vscpconfigwnd.h
# End Source File
# Begin Source File

SOURCE=..\..\common\vscptcpif.h
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\common\canalsuperwrapper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\crc.c
# End Source File
# Begin Source File

SOURCE=.\cw.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\dllist.c
# End Source File
# Begin Source File

SOURCE=..\..\common\dllwrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\EditIfDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\guid.cpp
# End Source File
# Begin Source File

SOURCE=.\MsgListWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\msglistwnd_level2.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectIfDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\vscp.h
# End Source File
# Begin Source File

SOURCE=.\vscpconfigwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\vscphelper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\vscptcpif.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\wx\msw\cross.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\std.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\wx.manifest
# End Source File
# End Target
# End Project
