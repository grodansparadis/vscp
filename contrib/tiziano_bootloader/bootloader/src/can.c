// *****************************************************************************
// FILENAME:                                                    
//                                                                           
// VERSION:                                                              
//                                                                          
// DESCRIPTION:                                   
//                                                                           
// TOOLS:                             		  
//                                                                           
// REVISION HISTORY                                                          
// Version  Author           Date        Remarks							  
//                                      
// *****************************************************************************

 
// @file
// These CAN Controller driver routines are designed to provide the
// application programmer with a higher level interface for the communication
// with each CAN Controller module.
// The CAN Controller Driver routines perform the following functions:
//    Initialization of the CAN Controller
//    Configuration of the CAN Controller for various baud rates
//    Preparation and transmission of CAN messages
//    Store received CAN messages in appropriate message buffers
//    Providing pre-defined values for a set of bit-rates for various
//        clock frequencies
//    Mode switching of the CAN Controller
//    Easy read/write access to CAN Controller registers


// ****************************************************************************
// Standard include files                                                    
// ****************************************************************************

// ****************************************************************************
// Project include files                                                     
// ****************************************************************************

#include  "LPC23xx.h"
#include  "config.h"
#include  "irq.h"
#include  "vic.h"
#include  "uart.h"
#include  "can.h"
#include  "utils.h"
#include  "cangaf.h"   // Includes Global Acceptance Filter   

// ****************************************************************************
// Types                                                                     
// ****************************************************************************
#define CAN_MSGBUFF_RX_SIZE	8
// ****************************************************************************
// Defines                                                                   
// ****************************************************************************

// ****************************************************************************
// External Functions                                                        
// ****************************************************************************
		
// ****************************************************************************
// External Variables                                                        
// ****************************************************************************

// ****************************************************************************
// Global Variables                                                          
// ****************************************************************************
CAN_RX MsgBuf_RX1[CAN_MSGBUFF_RX_SIZE], MsgBuf_RX2[CAN_MSGBUFF_RX_SIZE]; // TX and RX Buffers for CAN message
UINT32 p_rx1_rd = 0, p_rx1_wr = 0;
UINT32 p_rx2_rd = 0, p_rx2_wr = 0;
volatile UINT32 CAN1RxDone, CAN2RxDone;
UINT32 CANStatus;
UINT32 CAN1RxCount = 0,CAN2RxCount = 0; 
UINT32 CAN1ErrCount = 0, CAN2ErrCount = 0;
UINT32 CAN1TxWaitCount = 0; 



#ifdef CAN_STD_INDIVIDUAL
// Please, add or change the list of pre-defined   
// entries to the application specific needs       
const CAN_ACF  gkCAN_StdIndividualSection[] =
   {
     // Channel(1-2) ,              11-bit Identifier ( <7FF Double than ram size ) 
     {CAN_SCC_1,       0x0010},
     {CAN_SCC_1,       0x01AC},
     {CAN_SCC_1,       0x0245},
     {CAN_SCC_1,       0x025F}
   };								   

        
#define CAN_NUMBER_OF_STD_INDIVIDUAL_IDS (\
        sizeof gkCAN_StdIndividualSection /\
        sizeof gkCAN_StdIndividualSection[0])

#else
#define CAN_NUMBER_OF_STD_INDIVIDUAL_IDS  0
#endif


#ifdef CAN_STD_GROUP 
// Please, add or change the list of pre-defined   
// entries to the application specific needs       
const CAN_ACF gkCAN_StdGroupSection[] =
   {
     // Channel                11-bit Identifier 
     {CAN_SCC_1,       0x0300},        // lower bound, Group 1 
     {CAN_SCC_1,       0x037F},        // upper bound, Group 1 
	 {CAN_SCC_1,       0x0400},        // lower bound, Group 2 
	 {CAN_SCC_1,       0x047F}         // upper bound, Group 1 
   };
#define CAN_NUMBER_OF_STD_GROUP_IDS (\
        sizeof gkCAN_StdGroupSection /\
        sizeof gkCAN_StdGroupSection[0])

