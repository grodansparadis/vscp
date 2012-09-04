
;*****************************************************************************
;*
;*              Microchip CAN Bootloader
;*
;*****************************************************************************
;* FileName:        CANIO.asm
;* Dependencies:    
;* Processor:       PIC18F with CAN
;* Assembler:       MPASMWIN 03.10.04 or higher
;* Linker:          MPLINK 03.10.04 or higher
;* Company:         Microchip Technology Incorporated
;*
;* Software License Agreement
;*
;* The software supplied herewith by Microchip Technology Incorporated
;* (the "Company") is intended and supplied to you, the Company's
;* customer, for use solely and exclusively with products manufactured
;* by the Company. 
;*
;* The software is owned by the Company and/or its supplier, and is 
;* protected under applicable copyright laws. All rights are reserved. 
;* Any use in violation of the foregoing restrictions may subject the 
;* user to criminal sanctions under applicable laws, as well as to 
;* civil liability for the breach of the terms and conditions of this 
;* license.
;*
;* THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES, 
;* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED 
;* TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
;* PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT, 
;* IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR 
;* CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
;*
;*
;* Basic Operation:
;* The following is a CAN bootloader designed for PIC18F microcontrollers 
;* with built-in CAN such as the PIC18F458. The bootloader is designed to
;* be simple, small, flexible, and portable.
;*
;* The bootloader can compiled to one of two major modes of operation:
;*
;* PG Mode: In this mode the bootloader allows bi-directional communication
;* 			with the source. Thus the bootloading source can query the 
;* 			target and verify the data being written.
;*
;* P Mode: 	In this mode the bootloader allows only single direction 
;* 			communication, i.e. source -> target. In this mode programming
;*			verification is provided by performing self verification and 
;*			checksum of all written data (except for control data).
;* 
;* The bootloader is essencially a register controlled system. The control 
;* registers hold information that dictates how the bootloader functions. 
;* Such information includes a generic pointer to memory, control bits to 
;* assist special write and erase operations, and special command registers
;* to allow verification and release of control to the main application.
;*
;* After setting up the control registers, data can be sent to be written 
;* to or a request can be sent to read from the selected memory defined by 
;* the address. Depending on control settings the address may or may not 
;* automatically increment to the next address.
;*
;* Commands:
;* Put commands received from source  (Master --> Slave)
;* The count (DLC) can vary.
;* XXXXXXXXXXX 0 0 8 XXXXXXXX XXXXXX00 ADDRL ADDRH ADDRU RESVD CTLBT SPCMD CPDTL CPDTH
;* XXXXXXXXXXX 0 0 8 XXXXXXXX XXXXXX01 DATA0 DATA1 DATA2 DATA3 DATA4 DATA5 DATA6 DATA7
;*
;* The following responce commands are only used for PG mode.
;* Get commands received from source  (Master --> Slave)
;* Uses control registers to get data. Eight bytes are always assumed.
;* XXXXXXXXXXX 0 0 0 XXXXXXXX XXXXXX10 _NA__ _NA__ _NA__ _NA__ _NA__ _NA__ _NA__ _NA__
;* XXXXXXXXXXX 0 0 0 XXXXXXXX XXXXXX11 _NA__ _NA__ _NA__ _NA__ _NA__ _NA__ _NA__ _NA__
;*
;* Put commands sent upon receiving Get command  (Slave --> Master)
;* YYYYYYYYYYY 0 0 8 YYYYYYYY YYYYYY00 ADDRL ADDRH ADDRU RESVD STATS RESVD RESVD RESVD
;* YYYYYYYYYYY 0 0 8 YYYYYYYY YYYYYY01 DATA0 DATA1 DATA2 DATA3 DATA4 DATA5 DATA6 DATA7
;*
;* Put commands sent upon receiving Put command (if enabled) (Slave --> Master)
;* This is the acknowledge after a put.
;* YYYYYYYYYYY 0 0 0 YYYYYYYY YYYYYY00 _NA__ _NA__ _NA__ _NA__ _NA__ _NA__ _NA__ _NA__
;* YYYYYYYYYYY 0 0 0 YYYYYYYY YYYYYY01 _NA__ _NA__ _NA__ _NA__ _NA__ _NA__ _NA__ _NA__
;*
;* ADDRL - Bits 0 to 7 of the memory pointer.  
;* ADDRH - Bits 8 - 15 of the memory pointer.
;* ADDRU - Bits 16 - 23 of the memory pointer.
;* RESVD - Reserved for future use.
;* CTLBT - Control bits.
;* SPCMD - Special command.
;* CPDTL - Bits 0 - 7 of special command data.
;* CPDTH - Bits 8 - 15 of special command data.
;* DATAX - General data.
;*
;* Control bits:
;* MODE_WRT_UNLCK 	-	Set this to allow write and erase operations to memory.
;* MODE_ERASE_ONLY 	- 	Set this to only erase Program Memory on a put command. Must 
;*						be on 64 byte boundary.
;* MODE_AUTO_ERASE 	-	Set this to automatically erase Program Memory while writing data.
;* MODE_AUTO_INC 	-	Set this to automatically increment the pointer after writing.
;* MODE_ACK 		-	Set this to generate an acknowledge after a 'put' (PG Mode only)
;*
;* Special Commands:
;* CMD_NOP			0x00	Do nothing
;* CMD_RESET		0x01	Issue a soft reset
;* CMD_RST_CHKSM	0x02	Reset the checksum counter and verify
;* CMD_CHK_RUN		0x03	Add checksum to special data, if verify and zero checksum
;* 							then clear the last location of EEDATA.

