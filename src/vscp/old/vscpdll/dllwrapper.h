///////////////////////////////////////////////////////////////////////////////
// dllwrapper.h: interface for the CDllWrapper class
//
// This file is part is part of VSCP ( Very Simple Control Protocol )
// http://www.vscp.org)
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $RCSfile: dllwrapper.h,v $                                       
// $Date: 2005/02/16 15:02:37 $                                  
// $Author: akhe $                                              
// $Revision: 1.1 $ 
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLLWRAPPER_H__66E4FA3F_1CA1_405D_AAF1_5F9B1272D75A__INCLUDED_)
#define AFX_DLLWRAPPER_H__66E4FA3F_1CA1_405D_AAF1_5F9B1272D75A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "vscpdlldef.h"	// Holds the function declarations


class CDllWrapper  
{
public:
	CDllWrapper();
	virtual ~CDllWrapper();
	
	/*!
		initialize the dll wrapper

		@param path to the canal dll
		@return	true on success
	*/
	bool initialize( char * path );

		/**
		Open communication channel.

		@param szinterface is name of channel.
		@return true if channel is open or false if error or the channel is
		already opened.
	*/
	long doCmdOpen( char *szInterface = NULL, unsigned long flags = 0L );

	/**
		Close communication channel
		
		@return true if the close was successfull
	*/
	bool doCmdClose( void );

 
	/**
		Send a VSCP message

		@return true if success false if not.
	*/
	bool doCmdSend( PVSCPMSG pMsg );


	/**
		Receive a VSCP message

		@return true if success false if not.
	*/
	bool doCmdReceive( PVSCPMSG pMsg );


	/**
		Get the number of messages in the input queue

		@returne the number of messages available or if negative
		an error code.
	*/
	int doCmdDataAvailable( void );


	/**
		Set/Reset a filter through the pipe. 

		@return true if success false if not.
	*/
	bool doCmdFilter( vscpMsgFilter *pFilter );

 

protected:

	/// device id from open call
	long m_devid;

	/// Application instance handle
	HINSTANCE m_hinst;

	/// Function method holder for open
	LPFNDLL_VSCPOPEN m_proc_vscpopen;

	/// Function method holder for close
	LPFNDLL_VSCPCLOSE m_proc_vscpclose;

	/// Function method holder for send
	LPFNDLL_VSCPSEND m_proc_vscpsend;

	/// Function method holder for receive
	LPFNDLL_VSCPRECEIVE m_proc_vscpreceive;

	/// Function method holder for dataavailable
	LPFNDLL_VSCPDATAAVAILABLE m_proc_vscpdataavailable;

	/// Function method holder for setfilter
	LPFNDLL_VSCPSETFILTER m_proc_vscpsetfilter;

};

#endif // !defined(AFX_DLLWRAPPER_H__66E4FA3F_1CA1_405D_AAF1_5F9B1272D75A__INCLUDED_)
