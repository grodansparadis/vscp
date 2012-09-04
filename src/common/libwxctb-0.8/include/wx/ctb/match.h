/////////////////////////////////////////////////////////////////////////////
// Name:        match.h
// Purpose:
// Author:      Joachim Buermann
// Id:          $Id: match.h,v 1.1.1.1 2004/11/24 10:30:11 jb Exp $
// Copyright:   (c) 2001 Joachim Buermann
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if !defined ( __MATCH_H )
#define __MATCH_H
/*!
  \file match.h
  \brief public structs and functions for the matching mechanism.

  Some details
*/

/*!
  \def MATCH_MAXSMATCH
  defines the maximum count of tokens, we can use within an expression
  string. At this time, it was limited to 4.
*/
#define MATCH_MAXSMATCH 4

/*!
  \def MATCH_RESULTLEN
  the maximum size of the matched result string. This was set to 64
  for each token.
*/
#define MATCH_RESULTLEN 64	// must be 2^x

/*!
  \struct expr_result match.h

  \brief covers the result of some matching.
*/
struct expr_result {
    char matched[MATCH_RESULTLEN];
    char not_matched[MATCH_RESULTLEN];
    expr_result() {
	matched[0] = '\0';
    };
};

/*!
  \struct S_MATCH match.h

  \brief covers the internal rules for one token of the regular
  expression
  
*/
struct S_MATCH
{
    //! typ of token, 0 : set, 1 : string
    unsigned int t;
    //! error flag, 1 : error
    unsigned int e;
    //! good flag
    unsigned int g;
    /*!
      count of characters, which match the regular expression
    */
    unsigned long i;
    /*!
      count of characters, which don't match the regular expression
    */
    unsigned long j;
    /*!
      minimally count of characters, which have match the regular
      expression. Less than that occurs an error.
    */
    unsigned long n;
    /*!
      maximum count of characters, which can match the regular
      expression. More than that occurs an error.
    */
    unsigned long m;
    /*!
      internal index variable
    */
    unsigned int x;
    /*!
      position dependend flag, 1 means, the token is depended of
      it's position in the input stream. For example: You will match
      the first character, or you say, the first 'x' after some
      whitespaces. In this case, the 'x' match is depended from it's
      position, the flag will be set, after the first character not
      equal with a whitespace. 0 means position is unimportant.
    */
    unsigned int p;
    /*!
      internal index for string compare
    */
    unsigned int si;
    /*!
      internal index for matched characters
    */
    unsigned int ri;
    /*!
      the matching table, will be initiated by parsing one token
      of the regular expression string. After this, every setting bit
      stand for a valid character. So we can set all valid signs in
      32 Byte (8 longs).
    */
    unsigned long mtbl[8];
    /*!
      this is the internal buffer for a searched string.
      \warning the string length (one token) must be lower than
      MATCH_RESULTLEN. At this time, there is no check for a buffer
      overflow.
    */
    char s[MATCH_RESULTLEN];
    /*!
      this is the internal buffer for matched characters.
      \warning Be aware, that there was also no test for a overflow.
    */
    char r[MATCH_RESULTLEN];
};

/*!
  \struct M_MATCH match.h

  \brief covers up to MATCH_MAXMATCH single tokens of the regular
  expression.
  
  This can be enlarge to an upper value. For example: An expression string
  with 4 single tokens is like this: "\c \w+[Ll]ogin\n". Means, we look for
  the word Login or login, followed by a newline and with one or more
  whitespaces at the beginning.

  The tokens are: the whitespaces, the set 'L' oder 'l', thirds the string
  ogin and at last the newline.
*/
struct M_MATCH {
    /*! count of initiated S_MATCH structs */
    int cnt;
    /*! points to the current S_MATCH */
    int cur;			
    /*! here we put the matched characters of all S_MATCH
     */
    char r[MATCH_RESULTLEN * MATCH_MAXSMATCH];
    /*!
      the single S_MATCH, we need one for each token
     */
    S_MATCH S[MATCH_MAXSMATCH];
};

/*!
  \def SBIT(NR,TBL) *(long*)(TBL+(NR>>5)) |= (1L<<(NR%32))
  \brief set the correspondent bit for the character NR in the
  match table pointed by TBL
*/
#define SBIT(NR,TBL) *(long*)(TBL+(NR>>5)) |= (1L<<(NR%32))

/*!
  \def TBIT(NR,TBL) *(long*)(TBL+(NR>>5)) |= (1L<<(NR%32))
  \brief test the correspondent bit for the character NR in the
  match table pointed by TBL
*/
#define TBIT(NR,TBL) *(unsigned long*)(TBL+(NR>>5)) & 1L<<(NR%32)

/*!
  \def MPSH(SM,CHR) SM->r[SM->ri++] = CHR; SM->ri &= (MATCH_RESULTLEN-1)
  \brief puts the character CHR in the internal buffer for matched
  characters and limited to the buffer size.
*/
#define MPSH(SM,CHR) SM->r[SM->ri++] = CHR; SM->ri &= (MATCH_RESULTLEN-1)  
 
/*!
  \fn int match_single(M_MATCH* mm,
                       char* buf,
                       const char* expr_string,
                       struct expr_result* result)
  \brief does a completly match, this includes parsing an expression
  string, initiate the internal M_MATCH and compare this with an
  given string.

  \param mm M_MATCH struct, pointed by mm
  \param buf the input string for matching
  \param expr_string the regular expression
  \param result a struct, which covers internal results of the
  matching process
  \return this function returns three possible values:
  \li +1 : the input string was matched by the expression
  \li  0 : there are not enough characters in the input string, so we
  don't decide if it's matched or not.
  \li -1 : definitly no match
 */
int match_single(M_MATCH* mm,
		 char* buf,
		 const char* expr_string,
		 struct expr_result* result);

/*!
  \fn int match_init(M_MATCH* mm,const char* expr_string)
  \brief initiate the internal match tables and structs with the
  regular expression string.

  \param mm M_MATCH struct, which have to initiate, pointed by mm
  \param expr_string the regular expression
  \return there are three possible values:
  \li +1 expr_string is NULL or empty string
  \li  0 ok
  \li -1 there occurs an error in the expr_string
*/
int match_init(M_MATCH* mm,const char* expr_string);

/*!
  \fn int match(M_MATCH* mm,char* buf,struct expr_result* result)
  \brief after initiate match compares a string with the internal
  match tables.

  \param mm points to the struct with the internal match tables
  \param buf the input string
  \param result if not NULL, there are a lot of informations
  for this match.
  \return this function returns three possible values:
  \li +1 : the input string was matched by the expression
  \li  0 : there are not enough characters in the input string, so we
  don't decide if it's matched or not.
  \li -1 : definitly no match
*/ 
int match(M_MATCH* mm,char* buf,struct expr_result* result);

/*!
  \fn char* matched(M_MATCH* mm,unsigned int expr_no)
  \brief returns the matching result of an internal match token.
  \param mm points to a the M_MATCH struct, which was used for
  matching
  \param expr_no the token nummer within mm (means the token in
  the expression string)
  \return a pointer to the internal buffer of matched characters
  in the given token
*/
char* matched(M_MATCH* mm,unsigned int expr_no);

#endif
