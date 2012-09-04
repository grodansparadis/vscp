// ******************************************************************
//
// DESCRIPTION: Modulo gestione eventi
// AUTHOR: 		
//
// HISTORY:    
//
// ******************************************************************

#include "buff.h"
#include "key.h"
#include "led.h"
#include "evt.h"

//-----------------------------------------------------------------------------
void EventKeyProcess(unsigned event);
void EventPotProcess(unsigned event);
//-----------------------------------------------------------------------------

// ****************************************************************************
// EventProcess
// 
// FILENAME: evt.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************


void EventProcess(unsigned event)
{
	int source;

	source = (event >> 24) & 0xFF;

	switch(source)
	{
		case ID_SW:
			EventKeyProcess(event);
			break;
		case ID_POT:
			EventPotProcess(event);
			break;
		case ID_ADIAL:

			break;
	}
}

// ****************************************************************************
// EventKeyProcess
// 
// FILENAME: evt.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

void EventKeyProcess(unsigned event)
{
	
}

// ****************************************************************************
// EventPotProcess
// 
// FILENAME: evt.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

void EventPotProcess(unsigned event)
{
	
}
