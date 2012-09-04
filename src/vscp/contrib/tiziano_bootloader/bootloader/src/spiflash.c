// ******************************************************************
//
// DESCRIPTION: 
// AUTHOR: 		
//
// HISTORY:    
//
// ******************************************************************

#include "LPC23xx.H"
#include "config.h"
#include "uart.h"
#include "utils.h"
#include "ssp.h"
#include "spiflash.h"

///////////////////////////////////////////////////////////////////////////////
static UINT8 flashParams[8];
static UINT8 flashDummy[8] = 
{
		0,0,0,0,0,0,0,0
};
///////////////////////////////////////////////////////////////////////////////
void write_enable(void);
void write_in_progress(void);
void clear_fifo(void);
///////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
//
// Function Name: spiflash_identify()
//
// Description:
//
// Calling Sequence: 
//
// Returns:
//    0 - success, else error code
//
//*****************************************************************************

int spiflash_identify( UINT8 *pData )
{
	SSP0ClrCS();
	
	flashParams[0] = CMD_RDID;
	SSP0Send( flashParams, 1 );
	SSP0Send( flashParams, 1 );
	SSP0Send( flashParams, 1 );
	SSP0Send( flashParams, 1 );

	SSP0Receive( flashParams, 4 );
	pData[0] = flashParams[1];	// manufacturer identification (SPANSION=0x01) 
	pData[1] = flashParams[2]; 	// device identification (UNIFORM=0x02)
	pData[2] = flashParams[3];	// device identification (UNIFORM=0x12)
	
	SSP0SetCS();
	
	return 0; 
}

//*****************************************************************************
//
// Function Name: spiflash_chiperase()
//
// Description:
//
// Calling Sequence: 
//
// Returns:
//    0 - success, else error code
//
//*****************************************************************************

int spiflash_chiperase(void)
{
	write_enable();
	
	flashParams[0] = CMD_BE;
	SSP0ClrCS();
	SSP0Send( flashParams, 1 );
	SSP0Receive( flashParams, 1 );
	SSP0SetCS();

	write_in_progress();
	
	return 0;	
}

//*****************************************************************************
//
// Function Name: spiflash_erase()
//
// Description:
//    This function checks if the range of specified sectors is erased.
//
// Calling Sequence: 
//    address - address of sector to be erased
//
//    address = 0x000000 -sector 0
//    address = 0x010000 -sector 1
//    address = 0x020000 -sector 2
//    address = 0x030000 -sector 3
//    address = 0x040000 -sector 4
//    address = 0x050000 -sector 5
//    address = 0x060000 -sector 6
//    address = 0x070000 -sector 7
//
// Returns:
//    0 - success, else error code
//
//*****************************************************************************

int spiflash_erase(int address)
{
	write_enable();
	
	flashParams[0] = CMD_SE; 
	SSP0ClrCS();
	flashParams[1] = (address >> 16) & 0xFF;         
	flashParams[2] = (address >> 8) & 0xFF;           
	flashParams[3] = address & 0xFF; 
	SSP0Send( flashParams, 4 );
	SSP0Receive( flashParams, 4 );
	SSP0SetCS();
	
	write_in_progress();

	return 0;
}


//*****************************************************************************
//
// Function Name: spiflash_read()
//
// Description:
//
// Calling Sequence: 
//
// Returns:
//    0 - success, else error code
//
//*****************************************************************************

int spiflash_read( UINT32 start, UINT32 count, UINT8 *pData )
{
	int i,j;
		
	SSP0ClrCS();
	
	flashParams[0] = CMD_FASTREAD;
	flashParams[1] = (start >> 16) & 0xFF;         
	flashParams[2] = (start >> 8) & 0xFF;           
	flashParams[3] = start & 0xFF;
	flashParams[4] = 0;
	SSP0Send( flashParams, 5 );
	SSP0Receive( flashParams, 5 );

	j = count / 8;
	for(i=0; i < j; i++)
	{
		SSP0Send( flashDummy, 8 );
		SSP0Receive( &pData[i*8], 8 );
	}
	j = count % 8;
	if(j)
	{
		SSP0Send( flashDummy, j );
		SSP0Receive( &pData[i*8], j );
	}
	
	SSP0SetCS();
		
	return 0;
}

//*****************************************************************************
//
// Function Name: spiflash_write()
//
// Description:
//			   N.B: non si possono scrivere + di 256 byte
// Calling Sequence: 
//
// Returns:
//    0 - success, else error code
//
//*****************************************************************************

int spiflash_write( UINT32 start, UINT32 count, UINT8 *pData )
{
	
	write_enable();
	
	flashParams[0] = CMD_PP;
	SSP0ClrCS();
	flashParams[1] = (start >> 16) & 0xFF;         
	flashParams[2] = (start >> 8) & 0xFF;           
	flashParams[3] = start & 0xFF;
	SSP0Send( flashParams, 4 );
	SSP0Receive( flashParams, 4 );

	SSP0Send( pData, count );
	//SSP0Receive( pData, count );
	
	SSP0SetCS();

	write_in_progress();
	
	clear_fifo();
	
	return 0;
}

//*****************************************************************************
//
// Function Name: write_in_progress()
//
// Description:
//
// Calling Sequence: 
//
// Returns:
//
//*****************************************************************************

void write_in_progress(void)
{
	flashParams[0] = CMD_RDSR;
	SSP0ClrCS();
	SSP0Send( flashParams, 1 );
	SSP0Receive( flashParams, 1 );
	while(1)
	{
		SSP0Send( flashParams, 1 );
		SSP0Receive( flashParams, 1 ) ;
		if((flashParams[0] & STATUS_WIP) == 0)
			break;
	}
	
	SSP0SetCS();
}

//*****************************************************************************
//
// Function Name: write_enable()
//
// Description:
//
// Calling Sequence: 
//
// Returns:
//
//*****************************************************************************

void write_enable(void)
{
	SSP0ClrCS();
	flashParams[0] = CMD_WREN;
	SSP0Send( flashParams, 1 );
	SSP0Receive( flashParams, 1 );
	SSP0SetCS();
}

//*****************************************************************************
//
// Function Name: clear_fifo()
//
// Description:
//
// Calling Sequence: 
//
// Returns:
//
//*****************************************************************************

void clear_fifo(void)
{
	SSP0ClrFIFO();
}

//*****************************************************************************
//
// Function Name: open_spiflash()
//
// Description:
//
// Calling Sequence: 
//
// Returns:
//
//*****************************************************************************

void open_spiflash(void)
{
		
}
