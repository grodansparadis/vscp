///////////////////////////////////////////////////////////////////////////////
// File: vscp.c
//

/* ******************************************************************************
 * 	VSCP (Very Simple Control Protocol) 
 * 	http://www.vscp.org
 *
 * 	akhe@dofscandinavia.com
 *
 *  Copyright (C) 1995-2008 Ake Hedman, 
 *  D of Scandinavia, <akhe@dofscandinavia.com> 
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 
 *	This file is part of VSCP - Very Simple Control Protocol 	
 *	http://www.vscp.org
 *
 * ******************************************************************************
*/


//
// $RCSfile: vscp.c,v $
// $Revision: 1.6 $
//

#include <string.h>
#include <stdlib.h>
#include "can.h"
#include "uart.h"
#include "utils.h"
#include "vscp.h"
#include "vscp_class.h"
#include "vscp_type.h"

#ifndef FALSE
#define FALSE		0
#endif

#ifndef TRUE
#define TRUE    !FALSE
#endif

#ifndef ON
#define ON			!FALSE
#endif

#ifndef OFF
#define OFF			FALSE
#endif 


// Globals

// VSCP Data
uint8_t vscp_nickname;			// Node nickname

uint8_t vscp_errorcnt;			// VSCP/CAN errors
uint8_t vscp_alarmstatus;		// VSCP Alarm Status

uint8_t vscp_node_state;		// State machine state
uint8_t vscp_node_substate;	// State machine substate

uint8_t vscp_probe_cnt;			// Number of timout probes

// Incoming message
struct _imsg  vscp_imsg;

// Outgoing message
struct _omsg vscp_omsg;

uint8_t vscp_probe_address;			  // Address used during initialization
uint8_t vscp_initledfunc;			    // Init LED functionality

volatile uint16_t vscp_timer;		  // 1 ms timer counter
volatile uint8_t vscp_initbtncnt;	// init button counter
volatile uint8_t vscp_statuscnt;	// status LED counter

// page selector
uint16_t vscp_page_select;


///////////////////////////////////////////////////////////////////////////////
// sendVSCP
//

