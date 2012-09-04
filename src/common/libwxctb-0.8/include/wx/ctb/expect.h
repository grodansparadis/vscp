/////////////////////////////////////////////////////////////////////////////
// Name:        expect.h
// Purpose:
// Author:      Joachim Buermann
// Id:          $Id: expect.h,v 1.1.1.1 2004/11/24 10:30:11 jb Exp $
// Copyright:   (c) 2001 Joachim Buermann
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if !defined ( __EXPECT_H )
#define __EXPECT_H

#include "wx/ctb/iobase.h"
#include "wx/ctb/match.h"

/*!
  \class Expect
  Expect connects a device with a regular expression matching 
  mechanism, to send a given string an checks the answer against
  certain attributes. The device must be derivated from class dev.
  Expect handles the timeout and can repeats the sending string
  automatically, if there was received a special answer. (This
  feature is very especially for our instruments, which answer
  with a special char in different states).
*/
class Expect
{
protected:
    /*!
      The internal timeout. The default is 1 second, but you can
      change this with the Timeout member function.
    */
    int timeout;
    /*!
      The timeout flag for the internal timer. \sa class timer.
    */
    int timeout_flag;
    /*!
      The connected device
    */
    wxIOBase* pDevice;
    /*!
      The internal buffer, in which Expect writes the received data.
      The buffer is limited, but because of the stream based matching
      mechanism, there is no restriction in this circumstance.
    */
    char buf[256];
    /*!
      the answer, which will aborting the communication. This reflects
      a special attribute of our instruments. The default is an empty
      string, so you can ignore this.
    */
    char* abort_string;
    /*!
      same like the abort_string, only repeats the last broadcasting.
      The default is empty, and you can ignore this too.
    */
    char* retry_string;
    /*!
      a struct from the matching mechanism. In this you get the
      result of the last match.
    */
    expr_result ExprResult;
    /*!
      an internal data struct for the matching mechanism. With Expect,
      you don't need this. But look in file match.h for more
      information.
    */
    M_MATCH mm;
    /*!
      I don't know, if anybody can need this (myself too). So don't
      use it, because it will be waste and removed.
    */
    void* (*abort_fnc)(void*);
    /*!
      the internal retry counter, the default is 3. Due to a sending
      string will be repeat several times until the communciation
      is canceled.
    */
    int retry_count;
public:
    /*!
      the constructor. Needs a reference of a derivated class of dev.
      \param device the communication class
    */
    Expect(wxIOBase* device);
    /*!
      the destructor, at the moment he makes nothing.
    */
    ~Expect();
    /*!
      \warning this function is waste, so don't use it. I will removed it
      next time.
    */
    int Abort(const char* abort_string,void*(*f)(void*));
    /*!
      \warning obsolete, don't use this.
    */
    char* Matched() {return ExprResult.matched;};
    /*!
      \warning obsolete, don't use this.
    */
    char* NotMatched() {return ExprResult.not_matched;};
    /*!
      defines a string, which causes to retry the last sending.
      \param retry_string the answer string to signal a repeat
      \param count maximum count of repeats, default is three
      \return this function doesn't fail, so it everytime returns
      zero (no errors).
    */
    int Retry(const char* retry_string,int count = 3);
    /*!
      the main function of Expect class. It will send the null
      terminated string sendstr and wait for an answer, which
      must match the expectstr. This function blocks for the
      tune in timeout.
      \param sendstr the string, you will send through the device
      \param expectstr a regular expression (or only a simple
      string), which paraphrase the expected answer.
      \return three states:
      \li -1 the received data doesn't match the expectstr
      \li 0 means, there are not enough received  data, to come 
      to a clear decision
      \li 1 the received data match the expected string
    */
    int Send(char* sendstr,char* expectstr);
    /*!
      a extension of the Send member function. It handled also
      the abort_string and repeat_string. I think, this will be
      obsolete, so don't use it. Given parameter and return value
      are the same like Send.
    */
    int Sendv(char* sendstr,char* expectstr);
    /*!
      Returns the matched data.
      \return null terminate string of matched data.
    */
    char* Received();
    /*!
      Adjust the timeout interval.
      \param msecs timeout interval in milliseconds
    */
    int Timeout(int msecs);
};

/*! \example texpect.cpp
  This is a simple example of how to use the expect class. You will find it
  in examples/texpect.cpp.
 */

#endif
