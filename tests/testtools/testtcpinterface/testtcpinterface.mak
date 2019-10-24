# Microsoft Developer Studio Generated NMAKE File, Based on testtcpinterface.dsp
!IF "$(CFG)" == ""
CFG=testtcpinterface - Win32 Debug
!MESSAGE No configuration specified. Defaulting to testtcpinterface - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "testtcpinterface - Win32 Release" && "$(CFG)" != "testtcpinterface - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "testtcpinterface.mak" CFG="testtcpinterface - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "testtcpinterface - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "testtcpinterface - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "testtcpinterface - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\testtcpinterface.exe" "$(OUTDIR)\testtcpinterface.pch"


CLEAN :
	-@erase "$(INTDIR)\canaltcpif.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\testtcpinterface.obj"
	-@erase "$(INTDIR)\testtcpinterface.pch"
	-@erase "$(INTDIR)\testtcpinterface.res"
	-@erase "$(INTDIR)\testtcpinterfaceDlg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\testtcpinterface.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\testtcpinterface.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testtcpinterface.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib wsock32.lib netapi32.lib msvcrtd.lib libcd.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\testtcpinterface.pdb" /machine:I386 /out:"$(OUTDIR)\testtcpinterface.exe" 
LINK32_OBJS= \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\testtcpinterface.obj" \
	"$(INTDIR)\testtcpinterfaceDlg.obj" \
	"$(INTDIR)\testtcpinterface.res" \
	"$(INTDIR)\canaltcpif.obj"

"$(OUTDIR)\testtcpinterface.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "testtcpinterface - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\testtcpinterface.exe" "$(OUTDIR)\testtcpinterface.bsc" "$(OUTDIR)\testtcpinterface.pch"


CLEAN :
	-@erase "$(INTDIR)\canaltcpif.obj"
	-@erase "$(INTDIR)\canaltcpif.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\testtcpinterface.obj"
	-@erase "$(INTDIR)\testtcpinterface.pch"
	-@erase "$(INTDIR)\testtcpinterface.res"
	-@erase "$(INTDIR)\testtcpinterface.sbr"
	-@erase "$(INTDIR)\testtcpinterfaceDlg.obj"
	-@erase "$(INTDIR)\testtcpinterfaceDlg.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\testtcpinterface.bsc"
	-@erase "$(OUTDIR)\testtcpinterface.exe"
	-@erase "$(OUTDIR)\testtcpinterface.ilk"
	-@erase "$(OUTDIR)\testtcpinterface.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\testtcpinterface.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testtcpinterface.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\testtcpinterface.sbr" \
	"$(INTDIR)\testtcpinterfaceDlg.sbr" \
	"$(INTDIR)\canaltcpif.sbr"

"$(OUTDIR)\testtcpinterface.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmswd.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib wsock32.lib netapi32.lib msvcrtd.lib libcd.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\testtcpinterface.pdb" /debug /machine:I386 /out:"$(OUTDIR)\testtcpinterface.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\testtcpinterface.obj" \
	"$(INTDIR)\testtcpinterfaceDlg.obj" \
	"$(INTDIR)\testtcpinterface.res" \
	"$(INTDIR)\canaltcpif.obj"

"$(OUTDIR)\testtcpinterface.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("testtcpinterface.dep")
!INCLUDE "testtcpinterface.dep"
!ELSE 
!MESSAGE Warning: cannot find "testtcpinterface.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "testtcpinterface - Win32 Release" || "$(CFG)" == "testtcpinterface - Win32 Debug"
SOURCE=..\common\canaltcpif.cpp

!IF  "$(CFG)" == "testtcpinterface - Win32 Release"


"$(INTDIR)\canaltcpif.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "testtcpinterface - Win32 Debug"


"$(INTDIR)\canaltcpif.obj"	"$(INTDIR)\canaltcpif.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "testtcpinterface - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\testtcpinterface.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\testtcpinterface.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "testtcpinterface - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\testtcpinterface.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\testtcpinterface.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\testtcpinterface.cpp

!IF  "$(CFG)" == "testtcpinterface - Win32 Release"


"$(INTDIR)\testtcpinterface.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "testtcpinterface - Win32 Debug"


"$(INTDIR)\testtcpinterface.obj"	"$(INTDIR)\testtcpinterface.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\testtcpinterface.rc

"$(INTDIR)\testtcpinterface.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\testtcpinterfaceDlg.cpp

!IF  "$(CFG)" == "testtcpinterface - Win32 Release"


"$(INTDIR)\testtcpinterfaceDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "testtcpinterface - Win32 Debug"


"$(INTDIR)\testtcpinterfaceDlg.obj"	"$(INTDIR)\testtcpinterfaceDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

