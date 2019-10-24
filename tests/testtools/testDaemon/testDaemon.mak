# Microsoft Developer Studio Generated NMAKE File, Based on testDaemon.dsp
!IF "$(CFG)" == ""
CFG=testDaemon - Win32 Debug
!MESSAGE No configuration specified. Defaulting to testDaemon - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "testDaemon - Win32 Release" && "$(CFG)" != "testDaemon - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "testDaemon.mak" CFG="testDaemon - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "testDaemon - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "testDaemon - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "testDaemon - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\testDaemon.exe" "$(OUTDIR)\testDaemon.pch" "$(OUTDIR)\testDaemon.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\testDaemon.obj"
	-@erase "$(INTDIR)\testDaemon.pch"
	-@erase "$(INTDIR)\testDaemon.res"
	-@erase "$(INTDIR)\testDaemon.sbr"
	-@erase "$(INTDIR)\testDaemonDlg.obj"
	-@erase "$(INTDIR)\testDaemonDlg.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vscp.obj"
	-@erase "$(INTDIR)\vscp.sbr"
	-@erase "$(OUTDIR)\testDaemon.bsc"
	-@erase "$(OUTDIR)\testDaemon.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\testDaemon.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testDaemon.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level2_win32.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\testDaemon.sbr" \
	"$(INTDIR)\testDaemonDlg.sbr" \
	"$(INTDIR)\vscp.sbr"

"$(OUTDIR)\testDaemon.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\testDaemon.pdb" /machine:I386 /out:"$(OUTDIR)\testDaemon.exe" 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\testDaemon.obj" \
	"$(INTDIR)\testDaemonDlg.obj" \
	"$(INTDIR)\vscp.obj" \
	"$(INTDIR)\testDaemon.res"

"$(OUTDIR)\testDaemon.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\testDaemon.exe" "$(OUTDIR)\testDaemon.pch" "$(OUTDIR)\testDaemon.bsc"
   copy release\*.exe ..\deliveryx
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "testDaemon - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\testDaemon.exe" "$(OUTDIR)\testDaemon.pch" "$(OUTDIR)\testDaemon.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\testDaemon.obj"
	-@erase "$(INTDIR)\testDaemon.pch"
	-@erase "$(INTDIR)\testDaemon.res"
	-@erase "$(INTDIR)\testDaemon.sbr"
	-@erase "$(INTDIR)\testDaemonDlg.obj"
	-@erase "$(INTDIR)\testDaemonDlg.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vscp.obj"
	-@erase "$(INTDIR)\vscp.sbr"
	-@erase "$(OUTDIR)\testDaemon.bsc"
	-@erase "$(OUTDIR)\testDaemon.exe"
	-@erase "$(OUTDIR)\testDaemon.ilk"
	-@erase "$(OUTDIR)\testDaemon.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\testDaemon.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testDaemon.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level2_win32.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\testDaemon.sbr" \
	"$(INTDIR)\testDaemonDlg.sbr" \
	"$(INTDIR)\vscp.sbr"

"$(OUTDIR)\testDaemon.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmswd.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib wsock32.lib netapi32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\testDaemon.pdb" /debug /machine:I386 /out:"$(OUTDIR)\testDaemon.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\testDaemon.obj" \
	"$(INTDIR)\testDaemonDlg.obj" \
	"$(INTDIR)\vscp.obj" \
	"$(INTDIR)\testDaemon.res"

"$(OUTDIR)\testDaemon.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("testDaemon.dep")
!INCLUDE "testDaemon.dep"
!ELSE 
!MESSAGE Warning: cannot find "testDaemon.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "testDaemon - Win32 Release" || "$(CFG)" == "testDaemon - Win32 Debug"
SOURCE=..\..\canal\common\canalshmem_level2_win32.cpp

"$(INTDIR)\canalshmem_level2_win32.obj"	"$(INTDIR)\canalshmem_level2_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\crc.c

"$(INTDIR)\crc.obj"	"$(INTDIR)\crc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "testDaemon - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\testDaemon.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\testDaemon.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "testDaemon - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\testDaemon.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\testDaemon.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\testDaemon.cpp

"$(INTDIR)\testDaemon.obj"	"$(INTDIR)\testDaemon.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\testDaemon.rc

"$(INTDIR)\testDaemon.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\testDaemonDlg.cpp

"$(INTDIR)\testDaemonDlg.obj"	"$(INTDIR)\testDaemonDlg.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\common\vscp.c

"$(INTDIR)\vscp.obj"	"$(INTDIR)\vscp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

