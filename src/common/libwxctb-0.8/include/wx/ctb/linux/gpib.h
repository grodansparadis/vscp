/////////////////////////////////////////////////////////////////////////////
// Name:        linux/gpib.h
// Purpose:
// Author:      Joachim Buermann
// Copyright:   (c) 2001,2005 Joachim Buermann
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GPIB_H
#define _WX_GPIB_H

#include "../gpibx.h"

#define wxGPIB1 "gpib1"
#define wxGPIB2 "gpib2"

/*!
   \class wxGPIB
   
   \brief the linux version
*/
class wxGPIB : public wxGPIB_x
{
protected:
    int CloseDevice();
    int OpenDevice(const char* devname, void* dcs);
public:
    wxGPIB() {};
    ~wxGPIB() {Close();};
    int GetSettingsAsString(char* str, size_t size);
};
#endif
