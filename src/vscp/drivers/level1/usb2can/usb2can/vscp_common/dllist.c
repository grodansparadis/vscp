// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2008 Ake Hedman, D of Scandinavia, <akhe@eurosource.se>
// Copyright (C) 2005-2012 Gediminas Simanskis,8devices,<gediminas@8devices.com>
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

#define _POSIX

#include "sal.h"

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <time.h>
#endif
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "dllist.h"

///////////////////////////////////////////////////////////////////////////////
// init_list

void dll_init( struct DoubleLinkedList *pdll, unsigned char nSort )
{
  pdll->pHead = NULL;
  pdll->pTail = NULL;
  pdll->nSortOrder = nSort;
  pdll->nCount = 0;
};

///////////////////////////////////////////////////////////////////////////////
// addNodeFront  -  AKHE

BOOL dll_addNodeHead( struct DoubleLinkedList *pdll, struct dllnode *pInsertNode )
{
  if ( NULL == pdll ) return FALSE;
  if ( NULL == pInsertNode ) return FALSE;


  pInsertNode->pNext =  pdll->pHead;
  pInsertNode->pPrev = NULL;

  // if there already where nodes in the list let
  // previous node point at this one
  if ( NULL != pdll->pHead ) {
    pdll->pHead->pPrev = pInsertNode;
  }

  // If this is the first node - GS
  if ( NULL == pdll->pTail ) {
    pdll->pTail = pInsertNode;
  }

  // The Head always point at the first node
  pdll->pHead = pInsertNode;


  // Calculate the number of elements in the list
  dll_getNodeCount( pdll );

  return TRUE;
};

///////////////////////////////////////////////////////////////////////////////
// addNodeTail 

BOOL dll_addNodeTail( struct DoubleLinkedList *pdll, struct dllnode *pInsertNode )
{
  if ( NULL == pdll ) return FALSE;
  if ( NULL == pInsertNode ) return FALSE;
  
  pInsertNode->pNext = NULL; 
  pInsertNode->pPrev = pdll->pTail;
 	
  // if there already where nodes in the list let
  // previous node point at this one
  if ( NULL != pdll->pTail ) {
    pdll->pTail->pNext =  pInsertNode;
  }
  
  // If this is the first node
  if ( NULL == pdll->pHead ) {
    pdll->pHead = pInsertNode;	
  }
  
  // The tail always point at the end
  pdll->pTail = pInsertNode;

  // Calculate the number of elements in the list
  dll_getNodeCount( pdll );

  return TRUE;
};

///////////////////////////////////////////////////////////////////////////////
// addNodeBefore  -  AKHE

BOOL dll_addNodeBefore( struct DoubleLinkedList *pdll, 
                          struct dllnode *pNode,
                          struct dllnode *pInsertNode )
{
  if ( NULL == pdll ) return FALSE;
  if ( NULL == pNode ) return FALSE;
  if ( NULL == pInsertNode ) return FALSE;
  
  if ( NULL == pNode->pPrev ) {
    // There is no nodes before this node
    dll_addNodeHead( pdll, pInsertNode );
  }
  else {
    // Add the node between the two
    dll_insertNode( pdll, pNode, pInsertNode );		 
  }

  // Calculate the number of elements in the list
  dll_getNodeCount( pdll );

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// addNodeAfter	
 
BOOL dll_addNodeAfter( struct DoubleLinkedList *pdll, 
		       struct dllnode *pNode,
		       struct dllnode *pInsertNode )
{
  if ( NULL == pdll ) return FALSE;
  if ( NULL == pNode ) return FALSE;
  if ( NULL == pInsertNode ) return FALSE;
	
  if ( NULL == pNode->pNext ) {
    // There is no nodes after this one
    dll_addNodeTail( pdll, pInsertNode ); 	
  }
  else {
    // Add the node between the two
    dll_insertNode( pdll, pNode, pInsertNode );		 
  }

  // Calculate the number of elements in the list
  dll_getNodeCount( pdll );

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// insertNode

BOOL dll_insertNode( struct DoubleLinkedList *pdll, 
                      struct dllnode *pNode, 
                      struct dllnode *pInsertNode )
{
  //struct dllnode *pNode = malloc( sizeof( struct dllnode ) );
  if ( NULL == pdll ) return FALSE;
  if ( NULL == pNode ) return FALSE;
  if ( NULL == pInsertNode ) return FALSE;
	
  // First link in the new node
  pInsertNode->pPrev = pNode;
  pInsertNode->pNext = pNode->pNext;
	
  // Fix up the node before
  pNode->pNext =  pInsertNode;
	
  // Fix up the node after
  pNode->pNext->pPrev = pInsertNode;

