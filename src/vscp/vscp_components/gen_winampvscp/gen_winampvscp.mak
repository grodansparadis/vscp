# Microsoft Developer Studio Generated NMAKE File, Based on gen_winampvscp.dsp
!IF "$(CFG)" == ""
CFG=gen_winampvscp - Win32 Debug
!MESSAGE No configuration specified. Defaulting to gen_winampvscp - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "gen_winampvscp - Win32 Release" && "$(CFG)" != "gen_winampvscp - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gen_winampvscp.mak" CFG="gen_winampvscp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gen_winampvscp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gen_winampvscp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "gen_winampvscp - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\gen_winampvscp.dll"


CLEAN :
	-@erase "$(INTDIR)\configdialog.obj"
	-@erase "$(INTDIR)\configinfo.obj"
	-@erase "$(INTDIR)\configmanager.obj"
	-@erase "$(INTDIR)\gen_winampvscp.obj"
	-@erase "$(INTDIR)\gen_winampvscp.pch"
	-@erase "$(INTDIR)\gen_winampvscp.res"
	-@erase "$(INTDIR)\pluginmanager.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\gen_winampvscp.dll"
	-@erase "$(OUTDIR)\gen_winampvscp.exp"
	-@erase "$(OUTDIR)\gen_winampvscp.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D WINVER=0x500 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GEN_WINAMPVSCP_EXPORTS" /D ID3LIB_LINKOPTION=1 /Fp"$(INTDIR)\gen_winampvscp.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\gen_winampvscp.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gen_winampvscp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\gen_winampvscp.pdb" /machine:I386 /out:"$(OUTDIR)\gen_winampvscp.dll" /implib:"$(OUTDIR)\gen_winampvscp.lib" 
LINK32_OBJS= \
	"$(INTDIR)\configdialog.obj" \
	"$(INTDIR)\configinfo.obj" \
	"$(INTDIR)\configmanager.obj" \
	"$(INTDIR)\gen_winampvscp.obj" \
	"$(INTDIR)\pluginmanager.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\gen_winampvscp.res"

"$(OUTDIR)\gen_winampvscp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "gen_winampvscp - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\gen_winampvscp.dll" "$(OUTDIR)\gen_winampvscp.bsc"


CLEAN :
	-@erase "$(INTDIR)\configdialog.obj"
	-@erase "$(INTDIR)\configdialog.sbr"
	-@erase "$(INTDIR)\configinfo.obj"
	-@erase "$(INTDIR)\configinfo.sbr"
	-@erase "$(INTDIR)\configmanager.obj"
	-@erase "$(INTDIR)\configmanager.sbr"
	-@erase "$(INTDIR)\gen_winampvscp.obj"
	-@erase "$(INTDIR)\gen_winampvscp.pch"
	-@erase "$(INTDIR)\gen_winampvscp.res"
	-@erase "$(INTDIR)\gen_winampvscp.sbr"
	-@erase "$(INTDIR)\pluginmanager.obj"
	-@erase "$(INTDIR)\pluginmanager.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\util.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\gen_winampvscp.bsc"
	-@erase "$(OUTDIR)\gen_winampvscp.dll"
	-@erase "$(OUTDIR)\gen_winampvscp.exp"
	-@erase "$(OUTDIR)\gen_winampvscp.ilk"
	-@erase "$(OUTDIR)\gen_winampvscp.lib"
	-@erase "$(OUTDIR)\gen_winampvscp.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D WINVER=0x0500 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GEN_WINAMPVSCP_EXPORTS" /D ID3LIB_LINKOPTION=1 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\gen_winampvscp.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\gen_winampvscp.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gen_winampvscp.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\configdialog.sbr" \
	"$(INTDIR)\configinfo.sbr" \
	"$(INTDIR)\configmanager.sbr" \
	"$(INTDIR)\gen_winampvscp.sbr" \
	"$(INTDIR)\pluginmanager.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\util.sbr"

