# Microsoft Developer Studio Generated NMAKE File, Based on jcanalci.dsp
!IF "$(CFG)" == ""
CFG=jcanalci - Win32 Release
!MESSAGE No configuration specified. Defaulting to jcanalci - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "jcanalci - Win32 Debug" && "$(CFG)" != "jcanalci - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "jcanalci.mak" CFG="jcanalci - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jcanalci - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jcanalci - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "jcanalci - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : ".\jcanalci.dll"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\CanalSuperWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\jcanalci_wrap.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\jcanalci.exp"
	-@erase "$(OUTDIR)\jcanalci.lib"
	-@erase "$(OUTDIR)\jcanalci.pdb"
	-@erase ".\jcanalci.dll"
	-@erase ".\jcanalci.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(JAVA_INCLUDE)" /I "$(JAVA_INCLUDE)\win32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JCANALCI_EXPORTS" /Fp"$(INTDIR)\jcanalci.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\jcanalci.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\jcanalci.pdb" /debug /machine:I386 /out:"jcanalci.dll" /implib:"$(OUTDIR)\jcanalci.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\CanalSuperWrapper.obj" \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\jcanalci_wrap.obj"

".\jcanalci.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=Java compile post-build step
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : ".\jcanalci.dll"
   echo on
	"%JAVA_BIN%\javac" *.java
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "jcanalci - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : ".\jcanalci.dll"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\CanalSuperWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\jcanalci_wrap.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\jcanalci.exp"
	-@erase "$(OUTDIR)\jcanalci.lib"
	-@erase ".\jcanalci.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "$(JAVA_INCLUDE)" /I "$(JAVA_INCLUDE)\win32" /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JCANALCI_EXPORTS" /Fp"$(INTDIR)\jcanalci.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\jcanalci.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib libexpat.lib net.lib netapi32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\jcanalci.pdb" /machine:I386 /out:"jcanalci.dll" /implib:"$(OUTDIR)\jcanalci.lib" 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\CanalSuperWrapper.obj" \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\jcanalci_wrap.obj"

".\jcanalci.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=Java compile post-build step
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : ".\jcanalci.dll"
   echo on
	"%JAVA_BIN%\javac"  *.java
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

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
!IF EXISTS("jcanalci.dep")
!INCLUDE "jcanalci.dep"
!ELSE 
!MESSAGE Warning: cannot find "jcanalci.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "jcanalci - Win32 Debug" || "$(CFG)" == "jcanalci - Win32 Release"
SOURCE=..\..\..\..\common\canalshmem_level1_win32.cpp

"$(INTDIR)\canalshmem_level1_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\..\common\CanalSuperWrapper.cpp

"$(INTDIR)\CanalSuperWrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\..\common\DllWrapper.cpp

"$(INTDIR)\DllWrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\jcanalci_wrap.cxx

"$(INTDIR)\jcanalci_wrap.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\jcanalci.i

!IF  "$(CFG)" == "jcanalci - Win32 Debug"

InputPath=.\jcanalci.i
InputName=jcanalci

".\jcanalci_wrap.cxx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	echo In order to function correctly, please ensure the following environment variables are correctly set: 
	echo JAVA_INCLUDE: %JAVA_INCLUDE% 
	echo JAVA_BIN: %JAVA_BIN% 
	echo on 
	..\..\..\swig.exe -c++ -java $(InputPath) 
<< 
	

!ELSEIF  "$(CFG)" == "jcanalci - Win32 Release"

InputPath=.\jcanalci.i
InputName=jcanalci

".\jcanalci_wrap.cxx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	echo In order to function correctly, please ensure the following environment variables are correctly set: 
	echo JAVA_INCLUDE: %JAVA_INCLUDE% 
	echo JAVA_BIN: %JAVA_BIN% 
	echo on 
	d:\SWIG-1.3.24\swig.exe -c++ -java $(InputPath) 
<< 
	

!ENDIF 


!ENDIF 