  // Calculate the number of elements in the list
  dll_getNodeCount( pdll );

  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// addNode 
//

BOOL dll_addNode( struct DoubleLinkedList *pdll,  struct dllnode *pInsertNode )
{
  int nsearchResult = 0;
  struct dllnode *pNode;
	

  if ( NULL == pdll ) return FALSE;  
	
  if ( pdll->nSortOrder == SORT_NONE ) {
    // No sorting just add as last element
    dll_addNodeTail( pdll, pInsertNode );
    return TRUE;
  }
  
  if ( ( pdll->nSortOrder == SORT_STRING ) && 
       ( NULL == pInsertNode->pstrKey )  ) {
    return FALSE;
  }
  
  if ( ( pdll->nSortOrder == SORT_NUMERIC ) && 
       ( NULL == pInsertNode->pKey )  ) {
    return FALSE;
  }


  // Get Head node
  pNode = pdll->pHead;


  if ( NULL == pNode ) {
    // Add to tail - last id
    return dll_addNodeTail( pdll, pInsertNode );
  }
    
  // Search for a place to insert the new node

  while (pNode != NULL ) {

    if ( ( pdll->nSortOrder == SORT_STRING ) && 
	 ( NULL != pInsertNode->pstrKey ) ) {
      nsearchResult =  strcmp( pInsertNode->pstrKey, pNode->pstrKey );
    }
    else if ( ( pdll->nSortOrder == SORT_NUMERIC ) ) {
		
      if ( *pInsertNode->pKey == *pNode->pKey ) {
	nsearchResult = 0;
      }
      else if ( *pInsertNode->pKey > *pNode->pKey ) {
	nsearchResult = 2;
      }
      else {
	nsearchResult = -1;
      }
    }
      
    if ( 0 == nsearchResult ) {
      // Add after current item
      return dll_addNodeAfter( pdll, pNode, pInsertNode );
    }
    else if ( nsearchResult > 0 ) {
      // Add before current item
      return dll_addNodeBefore( pdll, pNode, pInsertNode );
    }
		
    pNode = pNode->pNext;
  }
  
  // If not found it should be added at the end
  return dll_addNodeTail( pdll, pInsertNode );
}



///////////////////////////////////////////////////////////////////////////////
// removeNode

BOOL dll_removeNode( struct DoubleLinkedList *pdll, struct dllnode *pNode )
{
  struct dllnode *pNext;
  struct dllnode *pPrev;

  if ( NULL == pdll ) return FALSE;
  if ( NULL == pNode ) return FALSE;
  
  if ( NULL != pNode ) {
    
    // Remove the object
    if ( NULL != pNode->pObject ) {
      free( pNode->pObject ); 
      pNode->pObject = NULL;
    }
    
    // Save links
    pNext = pNode->pNext;
    pPrev = pNode->pPrev;
    
    if ( NULL != pPrev ) {
      pPrev->pNext = pNext;
    }
    else {
      pdll->pHead = pNext;	
    }
		
    if ( NULL != pNext ) {
      pNext->pPrev =  pPrev;
    }
    else {
      pdll->pTail = pPrev;	
    }	
    
    // Remove the node
    free( pNode );
    pdll->nCount--;
  }

  // Calculate the number of elements in the list
  dll_getNodeCount( pdll );

  return TRUE;

};

///////////////////////////////////////////////////////////////////////////////
// getNodeCount

unsigned long dll_getNodeCount( struct DoubleLinkedList *pdll )
{
  uint32_t cnt = 0;
  struct dllnode *pNode;

  if ( NULL == pdll ) return FALSE;

  pNode = pdll->pHead;

  // If there is no head there is no data
  if ( NULL == pNode ) return 0;
  
  while ( pNode != NULL ) {
    cnt++;
    pNode = pNode->pNext;
  }

  // Store the count
  pdll->nCount = cnt;

  return cnt;		
};


///////////////////////////////////////////////////////////////////////////////
// removeAllNodes

BOOL dll_removeAllNodes( struct DoubleLinkedList *pdll )
{	
  if ( NULL == pdll ) return FALSE;

  while ( NULL != pdll->pHead ) {
    dll_removeNode( pdll, pdll->pTail );
  }	
  
  dll_init( pdll, 0 );

  pdll->nCount = 0;

  return TRUE;
};


///////////////////////////////////////////////////////////////////////////////
// findNodeFromID

struct dllnode *dll_findNodeFromID( struct DoubleLinkedList *pdll, 
                                      unsigned long nID )
{
  struct dllnode *pRV = NULL;
  struct dllnode *pNode;

  if ( NULL == pdll ) return FALSE;

  pNode = pdll->pHead;

  while ( pNode != NULL ) {
    if ( nID == *pNode->pKey ) {
      pRV = pNode;		
      break;
    }    
    pNode = pNode->pNext;
  }
  
  return pRV;
}

///////////////////////////////////////////////////////////////////////////////
// findNodeFromString

struct dllnode *dll_findNodeFromString( struct DoubleLinkedList *pdll, 
					char * strID )
{
  struct dllnode *pRV = NULL;
  struct dllnode *pNode;

  if ( NULL == pdll ) return FALSE;

  pNode = pdll->pHead;

  while ( pNode != NULL ) {
    if ( 0 == strcmp( strID, pNode->pstrKey ) ) {
      pRV = pNode;	
      break;
    }
    
    pNode = pNode->pNext;
  }
  
  return pRV;
}
