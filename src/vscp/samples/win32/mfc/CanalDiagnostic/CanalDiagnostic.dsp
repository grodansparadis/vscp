# Microsoft Developer Studio Project File - Name="CanalDiagnostic" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=CanalDiagnostic - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CanalDiagnostic.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CanalDiagnostic.mak" CFG="CanalDiagnostic - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CanalDiagnostic - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "CanalDiagnostic - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CanalDiagnostic - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "CANAL_MFCAPP" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x41d /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy release\*.exe ..\delivery
# End Special Build Tool

!ELSEIF  "$(CFG)" == "CanalDiagnostic - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\mswd" /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "CANAL_MFCAPP" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x41d /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "CanalDiagnostic - Win32 Release"
# Name "CanalDiagnostic - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CanalDiagnostic.cpp
# End Source File
# Begin Source File

SOURCE=.\CanalDiagnostic.rc
# End Source File
# Begin Source File

SOURCE=.\CanalDiagnosticDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\CanalDiagnosticView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\common\canalshmem_level1_win32.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\common\canalshmem_level1_win32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\common\canalsuperwrapper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\common\canalsuperwrapper.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\common\dllist.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\common\dllist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\common\DllWrapper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\common\dllwrapper.h
# End Source File
# Begin Source File

SOURCE=.\DocumentProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\NewDocument.cpp
# End Source File
# Begin Source File

SOURCE=.\SendBurst.cpp
# End Source File
# Begin Source File

SOURCE=.\Splash.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\common\canal.h
# End Source File
# Begin Source File

SOURCE=.\CanalDiagnostic.h
# End Source File
# Begin Source File

SOURCE=.\CanalDiagnosticDoc.h
# End Source File
# Begin Source File

SOURCE=.\CanalDiagnosticView.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\DocumentProperties.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\NewDocument.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SendBurst.h
# End Source File
# Begin Source File

SOURCE=.\Splash.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\CanalDiagnostic.ico
# End Source File
# Begin Source File

SOURCE=.\res\CanalDiagnostic.rc2
# End Source File
# Begin Source File

SOURCE=.\res\CanalDiagnosticDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\earth3.bmp
# End Source File
# Begin Source File

SOURCE=.\Splsh16.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
# Section CanalDiagnostic : {72ADFD7B-2C39-11D0-9903-00A0C91BC942}
# 	1:17:ID_INDICATOR_TIME:109
# 	1:17:ID_INDICATOR_DATE:108
# 	2:2:BH:
# 	2:17:ID_INDICATOR_TIME:ID_INDICATOR_TIME
# 	2:17:ID_INDICATOR_DATE:ID_INDICATOR_DATE
# End Section
# Section CanalDiagnostic : {72ADFD78-2C39-11D0-9903-00A0C91BC942}
# 	1:10:IDB_SPLASH:107
# 	2:21:SplashScreenInsertKey:4.0
# End Section
