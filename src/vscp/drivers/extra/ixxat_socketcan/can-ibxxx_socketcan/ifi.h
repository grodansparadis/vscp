//////////////////////////////////////////////////////////////////////////
// IXXAT Automation GmbH
//////////////////////////////////////////////////////////////////////////
/**
  @todo long description

  @file "IFI.h"
*/
//////////////////////////////////////////////////////////////////////////
// (C) 2002-2012 IXXAT Automation GmbH, all rights reserved
//////////////////////////////////////////////////////////////////////////

#ifndef IFI_H
#define IFI_H

//////////////////////////////////////////////////////////////////////////
//  include files
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//  constants and macros
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// data types
//////////////////////////////////////////////////////////////////////////

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


#define WR_VAL( val, mask, shift) (( ((u32) val) << shift) & mask)
#define RD_VAL( val, mask, shift) (( ((u32) val) & mask) >> shift)

//////////////////////////////////////////////////////////////////////////
// IFI register offsets
//////////////////////////////////////////////////////////////////////////
#define IFIREG_DLC             0
#define IFIREG_IDENTIFER       1
#define IFIREG_DATA14          2
#define IFIREG_DATA58          3
#define IFIREG_TIMCTRL         4
#define IFIREG_INTMASK         5
#define IFIREG_STATUS          6
#define IFIREG_ERROR           7
#define IFIREG_VERSION         8
#define IFIREG_FIFO            9
#define IFIREG_TIMESTAMP       10
#define IFIREG_CURTIMESTAMP    11
#define IFIREG_PARA            12
#define IFIREG_ERRORDETAIL     13
#define IFIREG_FILTERMASK      512
#define IFIREG_FILTERID        513

//////////////////////////////////////////////////////////////////////////
// IFI DLC Register Bits ( Register 0 )
//////////////////////////////////////////////////////////////////////////
#define IFI_R0_RD_DLC          0x0000000F
#define IFI_R0_RD_RTR          0x00000010
#define IFI_R0_RD_OBJ          0x0001FF00
#define IFI_R0_RD_SSM          0x00800000
#define IFI_R0_RD_FRN          0xFF000000

#define IFI_R0_DLC_S           0
#define IFI_R0_RTR_S           4
#define IFI_R0_OBJ_S           8
#define IFI_R0_SSM_S           23
#define IFI_R0_FRN_S           24

#define IFI_R0_WR_DLC          0x0000000F
#define IFI_R0_WR_RTR          0x00000010
#define IFI_R0_WR_SSM          0x00800000
#define IFI_R0_WR_FRN          0xFF000000

//////////////////////////////////////////////////////////////////////////
// IFI Identifier Register Bits ( Register 1 )
//////////////////////////////////////////////////////////////////////////
#define IFI_R1_ID           0x1FFFFFFF
#define IFI_R1_IDSTD        0x000007FF
#define IFI_R1_IDEXT_18_28  0x000007FF
#define IFI_R1_IDEXT_18_28_INV  IFI_R1_IDEXT_18_28 <<18
#define IFI_R1_IDEXT_00_17  0x1FFFF800
#define IFI_R1_IDEXT_00_17_INV  IFI_R1_IDEXT_00_17 >> 11
#define IFI_R1_IDE          0x20000000

#define IFI_R1_ID_S            0
#define IFI_R1_IDSTD_S         0
#define IFI_R1_IDEXT_18_28_S   0
#define IFI_R1_IDEXT_00_17_S   11
#define IFI_R1_IDE_S           29


//////////////////////////////////////////////////////////////////////////
// IFI Time Control Register Bits ( Register 4 )
//////////////////////////////////////////////////////////////////////////
#define IFI_R4_WR_TIMEB        0x0000001F // Time B
#define IFI_R4_WR_SJW          0x00000060 // SJW
#define IFI_R4_WR_S_TIMEB      0x00000080 // set Time B
#define IFI_R4_WR_TIMEA        0x00001F00 // Time A
#define IFI_R4_WR_S_SJW        0x00004000 // set SJW
#define IFI_R4_WR_S_TIMEA      0x00008000 // set Time A
#define IFI_R4_WR_PRESCALE     0x00FF0000 // prescaler
#define IFI_R4_WR_NMODE        0x01000000 // normal mode
#define IFI_R4_WR_HIMSG        0x04000000 // high priority message
#define IFI_R4_WR_TS_RESET     0x08000000 // Timestamp counter reset
#define IFI_R4_WR_SMODE        0x10000000 // silent mode
#define IFI_R4_WR_RPMSG        0x20000000 // remove pending message
#define IFI_R4_WR_S_SMODE      0x40000000 // set silent mode
#define IFI_R4_WR_S_PRESCALE   0x80000000 // set prescaler

