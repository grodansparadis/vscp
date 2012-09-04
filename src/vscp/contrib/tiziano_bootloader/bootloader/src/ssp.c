// ******************************************************************
//
// DESCRIPTION: 
// AUTHOR: 		
//
// HISTORY:    
//
// ******************************************************************

#include "LPC23xx.h"			
#include "config.h"
#include "vic.h"
#include "irq.h"
#include "ssp.h"

// statistics of all the interrupts 
UINT32 interruptRxStat = 0;
volatile UINT32 interruptOverRunStat = 0;
volatile UINT32 interruptRxTimeoutStat = 0;
volatile UINT32 int_cnt_last_fifo = 5;

//****************************************************************************
// Function name:		SSP0Handler
//
// Descriptions:		SSP port is used for SPI communication.
//						SSP0(SSP) interrupt handler
//						The algorithm is, if RXFIFO is at least half full, 
//						start receive until it's empty; if TXFIFO is at least
//						half empty, start transmit until it's full.
//						This will maximize the use of both FIFOs and performance.
//
// parameters:			None
// Returned value:		None
// 
//*****************************************************************************
	
void __attribute__ ((interrupt("IRQ"))) SSP0Handler(void)
{
	UINT32 regValue;
	  
	enableIRQ();				// handles nested interrupt 
	regValue = SSP0MIS;
	if ( regValue & SSPMIS_RORMIS )	// Receive overrun interrupt 
	{
		interruptOverRunStat++;
		SSP0ICR = SSPICR_RORIC;		// clear interrupt 
	}
	if ( regValue & SSPMIS_RTMIS )	// Receive timeout interrupt 
	{
		interruptRxTimeoutStat++;
		SSP0ICR = SSPICR_RTIC;		// clear interrupt 
	}

	// please be aware that, in main and ISR, CurrentRxIndex and CurrentTxIndex
	// are shared as global variables. It may create some race condition that main
	// and ISR manipulate these variables at the same time. SSPSR_BSY checking (polling)
	// in both main and ISR could prevent this kind of race condition 
	if ( regValue & SSPMIS_RXMIS )	// Rx at least half full 
	{
		interruptRxStat++;			// receive until it's empty 		
	}
  
  	if ( regValue & SSPMIS_TXMIS )	// Rx at least half full 
  	{
	  	if ( SSP0SR & SSPSR_TFE )	// tx  until it's empty 	
	   	{  int_cnt_last_fifo--;
		  	if (int_cnt_last_fifo==0)
		  	{
		  		FIO0SET = SSP0_SEL;	// set SSEL to high 
		   		FIO0SET = SPID_CS1;	// set SPID_CS1 to high 
		   		SSP0IMSC = 0;
		   		int_cnt_last_fifo=5; 
		  	}
	   	}
  	}  
  								
  	disableIRQ();
  	VICVectAddr = 0;		// Acknowledge Interrupt 
}

//****************************************************************************
// Function name:		SSP0Init
//
// Descriptions:		SSP0 port initialization routine
//				
// parameters:			None
// Returned value:		true or false, if the interrupt handler
//						can't be installed correctly, return false.
// 
//****************************************************************************

