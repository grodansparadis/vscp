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
 */

#include "stdafx.h"
#include "winlirc.h"
#include "drvdlg.h"
#include "server.h"

Cwinlirc app;

BEGIN_MESSAGE_MAP(Cwinlirc,CWinApp)
END_MESSAGE_MAP()

BOOL Cwinlirc::InitInstance()
{   
	AfxInitRichEdit();
	
	if((m_lpCmdLine[0]=='/' || m_lpCmdLine[0]=='-') 
	&& (m_lpCmdLine[1]=='d' || m_lpCmdLine[1]=='D'))
		debug=true;
	else
		debug=false;

	if(debug)
	{
#ifndef _DEBUG
		/* see where they want to save it */
		CFileDialog fd(FALSE,".txt","wldebug.txt",
			OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
			"Text Files (*.txt)|*.txt|All Files (*.*)|*.*||",
			NULL);
		fd.m_ofn.lpstrTitle="Save debugging information to...";
		if(fd.DoModal()==IDCANCEL)
		{
			MessageBox(NULL,"Debugging disabled.\n","WinLIRC",MB_OK);
			debug=false;
		}
		else
		{
			if((debugfile=fopen(fd.GetPathName(),"w"))==NULL)
			{
				MessageBox(NULL,"Could not create debug file;\ndebugging disabled.",
					"WinLIRC",MB_OK);
				debug=false;
			}
		}
#endif
	}

	DEBUG("Checking for other instances...\n");
	if(!CreateMutex(0,FALSE,"WinLIRC Multiple Instance Lockout") ||
		GetLastError()==ERROR_ALREADY_EXISTS)
	{
		HWND tmp;
		tmp=FindWindow(NULL,"WinLIRC");
		if(tmp==NULL)
		{
			MessageBox(NULL,"WinLIRC is already running","WinLIRC",MB_OK);
		}
		else
		{
			// bring it to the top
			CWnd winlirc;
			winlirc.Attach(tmp);
			CWnd *last=winlirc.GetLastActivePopup();
			if(!winlirc.IsWindowVisible())
				winlirc.ShowWindow(SW_SHOW);
			winlirc.SetForegroundWindow();
			last->SetForegroundWindow();
			winlirc.Detach();
		}
		return FALSE;
	}

	DEBUG("Setting process and thread priorities...\n");
	/* Process initialization and sanity checks */
	if(SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS)==0
	|| SetThreadPriority(THREAD_PRIORITY_IDLE)==0)
	{
		MessageBox(NULL,"Could not set thread priority.","WinLIRC",MB_OK|MB_ICONERROR);
		return FALSE;
	}
	
	DEBUG("Checking for high-resolution timer...\n");
	__int64 x=0;
	if(QueryPerformanceFrequency((LARGE_INTEGER *)&x)!=TRUE || x==0)
	{
		MessageBox(NULL,"This system does not support a high-resolution timer."
			       "Sorry.","WinLIRC",MB_OK|MB_ICONERROR);
		return FALSE;
	}

	DEBUG("Starting server...\n");
	server=NULL;
	if((server=new Cserver)==NULL)
	{
		MessageBox(NULL,"Could not create server object.\n","WinLIRC",MB_OK|MB_ICONERROR);
		if(server!=NULL) { delete server; server=NULL; dlg=NULL; }
		return FALSE;
	}

	if(server->init()==false)
	{
		MessageBox(NULL,"Could not start server.","WinLIRC",MB_OK|MB_ICONERROR);
		if(server!=NULL) { delete server; server=NULL; dlg=NULL; }
		return FALSE;
	}
	

	DEBUG("Creating main dialog...\n");
	if((dlg=new Cdrvdlg)==NULL || dlg->Create(IDD_DIALOG,NULL)==0)
	{
		MessageBox(NULL,"Could not create dialog boxes.\n","WinLIRC",MB_OK|MB_ICONERROR);
		if(dlg!=NULL) { delete dlg; dlg=NULL; }
		if(server!=NULL) { delete server; server=NULL; dlg=NULL; }
		return FALSE;
	}

	dlg->ShowWindow(SW_HIDE);
	dlg->UpdateWindow();
	m_pMainWnd=dlg;
	
	return TRUE;
	
}

int Cwinlirc::ExitInstance()
{
	if(server!=NULL)
		delete server;
	if(dlg!=NULL)
		delete dlg;

	if(debugfile!=NULL)
		fclose(debugfile);

	return CWinApp::ExitInstance();
}
