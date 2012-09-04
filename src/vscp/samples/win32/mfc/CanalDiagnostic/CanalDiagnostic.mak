# Microsoft Developer Studio Generated NMAKE File, Based on CanalDiagnostic.dsp
!IF "$(CFG)" == ""
CFG=CanalDiagnostic - Win32 Debug
!MESSAGE No configuration specified. Defaulting to CanalDiagnostic - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "CanalDiagnostic - Win32 Release" && "$(CFG)" != "CanalDiagnostic - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CanalDiagnostic.mak" CFG="CanalDiagnostic - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CanalDiagnostic - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "CanalDiagnostic - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "CanalDiagnostic - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\CanalDiagnostic.exe" "$(OUTDIR)\CanalDiagnostic.pch" "$(OUTDIR)\CanalDiagnostic.bsc"


CLEAN :
	-@erase "$(INTDIR)\CanalDiagnostic.obj"
	-@erase "$(INTDIR)\CanalDiagnostic.pch"
	-@erase "$(INTDIR)\CanalDiagnostic.res"
	-@erase "$(INTDIR)\CanalDiagnostic.sbr"
	-@erase "$(INTDIR)\CanalDiagnosticDoc.obj"
	-@erase "$(INTDIR)\CanalDiagnosticDoc.sbr"
	-@erase "$(INTDIR)\CanalDiagnosticView.obj"
	-@erase "$(INTDIR)\CanalDiagnosticView.sbr"
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\CanalSuperWrapper.obj"
	-@erase "$(INTDIR)\CanalSuperWrapper.sbr"
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\ChildFrm.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.sbr"
	-@erase "$(INTDIR)\DocumentProperties.obj"
	-@erase "$(INTDIR)\DocumentProperties.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\NewDocument.obj"
	-@erase "$(INTDIR)\NewDocument.sbr"
	-@erase "$(INTDIR)\SendBurst.obj"
	-@erase "$(INTDIR)\SendBurst.sbr"
	-@erase "$(INTDIR)\Splash.obj"
	-@erase "$(INTDIR)\Splash.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\CanalDiagnostic.bsc"
	-@erase "$(OUTDIR)\CanalDiagnostic.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "CANAL_MFCAPP" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\CanalDiagnostic.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\CanalDiagnostic.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\CanalDiagnostic.sbr" \
	"$(INTDIR)\CanalDiagnosticDoc.sbr" \
	"$(INTDIR)\CanalDiagnosticView.sbr" \
	"$(INTDIR)\ChildFrm.sbr" \
	"$(INTDIR)\DocumentProperties.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\NewDocument.sbr" \
	"$(INTDIR)\SendBurst.sbr" \
	"$(INTDIR)\Splash.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\CanalSuperWrapper.sbr" \
	"$(INTDIR)\DllWrapper.sbr"

"$(OUTDIR)\CanalDiagnostic.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\CanalDiagnostic.pdb" /machine:I386 /out:"$(OUTDIR)\CanalDiagnostic.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CanalDiagnostic.obj" \
	"$(INTDIR)\CanalDiagnosticDoc.obj" \
	"$(INTDIR)\CanalDiagnosticView.obj" \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\DocumentProperties.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\NewDocument.obj" \
	"$(INTDIR)\SendBurst.obj" \
	"$(INTDIR)\Splash.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\CanalDiagnostic.res" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\CanalSuperWrapper.obj" \
	"$(INTDIR)\DllWrapper.obj"

"$(OUTDIR)\CanalDiagnostic.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\CanalDiagnostic.exe" "$(OUTDIR)\CanalDiagnostic.pch" "$(OUTDIR)\CanalDiagnostic.bsc"
   copy release\*.exe ..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "CanalDiagnostic - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\CanalDiagnostic.exe" "$(OUTDIR)\CanalDiagnostic.pch" "$(OUTDIR)\CanalDiagnostic.bsc"


