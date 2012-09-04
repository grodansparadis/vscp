# Microsoft Developer Studio Generated NMAKE File, Based on cancmd.dsp
!IF "$(CFG)" == ""
CFG=cancmd - Win32 Debug
!MESSAGE No configuration specified. Defaulting to cancmd - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "cancmd - Win32 Release Unicode DLL" && "$(CFG)" != "cancmd - Win32 Debug Unicode DLL" && "$(CFG)" != "cancmd - Win32 Debug Unicode" && "$(CFG)" != "cancmd - Win32 Release DLL" && "$(CFG)" != "cancmd - Win32 Debug DLL" && "$(CFG)" != "cancmd - Win32 Release" && "$(CFG)" != "cancmd - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cancmd.mak" CFG="cancmd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cancmd - Win32 Release Unicode DLL" (based on "Win32 (x86) Console Application")
!MESSAGE "cancmd - Win32 Debug Unicode DLL" (based on "Win32 (x86) Console Application")
!MESSAGE "cancmd - Win32 Debug Unicode" (based on "Win32 (x86) Console Application")
!MESSAGE "cancmd - Win32 Release DLL" (based on "Win32 (x86) Console Application")
!MESSAGE "cancmd - Win32 Debug DLL" (based on "Win32 (x86) Console Application")
!MESSAGE "cancmd - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "cancmd - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cancmd - Win32 Release Unicode DLL"

OUTDIR=.\BaseReleaseUnicodeDll
INTDIR=.\BaseReleaseUnicodeDll
# Begin Custom Macros
OutDir=.\BaseReleaseUnicodeDll
# End Custom Macros

ALL : "$(OUTDIR)\cancmd.exe"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\cancmd.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\cancmd.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "../../include" /I "..\..\lib\basedllu" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /D "WXUSINGDLL" /D "_UNICODE" /D "UNICODE" /Fp"$(INTDIR)\cancmd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cancmd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib advapi32.lib shell32.lib wsock32.lib ..\..\lib\wxbase233u.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\cancmd.pdb" /machine:I386 /out:"$(OUTDIR)\cancmd.exe" 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\cancmd.obj"

"$(OUTDIR)\cancmd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug Unicode DLL"

OUTDIR=.\BaseReleaseUnicode
INTDIR=.\BaseReleaseUnicode
# Begin Custom Macros
OutDir=.\BaseReleaseUnicode
# End Custom Macros

ALL : "$(OUTDIR)\cancmd.exe"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\cancmd.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\cancmd.exe"
	-@erase "$(OUTDIR)\cancmd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "..\..\lib\basedllud" /I "..\..\lib\baseu" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /D "__WXDEBUG__" /D WXDEBUG=1 /D "WXUSINGDLL" /D "_UNICODE" /D "UNICODE" /D "NDEBUG" /Fp"$(INTDIR)\cancmd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cancmd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib advapi32.lib shell32.lib wsock32.lib ..\..\lib\wxbase233ud.lib kernel32.lib user32.lib advapi32.lib shell32.lib wsock32.lib ..\..\lib\zlib.lib ..\..\lib\regex.lib ..\..\lib\wxbaseu.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\cancmd.pdb" /debug /machine:I386 /out:"$(OUTDIR)\cancmd.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\cancmd.obj"

"$(OUTDIR)\cancmd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug Unicode"

OUTDIR=.\BaseDebugUnicode
INTDIR=.\BaseDebugUnicode
# Begin Custom Macros
OutDir=.\BaseDebugUnicode
# End Custom Macros

ALL : "$(OUTDIR)\cancmd.exe" "$(OUTDIR)\cancmd.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.sbr"
	-@erase "$(INTDIR)\cancmd.obj"
	-@erase "$(INTDIR)\cancmd.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\cancmd.bsc"
	-@erase "$(OUTDIR)\cancmd.exe"
	-@erase "$(OUTDIR)\cancmd.ilk"
	-@erase "$(OUTDIR)\cancmd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /D "__WXDEBUG__" /D WXDEBUG=1 /D "_UNICODE" /D "UNICODE" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\cancmd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cancmd.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\canalshmem_level2_win32.sbr" \
	"$(INTDIR)\cancmd.sbr"

"$(OUTDIR)\cancmd.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib advapi32.lib shell32.lib wsock32.lib ..\..\lib\zlibd.lib ..\..\lib\regexd.lib ..\..\lib\wxbaseud.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\cancmd.pdb" /debug /machine:I386 /out:"$(OUTDIR)\cancmd.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\cancmd.obj"

"$(OUTDIR)\cancmd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cancmd - Win32 Release DLL"

OUTDIR=.\BaseReleaseDll
INTDIR=.\BaseReleaseDll
# Begin Custom Macros
OutDir=.\BaseReleaseDll
# End Custom Macros

ALL : "$(OUTDIR)\cancmd.exe"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\cancmd.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\cancmd.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\mswdll\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /D "WXUSINGDLL" /Fp"$(INTDIR)\cancmd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cancmd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\cancmd.pdb" /machine:I386 /out:"$(OUTDIR)\cancmd.exe" 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\cancmd.obj"

"$(OUTDIR)\cancmd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug DLL"

OUTDIR=.\BaseDebugDll
INTDIR=.\BaseDebugDll
# Begin Custom Macros
OutDir=.\BaseDebugDll
# End Custom Macros