;* Memory Organization:
;*				|-------------------------------|
;*				|								| 0x000000 (Do not write here!)
;*				|			Boot Area			|
;*				|								|
;*				|-------------------------------|
;*				|								|
;*				|								|
;*				|								|
;*				|								|
;*				|			Prog Mem			|
;*				|								|
;*				|								|
;*				|								|
;*				|								| 0x1FFFFF
;*				|-------------------------------| 
;*				|		 	 User ID			| 0x200000
;*				|-------------------------------| 
;*				|:::::::::::::::::::::::::::::::|
;*				|:::::::::::::::::::::::::::::::|
;*				|-------------------------------| 
;*				|			  Config			| 0x300000
;*				|-------------------------------|
;*				|:::::::::::::::::::::::::::::::|
;*				|:::::::::::::::::::::::::::::::|
;*				|-------------------------------|
;*				|			 Device ID			| 0x3FFFFE
;*				|-------------------------------|
;*				|:::::::::::::::::::::::::::::::|
;*				|:::::::::::::::::::::::::::::::|
;*				|:::::::::::::::::::::::::::::::|
;*				|:::::::::::::::::::::::::::::::|
;*				|-------------------------------|
;*				|								| 0xF00000
;*				|			 EEDATA				|
;*				|		   (remapped)			|
;*				|								| (Last byte used as boot flag)
;*				|-------------------------------|
;*
;*
;* Author               Date        Comment
;*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;* Ross Fosler			11/26/02	First full revision	
;* 
;*****************************************************************************/
;
; Changes copyright (c) 2004 Ake Hedman, eurosource
;
; - Watchdog timer updated during wait for CAN message.
; - Symbolic init of mode control bits.


; *****************************************************************************
#include 	p18cxxx.inc
#include	canio.def
; *****************************************************************************





; *****************************************************************************
#ifndef		EEADRH
#define 	EEADRH	EEADR+1
#endif

#define		TRUE	1
#define		FALSE  	0

#define 	WREG1 	PRODH					; Alternate working register
#define		WREG2	PRODL


#define		MODE_WRT_UNLCK	_bootCtlBits,0	; Unlock write and erase
#define		MODE_ERASE_ONLY	_bootCtlBits,1	; Erase without write
#define		MODE_AUTO_ERASE	_bootCtlBits,2	; Enable auto erase before write
#define		MODE_AUTO_INC	_bootCtlBits,3	; Enable auto inc the address
#define		MODE_ACK		_bootCtlBits,4	; Acknowledge mode

