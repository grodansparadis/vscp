#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include <vscphelper.h>

int main()
{
    std::string value = "  MIXED  ";
    vscp_trim(value);
    vscp_makeLower(value);
    return value == "mixed" ? 0 : 1;
}
