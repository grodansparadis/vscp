// File: hashtable.h
//
// Copyright (C) 2000-2016 Ake Hedman, Grodans Paradis AB, akhe@grodansparadis.com
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
// $RCSfile: hashtable.h,v $                                       
// $Date: 2005/08/30 11:00:13 $                                  
// $Author: akhe $                                              
// $Revision: 1.33 $ 
//
// akhe@eurosource.se  Sun Feb 3 2002  started this project-file.
//
// $RCSfile: hashtable.h,v $                                       
// $Date: 2005/08/30 11:00:13 $                                  
// $Author: akhe $                                              
// $Revision: 1.33 $  
//

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "dllist.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Number of double linked lists (must be a prime number).
#define	HASH_TABLE_SIZE_PRIME 	101

/**
   This file implements a hash table with elements consisting of
   double linked lists.

   @author Ake Hedman
*/

struct HashTable
{
  /**
     The hash yable with the double linked lists
  */
  struct DoubleLinkedList *hashtbl[ HASH_TABLE_SIZE_PRIME ];
  
  /**
     Double linked list with numerical id's
  */
  struct DoubleLinkedList *pidlist;
  
};

/**
   Calculate the hash value from a string
   
   @param p Pointer to string ID
   @return Hash position
*/
unsigned short hash_calc( struct HashTable *pht, char *p );	

/**
   Get double linked list from hash
   
   @param p Pointer to string ID
   @return Pointer to a double linked list where this string ID
   should be found. If not found its not in the table.
*/
struct DoubleLinkedList *hash_getListfromHash( struct HashTable *, char * );

/**
   Add a node. A generic NodeObject is also created
   
   @param nID Numerical ID
   @param strID String ID
   @return True on success, false on failure.
*/
//bool hash_addNode( struct *pHashTable, unsigned long nID, char * strID );

/**
   Add a node
   
   @param pObject An object derived from a CNodeObject to add.
   @return True on success, false on failure.
*/
BOOL hash_addNode( struct HashTable *, struct dllnode * );

/**
   Remove a node
   
   @param pNode An object derived from a CNodeObject to remove.
   @return True on success, false on failure.
*/
BOOL hash_removeNode( struct HashTable *, struct dllnode * );

/**
   Remove a node
   
   @param nID Numerical ID for node to remove.
   @return True on success, false on failure.
*/
BOOL hash_removeNodeWithNumericID( struct HashTable *, unsigned long nID );

/**
   Remove a node
   
   @param strID String ID for node to remove.
   @return True on success, false on failure.
*/
BOOL hash_removeNodeWithStringID( struct HashTable *, char * );

/**
   Remove all nodes from the two tables-
*/
void hash_removeAllNodes( struct HashTable * );

struct Node *hash_findNode( struct HashTable *, struct node *);

/**
   Find a node from its numeric ID
   
   @param nID Numerical ID
   @return NULL if not found or pointer to the node if found.
*/
struct node *hash_findNodeFromID( struct HashTable *, unsigned long );

/**
   Find a node from its string ID
   
   @param strID String ID
   @return NULL if not found or pointer to the node if found.
*/	
struct node *hash_findNodeFromStringID( struct HashTable *, char * );

#ifdef __cplusplus
	}
#endif

#endif
