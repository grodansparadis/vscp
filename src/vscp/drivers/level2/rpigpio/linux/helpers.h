#include <algorithm> 
#include <functional> 
#include <vector>
#include <string>
#include <cctype>
#include <locale>

// wxString conversions - https://wiki.wxwidgets.org/Converting_everything_to_and_from_wxString

// String to upper case (in place)
static inline void makeUpper(std::string &s ) 
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) -> unsigned char { return std::toupper(c); });
}

// String to upper case (copying)
static inline std::string makeUpper_copy(std::string s) 
{
    makeUpper(s);
    return s;
}

// String to lower case (in place)
static inline void makeLower(std::string &s) 
{
    std::transform( s.begin(), s.end(),s.begin(), ::tolower );
}

// String to upper case (copying)
static inline std::string makeLower_copy(std::string s) 
{
    makeLower(s);
    return s;
}

// trim from start (in place)
static inline void ltrim(std::string &s) 
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) 
{
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) 
{
    ltrim(s);
    rtrim(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s) 
{
    ltrim(s);
    return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s) 
{
    rtrim(s);
    return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) 
{
    trim(s);
    return s;
}

// split for tokenizer
// https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c
static inline void
split( std::deque<std::string> & theStringVector,  
       const std::string  & theString,
       const std::string  & theDelimiter )
{
    size_t  start = 0, end = 0;

    while ( end != std::string::npos)
    {
        end = theString.find( theDelimiter, start);

        // If at end, use length=maxLength.  Else use length=end-start.
        theStringVector.push_back( theString.substr( start,
                       (end == std::string::npos) ? std::string::npos : end - start));

        // If at end, use start=maxSize.  Else use start=end+delimiter.
        start = (   ( end > (std::string::npos - theDelimiter.size()) )
                  ?  std::string::npos  :  end + theDelimiter.size());
    }
}