#else
#define CAN_NUMBER_OF_STD_GROUP_IDS  0
#endif

#ifdef CAN_EXT_INDIVIDUAL
// Please, add or change the list of pre-defined   
// entries to the application specific needs

const CAN_ACFx gkCAN_ExtIndividualSection[] =
   {
     // Channel                29-bit Identifier   ( =< 0x1FFFFFFF) 
     {CAN_SCC_1,       0x00002400},
     {CAN_SCC_1,       0x00002600},
	 {CAN_SCC_1,       0x00002700},
	 {CAN_SCC_1,       0x00002800},
	 {CAN_SCC_1,       0x0C140900}
   };

/*
const CAN_ACFx gkCAN_ExtIndividualSection[] =
   {
     // Channel                29-bit Identifier   ( =< 0x1FFFFFFF) 
     {CAN_SCC_1,       0x00000C00},
     {CAN_SCC_1,       0x00000D00},
	 {CAN_SCC_1,       0x00000E00},
	 {CAN_SCC_1,       0x00000F00},
	 {CAN_SCC_1,       0x00001000},
	 {CAN_SCC_1,       0x00001100},
	 {CAN_SCC_1,       0x00001200},
	 {CAN_SCC_1,       0x00001300},
	 {CAN_SCC_1,       0x00001400},
	 {CAN_SCC_1,       0x00001500},
	 {CAN_SCC_1,       0x00001600},
	 {CAN_SCC_1,       0x00001700},
	 {CAN_SCC_1,       0x00001800},
	 {CAN_SCC_1,       0x00001900},
	 {CAN_SCC_1,       0x00001A00},
	 {CAN_SCC_1,       0x00001B00},
	 {CAN_SCC_1,       0x00001C00}
   };
*/

#define CAN_NUMBER_OF_EXT_INDIVIDUAL_IDS (\
        sizeof gkCAN_ExtIndividualSection /\
        sizeof gkCAN_ExtIndividualSection[0])

#else
#define CAN_NUMBER_OF_EXT_INDIVIDUAL_IDS 0
#endif


#ifdef CAN_EXT_GROUP
// Please, add or change the list of pre-defined   
// entries to the application specific needs       
const CAN_ACFx gkCAN_ExtGroupSection[] =
   {
     // Channel                29-bit Identifier ( =< 0x1FFFFFFF) 
     {CAN_SCC_1,       0x00000000},     // lower bound, Group 1 
     {CAN_SCC_1,       0x0000FFFF},     // upper bound, Group 1
     {CAN_SCC_1,       0x0C1E1B00},//????{CAN_SCC_1,       0x0C001900},     // lower bound, Group 2 
     {CAN_SCC_1,       0x0C1E1BFF} //????{CAN_SCC_1,       0x0C0019FF}      // upper bound, Group 2

   };
#define CAN_NUMBER_OF_EXT_GROUP_IDS (\
        sizeof gkCAN_ExtGroupSection /\
        sizeof gkCAN_ExtGroupSection[0])

#else
#define CAN_NUMBER_OF_EXT_GROUP_IDS      0
#endif


// **************************************************************************
// Local Functions Prototypes                                                
// **************************************************************************
void __attribute__ ((interrupt("IRQ"))) irq_can(void);

// **************************************************************************
//                                                                           
// Implementation                                                            
//                                                                           
// **************************************************************************



// **************************************************************************
// FUNCTION:    init_can                                          
//                                                                           
// DESCRIPTION: Initialises the CAN Controller for channel 1,2,3,4          
//              - Enter the Reset Mode of the selected CAN Controller Unit   
//              - CAN Bit Timing                                             
//              - Enter Normal Operating Mode of the CAN Controller          
//                                                                           
// GLOBAL VARIABLES USED:                                                    
//                                                                           
// PARAMETERS:  canChannel: 1,2,3,4                                     
//              canBitrate: Register value, see also global.h for defines    
//                                                                           
// RETURN:      CANSTATUS: Status of operation                           
//                                                                           
//              CAN_OK - successful                                 
//                                                                           
// **************************************************************************

