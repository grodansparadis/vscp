//////////////////////////////////////////////////////////////////////////
// IXXAT Automation GmbH
//////////////////////////////////////////////////////////////////////////
/**
  This file defines the register layout for the IFI CAN FD
  controller.

  @file IFIfd.h
*/
//////////////////////////////////////////////////////////////////////////
// (C) 2002-2013 IXXAT Automation GmbH, all rights reserved
//////////////////////////////////////////////////////////////////////////

#ifndef IFIFD_H
#define IFIFD_H


/**
  IFI CAN Warning Level counter Limit
  If RX or TX error counter is equal to or above this limit
  the CAN controller is in Error Warning Level.
*/
#define IFICAN_WARNING_LEVEL_COUNTER_LIMIT   96

/**
  IFI CAN Busoff Status counter Limit
  If TX error counter is equal to or above this limit
  the CAN controller is in Busoff State.
*/
#define IFICAN_BUSOFF_STATE_COUNTER_LIMIT   256

//////////////////////////////////////////////////////////////////////////
// IFI CANfd register offsets
//////////////////////////////////////////////////////////////////////////
#define IFIFDREG_STSCMD          0
#define IFIFDREG_RX_STSCMD       1
#define IFIFDREG_TX_STSCMD       2
#define IFIFDREG_INTPEND         3
#define IFIFDREG_INTMASK         4
#define IFIFDREG_TIMCTRL         5
#define IFIFDREG_TIMCTRL_FAST    6
#define IFIFDREG_TRA_DELAY       7
#define IFIFDREG_ERROR           8
#define IFIFDREG_ERROR_ADD       9
#define IFIFDREG_TRA_SUSPEND     10
#define IFIFDREG_TRA_REPCOUNT    11
#define IFIFDREG_BUSTRAFFIC      12
#define IFIFDREG_TIM_CTRL        13
#define IFIFDREG_TIM_ACTUAL      14
#define IFIFDREG_TIM_LAST        15
#define IFIFDREG_RES1            16
#define IFIFDREG_ERROR_HIDDEN    17
#define IFIFDREG_COMP_PARA       18
#define IFIFDREG_CAN_CLOCK       19
#define IFIFDREG_SYS_CLOCK       20
#define IFIFDREG_REVISION        21
#define IFIFDREG_IP_CORE_ID      22
#define IFIFDREG_TEST            23

#define IFIFDREG_RX_FIFO         24

#define IFIFDREG_RX_FIFO_STAT    24  ///< on when frame number > 0 to get timestamp
#define IFIFDREG_RX_FIFO_TS      25  ///< time stamp
#define IFIFDREG_RX_FIFO_DLC     26
#define IFIFDREG_RX_FIFO_ID      27
#define IFIFDREG_RX_FIFO_DATA14  28
#define IFIFDREG_RX_FIFO_DATA58  29

#define IFIFDREG_TX_FIFO_SUSP    44
#define IFIFDREG_TX_FIFO_REPEAT  45
#define IFIFDREG_TX_FIFO_DLC     46
#define IFIFDREG_TX_FIFO_ID      47
#define IFIFDREG_TX_FIFO_DATA14  48
#define IFIFDREG_TX_FIFO_DATA58  49


#define IFIFDREG_FILTERMASK      512
#define IFIFDREG_FILTERID        513

#define IFIFDREG_FILTERCOUNT     256

//////////////////////////////////////////////////////////////////////////
// IFI CANfd Main Status and CommandRegister Bits ( Register 0 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_R0_RD_ERRACT       0x00000004
#define IFIFD_R0_RD_ERRPAS       0x00000008
#define IFIFD_R0_RD_BUSOFF       0x00000010
#define IFIFD_R0_RD_HARDRESET    0x40000000

#define IFIFD_R0_WR_N_MODE_START 0x80000000

#define IFIFD_R0_ENA             0x00000001
#define IFIFD_R0_BUSMON          0x00010000
#define IFIFD_R0_RESTRICTED      0x00020000
#define IFIFD_R0_LOOPBACK        0x00040000
#define IFIFD_R0_LOOPBACKE       0x00080000
#define IFIFD_R0_EDGEFILT_ENA    0x00100000  // ISO
#define IFIFD_R0_EXCEPT_ENA      0x00200000  // ISO
#define IFIFD_R0_64BitTS_ENA     0x00400000  // ISO
#define IFIFD_R0_CANFD_DIS       0x01000000
#define IFIFD_R0_ISO_ENA         0x02000000  // ISO
#define IFIFD_R0_7_9_8_8_BT      0x04000000  // ISO BitTiming
// the old IFI has the format 4_12_6_6

