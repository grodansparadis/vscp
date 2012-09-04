# Microsoft Developer Studio Generated NMAKE File, Based on syslogtest.dsp
!IF "$(CFG)" == ""
CFG=syslogtest - Win32 Debug
!MESSAGE No configuration specified. Defaulting to syslogtest - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "syslogtest - Win32 Release DLL" && "$(CFG)" != "syslogtest - Win32 Debug DLL" && "$(CFG)" != "syslogtest - Win32 Release" && "$(CFG)" != "syslogtest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "syslogtest.mak" CFG="syslogtest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "syslogtest - Win32 Release DLL" (based on "Win32 (x86) Application")
!MESSAGE "syslogtest - Win32 Debug DLL" (based on "Win32 (x86) Application")
!MESSAGE "syslogtest - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "syslogtest - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "syslogtest - Win32 Release DLL"

OUTDIR=.\ReleaseDll
INTDIR=.\ReleaseDll
# Begin Custom Macros
OutDir=.\ReleaseDll
# End Custom Macros

ALL : "$(OUTDIR)\syslogtest.exe"


CLEAN :
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\syslogobj.obj"
	-@erase "$(INTDIR)\syslogtest.obj"
	-@erase "$(INTDIR)\syslogtest.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\syslogtest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\mswdll\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\syslogtest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\syslogtest.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\syslogtest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib libexpat.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\syslogtest.pdb" /machine:I386 /out:"$(OUTDIR)\syslogtest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\syslogobj.obj" \
	"$(INTDIR)\syslogtest.obj" \
	"$(INTDIR)\syslogtest.res"

"$(OUTDIR)\syslogtest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "syslogtest - Win32 Debug DLL"

OUTDIR=.\DebugDll
INTDIR=.\DebugDll
# Begin Custom Macros
OutDir=.\DebugDll
# End Custom Macros

ALL : "$(OUTDIR)\syslogtest.exe"


CLEAN :
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\syslogobj.obj"
	-@erase "$(INTDIR)\syslogtest.obj"
	-@erase "$(INTDIR)\syslogtest.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\syslogtest.exe"
	-@erase "$(OUTDIR)\syslogtest.ilk"
	-@erase "$(OUTDIR)\syslogtest.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswdlld\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\syslogtest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\syslogtest.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\syslogtest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250d.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib libexpat.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\syslogtest.pdb" /debug /machine:I386 /out:"$(OUTDIR)\syslogtest.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\syslogobj.obj" \
	"$(INTDIR)\syslogtest.obj" \
	"$(INTDIR)\syslogtest.res"

"$(OUTDIR)\syslogtest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "syslogtest - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\syslogtest.exe"


CLEAN :
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\syslogobj.obj"
	-@erase "$(INTDIR)\syslogtest.obj"
	-@erase "$(INTDIR)\syslogtest.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\syslogtest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /Fp"$(INTDIR)\syslogtest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\syslogtest.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\syslogtest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib libexpat.lib libexpat.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\syslogtest.pdb" /machine:I386 /out:"$(OUTDIR)\syslogtest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\syslogobj.obj" \
	"$(INTDIR)\syslogtest.obj" \
	"$(INTDIR)\syslogtest.res"

"$(OUTDIR)\syslogtest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "syslogtest - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\syslogtest.exe" "$(OUTDIR)\syslogtest.bsc"


CLEAN :
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\syslogobj.obj"
	-@erase "$(INTDIR)\syslogobj.sbr"
	-@erase "$(INTDIR)\syslogtest.obj"
	-@erase "$(INTDIR)\syslogtest.res"
	-@erase "$(INTDIR)\syslogtest.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\syslogtest.bsc"
	-@erase "$(OUTDIR)\syslogtest.exe"
	-@erase "$(OUTDIR)\syslogtest.ilk"
	-@erase "$(OUTDIR)\syslogtest.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\syslogtest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\syslogtest.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\syslogtest.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\syslogobj.sbr" \
	"$(INTDIR)\syslogtest.sbr"

"$(OUTDIR)\syslogtest.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmswd.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib libexpat.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\syslogtest.pdb" /debug /machine:I386 /out:"$(OUTDIR)\syslogtest.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\syslogobj.obj" \
	"$(INTDIR)\syslogtest.obj" \
	"$(INTDIR)\syslogtest.res"

"$(OUTDIR)\syslogtest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("syslogtest.dep")
!INCLUDE "syslogtest.dep"
!ELSE 
!MESSAGE Warning: cannot find "syslogtest.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "syslogtest - Win32 Release DLL" || "$(CFG)" == "syslogtest - Win32 Debug DLL" || "$(CFG)" == "syslogtest - Win32 Release" || "$(CFG)" == "syslogtest - Win32 Debug"
SOURCE=..\..\..\..\..\common\dllist.c

!IF  "$(CFG)" == "syslogtest - Win32 Release DLL"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "syslogtest - Win32 Debug DLL"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "syslogtest - Win32 Release"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "syslogtest - Win32 Debug"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\syslogobj.cpp

!IF  "$(CFG)" == "syslogtest - Win32 Release DLL"


"$(INTDIR)\syslogobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "syslogtest - Win32 Debug DLL"


"$(INTDIR)\syslogobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "syslogtest - Win32 Release"


"$(INTDIR)\syslogobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "syslogtest - Win32 Debug"


"$(INTDIR)\syslogobj.obj"	"$(INTDIR)\syslogobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\syslogtest.cpp

!IF  "$(CFG)" == "syslogtest - Win32 Release DLL"


"$(INTDIR)\syslogtest.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "syslogtest - Win32 Debug DLL"


"$(INTDIR)\syslogtest.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "syslogtest - Win32 Release"


"$(INTDIR)\syslogtest.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "syslogtest - Win32 Debug"


"$(INTDIR)\syslogtest.obj"	"$(INTDIR)\syslogtest.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\syslogtest.rc

"$(INTDIR)\syslogtest.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

