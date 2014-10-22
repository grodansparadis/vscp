// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../vscphelperlib.h"


int _tmain(int argc, _TCHAR* argv[])
{
    long handle;
    handle = vscphlp_newSession();

    vscphlp_open( handle, 
                 "192.168.1.9:9598",
                 "admin",
                 "secret2" ); 

    vscphlp_noop( handle );

    vscphlp_close( handle );
    vscphlp_closeSession( handle );
   
	return 0;
}

