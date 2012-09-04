/////////////////////////////////////////////////////////////////////////////
// Name:        gpibx.h
// Purpose:     base class for gpib devices
// Author:      Joachim Buermann
// Id:          $Id: gpibx.h,v 1.2 2004/11/30 12:40:35 jb Exp $
// Copyright:   (c) 2001,2004 Joachim Buermann
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_GPIBX_H
#define __WX_GPIBX_H

#include "wx/ctb/iobase.h"

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

/*!
  \struct wxGPIB_DCS

  The device control struct for the gpib communication class.
  This struct should be used, if you refer advanced parameter.
*/
struct wxGPIB_DCS
{
    /*! primary address of GPIB device */
    int m_address1;
    /*! secondary address of GPIB device */
    int m_address2;
    /*! I/O timeout */
    wxGPIB_Timeout m_timeout;
    /*! EOT enable */
    bool m_eot;
    /*! EOS character */
    unsigned char m_eosChar;
    /*! EOS handling */
    unsigned char m_eosMode;
    wxGPIB_DCS() {
	   /*! set default device address to 1 */
	   m_address1 = 1;
	   m_address2 = 0;
	   m_timeout = wxGPIB_TO_3s;
	   m_eot = true;
	   m_eosChar = '\n';
	   m_eosMode = 0;
    };
    char* GetSettings(char* buf,size_t bufsize);
}; 

/*!
  \enum IOCTL calls for wxGPIB
  
  The following Ioctl calls are only valid for the wxGPIB
  class.
*/
enum {
    /*!
	 Set the adress of the via gpib connected device.
    */
    CTB_GPIB_SETADR = CTB_GPIB,
    /*!
	 Get the serial poll byte
    */
    CTB_GPIB_GETRSP,
    /*!
	 Get the GPIB status
    */
    CTB_GPIB_GETSTA,
    /*!
	 Get the last GPIB error number
    */
    CTB_GPIB_GETERR,
    /*!
	 Get the GPIB line status (hardware control lines) as an
	 integer. The lowest 8 bits correspond to the current state
	 of the lines.
    */
    CTB_GPIB_GETLINES,
    /*!
	 Set the GPIB specific timeout
    */
    CTB_GPIB_SETTIMEOUT,
    /*!
	 Forces the specified device to go to local program mode
    */
    CTB_GPIB_GTL,
    /*!
	 This routine can only be used if the specified GPIB 
	 Interface Board is the System Controller.
	 Remember that even though the REN line is asserted, 
	 the device(s) will not be put into remote state until is
	 addressed to listen by the Active Controller
    */
    CTB_GPIB_REN
};

/*!
   \class wxGPIB
   
   \brief the gpib base class
*/
class wxGPIB_x : public wxIOBase
{
protected:
    int m_board;
    int m_hd;
    int m_state;
    int m_error;
    int m_count;
    int m_asyncio;
    wxGPIB_DCS m_dcs;
public:
    wxGPIB_x() {
	   m_board = -1;
	   m_hd = -1;
	   m_asyncio = 0;
	   m_state = m_count = m_error = 0;
    };
    virtual ~wxGPIB_x() {};
    const char* ClassName() {return "wxGPIB";};
    virtual int GetError(char* buf,size_t buflen);
    virtual int GetSettingsAsString(char* str, size_t size) = 0;
    virtual int Ioctl(int cmd,void* args);
    int IsOpen() {
	   return m_hd >= 0;
    };
    int Read(char* buf,size_t len);
    int Write(char* buf,size_t len);
};

#endif
