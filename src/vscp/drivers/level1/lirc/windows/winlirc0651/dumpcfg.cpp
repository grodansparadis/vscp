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
 * Copyright (C) 1998 Pablo d'Angelo <pablo@ag-trek.allgaeu.org>
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 * RX device, some other stuff Copyright (C) 2002 Alexander Nesterovsky <Nsky@users.sourceforge.net>
 */

#include "dumpcfg.h"
#include "config.h"
#include "remote.h"
#include "globals.h"

void fprint_comment(FILE *f,struct ir_remote *rem)
{
	time_t timet;
	struct tm *tmp;

	timet=time(NULL);
	tmp=localtime(&timet);
	fprintf(f,
		"#\n"
		"# this config file was automatically generated\n"
		"# using %s on %s"									
		"#\n"
		"# contributed by \n"
		"#\n"
		"# brand:             %s\n"
		"# model:             \n"
		"# supported devices: \n"
		"#\n\n",LIRC_VERSION,asctime(tmp),
		rem->name);
}

void fprint_flags(FILE *f, int flags)
{
	int i;
	int begin=0;

	for(i=0;i<NR_FLAGS;i++)
	{
		if(flags&all_flags[i].flag)
		{
			if(begin==0) fprintf(f, "  flags ");
			else if(begin==1) fprintf(f,"|");
			fprintf(f,"%s",all_flags[i].name);
			begin=1;
		}
	}
	if(begin==1) fprintf(f,"\n");
}

void fprint_remotes(FILE *f, struct ir_remote *all){

    	while(all)
	{
                fprint_remote(f, all);
                fprintf(f, "\n\n");
                all=all->next;
        }
}

void fprint_remote_head(FILE *f, struct ir_remote *rem)
{
	fprintf(f, "begin remote\n\n");
	if(!is_raw(rem)){
		fprintf(f, "  name  %s\n",rem->name);
		fprintf(f, "  bits        %5d\n",rem->bits);
		fprint_flags(f,rem->flags);
		fprintf(f, "  eps         %5d\n",rem->eps);
		fprintf(f, "  aeps        %5d\n\n",rem->aeps);
		if(has_header(rem))
		{
			fprintf(f, "  header      %5d %5d\n",
				rem->phead, rem->shead);
		}
		fprintf(f, "  one         %5d %5d\n",rem->pone, rem->sone);
		fprintf(f, "  zero        %5d %5d\n",rem->pzero, rem->szero);
		if(rem->ptrail!=0)
		{
			fprintf(f, "  ptrail      %5d\n",rem->ptrail);
		}
		if(rem->plead!=0)
		{
			fprintf(f, "  plead       %5d\n",rem->plead);
		}
		if(has_foot(rem))
		{
			fprintf(f, "  foot        %5d %5d\n",
				rem->pfoot, rem->sfoot);
		}
		if(has_repeat(rem))
		{
			fprintf(f, "  repeat      %5d %5d\n",
				rem->prepeat, rem->srepeat);
		}
		if(rem->pre_data_bits>0)
		{
			fprintf(f, "  pre_data_bits   %d\n",rem->pre_data_bits);
#                       ifdef LONG_IR_CODE
			fprintf(f, "  pre_data       0x%I64X\n",rem->pre_data);
#                       else
			fprintf(f, "  pre_data       0x%lX\n",rem->pre_data);
#                       endif
		}
		if(rem->post_data_bits>0)
		{
			fprintf(f, "  post_data_bits  %d\n",rem->post_data_bits);
#                       ifdef LONG_IR_CODE
			fprintf(f, "  post_data      0x%I64X\n",rem->post_data);
#                       else
			fprintf(f, "  post_data      0x%lX\n",rem->post_data);
#                       endif
		}
		if(rem->pre_p!=0 && rem->pre_s!=0)
		{
			fprintf(f, "  pre         %5d %5d\n",
				rem->pre_p, rem->pre_s);
		}
		if(rem->post_p!=0 && rem->post_s!=0)
		{
			fprintf(f, "  post        %5d %5d\n",
				rem->post_p, rem->post_s);
		}
		fprintf(f, "  gap          %lu\n",rem->gap);
		if(has_repeat_gap(rem))
		{
			fprintf(f, "  repeat_gap   %lu\n",rem->repeat_gap);
		}
		if(rem->min_repeat>0)
		{
			fprintf(f, "  min_repeat      %d\n",rem->min_repeat);
		}
		fprintf(f, "  toggle_bit      %d\n\n",rem->toggle_bit);
	}
	else
	{
		fprintf(f, "  name   %s\n",rem->name);
		fprint_flags(f,rem->flags);
		fprintf(f, "  eps         %5d\n",rem->eps);
		fprintf(f, "  aeps        %5d\n\n",rem->aeps);
		fprintf(f, "  ptrail      %5d\n",rem->ptrail);
		fprintf(f, "  repeat %5d %5d\n",rem->prepeat, rem->srepeat);
		fprintf(f, "  gap    %lu\n\n",rem->gap);
	}
	if(rem->freq!=0)
	{
		fprintf(f, "  frequency    %u\n",rem->freq);
	}
	if(rem->duty_cycle!=0)
	{
        fprintf(f, "  duty_cycle   %u\n",rem->duty_cycle);
	}
	fprintf(f,"\n");
}

