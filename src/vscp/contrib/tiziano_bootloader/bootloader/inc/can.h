// *****************************************************************************
// FILENAME:      can.h                                                      
//                                                                           
// VERSION:                                                                  
//                                                                           
// DESCRIPTION:   Header File for CAN Driver CAN_Driver.c              
//                                                                          	
// TOOLS:         KEIL uVision3                                          	
//                ARM Development Suite ADS1.2                               
//                                                                           
//                                                                           
// REVISION HISTORY:                                                         
// Version  Author          Date          Remarks                            
// 1.0      Anthony Xie     10/10/2005    - -                                
// *****************************************************************************

// ---------------------------------------------------------------------------
// @file
// CAN Controller Driver Header File
// ---------------------------------------------------------------------------

#ifndef __CAN_H
#define __CAN_H

// ****************************************************************************
// Standard include files                                                    
// ****************************************************************************

// ****************************************************************************
// Project include files                                                     
// ****************************************************************************
#include "config.h"
#include "cangaf.h"


// ****************************************************************************
// Types                                                                     
// ****************************************************************************

// ****************************************************************************
// Defines for Global Acceptance Filter                                                                   
// ****************************************************************************

// Size of Acceptance Filter Sections

#define CAN_SIZE_OF_ACFMEMORY   2048 // Size of SJA1000 Look_up Table RAM    

// Pre-defined CAN Acceptance Filter RAM Address

//#define CAN_ACFMEM_BASE         CAN_ACCEPT_BASE_ADDR//0xE0038000
#define CAN_ACFMEM_BASE 0xE0038000

// ---------------------------------------------------------------------------
// Sections can be enabled or disabled to the application specific needs	  
// ---------------------------------------------------------------------------
//#define CAN_STD_INDIVIDUAL
//#define CAN_STD_GROUP      
#define CAN_EXT_INDIVIDUAL 
#define CAN_EXT_GROUP      

// ---------------------------------------------------------------------------
// Acceptance Filter Configuration Table Constants                           
// ---------------------------------------------------------------------------

#define CAN_SCC_1                     0	// SCC: Source CAN Cannel
#define CAN_SCC_2                     1

// ---------------------------------------------------------------------------
// Data structure for a CAN message to be transmitted
// ---------------------------------------------------------------------------

typedef struct
{
   UINT32   TFI;              // 0x8xxxxxxx for 29bits length ID, 0x0xxxxxxx for 11 bits
   UINT32   ID;
   UINT32   DataField[2];

} CAN_TX, *pCAN_TX;

// ---------------------------------------------------------------------------
// Data structure for a CAN message to be received
// ---------------------------------------------------------------------------

typedef struct
{
	UINT32   RFS;
	UINT32   ID;
	UINT32   DataField[2];

} CAN_RX, *pCAN_RX;

// ****************************************************************************
// Defines                                                                   
// ****************************************************************************

// ---------------------------------------------------------------------------
// Definition of return values
// ---------------------------------------------------------------------------
#define CAN_INITIAL                                    0

#define CAN_OK                                         5   
#define CAN_ERR                                  	   10   

#define CAN_ERR_WRONG_CAN_CHANNEL_NUMBER               30
#define CAN_ERR_WRONG_CAN_TxBUFFER_NUMBER              31
#define CAN_ERR_TRANSMIT_BUFFER1_NOT_FREE              40
#define CAN_ERR_TRANSMIT_BUFFER2_NOT_FREE              50
#define CAN_ERR_TRANSMIT_BUFFER3_NOT_FREE              60

#define CAN_ERR_TABLE_ERROR_IN_STD_INDIVIDUAL_SECTION  90
#define CAN_ERR_TABLE_ERROR_IN_STD_GROUP_SECTION       100
#define CAN_ERR_TABLE_ERROR_IN_EXT_INDIVIDUAL_SECTION  110
#define CAN_ERR_TABLE_ERROR_IN_EXT_GROUP_SECTION       120
#define CAN_ERR_NOT_ENOUGH_MEMORY_SPACE                130

///////////////////////////////////////////////////////////////////////////////
// Bit Timing Values
///////////////////////////////////////////////////////////////////////////////
//
// BRP+1 = Fpclk/(CANBitRate * QUANTAValue)
// QUANTAValue = 1 + (Tseg1+1) + (Tseg2+1)
// QUANTA value varies based on the Fpclk and sample point
// e.g. (1) sample point is 87.5%, Fpclk is 48Mhz
// the QUANTA should be 16
// (2) sample point is 90%, Fpclk is 12.5Mhz
// the QUANTA should be 10 
// Fpclk = Fclk /APBDIV
// or
//  BitRate = Fcclk/(APBDIV * (BRP+1) * ((Tseg1+1)+(Tseg2+1)+1))
// 	
// Here are some popular bit timing settings for LPC23xx, google on "SJA1000"
// CAN bit timing, the same IP used inside LPC2000 CAN controller. There are several 
// bit timing calculators on the internet. 
// http://www.port.de/engl/canprod/sv_req_form.html
// http://www.kvaser.com/can/index.htm

