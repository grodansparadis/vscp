/* ******************************************************************************
 * 	VSCP (Very Simple Control Protocol) 
 * 	http://www.vscp.org
 *
 *  IAR STR 73x demo board Module
 * 	Version information in version.h
 * 	akhe@dofscandinavia.com   
 *
 * Copyright (C) 1995-2008 Ake Hedman, 
 * D of Scandinavia, <akhe@dofscandinavia.com>
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
 * ******************************************************************************/

#include "LPC23xx.H"
#include "config.h"
#include <string.h>
#include <inttypes.h>
#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>
#include "can.h"
#include "utils.h"
#include "uart.h"
#include "edt_mng.h"
#include "emem.h"
#include "flash.h"
#include "upload.h"
#include "vscp_events.h"
#include "vscp_main.h"

#define VSCP_TYPE_PROTOCOL_ACTIVATENEWIMAGE_ACK		VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE + 1	// DA AGGIUNGERE (AKE)			
#define VSCP_TYPE_PROTOCOL_ACTIVATENEWIMAGE_NACK	VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE + 2	// DA AGGIUNGERE (AKE)
#define VSCP_TYPE_PROTOCOL_START_BLOCK_ACK			VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE + 3	// DA AGGIUNGERE (AKE)
#define VSCP_TYPE_PROTOCOL_START_BLOCK_NACK			VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE + 4	// DA AGGIUNGERE (AKE)


// Prototypes
void wait( void );

void init( void );
void init_app_eeprom( void );
void init_app_ram( void );

void doWork( void );

// VSCP Module firmware version
#define FIRMWARE_MAJOR_VERSION			0;
#define FIRMWARE_MINOR_VERSION			0;
#define FIRMWARE_SUB_MINOR_VERSION		1;

// The device URL (max 32 characters including null termination)
const char vscp_deviceURL[] = "www.dofscandinavia.com/demo73x_001.xml";

// The device GUID
const char vscp_deviceGUID[] = { 0x01, 0x00, 0x00, 0x00,
																	0x00, 0x00, 0x00, 0x00,
																	0x00, 0x00, 0x00, 0x00,
																	0xff, 0xff, 0xff, 0xff
																};

// The device USER id - normally in EEPROM															
const char vscp_userID[] = { 0x10, 0x20, 0x30, 0x40, 0x50 };	

// The device manufacturer id - normally in EEPROM														
const char vscp_ManufacturerDeviceID[] = { 0x11, 0x20, 0x30, 0x40,
										   0x64, 0x65, 0x40, 0x00 };	
											
// Simulated eeprom storage
//uint8_t eeprom_nickname;
uint8_t eeprom_segmentCRC;

volatile unsigned long measurement_clock;	// Clock for measurments
uint8_t boot_mode = 0;	
																						
