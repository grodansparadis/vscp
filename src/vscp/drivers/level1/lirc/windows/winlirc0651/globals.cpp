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

#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include "afxmt.h"
#include "stdafx.h"
#include <stdarg.h>

/* Debugging stuff */
bool debug=false;
FILE *debugfile=NULL;
char *__file="?";
int __line=0;
void winlirc_debug(char *format, ...)
{
	va_list args;
	va_start(args,format);

	char s[1024];
	char t[512];
	_vsnprintf(t,512,format,args);
	_snprintf(s,1024,"%s(%d) : %s",__file,__line,t);

#ifdef _DEBUG
	afxDump << s;
#else
	fprintf(debugfile,s);
	fflush(debugfile);
#endif

	va_end(args);
}


/* End of Debugging stuff */

struct flaglist all_flags[] = {
	{"RC5",			RC5},
	{"RC6",             RC6},
	{"RCMM",            RCMM},
	{"SHIFT_ENC",   SHIFT_ENC}, /* obsolete */
	{"SPACE_ENC",   SPACE_ENC},
	{"REVERSE",     REVERSE},
	{"NO_HEAD_REP", NO_HEAD_REP},
	{"NO_FOOT_REP", NO_FOOT_REP},
	{"CONST_LENGTH",CONST_LENGTH},
	{"RAW_CODES",   RAW_CODES},
	{"REPEAT_HEADER",   REPEAT_HEADER},
	{"SPECIAL_TRANSMITTER",   SPECIAL_TRANSMITTER},
	{NULL,0},
};

struct ir_remote remote;
struct ir_ncode code;
struct sbuf send_buffer;
struct rbuf rec_buffer;
struct ir_remote *global_remotes=NULL;
struct ir_remote *last_include_remote=NULL;
struct ir_remote *include_remotes[]={NULL};
struct ir_remote *last_remote=NULL;
struct ir_remote *repeat_remote=NULL;
struct ir_ncode *repeat_code=NULL;
struct ir_remote *free_remotes=NULL;
struct ir_remote *decoding=NULL;
class CIRDriver *ir_driver=NULL;
class Clearndlg *learn_dialog=NULL;
bool use_ir_hardware=true;
int line;
int parse_error=0;
int lirc;

CWinThread *IRThreadHandle=NULL;
CWinThread *DaemonThreadHandle=NULL;
CWinThread *LearnThreadHandle=NULL;
CWinThread *ServerThreadHandle=NULL;
CEvent IRThreadEvent;
CEvent DaemonThreadEvent;
CEvent LearnThreadEvent;
CEvent ServerThreadEvent;

CCriticalSection CS_global_remotes;

void KillThread(CWinThread **ThreadHandle, CEvent *ThreadEvent)
{
	while(*ThreadHandle!=NULL)
	{
		DWORD result=0;
		if(GetExitCodeThread((*ThreadHandle)->m_hThread,&result)==0) 
		{
			DEBUG("GetExitCodeThread failed, error=%d\n",GetLastError());
			DEBUG("(the thread may have already been terminated)\n");
			return;
		}
		if(result==STILL_ACTIVE)
		{
			ThreadEvent->SetEvent();
			if(WAIT_TIMEOUT==WaitForSingleObject((*ThreadHandle)->m_hThread,250/*INFINITE*/)) break; //maybe we just need to give it some time to quit
			ThreadEvent->ResetEvent();
			*ThreadHandle=NULL;
		}
	}
}
