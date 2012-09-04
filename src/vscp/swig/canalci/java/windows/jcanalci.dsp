# Microsoft Developer Studio Project File - Name="jcanalci" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=jcanalci - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "jcanalci.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "jcanalci.mak" CFG="jcanalci - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jcanalci - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jcanalci - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "jcanalci - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JCANALCI_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(JAVA_INCLUDE)" /I "$(JAVA_INCLUDE)\win32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JCANALCI_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"jcanalci.dll" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Java compile post-build step
PostBuild_Cmds=echo on	"%JAVA_BIN%\javac" *.java
# End Special Build Tool

!ELSEIF  "$(CFG)" == "jcanalci - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JCANALCI_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(JAVA_INCLUDE)" /I "$(JAVA_INCLUDE)\win32" /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JCANALCI_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib libexpat.lib net.lib netapi32.lib /nologo /dll /machine:I386 /out:"jcanalci.dll"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Java compile post-build step
PostBuild_Cmds=echo on	"%JAVA_BIN%\javac"  *.java
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "jcanalci - Win32 Debug"
# Name "jcanalci - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\common\canalshmem_level1_win32.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\common\CanalSuperWrapper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\common\DllWrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\jcanalci_wrap.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\common\canalshmem_level1_win32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\common\CanalSuperWrapper.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\common\dllwrapper.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\jcanalci.i

!IF  "$(CFG)" == "jcanalci - Win32 Debug"

# Begin Custom Build
InputPath=.\jcanalci.i
InputName=jcanalci

"$(InputName)_wrap.cxx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo In order to function correctly, please ensure the following environment variables are correctly set: 
	echo JAVA_INCLUDE: %JAVA_INCLUDE% 
	echo JAVA_BIN: %JAVA_BIN% 
	echo on 
	..\..\..\swig.exe -c++ -java $(InputPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "jcanalci - Win32 Release"

# Begin Custom Build
InputPath=.\jcanalci.i
InputName=jcanalci

"$(InputName)_wrap.cxx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo In order to function correctly, please ensure the following environment variables are correctly set: 
	echo JAVA_INCLUDE: %JAVA_INCLUDE% 
	echo JAVA_BIN: %JAVA_BIN% 
	echo on 
	d:\SWIG-1.3.24\swig.exe -c++ -java $(InputPath) 
	
# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
