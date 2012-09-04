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
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "winlirc.h"
#include "remote.h"
#include "config.h"
#include "globals.h"

void **init_void_array(struct void_array *ar,size_t chunk_size, size_t item_size)
{
	ar->chunk_size=chunk_size;
	ar->item_size=item_size;
	ar->nr_items=0;
	if(!(ar->ptr=calloc(chunk_size, ar->item_size))){
		DEBUG("out of memory\n");
		//free(ar->ptr);
		parse_error=1;
		return(NULL);
	}
	return((void **)(ar->ptr));
}

int add_void_array (struct void_array *ar, void * dataptr)
{
	void *ptr;
	
	if ((ar->nr_items%ar->chunk_size)==(ar->chunk_size)-1){
		/* I hope this works with the right alignment,
		if not we're screwed */
		if (!(ptr=realloc(ar->ptr,ar->item_size*((ar->nr_items)+(ar->chunk_size+1))))){
			DEBUG("out of memory\n");
			parse_error=1;
			return(0);
		}
		ar->ptr=ptr;
	}
	memcpy((char *)(ar->ptr)+(ar->item_size*ar->nr_items), dataptr, ar->item_size);
	ar->nr_items=(ar->nr_items)+1;
	memset((char *)(ar->ptr)+(ar->item_size*ar->nr_items), 0, ar->item_size);
	return(1);
}

inline void *get_void_array(struct void_array *ar)
{
    return(ar->ptr);
}

void *s_malloc(size_t size)
{
    void *ptr;
    if((ptr=malloc(size))==NULL){
        DEBUG("out of memory\n");
        parse_error=1;
        return(NULL);
    }
    memset(ptr, 0, size);
    return (ptr);
}

inline char *s_strdup(char * string)
{
    char *ptr;
    if(!(ptr=strdup(string))){
        DEBUG("out of memory\n");
        parse_error=1;
        return(NULL);
    }
    return (ptr);
}

/* my very own strtouq */
unsigned __int64 strtouq(char *val, char **endptr, int base)
{
	while(*val=='\t' || *val==' ') val++;
	if(base==0)
		if(val[0]=='0')
			if(val[1]=='x' || val[1]=='X')
			{
				base=16;
				val+=2;
			}
			else
			{
				val++;
				base=8;
			}
		else
			base=10;
	
	char convert[256];
	for(int i=0;i<255;i++)
	{
		if(i>='0' && i<='9') convert[i]=i-'0';
		else if(i>='a' && i<='f') convert[i]=(i-'a')+10;
		else if(i>='A' && i<='F') convert[i]=(i-'A')+10;
		else convert[i]=-1;
	}

	unsigned __int64 result=0;
	while(*val && convert[*val]!=-1)
	{
		result*=base;
		result+=convert[*val];
		val++;
	}
	*endptr=val;
	return result;
}


inline ir_code s_strtocode(char *val)
{
	ir_code code=0;
	char *endptr;

	errno=0;
#ifdef LONG_IR_CODE
	code=strtouq(val,&endptr,0);
	if((code==(unsigned __int64) -1 && errno==ERANGE) ||
	    strlen(endptr)!=0 || strlen(val)==0)
	{
		DEBUG("error in configfile line %d:\n",line);
		DEBUG("\"%s\": must be a valid (unsigned long long) number\n",val);
		parse_error=1;
		return(0);
	}
#else
	code=strtoul(val,&endptr,0);
	if(code==ULONG_MAX && errno==ERANGE)
	{
		DEBUG("error in configfile line %d\n",line);
		DEBUG("code is out of range\n");
		DEBUG("try compiling lircd with the LONG_IR_CODE option\n");
		parse_error=1;
		return(0);
	}
	else if(strlen(endptr)!=0 || strlen(val)==0)
	{
		DEBUG("error in configfile line %d:\n",line);
		DEBUG("\"%s\": must be a valid (unsigned long) number\n",val);
		parse_error=1;
		return(0);
	}
#endif
	return(code);
}

