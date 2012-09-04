# Microsoft Developer Studio Generated NMAKE File, Based on canallogger.dsp
!IF "$(CFG)" == ""
CFG=canallogger - Win32 Debug
!MESSAGE No configuration specified. Defaulting to canallogger - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "canallogger - Win32 Release" && "$(CFG)" != "canallogger - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "canallogger.mak" CFG="canallogger - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "canallogger - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "canallogger - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "canallogger - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\canallogger.dll"


CLEAN :
	-@erase "$(INTDIR)\canallogger.obj"
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\canallogger.dll"
	-@erase "$(OUTDIR)\canallogger.exp"
	-@erase "$(OUTDIR)\canallogger.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "canallogger_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\canallogger.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Xat11Reg.lib vci11un6.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\canallogger.pdb" /machine:I386 /def:".\canallogger.def" /out:"$(OUTDIR)\canallogger.dll" /implib:"$(OUTDIR)\canallogger.lib" 
DEF_FILE= \
	".\canallogger.def"
LINK32_OBJS= \
	"$(INTDIR)\canallogger.obj" \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\log.obj"

"$(OUTDIR)\canallogger.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\canallogger.dll"
   copy release\*.dll D:\WINNT\system32
	copy release\*.dll ..\..\..\delivery
	copy release\*.lib ..\..\..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "canallogger - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\canallogger.dll" "$(OUTDIR)\canallogger.bsc"


CLEAN :
	-@erase "$(INTDIR)\canallogger.obj"
	-@erase "$(INTDIR)\canallogger.sbr"
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dlldrvobj.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\log.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\canallogger.bsc"
	-@erase "$(OUTDIR)\canallogger.dll"
	-@erase "$(OUTDIR)\canallogger.exp"
	-@erase "$(OUTDIR)\canallogger.lib"
	-@erase "$(OUTDIR)\canallogger.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\based" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "canallogger_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\canallogger.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canallogger.sbr" \
	"$(INTDIR)\dlldrvobj.sbr" \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\log.sbr"

"$(OUTDIR)\canallogger.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wxmswd.lib Xat11Reg.lib vci11un6.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\canallogger.pdb" /debug /machine:I386 /def:".\canallogger.def" /out:"$(OUTDIR)\canallogger.dll" /implib:"$(OUTDIR)\canallogger.lib" /pdbtype:sept 
DEF_FILE= \
	".\canallogger.def"
LINK32_OBJS= \
	"$(INTDIR)\canallogger.obj" \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\log.obj"

"$(OUTDIR)\canallogger.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("canallogger.dep")
!INCLUDE "canallogger.dep"
!ELSE 
!MESSAGE Warning: cannot find "canallogger.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "canallogger - Win32 Release" || "$(CFG)" == "canallogger - Win32 Debug"
SOURCE=.\canallogger.cpp

!IF  "$(CFG)" == "canallogger - Win32 Release"


"$(INTDIR)\canallogger.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "canallogger - Win32 Debug"


"$(INTDIR)\canallogger.obj"	"$(INTDIR)\canallogger.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\common\dlldrvobj.cpp

!IF  "$(CFG)" == "canallogger - Win32 Release"


"$(INTDIR)\dlldrvobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canallogger - Win32 Debug"


"$(INTDIR)\dlldrvobj.obj"	"$(INTDIR)\dlldrvobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\..\common\dllist.c

!IF  "$(CFG)" == "canallogger - Win32 Release"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canallogger - Win32 Debug"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\log.cpp

!IF  "$(CFG)" == "canallogger - Win32 Release"


"$(INTDIR)\log.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "canallogger - Win32 Debug"


"$(INTDIR)\log.obj"	"$(INTDIR)\log.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

