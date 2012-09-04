# Microsoft Developer Studio Generated NMAKE File, Based on canalservice.dsp
!IF "$(CFG)" == ""
CFG=canalservice - Win32 Debug
!MESSAGE No configuration specified. Defaulting to canalservice - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "canalservice - Win32 Release" && "$(CFG)" != "canalservice - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "canalservice.mak" CFG="canalservice - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "canalservice - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "canalservice - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "canalservice - Win32 Release"

OUTDIR=.\release
INTDIR=.\release
# Begin Custom Macros
OutDir=.\release
# End Custom Macros

ALL : "$(OUTDIR)\canalservice.exe" "$(OUTDIR)\canalservice.bsc"


CLEAN :
	-@erase "$(INTDIR)\CanalServApp.obj"
	-@erase "$(INTDIR)\CanalServApp.sbr"
	-@erase "$(INTDIR)\canalservice.obj"
	-@erase "$(INTDIR)\canalservice.sbr"
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
	-@erase "$(INTDIR)\listenthread_win32.obj"
	-@erase "$(INTDIR)\listenthread_win32.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\NTService.obj"
	-@erase "$(INTDIR)\NTService.sbr"
	-@erase "$(INTDIR)\ntservmsg.res"
	-@erase "$(INTDIR)\tcplistenthread.obj"
	-@erase "$(INTDIR)\tcplistenthread.sbr"
	-@erase "$(INTDIR)\terminationthread.obj"
	-@erase "$(INTDIR)\terminationthread.sbr"
	-@erase "$(INTDIR)\udpreceivethread_win32.obj"
	-@erase "$(INTDIR)\udpreceivethread_win32.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vscp.obj"
	-@erase "$(INTDIR)\vscp.sbr"
	-@erase "$(OUTDIR)\canalservice.bsc"
	-@erase "$(OUTDIR)\canalservice.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "_CONSOLE" /D "_WIN32" /D _WIN32_WINNT=0x0500 /D "WIN32" /D "_MBCS" /D "_AFXDLL" /D "BUILD_CANALD_SERVICE" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\ntservmsg.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\canalservice.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\CanalServApp.sbr" \
	"$(INTDIR)\canalservice.sbr" \
	"$(INTDIR)\clientlist.sbr" \
	"$(INTDIR)\clientthread_level1_win32.sbr" \
	"$(INTDIR)\clientthread_level2_win32.sbr" \
	"$(INTDIR)\controlobject.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\devicelist.sbr" \
	"$(INTDIR)\devicethread_win32.sbr" \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\listenthread_win32.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\NTService.sbr" \
	"$(INTDIR)\tcplistenthread.sbr" \
	"$(INTDIR)\terminationthread.sbr" \
	"$(INTDIR)\udpreceivethread_win32.sbr" \
	"$(INTDIR)\vscp.sbr" \
	"$(INTDIR)\daemon_VSCP.sbr"

"$(OUTDIR)\canalservice.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\canalservice.pdb" /machine:I386 /out:"$(OUTDIR)\canalservice.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CanalServApp.obj" \
	"$(INTDIR)\canalservice.obj" \
	"$(INTDIR)\clientlist.obj" \
	"$(INTDIR)\clientthread_level1_win32.obj" \
	"$(INTDIR)\clientthread_level2_win32.obj" \
	"$(INTDIR)\controlobject.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\devicelist.obj" \
	"$(INTDIR)\devicethread_win32.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\listenthread_win32.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\NTService.obj" \
	"$(INTDIR)\tcplistenthread.obj" \
	"$(INTDIR)\terminationthread.obj" \
	"$(INTDIR)\udpreceivethread_win32.obj" \
	"$(INTDIR)\vscp.obj" \
	"$(INTDIR)\ntservmsg.res" \
	"$(INTDIR)\daemon_VSCP.obj"

"$(OUTDIR)\canalservice.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\canalservice.exe" "$(OUTDIR)\canalservice.bsc"
   copy release\*.exe ..\delivery
	copy release\*.exe D:\WINNT\system32
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "canalservice - Win32 Debug"

OUTDIR=.\debug
INTDIR=.\debug
# Begin Custom Macros
OutDir=.\debug
# End Custom Macros

ALL : "$(OUTDIR)\canalservice.exe" "$(OUTDIR)\canalservice.bsc"


