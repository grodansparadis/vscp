# Microsoft Developer Studio Generated NMAKE File, Based on mkpasswd.dsp
!IF "$(CFG)" == ""
CFG=mkpasswd - Win32 Debug
!MESSAGE No configuration specified. Defaulting to mkpasswd - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "mkpasswd - Win32 Release Unicode DLL" && "$(CFG)" != "mkpasswd - Win32 Debug Unicode DLL" && "$(CFG)" != "mkpasswd - Win32 Debug Unicode" && "$(CFG)" != "mkpasswd - Win32 Release DLL" && "$(CFG)" != "mkpasswd - Win32 Debug DLL" && "$(CFG)" != "mkpasswd - Win32 Release" && "$(CFG)" != "mkpasswd - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mkpasswd.mak" CFG="mkpasswd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mkpasswd - Win32 Release Unicode DLL" (based on "Win32 (x86) Console Application")
!MESSAGE "mkpasswd - Win32 Debug Unicode DLL" (based on "Win32 (x86) Console Application")
!MESSAGE "mkpasswd - Win32 Debug Unicode" (based on "Win32 (x86) Console Application")
!MESSAGE "mkpasswd - Win32 Release DLL" (based on "Win32 (x86) Console Application")
!MESSAGE "mkpasswd - Win32 Debug DLL" (based on "Win32 (x86) Console Application")
!MESSAGE "mkpasswd - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "mkpasswd - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "mkpasswd - Win32 Release Unicode DLL"

OUTDIR=.\BaseReleaseUnicodeDll
INTDIR=.\BaseReleaseUnicodeDll
# Begin Custom Macros
OutDir=.\BaseReleaseUnicodeDll
# End Custom Macros

ALL : "$(OUTDIR)\mkpasswd.exe"


CLEAN :
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\mkpasswd.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mkpasswd.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "../../include" /I "..\..\lib\basedllu" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /D "WXUSINGDLL" /D "_UNICODE" /D "UNICODE" /Fp"$(INTDIR)\mkpasswd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mkpasswd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib advapi32.lib shell32.lib wsock32.lib ..\..\lib\wxbase233u.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\mkpasswd.pdb" /machine:I386 /out:"$(OUTDIR)\mkpasswd.exe" 
LINK32_OBJS= \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\mkpasswd.obj"

"$(OUTDIR)\mkpasswd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Debug Unicode DLL"

OUTDIR=.\BaseReleaseUnicode
INTDIR=.\BaseReleaseUnicode
# Begin Custom Macros
OutDir=.\BaseReleaseUnicode
# End Custom Macros

ALL : "$(OUTDIR)\mkpasswd.exe"


CLEAN :
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\mkpasswd.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\mkpasswd.exe"
	-@erase "$(OUTDIR)\mkpasswd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "..\..\lib\basedllud" /I "..\..\lib\baseu" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /D "__WXDEBUG__" /D WXDEBUG=1 /D "WXUSINGDLL" /D "_UNICODE" /D "UNICODE" /D "NDEBUG" /Fp"$(INTDIR)\mkpasswd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mkpasswd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib advapi32.lib shell32.lib wsock32.lib ..\..\lib\wxbase233ud.lib kernel32.lib user32.lib advapi32.lib shell32.lib wsock32.lib ..\..\lib\zlib.lib ..\..\lib\regex.lib ..\..\lib\wxbaseu.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\mkpasswd.pdb" /debug /machine:I386 /out:"$(OUTDIR)\mkpasswd.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\mkpasswd.obj"

"$(OUTDIR)\mkpasswd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Debug Unicode"

OUTDIR=.\BaseDebugUnicode
INTDIR=.\BaseDebugUnicode
# Begin Custom Macros
OutDir=.\BaseDebugUnicode
# End Custom Macros

ALL : "$(OUTDIR)\mkpasswd.exe" "$(OUTDIR)\mkpasswd.bsc"


CLEAN :
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\mkpasswd.obj"
	-@erase "$(INTDIR)\mkpasswd.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\mkpasswd.bsc"
	-@erase "$(OUTDIR)\mkpasswd.exe"
	-@erase "$(OUTDIR)\mkpasswd.ilk"
	-@erase "$(OUTDIR)\mkpasswd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=0 /D "__WXDEBUG__" /D WXDEBUG=1 /D "_UNICODE" /D "UNICODE" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\mkpasswd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mkpasswd.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\mkpasswd.sbr"

"$(OUTDIR)\mkpasswd.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib advapi32.lib shell32.lib wsock32.lib ..\..\lib\zlibd.lib ..\..\lib\regexd.lib ..\..\lib\wxbaseud.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\mkpasswd.pdb" /debug /machine:I386 /out:"$(OUTDIR)\mkpasswd.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\mkpasswd.obj"

"$(OUTDIR)\mkpasswd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Release DLL"

OUTDIR=.\BaseReleaseDll
INTDIR=.\BaseReleaseDll
# Begin Custom Macros
OutDir=.\BaseReleaseDll
# End Custom Macros

ALL : "$(OUTDIR)\mkpasswd.exe"


