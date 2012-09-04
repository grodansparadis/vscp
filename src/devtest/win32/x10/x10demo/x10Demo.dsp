# Microsoft Developer Studio Project File - Name="x10Demo" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=x10Demo - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "x10Demo.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "x10Demo.mak" CFG="x10Demo - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "x10Demo - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "x10Demo - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "x10Demo - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /ZI /W3 /Od /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "_AFXDLL" /D "_MBCS" /Gm /Yu"stdafx.h" /GZ /c /GX 
# ADD CPP /nologo /MDd /ZI /W3 /Od /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "_AFXDLL" /D "_MBCS" /Gm /Yu"stdafx.h" /GZ /c /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /win32 
# ADD MTL /nologo /D"_DEBUG" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" /i "$(IntDir)" 
# ADD RSC /l 1033 /d "_DEBUG" /i "$(IntDir)" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /incremental:yes /debug /pdbtype:sept /subsystem:windows /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /incremental:yes /debug /pdbtype:sept /subsystem:windows /machine:ix86 

!ELSEIF  "$(CFG)" == "x10Demo - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /Zi /W3 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c /GX 
# ADD CPP /nologo /MD /Zi /W3 /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /win32 
# ADD MTL /nologo /D"NDEBUG" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" /i "$(IntDir)" 
# ADD RSC /l 1033 /d "NDEBUG" /i "$(IntDir)" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /incremental:no /debug /pdbtype:sept /subsystem:windows /opt:ref /opt:icf /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /incremental:no /debug /pdbtype:sept /subsystem:windows /opt:ref /opt:icf /machine:ix86 

!ENDIF

# Begin Target

# Name "x10Demo - Win32 Debug"
# Name "x10Demo - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;def;odl;idl;hpj;bat;asm;asmx"
# Begin Source File

SOURCE=.\stdafx.cpp

!IF  "$(CFG)" == "x10Demo - Win32 Debug"

# ADD CPP /nologo /Yc"stdafx.h" /GZ /GX 
!ELSEIF  "$(CFG)" == "x10Demo - Win32 Release"

# ADD CPP /nologo /Yc"stdafx.h" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=.\X10.cpp
# End Source File
# Begin Source File

SOURCE=.\x10Demo.cpp
# End Source File
# Begin Source File

SOURCE=.\x10DemoDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\X10.h
# End Source File
# Begin Source File

SOURCE=.\x10Demo.h
# End Source File
# Begin Source File

SOURCE=.\x10DemoDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx"
# Begin Source File

SOURCE=.\res\x10Demo.ico
# End Source File
# Begin Source File

SOURCE=.\x10Demo.rc
# End Source File
# Begin Source File

SOURCE=.\res\x10Demo.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\res\x10Demo.manifest
# End Source File
# End Target
# End Project

