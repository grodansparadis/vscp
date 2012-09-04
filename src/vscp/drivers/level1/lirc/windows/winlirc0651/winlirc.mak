# Microsoft Developer Studio Generated NMAKE File, Based on winlirc.dsp
!IF "$(CFG)" == ""
CFG=winlirc - Win32 Debug
!MESSAGE No configuration specified. Defaulting to winlirc - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "winlirc - Win32 Release" && "$(CFG)" != "winlirc - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "winlirc.mak" CFG="winlirc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "winlirc - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "winlirc - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "winlirc - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\winlirc.exe" "$(OUTDIR)\winlirc.bsc"


CLEAN :
	-@erase "$(INTDIR)\confdlg.obj"
	-@erase "$(INTDIR)\confdlg.sbr"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\config.sbr"
	-@erase "$(INTDIR)\drvdlg.obj"
	-@erase "$(INTDIR)\drvdlg.sbr"
	-@erase "$(INTDIR)\dumpcfg.obj"
	-@erase "$(INTDIR)\dumpcfg.sbr"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\globals.sbr"
	-@erase "$(INTDIR)\irconfig.obj"
	-@erase "$(INTDIR)\irconfig.sbr"
	-@erase "$(INTDIR)\irdriver.obj"
	-@erase "$(INTDIR)\irdriver.sbr"
	-@erase "$(INTDIR)\learndlg.obj"
	-@erase "$(INTDIR)\learndlg.sbr"
	-@erase "$(INTDIR)\remote.obj"
	-@erase "$(INTDIR)\remote.sbr"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\server.sbr"
	-@erase "$(INTDIR)\Trayicon.obj"
	-@erase "$(INTDIR)\Trayicon.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\winlirc.obj"
	-@erase "$(INTDIR)\winlirc.res"
	-@erase "$(INTDIR)\winlirc.sbr"
	-@erase "$(OUTDIR)\winlirc.bsc"
	-@erase "$(OUTDIR)\winlirc.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\winlirc.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\winlirc.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\winlirc.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\confdlg.sbr" \
	"$(INTDIR)\config.sbr" \
	"$(INTDIR)\drvdlg.sbr" \
	"$(INTDIR)\dumpcfg.sbr" \
	"$(INTDIR)\globals.sbr" \
	"$(INTDIR)\irconfig.sbr" \
	"$(INTDIR)\irdriver.sbr" \
	"$(INTDIR)\learndlg.sbr" \
	"$(INTDIR)\remote.sbr" \
	"$(INTDIR)\server.sbr" \
	"$(INTDIR)\Trayicon.sbr" \
	"$(INTDIR)\winlirc.sbr"

"$(OUTDIR)\winlirc.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=ws2_32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\winlirc.pdb" /machine:I386 /out:"$(OUTDIR)\winlirc.exe" 
LINK32_OBJS= \
	"$(INTDIR)\confdlg.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\drvdlg.obj" \
	"$(INTDIR)\dumpcfg.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\irconfig.obj" \
	"$(INTDIR)\irdriver.obj" \
	"$(INTDIR)\learndlg.obj" \
	"$(INTDIR)\remote.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\Trayicon.obj" \
	"$(INTDIR)\winlirc.obj" \
	"$(INTDIR)\winlirc.res"

"$(OUTDIR)\winlirc.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "winlirc - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\winlirc.exe" "$(OUTDIR)\winlirc.bsc"


