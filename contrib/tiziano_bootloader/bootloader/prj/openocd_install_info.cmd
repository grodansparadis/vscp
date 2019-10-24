rem -
rem Modify this to the installed OpenOCD-binaries
rem The following is for the precompiled binaries from Michael Fischer
rem with sources from OpenOCD SVN-version 100 installed on a
rem PC running Windows-2000 (german)
rem -
set OOCD_INSTALLDIR=C:\Programmi\openocd-r247\bin
set OOCD_BIN_PP=%OOCD_INSTALLDIR%\openocd-pp.exe

rem The used interface either FTDI(=WinARM-JTAG, JTAGKEY etc.) or PP(="Wiggler")
rem set OOCD_INTERFACE=PP