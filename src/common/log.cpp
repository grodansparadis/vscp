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
// log.cpp: implementation for the log class
//

#include "log.h"


namespace mflogger {

Log::Log(const std::string& Logfilename, const bool& append):
                logfilename(Logfilename), strftime_format("%G-%m-%dT%H:%M:%S "), bufflen(1000) {
   if(Logfilename.length())
      open(Logfilename, append);
}

Log::~Log() {
   logfile.close(); // not necassary, just seems write to do this.
}

bool Log::open(const std::string& Logfilename, const bool& append) {
   assert(Logfilename.length());

   logfile.close(); // be a good citizen
   logfile.clear(); // clean up flags
   logfilename = Logfilename;
   if(append)
      logfile.open(logfilename.c_str(), std::ios::binary | std::ios::out | std::ios::app);
   else
      logfile.open(logfilename.c_str(), std::ios::binary | std::ios::out);

   assert(!logfile.bad());
   return isgood();
}

std::string Log::log(const std::string& message, bool timeasstring) {
   assert(logfile.good());
   assert(message.length());
   assert(strftime_format.length());
   std::string retvalue;

   if(logfile.good()) {
   	  std::ostringstream b;
      clock_gettime(CLOCK_REALTIME, &t);
      if(timeasstring) {
         tm* tmstruct = localtime(&t.tv_sec);
         char buff[bufflen];
         strftime(buff, bufflen, strftime_format.c_str(), tmstruct);
         logfile << buff;
         b << buff;
      }
      else {
   	     logfile << "[" << t.tv_sec << "] ";
   	     b << "[" << t.tv_sec << "] ";
      }
   	  retvalue += b.str();
      logfile << message << "\n";
      retvalue += message;
   }
   return retvalue;
}

bool Log::isgood() const {
   return logfile.good();
}

Log& Log::operator<<(const char* in) {   
   if(logfile.good()) {
      clock_gettime(CLOCK_REALTIME, &t);
      tm* tmstruct = localtime(&t.tv_sec);
      char buff[bufflen];
      strftime(buff, bufflen, strftime_format.c_str(), tmstruct);
      logfile << buff;
      logfile << in << "\n";
   }
   return *this;
}
Log& Log::operator<<(const std::string& in) {
   if(logfile.good()) {
      clock_gettime(CLOCK_REALTIME, &t);
      tm* tmstruct = localtime(&t.tv_sec);
      char buff[bufflen];
      strftime(buff, bufflen, strftime_format.c_str(), tmstruct);
      logfile << buff;
      logfile << in << "\n";
   }
   return *this;
}
Log& Log::operator<<(char in) {
   if(logfile.good()) {
      clock_gettime(CLOCK_REALTIME, &t);
      tm* tmstruct = localtime(&t.tv_sec);
      char buff[bufflen];
      strftime(buff, bufflen, strftime_format.c_str(), tmstruct);
      logfile << buff;
      logfile << in << "\n";
   }
   return *this;
}
Log& Log::operator<<(int in) {
   if(logfile.good()) {
      clock_gettime(CLOCK_REALTIME, &t);
      tm* tmstruct = localtime(&t.tv_sec);
      char buff[bufflen];
      strftime(buff, bufflen, strftime_format.c_str(), tmstruct);
      logfile << buff;
      logfile << in << "\n";
   }
   return *this;
}
Log& Log::operator<<(short in) {
   if(logfile.good()) {
      clock_gettime(CLOCK_REALTIME, &t);
      tm* tmstruct = localtime(&t.tv_sec);
      char buff[bufflen];
      strftime(buff, bufflen, strftime_format.c_str(), tmstruct);
      logfile << buff;
      logfile << in << "\n";
   }
   return *this;
}
Log& Log::operator<<(long in) {
   if(logfile.good()) {
      clock_gettime(CLOCK_REALTIME, &t);
      tm* tmstruct = localtime(&t.tv_sec);
      char buff[bufflen];
      strftime(buff, bufflen, strftime_format.c_str(), tmstruct);
      logfile << buff;
      logfile << in << "\n";
   }
   return *this;
}
Log& Log::operator<<(float in) {
   if(logfile.good()) {
      clock_gettime(CLOCK_REALTIME, &t);
      tm* tmstruct = localtime(&t.tv_sec);
      char buff[bufflen];
      strftime(buff, bufflen, strftime_format.c_str(), tmstruct);
      logfile << buff;
      logfile << in << "\n";
   }
   return *this;
}
Log& Log::operator<<(double in) {
   if(logfile.good()) {
      clock_gettime(CLOCK_REALTIME, &t);
      tm* tmstruct = localtime(&t.tv_sec);
      char buff[bufflen];
      strftime(buff, bufflen, strftime_format.c_str(), tmstruct);
      logfile << buff;
      logfile << in << "\n";
   }
   return *this;
}
Log& Log::operator<<(long double in) {
   if(logfile.good()) {
      clock_gettime(CLOCK_REALTIME, &t);
      tm* tmstruct = localtime(&t.tv_sec);
      char buff[bufflen];
      strftime(buff, bufflen, strftime_format.c_str(), tmstruct);
      logfile << buff;
      logfile << in << "\n";
   }
   return *this;
}
Log& Log::operator<<(unsigned int in) {
   if(logfile.good()) {
      clock_gettime(CLOCK_REALTIME, &t);
      tm* tmstruct = localtime(&t.tv_sec);
      char buff[bufflen];
      strftime(buff, bufflen, strftime_format.c_str(), tmstruct);
      logfile << buff;
      logfile << in << "\n";
   }
   return *this;
}
Log& Log::operator<<(unsigned short in) {
   if(logfile.good()) {
      clock_gettime(CLOCK_REALTIME, &t);
      tm* tmstruct = localtime(&t.tv_sec);
      char buff[bufflen];
      strftime(buff, bufflen, strftime_format.c_str(), tmstruct);
      logfile << buff;
      logfile << in << "\n";
   }
   return *this;
}
Log& Log::operator<<(unsigned long in) {
   if(logfile.good()) {
      clock_gettime(CLOCK_REALTIME, &t);
      tm* tmstruct = localtime(&t.tv_sec);
      char buff[bufflen];
      strftime(buff, bufflen, strftime_format.c_str(), tmstruct);
      logfile << buff;
      logfile << in << "\n";
   }
   return *this;
}

void Log::strftime_prefix(const std::string& FORMAT) {
   if( (FORMAT.length() < bufflen) && (FORMAT.length()) )
	   strftime_format = FORMAT;
}

LogString::LogString(): strftime_format("%G-%m-%dT%H:%M:%S "), bufflen(1000) {
   // nothing to do
}

LogString::~LogString() {
   // nothing to do
}

// log message with timestamp/prefix. return the whole log line
std::string LogString::logstring(const std::string& message, bool timeasstring) {
   std::string retvalue;

   std::ostringstream b;
   clock_gettime(CLOCK_REALTIME, &t);
   if(timeasstring) {
      tm* tmstruct = localtime(&t.tv_sec);
      char buff[bufflen];
      strftime(buff, bufflen, strftime_format.c_str(), tmstruct);
      b << buff;
   }
   else {
   	  b << "[" << t.tv_sec << "] ";
   }
   retvalue += b.str();
   retvalue += message;

   return retvalue;
}
   
void LogString::strftime_prefix(const std::string& FORMAT) {
   if( (FORMAT.length() < bufflen) && (FORMAT.length()) )
	   strftime_format = FORMAT;
}

} // end namespace mflogger
