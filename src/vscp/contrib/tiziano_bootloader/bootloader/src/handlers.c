/*****************************************************************************
 *
 * Project          : GNUEasyWeb
 * Subproject       : 
 * Name             : handlers.c
 * Function         : Special exception handling
 * Designer         : K. Sterckx
 * Creation date    : 19/01/2007
 * Compiler         : GNU ARM
 * Processor        : LPC2368
 * Last update      :
 * Last updated by  :
 * History          :
 *
 *****************************************************************************
 *
 *  Called from vector table in startarm.s
 *
 ****************************************************************************/

#include "LPC23xx.h"

/*
 * Prototypes used to set attribute
 */
void Undef_Handler(void) __attribute__ ((interrupt));
void SWI_Handler(void)   __attribute__ ((interrupt));
void PAbt_Handler(void)  __attribute__ ((interrupt));
void DAbt_Handler(void)  __attribute__ ((interrupt));
void IRQ_Handler(void)   __attribute__ ((interrupt));
void FIQ_Handler(void)   __attribute__ ((interrupt));

/*
 * Implementation
 */
void Undef_Handler(void)
{
	while (1)
		;
}

void SWI_Handler(void)
{
	while (1)
		;
}

void PAbt_Handler(void)
{
	while (1)
		;
}

void DAbt_Handler(void)
{
	while (1)
		;
}

void FIQ_Handler(void)
{
	while (1)
		;
}


