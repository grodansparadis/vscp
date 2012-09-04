# Microsoft Developer Studio Generated NMAKE File, Based on cw.dsp
!IF "$(CFG)" == ""
CFG=cw - Win32 Debug
!MESSAGE No configuration specified. Defaulting to cw - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "cw - Win32 Release DLL" && "$(CFG)" != "cw - Win32 Debug DLL" && "$(CFG)" != "cw - Win32 Release" && "$(CFG)" != "cw - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cw.mak" CFG="cw - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cw - Win32 Release DLL" (based on "Win32 (x86) Application")
!MESSAGE "cw - Win32 Debug DLL" (based on "Win32 (x86) Application")
!MESSAGE "cw - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "cw - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "cw - Win32 Release DLL"

OUTDIR=.\ReleaseDll
INTDIR=.\ReleaseDll
# Begin Custom Macros
OutDir=.\ReleaseDll
# End Custom Macros

ALL : "$(OUTDIR)\cw.exe"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\canalsuperwrapper.obj"
	-@erase "$(INTDIR)\configfile.obj"
	-@erase "$(INTDIR)\cw.obj"
	-@erase "$(INTDIR)\cw.res"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\EditIfDlg.obj"
	-@erase "$(INTDIR)\MsgListWnd.obj"
	-@erase "$(INTDIR)\msglistwnd_level2.obj"
	-@erase "$(INTDIR)\SelectIfDlg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vscpconfigwnd.obj"
	-@erase "$(OUTDIR)\cw.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\mswdll\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\cw.pch" /YX"wx/wxprec.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\cw.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cw.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msvcrt.lib libc.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\cw.pdb" /machine:I386 /out:"$(OUTDIR)\cw.exe" 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\canalsuperwrapper.obj" \
	"$(INTDIR)\configfile.obj" \
	"$(INTDIR)\cw.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\EditIfDlg.obj" \
	"$(INTDIR)\MsgListWnd.obj" \
	"$(INTDIR)\msglistwnd_level2.obj" \
	"$(INTDIR)\SelectIfDlg.obj" \
	"$(INTDIR)\vscpconfigwnd.obj" \
	"$(INTDIR)\cw.res"

"$(OUTDIR)\cw.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cw - Win32 Debug DLL"

OUTDIR=.\DebugDll
INTDIR=.\DebugDll
# Begin Custom Macros
OutDir=.\DebugDll
# End Custom Macros

ALL : "$(OUTDIR)\cw.exe" "$(OUTDIR)\cw.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.sbr"
	-@erase "$(INTDIR)\canalsuperwrapper.obj"
	-@erase "$(INTDIR)\canalsuperwrapper.sbr"
	-@erase "$(INTDIR)\configfile.obj"
	-@erase "$(INTDIR)\configfile.sbr"
	-@erase "$(INTDIR)\cw.obj"
	-@erase "$(INTDIR)\cw.res"
	-@erase "$(INTDIR)\cw.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.sbr"
	-@erase "$(INTDIR)\EditIfDlg.obj"
	-@erase "$(INTDIR)\EditIfDlg.sbr"
	-@erase "$(INTDIR)\MsgListWnd.obj"
	-@erase "$(INTDIR)\MsgListWnd.sbr"
	-@erase "$(INTDIR)\msglistwnd_level2.obj"
	-@erase "$(INTDIR)\msglistwnd_level2.sbr"
	-@erase "$(INTDIR)\SelectIfDlg.obj"
	-@erase "$(INTDIR)\SelectIfDlg.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vscpconfigwnd.obj"
	-@erase "$(INTDIR)\vscpconfigwnd.sbr"
	-@erase "$(OUTDIR)\cw.bsc"
	-@erase "$(OUTDIR)\cw.exe"
	-@erase "$(OUTDIR)\cw.ilk"
	-@erase "$(OUTDIR)\cw.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswdlld\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /D "WXUSINGDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\cw.pch" /YX"wx/wxprec.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\cw.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cw.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\canalshmem_level2_win32.sbr" \
	"$(INTDIR)\canalsuperwrapper.sbr" \
	"$(INTDIR)\configfile.sbr" \
	"$(INTDIR)\cw.sbr" \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\DllWrapper.sbr" \
	"$(INTDIR)\EditIfDlg.sbr" \
	"$(INTDIR)\MsgListWnd.sbr" \
	"$(INTDIR)\msglistwnd_level2.sbr" \
	"$(INTDIR)\SelectIfDlg.sbr" \
	"$(INTDIR)\vscpconfigwnd.sbr"

"$(OUTDIR)\cw.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250d.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msvcrtd.lib libcd.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\cw.pdb" /debug /machine:I386 /out:"$(OUTDIR)\cw.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\canalsuperwrapper.obj" \
	"$(INTDIR)\configfile.obj" \
	"$(INTDIR)\cw.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\EditIfDlg.obj" \
	"$(INTDIR)\MsgListWnd.obj" \
	"$(INTDIR)\msglistwnd_level2.obj" \
	"$(INTDIR)\SelectIfDlg.obj" \
	"$(INTDIR)\vscpconfigwnd.obj" \
	"$(INTDIR)\cw.res"

