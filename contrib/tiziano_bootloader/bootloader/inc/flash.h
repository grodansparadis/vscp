/******************************************************************************
 *
 * $RCSfile: $
 * $Revision: $
 *
 * This module provides the interface definitions for flash.c
 * Copyright 2004, 2005 R O SoftWare
 * No guarantees, warrantees, or promises, implied or otherwise.
 * May be used for hobby or commercial purposes provided copyright
 * notice remains intact.
 *
 *****************************************************************************/
#ifndef INC_FLASH_H
#define INC_FLASH_H

// Results from flashInit()
#define LPC512K                 3
#define LPC256K                 2
#define LPC128K                 1
#define LPCUNKN                -1

// (Philips) Status Codes
#define CMD_SUCCESS             0
#define INVALID_CMD             1
#define SRC_ADDR_ERROR          2
#define DST_ADDR_ERROR          3
#define SRC_ADDR_NOT_MAPPED     4
#define DST_ADDR_NOT_MAPPED     5
#define COUNT_ERROR             6
#define INVALID_SECTOR          7
#define SECTOR_NOT_BLANK        8
#define SECTOR_NOT_PREPARED     9
#define COMPARE_ERROR           10
#define BUSY                    11
#define PARAM_ERROR             12
#define ADDR_ERROR              13
#define ADDR_NOT_MAPPED         14
#define CMD_LOCKED              15
#define INVALID_CODE            16
#define INVALID_BAUD_RATE       17
#define INVALID_STOP_BIT        18
#define CODE_READ_PROT_ENABLED  19

/******************************************************************************
 *
 * Function Name: flashBlank()
 *
 * Description:
 *    This function checks if the range of specified sectors is erased.
 *
 * Calling Sequence: 
 *    startSector - first sector to be erased
 *    endSector - last sector to be erased
 *
 * Returns:
 *    0 - success, else error code
 *
 *****************************************************************************/
int flashBlank(int startSector, int endSector);

/******************************************************************************
 *
 * Function Name: flashErase()
 *
 * Description:
 *    This function erases the range of specified sectors
 *
 * Calling Sequence: 
 *    startSector - first sector to be erased
 *    endSector - last sector to be erased
 *
 * Returns:
 *    0 - success, else error code
 *
 *****************************************************************************/
int flashErase(int startSector, int endSector);

/******************************************************************************
 *
 * Function Name: flashWrite()
 *
 * Description:
 *    This function writes byteCount bytes of data from the src location
 *    to the dst location in flash.
 *
 * Calling Sequence: 
 *    *dst - location in flash at which to start writing
 *    *src - location from which to get data (word boundary)
 *    byteCount - number of bytes to write
 *
 * Returns:
 *    0 - success, else error code
 *
 *  NOTE: When programming data into a sector that is already partially
 *  programmed, all new data MUST be programmed into previously
 *  unprogrammed 16-byte (128-bit) blocks.  This requirement is NOT
 *  currently checked or enforced by this routine.  See msgs by Philps
 *  Tech Support in the LPC2100 mailing list on Yahoo for more details.
 *
 *****************************************************************************/
int flashWrite(void *dst, void *src, unsigned byteCount);

/******************************************************************************
 *
 * Function Name: flashInit()
 *
 * Description:
 *    This function tests if the LPC has 128K or 256K of flash and
 *    returns the result.
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    LPC128K, LPC256K, or LPCUNKN (on error)
 *
 *****************************************************************************/
int flashInit(void);

///////////////////////////////////////////////////////////////////////////////
void execute_reset(void);
void execute_user_code(void);
BOOL user_code_present(void);
void write_flash_signature(void);
void save_nickname_upgrade(void);

#endif
