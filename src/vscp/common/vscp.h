/*
 FILE: vscp.h

 This file is part of the VSCP (https://www.vscp.org)

 The MIT License (MIT)

 Copyright © 2000-2022 Ake Hedman, the VSCP project
 <info@vscp.org>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

            !!!!!!!!!!!!!!!!!!!!  W A R N I N G  !!!!!!!!!!!!!!!!!!!!
 This file may be a copy of the original file. This is because the file is
 copied to other projects as a convenience. Thus editing the copy will not make
 it to the original and will be overwritten.
 The original file can be found in the vscp_softare source tree under
 src/vscp/common
*/

#ifndef _VSCP_H_
#define _VSCP_H_

#include <canal.h>
#include <inttypes.h>
#include <vscp_class.h>
#include <vscp_type.h>

#define VSCP_DEFAULT_UDP_PORT           33333
#define VSCP_DEFAULT_TCP_PORT           9598
#define VSCP_ANNOUNCE_MULTICAST_PORT    9598
#define VSCP_DEFAULT_MULTICAST_PORT     44444

#define VSCP_ADDRESS_SEGMENT_CONTROLLER 0x00
#define VSCP_ADDRESS_NEW_NODE           0xff

#define VSCP_MAX_DATA  (512) /* was 487 */
#define VSCP_SIZE_GUID (16)  /* GUID array size */

#define VSCP_LEVEL1 0 /* Changed 151104  Was 1/2 */
#define VSCP_LEVEL2 1

