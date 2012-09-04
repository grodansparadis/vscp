/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 * Modifications based on LIRC 0.6.x Copyright (C) 2000 Scott Baily <baily@uiuc.edu>
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#include "stdafx.h"
#include "winlirc.h"
#include "drvdlg.h"
#include "resource.h"
#include "confdlg.h"
#include "remote.h"
#include "globals.h"
#include <pbt.h> // power management stuff ??
#include "server.h" //so we can send SIGHUP

/////////////////////////////////////////////////////////////////////////////
// Cdrvdlg dialog


Cdrvdlg::Cdrvdlg(CWnd* pParent /*=NULL*/)
	: CDialog(Cdrvdlg::IDD, pParent),
	  ti(IDR_TRAYMENU)
{
	//{{AFX_DATA_INIT(Cdrvdlg)
	m_ircode_edit = _T("");
	m_remote_edit = _T("");
	m_reps_edit = 0;
	//}}AFX_DATA_INIT

	use_ir_hardware=true;
	::ir_driver=&driver;
	driver.drvdlg=this;
}


void Cdrvdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Cdrvdlg)
	DDX_Control(pDX, IDC_IRCODE_EDIT, m_IrCodeEditCombo);
	DDX_Control(pDX, IDC_REMOTE_EDIT, m_remote_DropDown);
	DDX_Text(pDX, IDC_IRCODE_EDIT, m_ircode_edit);
	DDV_MaxChars(pDX, m_ircode_edit, 64);
	DDX_Text(pDX, IDC_REMOTE_EDIT, m_remote_edit);
	DDV_MaxChars(pDX, m_remote_edit, 64);
	DDX_Text(pDX, IDC_REPS_EDIT, m_reps_edit);
	DDV_MinMaxInt(pDX, m_reps_edit, 0, 600);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Cdrvdlg, CDialog)
	//{{AFX_MSG_MAP(Cdrvdlg)
	ON_WM_CREATE()
	ON_COMMAND(ID_TOGGLEWINDOW, OnToggleWindow)
	ON_BN_CLICKED(IDC_CONFIG, OnConfig)
	ON_BN_CLICKED(IDC_HIDEME, OnHideme)
	ON_COMMAND(ID_EXITLIRC, OnExitLirc)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SENDCODE, OnSendcode)
	ON_WM_COPYDATA()
	ON_CBN_DROPDOWN(IDC_IRCODE_EDIT, OnDropdownIrcodeEdit)
	ON_MESSAGE(WM_TRAY, OnTrayNotification)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadcast)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Cdrvdlg message handlers

void Cdrvdlg::OnOK() 
{

	CDialog::OnOK();
	ti.SetIcon(0);
	DestroyWindow();
}

void Cdrvdlg::OnCancel() 
{
	CDialog::OnCancel();
	ti.SetIcon(0);
	DestroyWindow();
}

int Cdrvdlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	ti.SetNotificationWnd(this, WM_TRAY);

	if(DoInitializeDaemon()==false)
		return -1;
	
	return 0;	
}

afx_msg LRESULT Cdrvdlg::OnPowerBroadcast(WPARAM wPowerEvent,LPARAM lP)
	{
	LRESULT retval=TRUE;
	switch (wPowerEvent)
		{
			//can we suspend?
			// bit 0 false => automated or emergency power down 
		case PBT_APMQUERYSUSPEND:
			{
				//UI iff bit 0 is set
			BOOL bUIEnabled=(lP & 0x00000001)!=0;
			retval=TRUE;
			break;
			}
	
			//shutdown happening
		case PBT_APMSUSPEND:
			{
			driver.ResetPort(); //if we RequestDeviceWakeup instead we could wake on irevents
			retval=TRUE;		//if the RI pin was connected to the receiving device
			break;
			}

			//wake up from a critical power shutdown
		case PBT_APMRESUMECRITICAL:
			//standard wake up evrnt; UI is on
		case PBT_APMRESUMESUSPEND:
			//New for Win98;NT5
			//unattended wake up; no user interaction possible; screen
			//is probably off.
		case PBT_APMRESUMEAUTOMATIC:
			ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_INIT),"WinLIRC / Initializing");
			if(driver.InitPort(&config)==false)
			{
				DEBUG("InitPort failed\n");
				ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),"WinLIRC / Initialization Error");
				retval=false;
				break;
			}

			ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_OK),"WinLIRC / Ready");
			retval=TRUE;
			break;
							
			//system power source has changed, or 
			//battery life has just got into the near-critical state
		case PBT_APMPOWERSTATUSCHANGE:
			retval=TRUE;
			break;

			//we don't know what happened
		default:
			retval=TRUE;
			break;
		}
	return retval;
	}

