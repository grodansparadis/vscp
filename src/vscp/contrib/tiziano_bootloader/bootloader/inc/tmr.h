// ****************************************************************************
// 
// DESCRIPTION: definizioni per il TIMER MANAGER
// 
// AUTHOR:
// 
// HISTORY:    
// 
// ****************************************************************************


#ifndef _TMR_H
#define _TMR_H

#define TMR_PRESCALER			2	// 2 ms 

// Definizione indici timers 
enum 
{
	DUMMY_TIMER,
	TMR_MAX_TMR			// numero timer
};

///////////////////////////////////////////////////////////////////////////////

extern void   		open_tmr(void);
extern void   		TmrSetT(unsigned n, unsigned n_tick);
extern void   		TmrStart(unsigned n);
extern void   		TmrStop(unsigned n);
extern void   		TmrReset(unsigned n);
extern unsigned 	TmrChk(unsigned n);
extern void   		exec_tmr(void);

#endif
