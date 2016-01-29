#!/bin/sh
# This file is used to make duplicates of critical files for the 
# vscp_firmware repository

cp src/vscp/common/canal.h ../vscp_firmware/common
cp src/vscp/common/canal.h ../vscp_install_win32/files/include

cp src/vscp/common/vscp.h ../vscp_firmware/common
cp src/vscp/common/vscp.h ../vscp_install_win32/files/include

cp src/vscp/common/vscp_class.h ../vscp_firmware/common
cp src/vscp/common/vscp_class.h ../vscp_install_win32/files/include

cp src/vscp/common/vscp_type.h ../vscp_firmware/common
cp src/vscp/common/vscp_type.h ../vscp_install_win32/files/include

cp src/vscp/common/vscp_serial.h ../vscp_firmware/common
cp src/vscp/common/vscp_serial.h ../vscp_install_win32/files/include

cp src/vscp/common/canal_macro.h ../vscp_install_win32/files/include
cp src/vscp/common/canaldlldef.h ../vscp_install_win32/files/include
cp src/vscp/helperlib/vscphelperlib.h ../vscp_install_win32/files/include

