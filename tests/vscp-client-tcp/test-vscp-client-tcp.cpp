
#include "vscp-client-tcp.h"


int main()
{
    uint64_t wcyd;
    std::string strLine = "00-00-00-00-00-00-9E-1F";
    
    wcyd = 0;
    size_t pos;
    for ( int i=0; i<8; i++) {
        wcyd = wcyd << 8;
        printf("%s\n",strLine.c_str());
        wcyd += std::stoul(strLine,&pos,16);
        if ( 7 != i) {
            strLine = strLine.substr(pos+1);  // Take away first part of str "zz:"
        }
    }
    printf( "%" PRIx64 "\n",wcyd);
    return 0;
}
