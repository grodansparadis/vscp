// File: hashtable.c
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <stdlib.h>
#include "dllist.h"
#include "hashtable.h"

//////////////////////////////////////////////////////////////////////////////
// initHashTable

void hash_init( struct HashTable *pht )
{
  int i;

  // Create the double linked lists
  for ( i=0; i<HASH_TABLE_SIZE_PRIME; i++ ) {
    pht->hashtbl[ i ] = 
      (struct DoubleLinkedList *)malloc( sizeof( struct DoubleLinkedList ) );
    pht->hashtbl[ i ]->nSortOrder = SORT_STRING;
  }

  // Create the id list
  pht->pidlist = 
    (struct DoubleLinkedList *)malloc( sizeof( struct DoubleLinkedList ) );
  pht->pidlist->nSortOrder = SORT_NUMERIC;
}

//////////////////////////////////////////////////////////////////////////////
// cleanupHashTable

void hash_cleanup( struct HashTable *pht )
{
  int i;

  // Remove all nodes	
  hash_removeAllNodes( pht );

  // Delete all elements
  for ( i=0; i<HASH_TABLE_SIZE_PRIME; i++ ) {
    if ( NULL != pht->hashtbl[ i ] ) {
      free(  pht->hashtbl[ i ] );
    }
  }

  if ( NULL != pht->pidlist ) {
    free(  pht->pidlist );
  }
}

//////////////////////////////////////////////////////////////////////////////
// hash

unsigned short hash( struct HashTable *pht, char * p )
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

struct DoubleLinkedList *hash_getListfromHash(  struct HashTable *pht, char *p )
{
  return pht->hashtbl[ hash( pht, p ) ];
}

//////////////////////////////////////////////////////////////////////////////
// addNode

BOOL hash_addNode(  struct HashTable *pht, struct dllnode *pNode )
{
  // Must be valid object
  if ( NULL == pNode ) return FALSE;
    
  /*if ( NULL != dll_findNodeFromString( pNode ) ) {
    // Node is already there
    return FALSE;
  }
  else {
    // Add the node
    if ( pht->hashtbl[ hash( pNode->m_pstrSortKey ) ]->addNode( pNode ) ) {

      // Object Added - also add to numerical id list
      m_pidlist->sll_addNode( pNode );	
    }
    else {
      return FALSE;
    }
    }*/

  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// hash_findNode

struct Node *hash_findNode( struct HashTable *pht, struct node *pNode )
{
  return NULL;
}

//////////////////////////////////////////////////////////////////////////////
// findNode - find node form numeric id

struct Node *hash_findNodeFromNumericID(  struct HashTable *pht,
                     unsigned long nID )
{	
  //return pht->pidlist->dll_findNodeFromID( nID );
  return NULL;
}
    
//////////////////////////////////////////////////////////////////////////////
// findNode -  find node form string ID

struct node *hash_findNodeFromStringID( struct HashTable *pht, char *strID )
{
  //return pht->hashtbl[ hash( strID ) ]->dll_findNodeFromString( strID );
  return NULL;
}

//////////////////////////////////////////////////////////////////////////////
// removeNode

BOOL hash_removeNode( struct HashTable *pht, struct dllnode *pNode )
{
  // Must have something to work on
  if ( NULL == pNode ) return FALSE;
    
  // Get and remove the object
  if ( NULL != pNode->pObject ) {
    free( pNode->pObject );
  }

  return hash_removeNode( pht, pNode );
}

//////////////////////////////////////////////////////////////////////////////
// removeNodeWithNumericID

BOOL hash_removeNodeWithNumericID( struct HashTable *pht, unsigned long nID )
{
  //return hash_removeNodeFromID( hash_findNode( pht, nID ) );
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
// removeNodeWithStringID

BOOL hash_removeNodeWithStringID( struct HashTable *pht, char * strID )
{
  struct node *pNode = hash_findNodeFromStringID( pht, strID );

  // Must have something to work on
  if ( NULL == pNode ) return FALSE;

  /*  if ( NULL != pNode->pObject ) {
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
  */
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// removeAllNodes

void hash_removeAllNodes( struct HashTable *pht )
{
  //struct node *pNode;
  int i;
    
  // Remove the numerical id double linked list
  if ( NULL != pht->pidlist ) {

    // Assign a NULL pointer to  all node objects
    // so the object can be removed in the hash routines
    //while ( NULL != ( pNode = dll_getHeadNode( pht->pidlist ) ) ) {
    //  pNode->pObject = NULL;
    //  dll_removeNode( pht->pidlist, pNode );
    //
    //}

    // And finally remove all nodes
    dll_removeAllNodes( pht->pidlist );
  }

  // Remove the hash list for string ID's
  for ( i=0; i<HASH_TABLE_SIZE_PRIME; i++ ) {
    //hash_removeAllNodes( &pht->hashtbl[ i ] );	
  }
}
