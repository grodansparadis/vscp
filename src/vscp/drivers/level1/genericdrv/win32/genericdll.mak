# Microsoft Developer Studio Generated NMAKE File, Based on genericdll.dsp
!IF "$(CFG)" == ""
CFG=genericdll - Win32 Debug
!MESSAGE No configuration specified. Defaulting to genericdll - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "genericdll - Win32 Release" && "$(CFG)" != "genericdll - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "genericdll.mak" CFG="genericdll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "genericdll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "genericdll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "genericdll - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\canalgeneric.dll" "$(OUTDIR)\genericdll.pch"


CLEAN :
	-@erase "$(INTDIR)\genericdll.obj"
	-@erase "$(INTDIR)\genericdll.pch"
	-@erase "$(INTDIR)\genericdll.res"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\canalgeneric.dll"
	-@erase "$(OUTDIR)\canalgeneric.exp"
	-@erase "$(OUTDIR)\canalgeneric.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\genericdll.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\genericdll.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=Wsock32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\canalgeneric.pdb" /machine:I386 /def:".\genericdll.def" /out:"$(OUTDIR)\canalgeneric.dll" /implib:"$(OUTDIR)\canalgeneric.lib" 
DEF_FILE= \
	".\genericdll.def"
LINK32_OBJS= \
	"$(INTDIR)\genericdll.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\genericdll.res"

"$(OUTDIR)\canalgeneric.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "genericdll - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\canalgeneric.dll" "$(OUTDIR)\genericdll.pch" "$(OUTDIR)\genericdll.bsc"


CLEAN :
	-@erase "$(INTDIR)\genericdll.obj"
	-@erase "$(INTDIR)\genericdll.pch"
	-@erase "$(INTDIR)\genericdll.res"
	-@erase "$(INTDIR)\genericdll.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\canalgeneric.dll"
	-@erase "$(OUTDIR)\canalgeneric.exp"
	-@erase "$(OUTDIR)\canalgeneric.ilk"
	-@erase "$(OUTDIR)\canalgeneric.lib"
	-@erase "$(OUTDIR)\canalgeneric.pdb"
	-@erase "$(OUTDIR)\genericdll.bsc"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\genericdll.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\genericdll.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\genericdll.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\genericdll.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=Wsock32.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\canalgeneric.pdb" /debug /machine:I386 /def:".\genericdll.def" /out:"$(OUTDIR)\canalgeneric.dll" /implib:"$(OUTDIR)\canalgeneric.lib" /pdbtype:sept 
DEF_FILE= \
	".\genericdll.def"
LINK32_OBJS= \
	"$(INTDIR)\genericdll.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\genericdll.res"

"$(OUTDIR)\canalgeneric.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\canalgeneric.dll" "$(OUTDIR)\genericdll.pch" "$(OUTDIR)\genericdll.bsc"
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
!IF EXISTS("genericdll.dep")
!INCLUDE "genericdll.dep"
!ELSE 
!MESSAGE Warning: cannot find "genericdll.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "genericdll - Win32 Release" || "$(CFG)" == "genericdll - Win32 Debug"
SOURCE=.\genericdll.cpp

!IF  "$(CFG)" == "genericdll - Win32 Release"


"$(INTDIR)\genericdll.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "genericdll - Win32 Debug"


"$(INTDIR)\genericdll.obj"	"$(INTDIR)\genericdll.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\genericdll.rc

"$(INTDIR)\genericdll.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "genericdll - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\genericdll.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\genericdll.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "genericdll - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\genericdll.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\genericdll.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

