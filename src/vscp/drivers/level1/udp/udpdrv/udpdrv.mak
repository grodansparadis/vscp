# Microsoft Developer Studio Generated NMAKE File, Based on udpdrv.dsp
!IF "$(CFG)" == ""
CFG=udpdrv - Win32 Debug
!MESSAGE No configuration specified. Defaulting to udpdrv - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "udpdrv - Win32 Release" && "$(CFG)" != "udpdrv - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "udpdrv.mak" CFG="udpdrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "udpdrv - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "udpdrv - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "udpdrv - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\udpdrv.dll" "$(OUTDIR)\udpdrv.pch"


CLEAN :
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\udpdrv.obj"
	-@erase "$(INTDIR)\udpdrv.pch"
	-@erase "$(INTDIR)\udpdrv.res"
	-@erase "$(INTDIR)\UDPDrvObj.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\udpdrv.dll"
	-@erase "$(OUTDIR)\udpdrv.exp"
	-@erase "$(OUTDIR)\udpdrv.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\udpdrv.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\udpdrv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=mfcs42.lib MSVCRT.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\udpdrv.pdb" /machine:I386 /nodefaultlib:"MSVCRT.lib mfcs42.lib" /def:".\udpdrv.def" /out:"$(OUTDIR)\udpdrv.dll" /implib:"$(OUTDIR)\udpdrv.lib" 
DEF_FILE= \
	".\udpdrv.def"
LINK32_OBJS= \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\udpdrv.obj" \
	"$(INTDIR)\UDPDrvObj.obj" \
	"$(INTDIR)\udpdrv.res"

"$(OUTDIR)\udpdrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\udpdrv.dll" "$(OUTDIR)\udpdrv.pch"
   copy release\*.dll ..\...\..\delivery
	copy release\*.lib ..\..\..\delivery
	copy release\*.dll D:\WINNT\system32
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "udpdrv - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\udpdrv.dll" "$(OUTDIR)\udpdrv.pch" "$(OUTDIR)\udpdrv.bsc"


CLEAN :
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\udpdrv.obj"
	-@erase "$(INTDIR)\udpdrv.pch"
	-@erase "$(INTDIR)\udpdrv.res"
	-@erase "$(INTDIR)\udpdrv.sbr"
	-@erase "$(INTDIR)\UDPDrvObj.obj"
	-@erase "$(INTDIR)\UDPDrvObj.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\udpdrv.bsc"
	-@erase "$(OUTDIR)\udpdrv.dll"
	-@erase "$(OUTDIR)\udpdrv.exp"
	-@erase "$(OUTDIR)\udpdrv.ilk"
	-@erase "$(OUTDIR)\udpdrv.lib"
	-@erase "$(OUTDIR)\udpdrv.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\udpdrv.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\udpdrv.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\udpdrv.sbr" \
	"$(INTDIR)\UDPDrvObj.sbr"

"$(OUTDIR)\udpdrv.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=mfcs42d.lib MSVCRTD.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\udpdrv.pdb" /debug /machine:I386 /nodefaultlib:"MSVCRTD.lib mfcs42d.lib" /def:".\udpdrv.def" /out:"$(OUTDIR)\udpdrv.dll" /implib:"$(OUTDIR)\udpdrv.lib" /pdbtype:sept 
DEF_FILE= \
	".\udpdrv.def"
LINK32_OBJS= \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\udpdrv.obj" \
	"$(INTDIR)\UDPDrvObj.obj" \
	"$(INTDIR)\udpdrv.res"

"$(OUTDIR)\udpdrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("udpdrv.dep")
!INCLUDE "udpdrv.dep"
!ELSE 
!MESSAGE Warning: cannot find "udpdrv.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "udpdrv - Win32 Release" || "$(CFG)" == "udpdrv - Win32 Debug"
SOURCE=..\..\..\..\common\dllist.c

!IF  "$(CFG)" == "udpdrv - Win32 Release"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "udpdrv - Win32 Debug"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "udpdrv - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\udpdrv.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\udpdrv.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "udpdrv - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\udpdrv.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\udpdrv.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\udpdrv.cpp

!IF  "$(CFG)" == "udpdrv - Win32 Release"


"$(INTDIR)\udpdrv.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "udpdrv - Win32 Debug"


"$(INTDIR)\udpdrv.obj"	"$(INTDIR)\udpdrv.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\udpdrv.rc

"$(INTDIR)\udpdrv.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\UDPDrvObj.cpp

!IF  "$(CFG)" == "udpdrv - Win32 Release"


"$(INTDIR)\UDPDrvObj.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "udpdrv - Win32 Debug"


"$(INTDIR)\UDPDrvObj.obj"	"$(INTDIR)\UDPDrvObj.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

