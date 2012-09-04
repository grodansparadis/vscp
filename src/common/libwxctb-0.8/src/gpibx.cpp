/////////////////////////////////////////////////////////////////////////////
// Name:        gpibx.cpp
// Purpose:
// Author:      Joachim Buermann
// Id:          $Id: gpibx.cpp,v 1.1.1.1 2004/11/24 10:30:11 jb Exp $
// Copyright:   (c) 2001,2004 Joachim Buermann
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/ctb/gpibx.h"
#include <stdio.h>
#include <string.h>
#ifdef WIN32
# include "wx/ctb/win32/gpib-32.h"
# define snprintf _snprintf
#elif __GNUG__
# include <gpib/ib.h>
#endif

#define ASYNCRD 0x01
#define ASYNCWR 0x02

struct gpibErr_t {
    int m_errno;
    const char* m_err;
};

static gpibErr_t gpibErrors[] = {
    {0,"EDVR"},			  // DOS Error
    {1,"ECIC"},			  // Specified  GPIB Interface Board is Not 
						  // Active Controller
    {2,"ENOL"},			  // No present listing device
    {3,"EADR"},			  // GPIB Board has not been addressed properly
    {4,"EARG"},			  // Invalid argument
    {5,"ESAC"},			  // Specified GPIB Interface Board is not 
                                // System Controller
    {6,"EABO"},			  // I/O operation aborted (time-out)
    {7,"ENEB"},			  // Non-existent GPIB board
    {10,"EOIP"},			  // Routine not allowed during asynchronous 
                                // I/O operation
    {11,"ECAP"},			  // No capability for operation
    {12,"EFSO"},			  // File System Error
    {14,"EBUS"},			  // Command byte transfer error
    {15,"ESTB"},			  // Serial poll status byte lost
    {16,"ESQR"},			  // SRQ stuck in ON position
    {20,"ETAB"},			  // Table problem
    {247,"EINT"},			  // No interrupt configured on board
    {248,"EWMD"},			  // Windows is not in Enhanced mode
    {249,"EVDD"},			  // GPIB driver is not installed
    {250,"EOVR"},			  // Buffer Overflow
    {251,"ESML"},			  // Two library calls running simultaneously
    {252,"ECFG"},			  // Board type does not match GPIB.CFG
    {253,"ETMR"},			  // No Windows timers available
    {254,"ESLC"},			  // No Windows selectors available
    {255,"EBRK"}			  // Control-Break pressed
};

char* wxGPIB_DCS::GetSettings(char* buf,size_t bufsize) {
    const char* to[] = {
	   "None","10us","30us","100us","300us","1ms","3ms","10ms","30ms", 
	   "100ms","300ms","1s","3s","10s","30s","100s","300s","1000s"
    };
    snprintf(buf,bufsize,"Adr: (%i,%i) to:%s",
		   m_address1,
		   m_address2,
		   to[m_timeout]);
    return buf;
};

int wxGPIB_x::GetError(char* buf,size_t buflen)
{
    for(size_t i=0;i<(sizeof(gpibErrors)/sizeof(gpibErr_t));i++) {
	   if(gpibErrors[i].m_errno == m_error) {
		  return snprintf(buf,buflen,"%s",gpibErrors[i].m_err);
	   }
    }
    return 0;
};

