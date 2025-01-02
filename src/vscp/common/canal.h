/******************************************************************************
 canal.h

 This file is part of the VSCP (https://www.vscp.org)

 The MIT License (MIT)

 Copyright (C) 2000-2025 Ake Hedman, the VSCP project <info@vscp.org>

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
 copied to other projects as a convinience. Thus editing the copy will not make
 it to the original and will be overwritten.
 The original file can be foud in the vscp_softare source tree under
 src/vscp/common
*/

#ifndef ___CANAL_H___
#define ___CANAL_H___

/* This was needed to compile as a service - Don't know why??? */
#ifndef EXPORT
#define EXPORT
#endif

#include <stdint.h> // Deterministic integer definitions (C11)

#ifdef __linux__
#include <linux/can.h>
#elif _WIN32
#elif __APPLE__
#endif

#define CAN_MAX_STANDARD_ID 0x7ff
#define CAN_MAX_EXTENDED_ID 0x1fffffff

/*
 CANAL version
 =============
 This value is steped for each new version when
 the i/f is changed. This will hopefully not
 happen but probably will.
*/

#define CANAL_MAIN_VERSION  1
#define CANAL_MINOR_VERSION 1
#define CANAL_SUB_VERSION   0

/* Canal Levels */
#define CANAL_LEVEL_STANDARD   1 /* Level I driver           */
#define CANAL_LEVEL_NO_TCPIP   2 /* Limited Level II Driver  */
#define CANAL_LEVEL_USES_TCPIP 3 /* Full Level II Driver     */

/* VSCP daemon version information positions    */
#define POS_VSCPD_MAJOR_VERSION 1
#define POS_VSCPD_MINOR_VERSION 2
#define POS_VSCPD_SUB_VERSION   3

/* VSCP Daemon client Open types        */
#define CANAL_COMMAND_OPEN_VSCP_LEVEL1  1 /* VSCP Level I channel (CAN)   */
#define CANAL_COMMAND_OPEN_VSCP_LEVEL2  2 /* VSCP Level II channel        */
#define CANAL_COMMAND_OPEN_VSCP_CONTROL 3 /* Daemon Control channel       */

/* Interprocess success/failure codes   */
#define COMMAND_FAILURE 0
#define COMMAND_SUCCESS 1

/* Return types */

#define PACKAGE_ACK  0
#define PACKAGE_NACK 1

#define PACKAGE_TIMEOUT -1
#define PACKAGE_UNKNOWN -1

#define PACKAGE_FAILURE 0
#define PACKAGE_SUCCESS 1