; AKHE
#define		MODE_FLAG_WRT_UNLCK		0x01
#define		MODE_FLAG_ERASE_ONLY	0x02
#define		MODE_FLAG_AUTO_ERASE	0x04
#define		MODE_FLAG_AUTO_INC		0x08
#define		MODE_FLAG_ACK			0x10

#define		ERR_VERIFY		_bootErrStat,0	; Failed to verify 

#define		CMD_NOP			0x00
#define		CMD_RESET		0x01
#define		CMD_RST_CHKSM	0x02
#define		CMD_CHK_RUN		0x03
; *****************************************************************************


; *****************************************************************************
_MEM_IO_DATA	UDATA_ACS		0x00
; *****************************************************************************
_bootCtlMem							
_bootAddrL		RES	1				; Address info
_bootAddrH		RES	1
_bootAddrU		RES	1
_unused0		RES	1				; (Reserved)
_bootCtlBits	RES 1				; Boot Mode Control bits
_bootSpcCmd		RES	1				; Special boot commands
_bootChkL		RES	1				; Special boot command data
_bootChkH		RES	1				

_bootCount		RES	1

_bootChksmL		RES	1				; 16 bit checksum
_bootChksmH		RES	1

_bootErrStat	RES	1				; Error Status flags

_vscpNickname	RES	1				; VSCP Nickname - AKHE

; *****************************************************************************



; *****************************************************************************
_REMAP_STARTUP	CODE	RESET_VECT
; *****************************************************************************
ResetRemapped
; *****************************************************************************
_REMAP_INTV_H	CODE	HIGH_INT_VECT
; *****************************************************************************
IntVectHighRemapped
; *****************************************************************************
_REMAP_INTV_L	CODE	LOW_INT_VECT
; *****************************************************************************
IntVectLowRemapped



; *****************************************************************************
_STARTUP	CODE	0x00
; *****************************************************************************
	bra		_CANInit
	bra		_StartWrite
; *****************************************************************************
_INTV_H		CODE	0x08
; *****************************************************************************
#ifdef	NEAR_JUMP
	bra		IntVectHighRemapped
#else
	goto	IntVectHighRemapped
#endif
; *****************************************************************************
_INTV_L		CODE	0x18
; *****************************************************************************	
#ifdef	NEAR_JUMP
	bra		IntVectLowRemapped
#else
	goto	IntVectLowRemapped
#endif
; *****************************************************************************	



; *****************************************************************************
_CAN_IO_MODULE	CODE
; *****************************************************************************
; Function: 	VIOD _StartWrite(WREG _eecon_data)
;
; PreCondition:	Nothing
;
; Input:    	_eecon_data
;                               
;
; Output:   	Nothing. Self write timing started.
;
; Side 
; Effects: 		EECON1 is corrupted.
;				WREG is corrupted.
;
; Stack 
; Requirements: 1 level.
;
; Overview: 	Unlock and start the write or erase sequence to protected
;				memory. Function will wait until write is finished.
; *****************************************************************************
_StartWrite:
	movwf	EECON1
	
	btfss	MODE_WRT_UNLCK			; Stop if write locked
	return
		
	movlw	0x55					; Unlock
	movwf	EECON2
	movlw	0xAA
	movwf	EECON2
	bsf		EECON1, WR				; Start the write
	nop
	
	btfsc	EECON1, WR				; Wait (depends on mem type)
	bra		$ - 2
	return
; *****************************************************************************


; *****************************************************************************
; Function: 	 _bootChksm _UpdateChksum(WREG _bootChksmL)
;
; PreCondition:	Nothing
;
; Input:    	_bootChksmL
;                               
;
; Output:   	_bootChksm. This is a static 16 bit value stored in the 
;				Access Bank.
;
; Side 
; Effects: 		STATUS register is corrupted.
;
; Stack 
; Requirements: 1 level.
;
; Overview: 	This function adds a byte to the current 16 bit checksum 
;				count. WREG should contain the byte before being called.
;
;				The _bootChksm value is considered a part of the special 
;				register set for bootloading. Thus it is not visible.
;***************************************************************************
_UpdateChksum:
	addwf	_bootChksmL, F		 	; Keep a checksum
	btfsc	STATUS, C
	incf	_bootChksmH, F
	return
