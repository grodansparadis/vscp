// example1.c 
//

#include "stdio.h"
#include "../../vscphelperlib.h"


int main(int argc, char* argv[])
{
    long handle;
    handle = vscphlp_newSession();

    if ( VSCP_ERROR_SUCCESS == vscphlp_open( handle, 
                 				"127.0.0.1:9598",
                 				"admin",
    	             			"secret" ) ) {
	printf("\avscphlp_open: Success!\n");
	return -1;
    } 
    else {
        printf("\a * * * * * *  Failed to open connection!  * * * * * * *\n");
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_noop( handle ) ) {
       printf("vscphlp_noop: Success!\n");
    }   
    else {
        printf("\a * * * * * *  Failed noop! * * * * * *  v\n");
    }

    if ( VSCP_ERROR_SUCCESS == vscphlp_close( handle ) ) {
		printf("vscphlp_close: Success!\n");
	}
	
    vscphlp_closeSession( handle );
   
	return 0;
}