"$(OUTDIR)\cw.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cw - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\cw.exe"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\canalsuperwrapper.obj"
	-@erase "$(INTDIR)\configfile.obj"
	-@erase "$(INTDIR)\cw.obj"
	-@erase "$(INTDIR)\cw.res"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\EditIfDlg.obj"
	-@erase "$(INTDIR)\MsgListWnd.obj"
	-@erase "$(INTDIR)\msglistwnd_level2.obj"
	-@erase "$(INTDIR)\SelectIfDlg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vscpconfigwnd.obj"
	-@erase "$(OUTDIR)\cw.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /Fp"$(INTDIR)\cw.pch" /YX"wx/wxprec.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\cw.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cw.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msvcrt.lib libc.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\cw.pdb" /machine:I386 /out:"$(OUTDIR)\cw.exe" 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\canalsuperwrapper.obj" \
	"$(INTDIR)\configfile.obj" \
	"$(INTDIR)\cw.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\EditIfDlg.obj" \
	"$(INTDIR)\MsgListWnd.obj" \
	"$(INTDIR)\msglistwnd_level2.obj" \
	"$(INTDIR)\SelectIfDlg.obj" \
	"$(INTDIR)\vscpconfigwnd.obj" \
	"$(INTDIR)\cw.res"

"$(OUTDIR)\cw.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\cw.exe"
   copy release\*.exe ..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "cw - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\cw.exe" "$(OUTDIR)\cw.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\canalshmem_level2_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level2_win32.sbr"
	-@erase "$(INTDIR)\canalsuperwrapper.obj"
	-@erase "$(INTDIR)\canalsuperwrapper.sbr"
	-@erase "$(INTDIR)\configfile.obj"
	-@erase "$(INTDIR)\configfile.sbr"
	-@erase "$(INTDIR)\cw.obj"
	-@erase "$(INTDIR)\cw.res"
	-@erase "$(INTDIR)\cw.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.sbr"
	-@erase "$(INTDIR)\EditIfDlg.obj"
	-@erase "$(INTDIR)\EditIfDlg.sbr"
	-@erase "$(INTDIR)\MsgListWnd.obj"
	-@erase "$(INTDIR)\MsgListWnd.sbr"
	-@erase "$(INTDIR)\msglistwnd_level2.obj"
	-@erase "$(INTDIR)\msglistwnd_level2.sbr"
	-@erase "$(INTDIR)\SelectIfDlg.obj"
	-@erase "$(INTDIR)\SelectIfDlg.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vscpconfigwnd.obj"
	-@erase "$(INTDIR)\vscpconfigwnd.sbr"
	-@erase "$(OUTDIR)\cw.bsc"
	-@erase "$(OUTDIR)\cw.exe"
	-@erase "$(OUTDIR)\cw.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\cw.pch" /YX"wx/wxprec.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\cw.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cw.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\canalshmem_level2_win32.sbr" \
	"$(INTDIR)\canalsuperwrapper.sbr" \
	"$(INTDIR)\configfile.sbr" \
	"$(INTDIR)\cw.sbr" \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\DllWrapper.sbr" \
	"$(INTDIR)\EditIfDlg.sbr" \
	"$(INTDIR)\MsgListWnd.sbr" \
	"$(INTDIR)\msglistwnd_level2.sbr" \
	"$(INTDIR)\SelectIfDlg.sbr" \
	"$(INTDIR)\vscpconfigwnd.sbr"

"$(OUTDIR)\cw.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmswd.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib msvcrtd.lib libcd.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\cw.pdb" /debug /machine:I386 /out:"$(OUTDIR)\cw.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\canalshmem_level2_win32.obj" \
	"$(INTDIR)\canalsuperwrapper.obj" \
	"$(INTDIR)\configfile.obj" \
	"$(INTDIR)\cw.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\EditIfDlg.obj" \
	"$(INTDIR)\MsgListWnd.obj" \
	"$(INTDIR)\msglistwnd_level2.obj" \
	"$(INTDIR)\SelectIfDlg.obj" \
	"$(INTDIR)\vscpconfigwnd.obj" \
	"$(INTDIR)\cw.res"

"$(OUTDIR)\cw.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("cw.dep")
!INCLUDE "cw.dep"
!ELSE 
!MESSAGE Warning: cannot find "cw.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "cw - Win32 Release DLL" || "$(CFG)" == "cw - Win32 Debug DLL" || "$(CFG)" == "cw - Win32 Release" || "$(CFG)" == "cw - Win32 Debug"
SOURCE=.\cw.rc

"$(INTDIR)\cw.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=..\common\canalshmem_level1_win32.cpp

!IF  "$(CFG)" == "cw - Win32 Release DLL"


