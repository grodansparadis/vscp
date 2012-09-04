//*******************************************************************
//    DESCRIPTION: 	Modulo gestione memoria edit
//    HISTORY:    
//*******************************************************************

#include <LPC23xx.H>
#include "config.h"
#include "i2c.h"
#include "emem.h"
#include "led.h"
#include "key.h"
#include "vic.h"
#include "irq.h"
#include "uart.h"
#include "vscp.h"
#include "edt_mng.h"
//	#include "can.h"		// debug

//#define INIT_MESSAGE_TIME 2000 /* 2000*2.5ms=5s */

#define MODE_KEY	KEY1

//******************************************************************
//  EditMemoryInit
//  FILENAME:
//  PARAMETERS:
//  DESCRIPTION: 
//  RETURNS:
//*******************************************************************
void EditMemoryInit(int initstate)
{
	read_sw();
	if(GetKey(MODE_KEY))
	{
		// reset nickname -> nickname discovery
		vscp_setNickname( VSCP_ADDRESS_FREE );
		PrintString( (UINT8 const *)"\n\rRESET NICKNAME" );
	}
}

//******************************************************************
//  Funzioni di lettura/scrittura CONFIG
//  FILENAME:
//  PARAMETERS:
//  DESCRIPTION:
//  RETURNS:
//*******************************************************************
void EditReadConfig(CONFIG *config)
{
	int config_address,ee_address,count;
	
	config_address=(int)config;
    ee_address=EE_CONFIG_ADDRESS;
	count=sizeof(CONFIG);
	E2BlockRead((char *)config_address,ee_address,count);
}

void EditWriteConfig(CONFIG *config)
{
	int config_address,ee_address,count;
	config_address=(int)config;
    ee_address=EE_CONFIG_ADDRESS;
	count=sizeof(CONFIG);
    EditWriteOpCode(EE_OPCODE_VALUE_SETUP);
    
    E2BlockWrite((char *)config_address,ee_address,count);
    	
    EditWriteOpCode(EE_OPCODE_VALUE_READY);
}

//******************************************************************
//  Funzioni di lettura/scrittura/controllo magic ed opcode
//  FILENAME:
//  PARAMETERS:
//  DESCRIPTION:
//  RETURNS:
//******************************************************************

void EditWriteMagic(unsigned value)
{
	unsigned magic;
    magic=value;
    E2BlockWrite((char *)&magic,EE_MAGIC_ADDRESS,EE_MAGIC_SIZE);
}
int EditReadMagic(void)
{
	unsigned magic;
	E2BlockRead((char *)&magic,EE_MAGIC_ADDRESS,EE_MAGIC_SIZE);
	return magic;
}
int EditCheckMagic(void)
{
	unsigned magic;
    E2BlockRead((char *)&magic,EE_MAGIC_ADDRESS,EE_MAGIC_SIZE);
	if (magic==EE_MAGIC_VALUE)
	    return 1;
	return 0;
}
void EditReadOpCode(int *opcode)
{
	E2Read((char *)opcode,EE_OPCODE_ADDRESS);
}
void EditWriteOpCode(int opcode)
{
    E2Write(opcode,EE_OPCODE_ADDRESS);
}

//*****************************************************************
// SetStoreConfig
//  FILENAME: 
//  PARAMETERS:
//  DESCRIPTION: 
//				  
//				 
//  RETURNS:
//*****************************************************************

void test_emem(void)
{
	int data_rd;
	int data_wr,i;
	
	data_wr = 0xAA;
			
	for(i=0; i < EE_SIZE; i+=100)
	{
		E2Write(data_wr,i);
		E2Read((char *)&data_rd,i);

		if((data_rd & 0xFF) != data_wr)
		{
			PrintString( (UINT8 const *)"\n\rTEST EEPROM FAIL!!: ADDRESS=" );
			PrintInt((UINT32)i);
			return;
		} 
	}
	data_wr = 0xAA55AA55;
	EditWriteMagic(data_wr);
	E2BlockRead((char *)&data_rd,EE_MAGIC_ADDRESS,EE_MAGIC_SIZE);
	if (data_rd!=data_wr)
	{
		PrintString( (UINT8 const *)"\n\rTEST EEPROM FAIL!!: MAGIC ADDRESS" );
		return;
	}
	PrintString( (UINT8 const *)"\n\rTEST EEPROM PASS" );
	
}


