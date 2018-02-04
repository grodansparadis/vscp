//
// node.cpp	
// the EDA deamon.
//
// Copyright (C) 2000-2018 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
// $RCSfile: node.cpp,v $                                       
// $Date: 2005/08/30 11:00:13 $                                  
// $Author: akhe $                                              
// $Revision: 1.33 $ 
//
// akhe@eurosource.se  000829  started this project-file.
//
// $RCSfile: node.cpp,v $                                       
// $Date: 2005/08/30 11:00:13 $                                  
// $Author: akhe $                                              
// $Revision: 1.33 $  
//
//

#define _POSIX
typedef unsigned long u_long;

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
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
#include "edawin32.h"
#include <time.h>
#endif
#include <sys/types.h>
#include <signal.h>

#include <errno.h>
#include <signal.h>
#include "eda.h"
#include "configfile.h"
#include "node.h"

// * * * CNodeObject * * *

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CNodeObject::CNodeObject()
{
  // Nothing has been assigned yet
  m_pstrSortKey = NULL;
  m_pKey = NULL;
}


///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CNodeObject::~CNodeObject()
{
  if ( NULL != m_pstrSortKey ) {
    delete m_pstrSortKey;
  }
  
  if ( NULL != m_pKey ) {
    delete m_pKey;	
  }	
}
                            
// * * * CNode * * *


///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CNode::CNode()
{
  m_pObject = NULL;
  m_pNext = NULL;
  m_pPrev = NULL;
};


///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CNode::~CNode()
{
  if ( NULL != m_pObject ) {
    delete m_pObject;
  }	
};


// * * * CDoubleLinkedList * * *

///////////////////////////////////////////////////////////////////////////////
// CDoubleLinkedList

CDoubleLinkedList::CDoubleLinkedList( unsigned char nSort )
{
  Init( nSort );
};

///////////////////////////////////////////////////////////////////////////////
// ~CDoubleLinkedList

CDoubleLinkedList::~CDoubleLinkedList()
{	 
  removeAllNodes();
};

///////////////////////////////////////////////////////////////////////////////
// Init

void CDoubleLinkedList::Init( unsigned char nSort )
{
  m_pHead = NULL;
  m_pTail = NULL;
  m_nSort = nSort;
};

///////////////////////////////////////////////////////////////////////////////
// addNodeFront

void CDoubleLinkedList::addNodeHead( CNodeObject * pObject )
{
  CNode * pNode = new CNode();
 
  pNode->setNextNode( m_pHead );
  pNode->setPrevNode( NULL );


    
  // if there already where nodes in the list let
  // previous node point at this one
  if ( NULL != m_pHead ) {
    m_pHead->setPrevNode( pNode );
  }
 
  // The tail always point at the first node
  m_pHead = pNode;
  pNode->setObject( pObject );
};

///////////////////////////////////////////////////////////////////////////////
// addNodeTail

void CDoubleLinkedList::addNodeTail( CNodeObject * pObject )
{
  CNode * pNode = new CNode();
 
  pNode->setNextNode( NULL );
  pNode->setPrevNode( m_pTail );
    
  // if there already where nodes in the list let
  // previous node point at this one
  if ( NULL != m_pTail ) {
    m_pTail->setNextNode( pNode );
  }
  
  // If this is the first node
  if ( NULL == m_pHead ) {
    m_pHead = pNode;	
  }
  
  // The tail always point at the end
  m_pTail = pNode;
  pNode->setObject( pObject );
};

///////////////////////////////////////////////////////////////////////////////
// addNodeBefore

void CDoubleLinkedList::addNodeBefore( CNode * pNode2, CNodeObject * pObject )
{
  CNode * pNode1 = pNode2->getPrevNode();
    
  if ( NULL == pNode1 ) {
    // There is no nodes before this node
    addNodeHead( pObject ); 	
  }
  else {
    // Add the node between the two
    insertNode( pNode1, pNode2, pObject );		 
  }
}

///////////////////////////////////////////////////////////////////////////////
// addNodeAfter	
 
void CDoubleLinkedList::addNodeAfter( CNode * pNode1, CNodeObject * pObject )
{
  CNode * pNode2 = pNode1->getNextNode();
    
  if ( NULL == pNode2 ) {
    // There is no nodes after this one
    addNodeTail( pObject ); 	
  }
  else {
    // Add the node between the two
    insertNode( pNode1, pNode2, pObject );		 
  }
}

///////////////////////////////////////////////////////////////////////////////
// addNode

