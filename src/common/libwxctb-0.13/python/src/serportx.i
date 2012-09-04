%{
#include "wx/ctb-0.13/serportx.h"
%}

%include iobase.i

enum wxBaud
{
    wxBAUD_150=150,
    wxBAUD_300=300,
    wxBAUD_600=600,
    wxBAUD_1200=1200,
    wxBAUD_2400=2400,
    wxBAUD_4800=4800,
    wxBAUD_9600=9600,
    wxBAUD_19200=19200,
    wxBAUD_38400=38400,
    wxBAUD_57600=57600,
    wxBAUD_115200=115200,
    wxBAUD_230400=230400,
    wxBAUD_460800=460800,
    wxBAUD_921600=921600
};

enum wxParity
{
    wxPARITY_NONE,
    wxPARITY_ODD,
    wxPARITY_EVEN,
    wxPARITY_MARK,
    wxPARITY_SPACE
};

enum wxSerialLineState
{
    wxSERIAL_LINESTATE_DCD = 0x040,
    wxSERIAL_LINESTATE_CTS = 0x020,
    wxSERIAL_LINESTATE_DSR = 0x100,
    wxSERIAL_LINESTATE_DTR = 0x002,
    wxSERIAL_LINESTATE_RING = 0x080,
    wxSERIAL_LINESTATE_RTS = 0x004,
    wxSERIAL_LINESTATE_NULL = 0x000
};

struct wxSerialPort_DCS
{
    wxBaud baud;
    wxParity parity;
    unsigned char wordlen;
    unsigned char stopbits;
    bool rtscts;
    bool xonxoff;
    char buf[16];
    wxSerialPort_DCS();
    ~wxSerialPort_DCS();
    char* GetSettings();
}; 

struct wxSerialPort_EINFO
{
    int brk;
    int frame;
    int overrun;
    int parity;
    wxSerialPort_EINFO();
};

enum {
    CTB_SER_GETEINFO = CTB_SERIAL,
    CTB_SER_GETBRK,
    CTB_SER_GETFRM,
    CTB_SER_GETOVR,
    CTB_SER_GETPAR,
    CTB_SER_GETINQUE
};

class wxSerialPort_x : public wxIOBase
{
protected:
    wxSerialPort_DCS m_dcs;
    char m_devname[WXSERIALPORT_NAME_LEN];
public:
    wxSerialPort_x();
    virtual ~wxSerialPort_x();
    const char* ClassName();
    virtual int ChangeLineState(wxSerialLineState flags) = 0;
    virtual int ClrLineState(wxSerialLineState flags) = 0;
    virtual int GetLineState() = 0;
    virtual char* GetSettingsAsString();
    virtual int Ioctl(int cmd,void* args);
    virtual int SendBreak(int duration) = 0;
    virtual int SetBaudRate(wxBaud baudrate) = 0;
    virtual int SetLineState(wxSerialLineState flags) = 0;
};
