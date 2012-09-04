%{
#include "wx/ctb-0.13/win32/serport.h"	
%}

%include ../serportx.i

%pythoncode {
wxCOM1 = "com1"
wxCOM2 = "com2"
wxCOM3 = "com3"
wxCOM4 = "com4"
wxCOM5 = "com5"
wxCOM6 = "com6"
wxCOM7 = "com7"
wxCOM8 = "com8"
wxCOM9 = "com9"
wxCOM10 = "\\\\.\\com10"
wxCOM11 = "\\\\.\\com11"
wxCOM12 = "\\\\.\\com12"
wxCOM13 = "\\\\.\\com13"
wxCOM14 = "\\\\.\\com14"
wxCOM15 = "\\\\.\\com15"
wxCOM16 = "\\\\.\\com16"	
};

class wxSerialPort : public wxSerialPort_x
{
protected:
    HANDLE fd;
    OVERLAPPED ov;
    wxSerialPort_EINFO einfo;
      
    int CloseDevice();
    int OpenDevice(const char* devname, void* dcs);
public:
    wxSerialPort();
    ~wxSerialPort();

    int ChangeLineState(wxSerialLineState flags);
    int ClrLineState(wxSerialLineState flags);
    int GetLineState();
    int Ioctl(int cmd,void* args);
    int IsOpen();
    int Read(char* buf,size_t len);
    int SendBreak(int duration);
    int SetBaudRate(wxBaud baudrate);
    int SetLineState(wxSerialLineState flags);
    int Write(char* buf,size_t len);
};