CLEAN :
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\mkpasswd.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mkpasswd.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\mswdll\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D _WIN32_WINNT=0x0400 /D "_MT" /D wxUSE_GUI=0 /D "WXUSINGDLL" /Fp"$(INTDIR)\mkpasswd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mkpasswd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\mkpasswd.pdb" /machine:I386 /out:"$(OUTDIR)\mkpasswd.exe" 
LINK32_OBJS= \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\mkpasswd.obj"

"$(OUTDIR)\mkpasswd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Debug DLL"

OUTDIR=.\BaseDebugDll
INTDIR=.\BaseDebugDll
# Begin Custom Macros
OutDir=.\BaseDebugDll
# End Custom Macros

ALL : "$(OUTDIR)\mkpasswd.exe"


CLEAN :
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\mkpasswd.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\mkpasswd.exe"
	-@erase "$(OUTDIR)\mkpasswd.ilk"
	-@erase "$(OUTDIR)\mkpasswd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswdlld\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D _WIN32_WINNT=0x0400 /D "_MT" /D wxUSE_GUI=0 /D "__WXDEBUG__" /D WXDEBUG=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\mkpasswd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mkpasswd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250d.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\mkpasswd.pdb" /debug /machine:I386 /out:"$(OUTDIR)\mkpasswd.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\mkpasswd.obj"

"$(OUTDIR)\mkpasswd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Release"

OUTDIR=.\BaseRelease
INTDIR=.\BaseRelease
# Begin Custom Macros
OutDir=.\BaseRelease
# End Custom Macros

ALL : ".\Release\mkpasswd.exe" "$(OUTDIR)\mkpasswd.bsc"


CLEAN :
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\mkpasswd.obj"
	-@erase "$(INTDIR)\mkpasswd.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mkpasswd.bsc"
	-@erase ".\Release\mkpasswd.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D _WIN32_WINNT=0x0400 /D "_MT" /D wxUSE_GUI=0 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\mkpasswd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mkpasswd.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\mkpasswd.sbr"

"$(OUTDIR)\mkpasswd.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\mkpasswd.pdb" /machine:I386 /out:"Release/mkpasswd.exe" 
LINK32_OBJS= \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\mkpasswd.obj"

".\Release\mkpasswd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\BaseRelease
# End Custom Macros

$(DS_POSTBUILD_DEP) : ".\Release\mkpasswd.exe" "$(OUTDIR)\mkpasswd.bsc"
   copy release\*.exe ..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Debug"

OUTDIR=.\BaseDebug
INTDIR=.\BaseDebug
# Begin Custom Macros
OutDir=.\BaseDebug
# End Custom Macros

ALL : ".\Debug\mkpasswd.exe" "$(OUTDIR)\mkpasswd.bsc"


CLEAN :
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\mkpasswd.obj"
	-@erase "$(INTDIR)\mkpasswd.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\mkpasswd.bsc"
	-@erase "$(OUTDIR)\mkpasswd.pdb"
	-@erase ".\Debug\mkpasswd.exe"
	-@erase ".\Debug\mkpasswd.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D _WIN32_WINNT=0x0400 /D "_MT" /D wxUSE_GUI=0 /D "__WXDEBUG__" /D WXDEBUG=1 /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mkpasswd.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\mkpasswd.sbr"

"$(OUTDIR)\mkpasswd.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmswd.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\mkpasswd.pdb" /debug /machine:I386 /out:"Debug/mkpasswd.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\mkpasswd.obj"

".\Debug\mkpasswd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("mkpasswd.dep")
!INCLUDE "mkpasswd.dep"
!ELSE 
!MESSAGE Warning: cannot find "mkpasswd.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "mkpasswd - Win32 Release Unicode DLL" || "$(CFG)" == "mkpasswd - Win32 Debug Unicode DLL" || "$(CFG)" == "mkpasswd - Win32 Debug Unicode" || "$(CFG)" == "mkpasswd - Win32 Release DLL" || "$(CFG)" == "mkpasswd - Win32 Debug DLL" || "$(CFG)" == "mkpasswd - Win32 Release" || "$(CFG)" == "mkpasswd - Win32 Debug"
SOURCE=..\..\common\md5.cpp

!IF  "$(CFG)" == "mkpasswd - Win32 Release Unicode DLL"


"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Debug Unicode DLL"


"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Debug Unicode"


"$(INTDIR)\md5.obj"	"$(INTDIR)\md5.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Release DLL"


"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Debug DLL"


"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Release"


"$(INTDIR)\md5.obj"	"$(INTDIR)\md5.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Debug"


"$(INTDIR)\md5.obj"	"$(INTDIR)\md5.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\mkpasswd.cpp

!IF  "$(CFG)" == "mkpasswd - Win32 Release Unicode DLL"


"$(INTDIR)\mkpasswd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Debug Unicode DLL"


"$(INTDIR)\mkpasswd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Debug Unicode"


"$(INTDIR)\mkpasswd.obj"	"$(INTDIR)\mkpasswd.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Release DLL"


"$(INTDIR)\mkpasswd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Debug DLL"


"$(INTDIR)\mkpasswd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Release"


"$(INTDIR)\mkpasswd.obj"	"$(INTDIR)\mkpasswd.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "mkpasswd - Win32 Debug"


"$(INTDIR)\mkpasswd.obj"	"$(INTDIR)\mkpasswd.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

