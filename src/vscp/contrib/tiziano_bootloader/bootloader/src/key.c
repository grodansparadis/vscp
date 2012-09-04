// ******************************************************************
// DESCRIPTION: gestione pulsanti
// AUTHOR:
// HISTORY: 
// Queste funzioni gestiscono l'input per i pulsanti connessi 
// direttamente sulle porte   
// 
// ******************************************************************

#include "LPC23xx.H"
#include "buff.h"
#include "config.h"
#include "vic.h"
#include "irq.h"
#include "uart.h"
#include "key.h"

typedef struct 
{
	int KEYDataOld;
	int KEYTypeEvent;
	int KEYCounter;
	int KEYRun;
	int KEYState;
	int KEYRelAfterHold;
} KEY;

static int key_prescaler;
static KEY Key[N_KEY];
static int key_data_new[N_KEY];

extern int GetFreeRunning(void);

// ****************************************************************************
// open_Key
// FILENAME: key.c
// PARAMETERS:
// DESCRIPTION:
// RETURNS:
// ****************************************************************************

void open_key(void)								  
{
	int i;

	FIO2DIR &= ~(PIN_KEY1 | PIN_KEY2); 	// pin defined as Inputs

	key_prescaler = KEY_PRESCALER;
	for(i=0; i < N_KEY; i++)
	{
		Key[i].KEYTypeEvent	= KEY_NOEVENT;
		key_data_new[i]	    = 0; 
		Key[i].KEYDataOld	= 0;
		Key[i].KEYCounter	= 0;
		Key[i].KEYRun	 	= 0;
		Key[i].KEYState		= KEY_STATE_NORMAL;
		Key[i].KEYRelAfterHold 	= 0;
	}
}

// ****************************************************************************
// exec_key
// FILENAME: key.c
// PARAMETERS:
// DESCRIPTION: 
// RETURNS:
// ****************************************************************************

void exec_key(void)
{
	--key_prescaler;
	if (!key_prescaler) 
	{
		key_prescaler = KEY_PRESCALER;
		
		read_sw();
	}
}

// ****************************************************************************
// read_sw
// FILENAME: key.c
// PARAMETERS:
// DESCRIPTION: 
// RETURNS:
// ****************************************************************************

void read_sw(void)
{
	if((FIO2PIN0 & PIN_KEY1) == 0)
	{
		key_data_new[KEY1] = 1;
	}
	else
	{
		key_data_new[KEY1] = 0;
	}
	if((FIO2PIN0 & PIN_KEY2) == 0)
	{
		key_data_new[KEY2] = 1;
	}
	else
	{
		key_data_new[KEY2] = 0;
	}
}

// ****************************************************************************
// ReadKey
// FILENAME: key.c
// PARAMETERS:
// DESCRIPTION:
// RETURNS:
// ****************************************************************************

