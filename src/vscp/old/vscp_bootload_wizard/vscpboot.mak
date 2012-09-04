# Microsoft Developer Studio Generated NMAKE File, Based on vscpboot.dsp
!IF "$(CFG)" == ""
CFG=vscpboot - Win32 Debug
!MESSAGE No configuration specified. Defaulting to vscpboot - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "vscpboot - Win32 Release DLL" && "$(CFG)" != "vscpboot - Win32 Debug DLL" && "$(CFG)" != "vscpboot - Win32 Release" && "$(CFG)" != "vscpboot - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vscpboot.mak" CFG="vscpboot - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vscpboot - Win32 Release DLL" (based on "Win32 (x86) Application")
!MESSAGE "vscpboot - Win32 Debug DLL" (based on "Win32 (x86) Application")
!MESSAGE "vscpboot - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "vscpboot - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"

OUTDIR=.\ReleaseDll
INTDIR=.\ReleaseDll
# Begin Custom Macros
OutDir=.\ReleaseDll
# End Custom Macros

ALL : "$(OUTDIR)\vscpboot.exe"


CLEAN :
	-@erase "$(INTDIR)\BootControl.obj"
	-@erase "$(INTDIR)\BootHexFileInfo.obj"
	-@erase "$(INTDIR)\BootLoaderPage.obj"
	-@erase "$(INTDIR)\ByeByePage.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\CanalSuperWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\IntelHex.obj"
	-@erase "$(INTDIR)\NickNameSelectionPage.obj"
	-@erase "$(INTDIR)\SelectAlgorithm.obj"
	-@erase "$(INTDIR)\SelectDevice.obj"
	-@erase "$(INTDIR)\SelectInputFile.obj"
	-@erase "$(INTDIR)\SelectInterfacePage.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vscpboot.obj"
	-@erase "$(INTDIR)\vscpboot.res"
	-@erase "$(INTDIR)\Welcome.obj"
	-@erase "$(OUTDIR)\vscpboot.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\mswdll\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\vscpboot.pch" /YX"wx/wxprec.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vscpboot.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\vscpboot.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msvcrt.lib libc.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\vscpboot.pdb" /machine:I386 /out:"$(OUTDIR)\vscpboot.exe" 
LINK32_OBJS= \
	"$(INTDIR)\BootControl.obj" \
	"$(INTDIR)\BootHexFileInfo.obj" \
	"$(INTDIR)\BootLoaderPage.obj" \
	"$(INTDIR)\ByeByePage.obj" \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\CanalSuperWrapper.obj" \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\IntelHex.obj" \
	"$(INTDIR)\NickNameSelectionPage.obj" \
	"$(INTDIR)\SelectAlgorithm.obj" \
	"$(INTDIR)\SelectDevice.obj" \
	"$(INTDIR)\SelectInputFile.obj" \
	"$(INTDIR)\SelectInterfacePage.obj" \
	"$(INTDIR)\vscpboot.obj" \
	"$(INTDIR)\Welcome.obj" \
	"$(INTDIR)\vscpboot.res"

"$(OUTDIR)\vscpboot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"

OUTDIR=.\DebugDll
INTDIR=.\DebugDll
# Begin Custom Macros
OutDir=.\DebugDll
# End Custom Macros

ALL : "$(OUTDIR)\vscpboot.exe"


CLEAN :
	-@erase "$(INTDIR)\BootControl.obj"
	-@erase "$(INTDIR)\BootHexFileInfo.obj"
	-@erase "$(INTDIR)\BootLoaderPage.obj"
	-@erase "$(INTDIR)\ByeByePage.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\CanalSuperWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\IntelHex.obj"
	-@erase "$(INTDIR)\NickNameSelectionPage.obj"
	-@erase "$(INTDIR)\SelectAlgorithm.obj"
	-@erase "$(INTDIR)\SelectDevice.obj"
	-@erase "$(INTDIR)\SelectInputFile.obj"
	-@erase "$(INTDIR)\SelectInterfacePage.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vscpboot.obj"
	-@erase "$(INTDIR)\vscpboot.res"
	-@erase "$(INTDIR)\Welcome.obj"
	-@erase "$(OUTDIR)\vscpboot.exe"
	-@erase "$(OUTDIR)\vscpboot.ilk"
	-@erase "$(OUTDIR)\vscpboot.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswdll\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /D "WXUSINGDLL" /Fp"$(INTDIR)\vscpboot.pch" /YX"wx/wxprec.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vscpboot.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\vscpboot.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw250d.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msvcrtd.lib libcd.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\vscpboot.pdb" /debug /machine:I386 /out:"$(OUTDIR)\vscpboot.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\BootControl.obj" \
	"$(INTDIR)\BootHexFileInfo.obj" \
	"$(INTDIR)\BootLoaderPage.obj" \
	"$(INTDIR)\ByeByePage.obj" \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\CanalSuperWrapper.obj" \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\IntelHex.obj" \
	"$(INTDIR)\NickNameSelectionPage.obj" \
	"$(INTDIR)\SelectAlgorithm.obj" \
	"$(INTDIR)\SelectDevice.obj" \
	"$(INTDIR)\SelectInputFile.obj" \
	"$(INTDIR)\SelectInterfacePage.obj" \
	"$(INTDIR)\vscpboot.obj" \
	"$(INTDIR)\Welcome.obj" \
	"$(INTDIR)\vscpboot.res"

