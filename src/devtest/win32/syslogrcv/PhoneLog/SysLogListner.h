#if !defined(AFX_SYSLOGLISTNER_H__798138BE_5902_4BB2_9B6D_E3F1FFF80CA5__INCLUDED_)
#define AFX_SYSLOGLISTNER_H__798138BE_5902_4BB2_9B6D_E3F1FFF80CA5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SysLogListner.h : header file
//


class CUDPRecord : public CObject 
{

	//DECLARE_DYNAMIC( CUDPRecord )
	
public:
	CString m_IPAddress;
	UINT m_Port;
	int m_Size;
};


/////////////////////////////////////////////////////////////////////////////
// CSysLogListner command target

class CSysLogListner : public CAsyncSocket
{
// Attributes
public:

// Operations
public:
	CSysLogListner();
	virtual ~CSysLogListner();

	void Create( UINT nSocketPort, CString *str );

	bool Execute( int Operation, 
							char *wxCommand, 
							int nShow, 
							char *wxParams, 
							char *wxDir );

// Overrides
public:

	CString *m_pstrStatus;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSysLogListner)
	public:
	virtual void OnReceive(int nErrorCode);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CSysLogListner)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSLOGLISTNER_H__798138BE_5902_4BB2_9B6D_E3F1FFF80CA5__INCLUDED_)
