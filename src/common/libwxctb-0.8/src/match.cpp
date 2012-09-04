/////////////////////////////////////////////////////////////////////////////
// Name:        match.cpp
// Purpose:
// Author:      Joachim Buermann
// Id:          $Id: match.cpp,v 1.1.1.1 2004/11/24 10:30:11 jb Exp $
// Copyright:   (c) 2001 Joachim Buermann
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "wx/ctb/match.h"

static int denotes_range(S_MATCH*,char**);
static int quoted_char(S_MATCH*,char**);
static void sm_negate(S_MATCH*);

//
// ASCII Tabelle for the  SBIT and TBIT macros
//
/*
  0(8) 1(9) 2(A) 3(B) 4(C) 5(D) 6(E) 7(F)

  0x00  0x00 0x01 0x02 0x03 0x04 0x05 0x06 BEL
  BS   TAB  LF   VT   FF   CR   0x0E 0x0F
  0x10  0x10 0x11 0x12 0x13 0x14 0x15 0x16 0x17
  0x18 0x19 0x1A 0x1B 0x1C 0x1D 0x1E 0x1F

  0x20       !    "    #    $    %    &    '
  (    )    *    +    ,    -    .    /
  0x30  0    1    2    3    4    5    6    7
  8    9    :    ;    <    =    >    ?

  0x40  @    A    B    C    D    E    F    G
  H    I    J    K    L    M    N    O
  0x50  P    Q    R    S    T    U    V    W
  X    Y    Z    [    \    ]    ^    _

  0x60  `    a    b    c    d    e    f    g
  h    i    j    k    l    m    n    o
  0x70  p    q    r    s    t    u    v    w
  x    y    z    {    |    }    ~    0x7F
*/

//
// includes all characters, which don't allowed in a
// regular expression
//
long stbl[8] = {
    0L,
    0x00004F00,			// 00000000 00000000 01001111 00000000 
    0x68000000,			// 01101000 00000000 00000000 00000000
    0L,
    0L,
    0L,
    0L,
    0L
};

//
// includes the repeat operators +,*,{,}
//
long vtbl[8] = {
    0L,
    0x00000C00,			// 00000000 00000000 00001100 00000000
    0L,
    0x08000000,			// 00101000 00000000 00000000 00000000
    0L,
    0L,
    0L,
    0L
};

/*
  Parsing one condition of the regular   
  During this replace the pointer cp to the next.
  sm pointer to a struct, which obtains the result
  of the parsing
  cp a pointer to the next char in the regular expression
  string.
  returns zero on success, otherwise one
*/
static int match_compile_one(S_MATCH *sm,char*& cp)
{
    int negate = 0;

    // initiate mtbl, first nothing is allowd
    sm->n = sm->m = 1;
    // parse expr_string
    while(*cp) {
	   // if the current S_MATCH token is a string, special
	   // characters will abort the function
	   if(sm->t && TBIT(*cp,stbl)) return 0;
	   switch(*cp) {
	   case '[':		// here comes a set
		  cp++;
		  if(*cp == '^') {
			 negate = 1;
			 cp++;
		  }
		  while(*cp != ']') {
			 // oops, we have a set without a closed bracket
			 // this is an error
			 if(*cp == 0) return 1;
			 if(*cp == '\\') {
				// a quoted sign like \n for newline
				cp++;
				if(quoted_char(sm,&cp)) return 1;
			 }
			 else {
				SBIT(*cp,sm->mtbl);
			 }
			 cp++;
		  }
		  cp++;		// cp points after ']'
		  if(TBIT(*cp,vtbl)) continue;
		  goto END_WHILE;
	   case '+':		// one ore more
		  sm->n = 1;
		  sm->m = 0xFFFFFFFFL;
		  cp++;		// cp points after '+'
		  goto END_WHILE;
	   case '*':		// zero or more
		  sm->n = 0;
		  sm->m = 0xFFFFFFFFL;
		  cp++;		// cp points after '*'
		  goto END_WHILE;
	   case '{':		// a counter for the previous token
		  cp++;
		  if(denotes_range(sm,&cp)) return 1;
		  return 0;
	   case '^':		// the first position must match
		  sm->p = 1;
		  break;
	   case '.':
		  // all characters
		  memset(sm->mtbl,0xff,sizeof(sm->mtbl));
		  // except the not printable
		  *(long*)(sm->mtbl) = 0L;
		  cp++;
		  goto END_WHILE;
	   default:
		  // test if there follows a repeat operator. In this case
		  // the string finished at this point
		  if(TBIT(*(cp+1),vtbl)) {
			 if(!sm->t) {
				// this was the first character
				SBIT(*cp,sm->mtbl);
				break;
			 }
			 else {
				// the string was finished, the current sign
				// will be used for the next condition
				return 0;
			 }
		  }
		  else {
			 // all normal characters will be append for a string
			 // normal means: no special chars
			 sm->t = 1;
			 sm->s[sm->x++] = *cp;
			 sm->s[sm->x] = '\0';
		  }
		  break;
	   }
	   cp++;
    }
 END_WHILE:
    if(negate) {
	   sm_negate(sm);
	   negate = 0;
    }
    return 0;
};

