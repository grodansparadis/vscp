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
 * Copyright (C) 1996,97 Ralph Metzler <rjkm@thp.uni-koeln.de>
 * Copyright (C) 1998 Christoph Bartelmus <columbus@hit.handshake.de>
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 * Copyright (C) 2002 Scott Baily <baily@uiuc.edu>
 */

#ifndef _IR_REMOTE_H
#define _IR_REMOTE_H

#include "globals.h"

#include <sys/types.h>
#include <string.h>
#include <math.h>
#include "irdriver.h"

#define strcasecmp stricmp

int gettimeofday(struct mytimeval *, void *);


static inline int is_biphase(struct ir_remote *remote)
{
	if(remote && (remote->flags&RC5 || remote->flags&RC6)) return(1);
	else return(0);
}

static inline int is_rc5(struct ir_remote *remote)
{
        if(remote && remote->flags&RC5) return(1);
        else return(0);
}

static inline int is_rc6(struct ir_remote *remote)
{
        if(remote && remote->flags&RC6) return(1);
        else return(0);
}

static inline int is_rcmm(struct ir_remote *remote)
{
        if(remote && remote->flags&RCMM) return(1);
        else return(0);
}

static inline int is_raw(struct ir_remote *remote)
{
	if(remote && remote->flags&RAW_CODES) return(1);
	else return(0);
}

static inline int is_const(struct ir_remote *remote)
{
	if(remote && remote->flags&CONST_LENGTH) return(1);
	else return(0);
}

static inline int has_header(struct ir_remote *remote)
{
	if(remote && remote->phead>0 && remote->shead>0) return(1);
	else return(0);
}

static inline int has_foot(struct ir_remote *remote)
{
	if(remote && remote->pfoot>0 && remote->sfoot>0) return(1);
	else return(0);
}

static inline int has_repeat(struct ir_remote *remote)
{
	if(remote && remote->prepeat>0 && remote->srepeat>0) return(1);
	else return(0);
}

static inline int has_repeat_gap(struct ir_remote *remote)
{
	if(remote && remote->repeat_gap>0) return(1);
	else return(0);
}

static inline int has_pre(struct ir_remote *remote)
{
	if(remote && remote->pre_data_bits>0) return(1);
	else return(0);
}

static inline int has_post(struct ir_remote *remote)
{
	if(remote && remote->post_data_bits>0) return(1);
	else return(0);
}

static inline int is_pulse(unsigned long data)
{
	return(data&PULSE_BIT ? 1:0);
}

static inline int is_space(unsigned long data)
{
	return(!is_pulse(data));
}

/* check if delta is inside exdelta +/- exdelta*eps/100 */

static inline int expect(struct ir_remote *remote,int delta,int exdelta)
{
	if(abs(exdelta-delta)<exdelta*remote->eps/100 ||
	   abs(exdelta-delta)<remote->aeps)
		return 1;
	return 0;
}

struct ir_remote *get_ir_remote(char *name);
struct ir_ncode *get_ir_code(struct ir_remote *remote,char *name);
inline unsigned long time_left(struct mytimeval *current,struct mytimeval *last,
			       unsigned long gap);
void send_command(struct ir_remote *remote,struct ir_ncode *code);
//void send_space(unsigned long length);
//void send_pulse(unsigned long length);
void SetTransmitPort(HANDLE hCom, unsigned type);	//selects the serial port to transmit on
void clear_rec_buffer(unsigned long data);
int decode(struct ir_remote *remote);
char *decode_command(unsigned long data);

class Clearndlg;
class CIRDriver;

#ifdef LIRC_SERIAL_TRANSMITTER
#define MAXPULSEBYTES 256
static unsigned long pulse_width = 13; /* pulse/space ratio of 50/50 */
static unsigned long space_width = 13; /* 1000000/freq-pulse_width */
static int pulse_byte_length=78; //usecs per byte (tx soft carrier)
static HANDLE tPort; //port to transmit on
static void (*on) (void);  //pointer to on function for hard carrier transmitters
static void (*off) (void); //pointer to off function for hard carrier transmitters
static int (*send_pulse) (unsigned long); //pointer to send_pulse function
static int (*send_space) (unsigned long); //pointer to send_space function
static int pulsedata[MAXPULSEBYTES];
static unsigned transmittertype;
static __int64 freq;		//the frequency used by PerformanceCounter
static __int64 lasttime;	//last time counter was queried
#endif

int uwait(unsigned long usecs);	//wait usecs since lasttime
void send(unsigned long *raw, int cnt);	//transmit raw codes (untested)
void send (ir_ncode *data,struct ir_remote *rem,unsigned int reps=0);  //transmit an ircode
int init_timer();  //initializes QueryPerformanceCounter based timer

#endif
