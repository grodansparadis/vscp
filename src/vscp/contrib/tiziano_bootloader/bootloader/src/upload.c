// ******************************************************************
//
// DESCRIPTION: 
// AUTHOR: 		
//
// HISTORY:    
//
// ******************************************************************

#include "LPC23xx.H"
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "uart.h"
#include "crc.h"
#include "key.h"
#include "led.h"
#include "flash.h"
#include "upload.h"
#include "spiflash.h"

///////////////////////////////////////////////////////////////////////////////
#define BLOCKDATA_SIZE			512
#define INTERNAL_FLASH  		0
#define SPI_FLASH  				1
///////////////////////////////////////////////////////////////////////////////

#define LED_SIGN		LED1	
#define LED_PROT		LED2
#define LED_LIMITER		LED3
#define LED_FLAT		LED4
#define LED_USER		LED5
#define LED_HPF			LED6
#define LED_MON			LED7

#define LED_100HZ		LED4
#define LED_USER		LED5
#define LED_160HZ		LED6
#define LED_PHASEREV	LED7
///////////////////////////////////////////////////////////////////////////////

char verify_buf[512];
char data_buf[512];

static int block_number;
static int flash_type;
static int data_counter;
static int write_pointer;
static UINT16 checksum16;
static int totalchecksum16;

///////////////////////////////////////////////////////////////////////////////
int InternalFlashBlockProgram(int start, int n_byte);
int SpiFlashBlockProgram(int start, int n_byte);
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// open_upload
// 
// FILENAME: upload.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

void open_upload(void)
{
	open_spiflash();
	ResetChecksum();
}

// ****************************************************************************
// exec_upload
// 
// FILENAME: upload.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

void exec_upload(UINT32 ch)
{
	
}

// ****************************************************************************
// HandleStartBlock
// 
// FILENAME: upload.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

int HandleStartBlock(unsigned char* pindata,unsigned char* poutdata)
{
	// pdata[0] MSB block number
	// pdata[1] INTERNAL_FLASH/SPI_FLASH
	// pdata[2] ID
	// pdata[3] LSB block number
	int ret;
	block_number = (pindata[0] << 8) | (pindata[3]);
	flash_type = pindata[1];
	data_counter = 0;
	write_pointer = 0;
	checksum16 = 0;
	
	poutdata[0] = pindata[0];
	poutdata[1] = pindata[1];
	poutdata[2] = pindata[2];
	poutdata[3] = pindata[3];
	
	//PrintString((UINT8 const *)"\n\rblock_number=");
	//PrintInt((UINT32)block_number);
	led_on(LED_USER);
	if(!block_number)
	{
		// se è il primo blocco -> si esegue il chip erase
		switch(flash_type)
		{
			case INTERNAL_FLASH:
				ret = flashErase(USER_SECTOR_START, USER_SECTOR_END);
				PrintString((UINT8 const *)"\n\rflash erase ret=");
				PrintInt((UINT32)ret);
				ResetChecksum();
				break;
			case SPI_FLASH:
				ret = spiflash_chiperase();
				PrintString((UINT8 const *)"\n\rspi erase ret=");
				PrintInt((UINT32)ret);
				break;
			default:
				ret = 1;
				break;
		}
	}
	else
	{
		ret = 0;
		
	}
	poutdata[2] = ret;	// code error
	return ret;
}

// ****************************************************************************
// HandleBlockData
// 
// FILENAME: upload.c
// 						
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

