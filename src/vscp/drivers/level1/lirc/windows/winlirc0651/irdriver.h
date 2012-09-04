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
 * Modifications Copyright (C) 2000 Scott Baily <baily@uiuc.edu>
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#ifndef IRDRIVER_H
#define IRDRIVER_H

#include "globals.h"

#include "stdafx.h"
#include "winlirc.h"
#include "Trayicon.h"

class CIRConfig;
class CIRDriver {

private:	
	int sense;
	unsigned long devicetype;		
	unsigned long virtpulse;		
	OVERLAPPED ov;
	HANDLE hPort;
	unsigned long int cbuf[CBUF_LEN];
	int cbuf_start, cbuf_end;

public:
	CIRDriver();
	~CIRDriver();

	bool InitPort(CIRConfig *cfg, bool daemonize=true);
	void ResetPort(void);
	bool GetData(unsigned long int *dest);
	void ThreadProc(void);
	void DaemonThreadProc(void);
	HANDLE GetCommPort(void);	//returns the handle hPort

	HANDLE hDataReadyEvent;
	Cdrvdlg *drvdlg;

	unsigned long readdata(unsigned long maxusec, HANDLE ThreadEvent);

private:
	bool SetData(unsigned long int src);
};

#endif
