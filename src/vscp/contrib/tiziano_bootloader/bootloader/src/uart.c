// *****************************************************************************
// uart.c:  UART API file for NXP LPC23xx/24xx Family Microprocessors
//
// Copyright(C) 2006, NXP Semiconductor
// All rights reserved.
//
// History
// 2006.07.12  ver 1.00    Prelimnary version, first Release
//
// *****************************************************************************

#include "LPC23xx.h"                        
#include "config.h"
#include "vic.h"
#include "irq.h"
#include "upload.h"
#include "uart.h"

#define BUFSIZE UART_BUFSIZE

volatile UINT32 UART0Status, UART1Status;
volatile UINT8 UART0TxEmpty = 1, UART1TxEmpty = 1;
volatile UINT32 UART0Buffer[BUFSIZE], UART1Buffer[BUFSIZE];
volatile UINT32 ptr_uart0_wr = 0, ptr_uart1_wr = 0;
volatile UINT32 ptr_uart0_rd = 0, ptr_uart1_rd = 0;
static int uart_prescaler = UART_PRESCALER;

// **************************************************************************
// Function name:		IRQ_Uart0
//
// Descriptions:		UART0 interrupt handler
//
// parameters:			None
// Returned value:		None
// 
// **************************************************************************
void  __attribute__ ((interrupt("IRQ"))) IRQ_Uart0(void)
{
    UINT8 IIRValue, LSRValue;
    volatile UINT8 Dummy = Dummy;
    
    // VERIFICARE !! enableIRQ();				// handles nested interrupt 
    IIRValue = U0IIR;
    
    IIRValue >>= 1;			// skip pending bit in IIR 
    IIRValue &= 0x07;			// check bit 1~3, interrupt identification 
    if ( IIRValue == IIR_RLS )		// Receive Line Status 
    {
		LSRValue = U0LSR;
		// Receive Line Status 
		if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
		{
	    	// There are errors or break interrupt 
	    	// Read LSR will clear the interrupt 
	    	UART0Status = LSRValue;
	    	Dummy = U0RBR;		// Dummy read on RX to clear 
								// interrupt, then bail out 
	    	// VERIFICARE !! disableIRQ();
	    	VICVectAddr = 0;		// Acknowledge Interrupt 
	    	return;
		}
		if ( LSRValue & LSR_RDR )	// Receive Data Ready 
		{
	    	// If no error on RLS, normal ready, save into the data buffer
	    	// Note: read RBR will clear the interrupt 
	    	UART0Buffer[ptr_uart0_wr] = U0RBR;
	    	ptr_uart0_wr++;
	    	if ( ptr_uart0_wr == BUFSIZE )
	    	{
	    		ptr_uart0_wr = 0;		// buffer overflow 
	    	}	
		}
    }
    else if ( IIRValue == IIR_RDA )	// Receive Data Available 
    {
		// Receive Data Available 
		UART0Buffer[ptr_uart0_wr] = U0RBR;
		ptr_uart0_wr++;
		if ( ptr_uart0_wr == BUFSIZE )
		{
			ptr_uart0_wr = 0;		// buffer overflow 
		}
    }
    else if ( IIRValue == IIR_CTI )	// Character timeout indicator
    {
		// Character Time-out indicator 
		UART0Status |= 0x100;		// Bit 9 as the CTI error 
    }
    else if ( IIRValue == IIR_THRE )	// THRE, transmit holding register empty 
    {
		// THRE interrupt 
		LSRValue = U0LSR;		// Check status in the LSR to see if
								// valid data in U0THR or not 
		if ( LSRValue & LSR_THRE )
		{
	    	UART0TxEmpty = 1;
		}
		else
		{
	    	UART0TxEmpty = 0;
		}
    }
    
    // VERIFICARE !! disableIRQ();
    VICVectAddr = 0;		// Acknowledge Interrupt 
}