#define IFIFD_R0_HARD_RESET_VAL  0xDEADCAFD


//////////////////////////////////////////////////////////////////////////
// IFI CANfd Receiver Fifo Status and Command ( Register 1 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_R1_RD_EMPTY        0x00000100
#define IFIFD_R1_RD_M_HALF       0x00000200
#define IFIFD_R1_RD_M_THREEFOUR  0x00000400
#define IFIFD_R1_RD_M_SEVEIGHT   0x00000800
#define IFIFD_R1_RD_FULL         0x00001000
#define IFIFD_R1_RD_OVERFLOW     0x00002000

#define IFIFD_R1_RD_RBC          0xFFFF0000 // receive Fifo number
#define IFIFD_R1_RBC_S           16

#define IFIFD_R1_WR_REM_MSG      0x00000001 // Write 1 to remove message
#define IFIFD_R1_WR_FIFO_CLEAR   0x00000080 // Reset Fifo, Clear Bit to enable Fifo

//////////////////////////////////////////////////////////////////////////
// IFI CANfd Transmitter Fifo Status and Command ( Register 2 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_R2_RD_EMPTY        0x00000100
#define IFIFD_R2_RD_L_ONEEIGHT   0x00000200
#define IFIFD_R2_RD_L_ONEFOUR    0x00000400
#define IFIFD_R2_RD_L_HALF       0x00000800
#define IFIFD_R2_RD_FULL         0x00001000
#define IFIFD_R2_RD_OVERFLOW     0x00002000
#define IFIFD_R2_RD_INV_ACC      0x00004000
#define IFIFD_R2_RD_HP_PEND      0x00008000 // high priority pending

#define IFIFD_R2_RD_TBC          0xFFFF0000 // receive Fifo number
#define IFIFD_R2_TBC_S           16

#define IFIFD_R2_WR_ADD_MSG      0x00000001 // Write 1 to add message
#define IFIFD_R2_WR_HP           0x00000002 // announce high priority
#define IFIFD_R2_WR_REM_PEND     0x00000040 // remove pending message
#define IFIFD_R2_WR_FIFO_CLEAR   0x00000080 // Reset Fifo, Clear Bit to enable Fifo

//////////////////////////////////////////////////////////////////////////
// IFI CANfd Interrupt Pending Register Bits ( Register 3 )
// IFI CANfd Interrupt MASK Register Bits ( Register 4 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_IRQ_BO             0x00000001 // Bus Off
#define IFIFD_IRQ_EW             0x00000002 // Error Warning
#define IFIFD_IRQ_TS_OVR         0x00000100 // Timestamp Overrun
#define IFIFD_IRQ_ERRCNT2_OVR    0x00000200 // Error Counter II overrun
#define IFIFD_IRQ_ERR_INFO       0x00000400 //
#define IFIFD_IRQ_FDproto_except 0x00000800 // �FDProtocol Exception  ISO
#define IFIFD_IRQ_RMProto_Exc    0x00001000 // �RMProtocol Exception  ISO
                                            // there was a protocol exception

#define IFIFD_IRQ_TFIFO_EMPTY    0x00010000 // Transmit Fifo empty
#define IFIFD_IRQ_TFIFO_ONEEIGHT 0x00020000 // Transmit Fifo less 1/8 full
#define IFIFD_IRQ_TFIFO_ONEFOUR  0x00040000 // Transmit Fifo less 1/4 full
#define IFIFD_IRQ_TFIFO_HALF     0x00080000 // Transmit Fifo less half full
#define IFIFD_IRQ_TFIFO_FULL     0x00100000 // Transmit Fifo full
#define IFIFD_IRQ_TFIFO_OVERFLOW 0x00200000 // Transmit Fifo overflow
#define IFIFD_IRQ_TFIFO_OK       0x00400000 // Transmit OK IRQ ??? undocumented, to be approved

