# Microsoft Developer Studio Generated NMAKE File, Based on canpiedrv.dsp
!IF "$(CFG)" == ""
CFG=canpiedrv - Win32 Debug
!MESSAGE No configuration specified. Defaulting to canpiedrv - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "canpiedrv - Win32 Release" && "$(CFG)" != "canpiedrv - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "canpiedrv.mak" CFG="canpiedrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "canpiedrv - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "canpiedrv - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "canpiedrv - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\canpiedrv.dll" "$(OUTDIR)\canpiedrv.pch"


CLEAN :
	-@erase "$(INTDIR)\canpiedrv.obj"
	-@erase "$(INTDIR)\canpiedrv.pch"
	-@erase "$(INTDIR)\canpiedrv.res"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\canpiedrv.dll"
	-@erase "$(OUTDIR)\canpiedrv.exp"
	-@erase "$(OUTDIR)\canpiedrv.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\canpiedrv.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\canpiedrv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\canpiedrv.pdb" /machine:I386 /def:".\canpiedrv.def" /out:"$(OUTDIR)\canpiedrv.dll" /implib:"$(OUTDIR)\canpiedrv.lib" 
DEF_FILE= \
	".\canpiedrv.def"
LINK32_OBJS= \
	"$(INTDIR)\canpiedrv.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\canpiedrv.res"

"$(OUTDIR)\canpiedrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\canpiedrv.dll" "$(OUTDIR)\canpiedrv.pch"
   copy release\*.dll ..\..\..\delivery
	copy release\*.lib ..\..\..\delivery
	copy release\*.dll D:\WINNT\system32
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "canpiedrv - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\canpiedrv.dll" "$(OUTDIR)\canpiedrv.pch"


CLEAN :
	-@erase "$(INTDIR)\canpiedrv.obj"
	-@erase "$(INTDIR)\canpiedrv.pch"
	-@erase "$(INTDIR)\canpiedrv.res"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\canpiedrv.dll"
	-@erase "$(OUTDIR)\canpiedrv.exp"
	-@erase "$(OUTDIR)\canpiedrv.ilk"
	-@erase "$(OUTDIR)\canpiedrv.lib"
	-@erase "$(OUTDIR)\canpiedrv.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\canpiedrv.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\canpiedrv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\canpiedrv.pdb" /debug /machine:I386 /def:".\canpiedrv.def" /out:"$(OUTDIR)\canpiedrv.dll" /implib:"$(OUTDIR)\canpiedrv.lib" /pdbtype:sept 
DEF_FILE= \
	".\canpiedrv.def"
LINK32_OBJS= \
	"$(INTDIR)\canpiedrv.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\canpiedrv.res"

"$(OUTDIR)\canpiedrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("canpiedrv.dep")
!INCLUDE "canpiedrv.dep"
!ELSE 
!MESSAGE Warning: cannot find "canpiedrv.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "canpiedrv - Win32 Release" || "$(CFG)" == "canpiedrv - Win32 Debug"
SOURCE=.\canpiedrv.cpp

"$(INTDIR)\canpiedrv.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\canpiedrv.rc

"$(INTDIR)\canpiedrv.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "canpiedrv - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\canpiedrv.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\canpiedrv.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "canpiedrv - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\canpiedrv.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\canpiedrv.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

