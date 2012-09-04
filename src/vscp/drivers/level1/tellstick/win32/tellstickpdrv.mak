# Microsoft Developer Studio Generated NMAKE File, Based on apoxdrv.dsp
!IF "$(CFG)" == ""
CFG=apoxdrv - Win32 Debug
!MESSAGE No configuration specified. Defaulting to apoxdrv - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "apoxdrv - Win32 Release" && "$(CFG)" != "apoxdrv - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "apoxdrv.mak" CFG="apoxdrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "apoxdrv - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "apoxdrv - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "apoxdrv - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\apoxdrv.dll"


CLEAN :
	-@erase "$(INTDIR)\apoxdrv.obj"
	-@erase "$(INTDIR)\apoxobj.obj"
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\apoxdrv.dll"
	-@erase "$(OUTDIR)\apoxdrv.exp"
	-@erase "$(OUTDIR)\apoxdrv.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "apoxdrv_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\apoxdrv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib FTD2XX.LIB /nologo /dll /incremental:no /pdb:"$(OUTDIR)\apoxdrv.pdb" /machine:I386 /def:".\apoxdrv.def" /out:"$(OUTDIR)\apoxdrv.dll" /implib:"$(OUTDIR)\apoxdrv.lib" 
DEF_FILE= \
	".\apoxdrv.def"
LINK32_OBJS= \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\apoxdrv.obj" \
	"$(INTDIR)\apoxobj.obj"

"$(OUTDIR)\apoxdrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\apoxdrv.dll"
   copy release\*.dll D:\WINNT\system32
	copy release\*.dll ..\..\..\delivery
	copy release\*.lib ..\..\..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "apoxdrv - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\apoxdrv.dll" "$(OUTDIR)\apoxdrv.bsc"


CLEAN :
	-@erase "$(INTDIR)\apoxdrv.obj"
	-@erase "$(INTDIR)\apoxdrv.sbr"
	-@erase "$(INTDIR)\apoxobj.obj"
	-@erase "$(INTDIR)\apoxobj.sbr"
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dlldrvobj.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\apoxdrv.bsc"
	-@erase "$(OUTDIR)\apoxdrv.dll"
	-@erase "$(OUTDIR)\apoxdrv.exp"
	-@erase "$(OUTDIR)\apoxdrv.lib"
	-@erase "$(OUTDIR)\apoxdrv.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\based" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "apoxdrv_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\apoxdrv.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\dlldrvobj.sbr" \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\apoxdrv.sbr" \
	"$(INTDIR)\apoxobj.sbr"

"$(OUTDIR)\apoxdrv.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wxmswd.lib FTD2XX.LIB /nologo /dll /incremental:no /pdb:"$(OUTDIR)\apoxdrv.pdb" /debug /machine:I386 /def:".\apoxdrv.def" /out:"$(OUTDIR)\apoxdrv.dll" /implib:"$(OUTDIR)\apoxdrv.lib" /pdbtype:sept 
DEF_FILE= \
	".\apoxdrv.def"
LINK32_OBJS= \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\apoxdrv.obj" \
	"$(INTDIR)\apoxobj.obj"

"$(OUTDIR)\apoxdrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("apoxdrv.dep")
!INCLUDE "apoxdrv.dep"
!ELSE 
!MESSAGE Warning: cannot find "apoxdrv.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "apoxdrv - Win32 Release" || "$(CFG)" == "apoxdrv - Win32 Debug"
SOURCE=.\apoxdrv.cpp

!IF  "$(CFG)" == "apoxdrv - Win32 Release"


"$(INTDIR)\apoxdrv.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "apoxdrv - Win32 Debug"


"$(INTDIR)\apoxdrv.obj"	"$(INTDIR)\apoxdrv.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\common\apoxobj.cpp

!IF  "$(CFG)" == "apoxdrv - Win32 Release"


"$(INTDIR)\apoxobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "apoxdrv - Win32 Debug"


"$(INTDIR)\apoxobj.obj"	"$(INTDIR)\apoxobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\dlldrvobj.cpp

!IF  "$(CFG)" == "apoxdrv - Win32 Release"


"$(INTDIR)\dlldrvobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "apoxdrv - Win32 Debug"


"$(INTDIR)\dlldrvobj.obj"	"$(INTDIR)\dlldrvobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\..\common\dllist.c

!IF  "$(CFG)" == "apoxdrv - Win32 Release"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "apoxdrv - Win32 Debug"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

