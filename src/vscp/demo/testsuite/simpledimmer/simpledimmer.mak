# Microsoft Developer Studio Generated NMAKE File, Based on simpledimmer.dsp
!IF "$(CFG)" == ""
CFG=simpledimmer - Win32 Debug
!MESSAGE No configuration specified. Defaulting to simpledimmer - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "simpledimmer - Win32 Release DLL" && "$(CFG)" != "simpledimmer - Win32 Debug DLL" && "$(CFG)" != "simpledimmer - Win32 Release" && "$(CFG)" != "simpledimmer - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "simpledimmer.mak" CFG="simpledimmer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "simpledimmer - Win32 Release DLL" (based on "Win32 (x86) Application")
!MESSAGE "simpledimmer - Win32 Debug DLL" (based on "Win32 (x86) Application")
!MESSAGE "simpledimmer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "simpledimmer - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "simpledimmer - Win32 Release DLL"

OUTDIR=.\ReleaseDll
INTDIR=.\ReleaseDll
# Begin Custom Macros
OutDir=.\ReleaseDll
# End Custom Macros

ALL : "$(OUTDIR)\simpledimmer.exe" "$(OUTDIR)\simpledimmer.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\OptionDialog.obj"
	-@erase "$(INTDIR)\OptionDialog.sbr"
	-@erase "$(INTDIR)\simpledimmer.obj"
	-@erase "$(INTDIR)\simpledimmer.res"
	-@erase "$(INTDIR)\simpledimmer.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vscp.obj"
	-@erase "$(INTDIR)\vscp.sbr"
	-@erase "$(OUTDIR)\simpledimmer.bsc"
	-@erase "$(OUTDIR)\simpledimmer.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\mswdll\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "WXUSINGDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\simpledimmer.pch" /YX"wx/wxprec.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\simpledimmer.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simpledimmer.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level2_win32.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\OptionDialog.sbr" \
	"$(INTDIR)\simpledimmer.sbr" \
	"$(INTDIR)\vscp.sbr"

"$(OUTDIR)\simpledimmer.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msvcrt.lib libc.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\simpledimmer.pdb" /machine:I386 /out:"$(OUTDIR)\simpledimmer.exe" 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\OptionDialog.obj" \
	"$(INTDIR)\simpledimmer.obj" \
	"$(INTDIR)\vscp.obj" \
	"$(INTDIR)\simpledimmer.res"

"$(OUTDIR)\simpledimmer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Debug DLL"

OUTDIR=.\DebugDll
INTDIR=.\DebugDll
# Begin Custom Macros
OutDir=.\DebugDll
# End Custom Macros

ALL : "$(OUTDIR)\simpledimmer.exe"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\OptionDialog.obj"
	-@erase "$(INTDIR)\simpledimmer.obj"
	-@erase "$(INTDIR)\simpledimmer.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vscp.obj"
	-@erase "$(OUTDIR)\simpledimmer.exe"
	-@erase "$(OUTDIR)\simpledimmer.ilk"
	-@erase "$(OUTDIR)\simpledimmer.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswdll\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\simpledimmer.pch" /YX"wx/wxprec.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\simpledimmer.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simpledimmer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250d.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msvcrtd.lib libcd.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\simpledimmer.pdb" /debug /machine:I386 /out:"$(OUTDIR)\simpledimmer.exe" /simpledimmer 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\OptionDialog.obj" \
	"$(INTDIR)\simpledimmer.obj" \
	"$(INTDIR)\vscp.obj" \
	"$(INTDIR)\simpledimmer.res"

"$(OUTDIR)\simpledimmer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\simpledimmer.exe" "$(OUTDIR)\simpledimmer.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\OptionDialog.obj"
	-@erase "$(INTDIR)\OptionDialog.sbr"
	-@erase "$(INTDIR)\simpledimmer.obj"
	-@erase "$(INTDIR)\simpledimmer.res"
	-@erase "$(INTDIR)\simpledimmer.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vscp.obj"
	-@erase "$(INTDIR)\vscp.sbr"
	-@erase "$(OUTDIR)\simpledimmer.bsc"
	-@erase "$(OUTDIR)\simpledimmer.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\simpledimmer.pch" /YX"wx/wxprec.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\simpledimmer.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simpledimmer.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level2_win32.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\OptionDialog.sbr" \
	"$(INTDIR)\simpledimmer.sbr" \
	"$(INTDIR)\vscp.sbr"

"$(OUTDIR)\simpledimmer.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msvcrt.lib libc.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\simpledimmer.pdb" /machine:I386 /out:"$(OUTDIR)\simpledimmer.exe" 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\OptionDialog.obj" \
	"$(INTDIR)\simpledimmer.obj" \
	"$(INTDIR)\vscp.obj" \
	"$(INTDIR)\simpledimmer.res"