ALL : "$(OUTDIR)\cancmd.exe"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\cancmd.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\cancmd.exe"
	-@erase "$(OUTDIR)\cancmd.ilk"
	-@erase "$(OUTDIR)\cancmd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswdlld\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /D "__WXDEBUG__" /D WXDEBUG=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\cancmd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cancmd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250d.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\cancmd.pdb" /debug /machine:I386 /out:"$(OUTDIR)\cancmd.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\cancmd.obj"

"$(OUTDIR)\cancmd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cancmd - Win32 Release"

OUTDIR=.\BaseRelease
INTDIR=.\BaseRelease
# Begin Custom Macros
OutDir=.\BaseRelease
# End Custom Macros

ALL : ".\Release\cancmd.exe" "$(OUTDIR)\cancmd.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.sbr"
	-@erase "$(INTDIR)\cancmd.obj"
	-@erase "$(INTDIR)\cancmd.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\cancmd.bsc"
	-@erase ".\Release\cancmd.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\cancmd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cancmd.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\canalshmem_level2_win32.sbr" \
	"$(INTDIR)\cancmd.sbr"

"$(OUTDIR)\cancmd.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\cancmd.pdb" /machine:I386 /out:"Release/cancmd.exe" 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\cancmd.obj"

".\Release\cancmd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\BaseRelease
# End Custom Macros

$(DS_POSTBUILD_DEP) : ".\Release\cancmd.exe" "$(OUTDIR)\cancmd.bsc"
   copy release\*.exe ..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug"

OUTDIR=.\BaseDebug
INTDIR=.\BaseDebug
# Begin Custom Macros
OutDir=.\BaseDebug
# End Custom Macros

ALL : ".\Debug\cancmd.exe" "$(OUTDIR)\cancmd.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.sbr"
	-@erase "$(INTDIR)\cancmd.obj"
	-@erase "$(INTDIR)\cancmd.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\cancmd.bsc"
	-@erase "$(OUTDIR)\cancmd.pdb"
	-@erase ".\Debug\cancmd.exe"
	-@erase ".\Debug\cancmd.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D _WIN32_WINNT=0x0400 /D "_MT" /D wxUSE_GUI=0 /D "__WXDEBUG__" /D WXDEBUG=1 /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cancmd.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\canalshmem_level2_win32.sbr" \
	"$(INTDIR)\cancmd.sbr"

"$(OUTDIR)\cancmd.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmswd.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\cancmd.pdb" /debug /machine:I386 /out:"Debug/cancmd.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\cancmd.obj"

".\Debug\cancmd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("cancmd.dep")
!INCLUDE "cancmd.dep"
!ELSE 
!MESSAGE Warning: cannot find "cancmd.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "cancmd - Win32 Release Unicode DLL" || "$(CFG)" == "cancmd - Win32 Debug Unicode DLL" || "$(CFG)" == "cancmd - Win32 Debug Unicode" || "$(CFG)" == "cancmd - Win32 Release DLL" || "$(CFG)" == "cancmd - Win32 Debug DLL" || "$(CFG)" == "cancmd - Win32 Release" || "$(CFG)" == "cancmd - Win32 Debug"
SOURCE=..\common\canalshmem_level1_win32.cpp

!IF  "$(CFG)" == "cancmd - Win32 Release Unicode DLL"


"$(INTDIR)\canalshmem_level1_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug Unicode DLL"


"$(INTDIR)\canalshmem_level1_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug Unicode"


"$(INTDIR)\canalshmem_level1_win32.obj"	"$(INTDIR)\canalshmem_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cancmd - Win32 Release DLL"


"$(INTDIR)\canalshmem_level1_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug DLL"


"$(INTDIR)\canalshmem_level1_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cancmd - Win32 Release"


"$(INTDIR)\canalshmem_level1_win32.obj"	"$(INTDIR)\canalshmem_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug"


"$(INTDIR)\canalshmem_level1_win32.obj"	"$(INTDIR)\canalshmem_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\canalshmem_level2_win32.cpp

!IF  "$(CFG)" == "cancmd - Win32 Release Unicode DLL"


"$(INTDIR)\canalshmem_level2_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug Unicode DLL"


"$(INTDIR)\canalshmem_level2_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug Unicode"


"$(INTDIR)\canalshmem_level2_win32.obj"	"$(INTDIR)\canalshmem_level2_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cancmd - Win32 Release DLL"


"$(INTDIR)\canalshmem_level2_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug DLL"


"$(INTDIR)\canalshmem_level2_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cancmd - Win32 Release"


"$(INTDIR)\canalshmem_level2_win32.obj"	"$(INTDIR)\canalshmem_level2_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug"


"$(INTDIR)\canalshmem_level2_win32.obj"	"$(INTDIR)\canalshmem_level2_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\cancmd.cpp

!IF  "$(CFG)" == "cancmd - Win32 Release Unicode DLL"


"$(INTDIR)\cancmd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug Unicode DLL"


"$(INTDIR)\cancmd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug Unicode"


"$(INTDIR)\cancmd.obj"	"$(INTDIR)\cancmd.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cancmd - Win32 Release DLL"


"$(INTDIR)\cancmd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug DLL"


"$(INTDIR)\cancmd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cancmd - Win32 Release"


"$(INTDIR)\cancmd.obj"	"$(INTDIR)\cancmd.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cancmd - Win32 Debug"


"$(INTDIR)\cancmd.obj"	"$(INTDIR)\cancmd.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

