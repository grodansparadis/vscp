// msglog.h: interface for the CMsgLog class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2018 Ake Hedman, Grodans Paradis AB, akhe@grodansparadis.com
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
// $RCSfile: log.h,v $                                       
// $Date: 2005/10/14 16:49:49 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 

#if !defined(AFX_LOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
#define AFX_LOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef WIN32

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#else

#define _POSIX
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>

#endif

#include "../../../common/canal.h"
#include "../../../common/vscp.h"
#include "../../../common/canal_macro.h"
#include "../../../../common/dllist.h"

#define CANAL_DLL_LOGGER_OBJ_MUTEX	"___CANAL__DLL_LOGGER_OBJ_MUTEX____"

#define CANAL_LOG_LIST_MAX_MSG		2048

class CMsgLog  
{

public:

    /// Constructor
    CMsgLog();
    
    /// Destructor
    virtual ~CMsgLog();

    
    /*!
        Filter message

        @param pcanalMsg Pointer to CAN message
        @return True if message is accepted false if rejected
    */
    bool doFilter( canalMsg *pcanalMsg );

    
    /*!
        Set Filter
    */
    void setFilter( unsigned long filter );


    /*!
        Set Mask
    */
    void setMask( unsigned long mask);


    /*! 
        Open/create the logfile

        @param szFileName the name of the log file
        @param flags 	bit 1 = 0 Append, bit 1 = 1 Rewrite
        @return True on success.
    */
    bool open( const char * szFileName, unsigned long flags = 0 );

    /*!
        Flush and close the log file
    */
    void close( void );

    /*!
        Write a message out to the file
        @param pcanalMsg Pointer to CAN message
        @return True on success.
    */
    bool writeMsg( canalMsg *pMsg );

    /*!
        Get decimal or hex data value as unsigned long
        @param String representation of number.
        @return Converted number.
    */
    unsigned long getDataValue( const char *szData );

public:

    /// Run flag
    bool m_bRun;

    /// The log file handle
    FILE *m_flog;

    /*! 
        Log queue

        This is the log queue for messages going out to the
        log file.
    */
    DoubleLinkedList m_logList;

    /*!
        Tread id
    */
#ifdef WIN32
    HANDLE m_hTread;
#else
    pthread_t m_threadId;
#endif

    /*!
        Mutex for list.
    */
#ifdef WIN32	
    HANDLE m_logMutex;
#else
    pthread_mutex_t m_logMutex;
#endif

    /*!
        Filter for outgoing messages

        mask bit n	|	filter bit n	| msg id bit	|	result
        ===========================================================
            0				X					X			Accept				
            1				0					0			Accept
            1				0					1			Reject
            1				1					0			Reject
            1				1					1			Accept

        Formula is !( ( filter �d ) & mask )
    */
    uint32_t m_filter;

    /*!
        Mask for outgoing messages	
    */
    uint32_t m_mask;
};

#endif // !defined(AFX_LOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
