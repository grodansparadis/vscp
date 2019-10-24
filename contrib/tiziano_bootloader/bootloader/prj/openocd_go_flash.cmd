@echo off
call openocd_install_info.cmd

:LAB_PP
set OOCD_EXE=%OOCD_BIN_PP%
set OOCD_CFG=openocd_flash_lpc2378_pp.cfg

%OOCD_EXE% %OOCD_DBG% -f %OOCD_CFG%

:LAB_END

rem pause