; *****************************************************************************


; *****************************************************************************
; Function: 	 VOID _CANInit(CAN, BOOT)
;
; PreCondition:	Enter only after a reset has occured.
;
; Input:    	CAN control information, bootloader control information
;                               
; Output:   	None. 
;
; Side 
; Effects: 		N/A. Only run imediately after reset.
;
; Stack 
; Requirements: N/A
;
; Overview: 	This routine is technically not a function since it will not 
;				return when called. It has been written in a linear form to 
;				save space.	Thus 'call' and 'return' instructions are not 
;				included, but rather they are implied.
;
;				This routine tests the boot flags to determine if boot mode is 
; 				desired or normal operation is desired. If boot mode then the
;				routine initializes the CAN module defined by user input. It 
;				also resets some registers associated to bootloading.
; *****************************************************************************
_CANInit:		

	clrf	EECON1
	clrf	EEADR					; Point to first location of EEDATA
	clrf	EEADRH	
    bsf		EECON1, RD				; Read the control code
	incfsz	EEDATA, W

#ifdef	NEAR_JUMP
	bra		ResetRemapped			; If not 0xFF then normal reset
#else
	goto	ResetRemapped
#endif
	
	clrf	_bootSpcCmd				; Reset the special command register

	; Get Nickname from EEPROM and save in RAM
	movlw	0x01
	movwf	EEADR					; Point at nickname in EEPROM	
    bsf		EECON1, RD				; Read the control code
	movf	EEDATA, W
	movwf 	_vscpNickname
	
	;movlw	0x1C					; AKHE Reset the boot control bits
	movlw	( MODE_FLAG_AUTO_ERASE | MODE_FLAG_AUTO_INC | MODE_FLAG_ACK ) 
	movwf	_bootCtlBits
		
	movlb	d'15'					; Set Bank 15
	
	bcf		TRISB, CANTX			; Set the TX pin to output	
		
	movlw	CAN_RXF0SIDH			; Set filter 0
	movwf	RXF0SIDH
	movlw	CAN_RXF0SIDL
	movwf	RXF0SIDL
	comf	WREG					; Prevent filter 1 from causing a
	movwf	RXF1SIDL				; receive event
	movlw	CAN_RXF0EIDH
	movwf	RXF0EIDH
	movlw	CAN_RXF0EIDL
	movwf	RXF0EIDL
	
	movlw	CAN_RXM0SIDH			; Set mask
	movwf	RXM0SIDH
	movlw	CAN_RXM0SIDL
	movwf	RXM0SIDL
	movlw	CAN_RXM0EIDH
	movwf	RXM0EIDH
	movlw	CAN_RXM0EIDL
	movwf	RXM0EIDL
	
	movlw	CAN_BRGCON1				; Set bit rate
	movwf	BRGCON1
	movlw	CAN_BRGCON2
	movwf	BRGCON2
	movlw	CAN_BRGCON3
	movwf	BRGCON3
	
	movlw	CAN_CIOCON				; Set IO
	movwf	CIOCON
		
	clrf	CANCON					; Enter Normal mode

	bra 	_CANSendAck				; AKHE Inital messagefrom boot loader
; *****************************************************************************
	
	
	
	
	
; *****************************************************************************
; This routine is essencially a polling loop that waits for a 
; receive event from RXB0 of the CAN module. When data is 
; received, FSR0 is set to point to the TX or RX buffer depending
; upon whether the request was a 'put' or a 'get'. 
; *****************************************************************************
_CANMain:

	bcf		RXB0CON, RXFUL		; Clear the receive flag

	clrwdt						; AKHE: Clear watchdog on every turn
	btfss	RXB0CON, RXFUL		; Wait for a message
	bra		$ - 4				; AKHE: 
	
		
