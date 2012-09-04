# Microsoft Developer Studio Project File - Name="Testdc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Testdc - Win32 ReleaseDLL
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Testdc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Testdc.mak" CFG="Testdc - Win32 ReleaseDLL"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Testdc - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Testdc - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "Testdc - Win32 DebugDLL" (based on "Win32 (x86) Console Application")
!MESSAGE "Testdc - Win32 ReleaseDLL" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Testdc - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /GX /O2 /I "C:\c\wx\lib\msw" /I "C:\c\wx\include" /I "C:\c\wx\contrib\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /YX /FD /c
# ADD BASE RSC /l 0x410 /d "NDEBUG"
# ADD RSC /l 0x410 /i "C:\c\wx\include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib /nologo /subsystem:windows /machine:I386 /libpath:"C:\c\wx/contrib/lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /Zi /Od /I "C:\c\wx/lib/mswd" /I "C:\c\wx/include" /I "C:\c\wx/contrib/include" /I "C:\c\wx\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x410 /i "C:\c\wx/include" /i "C:\c\wx\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib zlibd.lib regexd.lib pngd.lib jpegd.lib tiffd.lib wxmswd.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"C:\c\wx/lib" /libpath:"C:\c\wx/contrib/lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Testdc___Win32_DebugDLL"
# PROP BASE Intermediate_Dir "Testdc___Win32_DebugDLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugDLL"
# PROP Intermediate_Dir "DebugDLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /Zi /Od /I "C:\c\wx\lib\mswdlld\\" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /FR /FD /GZ /c
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x410 /i "C:\c\wx\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wxmsw26d_core.lib wxbase26d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libcid.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Testdc___Win32_ReleaseDLL"
# PROP BASE Intermediate_Dir "Testdc___Win32_ReleaseDLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ReleaseDLL"
# PROP Intermediate_Dir "ReleaseDLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W4 /GX /O2 /I "C:\c\wx\lib\mswdll\\" /D "WIN32" /D "__WXMSW__" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /FD /c
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x410 /i "C:\c\wx\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wxmsw26_core.lib wxbase26.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib odbc32.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libci.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Testdc - Win32 Release"
# Name "Testdc - Win32 Debug"
# Name "Testdc - Win32 DebugDLL"
# Name "Testdc - Win32 ReleaseDLL"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AngularMeter.cpp

!IF  "$(CFG)" == "Testdc - Win32 Release"

# ADD CPP /YX

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

# ADD CPP /Yu"kprec.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AngularRegulator.cpp

!IF  "$(CFG)" == "Testdc - Win32 Release"

# ADD CPP /YX

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

# ADD CPP /Yu"kprec.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\BmpCheckBox.cpp

!IF  "$(CFG)" == "Testdc - Win32 Release"

# ADD CPP /YX

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

# ADD CPP /Yu"kprec.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\BmpSwitcher.cpp

!IF  "$(CFG)" == "Testdc - Win32 Release"

# ADD CPP /YX

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

# ADD CPP /Yu"kprec.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LCDClock.cpp

!IF  "$(CFG)" == "Testdc - Win32 Release"

# ADD CPP /YX

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

# ADD CPP /Yu"kprec.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LCDWindow.cpp

!IF  "$(CFG)" == "Testdc - Win32 Release"

# ADD CPP /YX

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

# ADD CPP /Yu"kprec.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LinearMeter.cpp

!IF  "$(CFG)" == "Testdc - Win32 Release"

# ADD CPP /YX

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

# ADD CPP /Yu"kprec.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LinearRegulator.cpp

!IF  "$(CFG)" == "Testdc - Win32 Release"

# ADD CPP /YX

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

# ADD CPP /Yu"kprec.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Testdc.rc
# End Source File
# Begin Source File

SOURCE=.\TestdcApp.cpp

!IF  "$(CFG)" == "Testdc - Win32 Release"

# ADD CPP /Fp"ReleaseDLL/Testdc.pch" /YX

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

# ADD CPP /Yu"kprec.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TestdcFrm.cpp

!IF  "$(CFG)" == "Testdc - Win32 Release"

# ADD CPP /YX

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

# ADD CPP /Yu"kprec.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TimeAlarm.cpp

!IF  "$(CFG)" == "Testdc - Win32 Release"

# ADD CPP /YX

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

# ADD CPP /Yu"kprec.h"

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

# ADD CPP /Yu"kprec.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wxprec.cpp
# ADD CPP /Yc"kprec.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AngularMeter.h
# End Source File
# Begin Source File

SOURCE=.\AngularRegulator.h
# End Source File
# Begin Source File

SOURCE=.\BmpCheckBox.h
# End Source File
# Begin Source File

SOURCE=.\BmpSwitcher.h
# End Source File
# Begin Source File

SOURCE=.\LCDClock.h
# End Source File
# Begin Source File

SOURCE=.\LCDWindow.h
# End Source File
# Begin Source File

SOURCE=.\LinearMeter.h
# End Source File
# Begin Source File

SOURCE=.\LinearRegulator.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\TestdcApp.h
# End Source File
# Begin Source File

SOURCE=.\TestdcFrm.h
# End Source File
# Begin Source File

SOURCE=.\TimeAlarm.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\wx\msw\blank.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\bullseye.cur
# End Source File
# Begin Source File

SOURCE=.\res\but.xpm
# End Source File
# Begin Source File

SOURCE=.\wx\msw\cdrom.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\computer.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\drive.ico
# End Source File
# Begin Source File

SOURCE=.\res\exefile.xpm
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

SOURCE=.\res\help.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\msw\magnif1.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\noentry.cur
# End Source File
# Begin Source File

SOURCE=.\res\ohsmile.xpm
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

SOURCE=.\res\sadsmile.xpm
# End Source File
# Begin Source File

SOURCE=.\wx\msw\size.cur
# End Source File
# Begin Source File

SOURCE=.\res\smile.xpm
# End Source File
# Begin Source File

SOURCE=.\res\Testdc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Testdc.xpm
# End Source File
# Begin Source File

SOURCE=.\res\Testdc32x32.xpm
# End Source File
# Begin Source File

SOURCE=.\wx\msw\watch1.cur
# End Source File
# End Group
# Begin Source File

SOURCE=.\CHANGES
# End Source File
# Begin Source File

SOURCE=".\KWIC-LICENSE"
# End Source File
# Begin Source File

SOURCE=.\makefile
# PROP Exclude_From_Scan -1

!IF  "$(CFG)" == "Testdc - Win32 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\makefile.gtk
# PROP Exclude_From_Scan -1

!IF  "$(CFG)" == "Testdc - Win32 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\makefile.mtf
# PROP Exclude_From_Scan -1

!IF  "$(CFG)" == "Testdc - Win32 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Testdc - Win32 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Testdc - Win32 DebugDLL"

!ELSEIF  "$(CFG)" == "Testdc - Win32 ReleaseDLL"

!ENDIF 

# End Source File
# End Target
# End Project
