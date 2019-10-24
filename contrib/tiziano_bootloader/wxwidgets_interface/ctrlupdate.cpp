/////////////////////////////////////////////////////////////////////////////
// Name:        ctrlupdate.cpp
// Purpose:     
// Author:      tiziano tosi
// Modified by: 
// Created:     20/05/2008 18:07:00
// RCS-ID:      
// Copyright:   proel
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "../pronetapp.h"
#include <wx/listimpl.cpp>
#include "../DeviceData/DeviceNode.h"
#include "../DeviceData/DeviceNodeList.h"
#include "../System/vscpregisterdef.h"
#include "threadupdate.h"
#include "ctrlupdate.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ctrlUpdate type definition
//

IMPLEMENT_CLASS( ctrlUpdate, wxEvtHandler )

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ctrlUpdate event table definition
//

BEGIN_EVENT_TABLE( ctrlUpdate, wxEvtHandler )
	EVT_COMMAND( ID_CTRLUPDATE, wxVSCP_CTRL_LOST, ctrlUpdate::eventLostCtrlIf )
	EVT_COMMAND( ID_CTRLUPDATE, wxUPDATE_EVENT, ctrlUpdate::eventThreadMessage )
END_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ctrlUpdate constructors
//

ctrlUpdate::ctrlUpdate()
{
	Init();
}

