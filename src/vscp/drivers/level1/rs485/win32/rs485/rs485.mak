# Microsoft Developer Studio Generated NMAKE File, Based on can232drv.dsp
!IF "$(CFG)" == ""
CFG=can232drv - Win32 Debug
!MESSAGE No configuration specified. Defaulting to can232drv - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "can232drv - Win32 Release" && "$(CFG)" != "can232drv - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "can232drv.mak" CFG="can232drv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "can232drv - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "can232drv - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "can232drv - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\can232drv.dll"


CLEAN :
	-@erase "$(INTDIR)\can232drv.obj"
	-@erase "$(INTDIR)\can232obj.obj"
	-@erase "$(INTDIR)\com_win32.obj"
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\can232drv.dll"
	-@erase "$(OUTDIR)\can232drv.exp"
	-@erase "$(OUTDIR)\can232drv.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "can232drv_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\can232drv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Xat11Reg.lib vci11un6.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\can232drv.pdb" /machine:I386 /def:".\can232drv.def" /out:"$(OUTDIR)\can232drv.dll" /implib:"$(OUTDIR)\can232drv.lib" 
DEF_FILE= \
	".\can232drv.def"
LINK32_OBJS= \
	"$(INTDIR)\can232drv.obj" \
	"$(INTDIR)\can232obj.obj" \
	"$(INTDIR)\com_win32.obj" \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\dllist.obj"

"$(OUTDIR)\can232drv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\can232drv.dll"
   copy release\*.dll D:\WINNT\system32
	copy release\*.dll ..\..\..\..\delivery
	copy release\*.lib ..\..\..\..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "can232drv - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\can232drv.dll" "$(OUTDIR)\can232drv.bsc"


CLEAN :
	-@erase "$(INTDIR)\can232drv.obj"
	-@erase "$(INTDIR)\can232drv.sbr"
	-@erase "$(INTDIR)\can232obj.obj"
	-@erase "$(INTDIR)\can232obj.sbr"
	-@erase "$(INTDIR)\com_win32.obj"
	-@erase "$(INTDIR)\com_win32.sbr"
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dlldrvobj.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\can232drv.bsc"
	-@erase "$(OUTDIR)\can232drv.dll"
	-@erase "$(OUTDIR)\can232drv.exp"
	-@erase "$(OUTDIR)\can232drv.ilk"
	-@erase "$(OUTDIR)\can232drv.lib"
	-@erase "$(OUTDIR)\can232drv.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\based" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "can232drv_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\can232drv.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\can232drv.sbr" \
	"$(INTDIR)\can232obj.sbr" \
	"$(INTDIR)\com_win32.sbr" \
	"$(INTDIR)\dlldrvobj.sbr" \
	"$(INTDIR)\dllist.sbr"

"$(OUTDIR)\can232drv.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wxmswd.lib Xat11Reg.lib vci11un6.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\can232drv.pdb" /debug /machine:I386 /def:".\can232drv.def" /out:"$(OUTDIR)\can232drv.dll" /implib:"$(OUTDIR)\can232drv.lib" /pdbtype:sept 
DEF_FILE= \
	".\can232drv.def"
LINK32_OBJS= \
	"$(INTDIR)\can232drv.obj" \
	"$(INTDIR)\can232obj.obj" \
	"$(INTDIR)\com_win32.obj" \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\dllist.obj"

"$(OUTDIR)\can232drv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("can232drv.dep")
!INCLUDE "can232drv.dep"
!ELSE 
!MESSAGE Warning: cannot find "can232drv.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "can232drv - Win32 Release" || "$(CFG)" == "can232drv - Win32 Debug"
SOURCE=.\can232drv.cpp

!IF  "$(CFG)" == "can232drv - Win32 Release"


"$(INTDIR)\can232drv.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "can232drv - Win32 Debug"


"$(INTDIR)\can232drv.obj"	"$(INTDIR)\can232drv.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\..\common\can232obj.cpp

!IF  "$(CFG)" == "can232drv - Win32 Release"


"$(INTDIR)\can232obj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "can232drv - Win32 Debug"


"$(INTDIR)\can232obj.obj"	"$(INTDIR)\can232obj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\..\..\common\com_win32.cpp

!IF  "$(CFG)" == "can232drv - Win32 Release"


"$(INTDIR)\com_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "can232drv - Win32 Debug"


"$(INTDIR)\com_win32.obj"	"$(INTDIR)\com_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\dlldrvobj.cpp

!IF  "$(CFG)" == "can232drv - Win32 Release"


"$(INTDIR)\dlldrvobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "can232drv - Win32 Debug"


"$(INTDIR)\dlldrvobj.obj"	"$(INTDIR)\dlldrvobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\..\..\common\dllist.c

!IF  "$(CFG)" == "can232drv - Win32 Release"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "can232drv - Win32 Debug"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