#define IFI_R4_RD_TIMEB        0x0000001F // Time B
#define IFI_R4_RD_TIMEA        0x00001F00 // Time A
#define IFI_R4_RD_PRESCALE     0x00FF0000 // prescaler
#define IFI_R4_RD_SJW          0x06000000 // SJW

#define IFI_R4_TIMEB_S         0
#define IFI_R4_TIMEA_S         8
#define IFI_R4_PRESCALE_S      16
#define IFI_R4_SJW_S           25

//////////////////////////////////////////////////////////////////////////
// IFI Interrupt Register Bits ( Register 5 )
//////////////////////////////////////////////////////////////////////////
#define IFI_IR_RD_BO      0x00000001 // Bus Off
#define IFI_IR_RD_EW      0x00000002 // Error Warning
#define IFI_IR_RD_TOK     0x00000040 // Transmit OK
#define IFI_IR_RD_EI      0x00010000 // ErrorInformation
#define IFI_IR_RD_TBE     0x01000000 // Transmit buffer empty INT reset
#define IFI_IR_RD_TBO     0x02000000 // Transmit buffer overflow INT reset
#define IFI_IR_RD_TBF     0x04000000 // Transmit buffer full INT reset
#define IFI_IR_RD_RNE     0x08000000 // Receive buffer not empty INT reset
#define IFI_IR_RD_RBO     0x10000000 // Receive buffer overflow INT reset
#define IFI_IR_RD_RBF     0x20000000 // Receive buffer full INT reset

#define IFI_IR_WR_EW      0x00000001 // Error Warning
#define IFI_IR_WR_BO      0x00000002 // Bus Off
#define IFI_IR_WR_SEM     0x00000080 // Set Error Interrupt Mask
#define IFI_IR_WR_TOK     0x00004000 // Transmit OK
#define IFI_IR_WR_STOK    0x00008000 // Set Transmit OK
#define IFI_IR_WR_EI      0x00010000 // ErrorInformation
#define IFI_IR_WR_SEIM    0x00800000 // Set Error Information Mask
#define IFI_IR_WR_TBE     0x01000000 // Transmit buffer empty INT reset
#define IFI_IR_WR_TBO     0x02000000 // Transmit buffer overflow INT reset
#define IFI_IR_WR_TBF     0x04000000 // Transmit buffer full INT reset
#define IFI_IR_WR_RNE     0x08000000 // Receive buffer not empty INT reset
#define IFI_IR_WR_RBO     0x10000000 // Receive buffer overflow INT reset
#define IFI_IR_WR_RBF     0x20000000 // Receive buffer full INT reset
#define IFI_IR_WR_SBM     0x80000000 // Set Buffer Int Mask

//////////////////////////////////////////////////////////////////////////
// IFI Status Register Bits ( Register 6 )
//////////////////////////////////////////////////////////////////////////
#define IFI_SR_RD_I_BOFF 0x00000001 // Busoff State
#define IFI_SR_RD_I_WARN 0x00000002 // warning state
#define IFI_SR_RD_EACT   0x00000004 // error active
#define IFI_SR_RD_EPAS   0x00000008 // error passive
#define IFI_SR_RD_SIL    0x00000010 // silent mode

#define IFI_SR_RD_I_TOK  0x00004000 // Transmit ok INT
#define IFI_SR_RD_MSG    0x00008000 // Message in Transmit

#define IFI_SR_RD_I_ERR  0x00010000 // Error Exist Int
#define IFI_SR_RD_HPM    0x00200000 // High Priority Message Active
#define IFI_SR_RD_TBF    0x00400000 // Transmit buffer full
#define IFI_SR_RD_RBF    0x00800000 // Receive buffer full

