# Microsoft Developer Studio Project File - Name="autoself" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=autoself - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "autoself.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "autoself.mak" CFG="autoself - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "autoself - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "autoself - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "autoself - Win32 Release Unicode" (based on "Win32 (x86) Application")
!MESSAGE "autoself - Win32 Debug Unicode" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF "$(CFG)" == "autoself - Win32 Release"

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
# ADD BASE CPP /nologo /FD /Fd"VCProjectRelease\autoself.pdb" /MD /O2 /GR /EHsc /W4  /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /DwxUSE_UNICODE=0  /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/msw" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include" /c
# ADD CPP /nologo /FD /Fd"VCProjectRelease\autoself.pdb" /MD /O2 /GR /EHsc /W4  /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /DwxUSE_UNICODE=0  /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/msw" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include" /c
# ADD BASE MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /DwxUSE_UNICODE=0 /mktyplib203 /win32
# ADD MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /DwxUSE_UNICODE=0 /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /DwxUSE_UNICODE=0 /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/msw" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include"
# ADD RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /DwxUSE_UNICODE=0 /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/msw" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32  wxmsw28_richtext.lib wxmsw28_aui.lib wxmsw28_html.lib wxmsw28_xrc.lib wxmsw28_core.lib wxbase28.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxmsw28_adv.lib wxmsw28_html.lib wxmsw28_xrc.lib wxbase28_net.lib wxbase28_xml.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"C:\wxWidgets-2.8.12/lib/vc_lib" /libpath:"c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /out:"VCProjectRelease\autoself.exe"
# ADD LINK32  wxmsw28_richtext.lib wxmsw28_aui.lib wxmsw28_html.lib wxmsw28_xrc.lib wxmsw28_core.lib wxbase28.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxmsw28_adv.lib wxmsw28_html.lib wxmsw28_xrc.lib wxbase28_net.lib wxbase28_xml.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"C:\wxWidgets-2.8.12/lib/vc_lib" /libpath:"c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /out:"VCProjectRelease\autoself.exe"

!ELSEIF "$(CFG)" == "autoself - Win32 Debug"

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
# ADD BASE CPP /nologo /FD /Fd"VCProjectDebug\autoself.pdb" /MDd /Od /GR /EHsc /W4 /Zi /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /DwxUSE_UNICODE=0  /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/mswd" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include" /c
# ADD CPP /nologo /FD /Fd"VCProjectDebug\autoself.pdb" /MDd /Od /GR /EHsc /W4 /Zi /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /DwxUSE_UNICODE=0  /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/mswd" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include" /c
# ADD BASE MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /DwxUSE_UNICODE=0 /mktyplib203 /win32
# ADD MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /DwxUSE_UNICODE=0 /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /DwxUSE_UNICODE=0 /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/mswd" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include"
# ADD RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /DwxUSE_UNICODE=0 /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/mswd" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32  wxmsw28d_richtext.lib wxmsw28d_aui.lib wxmsw28d_html.lib wxmsw28d_xrc.lib wxmsw28d_core.lib wxbase28d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxmsw28d_adv.lib wxmsw28d_html.lib wxmsw28d_xrc.lib wxbase28d_net.lib wxbase28d_xml.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"C:\wxWidgets-2.8.12/lib/vc_lib" /libpath:"c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /DEBUG /out:"VCProjectDebug\autoself.exe"
# ADD LINK32  wxmsw28d_richtext.lib wxmsw28d_aui.lib wxmsw28d_html.lib wxmsw28d_xrc.lib wxmsw28d_core.lib wxbase28d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxmsw28d_adv.lib wxmsw28d_html.lib wxmsw28d_xrc.lib wxbase28d_net.lib wxbase28d_xml.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"C:\wxWidgets-2.8.12/lib/vc_lib" /libpath:"c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /DEBUG /out:"VCProjectDebug\autoself.exe"

!ELSEIF "$(CFG)" == "autoself - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "VCProjectReleaseUnicode"
# PROP BASE Intermediate_Dir "VCProjectReleaseUnicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "VCProjectReleaseUnicode"
# PROP Intermediate_Dir "VCProjectReleaseUnicode"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /Fd"VCProjectReleaseUnicode\autoself.pdb" /MD /O2 /GR /EHsc /W4  /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /DwxUSE_UNICODE=0  /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/msw" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include" /c
# ADD CPP /nologo /FD /Fd"VCProjectReleaseUnicode\autoself.pdb" /MD /O2 /GR /EHsc /W4  /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /DwxUSE_UNICODE=0  /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/msw" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include" /c
# ADD BASE MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /DwxUSE_UNICODE=0 /mktyplib203 /win32
# ADD MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /DwxUSE_UNICODE=0 /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /DwxUSE_UNICODE=0 /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/msw" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include"
# ADD RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /DwxUSE_UNICODE=0 /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/msw" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32  wxmsw28_richtext.lib wxmsw28_aui.lib wxmsw28_html.lib wxmsw28_xrc.lib wxmsw28_core.lib wxbase28.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxmsw28_adv.lib wxmsw28_html.lib wxmsw28_xrc.lib wxbase28_net.lib wxbase28_xml.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"C:\wxWidgets-2.8.12/lib/vc_lib" /libpath:"c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /out:"VCProjectReleaseUnicode\autoself.exe"
# ADD LINK32  wxmsw28_richtext.lib wxmsw28_aui.lib wxmsw28_html.lib wxmsw28_xrc.lib wxmsw28_core.lib wxbase28.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxmsw28_adv.lib wxmsw28_html.lib wxmsw28_xrc.lib wxbase28_net.lib wxbase28_xml.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"C:\wxWidgets-2.8.12/lib/vc_lib" /libpath:"c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /out:"VCProjectReleaseUnicode\autoself.exe"