"$(OUTDIR)\vscpboot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\vscpboot.exe" "$(OUTDIR)\vscpboot.bsc"


CLEAN :
	-@erase "$(INTDIR)\BootControl.obj"
	-@erase "$(INTDIR)\BootControl.sbr"
	-@erase "$(INTDIR)\BootHexFileInfo.obj"
	-@erase "$(INTDIR)\BootHexFileInfo.sbr"
	-@erase "$(INTDIR)\BootLoaderPage.obj"
	-@erase "$(INTDIR)\BootLoaderPage.sbr"
	-@erase "$(INTDIR)\ByeByePage.obj"
	-@erase "$(INTDIR)\ByeByePage.sbr"
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\CanalSuperWrapper.obj"
	-@erase "$(INTDIR)\CanalSuperWrapper.sbr"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.sbr"
	-@erase "$(INTDIR)\IntelHex.obj"
	-@erase "$(INTDIR)\IntelHex.sbr"
	-@erase "$(INTDIR)\NickNameSelectionPage.obj"
	-@erase "$(INTDIR)\NickNameSelectionPage.sbr"
	-@erase "$(INTDIR)\SelectAlgorithm.obj"
	-@erase "$(INTDIR)\SelectAlgorithm.sbr"
	-@erase "$(INTDIR)\SelectDevice.obj"
	-@erase "$(INTDIR)\SelectDevice.sbr"
	-@erase "$(INTDIR)\SelectInputFile.obj"
	-@erase "$(INTDIR)\SelectInputFile.sbr"
	-@erase "$(INTDIR)\SelectInterfacePage.obj"
	-@erase "$(INTDIR)\SelectInterfacePage.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vscpboot.obj"
	-@erase "$(INTDIR)\vscpboot.res"
	-@erase "$(INTDIR)\vscpboot.sbr"
	-@erase "$(INTDIR)\Welcome.obj"
	-@erase "$(INTDIR)\Welcome.sbr"
	-@erase "$(OUTDIR)\vscpboot.bsc"
	-@erase "$(OUTDIR)\vscpboot.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /O2 /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\vscpboot.pch" /YX"wx/wxprec.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vscpboot.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\vscpboot.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\BootControl.sbr" \
	"$(INTDIR)\BootHexFileInfo.sbr" \
	"$(INTDIR)\BootLoaderPage.sbr" \
	"$(INTDIR)\ByeByePage.sbr" \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\CanalSuperWrapper.sbr" \
	"$(INTDIR)\DllWrapper.sbr" \
	"$(INTDIR)\IntelHex.sbr" \
	"$(INTDIR)\NickNameSelectionPage.sbr" \
	"$(INTDIR)\SelectAlgorithm.sbr" \
	"$(INTDIR)\SelectDevice.sbr" \
	"$(INTDIR)\SelectInputFile.sbr" \
	"$(INTDIR)\SelectInterfacePage.sbr" \
	"$(INTDIR)\vscpboot.sbr" \
	"$(INTDIR)\Welcome.sbr"

"$(OUTDIR)\vscpboot.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmsw.lib comctl32.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msvcrt.lib libc.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\vscpboot.pdb" /machine:I386 /out:"$(OUTDIR)\vscpboot.exe" 
LINK32_OBJS= \
	"$(INTDIR)\BootControl.obj" \
	"$(INTDIR)\BootHexFileInfo.obj" \
	"$(INTDIR)\BootLoaderPage.obj" \
	"$(INTDIR)\ByeByePage.obj" \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\CanalSuperWrapper.obj" \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\IntelHex.obj" \
	"$(INTDIR)\NickNameSelectionPage.obj" \
	"$(INTDIR)\SelectAlgorithm.obj" \
	"$(INTDIR)\SelectDevice.obj" \
	"$(INTDIR)\SelectInputFile.obj" \
	"$(INTDIR)\SelectInterfacePage.obj" \
	"$(INTDIR)\vscpboot.obj" \
	"$(INTDIR)\Welcome.obj" \
	"$(INTDIR)\vscpboot.res"

