%{
#include "wx/ctb-0.13/gpib.h"
%}

%include iobase.i

%typemap(in) void * dcs (wxGPIB_DCS tmp) {
	/* dont check for list */
	$1 = &tmp;
}

enum wxGPIB_Timeout
{
    wxGPIB_TO_NONE = 0, 
    wxGPIB_TO_10us, 
    wxGPIB_TO_30us, 
    wxGPIB_TO_100us,
    wxGPIB_TO_300us,
    wxGPIB_TO_1ms,  
    wxGPIB_TO_3ms,  
    wxGPIB_TO_10ms, 
    wxGPIB_TO_30ms, 
    wxGPIB_TO_100ms,
    wxGPIB_TO_300ms,
    wxGPIB_TO_1s,   
    wxGPIB_TO_3s,   
    wxGPIB_TO_10s,  
    wxGPIB_TO_30s,  
    wxGPIB_TO_100s, 
    wxGPIB_TO_300s, 
    wxGPIB_TO_1000s
};

struct wxGPIB_DCS
{
    int m_address1;
    int m_address2;
    wxGPIB_Timeout m_timeout;
    bool m_eot;
    unsigned char m_eosChar;
    unsigned char m_eosMode;
    wxGPIB_DCS();
    ~wxGPIB_DCS();
    char* GetSettings();
}; 

enum {
    CTB_GPIB_SETADR = CTB_GPIB,
    CTB_GPIB_GETRSP,
    CTB_GPIB_GETSTA,
    CTB_GPIB_GETERR,
    CTB_GPIB_GETLINES,
    CTB_GPIB_SETTIMEOUT,
    CTB_GPIB_GTL,
    CTB_GPIB_REN,
    CTB_GPIB_RESET_BUS,
    CTB_GPIB_SET_EOS_CHAR,
    CTB_GPIB_GET_EOS_CHAR,
    CTB_GPIB_SET_EOS_MODE,
    CTB_GPIB_GET_EOS_MODE
};

class wxGPIB : public wxIOBase
{
protected:
    int m_board;
    int m_hd;
    int m_state;
    int m_error;
    int m_count;
    int m_asyncio;
    wxGPIB_DCS m_dcs;
    int CloseDevice();
    int OpenDevice(const char* devname, void* dcs);
    virtual const char* GetErrorString(int error,bool detailed);
public:
    wxGPIB();
    virtual ~wxGPIB();
    const char* ClassName();
    virtual const char* GetErrorDescription(int error);
    virtual const char* GetErrorNotation(int error);
    virtual char* GetSettingsAsString();
    int Ibrd(char* buf,size_t len);
    int Ibwrt(char* buf,size_t len);
    virtual int Ioctl(int cmd,void* args);
    int IsOpen();
    int Read(char* buf,size_t len);
    int Write(char* buf,size_t len);

    static int FindListeners(int board = 0);

};