CANSTATUS init_can ( UINT32 canChannel, UINT32   canBitrate )
{	
   switch (canChannel)  // CAN Channel 
   {		
		case CANCHANNEL_1: 
    	  	PCONP |= PCPCAN1;	// Enable clock to the peripheral
    	  	// Enter Reset Mode 
			CAN1MOD = CAN_RESET_MODE;
			// Reset error counter when CANxMOD is in reset
			CAN1GSR = 0;
            // Write into Bus Timing Register 
			CAN1BTR = canBitrate;
            // Enter Operating Mode 
			CAN1MOD = CAN_SELFTEST_MODE;//CAN_OPERATING_MODE;
            break;

      	case CANCHANNEL_2: 
    	    PCONP |= PCPCAN2;	// Enable clock to the peripheral
    	  	// Enter Reset Mode 
			CAN2MOD = CAN_RESET_MODE;
			// Reset error counter when CANxMOD is in reset
			CAN1GSR = 0;
            // Write into Bus Timing Register 
			CAN2BTR = canBitrate;
            // Enter Operating Mode 
			CAN2MOD = CAN_OPERATING_MODE;
            break;

      default:// wrong channel number 
           	return (CAN_ERR_WRONG_CAN_CHANNEL_NUMBER);
	}
   	return (CAN_OK);
} // init_can () 


// **************************************************************************
// FUNCTION:    can_setafcmode                                       
//                                                                           
// DESCRIPTION: This function sets the Acceptance Filter mode                
//                                                                           
// GLOBAL VARIABLES USED: - -                                                
//                                                                           
// PARAMETERS:  ACFMode:  0 = Acceptance Filter On                           
//                        1 = Acceptance Filter Off  (reset status)                        
//                        3 = Acceptance Filter Bypass                       
//                        4 = Acceptance Filter On + FullCAN Mode enabled    
//                                                                           
// RETURN:      CANSTATUS: Status of operation                           
//                                                                           
//              CAN_OK     - successful                             
//                                                                           
//              CAN_ERR    - this mode is not supported             
//                                                                           
// **************************************************************************

CANSTATUS can_setafcmode(UINT8 ACFMode )
{
   switch (ACFMode)
   {
      case 0: // Acceptance Filter On 
			  CAN_AFMR = 0x00000000;
              break;
      case 1: // Acceptance Filter Off 
			  CAN_AFMR = 0x00000001;
              break;
      case 3: // Acceptance Filter Bypass 
 			  CAN_AFMR = 0x00000003;
              break;
      case 4: // Acceptance Filter FullCAN 
			  CAN_AFMR = 0x00000004;
              break;

      default:// Not supported 
              return (CAN_ERR);

   }
   return (CAN_OK);

} // can_setafcmode() 

// **************************************************************************
// FUNCTION:    can_setmode                                       
//                                                                           
// DESCRIPTION: This function sets the CAN Controller mode                   
//                                                                           
// GLOBAL VARIABLES USED:                                                    
//                                                                           
// PARAMETERS:  canChannel: 1,2,3,4                                      
//              CANMode   : 0 = Operating Mode                               
//                          1 = Reset Mode                                   
//                          2 = Listen Only Mode                             
//                          4 = Self Test Mode                               
//                                                                           
// RETURN:      CANSTATUS: Status of operation                           
//                                                                           
//              CAN_OK       - successful                           
//                                                                           
//              CAN_ERR      - this mode is not supported           
//                                                                           
// **************************************************************************

CANSTATUS can_setmode(UINT32 canChannel, UINT8 CANMode )
{
   UINT32 offset;

   offset = (canChannel-1) * 0x00004000;

   if ( (canChannel==0) || (canChannel>2))
   {
      return (CAN_ERR_WRONG_CAN_CHANNEL_NUMBER);
   }

   switch (CANMode)
   {
      case 0: // Operating Mode 
              OUTW((UINT32)&CAN1MOD + offset, CAN_OPERATING_MODE);
              break;
      case 1: // Reset Mode 
              OUTW((UINT32)&CAN1MOD + offset, CAN_RESET_MODE);
              break;
      case 2: // Listen Only Mode 
              OUTW((UINT32)&CAN1MOD + offset, CAN_RESET_MODE);
              OUTW((UINT32)&CAN1MOD + offset, CAN_LISTENONLY_MODE);
              break;
      case 4: // Self Test Mode 
              OUTW((UINT32)&CAN1MOD + offset, CAN_RESET_MODE);
              OUTW((UINT32)&CAN1MOD + offset, CAN_SELFTEST_MODE);
              break;

      default:// Not supported 
              return (CAN_ERR);

   }
   return (CAN_OK);

} // can_setafcmode() 