int wxGPIB_x::Ioctl(int cmd,void* args)
{
    switch(cmd) {
    case CTB_RESET:
	   if(m_hd >= 0) {
		  ibclr(m_hd);
		  return 0;
	   }
	   return -1;
    case CTB_GPIB_GETRSP: {
	   char spr = 0;
	   if(m_hd >= 0) {
		  ibrsp(m_hd,&spr);
		  *(int*)args = (int)spr;
		  return 0;
	   }
	   return 1; }
    case CTB_GPIB_GETSTA:
	   *(int*)args = m_state;
	   return 0;
    case CTB_GPIB_GETERR:
	   *(int*)args = m_error;
	   return 0;
    case CTB_GPIB_GETLINES: {
	   short state = 0;
	   if(m_hd >= 0) {
		  iblines(m_board,&state);
		  *(int*)args = (int)state;
		  return 0;
	   }
	   return -1; }
    case CTB_GPIB_SETTIMEOUT: {
	   if(m_hd >= 0) {
		  wxGPIB_Timeout timeout;
		  unsigned long to = *(unsigned long*)args;
		  // convert the timeout in ms (given by args) into the
		  // traditional NI-488.2 timeout period 
		  if(to > 1000000) timeout = wxGPIB_TO_1000s;
		  else if(to > 300000) timeout = wxGPIB_TO_300s;
		  else if(to > 100000) timeout = wxGPIB_TO_100s;
		  else if(to > 30000) timeout = wxGPIB_TO_30s;
		  else if(to > 10000) timeout = wxGPIB_TO_10s;
		  else if(to > 3000) timeout = wxGPIB_TO_3s;
		  else if(to > 1000) timeout = wxGPIB_TO_1s;
		  else if(to > 300) timeout = wxGPIB_TO_300ms;
		  else if(to > 100) timeout = wxGPIB_TO_100ms;
		  else if(to > 30) timeout = wxGPIB_TO_30ms;
		  else if(to > 10) timeout = wxGPIB_TO_10ms;
		  else if(to > 3) timeout = wxGPIB_TO_3ms;
		  else if(to > 1) timeout = wxGPIB_TO_1ms;
		  else timeout = wxGPIB_TO_NONE;
		  ibtmo(m_hd,timeout);
		  return 0; 
	   }
	   return -1; }
    case CTB_GPIB_GTL:
	   // Forces the specified device to go to local program mode
	   if(m_hd >= 0) {
		  ibloc(m_hd);
		  return 0;
	   }
	   return -1;
    case CTB_GPIB_REN:
	   // This routine can only be used if the specified GPIB 
	   // Interface Board is the System Controller.
	   // Remember that even though the REN line is asserted, 
	   // the device(s) will not be put into remote state until is
	   // addressed to listen by the Active Controller
	   if(m_hd) {
		  char adr = (char)m_dcs.m_address1;
		  ibsre(m_board,1);
		  ibcmd(m_board,&adr,1);
		  return 0;
	   }
	   return -1;
    }
    // error or unknown command
    return -1;
};

int wxGPIB_x::Read(char* buf,size_t len)
{
    static char* rdbuf;
    if(!m_asyncio) {
	   char spr = 0;
	   // ask the device if it is ready (this call blocks with Linux,
	   // so we reduced the serial poll timeout in the OpenDevice
	   // member function (look there)
	   ibrsp(m_hd,&spr);
	   if(spr & 0x10) {
		  // MAV Bit is set (message available), allocate memory on 
		  // heap. (Because the data transfer is asynchrone, the allocated
		  // memory must exist until the transfer finished. The GPIB
		  // device driver copy the data in it's interrupt service
		  // routine!!!)
		  m_asyncio |= ASYNCRD;
		  rdbuf = new char[len];
		  memset(rdbuf,0,len);
		  // start the asynchron data transfer
		  ibrda(m_hd,rdbuf,len);
 	   }
    }
    if(m_asyncio & ASYNCRD) {
	   ibwait(m_hd,0);
	   int state = ThreadIbsta();
	   if((state & CMPL+ERR) == 0) {
		  return 0;
	   }
	   // asynchrone I/O was finished
	   m_asyncio = 0;
	   // save state, error and count
	   m_state = ThreadIbsta();
	   m_count = ThreadIbcnt();
	   m_error = ThreadIberr();

	   memcpy(buf,rdbuf,len);
	   delete rdbuf;
	   return m_count;
    }
    return 0;
};

int wxGPIB_x::Write(char* buf,size_t len)
{
    if(!m_asyncio) {
	   m_asyncio |= ASYNCWR;
	   // start the asynchron data transfer
	   ibwrta(m_hd,buf,len);
    }
    if(m_asyncio & ASYNCWR) {
	   ibwait(m_hd,0);
	   int state = ThreadIbsta();
	   if((state & CMPL+ERR) == 0) {
		  return 0;
	   }
	   m_asyncio = 0;
	   // save state, error and count
	   m_state = ThreadIbsta();
	   m_count = ThreadIbcnt();
	   m_error = ThreadIberr();
	   return m_count;
    }
    // at this time, return 0 Bytes
    return 0;
};
