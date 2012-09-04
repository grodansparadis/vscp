# Microsoft Developer Studio Generated NMAKE File, Based on picblw.dsp
!IF "$(CFG)" == ""
CFG=picblw - Win32 Debug
!MESSAGE No configuration specified. Defaulting to picblw - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "picblw - Win32 Release DLL" && "$(CFG)" != "picblw - Win32 Debug DLL" && "$(CFG)" != "picblw - Win32 Release" && "$(CFG)" != "picblw - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "picblw.mak" CFG="picblw - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "picblw - Win32 Release DLL" (based on "Win32 (x86) Application")
!MESSAGE "picblw - Win32 Debug DLL" (based on "Win32 (x86) Application")
!MESSAGE "picblw - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "picblw - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "picblw - Win32 Release DLL"

OUTDIR=.\ReleaseDll
INTDIR=.\ReleaseDll
# Begin Custom Macros
OutDir=.\ReleaseDll
# End Custom Macros

ALL : "$(OUTDIR)\picblw.exe"


CLEAN :
	-@erase "$(INTDIR)\picblw.obj"
	-@erase "$(INTDIR)\picblw.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\picblw.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W4 /O2 /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\mswdll" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\picblw.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\picblw.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\picblw.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib wxmsw24.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\picblw.pdb" /machine:I386 /out:"$(OUTDIR)\picblw.exe" 
LINK32_OBJS= \
	"$(INTDIR)\picblw.obj" \
	"$(INTDIR)\picblw.res"

"$(OUTDIR)\picblw.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "picblw - Win32 Debug DLL"

OUTDIR=.\DebugDll
INTDIR=.\DebugDll
# Begin Custom Macros
OutDir=.\DebugDll
# End Custom Macros

ALL : "$(OUTDIR)\picblw.exe"


CLEAN :
	-@erase "$(INTDIR)\picblw.obj"
	-@erase "$(INTDIR)\picblw.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\picblw.exe"
	-@erase "$(OUTDIR)\picblw.ilk"
	-@erase "$(OUTDIR)\picblw.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W4 /Zi /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\mswdlld" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\picblw.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\picblw.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\picblw.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib wxmsw24d.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\picblw.pdb" /debug /machine:I386 /out:"$(OUTDIR)\picblw.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\picblw.obj" \
	"$(INTDIR)\picblw.res"

"$(OUTDIR)\picblw.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "picblw - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\picblw.exe"


CLEAN :
	-@erase "$(INTDIR)\picblw.obj"
	-@erase "$(INTDIR)\picblw.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\picblw.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W4 /O2 /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\msw" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /Fp"$(INTDIR)\picblw.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\picblw.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\picblw.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\picblw.pdb" /machine:I386 /out:"$(OUTDIR)\picblw.exe" 
LINK32_OBJS= \
	"$(INTDIR)\picblw.obj" \
	"$(INTDIR)\picblw.res"

"$(OUTDIR)\picblw.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "picblw - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\picblw.exe"


CLEAN :
	-@erase "$(INTDIR)\picblw.obj"
	-@erase "$(INTDIR)\picblw.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\picblw.exe"
	-@erase "$(OUTDIR)\picblw.ilk"
	-@erase "$(OUTDIR)\picblw.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W4 /Zi /Od /I "d:\wxWindows-2.4.2\nclude" /I "d:\wxWindows-2.4.2\lib\mswd" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /Fp"$(INTDIR)\picblw.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\picblw.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\picblw.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib zlibd.lib regexd.lib pngd.lib jpegd.lib tiffd.lib wxmswd.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\picblw.pdb" /debug /machine:I386 /out:"$(OUTDIR)\picblw.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\picblw.obj" \
	"$(INTDIR)\picblw.res"

"$(OUTDIR)\picblw.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("picblw.dep")
!INCLUDE "picblw.dep"
!ELSE 
!MESSAGE Warning: cannot find "picblw.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "picblw - Win32 Release DLL" || "$(CFG)" == "picblw - Win32 Debug DLL" || "$(CFG)" == "picblw - Win32 Release" || "$(CFG)" == "picblw - Win32 Debug"
SOURCE=.\picblw.cpp

"$(INTDIR)\picblw.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\picblw.rc

"$(INTDIR)\picblw.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

