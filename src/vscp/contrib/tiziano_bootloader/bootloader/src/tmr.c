// ******************************************************************
//
// DESCRIPTION: timer manager 
// 		   
// AUTHOR:
//
// HISTORY:    
//
// *******************************************************************

#include "tmr.h"

typedef struct  				// TIMER DATA STRUCTURE 
{                
    unsigned TmrEn;             // Flag indicante se il timer e' abilitato 
    unsigned TmrCtr;            // Valore corrente del timer (counting down) 
    unsigned TmrInit;         	// Valore iniziale del timer (cioe' quando il timer e' settato) 
} TMR;

TMR     TmrTbl[TMR_MAX_TMR];    // Tabella timer 
static unsigned TmrTickCtr;

// ****************************************************************************
// open_Tmr
// 
// FILENAME: tmr.c
// 
// PARAMETERS:
// 
// DESCRIPTION:	timer manager initialization
// 
// RETURNS:	None.
// 
// ****************************************************************************

void open_tmr(void)
{
    int i;
    TMR   *ptmr;

    ptmr = &TmrTbl[0];
    for (i = 0; i < TMR_MAX_TMR; i++)	// azzera e disabilita tutti i timer 
    {          
        ptmr->TmrEn   = 0;
        ptmr->TmrCtr  = 0;
        ptmr->TmrInit = 0;
        ptmr++;							 
    }
    TmrTickCtr = 0;
}

// ****************************************************************************
// TmrSetT
// 
// FILENAME: tmr.c
// 
// PARAMETERS: n e' il numero del timer 0..TMR_MAX_TMR-1
// 
// DESCRIPTION:	Setta il timer con il specificato numero di N*tick
// 
// RETURNS:	None.
// 
// ****************************************************************************

void TmrSetT(unsigned n, unsigned n_tick)
{
    TMR *ptmr;

    if (n < TMR_MAX_TMR) 
    {
        ptmr          = &TmrTbl[n];
        ptmr->TmrInit = n_tick;
        ptmr->TmrCtr  = n_tick;
    }
}

// ****************************************************************************
// TmrStart
// 
// FILENAME: tmr.c
// 
// PARAMETERS: n e' il timer che deve partire
// 
// DESCRIPTION:	fa partire il timer
// 
// RETURNS: None.
// 
// ****************************************************************************

void TmrStart(unsigned n)
{
    if (n < TMR_MAX_TMR) 
    {
        TmrTbl[n].TmrEn = 1;
    }
}

// ****************************************************************************
// TmrStop
// 
// FILENAME: tmr.c
// 
// PARAMETERS: n e' il timer che deve fermarsi
// 
// DESCRIPTION:	Questa funzione fa fermare il timer
// 
// RETURNS:	None.
// 
// ****************************************************************************

void TmrStop(unsigned n)
{
    if (n < TMR_MAX_TMR) 
    {
        TmrTbl[n].TmrEn = 0;
    }
}

// ****************************************************************************
// TmrReset
// 
// FILENAME: tmr.c
// 
// PARAMETERS: n e' il timer da resettare
// 
// DESCRIPTION:	fa il reload del timer con il suo valore iniziale
// 
// RETURNS: None.
// 
// ****************************************************************************

void TmrReset(unsigned n)
{
    TMR *ptmr;

    if (n < TMR_MAX_TMR) 
    {
        ptmr         = &TmrTbl[n];
        ptmr->TmrCtr = ptmr->TmrInit;	// Reload del conteggio 
    }
}

// ****************************************************************************
// TmrChk
// 
// FILENAME: tmr.c
// 
// PARAMETERS:  n      e' il timer da controllare
// 
// DESCRIPTION:	controlla se il timer ha finito il conteggio
// 
// RETURNS: 0      se il timer ha finito il conteggio
// 			       TmrCtr il tempo che rimane dalla fine del conteggio in tick 
// ****************************************************************************

unsigned TmrChk(unsigned n)
{
    unsigned int value;

    value = 0;
    if (n < TMR_MAX_TMR) 
    {
        value = TmrTbl[n].TmrCtr;
    }
    return (value);
}

// ****************************************************************************
// exec_Tmr
// 
// FILENAME: tmr.c
// 
// PARAMETERS:  None.
// 
// DESCRIPTION:	aggiornamento dei timer
// 
// RETURNS: None.
// 			
// ****************************************************************************

void exec_tmr(void)	
{
    TMR   *ptmr;
    int  i;
	//STROBE_OFF
    ptmr = &TmrTbl[0];                  // Punta all'inizio della tabella dei timer 

    for (i = 0; i < TMR_MAX_TMR; i++) 
    {
    	if (ptmr->TmrEn == 1) 			// Decrementa il timer solo se e' abilitato 
    	{                
    		if (ptmr->TmrCtr > 0) 
    		{
        		ptmr->TmrCtr--;
           	}
        }
    	if (ptmr->TmrCtr == 0) 			// controlla se il timer ha finito il conteggio 
    	{                  
            	ptmr->TmrEn = 0;        // Disabilita il timer quando e' in times out 
       	}
       	ptmr++;
    }
	//STROBE_ON
}
