# Microsoft Developer Studio Generated NMAKE File, Based on mr26.dsp
!IF "$(CFG)" == ""
CFG=mr26 - Win32 Debug
!MESSAGE No configuration specified. Defaulting to mr26 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "mr26 - Win32 Release" && "$(CFG)" != "mr26 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mr26.mak" CFG="mr26 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mr26 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "mr26 - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "mr26 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\mr26.exe"


CLEAN :
	-@erase "$(INTDIR)\Comm.obj"
	-@erase "$(INTDIR)\mr26.obj"
	-@erase "$(INTDIR)\mr26.pch"
	-@erase "$(INTDIR)\mr26.res"
	-@erase "$(INTDIR)\mr26Dlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mr26.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\mr26.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\mr26.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mr26.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\mr26.pdb" /machine:I386 /out:"$(OUTDIR)\mr26.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Comm.obj" \
	"$(INTDIR)\mr26.obj" \
	"$(INTDIR)\mr26Dlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\mr26.res"

"$(OUTDIR)\mr26.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mr26 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\mr26.exe" "$(OUTDIR)\mr26.pch" "$(OUTDIR)\mr26.bsc"


CLEAN :
	-@erase "$(INTDIR)\Comm.obj"
	-@erase "$(INTDIR)\Comm.sbr"
	-@erase "$(INTDIR)\mr26.obj"
	-@erase "$(INTDIR)\mr26.pch"
	-@erase "$(INTDIR)\mr26.res"
	-@erase "$(INTDIR)\mr26.sbr"
	-@erase "$(INTDIR)\mr26Dlg.obj"
	-@erase "$(INTDIR)\mr26Dlg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\mr26.bsc"
	-@erase "$(OUTDIR)\mr26.exe"
	-@erase "$(OUTDIR)\mr26.ilk"
	-@erase "$(OUTDIR)\mr26.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\mr26.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mr26.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Comm.sbr" \
	"$(INTDIR)\mr26.sbr" \
	"$(INTDIR)\mr26Dlg.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\mr26.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\mr26.pdb" /debug /machine:I386 /out:"$(OUTDIR)\mr26.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\Comm.obj" \
	"$(INTDIR)\mr26.obj" \
	"$(INTDIR)\mr26Dlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\mr26.res"

"$(OUTDIR)\mr26.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("mr26.dep")
!INCLUDE "mr26.dep"
!ELSE 
!MESSAGE Warning: cannot find "mr26.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "mr26 - Win32 Release" || "$(CFG)" == "mr26 - Win32 Debug"
SOURCE=.\Comm.cpp

!IF  "$(CFG)" == "mr26 - Win32 Release"


"$(INTDIR)\Comm.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mr26.pch"


!ELSEIF  "$(CFG)" == "mr26 - Win32 Debug"


"$(INTDIR)\Comm.obj"	"$(INTDIR)\Comm.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\mr26.cpp

!IF  "$(CFG)" == "mr26 - Win32 Release"


"$(INTDIR)\mr26.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mr26.pch"


!ELSEIF  "$(CFG)" == "mr26 - Win32 Debug"


"$(INTDIR)\mr26.obj"	"$(INTDIR)\mr26.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\mr26.rc

"$(INTDIR)\mr26.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\mr26Dlg.cpp

!IF  "$(CFG)" == "mr26 - Win32 Release"


"$(INTDIR)\mr26Dlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mr26.pch"


!ELSEIF  "$(CFG)" == "mr26 - Win32 Debug"


"$(INTDIR)\mr26Dlg.obj"	"$(INTDIR)\mr26Dlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "mr26 - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\mr26.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\mr26.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mr26 - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\mr26.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\mr26.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

