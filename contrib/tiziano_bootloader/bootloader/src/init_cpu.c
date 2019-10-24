// ****************************************************************************
// FILENAME:                                                    
//                                                                           
// VERSION:                                                              
//                                                                          
// DESCRIPTION:  Initialisation of CPU modules                                 
//                                                                           
// TOOLS:                             		  
//                                                                           
// REVISION HISTORY                                                          
// Version  Author           Date        Remarks							  
//                                      
// ****************************************************************************

#include "LPC23xx.h"

//*****************************************************************************
// Defines the clock values in Hz
//
//  Change when
//  - crystal changes
//  - Init_CPU code changes
//

#define RC_FREQUENCY       4000000

//****************************************************************************
// Defines the wait values used in Init_Wait
// Depends on CPU clk
// Must result in a wait time of around 500mS
//

#define CONFIG_INITWAIT_J  20
#define CONFIG_INITWAIT_I  (RC_FREQUENCY / 1000)

//****************************************************************************
// Used to halt the CPU a few ms so that JTAG can take control
// over processor before any other code is called.
//
// Called with interrupts disabled and before stack and variables are setup
//

static void wait(void)
{
}

void init_wait(void)
{
	volatile unsigned int i;
	volatile unsigned int j;

	for (j = 0; j < CONFIG_INITWAIT_J; j++)
	{
		for (i = 0; i < CONFIG_INITWAIT_I; i++)
		{
			wait();
		}
	}
}

//****************************************************************************
// Used to initialize the Clocks
//
// Called with interrupts disabled and before stack and variables are setup
//
// Crystal is 12.000MHz, this is the main clock, Fin
// PLL will be clocked from the main clock, we want a Fcco clock of 480MHz
// so it can be used to generate 60MHz for CPU and 48MHz for USB.
// Fcco = (2*M*Fin)/N, M=12, N=1 -> Fcco = 288MHz
// nel registro PLLCFG va scritto M-1 e N-1 
// Fcpu = 72MHz = 288MHz/4
// Fusb = 48MHz = 288MHz/6
//
// Fperipheral = Fcpu/4 for all, can be changed in Init_Modules
//
// Remark
// Unable to get oscillator running for CPU=60Mhz (M=20,N=1), because
// of bug in LPC, see errata sheet (Fosc limited to 290MHz)
//

void init_clocks(void)
{
	// Start main oscillator 
	SCS |= OSCEN;

	while ((SCS & OSCSTAT) == 0)
		;

	// Select PLL as source 
	CLKSRCSEL = PLL_SOURCE_OSCILLATOR;

	// Start PLL 
	PLLCFG    = 0x0000000B;
	PLLFEED   = 0xAA;
	PLLFEED   = 0x55;

	PLLCON    = PLLE;
	PLLFEED   = 0xAA;
	PLLFEED   = 0x55;

	// Wait until locked 
	while ((PLLSTAT & PLLLOCKED) == 0)
		;

	// Wait until M and N are correct 
	while ((PLLSTAT & 0xFF7FFF) != 0x0000000B)
		;

	// Setup CPU clock divider 
	CCLKCFG = 3;	// va scritto divisore - 1

	// Setup USB clock divider 
	USBCLKCFG = 5;	// va scritto divisore - 1

	// Setup Peripheral clock 
	PCLKSEL0 = 0;
	PCLKSEL1 = 0;

	// Switch to PLL clock 
	PLLCON   = PLLE | PLLC;
	PLLFEED  = 0xAA;
	PLLFEED  = 0x55;

	// Make sure we are connected to PLL 
	while ((PLLSTAT & PLLCONNECTED)==0)
		;
}

//*****************************************************************************
// Used to initialize the MAM
//
// Called with interrupts disabled and before stack and variables are setup
//

void init_mam(void)
{
	MAMCR  = MAM_DISABLED;  		// Off                                        
	MAMTIM = MAM_FETCH_CLOCKS_4;	// 4 Fetch cycles, gives better result than 3 
	MAMCR  = MAM_FULLY_ENABLED;  	// Fully enabled                              

	MEMMAP = 0x01;  // Uses interrupts vectors in flash 
}

//*****************************************************************************
// Used to initializes the CPU
//
// Some special setup before interrupts are enabled
//
// Called with interrupts disabled, stack and variables are setup
//

void init_cpu(void)
{
	
}

//*****************************************************************************
// Used to initialize the Pins
//
// Pins can also be setup in Startarm.s
//
// Called with interrupts disabled and before stack and variables are setup
//

void init_pins(void)
{
	// Setup P2.0 - P2.7 as outputs, disconnect them from ETM 
	PINSEL10 = 0;
	FIO2DIR  = 0x000000FF;
	FIO2MASK = 0x00000000;
	FIO2CLR  = 0xFF;

	// Enable FAST mode for PORT0 and PORT1 
	SCS |= (1<<0);
}

//*****************************************************************************
// Used to initializes the I/O Modules
//
// Called with interrupts disabled, stack and variables are setup
//

void init_modules(void)
{
	//init_lcd();
	//lcd_cur_off();
}

//*****************************************************************************
// Called when main is exited
//

void exit_main(void)
{
	
}
