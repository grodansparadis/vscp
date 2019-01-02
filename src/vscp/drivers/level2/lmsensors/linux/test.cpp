// test.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// Copyright (C) 2000-2019 Ake Hedman,
// Ake Hedman, Grodans Paradis AB,<akhe@grodansparadis.com>
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
//

#include <string>

#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <vscp_class.h>
#include <vscp_type.h>
#include <vscphelper.h>
#include <vscpremotetcpif.h>

#include "vscpl2drv_lmsensors.h"
#include "lmsensors.h"

int main()
{
	double val;
	// Temp for cpu core on this machine
	// /sys/class/hwmon/hwmon0/temp1_input
	char str[999];
	FILE *f;
		
	// Processor core0 temp
	f = fopen("/sys/class/hwmon/hwmon1/device/temp2_input", "r");
	if (f) {
		if (fscanf(f, "%s", str) != EOF) {
			printf("raw:%s\n", str);
		}
		val = atof(str) / 1000;
		printf("float:%f\n", val);
		printf("truncated:%g\n", val);
		fclose(f);
	}
	
	return EXIT_SUCCESS;
}

