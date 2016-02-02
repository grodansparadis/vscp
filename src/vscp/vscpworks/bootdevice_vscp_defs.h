// Copyright: (C) 2007-2016 
// Ake Hedman, Paradise of the Frog AB, <akhe@paradiseofthefrog.com>
#define MEMREG_PRG_START_COMMON         0x000000
#define MEMREG_PRG_END_COMMON           0x2fffff

#define MEMREG_CONFIG_START_COMMON      0x300000
#define MEMREG_CONFIG_END_COMMON        0x3fffff

#define MEMREG_EEPROM_START_COMMON      0xf00000
#define MEMREG_EEPROM_END_COMMON        0xffffff

#define BUFFER_SIZE_PROGRAM_COMMON      0x20000
#define BUFFER_SIZE_CONFIG_COMMON       0x2000
#define BUFFER_SIZE_EEPROM_COMMON       0x1000


#define INTEL_LINETYPE_DATA             0   // Data record.
#define INTEL_LINETYPE_EOF              1   // End Of File record.
#define INTEL_LINETYPE_EXTENDED_SEGMENT 2   // Extended segment address	record.
#define INTEL_LINETYPE_RESERVED         3   // Not used
#define INTEL_LINETYPE_EXTENDED_LINEAR  4   // Extended linear address record.	

#ifndef MAX_PATH
#define MAX_PATH                        512
#endif

// Timeout for response
#define BOOT_COMMAND_RESPONSE_TIMEOUT   5

// Memory Type
#define	MEM_TYPE_PROGRAM                0x00
#define MEM_TYPE_CONFIG                 0x01
#define MEM_TYPE_EEPROM                 0x02

#define VSCP_ENTER_BOOTLODER_MODE       0x0C  // VSCP_TYPE_PROTOCOL_ENTER_BOOT_LOADER  = 12

#define VSCP_PRIORITY_LOW_COMMON        0x07

// Used VSCP registers
#define VSCP_REG_PAGE_SELECT_MSB        0x92
#define VSCP_REG_PAGE_SELECT_LSB        0x93
#define VSCP_REG_GUID0                  0xD0
#define VSCP_REG_GUID3                  0xD3
#define VSCP_REG_GUID5                  0xD5
#define VSCP_REG_GUID7                  0xD7
