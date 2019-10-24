//*****************************************************************************
//
// $RCSfile: $
// $Revision: $
//
// This module provides interface routines to the LPC ARM Flash IAP.
// Copyright 2004, 2005 R O SoftWare
// No guarantees, warrantees, or promises, implied or otherwise.
// May be used for hobby or commercial purposes provided copyright
// notice remains intact.
//
//*****************************************************************************

#include <string.h>
#include "LPC23xx.H"
#include "config.h"
#include "vscp.h"
#include "utils.h"
	#include "uart.h"
#include "vic.h"
#include "flash.h"

// MEMMAP defines
#define MEMMAP_BBLK   0                 // Interrupt Vectors in Boot Block
#define MEMMAP_FLASH  1                 // Interrupt Vectors in Flash
#define MEMMAP_SRAM   2                 // Interrupt Vectors in SRAM

#define CCLK          (Fcpu)        // CPU Clock Speed

#define IAP_LOCATION  0x7FFFFFF1ul

static int lastSector;
static unsigned long flashParams[5];
static unsigned long buffer[512/sizeof (long)];
//*****************************************************************************
// 
// Function Name: callIAP()
// 
// Description:
//    This function loads the address of flashParams[] into r0 & r1
//    then calls the LPC's IAP (Thumb mode) code.
// 
// Calling Sequence: 
//    void
// 
// Returns:
//    void
//
// NOTES:
//    - We don't make this an inline function because the call to the
//    IAP code is made from assembly language therefore the compiler
//    doesn't realize other registers are possibly being altered.
//    - pFP is in the source list of the asm so its assignment to the
//    address of flashParams[] is not optimized away.
//    - 'lr' is not assigned prior to 'bx' so the IAP code will return
//    directly to this functions caller
//
//*****************************************************************************

static void callIAP(void) __attribute((naked));
static void callIAP(void)
{
	// fast way to get the address of flashParams[] directly into r0
	register void *pFP asm("r0") = flashParams;

	asm volatile(" mov r1, r0\n"          // copy &flashParams[] into r1
	             " bx  %[iapLocation]"    // 'bx' because IAP is Thumb mode
	             :
	             : "r" (pFP), [iapLocation] "r" (IAP_LOCATION)
	             : "r1" );
}

//*****************************************************************************
//
// Function Name: flashBlank()
//
// Description:
//    This function checks if the range of specified sectors is erased.
//
// Calling Sequence: 
//    startSector - first sector to be erased
//    endSector - last sector to be erased
//
// Returns:
//    0 - success, else error code
//
//*****************************************************************************

int flashBlank(int startSector, int endSector)
{
	flashParams[0] = 53;                  // blank check
	flashParams[1] = startSector;         // start sector
	flashParams[2] = endSector;           // end sector
	callIAP();
	return (int)flashParams[0];
}

//*****************************************************************************
//
// Function Name: flashErase()
//
// Description:
//    This function erases the range of specified sectors
//
// Calling Sequence: 
//    startSector - first sector to be erased
//    endSector - last sector to be erased
//
// Returns:
//    0 - success, else error code
//
//*****************************************************************************

int flashErase(int startSector, int endSector)
{
  	unsigned long cpsr;
  	unsigned char memmap;

  	cpsr = disableIRQ();                 // disable all interrupts
  	memmap = MEMMAP;                      // get current memory map
  	MEMMAP = MEMMAP_FLASH;                // map User Flash into low 64 bytes

  	flashParams[0] = 50;                  // prepare sector(s)
  	flashParams[1] = startSector;         // start sector
  	flashParams[2] = endSector;           // end sector
  	callIAP();

  	if (flashParams[0] == CMD_SUCCESS)
    {
    	flashParams[0] = 52;                // erase sector(s)
    	flashParams[1] = startSector;       // start sector
    	flashParams[2] = endSector;         // end sector
    	flashParams[3] = CCLK/1000;         // cpu clock freq in KHz
    	callIAP();
    }

  	MEMMAP = memmap & 0x03;               // restore the memory map
  	restoreIRQ(cpsr);                    // restore all interrupts
  	return (int)flashParams[0];
}

