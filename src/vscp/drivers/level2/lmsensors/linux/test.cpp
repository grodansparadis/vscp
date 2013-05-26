// test.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// Copyright (C) 2000-2013 
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

#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include "wx/cmdline.h"
#include "wx/tokenzr.h"
#include <math.h>

#ifdef WIN32
#include "../common/controlobject.h"
#else

#include <unistd.h>
#include <wchar.h>
#endif
//

#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <assert.h>

#include "wx/wxprec.h"
#include <wx/wx.h>
#include <wx/defs.h>
#include <wx/app.h>
#include <wx/log.h>
#include <wx/stdpaths.h>
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>
#include "../../../../common/vscphelper.h"
#include "../../../../common/vscptcpif.h"
#include "lmsensors.h"

int main()
{
	wxStandardPaths strpath;
	double val;
	// Temp for cpu core on this machine
	// /sys/class/hwmon/hwmon0/temp1_input
	char str[999];
	FILE *f;
	
	wxInitializer initializer;
	
	wxString strcfg;
	
	strcfg = strpath.GetConfigDir();
	printf("Config dir:%s\n",(const char *)strcfg.ToAscii());
	strcfg= strpath.GetDataDir() + _(" ");
	printf("Data dir:%s\n",(const char *)strcfg.ToAscii());
	strcfg = strpath.GetDocumentsDir() + _(" ");;
	printf("Document dir:%s\n",(const char *)strcfg.ToAscii());
	
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
	
	// Open the file
	wxFile file;
	if ( !file.Open(_("/sys/class/hwmon/hwmon1/device/temp2_input"))) {
		syslog(LOG_ERR,
				"%s",
				(const char *) "Workerthread. File to open lmsensors file. Terminating!");
		return -1;
	}
	
	char buf[1024];
	long lval;
	for ( long i=0; i<3; i++ ) {
		
		wxString strIteration;
		strIteration.Printf(_("%d"), i);
		strIteration.Trim();
		
		memset( buf, 0, sizeof(buf));
		file.Seek(0);
		if ( wxInvalidOffset != file.Read(buf, sizeof(buf)) ) {
			
			wxString str = wxString::FromAscii(buf);
			str.ToLong(&lval);
			printf("wxraw:%d %d\n", i, lval);
		}
		
	}
	
	file.Close();
	
	return EXIT_SUCCESS;
}