#ifdef __cplusplus
extern "C" {
#endif

/* * * * Data Structures * * * */

/*
    CanalMsg

    This is the fd message structure
*/

typedef struct structCanalMsg {
  unsigned long flags;     /* CAN message flags */
  unsigned long obid;      /* Used by driver for channel info etc. */
  unsigned long id;        /* CAN id (11-bit or 29-bit) */
  unsigned char sizeData;  /* Data size 0-8 */
  unsigned char data[8];   /* CAN Data	 */
  unsigned long timestamp; /* Relative time stamp for package in microseconds */
} canalMsg;

typedef canalMsg *PCANALMSG;

/*
    CanalMsgFD

    This is the fd message structure
*/

typedef struct structCanalMsgFD {
  uint32_t flags;     /* CAN message flags */
  uint32_t obid;      /* Used by driver for channel info etc. */
  uint32_t id;        /* CAN id (11-bit or 29-bit) */
  uint32_t timestamp; /* Relative time stamp for package in microseconds */
  uint8_t _reserved[7];
  uint8_t sizeData;                             /* Data size 0-8 (fd: 0-15) */
  uint8_t data[64] /*__attribute__((aligned(8)))*/; /* CAN Data	 */
} canalMsgFD;

typedef canalMsgFD *PCANALMSGFD;

/*
    CanalStatistics

    This is the general statistics structure
*/

typedef struct structCanalStatistics {
  unsigned long cntReceiveFrames;  /* # of receive frames */
  unsigned long cntTransmitFrames; /* # of transmitted frames */
  unsigned long cntReceiveData;    /* # of received data bytes */
  unsigned long cntTransmitData;   /* # of transmitted data bytes */
  unsigned long cntOverruns;       /* # of overruns */
  unsigned long cntBusWarnings;    /* # of bys warnings */
  unsigned long cntBusOff;         /* # of bus off's */
} canalStatistics;

typedef canalStatistics *PCANALSTATISTICS;

/*
    CanalStatus

    This is the general channel state structure
*/

typedef struct structCanalStatus {
  unsigned long channel_status;   /* Current state for channel */
  unsigned long lasterrorcode;    /* Last error code */
  unsigned long lasterrorsubcode; /* Last error sub code */
  char lasterrorstr[80];          /* Last error string */
} canalStatus;

typedef canalStatus *PCANALSTATUS;

/* This is the define for the received callback method */
#ifdef WIN32
typedef void(__stdcall *LPFNDLL_CANAL_RECEIVE_CALLBACK)(canalMsg *pMsg);
typedef void(__stdcall *LPFNDLL_CANAL_RECEIVE_FD_CALLBACK)(canalMsgFD *pMsg);
#endif

/* CAN driver open handle */
typedef long CANALHANDLE;

/*    * * * Prototypes * * *   */

/*!
    Open a CANAL device and get a handle to it.

    flags - Give extra info to the CANAL i/F

  @param  pDevice Physical device to connect to.
  @return Handle of device or -1 if error.
*/
#ifdef WIN32
long EXPORT
CanalOpen(const char *pDevice, unsigned long flags);
#else
long
CanalOpen(const char *pDevice, unsigned long flags);
#endif

/*!
    Close a CANAL channel.

    @param handle - Handle to open physical CANAL channel.
    @return zero on success or error-code on failure.
*/
#ifdef WIN32
int EXPORT
CanalClose(CANALHANDLE handle);
#else
int
CanalClose(CANALHANDLE handle);
#endif

/*!
    Get CANAL DLL supported level

    @return level for CANAL dll implementation. (deprecated)
*/
#ifdef WIN32
unsigned long EXPORT
CanalGetLevel(CANALHANDLE handle);
#else
unsigned long
CanalGetLevel(CANALHANDLE handle);
#endif

/*!
    Send a message on a CANAL channel.

    The instruction should block if not instructed not to
    do so when the interface was opened.

    @param handle - Handle to open physical CANAL channel.
    @param pCanMsg - Message to send.
    @return zero on success or error-code on failure.
*/
#ifdef WIN32
int EXPORT
CanalSend(CANALHANDLE handle, const PCANALMSG pCanalMsg);
#else
int
CanalSend(CANALHANDLE handle, PCANALMSG pCanalMsg);
#endif

/*!
    Send a FD message on a CANAL channel.

    The instruction should block if not instructed not to
    do so when the interface was opened.

    @param handle - Handle to open physical CANAL channel.
    @param pCanalMsgFD - Message to send.
    @return zero on success or error-code on failure.
*/
#ifdef WIN32
int EXPORT
CanalSendFD(CANALHANDLE handle, const PCANALMSGFD pCanalMsgFD);
#else
int
CanalSendFD(CANALHANDLE handle, PCANALMSG pCanalMsg);
#endif

/*!
    Send a blocking message on a CANAL channel.

    The instruction should block if not instructed not to
    do so when the interface was opened.

    @param handle - Handle to open physical CANAL channel.
    @param pCanMsg - Message to send.
    @param timeout - time-out in ms. 0 is forever.
    @return zero on success or error-code on failure.
*/
#ifdef WIN32
int EXPORT
CanalBlockingSend(CANALHANDLE handle, PCANALMSG pCanalMsg, unsigned long timeout);
#else
int
CanalBlockingSend(CANALHANDLE handle, PCANALMSG pCanalMsg, unsigned long timeout);
#endif

/*!
    Send a blocking fd message on a CANAL channel.

    The instruction should block if not instructed not to
    do so when the interface was opened.

    @param handle - Handle to open physical CANAL channel.
    @param pCanMsgFD - Message to send.
    @param timeout - time-out in ms. 0 is forever.
    @return zero on success or error-code on failure.
*/
#ifdef WIN32
int EXPORT
CanalBlockingSendFD(CANALHANDLE handle, PCANALMSGFD pCanalMsgFD, unsigned long timeout);
#else
int
CanalBlockingSendFD(CANALHANDLE handle, PCANALMSG pCanalMsg, unsigned long timeout);
#endif

/*!
    Receieve a message on a CANAL channel.

    The instruction should block if not instructed not to
    do so when the interface was opened.

        :!: Note that if bit 2 in flags of the CANAL message is set and error status
        message is returned from the adapter.

    @param handle - Handle to open physical CANAL channel.
    @param pCanMsg - Message to send.
    @return zero on success or error-code on failure.
*/
#ifdef WIN32
int EXPORT
CanalReceive(CANALHANDLE handle, PCANALMSG pCanalMsg);
#else
int
CanalReceive(CANALHANDLE handle, PCANALMSG pCanalMsg);
#endif

/*!
    Receieve a fd message on a CANAL channel.

    The instruction should block if not instructed not to
    do so when the interface was opened.

        :!: Note that if bit 2 in flags of the CANAL message is set and error status
        message is returned from the adapter.

    @param handle - Handle to open physical CANAL channel.
    @param pCanMsgFD - Message to send.
    @return zero on success or error-code on failure.
*/
#ifdef WIN32
int EXPORT
CanalReceiveFD(CANALHANDLE handle, PCANALMSGFD pCanalMsgFD);
#else
int
CanalReceiveFD(CANALHANDLE handle, PCANALMSG pCanalMsgFD);
#endif

/*!
    Blocking receive a message on a CANAL channel.

    The instruction should block if not instructed not to
    do so when the interface was opened.

        Note that if bit 2 in flags of the CANAL message is set and error status
        message is returned from the adapter.

    @param handle - Handle to open physical CANAL channel.
    @param pCanMsg - Message to send.
    @param timeout - time-out in ms. 0 is forever.
    @return zero on success or error-code on failure.
*/
#ifdef WIN32
int EXPORT
CanalBlockingReceive(CANALHANDLE handle, PCANALMSG pCanalMsg, unsigned long timeout);
#else
int
CanalBlockingReceive(CANALHANDLE handle, PCANALMSG pCanalMsg, unsigned long timeout);
#endif

/*!
    Blocking receive a fd message on a CANAL channel.

    The instruction should block if not instructed not to
    do so when the interface was opened.

        Note that if bit 2 in flags of the CANAL message is set and error status
        message is returned from the adapter.

    @param handle - Handle to open physical CANAL channel.
    @param pCanMsgFD - Message to send.
    @param timeout - time-out in ms. 0 is forever.
    @return zero on success or error-code on failure.
*/
#ifdef WIN32
int EXPORT
CanalBlockingReceiveFD(CANALHANDLE handle, PCANALMSG pCanalMsgFD, unsigned long timeout);
#else
int
CanalBlockingReceiveFD(CANALHANDLE handle, PCANALMSG pCanalMsgFD, unsigned long timeout);
#endif

/*!
    Check a CANAL channel for message availability.

    @param handle - Handle to open physical CANAL channel.
    @return Zero if no message is available or the number of messages if
    there are messages waiting to be received.
*/
#ifdef WIN32
int EXPORT
CanalDataAvailable(CANALHANDLE handle);
#else
int
CanalDataAvailable(CANALHANDLE handle);
#endif

/*!
    Get status for a CANAL channel

    @param handle Handle to open physical CANAL channel.
    @param pCanStatus Pointer to a CANAL status structure.
    @return zero on success or error-code on failure.
*/
#ifdef WIN32
int EXPORT
CanalGetStatus(CANALHANDLE handle, PCANALSTATUS pCanalStatus);
#else
int
CanalGetStatus(CANALHANDLE handle, PCANALSTATUS pCanalStatus);
#endif

/*!
    Get statistics for a CANAL channel

    @param handle Handle to open physical CANAL channel.
    @param pCanStatistics Pointer to a CANAL statistics structure.
    @return zero on success or error-code on failure.
*/
#ifdef WIN32
int EXPORT
CanalGetStatistics(CANALHANDLE handle, PCANALSTATISTICS pCanalStatistics);
#else
int
CanalGetStatistics(CANALHANDLE handle, PCANALSTATISTICS pCanalStatistics);
#endif

/*!
    Set the filter for a CANAL channel

    @param handle Handle to open physical CANAL channel.
    @param pCanStatistics Pointer to a CANAL statistics structure.
    @return zero on success or error-code on failure.
*/
#ifdef WIN32
int EXPORT
CanalSetFilter(CANALHANDLE handle, unsigned long filter);
#else
int
CanalSetFilter(CANALHANDLE handle, unsigned long filter);
#endif

/*!
    Set the mask for a CANAL channel

    @param handle Handle to open physical CANAL channel.
    @param mask for
    @return zero on success or error-code on failure.
*/
#ifdef WIN32
int EXPORT
CanalSetMask(CANALHANDLE handle, unsigned long mask);
#else
int
CanalSetMask(CANALHANDLE handle, unsigned long mask);
#endif

/*!
    Set the baudrate for a CANAL channel (deprecated)

    @param handle Handle to open physical CANAL channel.
    @param baudrate Baudrate for the channel
    @return zero on success or error-code on failure.
*/
#ifdef WIN32
int EXPORT
CanalSetBaudrate(CANALHANDLE handle, unsigned long baudrate);
#else
int
CanalSetBaudrate(CANALHANDLE handle, unsigned long baudrate);
#endif

/*!
    Get CANAL version

    @return version for CANAL i/f.
*/
#ifdef WIN32
unsigned long EXPORT
CanalGetVersion(void);
#else
unsigned long
CanalGetVersion(void);
#endif

/*!
    Get CANAL DLL version

    @return version for CANAL dll implementation.
*/
#ifdef WIN32
unsigned long EXPORT
CanalGetDllVersion(void);
#else
unsigned long
CanalGetDllVersion(void);
#endif

/*!
    Get CANAL vendor string

    @return version for CANAL dll implementation.
*/
#ifdef WIN32
const char *EXPORT
CanalGetVendorString(void);
#else
const char *
CanalGetVendorString(void);
#endif

/*!
    Get CANAL driver properties
*/
#ifdef WIN32
const char *EXPORT
CanalGetDriverInfo(void);
#else
const char *
CanalGetDriverInfo(void);
#endif

/*     * * * * Constants * * * *    */

/* CANAL Open i/f flags */
#define CANAL_NONBLOCK

/* Used for blocking calls to block forever */
#define CANAL_BLOCK_FOREVER 0

/* read/write are none blocking */

/* ID flags             */
#define CANAL_IDFLAG_STANDARD 0x00000000 /* Standard message id (11-bit) */
#define CANAL_IDFLAG_EXTENDED 0x00000001 /* Extended message id (29-bit) */
#define CANAL_IDFLAG_RTR      0x00000002 /* RTR-Frame */
#define CANAL_IDFLAG_STATUS   0x00000004 /* This package is a status indication (id holds error code) */
#define CANAL_IDFLAG_FD       0x00000008 /* This is a FD frame */
#define CANAL_IDFLAG_SEND     0x80000000 /* Reserved for use by application software to indicate send */

/* Communication speeds */
#define CANAL_BAUD_USER 0 /* User specified (In CANAL i/f DLL). */
#define CANAL_BAUD_1000 1 /*   1 Mbit */
#define CANAL_BAUD_800  2 /* 800 Kbit */
#define CANAL_BAUD_500  3 /* 500 Kbit */
#define CANAL_BAUD_250  4 /* 250 Kbit */
#define CANAL_BAUD_125  5 /* 125 Kbit */
#define CANAL_BAUD_100  6 /* 100 Kbit */
#define CANAL_BAUD_50   7 /*  50 Kbit */
#define CANAL_BAUD_20   8 /*  20 Kbit */
#define CANAL_BAUD_10   9 /*  10 Kbit */

/* Status message codes ( in received message ) */
#define CANAL_STATUSMSG_OK       0x00 /* Normal condition. */
#define CANAL_STATUSMSG_OVERRUN  0x01 /* Overrun occurred when sending data to CAN bus. */
#define CANAL_STATUSMSG_BUSLIGHT 0x02 /* Error counter has reached 96. */
#define CANAL_STATUSMSG_BUSHEAVY 0x03 /* Error counter has reached 128. */
#define CANAL_STATUSMSG_BUSOFF   0x04 /* Device is in BUSOFF. CANAL_STATUS_OK is */
                                      /*      sent when returning to operational mode. */
#define CANAL_STATUSMSG_STUFF 0x20    /* Stuff Error. */
#define CANAL_STATUSMSG_FORM  0x21    /* Form Error. */
#define CANAL_STATUSMSG_ACK   0x23    /* Ack Error. */
#define CANAL_STATUSMSG_BIT1  0x24    /* Bit1 Error. */
#define CANAL_STATUSMSG_BIT0  0x25    /* Bit0 Error. */
#define CANAL_STATUSMSG_CRC   0x27    /* CRC Error. */

/* Status codes (CANALSTATUS) (returned by status request) */
#define CANAL_STATUS_NONE                 0x00000000
#define CANAL_STATUS_ACTIVE               0x10000000
#define CANAL_STATUS_PASSIVE              0x40000000
#define CANAL_STATUS_BUS_OFF              0x80000000
#define CANAL_STATUS_BUS_WARN             0x20000000
#define CANAL_STATUS_PHY_FAULT            0x08000000
#define CANAL_STATUS_PHY_H                0x04000000
#define CANAL_STATUS_PHY_L                0x02000000
#define CANAL_STATUS_SLEEPING             0x01000000
#define CANAL_STATUS_STOPPED              0x00800000
#define CANAL_STATUS_RECIVE_BUFFER_FULL   0x00400000 /* Drivers buffer */
#define CANAL_STATUS_TRANSMIT_BUFFER_FULL 0x00200000 /* Drivers buffer */

/* Error Codes */
#define CANAL_ERROR_SUCCESS           0  /* All is OK */
#define CANAL_ERROR_BAUDRATE          1  /* Baud rate error */
#define CANAL_ERROR_BUS_OFF           2  /* Bus off error */
#define CANAL_ERROR_BUS_PASSIVE       3  /* Bus Passive error */
#define CANAL_ERROR_BUS_WARNING       4  /* Bus warning error */
#define CANAL_ERROR_CAN_ID            5  /* Invalid CAN ID */
#define CANAL_ERROR_CAN_MESSAGE       6  /* Invalid CAN message */
#define CANAL_ERROR_CHANNEL           7  /* Invalid channel */
#define CANAL_ERROR_FIFO_EMPTY        8  /* FIFO is empty */
#define CANAL_ERROR_FIFO_FULL         9  /* FIFI is full */
#define CANAL_ERROR_FIFO_SIZE         10 /* FIFO size error */
#define CANAL_ERROR_FIFO_WAIT         11
#define CANAL_ERROR_GENERIC           12 /* Generic error */
#define CANAL_ERROR_HARDWARE          13 /* Hardware error */
#define CANAL_ERROR_INIT_FAIL         14 /* Initialization failed */
#define CANAL_ERROR_INIT_MISSING      15
#define CANAL_ERROR_INIT_READY        16
#define CANAL_ERROR_NOT_SUPPORTED     17 /* Not supported */
#define CANAL_ERROR_OVERRUN           18 /* Overrun */
#define CANAL_ERROR_RCV_EMPTY         19 /* Receive buffer empty */
#define CANAL_ERROR_REGISTER          20 /* Register value error */
#define CANAL_ERROR_TRM_FULL          21
#define CANAL_ERROR_ERRFRM_STUFF      22 /* Error frame: stuff error detected */
#define CANAL_ERROR_ERRFRM_FORM       23 /* Error frame: form error detected */
#define CANAL_ERROR_ERRFRM_ACK        24 /* Error frame: acknowledge error */
#define CANAL_ERROR_ERRFRM_BIT1       25 /* Error frame: bit 1 error */
#define CANAL_ERROR_ERRFRM_BIT0       26 /* Error frame: bit 0 error */
#define CANAL_ERROR_ERRFRM_CRC        27 /* Error frame: CRC error */
#define CANAL_ERROR_LIBRARY           28 /* Unable to load library */
#define CANAL_ERROR_PROCADDRESS       29 /* Unable get library proc. address */
#define CANAL_ERROR_ONLY_ONE_INSTANCE 30 /* Only one instance allowed */
#define CANAL_ERROR_SUB_DRIVER        31 /* Problem with sub driver call */
#define CANAL_ERROR_TIMEOUT           32 /* Blocking call time-out */
#define CANAL_ERROR_NOT_OPEN          33 /* The device is not open. */
#define CANAL_ERROR_PARAMETER         34 /* A parameter is invalid. */
#define CANAL_ERROR_MEMORY            35 /* Memory exhausted. */
#define CANAL_ERROR_INTERNAL          36 /* Some kind of internal program error */
#define CANAL_ERROR_COMMUNICATION     37 /* Some kind of communication error */
#define CANAL_ERROR_USER              38 /* Login error */
// Version 1.1.0
#define CANAL_ERROR_MTU                   39 /* Frame does not fit */
#define CANAL_ERROR_FD_SUPPORT            40 /* Flexible data-rate is not supported on this interface */
#define CANAL_ERROR_FD_SUPPORT_ENABLE     41 /* Error on enabling fexible-data-rate support */
#define CANAL_ERROR_SOCKET_CREATE         42 /* Unable to create (socketcan) socket */
#define CANAL_ERROR_CONVERT_NAME_TO_INDEX 43 /* Unable to convert (socketcan) name to index */
#define CANAL_ERROR_SOCKET_BIND           44 /* Unable to bind to socket or equivalent */

/* Filter mask settings */
#define CANUSB_ACCEPTANCE_FILTER_ALL 0x00000000
#define CANUSB_ACCEPTANCE_MASK_ALL   0xFFFFFFFF

/*!
    XML CAN message template

    <canal flags="1234"
            id="1234"
            obid="1234"
            dataSize="1234"
            data="1,2,3,4..."
            timestamp="1234"  />
*/

#define CANAL_XML_MSG_TEMPLATE                                                                                         \
  "<canal "                                                                                                            \
  "flags=\"%lu\" "                                                                                                     \
  "id=\"%lu\" "                                                                                                        \
  "obid=\"%lu\" "                                                                                                      \
  "sizeData=\"%d\" "                                                                                                   \
  "data=\"%s\" "                                                                                                       \
  "timestamp=\"%lu\" "                                                                                                 \
  "/>"

/*!
    JSON CAN message template

    {
        "flags"     : 1234,
        "id"        : 1234,
        "obid"      : 1234,
        "sizeData"  : 1234,
        "data"      : [1,1,2,3...],
        "timestamp" : 1234
    }
*/

#define CANAL_JSON_MSG_TEMPLATE                                                                                        \
  "{"                                                                                                                  \
  "\"flags\": %lu,\n"                                                                                                  \
  "\"id\": %lu,\n"                                                                                                     \
  "\"obid\": %lu,\n"                                                                                                   \
  "\"sizeData\": %d,\n"                                                                                                \
  "\"data\": [%s],\n"                                                                                                  \
  "\"timestamp\": %lu,\n"                                                                                              \
  "}"

#ifdef __cplusplus
}
#endif

#endif /* ___CANAL_H___ */
