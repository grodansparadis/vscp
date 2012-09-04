# Microsoft Developer Studio Generated NMAKE File, Based on syslogdrv.dsp
!IF "$(CFG)" == ""
CFG=syslog - Win32 Debug
!MESSAGE No configuration specified. Defaulting to syslog - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "syslog - Win32 Release" && "$(CFG)" != "syslog - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "syslogdrv.mak" CFG="syslog - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "syslog - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "syslog - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "syslog - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\syslogdrv.dll"


CLEAN :
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\syslog.obj"
	-@erase "$(INTDIR)\syslogdrv.obj"
	-@erase "$(INTDIR)\syslogobj.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\syslogdrv.dll"
	-@erase "$(OUTDIR)\syslogdrv.exp"
	-@erase "$(OUTDIR)\syslogdrv.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\base" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SYSLOGDRV_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\syslogdrv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=zlibd.lib regexd.lib pngd.lib jpegd.lib tiffd.lib wxmsw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib libexpat.lib net.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\syslogdrv.pdb" /machine:I386 /def:".\syslogdrv.def" /out:"$(OUTDIR)\syslogdrv.dll" /implib:"$(OUTDIR)\syslogdrv.lib" 
DEF_FILE= \
	".\syslogdrv.def"
LINK32_OBJS= \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\syslogobj.obj" \
	"$(INTDIR)\syslogdrv.obj" \
	"$(INTDIR)\syslog.obj" \
	"$(INTDIR)\dllist.obj"

"$(OUTDIR)\syslogdrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\syslogdrv.dll"
   copy release\*.dll D:\WINNT\system32
	copy release\*.dll ..\..\..\delivery
	copy release\*.lib ..\..\..\delivery
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "syslog - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\syslogdrv.dll" "$(OUTDIR)\syslogdrv.bsc"


CLEAN :
	-@erase "$(INTDIR)\dlldrvobj.obj"
	-@erase "$(INTDIR)\dlldrvobj.sbr"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\dllist.sbr"
	-@erase "$(INTDIR)\syslog.obj"
	-@erase "$(INTDIR)\syslog.sbr"
	-@erase "$(INTDIR)\syslogdrv.obj"
	-@erase "$(INTDIR)\syslogdrv.sbr"
	-@erase "$(INTDIR)\syslogobj.obj"
	-@erase "$(INTDIR)\syslogobj.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\syslogdrv.bsc"
	-@erase "$(OUTDIR)\syslogdrv.dll"
	-@erase "$(OUTDIR)\syslogdrv.exp"
	-@erase "$(OUTDIR)\syslogdrv.lib"
	-@erase "$(OUTDIR)\syslogdrv.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "d:\wxWindows-2.4.2\include" /I "d:\wxWindows-2.4.2\lib\based" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SYSLOGDRV_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\syslogdrv.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\dlldrvobj.sbr" \
	"$(INTDIR)\syslogobj.sbr" \
	"$(INTDIR)\syslogdrv.sbr" \
	"$(INTDIR)\syslog.sbr" \
	"$(INTDIR)\dllist.sbr"

"$(OUTDIR)\syslogdrv.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=zlibd.lib regexd.lib pngd.lib jpegd.lib tiffd.lib wxmswd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib rpcrt4.lib wsock32.lib libexpat.lib netd.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\syslogdrv.pdb" /debug /machine:I386 /def:".\syslogdrv.def" /out:"$(OUTDIR)\syslogdrv.dll" /implib:"$(OUTDIR)\syslogdrv.lib" /pdbtype:sept 
DEF_FILE= \
	".\syslogdrv.def"
LINK32_OBJS= \
	"$(INTDIR)\dlldrvobj.obj" \
	"$(INTDIR)\syslogobj.obj" \
	"$(INTDIR)\syslogdrv.obj" \
	"$(INTDIR)\syslog.obj" \
	"$(INTDIR)\dllist.obj"

"$(OUTDIR)\syslogdrv.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("syslogdrv.dep")
!INCLUDE "syslogdrv.dep"
!ELSE 
!MESSAGE Warning: cannot find "syslogdrv.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "syslog - Win32 Release" || "$(CFG)" == "syslog - Win32 Debug"
SOURCE=..\common\dlldrvobj.cpp

!IF  "$(CFG)" == "syslog - Win32 Release"


"$(INTDIR)\dlldrvobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "syslog - Win32 Debug"


"$(INTDIR)\dlldrvobj.obj"	"$(INTDIR)\dlldrvobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\..\common\dllist.c

!IF  "$(CFG)" == "syslog - Win32 Release"


"$(INTDIR)\dllist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "syslog - Win32 Debug"


"$(INTDIR)\dllist.obj"	"$(INTDIR)\dllist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\syslog.cpp

!IF  "$(CFG)" == "syslog - Win32 Release"


"$(INTDIR)\syslog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "syslog - Win32 Debug"


"$(INTDIR)\syslog.obj"	"$(INTDIR)\syslog.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\syslogdrv.cpp

!IF  "$(CFG)" == "syslog - Win32 Release"


"$(INTDIR)\syslogdrv.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "syslog - Win32 Debug"


"$(INTDIR)\syslogdrv.obj"	"$(INTDIR)\syslogdrv.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\common\syslogobj.cpp

!IF  "$(CFG)" == "syslog - Win32 Release"


"$(INTDIR)\syslogobj.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "syslog - Win32 Debug"


"$(INTDIR)\syslogobj.obj"	"$(INTDIR)\syslogobj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