//*****************************************************************************
//
// Function Name: flashWrite()
//
// Description:
//    This function writes byteCount bytes of data from the src location
//    to the dst location in flash.
//
// Calling Sequence: 
//    *dst - location in flash at which to start writing
//    *src - location from which to get data (word boundary)
//    byteCount - number of bytes to write
//
// Returns:
//    0 - success, else error code
//
//  NOTE: When programming data into a sector that is already partially
//  programmed, all new data MUST be programmed into previously
//  unprogrammed 16-byte (128-bit) blocks.  This requirement is NOT
//  currently checked or enforced by this routine.  See msgs by Philps
//  Tech Support in the LPC2100 mailing list on Yahoo for more details.
//
//*****************************************************************************

int flashWrite(void *dst, void *src, unsigned byteCount)
{
  	unsigned char memmap;
  	unsigned long cpsr, count;
  	//unsigned long buffer[512/sizeof (long)];

  	if ((lastSector == 0) && (flashInit() == LPCUNKN))
    	return BUSY;

  	while (byteCount)
    {
    	cpsr = disableIRQ();               // disable all interrupts
    	memmap = MEMMAP;                    // get current memory map
    	MEMMAP = MEMMAP_FLASH;              // map User Flash into low 64 bytes

    	// prepare all sectors for programing
    	flashParams[0] = 50;                // prepare sectors
    	flashParams[1] = 0;                 // start sector
    	flashParams[2] = lastSector;        // last sector
    	callIAP();

    	if (flashParams[0] != CMD_SUCCESS)
      	{
      		MEMMAP = memmap & 0x03;           // restore the memory map
      		restoreIRQ(cpsr);                // restore all interrupts
      		return flashParams[0];
      	}
		
    	// check if dst is on 512 byte boundary
    	if ((unsigned long)dst & 0x1FF)
      	{
      		// no, copy 512 bytes from flash into buffer
     		memcpy(buffer, (void *)((unsigned long)dst & ~0x1FF), 512);

      		// copy src into buffer
      		count = MIN(byteCount, (512 - ((unsigned long)dst & 0x1FF)));
      		memcpy((char *)buffer + ((unsigned long)dst & 0x1FF), src, count);

		  	flashParams[0] = 51;              // copy RAM to Flash
		    flashParams[1] = (unsigned long)dst & ~0x1FF; // destination
		    flashParams[2] = (unsigned long)buffer; // source
		    flashParams[3] = 512;             // byte count
		    flashParams[4] = CCLK/1000;       // CCLK in KHz
      		callIAP();

      		MEMMAP = memmap & 0x03;           // restore the memory map
      		restoreIRQ(cpsr);                // restore all interrupts

      		if (flashParams[0] != CMD_SUCCESS)
      			return (int)flashParams[0];
      		
		    dst = (char *)dst + count;
		    src = (char *)src + count;
		    byteCount -= count;
      		continue;                         // restart the loop to prepare sectors again
      	}

	    // dst is now aligned on a 512 byte boundary
	    // find the biggest chunk we can program at a time
	    if (byteCount >= 8192)
	      	count = 8192;
	    else if (byteCount >= 4096)
	      	count = 4096;
	    else if (byteCount >= 1024)
	      	count = 1024;
	    else if (byteCount >= 512)
	      	count = 512;
	    else
	      	count = byteCount;

    	if (count < 512)
      	{
			memcpy(buffer, dst, 512);           // copy current flash contents into buffer
		    memcpy(buffer, src, count);          // update buffer with the new contents
		    flashParams[2] = (unsigned long)buffer; // source
		    flashParams[3] = 512;             // byte count
      	}
    	else
      	{
      		flashParams[2] = (unsigned long)src;
      		flashParams[3] = count;           // byte count
      	}

    	flashParams[0] = 51;                // copy RAM to Flash
    	flashParams[1] = (unsigned long)dst; // destination
    	flashParams[4] = CCLK/1000;         // CCLK in KHz
    	callIAP();

    	MEMMAP = memmap & 0x03;             // restore the memory map
    	restoreIRQ(cpsr);                  // restore all interrupts

    	if (flashParams[0] != CMD_SUCCESS)
    		return (int)flashParams[0];

	    dst = (char *)dst + count;
	    src = (char *)src + count;
	    byteCount -= count;
    }

  	return CMD_SUCCESS;
}

