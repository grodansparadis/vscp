# Microsoft Developer Studio Generated NMAKE File, Based on canald.dsp
!IF "$(CFG)" == ""
CFG=canald - Win32 Debug
!MESSAGE No configuration specified. Defaulting to canald - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "canald - Win32 Release DLL" && "$(CFG)" != "canald - Win32 Debug DLL" && "$(CFG)" != "canald - Win32 Release" && "$(CFG)" != "canald - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "canald.mak" CFG="canald - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "canald - Win32 Release DLL" (based on "Win32 (x86) Application")
!MESSAGE "canald - Win32 Debug DLL" (based on "Win32 (x86) Application")
!MESSAGE "canald - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "canald - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "canald - Win32 Release DLL"

OUTDIR=.\ReleaseDll
INTDIR=.\ReleaseDll
# Begin Custom Macros
OutDir=.\ReleaseDll
# End Custom Macros

ALL : "$(OUTDIR)\canald.exe"


CLEAN :
	-@erase "$(INTDIR)\canald.obj"
	-@erase "$(INTDIR)\clientlist.obj"
	-@erase "$(INTDIR)\clientthread_level1_win32.obj"
	-@erase "$(INTDIR)\clientthread_level2_win32.obj"
	-@erase "$(INTDIR)\controlobject.obj"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\daemon_VSCP.obj"
	-@erase "$(INTDIR)\devicelist.obj"
	-@erase "$(INTDIR)\devicethread_win32.obj"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dm.obj"
	-@erase "$(INTDIR)\listenthread_win32.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\tcplistenthread.obj"
	-@erase "$(INTDIR)\terminationthread.obj"
	-@erase "$(INTDIR)\udpreceivethread_win32.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vscp.obj"
	-@erase "$(OUTDIR)\canald.exe"
	-@erase "$(OUTDIR)\canald.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D _WIN32_WINNT=0x0400 /D "_MT" /D wxUSE_GUI=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\canald.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\canald.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib wsock32.lib netapi32.lib msvcrt.lib libc.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\canald.pdb" /machine:I386 /out:"$(OUTDIR)\canald.exe" 
LINK32_OBJS= \
	"$(INTDIR)\canald.obj" \
	"$(INTDIR)\clientlist.obj" \
	"$(INTDIR)\clientthread_level1_win32.obj" \
	"$(INTDIR)\clientthread_level2_win32.obj" \
	"$(INTDIR)\controlobject.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\daemon_VSCP.obj" \
	"$(INTDIR)\devicelist.obj" \
	"$(INTDIR)\devicethread_win32.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\dm.obj" \
	"$(INTDIR)\listenthread_win32.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\tcplistenthread.obj" \
	"$(INTDIR)\terminationthread.obj" \
	"$(INTDIR)\udpreceivethread_win32.obj" \
	"$(INTDIR)\vscp.obj"

"$(OUTDIR)\canald.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"

OUTDIR=.\DebugDll
INTDIR=.\DebugDll
# Begin Custom Macros
OutDir=.\DebugDll
# End Custom Macros

ALL : "$(OUTDIR)\canald.exe"


CLEAN :
	-@erase "$(INTDIR)\canald.obj"
	-@erase "$(INTDIR)\clientlist.obj"
	-@erase "$(INTDIR)\clientthread_level1_win32.obj"
	-@erase "$(INTDIR)\clientthread_level2_win32.obj"
	-@erase "$(INTDIR)\controlobject.obj"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\daemon_VSCP.obj"
	-@erase "$(INTDIR)\devicelist.obj"
	-@erase "$(INTDIR)\devicethread_win32.obj"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dm.obj"
	-@erase "$(INTDIR)\listenthread_win32.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\tcplistenthread.obj"
	-@erase "$(INTDIR)\terminationthread.obj"
	-@erase "$(INTDIR)\udpreceivethread_win32.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vscp.obj"
	-@erase "$(OUTDIR)\canald.exe"
	-@erase "$(OUTDIR)\canald.ilk"
	-@erase "$(OUTDIR)\canald.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x400 /D _WIN32_WINNT=0x0400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\canald.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\canald.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250d.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib wsock32.lib netapi32.lib msvcrtd.lib libcd.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\canald.pdb" /debug /machine:I386 /out:"$(OUTDIR)\canald.exe" /canald 