"$(OUTDIR)\gen_winampvscp.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=id3libD.lib ws2_32.lib comctl32.lib htmlhelp.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\gen_winampvscp.pdb" /debug /machine:I386 /out:"$(OUTDIR)\gen_winampvscp.dll" /implib:"$(OUTDIR)\gen_winampvscp.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\configdialog.obj" \
	"$(INTDIR)\configinfo.obj" \
	"$(INTDIR)\configmanager.obj" \
	"$(INTDIR)\gen_winampvscp.obj" \
	"$(INTDIR)\pluginmanager.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\gen_winampvscp.res"

"$(OUTDIR)\gen_winampvscp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("gen_winampvscp.dep")
!INCLUDE "gen_winampvscp.dep"
!ELSE 
!MESSAGE Warning: cannot find "gen_winampvscp.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "gen_winampvscp - Win32 Release" || "$(CFG)" == "gen_winampvscp - Win32 Debug"
SOURCE=.\configdialog.cpp

!IF  "$(CFG)" == "gen_winampvscp - Win32 Release"


"$(INTDIR)\configdialog.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gen_winampvscp.pch"


!ELSEIF  "$(CFG)" == "gen_winampvscp - Win32 Debug"


"$(INTDIR)\configdialog.obj"	"$(INTDIR)\configdialog.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gen_winampvscp.pch"


!ENDIF 

SOURCE=.\configinfo.cpp

!IF  "$(CFG)" == "gen_winampvscp - Win32 Release"


"$(INTDIR)\configinfo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gen_winampvscp.pch"


!ELSEIF  "$(CFG)" == "gen_winampvscp - Win32 Debug"


"$(INTDIR)\configinfo.obj"	"$(INTDIR)\configinfo.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gen_winampvscp.pch"


!ENDIF 

SOURCE=.\configmanager.cpp

!IF  "$(CFG)" == "gen_winampvscp - Win32 Release"


"$(INTDIR)\configmanager.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gen_winampvscp.pch"


!ELSEIF  "$(CFG)" == "gen_winampvscp - Win32 Debug"


"$(INTDIR)\configmanager.obj"	"$(INTDIR)\configmanager.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gen_winampvscp.pch"


!ENDIF 

SOURCE=.\gen_winampvscp.cpp

!IF  "$(CFG)" == "gen_winampvscp - Win32 Release"


"$(INTDIR)\gen_winampvscp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gen_winampvscp.pch"


!ELSEIF  "$(CFG)" == "gen_winampvscp - Win32 Debug"


"$(INTDIR)\gen_winampvscp.obj"	"$(INTDIR)\gen_winampvscp.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gen_winampvscp.pch"


!ENDIF 

SOURCE=.\gen_winampvscp.rc

"$(INTDIR)\gen_winampvscp.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\pluginmanager.cpp

!IF  "$(CFG)" == "gen_winampvscp - Win32 Release"


"$(INTDIR)\pluginmanager.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gen_winampvscp.pch"


!ELSEIF  "$(CFG)" == "gen_winampvscp - Win32 Debug"


"$(INTDIR)\pluginmanager.obj"	"$(INTDIR)\pluginmanager.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gen_winampvscp.pch"


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "gen_winampvscp - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /D WINVER=0x500 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GEN_WINAMPVSCP_EXPORTS" /D ID3LIB_LINKOPTION=1 /Fp"$(INTDIR)\gen_winampvscp.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\gen_winampvscp.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "gen_winampvscp - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /ZI /Od /D WINVER=0x0500 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GEN_WINAMPVSCP_EXPORTS" /D ID3LIB_LINKOPTION=1 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\gen_winampvscp.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\gen_winampvscp.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\util.cpp

!IF  "$(CFG)" == "gen_winampvscp - Win32 Release"


"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gen_winampvscp.pch"


!ELSEIF  "$(CFG)" == "gen_winampvscp - Win32 Debug"


"$(INTDIR)\util.obj"	"$(INTDIR)\util.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gen_winampvscp.pch"


!ENDIF 


!ENDIF 

