# Microsoft Developer Studio Generated NMAKE File, Based on python_cci.dsp
!IF "$(CFG)" == ""
CFG=python_cci - Win32 Release
!MESSAGE No configuration specified. Defaulting to python_cci - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "python_cci - Win32 Debug" && "$(CFG)" != "python_cci - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "python_cci.mak" CFG="python_cci - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "python_cci - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "python_cci - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "python_cci - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : ".\python_cci_wrap.cpp" ".\_python_cci.dll"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\CanalSuperWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\python_cci_wrap.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\_python_cci.exp"
	-@erase "$(OUTDIR)\_python_cci.lib"
	-@erase "$(OUTDIR)\_python_cci.pdb"
	-@erase ".\_python_cci.dll"
	-@erase ".\_python_cci.ilk"
	-@erase "python_cci_wrap.cpp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(PYTHON_INCLUDE)" /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PYTHONCCI_EXPORTS" /Fp"$(INTDIR)\python_cci.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\python_cci.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib "$(PYTHON_LIB)" /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\_python_cci.pdb" /debug /machine:I386 /out:"_python_cci.dll" /implib:"$(OUTDIR)\_python_cci.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\CanalSuperWrapper.obj" \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\python_cci_wrap.obj"

".\_python_cci.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "python_cci - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : ".\python_cci_wrap.cpp" ".\_python_cci.dll" "$(OUTDIR)\python_cci.bsc"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\canalshmem_level1_win32.sbr"
	-@erase "$(INTDIR)\CanalSuperWrapper.obj"
	-@erase "$(INTDIR)\CanalSuperWrapper.sbr"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.sbr"
	-@erase "$(INTDIR)\python_cci_wrap.obj"
	-@erase "$(INTDIR)\python_cci_wrap.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\_python_cci.exp"
	-@erase "$(OUTDIR)\_python_cci.lib"
	-@erase "$(OUTDIR)\python_cci.bsc"
	-@erase ".\_python_cci.dll"
	-@erase "python_cci_wrap.cpp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "$(PYTHON_INCLUDE)" /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PYTHONCCI_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\python_cci.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\canalshmem_level1_win32.sbr" \
	"$(INTDIR)\CanalSuperWrapper.sbr" \
	"$(INTDIR)\DllWrapper.sbr" \
	"$(INTDIR)\python_cci_wrap.sbr"

"$(OUTDIR)\python_cci.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib libexpat.lib net.lib netapi32.lib "$(PYTHON_LIB)" /nologo /dll /incremental:no /pdb:"$(OUTDIR)\_python_cci.pdb" /machine:I386 /out:"_python_cci.dll" /implib:"$(OUTDIR)\_python_cci.lib" 
LINK32_OBJS= \
	"$(INTDIR)\canalshmem_level1_win32.obj" \
	"$(INTDIR)\CanalSuperWrapper.obj" \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\python_cci_wrap.obj"

".\_python_cci.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("python_cci.dep")
!INCLUDE "python_cci.dep"
!ELSE 
!MESSAGE Warning: cannot find "python_cci.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "python_cci - Win32 Debug" || "$(CFG)" == "python_cci - Win32 Release"
SOURCE=..\..\..\..\common\canalshmem_level1_win32.cpp

!IF  "$(CFG)" == "python_cci - Win32 Debug"


"$(INTDIR)\canalshmem_level1_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "python_cci - Win32 Release"


"$(INTDIR)\canalshmem_level1_win32.obj"	"$(INTDIR)\canalshmem_level1_win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\..\common\CanalSuperWrapper.cpp

!IF  "$(CFG)" == "python_cci - Win32 Debug"


"$(INTDIR)\CanalSuperWrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "python_cci - Win32 Release"


"$(INTDIR)\CanalSuperWrapper.obj"	"$(INTDIR)\CanalSuperWrapper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\..\common\DllWrapper.cpp

!IF  "$(CFG)" == "python_cci - Win32 Debug"


"$(INTDIR)\DllWrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "python_cci - Win32 Release"


"$(INTDIR)\DllWrapper.obj"	"$(INTDIR)\DllWrapper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\python_cci_wrap.cxx

!IF  "$(CFG)" == "python_cci - Win32 Debug"


"$(INTDIR)\python_cci_wrap.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "python_cci - Win32 Release"


"$(INTDIR)\python_cci_wrap.obj"	"$(INTDIR)\python_cci_wrap.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\python_cci.i

!IF  "$(CFG)" == "python_cci - Win32 Debug"

InputPath=.\python_cci.i
InputName=python_cci

".\python_cci_wrap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	echo In order to function correctly, please ensure the following environment variables are correctly set: 
	echo PYTHON_INCLUDE: %PYTHON_INCLUDE% 
	echo PYTHON_LIB: %PYTHON_LIB% 
	echo on 
	D:\SWIG-1.3.24\swig -c++ -python $(InputPath) 
<< 
	

!ELSEIF  "$(CFG)" == "python_cci - Win32 Release"

InputPath=.\python_cci.i
InputName=python_cci

".\python_cci_wrap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	echo In order to function correctly, please ensure the following environment variables are correctly set: 
	echo PYTHON_INCLUDE: %PYTHON_INCLUDE% 
	echo PYTHON_LIB: %PYTHON_LIB% 
	echo on 
	D:\SWIG-1.3.24\swig -c++ -python $(InputPath) 
<< 
	

!ENDIF 


!ENDIF 

