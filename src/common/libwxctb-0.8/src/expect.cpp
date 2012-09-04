/////////////////////////////////////////////////////////////////////////////
// Name:        expect.cpp
// Purpose:
// Author:      Joachim Buermann
// Id:          $Id: expect.cpp,v 1.1.1.1 2004/11/24 10:30:11 jb Exp $
// Copyright:   (c) 2001 Joachim Buermann
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>

#include "wx/ctb/expect.h"
#include "wx/ctb/timer.h"

//#define DEBUG

Expect::Expect(wxIOBase* device)
{
    // default is 1 second
    timeout = 1000;
    pDevice = device;
    // abort_string and retry_string are very special, at
    // default, you never use it.
    abort_string = (char*)NULL;
    retry_string = (char*)NULL;
    retry_count = 3;
};

Expect::~Expect()
{
};

int Expect::Abort(const char* abortstring,void*(*f)(void*))
{
    abort_string = (char*)abortstring;
    abort_fnc = f;
    return 0;
};

/*!
  there are more than one single match tokens (S_MATCH),
  so we have to compound everyone to get the completly
  matched string. Destination is the buffer in M_MATCH.
  \todo checking for a buffer overflow in Expect::Received.
*/
char* Expect::Received()
{
    //! only for the last match
    for(int i=0;i<mm.cnt;i++) {
	   strcat(mm.r,mm.S[i].r);
    }
    mm.r[sizeof(mm.r)-1] = 0;
    return mm.r;
};

/*!
  Set the retry string and retry count.
*/
int Expect::Retry(const char* retry_string_,int retry_count_)
{
    retry_count = retry_count_;
    retry_string = (char*)retry_string_;
    return 0;
};

/*!
  \todo the match initialisation is waste for the same expectstr.
  In this circumstance, the matching struct only have to preset,
  not to reformated.
*/
int Expect::Send(char* sendstr,char* expectstr)
{
    int nread,result = -1;
    timeout_flag = 0;
    // a full initialisation of the match mechanism, perhaps
    // we can beware of this in some cases.
    match_init(&mm,expectstr);

    // the timeout is realized with the timer class
    timer TIMER(timeout,&timeout_flag,(void*(*)(void*))NULL);
    // a timeout interval with 0 never starts the timer, means
    // an endless timeout.
    if(timeout) TIMER.start();
    
    // write all available data throughout the device. This have to
    // do completly, so we used the blocked write version.
    pDevice->Writev(sendstr,strlen(sendstr),&timeout_flag);
    // oops, timeout occured during writing
    if(timeout_flag) {
	   return -1;
    }

    // reading data from the device until timeout is occured
    // or the expected data is definitive successful or definitive
    // wrong.
    while(!timeout_flag) {
#ifdef __WIN32__
	   // without this call Send will blocks (only win32)
	   // SleepEx get the current thread go to sleep, the first
	   // parameter is the sleeping time in milliseconds. The
	   // second parameter must be FALSE, so neither a WriteFileEx
	   // nor ReadFileEx can wakeup the thread before the sleeping
	   // time.
	   SleepEx(1,FALSE);
#endif
	   // look for received data, nread is the count of really
	   // received data, maximum is sizeof buf.
	   nread = pDevice->Read(buf,sizeof(buf)-1);	
	   if(nread > 0) {
		  // data were received, terminate the buffer
		  buf[nread] = '\0';
#ifdef DEBUG
		  // not very nice debug, i will replace it
		  printf("received: %i(%s)\n",nread,buf);
#endif
		  // match the current data
		  result = match(&mm,buf,&ExprResult);
#ifdef DEBUG
		  // debug dito
		  printf("match result: %i\n",result);
#endif
		  if(result == 0) {
			 // not enough data to come to a clear decision,
			 // read next
			 continue;
		  }
		  // result is -1 (wrong answer) or 1 (matched answer)
		  return result;
	   }
	   if(nread < 0) {
		  // a communication error occurs, we handled this as a
		  // wrong answer. It's the job of the caller program,
		  // to handle this situation.
		  result = -1;
		  break;
	   }
    }
    if(timeout_flag) {
	   // a timeout occurs
#ifdef DEBUG
	   printf("timeout occured\n");
#endif
	   result = -1;
    }
    return result;
};

int Expect::Sendv(char* sendstr,char* expectstr)
{
    int n = 0;
    while(n++ < retry_count) {
	   if(Send(sendstr,expectstr) > 0) {
		  char *cp = Received();
		  if(abort_string && !strcmp(abort_string,cp)) {
#ifdef DEBUG
			 printf("abort_fnc (%s)\n",cp);
#endif
			 abort_fnc(NULL);
			 return -1;
		  }
		  if(retry_string && !strcmp(retry_string,cp)) {
#ifdef DEBUG
			 printf("retry last command (%i)\n",n);
#endif
			 continue;
		  }
		  // neither retry nor abort, also ok
		  return 1;
	   }
    }
    // sorry, but sending several times also gives an error
    return -1;
};

int Expect::Timeout(int msecs)
{
    timeout = msecs;
    // this function doesn't fails
    return 0;
};