!ELSEIF "$(CFG)" == "autoself - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "VCProjectDebugUnicode"
# PROP BASE Intermediate_Dir "VCProjectDebugUnicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "VCProjectDebugUnicode"
# PROP Intermediate_Dir "VCProjectDebugUnicode"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /Fd"VCProjectDebugUnicode\autoself.pdb" /MDd /Od /GR /EHsc /W4 /Zi /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /D_UNICODE  /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/mswud" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include" /c
# ADD CPP /nologo /FD /Fd"VCProjectDebugUnicode\autoself.pdb" /MDd /Od /GR /EHsc /W4 /Zi /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /D_UNICODE  /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/mswud" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include" /c
# ADD BASE MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /D_UNICODE /mktyplib203 /win32
# ADD MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /D_UNICODE /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /D_UNICODE /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/mswud" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include"
# ADD RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /D_UNICODE /I "C:\wxWidgets-2.8.12/include" /I "C:\wxWidgets-2.8.12/contrib/include" /I "C:\wxWidgets-2.8.12/lib/vc_lib/mswud" /I "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\include"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32  wxmsw28ud_richtext.lib wxmsw28ud_aui.lib wxmsw28ud_html.lib wxmsw28ud_xrc.lib wxmsw28ud_core.lib wxbase28ud.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexud.lib wxmsw28ud_adv.lib wxmsw28ud_html.lib wxmsw28ud_xrc.lib wxbase28ud_net.lib wxbase28ud_xml.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"C:\wxWidgets-2.8.12/lib/vc_lib" /libpath:"c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /DEBUG /out:"VCProjectDebugUnicode\autoself.exe"
# ADD LINK32  wxmsw28ud_richtext.lib wxmsw28ud_aui.lib wxmsw28ud_html.lib wxmsw28ud_xrc.lib wxmsw28ud_core.lib wxbase28ud.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexud.lib wxmsw28ud_adv.lib wxmsw28ud_html.lib wxmsw28ud_xrc.lib wxbase28ud_net.lib wxbase28ud_xml.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"C:\wxWidgets-2.8.12/lib/vc_lib" /libpath:"c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /DEBUG /out:"VCProjectDebugUnicode\autoself.exe"

!ENDIF

# Begin Target

# Name "autoself - Win32 Release"
# Name "autoself - Win32 Debug"
# Name "autoself - Win32 Release Unicode"
# Name "autoself - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\common\crc.c"
# End Source File
# Begin Source File

SOURCE="..\..\common\canaltcpif.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\common\guid.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\common\vscp.cpp"
# End Source File
# Begin Source File

SOURCE="autoself.cpp"
# End Source File
# Begin Source File

SOURCE="dlgabout.cpp"
# End Source File
# Begin Source File

SOURCE="dlgnodeinfo.cpp"
# End Source File
# Begin Source File

SOURCE="dlgnodelist.cpp"
# End Source File
# Begin Source File

SOURCE="dlgselectunit.cpp"
# End Source File
# Begin Source File

SOURCE="dlgsettings.cpp"
# End Source File
# Begin Source File

SOURCE="frmNode.cpp"
# End Source File
# Begin Source File

SOURCE="frmmain.cpp"
# End Source File
# Begin Source File

SOURCE="frmmessagelog.cpp"
# End Source File
# Begin Source File

SOURCE="threadvscpwork.cpp"
# End Source File
# Begin Source File

SOURCE="autoself.rc"
# End Source File

# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\common\canal.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\canaltcpif.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\guid.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\vscp.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\vscp_classes.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\vscp_types.h"
# End Source File
# Begin Source File

SOURCE="autoself.h"
# End Source File
# Begin Source File

SOURCE="dlgabout.h"
# End Source File
# Begin Source File

SOURCE="dlgabout_symbols.h"
# End Source File
# Begin Source File

SOURCE="dlgnodeinfo.h"
# End Source File
# Begin Source File

SOURCE="dlgnodelist.h"
# End Source File
# Begin Source File

SOURCE="dlgselectunit.h"
# End Source File
# Begin Source File

SOURCE="dlgsettings.h"
# End Source File
# Begin Source File

SOURCE="frmNode.h"
# End Source File
# Begin Source File

SOURCE="frmmain.h"
# End Source File
# Begin Source File

SOURCE="frmmessagelog.h"
# End Source File
# Begin Source File

SOURCE="pictures.h"
# End Source File
# Begin Source File

SOURCE="platform.h"
# End Source File
# Begin Source File

SOURCE="threadvscpwork.h"
# End Source File

# End Group
# End Target
# End Project