// **************************************************************************
// FUNCTION:    can_loadacf                             
//                                                                           
// DESCRIPTION: The function hwCAN_LoadAcceptanceFilter configures   
//              the Acceptance Filter Memory. Filter constants which are     
//              pre-defined in the configuration file CAN_cfg.h      
//              are used for programming.                                    
//              In addition to the filter memory configuration, this         
//              function also configures the acceptance filter start address 
//              registers according to the number of CAN identifiers.        
//                                                                           
// GLOBAL VARIABLES USED:                                                    
//                                                                           
// PARAMETERS:                                                               
//                                                                           
// RETURN:      CANSTATUS: Status of operation                           
//                                                                           
//              CAN_OK     - successful                             
//                                                                           
//              CAN_ERR    - not successful -> table error          
// **************************************************************************

CANSTATUS can_loadacf (void)
{
	UINT32 address;
	UINT32 memory_address;
	UINT32 IDcount;
	UINT32 i;
	UINT32 acfword_pre;
	UINT32 acfword_upper;
	UINT32 acfword_lower;

	address = 0;

	// Set Standard Frame Individual Startaddress 
	CAN_SFF_SA = address;

	// ----------------------------------------------------------
	// ----------   Fill Standard Individual Section ------------
	// ----------------------------------------------------------

#ifdef CAN_STD_INDIVIDUAL

	IDcount = CAN_NUMBER_OF_STD_INDIVIDUAL_IDS;
  	acfword_pre = 0;
  	i = 0;

  	while (IDcount != 0)
  	{
     	acfword_lower = (gkCAN_StdIndividualSection[i].ID << 16)
                   + (gkCAN_StdIndividualSection[i].Channel << 29);
     
     	IDcount--;
     	if (IDcount == 0)
     	{
   	  		// odd number of identifiers -> disable ID ! 
   	  		acfword_upper = 0x00001FFF
                      + (gkCAN_StdIndividualSection[i].Channel << 13);
     	}
     	else
     	{
   	  		acfword_upper = (gkCAN_StdIndividualSection[i+1].ID)
                      + (gkCAN_StdIndividualSection[i+1].Channel << 13);
   	  		IDcount--;
     	}

     	// Check ascending numerical order 
     	if ((acfword_lower >> 16) >= acfword_upper)
     	{
   	  		// table error - violation of ascending numerical order
   	  		return(CAN_ERR_TABLE_ERROR_IN_STD_INDIVIDUAL_SECTION);
     	}
     	else
     	{   
   	  		if (acfword_pre >= (acfword_lower >> 16))
   	  		{
   		  		// table error - violation of ascending numerical order
   		  		return(CAN_ERR_TABLE_ERROR_IN_STD_INDIVIDUAL_SECTION);
   	  		}
   	  		else
   	  		{
   		  		// write configuration into Acceptance Filter Memeory 
   		  		OUTW( CAN_ACFMEM_BASE + address, acfword_lower + acfword_upper);
   		  		acfword_pre = acfword_upper;
   	  		}
     	}

     	// increment configuration table index 
     	i = i + 2;
     	// next Acceptance Filter Memory address 
     	address = address + 4;
  	};
#endif

	// Set Standard Frame Group Startaddress 
	CAN_SFF_GRP_SA = address;

// ----------------------------------------------------------
// ----------   Fill Standard Group Section -----------------
// ----------------------------------------------------------

#ifdef CAN_STD_GROUP

	IDcount = CAN_NUMBER_OF_STD_GROUP_IDS;
	acfword_pre = 0;
	i = 0;
	
	while (IDcount != 0)
	{
	  	acfword_lower = (gkCAN_StdGroupSection[i].ID << 16)
	                + (gkCAN_StdIndividualSection[i].Channel << 29);

	  	IDcount--;
	  	if (IDcount == 0)
	  	{
	     	// table error - odd number of identifiers 
	     	// not allowed for group definitions 
	     	return (CAN_ERR_TABLE_ERROR_IN_STD_GROUP_SECTION);
	  	}
	  	else
	  	{
	     	acfword_upper = (gkCAN_StdGroupSection[i+1].ID)
	                   + (gkCAN_StdIndividualSection[i+1].Channel << 13);
	     	IDcount--;
	  	}

	  	// Check ascending numerical order 
	  	if ((acfword_lower >> 16) >= acfword_upper)
	  	{
	     	// table error - violation of ascending numerical order 
	     	return(CAN_ERR_TABLE_ERROR_IN_STD_GROUP_SECTION);
	  	}
	  	else
	  	{
	     	if (acfword_pre >= (acfword_lower >> 16))
	     	{
	        	// table error - violation of ascending numerical order 
	        	return(CAN_ERR_TABLE_ERROR_IN_STD_GROUP_SECTION);
	     	}
	     	else
	     	{
	        	// write configuration into Acceptance Filter Memory 
	        	OUTW(CAN_ACFMEM_BASE + address, acfword_lower + acfword_upper);
	        	acfword_pre = acfword_upper;
	     	}
		}

 		// increment configuration table index 
 		i = i + 2;
 		// next Acceptance Filter Memory address 
 		address = address + 4;
	};

#endif

	// Set Extended Frame Individual Startaddress 
	CAN_EFF_SA = address;

// ----------------------------------------------------------
// ----------   Fill Extended Individual Section ------------
// ----------------------------------------------------------

#ifdef CAN_EXT_INDIVIDUAL

	IDcount = CAN_NUMBER_OF_EXT_INDIVIDUAL_IDS;
	i = 0;
	acfword_pre = 0;

	while (IDcount != 0)
	{
	  	acfword_lower = (gkCAN_ExtIndividualSection[i].IDx)
	                + (gkCAN_ExtIndividualSection[i].Channel << 29);

	  	IDcount--;

	  	// Check ascending numerical order with previous table entity 
	  	if ((i > 0) && (acfword_pre >= acfword_lower))
	  	{
	     	// table error - violation of ascending numerical order 
	     	return(CAN_ERR_TABLE_ERROR_IN_EXT_INDIVIDUAL_SECTION);
	  	}
	  	else
	  	{
	     	OUTW(CAN_ACFMEM_BASE + address, acfword_lower);
	     	// next Acceptance Filter Memory address 
	     	address = address + 4;

	     	// increment configuration table index 
	     	i = i + 1;
	     	acfword_pre = acfword_lower;
	  	}
	};

#endif

	// Set Extended Frame Group Startaddress 
	CAN_EFF_GRP_SA = address;

// ----------------------------------------------------------
// ----------   Fill Extended Group Section -----------------
// ----------------------------------------------------------

#ifdef CAN_EXT_GROUP

	IDcount = CAN_NUMBER_OF_EXT_GROUP_IDS;
	i = 0;
	acfword_pre = 0;

	while (IDcount != 0)
	{
  		acfword_lower = (gkCAN_ExtGroupSection[i].IDx)
                + (gkCAN_ExtGroupSection[i].Channel << 29);

  		// Check ascending numerical order with previous group 
  		if ((i > 0) && (acfword_pre >= acfword_lower))
  		{
     		// table error - violation of ascending numerical order 
     		return (CAN_ERR_TABLE_ERROR_IN_EXT_GROUP_SECTION);
  		}
  		else
  		{
     		IDcount--;

     		if (IDcount == 0)
     		{
        		// table error 'odd number of entities' 
        		return (CAN_ERR_TABLE_ERROR_IN_EXT_GROUP_SECTION);
     		}
     		else
     		{
        		acfword_upper = (gkCAN_ExtGroupSection[i+1].IDx)
                      + (gkCAN_ExtGroupSection[i+1].Channel << 29);
     			// Check ascending numerical order 
     			if (acfword_lower >= acfword_upper)
     			{
        			// table error - violation of ascending numerical order 
        			return (CAN_ERR_TABLE_ERROR_IN_EXT_GROUP_SECTION);
     			}
     			else
     			{
        			// write configuration into Acceptance Filter Memeory 
        			OUTW(CAN_ACFMEM_BASE + address, acfword_lower);
        			// next Acceptance Filter Memory address 
        			address = address + 4;
        			// write configuration into Acceptance Filter Memory 
        			OUTW(CAN_ACFMEM_BASE + address, acfword_upper);
        			// next Acceptance Filter Memory address 
        			address = address + 4;
        			acfword_pre = acfword_upper;
     			}
   			}
		}

 		// increment configuration table index 
 		i = i + 2;
 		IDcount--;
	};

#endif

	// Check Acceptance Filter Memory Space 
	memory_address = address ;

	if (memory_address > CAN_SIZE_OF_ACFMEMORY)
	{
	   	// not enough memory space for CAN Identifiers
		return (CAN_ERR_NOT_ENOUGH_MEMORY_SPACE);
	}
	else
	{
	   	// Set End of Table 
	   	CAN_EOT = address;
	}

	return (CAN_OK);
}

