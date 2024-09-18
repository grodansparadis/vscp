#!/bin/sh
# This file is used to make duplicates of critical files for the
# vscp_firmware repository

cp src/vscp/common/canal.h ../vscp-firmware/common
cp src/vscp/common/canal.h ../vscp-install-windows/files/include

cp src/vscp/common/vscp.h ../vscp-firmware/common
cp src/vscp/common/vscp.h ../vscp-install-windows/files/include

cp src/vscp/common/vscp-class.h ../vscp-firmware/common
cp src/vscp/common/vscp-class.h ../vscp-install-windows/files/include

cp src/vscp/common/vscp-type.h ../vscp-firmware/common
cp src/vscp/common/vscp-type.h ../vscp-install-windows/files/include

cp src/vscp/common/vscp-serial.h ../vscp-firmware/common
cp src/vscp/common/vscp-serial.h ../vscp-install-windows/files/include

cp src/vscp/common/canal-macro.h ../vscp-install-windows/files/include
cp src/vscp/common/canaldlldef.h ../vscp-install-windows/files/include
cp ~/development/VSCP/vscp-helper-lib/src/vscphelperlib.h ../vscp-install-windows/files/include
