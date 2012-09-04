// ***************************************************************
//
// DESCRIPTION: Enable and disable interrupt functions "stripped" 
// 		   
// AUTHOR:
//
// HISTORY:    
//
// *******************************************************************

#include "LPC23xx.H"
#include "config.h"
#include "irq.h"
#include "vic.h"

#define IRQ_MASK I_Bit

static inline unsigned asm_get_cpsr(void)
{
	unsigned long retval;
	asm volatile (" mrs  %0, cpsr" : "=r" (retval) : /* no inputs */  );
	return retval;
}

static inline void asm_set_cpsr(unsigned val)
{
	asm volatile (" msr  cpsr, %0" : /* no outputs */ : "r" (val)  );
}

unsigned enableIRQ(void)
{
	unsigned _cpsr;

	_cpsr = asm_get_cpsr();
	asm_set_cpsr(_cpsr & ~IRQ_MASK);
	return _cpsr;
}

unsigned disableIRQ(void)
{
	unsigned _cpsr;

	_cpsr = asm_get_cpsr();
	asm_set_cpsr(_cpsr | IRQ_MASK);
	return _cpsr;
}

unsigned restoreIRQ(unsigned oldCPSR)
{
	unsigned _cpsr;

	_cpsr = asm_get_cpsr();
	asm_set_cpsr((_cpsr & ~IRQ_MASK) | (oldCPSR & IRQ_MASK));
	return _cpsr;
}

//////////////////////////////////////////////////////////////////////////////

void init_vic(void) 
{
    int i = 0;
    int *vect_addr, *vect_cntl;
   	
    // initialize VIC
    VICIntEnClr = 0xFFFFFFFF;
    VICVectAddr = 0;
    VICIntSelect = 0;

    // set all the vector and vector control register to 0 
    for ( i = 0; i < VIC_SIZE; i++ )
    {
		vect_addr = (int *)(VIC_BASE_ADDR + VECT_ADDR_INDEX + i*4);
		vect_cntl = (int *)(VIC_BASE_ADDR + VECT_CNTL_INDEX + i*4);
		*vect_addr = 0x0;	
		*vect_cntl = 0xF;
    }
    return;
}

int install_irq( int IntNumber, void *HandlerAddr, int Priority )
{
	int *vect_addr;
	int *vect_cntl;
      
    VICIntEnClr = 1 << IntNumber;	// Disable Interrupt 
    if ( IntNumber >= VIC_SIZE )
    {
		return ( FALSE );
    }
    else
    {
		// find first un-assigned VIC address for the handler 
		vect_addr = (int *)(VIC_BASE_ADDR + VECT_ADDR_INDEX + IntNumber*4);
		vect_cntl = (int *)(VIC_BASE_ADDR + VECT_CNTL_INDEX + IntNumber*4);
		*vect_addr = (int)HandlerAddr;	// set interrupt vector 
		*vect_cntl = Priority;
		VICIntEnable = 1 << IntNumber;	// Enable Interrupt 
		return( TRUE );
    }
}