CLEAN :
	-@erase "$(INTDIR)\confdlg.obj"
	-@erase "$(INTDIR)\confdlg.sbr"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\config.sbr"
	-@erase "$(INTDIR)\drvdlg.obj"
	-@erase "$(INTDIR)\drvdlg.sbr"
	-@erase "$(INTDIR)\dumpcfg.obj"
	-@erase "$(INTDIR)\dumpcfg.sbr"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\globals.sbr"
	-@erase "$(INTDIR)\irconfig.obj"
	-@erase "$(INTDIR)\irconfig.sbr"
	-@erase "$(INTDIR)\irdriver.obj"
	-@erase "$(INTDIR)\irdriver.sbr"
	-@erase "$(INTDIR)\learndlg.obj"
	-@erase "$(INTDIR)\learndlg.sbr"
	-@erase "$(INTDIR)\remote.obj"
	-@erase "$(INTDIR)\remote.sbr"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\server.sbr"
	-@erase "$(INTDIR)\Trayicon.obj"
	-@erase "$(INTDIR)\Trayicon.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\winlirc.obj"
	-@erase "$(INTDIR)\winlirc.res"
	-@erase "$(INTDIR)\winlirc.sbr"
	-@erase "$(OUTDIR)\winlirc.bsc"
	-@erase "$(OUTDIR)\winlirc.exe"
	-@erase "$(OUTDIR)\winlirc.ilk"
	-@erase "$(OUTDIR)\winlirc.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\winlirc.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\winlirc.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\winlirc.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\confdlg.sbr" \
	"$(INTDIR)\config.sbr" \
	"$(INTDIR)\drvdlg.sbr" \
	"$(INTDIR)\dumpcfg.sbr" \
	"$(INTDIR)\globals.sbr" \
	"$(INTDIR)\irconfig.sbr" \
	"$(INTDIR)\irdriver.sbr" \
	"$(INTDIR)\learndlg.sbr" \
	"$(INTDIR)\remote.sbr" \
	"$(INTDIR)\server.sbr" \
	"$(INTDIR)\Trayicon.sbr" \
	"$(INTDIR)\winlirc.sbr"

"$(OUTDIR)\winlirc.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=ws2_32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\winlirc.pdb" /debug /machine:I386 /out:"$(OUTDIR)\winlirc.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\confdlg.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\drvdlg.obj" \
	"$(INTDIR)\dumpcfg.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\irconfig.obj" \
	"$(INTDIR)\irdriver.obj" \
	"$(INTDIR)\learndlg.obj" \
	"$(INTDIR)\remote.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\Trayicon.obj" \
	"$(INTDIR)\winlirc.obj" \
	"$(INTDIR)\winlirc.res"

"$(OUTDIR)\winlirc.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("winlirc.dep")
!INCLUDE "winlirc.dep"
!ELSE 
!MESSAGE Warning: cannot find "winlirc.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "winlirc - Win32 Release" || "$(CFG)" == "winlirc - Win32 Debug"
SOURCE=.\confdlg.cpp

"$(INTDIR)\confdlg.obj"	"$(INTDIR)\confdlg.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\config.cpp

"$(INTDIR)\config.obj"	"$(INTDIR)\config.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\drvdlg.cpp

"$(INTDIR)\drvdlg.obj"	"$(INTDIR)\drvdlg.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dumpcfg.cpp

"$(INTDIR)\dumpcfg.obj"	"$(INTDIR)\dumpcfg.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\globals.cpp

"$(INTDIR)\globals.obj"	"$(INTDIR)\globals.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\irconfig.cpp

"$(INTDIR)\irconfig.obj"	"$(INTDIR)\irconfig.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\irdriver.cpp

"$(INTDIR)\irdriver.obj"	"$(INTDIR)\irdriver.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\learndlg.cpp

"$(INTDIR)\learndlg.obj"	"$(INTDIR)\learndlg.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\remote.cpp

"$(INTDIR)\remote.obj"	"$(INTDIR)\remote.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\server.cpp

"$(INTDIR)\server.obj"	"$(INTDIR)\server.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Trayicon.cpp

"$(INTDIR)\Trayicon.obj"	"$(INTDIR)\Trayicon.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\winlirc.cpp

"$(INTDIR)\winlirc.obj"	"$(INTDIR)\winlirc.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\winlirc.rc

"$(INTDIR)\winlirc.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

