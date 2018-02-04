// File: hashtable.h
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