#define IFIFD_IRQ_RFIFO_NEMPTY    0x01000000 // Receive Fifo not empty
#define IFIFD_IRQ_RFIFO_NEMPTYPER 0x02000000 // Receive Fifo not empty per
#define IFIFD_IRQ_RFIFO_HALF      0x04000000 // Receive Fifo more half full
#define IFIFD_IRQ_RFIFO_THREEFOUR 0x08000000 // Receive Fifo more 3/4 full
#define IFIFD_IRQ_RFIFO_SEVEIGHT  0x10000000 // Receive Fifo more 7/8 full
#define IFIFD_IRQ_RFIFO_FULL      0x20000000 // Receive Fifo full
#define IFIFD_IRQ_RFIFO_OVERFLOW  0x40000000 // Receive Fifo overflow

#define IFIFD_IRQ_ALL   IFIFD_IRQ_BO             | IFIFD_IRQ_EW              | \
                        IFIFD_IRQ_TS_OVR         | IFIFD_IRQ_ERRCNT2_OVR     | \
                        IFIFD_IRQ_ERR_INFO       |                             \
                        IFIFD_IRQ_FDproto_except | IFIFD_IRQ_RMProto_Exc     | \
                        IFIFD_IRQ_TFIFO_EMPTY    | IFIFD_IRQ_TFIFO_ONEEIGHT  | \
                        IFIFD_IRQ_TFIFO_ONEFOUR  | IFIFD_IRQ_TFIFO_HALF      | \
                        IFIFD_IRQ_TFIFO_FULL     | IFIFD_IRQ_TFIFO_OVERFLOW  | \
                        IFIFD_IRQ_TFIFO_OK       |                             \
                        IFIFD_IRQ_RFIFO_NEMPTY   | IFIFD_IRQ_RFIFO_NEMPTYPER | \
                        IFIFD_IRQ_RFIFO_HALF     | IFIFD_IRQ_RFIFO_THREEFOUR | \
                        IFIFD_IRQ_RFIFO_SEVEIGHT | IFIFD_IRQ_RFIFO_FULL      | \
                        IFIFD_IRQ_RFIFO_OVERFLOW

//////////////////////////////////////////////////////////////////////////
// additional IFI CANfd Interrupt MASK Register Bits ( Register 4 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_R4_WR_SET_ERR_IRQ  0x00000080 // set Error IRQ-Mask
#define IFIFD_R4_WR_SET_TIM_IRQ  0x00008000 // set Timestamp IRQ-Mask
#define IFIFD_R4_WR_SET_TX_IRQ   0x00800000 // set Transmit IRQ-Mask
#define IFIFD_R4_WR_SET_RX_IRQ   0x80000000 // set Receive IRQ-Mask

//////////////////////////////////////////////////////////////////////////
// IFI CANfd Time Control Register Bits ( Register 5 / 6 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_BDRT_TIMEB           0x0000003F // Time B
#define IFIFD_BDRT_TIMEA           0x00003F00 // Time A
#define IFIFD_BDRT_PRESCALE        0x0FFF0000 // prescaler
#define IFIFD_BDRT_SJW             0xF0000000 // SJW

#define IFIFD_BDRT_TIMEB_S         0
#define IFIFD_BDRT_TIMEA_S         8
#define IFIFD_BDRT_PRESCALE_S      16
#define IFIFD_BDRT_SJW_S           28

#define IFIFD_BDRT_WR_S_SJW        0x00000040 // set SJW
#define IFIFD_BDRT_WR_S_TIMEB      0x00000080 // set Time B
#define IFIFD_BDRT_WR_S_PRESCALE   0x00004000 // set prescaler
#define IFIFD_BDRT_WR_S_TIMEA      0x00008000 // set Time A

#define IFIFD_BDRT_ISO_TIMEB       0x000000FF // Time B
#define IFIFD_BDRT_ISO_TIMEA       0x0000FF00 // Time A
#define IFIFD_BDRT_ISO_PRESCALE    0x01FF0000 // prescaler
#define IFIFD_BDRT_ISO_SJW         0xFE000000 // SJW

#define IFIFD_BDRT_ISO_TIMEB_S     0
#define IFIFD_BDRT_ISO_TIMEA_S     8
#define IFIFD_BDRT_ISO_PRESCALE_S  16
#define IFIFD_BDRT_ISO_SJW_S       25

// Annotation
// TIMEB can be set to -1 + fixed increment +2 -> 1


//////////////////////////////////////////////////////////////////////////
// IFI CANfd Transmitter Delay Register Bits ( Register 7 )
//////////////////////////////////////////////////////////////////////////

