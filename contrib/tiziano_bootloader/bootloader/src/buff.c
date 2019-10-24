// ******************************************************************
//
// DESCRIPTION: scrittura/lettura eventi pannello nel buffer 
// 		   di sistema
// AUTHOR:
//
// HISTORY:    
//
// *******************************************************************

#include "evt.h"
#include "config.h"
#include "led.h"
#include "key.h"
#include "ssp.h"
#include "emem.h"
#include "buff.h"

static unsigned buff[BUFF_SIZE];
static unsigned ptr_buff_wr;
static unsigned ptr_buff_rd;
static int wrbuff_prescaler;
static int rdbuff_prescaler;
// ****************************************************************************

// ****************************************************************************
// open_io
// 
// FILENAME: buff.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

void open_io(void)
{
	open_led();
	open_key();	
	SSP0Init();
}

// ****************************************************************************
// exec_io
// 
// FILENAME: buff.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

void exec_io(void)
{
	exec_led();
	exec_key();	
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// open_rdwr
// 
// FILENAME: buff.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

void open_rdwr(void)
{
	open_wr_buff();
	EditMemoryInit(0);
}

// ****************************************************************************
// exec_rdwr
// 
// FILENAME: buff.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

void exec_rdwr(void)
{
	exec_wr_buff();
	exec_rd_buff();	
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// open_wr_buff
// 
// FILENAME: buff.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

void open_wr_buff(void)
{
	int i;

	for(i=0; i < BUFF_SIZE; i++)
	{ 
		buff[i] = 0;
	}
	ptr_buff_wr = 0;
	ptr_buff_rd = 0;
	wrbuff_prescaler = WR_BUFF_PRESCALER;
	rdbuff_prescaler = RD_BUFF_PRESCALER;
}

// ****************************************************************************
// exec_wr_buff
// 
// FILENAME: buff.c
// 
// PARAMETERS:
// 
// DESCRIPTION: scrittura nel buffer di sistema eventi 
// 			 di pannello,deve essere chiamata ogni 50ms
// RETURNS:
// 
// ****************************************************************************

void exec_wr_buff(void) 
{
	--wrbuff_prescaler;
	if (!wrbuff_prescaler) 
	{	
		wrbuff_prescaler = WR_BUFF_PRESCALER;
		ReadKey();
	}
}

// ****************************************************************************
// store_event
// 
// FILENAME: buff.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

void store_event(unsigned *event)
{
	*(unsigned *)(buff+ptr_buff_wr) = *event;				   
    ptr_buff_wr = (ptr_buff_wr+1) & (BUFF_SIZE-1);			
}

// ****************************************************************************
// exec_rd_buff
// 
// FILENAME: buff.c
// 
// PARAMETERS:
// 
// DESCRIPTION: lettura nel buffer di sistema eventi 
// 			 di pannello
// RETURNS:
// 
// ****************************************************************************

void exec_rd_buff(void) 
{
	unsigned event;
	unsigned *ptr_buff = buff;
	
	--rdbuff_prescaler;
	if (!rdbuff_prescaler) 
	{
		rdbuff_prescaler = RD_BUFF_PRESCALER;

		while(ptr_buff_wr != ptr_buff_rd) 
		{
			event = *(unsigned *)(ptr_buff + ptr_buff_rd);	
	    	ptr_buff_rd = (ptr_buff_rd+1) & (BUFF_SIZE-1);	

			// processamento eventi 
			EventProcess(event);
		}
	}
}