#define IFI_SR_RD_I_TBE  0x01000000 // Transmit buffer empty INT
#define IFI_SR_RD_I_TB0  0x02000000 // Transmit buffer overflow INT
#define IFI_SR_RD_I_TBF  0x04000000 // Transmit buffer full INT
#define IFI_SR_RD_I_RNE  0x08000000 // Receive buffer not empty INT
#define IFI_SR_RD_I_RBO  0x10000000 // Receive buffer overflow INT
#define IFI_SR_RD_I_RBF  0x20000000 // Receive buffer full INT
#define IFI_SR_RD_TB     0x40000000 // Transmit busy
#define IFI_SR_RD_RB     0x80000000 // Receive busy

#define IFI_SR_WR_I_BOFF  0x00000001 // Busoff INT reset
#define IFI_SR_WR_I_WARN  0x00000002 // Error warning INT reset
#define IFI_SR_WR_SIL     0x00000080 // RecFifo read next value

#define IFI_SR_WR_I_TOK   0x00004000 // Transmit ok INT reset

#define IFI_SR_WR_I_ERR   0x00010000 // Error Exist Int reset

#define IFI_SR_WR_I_TBE   0x01000000 // Transmit buffer empty INT reset
#define IFI_SR_WR_I_TBO   0x02000000 // Transmit buffer overflow INT reset
#define IFI_SR_WR_I_TBF   0x04000000 // Transmit buffer full INT reset
#define IFI_SR_WR_I_RNE   0x08000000 // Receive buffer not empty INT reset
#define IFI_SR_WR_I_RBO   0x10000000 // Receive buffer overflow INT reset
#define IFI_SR_WR_I_RBF   0x20000000 // Receive buffer full INT reset

// mask for to reset Interrupt status
#define IFI_SR_RD_MASK  ( IFI_SR_RD_I_BOFF | IFI_SR_RD_I_WARN | IFI_SR_RD_I_TOK | IFI_SR_RD_I_ERR | \
                          IFI_SR_RD_I_TBE  | IFI_SR_RD_I_TB0  | IFI_SR_RD_I_TBF | \
                          IFI_SR_RD_I_RNE  | IFI_SR_RD_I_RBO  | IFI_SR_RD_I_RBF)

#define IFI_SR_RDI      ( IFI_SR_RD_I_RBF  | IFI_SR_RD_I_RBF  | IFI_SR_RD_I_RNE | IFI_SR_RD_RB )

#define IFI_SR_TDI      ( IFI_SR_RD_I_TBE  | IFI_SR_RD_I_TOK  )
#define IFI_SR_TDI_NOT  ( IFI_SR_RD_I_TBF  | IFI_SR_RD_I_TB0  | IFI_SR_RD_TBF )

//////////////////////////////////////////////////////////////////////////
// IFI error counter register Bits ( Register 7 )
//////////////////////////////////////////////////////////////////////////
#define IFI_R7_RD_TE      0x000001FF // Transmit Error Counter
#define IFI_R7_RD_RE      0x00FF0000 // Receive Error Counter

#define IFI_R7_RD_TE_S    0
#define IFI_R7_RD_RE_S    16

//////////////////////////////////////////////////////////////////////////
// IFI Version Register Bits ( Register 8 )
//////////////////////////////////////////////////////////////////////////
#define IFI_R8_RD_CR     0x000000FF // core revision
#define IFI_R8_RD_Q      0x0000FF00 // quartus
#define IFI_R8_RD_Y      0x00FF0000 // year
#define IFI_R8_RD_M      0xFF000000 // month

#define IFI_R8_RD_CR_S    0
#define IFI_R8_RD_Q_S     8
#define IFI_R8_RD_Y_S    16
#define IFI_R8_RD_M_S    24

//////////////////////////////////////////////////////////////////////////
// IFI Fifo Register Bits ( Register 9 )
//////////////////////////////////////////////////////////////////////////
#define IFI_R9_RD_TBC    0x000001FF // transmit buffer Counter
#define IFI_R9_RD_RBC    0x1FFF0000 // receive buffer Counter

