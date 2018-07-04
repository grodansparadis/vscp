#!/bin/sh
# This file is used to make duplicates of critical files for the 
# vscp_firmware repository

cp src/vscp/common/canal.h ../vscp-firmware/common
cp src/vscp/common/canal.h ../vscp-install-windows/files/include

cp src/vscp/common/vscp.h ../vscp-firmware/common
cp src/vscp/common/vscp.h ../vscp-install-windows/files/include

cp src/vscp/common/vscp_class.h ../vscp-firmware/common
cp src/vscp/common/vscp_class.h ../vscp-install-windows/files/include

cp src/vscp/common/vscp_type.h ../vscp-firmware/common
cp src/vscp/common/vscp_type.h ../vscp-install-windows/files/include

cp src/vscp/common/vscp_serial.h ../vscp-firmware/common
cp src/vscp/common/vscp_serial.h ../vscp-install-windows/files/include

cp src/vscp/common/canal_macro.h ../vscp-install-windows/files/include
cp src/vscp/common/canaldlldef.h ../vscp-install-windows/files/include
cp src/vscp/helperlib/vscphelperlib.h ../vscp-install-windows/files/include