// **************************************************************************
// Function name:		can_isr_rx1
// 
// Descriptions:		CAN Rx1 interrupt handler
// 
// parameters:			None
// Returned value:		None
// 
// **************************************************************************

void can_isr_rx1( void )
{
  	UINT32 *pDest;
  	// initialize destination pointer
	p_rx1_wr++;
	p_rx1_wr &= (CAN_MSGBUFF_RX_SIZE - 1);
  	pDest = (UINT32 *)&MsgBuf_RX1[p_rx1_wr];
  	
  	*pDest = CAN1RFS;  	// Frame

  	pDest++;
  	*pDest = CAN1RID; 	// ID
  	
  	//if(CAN1RID == 0x00001000)
  	//	CAN2RxCount++;
  	//if(CAN2RxCount == 64)
  	//	STROBE_ON;

  	pDest++;
  	*pDest = CAN1RDA; 	// Data A

  	pDest++;
  	*pDest = CAN1RDB; 	// Data B
  	
  	//PrintString((UINT8 const *)"\n\rRX=");
  	//PrintInt((UINT32)CAN1RID);
  	
	
  	CAN1RxDone = TRUE;
  	
  	CAN1CMR = 0x04; 	// release receive buffer
  	return;
}

// **************************************************************************
// Function name:		can_isr_rx2
// 
// Descriptions:		CAN Rx1 interrupt handler
// 
// parameters:			None
// Returned value:		None
// 
// **************************************************************************

