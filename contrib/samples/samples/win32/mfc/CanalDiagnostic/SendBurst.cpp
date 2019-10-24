// SendBurst.cpp : implementation file
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp 
//
// This file is part of the CANAL (http://www.vscp.org) 
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
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
// $RCSfile: SendBurst.cpp,v $                                       
// $Date: 2005/01/05 12:16:20 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

#include "stdafx.h"
#include "CanalDiagnostic.h"
#include "SendBurst.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SendBurst dialog


SendBurst::SendBurst(CWnd* pParent /*=NULL*/)
	: CDialog(SendBurst::IDD, pParent)
{
	//{{AFX_DATA_INIT(SendBurst)
	m_numberOfPackets = _T("");
	m_interPacketDelay = _T("");
	m_bAutoIncId = FALSE;
	m_bAutoIncData0 = FALSE;
	m_bAutoIncData1 = FALSE;
	m_bAutoIncData2 = FALSE;
	m_bAutoIncData = FALSE;
	m_bAutoIncData4 = FALSE;
	m_bAutoIncData5 = FALSE;
	m_bAutoIncData6 = FALSE;
	m_bAutoIncData7 = FALSE;
	//}}AFX_DATA_INIT
}


void SendBurst::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SendBurst)
	DDX_Control(pDX, IDC_SPIN2, m_spinInterPacketDelay);
	DDX_Control(pDX, IDC_SPIN1, m_spinNumberOfPackets);
	DDX_Text(pDX, IDC_EDIT_BURST_PACKETS, m_numberOfPackets);
	DDX_Text(pDX, IDC_EDIT_INTER_PACKET_DELAY, m_interPacketDelay);
	DDX_Check(pDX, IDC_CHECK_AUTOINC_ID, m_bAutoIncId);
	DDX_Check(pDX, IDC_CHECK_AUTOINC_DATA0, m_bAutoIncData0);
	DDX_Check(pDX, IDC_CHECK_AUTOINC_DATA1, m_bAutoIncData1);
	DDX_Check(pDX, IDC_CHECK_AUTOINC_DATA2, m_bAutoIncData2);
	DDX_Check(pDX, IDC_CHECK_AUTOINC_DATA3, m_bAutoIncData);
	DDX_Check(pDX, IDC_CHECK_AUTOINC_DATA4, m_bAutoIncData4);
	DDX_Check(pDX, IDC_CHECK_AUTOINC_DATA5, m_bAutoIncData5);
	DDX_Check(pDX, IDC_CHECK_AUTOINC_DATA6, m_bAutoIncData6);
	DDX_Check(pDX, IDC_CHECK_AUTOINC_DATA7, m_bAutoIncData7);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SendBurst, CDialog)
	//{{AFX_MSG_MAP(SendBurst)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SendBurst message handlers