int HandleBlockData(unsigned char* pindata,unsigned char* poutdata)
{
	int i;
	for(i=0; i < 8; i++)
	{
		data_buf[data_counter + i] = pindata[i];
	}
    data_counter += 8;
    
    //PrintString((UINT8 const *)"\n\rdata=");
    //PrintInt((UINT32)(data_counter/8));
	
	if(data_counter == BLOCKDATA_SIZE)
	{
		//PrintString((UINT8 const *)"\n\rend block");
		checksum16 = crcFast( (unsigned char const *)&data_buf[0], BLOCKDATA_SIZE );
		//PrintString((UINT8 const *)"\n\rCRC=");
		//PrintInt((UINT32)(checksum16));
		//PrintString((UINT8 const *)"\n\rblock=");
		//PrintInt((UINT32)(block_number));
		//if(flash_type == INTERNAL_FLASH)
		//	totalchecksum16 += checksum16;  
		write_pointer = block_number*BLOCKDATA_SIZE;
		poutdata[0] = (checksum16 >> 8) & 0xFF;		// MSB 16 bit CRC for block
		poutdata[1] = checksum16 & 0xFF;			// LSB 16 bit CRC for block
		poutdata[2] = (block_number >> 8) & 0xFF;	// MSB of block number 
		poutdata[3] = 0;
		poutdata[4] = 0;
		poutdata[5] = (block_number) & 0xFF;		// LSB of block number
		
		STROBE_ON;
		
		return 0;
	}
	
	return 2;
}

// ****************************************************************************
// HandleProgramBlock
// 
// FILENAME: upload.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

int HandleProgramBlock(unsigned char* pindata,unsigned char* poutdata)
{
	// pindata[0] MSB block number
	// pindata[1] INTERNAL_FLASH/SPI_FLASH
	// pindata[2]
	// pindata[3] LSB block number
	
	int ret = 1;
	int b_num = (pindata[0] << 8) | (pindata[3]);
	int f_type = pindata[1];
	
	if((b_num == block_number) && (f_type == flash_type))
	{
		switch(flash_type)
		{
			case INTERNAL_FLASH:
				ret = InternalFlashBlockProgram(write_pointer, BLOCKDATA_SIZE);
				led_off(LED_USER);
				if(ret)
				{
					PrintString((UINT8 const *)"\n\rflash error=");
					PrintInt((UINT32)write_pointer);
				}
				break;
			case SPI_FLASH:
				ret = SpiFlashBlockProgram(write_pointer, BLOCKDATA_SIZE);
				led_off(LED_USER);
				if(ret)
				{
					PrintString((UINT8 const *)"\n\rspi flash error=");
					PrintInt((UINT32)write_pointer);
				}
				if(totalchecksum16==0)
				{
					PrintString((UINT8 const *)"\n\rè avvenuto un reset");
				}
				break;
		}
	}
	
	if(ret == 0)
	{
		poutdata[0] = (block_number >> 8) & 0xFF;		// MSB block number
		poutdata[1] = flash_type;						// INTERNAL_FLASH/SPI_FLASH
		poutdata[2] = 0; 
		poutdata[3] = block_number & 0xFF;				// LSB block number
	}
	else
	{
		poutdata[0] = 0xFF;								// code error
		poutdata[1] = (block_number >> 8) & 0xFF;		// MSB block number
		poutdata[2] = flash_type;						// INTERNAL_FLASH/SPI_FLASH
		poutdata[3] = 0; 
		poutdata[4] = block_number & 0xFF;				// LSB block number
	}
	STROBE_OFF;
	return ret;
}

// ****************************************************************************
// HandleProgramBlock
// 
// FILENAME: upload.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