UINT32 SSP0Init( void )
{
  	UINT8 i, Dummy;

  	// enable clock to SSP0 for security reason. By default, it's enabled already 
  	PCONP |= PCSSP0;

	// Configure PIN connect block 
	// bit 32, 54, 76 are 0x10, bit 98 are 0x00 
	// port 0 bits 17, 18, 19, 20 are SSP port SCK0, MISO0, MOSI0, and SSEL0 
	// set SSEL to GPIO pin that you will have the totoal freedom to set/reset 
  	// the SPI chip-select pin 
  	// When DMA is enabled, enable USE_CS, or be careful with SSP0_SEL pin,
  	// clear SSP0_SEL before DMA starts, and set SSP0_SEL after finishing.  

  	PINSEL3 &= 0xFFFC3CFF;	// mask
  	PINSEL3 |= PINSEL3_P1_20_SCK0 | PINSEL3_P1_23_MISO0 | PINSEL3_P1_24_MOSI0;
  
  	FIO1DIR 	|= SSP0_SEL;		// SSEL is output 
  	FIO1MASK 	&=~SSP0_SEL;
  	FIO1SET 	 = SSP0_SEL;		// set SSEL to high 
   
  	
  	// Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 1, and SCR is 15 
  	SSP0CR0 = 0x07C7;//0x0787;

  	// SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 
  	SSP0CPSR = 0x2;	// speed = 562550 bit/sec

  	for ( i = 0; i < FIFOSIZE; i++ )
  	{
		Dummy = SSP0DR;		// clear the RxFIFO 
  	}

  	if ( install_irq( SPI0_INT, (void *)SSP0Handler, HIGHEST_PRIORITY ) == FALSE )
  	{
		return (FALSE);
  	}
	
   	// Device select as master, SSP Enabled, loopback operational mode 
  	// SSP0CR1 = SSPCR1_LBM | SSPCR1_SSE;  
  	SSP0CR1 =  SSPCR1_SSE;
  	// Set SSPINMS registers to enable interrupts 
  	// enable all error related interrupts 
  	//SSP0IMSC = SSPIMSC_RORIM | SSPIMSC_RTIM;
  	SSP0IMSC = 0;

  	return( TRUE );
}

//****************************************************************************
// Function name:		SSP0Send
//
// Descriptions:		Send a block of data to the SSP0 port, the 
//						first parameter is the buffer pointer, the 2nd 
//						parameter is the block length.
//
// parameters:			buffer pointer, and the block length
// Returned value:		None
// 
//****************************************************************************

void SSP0Send( UINT8 *buf, UINT32 Length )
{
  	UINT32 i;
  	//SSP0DR = 0xC5;
    
  	for ( i = 0; i < Length; i++ )
  	{
		// as long as TNF bit is set (TxFIFO is not full), I can always transmit 
		while ( !(SSP0SR & SSPSR_TNF) );
		SSP0DR = *buf;
		buf++;
		// Wait until the Busy bit is cleared 
		while ( (SSP0SR & SSPSR_BSY) );
  	}
  	
  	return; 
}

//****************************************************************************
// Function name:		SSP0Receive
// Descriptions:		the module will receive a block of data from 
//						the SSP0, the 2nd parameter is the block 
//						length.
// parameters:			buffer pointer, and block length
// Returned value:		None
// 
//****************************************************************************

void SSP0Receive( UINT8 *buf, UINT32 Length )
{
  	UINT32 i;
 
  	for ( i = 0; i < Length; i++ )
  	{
		// As long as Receive FIFO is not empty, I can always receive. 
		// since it's a loopback test, clock is shared for both TX and RX,
		// no need to write dummy byte to get clock to get the data 
		// if it's a peer-to-peer communication, SSPDR needs to be written
		// before a read can take place. 
		while ( !(SSP0SR & SSPSR_RNE) );
		*buf = SSP0DR;
		buf++;
  	}
  	return; 
}

//****************************************************************************
// Function name:		SSP0SetCS
// Descriptions:		 
//						 
//						
// parameters:			
// Returned value:		
// 
//****************************************************************************

void SSP0SetCS( void )
{
	FIO1SET	= SSP0_SEL; 
}

//****************************************************************************
// Function name:		SSP0ClearCS
// Descriptions:		 
//						 
//						
// parameters:			
// Returned value:		
// 
//****************************************************************************

void SSP0ClrCS( void )
{
	FIO1CLR	= SSP0_SEL; 
}

//****************************************************************************
// Function name:		SSP0ClrFIFO
// Descriptions:		 
//						 
//						
// parameters:			
// Returned value:		
// 
//****************************************************************************

void SSP0ClrFIFO( void )
{
	UINT8 data;
	
	while ( SSP0SR & SSPSR_RNE )
	{
		data = SSP0DR;
	}
}