"$(OUTDIR)\vscpboot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\vscpboot.exe" "$(OUTDIR)\vscpboot.bsc"
   copy release\*.exe ..\deliveryx
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\vscpboot.exe" "$(OUTDIR)\vscpboot.bsc"


CLEAN :
	-@erase "$(INTDIR)\BootControl.obj"
	-@erase "$(INTDIR)\BootControl.sbr"
	-@erase "$(INTDIR)\BootHexFileInfo.obj"
	-@erase "$(INTDIR)\BootHexFileInfo.sbr"
	-@erase "$(INTDIR)\BootLoaderPage.obj"
	-@erase "$(INTDIR)\BootLoaderPage.sbr"
	-@erase "$(INTDIR)\ByeByePage.obj"
	-@erase "$(INTDIR)\ByeByePage.sbr"
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\CanalSuperWrapper.obj"
	-@erase "$(INTDIR)\CanalSuperWrapper.sbr"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.sbr"
	-@erase "$(INTDIR)\IntelHex.obj"
	-@erase "$(INTDIR)\IntelHex.sbr"
	-@erase "$(INTDIR)\NickNameSelectionPage.obj"
	-@erase "$(INTDIR)\NickNameSelectionPage.sbr"
	-@erase "$(INTDIR)\SelectAlgorithm.obj"
	-@erase "$(INTDIR)\SelectAlgorithm.sbr"
	-@erase "$(INTDIR)\SelectDevice.obj"
	-@erase "$(INTDIR)\SelectDevice.sbr"
	-@erase "$(INTDIR)\SelectInputFile.obj"
	-@erase "$(INTDIR)\SelectInputFile.sbr"
	-@erase "$(INTDIR)\SelectInterfacePage.obj"
	-@erase "$(INTDIR)\SelectInterfacePage.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vscpboot.obj"
	-@erase "$(INTDIR)\vscpboot.res"
	-@erase "$(INTDIR)\vscpboot.sbr"
	-@erase "$(INTDIR)\Welcome.obj"
	-@erase "$(INTDIR)\Welcome.sbr"
	-@erase "$(OUTDIR)\vscpboot.bsc"
	-@erase "$(OUTDIR)\vscpboot.exe"
	-@erase "$(OUTDIR)\vscpboot.ilk"
	-@erase "$(OUTDIR)\vscpboot.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Zi /Od /I "$(WXWIN)\lib\mswd\\" /I "$(WXWIN)\include\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x400 /D "_MT" /D wxUSE_GUI=1 /D "__WXDEBUG__" /D WXDEBUG=1 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\vscpboot.pch" /YX"wx/wxprec.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vscpboot.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\vscpboot.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\BootControl.sbr" \
	"$(INTDIR)\BootHexFileInfo.sbr" \
	"$(INTDIR)\BootLoaderPage.sbr" \
	"$(INTDIR)\ByeByePage.sbr" \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\CanalSuperWrapper.sbr" \
	"$(INTDIR)\DllWrapper.sbr" \
	"$(INTDIR)\IntelHex.sbr" \
	"$(INTDIR)\NickNameSelectionPage.sbr" \
	"$(INTDIR)\SelectAlgorithm.sbr" \
	"$(INTDIR)\SelectDevice.sbr" \
	"$(INTDIR)\SelectInputFile.sbr" \
	"$(INTDIR)\SelectInterfacePage.sbr" \
	"$(INTDIR)\vscpboot.sbr" \
	"$(INTDIR)\Welcome.sbr"

"$(OUTDIR)\vscpboot.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(WXWIN)\lib\wxmswd.lib comctl32.lib rpcrt4.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msvcrtd.lib libcd.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\vscpboot.pdb" /debug /machine:I386 /out:"$(OUTDIR)\vscpboot.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\BootControl.obj" \
	"$(INTDIR)\BootHexFileInfo.obj" \
	"$(INTDIR)\BootLoaderPage.obj" \
	"$(INTDIR)\ByeByePage.obj" \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\CanalSuperWrapper.obj" \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\IntelHex.obj" \
	"$(INTDIR)\NickNameSelectionPage.obj" \
	"$(INTDIR)\SelectAlgorithm.obj" \
	"$(INTDIR)\SelectDevice.obj" \
	"$(INTDIR)\SelectInputFile.obj" \
	"$(INTDIR)\SelectInterfacePage.obj" \
	"$(INTDIR)\vscpboot.obj" \
	"$(INTDIR)\Welcome.obj" \
	"$(INTDIR)\vscpboot.res"