int HandleActivateNewImage(unsigned char* pindata,unsigned char* poutdata)
{
	// pindata[0] 8 bit CRC for entire internal flash (user program)
	// pindata[1]
	
	int ret = 1;
	
	if(((pindata[0] << 8) | pindata[1]) == (totalchecksum16 & 0xFFFF))
	{
		ret = 0;
	}
	else
	{
		poutdata[0] = 0xFF;	// code error
		PrintString((UINT8 const *)"\n\rtotalchecksum error:");
		PrintInt((UINT32)totalchecksum16);
		ret = 1;
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

int InternalFlashBlockProgram(int start, int n_byte)
{
	int i,dst_addr;
	char *src_addr;
	int byte_count;
	int ret;
	
	// qui siamo sicuri che il blocco dati ricevuto è corretto
	totalchecksum16 += checksum16;
	
	//PrintString((UINT8 const *)"\n\r");
	//PrintInt((UINT32)totalchecksum16);
	
	// se è il primo blocco bisogna saltare la zona della firma (primi 16 byte)
	if(start == 0)
	{
		dst_addr = USER_FLASH_START;
		src_addr = &data_buf[USER_FLASH_START - USER_PROGRAM_ADDRESS];
		byte_count = n_byte - (USER_FLASH_START - USER_PROGRAM_ADDRESS);
		//PrintString((UINT8 const *)"\n\rprog primo blocco");
	}
	else
	{
		dst_addr = USER_PROGRAM_ADDRESS + start;
		src_addr = &data_buf[0];
		byte_count = n_byte; 
	}
	
	ret = flashWrite((int *)(dst_addr), src_addr, byte_count);
	
	if(!ret)
	{
		// verify
		for(i=0; i < byte_count; i++)
		{
			if(src_addr[i] != *((char *)(dst_addr + i)))
				return 1;
		}
	}
	
	//PrintString((UINT8 const *)"\n\rbyte prog=");
	//PrintInt((UINT32)src_addr[0]);
	//PrintInt((UINT32)src_addr[1]);
	//PrintInt((UINT32)src_addr[2]);
	//PrintInt((UINT32)src_addr[3]);
	//int ret = flashWrite((int *)(USER_PROGRAM_ADDRESS + start), (char *)data_buf, n_byte);
	
	/*
	if(!ret)
	{
		// verify
		for(i=0; i < n_byte; i++)
		{
			if(data_buf[i] != *((char *)(USER_PROGRAM_ADDRESS + start + i)))
				return 1;
		}
	}
	*/
	
	return ret;
}

int SpiFlashBlockProgram(int start, int n_byte)
{
	int i,primi_256,secondi_256;
			
	primi_256 = (start/256 + 1)*256;
	
	spiflash_write( (UINT32)start, primi_256 - start, (UINT8 *)data_buf );
	spiflash_read( (UINT32)start, primi_256 - start, (UINT8 *)verify_buf );
	
	for(i=0; i < primi_256 - start; i++)
	{
		if(data_buf[i]!=verify_buf[i])
		{
			return 1;
		}
	}
	
	secondi_256 = (start/256 + 2)*256;
	
	if(start + n_byte > secondi_256)
	{
		spiflash_write( (UINT32)primi_256, 256, (UINT8 *)&data_buf[primi_256 - start] );
		spiflash_read( (UINT32)primi_256, 256, (UINT8 *)&verify_buf[primi_256 - start] );
		for(i=0; i < 256; i++)
		{
			if(data_buf[primi_256 - start + i]!=verify_buf[primi_256 - start + i])
			{
				return 1;
			}
		}
		spiflash_write( (UINT32)secondi_256, start + n_byte - secondi_256, (UINT8 *)&data_buf[256 + primi_256 - start] );
		spiflash_read( (UINT32)secondi_256, start + n_byte - secondi_256, (UINT8 *)&verify_buf[256 + primi_256 - start] );
		for(i=0; i < start + n_byte - secondi_256; i++)
		{
			if(data_buf[256 + primi_256 - start + i]!=verify_buf[256 + primi_256 - start + i])
			{
				return 1;
			}
		}
		
			
	}
	else
	{
		spiflash_write( (UINT32)primi_256, start + n_byte - primi_256, (UINT8 *)&data_buf[primi_256 - start] );
		spiflash_read( (UINT32)primi_256, start + n_byte - primi_256, (UINT8 *)&verify_buf[primi_256 - start] );
		for(i=0; i < start + n_byte - primi_256; i++)
		{
			if(data_buf[primi_256 - start + i]!=verify_buf[primi_256 - start + i])
			{
				return 1;
			}
		}
		
	}
	
	//PrintString((UINT8 const *)"\n\rbyte prog=");
	//PrintInt((UINT32)data_buf[0]);
	//PrintInt((UINT32)data_buf[1]);
	//PrintInt((UINT32)data_buf[2]);
	//PrintInt((UINT32)data_buf[3]);
	
	return 0;
	
}

// ****************************************************************************
// ResetChecksum
// 
// FILENAME: upload.c
// 
// PARAMETERS:
// 
// DESCRIPTION:
// 
// RETURNS:
// 
// ****************************************************************************

void ResetChecksum(void)
{
	crcInit();
	totalchecksum16 = 0;
}

