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

#ifndef WINLIRC_H
#define WINLIRC_H

#include "globals.h"

#include "stdafx.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class Cdrvdlg;
class Cserver;

class Cwinlirc : public CWinApp
{
public:
	Cdrvdlg *dlg;

	Cserver *server;

	virtual BOOL InitInstance();
	virtual int ExitInstance();
protected:
	DECLARE_MESSAGE_MAP()
};

#endif