// **************************************************************************
// Function name:		UART1Handler
//
// Descriptions:		UART1 interrupt handler
//
// parameters:			None
// Returned value:		None
// 
// **************************************************************************
void __attribute__ ((interrupt("IRQ"))) IRQ_Uart1(void)  
{
    UINT8 IIRValue, LSRValue;
    UINT8 Dummy = Dummy;

    // VERIFICARE !! enableIRQ();				// handles nested interrupt 
    IIRValue = U1IIR;
    
    IIRValue >>= 1;			// skip pending bit in IIR 
    IIRValue &= 0x07;			// check bit 1~3, interrupt identification
    if ( IIRValue == IIR_RLS )		// Receive Line Status 
    {
		LSRValue = U1LSR;
		// Receive Line Status 
		if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
		{
	    	// There are errors or break interrupt 
	    	// Read LSR will clear the interrupt 
	    	UART1Status = LSRValue;
	    	Dummy = U1RBR;		// Dummy read on RX to clear 
								// interrupt, then bail out 
	    	// VERIFICARE !! disableIRQ();
	    	VICVectAddr = 0;		// Acknowledge Interrupt 
	    	return;
		}
		if ( LSRValue & LSR_RDR )	// Receive Data Ready 			
		{
	    	// If no error on RLS, normal ready, save into the data buffer. 
	    	// Note: read RBR will clear the interrupt 
	    	UART1Buffer[ptr_uart1_wr] = U1RBR;
	    	ptr_uart1_wr++;
	    	if ( ptr_uart1_wr == BUFSIZE )
	    	{
	    		ptr_uart1_wr = 0;		// buffer overflow 
	    	}	
		}
    }
    else if ( IIRValue == IIR_RDA )	// Receive Data Available 
    {
		// Receive Data Available 
		UART1Buffer[ptr_uart1_wr] = U1RBR;
		ptr_uart1_wr++;
		if ( ptr_uart1_wr == BUFSIZE )
		{
			ptr_uart1_wr = 0;		// buffer overflow 
		}
    }
    else if ( IIRValue == IIR_CTI )	// Character timeout indicator 
    {
		// Character Time-out indicator 
		UART1Status |= 0x100;		// Bit 9 as the CTI error 
    }
    else if ( IIRValue == IIR_THRE )	// THRE, transmit holding register empty 
    {
		// THRE interrupt 
		LSRValue = U1LSR;		// Check status in the LSR to see if
								// valid data in U0THR or not 
		if ( LSRValue & LSR_THRE )
		{
	    	UART1TxEmpty = 1;
		}
		else
		{
	    	UART1TxEmpty = 0;
		}
    }
    
    // VERIFICARE !! disableIRQ();
    VICVectAddr = 0;		// Acknowledge Interrupt 
}

// **************************************************************************
// Function name:		init_uart
//
// Descriptions:		Initialize UART0 port, setup pin select,
//						clock, parity, stop bits, FIFO, etc.
//
// parameters:			portNum(0 or 1) and UART baudrate
// Returned value:		true or false, return false only if the 
//						interrupt handler can't be installed to the 
//						VIC table
// 
// **************************************************************************
UINT32 init_uart( UINT32 PortNum, UINT32 baudrate )
{
    UINT32 Fdiv;

	if ( PortNum == 0 )
	{
		PINSEL0 |= PINSEL0_P0_2_TXD0 | PINSEL0_P0_3_RXD0;	// P0.2-TxD0,P0.3-RxD0

    	U0LCR = 0x83;		// 8 bits, no Parity, 1 Stop bit 
    	Fdiv = ( Fpclk / 16 ) / baudrate ;	// baud rate 
    	U0DLM = Fdiv / 256;							
    	U0DLL = Fdiv % 256;
		U0LCR = 0x03;		// DLAB = 0 
    	U0FCR = 0x07;		// Enable and reset TX and RX FIFO. 

    	/*
    	if ( install_irq( UART0_INT, (void *)IRQ_Uart0, HIGHEST_PRIORITY ) == FALSE )
    	{
			return (FALSE);
    	}
   
    	U0IER = IER_RBR | IER_THRE | IER_RLS;	// Enable UART0 interrupt
    	*/ 
    	return (TRUE);
	}
	else if ( PortNum == 1 )
	{
					
		PINSEL0 |= PINSEL0_P0_15_TXD1;	// Enable TxD1 P0.15 
		PINSEL1 |= PINSEL1_P0_16_RXD1;	// Enable RxD1 P0.16 
    	U1LCR = 0x83;		// 8 bits, no Parity, 1 Stop bit 
    	Fdiv = ( Fpclk / 16 ) / baudrate ;	// baud rate 
    	U1DLM = Fdiv / 256;							
    	U1DLL = Fdiv % 256;
		U1LCR = 0x03;		// DLAB = 0 */
    	U1FCR = 0x07;		// Enable and reset TX and RX FIFO. 

    	if ( install_irq( UART1_INT, (void *)IRQ_Uart1, HIGHEST_PRIORITY ) == FALSE )
    	{
			return (FALSE);
    	}
   
    	//U1IER = IER_RBR | IER_THRE | IER_RLS;	// Enable UART1 interrupt 
    	U1IER = IER_RBR | IER_RLS;	// Enable UART1 interrupt
    	return (TRUE);
	}
	return( FALSE ); 
}

// **************************************************************************
// Function name:		uart_send
//
// Descriptions:		Send a block of data to the UART 0 port based
//						on the data length
//
// parameters:			portNum, buffer pointer, and data length
// Returned value:		None
// 
// **************************************************************************
void uart_send( UINT32 portNum, const UINT8 *BufferPtr, UINT32 Length )
{
	if ( portNum == 0 )
	{
    	while ( Length != 0 )
    	{
			// THRE status, contain valid data 
			while ( !(UART0TxEmpty & 0x01) );	
			U0THR = *BufferPtr;
			UART0TxEmpty = 0;	// not empty in the THR until it shifts out 
			BufferPtr++;
			Length--;
		}
    }
	else
	{
		while ( Length != 0 )
    	{
			// THRE status, contain valid data 
			while ( !(UART1TxEmpty & 0x01) );	
			U1THR = *BufferPtr;
			UART1TxEmpty = 0;	// not empty in the THR until it shifts out 
			BufferPtr++;
			Length--;
    	}
	}
    return;
}