#ifdef 	ALLOW_GET_CMD
	btfss	CAN_PG_BIT			; Put or get data?
	bra		_CANMainJp1
	
	lfsr	0, TXB0D0			; Set pointer to the transmit buffer
	movlw	0x08
	movwf	_bootCount			; Setup the count to eight
	movwf	WREG1
	bra		_CANMainJp2
#endif

_CANMainJp1
	lfsr	0, RXB0D0			; Set pointer to the receive buffer
	movf	RXB0DLC, W
	andlw	0x0F
	movwf	_bootCount			; Store the count
	movwf	WREG1
	bz		_CANMain			; Go back if no data specified for a put
	
_CANMainJp2
; *****************************************************************************



; *****************************************************************************
; Function: 	VOID _ReadWriteMemory()
;
; PreCondition:	Enter only after _CANMain().
;
; Input:    	None.
;                               
; Output:   	None. 
;
; Side 
; Effects: 		N/A. 
;
; Stack 
; Requirements: N/A
;
; Overview: 	This routine is technically not a function since it will not 
;				return when called. It has been written in a linear form to 
;				save space.	Thus 'call' and 'return' instructions are not 
;				included, but rather they are implied.
;
;				This is the memory I/O engine. A total of eight data 
; 				bytes are received and decoded. In addition two control
; 				bits are received, put/get and control/data.
;
; 				A pointer to the buffer is passed via FSR0 for reading or writing.
;
; 				The control register set contains a pointer, some control bits
; 				and special command registers.
;
; 				Control 
; 				<PG><CD><ADDRL><ADDRH><ADDRU><_RES_><CTLBT><SPCMD><CPDTL><CPDTH>
;
; 				Data
; 				<PG><CD><DATA0><DATA1><DATA2><DATA3><DATA4><DATA5><DATA6><DATA7>
;
; 				PG bit 	Put = 0, Get = 1
; 				CD bit	Control = 0, Data = 1
; *****************************************************************************
_ReadWriteMemory:						
	btfsc	CAN_CD_BIT				; Write/read data or control registers
	bra		_DataReg							


; *****************************************************************************
; This routine reads or writes the bootloader control registers.
; Then is executes any imediate command received.
_ControlReg	
	lfsr	1, _bootCtlMem
	
_ControlRegLp1
#ifdef 	ALLOW_GET_CMD
	btfsc	CAN_PG_BIT				; or copy control registers to buffer
	movff	POSTINC1, POSTINC0
	btfss	CAN_PG_BIT				; Copy the buffer to the control registers
#endif
	movff	POSTINC0, POSTINC1

	decfsz	WREG1, F
	bra		_ControlRegLp1	
	
#ifdef 	ALLOW_GET_CMD
	btfsc	CAN_PG_BIT	
	bra		_CANSendResponce		; Send responce if get
#endif
; *********************************************************

; *********************************************************	
; This is a no operation command. 
	movf	_bootSpcCmd, W			; NOP Command
	bz		_SpecialCmdJp2			; or send an acknowledge
; *********************************************************	

; *********************************************************	
; This is the reset command. 
	xorlw	CMD_RESET				; RESET Command
	btfsc	STATUS, Z
	reset
; *********************************************************	
	
; *********************************************************	
; This is the Selfcheck reset command. This routine
; resets the internal check registers, i.e. checksum and
; self verify.
	movf	_bootSpcCmd, W			; RESET_CHKSM Command
	xorlw	CMD_RST_CHKSM
	bnz		_SpecialCmdJp1
	
	clrf	_bootChksmH				; Reset chksum
	clrf	_bootChksmL
	bcf		ERR_VERIFY				; Clear the error verify flag
; *********************************************************	

