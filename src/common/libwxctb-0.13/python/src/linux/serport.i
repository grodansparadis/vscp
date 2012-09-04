%module serport

%{
#include "wx/ctb-0.13/linux/serport.h"
%}

%include ../serportx.i

%pythoncode {
wxCOM1 = "/dev/ttyS0"
wxCOM2 = "/dev/ttyS1"
wxCOM3 = "/dev/ttyS2"
wxCOM4 = "/dev/ttyS3"
wxCOM5 = "/dev/ttyS4"
wxCOM6 = "/dev/ttyS5"
wxCOM7 = "/dev/ttyS6"
wxCOM8 = "/dev/ttyS7"
wxCOM9 = "/dev/ttyS8"
};

class wxSerialPort : public wxSerialPort_x
{
protected:
    int fd;
    struct termios t, save_t;
    struct serial_icounter_struct save_info, last_info;
    speed_t AdaptBaudrate(wxBaud baud);
    
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
