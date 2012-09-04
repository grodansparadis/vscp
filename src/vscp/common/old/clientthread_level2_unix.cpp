// clientThread_level2_unix.cpp
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
// $RCSfile: clientthread_level2_unix.cpp,v $                                       
// $Date: 2005/10/04 09:23:18 $                                  
// $Author: akhe $                                              
// $Revision: 1.16 $ 

#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>

#include "wx/wx.h"
#include "wx/defs.h"

// Project includes
#include "version.h"
#include "canal.h"
#include "canal_unix_ipc.h"
#include "canalshmem_level2_unix.h"
#include "controlobject.h"
#include "../../common/dllist.h"

void clientThreadLevel2( void *pThreadObject )
{
  int rv;
  vscpEvent *pEvent;
  dllnode *pNode;
  struct sembuf sb;
  CControlObject *pctrlObject = NULL;
 
  pthread_attr_t thread_attr;
  pthread_attr_init( &thread_attr );
  pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );	 
	  
  _clientstruct clientInfo;
  memcpy( &clientInfo, (_clientstruct *)pThreadObject, sizeof( _clientstruct ) );
  pctrlObject = clientInfo.m_pctrlObject;
  
  // We need to create a clientobject and add this object to the list
  CClientItem *pClient = new CClientItem;
  if ( NULL == pClient ) {
	
    delete (_clientstruct *)pThreadObject;	// Delete the client structure
		
    rv = -1;
    pthread_exit( &rv );
 
  }
	
  // This is a Level II Client
  pClient->m_ClientLevel = CLIENT_ITEM_LEVEL2;
	
  // We need to get the shared memory
  void *m_pCommandMem;
  if ( -1 == (long)( m_pCommandMem = shmat( clientInfo.m_shmid, NULL, 0 ) ) ) {
    rv = -1;
    pthread_exit( &rv );
  }	
	
  struct __shmCanalLevelII *pLevel2 = (struct __shmCanalLevelII *)m_pCommandMem;
  void *pDataArea= pLevel2->m_data;
  vscpEvent *pEventArea = &pLevel2->m_VSCP_Event;
  pLevel2->m_command = VSCP_COMMAND_NOOP;
	
  int semCmd;
  semCmd = semget( IPC_PRIVATE, 1, IPC_CREAT | 0666 );
  if ( -1 == semCmd ) {
	
    // Release shared memory
    shmdt( (void *)m_pCommandMem );
	
    // Remove the share memory
    shmctl( clientInfo.m_shmid, IPC_RMID, 0 );
		
    pthread_exit( &rv );
  }
	
  // Share the command semaphore
  pLevel2->m_semCmd = semCmd;
				
  // Add the client to the Client List
  pctrlObject->m_wxClientMutex.Lock();
  pctrlObject->addClient( pClient );
  pctrlObject->m_wxClientMutex.Unlock();
	
  pLevel2->m_test = 0xaa55;	// Confirm to client that we are here 
				
  pClient->m_bOpen = true;
  while ( pClient->m_bOpen && !pctrlObject->m_bQuit ) {
			
    // Wait for command from client	
    sb.sem_num = CLIENT_CMD_SEM;
    sb.sem_op = -1;
    sb.sem_flg = 0;
    semop( semCmd, &sb, 1 );
				
    switch( pLevel2->m_command ) {
		
      // * * * *  O P E N  * * * *
    case VSCP_COMMAND_OPEN:
	
      pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;
      break;
					
      // * * * *  C L O S E  * * * *
    case VSCP_COMMAND_CLOSE:
			
      pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;	
      pClient->m_bOpen = false;	 
      break;
				
      // * * * *  S E N D  * * * *
    case VSCP_COMMAND_SEND:
			
      pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;
						
      pNode = new dllnode;
      if ( NULL != pNode ) {

	pEvent = new vscpEvent;		// Create new VSCP Message
						
	if ( NULL != pEvent ) {
					
	  // Save the originating clients id so
	  // this client dont get the message back
	  pNode->obid = pClient->m_clientID;
								
	  // Copy message								
	  memcpy( pEvent, pEventArea, sizeof( vscpEvent ) );
					
	  // And data...
	  if ( pEvent->sizeData > 0 ) {	
						
	    // Copy in data
	    pEvent->pdata = new uint8_t[ pEvent->sizeData ];
	    if ( NULL != pEvent->pdata ) {
	      memcpy( pEvent->pdata, pDataArea ,pEvent->sizeData );
	    }
							
	  }
	  else {
	    // No data
	    pEvent->pdata = NULL;
	  }
						
	  // Check if GUID of interface should be used
	  // as packet GUID
	  if ( pLevel2->m_cmdArg1 ) {
	    memcpy( pEvent->GUID, pClient->m_GUID, 16 );
	  }
						
	  // Statistics
	  pClient->m_statistics.cntTransmitData += pEvent->sizeData;
	  pClient->m_statistics.cntTransmitFrames++;

	  // We use the message id as the sort key for the message
	  // in the queue. The high part of this id is the priority
	  // for the message.
	  // The non-id part should possibly be nulled but this depends
	  // on if we regard the message as "sent" or not. If not, a 
	  // message with a lower id should be transfered first over the 
	  // bus.
								
	  pNode->Key = pEvent->head;
	  pNode->pKey = &pNode->Key;
	  pNode->pObject = pEvent;
								
	  // There must be room in the send queue
	  if ( pctrlObject->m_maxItemsInSendQueue > 
	       pctrlObject->m_sendLevel2Queue.nCount ) {						
	    pctrlObject->m_wxQ2OutMutex.Lock();
							
	    if ( !dll_addNode( &pctrlObject->m_sendLevel2Queue, pNode ) ) {
	      pLevel2->m_Response = CANAL_RESPONSE_ERROR;	
	      pLevel2->m_ResponseCode = CANAL_IFERROR_SEND_STORAGE;
	    }
	    else {
	      pLevel2->m_Response = CANAL_RESPONSE_SUCCESS; 
	    }

	    pctrlObject->m_wxQ2OutMutex.Lock();

	  }
	  else {
						
	    pLevel2->m_Response = CANAL_RESPONSE_ERROR;	
	    pLevel2->m_ResponseCode = CANAL_IFERROR_BUFFER_FULL;
									
	    if ( NULL != pNode ) {
	      if ( NULL != pNode->pObject ) {
		if ( NULL != pEvent->pdata )  delete [] pEvent->pdata;
		delete pEvent;
	      }
	      delete pNode;
	    }
							
	  }
	}
	else {
								
	  delete pNode;
								
	  pLevel2->m_Response = CANAL_RESPONSE_ERROR;
	  pLevel2->m_ResponseCode = CANAL_IFERROR_SEND_MSG_ALLOCATON;
						
	}
      } // dllNode
					  
      break;	
				
				
      // * * * *  R E C E I V E  * * * *
    case VSCP_COMMAND_RECEIVE:
				
      if ( !pClient->m_bOpen ) {
	pLevel2->m_Response = CANAL_RESPONSE_ERROR;
	pLevel2->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
      }
      else {
 	
	pctrlObject->m_wxClientMutex.Lock();
								
	dllnode *pNode; 
	vscpEvent *pEvent = (vscpEvent *)pEventArea;
										 
	if ( NULL != ( pNode = pClient->m_inputQueue.pHead ) ) {
						
	  // There is a message available
	  pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;	
						
	  // Copy message
	  memcpy( pEvent, pNode->pObject, sizeof( vscpEvent ) );
						
	  // If there is data it also must be copied
	  if ( ( pEvent->sizeData > 0 ) && ( NULL != pEvent->pdata ) ) {
	    memcpy( pDataArea, pEvent->pdata, pEvent->sizeData );							
	  }
						
	  // Remove the old data
	  if ( NULL != pEvent->pdata ) delete pEvent->pdata;
	  pEvent->pdata = NULL;	 // Data stored in message

	  // Remove the node
	  dll_removeNode( &pClient->m_inputQueue, pNode );

	}
				 		
	pctrlObject->m_wxClientMutex.Unlock();
 		
      }				 
      break;	
				
				
      // * * * *  D A T A  A V A I L A B L E  * * * *
    case VSCP_COMMAND_CHECKDATA:

      if ( !pClient->m_bOpen ) {
	pLevel2->m_Response = CANAL_RESPONSE_ERROR;
	pLevel2->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
      }
      else {
	pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;

	unsigned int *pnMsg = (unsigned int *)pDataArea;
	//*pnMsg = dll_getNodeCount( &pClient->m_inputQueue ); 
	*pnMsg = pClient->m_inputQueue.nCount;
      }
      break;	
			
			
      // * * * *  S T A T U S  * * * *
    case VSCP_COMMAND_STATUS:

      if ( !pClient->m_bOpen ) {
	pLevel2->m_Response = CANAL_RESPONSE_ERROR;
	pLevel2->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
      }
      else {
	pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;

	// Status
	memcpy( pDataArea, &pClient->m_status, sizeof( canalStatus ) );
			
      } 
      break;	
				
				
      // * * * *  S T A T I S T I C S  * * * *
    case VSCP_COMMAND_STATISTICS:

      if ( !pClient->m_bOpen ) {
	pLevel2->m_Response = CANAL_RESPONSE_ERROR;
	pLevel2->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
      }
      else {
	memcpy( pDataArea, 
		&pClient->m_statistics, 
		sizeof( &pClient->m_statistics ) );
	pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;
      } 
      break;
				
				
      // * * * *  F I L T E R  * * * *
    case VSCP_COMMAND_FILTER:

      if ( !pClient->m_bOpen ) {
	pLevel2->m_Response = CANAL_RESPONSE_ERROR;
	pLevel2->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
      }
      else {
						
	// Set filter
	memcpy( &pClient->m_filter, pDataArea, sizeof( vscpEventFilter ) );
	pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;

      }  
      break;	
				
			
      // * * * *  V E R S I O N  * * * *
    case VSCP_COMMAND_VERSION:
      {	
	// Get version
	char *p = (char *)pDataArea;
	*( p + POS_VSCPD_MAJOR_VERSION ) = CANAL_MAIN_VERSION;
	*( p + POS_VSCPD_MINOR_VERSION ) = CANAL_MINOR_VERSION;
	*( p + POS_VSCPD_SUB_VERSION ) = CANAL_SUB_VERSION;
      }		
      pLevel2->m_Response = CANAL_RESPONSE_SUCCESS; 	
      break;
				
				
      // * * * *  G E T  I / F  G U I D   * * * *
    case VSCP_COMMAND_GET_GUID:				
      memcpy( pDataArea, &pClient->m_GUID, 16 ); 
      pLevel2->m_Response = CANAL_RESPONSE_SUCCESS; 
      break;
				
				
      // * * * *  S E T  I / F  G U I D   * * * *
    case VSCP_COMMAND_SET_GUID:			
      memcpy( &pClient->m_GUID, pDataArea, 16 );
      pLevel2->m_Response = CANAL_RESPONSE_SUCCESS; 
      break;	
				
				
      // * * * *  G E T  C H A N N E L  I D   * * * *
    case VSCP_COMMAND_GET_CHID:
				
      break;	
				
				
      // * * * *  N O O P  * * * *
    case VSCP_COMMAND_NOOP:
			
      pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;
      break;


    default:
			
      pLevel2->m_Response = CANAL_RESPONSE_ERROR;
      pLevel2->m_ResponseCode = CANAL_IFERROR_UNKNOWN_COMMAND;
      break;			
		
    } // case

    // Done - Ready for new command	
    //wxLogDebug("Client Thread Level 2: Before Done");
    sb.sem_num = CLIENT_DONE_SEM;
    sb.sem_op = 1;
    sb.sem_flg = 0;
    semop( semCmd, &sb, 1 );
    //wxLogDebug("Client Thread Level 2: After Done");
				 
  } // while running
	 
	// Remove messages in the client queues
  	
  pctrlObject->m_wxClientMutex.Lock();
  pctrlObject->removeClient( pClient );
  pctrlObject->m_wxClientMutex.Unlock();

  // Release shared memory
  shmdt( (void *)m_pCommandMem );
	
  // Remove the share memory
  if( -1 == shmctl( clientInfo.m_shmid, IPC_RMID, 0 ) ) {
    ::wxLogError( _("ClientThread2: Failed to remove shared memory.\n") );
  }
	
  // remove the semaphore
  semctl( semCmd, 0, IPC_RMID, NULL );
	
  delete (_clientstruct *)pThreadObject;

  pthread_exit( &rv );
 
}

 