#ifdef __cplusplus
extern "C"
{
#endif

    /*          * * * General structure for VSCP * * *   */

    /* This structure is for VSCP Level II   */

    typedef struct
    {
        uint16_t crc; /* crc checksum (calculated from here to end) */
                      /* Used for UDP/Ethernet etc */

        uint32_t obid; /* Used by driver for channel info etc. */

        /* Time block - Always UTC time */
        uint16_t year;
        uint8_t month;  /* 1-12 */
        uint8_t day;    /* 1-31 */
        uint8_t hour;   /* 0-23 */
        uint8_t minute; /* 0-59 */
        uint8_t second; /* 0-59 */

        uint32_t
          timestamp; /* Relative time stamp for package in microseconds */
                     /* ~71 minutes before roll over */

        /* ----- CRC should be calculated from here to end + data block ----  */

        /*
            Bit 15 - This is a dumb node. No MDF, register, nothing.
            Bit 14 - GUID type
            Bit 13 - GUID type
            Bit 12 - GUID type (GUID is IP v.6 address.)
            Bit 8-11 = Reserved
            Bit 765 =  priority, Priority 0-7 where 0 is highest.
            Bit 4 = hard coded, true for a hard coded device.
            Bit 3 = Don't calculate CRC, false for CRC usage.
                  Just checked when CRC is used.
                  If set the CRC should be set to 0xAA55 for
                  the event to be accepted without a CRC check.
            Bit 2 = Rolling index.
            Bit 1 = Rolling index.
            Bit 0 = Rolling index.
        */
        uint16_t head;

        uint16_t vscp_class; /* VSCP class */
        uint16_t vscp_type;  /* VSCP type */
        uint8_t GUID[16];    /* Node globally unique id MSB(0) -> LSB(15) */
        uint16_t sizeData;   /* Number of valid data bytes */

        uint8_t* pdata; /* Pointer to data. Max 512 bytes */

    } vscpEvent;

    typedef vscpEvent* PVSCPEVENT;

    /*
        WARNING!!!
        This structure is for VSCP Level II with data embedded == big!!!
     */

    typedef struct
    {

        uint16_t crc; /* CRC checksum (calculated from here to end) */
                      /* Used for UDP/Ethernet etc */

        uint32_t obid; /* Used by driver for channel info etc. */

        /* Time block - Always UTC time */
        uint16_t year;
        uint8_t month;  /* 1-12 */
        uint8_t day;    /* 1-31 */
        uint8_t hour;   /* 0-23 */
        uint8_t minute; /* 0-59 */
        uint8_t second; /* 0-59 */

        uint32_t
          timestamp; /* Relative time stamp for package in microseconds. */
                     /* ~71 minutes before roll over */

        /* CRC should be calculated from here to end + data block */
        uint16_t head; /* Bit 15   GUID is IP v.6 address. */
        /*
            Bit 15 - This is a dumb node. No MDF, register, nothing.
            Bit 14 - GUID type
            Bit 13 - GUID type
            Bit 12 - GUID type (GUID is IP v.6 address.)
            Bit 8-11 = Reserved
            Bit 765 =  priority, Priority 0-7 where 0 is highest.
            Bit 4 = hard coded, true for a hard coded device.
            Bit 3 = Don't calculate CRC, false for CRC usage.
                  Just checked when CRC is used.
                  If set the CRC should be set to 0xAA55 for
                  the event to be accepted without a CRC check.
            Bit 2 = Rolling index.
            Bit 1 = Rolling index.
            Bit 0 = Rolling index.
        */
        uint16_t vscp_class; /* VSCP class   */
        uint16_t vscp_type;  /* VSCP type    */
        uint8_t GUID[16];    /* Node globally unique id MSB(0) -> LSB(15)    */
        uint16_t sizeData;   /* Number of valid data bytes   */

        uint8_t data[VSCP_MAX_DATA]; /* Pointer to data. Max. 512 bytes     */

    } vscpEventEx;

    typedef vscpEventEx* PVSCPEVENTEX;

/* Priorities in the header byte as or'in values */
/* Priorities goes from 0-7 where 0 is highest   */
#define VSCP_PRIORITY_0 0x00
#define VSCP_PRIORITY_1 0x20
#define VSCP_PRIORITY_2 0x40
#define VSCP_PRIORITY_3 0x60
#define VSCP_PRIORITY_4 0x80
#define VSCP_PRIORITY_5 0xA0
#define VSCP_PRIORITY_6 0xC0
#define VSCP_PRIORITY_7 0xE0

#define VSCP_PRIORITY_HIGH   0x00
#define VSCP_PRIORITY_LOW    0xE0
#define VSCP_PRIORITY_MEDIUM 0xC0
#define VSCP_PRIORITY_NORMAL 0x60

#define VSCP_HEADER_PRIORITY_MASK 0xE0

#define VSCP_HEADER_HARD_CODED 0x10 /* If set node nickname is hardcoded */
#define VSCP_HEADER_NO_CRC     0x08 /* Don't calculate CRC */

#define VSCP_NO_CRC_CALC 0x08 /* If set no CRC is calculated */

#define VSCP_HEADER16_DUMB      0x8000 /* This node is dumb */
#define VSCP_HEADER16_IPV6_GUID 0x1000 /* GUID is IPv6 address */

/* Bits 14/13/12 for GUID type */
#define VSCP_HEADER16_GUID_TYPE_STANDARD 0x0000 /* VSCP standard GUID */
#define VSCP_HEADER16_GUID_TYPE_IPV6     0x1000 /* GUID is IPv6 address */
/* https://www.sohamkamani.com/blog/2016/10/05/uuid1-vs-uuid4/ */
#define VSCP_HEADER16_GUID_TYPE_RFC4122V1                                      \
    0x2000 /* GUID is RFC 4122 Version 1 */
#define VSCP_HEADER16_GUID_TYPE_RFC4122V4                                      \
    0x3000 /* GUID is RFC 4122 Version 4 */

#define VSCP_MASK_PRIORITY  0xE0
#define VSCP_MASK_GUID_TYPE 0x8000 
#define VSCP_MASK_HARDCODED 0x10
#define VSCP_MASK_NOCRCCALC 0x08

#define VSCP_LEVEL1_MAXDATA 8
#define VSCP_LEVEL2_MAXDATA (512)

#define VSCP_NOCRC_CALC_DUMMY_CRC                                              \
    0xAA55 /* If no CRC cal bit is set the CRC value */
           /* should be set to this value for the CRC  */
           /* calculation to be skipped. */

#define VSCP_CAN_ID_HARD_CODED 0x02000000 /* Hard coded bit in CAN frame id */

/* GUID byte positions */
#define VSCP_GUID_MSB 0
#define VSCP_GUID_LSB 15

    /*
     * Filter structure for VSCP Level 2
     */

    typedef struct
    {
        uint8_t filter_priority; /* Priority  */
        uint8_t mask_priority;

        uint16_t filter_class; /* VSCP class */
        uint16_t mask_class;

        uint16_t filter_type; /* VSCP type */
        uint16_t mask_type;

        uint8_t filter_GUID[16]; /* Node address MSB -> LSB, LSB is node
                                    nickname id */
        uint8_t mask_GUID[16];   /* when interfacing the VSCP daemon. */

    } vscpEventFilter;

    typedef vscpEventFilter* PVSCPEVENTFILTER;

    /*
        Statistics

        This is the general statistics structure
    */

    typedef struct structVSCPStatistics
    {
        unsigned long cntReceiveFrames;  /* # of receive frames */
        unsigned long cntTransmitFrames; /* # of transmitted frames */
        unsigned long cntReceiveData;    /* # of received data bytes */
        unsigned long cntTransmitData;   /* # of transmitted data bytes */
        unsigned long cntOverruns;       /* # of overruns */
        unsigned long x;                 /* Currently undefined value */
        unsigned long y;                 /* Currently undefined value */
        unsigned long z;                 /* Currently undefined value */
    } VSCPStatistics;

    typedef VSCPStatistics* PVSCPSTATISTICS;

    /*
        VSCPStatus

        This is the general channel state structure
    */

#define VSCP_STATUS_ERROR_STRING_SIZE 80

    typedef struct structVSCPStatus
    {
        unsigned long channel_status;   /* Current state for channel */
        unsigned long lasterrorcode;    /* Last error code */
        unsigned long lasterrorsubcode; /* Last error sub code */
        char
          lasterrorstr[VSCP_STATUS_ERROR_STRING_SIZE]; /* Last error string */
    } VSCPStatus;

    typedef VSCPStatus* PVSCPSTATUS;

    /* VSCP LEVEL II UDP datagram offsets     */
    /*  Same format as multicast i used below */

    /*
        VSCP Channel Info

        This is the channel VSCP Info structure
    */

    typedef struct structVSCPChannelInfo
    {
        unsigned char channelType; /* Level I, Level II etc. from canal.h  */
        unsigned short channel;    /* daemon channel number */
        char GUID[16];             /* Channel GUID id */

    } VSCPChannelInfo;

    typedef VSCPChannelInfo* PVSCPCHANNELINFO;
    
/* Interface types */ 
#define VSCP_INTERFACE_TYPE_UNKNOWN		0
#define VSCP_INTERFACE_TYPE_INTERNAL		1
#define VSCP_INTERFACE_TYPE_LEVEL1DRV		2
#define VSCP_INTERFACE_TYPE_LEVEL2DRV		3
#define VSCP_INTERFACE_TYPE_CLIENT_TCPIP	4
#define VSCP_INTERFACE_TYPE_CLIENT_UDP		5
#define VSCP_INTERFACE_TYPE_CLIENT_WEB		6
#define VSCP_INTERFACE_TYPE_CLIENT_WEBSOCK	7
#define VSCP_INTERFACE_TYPE_CLIENT_REST		8


/* VSCP Encryption types */
#define VSCP_ENCRYPTION_NONE           0
#define VSCP_ENCRYPTION_AES128         1
#define VSCP_ENCRYPTION_AES192         2
#define VSCP_ENCRYPTION_AES256         3
#define VSCP_ENCRYPTION_FROM_TYPE_BYTE 15

/* VSCP Encryption tokens */
#define VSCP_ENCRYPTION_TOKEN_0 ""
#define VSCP_ENCRYPTION_TOKEN_1 "AES128"
#define VSCP_ENCRYPTION_TOKEN_2 "AES192"
#define VSCP_ENCRYPTION_TOKEN_3 "AES256"

    /* * * * Multicast on VSCP reserved IP 224.0.23.158 */

#define VSCP_MULTICAST_IPV4_ADDRESS_STR "224.0.23.158"

/* Packet frame format type = 0                         */
/*      without byte0 and CRC                           */
/*      total frame size is 1 + 34 + 2 + data-length    */
#define VSCP_MULTICAST_PACKET0_HEADER_LENGTH      35

/* Multicast packet ordinals */
#define VSCP_MULTICAST_PACKET0_POS_PKTTYPE        0
#define VSCP_MULTICAST_PACKET0_POS_HEAD           1
#define VSCP_MULTICAST_PACKET0_POS_HEAD_MSB       1
#define VSCP_MULTICAST_PACKET0_POS_HEAD_LSB       2
#define VSCP_MULTICAST_PACKET0_POS_TIMESTAMP      3
#define VSCP_MULTICAST_PACKET0_POS_YEAR           7
#define VSCP_MULTICAST_PACKET0_POS_YEAR_MSB       7
#define VSCP_MULTICAST_PACKET0_POS_YEAR_LSB       8
#define VSCP_MULTICAST_PACKET0_POS_MONTH          9
#define VSCP_MULTICAST_PACKET0_POS_DAY            10
#define VSCP_MULTICAST_PACKET0_POS_HOUR           11
#define VSCP_MULTICAST_PACKET0_POS_MINUTE         12
#define VSCP_MULTICAST_PACKET0_POS_SECOND         13
#define VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS     14
#define VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_MSB 14
#define VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_LSB 15
#define VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE      16
#define VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_MSB  16
#define VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_LSB  17
#define VSCP_MULTICAST_PACKET0_POS_VSCP_GUID      18
#define VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE      34
#define VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_MSB  34
#define VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_LSB  35
#define VSCP_MULTICAST_PACKET0_POS_VSCP_DATA      36

/* Two byte CRC follow here and if the frame is encrypted */
/* the initialization vector follows. */

// Maximum packet size (for buffer allocation)
#define VSCP_MULTICAST_PACKET0_MAX  (1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + VSCP_LEVEL2_MAXDATA + 16)

/* VSCP multicast packet types */
#define VSCP_MULTICAST_TYPE_EVENT 0

#define SET_VSCP_MULTICAST_TYPE(type, encryption)  ((type << 4) | encryption)
#define GET_VSCP_MULTICAST_PACKET_TYPE(type)       ((type >> 4) & 0x0f)
#define GET_VSCP_MULTICAST_PACKET_ENCRYPTION(type) ((type)&0x0f)

/* Multicast proxy CLASS=1026, TYPE=3  */
/* https://www.vscp.org/docs/vscpspec/doku.php?id=class2.information#type_3_0x0003_level_ii_proxy_node_heartbeat
 */
#define VSCP_MULTICAST_PROXY_HEARTBEAT_DATA_SIZE 192
#define VSCP_MULTICAST_PROXY_HEARTBEAT_POS_REALGUID                            \
    0 /* The real GUID for the node */
#define VSCP_MULTICAST_PROXY_HEARTBEAT_POS_IFGUID                              \
    32 /* GUID for interface node is on */
#define VSCP_MULTICAST_PROXY_HEARTBEAT_POS_IFLEVEL                             \
    48 /* 0=Level I node, 1=Level II node */
#define VSCP_MULTICAST_PROXY_HEARTBEAT_POS_NODENAME 64  /* Name of node */
#define VSCP_MULTICAST_PROXY_HEARTBEAT_POS_IFNAME   128 /* Name of interface */

/* Default key for VSCP Server - !!!! should only be used on test systems !!!!
 */
#define VSCP_DEFAULT_KEY16 = 'A4A86F7D7E119BA3F0CD06881E371B98'
#define VSCP_DEFAULT_KEY24 = 'A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B6'
#define VSCP_DEFAULT_KEY32                                                     \
    = 'A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E'

/* Bootloaders */
#define VSCP_BOOTLOADER_VSCP      0x00 /* VSCP boot loader algorithm */
#define VSCP_BOOTLOADER_PIC1      0x01 /* PIC algorithm 0 */
#define VSCP_BOOTLOADER_AVR1      0x10 /* AVR algorithm 0 */
#define VSCP_BOOTLOADER_LPC1      0x20 /* NXP/Philips/Freescale algorithm 0 */
#define VSPP_BOOTLOADER_NXP1      0x20 /* NXP/Philips/Freescale algorithm 0 */
#define VSCP_BOOTLOADER_ST        0x30 /* ST STR algorithm 0 */
#define VSCP_BOOTLOADER_FREESCALE 0x40 /* Freescale Kinetics algorithm 0 */
#define VSCP_BOOTLOADER_USER0     0xf0 /* Used defined bootloader 0 */
#define VSCP_BOOTLOADER_NONE1     0xf1 /* Used defined bootloader 1 */
#define VSCP_BOOTLOADER_NONE2     0xf2 /* Used defined bootloader 2 */
#define VSCP_BOOTLOADER_NONE3     0xf3 /* Used defined bootloader 3 */
#define VSCP_BOOTLOADER_NONE4     0xf4 /* Used defined bootloader 4 */
#define VSCP_BOOTLOADER_NONE5     0xf5 /* Used defined bootloader 5 */
#define VSCP_BOOTLOADER_NONE6     0xf6 /* Used defined bootloader 6 */
#define VSCP_BOOTLOADER_NONE7     0xf7 /* Used defined bootloader 7 */
#define VSCP_BOOTLOADER_NONE8     0xf8 /* Used defined bootloader 8 */
#define VSCP_BOOTLOADER_NONE9     0xf9 /* Used defined bootloader 9 */
#define VSCP_BOOTLOADER_NONE10    0xfa /* Used defined bootloader 10 */
#define VSCP_BOOTLOADER_NONE11    0xfb /* Used defined bootloader 11 */
#define VSCP_BOOTLOADER_NONE12    0xfc /* Used defined bootloader 12 */
#define VSCP_BOOTLOADER_NONE13    0xfd /* Used defined bootloader 13 */
#define VSCP_BOOTLOADER_NONE14    0xfe /* Used defined bootloader 14 */
#define VSCP_BOOTLOADER_NONE15    0xff /* No bootloader available */

/*          * * * Data Coding for VSCP packets * * *   */

/* Data format masks */
#define VSCP_MASK_DATACODING_TYPE  0xE0 /* Bits 5,6,7 */
#define VSCP_MASK_DATACODING_UNIT  0x18 /* Bits 3,4   */
#define VSCP_MASK_DATACODING_INDEX 0x07 /* Bits 0,1,2 */

/* These bits are coded in the three MSB bits of the first data byte   */
/* of measurement data and tells the type of the data that follows.    */
#define VSCP_DATACODING_BIT        0x00
#define VSCP_DATACODING_BYTE       0x20
#define VSCP_DATACODING_STRING     0x40
#define VSCP_DATACODING_INTEGER    0x60
#define VSCP_DATACODING_NORMALIZED 0x80
#define VSCP_DATACODING_SINGLE     0xA0 /* single precision float */
#define VSCP_DATACODING_DOUBLE     0xC0 /* double precision float */
#define VSCP_DATACODING_RESERVED2  0xE0

/*
 These bits are coded in the four least significant bits of the first data byte
 in a packet and tells how the following data should be interpreted. For a flow
 sensor the default format can be litres/minute. Other formats such as m3/second
 can be defined by the node if it which. However it must always be able to
 report in the default format.
 */
#define VSCP_DATACODING_INTERPRETION_DEFAULT 0

/* Get data coding type */
#define VSCP_DATACODING_TYPE(b) (VSCP_MASK_DATACODING_TYPE & b)

/* Get data coding unit */
#define VSCP_DATACODING_UNIT(b) ((VSCP_MASK_DATACODING_UNIT & b) >> 3)

/* Get data coding sensor index */
#define VSCP_DATACODING_INDEX(b) (VSCP_MASK_DATACODING_INDEX & b)

/* CRC8 Constants */
#define VSCP_CRC8_POLYNOMIAL 0x18
#define VSCP_CRC8_REMINDER   0x00

/* CRC16 Constants */
#define VSCP_CRC16_POLYNOMIAL 0x1021
#define VSCP_CRC16_REMINDER   0xFFFF

/* CRC32 Constants */
#define VSCP_CRC32_POLYNOMIAL 0x04C11DB7
#define VSCP_CRC32_REMINDER   0xFFFFFFFF

    /* Node data - the required registers are fetched from this */
    /*	structure                                               */
    struct vscpMyNode
    {
        unsigned char GUID[16];
        unsigned char nicknameID;
    };

/* * * * Standard VSCP registers * * *  */

/* Register defines above 0x7f (Level I) / 0xffffff7f (Level II)  */
#define VSCP_STD_REGISTER_ALARM_STATUS 0x80

#define VSCP_STD_REGISTER_MAJOR_VERSION 0x81
#define VSCP_STD_REGISTER_MINOR_VERSION 0x82
#define VSCP_STD_REGISTER_SUB_VERSION   0x83

/* 0x84 - 0x88  -   User id space           */
#define VSCP_STD_REGISTER_USER_ID 0x84

/* 0x89 - 0x8C  -   Manufacturer id space   */
#define VSCP_STD_REGISTER_USER_MANDEV_ID 0x89

/* 0x8D -0x90   -                           */
#define VSCP_STD_REGISTER_USER_MANSUBDEV_ID 0x8D

/* Nickname id                              */
#define VSCP_STD_REGISTER_NICKNAME_ID 0x91

/* Selected page                            */
#define VSCP_STD_REGISTER_PAGE_SELECT_MSB 0x92
#define VSCP_STD_REGISTER_PAGE_SELECT_LSB 0x93

/* Firmware version                         */
#define VSCP_STD_REGISTER_FIRMWARE_MAJOR    0x94
#define VSCP_STD_REGISTER_FIRMWARE_MINOR    0x95
#define VSCP_STD_REGISTER_FIRMWARE_SUBMINOR 0x96

#define VSCP_STD_REGISTER_BOOT_LOADER 0x97
#define VSCP_STD_REGISTER_BUFFER_SIZE 0x98
#define VSCP_STD_REGISTER_PAGES_COUNT 0x99

/* Unsigned 32-bit integer for family code */
#define VSCP_STD_REGISTER_PAGES_FAMILY_CODE 0x9A

/* Unsigned 32-bit integer for device type */
#define VSCP_STD_REGISTER_PAGES_DEVICE_TYPE 0x9E

/* Firmware code for device. */
#define VSCP_STD_REGISTER_PAGES_FIRMWARE_CODE_MSB 0xA3 
#define VSCP_STD_REGISTER_PAGES_FIRMWARE_CODE_LSB 0xA4

/* 0xd0 - 0xdf  - GUID                      */
#define VSCP_STD_REGISTER_GUID 0xD0

/* 0xe0 - 0xff  - MDF                       */
#define VSCP_STD_REGISTER_DEVICE_URL 0xE0

/* Level I Decision Matrix                  */
#define VSCP_LEVEL1_DM_ROW_SIZE 8

#define VSCP_LEVEL1_DM_OFFSET_OADDR        0
#define VSCP_LEVEL1_DM_OFFSET_FLAGS        1
#define VSCP_LEVEL1_DM_OFFSET_CLASS_MASK   2
#define VSCP_LEVEL1_DM_OFFSET_CLASS_FILTER 3
#define VSCP_LEVEL1_DM_OFFSET_TYPE_MASK    4
#define VSCP_LEVEL1_DM_OFFSET_TYPE_FILTER  5
#define VSCP_LEVEL1_DM_OFFSET_ACTION       6
#define VSCP_LEVEL1_DM_OFFSET_ACTION_PARAM 7

/* Flag bits for level 1 DM flags */
#define VSCP_LEVEL1_DM_FLAG_ENABLED           0x80
#define VSCP_LEVEL1_DM_FLAG_ORIGIN_MATCH      0x40
#define VSCP_LEVEL1_DM_FLAG_ORIGIN_HARDCODED  0x20
#define VSCP_LEVEL1_DM_FLAG_MATCH_ZONE        0x10
#define VSCP_LEVEL1_DM_FLAG_MATCH_SUBZONE     0x08
#define VSCP_LEVEL1_DM_FLAG_CLASS_MASK_BIT8   0x02
#define VSCP_LEVEL1_DM_FLAG_CLASS_FILTER_BIT8 0x01

    /*
        Bits for VSCP server 64/16-bit capability code 
        (WCYD - What Can You Do) used by CLASS1.PROTOCOL, 
        HIGH END SERVER RESPONSE and low end 16-bits for
        CLASS2.PROTOCOL, HIGH END SERVER HEART BEAT
    */

#define VSCP_SERVER_CAPABILITY_REMOTE_VARIABLE    (1ll << 63) //
#define VSCP_SERVER_CAPABILITY_DECISION_MATRIX    (1ll << 62) //
#define VSCP_SERVER_CAPABILITY_INTERFACE          (1ll << 61) //
#define VSCP_SERVER_CAPABILITY_TCPIP              (1ll << 15) // 32768
#define VSCP_SERVER_CAPABILITY_UDP                (1ll << 14) // 16384
#define VSCP_SERVER_CAPABILITY_MULTICAST_ANNOUNCE (1ll << 13) // 8192
#define VSCP_SERVER_CAPABILITY_RAWETH             (1ll << 12) // 4196
#define VSCP_SERVER_CAPABILITY_WEB                (1ll << 11) // 2048
#define VSCP_SERVER_CAPABILITY_WEBSOCKET          (1ll << 10) // 1024
#define VSCP_SERVER_CAPABILITY_REST               (1ll << 9)  // 512
#define VSCP_SERVER_CAPABILITY_MULTICAST_CHANNEL  (1ll << 8)  // 256
#define VSCP_SERVER_CAPABILITY_RESERVED           (1ll << 7)  // 128
#define VSCP_SERVER_CAPABILITY_IP6                (1ll << 6)  // 64
#define VSCP_SERVER_CAPABILITY_IP4                (1ll << 5)  // 32
#define VSCP_SERVER_CAPABILITY_SSL                (1ll << 4)  // 16
#define VSCP_SERVER_CAPABILITY_TWO_CONNECTIONS    (1ll << 3)  // 8
#define VSCP_SERVER_CAPABILITY_AES256             (1ll << 2)  // 4
#define VSCP_SERVER_CAPABILITY_AES192             (1ll << 1)  // 2
#define VSCP_SERVER_CAPABILITY_AES128             1ll         // 1
#define VSCP_SERVER_CAPABILITY_NONE               0         // No capabilities

/*
    Offsets into the data of the capabilities event
    VSCP_CLASS2_PROTOCOL, Type=20/VSCP2_TYPE_PROTOCOL_HIGH_END_SERVER_CAPS
*/
#define VSCP_CAPABILITY_OFFSET_CAP_ARRAY     0
#define VSCP_CAPABILITY_OFFSET_GUID          8
#define VSCP_CAPABILITY_OFFSET_IP_ADDR       24
#define VSCP_CAPABILITY_OFFSET_SRV_NAME      40
#define VSCP_CAPABILITY_OFFSET_NON_STD_PORTS 104

/* Error Codes */
#define VSCP_ERROR_SUCCESS           0  /* All is OK */
#define VSCP_ERROR_ERROR             -1 /* Error */
#define VSCP_ERROR_CHANNEL           7  /* Invalid channel */
#define VSCP_ERROR_FIFO_EMPTY        8  /* FIFO is empty */
#define VSCP_ERROR_FIFO_FULL         9  /* FIFO is full */
#define VSCP_ERROR_FIFO_SIZE         10 /* FIFO size error */
#define VSCP_ERROR_FIFO_WAIT         11 /* FIFO wait failed */
#define VSCP_ERROR_GENERIC           12 /* Generic error */
#define VSCP_ERROR_HARDWARE          13 /* Hardware error */
#define VSCP_ERROR_INIT_FAIL         14 /* Initialization failed */
#define VSCP_ERROR_INIT_MISSING      15 /* No init */
#define VSCP_ERROR_INIT_READY        16 /* Failed init */
#define VSCP_ERROR_NOT_SUPPORTED     17 /* Not supported */
#define VSCP_ERROR_OVERRUN           18 /* Overrun */
#define VSCP_ERROR_RCV_EMPTY         19 /* Receive buffer empty */
#define VSCP_ERROR_REGISTER          20 /* Register value error */
#define VSCP_ERROR_TRM_FULL          21 /* Transmit buffer full */
#define VSCP_ERROR_LIBRARY           28 /* Unable to load library */
#define VSCP_ERROR_PROCADDRESS       29 /* Unable get library proc. address */
#define VSCP_ERROR_ONLY_ONE_INSTANCE 30 /* Only one instance allowed */
#define VSCP_ERROR_SUB_DRIVER        31 /* Problem with sub driver call */
#define VSCP_ERROR_TIMEOUT           32 /* Time-out */
#define VSCP_ERROR_NOT_OPEN          33 /* The device is not open. */
#define VSCP_ERROR_PARAMETER         34 /* A parameter is invalid. */
#define VSCP_ERROR_MEMORY            35 /* Memory exhausted. */
#define VSCP_ERROR_INTERNAL          36 /* Some kind of internal program error */
#define VSCP_ERROR_COMMUNICATION     37 /* Some kind of communication error */
#define VSCP_ERROR_USER              38 /* Login error user name */
#define VSCP_ERROR_PASSWORD          39 /* Login error password */
#define VSCP_ERROR_CONNECTION        40 /* Could not connect */
#define VSCP_ERROR_INVALID_HANDLE    41 /* The handle is not valid */
#define VSCP_ERROR_OPERATION_FAILED  42 /* Operation failed for some reason */
#define VSCP_ERROR_BUFFER_TO_SMALL                                             \
    43 /* Supplied buffer is to small to fit content */
#define VSCP_ERROR_UNKNOWN_ITEM                                                \
    44 /* Requested item (remote variable) is unknown */
#define VSCP_ERROR_ALREADY_DEFINED      45 /* The name is already in use. */
#define VSCP_ERROR_WRITE_ERROR          46 /* Error when writing data */
#define VSCP_ERROR_WRITE                46 /* Error when writing data */
#define VSCP_ERROR_STOPPED              47 /* Operation stopped or aborted */
#define VSCP_ERROR_INVALID_POINTER      48 /* Pointer with invalid value */
#define VSCP_ERROR_INVALID_PERMISSION   49 /* Not allowed to do that */
#define VSCP_ERROR_INVALID_PATH         50 /* Invalid path (permissions) */
#define VSCP_ERROR_ERRNO                51 /* General error, errno variable holds error */
#define VSCP_ERROR_INTERRUPTED          52 /* Interrupted by signal or other cause */
#define VSCP_ERROR_MISSING              53 /* Value, paramter or something else is missing */ 
#define VSCP_ERROR_NOT_CONNECTED        54 /* There is no connection */
#define VSCP_ERROR_READ_ONLY            55 /* Item (variable) is read only */  
#define VSCP_ERROR_INVALID_TYPE         56 /* Item (variable) is of wrong type */
#define VSCP_ERROR_PERMISSION           57 /* Does hot have permission to do that */
#define VSCP_ERROR_INVALID_SYNTAX       58 /* Syntax is invalid */
#define VSCP_ERROR_INDEX_OOB            59 /* Index is out of bounds */  
#define VSCP_ERROR_MTU                  60 /* Frame does not fit */
#define VSCP_ERROR_SOCKET               61 /* Unable to create socket or other socket error*/
#define VSCP_ERROR_PARSING              62 /* Failed to parse input */

/*!
    HLO (High Level Object) type (bits 7,6,5,4)
*/

#define VSCP_HLO_TYPE_UTF8                   0
#define VSCP_HLO_TYPE_XML                    1
#define VSCP_HLO_TYPE_JSON                   2
#define VSCP_HLO_TYPE_BASE64                 3
#define VSCP_HLO_TYPE_USER_DEFINED           15

/*!
    HLO (High Level Object) encryption (bits 3,2,1,0)
*/

#define VSCP_HLO_ENCRYPTION_NONE             0
#define VSCP_HLO_ENCRYPTION_AES128           1
#define VSCP_HLO_ENCRYPTION_AES192           2
#define VSCP_HLO_ENCRYPTION_AES256           3

/*
    Template for VSCP XML event data

    data:
datetime,vscpHead,vscpObId,vscpDateTime,vscpTimeStamp,vscpClass,vscpType,vscpGuid,vscpData,note

<event
     vscpHead="3"
     vscpObId="1234"
     vscpDateTime="2017-01-13T10:16:02"
     vscpTimeStamp="50817"
     vscpClass="10"
     vscpType="6"
     vscpGuid="00:00:00:00:00:00:00:00:00:00:00:00:00:01:00:02"
     vscpData="0x48,0x34,0x35,0x2E,0x34,0x36,0x34" />

 */
#define VSCP_XML_EVENT_TEMPLATE                                                    \
    "<event "                                                                      \
    "vscpHead=\"%d\" "                                                             \
    "vscpObId=\"%lu\" "                                                            \
    "vscpDateTime=\"%s\" "                                                         \
    "vscpTimeStamp=\"%lu\" "                                                       \
    "vscpClass=\"%d\" "                                                            \
    "vscpType=\"%d\" "                                                             \
    "vscpGuid=\"%s\" "                                                             \
    "vscpData=\"%s\" "                                                             \
    "/>"

/*

    Template for VSCP JSON event data
    data: datetime,vscpHead,vscpObId,datetime,timestamp,class,type,guid,data,note

{
    "vscpHead": 2,
    "vscpObId": 123,
    "vscpDateTime": "2017-01-13T10:16:02Z",
    "vscpTimeStamp":50817,
    "vscpClass": 10,
    "vscpType": 8,
    "vscpGuid": "00:00:00:00:00:00:00:00:00:00:00:00:00:01:00:02",
    "vscpData": [1,2,3,4,5,6,7],
    "vscpNote": "This is some text"
}
*/
#define VSCP_JSON_EVENT_TEMPLATE                                                   \
    "{\n"                                                                          \
    "\"vscpHead\": %d,\n"                                                          \
    "\"vscpObId\":  %lu,\n"                                                        \
    "\"vscpDateTime\": \"%s\",\n"                                                  \
    "\"vscpTimeStamp\": %lu,\n"                                                    \
    "\"vscpClass\": %d,\n"                                                         \
    "\"vscpType\": %d,\n"                                                          \
    "\"vscpGuid\": \"%s\",\n"                                                      \
    "\"vscpData\": [%s],\n"                                                        \
    "\"vscpNote\": \"%s\"\n"                                                       \
    "}"

/*!

    Template for VSCP HTML event data

    data: vscpDateTime,vscpClass,type,data-count,data,guid,vscpHead,vscpTimeStamp,vscpObId,note

<h2>VSCP Event</h2>
<p>
Time: 2017-01-13T10:16:02 <br>
</p>
<p>
Class: 10 <br>
Type: 6 <br>
</p>
<p>
Data count: 6<br>
Data: 1,2,3,4,5,6,7<br>
</p>
<p>
From GUID: 00:00:00:00:00:00:00:00:00:00:00:00:00:01:00:02<br>
</p>
<p>
vscpHead: 6 <br>
DateTime: 2013-11-02T12:34:22Z
Timestamp: 1234 <br>
ObId: 1234 <br>
note: This is a note <br>
</p>

*/
#define VSCP_HTML_EVENT_TEMPLATE                                               \
    "<h2>VSCP Event</h2> "                                                     \
    "<p>"                                                                      \
    "Class: %d <br>"                                                           \
    "Type: %d <br>"                                                            \
    "</p>"                                                                     \
    "<p>"                                                                      \
    "Data count: %d<br>"                                                       \
    "Data: %s<br>"                                                             \
    "</p>"                                                                     \
    "<p>"                                                                      \
    "From GUID: %s<br>"                                                        \
    "</p>"                                                                     \
    "<p>"                                                                      \
    "vscpHead: %d <br>"                                                        \
    "<p>"                                                                      \
    "DateTime: %s <br>"                                                        \
    "</p>"                                                                     \
    "Timestamp: %lu <br>"                                                      \
    "ObId: %lu <br>"                                                           \
    "note: %s <br>"                                                            \
    "</p>"

#define VSCP_XML_FILTER_TEMPLATE                                               \
    "<filter mask_priority=\"%d\" \n"                                          \
    "mask_class=\"%d\" \n"                                                     \
    "mask_type=\"%d\" \n"                                                      \
    "mask_guid=\"%s\" \n"                                                      \
    "filter_priority=\"%d\" \n"                                                \
    "filter_class=\"%d\" \n"                                                   \
    "filter_type=\"%d\" \n"                                                    \
    "filter_guid=\"%s\" />"

#define VSCP_JSON_FILTER_TEMPLATE                                              \
    "{\n"                                                                      \
    "\"mask_priority\": %d,\n"                                                 \
    "\"mask_class\": %d,\n"                                                    \
    "\"mask_type\": %d,\n"                                                     \
    "\"mask_guid\": \"%s\",\n"                                                 \
    "\"filter_priority\": %d,\n"                                               \
    "\"filter_class\": %d,\n"                                                  \
    "\"filter_type\": %d,\n"                                                   \
    "\"filter_guid\": \"%s\",\n"                                               \
    "}"

// MQTT message formats
enum enumMqttMsgFormat {jsonfmt,xmlfmt,strfmt,binfmt,autofmt};

#ifdef __cplusplus
}
#endif

#endif /* _VSCP_H_ */
