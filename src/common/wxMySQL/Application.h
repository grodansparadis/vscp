#ifndef _APPLICATION_H
#define _APPLICATION_H

#include "Main.h"

class wxMySQLTestApp : public wxApp
{	
public:
	virtual bool OnInit();
};

IMPLEMENT_APP(wxMySQLTestApp)

#endif