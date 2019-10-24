///////////////////////////////////////////////////////////////////////////////
//
// File: vscp_events.c
//
// 
// Copyright (C) 2005 Ake Hedman, akhe@eurosource.se
// http://www.vscp.org
//
// $RCSfile: hulkevents.c,v $
// $Revision: 1.1 $
//



#include "LPC23xx.H"
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "can.h"
#include "utils.h"
#include "emem.h"
#include "irq.h"
#include "vic.h"
#include "uart.h"
#include <vscp_class.h>
#include <vscp_type.h>
#include "vscp.h"
#include "vscp_events.h"

static int measurement_prescaler = MEASUREMENT_PRESCALER;
static int measurement_counter = 0;

///////////////////////////////////////////////////////////////////////////////

typedef struct _controlstate_tag  
{
	unsigned m_DevType				:8;
	unsigned m_BootMode				:1;
	unsigned m_Free					:4;
	unsigned m_Vendor				:3;
	unsigned m_Signal_in_1			:1;
	unsigned m_Signal_in_2			:1;
	unsigned m_Clip_in_1			:1;
	unsigned m_Clip_in_2			:1;
	unsigned m_Limit_out_1			:1;
	unsigned m_Limit_out_2			:1;
	unsigned m_RmsProt				:1; 
	unsigned m_OvlTemp				:1;
} CONTROLSTATE;


///////////////////////////////////////////////////////////////////////////////
// doMeasurement
//
void doMeasurement(void)
{
	
	--measurement_prescaler;
	if (!measurement_prescaler) 
	{
		measurement_prescaler = MEASUREMENT_PRESCALER;
		measurement_counter++;
	}
}

///////////////////////////////////////////////////////////////////////////////
// readControlState
//

void readControlState( CONTROLSTATE* p_state )
{
	p_state->m_DevType      = DEV_TYPE;
	
	// sta girando il bootloader -> si forza comunque a 1
	p_state->m_BootMode	= 1;
	
	// vendor
	p_state->m_Vendor = PROEL_VENDOR;
		
	p_state->m_Signal_in_1	= 0;
	p_state->m_Signal_in_2	= 1;
	p_state->m_Clip_in_1	= 0;
	p_state->m_Clip_in_2	= 1;
	p_state->m_Limit_out_1	= 0;
	p_state->m_Limit_out_2	= 1;
	p_state->m_RmsProt		= 0;
	p_state->m_OvlTemp		= 1;
}

///////////////////////////////////////////////////////////////////////////////
// sendHeartBeat
//

int sendHeartBeat( void )
{
	unsigned i;
	CONTROLSTATE state;
	unsigned char* p_char;
	
	readControlState( &state );
	
	p_char = (unsigned char *)&state; 
	
	vscp_omsg.priority = VSCP_PRIORITY_MEDIUM;
	vscp_omsg.flags = VSCP_VALID_MSG + 3;
	vscp_omsg.class = VSCP_CLASS1_INFORMATION;
	vscp_omsg.type = VSCP_TYPE_INFORMATION_NODE_HEARTBEAT;
	
	for(i=0; i < sizeof(CONTROLSTATE); i++)
	{
		vscp_omsg.data[i] = p_char[i];	
	}
	
	// Send data
	if ( !vscp_sendEvent() ) {
		return FALSE;
	}

	return TRUE;
}


