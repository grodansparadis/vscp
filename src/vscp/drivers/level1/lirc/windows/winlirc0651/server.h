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

#ifndef SERVER__H
#define SERVER__H

#include "globals.h"

#include "stdafx.h"
#include "winlirc.h"
#include "drvdlg.h"
#include "irdriver.h"
#include <atlbase.h> //password stuff


class Cserver {
public:

	Cserver();
	~Cserver();

	bool startserver(void);
	void stopserver(void);

	bool init(void);
	void send(const char *s); 

	void ThreadProc(void);

private:
	void reply(const char *command,int client,bool success,const char *data);
	SOCKET server;
	SOCKET clients[MAX_CLIENTS];
	CString password; //password for transmission
	int tcp_port; //tcp port for server
};

#endif
