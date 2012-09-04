/////////////////////////////////////////////////////////////////////////////
// Name:        ctb.h
// Purpose:
// Author:      Joachim Buermann
// Id:          $Id: ctb.h,v 1.1.1.1 2004/11/24 10:30:11 jb Exp $
// Copyright:   (c) 2001 Joachim Buermann
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

/*!
  \mainpage ctb overview

  ctb (communication toolbox) was realized, to simplify the
  communication with other instruments throughout the serial com
  ports. To make my life easier, it should works with linux (I
  develope my application with linux), and all win32 plattforms
  (excepted windows 3.1, which is a only 16bit OS).

  On the base ctb defines a abstract class wxIOBase, which must be derivate
  for several interfaces (at now this was done only for the RS232
  comports).
 
  ctb is composed of four parts:
  \li \c wxSerialPort class
  \li \c timer class
  \li \c expect class, using
  \li \c match functions
 
  \section wxSerialPort wxSerialPort class

  The class for the serial ports is named as wxSerialPort.
  wxSerialPort is a wrapper for non blocked reading and writing. This
  is easy under linux, but with windows a lot more tricky.
  wxSerialPort is as simple as possible. It doesn't create any gui
  events or signals, so it works also standalone. It's also not a
  device driver, means, you must call the read method, if you look for
  receiving data.

  You can write any desired data with any length (length type is
  size_t, i think, on win32 and linux this is a 32Bit integer) and
  wxSerialPort returns the really writen data length, also you can
  read a lot of data and wxSerialPort returns the really received data
  count.
  
  Both, read and write returns immediatelly. Using these, the program
  never blocks.

  Also dev implements a blocked read and write. You can use these
  functions, if you want a definitiv count of data and never accept
  less than this. Because there is a difficulty, when the communication
  is interrupted or death, both blocked functions get a timeout flag
  to returns after a given time interval.

  The timeouts will be handled with the second timer class.

  \section timer timer class
  
  The idea of the timer class is to base on the Unix C alarm function.
  You create a timer with a given alarm time and a adress of a flag,
  which the timer must set after the time is over.
  
  Because the alarm function cannot used more than once in the same
  process (under windows I don't know a similar function), every
  timer instance will be a separate thread after starting it.  So you
  can start a timer and continue in your program, make a lot of
  things and test the flag whenever you want this. (For example,
  you read/write a given count of data) *1.
  
  Mostly you have the situation to send a special string to an
  instrument and wait for an specific answer. For example, you say 
  'do that' and the instrument answered with 'OK' or 'ERROR'.
  (This is the scenario for the wonderful expect tool (I know it only
  within linux...)

  And this is the third part of ctb.

  \remarks *1 I think, it's a better style, to request a given count
  of data in 100ms (for example) and trap the situation, if there are
  not enough data after this time. And not do this for every byte!

  \section expect Expect class

  The Expect class will be connected with a dev class. (At now this
  only must the devCUA class, but everyone can derivate new classes
  from the dev class, for handling another interfaces).

  You can call Expect with a given 'do that' instruction string, and
  a regular expression, which must match the answer. Expect than send
  the instuction and wait a given timeout for the answer. The 
  matching mechanism is stream based, so Expect must not have a given
  count of characters, to decide.

  For example you can do: \c Expect::Send("","[Ll]ogin") and Expect
  will return true, if it received "login" or "Login". You also can
  do: \c Expect::Send("#","^[*?!]{1}"), means Expect sends a '#' and
  expects a '*', '!' or '?' at the first character. (The regular
  expression rules are similar to perl (but beware to think, it's
  the same or bug free...). They are also in process.

  \section match match
  The file match.cpp promise a stream base string matching. It was
  used in main by the expect class, but you can also take the functions
  for other things, like input watching in a text field and so one...
  There is a powerful regular expression library called regex or
  similar, but I don't get it to run under windows. And I don't know,
  if it is stream based. So I write it for myself (and write if
  furthermore, because there are also some known bugs).

  \section last last not least
  ctb works for me, I never say, it works without an error. I'm also
  busy doing other things, so if everyone want to help to speed things
  up, please let me know.
*/
