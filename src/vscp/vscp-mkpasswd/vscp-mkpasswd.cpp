///////////////////////////////////////////////////////////////////////////////
// vscp-mkpasswd.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright (C) 2000-2025
// Ake Hedman, the VSCP project,<info@vscp.org>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#include <string>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>

#include <vscp.h>
#include <vscp-aes.h>
#include <fastpbkdf2.h>
#include "vscp-mkpasswd.h"

int main(int argc, char **argv)
{
    int i;
    std::string password;
    uint8_t salt[16];
    uint8_t buf[32];

    if (argc < 2) {
        printf("Make storable password hash for VSCP & Friends.\n");
        printf("Format is \n");
        printf("    mkpasswd 'password'\n");
        return -1;
    }

    password = argv[1];
    if (password.size() < 12) {
        printf("It is encouraged to use passwords with a length > 12 bytes.\n");
    }

    // Get random IV
    if (16 != getRandomIV(salt, 16)) {
        printf("Unable to generate IV. Terminating.\n");
        return -1;
    }

    fastpbkdf2_hmac_sha256((const uint8_t *)argv[1], strlen(argv[1]),
                            salt, 16,
                            70000,
                            buf, 32 );

    printf("\nresult = ");
    for ( i=0; i<16; i++ ) {
        printf("%02X", salt[i]);
    }

    printf(";");
    for ( i=0; i<32; i++ ) {
        printf("%02X", buf[i]);
    }
    
    printf("\n");


    return 0;
}