; *********************************************************	
; This is the Test and Run command. The checksum is 
; verified, and the self-write verification bit is checked.
; If both pass, then the boot flag is cleared.
_SpecialCmdJp1
	movf	_bootSpcCmd, W			; RUN_CHKSM Command
	xorlw	CMD_CHK_RUN
	bnz		_SpecialCmdJp2

	movf	_bootChkL, W			; Add the control byte
	addwf	_bootChksmL, F
	bnz		_SpecialCmdJp2
	movf	_bootChkH, W
	addwfc	_bootChksmH, F
	bnz		_SpecialCmdJp2
	
	btfsc	ERR_VERIFY				; Look for verify errors
	bra		_SpecialCmdJp2
	
	clrf	EEADR					; AKHE - Point to first location of EEDATA 
	clrf	EEADRH	
	clrf	EEDATA					; and clear the data
	movlw	b'00000100'				; Setup for EEData
	rcall	_StartWrite
	
_SpecialCmdJp2	
#ifdef 	ALLOW_GET_CMD
	bra		_CANSendAck				; or send an acknowledge
#else
	bra		_CANMain
#endif					
; *****************************************************************************
	


; *****************************************************************************	
; This is a jump routine to branch to the appropriate memory 
; access function. The high byte of the 24-bit pointer is used 
; to determine which memory to access. All program memorys 
; (including Config and User IDs) are directly mapped. 
; EEDATA is remapped.
;

_DataReg

; *********************************************************								
_SetPointers
	movf	_bootAddrU, W			; Copy upper pointer
	movwf	TBLPTRU
	andlw	0xF0					; Filter
	movwf	WREG2
	
	movf	_bootAddrH, W			; Copy the high pointer
	movwf	TBLPTRH
	movwf	EEADRH	
	
	movf	_bootAddrL, W			; Copy the low pointer
	movwf	TBLPTRL
	movwf	EEADR
	
	btfss	MODE_AUTO_INC			; Adjust the pointer if auto inc is enabled
	bra		_SetPointersJp1
	
	movf	_bootCount, W			; add the count to the pointer
	addwf	_bootAddrL, F
	clrf	WREG
	addwfc	_bootAddrH, F
	addwfc	_bootAddrU, F
	
_SetPointersJp1	
; *********************************************************


; *********************************************************
_Decode
	movlw	0x30					; Program memory < 0x300000
	cpfslt	WREG2
	bra		_DecodeJp1
#ifdef 	ALLOW_GET_CMD
	btfsc	CAN_PG_BIT
	bra		_PMRead
#endif
	bra		_PMEraseWrite
	
		
_DecodeJp1	
	movf	WREG2,W					; Config memory = 0x300000
	xorlw	0x30
	bnz		_DecodeJp2
#ifdef 	ALLOW_GET_CMD
	btfsc	CAN_PG_BIT
	bra		_PMRead
#endif
	bra		_CFGWrite
	
	
_DecodeJp2	
	movf	WREG2,W					; EEPROM data = 0xF00000
	xorlw	0xF0
	bnz		_CANMain
#ifdef 	ALLOW_GET_CMD
	btfsc	CAN_PG_BIT
	bra		_EERead
#endif
	bra		_EEWrite
; *****************************************************************************
	
	
	
; *****************************************************************************
; Function: 	VOID _PMRead()
;				VOID _PMEraseWrite()
;
; PreCondition:	WREG1 and FSR0 must be loaded with the count and address of
;				the source data.
;
; Input:    	None.
;                               
; Output:   	None. 
;
; Side 
; Effects: 		N/A. 
;
; Stack 
; Requirements: N/A
;
; Overview: 	These routines are technically not functions since they will not 
;				return when called. They have been written in a linear form to 
;				save space.	Thus 'call' and 'return' instructions are not 
;				included, but rather they are implied.	
;
; 				These are the program memory read/write functions. Erase is
; 				available through control flags. An automatic erase option
; 				is also available. A write lock indicator is in place to 
; 				insure intentional write operations.
;
; 				Note: write operations must be on 8-byte boundries and 
; 				must be 8 bytes long. Also erase operations can only
; 				occur on 64-byte boundries.
; *****************************************************************************
#ifdef 	ALLOW_GET_CMD
_PMRead:
	tblrd	*+						; Fill the buffer
	movff	TABLAT, POSTINC0
	decfsz	WREG1, F		
	bra		_PMRead					; Not finished then repeat
	
	bra		_CANSendResponce
