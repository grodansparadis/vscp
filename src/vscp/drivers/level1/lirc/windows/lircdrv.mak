# Microsoft Developer Studio Generated NMAKE File, Based on lircdrv.dsp
!IF "$(CFG)" == ""
CFG=lircdrv - Win32 Debug
!MESSAGE No configuration specified. Defaulting to lircdrv - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "lircdrv - Win32 Release" && "$(CFG)" != "lircdrv - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "lircdrv.mak" CFG="lircdrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "lircdrv - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "lircdrv - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "lircdrv - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\lircdrv.dll"


CLEAN :
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\lircdrv.obj"
	-@erase "$(INTDIR)\lircobj.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\lircdrv.dll"
	-@erase "$(OUTDIR)\lircdrv.exp"
	-@erase "$(OUTDIR)\lircdrv.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\base" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIRCDRV_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\lircdrv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=zlibd.lib regexd.lib pngd.lib jpegd.lib tiffd.lib wxmsw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib libexpat.lib net.lib  /nologo /dll /incremental:no /pdb:"$(OUTDIR)\lircdrv.pdb" /machine:I386 /def:".\lircdrv.def" /out:"$(OUTDIR)\lircdrv.dll" /implib:"$(OUTDIR)\lircdrv.lib" 
DEF_FILE= \
	".\lircdrv.def"
LINK32_OBJS= \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\lircdrv.obj" \
	"$(INTDIR)\lircobj.obj"

"$(OUTDIR)\lircdrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\lircdrv.dll"
   copy release\*.dll D:\WINNT\system32
	copy release\*.dll ..\..\..\delivery
	copy release\*.lib ..\..\..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "lircdrv - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\lircdrv.dll" "$(OUTDIR)\lircdrv.bsc"


CLEAN :
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dlldrvobj.sbr"
	-@erase "$(INTDIR)\lircdrv.obj"
	-@erase "$(INTDIR)\lircdrv.sbr"
	-@erase "$(INTDIR)\lircobj.obj"
	-@erase "$(INTDIR)\lircobj.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\lircdrv.bsc"
	-@erase "$(OUTDIR)\lircdrv.dll"
	-@erase "$(OUTDIR)\lircdrv.exp"
	-@erase "$(OUTDIR)\lircdrv.lib"
	-@erase "$(OUTDIR)\lircdrv.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\based" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIRCDRV_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\lircdrv.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\dlldrvobj.sbr" \
	"$(INTDIR)\lircdrv.sbr" \
	"$(INTDIR)\lircobj.sbr"

"$(OUTDIR)\lircdrv.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=zlibd.lib regexd.lib pngd.lib jpegd.lib tiffd.lib wxmswd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib libexpat.lib netd.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\lircdrv.pdb" /debug /machine:I386 /def:".\lircdrv.def" /out:"$(OUTDIR)\lircdrv.dll" /implib:"$(OUTDIR)\lircdrv.lib" /pdbtype:sept 
DEF_FILE= \
	".\lircdrv.def"
LINK32_OBJS= \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\lircdrv.obj" \
	"$(INTDIR)\lircobj.obj"

"$(OUTDIR)\lircdrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("lircdrv.dep")
!INCLUDE "lircdrv.dep"
!ELSE 
!MESSAGE Warning: cannot find "lircdrv.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "lircdrv - Win32 Release" || "$(CFG)" == "lircdrv - Win32 Debug"
SOURCE=..\common\dlldrvobj.cpp

!IF  "$(CFG)" == "lircdrv - Win32 Release"


"$(INTDIR)\dlldrvobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "lircdrv - Win32 Debug"


"$(INTDIR)\dlldrvobj.obj"	"$(INTDIR)\dlldrvobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\lircdrv.cpp

!IF  "$(CFG)" == "lircdrv - Win32 Release"


"$(INTDIR)\lircdrv.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "lircdrv - Win32 Debug"


"$(INTDIR)\lircdrv.obj"	"$(INTDIR)\lircdrv.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\common\lircobj.cpp

!IF  "$(CFG)" == "lircdrv - Win32 Release"


"$(INTDIR)\lircobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "lircdrv - Win32 Debug"


"$(INTDIR)\lircobj.obj"	"$(INTDIR)\lircobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

