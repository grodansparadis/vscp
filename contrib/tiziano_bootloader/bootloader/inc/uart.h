// *****************************************************************************
// uart.h:  Header file for NXP LPC23xx Family Microprocessors
// 
// Copyright(C) 2006, NXP Semiconductor
// All rights reserved.
// 
// History
// 2006.09.01  ver 1.00    Prelimnary version, first Release
// 
// *****************************************************************************

// modified by Martin Thomas (check for __irq makro)

#ifndef __UART_H 
#define __UART_H

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

#define UART_BUFSIZE		0x40
#define UART_PRESCALER		1

///////////////////////////////////////////////////////////////////////////////
void uart_send( UINT32 portNum, const UINT8 *BufferPtr, UINT32 Length );
void open_uart(void);
void exec_uart(void);
UINT32 get_char (void);
void put_char(UINT32 ch);
UINT32 send_4bit_to_uart0( UINT32 Value_Hex_4bits);
UINT32 PrintString(  UINT8 const * pscreen_print );
UINT32 PrintInt( UINT32 value);
#endif // end __UART_H 