// definition of first IFI CAN-FD implementation ( non ISO )
// #define IFIFD_R7_TX_DELAY_MEAS    0x00FF0000  // measured transmitter delay in can_clocks
#define IFIFD_R7_TX_DELAY_MEAS      0xFFFF0000  // measured transmitter delay in can_clocks
#define IFIFD_R7_TX_DELAY_MEAS_S    16

#define IFIFD_R7_TX_DELAY_CFG       0x000000FF  // TX delay in can_Clocks
#define IFIFD_R7_TX_DELAY_CFG_S     0

#define IFIFD_R7_TX_DELAY_ISO_CFG   0x00000FFF  // TX delay in can_Clocks
#define IFIFD_R7_TX_DELAY_ISO_CFG_S 0

#define IFIFD_R7_TX_DELAY_ENA       0x00000100  // enable transmitter delay (set to 1 when using BRS)
#define IFIFD_R7_TX_DELAY_ISO_ENA   0x00008000  // enable transmitter delay (set to 1 when using BRS)
#define IFIFD_R7_TX_DELAY_ISO_ABS   0x00004000  // absolute transmitter delay (doesn't add to measured transmitter Delay )


//////////////////////////////////////////////////////////////////////////
// IFI CANfd Error Counter Register Bits ( Register 8 )
//////////////////////////////////////////////////////////////////////////

///> transmit error counter using CAN rules
#define IFIFD_R8_TX_ERROR_COUNT    0x000001FF

///> receive error counter using CAN rules
#define IFIFD_R8_RX_ERROR_COUNT    0x00FF0000

#define IFIFD_R8_TX_ERROR_COUNT_S  0
#define IFIFD_R8_RX_ERROR_COUNT_S  16

//////////////////////////////////////////////////////////////////////////
// IFI CANfd additional Error Counter Register Bits ( Register 9 )
//
// note: These 4 ErrorCouterII are reset with setting the normal-Bit
//////////////////////////////////////////////////////////////////////////

///> count error during dataphase if transmitter ( slow baudrate )
#define IFIFD_R9_TX_ERRDATA_SLOW   0x000000FF

///> count error during dataphase if transmitter ( fast baudrate )
#define IFIFD_R9_TX_ERRDATA_FAST   0x0000FF00

///> count error during arbitrationphase if receiver ( slow baudrate )
#define IFIFD_R9_RX_ERRARB_SLOW    0x00FF0000

///> count error during dataphase if receiver ( fast baudrate )
#define IFIFD_R9_RX_ERRDATA_FAST   0xFF000000

#define IFIFD_R9_TX_ERRDATA_SLOW_S 0
#define IFIFD_R9_TX_ERRDATA_FAST_S 8
#define IFIFD_R9_RX_ERRARB_SLOW_S  16
#define IFIFD_R9_RX_ERRDATA_FAST_S 24

//////////////////////////////////////////////////////////////////////////
// IFI CANfd Bus Traffic Rate ( Register 12 )
//////////////////////////////////////////////////////////////////////////

///> frames since the start when bus traffic Bit 30 set
#define IFIFD_R12_BT_FRAMES        0x3FFFFFFF

///> traffic rate in % 0xFFFF -> 100%
#define IFIFD_R12_BT_PERCENT       0x0000FFFF

///> restart bus traffic rate by writing 1
#define IFIFD_R12_BT_START         0x80000000

///> 0:measure rate in percent, 1: count frames (SOF)
#define IFIFD_R12_BT_MODE          0x40000000

//////////////////////////////////////////////////////////////////////////
// IFI CANfd Bus Traffic Rate ( Register 13 )
//////////////////////////////////////////////////////////////////////////

///> reset timestamp counter by writing 1
#define IFIFD_R13_TSTAMP_RESET     0x00000001

///> use TimeStamp_reset input as Timestamp reset
#define IFIFD_R13_TSTAMP_USE_R_INP 0x00000002

///> use TimeStamp_clock input as Timestamp clock, else 1 �s is used
#define IFIFD_R13_TSTAMP_USE_C_INP 0x00000004

///> select SOF as trigger for Timestamp,else use ACK as trigger
#define IFIFD_R13_TSTAMP_USE_SOF   0x00000008