bool CDoubleLinkedList::addNode( CNodeObject * pObject )
{
  int nsearchResult = 0;
  CNode * pNode = m_pHead;
    
  if ( m_nSort == SORT_NONE ) {
    // No sorting just add as last element
    addNodeTail( pObject );
    
    return true;
  }
  
  if ( ( m_nSort == SORT_STRING ) && ( NULL == pObject->m_pstrSortKey )  ) {
    return false;
  }
  
  if ( ( m_nSort == SORT_NUMERIC ) && ( NULL == pObject->m_pKey )  ) {
    return false;
  }
  
  if ( NULL != pNode ) {
    
    while (pNode != NULL ) {
      if ( ( m_nSort == SORT_STRING ) && ( NULL != pObject->m_pstrSortKey ) ) {
    nsearchResult =  strcmp( pNode->getObject()->m_pstrSortKey,
                 pObject->m_pstrSortKey );
      }
      else if ( ( m_nSort == SORT_NUMERIC ) ) {
    if ( *pObject->m_pKey == *pNode->getObject()->m_pKey ) {
      nsearchResult = 0;
    }
    else if ( *pObject->m_pKey > *pNode->getObject()->m_pKey ) {
      nsearchResult = 2;
    }
    else {
      nsearchResult = -1;
    }
      }
      
      if ( 0 == nsearchResult ) {
    // Entry already in table - Not allowed
    return false;
      }
      else if ( nsearchResult > 0 ) {
    // Add before current item
    addNodeBefore( pNode, pObject );
    return true;
      }
        
      pNode = pNode->getNextNode();
    }
  }
  else {	
    // Add to tail - last id
    addNodeTail( pObject );
  }
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// insertNode

void CDoubleLinkedList::insertNode( CNode * pNode1, 
                    CNode * pNode2,
                    CNodeObject * pObject )
{
  CNode * pNode = new CNode();
  pNode->setObject( pObject );
    
  // First link in the new node
  pNode->setPrevNode( pNode2 );
  pNode->setNextNode( pNode1 );
    
  // Fix up the node before
  pNode1->setNextNode( pNode );
    
  // Fix up the node after
  pNode2->setPrevNode( pNode );
}

///////////////////////////////////////////////////////////////////////////////
// removeNode

void CDoubleLinkedList::removeNode( CNode * pNode )
{
  if ( NULL != pNode ) {
    // Remove the object
    CNodeObject * pObject = pNode->getObject();
    if ( NULL != pObject ) {
      delete pObject; 
      pNode->setObject( NULL );
    }
    
    CNode * m_pNext = pNode->getNextNode();
    CNode * m_pPrev = pNode->getPrevNode();
    
    if ( NULL != m_pPrev ) {
      m_pPrev->setNextNode( m_pNext );
    }
    else {
      m_pHead = m_pNext;	
    }
        
    if ( NULL != m_pNext ) {
      m_pNext->setPrevNode( m_pPrev );
    }
    else {
      m_pTail = m_pPrev;	
    }	
    
    // Remove the node
    delete pNode;
  }
};

///////////////////////////////////////////////////////////////////////////////
// getNodeCount

unsigned int CDoubleLinkedList::getNodeCount()
{
  unsigned int cnt = 0;
  CNode * pNode = m_pHead;
  
  while (pNode != NULL ) {
    cnt++;
    pNode = pNode->getNextNode();
  }
  return cnt;		
};


///////////////////////////////////////////////////////////////////////////////
// removeAllNodes

void CDoubleLinkedList::removeAllNodes()
{	
  while ( NULL != m_pHead ) {
    removeNode( m_pTail );
  }	
  
  Init();
};

///////////////////////////////////////////////////////////////////////////////
// findNode

CNode * CDoubleLinkedList::findNode( unsigned long  nID )
{
  CNode * pRV = NULL;
  CNode * pNode = m_pHead;
    
  while (pNode != NULL ) {
    if ( nID == *pNode->getObject()->m_pKey ) {
      pRV = pNode;		
      break;
    }
    
    
    pNode = pNode->getNextNode();
  }
  
  return pRV;
}

///////////////////////////////////////////////////////////////////////////////
// findNode

CNode * CDoubleLinkedList::findNode( char * strID )
{
  CNode * pRV = NULL;
  CNode * pNode = m_pHead;
    
  while (pNode != NULL ) {
    if ( 0 == strcmp( strID, pNode->getObject()->m_pstrSortKey ) ) {
      pRV = pNode;	
      break;
    }
    
    pNode = pNode->getNextNode();
  }
  
  return pRV;
}
