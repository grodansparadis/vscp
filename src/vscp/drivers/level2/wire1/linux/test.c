// readw1.cpp : 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP Project (http://www.vscp.org) 
//
// Copyright (C) 2000-2018 Ake Hedman, 
// Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>

int main()
{
	FILE * pFile;
	char line1[80];
	char line2[80];
	unsigned int id[9];
	int temperature;

	pFile = fopen ("/sys/bus/w1/devices/10-00080192afa8/w1_slave" , "r");
	if (pFile == NULL) {
		perror ("Error opening file\n");
		return -1;
   }
   else {
	
		// Read line 1
		if ( fgets( line1, sizeof( line1 ), pFile ) != NULL ) {
			putchar('|');
			puts( line1 );
			putchar('|');
		}
		else {
			printf("Failed to read line 1");
			
			// Close the file
			fclose (pFile);
			
			return -1;
		}
	
		// Read line 2
		if ( fgets ( line2, sizeof( line2 ) , pFile ) != NULL ) {
			putchar('*');
			puts( line2 );
			putchar('*');
		}
		else {
			printf("Failed to read line 2");
			
			// Close the file
			fclose (pFile);
			
			return -1;
		}
	
		// Close the file
		fclose (pFile);
		
		// 08 00 4b 46 ff ff 0d 10 ff : crc=ff YES
		sscanf( line1, "%02x %02x %02x %02x %02x %02x %02x %02x %02x",
				&id[0],&id[1],&id[2],&id[3],&id[4],&id[5],&id[6],&id[7],&id[8] );
		
		printf("\n\n%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
				id[0],id[1],id[2],id[3],id[4],id[5],id[6],id[7],id[8]);
		
		if ( NULL != strstr( line1, "YES" ) ) {
			printf("YES found\n");
		}	
		
		sscanf( line2, "%*s %*s %*s %*s %*s %*s %*s %*s %*s t=%d", &temperature );
				
		printf("Temperature is %f \n", (float)temperature/1000 );
   }
   
	return 0;
}
