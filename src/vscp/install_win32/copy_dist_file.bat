REM Create folder
mkdir files
mkdir files\system
mkdit files\drivers
mkdir files\drivers\level1
mkdir files\drivers\level1\lib
mkdir files\drivers\level2
mkdir files\drivers\level2\lib
mkdir files\include
mkdir files\cpp
mkdir files\lib
mkdir files\doc
mkdir files\work
mkdir files\examples
mkdir files\wx
mkdir files\vscpworks
mkdir files\vscpd

REM Work programs
copy vcredist_x86.exe files\work
copy WinPcap_4_1_2.exe files\work
copy vscpworks_config\* files\vscpworks
copy vscpd_config\* files\vscpd

REM Client program
copy ..\build\release\vscpworks.exe files
copy ..\build\release\vscpcmd.exe files
copy ..\build\release\mkpasswd.exe files
copy README.txt files
copy iflist.exe files
copy "..\build\release\simplebutton.exe" files\examples
copy "..\build\release\simplelamp.exe" files\examples
copy "..\build\release\simpledimmer.exe" files\examples

REM Server components
copy "..\build\Release\vscpservice.exe" files
copy "..\build\Release\vscpd.exe" files
copy "..\build\Release\mkpasswd.exe" files

REM Copy test file for DM
copy test.txt files

REM Development tools  
copy "..\build\Release\vscphelper.dll" files\system
copy "..\build\Release\vscphelper.lib" files\lib

REM Drivers
copy "..\build\Release\apoxdrv.dll" files\system
copy "..\build\Release\apoxdrv.dll" files\drivers\level1
copy "..\build\Release\apoxdrv.lib" files\drivers\level1\lib

copy "..\build\Release\can232drv.dll" files\system
copy "..\build\Release\can232drv.dll" files\drivers\level1
copy "..\build\Release\can232drv.lib" files\drivers\level1\lib

copy "..\build\Release\can4vscpdrv.dll" files\system
copy "..\build\Release\can4vscpdrv.dll" files\drivers\level1
copy "..\build\Release\can4vscpdrv.lib" files\drivers\level1\lib

copy "..\build\Release\ccsdrv.dll" files\system
copy "..\build\Release\ccsdrv.dll" files\drivers\level1
copy "..\build\Release\ccsdrv.lib" files\drivers\level1\lib

copy "..\build\Release\ixxatvcidrv.dll" files\system
copy "..\build\Release\ixxatvcidrv.dll" files\drivers\level1
copy "..\build\Release\ixxatvcidrv.lib" files\drivers\level1\lib

copy "..\build\Release\lircdrv.dll" files\system
copy "..\build\Release\lircdrv.dll" files\drivers\level1
copy "..\build\Release\lircdrv.lib" files\drivers\level1\lib

copy "..\build\Release\canallogger.dll" files\system
copy "..\build\Release\canallogger.dll" files\drivers\level1
copy "..\build\Release\canallogger.lib" files\drivers\level1\lib

copy "..\build\Release\peakdrv.dll" files\system
copy "..\build\Release\peakdrv.dll" files\drivers\level1
copy "..\build\Release\peakdrv.lib" files\drivers\level1\lib

copy "..\build\Release\syslogdrv.dll" files\system
copy "..\build\Release\syslogdrv.dll" files\drivers\level1
copy "..\build\Release\syslogdrv.lib" files\drivers\level1\lib

copy "..\build\Release\vectordrv.dll" files\system
copy "..\build\Release\vectordrv.dll" files\drivers\level1
copy "..\build\Release\vectordrv.lib" files\drivers\level1\lib

copy "..\build\Release\xapdrv.dll" files\system
copy "..\build\Release\xapdrv.dll" files\drivers\level1
copy "..\build\Release\xapdrv.lib" files\drivers\level1\lib

copy "..\build\Release\zanthicdrv.dll" files\system
copy "..\build\Release\zanthicdrv.dll" files\drivers\level1
copy "..\build\Release\zanthicdrv.lib" files\drivers\level1\lib

copy "c:\users\akhe\my documents\development\lawicel\can\canusbdrv\Release\canusbdrv.dll" files\system
copy "c:\users\akhe\my documents\development\lawicel\can\canusbdrv\Release\canusbdrv.dll" files\drivers\level1
copy "c:\users\akhe\my documents\development\lawicel\can\canusbdrv\Release\canusbdrv.lib" files\drivers\level1\lib

copy "..\build\Release\tellstickdrv.dll" files\system
copy "..\build\Release\tellstickdrv.dll" files\drivers\level1
copy "c:\users\akhe\my documents\development\lawicel\can\adaptercheck\Release\adaptercheck.exe" files\drivers\level1
copy "..\build\Release\tellstickdrv.lib" files\drivers\level1\lib

copy "..\build\Release\vscpbtdetect.dll" files\system
copy "..\build\Release\vscpbtdetect.dll" files\drivers\level2
copy "..\build\Release\vscpbtdetect.lib" files\drivers\level2\lib

copy "..\build\Release\vscplogger.dll" files\system
copy "..\build\Release\vscplogger.dll" files\drivers\level2
copy "..\build\Release\vscplogger.lib" files\drivers\level2\lib

copy "..\build\Release\rawethernet.dll" files\system
copy "..\build\Release\rawethernet.dll" files\drivers\level2
copy "..\build\Release\rawethernet.lib" files\drivers\level2\lib


REM Copy development files
copy ..\common\canal.h files\include
copy ..\common\vscp.h files\include
copy ..\common\canal_macro.h files\include
copy ..\common\canaldlldef.h files\include
copy ..\common\canalsuperwrapper.cpp files\cpp
copy ..\common\canalsuperwrapper.h files\cpp
copy ..\common\vscphelper.cpp files\cpp
copy ..\common\vscp_class.h files\include
copy ..\common\vscp_type.h files\include
copy ..\common\vscptcpif.cpp files\cpp
copy ..\common\vscptcpif.h files\cpp
copy ..\common\guid.cpp files\cpp
copy ..\common\guid.h files\cpp
copy ..\common\vscphelper.cpp files\cpp
copy ..\common\vscphelper.h files\cpp
copy ..\..\common\crc.c files\cpp
copy ..\..\common\crc.h files\cpp
copy ..\..\common\md5.cpp files\cpp
copy ..\..\common\md5.h files\cpp
copy ..\..\common\dllist.c files\cpp
copy ..\..\common\dllist.h files\cpp

REM Copy doc files
copy ..\..\..\docs\vscp\vscp_specification\vscp_spec_1_7_14.pdf files\doc
copy ..\..\..\docs\vscp\drupal-vscp.pdf files\doc
copy ..\..\..\docs\canal\canal_specification_1_11.pdf files\doc

REM Copy work files
copy ..\..\..\docs\vscp\vscp_specification\vscp_spec_1_7_14.pdf files\work

REm wx dll's
copy \wxWidgets-2.8.12\lib\vc_dll\*.dll files\wx

REm support dll's
copy ..\contrib\libwebsocket\dll\win32\libwebsocketswin32.dll files\wx