int8_t sendVSCPFrame( uint16_t vscpclass, 
						uint16_t vscptype, 
						uint8_t size, 
						uint8_t *pData )
{
	uint32_t id;
	
	id = ( (uint32_t)vscpclass << 16 ) + (uint32_t)( vscptype << 8 ) +  vscp_nickname;
	
	if ( !sendCANFrame( id, size, pData ) ) {
		return FALSE;
	}
	

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// getVSCPFrame
//

int8_t getVSCPFrame( uint16_t *pvscpclass, 
						uint16_t *pvscptype, 
						uint8_t *pNodeId, 
						uint8_t *psize, 
						uint8_t *pData )
{
	uint32_t id;
	
	if ( !getCANFrame( &id, psize, pData ) ) {
		return FALSE;
	}

	*pNodeId = id & 0x0ff;
	*pvscptype = ( id >> 8 ) & 0xff;
	*pvscpclass = ( id >> 16 ) & 0x1ff;

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_init
//

void vscp_init( void )
{
	vscp_initledfunc = VSCP_LED_BLINK1;

	// read the nickname id
	vscp_nickname = vscp_getNickname();		

	//	if zero set to uninitialized
	if ( !vscp_nickname ) vscp_nickname = VSCP_ADDRESS_FREE;

	// Init incoming message
	vscp_imsg.flag = 0;	
	vscp_imsg.priority = 0;
	vscp_imsg.class = 0;
	vscp_imsg.type = 0;

	// Init outgoing message
	vscp_omsg.flags = 0;
	vscp_omsg.priority = 0;	
	vscp_omsg.class = 0;
	vscp_omsg.type = 0;

	vscp_errorcnt = 0;					// No errors yet
	vscp_alarmstatus = 0;				// No alarmstatus

	vscp_probe_address = 0;

	// Initial state
	vscp_node_state = VSCP_STATE_STARTUP;
	vscp_node_substate = VSCP_SUBSTATE_NONE;

	vscp_timer = 0;
	vscp_probe_cnt = 0;
	vscp_page_select = 0;
	
}

///////////////////////////////////////////////////////////////////////////////
// vscp_check_pstorage
//
// Check control position integrity and restore EEPROM
// if invalid.
//

int8_t vscp_check_pstorage( void )
{
	// controlbyte == 01xxxxxx means initialized
	// everything else is uninitialized
	if ( ( vscp_getSegmentCRC() & 0xc0 ) == 0x40 ) {
		return TRUE;
	}
	// No nickname yet.
	vscp_setNickname( 0xff );

	// No segment CRC yet.
	vscp_setSegmentCRC( 0x00 );

	// Initial startup
	// write allowed
	vscp_setControlByte( 0x90 );

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_error
//

void vscp_error( void )
{
	vscp_initledfunc = VSCP_LED_OFF;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_probe
//

void vscp_probe( void )
{
	switch ( vscp_node_substate ) {
	
		case VSCP_SUBSTATE_NONE:

			if ( VSCP_ADDRESS_FREE != vscp_probe_address ) {
				
				vscp_omsg.flags = VSCP_VALID_MSG + 1 ;	// one databyte 
				vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
				vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
				vscp_omsg.type = VSCP_TYPE_PROTOCOL_NEW_NODE_ONLINE;
				vscp_omsg.data[ 0 ] = vscp_probe_address;			
				
				// send the probe
				vscp_sendEvent();

				vscp_node_substate = VSCP_SUBSTATE_INIT_PROBE_SENT;
				vscp_timer = 0;	
				
			}
			else {

				// No free address -> error
				vscp_node_state = VSCP_STATE_ERROR;
				
				// Tell system we are giving up
				vscp_omsg.flags = VSCP_VALID_MSG + 1 ;	// one databyte 
				vscp_omsg.priority = VSCP_PRIORITY_HIGH;
				vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
				vscp_omsg.type = VSCP_TYPE_PROTOCOL_NO_NICKNAME;			
				
				// send the error event
				vscp_sendEvent();

			}
			break;	

		case VSCP_SUBSTATE_INIT_PROBE_SENT:
			
			if ( vscp_imsg.flag & VSCP_VALID_MSG ) {	// incoming message?
				PrintString( (UINT8 const *)"\n\rricevuto" );
				// Yes, incoming message
				if ( ( VSCP_CLASS1_PROTOCOL == vscp_imsg.class ) && 
					( VSCP_TYPE_PROTOCOL_PROBE_ACK == vscp_imsg.type ) ) {
					//PrintString( "\n\rricevuto" );
					// Yes it was an ack from the segment master or a node
					if ( 0 == vscp_probe_address ) {

						// Master controller answered
						// wait for address
						vscp_node_state = VSCP_STATE_PREACTIVE;
						vscp_timer = 0; // reset timer
						PrintString((UINT8 const *)"\n\rVSCP PRECATIVATE");
					}
					else {

						// node answered, try next address
						vscp_probe_address++;
						vscp_node_substate = VSCP_SUBSTATE_NONE;
						vscp_probe_cnt = 0;
						
					}	
				}
			}
			else {

				if ( vscp_timer > VSCP_PROBE_TIMEOUT ) {	// Check for timeout
 
					vscp_probe_cnt++;	// Another timeout
					
					if ( vscp_probe_cnt > 3 ) { 
					
						// Yes we have a timeout
						if ( 0 == vscp_probe_address ) {			// master controller probe?
	
							// No master controler on segment, try next node
							vscp_probe_address++;
							vscp_node_substate = VSCP_SUBSTATE_NONE;

						}
						else {
							 
							// We have found a free address - use it
							vscp_nickname = vscp_probe_address;
							vscp_node_state = VSCP_STATE_ACTIVE;
							vscp_node_substate = VSCP_SUBSTATE_NONE;
							vscp_setNickname( vscp_nickname );
							vscp_setSegmentCRC( 0x40 );			// segment code (non server segment )

							// Report success
							vscp_probe_cnt = 0;
							vscp_active();
							PrintString( (UINT8 const *)"\n\rnickname:" );
							PrintInt((UINT32)vscp_nickname);
							PrintString((UINT8 const *)"\n\rVSCP ACTIVE");
						
						}
					}
					else {
						vscp_node_substate = VSCP_SUBSTATE_NONE;
					}
				} // Timeout

			}			
			break;

		case VSCP_SUBSTATE_INIT_PROBE_ACK:
			break;

		default:
			vscp_node_substate = VSCP_SUBSTATE_NONE;
			break;
	}

	vscp_imsg.flag = 0;	
	
}

///////////////////////////////////////////////////////////////////////////////
// vscp_preactive
//

void vscp_rcv_preactive( void )
{
	if ( vscp_imsg.flag & VSCP_VALID_MSG ) {	// incoming message?

		if ( ( VSCP_CLASS1_PROTOCOL == vscp_imsg.class )  && 
						(  VSCP_TYPE_PROTOCOL_SET_NICKNAME == vscp_imsg.type ) &&
						(  VSCP_ADDRESS_FREE == vscp_imsg.data[ 0 ] ) ) {
						
			// Assign nickname
			vscp_nickname = vscp_imsg.data[ 2 ];
			vscp_setNickname( vscp_nickname );	
			vscp_setSegmentCRC( 0x40 );
	
			// Go active state
			vscp_node_state = VSCP_STATE_ACTIVE;
			PrintString((UINT8 const *)"\n\rNICKNAME=");
			PrintInt((UINT32)vscp_nickname);
			PrintString((UINT8 const *)"\n\rVSCP ACTIVE");
		}		
	}
	else {
		// Check for time out
		if ( vscp_timer > VSCP_PROBE_TIMEOUT ) {
			// Yes, we have a timeout
			vscp_nickname = VSCP_ADDRESS_FREE;
			vscp_setNickname( VSCP_ADDRESS_FREE );
			vscp_init();	
		}				
	}
}

///////////////////////////////////////////////////////////////////////////////
// vscp_active
//

void vscp_active( void )
{
	vscp_omsg.flags = VSCP_VALID_MSG + 1 ;	// one databyte 
	vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
	vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
	vscp_omsg.type = VSCP_TYPE_PROTOCOL_NEW_NODE_ONLINE;
	vscp_omsg.data[ 0 ] = vscp_nickname;			
				
	// send the message
	vscp_sendEvent();
	
	vscp_initledfunc = VSCP_LED_ON;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_sendEvent
//

int8_t vscp_sendEvent( void )
{
    uint32_t id = ( (uint32_t)vscp_omsg.priority << 26 ) |
						( (uint32_t)vscp_omsg.class << 16 ) |
						( (uint32_t)vscp_omsg.type << 8) |
							 vscp_nickname;		// nodeaddress (our address)

	if ( !sendCANFrame( id, (vscp_omsg.flags & 0x0f), vscp_omsg.data ) ) {
		// Failed to send message
		vscp_errorcnt++;
		return FALSE;
	}
	vscp_omsg.flags = 0;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_getEvent
//

int8_t vscp_getEvent( void )
{
	uint32_t id;
	uint8_t dlc;

	// Dont read in new message if there already is a message
	// in the input buffer
	if ( vscp_imsg.flag & VSCP_VALID_MSG ) return FALSE;

	if ( getCANFrame( &id, &dlc, vscp_imsg.data ) ) {

		vscp_imsg.flag = dlc;
		vscp_imsg.priority = (uint16_t)( 0x07 & ( id >> 26 ) );
		vscp_imsg.class = (uint16_t)( 0x1ff & ( id >> 16 ) );
		vscp_imsg.type = (uint8_t)( 0xff & ( id >> 8 ) ); 
		vscp_imsg.oaddr = (uint8_t)( 0xff & id );

		vscp_imsg.flag |= VSCP_VALID_MSG;

		return TRUE;
	}	
	
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_rcv_heartbeat
//

void vscp_rcv_heartbeat( void )
{
	if ( ( 5 == ( vscp_imsg.flag & 0x0f ) ) && 
			( vscp_getSegmentCRC() != vscp_imsg.data[ 0 ] ) ) {

		// Stored CRC are different than received
		// We must be on a different segment
		vscp_setSegmentCRC( vscp_imsg.data[ 0 ] );
		
		// Introduce ourself in the proper way and start from the beginning
		vscp_nickname = VSCP_ADDRESS_FREE;
		vscp_setNickname( VSCP_ADDRESS_FREE );
		vscp_node_state = VSCP_STATE_INIT;

	}		
}

///////////////////////////////////////////////////////////////////////////////
// vscp_rcv_set_nickname
//

void vscp_rcv_set_nickname( void )
{
	if ( ( 2 == ( vscp_imsg.flag & 0x0f ) ) && 
					( vscp_nickname == vscp_imsg.data[ 0 ] ) ) {

		// Yes, we are addressed
		vscp_nickname = vscp_imsg.data[ 1 ];
		vscp_setNickname( vscp_nickname );		
		vscp_setSegmentCRC( 0x40 );
	}	
}

///////////////////////////////////////////////////////////////////////////////
// vscp_rcv_drop_nickname
//

void vscp_rcv_drop_nickname( void )
{
	if ( ( 1 == ( vscp_imsg.flag & 0x0f ) ) && 
					( vscp_nickname == vscp_imsg.data[ 0 ] ) ) {
		
		// Yes, we are addressed
		vscp_nickname = VSCP_ADDRESS_FREE;
		vscp_setNickname( VSCP_ADDRESS_FREE );
		vscp_init();

	}
}

///////////////////////////////////////////////////////////////////////////////
// vscp_rcv_new_node_online
//

void vscp_rcv_new_node_online( void )
{
	PrintString( (UINT8 const *)"\n\rNEW_NODE_ONLINE" );
	if ( ( 1 == ( vscp_imsg.flag & 0x0f ) ) && 
					( vscp_nickname == vscp_imsg.data[ 0 ] ) ) {

		// This is a probe which use our nickname
		// we have to respond to tell the new node that
		// ths nickname is in use
		//PrintString( "\n\rNEW_NODE_ONLINE" );
		vscp_omsg.flags = VSCP_VALID_MSG;
		vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
		vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
		vscp_omsg.type = VSCP_TYPE_PROTOCOL_PROBE_ACK;		
		vscp_sendEvent();				

	}
}

///////////////////////////////////////////////////////////////////////////////
// vscp_rcv_readreg
//

void vscp_rcv_readreg( void )
{
	vscp_omsg.priority = VSCP_PRIORITY_HIGH;
	vscp_omsg.flags = VSCP_VALID_MSG + 2;
	vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
	vscp_omsg.type = VSCP_TYPE_PROTOCOL_RW_RESPONSE;
	
	vscp_omsg.data[ 0 ] =  vscp_imsg.data[ 1 ];
	vscp_omsg.data[ 1 ] =  0;  // Default

	if ( VSCP_REG_ALARMSTATUS == vscp_imsg.data[ 1 ] ) {

		// * * * Read alarm status register * * * 
		vscp_omsg.data[ 1 ] = vscp_alarmstatus;
		vscp_alarmstatus = 0x00;		// Reset alarm status

	}

	else if ( VSCP_REG_VSCP_MAJOR_VERSION == vscp_imsg.data[ 1 ] ) {

		// * * * VSCP Protocol Major Version * * * 
		vscp_omsg.data[ 1 ] = VSCP_MAJOR_VERSION;	

	}

	else if ( VSCP_REG_VSCP_MINOR_VERSION == vscp_imsg.data[ 1 ] ) {
		
 		// * * * VSCP Protocol Minor Version * * * 
		vscp_omsg.data[ 1 ] = VSCP_MINOR_VERSION;	

	}

	else if ( VSCP_REG_NODE_CONTROL == vscp_imsg.data[ 1 ] ) {
		
 		// * * * Reserved * * * 
		vscp_omsg.data[ 1 ] = 0;	

	}

	else if ( VSCP_FIRMWARE_MAJOR_VERSION == vscp_imsg.data[ 1 ] ) {
		
 		// * * * Get firmware Major version * * * 
		vscp_omsg.data[ 1 ] = vscp_getMajorVersion();	

	}

	else if ( VSCP_FIRMWARE_MINOR_VERSION == vscp_imsg.data[ 1 ] ) {
		
 		// * * * Get firmware Minor version * * * 
		vscp_omsg.data[ 1 ] = vscp_getMinorVersion();	

	}

	else if ( VSCP_FIRMWARE_SUB_MINOR_VERSION == vscp_imsg.data[ 1 ] ) {
		
 		// * * * Get firmware Sub Minor version * * * 
		vscp_omsg.data[ 1 ] = vscp_getSubMinorVersion();	

	}

	else if ( vscp_imsg.data[ 1 ] < VSCP_REG_MANUFACTUR_ID0 ) {

		// * * * Read from persitant locations * * * 
		vscp_omsg.data[ 1 ] = vscp_getUserID( vscp_imsg.data[ 1 ] - VSCP_REG_USERID0 );	

	}

	else if ( ( vscp_imsg.data[ 1 ] > VSCP_REG_USERID4 )  &&  ( vscp_imsg.data[ 1 ] < VSCP_REG_NICKNAME_ID ) ) {
		
 		// * * * Manufacturer ID information * * * 
		vscp_omsg.data[ 1 ] = vscp_getManufacturerId( vscp_imsg.data[ 1 ] - VSCP_REG_MANUFACTUR_ID0 ); 			

	}

	else if ( VSCP_REG_NICKNAME_ID == vscp_imsg.data[ 1 ] ) {

		// * * * nickname id * * * 
		vscp_omsg.data[ 1 ] = vscp_nickname;

	}

	else if ( VSCP_REG_PAGE_SELECT_LSB == vscp_imsg.data[ 1 ] ) {
		
 		// * * * Page select LSB * * * 
		vscp_omsg.data[ 1 ] = ( vscp_page_select & 0xff );	

	}	

	else if ( VSCP_REG_PAGE_SELECT_MSB == vscp_imsg.data[ 1 ] ) {
		
 		// * * * Page select MSB * * * 
		vscp_omsg.data[ 1 ] = ( vscp_page_select >> 8 ) & 0xff;	

	}
	
	else if ( VSCP_BOOT_LOADER_ALGORITHM == vscp_imsg.data[ 1 ] ) {
		// * * * Boot loader algorithm * * *
		vscp_omsg.data[ 1 ] = VSCP_BOOTLOADER_PIC1;
	}
	
	else if ( VSCP_BUFFER_SIZE == vscp_imsg.data[ 1 ] ) {
		// * * * Buffer size * * *
		vscp_omsg.data[ 1 ] = vscp_getBufferSize();	
	}
			
	else if ( ( vscp_imsg.data[ 1 ] > ( VSCP_REG_GUIID - 1 ) ) && ( vscp_imsg.data[ 1 ] < VSCP_REG_DEVICE_URL )  ) {

		// * * * GUID * * * 
		vscp_omsg.data[ 1 ] = vscp_getGUID( vscp_imsg.data[ 1 ] - VSCP_REG_GUIID );		

	}

	else {

		// * * * The device URL * * * 
		vscp_omsg.data[ 1 ] = vscp_getMDF_URL( vscp_imsg.data[ 1 ] - VSCP_REG_DEVICE_URL );	

	}

	vscp_sendEvent();
}

///////////////////////////////////////////////////////////////////////////////
// vscp_rcv_writereg
//

void vscp_rcv_writereg( void )
{
	vscp_omsg.priority = VSCP_PRIORITY_MEDIUM;
	vscp_omsg.flags = VSCP_VALID_MSG + 2;
	vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
	vscp_omsg.type = VSCP_TYPE_PROTOCOL_RW_RESPONSE;
	vscp_omsg.data[ 0 ] = vscp_nickname; 

	if ( ( vscp_imsg.data[ 1 ] > ( VSCP_REG_VSCP_MINOR_VERSION + 1 ) ) && 
		( vscp_imsg.data[ 1 ] < VSCP_REG_MANUFACTUR_ID0 ) ) {
	 
		// * * * User Client ID * * *
		vscp_setUserID( ( vscp_imsg.data[ 1 ] - VSCP_REG_USERID0 ), vscp_imsg.data[ 2 ] );		
		vscp_omsg.data[ 1 ] = vscp_getUserID( ( vscp_imsg.data[ 1 ] - VSCP_REG_USERID0 ) );
	
	}

	else if ( VSCP_REG_PAGE_SELECT_MSB == vscp_imsg.data[ 1 ] ) {

		// * * * Page select register MSB * * *
		vscp_page_select = ( vscp_page_select & 0xff00 ) | ( (uint16_t)vscp_imsg.data[ 2 ] << 8 );
		vscp_omsg.data[ 1 ] = ( vscp_page_select >> 8 ) & 0xff;
	}
	
	else if ( VSCP_REG_PAGE_SELECT_LSB == vscp_imsg.data[ 1 ] ) {

		// * * * Page select register LSB * * *
		vscp_page_select = ( vscp_page_select & 0xff ) | vscp_imsg.data[ 2 ];
		vscp_omsg.data[ 1 ] = ( vscp_page_select & 0xff );
	}	

#ifdef ENABLE_WRITE_2PROTECTED_LOCATIONS

	// Write manufacturer id configuration information
	else if ( ( vscp_imsg.data[ 1 ] > VSCP_REG_USERID4 )  &&  ( vscp_imsg.data[ 1 ] < VSCP_REG_NICKNAME_ID ) ) {
		// page select must be 0xffff for writes to be possible
		if ( ( 0xff != ( ( vscp_page_select >> 8 ) & 0xff ) ) || 
				( 0xff != ( vscp_page_select & 0xff ) ) ) {
			// return complement to indicate error
			vscp_omsg.data[ 1 ] = ~vscp_imsg.data[ 2 ];	
		}
		else {
			// Write
			vscp_setManufacturerId( vscp_imsg.data[ 1 ] - VSCP_REG_MANUFACTUR_ID0, vscp_imsg.data[ 2 ] );
			vscp_omsg.data[ 1 ] = vscp_getManufacturerId( vscp_imsg.data[ 1 ] - VSCP_REG_MANUFACTUR_ID0 );
		}
	}
	// Write GUID configuration information
	else if ( ( vscp_imsg.data[ 1 ] > ( VSCP_REG_GUIID - 1 ) ) && ( vscp_imsg.data[ 1 ] < VSCP_REG_DEVICE_URL )  ) {
		// page must be 0xffff for writes to be possible
		if ( ( 0xff != ( ( vscp_page_select >> 8 ) & 0xff ) ) || 
				( 0xff != ( vscp_page_select & 0xff ) ) )  {
			// return complement to indicate error
			vscp_omsg.data[ 1 ] = ~vscp_imsg.data[ 2 ];	
		}
		else {
			vscp_setGUID( vscp_imsg.data[ 1 ] - VSCP_REG_GUIID, vscp_imsg.data[ 2 ] );
			vscp_omsg.data[ 1 ] = vscp_getGUID( vscp_imsg.data[ 1 ] - VSCP_REG_GUIID );
		}
	}
	
#endif

	else {
		// return complement to indicate error
		vscp_omsg.data[ 1 ] = ~vscp_imsg.data[ 2 ];		
	}

	vscp_sendEvent();	
}


