/////////////////////////////////////////////////////////////////////////////
// Name:        linux/gpib.cpp
// Purpose:
// Author:      Joachim Buermann
// Copyright:   (c) 2001,2004 Joachim Buermann
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/ctb/gpib.h"
#include <stdio.h>
#include <string.h>
#include <gpib/ib.h>

int wxGPIB::CloseDevice()
{
    if(m_hd != -1) {
	   // goto local...
	   ibloc(m_hd);
	   // ...and switch device offline
	   ibonl(m_hd,0);
	   m_hd = -1;
	   m_board = -1;
    }
    return 0;
};

int wxGPIB::GetSettingsAsString(char* buf, size_t bufsize)
{
    char tmpbuf[32];
    return snprintf(buf,bufsize,"%s /dev/gpib%i",
		   m_dcs.GetSettings(tmpbuf,sizeof(tmpbuf)),
		   m_board);
};

int wxGPIB::OpenDevice(const char* devname, void* dcs)
{
    // if dcs isn't NULL, type cast
    if(dcs) m_dcs = *(wxGPIB_DCS*)dcs;
    
    if(strncmp(devname,"gpib1",5) == 0) m_board = 0;
    else if(strncmp(devname,"gpib2",5) == 0) m_board = 1;
    if(m_board < 0) {
	   return -1;
    }
    // check for a valid timeout
    if((unsigned int)m_dcs.m_timeout > wxGPIB_TO_1000s) {
	   m_dcs.m_timeout = wxGPIB_TO_1000s;
    }
    m_hd = ibdev(m_board,
			  m_dcs.m_address1,
			  m_dcs.m_address2,
			  m_dcs.m_timeout,
			  m_dcs.m_eot,
			  (m_dcs.m_eosMode << 8) | m_dcs.m_eosChar);
    if(m_hd < 0) {
	   // no gpib controller installed (not found)
	   return -2;
    }
    // test for a connected listener (device with given address)
    short int listen = 0;
    ibln(m_board,m_dcs.m_address1,NO_SAD,&listen);
    if(!listen) {
	   // no listener at the given address
 	   CloseDevice();
 	   return -3;
    }
    // the default length of time the driver waits for a seriel poll 
    // response is set to 1s (T1s = 11). Because we doesn't want to
    // block for this time, we reduce it to (T10ms = 7)
    ibconfig(m_hd,IbcSPollTime,T10ms);
    // reset device
    ibclr(m_hd);
    // save state, error and count
    m_state = ThreadIbsta();
    m_count = ThreadIbcnt();
    m_error = ThreadIberr();
    // no error
    return 0;
};
