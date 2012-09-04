# Microsoft Developer Studio Generated NMAKE File, Based on can232drvTest.dsp
!IF "$(CFG)" == ""
CFG=can232drvTest - Win32 Debug
!MESSAGE No configuration specified. Defaulting to can232drvTest - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "can232drvTest - Win32 Release" && "$(CFG)" != "can232drvTest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "can232drvTest.mak" CFG="can232drvTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "can232drvTest - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "can232drvTest - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "can232drvTest - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\can232drvTest.exe" "$(OUTDIR)\can232drvTest.pch"


CLEAN :
	-@erase "$(INTDIR)\can232drvTest.obj"
	-@erase "$(INTDIR)\can232drvTest.pch"
	-@erase "$(INTDIR)\can232drvTest.res"
	-@erase "$(INTDIR)\can232drvTestDlg.obj"
	-@erase "$(INTDIR)\can232obj.obj"
	-@erase "$(INTDIR)\com_win32.obj"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\can232drvTest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\can232drvTest.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\can232drvTest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\can232drvTest.pdb" /machine:I386 /out:"$(OUTDIR)\can232drvTest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\can232drvTest.obj" \
	"$(INTDIR)\can232drvTestDlg.obj" \
	"$(INTDIR)\can232obj.obj" \
	"$(INTDIR)\com_win32.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\can232drvTest.res"

"$(OUTDIR)\can232drvTest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "can232drvTest - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\can232drvTest.exe" "$(OUTDIR)\can232drvTest.pch" "$(OUTDIR)\can232drvTest.bsc"


CLEAN :
	-@erase "$(INTDIR)\can232drvTest.obj"
	-@erase "$(INTDIR)\can232drvTest.pch"
	-@erase "$(INTDIR)\can232drvTest.res"
	-@erase "$(INTDIR)\can232drvTest.sbr"
	-@erase "$(INTDIR)\can232drvTestDlg.obj"
	-@erase "$(INTDIR)\can232drvTestDlg.sbr"
	-@erase "$(INTDIR)\can232obj.obj"
	-@erase "$(INTDIR)\can232obj.sbr"
	-@erase "$(INTDIR)\com_win32.obj"
	-@erase "$(INTDIR)\com_win32.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\can232drvTest.bsc"
	-@erase "$(OUTDIR)\can232drvTest.exe"
	-@erase "$(OUTDIR)\can232drvTest.ilk"
	-@erase "$(OUTDIR)\can232drvTest.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\can232drvTest.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\can232drvTest.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\can232drvTest.sbr" \
	"$(INTDIR)\can232drvTestDlg.sbr" \
	"$(INTDIR)\can232obj.sbr" \
	"$(INTDIR)\com_win32.sbr" \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\can232drvTest.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\can232drvTest.pdb" /debug /machine:I386 /out:"$(OUTDIR)\can232drvTest.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\can232drvTest.obj" \
	"$(INTDIR)\can232drvTestDlg.obj" \
	"$(INTDIR)\can232obj.obj" \
	"$(INTDIR)\com_win32.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\can232drvTest.res"

"$(OUTDIR)\can232drvTest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("can232drvTest.dep")
!INCLUDE "can232drvTest.dep"
!ELSE 
!MESSAGE Warning: cannot find "can232drvTest.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "can232drvTest - Win32 Release" || "$(CFG)" == "can232drvTest - Win32 Debug"
SOURCE=.\can232drvTest.cpp

!IF  "$(CFG)" == "can232drvTest - Win32 Release"


"$(INTDIR)\can232drvTest.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "can232drvTest - Win32 Debug"


"$(INTDIR)\can232drvTest.obj"	"$(INTDIR)\can232drvTest.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\can232drvTest.rc

"$(INTDIR)\can232drvTest.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\can232drvTestDlg.cpp

!IF  "$(CFG)" == "can232drvTest - Win32 Release"


"$(INTDIR)\can232drvTestDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "can232drvTest - Win32 Debug"


"$(INTDIR)\can232drvTestDlg.obj"	"$(INTDIR)\can232drvTestDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\..\common\can232obj.cpp

!IF  "$(CFG)" == "can232drvTest - Win32 Release"


"$(INTDIR)\can232obj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "can232drvTest - Win32 Debug"


"$(INTDIR)\can232obj.obj"	"$(INTDIR)\can232obj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\..\..\common\com_win32.cpp

!IF  "$(CFG)" == "can232drvTest - Win32 Release"


"$(INTDIR)\com_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "can232drvTest - Win32 Debug"


"$(INTDIR)\com_win32.obj"	"$(INTDIR)\com_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\..\..\common\dllist.c

!IF  "$(CFG)" == "can232drvTest - Win32 Release"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "can232drvTest - Win32 Debug"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "can232drvTest - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\can232drvTest.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\can232drvTest.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "can232drvTest - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\can232drvTest.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\can232drvTest.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

