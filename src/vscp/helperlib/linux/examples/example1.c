// example1.c 
//

#include "../../vscphelperlib.h"


int main(int argc, char* argv[])
{
    long handle;
    handle = vscphlp_newSession();

    vscphlp_open( handle, 
                 "192.168.1.9:9598",
                 "admin",
                 "secret" ); 

    vscphlp_noop( handle );

    vscphlp_close( handle );
    vscphlp_closeSession( handle );
   
	return 0;
}