ctrlUpdate::ctrlUpdate( CDeviceNodeList* plistNode )
{
	Init();
    m_plistNode = plistNode;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ctrlUpdate destructor
//

ctrlUpdate::~ctrlUpdate()
{
	m_Thread.stopWorkerThreads();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Member initialisationn
//

void ctrlUpdate::Init()
{
	m_CtrlObject.m_pVSCPSessionWnd = (wxWindow *)this;
	m_CtrlObject.m_winid = ID_CTRLUPDATE;
    m_Thread.m_pCtrlObject = &m_CtrlObject;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// eventThreadMessage
//

void ctrlUpdate::eventThreadMessage( wxCommandEvent &event )
{
	int write_pointer,checksum16;
	unsigned long intflash_blocksize,max_intflash_blocknumber,extflash_blocksize,max_extflash_blocknumber;
	int block_number;
	int i = event.GetInt();
	int id = i & 0xFFFF;
	int type = (i >> 16) & 0xFFFF;
	CDeviceNode* pNode = m_plistNode->GetNodeByNickName(id);
	wxString strData = event.GetString();
	vscpEvent* pVscp = new vscpEvent;
	getVscpDataFromString( pVscp, strData );

	if(pNode != NULL)
	{
		if(event.GetExtraLong()==0)
		{
			switch(type)
			{
				case VSCP_TYPE_PROTOCOL_ACK_BOOT_LOADER:
					// pVscp->pdata[ 0 ] = 2;	MSB internal flash block size
					// pVscp->pdata[ 1 ] = 2;	MSB spi flash block size
					// pVscp->pdata[ 2 ] = 0;	LSB spi flash block size
					// pVscp->pdata[ 3 ] = 0;	LSB internal flash block size
					// pVscp->pdata[ 4 ] = 1;	MSB internal flash number of block avalaible
					// pVscp->pdata[ 5 ] = 4;	MSB spi flash number of block avalaible
					// pVscp->pdata[ 6 ] = 0;	LSB spi flash number of block avalaible
					// pVscp->pdata[ 7 ] = 0;	LSB internal flash number of block avalaible
					intflash_blocksize = (pVscp->pdata[0] << 8) | pVscp->pdata[3];
					extflash_blocksize = (pVscp->pdata[1] << 8) | pVscp->pdata[2];
					max_intflash_blocknumber = (pVscp->pdata[4] << 8) | pVscp->pdata[7];
					max_extflash_blocknumber = (pVscp->pdata[5] << 8) | pVscp->pdata[6];

					if((intflash_blocksize == BLOCKDATA_SIZE)	&&
				       (extflash_blocksize == BLOCKDATA_SIZE)	&&
					   (max_intflash_blocknumber >= (m_IntFlashBinaryLength/BLOCKDATA_SIZE))&&
					   (max_extflash_blocknumber >= (m_ExtFlashBinaryLength/BLOCKDATA_SIZE))
					  )
					{
						crcInit();
						m_nTotalChecksum = 0;
						m_nCurrentBlockNumber = 0;
						m_nCurrentFlashType = INT_FLASH;	// internal flash
						// attenzione: questo messaggio (VSCP_TYPE_PROTOCOL_ACK_BOOT_LOADER) è l'ultimo arrivato dal 
						// programma user, ora ha preso il controllo il bootloader; è probabile che sia necessario
						// un delay prima di trasmettere il messaggio di "start block transfer"
						wxMilliSleep( 2000 );//wxMilliSleep( 500 );//wxMilliSleep( 200 );
						StartBlockTransferMsg( pNode->GetNickName(), m_nCurrentBlockNumber, m_nCurrentFlashType );
					}
					else
					{
						UpdateError(pNode);
					}

					//::wxGetApp().logMsg ( _("event ack bootloader"), DAEMON_LOGMSG_CRITICAL );
					break;
				case VSCP_TYPE_PROTOCOL_START_BLOCK_ACK:
					// pVscp->pdata[0] MSB block number
					// pVscp->pdata[1] INTERNAL_FLASH/SPI_FLASH
					// pVscp->pdata[2]
					// pVscp->pdata[3] LSB block number
					if(m_nCurrentFlashType == pVscp->pdata[1])
					{
						switch(m_nCurrentFlashType)
						{
							case INT_FLASH:
								DataBlockTransferMsg();
								m_nChecksum = crcFast( &m_pIntFlashBinaryContent[USER_PROGRAM_ADDRESS + m_nCurrentBlockNumber*BLOCKDATA_SIZE], BLOCKDATA_SIZE );//crcFast( &m_pIntFlashBinaryContent[30208], BLOCKDATA_SIZE );//crcFast( &m_pIntFlashBinaryContent[USER_PROGRAM_ADDRESS + m_nCurrentBlockNumber*BLOCKDATA_SIZE], BLOCKDATA_SIZE );
								//m_nTotalChecksum += m_nChecksum;
								break;
							case EXT_FLASH:

								DataBlockTransferMsg();
								m_nChecksum = crcFast( &m_pExtFlashBinaryContent[m_nCurrentBlockNumber*BLOCKDATA_SIZE], BLOCKDATA_SIZE );
								break;
						}
					}
					else
					{
						UpdateError(pNode);
					}
					//::wxGetApp().logMsg ( _("event ack data block"), DAEMON_LOGMSG_CRITICAL );
					break;
				case VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK:
					// pVscp->pdata[0] = (checksum16 >> 8) & 0xFF;		MSB 16 bit CRC for block
					// pVscp->pdata[1] = checksum16 & 0xFF;				LSB 16 bit CRC for block
					// pVscp->pdata[2] = (write_pointer >> 8) & 0xFF;	MSB of block number 
					// pVscp->pdata[3] = 0;
					// pVscp->pdata[4] = 0;
					// pVscp->pdata[5] = write_pointer & 0xFF;			LSB of block number
					checksum16 = (pVscp->pdata[0] << 8) | pVscp->pdata[1];
					write_pointer = (pVscp->pdata[2] << 8) | pVscp->pdata[5];
					if((checksum16 == m_nChecksum) && (write_pointer == m_nCurrentBlockNumber))
						BlockProgramMsg( m_nCurrentBlockNumber, m_nCurrentFlashType );
					else
					{
						// in questo caso di errore si ritrasmette il blocco
						StartBlockTransferMsg( pNode->GetNickName(), m_nCurrentBlockNumber, m_nCurrentFlashType );
						//UpdateError(pNode);
					}

					//::wxGetApp().logMsg ( _("event ack data block"), DAEMON_LOGMSG_CRITICAL );
					break;
				case VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK:
					// pVscp->pdata[0] MSB block number
					// pVscp->pdata[1] INTERNAL_FLASH/SPI_FLASH
					// pVscp->pdata[2]
					// pVscp->pdata[3] LSB block number
					block_number = (pVscp->pdata[0] << 8) | pVscp->pdata[3];
					if((block_number == m_nCurrentBlockNumber) && (pVscp->pdata[1] == m_nCurrentFlashType))
					{
						switch(m_nCurrentFlashType)
						{
							case INT_FLASH:
								m_nCurrentBlockNumber++;
								//::wxGetApp().logMsg ( _("event PROGRAM ACK"), DAEMON_LOGMSG_CRITICAL );
								m_nTotalChecksum += m_nChecksum;
								if(m_nCurrentBlockNumber == (int)((m_IntFlashBinaryLength - USER_PROGRAM_ADDRESS)/BLOCKDATA_SIZE))
								{
									m_nCurrentBlockNumber = 0;
									m_nCurrentFlashType = EXT_FLASH;
									
							//ActivateNewImageMsg();	// DEBUG!!
								}
							//else // DEBUG!!
							//{// DEBUG!!
								// avanzamento progress control della dialog
								pNode->FirmwareProgressStep();
								wxGetApp().Yield();
								StartBlockTransferMsg( pNode->GetNickName(), m_nCurrentBlockNumber, m_nCurrentFlashType );
							//}// DEBUG!!
								break;
							case EXT_FLASH:
								m_nCurrentBlockNumber++;
								if(m_nCurrentBlockNumber == (int)(m_ExtFlashBinaryLength/BLOCKDATA_SIZE))
								{
									ActivateNewImageMsg();
								}
								else
								{
									// avanzamento progress control della dialog
									pNode->FirmwareProgressStep();
									wxGetApp().Yield();
									StartBlockTransferMsg( pNode->GetNickName(), m_nCurrentBlockNumber, m_nCurrentFlashType );
								}
								break;
						}
					}
					else
					{
						UpdateError(pNode);
					}

					//::wxGetApp().logMsg ( _("event ack program block"), DAEMON_LOGMSG_CRITICAL );
					break;
				case VSCP_TYPE_PROTOCOL_ACTIVATENEWIMAGE_ACK:
					// delay per attendere che il nodo remoto abbia finito la fase di 
					// inizializzazione altrimenti quando l'utente chiude la dialog di update
					// c'e' il rischio che il gestore dell' heartbeat non rilevi il nuovo stato (stato user)
					// del nodo
					wxMilliSleep( 8000 );

					m_tDoneUpgrade = wxDateTime::Now();
					::wxGetApp().logMsg ( wxT("Upgrade Finished... taking: ") + (m_tDoneUpgrade - m_tStartUpgrade).Format(), DAEMON_LOGMSG_CRITICAL );

					// avvisa la dialog che è finito il processo di update
					pNode->EndFirmwareProgress(FIRMWAREUPDATEOK);
					break;

				case VSCP_TYPE_PROTOCOL_NACK_BOOT_LOADER:
					UpdateError(pNode);
					::wxGetApp().logMsg ( _("event NACK bootloader"), DAEMON_LOGMSG_CRITICAL );
					break;
				case VSCP_TYPE_PROTOCOL_START_BLOCK_NACK:
					UpdateError(pNode);
					::wxGetApp().logMsg ( _("event NACK start block"), DAEMON_LOGMSG_CRITICAL );
					break;
				case VSCP_TYPE_PROTOCOL_BLOCK_DATA_NACK:
					UpdateError(pNode);
					::wxGetApp().logMsg ( _("event NACK data block"), DAEMON_LOGMSG_CRITICAL );
					break;
				case VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_NACK:
					UpdateError(pNode);
					::wxGetApp().logMsg ( _("event NACK program block"), DAEMON_LOGMSG_CRITICAL );
					break;
				case VSCP_TYPE_PROTOCOL_ACTIVATENEWIMAGE_NACK:
					UpdateError(pNode);
					::wxGetApp().logMsg ( _("event NACK activate new image"), DAEMON_LOGMSG_CRITICAL );
					break;
			}
		}
		else
		{
			UpdateError(pNode);
		}
	}

	deleteVSCPevent( pVscp );
}

/////////////////////////////////////////////////////////////////// 
// eventLostCtrlIf
//

void ctrlUpdate::eventLostCtrlIf( wxCommandEvent& WXUNUSED(event) )
{

}

/////////////////////////////////////////////////////////////////// 
/////////////////////////////////////////////////////////////////// 
/////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EnterBootLoaderMsg
//

int ctrlUpdate::EnterBootloaderModeMsg( int id,  unsigned char* pBinMicroArray,
											int& nBinMicroSize,
											unsigned char* pBinDspArray,
											int& nBinDspSize )
{
	m_nNickname = id;
	m_pIntFlashBinaryContent = pBinMicroArray;              
    m_IntFlashBinaryLength = (unsigned long)nBinMicroSize;
	m_pExtFlashBinaryContent = pBinDspArray;              
    m_ExtFlashBinaryLength = (unsigned long)nBinDspSize;
	
	m_tStartUpgrade = wxDateTime::Now();

	m_CtrlObject.m_mutexOutQueue.Lock();
    vscpEvent *pEvent = new vscpEvent;
    pEvent->head = 0;
	pEvent->vscp_class = VSCP_CLASS_PROTOCOL_FUNCTIONALITY;			
	pEvent->vscp_type = VSCP_TYPE_PROTOCOL_ENTER_BOOT_LOADER;           
	pEvent->sizeData = 8;									
	pEvent->pdata = new unsigned char[ pEvent->sizeData ];
	pEvent->pdata[0] = id;						// nodeid
	pEvent->pdata[1] = VSCP_BOOTLOADER_LPC1;	// bootloader algorithm code
	pEvent->pdata[2] = 0;						// GUID byte 0
	pEvent->pdata[3] = 0;						// GUID byte 3
	pEvent->pdata[4] = 0;						// GUID byte 5
	pEvent->pdata[5] = 0;						// GUID byte 7
	pEvent->pdata[6] = 0;						// content of Page select MSB register
	pEvent->pdata[7] = 0;						// content of Page select LSB register
	pEvent->GUID[12] = m_nNickname;
	m_CtrlObject.m_outQueue.Append( pEvent );
    m_CtrlObject.m_semOutQue.Post();
    m_CtrlObject.m_mutexOutQueue.Unlock();
	//::wxGetApp().logMsg ( _("start enter bootloader"), DAEMON_LOGMSG_CRITICAL );
	
	
	wxMutexLocker lock( *m_CtrlObject.m_pMutex );
    m_CtrlObject.m_pCondition->Wait();
	//::wxGetApp().logMsg ( _("enter bootloader"), DAEMON_LOGMSG_CRITICAL );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// StartBlockTransferMsg
//

int ctrlUpdate::StartBlockTransferMsg( int id, int nBlockNumber, int nWhichFlash )
{
	m_CtrlObject.m_mutexOutQueue.Lock();
    vscpEvent *pEvent = new vscpEvent;
    pEvent->head = 0;
	pEvent->vscp_class = VSCP_CLASS_PROTOCOL_FUNCTIONALITY;			
	pEvent->vscp_type = VSCP_TYPE_PROTOCOL_START_BLOCK;           
	pEvent->sizeData = 4;									
	pEvent->pdata = new unsigned char[ pEvent->sizeData ];
	pEvent->pdata[0] = (nBlockNumber >> 8) & 0xFF;	// MSB block number
	pEvent->pdata[1] = nWhichFlash;					// INTERNAL_FLASH/SPI_FLASH
	pEvent->pdata[2] = id;							// ID
	pEvent->pdata[3] = nBlockNumber & 0xFF;			// LSB block number
	pEvent->GUID[12] = m_nNickname;
	m_CtrlObject.m_outQueue.Append( pEvent );
    m_CtrlObject.m_semOutQue.Post();
    m_CtrlObject.m_mutexOutQueue.Unlock();
	//::wxGetApp().logMsg ( _("start start block transfer"), DAEMON_LOGMSG_CRITICAL );
	
	wxMutexLocker lock( *m_CtrlObject.m_pMutex );
    m_CtrlObject.m_pCondition->Wait();
	//::wxGetApp().logMsg ( _("start block transfer"), DAEMON_LOGMSG_CRITICAL );
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DataBlockTransferMsg
//
int ctrlUpdate::DataBlockTransferMsg()
{
	wxMutexLocker lock( *m_CtrlObject.m_pMutex );
	
	uint8_t* pData;
	int j;

	for(j=0; j < BLOCKDATA_SIZE/8; j++)
	{
		switch(m_nCurrentFlashType)
		{
			case INT_FLASH:
				pData = &m_pIntFlashBinaryContent[USER_PROGRAM_ADDRESS + m_nCurrentBlockNumber*BLOCKDATA_SIZE + j*8];//&m_pIntFlashBinaryContent[30208+j*8];//&m_pIntFlashBinaryContent[USER_PROGRAM_ADDRESS + m_nCurrentBlockNumber*BLOCKDATA_SIZE + j*8];
				break;
			case EXT_FLASH:
				pData = &m_pExtFlashBinaryContent[m_nCurrentBlockNumber*BLOCKDATA_SIZE + j*8];
				break;
		}
		
		vscpEvent *pEvent = new vscpEvent;
		pEvent->head = 0;
		pEvent->vscp_class = VSCP_CLASS_PROTOCOL_FUNCTIONALITY;			
		pEvent->vscp_type = VSCP_TYPE_PROTOCOL_BLOCK_DATA;
		pEvent->sizeData = 8;
		pEvent->GUID[3] = (uint8_t)(j >> 8);					// MSB n.messaggi
		pEvent->GUID[4] = j;									// LSB n.messaggi
		pEvent->GUID[12] = m_nNickname;
		pEvent->pdata = new unsigned char[ pEvent->sizeData ];
		for(int i=0; i < pEvent->sizeData; i++)
			pEvent->pdata[i] = pData[i];
		m_CtrlObject.m_mutexOutQueue.Lock();
		m_CtrlObject.m_outQueue.Append( pEvent );
		m_CtrlObject.m_semOutQue.Post();
		m_CtrlObject.m_mutexOutQueue.Unlock();
		//wxMilliSleep( 6 );//wxMilliSleep( 8 );
	}

	m_CtrlObject.m_pCondition->Wait();
	
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BlockProgramMsg
//

int ctrlUpdate::BlockProgramMsg( int nBlockNumber, int nWhichFlash )
{
	m_CtrlObject.m_mutexOutQueue.Lock();
    vscpEvent *pEvent = new vscpEvent;
    pEvent->head = 0;
	pEvent->vscp_class = VSCP_CLASS_PROTOCOL_FUNCTIONALITY;			
	pEvent->vscp_type = VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA;           
	pEvent->sizeData = 4;									
	pEvent->pdata = new unsigned char[ pEvent->sizeData ];
	pEvent->pdata[0] = (nBlockNumber >> 8) & 0xFF;	// MSB block number
	pEvent->pdata[1] = nWhichFlash;					// INTERNAL_FLASH/SPI_FLASH
	pEvent->pdata[2] = 0;							// 
	pEvent->pdata[3] = nBlockNumber & 0xFF;			// LSB block number
	pEvent->GUID[12] = m_nNickname;
	m_CtrlObject.m_outQueue.Append( pEvent );
    m_CtrlObject.m_semOutQue.Post();
    m_CtrlObject.m_mutexOutQueue.Unlock();
	//::wxGetApp().logMsg ( _("start block program"), DAEMON_LOGMSG_CRITICAL );
	
	wxMutexLocker lock( *m_CtrlObject.m_pMutex );
    m_CtrlObject.m_pCondition->Wait();
	//::wxGetApp().logMsg ( _("block program"), DAEMON_LOGMSG_CRITICAL );
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ActivateNewImageMsg
//

int ctrlUpdate::ActivateNewImageMsg()
{
	m_CtrlObject.m_mutexOutQueue.Lock();
    vscpEvent *pEvent = new vscpEvent;
    pEvent->head = 0;
	pEvent->vscp_class = VSCP_CLASS_PROTOCOL_FUNCTIONALITY;			
	pEvent->vscp_type = VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE;           
	pEvent->sizeData = 2;									
	pEvent->pdata = new unsigned char[ pEvent->sizeData ];
	pEvent->pdata[0] = (m_nTotalChecksum >> 8) & 0xFF;	// MSB crc full flash data
	pEvent->pdata[1] = m_nTotalChecksum & 0xFF;			// LSB crc full flash data
	pEvent->GUID[12] = m_nNickname;
	m_CtrlObject.m_outQueue.Append( pEvent );
    m_CtrlObject.m_semOutQue.Post();
    m_CtrlObject.m_mutexOutQueue.Unlock();
	::wxGetApp().logMsg ( _("start activate new image"), DAEMON_LOGMSG_CRITICAL );
	
	wxMutexLocker lock( *m_CtrlObject.m_pMutex );
    m_CtrlObject.m_pCondition->Wait();
	::wxGetApp().logMsg ( _("activate new image"), DAEMON_LOGMSG_CRITICAL );
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UpdateError
//

void ctrlUpdate::UpdateError(CDeviceNode* pNode)
{
	// avvisa la dialog che è fallita la procedura di update
	pNode->EndFirmwareProgress(FIRMWAREUPDATEERROR);
}