LRESULT Cdrvdlg::OnTrayNotification(WPARAM uID, LPARAM lEvent)
{
	if(AllowTrayNotification)
		return ti.OnTrayNotification(uID, lEvent);
	else
		return 0;
}

void Cdrvdlg::OnToggleWindow() 
{
	if(IsWindowVisible())
	{
		ShowWindow(SW_HIDE);
		UpdateWindow();
	}
	else
	{
		ShowWindow(SW_SHOW);
		SetForegroundWindow();
		UpdateWindow();
	}
}

void Cdrvdlg::OnHideme() 
{
	if(IsWindowVisible())
		OnToggleWindow();
}

void Cdrvdlg::GoGreen()
{
	if(SetTimer(1,250,NULL))
		ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_RECV),"WinLIRC / Received Signal");
}
void Cdrvdlg::GoBlue()
{
	if(SetTimer(1,250,NULL))
		ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_SEND),"WinLIRC / Sent Signal");
}

void Cdrvdlg::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent==1)
	{
		KillTimer(1);
		ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_OK),"WinLIRC / Ready");
	}
	
	CDialog::OnTimer(nIDEvent);
}

bool Cdrvdlg::DoInitializeDaemon()
{
	AllowTrayNotification=false;
	for(;;)
	{
		if(InitializeDaemon()==true)
		{
			AllowTrayNotification=true;
			return true;
		}
		
		if(!IsWindowVisible())
			OnToggleWindow();

		if(MessageBox(	"WinLIRC failed to initialize.\n"
						"Would you like to change the configuration\n"
						"and try again?","WinLIRC Error",MB_OKCANCEL)==IDCANCEL)
			return false;
		
		Cconfdlg dlg(this);
		dlg.DoModal();
	}
}

bool Cdrvdlg::InitializeDaemon()
{
	CWaitCursor foo;

	if(config.ReadConfig(&driver)==false)
	{
		ti.notrayicon = config.notrayicon;
		if (ti.notrayicon) ti.SetIcon((HICON)NULL,NULL);
		DEBUG("ReadConfig failed\n");
		ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),"WinLIRC / Initialization Error");
		return false;
	}
	ti.notrayicon = config.notrayicon;
	if (ti.notrayicon) ti.SetIcon((HICON)NULL,NULL);
	
	ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_INIT),"WinLIRC / Initializing");
	if(driver.InitPort(&config)==false)
	{
		DEBUG("InitPort failed\n");
		ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),"WinLIRC / Initialization Error");
		return false;
	}
	((Cwinlirc *)AfxGetApp())->server->send("BEGIN\nSIGHUP\nEND\n");
	ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_OK),"WinLIRC / Ready");
	return true;
}

void Cdrvdlg::OnConfig() 
{
	AllowTrayNotification=false;
	Cconfdlg dlg(this);
	KillTimer(1);
	ti.SetIcon(AfxGetApp()->LoadIcon(IDI_LIRC_ERROR),"WinLIRC / Disabled During Configuration");
	driver.ResetPort();
	dlg.DoModal();
	if(DoInitializeDaemon()==false)
		OnCancel();
	UpdateRemoteComboLists();
}


void Cdrvdlg::OnExitLirc() 
{
	OnCancel();
}


BOOL Cdrvdlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_ircode_edit="";
	UpdateData(FALSE);
	GetDlgItem(IDC_VERSION)->SetWindowText(WINLIRC_VERSION);
	UpdateRemoteComboLists();
	return TRUE;
}

static struct ir_remote * Remotes_GetRemote(struct ir_remote* remotes, const char* const name)
{
	ir_remote* iter = remotes;
	while (iter != NULL)
	{
		if (stricmp(name,iter->name) == 0)
		{
			return iter;
		}
		iter=iter->next;	//look for remote
	}
	return NULL;
}