#define IFI_R9_WR_RRB    0x80000000 // reset receive buffer
#define IFI_R9_WR_RTB    0x00008000 // reset transmit Counter

#define IFI_R9_RD_TBC_S  0
#define IFI_R9_RD_RBC_S  16


//////////////////////////////////////////////////////////////////////////
// IFI parameter Register Bits ( Register 12 )
//////////////////////////////////////////////////////////////////////////
#define IFI_R12_RD_TIM   0x00000001 // timer on off
#define IFI_R12_RD_FNO   0x000000F0 // filter number
#define IFI_R12_RD_TBS   0x00000F00 // transmit buffer size
#define IFI_R12_RD_RBS   0x0000F000 // receive buffer size
#define IFI_R12_RD_DBW   0x000F0000 // width
#define IFI_R12_RD_M_N   0x00100000 // Mhz or ns
#define IFI_R12_RD_CLK   0xFF000000 // clock

#define IFI_R12_RD_TIM_S  0
#define IFI_R12_RD_FNO_S  4
#define IFI_R12_RD_TBS_S  8
#define IFI_R12_RD_RBS_S 12
#define IFI_R12_RD_DBW_S 16
#define IFI_R12_RD_M_N_S 20
#define IFI_R12_RD_CLK_S 24


#define IFI_R12_FNO_VAL8     8     //Filter value  8 =>  64 Filters and Masks
#define IFI_R12_FNO_CONV8   64     //Filter value  8 =>  64 Filters and Masks
#define IFI_R12_FNO_VAL9     9     //Filter value  9 => 128 Filters and Masks
#define IFI_R12_FNO_CONV9  128     //Filter value  9 => 128 Filters and Masks
#define IFI_R12_FNO_VAL10   10     //Filter value 10 => 256 Filters and Masks
#define IFI_R12_FNO_CONV10 256     //Filter value 10 => 256 Filters and Masks


//////////////////////////////////////////////////////////////////////////
// IFI error register Bits ( Register 13 )
//////////////////////////////////////////////////////////////////////////
#define IFI_R13_RD_ERR_FIRST   0x000000FF // error first
#define IFI_R13_RD_ERR_ALL     0x0000FF00 // error all
#define IFI_R13_RD_BIT_POS     0x00FF0000 // bit position
#define IFI_R13_RD_SSM         0x01000000 // single shot mode enable
#define IFI_R13_RD_ERR_ENA     0x80000000 // error register enable

#define IFI_R13_WR_SSM         0x01000000 // single shot mode enable
#define IFI_R13_WR_ERR_RST     0x40000000 // error register reset
#define IFI_R13_WR_ERR_ENA     0x80000000 // error register enable

#define IFI_R13_ERR_FIRST_S    0
#define IFI_R13_ERR_ALL_S      8
#define IFI_R13_BIT_POS_S     16


#define IFI_R13_CANERR_OVERLOAD 0x01
#define IFI_R13_CANERR_ACK      0x02
#define IFI_R13_CANERR_BIT0     0x04
#define IFI_R13_CANERR_BIT1     0x08
#define IFI_R13_CANERR_STUFF    0x10
#define IFI_R13_CANERR_CRC      0x20
#define IFI_R13_CANERR_FORM     0x40
#define IFI_R13_CANERR_RES      0x80

//////////////////////////////////////////////////////////////////////////
// IFI filter mask Bits ( Register 512 )
//////////////////////////////////////////////////////////////////////////
#define IFI_R512_RD_FMASK      0x1FFFFFFF // filter mask
#define IFI_R512_RD_FMEXT      0x20000000 // extended ?
#define IFI_R512_RD_FMVALID    0x80000000 // valid ?

//////////////////////////////////////////////////////////////////////////
// IFI identifier bits ( Register 513 )
//////////////////////////////////////////////////////////////////////////
#define IFI_R513_RD_FID        0x1FFFFFFF // filter mask
#define IFI_R513_RD_FIDEXT     0x20000000 // extended ?
#define IFI_R513_RD_FIDVALID   0x80000000 // valid ?

#endif
