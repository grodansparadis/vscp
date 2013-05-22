// test.c : 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2007 Johan Hedlund,  <kungjohan@gmail.com>
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

#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <assert.h>


int main()
{
    double val;
    // /sys/class/hwmon/hwmon0/temp1_input
    char str[999];
    FILE *file;
    // Processor core temp
    file = fopen( "/sys/class/hwmon/hwmon0/temp1_input" , "r");
    if (file) {
        while (fscanf(file, "%s", str)!=EOF)
            printf("%s\n",str);
            val = atof( str )/1000;
            printf("%f\n",val);
            printf("%g\n",val);
        fclose(file);
    }
    return EXIT_SUCCESS;
}

