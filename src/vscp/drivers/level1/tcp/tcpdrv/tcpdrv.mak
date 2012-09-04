# Microsoft Developer Studio Generated NMAKE File, Based on tcpdrv.dsp
!IF "$(CFG)" == ""
CFG=tcpdrv - Win32 Debug
!MESSAGE No configuration specified. Defaulting to tcpdrv - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "tcpdrv - Win32 Release" && "$(CFG)" != "tcpdrv - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tcpdrv.mak" CFG="tcpdrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tcpdrv - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tcpdrv - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tcpdrv - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\canaltcpdrv.dll" "$(OUTDIR)\tcpdrv.pch"


CLEAN :
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\tcpdrv.obj"
	-@erase "$(INTDIR)\tcpdrv.pch"
	-@erase "$(INTDIR)\tcpdrv.res"
	-@erase "$(INTDIR)\TCPDrvObj.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\canaltcpdrv.dll"
	-@erase "$(OUTDIR)\canaltcpdrv.exp"
	-@erase "$(OUTDIR)\canaltcpdrv.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\tcpdrv.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tcpdrv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=mfcs42.lib MSVCRT.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\canaltcpdrv.pdb" /machine:I386 /nodefaultlib:"MSVCRT.lib mfcs42.lib" /def:".\tcpdrv.def" /out:"$(OUTDIR)\canaltcpdrv.dll" /implib:"$(OUTDIR)\canaltcpdrv.lib" 
DEF_FILE= \
	".\tcpdrv.def"
LINK32_OBJS= \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\tcpdrv.obj" \
	"$(INTDIR)\TCPDrvObj.obj" \
	"$(INTDIR)\tcpdrv.res" \
	"$(INTDIR)\dllist.obj"

"$(OUTDIR)\canaltcpdrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\canaltcpdrv.dll" "$(OUTDIR)\tcpdrv.pch"
   copy release\*.dll ..\..\..\..\delivery
	copy release\*.lib ..\..\..\..\delivery
	copy release\*.dll D:\WINNT\system32
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "tcpdrv - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\canaltcpdrv.dll" "$(OUTDIR)\tcpdrv.pch"


CLEAN :
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\tcpdrv.obj"
	-@erase "$(INTDIR)\tcpdrv.pch"
	-@erase "$(INTDIR)\tcpdrv.res"
	-@erase "$(INTDIR)\TCPDrvObj.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\canaltcpdrv.dll"
	-@erase "$(OUTDIR)\canaltcpdrv.exp"
	-@erase "$(OUTDIR)\canaltcpdrv.ilk"
	-@erase "$(OUTDIR)\canaltcpdrv.lib"
	-@erase "$(OUTDIR)\canaltcpdrv.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\tcpdrv.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tcpdrv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=mfcs42d.lib MSVCRTD.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\canaltcpdrv.pdb" /debug /machine:I386 /nodefaultlib:"MSVCRTD.lib mfcs42d.lib" /def:".\tcpdrv.def" /out:"$(OUTDIR)\canaltcpdrv.dll" /implib:"$(OUTDIR)\canaltcpdrv.lib" /pdbtype:sept 
DEF_FILE= \
	".\tcpdrv.def"
LINK32_OBJS= \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\tcpdrv.obj" \
	"$(INTDIR)\TCPDrvObj.obj" \
	"$(INTDIR)\tcpdrv.res" \
	"$(INTDIR)\dllist.obj"

"$(OUTDIR)\canaltcpdrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("tcpdrv.dep")
!INCLUDE "tcpdrv.dep"
!ELSE 
!MESSAGE Warning: cannot find "tcpdrv.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "tcpdrv - Win32 Release" || "$(CFG)" == "tcpdrv - Win32 Debug"
SOURCE=..\..\..\..\common\dllist.c

"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "tcpdrv - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\tcpdrv.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\tcpdrv.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tcpdrv - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\tcpdrv.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\tcpdrv.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\tcpdrv.cpp

"$(INTDIR)\tcpdrv.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tcpdrv.rc

"$(INTDIR)\tcpdrv.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\TCPDrvObj.cpp

"$(INTDIR)\TCPDrvObj.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