"$(OUTDIR)\simpledimmer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\simpledimmer.exe" "$(OUTDIR)\simpledimmer.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\OptionDialog.obj"
	-@erase "$(INTDIR)\OptionDialog.sbr"
	-@erase "$(INTDIR)\simpledimmer.obj"
	-@erase "$(INTDIR)\simpledimmer.res"
	-@erase "$(INTDIR)\simpledimmer.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vscp.obj"
	-@erase "$(INTDIR)\vscp.sbr"
	-@erase "$(OUTDIR)\simpledimmer.bsc"
	-@erase "$(OUTDIR)\simpledimmer.exe"
	-@erase "$(OUTDIR)\simpledimmer.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\simpledimmer.pch" /YX"wx/wxprec.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\simpledimmer.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simpledimmer.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level2_win32.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\OptionDialog.sbr" \
	"$(INTDIR)\simpledimmer.sbr" \
	"$(INTDIR)\vscp.sbr"

"$(OUTDIR)\simpledimmer.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmswd.lib comctl32.lib rpcrt4.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msvcrtd.lib libcd.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\simpledimmer.pdb" /debug /machine:I386 /nodefaultlib:"libcd.lib msvcrtd.lib" /out:"$(OUTDIR)\simpledimmer.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\OptionDialog.obj" \
	"$(INTDIR)\simpledimmer.obj" \
	"$(INTDIR)\vscp.obj" \
	"$(INTDIR)\simpledimmer.res"

"$(OUTDIR)\simpledimmer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("simpledimmer.dep")
!INCLUDE "simpledimmer.dep"
!ELSE 
!MESSAGE Warning: cannot find "simpledimmer.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "simpledimmer - Win32 Release DLL" || "$(CFG)" == "simpledimmer - Win32 Debug DLL" || "$(CFG)" == "simpledimmer - Win32 Release" || "$(CFG)" == "simpledimmer - Win32 Debug"
SOURCE=..\..\..\canal\common\canalshmem_level2_win32.cpp

!IF  "$(CFG)" == "simpledimmer - Win32 Release DLL"


"$(INTDIR)\canalshmem_level2_win32.obj"	"$(INTDIR)\canalshmem_level2_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Debug DLL"


"$(INTDIR)\canalshmem_level2_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Release"


"$(INTDIR)\canalshmem_level2_win32.obj"	"$(INTDIR)\canalshmem_level2_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Debug"


"$(INTDIR)\canalshmem_level2_win32.obj"	"$(INTDIR)\canalshmem_level2_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\common\crc.c

!IF  "$(CFG)" == "simpledimmer - Win32 Release DLL"


"$(INTDIR)\crc.obj"	"$(INTDIR)\crc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Debug DLL"


"$(INTDIR)\crc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Release"


"$(INTDIR)\crc.obj"	"$(INTDIR)\crc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Debug"


"$(INTDIR)\crc.obj"	"$(INTDIR)\crc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\OptionDialog.cpp

!IF  "$(CFG)" == "simpledimmer - Win32 Release DLL"


"$(INTDIR)\OptionDialog.obj"	"$(INTDIR)\OptionDialog.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Debug DLL"


"$(INTDIR)\OptionDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Release"


"$(INTDIR)\OptionDialog.obj"	"$(INTDIR)\OptionDialog.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Debug"


"$(INTDIR)\OptionDialog.obj"	"$(INTDIR)\OptionDialog.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\simpledimmer.cpp

!IF  "$(CFG)" == "simpledimmer - Win32 Release DLL"


"$(INTDIR)\simpledimmer.obj"	"$(INTDIR)\simpledimmer.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Debug DLL"


"$(INTDIR)\simpledimmer.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Release"


"$(INTDIR)\simpledimmer.obj"	"$(INTDIR)\simpledimmer.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Debug"


"$(INTDIR)\simpledimmer.obj"	"$(INTDIR)\simpledimmer.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\simpledimmer.rc

"$(INTDIR)\simpledimmer.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=..\..\common\vscp.c

!IF  "$(CFG)" == "simpledimmer - Win32 Release DLL"


"$(INTDIR)\vscp.obj"	"$(INTDIR)\vscp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Debug DLL"


"$(INTDIR)\vscp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Release"


"$(INTDIR)\vscp.obj"	"$(INTDIR)\vscp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "simpledimmer - Win32 Debug"


"$(INTDIR)\vscp.obj"	"$(INTDIR)\vscp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