unsigned long s_strtoul(char *val)
{
	unsigned long n;
	char *endptr;

	errno=0;
	n=strtoul(val,&endptr,0);
	if((n==ULONG_MAX && errno==ERANGE)
	   || strlen(endptr)!=0 || strlen(val)==0)
	{
		DEBUG("error in configfile line %d:\n",line);
		DEBUG("\"%s\": must be a valid (unsigned long) number\n",val);
		parse_error=1;
		return(0);
	}
	return(n);
}

int s_strtoi(char *val)
{
	char *endptr;
	long n;
	int h;
	
	errno=0;
	n=strtol(val,&endptr,0);
	h=(int) n;
	if(((n==LONG_MAX || n==LONG_MIN) && errno==ERANGE)
	   || strlen(endptr)!=0 || strlen(val)==0 || n!=((long) h))
	{
		DEBUG("error in configfile line %d:\n",line);
		DEBUG("\"%s\": must be a valid (int) number\n",val);
		parse_error=1;
		return(0);
	}
	return(n);
}

unsigned int s_strtoui(char *val)
{
        char *endptr;
        unsigned long n;
        unsigned int h;

        errno=0;
		n=strtoul(val,&endptr,0);
        h=(unsigned int) n;
		if((n==ULONG_MAX && errno==ERANGE)
		   || strlen(endptr)!=0 || strlen(val)==0 || n!=((unsigned long) h))
        {
                DEBUG("error in configfile line %d:\n",line);
                DEBUG("\"%s\": must be a valid (unsigned int) number\n",val);
                parse_error=1;
                return(0);
        }
        return(n);
}

int checkMode(int is_mode, int c_mode, char *error)
{

    if (is_mode!=c_mode)
	{
		DEBUG("error in configfile line %d:\n",line);
		DEBUG("\"%s\" isn´t valid at this position\n",error);
		parse_error=1;
		return(0);
	}
    return(1);
}

int addSignal(struct void_array *signals, char *val)
{
	unsigned long t;
	
	t=s_strtoul(val);
	if (parse_error) return(0);
	if (!add_void_array(signals, &t)){
		return(0);
	}
    return(1);
}
	      
struct ir_ncode *defineCode(char *key, char *val, struct ir_ncode *code)
{
        code->name=s_strdup(key);
        code->code=s_strtocode(val);
        return(code);
}

int parseFlags(char *val,char *val2)
{
        struct flaglist *flaglptr;
	int flags=0;
	char *flag,*help;

	if(val2!=NULL)
	{
		DEBUG("error in configfile line %d:\n",line);
		DEBUG("bad flags \"%s %s\"\n",val,val2);
		parse_error=1;
		return(0);
	}
	
	flag=help=val;


	while(flag!=NULL)
	{
		while(*help!='|' && *help!=0) help++;
		if(*help=='|')
		{
			*help=0;help++;
		}
		else
		{
			help=NULL;
		}
	
		flaglptr=all_flags;
		while(flaglptr->name!=NULL){
			if(strcasecmp(flaglptr->name,flag)==0){
				flags=flags|flaglptr->flag;
				break;
			}
			flaglptr++;
		}
		if(flaglptr->name==NULL)
		{
			DEBUG("error in configfile line %d:\n",line);
			DEBUG("unknown flag: \"%s\"\n",flag);
			parse_error=1;
			return(0);
		}
		flag=help;
	}

        return(flags);
}

