# Microsoft Developer Studio Generated NMAKE File, Based on testPipeIf.dsp
!IF "$(CFG)" == ""
CFG=testPipeIf - Win32 Debug
!MESSAGE No configuration specified. Defaulting to testPipeIf - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "testPipeIf - Win32 Release" && "$(CFG)" != "testPipeIf - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "testPipeIf.mak" CFG="testPipeIf - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "testPipeIf - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "testPipeIf - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "testPipeIf - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\testPipeIf.exe" "$(OUTDIR)\testPipeIf.pch" "$(OUTDIR)\testPipeIf.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\testPipeIf.obj"
	-@erase "$(INTDIR)\testPipeIf.pch"
	-@erase "$(INTDIR)\testPipeIf.res"
	-@erase "$(INTDIR)\testPipeIf.sbr"
	-@erase "$(INTDIR)\testPipeIfDlg.obj"
	-@erase "$(INTDIR)\testPipeIfDlg.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\testPipeIf.bsc"
	-@erase "$(OUTDIR)\testPipeIf.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\testPipeIf.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testPipeIf.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\testPipeIf.sbr" \
	"$(INTDIR)\testPipeIfDlg.sbr"

"$(OUTDIR)\testPipeIf.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib libexpat.lib net.lib netapi32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\testPipeIf.pdb" /machine:I386 /out:"$(OUTDIR)\testPipeIf.exe" 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\testPipeIf.obj" \
	"$(INTDIR)\testPipeIfDlg.obj" \
	"$(INTDIR)\testPipeIf.res"

"$(OUTDIR)\testPipeIf.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\testPipeIf.exe" "$(OUTDIR)\testPipeIf.pch" "$(OUTDIR)\testPipeIf.bsc"
   copy release\*.exe ..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "testPipeIf - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\testPipeIf.exe" "$(OUTDIR)\testPipeIf.pch" "$(OUTDIR)\testPipeIf.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\testPipeIf.obj"
	-@erase "$(INTDIR)\testPipeIf.pch"
	-@erase "$(INTDIR)\testPipeIf.res"
	-@erase "$(INTDIR)\testPipeIf.sbr"
	-@erase "$(INTDIR)\testPipeIfDlg.obj"
	-@erase "$(INTDIR)\testPipeIfDlg.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\testPipeIf.bsc"
	-@erase "$(OUTDIR)\testPipeIf.exe"
	-@erase "$(OUTDIR)\testPipeIf.ilk"
	-@erase "$(OUTDIR)\testPipeIf.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\mswd" /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\testPipeIf.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testPipeIf.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\testPipeIf.sbr" \
	"$(INTDIR)\testPipeIfDlg.sbr"

"$(OUTDIR)\testPipeIf.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=zlibd.lib regexd.lib pngd.lib jpegd.lib tiffd.lib wxmswd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib netapi32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\testPipeIf.pdb" /debug /machine:I386 /out:"$(OUTDIR)\testPipeIf.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\testPipeIf.obj" \
	"$(INTDIR)\testPipeIfDlg.obj" \
	"$(INTDIR)\testPipeIf.res"

"$(OUTDIR)\testPipeIf.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("testPipeIf.dep")
!INCLUDE "testPipeIf.dep"
!ELSE 
!MESSAGE Warning: cannot find "testPipeIf.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "testPipeIf - Win32 Release" || "$(CFG)" == "testPipeIf - Win32 Debug"
SOURCE=..\common\canalshmem_level1_win32.cpp

"$(INTDIR)\canalshmem_level1_win32.obj"	"$(INTDIR)\canalshmem_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "testPipeIf - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\testPipeIf.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\testPipeIf.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "testPipeIf - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\mswd" /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\testPipeIf.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\testPipeIf.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\testPipeIf.cpp

"$(INTDIR)\testPipeIf.obj"	"$(INTDIR)\testPipeIf.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\testPipeIf.rc

"$(INTDIR)\testPipeIf.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\testPipeIfDlg.cpp

"$(INTDIR)\testPipeIfDlg.obj"	"$(INTDIR)\testPipeIfDlg.sbr" : $(SOURCE) "$(INTDIR)"



!ENDIF 