#define CAN_BITRATE		BITRATE125K18MHZ

// Bit Timing Values for 18MHz(72Mhz CCLK) clk frequency
#define BITRATE125K18MHZ		0x001C0008
#define BITRATE250K18MHZ		0x002D4003
#define BITRATE500K18MHZ		0x002D4001//0x002D1001
#define BITRATE1000K18MHZ		0x002D4000

// Pre-defined CAN channel identifier
#define CANCHANNEL_1            0x01
#define CANCHANNEL_2            0x02

// Pre-defined transmit buffer identifier
#define TXB_1                   0x01
#define TXB_2                   0x02
#define TXB_3                   0x03

// Pre-defined Acceptance filter modes
#define ACC_ON                  0x00
#define ACC_OFF                 0x01
#define ACC_BYPASS              0x03

// Pre-defined CAN controller operating modes
#define CAN_RESET_MODE           0x01
#define CAN_OPERATING_MODE       0x00
#define CAN_LISTENONLY_MODE      0x02
#define CAN_SELFTEST_MODE        0x04

//*****************************************************************************
// External Global Variables                                                 
//*****************************************************************************
extern CAN_TX CAN_Send_Data[];
extern CAN_RX CAN_Rcv_Data[];

extern const CAN_ACF gkCAN_StdIndividualSection[];
extern const CAN_ACF gkCAN_StdGroupSection[];
extern const CAN_ACFx gkCAN_ExtIndividualSection[];
extern const CAN_ACFx gkCAN_ExtGroupSection[];

//*****************************************************************************
// External Functions                                                        
//*****************************************************************************


// Calling this function initializes the selected CAN Controller with the
// specified Bit Timing and puts the CAN Controller into Operating Mode.
//
// @param[in]   canChannel      Channel number of CAN Controller
// @param[in]   canBitrate      Pre-defined CAN Bit Timing value
//
// @return      The following error codes can be returned:
//              - CAN_OK,
//              on successful completion.
CANSTATUS init_can(UINT32 canChannel, UINT32 canBitrate);
                             
// Calling this function changes the CAN Controller mode.
//
// @param[in]   canChannel      Channel number of CAN Controller
// @param[in]   CANMode         desired CAN controller mode
//
// @return      The following error codes can be returned;
//              - CAN_OK,
//                on successful completion.
//              - CAN_ERR in case of an error.
CANSTATUS can_setmode(UINT32 canChannel, UINT8 CANMode);
                                     
// Calling this function changes the Acceptance Filter mode.
//
// @param[in]   ACFMode         desired acceptance filter mode
//
// @return      The following error codes can be returned;
//              - CAN_OK,
//                on successful completion.
//              - CAN_ERR in case of an error.
CANSTATUS can_setafcmode(UINT8 ACFMode);

// Calling this function forces the CPU to copy transmit data from
// User RAM into a certain transmit buffer. A 'buffer not free'
// return code is given if a certain transmit buffers is occupied.
//
// @param[in]   canChannel      Channel number of CAN Controller
// @param[in]   pTransmitBuf    pointer to a Transmit Buffer in User RAM
// @param[in]   txbuffer        Transmit Buffer number: 1,2 or 3
//
// @return      The following error codes can be returned;
//              - CAN_OK,
//              and in case of errors:
//              - CAN_ERR_WRONG_CAN_CHANNEL_NUMBER
//              - CAN_ERR
//              - CAN_ERR_TRANSMIT_BUFFER1_NOT_FREE
//              - CAN_ERR_TRANSMIT_BUFFER2_NOT_FREE
//              - CAN_ERR_TRANSMIT_BUFFER3_NOT_FREE
CANSTATUS can_sendmsg(UINT32 canChannel, pCAN_TX pTransmitBuf, UINT32 txbuffer);

// Calling this function loads the Acceptance Filter Look_up table RAM with
// pre-defined values stored in CAN_cfg.h. Depending on the number of
// pre-defined identifiers the register start addresses for the Acceptance Filter
// RAM are calculated and initialised automatically.
//
// @return      The following error codes can be returned;
//              - CAN_OK,
//              and in case of errors:
//              - CAN_ERR
CANSTATUS can_loadacf(void);

void open_can(void);
UINT32 can_outmsg(UINT32 can_channel, pCAN_TX pTxBuf);
UINT32 can_receivemessage(UINT32 canChannel,CAN_RX *pCanMsg);
UINT32 can_sendmessage(UINT32 canChannel, pCAN_TX pCanMsg);
#endif //__CAN_H