uint8_t seconds;							// counter for seconds
uint8_t minutes;							// counter for minutes
uint8_t hours;								// Counter for hours

	
int exec_vscp(void)
{
	
	// Check for any valid CAN message
	vscp_imsg.flag = 0;
	vscp_getEvent();
	
	// do a meaurement if needed
	if(vscp_node_state == VSCP_STATE_ACTIVE)
		doMeasurement();
	
	if ( measurement_clock > 1000 ) 
	{
		measurement_clock = 0;
		
		if(vscp_node_state == VSCP_STATE_ACTIVE)
			sendHeartBeat();
		
		seconds++;
		if ( seconds > 59 ) 
		{
			seconds = 0;
			minutes++;
			if ( minutes > 59 ) 
			{
				minutes = 0;
				hours++;
			}
			if ( hours > 23 ) 
			{
				hours = 0;
			}
		}				
	}
	
	switch ( vscp_node_state ) 
	{
		case VSCP_STATE_STARTUP:			// Cold/warm reset
			PrintString((UINT8 const *)"\n\rVSCP STARTUP");
			// Get nickname from EEPROM
			if ( VSCP_ADDRESS_FREE == vscp_nickname ) 
			{
				// new on segment need a nickname
				vscp_node_state = VSCP_STATE_INIT;
				PrintString((UINT8 const *)"\n\rVSCP INIT");
			}
			else 
			{
				PrintString((UINT8 const *)"\n\rNICKNAME=");
				PrintInt((UINT32)vscp_nickname);
				PrintString((UINT8 const *)"\n\rVSCP ACTIVE");
				// been here before - go on
				vscp_node_state = VSCP_STATE_ACTIVE;
				vscp_active();
			}
			break;
	
		case VSCP_STATE_INIT:			// Assigning nickname
			vscp_probe();
			break;

		case VSCP_STATE_PREACTIVE:		// Waiting for host initialisation
			vscp_rcv_preactive();					
			break;

		case VSCP_STATE_ACTIVE:			// The normal state
							
			if ( vscp_imsg.flag & VSCP_VALID_MSG ) 	// incoming event?
			{	
				// Yes, incoming message
				if ( VSCP_CLASS1_PROTOCOL == vscp_imsg.class ) 
				{	
					switch( vscp_imsg.type ) 
					{
						case VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT:
							vscp_rcv_heartbeat();
							break;
	
						case VSCP_TYPE_PROTOCOL_NEW_NODE_ONLINE:
							vscp_rcv_new_node_online();
							break;
									
						case VSCP_TYPE_PROTOCOL_SET_NICKNAME:
							vscp_rcv_set_nickname();
							break;
								
						case VSCP_TYPE_PROTOCOL_DROP_NICKNAME:
							vscp_rcv_drop_nickname();
							break;
	
						case VSCP_TYPE_PROTOCOL_READ_REGISTER:
							if ( ( 2 == ( vscp_imsg.flag & 0x0f ) ) && 
											( vscp_nickname == vscp_imsg.data[ 0 ] ) ) 
							{
	
								if ( vscp_imsg.data[ 1 ] >= 0x80 )
								{
									// Read VSCP register
									vscp_rcv_readreg();
								}
							}
							break;
	
						case VSCP_TYPE_PROTOCOL_WRITE_REGISTER:
							if ( ( 3 == ( vscp_imsg.flag & 0x0f ) ) && 
											( vscp_nickname == vscp_imsg.data[ 0 ] ) ) 
							{
								if ( vscp_imsg.data[ 1 ] >= 0x80 ) 
								{
									// Write VSCP register
									vscp_rcv_writereg();
								}
							}
							break;
							
														
						case VSCP_TYPE_PROTOCOL_ENTER_BOOT_LOADER:
							if ( (vscp_nickname == vscp_imsg.data[ 0 ]) && (vscp_getBootLoaderAlgorithm() == vscp_imsg.data[ 1 ]))  
							{
								vscp_omsg.flags = VSCP_VALID_MSG + 8 ;	 
								vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
								vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
								vscp_omsg.type = VSCP_TYPE_PROTOCOL_ACK_BOOT_LOADER;
								vscp_omsg.data[ 0 ] = 2;	// MSB internal flash block size
								vscp_omsg.data[ 1 ] = 2; 	// MSB spi flash block size
								vscp_omsg.data[ 2 ] = 0;	// LSB spi flash block size
								vscp_omsg.data[ 3 ] = 0;	// LSB internal flash block size
								vscp_omsg.data[ 4 ] = 1;	// MSB internal flash number of block avalaible
								vscp_omsg.data[ 5 ] = 4;	// MSB spi flash number of block avalaible
								vscp_omsg.data[ 6 ] = 0;	// LSB spi flash number of block avalaible
								vscp_omsg.data[ 7 ] = 0;	// LSB internal flash number of block avalaible
								
								vscp_setBootLoaderMode();
								
								// send the probe
								vscp_sendEvent();
							}
							break;
							
						case VSCP_TYPE_PROTOCOL_START_BLOCK:
							if ( vscp_nickname == vscp_imsg.data[ 2 ])
							{
								vscp_setBootLoaderMode();
								switch(HandleStartBlock(&vscp_imsg.data[ 0 ],&vscp_omsg.data[ 0 ]))
								{
									case 0:
										vscp_omsg.flags = VSCP_VALID_MSG + 4 ;	 
										vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
										vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
										vscp_omsg.type = VSCP_TYPE_PROTOCOL_START_BLOCK_ACK;
										//PrintString((UINT8 const *)"\n\rDEBUG:VSCP_TYPE_PROTOCOL_START_BLOCK_ACK");
										vscp_sendEvent();
										break;
									case 1:
										vscp_omsg.flags = VSCP_VALID_MSG + 4 ;	 
										vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
										vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
										vscp_omsg.type = VSCP_TYPE_PROTOCOL_START_BLOCK_NACK;
										PrintString((UINT8 const *)"\n\rDEBUG:VSCP_TYPE_PROTOCOL_START_BLOCK_NACK");
										vscp_sendEvent();
										break;
									default:
										break;
								}
								break;
							}
						case VSCP_TYPE_PROTOCOL_BLOCK_DATA:
							if(vscp_getBootLoaderMode()==VSCP_BOOT_FLAG)
							{
								switch(HandleBlockData(&vscp_imsg.data[ 0 ],&vscp_omsg.data[ 0 ]))
								{
									case 0:
										vscp_omsg.flags = VSCP_VALID_MSG + 6 ;	 
										vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
										vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
										vscp_omsg.type = VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK;
										//PrintString((UINT8 const *)"\n\rDEBUG:VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK");
										//PrintString((UINT8 const *)"\n\rblock ack");
										vscp_sendEvent();
										break;
									case 1:
										vscp_omsg.flags = VSCP_VALID_MSG + 5 ;	 
										vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
										vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
										vscp_omsg.type = VSCP_TYPE_PROTOCOL_BLOCK_DATA_NACK;
										PrintString((UINT8 const *)"\n\rDEBUG:VSCP_TYPE_PROTOCOL_BLOCK_DATA_NACK");
										vscp_sendEvent();
										break;
									default:
										break;
								}
							}
							break;
						case VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA:
							if(vscp_getBootLoaderMode()==VSCP_BOOT_FLAG)
							{
								switch(HandleProgramBlock(&vscp_imsg.data[ 0 ],&vscp_omsg.data[ 0 ]))
								{
									case 0:
										vscp_omsg.flags = VSCP_VALID_MSG + 4 ;	 
										vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
										vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
										vscp_omsg.type = VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK;
										vscp_sendEvent();
										break;
									default:
										vscp_omsg.flags = VSCP_VALID_MSG + 5 ;	 
										vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
										vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
										vscp_omsg.type = VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_NACK;
										PrintString((UINT8 const *)"\n\rDEBUG:VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_NACK");
										vscp_sendEvent();
										break;
								}
							}
							break;
						case VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE:
							if(vscp_getBootLoaderMode()==VSCP_BOOT_FLAG)
							{
								switch(HandleActivateNewImage(&vscp_imsg.data[ 0 ],&vscp_omsg.data[ 0 ]))
								{
									case 0:
										vscp_omsg.flags = VSCP_VALID_MSG + 8 ;	 
										vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
										vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
										vscp_omsg.type = VSCP_TYPE_PROTOCOL_ACTIVATENEWIMAGE_ACK;
										vscp_omsg.data[ 0 ] = 2;	// MSB internal flash block size
										vscp_omsg.data[ 1 ] = 2; 	// MSB spi flash block size
										vscp_omsg.data[ 2 ] = 0;	// LSB spi flash block size
										vscp_omsg.data[ 3 ] = 0;	// LSB internal flash block size
										vscp_omsg.data[ 4 ] = 1;	// MSB internal flash number of block avalaible
										vscp_omsg.data[ 5 ] = 4;	// MSB spi flash number of block avalaible
										vscp_omsg.data[ 6 ] = 0;	// LSB spi flash number of block avalaible
										vscp_omsg.data[ 7 ] = 0;	// LSB internal flash number of block avalaible
										vscp_sendEvent();
										
										vscp_resetBootLoaderMode();
										
										save_nickname_upgrade();
										// jump to user program ...
										execute_reset();//execute_user_code();
										break;
									default:
										vscp_omsg.flags = VSCP_VALID_MSG + 1 ;	 
										vscp_omsg.priority = VSCP_PRIORITY_HIGH;	
										vscp_omsg.class = VSCP_CLASS1_PROTOCOL;
										vscp_omsg.type = VSCP_TYPE_PROTOCOL_ACTIVATENEWIMAGE_NACK;
										vscp_sendEvent();
										break;
								}
							}
							break;
	
						default:
							// Do work load
							break;
					}							
				} 
			} // Incoming event
			break;

		case VSCP_STATE_ERROR:			// Everything is *very* *very* bad.
			PrintString((UINT8 const *)"\n\rVSCP ERROR");
			vscp_error();
			break;

		default:					// Should not be here...
			vscp_node_state = VSCP_STATE_STARTUP;
			break;

	} // switch 

	doWork();	
	
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// init
//
// Initialize the controller and pheriperals
//

void open_vscp( void ) 
{
	init_app_ram();

  	open_can();
  	
  	// Check VSCP persistent storage and
	// restore if needed
	if ( !vscp_check_pstorage() ) {
	
		// Spoiled or not initialized - reinitialize
		init_app_eeprom();	

	}

	vscp_init();			// Initialize the VSCP functionality
	
}


///////////////////////////////////////////////////////////////////////////////
// init_app_ram
//

void init_app_ram( void )
{
	//eeprom_nickname = VSCP_ADDRESS_FREE;
	eeprom_segmentCRC = 0x40;
}


////////////////////////////////////////////////////////////////////////////////
// init_app_eeprom
//

void init_app_eeprom( void )
{
	// Nothing to do as we are out of EEPROM
}

////////////////////////////////////////////////////////////////////////////////
//  wait


void wait( void )
{
  	UINT32 i;

  	for (i=100000L; i!=0 ; i-- ) {}
}

///////////////////////////////////////////////////////////////////////////////
//                             APPLICATION
//////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// doWork
//

void doWork( void ) 
{

}

///////////////////////////////////////////////////////////////////////////////
//                        VSCP Required Methods
//////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//  sendCANFrame
//

int8_t sendCANFrame( uint32_t id, uint8_t dlc, uint8_t *pdata )
{
	CAN_TX msg;
		
	msg.ID = id;
	msg.TFI = 0x80000000 | (dlc << 16);	// extended id + dlc
	memcpy( msg.DataField, pdata, dlc );
			
	// Send the frame 
	if ( !can_sendmessage( CANCHANNEL_1,&msg ) ) 
	{
		// Failed to send message
		return FALSE;
	}
	vscp_omsg.flags = 0;	// Out msg sent
	
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
//  getCANFrame
//

int8_t getCANFrame( uint32_t *pid, uint8_t *psize, uint8_t *pData )
{
	CAN_RX msg;

	// Dont read in new message if there already is a message
	// in the input buffer
	if ( vscp_imsg.flag & VSCP_VALID_MSG ) return FALSE;

	if(can_receivemessage(CANCHANNEL_1,&msg))
	{
		// We already no this is an extended id so we dont't bother
		// with that info.
		*pid = msg.ID;	
		*psize = (msg.RFS >> 16)&0xF;
		memcpy( pData, msg.DataField, *psize );
		return TRUE;
	}
	return FALSE;
}



///////////////////////////////////////////////////////////////////////////////
// Get Major version number for this hardware module
//

unsigned char vscp_getMajorVersion()
{
	return FIRMWARE_MAJOR_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// Get Minor version number for this hardware module
//

unsigned char vscp_getMinorVersion()
{
	return FIRMWARE_MINOR_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// Get Subminor version number for this hardware module
//

unsigned char vscp_getSubMinorVersion()
{
	return FIRMWARE_SUB_MINOR_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// getVSCP_GUID
//
// Get GUID from EEPROM
//

uint8_t vscp_getGUID( uint8_t idx )
{
	return vscp_deviceGUID[ idx ];		
}


///////////////////////////////////////////////////////////////////////////////
// getDeviceURL
//
// Get device URL from EEPROM
//

uint8_t vscp_getMDF_URL( uint8_t idx )
{
	return vscp_deviceURL[ idx ];
}


//////////////////////////////////////////////////////////////////////////////
// Get Manufacturer id and subid from EEPROM
//

uint8_t vscp_getUserID( uint8_t idx )
{
	return vscp_userID[ idx ];	
}


///////////////////////////////////////////////////////////////////////////////
//  setVSCPUserID
//

void vscp_setUserID( uint8_t idx, uint8_t data )
{
	// Normally this should be stired in EEPROM but
	// no EEPROM is available on this board so we just
	// ignore this request.
}

///////////////////////////////////////////////////////////////////////////////
// getVSCPManufacturerId
// 
// Get Manufacturer id and subid from EEPROM
//

uint8_t vscp_getManufacturerId( uint8_t idx )
{
	return vscp_ManufacturerDeviceID[ idx ];	
}

///////////////////////////////////////////////////////////////////////////////
// setVSCPManufacturerId
// 
// Set Manufacturer id and subid from EEPROM
//

void vscp_setManufacturerId( uint8_t idx, uint8_t data )
{
	// Normally this should be stired in EEPROM but
	// no EEPROM is available on this board so we just
	// ignore this request.	
}


///////////////////////////////////////////////////////////////////////////////
// Get the bootloader algorithm code
//

uint8_t vscp_getBootLoaderAlgorithm( void ) 
{
	return VSCP_BOOTLOADER_LPC1;	
}

///////////////////////////////////////////////////////////////////////////////
// Set the bootloader mode
//

void vscp_setBootLoaderMode( void ) 
{
	// rovinare la signature in flash ci pensa il chip erase
	
	boot_mode = VSCP_BOOT_FLAG;
}

///////////////////////////////////////////////////////////////////////////////
// Get the bootloader mode
//

uint8_t vscp_getBootLoaderMode( void ) 
{
	return boot_mode;
}

///////////////////////////////////////////////////////////////////////////////
// Exit from bootloader mode
//

void vscp_resetBootLoaderMode( void ) 
{
	// firma in flash
	write_flash_signature();
	
	// rovinare il magic in eeprom è necessario per fare il memory puff
	// dopo ogni update del firmware perchè le strutture dati possono
	// essere cambiate, questo comporta la cancellazione del nickname e
	// quindi l' invocazione del nickname discovery alla fine della procedura
	// di update
	EditWriteMagic(VSCP_BOOT_FLAG);
	
	boot_mode = 0;
}

//////////////////////////////////////////////////////////////////////////////
// Get the buffer size
//

uint8_t vscp_getBufferSize( void ) 
{
	return 8;	// Standard CAN frame
}


///////////////////////////////////////////////////////////////////////////////
//  getNickname
//

uint8_t vscp_getNickname( void )
{
	CONFIG config;
	EditReadConfig(&config);
	return config.m_nickname;
}


///////////////////////////////////////////////////////////////////////////////
//  setNickname
//

void vscp_setNickname( uint8_t nickname )
{
	CONFIG config;
	EditReadConfig(&config);
	config.m_nickname = nickname;
	EditWriteConfig(&config);
}

///////////////////////////////////////////////////////////////////////////////
//  getSegmentCRC
//

uint8_t vscp_getSegmentCRC( void )
{
	// Normally this should be stired in EEPROM but
	// no EEPROM is available on this board so we just
	// ignore this request
	return eeprom_segmentCRC;
}

///////////////////////////////////////////////////////////////////////////////
//  setSegmentCRC
//

void vscp_setSegmentCRC( uint8_t crc )
{
	// Normally this should be stired in EEPROM but
	// no EEPROM is available on this board so we just
	// ignore this request
	eeprom_segmentCRC = crc;
}


///////////////////////////////////////////////////////////////////////////////
//  setVSCPControlByte
//

void vscp_setControlByte( uint8_t ctrl )
{
	// Normally this should be stired in EEPROM but
	// no EEPROM is available on this board so we just
	// ignore this request
}


///////////////////////////////////////////////////////////////////////////////
//  getVSCPControlByte
//

uint8_t vscp_getControlByte( void )
{
	// Normally this should be stired in EEPROM but
	// no EEPROM is available on this board so we just
	// ignore this request
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
//  getVSCPControlByte
//

uint8_t vscp_getPageSelect( uint8_t idx )
{
	// No paging used so we ignore this
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//  getVSCPControlByte
//

void vscp_setPageSelect( uint8_t idx, uint8_t data )
{
	// No paging used so we ignore this
}




















