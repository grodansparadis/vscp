/****************************************************************************
* FILENAME:      CONFIG.H                                                   *
*                                                                           *
* VERSION:       V1.0                                                       *
*                                                                           *
* DESCRIPTION:   Header file for system and functions                       *  
*                                                                          	*
* TOOLS:         KEIL uVision3                                          	*
*                ARM Development Suite ADS1.2                               *
*                                                                           *
* REVISION HISTORY:                                                         *
* Version  Author          Date          Remarks                            *
* 1.0      Anthony Xie     10/10/2005    - -                                *
*****************************************************************************/


//****************************************************************************************************
//**File name: config.h
//**Function:  config file for system and functions
//********************************************************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//--------------------------------------------------------------------------
// Standard Data Types
//--------------------------------------------------------------------------
//                      Type
//                      Pointer
//
typedef signed   char   INT8,       	// 8 bit    signed integer 
                        *pINT8;     	// 8 bit    signed integer 
typedef signed   short  INT16,      	// 16 bit   signed integer 
                        *pINT16;    	// 16 bit   signed integer 
typedef signed   long   INT32,      	// 32 bit   signed integer 
                        *pINT32;    	// 32 bit   signed integer 
typedef unsigned char   UINT8,      	// 8 bit  unsigned integer 
                        *pUINT8;    	// 8 bit  unsigned integer 
typedef unsigned short  UINT16,     	// 16 bit unsigned integer 
                        *pUINT16;   	// 16 bit unsigned integer 
typedef unsigned long   UINT32,     	// 32 bit unsigned integer 
                        *pUINT32,   	// 32 bit unsigned integer 
						CANSTATUS,  	// General return code 
						*pCANSTATUS;	// General return code 
typedef float           FLOAT,      	// 32 bit floating point 
                        *pFLOAT;    	// 32 bit floating point 
typedef double          DOUBLE,     	// 32/64 bit floating point 
                        *pDOUBLE;   	// 32/64 bit floating point 
typedef unsigned int    BOOL,       	// Boolean (TM_TRUE or TM_FALSE) 
                        *pBOOL;     	// Boolean (TM_TRUE or TM_FALSE) 
typedef char            CHAR,       	// 8 bit character 
                        *pCHAR;     	// 8 bit character

/////////////////////////////////////////////////////////////////////



//*******************************
//* Memory             
//*******************************
#define INT_FLASH_SIZE     		0x20000

#define USER_PROGRAM_ADDRESS	0x6000	
#define USER_SECTOR_START		6
#define USER_SECTOR_END			10

#define USER_FLASH_START  		0x6010
#define BOOT_FLASH_START  		0

#define INT_FLASH_SIGNATURE			0x38598ad0
#define INT_FLASH_SIGNATURE_ADDRESS	0x6000

#define INT_FLASH_NICKNAME_SIGNATURE	0x19283000

//*******************************
//* Clock configue             
//*******************************
#define Fosc     	12000000
#define Fcco        288000000
#define Fcpu       	(Fcco / 4)	// 72000000
#define Fpclk       (Fcpu / 4) 
#define Fusbclk     (Fcco / 6)

//*******************************
//* Strobe             
//*******************************
#define STROBE_INIT FIO1DIR |= PORT1_BIT0	// P1.0
#define STROBE_ON	FIO1SET = PORT1_BIT0
#define STROBE_OFF	FIO1CLR = PORT1_BIT0

//*******************************
//* Vscp indicator             
//*******************************
#define PIN_VSCPLED PORT1_BIT1				// P1.1
#define PIN_VSCPLED_INIT FIO1DIR |= PORT1_BIT1	
#define PIN_VSCPLED_ON 	FIO1SET = PIN_VSCPLED
#define PIN_VSCPLED_OFF FIO1CLR = PIN_VSCPLED
#define PIN_VSCPLED_TEST FIO1PIN0 & PIN_VSCPLED

#endif /* __CONFIG_H */
