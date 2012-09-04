# Microsoft Developer Studio Project File - Name="fli4lcid" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=fli4lcid - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fli4lcid.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fli4lcid.mak" CFG="fli4lcid - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fli4lcid - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "fli4lcid - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF "$(CFG)" == "fli4lcid - Win32 Release"

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
# ADD BASE CPP /nologo /FD /Fd"VCProjectRelease\fli4lcid.pdb" /MD /O2 /GR /EHsc /W4  /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /I "/home/akhe/development/wxGTK-2.8.6/include" /I "/home/akhe/development/wxGTK-2.8.6/contrib/include" /I "/home/akhe/development/wxGTK-2.8.6/lib/vc_lib/gtk2"  /c
# ADD CPP /nologo /FD /Fd"VCProjectRelease\fli4lcid.pdb" /MD /O2 /GR /EHsc /W4  /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /I "/home/akhe/development/wxGTK-2.8.6/include" /I "/home/akhe/development/wxGTK-2.8.6/contrib/include" /I "/home/akhe/development/wxGTK-2.8.6/lib/vc_lib/gtk2"  /c
# ADD BASE MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /mktyplib203 /win32
# ADD MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /I "/home/akhe/development/wxGTK-2.8.6/include" /I "/home/akhe/development/wxGTK-2.8.6/contrib/include" /I "/home/akhe/development/wxGTK-2.8.6/lib/vc_lib/gtk2" 
# ADD RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /I "/home/akhe/development/wxGTK-2.8.6/include" /I "/home/akhe/development/wxGTK-2.8.6/contrib/include" /I "/home/akhe/development/wxGTK-2.8.6/lib/vc_lib/gtk2" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxgtk228_richtext.lib wxgtk228_aui.lib wxgtk228_core.lib wxbase28.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxgtk228_adv.lib wxgtk228_html.lib wxgtk228_xrc.lib wxbase28_net.lib wxbase28_xml.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"/home/akhe/development/wxGTK-2.8.6/lib/vc_lib"  /nologo /SUBSYSTEM:WINDOWS /machine:i386 /out:"VCProjectRelease\fli4lcid.exe"
# ADD LINK32 wxgtk228_richtext.lib wxgtk228_aui.lib wxgtk228_core.lib wxbase28.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxgtk228_adv.lib wxgtk228_html.lib wxgtk228_xrc.lib wxbase28_net.lib wxbase28_xml.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"/home/akhe/development/wxGTK-2.8.6/lib/vc_lib"  /nologo /SUBSYSTEM:WINDOWS /machine:i386 /out:"VCProjectRelease\fli4lcid.exe"

!ELSEIF "$(CFG)" == "fli4lcid - Win32 Debug"

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
# ADD BASE CPP /nologo /FD /Fd"VCProjectDebug\fli4lcid.pdb" /MDd /Od /GR /EHsc /W4 /Zi /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /I "/home/akhe/development/wxGTK-2.8.6/include" /I "/home/akhe/development/wxGTK-2.8.6/contrib/include" /I "/home/akhe/development/wxGTK-2.8.6/lib/vc_lib/gtk2d"  /c
# ADD CPP /nologo /FD /Fd"VCProjectDebug\fli4lcid.pdb" /MDd /Od /GR /EHsc /W4 /Zi /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /I "/home/akhe/development/wxGTK-2.8.6/include" /I "/home/akhe/development/wxGTK-2.8.6/contrib/include" /I "/home/akhe/development/wxGTK-2.8.6/lib/vc_lib/gtk2d"  /c
# ADD BASE MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /mktyplib203 /win32
# ADD MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /I "/home/akhe/development/wxGTK-2.8.6/include" /I "/home/akhe/development/wxGTK-2.8.6/contrib/include" /I "/home/akhe/development/wxGTK-2.8.6/lib/vc_lib/gtk2d" 
# ADD RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /I "/home/akhe/development/wxGTK-2.8.6/include" /I "/home/akhe/development/wxGTK-2.8.6/contrib/include" /I "/home/akhe/development/wxGTK-2.8.6/lib/vc_lib/gtk2d" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxgtk228d_richtext.lib wxgtk228d_aui.lib wxgtk228d_core.lib wxbase28d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxgtk228d_adv.lib wxgtk228d_html.lib wxgtk228d_xrc.lib wxbase28d_net.lib wxbase28d_xml.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"/home/akhe/development/wxGTK-2.8.6/lib/vc_lib"  /nologo /SUBSYSTEM:WINDOWS /machine:i386 /DEBUG /out:"VCProjectDebug\fli4lcid.exe"
# ADD LINK32 wxgtk228d_richtext.lib wxgtk228d_aui.lib wxgtk228d_core.lib wxbase28d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxgtk228d_adv.lib wxgtk228d_html.lib wxgtk228d_xrc.lib wxbase28d_net.lib wxbase28d_xml.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"/home/akhe/development/wxGTK-2.8.6/lib/vc_lib"  /nologo /SUBSYSTEM:WINDOWS /machine:i386 /DEBUG /out:"VCProjectDebug\fli4lcid.exe"

!ENDIF

# Begin Target

# Name "fli4lcid - Win32 Release"
# Name "fli4lcid - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE="fli4lcidapp.cpp"
# End Source File
# Begin Source File

SOURCE="fli4lcidtest.cpp"
# End Source File
# Begin Source File

SOURCE="fli4lcid.rc"
# End Source File

# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE="fli4lcidapp.h"
# End Source File
# Begin Source File

SOURCE="fli4lcidtest.h"
# End Source File

# End Group
# End Target
# End Project