/*
  match_one only be called from internal. There is no use,
  to call this function outside this file.
*/
static int match_one(S_MATCH* sm,
				 char*& string,
				 struct expr_result* result)
{
    // it's a string
    if(sm->t) {
	   while(*string) {
		  // compare string with match table byte as byte
		  if(*string != sm->s[sm->si]) {
			 if(sm->si < sm->x) sm->e = 1;
			 return -1;
		  }
		  sm->si++;
		  MPSH(sm,*string);
		  string++;
		  sm->g = 1;
	   }
	   return 0;
    }
    // it's a single char or a set of chars 
    while(*string) {
	   if(TBIT(*string,sm->mtbl)) {
		  // increment counter for matched chars
		  sm->i++;
		  // after the first matched char, we must set
		  // the position dependence flag
		  sm->p = 1;
		  sm->g = 1;
		  // compare with the allowed count
		  if(sm->i > sm->m) {
			 return -1;	// to many chars
		  }
		  MPSH(sm,*string);
		  string++;
		  if(sm->n == sm->m == sm->i) {
			 return 1;
		  }
	   }
	   else {
		  if(!sm->i && !sm->n) {
			 sm->g = 1;
			 return -1;
		  }
		  if((sm->i < sm->n) || (sm->i > sm->m))
			 sm->e = 1;
		  if(sm->p) {
			 return -1;
		  }
	   }
    }
    return 0;
}


int match_single(M_MATCH* mm,
			  char* cp,
			  const char* expr_string,
			  struct expr_result* result)
{
    if(match_init(mm,expr_string)) {
	   return -1;
    }
    return match(mm,cp,result);
}

/*
  returns three possible values:
  +1 : the input string was matched by the expression
  0 : there are not enough characters in the input string, so we
  don't decide if it's matched or not.
  -1 : definitly no match
*/
int match(M_MATCH* mm,char* cp,struct expr_result* result)
{
    int res;
    while(mm->cur < mm->cnt) {
	   if((res = match_one(&mm->S[mm->cur],cp,result)) < 0) {
		  // no more testing
		  if(mm->S[mm->cur].e) return -1;
		  if(!mm->S[mm->cur].g) return -1;
		  else {
			 mm->cur++;	// next condition
			 continue;
		  }
	   }
	   // condition for substrings
	   if(!*cp) return res;
    }
    // all conditions were tested
    if(result) {
	   memcpy(result->matched,mm->r,MATCH_RESULTLEN * MATCH_MAXSMATCH);
    }
    return 1;
}

/*
  before we can match a string, we first have to analyse the
  regular expression string.
*/
int match_init(M_MATCH* mm,const char* expr_string)
{
    S_MATCH* sm;

    memset(mm,0,sizeof(M_MATCH));

    if(expr_string && *expr_string) {
	   char *cp = (char*)expr_string;
	   while(*cp) {
		  if(mm->cnt >= MATCH_MAXSMATCH) {
			 // to many conditions (means tokens) in the
			 // regular expression
			 return -1;
		  }
		  sm = &mm->S[mm->cnt];
		  if(match_compile_one(sm,cp)) {
			 // an error occurs (syntax error)
			 return -1;
		  }
		  if(mm->cnt) {
			 // after the first token, the next one's are
			 // have to depending on the character position!!!
			 sm->p = 1;
		  }
		  mm->cnt++;
	   }
	   return 0;
    }
    // expr_string was NULL or an empty string
    return 1;
};

char* matched(M_MATCH* mm,unsigned int expr_no)
{
    // invalid S_MATCH index results in a null pointer
    if((int)expr_no >= mm->cnt) return 0L;
    // the result maybe a empty string, but the pointer
    // have to point to a valid adress
    return mm->S[expr_no].r;
};

/*
  here we handle a given const repeat operator
*/
int denotes_range(S_MATCH* sm,char** cp)
{
    char* err;

    sm->n = strtol(*cp,&err,10);
    sm->m = sm->n;
    if(err) {
	   if(*err == ',') {
		  sm->m = strtol(++err,&err,10);
	   }
	   if(*err != '}') return 1;
	   else *cp = ++err;
    }
    return 0;
};

/*
  here we handle quoted chars
*/
int quoted_char(S_MATCH* sm,char** cp)
{
    switch(**cp) {
    case 'd':
	   // match all numbers
	   *(long*)(sm->mtbl + 1) |= 0x03FF0000;
	   break;
    case 'D':
	   // match all printable chars except to the numbers
	   memset(sm->mtbl,0xFF,sizeof(sm->mtbl));
	   *(long*)(sm->mtbl + 1) |= ~0x03FF0000;
	   break;
    case 'n':
	   SBIT('\n',sm->mtbl);
	   break;
    case 'r':
	   SBIT('\r',sm->mtbl);
	   break;
    case 's':
	   // white spaces (ascii(9-13) and ' ')
	   *(long*)(sm->mtbl) |= 0x007C0000;
	   SBIT(' ',sm->mtbl);
	   break;
    case 'w':
	   // all alpha numeric
	   *(long*)(sm->mtbl + 1) |= 0x03FF0000;
	   *(long*)(sm->mtbl + 2) |= 0x7FFFFFE0;
	   *(long*)(sm->mtbl + 3) |= 0x7FFFFFE0;
	   break;
    case 'W':
	   // all not alpha numeric
	   *(long*)(sm->mtbl + 1) |= 0x03FF0000;
	   *(long*)(sm->mtbl + 2) |= 0x7FFFFFE0;
	   *(long*)(sm->mtbl + 3) |= 0x7FFFFFE0;
	   sm_negate(sm);
	   break;
    default:
	   SBIT(**cp,sm->mtbl);	
	   break;
    }
    *cp++;
    return 0;
}

/*
  toggle all bits in the given match table. Due to all allowed chars
  are now forbidden, and all forbidden chars will be allowed.
*/
void sm_negate(S_MATCH* sm)
{
    unsigned long *lp = sm->mtbl;
    for(int i=0;i<8;i++) {
	   *lp++ ^= 0xFFFFFFFFL;
    }
}
