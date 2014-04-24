/*
 * Winamp httpQ Plugin
 * Copyright (C) 1999-2003 Kosta Arvanitis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Kosta Arvanitis (karvanitis@hotmail.com)
 */

#include "stdafx.h"
#include "gen_winampvscp.h"

#include "util.h"
#include <shellapi.h>
#include <stdlib.h> 
#include <stdio.h>


// find the last occurrence of t in s.
/*
char *strrstr(const char *s, const char *t)
{
  size_t slen, tlen;
  const char *p;

  if(!s || !t)
    return NULL;

  slen = strlen(s);
  tlen = strlen(t);

  if(tlen > slen)
    return NULL;

  for(p = s + (slen - tlen); p >= s; --p)
  {
    if(*p == *t && strncmp(p, t, tlen) == 0)
      return (char *)p;
  }

  return NULL;
}
*/

// find the last occurrence of t in s
int lastindexof(const char *s, const char *t)
{
  size_t slen, tlen;
  const char *p;

  if(!s || !t)
    return -1;

  slen = strlen(s);
  tlen = strlen(t);

  if(tlen > slen)
    return -1;

  for(p = s + (slen - tlen); p >= s; --p)
  {
    if(*p == *t && strncmp(p, t, tlen) == 0)
      return p - s;
  }

  return -1;
}

char *strmemdup(const char *s, size_t n)
{
    char *res;

    if (s == NULL)
        return NULL;

    res = (char *)malloc(n + 1);
    memcpy(res, s, n);
    res[n] = '\0';
    
    return res;
}

void strmemfree(const char *s)
{
    if (s != NULL)
        free((void *)s);
}

// convert s to lower case 
char *strtolower(char *s)             
{
	char *ret = s ;
	for ( ; *s; s++) 
    {
        if(isupper(*s)) 
            *s = tolower(*s);
    }
	return ret;
}

// convert a null terminated ascii string for a hex number to its integer value.  
int axtoi(const char *x)
{
	int ch, i = 0;

    while((ch = *x++) != 0) 
	{ 
		if(isdigit(ch))
            ch -= '0';
		else if(isupper(ch))
            ch += (10 - 'A'); 
        else
            ch += (10 - 'a');
        i = (i << 4) + ch;
    }
    return i; 
}

// decodes a url string
void urldecode(char *str)
{
	char tmp[2];

	int j = strlen(str);
	for(int i=0; i < j; i++)
	{
		if(str[i] == '%' && (i+2) < j)
		{
			tmp[0] = str[i+1]; tmp[1] = str[i+2]; tmp[2] = '\0';
			str[i] = axtoi(tmp);

			strcpy(&str[i+1], &str[i+3]);
			j -= 2;
		}
	}
}


bool openurl(const char *url)
{
	return ((long)ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL) <= 32);
}


bool filexists(char *filepath)
{
	OFSTRUCT stFile; 
	HFILE hFile; 
	sprintf(stFile.szPathName, "%s", filepath);
	hFile = OpenFile(filepath,  &stFile, OF_EXIST);
	return(hFile != HFILE_ERROR);
}

