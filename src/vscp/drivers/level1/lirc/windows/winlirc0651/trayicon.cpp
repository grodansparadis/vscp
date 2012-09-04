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
 * Derived from published code by Paul DiLascia.
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#include "stdafx.h"
#include <afxpriv.h>
#include "trayicon.h"
#include "winlirc.h"

IMPLEMENT_DYNAMIC(CTrayIcon, CCmdTarget)

CTrayIcon::CTrayIcon(unsigned int uID)
{
	memset(&icondata,0,sizeof(icondata));
	icondata.cbSize=sizeof(icondata);
	icondata.uID=uID;
	AfxLoadString(uID,icondata.szTip,sizeof(icondata.szTip));
}

CTrayIcon::~CTrayIcon()
{
	SetIcon(0);
}

void CTrayIcon::SetNotificationWnd(CWnd *notifywnd, unsigned int message)
{
	if(notifywnd!=NULL && !::IsWindow(notifywnd->GetSafeHwnd()))
	{
		DEBUG("Invalid window\n");
		return;
	}

	icondata.hWnd=notifywnd->GetSafeHwnd();

	if(message!=0 && message<WM_USER)
	{
		DEBUG("Invalid message\n");
		message=0;
	}
	icondata.uCallbackMessage=message;
}

bool CTrayIcon::SetIcon(unsigned int uID)
{ 
	HICON icon=NULL;
	if(uID)
	{
		AfxLoadString(uID,icondata.szTip,sizeof(icondata.szTip));
		icon=AfxGetApp()->LoadIcon(uID);
	}
	return SetIcon(icon,NULL);
}

bool CTrayIcon::SetIcon(HICON icon, const char *tip) 
{
	unsigned int msg;
	icondata.uFlags=0;

	if(icon)
	{
		if (notrayicon) return true;			
		if(icondata.hIcon) msg=NIM_MODIFY;
		else msg=NIM_ADD;
		icondata.hIcon=icon;
		icondata.uFlags|=NIF_ICON;
	}
	else
	{
		if(icondata.hIcon==NULL)
			return true;
		msg=NIM_DELETE;
	}

	if(tip)
		strncpy(icondata.szTip,tip,sizeof(icondata.szTip));
	if(*icondata.szTip)
		icondata.uFlags|=NIF_TIP;

	if(icondata.uCallbackMessage && icondata.hWnd)
		icondata.uFlags|=NIF_MESSAGE;

	int ret=Shell_NotifyIcon(msg,&icondata);
	if(msg==NIM_DELETE || !ret)
		icondata.hIcon=NULL;
	return (ret==TRUE);
}

LRESULT CTrayIcon::OnTrayNotification(WPARAM id, LPARAM event)
{
	if(id!=icondata.uID || (event!=WM_RBUTTONUP && event!=WM_LBUTTONDBLCLK))
		return 0;

	// resource menu with same ID as icon will be used as popup menu
	CMenu menu;
	if(!menu.LoadMenu(icondata.uID)) return 0;
	CMenu *submenu=menu.GetSubMenu(0); 
	if(!submenu) 
		return 0;

	if(event==WM_RBUTTONUP)
	{
		::SetMenuDefaultItem(submenu->m_hMenu,0,true);
		CPoint mouse;
		GetCursorPos(&mouse);
		::SetForegroundWindow(icondata.hWnd);	
		::TrackPopupMenu(submenu->m_hMenu,0,mouse.x,mouse.y,0,icondata.hWnd,NULL);
	}
	else
	{
		::SendMessage(icondata.hWnd,WM_COMMAND,submenu->GetMenuItemID(0),0);
	}

	return 1;
}

bool CTrayIcon::SetStandardIcon(const char *iconname, const char *tip)
{
	return SetIcon(::LoadIcon(NULL,iconname),tip);
}

bool CTrayIcon::SetIcon(const char *resname, const char *tip)
{
	return SetIcon(resname?AfxGetApp()->LoadIcon(resname):NULL,tip);
}
