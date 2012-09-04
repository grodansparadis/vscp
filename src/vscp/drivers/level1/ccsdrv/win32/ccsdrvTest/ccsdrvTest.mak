# Microsoft Developer Studio Generated NMAKE File, Based on ccsdrvTest.dsp
!IF "$(CFG)" == ""
CFG=ccsdrvTest - Win32 Debug
!MESSAGE No configuration specified. Defaulting to ccsdrvTest - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ccsdrvTest - Win32 Release" && "$(CFG)" != "ccsdrvTest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ccsdrvTest.mak" CFG="ccsdrvTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ccsdrvTest - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ccsdrvTest - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "ccsdrvTest - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\ccsdrvTest.exe" "$(OUTDIR)\ccsdrvTest.pch"


CLEAN :
	-@erase "$(INTDIR)\ccsdrvTest.obj"
	-@erase "$(INTDIR)\ccsdrvTest.pch"
	-@erase "$(INTDIR)\ccsdrvTest.res"
	-@erase "$(INTDIR)\ccsdrvTestDlg.obj"
	-@erase "$(INTDIR)\ccsobj.obj"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\ccsdrvTest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\ccsdrvTest.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ccsdrvTest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=can.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\ccsdrvTest.pdb" /machine:I386 /out:"$(OUTDIR)\ccsdrvTest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ccsdrvTest.obj" \
	"$(INTDIR)\ccsdrvTestDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\ccsdrvTest.res" \
	"$(INTDIR)\ccsobj.obj" \
	"$(INTDIR)\dllist.obj"

"$(OUTDIR)\ccsdrvTest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ccsdrvTest - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\ccsdrvTest.exe" "$(OUTDIR)\ccsdrvTest.pch" "$(OUTDIR)\ccsdrvTest.bsc"


CLEAN :
	-@erase "$(INTDIR)\ccsdrvTest.obj"
	-@erase "$(INTDIR)\ccsdrvTest.pch"
	-@erase "$(INTDIR)\ccsdrvTest.res"
	-@erase "$(INTDIR)\ccsdrvTest.sbr"
	-@erase "$(INTDIR)\ccsdrvTestDlg.obj"
	-@erase "$(INTDIR)\ccsdrvTestDlg.sbr"
	-@erase "$(INTDIR)\ccsobj.obj"
	-@erase "$(INTDIR)\ccsobj.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\ccsdrvTest.bsc"
	-@erase "$(OUTDIR)\ccsdrvTest.exe"
	-@erase "$(OUTDIR)\ccsdrvTest.ilk"
	-@erase "$(OUTDIR)\ccsdrvTest.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\ccsdrvTest.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ccsdrvTest.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ccsdrvTest.sbr" \
	"$(INTDIR)\ccsdrvTestDlg.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\ccsobj.sbr" \
	"$(INTDIR)\dllist.sbr"

"$(OUTDIR)\ccsdrvTest.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=candbg.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\ccsdrvTest.pdb" /debug /machine:I386 /out:"$(OUTDIR)\ccsdrvTest.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\ccsdrvTest.obj" \
	"$(INTDIR)\ccsdrvTestDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\ccsdrvTest.res" \
	"$(INTDIR)\ccsobj.obj" \
	"$(INTDIR)\dllist.obj"

"$(OUTDIR)\ccsdrvTest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("ccsdrvTest.dep")
!INCLUDE "ccsdrvTest.dep"
!ELSE 
!MESSAGE Warning: cannot find "ccsdrvTest.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ccsdrvTest - Win32 Release" || "$(CFG)" == "ccsdrvTest - Win32 Debug"
SOURCE=.\ccsdrvTest.cpp

!IF  "$(CFG)" == "ccsdrvTest - Win32 Release"


"$(INTDIR)\ccsdrvTest.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "ccsdrvTest - Win32 Debug"


"$(INTDIR)\ccsdrvTest.obj"	"$(INTDIR)\ccsdrvTest.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ccsdrvTest.rc

"$(INTDIR)\ccsdrvTest.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\ccsdrvTestDlg.cpp

!IF  "$(CFG)" == "ccsdrvTest - Win32 Release"


"$(INTDIR)\ccsdrvTestDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "ccsdrvTest - Win32 Debug"


"$(INTDIR)\ccsdrvTestDlg.obj"	"$(INTDIR)\ccsdrvTestDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\..\common\ccsobj.cpp

!IF  "$(CFG)" == "ccsdrvTest - Win32 Release"


"$(INTDIR)\ccsobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ccsdrvTest - Win32 Debug"


"$(INTDIR)\ccsobj.obj"	"$(INTDIR)\ccsobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\..\..\common\dllist.c

!IF  "$(CFG)" == "ccsdrvTest - Win32 Release"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ccsdrvTest - Win32 Debug"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "ccsdrvTest - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\ccsdrvTest.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\ccsdrvTest.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ccsdrvTest - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\ccsdrvTest.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\ccsdrvTest.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