#endif
; *********************************************************

; *********************************************************
_PMEraseWrite:
	btfss	MODE_AUTO_ERASE			; Erase if auto erase is requested
	bra		_PMWrite

_PMErase:
	movf	TBLPTRL, W				; Check for a valid 64 byte border
	andlw	b'00111111'
	bnz		_PMWrite

_PMEraseJp1
	movlw	b'10010100'				; Setup erase
	rcall	_StartWrite				; Erase the row

_PMWrite:
	btfsc	MODE_ERASE_ONLY			; Don't write if erase only is requested
#ifdef 	ALLOW_GET_CMD
	bra		_CANSendAck
#else
	bra		_CANMain
#endif

	movf	TBLPTRL, W				; Check for a valid 8 byte border
	andlw	b'00000111'
	bnz		_CANMain	
 	
	movlw	0x08
	movwf	WREG1

_PMWriteLp1
	movf	POSTINC0, W 			; Load the holding registers
	movwf	TABLAT
	
	rcall	_UpdateChksum			; Adjust the checksum

	tblwt	*+					
	
	decfsz	WREG1, F
	bra		_PMWriteLp1
 
#ifdef 	MODE_SELF_VERIFY 
 	movlw	0x08
 	movwf	WREG1
_PMWriteLp2
	tblrd	*-						; Point back into the block
	movf	POSTDEC0, W
	decfsz	WREG1, F
	bra		_PMWriteLp2

	movlw	b'10000100'				; Setup writes
	rcall	_StartWrite				; Write the data

	movlw	0x08
	movwf	WREG1

_PMReadBackLp1
	tblrd	*+						; Test the data
	movf	TABLAT, W
	xorwf	POSTINC0, W
	btfss	STATUS, Z
	bsf		ERR_VERIFY
	
	decfsz	WREG1, F		
	bra		_PMReadBackLp1			; Not finished then repeat

#else
	tblrd	*-						; Point back into the block

	movlw	b'10000100'				; Setup writes
	rcall	_StartWrite				; Write the data

	tblrd	*+						; Return the pointer position	
#endif

#ifdef 	ALLOW_GET_CMD
	bra		_CANSendAck
#else
	bra		_CANMain
#endif
; *****************************************************************************



	
; *****************************************************************************
; Function: 	VOID _CFGWrite()
;				VOID _CFGRead()
;
; PreCondition:	WREG1 and FSR0 must be loaded with the count and address of
;				the source data.
;
; Input:    	None.
;                               
; Output:   	None. 
;
; Side 
; Effects: 		N/A. 
;
; Stack 
; Requirements: N/A
;
; Overview: 	These routines are technically not functions since they will not 
;				return when called. They have been written in a linear form to 
;				save space.	Thus 'call' and 'return' instructions are not 
;				included, but rather they are implied.	
;
; 				This is the Config memory read/write functions. Read is 
; 				actually the same for standard program memory, so any read
;				request is passed directly to _PMRead.
; *****************************************************************************
_CFGWrite:		
#ifdef MODE_SELF_VERIFY				; Write to config area
	movf	INDF0, W				; Load data
#else
	movf	POSTINC0, W
#endif 				
	movwf	TABLAT
	
	rcall	_UpdateChksum			; Adjust the checksum
	
	tblwt	*						; Write the data

	movlw	b'11000100'
	rcall	_StartWrite				

	tblrd	*+						; Move the pointers and verify
	
#ifdef MODE_SELF_VERIFY
	movf	TABLAT, W
	xorwf	POSTINC0, W
	btfss	STATUS, Z
	bsf		ERR_VERIFY
#endif

	decfsz	WREG1, F
	bra		_CFGWrite				; Not finished then repeat
	
#ifdef 	ALLOW_GET_CMD
	bra		_CANSendAck
#else
	bra		_CANMain
#endif
; *****************************************************************************