void defineRemote(char * key, char * val, char *val2, struct ir_remote *rem)
{
	if ((strcasecmp("name",key))==0){
		if(rem->name!=NULL) free(rem->name);
		rem->name=s_strdup(val);
	}

	else if ((strcasecmp("bits",key))==0){
		rem->bits=s_strtoi(val);
	}

	else if (strcasecmp("flags",key)==0){
		rem->flags|=parseFlags(val,val2);
	}

	else if (strcasecmp("eps",key)==0){
		rem->eps=s_strtoi(val);
	}

	else if (strcasecmp("aeps",key)==0){
		rem->aeps=s_strtoi(val);
	}

	else if (strcasecmp("header",key)==0){
		rem->phead=s_strtoi(val);
		rem->shead=s_strtoi(val2);
	}

	else if (strcasecmp("one",key)==0){
		rem->pone=s_strtoi(val);
		rem->sone=s_strtoi(val2);
	}

	else if (strcasecmp("zero",key)==0){
		rem->pzero=s_strtoi(val);
		rem->szero=s_strtoi(val2);
	}

	else if (strcasecmp("plead",key)==0){
		rem->plead=s_strtoi(val);
	}

	else if (strcasecmp("ptrail",key)==0){
		rem->ptrail=s_strtoi(val);
	}

	else if (strcasecmp("foot",key)==0){
		rem->pfoot=s_strtoi(val);
		rem->sfoot=s_strtoi(val2);
	}

	else if (strcasecmp("repeat",key)==0){
		rem->prepeat=s_strtoi(val);
		rem->srepeat=s_strtoi(val2);
	}

	else if (strcasecmp("pre_data_bits",key)==0){
		rem->pre_data_bits=s_strtoi(val);
	}

	else if (strcasecmp("pre_data",key)==0){
		rem->pre_data=s_strtocode(val);
	}

	else if (strcasecmp("post_data_bits",key)==0){
		rem->post_data_bits=s_strtoi(val);
	}

	else if (strcasecmp("post_data",key)==0){
		rem->post_data=s_strtocode(val);
	}

	else if (strcasecmp("pre",key)==0){
		rem->pre_p=s_strtoi(val);
		rem->pre_s=s_strtoi(val2);
	}

	else if (strcasecmp("post",key)==0){
		rem->post_p=s_strtoi(val);
		rem->post_s=s_strtoi(val2);
	}

	else if (strcasecmp("gap",key)==0){
		rem->gap=s_strtoul(val);
	}
	
	else if (strcasecmp("repeat_gap",key)==0){
		rem->repeat_gap=s_strtoul(val);
	}

	else if (strcasecmp("toggle_bit",key)==0){
        rem->toggle_bit=s_strtoi(val);
	}
	/* obsolete name */
	else if (strcasecmp("repeat_bit",key)==0){
        rem->toggle_bit=s_strtoi(val);
	}
	else if (strcasecmp("min_repeat",key)==0){
        rem->min_repeat=s_strtoi(val);
	}
	else if (strcasecmp("frequency",key)==0){
        rem->freq=s_strtoui(val);
	}
	else if (strcasecmp("duty_cycle",key)==0){
        rem->duty_cycle=s_strtoui(val);
	/* WinLIRC ONLY! */
	}else if (strcasecmp("transmitter",key)==0){
        rem->transmitter=s_strtoui(val);
	}else{
		if(val2){
			DEBUG("error in configfile line %d:\n",line);
			DEBUG("unknown definition: \"%s %s %s\"\n",key,val,val2);
		}else{
			DEBUG("error in configfile line %d:\n",line);
			DEBUG("unknown definition: \"%s %s\"\n",key,val);
		}
		parse_error=1;
	}
}
    
struct ir_remote * read_config(FILE *f)
{
	char buf[LINE_LEN+1], *key, *val, *val2;
	int len;
	struct ir_remote *top_rem=NULL,*rem=NULL;
	struct void_array codes_list,raw_codes,signals;
	struct ir_ncode raw_code={NULL,0,0,NULL};
	struct ir_ncode name_code={NULL,0,0,NULL};
	int mode=ID_none;

	codes_list.ptr=raw_codes.ptr=signals.ptr=NULL;
	
	line=0;
	parse_error=0;
	
