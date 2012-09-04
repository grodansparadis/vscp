# Microsoft Developer Studio Project File - Name="vscpworks" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=vscpworks - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vscpworks.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vscpworks.mak" CFG="vscpworks - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vscpworks - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF "$(CFG)" == "vscpworks - Win32 Debug"

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
# ADD BASE CPP /nologo /FD /Fd"VCProjectDebug\vscpworks.pdb" /MDd /Od /GR /EHsc /W4 /Zi /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG  /I "C:\wxWidgets-2.8.10/include" /I "C:\wxWidgets-2.8.10/contrib/include" /I "C:\wxWidgets-2.8.10/lib/vc_lib/mswd" /I "C:\Program Files (x86)\Microsoft SDKs\Windows\v5.0\include" /I "c:\Program Files (x86)\Microsoft Visual Studio 9.0\vc\include" /c
# ADD CPP /nologo /FD /Fd"VCProjectDebug\vscpworks.pdb" /MDd /Od /GR /EHsc /W4 /Zi /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG  /I "C:\wxWidgets-2.8.10/include" /I "C:\wxWidgets-2.8.10/contrib/include" /I "C:\wxWidgets-2.8.10/lib/vc_lib/mswd" /I "C:\Program Files (x86)\Microsoft SDKs\Windows\v5.0\include" /I "c:\Program Files (x86)\Microsoft Visual Studio 9.0\vc\include" /c
# ADD BASE MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /mktyplib203 /win32
# ADD MTL /nologo /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /mktyplib203 /win32
# ADD BASE RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /I "C:\wxWidgets-2.8.10/include" /I "C:\wxWidgets-2.8.10/contrib/include" /I "C:\wxWidgets-2.8.10/lib/vc_lib/mswd" /I "C:\Program Files (x86)\Microsoft SDKs\Windows\v5.0\include" /I "c:\Program Files (x86)\Microsoft Visual Studio 9.0\vc\include"
# ADD RSC /l 0x405 /DNOPCH /DWIN32 /D__WXMSW__ /D_WINDOWS /D__WXDEBUG__ /D_DEBUG /I "C:\wxWidgets-2.8.10/include" /I "C:\wxWidgets-2.8.10/contrib/include" /I "C:\wxWidgets-2.8.10/lib/vc_lib/mswd" /I "C:\Program Files (x86)\Microsoft SDKs\Windows\v5.0\include" /I "c:\Program Files (x86)\Microsoft Visual Studio 9.0\vc\include"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32  wxmsw28d_richtext.lib wxmsw28d_aui.lib wxmsw28d_media.lib wxmsw28d_html.lib wxmsw28d_xrc.lib wxmsw28d_core.lib wxbase28d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxmsw28d_adv.lib wxmsw28d_html.lib wxmsw28d_xrc.lib wxbase28d_net.lib wxbase28d_xml.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"C:\wxWidgets-2.8.10/lib/vc_lib" /libpath:"C:\Program Files (x86)\Microsoft SDKs\Windows\v5.0\lib" /libpath:"c:\Program Files (x86)\Microsoft Visual Studio 9.0\vc\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /DEBUG /out:"VCProjectDebug\vscpworks.exe"
# ADD LINK32  wxmsw28d_richtext.lib wxmsw28d_aui.lib wxmsw28d_media.lib wxmsw28d_html.lib wxmsw28d_xrc.lib wxmsw28d_core.lib wxbase28d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxmsw28d_adv.lib wxmsw28d_html.lib wxmsw28d_xrc.lib wxbase28d_net.lib wxbase28d_xml.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib /libpath:"C:\wxWidgets-2.8.10/lib/vc_lib" /libpath:"C:\Program Files (x86)\Microsoft SDKs\Windows\v5.0\lib" /libpath:"c:\Program Files (x86)\Microsoft Visual Studio 9.0\vc\lib" /nologo /SUBSYSTEM:WINDOWS /machine:i386 /DEBUG /out:"VCProjectDebug\vscpworks.exe"

!ENDIF

# Begin Target

# Name "vscpworks - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\common\crc.c"
# End Source File
# Begin Source File

SOURCE="..\common\canalsuperwrapper.cpp"
# End Source File
# Begin Source File

SOURCE="..\common\dllwrapper.cpp"
# End Source File
# Begin Source File

SOURCE="..\common\guid.cpp"
# End Source File
# Begin Source File

SOURCE="..\common\mdf.cpp"
# End Source File
# Begin Source File

SOURCE="..\common\vscphelper.cpp"
# End Source File
# Begin Source File

SOURCE="..\common\vscptcpif.cpp"
# End Source File
# Begin Source File

SOURCE="dialogabstractionedit.cpp"
# End Source File
# Begin Source File

SOURCE="dialogeditlevelidmrow.cpp"
# End Source File
# Begin Source File

SOURCE="dialogsetfiltermask.cpp"
# End Source File
# Begin Source File

SOURCE="dlgabout.cpp"
# End Source File
# Begin Source File

SOURCE="dlgconfiguration.cpp"
# End Source File
# Begin Source File

SOURCE="dlgnewvscpsession.cpp"
# End Source File
# Begin Source File

