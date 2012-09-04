# Microsoft Developer Studio Generated NMAKE File, Based on pd.dsp
!IF "$(CFG)" == ""
CFG=pd - Win32 Debug
!MESSAGE No configuration specified. Defaulting to pd - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "pd - Win32 Release DLL" && "$(CFG)" != "pd - Win32 Debug DLL" && "$(CFG)" != "pd - Win32 Release" && "$(CFG)" != "pd - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pd.mak" CFG="pd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pd - Win32 Release DLL" (based on "Win32 (x86) Application")
!MESSAGE "pd - Win32 Debug DLL" (based on "Win32 (x86) Application")
!MESSAGE "pd - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "pd - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "pd - Win32 Release DLL"

OUTDIR=.\ReleaseDll
INTDIR=.\ReleaseDll
# Begin Custom Macros
OutDir=.\ReleaseDll
# End Custom Macros

ALL : "$(OUTDIR)\pd.exe"


CLEAN :
	-@erase "$(INTDIR)\automtn.obj"
	-@erase "$(INTDIR)\LrfMemoObject.obj"
	-@erase "$(INTDIR)\OptionDialog.obj"
	-@erase "$(INTDIR)\pd.obj"
	-@erase "$(INTDIR)\pd.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\pd.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W4 /O2 /I "../../include" /I "..\..\lib\mswdll" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\pd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\pd.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\pd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\lib\wxmsw24.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib libexpat.dll /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\pd.pdb" /machine:I386 /out:"$(OUTDIR)\pd.exe" 
LINK32_OBJS= \
	"$(INTDIR)\automtn.obj" \
	"$(INTDIR)\LrfMemoObject.obj" \
	"$(INTDIR)\OptionDialog.obj" \
	"$(INTDIR)\pd.obj" \
	"$(INTDIR)\pd.res"

"$(OUTDIR)\pd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "pd - Win32 Debug DLL"

OUTDIR=.\DebugDll
INTDIR=.\DebugDll
# Begin Custom Macros
OutDir=.\DebugDll
# End Custom Macros

ALL : "$(OUTDIR)\pd.exe"


CLEAN :
	-@erase "$(INTDIR)\automtn.obj"
	-@erase "$(INTDIR)\LrfMemoObject.obj"
	-@erase "$(INTDIR)\OptionDialog.obj"
	-@erase "$(INTDIR)\pd.obj"
	-@erase "$(INTDIR)\pd.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\pd.exe"
	-@erase "$(OUTDIR)\pd.ilk"
	-@erase "$(OUTDIR)\pd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W4 /Zi /Od /I "../../include" /I "..\..\lib\mswdlld" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\pd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\pd.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\pd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\lib\wxmsw24d.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib libexpat.dll /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\pd.pdb" /debug /machine:I386 /out:"$(OUTDIR)\pd.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\automtn.obj" \
	"$(INTDIR)\LrfMemoObject.obj" \
	"$(INTDIR)\OptionDialog.obj" \
	"$(INTDIR)\pd.obj" \
	"$(INTDIR)\pd.res"

"$(OUTDIR)\pd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "pd - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\pd.exe"


CLEAN :
	-@erase "$(INTDIR)\automtn.obj"
	-@erase "$(INTDIR)\LrfMemoObject.obj"
	-@erase "$(INTDIR)\OptionDialog.obj"
	-@erase "$(INTDIR)\pd.obj"
	-@erase "$(INTDIR)\pd.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\pd.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W4 /O2 /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\msw" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /Fp"$(INTDIR)\pd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\pd.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\pd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib libexpat.lib net.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\pd.pdb" /machine:I386 /out:"$(OUTDIR)\pd.exe" 
LINK32_OBJS= \
	"$(INTDIR)\automtn.obj" \
	"$(INTDIR)\LrfMemoObject.obj" \
	"$(INTDIR)\OptionDialog.obj" \
	"$(INTDIR)\pd.obj" \
	"$(INTDIR)\pd.res"