"$(INTDIR)\canalshmem_level1_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Debug DLL"


"$(INTDIR)\canalshmem_level1_win32.obj"	"$(INTDIR)\canalshmem_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Release"


"$(INTDIR)\canalshmem_level1_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Debug"


"$(INTDIR)\canalshmem_level1_win32.obj"	"$(INTDIR)\canalshmem_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\canalshmem_level2_win32.cpp

!IF  "$(CFG)" == "cw - Win32 Release DLL"


"$(INTDIR)\canalshmem_level2_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Debug DLL"


"$(INTDIR)\canalshmem_level2_win32.obj"	"$(INTDIR)\canalshmem_level2_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Release"


"$(INTDIR)\canalshmem_level2_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Debug"


"$(INTDIR)\canalshmem_level2_win32.obj"	"$(INTDIR)\canalshmem_level2_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\canalsuperwrapper.cpp

!IF  "$(CFG)" == "cw - Win32 Release DLL"


"$(INTDIR)\canalsuperwrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Debug DLL"


"$(INTDIR)\canalsuperwrapper.obj"	"$(INTDIR)\canalsuperwrapper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Release"


"$(INTDIR)\canalsuperwrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Debug"


"$(INTDIR)\canalsuperwrapper.obj"	"$(INTDIR)\canalsuperwrapper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\configfile.cpp

!IF  "$(CFG)" == "cw - Win32 Release DLL"


"$(INTDIR)\configfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Debug DLL"


"$(INTDIR)\configfile.obj"	"$(INTDIR)\configfile.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Release"


"$(INTDIR)\configfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Debug"


"$(INTDIR)\configfile.obj"	"$(INTDIR)\configfile.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\cw.cpp

!IF  "$(CFG)" == "cw - Win32 Release DLL"


"$(INTDIR)\cw.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Debug DLL"


"$(INTDIR)\cw.obj"	"$(INTDIR)\cw.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Release"


"$(INTDIR)\cw.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Debug"


"$(INTDIR)\cw.obj"	"$(INTDIR)\cw.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\..\common\dllist.c

!IF  "$(CFG)" == "cw - Win32 Release DLL"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Debug DLL"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Release"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Debug"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\DllWrapper.cpp

!IF  "$(CFG)" == "cw - Win32 Release DLL"


"$(INTDIR)\DllWrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Debug DLL"


"$(INTDIR)\DllWrapper.obj"	"$(INTDIR)\DllWrapper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Release"


"$(INTDIR)\DllWrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cw - Win32 Debug"


"$(INTDIR)\DllWrapper.obj"	"$(INTDIR)\DllWrapper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\EditIfDlg.cpp

!IF  "$(CFG)" == "cw - Win32 Release DLL"


"$(INTDIR)\EditIfDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Debug DLL"


"$(INTDIR)\EditIfDlg.obj"	"$(INTDIR)\EditIfDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Release"


"$(INTDIR)\EditIfDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Debug"


"$(INTDIR)\EditIfDlg.obj"	"$(INTDIR)\EditIfDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\MsgListWnd.cpp

!IF  "$(CFG)" == "cw - Win32 Release DLL"


"$(INTDIR)\MsgListWnd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Debug DLL"


"$(INTDIR)\MsgListWnd.obj"	"$(INTDIR)\MsgListWnd.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Release"


"$(INTDIR)\MsgListWnd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Debug"


"$(INTDIR)\MsgListWnd.obj"	"$(INTDIR)\MsgListWnd.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\msglistwnd_level2.cpp

!IF  "$(CFG)" == "cw - Win32 Release DLL"


"$(INTDIR)\msglistwnd_level2.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Debug DLL"


"$(INTDIR)\msglistwnd_level2.obj"	"$(INTDIR)\msglistwnd_level2.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Release"


"$(INTDIR)\msglistwnd_level2.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Debug"


"$(INTDIR)\msglistwnd_level2.obj"	"$(INTDIR)\msglistwnd_level2.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\SelectIfDlg.cpp

!IF  "$(CFG)" == "cw - Win32 Release DLL"


"$(INTDIR)\SelectIfDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Debug DLL"


"$(INTDIR)\SelectIfDlg.obj"	"$(INTDIR)\SelectIfDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Release"


"$(INTDIR)\SelectIfDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Debug"


"$(INTDIR)\SelectIfDlg.obj"	"$(INTDIR)\SelectIfDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\vscpconfigwnd.cpp

!IF  "$(CFG)" == "cw - Win32 Release DLL"


"$(INTDIR)\vscpconfigwnd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Debug DLL"


"$(INTDIR)\vscpconfigwnd.obj"	"$(INTDIR)\vscpconfigwnd.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Release"


"$(INTDIR)\vscpconfigwnd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "cw - Win32 Debug"


"$(INTDIR)\vscpconfigwnd.obj"	"$(INTDIR)\vscpconfigwnd.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