; *****************************************************************************
; Function: 	VOID _EERead()
;				VOID _EEWrite()
;
; PreCondition:	WREG1 and FSR0 must be loaded with the count and address of
;				the source data.
;
; Input:    	None.
;                               
; Output:   	None. 
;
; Side 
; Effects: 		N/A. 
;
; Stack 
; Requirements: N/A
;
; Overview: 	These routines are technically not functions since they will not 
;				return when called. They have been written in a linear form to 
;				save space.	Thus 'call' and 'return' instructions are not 
;				included, but rather they are implied.	
;
; 				This is the EEDATA memory read/write functions.
; *****************************************************************************
#ifdef 	ALLOW_GET_CMD
_EERead:
	clrf	EECON1 

	bsf		EECON1, RD				; Read the data
	movff	EEDATA, POSTINC0
	
	infsnz	EEADR, F				; Adjust EEDATA pointer
	incf	EEADRH, F

	decfsz	WREG1, F
	bra		_EERead					; Not finished then repeat

	bra		_CANSendResponce
#endif
; *********************************************************
	
; *********************************************************
_EEWrite:
#ifdef MODE_SELF_VERIFY
	movf	INDF0, W				; Load data
#else
	movf	POSTINC0, W
#endif
	movwf	EEDATA
	
	rcall	_UpdateChksum			; Adjust the checksum

	movlw	b'00000100'				; Setup for EEData
	rcall	_StartWrite				; and write
	
#ifdef MODE_SELF_VERIFY
	clrf	EECON1 					; Read back the data
	bsf		EECON1, RD				; verify the data
	movf	EEDATA, W				; and adjust pointer
	xorwf	POSTINC0, W				
	btfss	STATUS, Z
	bsf		ERR_VERIFY
#endif
	
	infsnz	EEADR, F				; Adjust EEDATA pointer
	incf	EEADRH, F

	decfsz	WREG1, F		
	bra		_EEWrite				; Not finished then repeat

#ifdef 	ALLOW_GET_CMD
#else
	bra		_CANMain
#endif
; *****************************************************************************




; *****************************************************************************
; Function: 	VOID _CANSendAck()
;				VOID _CANSendResponce()
;
; PreCondition:	TXB0 must be preloaded with the data.
;
; Input:    	None.
;                               
; Output:   	None. 
;
; Side 
; Effects: 		N/A. 
;
; Stack 
; Requirements: N/A
;
; Overview: 	These routines are technically not functions since they will not 
;				return when called. They have been written in a linear form to 
;				save space.	Thus 'call' and 'return' instructions are not 
;				included, but rather they are implied.	
;
; 				These routines are used for 'talking back' to the source. The 
;				_CANSendAck routine sends an empty message to indicate 
;				acknowledgement of a memory write operation. The 
;				_CANSendResponce is used to send data back to the source.
; *****************************************************************************
#ifdef 	ALLOW_GET_CMD
_CANSendAck:
	btfss	MODE_ACK
	bra		_CANMain
	clrf	TXB0DLC					; Setup for a 0 byte transmission
	bra		_CANSendMessage
#endif
; *********************************************************

; *********************************************************
#ifdef 	ALLOW_GET_CMD
_CANSendResponce:
	movlw	0x08					; Setup for 8 byte transmission
	movwf	TXB0DLC

_CANSendMessage
	btfsc	TXB0CON,TXREQ			; Wait for the buffer to empty
	bra		$ - 2

	movlw	CAN_TXB0SIDH			; Set ID
	movwf	TXB0SIDH
	movlw	CAN_TXB0SIDL
	movwf	TXB0SIDL
	movlw	CAN_TXB0EIDH
	movwf	TXB0EIDH
	;movlw	CAN_TXB0EIDL
	movf	_vscpNickname,w			; reply with nickname as id
	movwf	TXB0EIDL
	
	bsf		CANTX_CD_BIT			; Setup the command bit
	btfss	CAN_CD_BIT
	bcf		CANTX_CD_BIT
			
	bsf		TXB0CON, TXREQ			; Start the transmission
	
	bra		_CANMain
#endif
; *****************************************************************************	


	END
