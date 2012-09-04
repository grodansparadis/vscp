# Microsoft Developer Studio Generated NMAKE File, Based on lirctest.dsp
!IF "$(CFG)" == ""
CFG=lirctest - Win32 Debug
!MESSAGE No configuration specified. Defaulting to lirctest - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "lirctest - Win32 Release DLL" && "$(CFG)" != "lirctest - Win32 Debug DLL" && "$(CFG)" != "lirctest - Win32 Release" && "$(CFG)" != "lirctest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "lirctest.mak" CFG="lirctest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "lirctest - Win32 Release DLL" (based on "Win32 (x86) Application")
!MESSAGE "lirctest - Win32 Debug DLL" (based on "Win32 (x86) Application")
!MESSAGE "lirctest - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "lirctest - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "lirctest - Win32 Release DLL"

OUTDIR=.\ReleaseDll
INTDIR=.\ReleaseDll
# Begin Custom Macros
OutDir=.\ReleaseDll
# End Custom Macros

ALL : "$(OUTDIR)\lirctest.exe"


CLEAN :
	-@erase "$(INTDIR)\lircobj.obj"
	-@erase "$(INTDIR)\lirctest.obj"
	-@erase "$(INTDIR)\lirctest.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\lirctest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\mswdll\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\lirctest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\lirctest.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\lirctest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib libexpat.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\lirctest.pdb" /machine:I386 /out:"$(OUTDIR)\lirctest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\lircobj.obj" \
	"$(INTDIR)\lirctest.obj" \
	"$(INTDIR)\lirctest.res"

"$(OUTDIR)\lirctest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "lirctest - Win32 Debug DLL"

OUTDIR=.\DebugDll
INTDIR=.\DebugDll
# Begin Custom Macros
OutDir=.\DebugDll
# End Custom Macros

ALL : "$(OUTDIR)\lirctest.exe"


CLEAN :
	-@erase "$(INTDIR)\lircobj.obj"
	-@erase "$(INTDIR)\lirctest.obj"
	-@erase "$(INTDIR)\lirctest.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\lirctest.exe"
	-@erase "$(OUTDIR)\lirctest.ilk"
	-@erase "$(OUTDIR)\lirctest.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswdlld\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\lirctest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\lirctest.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\lirctest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250d.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib libexpat.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\lirctest.pdb" /debug /machine:I386 /out:"$(OUTDIR)\lirctest.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\lircobj.obj" \
	"$(INTDIR)\lirctest.obj" \
	"$(INTDIR)\lirctest.res"

"$(OUTDIR)\lirctest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "lirctest - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\lirctest.exe"


CLEAN :
	-@erase "$(INTDIR)\lircobj.obj"
	-@erase "$(INTDIR)\lirctest.obj"
	-@erase "$(INTDIR)\lirctest.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\lirctest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /Fp"$(INTDIR)\lirctest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\lirctest.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\lirctest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib libexpat.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\lirctest.pdb" /machine:I386 /out:"$(OUTDIR)\lirctest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\lircobj.obj" \
	"$(INTDIR)\lirctest.obj" \
	"$(INTDIR)\lirctest.res"

"$(OUTDIR)\lirctest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "lirctest - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\lirctest.exe" "$(OUTDIR)\lirctest.bsc"


CLEAN :
	-@erase "$(INTDIR)\lircobj.obj"
	-@erase "$(INTDIR)\lircobj.sbr"
	-@erase "$(INTDIR)\lirctest.obj"
	-@erase "$(INTDIR)\lirctest.res"
	-@erase "$(INTDIR)\lirctest.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\lirctest.bsc"
	-@erase "$(OUTDIR)\lirctest.exe"
	-@erase "$(OUTDIR)\lirctest.ilk"
	-@erase "$(OUTDIR)\lirctest.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\lirctest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\lirctest.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\lirctest.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\lircobj.sbr" \
	"$(INTDIR)\lirctest.sbr"

"$(OUTDIR)\lirctest.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmswd.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib libexpat.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\lirctest.pdb" /debug /machine:I386 /out:"$(OUTDIR)\lirctest.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\lircobj.obj" \
	"$(INTDIR)\lirctest.obj" \
	"$(INTDIR)\lirctest.res"

"$(OUTDIR)\lirctest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("lirctest.dep")
!INCLUDE "lirctest.dep"
!ELSE 
!MESSAGE Warning: cannot find "lirctest.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "lirctest - Win32 Release DLL" || "$(CFG)" == "lirctest - Win32 Debug DLL" || "$(CFG)" == "lirctest - Win32 Release" || "$(CFG)" == "lirctest - Win32 Debug"
SOURCE=..\..\common\lircobj.cpp

!IF  "$(CFG)" == "lirctest - Win32 Release DLL"


"$(INTDIR)\lircobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "lirctest - Win32 Debug DLL"


"$(INTDIR)\lircobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "lirctest - Win32 Release"


"$(INTDIR)\lircobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "lirctest - Win32 Debug"


"$(INTDIR)\lircobj.obj"	"$(INTDIR)\lircobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\lirctest.cpp

!IF  "$(CFG)" == "lirctest - Win32 Release DLL"


"$(INTDIR)\lirctest.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "lirctest - Win32 Debug DLL"


"$(INTDIR)\lirctest.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "lirctest - Win32 Release"


"$(INTDIR)\lirctest.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "lirctest - Win32 Debug"


"$(INTDIR)\lirctest.obj"	"$(INTDIR)\lirctest.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lirctest.rc

"$(INTDIR)\lirctest.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

