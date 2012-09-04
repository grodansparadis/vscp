// BootControl.h: interface for the CBootControl class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// $RCSfile: BootControl.h,v $                                       
// $Date: 2005/07/24 21:20:29 $                                  
// $Author: akhe $                                              
// $Revision: 1.8 $ 
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOOTCONTROL_H__0267EB3C_F400_4A24_B5BA_452BCECB5D50__INCLUDED_)
#define AFX_BOOTCONTROL_H__0267EB3C_F400_4A24_B5BA_452BCECB5D50__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../common/canal.h"
#include "../common/CanalSuperWrapper.h"
#include "IntelHex.h"

// Timeout for response
#define BOOT_COMMAND_RESPONSE_TIMEOUT	5

// flags
// CONTROL is defined as follows
//
// Bit 0:	MODE_WRT_UNLCK		Set this to allow write and erase to memory. 
// Bit 1:	MODE_ERASE_ONLY		Set this to only erase program memory on a put command. 					Must be on a 64-bit boundary.
// Bit 2:	MODE_AUTO_ERASE		Set this to automatically erase program memory while writing 					data.
// Bit 3:	MODE_AUTO_INC		Set this to automatically increment the pointer after a write.
// Bit 4:	MODE_ACK			Set to get acknowledge.
// Bit 5:	undefined.			
// Bit 6:	undefined.			
// Bit 7:	undefined.	

#define MODE_WRT_UNLCK		1		
#define MODE_ERASE_ONLY		2
#define MODE_AUTO_ERASE		4
#define MODE_AUTO_INC		8	
#define MODE_ACK			16

// Boot Commands
#define CMD_NOP				0x00	// No operation - Do nothing
#define CMD_RESET			0x01	// Reset the device.
#define CMD_RST_CHKSM		0x02	// Reset the checksum counter and verify.
#define CMD_CHK_RUN			0x03	// Add checksum to CMD_DATA_LOW and
 									// CMD_DATA_HIGH, if verify and zero checksum
									// then clear the last location of EEDATA.	

// Memory Type
#define	MEM_TYPE_PROGRAM	0
#define MEM_TYPE_CONFIG		1
#define MEM_TYPE_EEPROM		2


// CAN message ID's
#define ID_PUT_BASE_INFO			0x00001000	// Write address information.
#define ID_PUT_DATA					0x00001100	// Write data block.
#define ID_GET_BASE_INFO			0x00001200	// Get address information.
#define ID_GET_DATA					0x00001300	// Get data block.

#define ID_RESPONSE_PUT_BASE_INFO	0x00001400	// Response for put info request.
#define ID_RESPONSE_PUT_DATA		0x00001500	// Response for put data request.
#define ID_RESPONSE_GET_BASE_INFO	0x00001400	// Response for get info request.
#define ID_RESPONSE_GET_DATA		0x00001500	// Response for get data request.

// USed VSCP commands
#define VSCP_READ_REGISTER			0x09
#define VSCP_RW_RESPONSE			0x0A
#define VSCP_WRITE_REGISTER			0x0B
#define VSCP_ENTER_BOOTLODER_MODE	0x0C

// Used VSCP registers
#define VSCP_REG_PAGE_SELECT_MSB	0x92
#define VSCP_REG_PAGE_SELECT_LSB	0x93
#define VSCP_REG_GUID0				0xD0
#define VSCP_REG_GUID3				0xD3
#define VSCP_REG_GUID5				0xD5
#define VSCP_REG_GUID7				0xD7

typedef struct _bootclientItem {
	unsigned char m_nickname;		// Nickname for node
	unsigned char m_bootalgorithm;	// Bootloader algorithm to use
	unsigned char m_pageMSB;		// MSB of current page
	unsigned char m_pageLSB;		// LSB of current page
	unsigned char m_GUID0;			// GUID byte 0
	unsigned char m_GUID3;			// GUID byte 3
	unsigned char m_GUID5;			// GUID byte 5
	unsigned char m_GUID7;			// GUID byte 7
} bootclientItem;


class CBootControl  
{

public:

	CBootControl();
	virtual ~CBootControl();
 

	/*!
		Load an Intel hex file to the image

		@param path Path to file
		@return True on success 
	*/
	bool loadIntelHexFile( const char *path );


	/*!
		Add a new nickname ID to the list of clients
		we want to update code in.
	*/
	void addClient( unsigned char nicknameID );

	/*!
		Return the number of clients

		@return Number of clients
	*/
	unsigned char getClientCnt( void );

	/*!
		Set the response timeout in seconds

		@param timeout Response timeout.
	*/
	void setResponseTimeout( unsigned long timeout ) { m_responseTimeout = timeout; };

	/*!
		Set a device in bootmode

		@param index into device list
	*/
	bool setDeviceInBootMode( unsigned char idx );
			
	/*!
		Read register from node

		@param node nickname for node
		@param reg register to read
		@param val pointer to register value

	*/
	bool readRegister( unsigned char node, unsigned char reg, unsigned char *value );

//private:


	/*!
		Write to device control registry

		@param addr Address to set as start address
		@param flags Control Flags
		@param cmd Boot command
		@param cmdData0 Boot command data byte 0
		@param cmdData1 Boot command data byte 1

	*/
	bool writeDeviceControlRegs( unsigned long addr,
									unsigned char flags = 
										( MODE_WRT_UNLCK | MODE_AUTO_ERASE | MODE_AUTO_INC | MODE_ACK ),
									unsigned char cmd = CMD_NOP,
									unsigned char cmdData0 = 0,
									unsigned char cmdData1 = 0 );


	/*!
		Write to device memory. Address information is 
		accounted for internally.

		@param pchecksum Pointer to running checksum for written data
		@return true on success.
	*/
	bool writeDeviceMemory( unsigned long * pchecksum );


	/*!
		Check for response from nodes.

		This routine is used as a check for response from nodes under boot. 
		Node id's must have been added using addClient.
		The supplied id is valid from bit 8 and upwards. The lower eight bits
		are the id.
		Only extended messages are accepted as a valid response.

		@param id Response code to look for.
		@return true on success.
	*/
	bool checkResponse( unsigned long id );

	/*!
		Set CANAl device to use for communication
	*/
	void setCanalDevice( devItem * pItem );


	/// Memory image for the device
	CIntelHex m_memImage;

	/// Client if list
	bootclientItem m_clientList[ 256 ];	

protected:

	/// The CANAL wrapper for interfaces
	CCanalSuperWrapper* m_pCanalSuperWrapper;

	/// Flag for handshake with node
	bool m_bHandshake;	

	/// Number of clients
	unsigned char m_nClients;

	/// Response Timeout in seconds (zero for know infinite)
	unsigned long m_responseTimeout;

	/// Internal address pointer
	unsigned long m_pAddr;

	/// Memory type to program
	unsigned char m_Type;

};

#endif // !defined(AFX_BOOTCONTROL_H__0267EB3C_F400_4A24_B5BA_452BCECB5D50__INCLUDED_)