"$(OUTDIR)\pd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "pd - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\pd.exe" "$(OUTDIR)\pd.bsc"


CLEAN :
	-@erase "$(INTDIR)\automtn.obj"
	-@erase "$(INTDIR)\automtn.sbr"
	-@erase "$(INTDIR)\LrfMemoObject.obj"
	-@erase "$(INTDIR)\LrfMemoObject.sbr"
	-@erase "$(INTDIR)\OptionDialog.obj"
	-@erase "$(INTDIR)\OptionDialog.sbr"
	-@erase "$(INTDIR)\pd.obj"
	-@erase "$(INTDIR)\pd.res"
	-@erase "$(INTDIR)\pd.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\pd.bsc"
	-@erase "$(OUTDIR)\pd.exe"
	-@erase "$(OUTDIR)\pd.ilk"
	-@erase "$(OUTDIR)\pd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W4 /Zi /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\mswd" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\pd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\pd.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\pd.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\automtn.sbr" \
	"$(INTDIR)\LrfMemoObject.sbr" \
	"$(INTDIR)\OptionDialog.sbr" \
	"$(INTDIR)\pd.sbr"

"$(OUTDIR)\pd.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=zlibd.lib regexd.lib pngd.lib jpegd.lib tiffd.lib wxmswd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib libexpat.lib netd.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\pd.pdb" /debug /machine:I386 /out:"$(OUTDIR)\pd.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\automtn.obj" \
	"$(INTDIR)\LrfMemoObject.obj" \
	"$(INTDIR)\OptionDialog.obj" \
	"$(INTDIR)\pd.obj" \
	"$(INTDIR)\pd.res"

"$(OUTDIR)\pd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("pd.dep")
!INCLUDE "pd.dep"
!ELSE 
!MESSAGE Warning: cannot find "pd.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "pd - Win32 Release DLL" || "$(CFG)" == "pd - Win32 Debug DLL" || "$(CFG)" == "pd - Win32 Release" || "$(CFG)" == "pd - Win32 Debug"
SOURCE="..\..\..\wxWindows-2.4.2\src\msw\ole\automtn.cpp"

!IF  "$(CFG)" == "pd - Win32 Release DLL"


"$(INTDIR)\automtn.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pd - Win32 Debug DLL"


"$(INTDIR)\automtn.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pd - Win32 Release"


"$(INTDIR)\automtn.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pd - Win32 Debug"


"$(INTDIR)\automtn.obj"	"$(INTDIR)\automtn.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\LrfMemoObject.cpp

!IF  "$(CFG)" == "pd - Win32 Release DLL"


"$(INTDIR)\LrfMemoObject.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "pd - Win32 Debug DLL"


"$(INTDIR)\LrfMemoObject.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "pd - Win32 Release"


"$(INTDIR)\LrfMemoObject.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "pd - Win32 Debug"


"$(INTDIR)\LrfMemoObject.obj"	"$(INTDIR)\LrfMemoObject.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\OptionDialog.cpp

!IF  "$(CFG)" == "pd - Win32 Release DLL"


"$(INTDIR)\OptionDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "pd - Win32 Debug DLL"


"$(INTDIR)\OptionDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "pd - Win32 Release"


"$(INTDIR)\OptionDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "pd - Win32 Debug"


"$(INTDIR)\OptionDialog.obj"	"$(INTDIR)\OptionDialog.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\pd.cpp

!IF  "$(CFG)" == "pd - Win32 Release DLL"


"$(INTDIR)\pd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "pd - Win32 Debug DLL"


"$(INTDIR)\pd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "pd - Win32 Release"


"$(INTDIR)\pd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "pd - Win32 Debug"


"$(INTDIR)\pd.obj"	"$(INTDIR)\pd.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\pd.rc

"$(INTDIR)\pd.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

