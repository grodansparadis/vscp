# Microsoft Developer Studio Generated NMAKE File, Based on canaldll.dsp
!IF "$(CFG)" == ""
CFG=canaldll - Win32 Debug
!MESSAGE No configuration specified. Defaulting to canaldll - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "canaldll - Win32 Release" && "$(CFG)" != "canaldll - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "canaldll.mak" CFG="canaldll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "canaldll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "canaldll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "canaldll - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\canaldll.dll" "$(OUTDIR)\canaldll.pch" "$(OUTDIR)\canaldll.bsc"

!ELSE 

ALL : "testifdll - Win32 Release" "$(OUTDIR)\canaldll.dll" "$(OUTDIR)\canaldll.pch" "$(OUTDIR)\canaldll.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"testifdll - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\canaldll.obj"
	-@erase "$(INTDIR)\canaldll.pch"
	-@erase "$(INTDIR)\canaldll.res"
	-@erase "$(INTDIR)\canaldll.sbr"
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\canaldll.bsc"
	-@erase "$(OUTDIR)\canaldll.dll"
	-@erase "$(OUTDIR)\canaldll.exp"
	-@erase "$(OUTDIR)\canaldll.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\canaldll.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\canaldll.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canaldll.sbr" \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\canaldll.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib libexpat.lib net.lib netapi32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\canaldll.pdb" /machine:I386 /def:".\canaldll.def" /out:"$(OUTDIR)\canaldll.dll" /implib:"$(OUTDIR)\canaldll.lib" 
DEF_FILE= \
	".\canaldll.def"
LINK32_OBJS= \
	"$(INTDIR)\canaldll.obj" \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\canaldll.res"

"$(OUTDIR)\canaldll.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "testifdll - Win32 Release" "$(OUTDIR)\canaldll.dll" "$(OUTDIR)\canaldll.pch" "$(OUTDIR)\canaldll.bsc"
   copy release\*.dll ..\delivery
	copy release\*.lib ..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "canaldll - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\canaldll.dll" "$(OUTDIR)\canaldll.pch" "$(OUTDIR)\canaldll.bsc"

!ELSE 

ALL : "testifdll - Win32 Debug" "$(OUTDIR)\canaldll.dll" "$(OUTDIR)\canaldll.pch" "$(OUTDIR)\canaldll.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"testifdll - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\canaldll.obj"
	-@erase "$(INTDIR)\canaldll.pch"
	-@erase "$(INTDIR)\canaldll.res"
	-@erase "$(INTDIR)\canaldll.sbr"
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\canaldll.bsc"
	-@erase "$(OUTDIR)\canaldll.dll"
	-@erase "$(OUTDIR)\canaldll.exp"
	-@erase "$(OUTDIR)\canaldll.ilk"
	-@erase "$(OUTDIR)\canaldll.lib"
	-@erase "$(OUTDIR)\canaldll.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\mswd" /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\canaldll.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\canaldll.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canaldll.sbr" \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\canaldll.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=zlibd.lib regexd.lib pngd.lib jpegd.lib tiffd.lib wxmswd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib netapi32.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\canaldll.pdb" /debug /machine:I386 /def:".\canaldll.def" /out:"$(OUTDIR)\canaldll.dll" /implib:"$(OUTDIR)\canaldll.lib" /pdbtype:sept 
DEF_FILE= \
	".\canaldll.def"
LINK32_OBJS= \
	"$(INTDIR)\canaldll.obj" \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\canaldll.res"

"$(OUTDIR)\canaldll.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("canaldll.dep")
!INCLUDE "canaldll.dep"
!ELSE 
!MESSAGE Warning: cannot find "canaldll.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "canaldll - Win32 Release" || "$(CFG)" == "canaldll - Win32 Debug"
SOURCE=.\canaldll.cpp

"$(INTDIR)\canaldll.obj"	"$(INTDIR)\canaldll.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\canaldll.rc

"$(INTDIR)\canaldll.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=..\common\canalshmem_level1_win32.cpp

"$(INTDIR)\canalshmem_level1_win32.obj"	"$(INTDIR)\canalshmem_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "canaldll - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\canaldll.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\canaldll.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "canaldll - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\mswd" /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\canaldll.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\canaldll.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

!IF  "$(CFG)" == "canaldll - Win32 Release"

"testifdll - Win32 Release" : 
   cd "\dev\can\can\src\canal\testifdll"
   $(MAKE) /$(MAKEFLAGS) /F .\testifdll.mak CFG="testifdll - Win32 Release" 
   cd "..\canaldll"

"testifdll - Win32 ReleaseCLEAN" : 
   cd "\dev\can\can\src\canal\testifdll"
   $(MAKE) /$(MAKEFLAGS) /F .\testifdll.mak CFG="testifdll - Win32 Release" RECURSE=1 CLEAN 
   cd "..\canaldll"

!ELSEIF  "$(CFG)" == "canaldll - Win32 Debug"

"testifdll - Win32 Debug" : 
   cd "\dev\can\can\src\canal\testifdll"
   $(MAKE) /$(MAKEFLAGS) /F .\testifdll.mak CFG="testifdll - Win32 Debug" 
   cd "..\canaldll"

"testifdll - Win32 DebugCLEAN" : 
   cd "\dev\can\can\src\canal\testifdll"
   $(MAKE) /$(MAKEFLAGS) /F .\testifdll.mak CFG="testifdll - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\canaldll"

!ENDIF 


!ENDIF 

