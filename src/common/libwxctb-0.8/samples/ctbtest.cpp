#include "wx/ctb/getopt.h"
#if ( GPIB )
# include "wx/ctb/gpib.h"
#endif
#include "wx/ctb/iobase.h"
#include "wx/ctb/serport.h"
#include "wx/ctb/timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ----------------- options -------------------------------
const char* options="b:hd:";

const char* helpMessage =
{
    "A simple serial port and GPIB class test\n"
    "Simply sends \"*idn?\" throughout the given interface and\n"
    "reads the answer\n"
    "(SCPI command for identify the connected device)\n"
    "ctbtest [options]\n"
    "available options are:\n"
    "-b     : baudrate [1200...115200], default is 38400\n"
    "-d     : connected device, default is COM1\n"
    "-h     : print this\n"
    "-t     : communication timeout in ms (default is 1000ms)\n"
};

// ----------------- globals -------------------------------
wxBaud baudrate = wxBAUD_38400;
char *devname = wxCOM1;
int timeout = 1000;

int main(int argc,char* argv[])
{
    int quit = 0;
    int val;

    while ((val=getopt(argc,argv,(char*)options))!=EOF) {
	   switch (val) {
	   case 'b' : baudrate = (wxBaud)strtol(optarg,NULL,10); break;
	   case 'd' : devname = optarg; break;
	   case 'h' : printf(helpMessage); exit(0);
	   case 't' : timeout = strtol(optarg,NULL,10); break;
	   }
    }
    
    // like a virtual instrument in NI
    wxIOBase* dev;

#if ( GPIB )
    if(!strncmp(devname,wxGPIB1,strlen(wxGPIB1))) {
	   // device is GPIB
	   dev = new wxGPIB();
	   // try to open the device at address 1 (default)
	   if(dev->Open(devname) < 0) {
		  printf("Cannot open %s\n",devname);
		  delete dev;
		  return -1;
	   }
    }
    else {
#endif
	   // device is a serial port
	   dev = new wxSerialPort();
	   // try to open the given port
	   if(dev->Open(devname) < 0) {
		  printf("Cannot open %s\n",devname);
		  delete dev;
		  return -1;
	   }
	   // set the baudrate
	   ((wxSerialPort*)dev)->SetBaudRate(baudrate);
#if ( GPIB )
    }
#endif
    // ok, device is ready for communication
    
    // we are using a timer for timeout test. After the given timeout
    // is reached, the to value will be set to 1
    int to = 0;
    timer t(timeout,&to,NULL);
    t.start();
    dev->Writev("*idn?\n",6,&to);

    char* receivedBytes = NULL;
    int rd = dev->ReadUntilEOS(&receivedBytes,"\n",1000);
    if(rd > 0) {
	   receivedBytes[rd] = 0;
	   printf("answer (%i): %s\n",rd,receivedBytes);
    }
    dev->Close();
    delete dev;
    return 0;
}
