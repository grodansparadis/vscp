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
 * Copyright (C) 1998 Pablo d'Angelo (pablo@ag-trek.allgaeu.org)
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 */

#ifndef  _CONFIG_H
#define  _CONFIG_H

#include "globals.h"

#include "remote.h"
#include <sys/types.h>

/*
  config stuff
*/

void **init_void_array(struct void_array *ar,size_t chunk_size, size_t item_size);
int add_void_array(struct void_array *ar, void * data);
inline void * get_void_array(struct void_array *ar);

/* some safer functions */
void * s_malloc(size_t size);
char * s_strdup(char * string);
ir_code s_strtocode(char *val);
unsigned long  s_strtoul(char *val);
int s_strtoi(char *val);
unsigned int s_strtoui(char *val);

int checkMode(int is_mode, int c_mode, char *error);
int parseFlags(char *val,char *val2);
int addSignal(struct void_array *signals, char *val);
struct ir_ncode * defineCode(char *key, char *val, struct ir_ncode *code);
void defineRemote(char * key, char * val, char *val2, struct ir_remote *rem);
struct ir_remote *read_config(FILE *f);
void free_config(struct ir_remote *remotes);

#endif
