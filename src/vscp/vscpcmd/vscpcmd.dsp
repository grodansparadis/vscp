# Microsoft Developer Studio Project File - Name="vscpcmd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=vscpcmd - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vscpcmd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vscpcmd.mak" CFG="vscpcmd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vscpcmd - Win32 Release Unicode DLL" (based on "Win32 (x86) Console Application")
!MESSAGE "vscpcmd - Win32 Release Unicode" (based on "Win32 (x86) Console Application")
!MESSAGE "vscpcmd - Win32 Debug Unicode DLL" (based on "Win32 (x86) Console Application")
!MESSAGE "vscpcmd - Win32 Debug Unicode" (based on "Win32 (x86) Console Application")
!MESSAGE "vscpcmd - Win32 Release DLL" (based on "Win32 (x86) Console Application")
!MESSAGE "vscpcmd - Win32 Debug DLL" (based on "Win32 (x86) Console Application")
!MESSAGE "vscpcmd - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "vscpcmd - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vscpcmd - Win32 Release Unicode DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "BaseReleaseUnicodeDll"
# PROP BASE Intermediate_Dir "BaseReleaseUnicodeDll"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "BaseReleaseUnicodeDll"
# PROP Intermediate_Dir "BaseReleaseUnicodeDll"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /D "WXUSINGDLL" /D "_UNICODE" /D "UNICODE" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /i "../../include" /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib advapi32.lib wsock32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib advapi32.lib shell32.lib wsock32.lib /nologo /subsystem:console /machine:I386 /libpath:"$(wxwin)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "vscpcmd - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "BaseReleaseUnicode"
# PROP BASE Intermediate_Dir "BaseReleaseUnicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "BaseReleaseUnicode"
# PROP Intermediate_Dir "BaseReleaseUnicode"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /D "WXUSINGDLL" /D "_UNICODE" /D "UNICODE" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /i "../../include" /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib advapi32.lib wsock32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib advapi32.lib shell32.lib wsock32.lib /nologo /subsystem:console /machine:I386 /libpath:"$(wxwin)\lib\vc_lib"


!ELSEIF  "$(CFG)" == "vscpcmd - Win32 Debug Unicode DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "BaseReleaseUnicode"
# PROP BASE Intermediate_Dir "BaseReleaseUnicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "BaseReleaseUnicode"
# PROP Intermediate_Dir "BaseReleaseUnicode"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /Zi /O2 /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /O2 /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /D "__WXDEBUG__" /D WXDEBUG=1 /D "WXUSINGDLL" /D "_UNICODE" /D "UNICODE" /D "NDEBUG" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /i "../../include" /d "_DEBUG" /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "_DEBUG" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib advapi32.lib wsock32.lib kernel32.lib user32.lib advapi32.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"$(wxwin)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "vscpcmd - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "BaseDebugUnicode"
# PROP BASE Intermediate_Dir "BaseDebugUnicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "BaseDebugUnicode"
# PROP Intermediate_Dir "BaseDebugUnicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /D "__WXDEBUG__" /D WXDEBUG=1 /D "_UNICODE" /D "UNICODE" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /i "../../include" /d "_DEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib advapi32.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"$(wxwin)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "vscpcmd - Win32 Release DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "BaseReleaseDll"
# PROP BASE Intermediate_Dir "BaseReleaseDll"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "BaseReleaseDll"
# PROP Intermediate_Dir "BaseReleaseDll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "NDEBUG_CONSOLE" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /i "../../include" /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib advapi32.lib wsock32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386 /libpath:"$(wxwin)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "vscpcmd - Win32 Debug DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "BaseDebugDll"
# PROP BASE Intermediate_Dir "BaseDebugDll"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "BaseDebugDll"
# PROP Intermediate_Dir "BaseDebugDll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "_DEBUG_CONSOLE" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /i "../../include" /d "_DEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib advapi32.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"$(wxwin)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "vscpcmd - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "BaseRelease"
# PROP BASE Intermediate_Dir "BaseRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "BaseRelease"
# PROP Intermediate_Dir "BaseRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /i "../../include" /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib advapi32.lib wsock32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386 /out:"Release/vscpcmd.exe" /libpath:"$(wxwin)\lib\vc_lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy release\*.exe ..\delivery
# End Special Build Tool

!ELSEIF  "$(CFG)" == "vscpcmd - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "BaseDebug"
# PROP BASE Intermediate_Dir "BaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "BaseDebug"
# PROP Intermediate_Dir "BaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /I "$(wxwin)\include\msvc" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D _WIN32_WINNT=0x0400 /D "_MT" /D wxUSE_GUI=0 /D "__WXDEBUG__" /D WXDEBUG=1 /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /i "../../include" /d "_DEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib advapi32.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/vscpcmd.exe" /pdbtype:sept /libpath:"$(wxwin)\lib\vc_lib"

!ENDIF 

# Begin Target

# Name "vscpcmd - Win32 Release Unicode DLL"
# Name "vscpcmd - Win32 Release Unicode"
# Name "vscpcmd - Win32 Debug Unicode DLL"
# Name "vscpcmd - Win32 Debug Unicode"
# Name "vscpcmd - Win32 Release DLL"
# Name "vscpcmd - Win32 Debug DLL"
# Name "vscpcmd - Win32 Release"
# Name "vscpcmd - Win32 Debug"
# Begin Source File

SOURCE=..\common\controlobject.h
# End Source File
# Begin Source File

SOURCE=..\..\common\crc.c
# End Source File
# Begin Source File

SOURCE=.\vscpcmd.cpp
# End Source File
# Begin Source File

SOURCE=.\vscpcmd.h
# End Source File
# Begin Source File

SOURCE=..\common\vscphelper.cpp
# End Source File
# Begin Source File

SOURCE=..\common\vscptcpif.cpp
# End Source File
# End Target
# End Project
