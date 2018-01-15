/*
  Name:     node.h
*/
// 
// the EDA deamon.
//
// Copyright (C) 2000-2018 Grodans Paradis AB, akhe@grodansparadis.com
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
// $RCSfile: node.h,v $                                       
// $Date: 2005/08/30 11:00:13 $                                  
// $Author: akhe $                                              
// $Revision: 1.33 $ 
//
// akhe@eurosource.se  000829  started this project-file.
//
//
// $RCSfile: node.h,v $                                       
// $Date: 2005/08/30 11:00:13 $                                  
// $Author: akhe $                                              
// $Revision: 1.33 $  
//
//

#ifndef H_NODE_H
#define H_NODE_H

#ifndef WIN32
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#else
#include <time.h>
#endif
#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/types.h>



/**
   The base class to derive from for object storage
*/

class CNodeObject
{

 public:
    
  /**
     Constructor
  */
  CNodeObject();
    
  /**
     destructor
  */
  ~CNodeObject();
    
 public:

  /**
     String key used for alphanumeric sort
  */
  char * m_pstrSortKey;	
  
  /**
     Numeric key used for numeric sort
  */
  __u32 * m_pKey;
  
};

/**
   A class to handle a node for the double linked list
*/

class CNode
{
  
 public:
  
  /**
     Constructor
  */
  CNode( void );
    
  /**
     Destructor
  */
  virtual ~CNode( void );
    
  /**
     Get object embedded in node.
     
     @return A pointer to the object or NULL if the is 
     no object.
  */
  CNodeObject * getObject( void ) { return m_pObject; };
  
  /**
     Set object for a node.
     
     @param Pointer to object.
  */
  void setObject( CNodeObject * pObject ) { m_pObject = pObject; };
  
  /**
     Set the link to the following node
     
     @param Next node object.
  */
  void setNextNode( CNode * p ) { m_pNext = p; };
  
  /**
     Set the link to the previous node
     
     @param Previous node object.
  */
  void setPrevNode( CNode * p ) { m_pPrev = p; };
  
  /**
     Get the next node.
     
     @return Next node.
  */
  CNode * getNextNode( void ) { return m_pNext; };
  
  /**
     Get the previous node.
     
     @return Previous node.
  */
  CNode * getPrevNode( void ) { return m_pPrev; };
  
 protected:
  
  /**
     A pointer to the embedded node object
  */
  CNodeObject * m_pObject;
  
  /**
     Pointer to the next node
  */
  CNode * m_pNext;
  
  
  /**
     Pointer to the previous node
  */
  CNode * m_pPrev;
  
 private:

};

/**
   A generic double linkled list
*/

class CDoubleLinkedList 
{
  
 public:
  
  /**
     Constructor
  */
  CDoubleLinkedList( unsigned char nSort = SORT_NONE );
  
  /**
     Destructor
  */
  virtual ~CDoubleLinkedList();
  
  /**
     Sorttypes for the list
  */
  enum _sorttype { SORT_NONE, SORT_STRING, SORT_NUMERIC };
  
  /**
     Add a node to the front of the linked list.
  */
  void addNodeHead( CNodeObject * pObject );
  
  /**
     Add a node to the end of the linked list.
  */
  void addNodeTail( CNodeObject * pObject );
  
  
  /**
     Add a node before another node
  */
  void addNodeBefore( CNode * pNode, CNodeObject * pObject );
  
  /**
     Add a node after another node.
  */
  void addNodeAfter( CNode * pNode, CNodeObject * pObject );
  
  
  /**
     Add a node sorted by the current sort order 
  */					
  bool addNode( CNodeObject * pObject );
  
  
  /**
     Get the node count.
  */
  unsigned int getNodeCount( void );
  
  /**
     Remove all nodes form the linked list.
  */									
  void removeAllNodes( void );
  
  /**
     Remove a specific node form the linked list
  */
  void removeNode( CNode * pNode );		
  
  /**
     Get the first node from the linked list.
  */							
  CNode * getHeadNode( void ) { return m_pHead; };
  /**
     Get the last node from the linked list.
  */
  CNode * getTailNode( void ) { return m_pTail; };	
  
  /**
     Get the next nod in the list.
  */
  CNode * getNextNode( void ) { return ( m_pTail->getNextNode() ); };
  /**
     Get the previous node from the list.
  */
  CNode * getPrevNode( void ) { return ( m_pTail->getPrevNode() ); };
  
  /**
     Get node from its numerical key
     
     @param Numerical key for node.
     @return Found Object
     @return NULL if no match found.
  */
  CNode * findNode( unsigned long nID );
  
  /**
     Get node from its string key
     
     @param String key for node.
     @return Found Object
     @return NULL if no match found.
  */
  CNode * findNode( char * strID );
  
  
 protected:
  
  /**
     Insert a node and an object between two other nodes 
  */
  void insertNode( CNode * pNode1, 
                        CNode * pNode2,
                        CNodeObject * pObject );
  
 protected:
  /**
     Sort order
     
     0 - not sorted
     1 - sort on string key.
     2 - sort on numeric key.
     
     Note that items can be retrived in accending/decending order by 
     retriving them from the head or from the tail  of the list.
  */	
  unsigned char m_nSort;
  
 private:	
  
  /**
     Initialize the linked list
  */
  void Init( unsigned char nSort = SORT_NONE  );
  
  /**
     Pointer to the head of the linked list
  */
  CNode * m_pHead;
  
  /**
     Pointer to the tail of the linked list
  */
  CNode * m_pTail;
    
};

#endif // H_NODE_H