///> Timestamp Counter Overrun int Bit 32, clear by writing 1
#define IFIFD_R13_TSTAMP_OVR_CLEAR 0x80000000

//////////////////////////////////////////////////////////////////////////
// IFI CANfd Errro Register ( Register 17 ) //HIDDEN//
//////////////////////////////////////////////////////////////////////////

///> reset timestamp counter by writing 1
#define IFIFD_R17_TSTAMP_RESET     0x00000001

#define IFIFD_R17_RD_ERR_FIRST   0x000000FF // error first
#define IFIFD_R17_RD_ERR_ALL     0x00007F00 // error all
#define IFIFD_R17_RD_BIT_POS     0x03FF0000 // bit position

#define IFIFD_R17_WR_ERF_RST     0x04000000 // reset error frame
#define IFIFD_R17_WR_OVR_RST     0x08000000 // reset overload
#define IFIFD_R17_WR_ERR_PEND    0x10000000 // error frame pending
#define IFIFD_R17_WR_OVR_PEND    0x20000000 // overload pending
#define IFIFD_R17_WR_ERR_RST     0x40000000 // Error rest (for bits 25..0)
#define IFIFD_R17_WR_ERR_ENA     0x80000000 // Error enable

#define IFIFD_R17_ERR_FIRST_S    0
#define IFIFD_R17_ERR_ALL_S      8
#define IFIFD_R17_ERR_BIT_POS_S 16

#define IFIFD_R17_CANERR_OVERLOAD 0x01
#define IFIFD_R17_CANERR_ACK      0x02
#define IFIFD_R17_CANERR_BIT0     0x04
#define IFIFD_R17_CANERR_BIT1     0x08
#define IFIFD_R17_CANERR_STUFF    0x10
#define IFIFD_R17_CANERR_CRC      0x20
#define IFIFD_R17_CANERR_FORM     0x40
#define IFIFD_R17_CANERR_FAST     0x80

//////////////////////////////////////////////////////////////////////////
// IFI CANfd compiler parameter Register Bits ( Register 18 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_R18_RD_TX_SIZE        0x000000FF // transmit fifo size in kByte
#define IFIFD_R18_RD_RX_SIZE        0x0000FF00 // receive fifo size in kByte
#define IFIFD_R18_RD_SINGLE_CLOCK   0x00010000 // user single clock
#define IFIFD_R18_RD_NO_TSTAMP      0x00020000 // no Timestamp
#define IFIFD_R18_RD_NO_FILTER      0x00040000 // no filter
#define IFIFD_R18_RD_NO_BUSSTAT     0x00080000 // no bus statistic

// ID format ( 0: IFI_legacy, 1: CANalyzer, 2: ID_other )
#define IFIFD_R18_RD_ID_FORMAT0     0x00300000 // ID format ( 0 IFI_legacy )

#define IFIFD_R18_RD_TX_SIZE_S      0
#define IFIFD_R18_RD_RX_SIZE_S      8
#define IFIFD_R18_RD_ID_FORMAT0_S  20

//////////////////////////////////////////////////////////////////////////
// IFI CANfd revision Register Bits ( Register 21 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_R21_RD_CR     0x000000FF // core revision
#define IFIFD_R21_RD_Q      0x0000FF00 // quartus
#define IFIFD_R21_RD_Y      0x00FF0000 // year
#define IFIFD_R21_RD_M      0xFF000000 // month

#define IFIFD_R21_RD_CR_S    0
#define IFIFD_R21_RD_Q_S     8
#define IFIFD_R21_RD_Y_S    16
#define IFIFD_R21_RD_M_S    24

//////////////////////////////////////////////////////////////////////////
// IFI CANfd Receive FIFO DLC Register bits ( Register 26 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_RXFIFO_DLC         0x0000000F // data length code
#define IFIFD_RXFIFO_RTR         0x00000010 // Remote Transmission Request ( not with EDL)
#define IFIFD_RXFIFO_EDL         0x00000020 // Extended Data Length (CAN-FD)
#define IFIFD_RXFIFO_BRS         0x00000040 // Bit Rate Switch (only with EDL)
#define IFIFD_RXFIFO_ESI         0x00000080 // Error State Indicator (only with EDL, RX only)
#define IFIFD_RXFIFO_OBJ         0x0001FF00 // Filter Object number of a received message
#define IFIFD_RXFIFO_FRN         0xFF000000 // Frame number for Transmit Timestamp