//*****************************************************************************
//
// Function Name: flashInit()
//
// Description:
//    This function tests if the LPC has 128K or 256K of flash and
//    returns the result.
//
// Calling Sequence: 
//    void
//
// Returns:
//    LPC128K, LPC256K, or LPCUNKN (on error)
//
//*****************************************************************************

int flashInit(void)
{
  	lastSector = 0;                       // clear last usable sector #

  	switch (flashBlank(14, 14))           // check last sector of 256K parts
    {
    	case CMD_SUCCESS:
    	case SECTOR_NOT_BLANK:
      		lastSector = 14;                  // store last usable sector #
      		return LPC256K;

    	case INVALID_SECTOR:
      		break;

    	default:
      		return LPCUNKN;
    }

  	switch (flashBlank(10, 10))           // check last sector of 128K parts
    {
    	case CMD_SUCCESS:
    	case SECTOR_NOT_BLANK:
      		lastSector = 10;                  // store last usable sector #
      		return LPC128K;

    	default:
      		break;
    }

	switch (flashBlank(27, 27))           // check last sector of 128K parts
    {
    	case CMD_SUCCESS:
    	case SECTOR_NOT_BLANK:
      		lastSector = 27;                  // store last usable sector #
      		return LPC512K;

    	default:
      		break;
    }

  	return LPCUNKN;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
//
// Function Name: execute_reset()
//
// Description:
//
// Calling Sequence: 
//    void
//
// Returns:
//
//*****************************************************************************

void execute_reset(void)
{
	disableIRQ();
	init_vic();
	
	WDMOD  = (WDEN | WDRESET);                                           // ensure watchdog is enabled
    WDFEED = 0xaa;                                                       // command reset to core and peripherals
    WDFEED = 0x55;
    WDFEED = 0xaa;
    WDFEED = 0;
	
}

//*****************************************************************************
//
// Function Name: execute_user_code()
//
// Description:
//
// Calling Sequence: 
//    void
//
// Returns:
//
//*****************************************************************************

void execute_user_code(void)
{
	disableIRQ();
	init_vic();
	
	void (*user_code_entry)(void);
    user_code_entry = (void (*)(void))USER_FLASH_START;
    user_code_entry();
}

//*****************************************************************************
//
// Function Name: user_code_present()
//
// Description:
//
// Calling Sequence: 
//    void
//
// Returns:
//
//*****************************************************************************

BOOL user_code_present(void)
{
	if(*((int *)(ETHERNET_RAM_START_ADDRESS + ETHERNET_RAM_SIZE - 4)) == INT_FLASH_SIGNATURE)
	{
		// forzato  dal programma user -> bisogna rimanere nel bootloader
		*((int *)(ETHERNET_RAM_START_ADDRESS + ETHERNET_RAM_SIZE - 4)) = 0;
		return (FALSE);	
	}
	if(*((int *)(INT_FLASH_SIGNATURE_ADDRESS)) == INT_FLASH_SIGNATURE)
    	return (TRUE);
    else
    	return (FALSE);
}

//*****************************************************************************
//
// Function Name: write_flash_signature()
//
// Description:
//
// Calling Sequence: 
//    void
//
// Returns:
//
//*****************************************************************************

void write_flash_signature(void)
{
	int signature[1];
	
	signature[0]= INT_FLASH_SIGNATURE;
	flashWrite((int *)(INT_FLASH_SIGNATURE_ADDRESS), (int *)signature, 4);
}

//*****************************************************************************
//
// Function Name: save_nickname_upgrade()
//
// Description:
//
// Calling Sequence: 
//    void
//
// Returns:
//
//*****************************************************************************

void save_nickname_upgrade(void)
{
	*((int *)(ETHERNET_RAM_START_ADDRESS + ETHERNET_RAM_SIZE - 8)) = INT_FLASH_NICKNAME_SIGNATURE | vscp_nickname;
}