void Cdrvdlg::OnSendcode() 
{
	UpdateData(TRUE);
	struct ir_ncode *codes;
	struct ir_remote *sender;
	
	sender=global_remotes;
	m_remote_edit.TrimRight();
	m_ircode_edit.TrimRight();
	m_remote_edit.TrimLeft();
	m_ircode_edit.TrimLeft();
	//while (sender!=NULL && sender->next!=NULL && stricmp(m_remote_edit,sender->name)) sender=sender->next;	//look for remote
	sender = Remotes_GetRemote(global_remotes, m_remote_edit);
	//if (sender==NULL || stricmp(m_remote_edit,sender->name) )	MessageBox("No match found for remote!");
	if (sender==NULL) MessageBox("No match found for remote!");
	else
	{
		codes=sender->codes;
		while (codes->name!=NULL && stricmp(m_ircode_edit,codes->name)) codes++;
		if (codes==NULL || codes->name==NULL) MessageBox("No match found for ircode!");	//look for ircode
		else
		{
			if (m_reps_edit < sender->min_repeat)
			{
				m_reps_edit=sender->min_repeat;  //set minimum number of repeats
				UpdateData(FALSE);					//update the display
			}
			send(codes,sender,m_reps_edit);				//transmit the ircode
			GoBlue();
		}
	}
}

LRESULT Cdrvdlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
// handles a transmission command recieved from another application
// pCopyDataStruct->lpData should point to a string of the following format
// remotename	ircodename	reps
// where reps is an optional parameter indicating the number of times to repeat the code (default=0)
{
	
	CString tosend = (LPCSTR) (pCopyDataStruct->lpData);
	CString remotename,codename,times;
	struct ir_ncode *codes;
	struct ir_remote *sender;
	int i,j;
	LRESULT success=0;

	i=tosend.FindOneOf(" \t");
	if (i!=-1)
	{
		remotename=tosend.Left(i);
		codename=tosend.Mid(i);
		codename.TrimLeft();
		j=codename.FindOneOf(" \t");
		if (j>0)
		{
			times=codename.Mid(j);
			codename=codename.Left(j);
			j=atoi(times);
		} else j=0;						//should replace with min reps when that's implemented
		codename.TrimRight();

		sender=global_remotes;
		while (sender!=NULL /*&& sender->next!=NULL*/ && stricmp(remotename,sender->name)) sender=sender->next;
		if (sender!=NULL /*&& !stricmp(remotename,sender->name)*/)
		{
			codes=sender->codes;
			while (codes->name!=NULL && stricmp(codename,codes->name)) codes++;
			if (codes!=NULL && codes->name!=NULL)
			{
				if (j < sender->min_repeat) j=sender->min_repeat;  //set minimum number of repeats
				send(codes,sender,j);					//transmit the code
				GoBlue();								//turn icon blue to indicate a transmission
				success=1;
			} else success=-2; //unknown code
		} else success=-1; //unknown remote
	}
	return(success);
//	return CDialog::OnCopyData(pWnd, pCopyDataStruct);
}

void Cdrvdlg::UpdateRemoteComboLists()
{
	UpdateData(TRUE);
	m_remote_DropDown.ResetContent();

	//Fill remote combo box
	struct ir_remote* sender=global_remotes;
	while (sender!=NULL)
	{
		m_remote_DropDown.AddString(sender->name);
		sender=sender->next;
	}
	//Set selected item
	if (m_remote_DropDown.SelectString(-1,m_remote_edit) == CB_ERR)
	{
		//Did not find remote selected before, select first
		m_remote_DropDown.SetCurSel(0);
	}
	UpdateData(FALSE);

	UpdateIrCodeComboLists();
}

void Cdrvdlg::UpdateIrCodeComboLists()
{
	UpdateData(TRUE);
	
	//Retrieve pointer to remote by name
	struct ir_remote* selected_remote = Remotes_GetRemote(global_remotes,m_remote_edit);

	m_IrCodeEditCombo.ResetContent();

	if (selected_remote)
	{
		ir_ncode* codes = selected_remote->codes;
		while (codes && codes->name!=NULL)
		{
			m_IrCodeEditCombo.AddString(codes->name);
			codes++;
		}
	}

	if (m_IrCodeEditCombo.SelectString(-1,m_ircode_edit) == CB_ERR)
	{
		m_IrCodeEditCombo.SetCurSel(0);
	}

	UpdateData(FALSE);
}

void Cdrvdlg::OnDropdownIrcodeEdit() 
{
	// TODO: Add your control notification handler code here
	UpdateIrCodeComboLists();
}
