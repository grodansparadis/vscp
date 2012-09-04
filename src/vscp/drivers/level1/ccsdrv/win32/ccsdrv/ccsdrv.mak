# Microsoft Developer Studio Generated NMAKE File, Based on ccsdrv.dsp
!IF "$(CFG)" == ""
CFG=ccsdrv - Win32 Debug
!MESSAGE No configuration specified. Defaulting to ccsdrv - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ccsdrv - Win32 Release" && "$(CFG)" != "ccsdrv - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ccsdrv.mak" CFG="ccsdrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ccsdrv - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ccsdrv - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "ccsdrv - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\ccsdrv.dll"


CLEAN :
	-@erase "$(INTDIR)\ccsdrv.obj"
	-@erase "$(INTDIR)\CCSObj.obj"
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\ccsdrv.dll"
	-@erase "$(OUTDIR)\ccsdrv.exp"
	-@erase "$(OUTDIR)\ccsdrv.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ccsdrv_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ccsdrv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib can.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\ccsdrv.pdb" /machine:I386 /def:".\ccsdrv.def" /out:"$(OUTDIR)\ccsdrv.dll" /implib:"$(OUTDIR)\ccsdrv.lib" 
DEF_FILE= \
	".\ccsdrv.def"
LINK32_OBJS= \
	"$(INTDIR)\ccsdrv.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\CCSObj.obj"

"$(OUTDIR)\ccsdrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\ccsdrv.dll"
   copy release\*.dll D:\WINNT\system32
	copy release\*.dll ..\..\..\..\delivery
	copy release\*.lib ..\..\..\..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "ccsdrv - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\ccsdrv.dll" "$(OUTDIR)\ccsdrv.bsc"


CLEAN :
	-@erase "$(INTDIR)\ccsdrv.obj"
	-@erase "$(INTDIR)\ccsdrv.sbr"
	-@erase "$(INTDIR)\CCSObj.obj"
	-@erase "$(INTDIR)\CCSObj.sbr"
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dlldrvobj.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\ccsdrv.bsc"
	-@erase "$(OUTDIR)\ccsdrv.dll"
	-@erase "$(OUTDIR)\ccsdrv.exp"
	-@erase "$(OUTDIR)\ccsdrv.ilk"
	-@erase "$(OUTDIR)\ccsdrv.lib"
	-@erase "$(OUTDIR)\ccsdrv.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\based" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ccsdrv_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ccsdrv.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ccsdrv.sbr" \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\dlldrvobj.sbr" \
	"$(INTDIR)\CCSObj.sbr"

"$(OUTDIR)\ccsdrv.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wxmswd.lib candbg.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\ccsdrv.pdb" /debug /machine:I386 /def:".\ccsdrv.def" /out:"$(OUTDIR)\ccsdrv.dll" /implib:"$(OUTDIR)\ccsdrv.lib" /pdbtype:sept 
DEF_FILE= \
	".\ccsdrv.def"
LINK32_OBJS= \
	"$(INTDIR)\ccsdrv.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\CCSObj.obj"

"$(OUTDIR)\ccsdrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("ccsdrv.dep")
!INCLUDE "ccsdrv.dep"
!ELSE 
!MESSAGE Warning: cannot find "ccsdrv.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ccsdrv - Win32 Release" || "$(CFG)" == "ccsdrv - Win32 Debug"
SOURCE=.\ccsdrv.cpp

!IF  "$(CFG)" == "ccsdrv - Win32 Release"


"$(INTDIR)\ccsdrv.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "ccsdrv - Win32 Debug"


"$(INTDIR)\ccsdrv.obj"	"$(INTDIR)\ccsdrv.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\..\common\CCSObj.cpp

!IF  "$(CFG)" == "ccsdrv - Win32 Release"


"$(INTDIR)\CCSObj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ccsdrv - Win32 Debug"


"$(INTDIR)\CCSObj.obj"	"$(INTDIR)\CCSObj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\dlldrvobj.cpp

!IF  "$(CFG)" == "ccsdrv - Win32 Release"


"$(INTDIR)\dlldrvobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ccsdrv - Win32 Debug"


"$(INTDIR)\dlldrvobj.obj"	"$(INTDIR)\dlldrvobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\..\..\common\dllist.c

!IF  "$(CFG)" == "ccsdrv - Win32 Release"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ccsdrv - Win32 Debug"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