LINK32_OBJS= \
	"$(INTDIR)\canald.obj" \
	"$(INTDIR)\clientlist.obj" \
	"$(INTDIR)\clientthread_level1_win32.obj" \
	"$(INTDIR)\clientthread_level2_win32.obj" \
	"$(INTDIR)\controlobject.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\daemon_VSCP.obj" \
	"$(INTDIR)\devicelist.obj" \
	"$(INTDIR)\devicethread_win32.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\dm.obj" \
	"$(INTDIR)\listenthread_win32.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\tcplistenthread.obj" \
	"$(INTDIR)\terminationthread.obj" \
	"$(INTDIR)\udpreceivethread_win32.obj" \
	"$(INTDIR)\vscp.obj"

"$(OUTDIR)\canald.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "canald - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\canald.exe" "$(OUTDIR)\canald.bsc"


CLEAN :
	-@erase "$(INTDIR)\canald.obj"
	-@erase "$(INTDIR)\canald.sbr"
	-@erase "$(INTDIR)\clientlist.obj"
	-@erase "$(INTDIR)\clientlist.sbr"
	-@erase "$(INTDIR)\clientthread_level1_win32.obj"
	-@erase "$(INTDIR)\clientthread_level1_win32.sbr"
	-@erase "$(INTDIR)\clientthread_level2_win32.obj"
	-@erase "$(INTDIR)\clientthread_level2_win32.sbr"
	-@erase "$(INTDIR)\controlobject.obj"
	-@erase "$(INTDIR)\controlobject.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\daemon_VSCP.obj"
	-@erase "$(INTDIR)\daemon_VSCP.sbr"
	-@erase "$(INTDIR)\devicelist.obj"
	-@erase "$(INTDIR)\devicelist.sbr"
	-@erase "$(INTDIR)\devicethread_win32.obj"
	-@erase "$(INTDIR)\devicethread_win32.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\dm.obj"
	-@erase "$(INTDIR)\dm.sbr"
	-@erase "$(INTDIR)\listenthread_win32.obj"
	-@erase "$(INTDIR)\listenthread_win32.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\tcplistenthread.obj"
	-@erase "$(INTDIR)\tcplistenthread.sbr"
	-@erase "$(INTDIR)\terminationthread.obj"
	-@erase "$(INTDIR)\terminationthread.sbr"
	-@erase "$(INTDIR)\udpreceivethread_win32.obj"
	-@erase "$(INTDIR)\udpreceivethread_win32.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vscp.obj"
	-@erase "$(INTDIR)\vscp.sbr"
	-@erase "$(OUTDIR)\canald.bsc"
	-@erase "$(OUTDIR)\canald.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x0400 /D _WIN32_WINNT=0x0400 /D "_MT" /D wxUSE_GUI=1 /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\canald.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canald.sbr" \
	"$(INTDIR)\clientlist.sbr" \
	"$(INTDIR)\clientthread_level1_win32.sbr" \
	"$(INTDIR)\clientthread_level2_win32.sbr" \
	"$(INTDIR)\controlobject.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\daemon_VSCP.sbr" \
	"$(INTDIR)\devicelist.sbr" \
	"$(INTDIR)\devicethread_win32.sbr" \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\dm.sbr" \
	"$(INTDIR)\listenthread_win32.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\tcplistenthread.sbr" \
	"$(INTDIR)\terminationthread.sbr" \
	"$(INTDIR)\udpreceivethread_win32.sbr" \
	"$(INTDIR)\vscp.sbr"

"$(OUTDIR)\canald.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib wsock32.lib netapi32.lib msvcrt.lib libc.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\canald.pdb" /machine:I386 /out:"$(OUTDIR)\canald.exe" 
LINK32_OBJS= \
	"$(INTDIR)\canald.obj" \
	"$(INTDIR)\clientlist.obj" \
	"$(INTDIR)\clientthread_level1_win32.obj" \
	"$(INTDIR)\clientthread_level2_win32.obj" \
	"$(INTDIR)\controlobject.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\daemon_VSCP.obj" \
	"$(INTDIR)\devicelist.obj" \
	"$(INTDIR)\devicethread_win32.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\dm.obj" \
	"$(INTDIR)\listenthread_win32.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\tcplistenthread.obj" \
	"$(INTDIR)\terminationthread.obj" \
	"$(INTDIR)\udpreceivethread_win32.obj" \
	"$(INTDIR)\vscp.obj"

