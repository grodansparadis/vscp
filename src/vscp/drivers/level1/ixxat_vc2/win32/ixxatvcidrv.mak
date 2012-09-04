# Microsoft Developer Studio Generated NMAKE File, Based on ixxatvcidrv.dsp
!IF "$(CFG)" == ""
CFG=ixxatvcidrv - Win32 Debug
!MESSAGE No configuration specified. Defaulting to ixxatvcidrv - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ixxatvcidrv - Win32 Release" && "$(CFG)" != "ixxatvcidrv - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ixxatvcidrv.mak" CFG="ixxatvcidrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ixxatvcidrv - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ixxatvcidrv - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "ixxatvcidrv - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\ixxatvcidrv.dll"


CLEAN :
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\ixxatvcidrv.obj"
	-@erase "$(INTDIR)\ixxobj.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\ixxatvcidrv.dll"
	-@erase "$(OUTDIR)\ixxatvcidrv.exp"
	-@erase "$(OUTDIR)\ixxatvcidrv.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ixxatvcidrv_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ixxatvcidrv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Xat11Reg.lib vci11un6.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\ixxatvcidrv.pdb" /machine:I386 /def:".\ixxatvcidrv.def" /out:"$(OUTDIR)\ixxatvcidrv.dll" /implib:"$(OUTDIR)\ixxatvcidrv.lib" 
DEF_FILE= \
	".\ixxatvcidrv.def"
LINK32_OBJS= \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\ixxatvcidrv.obj" \
	"$(INTDIR)\ixxobj.obj"

"$(OUTDIR)\ixxatvcidrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\ixxatvcidrv.dll"
   copy release\*.dll D:\WINNT\system32
	copy release\*.dll ..\..\..\delivery
	copy release\*.lib ..\..\..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "ixxatvcidrv - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\ixxatvcidrv.dll" "$(OUTDIR)\ixxatvcidrv.bsc"


CLEAN :
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dlldrvobj.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\ixxatvcidrv.obj"
	-@erase "$(INTDIR)\ixxatvcidrv.sbr"
	-@erase "$(INTDIR)\ixxobj.obj"
	-@erase "$(INTDIR)\ixxobj.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\ixxatvcidrv.bsc"
	-@erase "$(OUTDIR)\ixxatvcidrv.dll"
	-@erase "$(OUTDIR)\ixxatvcidrv.exp"
	-@erase "$(OUTDIR)\ixxatvcidrv.lib"
	-@erase "$(OUTDIR)\ixxatvcidrv.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /GB /Gd /MTd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\based" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ixxatvcidrv_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ixxatvcidrv.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\dlldrvobj.sbr" \
	"$(INTDIR)\dllist.sbr" \
	"$(INTDIR)\ixxatvcidrv.sbr" \
	"$(INTDIR)\ixxobj.sbr"

"$(OUTDIR)\ixxatvcidrv.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wxmswd.lib Xat11Reg.lib vci11un6.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\ixxatvcidrv.pdb" /debug /machine:I386 /def:".\ixxatvcidrv.def" /out:"$(OUTDIR)\ixxatvcidrv.dll" /implib:"$(OUTDIR)\ixxatvcidrv.lib" /pdbtype:sept 
DEF_FILE= \
	".\ixxatvcidrv.def"
LINK32_OBJS= \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\ixxatvcidrv.obj" \
	"$(INTDIR)\ixxobj.obj"

"$(OUTDIR)\ixxatvcidrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("ixxatvcidrv.dep")
!INCLUDE "ixxatvcidrv.dep"
!ELSE 
!MESSAGE Warning: cannot find "ixxatvcidrv.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ixxatvcidrv - Win32 Release" || "$(CFG)" == "ixxatvcidrv - Win32 Debug"
SOURCE=..\common\dlldrvobj.cpp

!IF  "$(CFG)" == "ixxatvcidrv - Win32 Release"


"$(INTDIR)\dlldrvobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ixxatvcidrv - Win32 Debug"


"$(INTDIR)\dlldrvobj.obj"	"$(INTDIR)\dlldrvobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\..\common\dllist.c

!IF  "$(CFG)" == "ixxatvcidrv - Win32 Release"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ixxatvcidrv - Win32 Debug"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ixxatvcidrv.cpp

!IF  "$(CFG)" == "ixxatvcidrv - Win32 Release"


"$(INTDIR)\ixxatvcidrv.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "ixxatvcidrv - Win32 Debug"


"$(INTDIR)\ixxatvcidrv.obj"	"$(INTDIR)\ixxatvcidrv.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\common\ixxobj.cpp

!IF  "$(CFG)" == "ixxatvcidrv - Win32 Release"


"$(INTDIR)\ixxobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ixxatvcidrv - Win32 Debug"


"$(INTDIR)\ixxobj.obj"	"$(INTDIR)\ixxobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

