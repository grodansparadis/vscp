// ****************************************************************************
// 
// DESCRIPTION: definizioni per gestione lettura pulsanti
// 
// AUTHOR:
// 
// HISTORY:    
// 
// ****************************************************************************


#ifndef __KEY_H
#define __KEY_H

#define PIN_KEY1	PORT2_BIT2
#define PIN_KEY2	PORT2_BIT0

typedef enum KEYS_tag
{
	KEY1,	// MODE
	KEY2,	// PHASE-REVERSE
	N_KEY
} enKEY;

#define KEY_PRESCALER  		50		// prescaler per lettura key = 50 mS (fs=96k)
#define N_TICKPRESSHOLD		1200 	// 1.25sec n° di tick per attivare il press&hold
#define N_TICKPRESSPIN		72  	// 75ms	n° di tick per attivare il spin inc o dec
#define STATO_KEY			0x0		// FFFFFFFF per tasto a riposo = 0 00000000 per tasto a riposo = 1

#define	KEY_NOEVENT		-1
#define	KEY_PRESS		0
#define	KEY_RELEASE		1
#define	KEY_PRESS_HOLD	2
#define	KEY_PRESS_SPIN	3

#define	KEY_STATE_NORMAL 0
#define	KEY_STATE_SPIN	 1	// stato con auto incremento o decremento abilitato

// ----------------------------------------------------------------------------
extern void read_sw(void);
extern void open_key(void);
extern void exec_key(void);
extern void ReadKey(void);
extern int GetKey(int);
extern void ResetKeyPrescaler(void);
extern void ClearKey(void);
extern int GetRelAfterHold(int code_key);
extern void SetRelAfterHold(int code_key, int value);
extern void test_key(void);
// ----------------------------------------------------------------------------

#endif
