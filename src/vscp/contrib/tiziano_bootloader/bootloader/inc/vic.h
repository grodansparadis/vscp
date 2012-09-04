// ****************************************************************************
//   
//
//  Copyright(C) 2006, NXP Semiconductor
//  All rights reserved.
//
//  History
//  2006.07.13  ver 1.00    Prelimnary version, first Release
//
// ****************************************************************************
#ifndef __VIC_H 
#define __VIC_H

unsigned enableIRQ(void);
unsigned disableIRQ(void);
unsigned restoreIRQ(unsigned oldCPSR);
void init_vic(void);
int install_irq( int IntNumber, void *HandlerAddr, int Priority );

#endif

