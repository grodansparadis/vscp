# Microsoft Developer Studio Project File - Name="vscpsimnode" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=vscpsimnode - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vscpsimnode.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vscpsimnode.mak" CFG="vscpsimnode - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vscpsimnode - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "vscpsimnode - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF "$(CFG)" == "vscpsimnode - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "VCProjectRelease"
# PROP BASE Intermediate_Dir "VCProjectRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "VCProjectRelease"
# PROP Intermediate_Dir "VCProjectRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /Fd"VCProjectRelease\vscpsimnode.pdb" /MD /O2 /GR /EHsc /W4  /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS  /I "d:\wxWidgets-2.8.11/include" /I "d:\wxWidgets-2.8.11/contrib/include" /I "d:\wxWidgets-2.8.11/lib/vc_lib/gtk2" /I "C:\Program Files\Microsoft SDKs\Windows\v7.1\include" /I "D:\Microsoft Visual Studio 10.0\vc\include" /c
# ADD CPP /nologo /FD /Fd"VCProjectRelease\vscpsimnode.pdb" /MD /O2 /GR /EHsc /W4  /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS  /I "d:\wxWidgets-2.8.11/include" /I "d:\wxWidgets-2.8.11/contrib/include" /I "d:\wxWidgets-2.8.11/lib/vc_lib/gtk2" /I "C:\Program Files\Microsoft SDKs\Windows\v7.1\include" /I "D:\Microsoft Visual Studio 10.0\vc\include" /c
# ADD BASE MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /mktyplib203 /win32
# ADD MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /I "d:\wxWidgets-2.8.11/include" /I "d:\wxWidgets-2.8.11/contrib/include" /I "d:\wxWidgets-2.8.11/lib/vc_lib/gtk2" /I "C:\Program Files\Microsoft SDKs\Windows\v7.1\include" /I "D:\Microsoft Visual Studio 10.0\vc\include"
# ADD RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /I "d:\wxWidgets-2.8.11/include" /I "d:\wxWidgets-2.8.11/contrib/include" /I "d:\wxWidgets-2.8.11/lib/vc_lib/gtk2" /I "C:\Program Files\Microsoft SDKs\Windows\v7.1\include" /I "D:\Microsoft Visual Studio 10.0\vc\include"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32  wxgtk228_richtext.lib wxgtk228_aui.lib wxgtk228_html.lib wxgtk228_xrc.lib wxgtk228_core.lib wxbase28.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxgtk228_adv.lib wxgtk228_html.lib wxgtk228_xrc.lib wxbase28_net.lib wxbase28_xml.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"d:\wxWidgets-2.8.11/lib/vc_lib" /libpath:"C:\Program Files\Microsoft SDKs\Windows\v7.1\lib" /libpath:"D:\Microsoft Visual Studio 10.0\vc\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /out:"VCProjectRelease\vscpsimnode.exe"
# ADD LINK32  wxgtk228_richtext.lib wxgtk228_aui.lib wxgtk228_html.lib wxgtk228_xrc.lib wxgtk228_core.lib wxbase28.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxgtk228_adv.lib wxgtk228_html.lib wxgtk228_xrc.lib wxbase28_net.lib wxbase28_xml.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"d:\wxWidgets-2.8.11/lib/vc_lib" /libpath:"C:\Program Files\Microsoft SDKs\Windows\v7.1\lib" /libpath:"D:\Microsoft Visual Studio 10.0\vc\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /out:"VCProjectRelease\vscpsimnode.exe"