#define IFIFD_RXFIFO_DLC_S        0
#define IFIFD_RXFIFO_OBJ_S        8
#define IFIFD_RXFIFO_FRN_S       24


//////////////////////////////////////////////////////////////////////////
// IFI CANfd Receive FIFO Identifier Register Bits ( Register 27 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_RXFIFO_ID           0x1FFFFFFF
#define IFIFD_RXFIFO_IDSTD        0x000007FF
#define IFIFD_RXFIFO_IDEXT_18_28  0x000007FF
#define IFIFD_RXFIFO_IDEXT_00_17  0x1FFFF800
#define IFIFD_RXFIFO_IDE          0x20000000

#define IFIFD_RXFIFO_ID_S            0
#define IFIFD_RXFIFO_IDSTD_S         0
#define IFIFD_RXFIFO_IDEXT_18_28_S   0
#define IFIFD_RXFIFO_IDEXT_00_17_S   11

//////////////////////////////////////////////////////////////////////////
// IFI CANfd Transmit FIFO Suspend Register Bits ( Register 44 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_TXFIFO_SUSP_DIS    0xFFFFFFFF

//////////////////////////////////////////////////////////////////////////
// IFI CANfd Transmit FIFO Repetition Register Bits ( Register 45 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_TXFIFO_REP_DEF     0xFFFFFFFF

//////////////////////////////////////////////////////////////////////////
// IFI CANfd Transmit FIFO DLC Register bits ( Register 46 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_TXFIFO_DLC         0x0000000F // data length code
#define IFIFD_TXFIFO_RTR         0x00000010 // Remote Transmission Request ( not with EDL)
#define IFIFD_TXFIFO_EDL         0x00000020 // Extended Data Length (CAN-FD)
#define IFIFD_TXFIFO_BRS         0x00000040 // Bit Rate Switch (only with EDL)

#define IFIFD_TXFIFO_FRN         0xFF000000 // Frame number for Transmit Timestamp

#define IFIFD_TXFIFO_DLC_S        0
#define IFIFD_TXFIFO_FRN_S       24

//////////////////////////////////////////////////////////////////////////
// IFI CANfd Transmit FIFO Identifier Register Bits ( Register 47 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_TXFIFO_ID           0x1FFFFFFF
#define IFIFD_TXFIFO_IDSTD        0x000007FF
#define IFIFD_TXFIFO_IDEXT_18_28  0x000007FF
#define IFIFD_TXFIFO_IDEXT_00_17  0x1FFFF800
#define IFIFD_TXFIFO_IDE          0x20000000

#define IFIFD_TXFIFO_ID_S            0
#define IFIFD_TXFIFO_IDSTD_S         0
#define IFIFD_TXFIFO_IDEXT_18_28_S   0
#define IFIFD_TXFIFO_IDEXT_00_17_S   11




// mask for to reset Interrupt status
//
// #define IFIFD_SR_RDI     (IFIFD_SR_RD_I_RBF | IFIFD_SR_RD_I_RBF | IFIFD_SR_RD_I_RNE | IFIFD_SR_RD_RB)
//
// #define IFIFD_SR_TDI     (IFIFD_SR_RD_I_TBE | IFIFD_SR_RD_I_TOK)
// #define IFIFD_SR_TDI_NOT (IFIFD_SR_RD_I_TBF | IFIFD_SR_RD_I_TB0 | IFIFD_SR_RD_TBF)

//////////////////////////////////////////////////////////////////////////
// IFI CANfd filter mask Bits ( Register 512 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_R512_RD_FMASK      0x1FFFFFFF // filter mask
#define IFIFD_R512_RD_FMEXT      0x20000000 // extended ?
#define IFIFD_R512_RD_FMEDL      0x40000000 // extended data length ?
#define IFIFD_R512_RD_FMVALID    0x80000000 // valid ?

//////////////////////////////////////////////////////////////////////////
// IFI CANfd identifier bits ( Register 513 )
//////////////////////////////////////////////////////////////////////////
#define IFIFD_R513_RD_FID        0x1FFFFFFF // filter mask
#define IFIFD_R513_RD_FIDEXT     0x20000000 // extended ?
#define IFIFD_R513_RD_FIDCANFD   0x40000000 // use CANfd Frame?
#define IFIFD_R513_RD_FIDVALID   0x80000000 // valid ?

#endif