CLEAN :
	-@erase "$(INTDIR)\CanalDiagnostic.obj"
	-@erase "$(INTDIR)\CanalDiagnostic.pch"
	-@erase "$(INTDIR)\CanalDiagnostic.res"
	-@erase "$(INTDIR)\CanalDiagnostic.sbr"
	-@erase "$(INTDIR)\CanalDiagnosticDoc.obj"
	-@erase "$(INTDIR)\CanalDiagnosticDoc.sbr"
	-@erase "$(INTDIR)\CanalDiagnosticView.obj"
	-@erase "$(INTDIR)\CanalDiagnosticView.sbr"
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\CanalSuperWrapper.obj"
	-@erase "$(INTDIR)\CanalSuperWrapper.sbr"
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\ChildFrm.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.sbr"
	-@erase "$(INTDIR)\DocumentProperties.obj"
	-@erase "$(INTDIR)\DocumentProperties.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\NewDocument.obj"
	-@erase "$(INTDIR)\NewDocument.sbr"
	-@erase "$(INTDIR)\SendBurst.obj"
	-@erase "$(INTDIR)\SendBurst.sbr"
	-@erase "$(INTDIR)\Splash.obj"
	-@erase "$(INTDIR)\Splash.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\CanalDiagnostic.bsc"
	-@erase "$(OUTDIR)\CanalDiagnostic.exe"
	-@erase "$(OUTDIR)\CanalDiagnostic.ilk"
	-@erase "$(OUTDIR)\CanalDiagnostic.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\mswd" /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "CANAL_MFCAPP" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\CanalDiagnostic.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\CanalDiagnostic.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\CanalDiagnostic.sbr" \
	"$(INTDIR)\CanalDiagnosticDoc.sbr" \
	"$(INTDIR)\CanalDiagnosticView.sbr" \
	"$(INTDIR)\ChildFrm.sbr" \
	"$(INTDIR)\DocumentProperties.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\NewDocument.sbr" \
	"$(INTDIR)\SendBurst.sbr" \
	"$(INTDIR)\Splash.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\CanalSuperWrapper.sbr" \
	"$(INTDIR)\DllWrapper.sbr"

"$(OUTDIR)\CanalDiagnostic.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\CanalDiagnostic.pdb" /debug /machine:I386 /out:"$(OUTDIR)\CanalDiagnostic.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\CanalDiagnostic.obj" \
	"$(INTDIR)\CanalDiagnosticDoc.obj" \
	"$(INTDIR)\CanalDiagnosticView.obj" \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\DocumentProperties.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\NewDocument.obj" \
	"$(INTDIR)\SendBurst.obj" \
	"$(INTDIR)\Splash.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\CanalDiagnostic.res" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\CanalSuperWrapper.obj" \
	"$(INTDIR)\DllWrapper.obj"

"$(OUTDIR)\CanalDiagnostic.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("CanalDiagnostic.dep")
!INCLUDE "CanalDiagnostic.dep"
!ELSE 
!MESSAGE Warning: cannot find "CanalDiagnostic.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "CanalDiagnostic - Win32 Release" || "$(CFG)" == "CanalDiagnostic - Win32 Debug"
SOURCE=.\CanalDiagnostic.cpp

"$(INTDIR)\CanalDiagnostic.obj"	"$(INTDIR)\CanalDiagnostic.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\CanalDiagnostic.rc

"$(INTDIR)\CanalDiagnostic.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\CanalDiagnosticDoc.cpp

"$(INTDIR)\CanalDiagnosticDoc.obj"	"$(INTDIR)\CanalDiagnosticDoc.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\CanalDiagnosticView.cpp

"$(INTDIR)\CanalDiagnosticView.obj"	"$(INTDIR)\CanalDiagnosticView.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\..\..\..\common\canalshmem_level1_win32.cpp

"$(INTDIR)\canalshmem_level1_win32.obj"	"$(INTDIR)\canalshmem_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\..\common\canalsuperwrapper.cpp

"$(INTDIR)\CanalSuperWrapper.obj"	"$(INTDIR)\CanalSuperWrapper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ChildFrm.cpp

"$(INTDIR)\ChildFrm.obj"	"$(INTDIR)\ChildFrm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\..\..\..\..\common\dllist.c

"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\..\common\DllWrapper.cpp

"$(INTDIR)\DllWrapper.obj"	"$(INTDIR)\DllWrapper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\DocumentProperties.cpp

"$(INTDIR)\DocumentProperties.obj"	"$(INTDIR)\DocumentProperties.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MainFrm.cpp

"$(INTDIR)\MainFrm.obj"	"$(INTDIR)\MainFrm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NewDocument.cpp

"$(INTDIR)\NewDocument.obj"	"$(INTDIR)\NewDocument.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\SendBurst.cpp

"$(INTDIR)\SendBurst.obj"	"$(INTDIR)\SendBurst.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Splash.cpp

"$(INTDIR)\Splash.obj"	"$(INTDIR)\Splash.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "CanalDiagnostic - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "CANAL_MFCAPP" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\CanalDiagnostic.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\CanalDiagnostic.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "CanalDiagnostic - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\mswd" /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "CANAL_MFCAPP" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\CanalDiagnostic.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\CanalDiagnostic.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

