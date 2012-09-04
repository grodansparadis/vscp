# Microsoft Developer Studio Generated NMAKE File, Based on java_cci.dsp
!IF "$(CFG)" == ""
CFG=java_cci - Win32 Release
!MESSAGE No configuration specified. Defaulting to java_cci - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "java_cci - Win32 Debug" && "$(CFG)" != "java_cci - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "java_cci.mak" CFG="java_cci - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "java_cci - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "java_cci - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "java_cci - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : ".\java_cci.dll"


CLEAN :
	-@erase "$(INTDIR)\java_cci.obj"
	-@erase "$(INTDIR)\java_cci_wrap.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\java_cci.exp"
	-@erase "$(OUTDIR)\java_cci.lib"
	-@erase "$(OUTDIR)\java_cci.pdb"
	-@erase ".\java_cci.dll"
	-@erase ".\java_cci.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(JAVA_INCLUDE)" /I "$(JAVA_INCLUDE)\win32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JAVACCI_EXPORTS" /Fp"$(INTDIR)\java_cci.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\java_cci.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\java_cci.pdb" /debug /machine:I386 /out:"java_cci.dll" /implib:"$(OUTDIR)\java_cci.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\java_cci.obj" \
	"$(INTDIR)\java_cci_wrap.obj"

".\java_cci.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=Java compile post-build step
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : ".\java_cci.dll"
   echo on
	"%JAVA_BIN%\javac" *.java
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "java_cci - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : ".\java_cci.dll" "$(OUTDIR)\java_cci.bsc"


CLEAN :
	-@erase "$(INTDIR)\java_cci.obj"
	-@erase "$(INTDIR)\java_cci.sbr"
	-@erase "$(INTDIR)\java_cci_wrap.obj"
	-@erase "$(INTDIR)\java_cci_wrap.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\java_cci.bsc"
	-@erase "$(OUTDIR)\java_cci.exp"
	-@erase "$(OUTDIR)\java_cci.lib"
	-@erase ".\java_cci.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "$(JAVA_INCLUDE)" /I "$(JAVA_INCLUDE)\win32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JAVACCI_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\java_cci.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\java_cci.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\java_cci.sbr" \
	"$(INTDIR)\java_cci_wrap.sbr"

"$(OUTDIR)\java_cci.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  ../../../../delivery/canaldll.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\java_cci.pdb" /machine:I386 /out:"java_cci.dll" /implib:"$(OUTDIR)\java_cci.lib" 
LINK32_OBJS= \
	"$(INTDIR)\java_cci.obj" \
	"$(INTDIR)\java_cci_wrap.obj"

".\java_cci.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=Java compile post-build step
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : ".\java_cci.dll" "$(OUTDIR)\java_cci.bsc"
   echo on
	"%JAVA_BIN%\javac" *.java
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
!IF EXISTS("java_cci.dep")
!INCLUDE "java_cci.dep"
!ELSE 
!MESSAGE Warning: cannot find "java_cci.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "java_cci - Win32 Debug" || "$(CFG)" == "java_cci - Win32 Release"
SOURCE=.\java_cci.c

!IF  "$(CFG)" == "java_cci - Win32 Debug"


"$(INTDIR)\java_cci.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "java_cci - Win32 Release"


"$(INTDIR)\java_cci.obj"	"$(INTDIR)\java_cci.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\java_cci_wrap.c

!IF  "$(CFG)" == "java_cci - Win32 Debug"


"$(INTDIR)\java_cci_wrap.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "java_cci - Win32 Release"


"$(INTDIR)\java_cci_wrap.obj"	"$(INTDIR)\java_cci_wrap.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\java_cci.i

!IF  "$(CFG)" == "java_cci - Win32 Debug"

InputPath=.\java_cci.i
InputName=java_cci

".\java_cci_wrap.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	echo In order to function correctly, please ensure the following environment variables are correctly set: 
	echo JAVA_INCLUDE: %JAVA_INCLUDE% 
	echo JAVA_BIN: %JAVA_BIN% 
	echo on 
	..\..\..\swig.exe -java $(InputPath) 
<< 
	

!ELSEIF  "$(CFG)" == "java_cci - Win32 Release"

InputPath=.\java_cci.i
InputName=java_cci

".\java_cci_wrap.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	echo In order to function correctly, please ensure the following environment variables are correctly set: 
	echo JAVA_INCLUDE: %JAVA_INCLUDE% 
	echo JAVA_BIN: %JAVA_BIN% 
	echo on 
	d:\SWIG-1.3.24\swig.exe -java $(InputPath) 
<< 
	

!ENDIF 


!ENDIF 

