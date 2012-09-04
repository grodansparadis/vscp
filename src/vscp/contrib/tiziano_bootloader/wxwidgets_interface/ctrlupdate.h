/////////////////////////////////////////////////////////////////////////////
// Name:        ctrlupdate.h
// Purpose:     
// Author:      tiziano tosi
// Modified by: 
// Created:     20/05/2008 18:07:00
// RCS-ID:      
// Copyright:   proel
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _CTRLUPDATE_H_
#define _CTRLUPDATE_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/event.h"
////@end includes

#include "threadupdate.h"
#include "thread_symbols.h"

typedef unsigned char BINARY;

#define VSCP_TYPE_PROTOCOL_ACTIVATENEWIMAGE_ACK		48				
#define VSCP_TYPE_PROTOCOL_ACTIVATENEWIMAGE_NACK	49
#define VSCP_TYPE_PROTOCOL_START_BLOCK_ACK			50
#define VSCP_TYPE_PROTOCOL_START_BLOCK_NACK			51


#define BLOCKDATA_SIZE			512
#define INT_FLASH  				0
#define EXT_FLASH  				1
#define VSCP_BOOTLOADER_LPC1	0x10
#define USER_PROGRAM_ADDRESS	0x6000

class CDeviceNode;
class CDeviceNodeList;

class ctrlUpdate: public wxEvtHandler
{    
    DECLARE_CLASS( ctrlUpdate )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
	ctrlUpdate();
    ctrlUpdate(CDeviceNodeList* plistNode);

    /// Destructor
    ~ctrlUpdate();

    /// Initialises member variables
    void Init();

	/// Worker thread messages
	void eventLostCtrlIf( wxCommandEvent &event );
	void eventThreadMessage( wxCommandEvent &event );

	int EnterBootloaderModeMsg( int id, unsigned char* pBinMicroArray,
										int& nBinMicroSize,
										unsigned char* pBinDspArray,
										int& nBinDspSize );
	int StartBlockTransferMsg( int id, int nBlockNumber, int nWhichFlash );
	int DataBlockTransferMsg();
	int BlockProgramMsg( int nBlockNumber, int nWhichFlash );
	int ActivateNewImageMsg();
	void UpdateError(CDeviceNode* pNode);

	/// Common control object
	ctrlObj m_CtrlObject;
	/// Thread object    
    ThreadUpdate m_Thread;

    CDeviceNodeList* m_plistNode;

	enum {
		ID_CTRLUPDATE = wxID_ANY
	};

protected:
	int m_nCurrentBlockNumber;
	int m_nCurrentFlashType;
	BINARY *m_pIntFlashBinaryContent;              
    unsigned long m_IntFlashBinaryLength;
	BINARY *m_pExtFlashBinaryContent;              
    unsigned long m_ExtFlashBinaryLength;
	unsigned short m_nChecksum;
	unsigned int m_nTotalChecksum;
	int m_nNickname;
	wxDateTime m_tStartUpgrade;
	wxDateTime m_tDoneUpgrade;

};

#endif
    // _CTRLUPDATE_H_
