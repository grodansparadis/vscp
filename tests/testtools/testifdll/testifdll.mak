# Microsoft Developer Studio Generated NMAKE File, Based on testifdll.dsp
!IF "$(CFG)" == ""
CFG=testifdll - Win32 Debug
!MESSAGE No configuration specified. Defaulting to testifdll - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "testifdll - Win32 Release" && "$(CFG)" != "testifdll - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "testifdll.mak" CFG="testifdll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "testifdll - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "testifdll - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "testifdll - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\testifdll.exe" "$(OUTDIR)\testifdll.pch" "$(OUTDIR)\testifdll.bsc"


CLEAN :
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\testifdll.obj"
	-@erase "$(INTDIR)\testifdll.pch"
	-@erase "$(INTDIR)\testifdll.res"
	-@erase "$(INTDIR)\testifdll.sbr"
	-@erase "$(INTDIR)\testifdllDlg.obj"
	-@erase "$(INTDIR)\testifdllDlg.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\testifdll.bsc"
	-@erase "$(OUTDIR)\testifdll.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\testifdll.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testifdll.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\DllWrapper.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\testifdll.sbr" \
	"$(INTDIR)\testifdllDlg.sbr"

"$(OUTDIR)\testifdll.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=zlibd.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib netapi32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\testifdll.pdb" /machine:I386 /out:"$(OUTDIR)\testifdll.exe" 
LINK32_OBJS= \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\testifdll.obj" \
	"$(INTDIR)\testifdllDlg.obj" \
	"$(INTDIR)\testifdll.res"

"$(OUTDIR)\testifdll.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\testifdll.exe" "$(OUTDIR)\testifdll.pch" "$(OUTDIR)\testifdll.bsc"
   copy release\*.exe ..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "testifdll - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\testifdll.exe" "$(OUTDIR)\testifdll.pch"


CLEAN :
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\testifdll.obj"
	-@erase "$(INTDIR)\testifdll.pch"
	-@erase "$(INTDIR)\testifdll.res"
	-@erase "$(INTDIR)\testifdllDlg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\testifdll.exe"
	-@erase "$(OUTDIR)\testifdll.ilk"
	-@erase "$(OUTDIR)\testifdll.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\mswd" /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\testifdll.pch" /YX"wx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\testifdll.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testifdll.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=zlibd.lib regexd.lib pngd.lib jpegd.lib tiffd.lib wxmswd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib netapi32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\testifdll.pdb" /debug /machine:I386 /out:"$(OUTDIR)\testifdll.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\testifdll.obj" \
	"$(INTDIR)\testifdllDlg.obj" \
	"$(INTDIR)\testifdll.res"

"$(OUTDIR)\testifdll.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("testifdll.dep")
!INCLUDE "testifdll.dep"
!ELSE 
!MESSAGE Warning: cannot find "testifdll.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "testifdll - Win32 Release" || "$(CFG)" == "testifdll - Win32 Debug"
SOURCE=..\common\DllWrapper.cpp

!IF  "$(CFG)" == "testifdll - Win32 Release"


"$(INTDIR)\DllWrapper.obj"	"$(INTDIR)\DllWrapper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "testifdll - Win32 Debug"


"$(INTDIR)\DllWrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "testifdll - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\testifdll.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\testifdll.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "testifdll - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\mswd" /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\testifdll.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\testifdll.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\testifdll.cpp

!IF  "$(CFG)" == "testifdll - Win32 Release"


"$(INTDIR)\testifdll.obj"	"$(INTDIR)\testifdll.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "testifdll - Win32 Debug"


"$(INTDIR)\testifdll.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\testifdll.rc

"$(INTDIR)\testifdll.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\testifdllDlg.cpp

!IF  "$(CFG)" == "testifdll - Win32 Release"


"$(INTDIR)\testifdllDlg.obj"	"$(INTDIR)\testifdllDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "testifdll - Win32 Debug"


"$(INTDIR)\testifdllDlg.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

