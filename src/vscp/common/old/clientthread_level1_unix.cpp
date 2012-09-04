// clientThread_level1_unix.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2008 Ake Hedman, D of Scandinavia,<akhe@eurosource.se>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// $RCSfile: clientthread_level1_unix.cpp,v $                                       
// $Date: 2005/10/04 09:23:18 $                                  
// $Author: akhe $                                              
// $Revision: 1.18 $ 

#define _POSIX
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>

#include "wx/wx.h"
#include "wx/defs.h"

// Common includes
#include "canal_macro.h"
#include "canal_unix_ipc.h"
#include "canalshmem_level1_unix.h"
#include "controlobject.h"
#include "../../common/dllist.h"
#include "canal.h"
#include "version.h"

#define VENDOR_STRING	"eurosource, Sweden. http://www.eurosource.se\0"

void clientThreadLevel1( void *pThreadObject )
{
  int rv;
  canalMsg *pMsg;
  dllnode *pNode;
  struct sembuf sb;
  CControlObject *pctrlObject = NULL;
 
  pthread_attr_t thread_attr;
  pthread_attr_init( &thread_attr );
  pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );	 
  
  _clientstruct clientInfo;
  memcpy( &clientInfo, (_clientstruct *)pThreadObject, sizeof( _clientstruct ) );
  pctrlObject = clientInfo.m_pctrlObject;

  wxLogDebug( _("Client Thread Level 1: Started") );
  
  // We need to create a clientobject and add this object to the list
  CClientItem *pClient = new CClientItem;
  if ( NULL == pClient ) {
	
    wxLogError( _("ClientThread LI: Failed to allocate client item. Possibly  memory problem?") );
    delete (_clientstruct *)pThreadObject;	// Delete the client structure
		
    rv = -1;
    pthread_exit( &rv );
 
  }
	
  // We need to get the shared memory
  void *m_pCommandMem;
  if ( -1 == (long)( m_pCommandMem = shmat( clientInfo.m_shmid, NULL, 0 ) ) ) {
    wxLogError( _("ClientThread LI: Failed to get shared memory?") );
    rv = -1;
    pthread_exit( &rv );
  }	
	
  struct __shmCanalLevelI *pLevel1 = (struct __shmCanalLevelI *)m_pCommandMem;
  void *pDataArea =  pLevel1->m_data;
  pLevel1->m_command = CANAL_COMMAND_NOOP;
	
  int semCmd;
  semCmd = semget( IPC_PRIVATE, 2, IPC_CREAT | 0666 );
  if ( -1 == semCmd ) {
	
    switch( errno) {

    case EACCES:
      wxLogError( _("ClientThread LI: Failed to get semaphors. EACCES") );
      break;
 
    case EEXIST:
      wxLogError( _("ClientThread LI: Failed to get semaphors. EEXIST:") );
      break;

    case ENOENT:
      wxLogError( _("ClientThread LI: Failed to get semaphors. ENOENT:") );
      break;

    case EINVAL:
      wxLogError( _("ClientThread LI: Failed to get semaphors. EINVAL:") );
      break;

    case ENOMEM:
      wxLogError( _("ClientThread LI: Failed to get semaphors. ENOMEM") );
      break;

    case ENOSPC:
      wxLogError( _("ClientThread LI: Failed to get semaphors. ENOSPC") );
      break;

    default:
      wxLogError( _("ClientThread LI: Failed to get semaphors. errno=%d"), errno );
      break;
 
    }

    // Release shared memory
    shmdt( (void *)m_pCommandMem );
	
    // Remove the share memory
    shmctl( clientInfo.m_shmid, IPC_RMID, 0 );
		
    pthread_exit( &rv );
  }
	
  // Share the command semaphore
  pLevel1->m_semCmd = semCmd;
				
  // Add the client to the Client List 
  pctrlObject->m_wxClientMutex.Lock();
  pctrlObject->addClient( pClient );
  pctrlObject->m_wxClientMutex.Unlock();
 
  wxLogDebug( _("Client Thread Level 1: Client added to the client queue")); 
	
  pLevel1->m_test = 0xaa55;	// Confirm to client that we are here 
				
  pClient->m_bOpen = true;
  while ( pClient->m_bOpen && !pctrlObject->m_bQuit ) {
			
    // Wait for command from client
    wxLogDebug(_("Client Thread Level 1: Waiting for command"));
    sb.sem_num = CLIENT_CMD_SEM;
    sb.sem_op = -1;
    sb.sem_flg = 0;
    semop( semCmd, &sb, 1 );
    wxLogDebug(_("Client Thread Level 1: Command received"));
		
    switch( pLevel1->m_command ) {
		
      // * * * *  O P E N  * * * *
    case CANAL_COMMAND_OPEN:
      wxLogDebug(_("Client Thread Level 1: Open"));
      pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;
      break;
					
      // * * * *  C L O S E  * * * *
    case CANAL_COMMAND_CLOSE:
			
      pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;
      wxLogDebug(_("Client Thread Level 1: Close"));
      pClient->m_bOpen = false;	 
      break;
				
      // * * * *  S E N D  * * * *
    case CANAL_COMMAND_SEND:
			
      wxLogDebug(_("Client Thread Level 1: Send"));
      pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;
						
      pNode = new dllnode;					
      if ( NULL != pNode ) {

	pMsg = new canalMsg;	// Create new Canal Message
						
	if ( NULL != pMsg ) {
								
	  // Save the originating clients id so
	  // this client dont get the message back
	  pNode->obid = pClient->m_clientID;
								
	  // Copy message			
	  memcpy( pMsg, pDataArea, sizeof( canalMsg ) );

	  // Statistics
	  pClient->m_statistics.cntTransmitData += pMsg->sizeData;
	  pClient->m_statistics.cntTransmitFrames++;

	  // We use the message id as the sort key for the message
	  // in the queue. The high part of this id is the priority
	  // for the message.
	  // The non-id part should possibly be nulled but this depends
	  // on if we regard the message as "sent" or not. If not, a 
	  // message with a lower id should be transfered first over the 
	  // bus.
								
	  pNode->Key = pMsg->id;
	  pNode->pKey = &pNode->Key;
	  pNode->pObject = pMsg;
								
	  // There must be room in the send queue
	  if ( pctrlObject->m_maxItemsInSendQueue > 
	       pctrlObject->m_sendLevel1Queue.nCount ) {						
	    pctrlObject->m_wxQ1OutMutex.Lock();

	    if ( !dll_addNode( &pctrlObject->m_sendLevel1Queue, pNode ) ) {
	      pLevel1->m_Response = CANAL_RESPONSE_ERROR;	
	      pLevel1->m_ResponseCode = CANAL_IFERROR_SEND_STORAGE;
	    }
	    else {
	      pLevel1->m_Response = CANAL_RESPONSE_SUCCESS; 
	    }

	    pctrlObject->m_wxQ1OutMutex.Unlock();

	  }
	  else {
						
	    pLevel1->m_Response = CANAL_RESPONSE_ERROR;	
	    pLevel1->m_ResponseCode = CANAL_IFERROR_BUFFER_FULL;
									
	    if ( NULL != pNode ) {
	      if ( NULL != pNode->pObject ) {
		delete pMsg;
	      }
	      delete pNode;
	    }
	  }
	}
	else {
								
	  delete pNode;
								
	  pLevel1->m_Response = CANAL_RESPONSE_ERROR;
	  pLevel1->m_ResponseCode = CANAL_IFERROR_SEND_MSG_ALLOCATON;
	}
      } // dllNode 
      wxLogDebug(_("Client Thread Level 1: Send - Done"));
      break;	
				
				
      // * * * *  R E C E I V E  * * * *
    case CANAL_COMMAND_RECEIVE:
		
      wxLogDebug(_("Client Thread Level 1: Receive"));
      if ( !pClient->m_bOpen ) {
	pLevel1->m_Response = CANAL_RESPONSE_ERROR;
	pLevel1->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
      }
      else {
 
	pClient->m_wxMsgMutex.Lock();	
								
	dllnode *pNode; 
	canalMsg *pMsg = (canalMsg *)pDataArea;
						 
	if ( NULL != ( pNode = pClient->m_inputQueue.pHead ) ) {
						
	  // There is a message available
	  pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;	
	  memcpy( pMsg, pNode->pObject, sizeof( canalMsg ) );

	  // Remove the node
	  dll_removeNode( &pClient->m_inputQueue, pNode );

	}
	
	pClient->m_wxMsgMutex.Unlock();			 		
 		
      }				 
      break;	
				
				
      // * * * *  D A T A  A V A I L A B L E  * * * *
    case CANAL_COMMAND_CHECKDATA:

      if ( !pClient->m_bOpen ) {
	// Not Open
	pLevel1->m_Response = CANAL_RESPONSE_ERROR;
	pLevel1->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
      }
      else {
	pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;

	unsigned int *pnMsg = (unsigned int *)pDataArea;
	//*pnMsg = dll_getNodeCount( &pClient->m_inputQueue ); 
	*pnMsg = pClient->m_inputQueue.nCount;
      }
      break;	
			
			
      // * * * *  S T A T U S  * * * *
    case CANAL_COMMAND_STATUS:

      if ( !pClient->m_bOpen ) {
	pLevel1->m_Response = CANAL_RESPONSE_ERROR;
	pLevel1->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
      }
      else {
	pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;

	// Status
	memcpy( pDataArea, &pClient->m_status, sizeof( canalStatus ) );
			
      } 
      break;	
				
				
      // * * * *  S T A T I S T I C S  * * * *
    case CANAL_COMMAND_STATISTICS:

      if ( !pClient->m_bOpen ) {
	pLevel1->m_Response = CANAL_RESPONSE_ERROR;
	pLevel1->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
      }
      else {
	memcpy( pDataArea, &pClient->m_statistics, sizeof( &pClient->m_statistics ) );
	pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;
      } 
      break;
				
				
      // * * * *  F I L T E R  * * * *
    case CANAL_COMMAND_FILTER:

      if ( !pClient->m_bOpen ) {
	pLevel1->m_Response = CANAL_RESPONSE_ERROR;
	pLevel1->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
      }
      else {
						
	// Set filter
	memcpy(  &pClient->m_filter, pDataArea, sizeof( pClient->m_filter ) );
	pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;
      }  
      break;	
				
      // * * * *  M A S K  * * * *
    case CANAL_COMMAND_MASK:
					
      if ( !pClient->m_bOpen ) {
	pLevel1->m_Response = CANAL_RESPONSE_ERROR;
	pLevel1->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
      }
      else {
						
	// Set mask				
	memcpy( &pClient->m_mask, pDataArea, sizeof( pClient->m_mask ) );
	pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;

      } 	
      break;	
	
	
      // * * * *  B A U D R A T E  * * * *
    case CANAL_COMMAND_BAUDRATE:

      if ( !pClient->m_bOpen ) {
	pLevel1->m_Response = CANAL_RESPONSE_ERROR;
	pLevel1->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
      }
      else {
						
	// Set baudrate
	// ============
	// We don't have yo do more here the baudrate change
	// is just simulated in the pipe interface
	pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;
      } 	
      break;
				
      // * * * *  V E R S I O N  * * * *
    case CANAL_COMMAND_VERSION:
      {	
	// Get version
	char *p = (char *)pDataArea;
	*( p + POS_VSCPD_MAJOR_VERSION ) = VSCPD_MAJOR_VERSION;
	*( p + POS_VSCPD_MINOR_VERSION ) = VSCPD_MINOR_VERSION;
	*( p + POS_VSCPD_SUB_VERSION ) = VSCPD_SUB_VERSION;
      }		
      pLevel1->m_Response = CANAL_RESPONSE_SUCCESS; 	
      break;

      // * * * *  D L L - V E R S I O N  * * * *
    case CANAL_COMMAND_DLL_VERSION:
					
      // Get DLL version
      // ===============
      // This application returns the vscp daemon
      // version.
      {
	char *p = (char *)pDataArea;
	*( p + POS_VSCPD_MAJOR_VERSION ) = VSCPD_MAJOR_VERSION; 
	*( p + POS_VSCPD_MINOR_VERSION ) = VSCPD_MINOR_VERSION;
	*( p + POS_VSCPD_SUB_VERSION ) = VSCPD_SUB_VERSION;
	*( p + POS_VSCPD_SUB_VERSION + 1 ) = 0;
      }
      pLevel1->m_Response = CANAL_RESPONSE_SUCCESS; 	
      break;

      // * * * *  V E N D O R  S T R I N G  * * * *
    case CANAL_COMMAND_VENDOR_STRING:
      // Get vendor string				
      pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;	
      {
	char *p = (char *)pDataArea;
	strcpy( p, VENDOR_STRING );
      }
      break;

      // * * * *  L E V E L  * * * *
    case CANAL_COMMAND_LEVEL:
					
      // Get i/f supported levels
      {
	char *p = (char *)pDataArea;
	*( p + 0 ) = 0; 
	*( p + 1 ) = 0;
	*( p + 2 ) = 0;
	*( p + 3 ) = CANAL_LEVEL_STANDARD;
      }
      pLevel1->m_Response = CANAL_RESPONSE_SUCCESS; 	
      break;
					
      // * * * *  N O O P  * * * *
    case CANAL_COMMAND_NOOP:
			
      pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;
      break;
				
    default:
			
      pLevel1->m_Response = CANAL_RESPONSE_ERROR;
      pLevel1->m_ResponseCode = CANAL_IFERROR_UNKNOWN_COMMAND;
      break;	
		
    } // Select

    // Done - Ready for new command	
    //wxLogDebug("Client Thread Level 1: Before Done");
    sb.sem_num = CLIENT_DONE_SEM;
    sb.sem_op = 1;
    sb.sem_flg = 0;
    semop( semCmd, &sb, 1 );		
    //wxLogDebug("Client Thread Level 1: After Done");
				 
  } // while running


	// Remove messages in the client queues
  pctrlObject->m_wxReceiveMutex.Lock();
  dll_removeAllNodes( &pClient->m_inputQueue );
  pctrlObject->m_wxReceiveMutex.Unlock();
	
  pctrlObject->removeClient( pClient );
	
  // Release shared memory
  shmdt( (void *)m_pCommandMem );
	
  // Remove the share memory
  if( -1 == shmctl( clientInfo.m_shmid, IPC_RMID, 0 ) ) {
    wxLogError(_("ClientThread LI: Failed to remove shared memory"));
  }
	
  // remove the semaphore
  semctl( semCmd, 0, IPC_RMID, NULL );
	
  delete (_clientstruct *)pThreadObject;

  pthread_exit( &rv );
 
}




 

