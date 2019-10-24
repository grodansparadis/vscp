# Microsoft Developer Studio Generated NMAKE File, Based on tesifdll.dsp
!IF "$(CFG)" == ""
CFG=tesifdll - Win32 Debug
!MESSAGE No configuration specified. Defaulting to tesifdll - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "tesifdll - Win32 Release" && "$(CFG)" != "tesifdll - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tesifdll.mak" CFG="tesifdll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tesifdll - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "tesifdll - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "tesifdll - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\tesifdll.exe" "$(OUTDIR)\tesifdll.pch"


CLEAN :
	-@erase "$(INTDIR)\dllwrapper.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\tesifdll.obj"
	-@erase "$(INTDIR)\tesifdll.pch"
	-@erase "$(INTDIR)\tesifdll.res"
	-@erase "$(INTDIR)\tesifdllDlg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\tesifdll.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\tesifdll.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tesifdll.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=mfcs42.lib msvcrt.lib $(WXWIN)\lib\wxmsw.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\tesifdll.pdb" /machine:I386 /nodefaultlib:"msvcrt.lib mfcs42.lib" /out:"$(OUTDIR)\tesifdll.exe" 
LINK32_OBJS= \
	"$(INTDIR)\dllwrapper.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\tesifdll.obj" \
	"$(INTDIR)\tesifdllDlg.obj" \
	"$(INTDIR)\tesifdll.res"

"$(OUTDIR)\tesifdll.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tesifdll - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\tesifdll.exe" "$(OUTDIR)\tesifdll.pch" "$(OUTDIR)\tesifdll.bsc"


CLEAN :
	-@erase "$(INTDIR)\dllwrapper.obj"
	-@erase "$(INTDIR)\dllwrapper.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\tesifdll.obj"
	-@erase "$(INTDIR)\tesifdll.pch"
	-@erase "$(INTDIR)\tesifdll.res"
	-@erase "$(INTDIR)\tesifdll.sbr"
	-@erase "$(INTDIR)\tesifdllDlg.obj"
	-@erase "$(INTDIR)\tesifdllDlg.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\tesifdll.bsc"
	-@erase "$(OUTDIR)\tesifdll.exe"
	-@erase "$(OUTDIR)\tesifdll.ilk"
	-@erase "$(OUTDIR)\tesifdll.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\tesifdll.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tesifdll.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\dllwrapper.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\tesifdll.sbr" \
	"$(INTDIR)\tesifdllDlg.sbr"

"$(OUTDIR)\tesifdll.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=MSVCRTD.lib nafxcwd.lib $(WXWIN)\lib\wxmswd.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\tesifdll.pdb" /debug /machine:I386 /nodefaultlib:"MSVCRTD.lib nafxcwd.lib" /out:"$(OUTDIR)\tesifdll.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\dllwrapper.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\tesifdll.obj" \
	"$(INTDIR)\tesifdllDlg.obj" \
	"$(INTDIR)\tesifdll.res"

"$(OUTDIR)\tesifdll.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("tesifdll.dep")
!INCLUDE "tesifdll.dep"
!ELSE 
!MESSAGE Warning: cannot find "tesifdll.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "tesifdll - Win32 Release" || "$(CFG)" == "tesifdll - Win32 Debug"
SOURCE=..\..\canal\common\dllwrapper.cpp

!IF  "$(CFG)" == "tesifdll - Win32 Release"


"$(INTDIR)\dllwrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tesifdll - Win32 Debug"


"$(INTDIR)\dllwrapper.obj"	"$(INTDIR)\dllwrapper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "tesifdll - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Fp"$(INTDIR)\tesifdll.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\tesifdll.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tesifdll - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\tesifdll.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\tesifdll.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\tesifdll.cpp

!IF  "$(CFG)" == "tesifdll - Win32 Release"


"$(INTDIR)\tesifdll.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tesifdll - Win32 Debug"


"$(INTDIR)\tesifdll.obj"	"$(INTDIR)\tesifdll.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\tesifdll.rc

"$(INTDIR)\tesifdll.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\tesifdllDlg.cpp

!IF  "$(CFG)" == "tesifdll - Win32 Release"


"$(INTDIR)\tesifdllDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tesifdll - Win32 Debug"


"$(INTDIR)\tesifdllDlg.obj"	"$(INTDIR)\tesifdllDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