	while(!parse_error && fgets(buf,LINE_LEN,f)!=NULL)
	{
		line++;
		len=strlen(buf);
		if(len==LINE_LEN && buf[len-1]!='\n')
		{
			DEBUG("line %d too long in config file\n",line);
			parse_error=1;
			break;
		}
		
		/* ignore comments */
		len--;
		if(buf[len]=='\n') buf[len]=0;
		if(buf[0]=='#'){
			continue;
		}
		key=strtok(buf," \t");
		/* ignore empty lines */
		if(key==NULL) continue;
		val=strtok(NULL, " \t");
		if(val!=NULL){
			val2=strtok(NULL, " \t");
			if (strcasecmp("begin",key)==0){
				if (strcasecmp("codes", val)==0){
					/* init codes mode */
					if (!checkMode(mode, ID_remote,
						"begin codes")) { parse_error=1; continue; }
					if (rem->codes){
						DEBUG("error in configfile line %d:\n",line);
						DEBUG("codes are already defined\n");
						parse_error=1; continue;
					}
					
					init_void_array(&codes_list,30, sizeof(struct ir_ncode));
					mode=ID_codes;
				}
				else if(strcasecmp("raw_codes",val)==0){
					/* init raw_codes mode */
					if(!checkMode(mode, ID_remote,
						"begin raw_codes")) { parse_error=1; continue; }
					if (rem->codes){
						DEBUG("error in configfile line %d:\n",line);
						DEBUG("codes are already defined\n");
						parse_error=1; continue;
					}
					rem->flags|=RAW_CODES;
					raw_code.code=0;
					init_void_array(&raw_codes,30, sizeof(struct ir_ncode));
					mode=ID_raw_codes;
				}else if(strcasecmp("remote",val)==0){
					/* create new remote */
					if(!checkMode(mode, ID_none,
						"begin remote")) { parse_error=1; continue; }
					mode=ID_remote;
					if (!top_rem){
						/* create first remote */
						rem=top_rem=(struct ir_remote *)
							s_malloc(sizeof(struct ir_remote));
					}else{
						/* create new remote */
						rem->next=(struct ir_remote *)
							s_malloc(sizeof(struct ir_remote));;
						rem=rem->next;
					}
				}else if(mode==ID_codes){
					add_void_array(&codes_list, defineCode(key, val, &name_code));
				}else{
					DEBUG("error in configfile line %d:\n",line);
					DEBUG("unknown section \"%s\"\n",val);
					parse_error=1;
				}
			}else if (strcasecmp("end",key)==0){
				
				if (strcasecmp("codes", val)==0){
					/* end Codes mode */
					if (!checkMode(mode, ID_codes,
						"end codes")) { parse_error=1; continue; }
					rem->codes=(struct ir_ncode *)
						get_void_array(&codes_list);
					mode=ID_remote;     /* switch back */
					
				}else if(strcasecmp("raw_codes",val)==0){
					/* end raw codes mode */
					
					if(mode==ID_raw_name){
						raw_code.signals=(unsigned long *) //(ir_code *)
							get_void_array(&signals);
						raw_code.length=signals.nr_items;
						if(raw_code.length%2==0)
						{
							DEBUG("error in configfile line %d:\n",line);
							DEBUG("bad signal length\n");
							parse_error=1;
						}
						if(!add_void_array(&raw_codes, &raw_code))
						{ parse_error=1; continue; }
						mode=ID_raw_codes;
					}
					if(!checkMode(mode,ID_raw_codes,
						"end raw_codes")) { parse_error=1; continue; }
					rem->codes=(struct ir_ncode *)
						get_void_array(&raw_codes);
					mode=ID_remote;     /* switch back */
				}else if(strcasecmp("remote",val)==0){
					/* end remote mode */
					/* print_remote(rem); */
					if (!checkMode(mode,ID_remote,
						"end remote")) { parse_error=1; continue; }
					if (!rem->name){
						DEBUG("you must specify a remote name\n");
						parse_error=1; continue;
					}
					
					/* not really necessary because we
					clear the alloced memory */
					rem->next=NULL;
					rem->last_code=NULL;
					mode=ID_none;     /* switch back */
					if(has_repeat_gap(rem) && 
						is_const(rem))
					{
						DEBUG("WARNING: repeat_gap will be ignored if CONST_LENGTH flag is set\n");
					}
				}else if(mode==ID_codes){
					add_void_array(&codes_list, defineCode(key, val, &name_code));
				}else{
					DEBUG("error in configfile line %d:\n",line);
					DEBUG("unknown section \"%s\"\n",val);
					parse_error=1;
					
				}
			} else {
				
				switch (mode){
				case ID_remote:
					defineRemote(key, val, val2, rem);
					break;
				case ID_codes:
					add_void_array(&codes_list, defineCode(key, val, &name_code));
					break;
				case ID_raw_codes:
				case ID_raw_name:
					if(strcasecmp("name",key)==0){
						if(mode==ID_raw_name)
						{
							raw_code.signals=(unsigned long *) //(ir_code *)
								get_void_array(&signals);
							raw_code.length=signals.nr_items;
							if(raw_code.length%2==0)
							{
								DEBUG("error in configfile line %d:\n",line);
								DEBUG("bad signal length\n");
								parse_error=1;
							}
							if(!add_void_array(&raw_codes, &raw_code))
								{ parse_error=1; continue; }
						}
						if(!(raw_code.name=s_strdup(val))){
							{ parse_error=1; continue; }
						}
						raw_code.code++;
						init_void_array(&signals,50,sizeof(unsigned long));
						mode=ID_raw_name;
					}else{
						if(mode==ID_raw_codes)
						{
							DEBUG("no name for signal defined at line %d\n",line);
							{ parse_error=1; continue; }
						}
						if(!addSignal(&signals, key)) { parse_error=1; continue; }
						if(!addSignal(&signals, val)) { parse_error=1; continue; }
						if (val2){
							if (!addSignal(&signals, val2)){
								parse_error=1; continue;
							}
						}
						while ((val=strtok(NULL," \t"))){
							if (!addSignal(&signals, val)) { parse_error=1; continue; }
						}
					}
					break;
				}
			}
		}else if(mode==ID_raw_name){
			if(!addSignal(&signals, key)){
				{ parse_error=1; continue; }
			}
		}else{
			DEBUG("error in configfile line %d.\n",line);
			{ parse_error=1; continue; }
		}
	}
	if(mode!=ID_none)
	{
		switch(mode)
		{
		case ID_raw_name:
			   if(raw_code.name!=NULL)
			   {
					   free(raw_code.name);
					   if(get_void_array(&signals)!=NULL)
							   free(get_void_array(&signals));
			   }
		case ID_raw_codes:
			   rem->codes=(struct ir_ncode *)get_void_array(&raw_codes);
			   break;
		case ID_codes:
			   rem->codes=(struct ir_ncode *)get_void_array(&codes_list);
			   break;
		}
		if(!parse_error)
		{
			   DEBUG("unexpected end of file\n");
			   parse_error=1;
		}
	}
	if (parse_error){
		free_config(top_rem);
		top_rem=NULL;
	}

	return (top_rem);
}

void free_config(struct ir_remote *remotes)
{
	struct ir_remote *next;
	struct ir_ncode *codes;
	
	while(remotes!=NULL)
	{
		next=remotes->next;

		if(remotes->name!=NULL) 
			free(remotes->name);
		if(remotes->codes!=NULL)
		{
			codes=remotes->codes;
			while(codes->name!=NULL)
			{
				free(codes->name);
				if(codes->signals!=NULL)
					free(codes->signals);
				codes++;
			}
			free(remotes->codes);
		}
		free(remotes);
		remotes=next;
	}
}