void fprint_remote_foot(FILE *f, struct ir_remote *rem)
{
	fprintf(f, "end remote\n");
}

void fprint_remote_signal_head(FILE *f, struct ir_remote *rem)
{
	if(!is_raw(rem))
		fprintf(f, "      begin codes\n");
	else
		fprintf(f, "      begin raw_codes\n\n");
}

void fprint_remote_signal_foot(FILE *f, struct ir_remote *rem)
{
	if(!is_raw(rem))
		fprintf(f, "      end codes\n\n");
	else
		fprintf(f, "      end raw_codes\n\n");
}

void fprint_remote_signal(FILE *f,struct ir_remote *rem, struct ir_ncode *codes)
{
	int i,j;

	if(!is_raw(rem))
	{
#               ifdef LONG_IR_CODE
		fprintf(f, "          %-24s 0x%016I64X\n",codes->name, codes->code);
#               else
		fprintf(f, "          %-24s 0x%016lX\n",codes->name, codes->code);
#               endif
	}
	else
	{
		fprintf(f, "          name %s\n",codes->name);
		j=0;
		for(i=0;i<codes->length;i++){
			if (j==0){
				fprintf(f, "          %7lu",codes->signals[i]);
			}else if (j<5){
				fprintf(f, " %7lu",codes->signals[i]);
			}else{
				fprintf(f, " %7lu\n",codes->signals[i]);
				j=-1;
			}
			j++;
		}
		codes++;
		if (j==0)
		{
			fprintf(f,"\n");
		}else
		{
			fprintf(f,"\n\n");
			j=0;
		}
	}
}

void fprint_remote_signals(FILE *f, struct ir_remote *rem)
{
    struct ir_ncode *codes;
	
	fprint_remote_signal_head(f,rem);
	codes=rem->codes;
	while(codes->name!=NULL)
	{
		fprint_remote_signal(f,rem,codes);
		codes++;
	}
	fprint_remote_signal_foot(f,rem);
}


void fprint_remote(FILE *f, struct ir_remote *rem)
{	
	fprint_comment(f,rem);
	fprint_remote_head(f,rem);
	fprint_remote_signals(f,rem);
	fprint_remote_foot(f,rem);
}

void fprint_copyright(FILE *fout)
{
	/* As this program is distributed under GPL you could just
	   remove this copyright notice and the config files generated
	   with the modified program would automatically be covered by
	   the GPL. Although I am aware of this I will not prevent it.

	   I hope that nobody will do so because the license I put on
	   the config files is not really a restriction. Instead it
	   emphasizes the spirit of the GPL to make things available
	   to everybody. */

	fprintf(fout,
		"\n"
		"# Copyright (C) 1999 Christoph Bartelmus\n"
		"#\n"
		"# You may only use this file if you make it available to others,\n"
		"# i.e. if you send it to <lirc@bartelmus.de>\n");
}