// **************************************************************************
// Function name:		open_uart
//
// Descriptions:		
//						
//
// parameters:			
// Returned value:		
// 
// **************************************************************************

void open_uart(void)
{
	//-----------------------------------------------
	// Config UART0
	//-----------------------------------------------  
	init_uart( 0, 57600 );
}

// **************************************************************************
// Function name:		exec_uart
//
// Descriptions:		
//						
//
// parameters:			
// Returned value:		
// 
// **************************************************************************

void exec_uart(void)
{
	UINT32 ch;
		
	--uart_prescaler;
	if (!uart_prescaler) 
	{
		uart_prescaler = UART_PRESCALER;
		
		while(ptr_uart0_wr != ptr_uart0_rd) 
		{
			ch = UART0Buffer[ptr_uart0_rd];
			ptr_uart0_rd = (ptr_uart0_rd + 1) & (BUFSIZE - 1);	

			exec_upload(ch);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// **************************************************************************
// FUNCTION: put_char                                
//                                                                           
// DESCRIPTION: Put character to Serial Port     
//                                                                           
// GLOBAL VARIABLES USED:                                                    
//                                                                           
// PARAMETERS:                                                               
//                                                                           
// RETURN:           
// **************************************************************************

void put_char(UINT32 ch)  
{             
	while( (U0LSR & UART_THRE) == 0 );
	
	U0THR = ch;
	
}

// **************************************************************************
// FUNCTION: get_char                                
//                                                                           
// DESCRIPTION: Read character from Serial Port     
//                                                                           
// GLOBAL VARIABLES USED:                                                    
//                                                                           
// PARAMETERS:                                                               
//                                                                           
// RETURN:           
// **************************************************************************

UINT32 get_char (void)  
{   

	while (!(U0LSR & 0x01));

	return (U0RBR);
}

// **************************************************************************
// FUNCTION:    PrintString                             
//                                                                           
// DESCRIPTION: Print Chars to Screen by Uart0     
//                                                                           
// GLOBAL VARIABLES USED:                                                    
//                                                                           
// PARAMETERS:                                                               
//                                                                           
// RETURN:           
// **************************************************************************
UINT32 PrintString(  UINT8 const * pscreen_print )
{
	while( *pscreen_print != '\0' )
    {
		U0THR = *pscreen_print;
		while( (U0LSR & UART_THRE) == 0 );
		pscreen_print++;
	}
	
	return(0);
}

// **************************************************************************
// FUNCTION:    send_4bit_to_uart0                             
//                                                                           
// DESCRIPTION: Print 4bits hex data to Screen by Uart0     
//                                                                           
// GLOBAL VARIABLES USED:                                                    
//                                                                           
// PARAMETERS:                                                               
//                                                                           
// RETURN:           
// **************************************************************************
UINT32 send_4bit_to_uart0( UINT32 Value_Hex_4bits)
{
  switch(Value_Hex_4bits)
  {
	  case 0:
	  case 1:
	  case 2:
	  case 3:
	  case 4:
	  case 5:
	  case 6:
	  case 7:
	  case 8:
	  case 9: U0THR = Value_Hex_4bits + 48;  // print "0 - 9"
	    break;
	  case 10:
	  case 11:
	  case 12:
	  case 13:
	  case 14:
	  case 15: U0THR = Value_Hex_4bits + 55; // print "A - F"
	    break;
      case 44:                               // print ","
      case 45: U0THR = Value_Hex_4bits;      // print "-"
		break;
	  default:	    
	    break;
  }
  
  while( (U0LSR & UART_THRE) == 0 );
  return(0);
}

// **************************************************************************
// FUNCTION:    PrintInt                             
//                                                                           
// DESCRIPTION: Print an unsigned integer data to Screen by Uart0     
//                                                                           
// GLOBAL VARIABLES USED:                                                    
//                                                                           
// PARAMETERS:                                                               
//                                                                           
// RETURN:           
// **************************************************************************

UINT32 PrintInt( UINT32 value)
{
	send_4bit_to_uart0( (value & 0xF0000000)>>28 );
	send_4bit_to_uart0( (value & 0x0F000000)>>24 );
	send_4bit_to_uart0( (value & 0x00F00000)>>20 );
	send_4bit_to_uart0( (value & 0x000F0000)>>16 );
	send_4bit_to_uart0( (value & 0x0000F000)>>12 );
	send_4bit_to_uart0( (value & 0x00000F00)>>8  );
	send_4bit_to_uart0( (value & 0x000000F0)>>4  );
	send_4bit_to_uart0( (value & 0x0000000F)     );
	return(0);
}



