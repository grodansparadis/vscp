// File: hashtable.cpp
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
// $RCSfile: hashtable.cpp,v $                                       
// $Date: 2005/08/30 11:00:13 $                                  
// $Author: akhe $                                              
// $Revision: 1.33 $ 
//
// akhe@eurosource.se  Sun Feb 3 2002  started this project-file.
//
// $RCSfile: hashtable.cpp,v $                                       
// $Date: 2005/08/30 11:00:13 $                                  
// $Author: akhe $                                              
// $Revision: 1.33 $  
//

#include "node.h"
#include "hashtable.h"

//////////////////////////////////////////////////////////////////////////////
// CHashTable

CHashTable::CHashTable()
{
  // Create the double linked lists
  for ( int i=0; i<HASH_TABLE_SIZE_PRIME; i++ ) {
    m_hashtbl[ i ] = new CDoubleLinkedList( CDoubleLinkedList::SORT_STRING );
  }

  // Create the id list
  m_pidlist = new CDoubleLinkedList( CDoubleLinkedList::SORT_NUMERIC );
}

//////////////////////////////////////////////////////////////////////////////
// ~CHashTable

CHashTable::~CHashTable()
{
  // Remove all nodes	
  removeAllNodes();

  // Delete all elements
  for ( int i=0; i<HASH_TABLE_SIZE_PRIME; i++ ) {
    if ( NULL != m_hashtbl[ i ] ) {
      delete m_hashtbl[ i ];
    }
  }

  if ( NULL != m_pidlist ) {
    delete m_pidlist;
  }
}

//////////////////////////////////////////////////////////////////////////////
// hash

unsigned short CHashTable::hash( char * p )
{
  int h = 0;
	
  while ( *p ) {
    h = ( (64*h + *p ) % HASH_TABLE_SIZE_PRIME );
    p++;
  }
  
  return h;
}

//////////////////////////////////////////////////////////////////////////////
// getListfromHash

CDoubleLinkedList *CHashTable::getListfromHash( char * p )
{
  return m_hashtbl[ hash( p ) ];
}

//////////////////////////////////////////////////////////////////////////////
// addNode

bool CHashTable::addNode( CNodeObject *pObject )
{
  // Must be valid object
  if ( NULL == pObject ) return false;
	
  if ( NULL != findNode( pObject ) ) {
    // Node is already there
    return false;
  }
  else {
    // Add the node
    if ( m_hashtbl[ hash( pObject->m_pstrSortKey ) ]->addNode( pObject ) ) {

      // Object Added - also add to numerical id list
      m_pidlist->addNode( pObject );	
    }
    else {
      return false;
    }
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////
// findNode - find node form numeric id

CNode *CHashTable::findNode( unsigned long nID )
{	
  return m_pidlist->findNode(nID );
}
	
//////////////////////////////////////////////////////////////////////////////
// findNode -  find node form string ID

CNode *CHashTable::findNode( char * strID )
{
  return m_hashtbl[ hash( strID ) ]->findNode( strID );
}

//////////////////////////////////////////////////////////////////////////////
// removeNode

bool CHashTable::removeNode( CNode *pNode )
{
  // Must have something to work on
  if ( NULL == pNode ) return false;
	
  // Get the object
  CNodeObject *pObject = pNode->getObject();

  return removeNode( pObject );
}

//////////////////////////////////////////////////////////////////////////////
// removeNode

bool CHashTable::removeNode( CNodeObject *pObject )
{
  // Must have something to work on
  if ( NULL == pObject ) return false;

  return removeNode( pObject->m_pstrSortKey );
}

//////////////////////////////////////////////////////////////////////////////
// removeNode

bool CHashTable::removeNode( unsigned long nID )
{
  return removeNode( findNode( nID ) );
}

//////////////////////////////////////////////////////////////////////////////
// removeNode

bool CHashTable::removeNode( char * strID )
{
  CNode *pNode = findNode( strID );

  // Must have something to work on
  if ( NULL == pNode ) return false;

  CNodeObject *pObject = pNode->getObject();
  if ( NULL != pObject ) {
    // Note that the node object should not be removed here as
    // this is the same object as in the hash list. The removal should
    // be done in the hash list instead. To prevent this we set
    // assign a NULL pointer for the  node object before we delete the
    // node.
    CNode *pNodeNumeric = m_pidlist->findNode( *pObject->m_pKey );		
    if ( NULL != pNodeNumeric ) {
      pNodeNumeric->setObject( NULL );
      // Now its safe to remove the node.	
      m_pidlist->removeNode( pNodeNumeric );
    }
  }

  // Remove the string id node and driver object
  m_hashtbl[ hash( strID ) ]->removeNode( pNode );	

  return true;
}

//////////////////////////////////////////////////////////////////////////////
// removeAllNodes

void CHashTable::removeAllNodes( void )
{
  CNode *pNode;
	
  // Remove the numerical id double linked list
  if ( NULL != m_pidlist ) {

    // Assign a NULL pointer to  all node objects
    // so the object can be removed in the hash routines
    while ( NULL != ( pNode = m_pidlist->getHeadNode() ) ) {
      pNode->setObject( NULL );
      m_pidlist->removeNode( pNode );
    }

    // And finally remove all nodes
    m_pidlist->removeAllNodes();
  }

  // Remove the hash list for string ID's
  for ( int i=0; i<HASH_TABLE_SIZE_PRIME; i++ ) {
    m_hashtbl[ i ]->removeAllNodes();	
  }
}
