// ******************************************************************
//
// DESCRIPTION: 
// AUTHOR: 		
//
// HISTORY:    
//
// ******************************************************************

#ifndef _SPIFLASH_H
#define _SPIFLASH_H

#define CMD_READ				0x03
#define CMD_FASTREAD            0x0B
#define CMD_RDID				0x9F
#define CMD_READID          	0x90
#define CMD_WREN				0x06
#define CMD_WRDI				0x04
#define CMD_SE					0xDB
#define CMD_BE          		0xC7
#define CMD_PP        			0x02
#define CMD_RDSR				0x05
#define CMD_WRSR           		0x01
#define CMD_DP                  0xB9
#define CMD_RES             	0xAB

///////////////////////////////////////////////////////////////////////////////
#define STATUS_WIP				1
#define STATUS_WEL				2
#define STATUS_BP0				4
#define STATUS_BP1				8
#define STATUS_BP2				16
#define STATUS_SRWD				128

///////////////////////////////////////////////////////////////////////////////
#define SA0 	0x000000
#define SA1		0x010000
#define SA2		0x020000
#define SA3		0x030000
#define SA4		0x040000
#define SA5		0x050000
#define SA6		0x060000
#define SA7		0x070000

///////////////////////////////////////////////////////////////////////////////
int spiflash_chiperase(void);
int spiflash_erase(int address);
int spiflash_identify( UINT8 *pData );
int spiflash_read( UINT32 start, UINT32 count, UINT8 *pData );
int spiflash_write( UINT32 start, UINT32 count, UINT8 *pData );
void open_spiflash(void);

#endif