"$(OUTDIR)\canald.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\canald.exe" "$(OUTDIR)\canald.bsc"
   copy release\*.exe ..\..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "canald - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\canald.exe" "$(OUTDIR)\canald.bsc"


CLEAN :
	-@erase "$(INTDIR)\canald.obj"
	-@erase "$(INTDIR)\canald.sbr"
	-@erase "$(INTDIR)\clientlist.obj"
	-@erase "$(INTDIR)\clientlist.sbr"
	-@erase "$(INTDIR)\clientthread_level1_win32.obj"
	-@erase "$(INTDIR)\clientthread_level1_win32.sbr"
	-@erase "$(INTDIR)\clientthread_level2_win32.obj"
	-@erase "$(INTDIR)\clientthread_level2_win32.sbr"
	-@erase "$(INTDIR)\controlobject.obj"
	-@erase "$(INTDIR)\controlobject.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\daemon_VSCP.obj"
	-@erase "$(INTDIR)\daemon_VSCP.sbr"
	-@erase "$(INTDIR)\devicelist.obj"
	-@erase "$(INTDIR)\devicelist.sbr"
	-@erase "$(INTDIR)\devicethread_win32.obj"
	-@erase "$(INTDIR)\devicethread_win32.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\dm.obj"
	-@erase "$(INTDIR)\dm.sbr"
	-@erase "$(INTDIR)\listenthread_win32.obj"
	-@erase "$(INTDIR)\listenthread_win32.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\tcplistenthread.obj"
	-@erase "$(INTDIR)\tcplistenthread.sbr"
	-@erase "$(INTDIR)\terminationthread.obj"
	-@erase "$(INTDIR)\terminationthread.sbr"
	-@erase "$(INTDIR)\udpreceivethread_win32.obj"
	-@erase "$(INTDIR)\udpreceivethread_win32.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vscp.obj"
	-@erase "$(INTDIR)\vscp.sbr"
	-@erase "$(OUTDIR)\canald.bsc"
	-@erase "$(OUTDIR)\canald.exe"
	-@erase "$(OUTDIR)\canald.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D WINVER=0x0400 /D _WIN32_WINNT=0x0400 /D "_MT" /D wxUSE_GUI=0 /D "__WXDEBUG__" /D WXDEBUG=1 /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\canald.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canald.sbr" \
	"$(INTDIR)\clientlist.sbr" \
	"$(INTDIR)\clientthread_level1_win32.sbr" \
	"$(INTDIR)\clientthread_level2_win32.sbr" \
	"$(INTDIR)\controlobject.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\daemon_VSCP.sbr" \
	"$(INTDIR)\devicelist.sbr" \
	"$(INTDIR)\devicethread_win32.sbr" \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\dm.sbr" \
	"$(INTDIR)\listenthread_win32.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\tcplistenthread.sbr" \
	"$(INTDIR)\terminationthread.sbr" \
	"$(INTDIR)\udpreceivethread_win32.sbr" \
	"$(INTDIR)\vscp.sbr"

"$(OUTDIR)\canald.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmswd.lib comctl32.lib wsock32.lib netapi32.lib rpcrt4.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib MSVCRTD.lib libcd.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\canald.pdb" /debug /machine:I386 /nodefaultlib:"libcd.lib MSVCRTD.lib" /out:"$(OUTDIR)\canald.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\canald.obj" \
	"$(INTDIR)\clientlist.obj" \
	"$(INTDIR)\clientthread_level1_win32.obj" \
	"$(INTDIR)\clientthread_level2_win32.obj" \
	"$(INTDIR)\controlobject.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\daemon_VSCP.obj" \
	"$(INTDIR)\devicelist.obj" \
	"$(INTDIR)\devicethread_win32.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\dm.obj" \
	"$(INTDIR)\listenthread_win32.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\tcplistenthread.obj" \
	"$(INTDIR)\terminationthread.obj" \
	"$(INTDIR)\udpreceivethread_win32.obj" \
	"$(INTDIR)\vscp.obj"

