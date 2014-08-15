//
//  mflogger library is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation.
//
//  mflogger library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with mflogger library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//   http://www.mumbleface.net
//
// log.h: interface for the Log class
//

#ifndef __MF_LOG_H__
#define __MF_LOG_H__

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

namespace mflogger {

class Log  {
 public:
   // default time prefix ISO8601: YYYY-MM-DDTHH:MM:SS
   //                          e.g 1983-04-17T14:23:05
   // see strftime for format string
   Log(const std::string& Logfilename, const bool& append = true);
   ~Log();

   // open/create log file for logging
   bool open(const std::string& Logfilename, const bool& append = true);

   // is open ready for log input
   bool isgood() const;

   // log message with timestamp/prefix. return the whole log line
   std::string log(const std::string& message, bool timeasstring = true);

   Log& operator<<(const char* in);
   Log& operator<<(const std::string& in);
   Log& operator<<(char in);
   Log& operator<<(int in);
   Log& operator<<(short in);
   Log& operator<<(long in);
   Log& operator<<(float in);
   Log& operator<<(double in);
   Log& operator<<(long double in);
   Log& operator<<(unsigned int in);
   Log& operator<<(unsigned short in);
   Log& operator<<(unsigned long in);

   // Change the time/date prefix in the log
   // see strftime options for format of the string
   void strftime_prefix(const std::string& FORMAT);
 private:
   timespec t;
   std::string logfilename;
   std::string strftime_format;
   std::ofstream logfile;
   const unsigned int bufflen;

   // disallow copying
   Log(const Log&);
   Log& operator= (const Log& log);
};

class LogString  {
 public:
   // default time prefix ISO8601: YYYY-MM-DDTHH:MM:SS
   //                          e.g 1983-04-17T14:23:05
   // see strftime for format string
   LogString();
   ~LogString();

   // log message with timestamp/prefix. return the whole log line
   std::string logstring(const std::string& message, bool timeasstring = true);

   // Change the time/date prefix in the log
   // see strftime options for format of the string
   void strftime_prefix(const std::string& FORMAT);
 private:
   timespec t;
   std::string strftime_format;
   const unsigned int bufflen;

   // disallow copying
   LogString(const LogString&);
   LogString& operator= (const LogString& logstring);
};

} // end namespace mflogger

// bring in the implementation
#include "log.cpp"

#endif // !ifndef __MF_LOG_H__

