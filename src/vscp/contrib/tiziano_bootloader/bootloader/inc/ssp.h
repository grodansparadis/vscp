// ******************************************************************
//
// DESCRIPTION: 
// AUTHOR: 		
//
// HISTORY:    
//
// ******************************************************************

#ifndef __SSP_H__
#define __SSP_H__

#define DMA_ENABLED		1

// if USE_CS is zero, set SSEL as GPIO that you have total control of the sequence 
#define USE_CS			1

// SPI read and write buffer size 
#define BUFSIZE			256
#define FIFOSIZE		8

// SSP select pin 
#define SSP0_SEL		PORT1_BIT21
#define SPID_CS1		1 << 11
#define SPID_CS2		1 << 10
#define SPID_ACK1		1 << 8
#define SPID_ACK2		1 << 9

	
// SSP Status register 
#define SSPSR_TFE		1 << 0
#define SSPSR_TNF		1 << 1 
#define SSPSR_RNE		1 << 2
#define SSPSR_RFF		1 << 3 
#define SSPSR_BSY		1 << 4

// SSP0 CR0 register 
#define SSPCR0_DSS		1 << 0
#define SSPCR0_FRF		1 << 4
#define SSPCR0_SPO		1 << 6
#define SSPCR0_SPH		1 << 7
#define SSPCR0_SCR		1 << 8

// SSP0 CR1 register 
#define SSPCR1_LBM		1 << 0
#define SSPCR1_SSE		1 << 1
#define SSPCR1_MS		1 << 2
#define SSPCR1_SOD		1 << 3

// SSP0 Interrupt Mask Set/Clear register 
#define SSPIMSC_RORIM	1 << 0
#define SSPIMSC_RTIM	1 << 1
#define SSPIMSC_RXIM	1 << 2
#define SSPIMSC_TXIM	1 << 3

// SSP0 Interrupt Status register 
#define SSPRIS_RORRIS	1 << 0
#define SSPRIS_RTRIS	1 << 1
#define SSPRIS_RXRIS	1 << 2
#define SSPRIS_TXRIS	1 << 3

// SSP0 Masked Interrupt register 
#define SSPMIS_RORMIS	1 << 0
#define SSPMIS_RTMIS	1 << 1
#define SSPMIS_RXMIS	1 << 2
#define SSPMIS_TXMIS	1 << 3

// SSP0 Interrupt clear register 
#define SSPICR_RORIC	1 << 0
#define SSPICR_RTIC		1 << 1

// If RX_INTERRUPT is enabled, the SSP RX will be handled in the ISR
 
extern UINT32 SSP0Init( void );
extern void SSP0Send( UINT8 *Buf, UINT32 Length );
extern void SSP0Receive( UINT8 *buf, UINT32 Length );
extern void SSP0SetCS( void );
extern void SSP0ClrCS( void );
extern void SSP0ClrFIFO( void );

#endif  // __SSP_H__ 