void can_isr_rx2( void )
{
  	UINT32 *pDest;

  	// initialize destination pointer
	p_rx2_wr++;
	p_rx2_wr &= (CAN_MSGBUFF_RX_SIZE - 1);
  	pDest = (UINT32 *)&MsgBuf_RX2[p_rx2_wr];
  	*pDest = CAN2RFS;  	// Frame

  	pDest++;
  	*pDest = CAN2RID; 	// ID

  	pDest++;
  	*pDest = CAN2RDA; 	// Data A

  	pDest++;
  	*pDest = CAN2RDB; 	// Data B

  	CAN2RxDone = TRUE;
  	CAN2CMR = 0x04; 	// release receive buffer
  	return;
}

/*****************************************************************************
** Function name:		irq_can
**
** Descriptions:		CAN interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void __attribute__ ((interrupt("IRQ"))) irq_can(void)
{		
	CANStatus = CAN_RX_SR;
  	if ( CANStatus & (1 << 8) )
  	{
		CAN1RxCount++;
		can_isr_rx1();
		//PrintString((UINT8 const *)"\n\rDEBUG:CAN RX");
  	}
  	if ( CAN1GSR & (1 << 6 ) )
  	{
  		// The error count includes both TX and RX 
		CAN1ErrCount = (CAN1GSR >> 16 );
		PrintString((UINT8 const *)"\n\rDEBUG:CAN ERR");
		PrintInt((UINT32)(CAN1ErrCount));
  	}
  	
  	VICVectAddr = 0;		// Acknowledge Interrupt 
}

// **************************************************************************
// FUNCTION:    open_can                                          
//                                                                           
// DESCRIPTION:   
//                 
// GLOBAL VARIABLES USED:                                                    
//                                                                           
// PARAMETERS:  
//                 
//                                                                           
// RETURN:                                 
//                                                                           
// **************************************************************************

void open_can(void)
{
	
//-----------------------------------------------
//Config GPIO
//-----------------------------------------------
  	PINSEL0 |= PINSEL0_P0_0_RD1 | PINSEL0_P0_1_TD1;			// P0.0-RXD, P0.1-TXD CAN1

//-----------------------------------------------
//Config CAN1
//-----------------------------------------------  
  	init_can( CANCHANNEL_1, CAN_BITRATE );

  	can_setafcmode( ACC_BYPASS );
  	can_loadacf();
  	can_setafcmode( ACC_ON );
  	
  	CAN1IER = 0x01 | 0x8;	// Enable receive interrupts

//-----------------------------------------------
//Config VIC, 
//In <Starup.s> enable IRQ by CLR I bit in CPSR
//Vectored interrupt CAN2Rx by slot-2
//-----------------------------------------------  
  
	install_irq( CAN_INT, irq_can, CAN_INT );
}

// **************************************************************************
// FUNCTION:    can_outmsg                                          
//                                                                           
// DESCRIPTION:   
//                 
// GLOBAL VARIABLES USED:                                                    
//                                                                           
// PARAMETERS:  
//                 
//                                                                           
// RETURN:                                 
//                                                                           
// **************************************************************************

UINT32 can_outmsg(UINT32 canChannel, pCAN_TX pTxBuf)
{
	UINT32 Status;
	
	switch(canChannel)
	{
		case CANCHANNEL_1:
			Status = CAN1SR;
			if ( Status & 0x00000004 )
			{
				CAN1TFI1 = pTxBuf->TFI & 0xC00F0000;
				CAN1TID1 = pTxBuf->ID;
				CAN1TDA1 = pTxBuf->DataField[0];
				CAN1TDB1 = pTxBuf->DataField[1];
				CAN1CMR = 0x21;
				//PrintString((UINT8 const *)"\n\rDEBUG:VSCP TX CH1");
				return ( CAN_OK );
			}
			else if ( Status & 0x00000400 )
			{
				CAN1TFI2 = pTxBuf->TFI & 0xC00F0000;
				CAN1TID2 = pTxBuf->ID;
				CAN1TDA2 = pTxBuf->DataField[0];
				CAN1TDB2 = pTxBuf->DataField[1];
				CAN1CMR = 0x41;
				PrintString((UINT8 const *)"\n\rDEBUG:VSCP TX CH2");
				return ( CAN_OK );
			}
			else if ( Status & 0x00040000 )
			{	
				CAN1TFI3 = pTxBuf->TFI & 0xC00F0000;
				CAN1TID3 = pTxBuf->ID;
				CAN1TDA3 = pTxBuf->DataField[0];
				CAN1TDB3 = pTxBuf->DataField[1];
				CAN1CMR = 0x81;
				PrintString((UINT8 const *)"\n\rDEBUG:VSCP TX CH3");
				return ( CAN_OK );
			}
			break;
		case CANCHANNEL_2:
			Status = CAN2SR;
			if ( Status & 0x00000004 )
			{
				CAN2TFI1 = pTxBuf->TFI & 0xC00F0000;
				CAN2TID1 = pTxBuf->ID;
				CAN2TDA1 = pTxBuf->DataField[0];
				CAN2TDB1 = pTxBuf->DataField[1];
				CAN2CMR = 0x21;
				return ( CAN_OK );
			}
			else if ( Status & 0x00000400 )
			{
				CAN2TFI2 = pTxBuf->TFI & 0xC00F0000;
				CAN2TID2 = pTxBuf->ID;
				CAN2TDA2 = pTxBuf->DataField[0];
				CAN2TDB2 = pTxBuf->DataField[1];
				CAN2CMR = 0x41;
				return ( CAN_OK );
			}
			else if ( Status & 0x00040000 )
			{	
				CAN2TFI3 = pTxBuf->TFI & 0xC00F0000;
				CAN2TID3 = pTxBuf->ID;
				CAN2TDA3 = pTxBuf->DataField[0];
				CAN2TDB3 = pTxBuf->DataField[1];
				CAN2CMR = 0x81;
				return ( CAN_OK );
			}
			break;
	}
	
	return ( CAN_ERR );
}

///////////////////////////////////////////////////////////////////////////////
                                 
// **************************************************************************
// FUNCTION:    can_receivemsg                                          
//                                                                           
// DESCRIPTION:   
//                 
// GLOBAL VARIABLES USED:                                                    
//                                                                           
// PARAMETERS:  
//                 
//                                                                           
// RETURN:                                 
//                                                                           
// **************************************************************************

UINT32 can_receivemessage(UINT32 canChannel,CAN_RX *pCanMsg)
{
	switch(canChannel)
	{
		case CANCHANNEL_1: 
			if(p_rx1_wr != p_rx1_rd)
			{
				p_rx1_rd++;
				p_rx1_rd &= (CAN_MSGBUFF_RX_SIZE - 1);
				*pCanMsg = MsgBuf_RX1[p_rx1_rd];
				// debug
				//print_canmessage(pCanMsg);
				//PrintString((UINT8 const *)"\n\rDEBUG:can rx");
				//
				CAN1RxDone = FALSE;
				return TRUE;
			}
			break;
		case CANCHANNEL_2: 
			if(p_rx2_wr != p_rx2_rd)
			{
				p_rx2_rd++;
				p_rx2_rd &= (CAN_MSGBUFF_RX_SIZE - 1);
				*pCanMsg = MsgBuf_RX2[p_rx2_rd];
				CAN2RxDone = FALSE;
				return TRUE;
			}
			break;
	}
	return FALSE;
}

// **************************************************************************
// FUNCTION:    can_sendmessage                                          
//                                                                           
// DESCRIPTION:   
//                 
// GLOBAL VARIABLES USED:                                                    
//                                                                           
// PARAMETERS:  
//                 
//                                                                           
// RETURN:                                 
//                                                                           
// **************************************************************************

UINT32 can_sendmessage(UINT32 canChannel, pCAN_TX pCanMsg)
{
	
	switch(canChannel)
	{
		case CANCHANNEL_1:
			while((CAN1GSR & (1 << 3))==0)	// fino a che tutte le richieste di trasmissione sono state completate
			{
				PrintString((UINT8 const *)"\n\rBOOT1:CAN1GSR=");
				PrintInt((UINT32)CAN1GSR);
			}
			if(can_outmsg (CANCHANNEL_1, pCanMsg) == CAN_OK)
			{
				return TRUE;
			}
			else
			{
				PrintString((UINT8 const *)"\n\rBOOT2:CAN1GSR=");
				PrintInt((UINT32)CAN1GSR);
			}
			break;
		case CANCHANNEL_2:
			while((CAN2GSR & (1 << 3))==0)
			{
				if ( CAN2GSR & (1 << 6 )) 
				{
					if ((CAN2GSR >> 24) & (0xFF))
					{
						CAN2MOD = CAN_SELFTEST_MODE;
						//PrintString((UINT8 const *)"\n\rDEBUG:SELFTEST=");
						//PrintInt((UINT32)CAN1GSR);
						return FALSE;
					}
				}
			}
			if(can_outmsg (CANCHANNEL_2, pCanMsg) == CAN_OK)
			{
				CAN2MOD = CAN_SELFTEST_MODE;
				return TRUE;
			}
			break;
	}
	return FALSE;
}

