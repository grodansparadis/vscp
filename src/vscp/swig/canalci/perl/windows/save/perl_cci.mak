# Microsoft Developer Studio Generated NMAKE File, Based on perl_cci.dsp
!IF "$(CFG)" == ""
CFG=perl_cci - Win32 Release
!MESSAGE No configuration specified. Defaulting to perl_cci - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "perl_cci - Win32 Debug" && "$(CFG)" != "perl_cci - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "perl_cci.mak" CFG="perl_cci - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "perl_cci - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "perl_cci - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "perl_cci - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : ".\perl_cci_wrap.cpp" ".\perl_cci.dll"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\CanalSuperWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\perl_cci.obj"
	-@erase "$(INTDIR)\perl_cci_wrap.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\perl_cci.exp"
	-@erase "$(OUTDIR)\perl_cci.lib"
	-@erase "$(OUTDIR)\perl_cci.pdb"
	-@erase ".\perl_cci.dll"
	-@erase ".\perl_cci.ilk"
	-@erase "perl_cci_wrap.cpp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(PERL5_INCLUDE)" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PERL_CCI_EXPORTS" /Fp"$(INTDIR)\perl_cci.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\perl_cci.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib "$(PERL5_LIB)" /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\perl_cci.pdb" /debug /machine:I386 /out:"perl_cci.dll" /implib:"$(OUTDIR)\perl_cci.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\perl_cci.obj" \
	"$(INTDIR)\perl_cci_wrap.obj" \
	"$(INTDIR)\CanalSuperWrapper.obj" \
	"$(INTDIR)\canalshmem_level1_win32.obj"

".\perl_cci.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "perl_cci - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : ".\perl_cci_wrap.cpp" ".\perl_cci.dll"


CLEAN :
	-@erase "$(INTDIR)\canalshmem_level1_win32.obj"
	-@erase "$(INTDIR)\CanalSuperWrapper.obj"
	-@erase "$(INTDIR)\DllWrapper.obj"
	-@erase "$(INTDIR)\perl_cci.obj"
	-@erase "$(INTDIR)\perl_cci_wrap.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\perl_cci.exp"
	-@erase "$(OUTDIR)\perl_cci.lib"
	-@erase ".\perl_cci.dll"
	-@erase "perl_cci_wrap.cpp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "$(PERL5_INCLUDE)" /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\msw" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PERL_CCI_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\perl_cci.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=zlib.lib regex.lib png.lib jpeg.lib tiff.lib wxmsw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib libexpat.lib net.lib netapi32.lib "$(PERL5_LIB)" /nologo /dll /incremental:no /pdb:"$(OUTDIR)\perl_cci.pdb" /machine:I386 /out:"perl_cci.dll" /implib:"$(OUTDIR)\perl_cci.lib" 
LINK32_OBJS= \
	"$(INTDIR)\DllWrapper.obj" \
	"$(INTDIR)\perl_cci.obj" \
	"$(INTDIR)\perl_cci_wrap.obj" \
	"$(INTDIR)\CanalSuperWrapper.obj" \
	"$(INTDIR)\canalshmem_level1_win32.obj"

".\perl_cci.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("perl_cci.dep")
!INCLUDE "perl_cci.dep"
!ELSE 
!MESSAGE Warning: cannot find "perl_cci.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "perl_cci - Win32 Debug" || "$(CFG)" == "perl_cci - Win32 Release"
SOURCE=..\..\..\..\common\canalshmem_level1_win32.cpp

"$(INTDIR)\canalshmem_level1_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\..\common\CanalSuperWrapper.cpp

"$(INTDIR)\CanalSuperWrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\..\common\DllWrapper.cpp

"$(INTDIR)\DllWrapper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\perl_cci.cpp

"$(INTDIR)\perl_cci.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\perl_cci_wrap.cxx

"$(INTDIR)\perl_cci_wrap.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\perl_cci.i

!IF  "$(CFG)" == "perl_cci - Win32 Debug"

InputPath=.\perl_cci.i
InputName=perl_cci

".\perl_cci_wrap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	echo In order to function correctly, please ensure the following environment variables are correctly set: 
	echo PERL5_INCLUDE: %PERL5_INCLUDE% 
	echo PERL5_LIB: %PERL5_LIB% 
	echo on 
	..\..\..\swig -c++ -perl5 $(InputPath) 
<< 
	

!ELSEIF  "$(CFG)" == "perl_cci - Win32 Release"

InputPath=.\perl_cci.i
InputName=perl_cci

".\perl_cci_wrap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	echo In order to function correctly, please ensure the following environment variables are correctly set: 
	echo PERL5_INCLUDE: %PERL5_INCLUDE% 
	echo PERL5_LIB: %PERL5_LIB% 
	echo on 
	d:\swig-1.3.24\swig.exe -c++ -perl5 $(InputPath) 
<< 
	

!ENDIF 


!ENDIF 

