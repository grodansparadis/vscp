// VSCPPipe.h: interface for the CVSCPPipe class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part is part of CANAL (CAN Abstraction Layer)
//  (http://www.vscp.org)
//
// Copyright (C) 2000-2008 Ake Hedman, D of Scandinavia, <akhe@eurosource.se>
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// $RCSfile: vscppipe.h,v $                                       
// $Date: 2005/01/05 12:50:53 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VSCPPIPE_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)
#define AFX_VSCPPIPE_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_

#ifdef __GNUG__
    //#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
    #include  "wx/ownerdrw.h"
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../common/vscp.h"

/// Interprocess success/failure codes
#define PIPE_FAILURE			0
#define PIPE_SUCCESS			1

#define PIPE_BUF_SIZE			512
#define PIPE_TIMEOUT			1000

/// Return types

#define PACKAGE_ACK				0
#define PACKAGE_NACK			1

#define PACKAGE_TIMEOUT			-1
#define PACKAGE_UNKNOWN			-1

#define PACKAGE_FAILURE			0
#define PACKAGE_SUCCESS			1

#ifdef WIN32

/**
	Name for the canal client pipes 
*/
#define VSCP_CLIENT_PIPE "\\\\.\\pipe\\vscpclientpipe___"

/// \def Windows: Pipe 
#define VSCP_TERMINATOR_PIPE "\\\\.\\pipe\\vscppipeterminator___"
	
#else			//  * * * U N I X * * *

#define VSCP_CLIENT_PIPE "/tmp/vscpclientpipe"

/// \def Windows: Pipe 
#define VSCP_TERMINATOR_PIPE "/tmp/vscppipeterminator"

/// Pipe for interprocess communication.
#define VSCP_PIPE	"/tmp/vscppipe"

#endif // Linux

class CVSCPPipe  
{

public:

	CVSCPPipe();
	virtual ~CVSCPPipe();

public:
	
	/*!
		Open communication channel.

		@param szinterface is name of channel.
		@return true if channel is open or false if error or the channel is
		already opened.
	*/
	bool doCmdOpen( const char *szInterface = NULL, unsigned long flags = 0);

	/*!
		Close communication channel
		
		@return true if the close was successfull
	*/
	bool doCmdClose( void );



	/*!
		Send a CAN message through the pipe. 

		@return true if success false if not.
	*/
	bool doCmdSend( vscpMsg *pMsg );


	/*!
		Receive a CAN message through the pipe. 

		@return true if success false if not.
	*/
	bool doCmdReceive( vscpMsg *pMsg, unsigned long id = 0 );


	/*!
		Get the number of messages in the input queue

		@return the number of messages available or if negative
		an error code.
	*/
	int doCmdDataAvailable( void );


	/*!
		Test interface.
	*/
	bool doCmdNOOP( void );

	/*!
		Set the filter for the pipe.
	*/
	bool doCmdFilter(  unsigned long filter );

	/*!
		Set the mask for the pipe.
	*/
	bool doCmdMask(  unsigned long mask );

protected:

	/*!
		Handle to an open pipe
	*/
#ifdef WIN32	
	HANDLE m_hPipe;
#else
	int m_pipe;
#endif

	
	/// Error storage
	unsigned long m_err;

};

#endif // !defined(AFX_VSCPPIPE_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)
