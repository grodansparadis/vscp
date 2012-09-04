# Microsoft Developer Studio Generated NMAKE File, Based on peakdrv.dsp
!IF "$(CFG)" == ""
CFG=peakdrv - Win32 Debug
!MESSAGE No configuration specified. Defaulting to peakdrv - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "peakdrv - Win32 Release" && "$(CFG)" != "peakdrv - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "peakdrv.mak" CFG="peakdrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "peakdrv - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "peakdrv - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "peakdrv - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\peakdrv.dll"


CLEAN :
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\peakdrv.obj"
	-@erase "$(INTDIR)\peakobj.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\peakdrv.dll"
	-@erase "$(OUTDIR)\peakdrv.exp"
	-@erase "$(OUTDIR)\peakdrv.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "$(WXWIN)\lib\msw\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "peakdrv_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\peakdrv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\peakdrv.pdb" /machine:I386 /def:".\peakdrv.def" /out:"$(OUTDIR)\peakdrv.dll" /implib:"$(OUTDIR)\peakdrv.lib" 
DEF_FILE= \
	".\peakdrv.def"
LINK32_OBJS= \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\peakdrv.obj" \
	"$(INTDIR)\peakobj.obj"

"$(OUTDIR)\peakdrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\peakdrv.dll"
   copy release\*.dll D:\WINNT\system32
	copy release\*.dll ..\..\..\delivery
	copy release\*.lib ..\..\..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "peakdrv - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\peakdrv.dll" "$(OUTDIR)\peakdrv.bsc"


CLEAN :
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dlldrvobj.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\peakdrv.obj"
	-@erase "$(INTDIR)\peakdrv.sbr"
	-@erase "$(INTDIR)\peakobj.obj"
	-@erase "$(INTDIR)\peakobj.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\peakdrv.bsc"
	-@erase "$(OUTDIR)\peakdrv.dll"
	-@erase "$(OUTDIR)\peakdrv.exp"
	-@erase "$(OUTDIR)\peakdrv.lib"
	-@erase "$(OUTDIR)\peakdrv.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "peakdrv_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\peakdrv.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\dlldrvobj.sbr" \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\peakdrv.sbr" \
	"$(INTDIR)\peakobj.sbr"

"$(OUTDIR)\peakdrv.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmswd.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\peakdrv.pdb" /debug /machine:I386 /def:".\peakdrv.def" /out:"$(OUTDIR)\peakdrv.dll" /implib:"$(OUTDIR)\peakdrv.lib" /pdbtype:sept 
DEF_FILE= \
	".\peakdrv.def"
LINK32_OBJS= \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\peakdrv.obj" \
	"$(INTDIR)\peakobj.obj"

"$(OUTDIR)\peakdrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("peakdrv.dep")
!INCLUDE "peakdrv.dep"
!ELSE 
!MESSAGE Warning: cannot find "peakdrv.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "peakdrv - Win32 Release" || "$(CFG)" == "peakdrv - Win32 Debug"
SOURCE=..\common\dlldrvobj.cpp

!IF  "$(CFG)" == "peakdrv - Win32 Release"


"$(INTDIR)\dlldrvobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "peakdrv - Win32 Debug"


"$(INTDIR)\dlldrvobj.obj"	"$(INTDIR)\dlldrvobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\..\common\dllist.c

!IF  "$(CFG)" == "peakdrv - Win32 Release"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "peakdrv - Win32 Debug"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\peakdrv.cpp

!IF  "$(CFG)" == "peakdrv - Win32 Release"


"$(INTDIR)\peakdrv.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "peakdrv - Win32 Debug"


"$(INTDIR)\peakdrv.obj"	"$(INTDIR)\peakdrv.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\common\peakobj.cpp

!IF  "$(CFG)" == "peakdrv - Win32 Release"


"$(INTDIR)\peakobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "peakdrv - Win32 Debug"


"$(INTDIR)\peakobj.obj"	"$(INTDIR)\peakobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