SOURCE="dlgselectdaemoninterface.cpp"
# End Source File
# Begin Source File

SOURCE="dlgvscpfilter.cpp"
# End Source File
# Begin Source File

SOURCE="dlgvscpinterfacesettings.cpp"
# End Source File
# Begin Source File

SOURCE="dlgvscpmsg.cpp"
# End Source File
# Begin Source File

SOURCE="dlgvscptrmit.cpp"
# End Source File
# Begin Source File

SOURCE="frmdeviceconfig.cpp"
# End Source File
# Begin Source File

SOURCE="frmmain.cpp"
# End Source File
# Begin Source File

SOURCE="frmmdfeditor.cpp"
# End Source File
# Begin Source File

SOURCE="frmscanfordevices.cpp"
# End Source File
# Begin Source File

SOURCE="frmvscpsession.cpp"
# End Source File
# Begin Source File

SOURCE="gridtable.cpp"
# End Source File
# Begin Source File

SOURCE="readguid.cpp"
# End Source File
# Begin Source File

SOURCE="readmdffile.cpp"
# End Source File
# Begin Source File

SOURCE="readmsfurl.cpp"
# End Source File
# Begin Source File

SOURCE="readregister.cpp"
# End Source File
# Begin Source File

SOURCE="threadvscpwork.cpp"
# End Source File
# Begin Source File

SOURCE="vscpbootloader.cpp"
# End Source File
# Begin Source File

SOURCE="vscptxobj.cpp"
# End Source File
# Begin Source File

SOURCE="vscpworks.cpp"
# End Source File
# Begin Source File

SOURCE="writeregister.cpp"
# End Source File
# Begin Source File

SOURCE="vscpworks.rc"
# End Source File

# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\common\crc.h"
# End Source File
# Begin Source File

SOURCE="..\common\canal.h"
# End Source File
# Begin Source File

SOURCE="..\common\canaldlldef.h"
# End Source File
# Begin Source File

SOURCE="..\common\canalsuperwrapper.h"
# End Source File
# Begin Source File

SOURCE="..\common\devicelist.h"
# End Source File
# Begin Source File

SOURCE="..\common\devitem.h"
# End Source File
# Begin Source File

SOURCE="..\common\dllwrapper.h"
# End Source File
# Begin Source File

SOURCE="..\common\guid.h"
# End Source File
# Begin Source File

SOURCE="..\common\mdf.h"
# End Source File
# Begin Source File

SOURCE="..\common\version.h"
# End Source File
# Begin Source File

SOURCE="..\common\vscp.h"
# End Source File
# Begin Source File

SOURCE="..\common\vscp_class.h"
# End Source File
# Begin Source File

SOURCE="..\common\vscp_type.h"
# End Source File
# Begin Source File

SOURCE="..\common\vscphelper.h"
# End Source File
# Begin Source File

SOURCE="..\common\vscptcpif.h"
# End Source File
# Begin Source File

SOURCE="dialogabstractionedit.h"
# End Source File
# Begin Source File

SOURCE="dialogeditlevelidmrow.h"
# End Source File
# Begin Source File

SOURCE="dialogsetfiltermask.h"
# End Source File
# Begin Source File

SOURCE="dlgabout.h"
# End Source File
# Begin Source File

SOURCE="dlgabout_symbols.h"
# End Source File
# Begin Source File

SOURCE="dlgconfiguration.h"
# End Source File
# Begin Source File

SOURCE="dlgnewvscpsession.h"
# End Source File
# Begin Source File

SOURCE="dlgselectdaemoninterface.h"
# End Source File
# Begin Source File

SOURCE="dlgvscpfilter.h"
# End Source File
# Begin Source File

SOURCE="dlgvscpinterfacesettings.h"
# End Source File
# Begin Source File

SOURCE="dlgvscpmsg.h"
# End Source File
# Begin Source File

SOURCE="dlgvscptrmit.h"
# End Source File
# Begin Source File

SOURCE="frmdeviceconfig.h"
# End Source File
# Begin Source File

SOURCE="frmmain.h"
# End Source File
# Begin Source File

SOURCE="frmmdfeditor.h"
# End Source File
# Begin Source File

SOURCE="frmscanfordevices.h"
# End Source File
# Begin Source File

SOURCE="frmvscpsession.h"
# End Source File
# Begin Source File

SOURCE="gridtable.h"
# End Source File
# Begin Source File

SOURCE="platform.h"
# End Source File
# Begin Source File

SOURCE="readguid.h"
# End Source File
# Begin Source File

SOURCE="readmdffile.h"
# End Source File
# Begin Source File

SOURCE="readmsfurl.h"
# End Source File
# Begin Source File

SOURCE="readregiister.h"
# End Source File
# Begin Source File

SOURCE="threadvscpwork.h"
# End Source File
# Begin Source File

SOURCE="vscpbootloader.h"
# End Source File
# Begin Source File

SOURCE="vscptxobj.h"
# End Source File
# Begin Source File

SOURCE="vscpworks.h"
# End Source File
# Begin Source File

SOURCE="writeregister.h"
# End Source File

# End Group
# End Target
# End Project

