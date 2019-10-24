# Microsoft Developer Studio Project File - Name="testInterface" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=testInterface - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "testInterface.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "testInterface.mak" CFG="testInterface - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "testInterface - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "testInterface - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF "$(CFG)" == "testInterface - Win32 Release"

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
# ADD BASE CPP /nologo /FD /Fd"VCProjectRelease\testInterface.pdb" /MD /O2 /GR /EHsc /W4  /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D_UNICODE /I "D:\wxWidgets-2.8.7/include" /I "D:\wxWidgets-2.8.7/contrib/include" /I "D:\wxWidgets-2.8.7/lib/vc_lib/mswu" /I "D:\Program\Microsoft Visual Studio\vc98\include" /c
# ADD CPP /nologo /FD /Fd"VCProjectRelease\testInterface.pdb" /MD /O2 /GR /EHsc /W4  /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D_UNICODE /I "D:\wxWidgets-2.8.7/include" /I "D:\wxWidgets-2.8.7/contrib/include" /I "D:\wxWidgets-2.8.7/lib/vc_lib/mswu" /I "D:\Program\Microsoft Visual Studio\vc98\include" /c
# ADD BASE MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D_UNICODE /mktyplib203 /win32
# ADD MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D_UNICODE /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D_UNICODE /I "D:\wxWidgets-2.8.7/include" /I "D:\wxWidgets-2.8.7/contrib/include" /I "D:\wxWidgets-2.8.7/lib/vc_lib/mswu" /I "D:\Program\Microsoft Visual Studio\vc98\include"
# ADD RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D_UNICODE /I "D:\wxWidgets-2.8.7/include" /I "D:\wxWidgets-2.8.7/contrib/include" /I "D:\wxWidgets-2.8.7/lib/vc_lib/mswu" /I "D:\Program\Microsoft Visual Studio\vc98\include"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmsw28u_richtext.lib wxmsw28u_aui.lib wxmsw28u_core.lib wxbase28u.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregexu.lib wxmsw28u_adv.lib wxmsw28u_html.lib wxmsw28u_xrc.lib wxbase28u_net.lib wxbase28u_xml.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"D:\wxWidgets-2.8.7/lib/vc_lib" /libpath:"D:\Program\Microsoft Visual Studio\vc98\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /out:"VCProjectRelease\testInterface.exe"
# ADD LINK32 wxmsw28u_richtext.lib wxmsw28u_aui.lib wxmsw28u_core.lib wxbase28u.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregexu.lib wxmsw28u_adv.lib wxmsw28u_html.lib wxmsw28u_xrc.lib wxbase28u_net.lib wxbase28u_xml.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"D:\wxWidgets-2.8.7/lib/vc_lib" /libpath:"D:\Program\Microsoft Visual Studio\vc98\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /out:"VCProjectRelease\testInterface.exe"

!ELSEIF "$(CFG)" == "testInterface - Win32 Debug"

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
# ADD BASE CPP /nologo /FD /Fd"VCProjectDebug\testInterface.pdb" /MDd /Od /GR /EHsc /W4 /Zi /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /D_UNICODE /I "D:\wxWidgets-2.8.7/include" /I "D:\wxWidgets-2.8.7/contrib/include" /I "D:\wxWidgets-2.8.7/lib/vc_lib/mswud" /I "D:\Program\Microsoft Visual Studio\vc98\include" /c
# ADD CPP /nologo /FD /Fd"VCProjectDebug\testInterface.pdb" /MDd /Od /GR /EHsc /W4 /Zi /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /D_UNICODE /I "D:\wxWidgets-2.8.7/include" /I "D:\wxWidgets-2.8.7/contrib/include" /I "D:\wxWidgets-2.8.7/lib/vc_lib/mswud" /I "D:\Program\Microsoft Visual Studio\vc98\include" /c
# ADD BASE MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /D_UNICODE /mktyplib203 /win32
# ADD MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /D_UNICODE /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /D_UNICODE /I "D:\wxWidgets-2.8.7/include" /I "D:\wxWidgets-2.8.7/contrib/include" /I "D:\wxWidgets-2.8.7/lib/vc_lib/mswud" /I "D:\Program\Microsoft Visual Studio\vc98\include"
# ADD RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /D_UNICODE /I "D:\wxWidgets-2.8.7/include" /I "D:\wxWidgets-2.8.7/contrib/include" /I "D:\wxWidgets-2.8.7/lib/vc_lib/mswud" /I "D:\Program\Microsoft Visual Studio\vc98\include"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmsw28ud_richtext.lib wxmsw28ud_aui.lib wxmsw28ud_core.lib wxbase28ud.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexud.lib wxmsw28ud_adv.lib wxmsw28ud_html.lib wxmsw28ud_xrc.lib wxbase28ud_net.lib wxbase28ud_xml.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"D:\wxWidgets-2.8.7/lib/vc_lib" /libpath:"D:\Program\Microsoft Visual Studio\vc98\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /DEBUG /out:"VCProjectDebug\testInterface.exe"
# ADD LINK32 wxmsw28ud_richtext.lib wxmsw28ud_aui.lib wxmsw28ud_core.lib wxbase28ud.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexud.lib wxmsw28ud_adv.lib wxmsw28ud_html.lib wxmsw28ud_xrc.lib wxbase28ud_net.lib wxbase28ud_xml.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"D:\wxWidgets-2.8.7/lib/vc_lib" /libpath:"D:\Program\Microsoft Visual Studio\vc98\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /DEBUG /out:"VCProjectDebug\testInterface.exe"

!ENDIF

# Begin Target

# Name "testInterface - Win32 Release"
# Name "testInterface - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE="testinterfaceapp.cpp"
# End Source File
# Begin Source File

SOURCE="testinterface.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\common\canalsuperwrapper.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\common\crc.c"
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

SOURCE="testInterface.rc"
# End Source File

# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE="testinterfaceapp.h"
# End Source File
# Begin Source File

SOURCE="testinterface.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\canalsuperwrapper.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\common\crc.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\dllwrapper.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\guid.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\vscphelper.h"
# End Source File
# Begin Source File

SOURCE="..\..\common\vscptcpif.h"
# End Source File

# End Group
# End Target
# End Project

