# Microsoft Developer Studio Generated NMAKE File, Based on vscpdll.dsp
!IF "$(CFG)" == ""
CFG=vscpdll - Win32 Debug
!MESSAGE No configuration specified. Defaulting to vscpdll - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "vscpdll - Win32 Release" && "$(CFG)" != "vscpdll - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vscpdll.mak" CFG="vscpdll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vscpdll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "vscpdll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "vscpdll - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\vscp.dll" "$(OUTDIR)\vscpdll.pch"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vscpdll.obj"
	-@erase "$(INTDIR)\vscpdll.pch"
	-@erase "$(INTDIR)\vscpdll.res"
	-@erase "$(OUTDIR)\vscp.dll"
	-@erase "$(OUTDIR)\vscp.exp"
	-@erase "$(OUTDIR)\vscp.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_WINDLL" /D "_AFXDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\vscpdll.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\vscpdll.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=mfcs42.lib MSVCRT.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\vscp.pdb" /machine:I386 /nodefaultlib:"MSVCRT.lib mfcs42.lib" /def:".\vscpdll.def" /out:"$(OUTDIR)\vscp.dll" /implib:"$(OUTDIR)\vscp.lib" 
DEF_FILE= \
	".\vscpdll.def"
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\vscpdll.obj" \
	"$(INTDIR)\vscpdll.res"

"$(OUTDIR)\vscp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\vscp.dll" "$(OUTDIR)\vscpdll.pch"
   copy release\*.dll ..\deliveryx
	copy release\*.lib ..\deliveryx
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "vscpdll - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\vscp.dll" "$(OUTDIR)\vscpdll.pch" "$(OUTDIR)\vscpdll.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vscpdll.obj"
	-@erase "$(INTDIR)\vscpdll.pch"
	-@erase "$(INTDIR)\vscpdll.res"
	-@erase "$(INTDIR)\vscpdll.sbr"
	-@erase "$(OUTDIR)\vscp.dll"
	-@erase "$(OUTDIR)\vscp.exp"
	-@erase "$(OUTDIR)\vscp.ilk"
	-@erase "$(OUTDIR)\vscp.lib"
	-@erase "$(OUTDIR)\vscp.pdb"
	-@erase "$(OUTDIR)\vscpdll.bsc"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_WINDLL" /D "_AFXDLL" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\vscpdll.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\vscpdll.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level2_win32.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\vscpdll.sbr"

"$(OUTDIR)\vscpdll.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmswd.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib wsock32.lib netapi32.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\vscp.pdb" /debug /machine:I386 /nodefaultlib:"mfcs42d.lib MSVCRTD.lib" /def:".\vscpdll.def" /out:"$(OUTDIR)\vscp.dll" /implib:"$(OUTDIR)\vscp.lib" /pdbtype:sept 
DEF_FILE= \
	".\vscpdll.def"
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\vscpdll.obj" \
	"$(INTDIR)\vscpdll.res"

"$(OUTDIR)\vscp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("vscpdll.dep")
!INCLUDE "vscpdll.dep"
!ELSE 
!MESSAGE Warning: cannot find "vscpdll.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "vscpdll - Win32 Release" || "$(CFG)" == "vscpdll - Win32 Debug"
SOURCE=..\..\canal\common\canalshmem_level2_win32.cpp

!IF  "$(CFG)" == "vscpdll - Win32 Release"


"$(INTDIR)\canalshmem_level2_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vscpdll - Win32 Debug"


"$(INTDIR)\canalshmem_level2_win32.obj"	"$(INTDIR)\canalshmem_level2_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "vscpdll - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_WINDLL" /D "_AFXDLL" /Fp"$(INTDIR)\vscpdll.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\vscpdll.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "vscpdll - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_WINDLL" /D "_AFXDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\vscpdll.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\vscpdll.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\vscpdll.cpp

!IF  "$(CFG)" == "vscpdll - Win32 Release"


"$(INTDIR)\vscpdll.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpdll - Win32 Debug"


"$(INTDIR)\vscpdll.obj"	"$(INTDIR)\vscpdll.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\vscpdll.rc

"$(INTDIR)\vscpdll.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

