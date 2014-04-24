// CanalDiagnosticDoc.cpp : implementation of the CCanalDiagnosticDoc class
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
// $RCSfile: CanalDiagnosticDoc.cpp,v $                                       
// $Date: 2005/01/05 12:16:20 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 

#include "stdafx.h"
#include "CanalDiagnostic.h"
#include "CanalDiagnosticDoc.h"
#include "NewDocument.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCanalDiagnosticDoc

IMPLEMENT_DYNCREATE(CCanalDiagnosticDoc, CDocument)

BEGIN_MESSAGE_MAP(CCanalDiagnosticDoc, CDocument)
	//{{AFX_MSG_MAP(CCanalDiagnosticDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCanalDiagnosticDoc construction/destruction

CCanalDiagnosticDoc::CCanalDiagnosticDoc()
{
	// Initialize the device structure
	m_itemDevice.id = 0;
	m_itemDevice.deviceStr[ 0 ] = 0;
	m_itemDevice.name[ 0 ] = 0;
	m_itemDevice.path[ 0 ] = 0;
	m_itemDevice.flags = 0;
	m_itemDevice.inbufsize = 0;
	m_itemDevice.outbufsize = 0;

	m_bOpen = false;

	dll_init( &m_msgList, SORT_NONE );

	m_pNode = NULL;
	m_pLastNode = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Destructor

CCanalDiagnosticDoc::~CCanalDiagnosticDoc()
{
	if ( m_bOpen ) OnCloseDocument();	
}

/////////////////////////////////////////////////////////////////////////////
// OnNewDocument

BOOL CCanalDiagnosticDoc::OnNewDocument()
{	

	if ( !CDocument::OnNewDocument() ) {
		return FALSE;
	}

	CNewDocument dlgDoc;
	if ( IDOK != dlgDoc.DoModal() ) {
		return FALSE;
	}

	// Get data for the selected device
	dlgDoc.getDeviceProfile( &m_itemDevice );

	if ( SHT_INTERFACE == m_itemDevice.id ) {

		////////////////////////////////////
		// Work trough the SHT interface //
		////////////////////////////////////

		// First check if the CANAL daemon is 
		// available

		if ( !m_canalif.doCmdOpen( NULL, 0 ) ) {
			AfxMessageBox("Failed to open channel to CANAL daemon!");
			return false;
		}

		if ( !m_canalif.doCmdNOOP() ) {
			// Failure
			AfxMessageBox("Unable to talk to the CANAL daemon.");
			return false;
		}

		m_bOpen = true;
		
		
	}
	else{

		////////////////////////////////////
		//   Handle the selected device   //
		////////////////////////////////////
		
		if ( !m_canalDll.initialize( (char *)m_itemDevice.path ) ) {
			AfxMessageBox("Failed to initialize device!");
			return false;
		}
		if ( !m_canalDll.doCmdOpen( (char *)m_itemDevice.deviceStr, m_itemDevice.flags ) ) {
			AfxMessageBox("Failed to open device!");
			return false;
		}
		
		m_bOpen = true;
		
	}
	
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CCanalDiagnosticDoc serialization

void CCanalDiagnosticDoc::Serialize(CArchive& ar)
{
	if ( ar.IsStoring() ) {
		// TODO: add storing code here
	}
	else {
		// TODO: add loading code here
	}
}


/////////////////////////////////////////////////////////////////////////////
// CCanalDiagnosticDoc diagnostics

#ifdef _DEBUG
void CCanalDiagnosticDoc::AssertValid() const
{
	CDocument::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////
// Dump

void CCanalDiagnosticDoc::Dump( CDumpContext& dc ) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCanalDiagnosticDoc commands

void CCanalDiagnosticDoc::OnCloseDocument() 
{	
	if ( m_bOpen ) {

		if ( SHT_INTERFACE == m_itemDevice.id ) {
			m_canalif.doCmdClose();
			m_bOpen = false;
		}
		else {
			m_canalDll.doCmdClose();
			m_bOpen = false;
		}
	}

	// Empty the list
	dll_removeAllNodes( &m_msgList );

	CDocument::OnCloseDocument();
}

/////////////////////////////////////////////////////////////////////////////
// checkForData
//

bool CCanalDiagnosticDoc::checkForData( void )
{
	int cnt;
	canalMsg Msg;
	bool rv = false;

	// Nothing to do if not open
	if ( !m_bOpen ) return false;

	if ( m_bOpen && ( m_msgList.nCount < MAX_NUMBER_OF_MSG ) ) {

		if ( SHT_INTERFACE == m_itemDevice.id ) {
			
			if ( cnt = m_canalif.doCmdDataAvailable() ) {

				for ( int i=0; i<cnt; i++ ) {
				
					if ( m_canalif.doCmdReceive( &Msg ) ) {
					
						dllnode *pNode = new dllnode;

						if ( NULL != pNode ) {

							canalMsg *pnewMsg = new canalMsg;

							pNode->pObject = pnewMsg;
							pNode->pKey = NULL;
							pNode->pstrKey = NULL;

							if ( NULL != pnewMsg ) {
								memcpy( pnewMsg, &Msg, sizeof( canalMsg ) );
							}
				
							dll_addNode( &m_msgList, pNode );
							rv = true;

						}
					}
				}
			}
		}
		else {
			
			if ( cnt = m_canalDll.doCmdDataAvailable() ) {
				
				for ( int i=0; i<cnt; i++ ) {
				
					if ( m_canalDll.doCmdReceive( &Msg ) ) {
					
						dllnode *pNode = new dllnode;

						if ( NULL != pNode ) {

							canalMsg *pnewMsg = new canalMsg;

							pNode->pObject = pnewMsg;
							pNode->pKey = NULL;
							pNode->pstrKey = NULL;

							if ( NULL != pnewMsg ) {
								memcpy( pnewMsg, &Msg, sizeof( canalMsg ) );
							}
				
							dll_addNode( &m_msgList, pNode );
							rv = true;

						}
					}
				}
			}
		}
	}	

	return rv;
}


/////////////////////////////////////////////////////////////////////////////
// sendMessage
//

bool CCanalDiagnosticDoc::sendMessage( canalMsg *pMsg )
{
	bool rv = false;

	if ( SHT_INTERFACE == m_itemDevice.id ) { 
		rv = m_canalif.doCmdSend( pMsg );
	}
	else {
		rv = m_canalDll.doCmdSend( pMsg );
	}

	// Add message to queue
	dllnode *pNode = new dllnode;

	if ( NULL != pNode ) {

		canalMsg *pnewMsg = new canalMsg;

		pNode->pObject = pnewMsg;
		pNode->pKey = NULL;
		pNode->pstrKey = NULL;

		if ( NULL != pnewMsg ) {			
			memcpy( pnewMsg, pMsg, sizeof( canalMsg ) );
			pnewMsg->flags = CANAL_IDFLAG_SEND;
			pnewMsg->timestamp = GetTickCount();
		}
				
		dll_addNode( &m_msgList, pNode );
		rv = true;

	}

	UpdateAllViews( NULL );

	return rv;
}


/////////////////////////////////////////////////////////////////////////////
// getNextCanMsg
//

bool CCanalDiagnosticDoc::getNextCanMsg( canalMsg *pMsg )
{
	// Must be a vliad message structure supplied
	if ( NULL == pMsg ) return false;

	// Must be something in the list
	if ( NULL == m_msgList.pHead ) return false;

	if ( NULL == m_pLastNode ) {
		m_pNode =  m_msgList.pHead;	
	}

	if ( NULL == m_pNode ) {
		m_pNode = m_pLastNode->pNext;
		if ( NULL == m_pNode ) return false; 	
	}

	// Fetch the message
	memcpy( pMsg, m_pNode->pObject, sizeof( canalMsg ) );

	// Save the current node
	m_pLastNode = m_pNode;

	m_pNode = m_pNode->pNext; 
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// resetMsgChain
//

void CCanalDiagnosticDoc::resetMsgChain( void )
{
	m_pNode =  NULL;  
	m_pLastNode =  NULL;
}