"$(OUTDIR)\vscpboot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("vscpboot.dep")
!INCLUDE "vscpboot.dep"
!ELSE 
!MESSAGE Warning: cannot find "vscpboot.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "vscpboot - Win32 Release DLL" || "$(CFG)" == "vscpboot - Win32 Debug DLL" || "$(CFG)" == "vscpboot - Win32 Release" || "$(CFG)" == "vscpboot - Win32 Debug"
SOURCE=.\vscpboot.rc

"$(INTDIR)\vscpboot.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\BootControl.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\BootControl.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\BootControl.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\BootControl.obj"	"$(INTDIR)\BootControl.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\BootControl.obj"	"$(INTDIR)\BootControl.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\BootHexFileInfo.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\BootHexFileInfo.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\BootHexFileInfo.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\BootHexFileInfo.obj"	"$(INTDIR)\BootHexFileInfo.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\BootHexFileInfo.obj"	"$(INTDIR)\BootHexFileInfo.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\BootLoaderPage.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\BootLoaderPage.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\BootLoaderPage.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\BootLoaderPage.obj"	"$(INTDIR)\BootLoaderPage.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\BootLoaderPage.obj"	"$(INTDIR)\BootLoaderPage.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ByeByePage.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\ByeByePage.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\ByeByePage.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\ByeByePage.obj"	"$(INTDIR)\ByeByePage.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\ByeByePage.obj"	"$(INTDIR)\ByeByePage.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\..\canal\common\canalshmem_level1_win32.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\canalshmem_level1_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\canalshmem_level1_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\canalshmem_level1_win32.obj"	"$(INTDIR)\canalshmem_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\canalshmem_level1_win32.obj"	"$(INTDIR)\canalshmem_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\canal\common\CanalSuperWrapper.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\CanalSuperWrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\CanalSuperWrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\CanalSuperWrapper.obj"	"$(INTDIR)\CanalSuperWrapper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\CanalSuperWrapper.obj"	"$(INTDIR)\CanalSuperWrapper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\canal\common\DllWrapper.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\DllWrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\DllWrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\DllWrapper.obj"	"$(INTDIR)\DllWrapper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\DllWrapper.obj"	"$(INTDIR)\DllWrapper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\IntelHex.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\IntelHex.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\IntelHex.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\IntelHex.obj"	"$(INTDIR)\IntelHex.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\IntelHex.obj"	"$(INTDIR)\IntelHex.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\NickNameSelectionPage.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\NickNameSelectionPage.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\NickNameSelectionPage.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\NickNameSelectionPage.obj"	"$(INTDIR)\NickNameSelectionPage.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\NickNameSelectionPage.obj"	"$(INTDIR)\NickNameSelectionPage.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\SelectAlgorithm.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\SelectAlgorithm.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\SelectAlgorithm.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\SelectAlgorithm.obj"	"$(INTDIR)\SelectAlgorithm.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\SelectAlgorithm.obj"	"$(INTDIR)\SelectAlgorithm.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\..\canal\common\SelectDevice.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\SelectDevice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\SelectDevice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\SelectDevice.obj"	"$(INTDIR)\SelectDevice.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\SelectDevice.obj"	"$(INTDIR)\SelectDevice.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\SelectInputFile.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\SelectInputFile.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\SelectInputFile.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\SelectInputFile.obj"	"$(INTDIR)\SelectInputFile.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\SelectInputFile.obj"	"$(INTDIR)\SelectInputFile.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\SelectInterfacePage.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\SelectInterfacePage.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\SelectInterfacePage.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\SelectInterfacePage.obj"	"$(INTDIR)\SelectInterfacePage.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\SelectInterfacePage.obj"	"$(INTDIR)\SelectInterfacePage.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\vscpboot.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\vscpboot.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\vscpboot.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\vscpboot.obj"	"$(INTDIR)\vscpboot.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\vscpboot.obj"	"$(INTDIR)\vscpboot.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\Welcome.cpp

!IF  "$(CFG)" == "vscpboot - Win32 Release DLL"


"$(INTDIR)\Welcome.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug DLL"


"$(INTDIR)\Welcome.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Release"


"$(INTDIR)\Welcome.obj"	"$(INTDIR)\Welcome.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vscpboot - Win32 Debug"


"$(INTDIR)\Welcome.obj"	"$(INTDIR)\Welcome.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

