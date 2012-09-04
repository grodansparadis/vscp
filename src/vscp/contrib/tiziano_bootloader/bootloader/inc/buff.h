// ****************************************************************************
// 
// DESCRIPTION: definizioni per buffer di sistema
// 
// AUTHOR:
// 
// HISTORY:    
// 
// ****************************************************************************

#ifndef __BUFF_H
#define __BUFF_H

#define	BUFF_SIZE			16
#define WR_BUFF_PRESCALER  	50	// 50 mS 
#define RD_BUFF_PRESCALER  	50	// 50 mS 	

// ------------------------------------------------------------------
// BYTE4=ID,BYTE3=N.CANALE,BYTE2=CODICE ELEMENTO,BYTE1=VALORE	   
// ------------------------------------------------------------------

// ******************************************************************
//			MESSAGGI POT/SW/A-DIAL      						   	
// ******************************************************************
// ID (PROVENIENZA) 
#define ID_SW			0		// provenienza tasti 
#define ID_POT			1		// 	   "	   ADC (potenziometri) 
#define ID_ADIAL		2		//     "       ALPHA-DIAL 
#define ID_DUMMY		255		// non fa niente,gli altri byte sono a 0 

// STATUS (N.CANALE) 

// CODICE ELEMENTO 

// VALORE 
#define KEY_ON			0x75	// tasto premuto (tasti) 
#define KEY_OFF			0x76	// tasto rilasciato (tasti) 
#define PRESS_HOLD		0x77	// tasto mantenuto premuto per un certo tempo (tasti) 
#define TWICE			0x78	// rapida successione di 2 premute (tasti) 
#define PRESS_SPIN		0x79	
// ------------------------------------------------------------------
//							MACRO							       
// ------------------------------------------------------------------
#define POT_EVT(canale,elemento,value)	(((ID_POT << 24)&0xFF000000) | ((canale << 16)&0xFF0000) | ((elemento << 8)&0xFF00) | (value&0xFF))
#define SW_EVT(canale,elemento,value)	(((ID_SW << 24)&0xFF000000) | ((canale << 16)&0xFF0000) | ((elemento << 8)&0xFF00) | (value&0xFF))
#define DIAL_EVT(canale,n_dial,segno,value)	(((ID_ADIAL << 24)&0xFF000000) | ((canale << 16)&0xFF0000) | ((n_dial << 12)&0xF000) | ((segno << 8)&0xF00) | (value&0xFF))
// ------------------------------------------------------------------
//							PROTOTIPI							   
// ------------------------------------------------------------------
extern void open_io(void);
extern void exec_io(void);
extern void open_rdwr(void);
extern void exec_rdwr(void);
extern void open_wr_buff(void);
extern void exec_wr_buff(void); 
extern void store_event(unsigned *);
extern void exec_rd_buff(void);

#endif