"$(OUTDIR)\canald.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("canald.dep")
!INCLUDE "canald.dep"
!ELSE 
!MESSAGE Warning: cannot find "canald.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "canald - Win32 Release DLL" || "$(CFG)" == "canald - Win32 Debug DLL" || "$(CFG)" == "canald - Win32 Release" || "$(CFG)" == "canald - Win32 Debug"
SOURCE=.\canald.cpp

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\canald.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\canald.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\canald.obj"	"$(INTDIR)\canald.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\canald.obj"	"$(INTDIR)\canald.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\..\common\clientlist.cpp

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\clientlist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\clientlist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\clientlist.obj"	"$(INTDIR)\clientlist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\clientlist.obj"	"$(INTDIR)\clientlist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\clientthread_level1_win32.cpp

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\clientthread_level1_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\clientthread_level1_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\clientthread_level1_win32.obj"	"$(INTDIR)\clientthread_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\clientthread_level1_win32.obj"	"$(INTDIR)\clientthread_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\clientthread_level2_win32.cpp

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\clientthread_level2_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\clientthread_level2_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\clientthread_level2_win32.obj"	"$(INTDIR)\clientthread_level2_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\clientthread_level2_win32.obj"	"$(INTDIR)\clientthread_level2_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\controlobject.cpp

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\controlobject.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\controlobject.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\controlobject.obj"	"$(INTDIR)\controlobject.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\controlobject.obj"	"$(INTDIR)\controlobject.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\common\crc.c

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\crc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\crc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\crc.obj"	"$(INTDIR)\crc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\crc.obj"	"$(INTDIR)\crc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\daemon_VSCP.cpp

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\daemon_VSCP.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\daemon_VSCP.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\daemon_VSCP.obj"	"$(INTDIR)\daemon_VSCP.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\daemon_VSCP.obj"	"$(INTDIR)\daemon_VSCP.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\devicelist.cpp

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\devicelist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\devicelist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\devicelist.obj"	"$(INTDIR)\devicelist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\devicelist.obj"	"$(INTDIR)\devicelist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\devicethread_win32.cpp

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\devicethread_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\devicethread_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\devicethread_win32.obj"	"$(INTDIR)\devicethread_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\devicethread_win32.obj"	"$(INTDIR)\devicethread_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\common\dllist.c

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\dm.cpp

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\dm.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\dm.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\dm.obj"	"$(INTDIR)\dm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\dm.obj"	"$(INTDIR)\dm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\listenthread_win32.cpp

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\listenthread_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\listenthread_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\listenthread_win32.obj"	"$(INTDIR)\listenthread_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\listenthread_win32.obj"	"$(INTDIR)\listenthread_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\common\md5.cpp

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\md5.obj"	"$(INTDIR)\md5.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\md5.obj"	"$(INTDIR)\md5.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\tcplistenthread.cpp

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\tcplistenthread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\tcplistenthread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\tcplistenthread.obj"	"$(INTDIR)\tcplistenthread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\tcplistenthread.obj"	"$(INTDIR)\tcplistenthread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\terminationthread.cpp

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\terminationthread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\terminationthread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\terminationthread.obj"	"$(INTDIR)\terminationthread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\terminationthread.obj"	"$(INTDIR)\terminationthread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\udpreceivethread_win32.cpp

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\udpreceivethread_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\udpreceivethread_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\udpreceivethread_win32.obj"	"$(INTDIR)\udpreceivethread_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\udpreceivethread_win32.obj"	"$(INTDIR)\udpreceivethread_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\vscp\common\vscp.c

!IF  "$(CFG)" == "canald - Win32 Release DLL"


"$(INTDIR)\vscp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug DLL"


"$(INTDIR)\vscp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Release"


"$(INTDIR)\vscp.obj"	"$(INTDIR)\vscp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canald - Win32 Debug"


"$(INTDIR)\vscp.obj"	"$(INTDIR)\vscp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

