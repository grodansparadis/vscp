///////////////////////////////////////////////////////////////////////////////
// noop.cpp
//
// https://www.vscp.org   the VSCP project   info@vscp.org
//https://www.vscp.org/docs/vscphelper/doku.php?id=classes_vscpremotetcpipif_metods

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <vscpremotetcpif.h>
#include <vscphelper.h>

// Output text for happy moments if set to one
#define verbose_success		0

// Output text for sad moments if set to one
#define verbose_error		1

// Output text for talky moments if set to 1
#define verbose_info		1

// Standard connection (a VSCP daemon must be running here)
#define INTERFACE1_HOST 	"127.0.0.1:9598"
#define INTERFACE1_USER 	"admin"
#define INTERFACE1_PASSWORD "secret"


// Count errors in the test
int error_cnt = 0;

int main(int argc, char* argv[])
{
    int rv;
    char *pHost = (char *)INTERFACE1_HOST;
    char *pUser = (char *)INTERFACE1_USER;
    char *pPassword = (char *)INTERFACE1_PASSWORD;

    VscpRemoteTcpIf m_vscpif;

    if ( argc > 1 ) {

        // Host ("localhost:9598"")
        if ( argc >= 2 ) {
            pHost = argv[1];
        }

        // Username ("admin)
        if ( argc >= 3 ) {
            pUser = argv[2];
        }

        // Password ("secret")
        if ( argc >= 4 ) {
            pPassword = argv[3];
        }
    }


    if ( verbose_info ) {
       printf("- VSCP remote interface class test program\n");
       printf("- ========================================\n\n\n");
    }

    // Open the interface
    if ( CANAL_ERROR_SUCCESS != m_vscpif.doCmdOpen( pHost, pUser, pPassword ) ) {
        printf("Error: Failed to open interface - closing.\n");
        return -1;
    }

    if ( verbose_info ) printf("- Connected to server.\n");


    // ------ Do work here ------


    // NOOP command
    if ( verbose_info ) printf("- Executing NOOP command on server.\n");
    if ( CANAL_ERROR_SUCCESS != m_vscpif.doCmdNOOP() ) {
        printf("Error: Failed to run NOOP command.\n");
        error_cnt++;
    }


    // ------ Do work here ------

    // Close the connection
    if ( verbose_info ) printf("- Disconnecting from server.\n");
    if ( CANAL_ERROR_SUCCESS != m_vscpif.doCmdClose() ) {
        printf("Error: Failed to close interface\n");
        error_cnt++;
    }

    printf("\n\nHit ENTER to terminate Error Count= %d\n", error_cnt );
    (void)getchar();

    return 0;

}
