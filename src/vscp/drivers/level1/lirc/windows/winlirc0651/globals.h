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
 * Modifications based on LIRC 0.6.1 Copyright (C) 2000 Scott Baily <baily@uiuc.edu>
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdlib.h>

/* Debugging stuff */
extern bool debug;
extern FILE *debugfile;
extern char *__file;
extern int __line;
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG if(debug) __file=__FILE__,__line=__LINE__,winlirc_debug
extern void winlirc_debug(char *format, ...);
/* End of Debugging stuff */

/* Constants */
#define LINE_LEN 1024
#define CBUF_LEN 1024
#include "version.h"
#define LIRC_VERSION "WinLIRC " WINLIRC_VERSION " (LIRC 0.6.1pre3)"
#define BUTTON 80
#define MAX_SIGNALS 200
#define AEPS 100
#define EPS 25	// only a default -- can be changed when learning a remote
#define WBUF_SIZE (256)
#define RBUF_SIZE (256)
#define PACKET_SIZE (256)
#define PULSE_BIT 0x1000000
#define REC_SYNC 8
#define WM_TRAY (WM_USER+34)
#define MAX_CLIENTS 8
#define IR_PORT 8765
#define LISTENQ 4
#define LONG_IR_CODE
#define NR_FLAGS 10
#define RC5				0x0001      /* IR data follows RC5 protocol */
#define RC6				0x0002      /* IR data follows RC6 protocol */
#define RCMM			0x0004      /* IR data follows RC-MM protocol */
#define SPACE_ENC		0x0008	    /* IR data is space encoded */
#define REVERSE			0x0010
#define NO_HEAD_REP		0x0020	    /* no header for key repeates */
#define NO_FOOT_REP		0x0040	    /* no foot for key repeates */
#define CONST_LENGTH    0x0080      /* signal length+gap is always constant */
#define RAW_CODES       0x0100      /* for internal use only */
#define REPEAT_HEADER   0x0200		/* header is also sent before repeat code */
#define SHIFT_ENC		RC5			/* IR data is shift encoded (name obsolete) */
#define SPECIAL_TRANSMITTER 0x0400  /* the default transmitter type is overrridden by remote definition WINLIRC ONLY! */

#define LIRC_SERIAL_TRANSMITTER
#define HARDCARRIER		0x0001		/* the transmitter generates its own carrier modulation */
#define TXTRANSMITTER	0x0002		/* the transmitter uses the TX pin */
#define INVERTED		0x0004		/* the transmitter polarity is opposite */

/* typedefs */
#ifdef LONG_IR_CODE
typedef unsigned __int64 ir_code;
#else
typedef unsigned long ir_code;
#endif

/* Structure definitions */
struct mytimeval {
	long tv_sec;
	long tv_usec;
};
struct flaglist {
	char *name;
	int flag;
};
struct ptr_array {
	void **ptr;
	size_t nr_items;
	size_t chunk_size;
};
struct void_array {
	void *ptr;
	size_t item_size;
	size_t nr_items;
	size_t chunk_size;
};
struct sbuf {
	unsigned long data[WBUF_SIZE];
	int wptr;
	int too_long;
	int is_biphase;
	unsigned long pendingp;
	unsigned long pendings;
	unsigned long sum;
};
struct rbuf {
	unsigned long data[RBUF_SIZE];
	int rptr;
	int wptr;
	int too_long;
	int is_biphase;
	unsigned long pendingp;
	unsigned long pendings;
	unsigned long sum;
};
struct ir_ncode {
	char *name;
	ir_code code;
	int length;
	unsigned long *signals;
};
struct ir_remote 
{
	char *name;                 /* name of remote control */
	struct ir_ncode *codes;
	int bits;                   /* bits (length of code) */
	int flags;                  /* flags */
	int eps;                    /* eps (_relative_ tolerance) */
	int aeps;                   /* aeps (_absolute_ tolerance) */
	int phead,shead;            /* header */
	int pthree,sthree;			/* 3 (only used for RC-MM) */
	int ptwo,stwo;				/* 2 (only used for RC-MM) */
	int pone,sone;              /* 1 */
	int pzero,szero;            /* 0 */
	int plead;				    /* leading pulse */
	int ptrail;                 /* trailing pulse */
	int pfoot,sfoot;            /* foot */
	int prepeat,srepeat;	    /* indicate repeating */
	int pre_data_bits;          /* length of pre_data */
	ir_code pre_data;           /* data which the remote sends before keycode */
	int post_data_bits;         /* length of post_data */
	ir_code post_data;          /* data which the remote sends after keycode */
	int pre_p,pre_s;            /* signal between pre_data and keycode */
	int post_p, post_s;         /* signal between keycode and post_code */
	unsigned long gap;          /* time between signals in usecs */
	unsigned long repeat_gap;   /* time between two repeat codes if different from gap */
	int toggle_bit;             /* 1..bits */
	int min_repeat;             /* code is repeated at least x times; code sent once -> min_repeat=0 */
	unsigned int freq;          /* modulation frequency */
	unsigned int duty_cycle;    /* 0<duty cycle<=100 */
	unsigned transmitter;		/* specific transmitter type configured on per remote basis WINLIRC ONLY! */

	/* end of user editable values */

    int repeat_state;
	struct ir_ncode *last_code;
	int reps;
	struct mytimeval last_send;
	unsigned long remaining_gap;/* remember gap for CONST_LENGTH remotes */
    struct ir_remote *next;
};
struct lengths {
	unsigned int count;
	unsigned long sum,upper_bound,lower_bound,min,max;
	struct lengths *next;
};


/* enums */
enum directive {ID_none,ID_remote,ID_codes,ID_raw_codes,ID_raw_name};

/* externs */
extern struct flaglist all_flags[];
extern struct ir_remote remote;
extern struct ir_ncode code;
extern struct sbuf send_buffer;
extern struct rbuf rec_buffer;
extern struct ir_remote *global_remotes;
extern struct ir_remote *last_include_remote;
extern struct ir_remote *include_remotes[];
extern struct ir_remote *last_remote;
extern struct ir_remote *repeat_remote;
extern struct ir_ncode *repeat_code;
extern struct ir_remote *free_remotes;
extern struct ir_remote *decoding;
extern class CIRDriver *ir_driver;
extern class Clearndlg *learn_dialog;
extern bool use_ir_hardware;
extern int line;
extern int parse_error;
extern int lirc;


/* Change this stuff */
extern class CCriticalSection CS_global_remotes;
extern class CWinThread *IRThreadHandle;
extern class CWinThread *DaemonThreadHandle;
extern class CWinThread *LearnThreadHandle;
extern class CWinThread *ServerThreadHandle;
extern class CEvent IRThreadEvent;
extern class CEvent DaemonThreadEvent;
extern class CEvent LearnThreadEvent;
extern class CEvent ServerThreadEvent;

void KillThread(CWinThread **ThreadHandle, CEvent *ThreadEvent);

#endif