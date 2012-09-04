# Microsoft Developer Studio Generated NMAKE File, Based on simdll.dsp
!IF "$(CFG)" == ""
CFG=simdll - Win32 Debug
!MESSAGE No configuration specified. Defaulting to simdll - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "simdll - Win32 Release" && "$(CFG)" != "simdll - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "simdll.mak" CFG="simdll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "simdll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "simdll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "simdll - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\canalsim.dll" "$(OUTDIR)\simdll.pch"


CLEAN :
	-@erase "$(INTDIR)\simdll.obj"
	-@erase "$(INTDIR)\simdll.pch"
	-@erase "$(INTDIR)\simdll.res"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\canalsim.dll"
	-@erase "$(OUTDIR)\canalsim.exp"
	-@erase "$(OUTDIR)\canalsim.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\simdll.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simdll.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=Wsock32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\canalsim.pdb" /machine:I386 /def:".\simdll.def" /out:"$(OUTDIR)\canalsim.dll" /implib:"$(OUTDIR)\canalsim.lib" 
DEF_FILE= \
	".\simdll.def"
LINK32_OBJS= \
	"$(INTDIR)\simdll.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\simdll.res"

"$(OUTDIR)\canalsim.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\canalsim.dll" "$(OUTDIR)\simdll.pch"
   copy release\*.dll D:\WINNT\system32
	copy release\*.dll ..\..\..\..\delivery
	copy release\*.lib ..\..\..\..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "simdll - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\canalsim.dll" "$(OUTDIR)\simdll.pch"


CLEAN :
	-@erase "$(INTDIR)\simdll.obj"
	-@erase "$(INTDIR)\simdll.pch"
	-@erase "$(INTDIR)\simdll.res"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\canalsim.dll"
	-@erase "$(OUTDIR)\canalsim.exp"
	-@erase "$(OUTDIR)\canalsim.ilk"
	-@erase "$(OUTDIR)\canalsim.lib"
	-@erase "$(OUTDIR)\canalsim.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\simdll.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simdll.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=Wsock32.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\canalsim.pdb" /debug /machine:I386 /def:".\simdll.def" /out:"$(OUTDIR)\canalsim.dll" /implib:"$(OUTDIR)\canalsim.lib" /pdbtype:sept 
DEF_FILE= \
	".\simdll.def"
LINK32_OBJS= \
	"$(INTDIR)\simdll.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\simdll.res"

"$(OUTDIR)\canalsim.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\canalsim.dll" "$(OUTDIR)\simdll.pch"
   copy debug\*.dll D:\WINNT\system32
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

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
!IF EXISTS("simdll.dep")
!INCLUDE "simdll.dep"
!ELSE 
!MESSAGE Warning: cannot find "simdll.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "simdll - Win32 Release" || "$(CFG)" == "simdll - Win32 Debug"
SOURCE=.\simdll.cpp

"$(INTDIR)\simdll.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\simdll.rc

"$(INTDIR)\simdll.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "simdll - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\simdll.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\simdll.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "simdll - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\simdll.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\simdll.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

