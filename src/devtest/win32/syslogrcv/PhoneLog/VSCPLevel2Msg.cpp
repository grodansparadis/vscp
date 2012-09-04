// VSCPLevel2Msg.cpp: implementation of the CVSCPLevel2Msg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PhoneLog.h"
#include "VSCPLevel2Msg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVSCPLevel2Msg::CVSCPLevel2Msg()
{
	m_pData = NULL;	// No Data
	m_size = 0;

	m_class = 0;	// No class
	m_type = 0;		// No type

	m_priority = VSCPII_PRIORITY_LOW;

	m_bHardCoded = false;	// Not hardcoded	
}

CVSCPLevel2Msg::~CVSCPLevel2Msg()
{
	if ( NULL != m_pData ) {
		delete [] m_pData;
	}
}
