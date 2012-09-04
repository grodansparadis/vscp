# Microsoft Developer Studio Generated NMAKE File, Based on irtest.dsp
!IF "$(CFG)" == ""
CFG=irtest - Win32 Debug
!MESSAGE No configuration specified. Defaulting to irtest - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "irtest - Win32 Release" && "$(CFG)" != "irtest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "irtest.mak" CFG="irtest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "irtest - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "irtest - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "irtest - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\irtest.exe"


CLEAN :
	-@erase "$(INTDIR)\irtest.obj"
	-@erase "$(INTDIR)\irtest.pch"
	-@erase "$(INTDIR)\irtest.res"
	-@erase "$(INTDIR)\irtestDlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\irtest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\irtest.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\irtest.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\irtest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\irtest.pdb" /machine:I386 /out:"$(OUTDIR)\irtest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\irtest.obj" \
	"$(INTDIR)\irtestDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\irtest.res"

"$(OUTDIR)\irtest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "irtest - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\irtest.exe"


CLEAN :
	-@erase "$(INTDIR)\irtest.obj"
	-@erase "$(INTDIR)\irtest.pch"
	-@erase "$(INTDIR)\irtest.res"
	-@erase "$(INTDIR)\irtestDlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\irtest.exe"
	-@erase "$(OUTDIR)\irtest.ilk"
	-@erase "$(OUTDIR)\irtest.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\irtest.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ   /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\irtest.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\irtest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\irtest.pdb" /debug /machine:I386 /out:"$(OUTDIR)\irtest.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\irtest.obj" \
	"$(INTDIR)\irtestDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\irtest.res"

"$(OUTDIR)\irtest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("irtest.dep")
!INCLUDE "irtest.dep"
!ELSE 
!MESSAGE Warning: cannot find "irtest.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "irtest - Win32 Release" || "$(CFG)" == "irtest - Win32 Debug"
SOURCE=.\irtest.cpp

"$(INTDIR)\irtest.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\irtest.pch"


SOURCE=.\irtest.rc

"$(INTDIR)\irtest.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\irtestDlg.cpp

"$(INTDIR)\irtestDlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\irtest.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "irtest - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\irtest.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\irtest.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "irtest - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\irtest.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ   /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\irtest.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