!ELSEIF "$(CFG)" == "vscpsimnode - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "VCProjectDebug"
# PROP BASE Intermediate_Dir "VCProjectDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "VCProjectDebug"
# PROP Intermediate_Dir "VCProjectDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /Fd"VCProjectDebug\vscpsimnode.pdb" /MDd /Od /GR /EHsc /W4 /Zi /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG  /I "d:\wxWidgets-2.8.11/include" /I "d:\wxWidgets-2.8.11/contrib/include" /I "d:\wxWidgets-2.8.11/lib/vc_lib/gtk2d" /I "C:\Program Files\Microsoft SDKs\Windows\v7.1\include" /I "D:\Microsoft Visual Studio 10.0\vc\include" /c
# ADD CPP /nologo /FD /Fd"VCProjectDebug\vscpsimnode.pdb" /MDd /Od /GR /EHsc /W4 /Zi /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG  /I "d:\wxWidgets-2.8.11/include" /I "d:\wxWidgets-2.8.11/contrib/include" /I "d:\wxWidgets-2.8.11/lib/vc_lib/gtk2d" /I "C:\Program Files\Microsoft SDKs\Windows\v7.1\include" /I "D:\Microsoft Visual Studio 10.0\vc\include" /c
# ADD BASE MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /mktyplib203 /win32
# ADD MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /I "d:\wxWidgets-2.8.11/include" /I "d:\wxWidgets-2.8.11/contrib/include" /I "d:\wxWidgets-2.8.11/lib/vc_lib/gtk2d" /I "C:\Program Files\Microsoft SDKs\Windows\v7.1\include" /I "D:\Microsoft Visual Studio 10.0\vc\include"
# ADD RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /I "d:\wxWidgets-2.8.11/include" /I "d:\wxWidgets-2.8.11/contrib/include" /I "d:\wxWidgets-2.8.11/lib/vc_lib/gtk2d" /I "C:\Program Files\Microsoft SDKs\Windows\v7.1\include" /I "D:\Microsoft Visual Studio 10.0\vc\include"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32  wxgtk228d_richtext.lib wxgtk228d_aui.lib wxgtk228d_html.lib wxgtk228d_xrc.lib wxgtk228d_core.lib wxbase28d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxgtk228d_adv.lib wxgtk228d_html.lib wxgtk228d_xrc.lib wxbase28d_net.lib wxbase28d_xml.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"d:\wxWidgets-2.8.11/lib/vc_lib" /libpath:"C:\Program Files\Microsoft SDKs\Windows\v7.1\lib" /libpath:"D:\Microsoft Visual Studio 10.0\vc\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /DEBUG /out:"VCProjectDebug\vscpsimnode.exe"
# ADD LINK32  wxgtk228d_richtext.lib wxgtk228d_aui.lib wxgtk228d_html.lib wxgtk228d_xrc.lib wxgtk228d_core.lib wxbase28d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxgtk228d_adv.lib wxgtk228d_html.lib wxgtk228d_xrc.lib wxbase28d_net.lib wxbase28d_xml.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"d:\wxWidgets-2.8.11/lib/vc_lib" /libpath:"C:\Program Files\Microsoft SDKs\Windows\v7.1\lib" /libpath:"D:\Microsoft Visual Studio 10.0\vc\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /DEBUG /out:"VCProjectDebug\vscpsimnode.exe"

!ENDIF

# Begin Target

# Name "vscpsimnode - Win32 Release"
# Name "vscpsimnode - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\common\crc.c"
# End Source File
# Begin Source File

SOURCE="..\..\common\canalsuperwrapper.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\common\dllwrapper.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\common\guid.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\common\vscphelper.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\common\vscptcpif.cpp"
# End Source File
# Begin Source File

SOURCE="defaultsettings.cpp"
# End Source File
# Begin Source File

SOURCE="dlgsimulatedvscpnode.cpp"
# End Source File
# Begin Source File

SOURCE="vscpsimnodeapp.cpp"
# End Source File
# Begin Source File

SOURCE="vscpsimnode.rc"
# End Source File

# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\common\crc.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\canal.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\canalsuperwrapper.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\dllwrapper.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\guid.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\vscp.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\vscp_class.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\vscp_type.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\vscphelper.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\vscptcpif.h"
# End Source File
# Begin Source File

SOURCE="defaultsettings.h"
# End Source File
# Begin Source File

SOURCE="dlgsimulatedvscpnode.h"
# End Source File
# Begin Source File

SOURCE="vscpsimnodeapp.h"
# End Source File

# End Group
# End Target
# End Project

