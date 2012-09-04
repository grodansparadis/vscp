# Microsoft Developer Studio Generated NMAKE File, Based on testpcan.dsp
!IF "$(CFG)" == ""
CFG=testpcan - Win32 Debug
!MESSAGE No configuration specified. Defaulting to testpcan - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "testpcan - Win32 Release" && "$(CFG)" != "testpcan - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "testpcan.mak" CFG="testpcan - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "testpcan - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "testpcan - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "testpcan - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\testpcan.exe" "$(OUTDIR)\testpcan.pch"


CLEAN :
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\peakobj.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\testpcan.obj"
	-@erase "$(INTDIR)\testpcan.pch"
	-@erase "$(INTDIR)\testpcan.res"
	-@erase "$(INTDIR)\testpcanDlg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\testpcan.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\testpcan.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testpcan.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=nafxcw.lib LIBCMT.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\testpcan.pdb" /machine:I386 /nodefaultlib:"LIBCMT.lib nafxcw.lib" /out:"$(OUTDIR)\testpcan.exe" 
LINK32_OBJS= \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\peakobj.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\testpcan.obj" \
	"$(INTDIR)\testpcanDlg.obj" \
	"$(INTDIR)\testpcan.res"

"$(OUTDIR)\testpcan.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "testpcan - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\testpcan.exe" "$(OUTDIR)\testpcan.pch" "$(OUTDIR)\testpcan.bsc"


CLEAN :
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\peakobj.obj"
	-@erase "$(INTDIR)\peakobj.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\testpcan.obj"
	-@erase "$(INTDIR)\testpcan.pch"
	-@erase "$(INTDIR)\testpcan.res"
	-@erase "$(INTDIR)\testpcan.sbr"
	-@erase "$(INTDIR)\testpcanDlg.obj"
	-@erase "$(INTDIR)\testpcanDlg.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\testpcan.bsc"
	-@erase "$(OUTDIR)\testpcan.exe"
	-@erase "$(OUTDIR)\testpcan.ilk"
	-@erase "$(OUTDIR)\testpcan.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\testpcan.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testpcan.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\peakobj.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\testpcan.sbr" \
	"$(INTDIR)\testpcanDlg.sbr"

"$(OUTDIR)\testpcan.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=nafxcwd.lib LIBCMTD.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\testpcan.pdb" /debug /machine:I386 /nodefaultlib:"LIBCMTD.lib nafxcwd.lib" /out:"$(OUTDIR)\testpcan.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\peakobj.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\testpcan.obj" \
	"$(INTDIR)\testpcanDlg.obj" \
	"$(INTDIR)\testpcan.res"

"$(OUTDIR)\testpcan.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("testpcan.dep")
!INCLUDE "testpcan.dep"
!ELSE 
!MESSAGE Warning: cannot find "testpcan.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "testpcan - Win32 Release" || "$(CFG)" == "testpcan - Win32 Debug"
SOURCE=..\..\..\..\..\common\dllist.c

!IF  "$(CFG)" == "testpcan - Win32 Release"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "testpcan - Win32 Debug"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\peakobj.cpp

!IF  "$(CFG)" == "testpcan - Win32 Release"


"$(INTDIR)\peakobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "testpcan - Win32 Debug"


"$(INTDIR)\peakobj.obj"	"$(INTDIR)\peakobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "testpcan - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\testpcan.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\testpcan.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "testpcan - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\testpcan.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\testpcan.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\testpcan.cpp

!IF  "$(CFG)" == "testpcan - Win32 Release"


"$(INTDIR)\testpcan.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "testpcan - Win32 Debug"


"$(INTDIR)\testpcan.obj"	"$(INTDIR)\testpcan.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\testpcan.rc

"$(INTDIR)\testpcan.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\testpcanDlg.cpp

!IF  "$(CFG)" == "testpcan - Win32 Release"


"$(INTDIR)\testpcanDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "testpcan - Win32 Debug"


"$(INTDIR)\testpcanDlg.obj"	"$(INTDIR)\testpcanDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

