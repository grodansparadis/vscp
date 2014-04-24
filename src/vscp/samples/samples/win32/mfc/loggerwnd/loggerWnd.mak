# Microsoft Developer Studio Generated NMAKE File, Based on loggerWnd.dsp
!IF "$(CFG)" == ""
CFG=loggerWnd - Win32 Debug
!MESSAGE No configuration specified. Defaulting to loggerWnd - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "loggerWnd - Win32 Release" && "$(CFG)" != "loggerWnd - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "loggerWnd.mak" CFG="loggerWnd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "loggerWnd - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "loggerWnd - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "loggerWnd - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\loggerWnd.exe"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\DlgFilter.obj"
	-@erase "$(INTDIR)\loggerWnd.obj"
	-@erase "$(INTDIR)\loggerWnd.res"
	-@erase "$(INTDIR)\loggerWndDlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\loggerWnd.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\loggerWnd.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\loggerWnd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=Wsock32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\loggerWnd.pdb" /machine:I386 /out:"$(OUTDIR)\loggerWnd.exe" 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\DlgFilter.obj" \
	"$(INTDIR)\loggerWnd.obj" \
	"$(INTDIR)\loggerWndDlg.obj" \
	"$(INTDIR)\loggerWnd.res" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\loggerWnd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\loggerWnd.exe"
   copy release\*.exe ..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "loggerWnd - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\loggerWnd.exe" "$(OUTDIR)\loggerWnd.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\DlgFilter.obj"
	-@erase "$(INTDIR)\DlgFilter.sbr"
	-@erase "$(INTDIR)\loggerWnd.obj"
	-@erase "$(INTDIR)\loggerWnd.res"
	-@erase "$(INTDIR)\loggerWnd.sbr"
	-@erase "$(INTDIR)\loggerWndDlg.obj"
	-@erase "$(INTDIR)\loggerWndDlg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\loggerWnd.bsc"
	-@erase "$(OUTDIR)\loggerWnd.exe"
	-@erase "$(OUTDIR)\loggerWnd.ilk"
	-@erase "$(OUTDIR)\loggerWnd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\mswd" /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\loggerWnd.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\loggerWnd.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\DlgFilter.sbr" \
	"$(INTDIR)\loggerWnd.sbr" \
	"$(INTDIR)\loggerWndDlg.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\loggerWnd.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=Wsock32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\loggerWnd.pdb" /debug /machine:I386 /out:"$(OUTDIR)\loggerWnd.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\DlgFilter.obj" \
	"$(INTDIR)\loggerWnd.obj" \
	"$(INTDIR)\loggerWndDlg.obj" \
	"$(INTDIR)\loggerWnd.res" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\loggerWnd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("loggerWnd.dep")
!INCLUDE "loggerWnd.dep"
!ELSE 
!MESSAGE Warning: cannot find "loggerWnd.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "loggerWnd - Win32 Release" || "$(CFG)" == "loggerWnd - Win32 Debug"
SOURCE=..\common\canalshmem_level1_win32.cpp

!IF  "$(CFG)" == "loggerWnd - Win32 Release"


"$(INTDIR)\canalshmem_level1_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "loggerWnd - Win32 Debug"


"$(INTDIR)\canalshmem_level1_win32.obj"	"$(INTDIR)\canalshmem_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\DlgFilter.cpp

!IF  "$(CFG)" == "loggerWnd - Win32 Release"


"$(INTDIR)\DlgFilter.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "loggerWnd - Win32 Debug"


"$(INTDIR)\DlgFilter.obj"	"$(INTDIR)\DlgFilter.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\loggerWnd.cpp

!IF  "$(CFG)" == "loggerWnd - Win32 Release"


"$(INTDIR)\loggerWnd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "loggerWnd - Win32 Debug"


"$(INTDIR)\loggerWnd.obj"	"$(INTDIR)\loggerWnd.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\loggerWnd.rc

"$(INTDIR)\loggerWnd.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\loggerWndDlg.cpp

!IF  "$(CFG)" == "loggerWnd - Win32 Release"


"$(INTDIR)\loggerWndDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "loggerWnd - Win32 Debug"


"$(INTDIR)\loggerWndDlg.obj"	"$(INTDIR)\loggerWndDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "loggerWnd - Win32 Release"


"$(INTDIR)\StdAfx.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "loggerWnd - Win32 Debug"


"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

