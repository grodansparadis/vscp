# Microsoft Developer Studio Generated NMAKE File, Based on testDriver.dsp
!IF "$(CFG)" == ""
CFG=testDriver - Win32 Debug
!MESSAGE No configuration specified. Defaulting to testDriver - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "testDriver - Win32 Release" && "$(CFG)" != "testDriver - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "testDriver.mak" CFG="testDriver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "testDriver - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "testDriver - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "testDriver - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\testDriver.exe" "$(OUTDIR)\testDriver.pch" "$(OUTDIR)\testDriver.bsc"


CLEAN :
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\testDriver.obj"
	-@erase "$(INTDIR)\testDriver.pch"
	-@erase "$(INTDIR)\testDriver.res"
	-@erase "$(INTDIR)\testDriver.sbr"
	-@erase "$(INTDIR)\testDriverDlg.obj"
	-@erase "$(INTDIR)\testDriverDlg.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\testDriver.bsc"
	-@erase "$(OUTDIR)\testDriver.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\testDriver.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testDriver.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\testDriver.sbr" \
	"$(INTDIR)\testDriverDlg.sbr"

"$(OUTDIR)\testDriver.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib FTD2XX.lib ../../libs/Microsoft/canusbdrv.lib  /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\testDriver.pdb" /machine:I386 /out:"$(OUTDIR)\testDriver.exe" 
LINK32_OBJS= \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\testDriver.obj" \
	"$(INTDIR)\testDriverDlg.obj" \
	"$(INTDIR)\testDriver.res"

"$(OUTDIR)\testDriver.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\testDriver.exe" "$(OUTDIR)\testDriver.pch" "$(OUTDIR)\testDriver.bsc"
   copy release\*.exe ..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "testDriver - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\testDriver.exe" "$(OUTDIR)\testDriver.pch" "$(OUTDIR)\testDriver.bsc"


CLEAN :
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\testDriver.obj"
	-@erase "$(INTDIR)\testDriver.pch"
	-@erase "$(INTDIR)\testDriver.res"
	-@erase "$(INTDIR)\testDriver.sbr"
	-@erase "$(INTDIR)\testDriverDlg.obj"
	-@erase "$(INTDIR)\testDriverDlg.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\testDriver.bsc"
	-@erase "$(OUTDIR)\testDriver.exe"
	-@erase "$(OUTDIR)\testDriver.ilk"
	-@erase "$(OUTDIR)\testDriver.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\testDriver.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testDriver.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\testDriver.sbr" \
	"$(INTDIR)\testDriverDlg.sbr"

"$(OUTDIR)\testDriver.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib FTD2XX.lib ../../libs/Microsoft/canusbdrv.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\testDriver.pdb" /debug /machine:I386 /out:"$(OUTDIR)\testDriver.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\testDriver.obj" \
	"$(INTDIR)\testDriverDlg.obj" \
	"$(INTDIR)\testDriver.res"

"$(OUTDIR)\testDriver.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("testDriver.dep")
!INCLUDE "testDriver.dep"
!ELSE 
!MESSAGE Warning: cannot find "testDriver.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "testDriver - Win32 Release" || "$(CFG)" == "testDriver - Win32 Debug"
SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "testDriver - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\testDriver.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\testDriver.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "testDriver - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\testDriver.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\testDriver.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\testDriver.cpp

"$(INTDIR)\testDriver.obj"	"$(INTDIR)\testDriver.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\testDriver.rc

"$(INTDIR)\testDriver.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\testDriverDlg.cpp

"$(INTDIR)\testDriverDlg.obj"	"$(INTDIR)\testDriverDlg.sbr" : $(SOURCE) "$(INTDIR)"



!ENDIF 