CLEAN :
	-@erase "$(INTDIR)\CanalServApp.obj"
	-@erase "$(INTDIR)\CanalServApp.sbr"
	-@erase "$(INTDIR)\canalservice.obj"
	-@erase "$(INTDIR)\canalservice.sbr"
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
	-@erase "$(INTDIR)\listenthread_win32.obj"
	-@erase "$(INTDIR)\listenthread_win32.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\NTService.obj"
	-@erase "$(INTDIR)\NTService.sbr"
	-@erase "$(INTDIR)\ntservmsg.res"
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
	-@erase "$(OUTDIR)\canalservice.bsc"
	-@erase "$(OUTDIR)\canalservice.exe"
	-@erase "$(OUTDIR)\canalservice.ilk"
	-@erase "$(OUTDIR)\canalservice.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp1 /MDd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "_WIN32" /D _WIN32_WINNT=0x0500 /D "_MBCS" /D "WIN32" /D "_DEBUG" /D "CANAL" /D "WINDOWS" /D "_AFXDLL" /D "BUILD_CANALD_SERVICE" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\ntservmsg.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\canalservice.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\CanalServApp.sbr" \
	"$(INTDIR)\canalservice.sbr" \
	"$(INTDIR)\clientlist.sbr" \
	"$(INTDIR)\clientthread_level1_win32.sbr" \
	"$(INTDIR)\clientthread_level2_win32.sbr" \
	"$(INTDIR)\controlobject.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\devicelist.sbr" \
	"$(INTDIR)\devicethread_win32.sbr" \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\listenthread_win32.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\NTService.sbr" \
	"$(INTDIR)\tcplistenthread.sbr" \
	"$(INTDIR)\terminationthread.sbr" \
	"$(INTDIR)\udpreceivethread_win32.sbr" \
	"$(INTDIR)\vscp.sbr" \
	"$(INTDIR)\daemon_VSCP.sbr"

"$(OUTDIR)\canalservice.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmswd.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\canalservice.pdb" /debug /machine:I386 /out:"$(OUTDIR)\canalservice.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\CanalServApp.obj" \
	"$(INTDIR)\canalservice.obj" \
	"$(INTDIR)\clientlist.obj" \
	"$(INTDIR)\clientthread_level1_win32.obj" \
	"$(INTDIR)\clientthread_level2_win32.obj" \
	"$(INTDIR)\controlobject.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\devicelist.obj" \
	"$(INTDIR)\devicethread_win32.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\listenthread_win32.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\NTService.obj" \
	"$(INTDIR)\tcplistenthread.obj" \
	"$(INTDIR)\terminationthread.obj" \
	"$(INTDIR)\udpreceivethread_win32.obj" \
	"$(INTDIR)\vscp.obj" \
	"$(INTDIR)\ntservmsg.res" \
	"$(INTDIR)\daemon_VSCP.obj"

"$(OUTDIR)\canalservice.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("canalservice.dep")
!INCLUDE "canalservice.dep"
!ELSE 
!MESSAGE Warning: cannot find "canalservice.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "canalservice - Win32 Release" || "$(CFG)" == "canalservice - Win32 Debug"
SOURCE=.\CanalServApp.cpp

"$(INTDIR)\CanalServApp.obj"	"$(INTDIR)\CanalServApp.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\canalservice.cpp

"$(INTDIR)\canalservice.obj"	"$(INTDIR)\canalservice.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\common\clientlist.cpp

"$(INTDIR)\clientlist.obj"	"$(INTDIR)\clientlist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\clientthread_level1_win32.cpp

"$(INTDIR)\clientthread_level1_win32.obj"	"$(INTDIR)\clientthread_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\clientthread_level2_win32.cpp

"$(INTDIR)\clientthread_level2_win32.obj"	"$(INTDIR)\clientthread_level2_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\controlobject.cpp

"$(INTDIR)\controlobject.obj"	"$(INTDIR)\controlobject.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\crc.c

"$(INTDIR)\crc.obj"	"$(INTDIR)\crc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\daemon_VSCP.cpp

"$(INTDIR)\daemon_VSCP.obj"	"$(INTDIR)\daemon_VSCP.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\devicelist.cpp

"$(INTDIR)\devicelist.obj"	"$(INTDIR)\devicelist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\devicethread_win32.cpp

"$(INTDIR)\devicethread_win32.obj"	"$(INTDIR)\devicethread_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\dllist.c

"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\listenthread_win32.cpp

"$(INTDIR)\listenthread_win32.obj"	"$(INTDIR)\listenthread_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\md5.cpp

"$(INTDIR)\md5.obj"	"$(INTDIR)\md5.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\NTService.cpp

"$(INTDIR)\NTService.obj"	"$(INTDIR)\NTService.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\tcplistenthread.cpp

"$(INTDIR)\tcplistenthread.obj"	"$(INTDIR)\tcplistenthread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\terminationthread.cpp

"$(INTDIR)\terminationthread.obj"	"$(INTDIR)\terminationthread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\udpreceivethread_win32.cpp

"$(INTDIR)\udpreceivethread_win32.obj"	"$(INTDIR)\udpreceivethread_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\vscp\common\vscp.c

"$(INTDIR)\vscp.obj"	"$(INTDIR)\vscp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\CanalServApp.rc
SOURCE=..\..\common\ntservmsg.rc

!IF  "$(CFG)" == "canalservice - Win32 Release"


"$(INTDIR)\ntservmsg.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\ntservmsg.res" /i "\dev\can\can\src\common" /d "NDEBUG" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "canalservice - Win32 Debug"


"$(INTDIR)\ntservmsg.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\ntservmsg.res" /i "\dev\can\can\src\common" /d "_DEBUG" /d "_AFXDLL" $(SOURCE)


!ENDIF 


!ENDIF 

