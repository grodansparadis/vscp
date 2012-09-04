//******************************************************************************
// FILENAME:                                                                     
//                                                                             
// VERSION:                                                                    
//                                                                             
// DESCRIPTION:   Philips LPC2000 CAN Controller Driver with SJA1000                                
//				  Demo by UART0 with KEIL MCB2100 Evaluation Board			  
//				  Code runing indicate by LED4 with Timer1 interrupt			  
//                UART0 tested with Receive "a" NOT LED1, Receive "b" NOT LED2
//                                                                            
// TOOLS:         KEIL uVision3                                     			 
//                ARM Development Suite ADS1.2								  
//                KEIL MCB2100 Evaluation Board               				  
//                                                                            
// REVISION HISTORY                                                           
// Version  Author           Date        Remarks								 
// 1.0      Anthony Xie      10/10/2005    - -                                     
//******************************************************************************

// -----------------------------------------------
// Project include files                                                     
// -----------------------------------------------
#include "LPC23xx.H"
#include "config.h"
#include "uart.h"
#include "utils.h"

extern int* __end_of_text__;
//-----------------------------------------------
//Constant Definitions
//-----------------------------------------------

static UINT8 datetime_text[2][16+1] = {__DATE__, // Buffer for LCD text
                                          __TIME__ };

const UINT8 WelcomeString[] = 
"\n\r\n\r$$$$$$$ PHILIPS LPC2364 BOOTLOADER $$$$$$$\n\n\r\
\n\r";

//---------------------------------------------
//Global Variable Definitions
//---------------------------------------------


//***************************************************************************
// Name: delay()
// Function: Delay 
// Input(s): delaytime	1 about 284.6 us
// Returns : none
//***************************************************************************

void  delay(UINT32  delaytime)
{  
	volatile UINT32 i = (delaytime << 10);
		
		for(; i; --i)
		{
			asm volatile ("nop");
		}
}

//***************************************************************************
// Name: print_welcomemessage()
// Function:  
// Input(s): 
// Returns : 
//***************************************************************************

void print_welcomemessage(void)
{
	PrintString( WelcomeString );
	PrintString( (UINT8 const *)datetime_text[0] );
	PrintString( (UINT8 const *)"\n\r" );
	PrintString( (UINT8 const *)datetime_text[1] );
	PrintString( (UINT8 const *)"\n\r" );
}