void ReadKey(void)
{
	int i;
	int new,old,pushes,releases;
	unsigned event;

	for(i=0; i < N_KEY; i++)
	{
		new=key_data_new[i];
		old=Key[i].KEYDataOld;
		Key[i].KEYDataOld = new;
		if(old != new)
		{	
			pushes = (old^new) & new;
			releases = (old^new) & old;
			if(pushes)
			{
				Key[i].KEYTypeEvent	= KEY_PRESS;
				Key[i].KEYCounter 	= GetFreeRunning();
				Key[i].KEYRun	 	= 1;
				Key[i].KEYState 	= KEY_STATE_NORMAL;
				event=SW_EVT(0,i,KEY_ON);
				store_event(&event);
				//PrintString( (UINT8 const *)"\n\rDEBUG:KEY ON EVT: key=" );
				//PrintInt((UINT32)i);
			}
			else if(releases)
			{
				Key[i].KEYTypeEvent	= KEY_RELEASE;
				Key[i].KEYCounter 	= GetFreeRunning();
				Key[i].KEYRun	 	= 0;
				Key[i].KEYState 	= KEY_STATE_NORMAL;
				event=SW_EVT(0,i,KEY_OFF);
				store_event(&event);
				//PrintString( (UINT8 const *)"\n\rDEBUG:KEY OFF EVT: key=" );
				//PrintInt((UINT32)i);
			}
			else
			{
				Key[i].KEYTypeEvent	= KEY_NOEVENT;
				Key[i].KEYCounter 	= GetFreeRunning();
				Key[i].KEYRun	 	= 0;
				Key[i].KEYState 	= KEY_STATE_NORMAL;
			}
		}
		else
		{
			if(Key[i].KEYRun == 1)
			{
				if(Key[i].KEYState == KEY_STATE_NORMAL)
				{
					if(GetFreeRunning() > (Key[i].KEYCounter + N_TICKPRESSHOLD))
					{
						Key[i].KEYTypeEvent = KEY_PRESS_HOLD;
						Key[i].KEYState 	= KEY_STATE_SPIN;
						Key[i].KEYCounter 	= GetFreeRunning();
						event=SW_EVT(0,i,PRESS_HOLD);
						store_event(&event);
						//PrintString( (UINT8 const *)"\n\rDEBUG:KEY PRESS&HOLD EVT: key=" );
						//PrintInt((UINT32)i);
					}
					else
					{
						Key[i].KEYTypeEvent = KEY_NOEVENT;
					}
				}
				else if(Key[i].KEYState == KEY_STATE_SPIN)
				{
					if(GetFreeRunning() > (Key[i].KEYCounter + N_TICKPRESSPIN))
					{
						Key[i].KEYTypeEvent = KEY_PRESS_SPIN;
						Key[i].KEYCounter 	= GetFreeRunning();
						event=SW_EVT(0,i,PRESS_SPIN);
						store_event(&event);
						//PrintString( (UINT8 const *)"\n\rDEBUG:KEY SPIN EVT: key=" );
						//PrintInt((UINT32)i);
					}
					else
					{
						Key[i].KEYTypeEvent = KEY_NOEVENT;
					}
				}
			}
			else
			{
				Key[i].KEYTypeEvent = KEY_NOEVENT;
			}
		}
	}
} 

// ****************************************************************************
// GetKey
// FILENAME: key.c
// PARAMETERS: codice switch
// DESCRIPTION:
// RETURNS:  
// ****************************************************************************

int GetKey(int code_key)
{
	return key_data_new[code_key];
}

// ****************************************************************************
// ResetKeyPrescaler
// FILENAME: key.c
// PARAMETERS: codice switch
// DESCRIPTION:
// RETURNS:  
// ****************************************************************************

void ResetKeyPrescaler(void)
{
	key_prescaler = 1;
}

// ****************************************************************************
// ClearKey
// FILENAME: key.c
// PARAMETERS: codice switch
// DESCRIPTION:
// RETURNS:  
// ****************************************************************************

void ClearKey(void)
{
	int i;
	for(i=0; i < N_KEY; i++)
	{
		Key[i].KEYTypeEvent	= KEY_NOEVENT;
		key_data_new[i]	    = 1; 
		Key[i].KEYDataOld	= 1;
		Key[i].KEYCounter	= 0;
		Key[i].KEYRun	 	= 0;
		Key[i].KEYState		= KEY_STATE_NORMAL;
		Key[i].KEYRelAfterHold 	= 0;
	}
}

// ****************************************************************************
// test_key
// FILENAME: key.c
// PARAMETERS: codice switch
// DESCRIPTION:
// RETURNS:  
// ****************************************************************************

void test_key(void)
{
	
}

///////////////////////////////////////////////////////////////////////////////

int GetRelAfterHold(int code_key)
{
	return Key[code_key].KEYRelAfterHold;
}

void SetRelAfterHold(int code_key, int value)
{
	Key[code_key].KEYRelAfterHold = value;
}
